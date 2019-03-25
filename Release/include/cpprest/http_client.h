/***
* Copyright (C) Microsoft. All rights reserved.
* Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
*
* =+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
*
* HTTP Library: Client-side APIs.
*
* For the latest on this and related APIs, please see: https://github.com/Microsoft/cpprestsdk
*
* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
****/
#pragma once

#ifndef _CASA_HTTP_CLIENT_H
#define _CASA_HTTP_CLIENT_H

#if defined (__cplusplus_winrt)
#if !defined(__WRL_NO_DEFAULT_LIB__)
#define __WRL_NO_DEFAULT_LIB__
#endif
#include <wrl.h>
#include <msxml6.h>
namespace web { namespace http{namespace client{
typedef IXMLHTTPRequest2* native_handle;}}}
#else
namespace web {
	namespace http {
		class http_request;
		class http_response;
	}
}

namespace web { namespace http{namespace client{
typedef void* native_handle;}}}
#endif // __cplusplus_winrt

#include <memory>
#include <limits>

#include "pplx/pplxtasks.h"
#include "cpprest/http_msg.h"
#include "cpprest/json.h"
#include "cpprest/uri.h"
#include "cpprest/details/web_utilities.h"
#include "cpprest/details/basic_types.h"
#include "cpprest/asyncrt_utils.h"

#include <cpprest\http_client_config.h>

/// The web namespace contains functionality common to multiple protocols like HTTP and WebSockets.
namespace web
{
/// Declarations and functionality for the HTTP protocol.
namespace http
{
/// HTTP client side library.
namespace client
{

// credentials and web_proxy class has been moved from web::http::client namespace to web namespace.
// The below using declarations ensure we don't break existing code.
// Please use the web::credentials and web::web_proxy class going forward.
using web::credentials;
using web::web_proxy;

/// <summary>
/// HTTP client class, used to maintain a connection to an HTTP service for an extended session.
/// </summary>
class http_client
{
public:
    /// <summary>
    /// Creates a new http_client connected to specified uri.
    /// </summary>
    /// <param name="base_uri">A string representation of the base uri to be used for all requests. Must start with either "http://" or "https://"</param>
    _ASYNCRTIMP http_client(const uri &base_uri);

    /// <summary>
    /// Creates a new http_client connected to specified uri.
    /// </summary>
    /// <param name="base_uri">A string representation of the base uri to be used for all requests. Must start with either "http://" or "https://"</param>
    /// <param name="client_config">The http client configuration object containing the possible configuration options to initialize the <c>http_client</c>. </param>
    _ASYNCRTIMP http_client(const uri &base_uri, const http_client_config &client_config);

    /// <summary>
    /// Note the destructor doesn't necessarily close the connection and release resources.
    /// The connection is reference counted with the http_responses.
    /// </summary>
    _ASYNCRTIMP ~http_client() CPPREST_NOEXCEPT;

    /// <summary>
    /// Gets the base URI.
    /// </summary>
    /// <returns>
    /// A base URI initialized in constructor
    /// </returns>
    _ASYNCRTIMP const uri& base_uri() const;

    /// <summary>
    /// Get client configuration object
    /// </summary>
    /// <returns>A reference to the client configuration object.</returns>
    _ASYNCRTIMP const http_client_config& client_config() const;

    /// <summary>
    /// Asynchronously sends an HTTP request.
    /// </summary>
    /// <param name="request">Request to send.</param>
    /// <param name="token">Cancellation token for cancellation of this request operation.</param>
    /// <returns>An asynchronous operation that is completed once a response from the request is received.</returns>
    _ASYNCRTIMP pplx::task<http_response> request(http_request request, const pplx::cancellation_token &token = pplx::cancellation_token::none());

    /// <summary>
    /// Asynchronously sends an HTTP request.
    /// </summary>
    /// <param name="mtd">HTTP request method.</param>
    /// <param name="token">Cancellation token for cancellation of this request operation.</param>
    /// <returns>An asynchronous operation that is completed once a response from the request is received.</returns>
    pplx::task<http_response> request(const method &mtd, const pplx::cancellation_token &token = pplx::cancellation_token::none())
    {
        http_request msg(mtd);
        return request(msg, token);
    }

