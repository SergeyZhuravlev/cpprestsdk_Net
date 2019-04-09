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
#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>
#include <system_error>
#include <mutex>

#include "pplx/pplxtasks.h"
#include "cpprest/json.h"
#include "cpprest/uri.h"
#include "cpprest/http_headers.h"
#include "cpprest/details/cpprest_compat.h"
#include "cpprest/asyncrt_utils.h"
#include "cpprest/streams.h"
#include "cpprest/containerstream.h"
#include "details/http_response_proxy.h"
#include "cpprest/http_exception.h"
#include "details/http_request_proxy.h"
#include "cpprest/details/http_helpers.h"
#include <pplinterface.h>

namespace web
{
namespace http
{
	class http_response;
	class http_request;
	namespace details
	{
		class _http_response;
		class _http_request;
		_ASYNCRTIMP bool is_content_type_textual(const utility::string_t &content_type);
		_ASYNCRTIMP bool is_content_type_json(const utility::string_t &content_type);
		_ASYNCRTIMP void parse_content_type_and_charset(const utility::string_t &content_type, utility::string_t &content, utility::string_t &charset);
		_ASYNCRTIMP utility::string_t http_headers_body_to_string(const web::http::http_headers &headers, concurrency::streams::istream instream);
		utility::string_t convert_body_to_string_t(const utility::string_t &content_type, concurrency::streams::istream instream);
		inline const utility::char_t * stream_was_set_explicitly = _XPLATSTR("A stream was set on the message and extraction is not possible");
		inline const utility::char_t * unsupported_charset = _XPLATSTR("Charset must be iso-8859-1, utf-8, utf-16, utf-16le, or utf-16be to be extracted.");
	}

// URI class has been moved from web::http namespace to web namespace.
// The below using declarations ensure we don't break existing code.
// Please use the web::uri class going forward.
using web::uri;
using web::uri_builder;

namespace client
{
    class http_client;
}

/// Message direction
namespace message_direction
{
    /// <summary>
    /// Enumeration used to denote the direction of a message: a request with a body is
    /// an upload, a response with a body is a download.
    /// </summary>
    enum direction {
        upload,
        download
    };
}

typedef std::function<void(message_direction::direction, utility::size64_t)> progress_handler;

namespace details
{

/// <summary>
/// Base class for HTTP messages.
/// This class is to store common functionality so it isn't duplicated on
/// both the request and response side.
/// </summary>
template<class Base>
class /*_ASYNCRTIMP*/ http_msg_base: public Base
{
public:

    friend class http::client::http_client;

    /*_ASYNCRTIMP*/ http_msg_base();

    virtual ~http_msg_base() {}

    http_headers &headers() { return m_headers; }

    /*_ASYNCRTIMP*/ void set_body(const concurrency::streams::istream &instream, const utf8string &contentType);
    /*_ASYNCRTIMP*/ void set_body(const concurrency::streams::istream &instream, const utf16string &contentType);
    /*_ASYNCRTIMP*/ void set_body(const concurrency::streams::istream &instream, utility::size64_t contentLength, const utf8string &contentType);
    /*_ASYNCRTIMP*/ void set_body(const concurrency::streams::istream &instream, utility::size64_t contentLength, const utf16string &contentType);

    /// <summary>
    /// Helper function for extract functions. Parses the Content-Type header and check to make sure it matches,
    /// throws an exception if not.
    /// </summary>
    /// <param name="ignore_content_type">If true ignores the Content-Type header value.</param>
    /// <param name="check_content_type">Function to verify additional information on Content-Type.</param>
    /// <returns>A string containing the charset, an empty string if no Content-Type header is empty.</returns>
    utility::string_t parse_and_check_content_type(bool ignore_content_type, const std::function<bool(const utility::string_t&)> &check_content_type);

    /*_ASYNCRTIMP*/ utf8string extract_utf8string(bool ignore_content_type = false);
    /*_ASYNCRTIMP*/ utf16string extract_utf16string(bool ignore_content_type = false);
    /*_ASYNCRTIMP*/ utility::string_t extract_string(bool ignore_content_type = false);

    /*_ASYNCRTIMP*/ json::value _extract_json(bool ignore_content_type = false);
    /*_ASYNCRTIMP*/ std::vector<unsigned char> _extract_vector();

    virtual /*_ASYNCRTIMP*/ utility::string_t to_string() const;

    /// <summary>
    /// Set the stream through which the message body could be read
    /// </summary>
    void set_instream(const concurrency::streams::istream &instream)  { m_inStream = instream; }

    /// <summary>
    /// Get the stream through which the message body could be read
    /// </summary>
    const concurrency::streams::istream & instream() const { return m_inStream; }

    /// <summary>
    /// Set the stream through which the message body could be written
    /// </summary>
    void set_outstream(const concurrency::streams::ostream &outstream, bool is_default)  { m_outStream = outstream; m_default_outstream = is_default; }

    /// <summary>
    /// Get the stream through which the message body could be written
    /// </summary>
    const concurrency::streams::ostream & outstream() const { return m_outStream; }

	void _set_content_ready(uint64_t contentSize) { m_data_available.store(contentSize); }

	virtual pplx::task<utility::size64_t> _get_data_available() const = 0;

    /// <summary>
    /// Prepare the message with an output stream to receive network data
    /// </summary>
    /*_ASYNCRTIMP*/ void _prepare_to_receive_data();

