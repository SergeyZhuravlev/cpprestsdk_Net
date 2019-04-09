/***
* Copyright (C) Microsoft. All rights reserved.
* Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
*
* =+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
*
* HTTP Library: Request and reply message definitions.
*
* For the latest on this and related APIs, please see: https://github.com/Microsoft/cpprestsdk
*
* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
****/
#include "stdafx.h"
#include "..\..\include\cpprest\details\internal_http_helpers.h"
#include <pplinterface.h>

#undef min
#undef max

using namespace web;
using namespace utility;
using namespace concurrency;
using namespace utility::conversions;
using namespace http::details;

namespace web { namespace http
{

#define CRLF _XPLATSTR("\r\n")

namespace details {

utility::string_t flatten_http_headers(const http_headers &headers)
{
    utility::string_t flattened_headers;
    for (auto iter = headers.begin(); iter != headers.end(); ++iter)
    {
        flattened_headers.append(iter->first);
        flattened_headers.push_back(':');
        flattened_headers.append(iter->second);
        flattened_headers.append(CRLF);
    }
    return flattened_headers;
}

#if defined(_WIN32)
void parse_headers_string(_Inout_z_ utf16char *headersStr, http_headers &headers)
{
    utf16char *context = nullptr;
    utf16char *line = wcstok_s(headersStr, CRLF, &context);
    while (line != nullptr)
    {
        const utility::string_t header_line(line);
        const size_t colonIndex = header_line.find_first_of(_XPLATSTR(":"));
        if (colonIndex != utility::string_t::npos)
        {
            utility::string_t key = header_line.substr(0, colonIndex);
            utility::string_t value = header_line.substr(colonIndex + 1, header_line.length() - colonIndex - 1);
            http::details::trim_whitespace(key);
            http::details::trim_whitespace(value);
            headers.add(key, value);
        }
        line = wcstok_s(nullptr, CRLF, &context);
    }
}
#endif
}

static bool is_content_type_one_of(const utility::string_t *first, const utility::string_t *last, const utility::string_t &value)
{
    while (first != last)
    {
        if (utility::details::str_icmp(*first, value))
        {
            return true;
        }
        ++first;
    }
    return false;
}

namespace details
{
	/// <summary>
	/// Determines whether or not the given content type is 'textual' according the feature specifications.
	/// </summary>
	bool is_content_type_textual(const utility::string_t &content_type)
	{
#if !defined(_WIN32) || _MSC_VER >= 1900
		static const utility::string_t textual_types[] = {
			mime_types::message_http,
			mime_types::application_json,
			mime_types::application_xml,
			mime_types::application_atom_xml,
			mime_types::application_http,
			mime_types::application_x_www_form_urlencoded
		};
#endif

		if (content_type.size() >= 4 && utility::details::str_icmp(content_type.substr(0, 4), _XPLATSTR("text")))
		{
			return true;
		}
		return (is_content_type_one_of(std::begin(textual_types), std::end(textual_types), content_type));
	}

	// Remove once VS 2013 is no longer supported.
#if defined(_WIN32) && _MSC_VER < 1900
// Not referring to mime_types to avoid static initialization order fiasco.
	static const utility::string_t json_types[] = {
		U("application/json"),
		U("application/x-json"),
		U("text/json"),
		U("text/x-json"),
		U("text/javascript"),
		U("text/x-javascript"),
		U("application/javascript"),
		U("application/x-javascript")
	};
#endif

	/// <summary>
	/// Determines whether or not the given content type is JSON according the feature specifications.
	/// </summary>
	bool is_content_type_json(const utility::string_t &content_type)
	{
#if !defined(_WIN32) || _MSC_VER >= 1900
		const utility::string_t json_types[] = {
			mime_types::application_json,
			mime_types::application_xjson,
			mime_types::text_json,
			mime_types::text_xjson,
			mime_types::text_javascript,
			mime_types::text_xjavascript,
			mime_types::application_javascript,
			mime_types::application_xjavascript
		};
#endif

		return (is_content_type_one_of(std::begin(json_types), std::end(json_types), content_type));
	}
}

/// <summary>
/// Gets the default charset for given content type. If the MIME type is not textual or recognized Latin1 will be returned.
/// </summary>
static utility::string_t get_default_charset(const utility::string_t &content_type)
{
    // We are defaulting everything to Latin1 except JSON which is utf-8.
    if (details::is_content_type_json(content_type))
    {
        return charset_types::utf8;
    }
    else
    {
        return charset_types::latin1;
    }
}

namespace details
{
	/// <summary>
	/// Parses the given Content-Type header value to get out actual content type and charset.
	/// If the charset isn't specified the default charset for the content type will be set.
	/// </summary>
	void parse_content_type_and_charset(const utility::string_t &content_type, utility::string_t &content, utility::string_t &charset)
	{
		const size_t semi_colon_index = content_type.find_first_of(_XPLATSTR(";"));

		// No charset specified.
		if (semi_colon_index == utility::string_t::npos)
		{
			content = content_type;
			trim_whitespace(content);
			charset = get_default_charset(content);
			return;
		}

		// Split into content type and second part which could be charset.
		content = content_type.substr(0, semi_colon_index);
		trim_whitespace(content);
		utility::string_t possible_charset = content_type.substr(semi_colon_index + 1);
		trim_whitespace(possible_charset);
		const size_t equals_index = possible_charset.find_first_of(_XPLATSTR("="));

		// No charset specified.
		if (equals_index == utility::string_t::npos)
		{
			charset = get_default_charset(content);
			return;
		}

		// Split and make sure 'charset'
		utility::string_t charset_key = possible_charset.substr(0, equals_index);
		trim_whitespace(charset_key);
		if (!utility::details::str_icmp(charset_key, _XPLATSTR("charset")))
		{
			charset = get_default_charset(content);
			return;
		}
		charset = possible_charset.substr(equals_index + 1);
		// Remove the redundant ';' at the end of charset.
		while (charset.back() == ';')
		{
			charset.pop_back();
		}
		trim_whitespace(charset);
		if (charset.front() == _XPLATSTR('"') && charset.back() == _XPLATSTR('"'))
		{
			charset = charset.substr(1, charset.size() - 2);
			trim_whitespace(charset);
		}
	}