    /// <summary>
    /// Asynchronously sends an HTTP request.
    /// </summary>
    /// <param name="mtd">HTTP request method.</param>
    /// <param name="path_query_fragment">String containing the path, query, and fragment, relative to the http_client's base URI.</param>
    /// <param name="token">Cancellation token for cancellation of this request operation.</param>
    /// <returns>An asynchronous operation that is completed once a response from the request is received.</returns>
    pplx::task<http_response> request(
        const method &mtd,
        const utility::string_t &path_query_fragment,
        const pplx::cancellation_token &token = pplx::cancellation_token::none())
    {
        http_request msg(mtd);
        msg.set_request_uri(path_query_fragment);
        return request(msg, token);
    }

    /// <summary>
    /// Asynchronously sends an HTTP request.
    /// </summary>
    /// <param name="mtd">HTTP request method.</param>
    /// <param name="path_query_fragment">String containing the path, query, and fragment, relative to the http_client's base URI.</param>
    /// <param name="body_data">The data to be used as the message body, represented using the json object library.</param>
    /// <param name="token">Cancellation token for cancellation of this request operation.</param>
    /// <returns>An asynchronous operation that is completed once a response from the request is received.</returns>
    pplx::task<http_response> request(
        const method &mtd,
        const utility::string_t &path_query_fragment,
        const json::value &body_data,
        const pplx::cancellation_token &token = pplx::cancellation_token::none())
    {
        http_request msg(mtd);
        msg.set_request_uri(path_query_fragment);
        msg.set_body(body_data);
        return request(msg, token);
    }

    /// <summary>
    /// Asynchronously sends an HTTP request with a string body. Assumes the
    /// character encoding of the string is UTF-8.
    /// </summary>
    /// <param name="mtd">HTTP request method.</param>
    /// <param name="path_query_fragment">String containing the path, query, and fragment, relative to the http_client's base URI.</param>
    /// <param name="content_type">A string holding the MIME type of the message body.</param>
    /// <param name="body_data">String containing the text to use in the message body.</param>
    /// <param name="token">Cancellation token for cancellation of this request operation.</param>
    /// <returns>An asynchronous operation that is completed once a response from the request is received.</returns>
    pplx::task<http_response> request(
        const method &mtd,
        const utf8string &path_query_fragment,
        const utf8string &body_data,
        const utf8string &content_type = "text/plain; charset=utf-8",
        const pplx::cancellation_token &token = pplx::cancellation_token::none())
    {
        http_request msg(mtd);
        msg.set_request_uri(::utility::conversions::to_string_t(path_query_fragment));
        msg.set_body(body_data, content_type);
        return request(msg, token);
    }

    /// <summary>
    /// Asynchronously sends an HTTP request with a string body. Assumes the
    /// character encoding of the string is UTF-8.
    /// </summary>
    /// <param name="mtd">HTTP request method.</param>
    /// <param name="path_query_fragment">String containing the path, query, and fragment, relative to the http_client's base URI.</param>
    /// <param name="content_type">A string holding the MIME type of the message body.</param>
    /// <param name="body_data">String containing the text to use in the message body.</param>
    /// <param name="token">Cancellation token for cancellation of this request operation.</param>
    /// <returns>An asynchronous operation that is completed once a response from the request is received.</returns>
    pplx::task<http_response> request(
        const method &mtd,
        const utf8string &path_query_fragment,
        utf8string &&body_data,
        const utf8string &content_type = "text/plain; charset=utf-8",
        const pplx::cancellation_token &token = pplx::cancellation_token::none())
    {
        http_request msg(mtd);
        msg.set_request_uri(::utility::conversions::to_string_t(path_query_fragment));
        msg.set_body(std::move(body_data), content_type);
        return request(msg, token);
    }