    /// <summary>
    /// Determine the content length
    /// </summary>
    /// <returns>
    /// size_t::max if there is content with unknown length (transfer_encoding:chunked)
    /// 0           if there is no content
    /// length      if there is content with known length
    /// </returns>
    /// <remarks>
    /// This routine should only be called after a msg (request/response) has been
    /// completely constructed.
    /// </remarks>
    /*_ASYNCRTIMP*/ size_t _get_content_length();

protected:

    /// <summary>
    /// Stream to read the message body.
    /// By default this is an invalid stream. The user could set the instream on
    /// a request by calling set_request_stream(...). This would also be set when
    /// set_body() is called - a stream from the body is constructed and set.
    /// Even in the presense of msg body this stream could be invalid. An example
    /// would be when the user sets an ostream for the response. With that API the
    /// user does not provide the ability to read the msg body.
    /// Thus m_instream is valid when there is a msg body and it can actually be read
    /// </summary>
    concurrency::streams::istream m_inStream;

    /// <summary>
    /// stream to write the msg body
    /// By default this is an invalid stream. The user could set this on the response
    /// (for http_client). In all the other cases we would construct one to transfer
    /// the data from the network into the message body.
    /// </summary>
    concurrency::streams::ostream m_outStream;

    http_headers m_headers;
    bool m_default_outstream = {};

    /// <summary> The TCE is used to signal the availability of the message body. </summary>
    std::atomic<utility::size64_t> m_data_available = {};
};

#include "..\..\include\cpprest\details\http_msg_realization.h"
namespace 
{
	pplx::task<http_response> _http_response_to_http_response(std::weak_ptr<details::_http_response> response);
	pplx::task<http_response> _http_response_to_http_response(pplx::task<std::weak_ptr<details::_http_response>> response);
	pplx::task<http_request> _http_request_to_http_request(pplx::task<std::weak_ptr<details::_http_request>> request);
	pplx::task<http_request> _http_request_to_http_request(std::weak_ptr<details::_http_request> request);
}

class _http_request;

/// <summary>
/// Internal representation of an HTTP response.
/// </summary>
class _http_response final : public http::details::http_msg_base<http::details::http_response_proxy>
{
public:
    _http_response() : m_status_code((std::numeric_limits<uint16_t>::max)()) { }

    _http_response(http::status_code code) : m_status_code(code) {}

	_http_response(std::shared_ptr<_http_request> request) : m_request(request), m_status_code((std::numeric_limits<uint16_t>::max)()) {}

    http::status_code status_code() const { return m_status_code; }

    void set_status_code(http::status_code code) { m_status_code = code; }

    const http::reason_phrase & reason_phrase() const { return m_reason_phrase; }

    void set_reason_phrase(const http::reason_phrase &reason) { m_reason_phrase = reason; }

	_ASYNCRTIMP void _set_response_body(const std::shared_ptr<Concurrency::streams::istreambuf_type_erasure>& streambuf_te,
                  utility::size64_t contentLength, const utf16string& contentType) override;

    _ASYNCRTIMP utility::string_t to_string() const;

	std::weak_ptr<_http_request> weak_request()
	{
		return m_request;
	}

	pplx::task<utility::size64_t> _get_data_available() const override { return pplx::task_from_result(m_data_available.load()); }

private:
    http::status_code m_status_code = {};
    http::reason_phrase m_reason_phrase;
	std::weak_ptr<_http_request> m_request;
};

} // namespace details


/// <summary>
/// Represents an HTTP response.
/// </summary>
class http_response
{
public:

    /// <summary>
    /// Constructs a response with an empty status code, no headers, and no body.
    /// </summary>
    /// <returns>A new HTTP response.</returns>
    http_response() : _m_impl(std::make_shared<details::_http_response>()) { }

	/// <summary>
	/// 
	/// </summary>
	/// <returns>A new HTTP response.</returns>
	http_response(const std::shared_ptr<http::details::_http_response>& http_response_impl) : _m_impl(http_response_impl) { }

    /// <summary>
    /// Constructs a response with given status code, no headers, and no body.
    /// </summary>
    /// <param name="code">HTTP status code to use in response.</param>
    /// <returns>A new HTTP response.</returns>
    http_response(http::status_code code)
        : _m_impl(std::make_shared<details::_http_response>(code)) { }

    /// <summary>
    /// Gets the status code of the response message.
    /// </summary>
    /// <returns>status code.</returns>
    http::status_code status_code() const { return _m_impl->status_code(); }

    /// <summary>
    /// Sets the status code of the response message.
    /// </summary>
    /// <param name="code">Status code to set.</param>
    /// <remarks>
    /// This will overwrite any previously set status code.
    /// </remarks>
    void set_status_code(http::status_code code) const { _m_impl->set_status_code(code); }

    /// <summary>
    /// Gets the reason phrase of the response message.
    /// If no reason phrase is set it will default to the standard one corresponding to the status code.
    /// </summary>
    /// <returns>Reason phrase.</returns>
    const http::reason_phrase & reason_phrase() const { return _m_impl->reason_phrase(); }

    /// <summary>
    /// Sets the reason phrase of the response message.
    /// If no reason phrase is set it will default to the standard one corresponding to the status code.
    /// </summary>
    /// <param name="reason">The reason phrase to set.</param>
    void set_reason_phrase(const http::reason_phrase &reason) const { _m_impl->set_reason_phrase(reason); }

