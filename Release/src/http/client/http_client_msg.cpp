/***
* Copyright (C) Microsoft. All rights reserved.
* Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
*
* =+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
*
* HTTP Library: Request and reply message definitions (client side).
*
* For the latest on this and related APIs, please see: https://github.com/Microsoft/cpprestsdk
*
* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
****/
#include "stdafx.h"
#include "..\..\include\cpprest\details\internal_http_helpers.h"
#include "cpprest\streambuf_type_erasure.h"

namespace web { namespace http
{

uri details::_http_request::relative_uri() const
{
    // If the listener path is empty, then just return the request URI.
    if(m_listener_path.empty() || m_listener_path == _XPLATSTR("/"))
    {
        return m_uri.resource();
    }

    utility::string_t prefix = uri::decode(m_listener_path);
    utility::string_t path = uri::decode(m_uri.resource().to_string());
    if(path.empty())
    {
        path = _XPLATSTR("/");
    }

    auto pos = path.find(prefix);
    if (pos == 0)
    {
        return uri(uri::encode_uri(path.erase(0, prefix.length())));
    }
    else
    {
        throw http_exception(_XPLATSTR("Error: request was not prefixed with listener uri"));
    }
}

uri details::_http_request::absolute_uri() const
{
    if (m_base_uri.is_empty())
    {
        return m_uri;
    }
    else
    {
        return uri_builder(m_base_uri).append(m_uri).to_uri();
    }
}

void details::_http_request::set_request_uri(const uri& relative)
{
    m_uri = relative;
}

namespace
{
	int64_t getStreamSize(concurrency::streams::istream &inputStream_)
	{
		auto inputStream = concurrency::streams::async_istream<concurrency::streams::istream::traits::char_type>(inputStream_);
		auto currentPosition = inputStream.tellg();
		inputStream.seekg(0, std::ios::end);
		auto streamSize = inputStream.tellg();
		inputStream.seekg(currentPosition);
		inputStream.clear();
		return static_cast<int64_t>(streamSize);
	}
}

std::optional<int64_t> details::_http_request::_request_stream_length()
{
	auto requestBodySource = instream();
	if (!requestBodySource)
		throw std::logic_error("logic error in automatic content length detector");
	if (!requestBodySource.can_seek())
		return std::nullopt;
	return getStreamSize(requestBodySource);
}

void details::_http_request::_request_stream_writer(
	std::shared_ptr<Concurrency::streams::istreambuf_type_erasure> streambuf_te, std::optional<Concurrency::streams::istreambuf_type_erasure::size_type> contentSize)
{
	typedef Concurrency::streams::istreambuf_type_erasure::size_type size_type;
	if (!streambuf_te)
		return;
	auto requestBodySource = instream();
	if (!requestBodySource)
		return;
	/*if (!contentSize)
		throw new std::logic_error("Content-Length not setted before send request");*/
	Concurrency::streams::streambuf<uint8_t> bodyDestination {
		std::make_shared<Concurrency::streams::streambuf_type_erasure<uint8_t>>(streambuf_te, std::ios_base::out) };
	auto flush = [&](pplx::task<size_t> writed)
	{
		(void)writed.get();
		return bodyDestination
		       .sync()
		       .then([writed](pplx::task<void> ignored)
		       {
			       ignored.get();
			       return 0;
		       });
	};
	if (contentSize)
	{
		const size_type bufferSize = 16 * 1024;
		size_type readRemains = contentSize.value()-bufferSize;
		auto reader = [&](pplx::task<size_t> readed)
		{
			auto reader_impl = [&](pplx::task<size_t> readed, auto self)
			{
				if (readed.get() <= 0)
					return pplx::task_from_result<size_t>(0);
				const auto nextBufferSize = static_cast<size_t>((std::min)(bufferSize, readRemains));
				if (nextBufferSize <= 0 || readRemains <= 0)
					return pplx::task_from_result<size_t>(0);
				readRemains -= nextBufferSize;
				return requestBodySource
					.read(bodyDestination, nextBufferSize)
					.then([self](pplx::task<size_t> readed) {return self(readed, self); }, m_cancellationToken);
			};

			return reader_impl(readed, reader_impl);
		};
		(void)requestBodySource
			.read(bodyDestination, static_cast<size_t>(bufferSize))
			.then(reader, m_cancellationToken)
			.then(flush)
			.get();
	}
	else
	{
		//This branch should not be called
		(void)requestBodySource
			.read_to_end(bodyDestination)
			.then(flush)
			.get();
	}
}

void details::_http_request::_response_explicit_stream_writer(
	std::shared_ptr<Concurrency::streams::istreambuf_type_erasure> streambuf_te)
{
	typedef Concurrency::streams::istreambuf_type_erasure::size_type size_type;
	if (!streambuf_te)
		throw std::invalid_argument(__FUNCSIG__ ": empty streambuf_te, but must at this point by internal logic");
	auto explicitResponseDestination = _response_stream();
	if (!explicitResponseDestination)
		throw std::logic_error(__FUNCSIG__ ": outstream is not setted, but must at this point by internal logic");
	auto explicitResponseSource = Concurrency::streams::streambuf<uint8_t>(
		std::make_shared<Concurrency::streams::streambuf_type_erasure<uint8_t>>(streambuf_te, std::ios_base::in));

	const size_t bufferSize = 16 * 1024;

	auto writer = [&](pplx::task<size_t> writed)
	{
		auto writer_impl = [&](pplx::task<size_t> writed, auto self)
		{
			if (writed.get() <= 0)
				return pplx::task_from_result<size_t>(0);
			return explicitResponseDestination
				.write(explicitResponseSource, bufferSize)
				.then([self](pplx::task<size_t> writed) {return self(writed, self); }, m_cancellationToken);
		};
		
		return writer_impl(writed, writer_impl);
	};
	auto flush = [&](pplx::task<size_t> writed)
	{
		(void)writed.get();
		return explicitResponseDestination
		       .flush()
		       .then([&explicitResponseDestination, writed](pplx::task<void> ignored)
		       {
			       ignored.get();
			       return 0;
		       });
	};
	(void)explicitResponseDestination
	      .write(explicitResponseSource, bufferSize)
	      .then(writer, m_cancellationToken)
	      .then(flush)
	      .get();
}

utility::string_t details::_http_request::to_string() const
{
    utility::ostringstream_t buffer;
    buffer.imbue(std::locale::classic());
    buffer << m_method << _XPLATSTR(" ") << (this->m_uri.is_empty() ? _XPLATSTR("/") : this->m_uri.to_string()) << _XPLATSTR(" HTTP/1.1\r\n");
    buffer << http_msg_base::to_string();
    return buffer.str();
}
namespace
{
	pplx::task<std::weak_ptr<details::_http_response>> http_response_to__http_response(pplx::task<http_response> response)
	{
		return response.then([](const http_response& response) {return http_response::pimpl_type::weak_type(response._get_impl()); });
	}
}

pplx::task<http_response> details::_http_request::get_response()
{
	std::lock_guard lock(_make_response_task_protector);
	if (m_response._GetImpl())
		return m_response.then(_http_response_task_to_http_response);
	register_request_aborter();
	auto response = pplx::create_task([this_ = this->_request_impl_from_this()]
	{
		auto response_impl = std::static_pointer_cast<details::_http_response>(this_->get_response_cli());
		auto result = http_response(response_impl);
		return result;
	}, pplx::task_options(m_cancellationToken));
	m_response = http_response_to__http_response(response);
	return response;
}

void details::_http_response::_set_response_body(
	const std::shared_ptr<Concurrency::streams::istreambuf_type_erasure>& streambuf_te, utility::size64_t contentLength,
	const utf16string& contentType)
{
	Concurrency::streams::streambuf<uint8_t> buf{
		std::make_shared<Concurrency::streams::streambuf_type_erasure<uint8_t>>(streambuf_te, std::ios_base::in)
	};
	set_body(buf.create_istream(), contentLength, contentType);
}

utility::string_t details::_http_response::to_string() const
{
    // If the user didn't explicitly set a reason phrase then we should have it default
    // if they used one of the standard known status codes.
    auto reason_phrase = m_reason_phrase;
    if(reason_phrase.empty())
    {
        reason_phrase = get_default_reason_phrase(status_code());
    }

    utility::ostringstream_t buffer;
    buffer.imbue(std::locale::classic());
    buffer << _XPLATSTR("HTTP/1.1 ") << m_status_code << _XPLATSTR(" ") << reason_phrase << _XPLATSTR("\r\n");

    buffer << http_msg_base::to_string();
    return buffer.str();
}

}} // namespace web::http