    /// <summary>
    /// Asynchronously sends an HTTP request with a string body. Assumes the
    /// character encoding of the string is UTF-16 will perform conversion to UTF-8.
    /// </summary>
    /// <param name="mtd">HTTP request method.</param>
    /// <param name="path_query_fragment">String containing the path, query, and fragment, relative to the http_client's base URI.</param>
    /// <param name="content_type">A string holding the MIME type of the message body.</param>
    /// <param name="body_data">String containing the text to use in the message body.</param>
    /// <param name="token">Cancellation token for cancellation of this request operation.</param>
    /// <returns>An asynchronous operation that is completed once a response from the request is received.</returns>
    pplx::task<http_response> request(
        const method &mtd,
        const utf16string &path_query_fragment,
        const utf16string &body_data,
        const utf16string &content_type = utility::conversions::to_utf16string("text/plain"),
        const pplx::cancellation_token &token = pplx::cancellation_token::none())
    {
        http_request msg(mtd);
        msg.set_request_uri(::utility::conversions::to_string_t(path_query_fragment));
        msg.set_body(body_data, content_type);
        return request(msg, token);
    }

    /// <summary>
    /// Asynchronously sends an HTTP request with a string body. Assumes the
    /// character encoding of the string is UTF-8.
    /// </summary>
    /// <param name="mtd">HTTP request method.</param>
    /// <param name="path_query_fragment">String containing the path, query, and fragment, relative to the http_client's base URI.</param>
    /// <param name="body_data">String containing the text to use in the message body.</param>
    /// <param name="token">Cancellation token for cancellation of this request operation.</param>
    /// <returns>An asynchronous operation that is completed once a response from the request is received.</returns>
    pplx::task<http_response> request(
        const method &mtd,
        const utf8string &path_query_fragment,
        const utf8string &body_data,
        const pplx::cancellation_token &token)
    {
        return request(mtd, path_query_fragment, body_data, "text/plain; charset=utf-8", token);
    }

    /// <summary>
    /// Asynchronously sends an HTTP request with a string body. Assumes the
    /// character encoding of the string is UTF-8.
    /// </summary>
    /// <param name="mtd">HTTP request method.</param>
    /// <param name="path_query_fragment">String containing the path, query, and fragment, relative to the http_client's base URI.</param>
    /// <param name="body_data">String containing the text to use in the message body.</param>
    /// <param name="token">Cancellation token for cancellation of this request operation.</param>
    /// <returns>An asynchronous operation that is completed once a response from the request is received.</returns>
    pplx::task<http_response> request(
        const method &mtd,
        const utf8string &path_query_fragment,
        utf8string &&body_data,
        const pplx::cancellation_token &token)
    {
        http_request msg(mtd);
        msg.set_request_uri(::utility::conversions::to_string_t(path_query_fragment));
        msg.set_body(std::move(body_data), "text/plain; charset=utf-8");
        return request(msg, token);
    }

    /// <summary>
    /// Asynchronously sends an HTTP request with a string body. Assumes
    /// the character encoding of the string is UTF-16 will perform conversion to UTF-8.
    /// </summary>
    /// <param name="mtd">HTTP request method.</param>
    /// <param name="path_query_fragment">String containing the path, query, and fragment, relative to the http_client's base URI.</param>
    /// <param name="body_data">String containing the text to use in the message body.</param>
    /// <param name="token">Cancellation token for cancellation of this request operation.</param>
    /// <returns>An asynchronous operation that is completed once a response from the request is received.</returns>
    pplx::task<http_response> request(
        const method &mtd,
        const utf16string &path_query_fragment,
        const utf16string &body_data,
        const pplx::cancellation_token &token)
    {
        return request(mtd, path_query_fragment, body_data, ::utility::conversions::to_utf16string("text/plain"), token);
    }

#if !defined (__cplusplus_winrt)
    /// <summary>
    /// Asynchronously sends an HTTP request.
    /// </summary>
    /// <param name="mtd">HTTP request method.</param>
    /// <param name="path_query_fragment">String containing the path, query, and fragment, relative to the http_client's base URI.</param>
    /// <param name="body">An asynchronous stream representing the body data.</param>
    /// <param name="content_type">A string holding the MIME type of the message body.</param>
    /// <param name="token">Cancellation token for cancellation of this request operation.</param>
    /// <returns>A task that is completed once a response from the request is received.</returns>
    pplx::task<http_response> request(
        const method &mtd,
        const utility::string_t &path_query_fragment,
        const concurrency::streams::istream &body,
        const utility::string_t &content_type = _XPLATSTR("application/octet-stream"),
        const pplx::cancellation_token &token = pplx::cancellation_token::none())
    {
        http_request msg(mtd);
        msg.set_request_uri(path_query_fragment);
        msg.set_body(body, content_type);
        return request(msg, token);
    }