    /// <summary>
    /// Gets the headers of the response message.
    /// </summary>
    /// <returns>HTTP headers for this response.</returns>
    /// <remarks>
    /// Use the <seealso cref="http_headers::add Method"/> to fill in desired headers.
    /// </remarks>
    http_headers &headers() { return _m_impl->headers(); }

    /// <summary>
    /// Gets a const reference to the headers of the response message.
    /// </summary>
    /// <returns>HTTP headers for this response.</returns>
    const http_headers &headers() const { return _m_impl->headers(); }

    /// <summary>
    /// Generates a string representation of the message, including the body when possible.
    /// Mainly this should be used for debugging purposes as it has to copy the
    /// message body and doesn't have excellent performance.
    /// </summary>
    /// <returns>A string representation of this HTTP request.</returns>
    /// <remarks>Note this function is synchronous and doesn't wait for the
    /// entire message body to arrive. If the message body has arrived by the time this
    /// function is called and it is has a textual Content-Type it will be included.
    /// Otherwise just the headers will be present.</remarks>
    utility::string_t to_string() const { return _m_impl->to_string(); }

    /// <summary>
    /// Extracts the body of the response message as a string value, checking that the content type is a MIME text type.
    /// A body can only be extracted once because in some cases an optimization is made where the data is 'moved' out.
    /// </summary>
    /// <param name="ignore_content_type">If true, ignores the Content-Type header and assumes text.</param>
    /// <returns>String containing body of the message.</returns>
    pplx::task<utility::string_t> extract_string(bool ignore_content_type = false) const
    {
        auto impl = _m_impl;
        return pplx::create_task(_m_impl->_get_data_available()).then([impl, ignore_content_type](utility::size64_t) { return impl->extract_string(ignore_content_type); });
    }

    /// <summary>
    /// Extracts the body of the response message as a UTF-8 string value, checking that the content type is a MIME text type.
    /// A body can only be extracted once because in some cases an optimization is made where the data is 'moved' out.
    /// </summary>
    /// <param name="ignore_content_type">If true, ignores the Content-Type header and assumes text.</param>
    /// <returns>String containing body of the message.</returns>
    pplx::task<utf8string> extract_utf8string(bool ignore_content_type = false) const
    {
        auto impl = _m_impl;
        return pplx::create_task(_m_impl->_get_data_available()).then([impl, ignore_content_type](utility::size64_t) { return impl->extract_utf8string(ignore_content_type); });
    }

    /// <summary>
    /// Extracts the body of the response message as a UTF-16 string value, checking that the content type is a MIME text type.
    /// A body can only be extracted once because in some cases an optimization is made where the data is 'moved' out.
    /// </summary>
    /// <param name="ignore_content_type">If true, ignores the Content-Type header and assumes text.</param>
    /// <returns>String containing body of the message.</returns>
    pplx::task<utf16string> extract_utf16string(bool ignore_content_type = false) const
    {
        auto impl = _m_impl;
        return pplx::create_task(_m_impl->_get_data_available()).then([impl, ignore_content_type](utility::size64_t) { return impl->extract_utf16string(ignore_content_type); });
    }

    /// <summary>
    /// Extracts the body of the response message into a json value, checking that the content type is application/json.
    /// A body can only be extracted once because in some cases an optimization is made where the data is 'moved' out.
    /// </summary>
    /// <param name="ignore_content_type">If true, ignores the Content-Type header and assumes json.</param>
    /// <returns>JSON value from the body of this message.</returns>
    pplx::task<json::value> extract_json(bool ignore_content_type = false) const
    {
        auto impl = _m_impl;
        return pplx::create_task(_m_impl->_get_data_available()).then([impl, ignore_content_type](utility::size64_t) { return impl->_extract_json(ignore_content_type); });
    }

    /// <summary>
    /// Extracts the body of the response message into a vector of bytes.
    /// </summary>
    /// <returns>The body of the message as a vector of bytes.</returns>
    pplx::task<std::vector<unsigned char>> extract_vector() const
    {
        auto impl = _m_impl;
        return pplx::create_task(_m_impl->_get_data_available()).then([impl](utility::size64_t) { return impl->_extract_vector(); });
    }

    /// <summary>
    /// Sets the body of the message to a textual string and set the "Content-Type" header. Assumes
    /// the character encoding of the string is UTF-8.
    /// </summary>
    /// <param name="body_text">String containing body text.</param>
    /// <param name="content_type">MIME type to set the "Content-Type" header to. Default to "text/plain; charset=utf-8".</param>
    /// <remarks>
    /// This will overwrite any previously set body data and "Content-Type" header.
    /// </remarks>
    void set_body(utf8string &&body_text, const utf8string &content_type = utf8string("text/plain; charset=utf-8"))
    {
        const auto length = body_text.size();
        _m_impl->set_body(concurrency::streams::bytestream::open_istream<std::string>(std::move(body_text)), length, content_type);
    }

    /// <summary>
    /// Sets the body of the message to a textual string and set the "Content-Type" header. Assumes
    /// the character encoding of the string is UTF-8.
    /// </summary>
    /// <param name="body_text">String containing body text.</param>
    /// <param name="content_type">MIME type to set the "Content-Type" header to. Default to "text/plain; charset=utf-8".</param>
    /// <remarks>
    /// This will overwrite any previously set body data and "Content-Type" header.
    /// </remarks>
    void set_body(const utf8string &body_text, const utf8string &content_type = utf8string("text/plain; charset=utf-8"))
    {
        _m_impl->set_body(concurrency::streams::bytestream::open_istream<std::string>(body_text), body_text.size(), content_type);
    }