	//
	// Helper function to generate a wstring from given http_headers and message body.
	//
	utility::string_t http_headers_body_to_string(const http_headers &headers, concurrency::streams::istream instream)
	{
		utility::ostringstream_t buffer;
		buffer.imbue(std::locale::classic());

		for (const auto &header : headers)
		{
			buffer << header.first << _XPLATSTR(": ") << header.second << CRLF;
		}
		buffer << CRLF;

		utility::string_t content_type;
		if (headers.match(http::header_names::content_type, content_type))
		{
			buffer << convert_body_to_string_t(content_type, instream);
		}

		return buffer.str();
	}

	// Helper function to convert message body without extracting.
	utility::string_t convert_body_to_string_t(const utility::string_t &content_type, concurrency::streams::istream instream)
	{
		if (!instream)
		{
			// The instream is not yet set
			return utility::string_t();
		}

		concurrency::streams::streambuf<uint8_t>  streambuf = instream.streambuf();

		_ASSERTE((bool)streambuf);
		_ASSERTE(streambuf.is_open());
		_ASSERTE(streambuf.can_read());

		utility::string_t content, charset;
		details::parse_content_type_and_charset(content_type, content, charset);

		// Content-Type must have textual type.
		if (!details::is_content_type_textual(content) || streambuf.in_avail() == 0)
		{
			return utility::string_t();
		}

		// Latin1
		if (utility::details::str_icmp(charset, charset_types::latin1))
		{
			std::string body;
			body.resize(streambuf.in_avail());
			if (streambuf.scopy((unsigned char *)&body[0], body.size()) == 0) return string_t();
			return to_string_t(latin1_to_utf16(std::move(body)));
		}

		// utf-8.
		else if (utility::details::str_icmp(charset, charset_types::utf8))
		{
			std::string body;
			body.resize(streambuf.in_avail());
			if (streambuf.scopy((unsigned char *)&body[0], body.size()) == 0) return string_t();
			return to_string_t(std::move(body));
		}

		// utf-16.
		else if (utility::details::str_icmp(charset, charset_types::utf16))
		{
			utf16string body;
			body.resize(streambuf.in_avail() / sizeof(utf16string::value_type));
			if (streambuf.scopy((unsigned char *)&body[0], body.size() * sizeof(utf16string::value_type)) == 0) return string_t();
			return convert_utf16_to_string_t(std::move(body));
		}

		// utf-16le
		else if (utility::details::str_icmp(charset, charset_types::utf16le))
		{
			utf16string body;
			body.resize(streambuf.in_avail() / sizeof(utf16string::value_type));
			if (streambuf.scopy((unsigned char *)&body[0], body.size() * sizeof(utf16string::value_type)) == 0) return string_t();
			return convert_utf16le_to_string_t(std::move(body), false);
		}

		// utf-16be
		else if (utility::details::str_icmp(charset, charset_types::utf16be))
		{
			utf16string body;
			body.resize(streambuf.in_avail() / sizeof(utf16string::value_type));
			if (streambuf.scopy((unsigned char *)&body[0], body.size() * sizeof(utf16string::value_type)) == 0) return string_t();
			return convert_utf16be_to_string_t(std::move(body), false);
		}

		else
		{
			return utility::string_t();
		}
	}
}

details::_http_request::_http_request()
	: _client_config(),
	m_method(),
	//m_initiated_response(0),
	m_cancellationToken(pplx::cancellation_token::none())
{
	/*if (m_method.empty())
	{
		throw std::invalid_argument("Invalid HTTP method specified. Method can't be an empty string.");
	}*/
}

details::_http_request::_http_request(http::method mtd)
  : _client_config(),
	m_method(std::move(mtd)),
    //m_initiated_response(0),
    m_cancellationToken(pplx::cancellation_token::none())
{
    if(m_method.empty())
    {
        throw std::invalid_argument("Invalid HTTP method specified. Method can't be an empty string.");
    }
}

std::shared_ptr<http::details::http_response_proxy> _http_request::make_empty_response()
{
	auto result = std::make_shared<http::details::_http_response>(_request_impl_from_this());
	result->set_outstream(this->_response_stream(), false);
	return result;
}

std::shared_ptr<details::_http_request> _http_request::_request_impl_from_this()
{
	return std::static_pointer_cast<details::_http_request>(this->shared_from_this());
}

void details::_http_request::register_request_aborter()
{
	if (m_cancellationToken.is_canceled())
		throw task_canceled("_http_request task cancelled");
	if (!m_cancellationToken.is_cancelable())
		return;
	m_cancellationToken.register_callback([m_cancellationToken = m_cancellationToken, this_ = weak_from_this()]
	{
		auto this__ = this_.lock();
		if (!this__)
			return;
		std::static_pointer_cast<details::_http_request>(this__)->abort_request();
	});
}

}} // namespace web::http