    /// <summary>
    /// Asynchronously sends an HTTP request.
    /// </summary>
    /// <param name="mtd">HTTP request method.</param>
    /// <param name="path_query_fragment">String containing the path, query, and fragment, relative to the http_client's base URI.</param>
    /// <param name="body">An asynchronous stream representing the body data.</param>
    /// <param name="token">Cancellation token for cancellation of this request operation.</param>
    /// <returns>A task that is completed once a response from the request is received.</returns>
    pplx::task<http_response> request(
        const method &mtd,
        const utility::string_t &path_query_fragment,
        const concurrency::streams::istream &body,
        const pplx::cancellation_token &token)
    {
        return request(mtd, path_query_fragment, body, _XPLATSTR("application/octet-stream"), token);
    }
#endif // __cplusplus_winrt

    /// <summary>
    /// Asynchronously sends an HTTP request.
    /// </summary>
    /// <param name="mtd">HTTP request method.</param>
    /// <param name="path_query_fragment">String containing the path, query, and fragment, relative to the http_client's base URI.</param>
    /// <param name="body">An asynchronous stream representing the body data.</param>
    /// <param name="content_length">Size of the message body.</param>
    /// <param name="content_type">A string holding the MIME type of the message body.</param>
    /// <param name="token">Cancellation token for cancellation of this request operation.</param>
    /// <returns>A task that is completed once a response from the request is received.</returns>
    /// <remarks>Winrt requires to provide content_length.</remarks>
    pplx::task<http_response> request(
        const method &mtd,
        const utility::string_t &path_query_fragment,
        const concurrency::streams::istream &body,
        size_t content_length,
        const utility::string_t &content_type = _XPLATSTR("application/octet-stream"),
        const pplx::cancellation_token &token = pplx::cancellation_token::none())
    {
        http_request msg(mtd);
        msg.set_request_uri(path_query_fragment);
        msg.set_body(body, content_length, content_type);
        return request(msg, token);
    }

    /// <summary>
    /// Asynchronously sends an HTTP request.
    /// </summary>
    /// <param name="mtd">HTTP request method.</param>
    /// <param name="path_query_fragment">String containing the path, query, and fragment, relative to the http_client's base URI.</param>
    /// <param name="body">An asynchronous stream representing the body data.</param>
    /// <param name="content_length">Size of the message body.</param>
    /// <param name="token">Cancellation token for cancellation of this request operation.</param>
    /// <returns>A task that is completed once a response from the request is received.</returns>
    /// <remarks>Winrt requires to provide content_length.</remarks>
    pplx::task<http_response> request(
        const method &mtd,
        const utility::string_t &path_query_fragment,
        const concurrency::streams::istream &body,
        size_t content_length,
        const pplx::cancellation_token &token)
    {
        return request(mtd, path_query_fragment, body, content_length, _XPLATSTR("application/octet-stream"), token);
    }

private:

	http_client_config _client_config;
	uri _base_uri;
};

namespace details {
#if defined(_WIN32)
extern const utility::char_t * get_with_body_err_msg;
#endif

}

}}}

#endif