    /// <summary>
    /// Sets the body of the message to a textual string and set the "Content-Type" header. Assumes
    /// the character encoding of the string is UTF-16 will perform conversion to UTF-8.
    /// </summary>
    /// <param name="body_text">String containing body text.</param>
    /// <param name="content_type">MIME type to set the "Content-Type" header to. Default to "text/plain".</param>
    /// <remarks>
    /// This will overwrite any previously set body data and "Content-Type" header.
    /// </remarks>
    void set_body(const utf16string &body_text, utf16string content_type = utility::conversions::to_utf16string("text/plain"))
    {
        if (content_type.find(::utility::conversions::to_utf16string("charset=")) != content_type.npos)
        {
            throw std::invalid_argument("content_type can't contain a 'charset'.");
        }

        auto utf8body = utility::conversions::utf16_to_utf8(body_text);
        auto length = utf8body.size();
        _m_impl->set_body(concurrency::streams::bytestream::open_istream<std::string>(
        		std::move(utf8body)),
        		length,
        		std::move(content_type.append(::utility::conversions::to_utf16string("; charset=utf-8"))));
    }

    /// <summary>
    /// Sets the body of the message to contain json value. If the 'Content-Type'
    /// header hasn't already been set it will be set to 'application/json'.
    /// </summary>
    /// <param name="body_text">json value.</param>
    /// <remarks>
    /// This will overwrite any previously set body data.
    /// </remarks>
    void set_body(const json::value &body_data)
    {
        auto body_text = utility::conversions::to_utf8string(body_data.serialize());
        auto length = body_text.size();
        set_body(concurrency::streams::bytestream::open_istream(std::move(body_text)), length, _XPLATSTR("application/json"));
    }

    /// <summary>
    /// Sets the body of the message to the contents of a byte vector. If the 'Content-Type'
    /// header hasn't already been set it will be set to 'application/octet-stream'.
    /// </summary>
    /// <param name="body_data">Vector containing body data.</param>
    /// <remarks>
    /// This will overwrite any previously set body data.
    /// </remarks>
    void set_body(std::vector<unsigned char> &&body_data)
    {
        auto length = body_data.size();
        set_body(concurrency::streams::bytestream::open_istream(std::move(body_data)), length);
    }

    /// <summary>
    /// Sets the body of the message to the contents of a byte vector. If the 'Content-Type'
    /// header hasn't already been set it will be set to 'application/octet-stream'.
    /// </summary>
    /// <param name="body_data">Vector containing body data.</param>
    /// <remarks>
    /// This will overwrite any previously set body data.
    /// </remarks>
    void set_body(const std::vector<unsigned char> &body_data)
    {
        set_body(concurrency::streams::bytestream::open_istream(body_data), body_data.size());
    }

    /// <summary>
    /// Defines a stream that will be relied on to provide the body of the HTTP message when it is
    /// sent.
    /// </summary>
    /// <param name="stream">A readable, open asynchronous stream.</param>
    /// <param name="content_type">A string holding the MIME type of the message body.</param>
    /// <remarks>
    /// This cannot be used in conjunction with any other means of setting the body of the request.
    /// The stream will not be read until the message is sent.
    /// </remarks>
    void set_body(const concurrency::streams::istream &stream, const utility::string_t &content_type = _XPLATSTR("application/octet-stream"))
    {
        _m_impl->set_body(stream, content_type);
    }

    /// <summary>
    /// Defines a stream that will be relied on to provide the body of the HTTP message when it is
    /// sent.
    /// </summary>
    /// <param name="stream">A readable, open asynchronous stream.</param>
    /// <param name="content_length">The size of the data to be sent in the body.</param>
    /// <param name="content_type">A string holding the MIME type of the message body.</param>
    /// <remarks>
    /// This cannot be used in conjunction with any other means of setting the body of the request.
    /// The stream will not be read until the message is sent.
    /// </remarks>
    void set_body(const concurrency::streams::istream &stream, utility::size64_t content_length, const utility::string_t &content_type = _XPLATSTR("application/octet-stream"))
    {
        _m_impl->set_body(stream, content_length, content_type);
    }

    /// <summary>
    /// Produces a stream which the caller may use to retrieve data from an incoming request.
    /// </summary>
    /// <returns>A readable, open asynchronous stream.</returns>
    /// <remarks>
    /// This cannot be used in conjunction with any other means of getting the body of the request.
    /// It is not necessary to wait until the message has been sent before starting to write to the
    /// stream, but it is advisable to do so, since it will allow the network I/O to start earlier
    /// and the work of sending data can be overlapped with the production of more data.
    /// </remarks>
    concurrency::streams::istream body() const
    {
        return _m_impl->instream();
    }

	using pimpl_type = std::shared_ptr<http::details::_http_response>;

    /// <summary>
    /// Signals the user (client) when all the data for this response message has been received.
    /// </summary>
    /// <returns>A <c>task</c> which is completed when all of the response body has been received.</returns>
    pplx::task<http::http_response> content_ready() const
    {
		return _get_impl()->_get_data_available()
    		.then([_m_impl = pimpl_type::weak_type(_get_impl())](utility::size64_t)
			{
				return details::_http_response_to_http_response(_m_impl).get();
			});
    }

	pimpl_type _get_impl() const { return _m_impl; }

private:
	pimpl_type _m_impl;
};

namespace details {
/// <summary>
/// Internal representation of an HTTP request message.
/// </summary>
class _http_request final : public http::details::http_msg_base<http::details::http_request_proxy>//, public std::enable_shared_from_this<_http_request>
{
public:

	explicit _http_request();

    _ASYNCRTIMP _http_request(http::method mtd);

    virtual ~_http_request() {}

    http::method &method() { return m_method; }

    uri &request_uri() { return m_uri; }

    _ASYNCRTIMP uri absolute_uri() const;

    _ASYNCRTIMP uri relative_uri() const;

    _ASYNCRTIMP void set_request_uri(const uri&);

    //const utility::string_t& remote_address() const { return m_remote_address; }

    const pplx::cancellation_token &cancellation_token() const { return m_cancellationToken; }

	web::http::client::http_client_config &client_config() override { return _client_config; }

	const web::http::client::http_client_config &client_config() const override { return _client_config; }

    void set_cancellation_token(const pplx::cancellation_token &token)
    {
        m_cancellationToken = token;
    }

	bool has_request_body() override
    {
		return instream();
    }

	_ASYNCRTIMP std::optional<int64_t> _request_stream_length() override;

	_ASYNCRTIMP void _request_stream_writer(std::shared_ptr<Concurrency::streams::istreambuf_type_erasure> streambuf_te, std::optional<Concurrency::streams::istreambuf_type_erasure::size_type> contentSize) override;

	bool _has_explicit_response_stream() override
	{
		return m_response_stream;
	}

	_ASYNCRTIMP void _response_explicit_stream_writer(
		std::shared_ptr<Concurrency::streams::istreambuf_type_erasure> streambuf_te) override;

    _ASYNCRTIMP utility::string_t to_string() const;

	_ASYNCRTIMP pplx::task<http_response> get_response();

    void set_response_stream(const concurrency::streams::ostream &stream)
    {
        m_response_stream = stream;
    }

    void set_progress_handler(const progress_handler &handler)
    {
		(void)handler;
        //m_progress_handler = std::make_shared<progress_handler>(handler);
    }

    const concurrency::streams::ostream & _response_stream() const { return m_response_stream; }

    //const std::shared_ptr<progress_handler> & _progress_handler() const { return m_progress_handler; }

    void _set_base_uri(const http::uri &base_uri) { m_base_uri = base_uri; }

    //void _set_remote_address(const utility::string_t &remote_address) { m_remote_address = remote_address; }

	pplx::task<utility::size64_t> _get_data_available() const override 
	{ 
		auto this_ = const_cast<_http_request*>(this);
		std::weak_ptr this_weak = this_->_request_impl_from_this();
		return this_->get_response()
    		.then([this_weak](const auto& response)
		{
			return std::shared_ptr(this_weak)->m_data_available.load();
		});
	}

protected:
	_ASYNCRTIMP std::shared_ptr<http::details::http_response_proxy> make_empty_response() override;
	_ASYNCRTIMP std::shared_ptr<details::_http_request> _request_impl_from_this() override;

private:

	_ASYNCRTIMP void register_request_aborter();

	web::http::client::http_client_config _client_config;

    http::method m_method;

    // Tracks whether or not a response has already been started for this message.
    // pplx::details::atomic_long m_initiated_response;

    pplx::cancellation_token m_cancellationToken;

    http::uri m_base_uri;
    http::uri m_uri;
    utility::string_t m_listener_path;

    concurrency::streams::ostream m_response_stream;

    //std::shared_ptr<progress_handler> m_progress_handler;

    //utility::string_t m_remote_address;

	pplx::task<std::weak_ptr<_http_response>> m_response;
	std::recursive_mutex _make_response_task_protector;
};


}  // namespace details

/// <summary>
/// Represents an HTTP request.
/// </summary>
class http_request
{
public:
    /// <summary>
    /// Constructs a new HTTP request with the 'GET' method.
    /// </summary>
    http_request()
        : _m_impl(std::make_shared<http::details::_http_request>(methods::GET)) {}

	/// <summary>
	/// 
	/// </summary>
	/// <returns>A new HTTP request.</returns>
	http_request(const std::shared_ptr<http::details::_http_request>& http_response_impl) : _m_impl(http_response_impl) { }


	http_request(const http_request& request) = default;
	http_request& operator=(const http_request& request) = default;

    /// <summary>
    /// Constructs a new HTTP request with the given request method.
    /// </summary>
    /// <param name="mtd">Request method.</param>
    http_request(http::method mtd)
        : _m_impl(std::make_shared<http::details::_http_request>(std::move(mtd))) {}

    /// <summary>
    /// Destructor frees any held resources.
    /// </summary>
    ~http_request() {}

	const http::client::http_client_config &client_config() const { return _m_impl->client_config(); }

	void set_client_config(const http::client::http_client_config &client_config) const { _m_impl->client_config() = client_config; }

    /// <summary>
    /// Get the method (GET/PUT/POST/DELETE) of the request message.
    /// </summary>
    /// <returns>Request method of this HTTP request.</returns>
    const http::method &method() const { return _m_impl->method(); }

    /// <summary>
    /// Set the method (GET/PUT/POST/DELETE) of the request message.
    /// </summary>
    /// <param name="method">Request method of this HTTP request.</param>
    void set_method(const http::method &method) const { _m_impl->method() = method; }

    /// <summary>
    /// Get the underling URI of the request message.
    /// </summary>
    /// <returns>The uri of this message.</returns>
    const uri & request_uri() const { return _m_impl->request_uri(); }

    /// <summary>
    /// Set the underling URI of the request message.
    /// </summary>
    /// <param name="uri">The uri for this message.</param>
    void set_request_uri(const uri& uri) { return _m_impl->set_request_uri(uri); }

    /// <summary>
    /// Gets a reference the URI path, query, and fragment part of this request message.
    /// This will be appended to the base URI specified at construction of the http_client.
    /// </summary>
    /// <returns>A string.</returns>
    /// <remarks>When the request is the one passed to a listener's handler, the
    /// relative URI is the request URI less the listener's path. In all other circumstances,
    /// request_uri() and relative_uri() will return the same value.
    /// </remarks>
    uri relative_uri() const { return _m_impl->relative_uri(); }

    /// <summary>
    /// Get an absolute URI with scheme, host, port, path, query, and fragment part of
    /// the request message.
    /// </summary>
    /// <remarks>Absolute URI is only valid after this http_request object has been passed
    /// to http_client::request().
    /// </remarks>
    uri absolute_uri() const { return _m_impl->absolute_uri(); }

    /// <summary>
    /// Gets a reference to the headers of the response message.
    /// </summary>
    /// <returns>HTTP headers for this response.</returns>
    /// <remarks>
    /// Use the http_headers::add to fill in desired headers.
    /// </remarks>
    http_headers &headers() { return _m_impl->headers(); }

    /// <summary>
    /// Gets a const reference to the headers of the response message.
    /// </summary>
    /// <returns>HTTP headers for this response.</returns>
    /// <remarks>
    /// Use the http_headers::add to fill in desired headers.
    /// </remarks>
    const http_headers &headers() const { return _m_impl->headers(); }

    /// <summary>
    /// Returns a string representation of the remote IP address.
    /// </summary>
    /// <returns>The remote IP address.</returns>
    //const utility::string_t& get_remote_address() const { return _m_impl->remote_address(); }

    /// <summary>
    /// Extract the body of the request message as a string value, checking that the content type is a MIME text type.
    /// A body can only be extracted once because in some cases an optimization is made where the data is 'moved' out.
    /// </summary>
    /// <param name="ignore_content_type">If true, ignores the Content-Type header and assumes UTF-8.</param>
    /// <returns>String containing body of the message.</returns>
    pplx::task<utility::string_t> extract_string(bool ignore_content_type = false)
    {
        auto impl = _m_impl;
        return pplx::create_task(_m_impl->_get_data_available()).then([impl, ignore_content_type](utility::size64_t) { return impl->extract_string(ignore_content_type); });
    }

    /// <summary>
    /// Extract the body of the request message as a UTF-8 string value, checking that the content type is a MIME text type.
    /// A body can only be extracted once because in some cases an optimization is made where the data is 'moved' out.
    /// </summary>
    /// <param name="ignore_content_type">If true, ignores the Content-Type header and assumes UTF-8.</param>
    /// <returns>String containing body of the message.</returns>
    pplx::task<utf8string> extract_utf8string(bool ignore_content_type = false)
    {
        auto impl = _m_impl;
        return pplx::create_task(_m_impl->_get_data_available()).then([impl, ignore_content_type](utility::size64_t) { return impl->extract_utf8string(ignore_content_type); });
    }

    /// <summary>
    /// Extract the body of the request message as a UTF-16 string value, checking that the content type is a MIME text type.
    /// A body can only be extracted once because in some cases an optimization is made where the data is 'moved' out.
    /// </summary>
    /// <param name="ignore_content_type">If true, ignores the Content-Type header and assumes UTF-16.</param>
    /// <returns>String containing body of the message.</returns>
    pplx::task<utf16string> extract_utf16string(bool ignore_content_type = false)
    {
        auto impl = _m_impl;
        return pplx::create_task(_m_impl->_get_data_available()).then([impl, ignore_content_type](utility::size64_t) { return impl->extract_utf16string(ignore_content_type); });
    }

    /// <summary>
    /// Extracts the body of the request message into a json value, checking that the content type is application/json.
    /// A body can only be extracted once because in some cases an optimization is made where the data is 'moved' out.
    /// </summary>
    /// <param name="ignore_content_type">If true, ignores the Content-Type header and assumes UTF-8.</param>
    /// <returns>JSON value from the body of this message.</returns>
    pplx::task<json::value> extract_json(bool ignore_content_type = false) const
    {
        auto impl = _m_impl;
        return pplx::create_task(_m_impl->_get_data_available()).then([impl, ignore_content_type](utility::size64_t) { return impl->_extract_json(ignore_content_type); });
    }

    /// <summary>
    /// Extract the body of the response message into a vector of bytes. Extracting a vector can be done on
    /// </summary>
    /// <returns>The body of the message as a vector of bytes.</returns>
    pplx::task<std::vector<unsigned char>> extract_vector() const
    {
        auto impl = _m_impl;
        return pplx::create_task(_m_impl->_get_data_available()).then([impl](utility::size64_t) { return impl->_extract_vector(); });
    }

    /// <summary>
    /// Sets the body of the message to a textual string and set the "Content-Type" header. Assumes
    /// the character encoding of the string is UTF-8.
    /// </summary>
    /// <param name="body_text">String containing body text.</param>
    /// <param name="content_type">MIME type to set the "Content-Type" header to. Default to "text/plain; charset=utf-8".</param>
    /// <remarks>
    /// This will overwrite any previously set body data and "Content-Type" header.
    /// </remarks>
    void set_body(utf8string &&body_text, const utf8string &content_type = utf8string("text/plain; charset=utf-8"))
    {
        const auto length = body_text.size();
        _m_impl->set_body(concurrency::streams::bytestream::open_istream<std::string>(std::move(body_text)), length, content_type);
    }

    /// <summary>
    /// Sets the body of the message to a textual string and set the "Content-Type" header. Assumes
    /// the character encoding of the string is UTF-8.
    /// </summary>
    /// <param name="body_text">String containing body text.</param>
    /// <param name="content_type">MIME type to set the "Content-Type" header to. Default to "text/plain; charset=utf-8".</param>
    /// <remarks>
    /// This will overwrite any previously set body data and "Content-Type" header.
    /// </remarks>
    void set_body(const utf8string &body_text, const utf8string &content_type = utf8string("text/plain; charset=utf-8"))
    {
        _m_impl->set_body(concurrency::streams::bytestream::open_istream<std::string>(body_text), body_text.size(), content_type);
    }

    /// <summary>
    /// Sets the body of the message to a textual string and set the "Content-Type" header. Assumes
    /// the character encoding of the string is UTF-16 will perform conversion to UTF-8.
    /// </summary>
    /// </summary>
    /// <param name="body_text">String containing body text.</param>
    /// <param name="content_type">MIME type to set the "Content-Type" header to. Default to "text/plain".</param>
    /// <remarks>
    /// This will overwrite any previously set body data and "Content-Type" header.
    /// </remarks>
    void set_body(const utf16string &body_text, utf16string content_type = utility::conversions::to_utf16string("text/plain"))
    {
        if(content_type.find(::utility::conversions::to_utf16string("charset=")) != content_type.npos)
        {
            throw std::invalid_argument("content_type can't contain a 'charset'.");
        }

        auto utf8body = utility::conversions::utf16_to_utf8(body_text);
        auto length = utf8body.size();
        _m_impl->set_body(concurrency::streams::bytestream::open_istream(
        		std::move(utf8body)),
        		length,
        		std::move(content_type.append(::utility::conversions::to_utf16string("; charset=utf-8"))));
    }

    /// <summary>
    /// Sets the body of the message to contain json value. If the 'Content-Type'
    /// header hasn't already been set it will be set to 'application/json'.
    /// </summary>
    /// <param name="body_data">json value.</param>
    /// <remarks>
    /// This will overwrite any previously set body data.
    /// </remarks>
    void set_body(const json::value &body_data)
    {
        auto body_text = utility::conversions::to_utf8string(body_data.serialize());
        auto length = body_text.size();
        _m_impl->set_body(concurrency::streams::bytestream::open_istream(std::move(body_text)), length, _XPLATSTR("application/json"));
    }

    /// <summary>
    /// Sets the body of the message to the contents of a byte vector. If the 'Content-Type'
    /// header hasn't already been set it will be set to 'application/octet-stream'.
    /// </summary>
    /// <param name="body_data">Vector containing body data.</param>
    /// <remarks>
    /// This will overwrite any previously set body data.
    /// </remarks>
    void set_body(std::vector<unsigned char> &&body_data)
    {
        auto length = body_data.size();
        _m_impl->set_body(concurrency::streams::bytestream::open_istream(std::move(body_data)), length, _XPLATSTR("application/octet-stream"));
    }

    /// <summary>
    /// Sets the body of the message to the contents of a byte vector. If the 'Content-Type'
    /// header hasn't already been set it will be set to 'application/octet-stream'.
    /// </summary>
    /// <param name="body_data">Vector containing body data.</param>
    /// <remarks>
    /// This will overwrite any previously set body data.
    /// </remarks>
    void set_body(const std::vector<unsigned char> &body_data)
    {
        set_body(concurrency::streams::bytestream::open_istream(body_data), body_data.size());
    }

    /// <summary>
    /// Defines a stream that will be relied on to provide the body of the HTTP message when it is
    /// sent.
    /// </summary>
    /// <param name="stream">A readable, open asynchronous stream.</param>
    /// <param name="content_type">A string holding the MIME type of the message body.</param>
    /// <remarks>
    /// This cannot be used in conjunction with any other means of setting the body of the request.
    /// The stream will not be read until the message is sent.
    /// </remarks>
    void set_body(const concurrency::streams::istream &stream, const utility::string_t &content_type = _XPLATSTR("application/octet-stream"))
    {
        _m_impl->set_body(stream, content_type);
    }

    /// <summary>
    /// Defines a stream that will be relied on to provide the body of the HTTP message when it is
    /// sent.
    /// </summary>
    /// <param name="stream">A readable, open asynchronous stream.</param>
    /// <param name="content_length">The size of the data to be sent in the body.</param>
    /// <param name="content_type">A string holding the MIME type of the message body.</param>
    /// <remarks>
    /// This cannot be used in conjunction with any other means of setting the body of the request.
    /// The stream will not be read until the message is sent.
    /// </remarks>
    void set_body(const concurrency::streams::istream &stream, utility::size64_t content_length, const utility::string_t &content_type = _XPLATSTR("application/octet-stream"))
    {
        _m_impl->set_body(stream, content_length, content_type);
    }

    /// <summary>
    /// Produces a stream which the caller may use to retrieve data from an incoming request.
    /// </summary>
    /// <returns>A readable, open asynchronous stream.</returns>
    /// <remarks>
    /// This cannot be used in conjunction with any other means of getting the body of the request.
    /// It is not necessary to wait until the message has been sent before starting to write to the
    /// stream, but it is advisable to do so, since it will allow the network I/O to start earlier
    /// and the work of sending data can be overlapped with the production of more data.
    /// </remarks>
    concurrency::streams::istream body() const
    {
        return _m_impl->instream();
    }

    /// <summary>
    /// Defines a stream that will be relied on to hold the body of the HTTP response message that
    /// results from the request.
    /// </summary>
    /// <param name="stream">A writable, open asynchronous stream.</param>
    /// <remarks>
    /// If this function is called, the body of the response should not be accessed in any other
    /// way.
    /// </remarks>
    void set_response_stream(const concurrency::streams::ostream &stream)
    {
        return _m_impl->set_response_stream(stream);
    }

	///Сейчас не реализовано
    /// <summary>
    /// Defines a callback function that will be invoked for every chunk of data uploaded or downloaded
    /// as part of the request.
    /// </summary>
    /// <param name="handler">A function representing the progress handler. It's parameters are:
    ///    up:       a <c>message_direction::direction</c> value  indicating the direction of the message
    ///              that is being reported.
    ///    progress: the number of bytes that have been processed so far.
    /// </param>
    /// <remarks>
    ///   This function will be called at least once for upload and at least once for
    ///   the download body, unless there is some exception generated. An HTTP message with an error
    ///   code is not an exception. This means, that even if there is no body, the progress handler
    ///   will be called.
    ///
    ///   Setting the chunk size on the http_client does not guarantee that the client will be using
    ///   exactly that increment for uploading and downloading data.
    ///
    ///   The handler will be called only once for each combination of argument values, in order. Depending
    ///   on how a service responds, some download values may come before all upload values have been
    ///   reported.
    ///
    ///   The progress handler will be called on the thread processing the request. This means that
    ///   the implementation of the handler must take care not to block the thread or do anything
    ///   that takes significant amounts of time. In particular, do not do any kind of I/O from within
    ///   the handler, do not update user interfaces, and to not acquire any locks. If such activities
    ///   are necessary, it is the handler's responsibility to execute that work on a separate thread.
    /// </remarks>
    void set_progress_handler(const progress_handler &handler)
    {
        return _m_impl->set_progress_handler(handler);
    }

    /// <summary>
    /// Signals the user (listener) when all the data for this request message has been received.
    /// </summary>
    /// <returns>A <c>task</c> which is completed when all of the response body has been received</returns>

	pplx::task<http::http_request> content_ready() const
	{
		return _get_impl()->_get_data_available()
    		.then([_m_impl = pimpl_type::weak_type(_get_impl())](utility::size64_t)
			{
				return details::_http_request_to_http_request(_m_impl).get();
			});
	}

    /// <summary>
    /// Gets a task representing the response that will eventually be sent.
    /// </summary>
    /// <returns>A task that is completed once response is sent.</returns>
    pplx::task<http_response> get_response() const
    {
		if (method().empty())
			throw std::runtime_error("Uninitialized http request method");
        return _m_impl->get_response();
    }

    /// <summary>
    /// Generates a string representation of the message, including the body when possible.
    /// Mainly this should be used for debugging purposes as it has to copy the
    /// message body and doesn't have excellent performance.
    /// </summary>
    /// <returns>A string representation of this HTTP request.</returns>
    /// <remarks>Note this function is synchronous and doesn't wait for the
    /// entire message body to arrive. If the message body has arrived by the time this
    /// function is called and it is has a textual Content-Type it will be included.
    /// Otherwise just the headers will be present.</remarks>
    utility::string_t to_string() const { return _m_impl->to_string(); }

    const std::shared_ptr<http::details::_http_request> & _get_impl() const { return _m_impl; }

    void _set_cancellation_token(const pplx::cancellation_token &token)
    {
        _m_impl->set_cancellation_token(token);
    }

    const pplx::cancellation_token & _cancellation_token() const
    {
        return _m_impl->cancellation_token();
    }

    void _set_base_uri(const http::uri &base_uri)
    {
        _m_impl->_set_base_uri(base_uri);
    }

	using pimpl_type = std::shared_ptr<http::details::_http_request>;

private:
    friend class http::details::_http_request;
    friend class http::client::http_client;

	pimpl_type _m_impl;
};

namespace details
{
	namespace
	{
		pplx::task<http_response> _http_response_to_http_response(http_response::pimpl_type::weak_type response)
		{
			const http_response result{ http_response::pimpl_type(response) };
			return pplx::task_from_result<http_response>(result);
		}

		pplx::task<http_response> _http_response_task_to_http_response(pplx::task<http_response::pimpl_type::weak_type> response)
		{
			return response.then([](http_response::pimpl_type::weak_type response) { return _http_response_to_http_response(response); });
		}

		pplx::task<http_request> _http_request_to_http_request(http_request::pimpl_type::weak_type request)
		{
			const http_request result{ http_request::pimpl_type(request) };
			return pplx::task_from_result<http_request>(result);
		}

		pplx::task<http_request> _http_request_task_to_http_request(pplx::task<http_request::pimpl_type::weak_type> request)
		{
			return request.then([](http_request::pimpl_type::weak_type request) { return _http_request_to_http_request(request); });
		}
	}
}

}}
