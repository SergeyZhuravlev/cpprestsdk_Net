/***
* Copyright (C) Microsoft. All rights reserved.
* Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
*
* =+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
*
* HTTP Library: Client-side APIs.
*
* This file contains shared code across all http_client implementations.
*
* For the latest on this and related APIs, please see: https://github.com/Microsoft/cpprestsdk
*
* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
****/

#include "stdafx.h"

namespace web { namespace http { namespace client {

// Helper function to check to make sure the uri is valid.
static void verify_uri(const uri &uri)
{
    // Some things like proper URI schema are verified by the URI class.
    // We only need to check certain things specific to HTTP.
    if (uri.scheme() != _XPLATSTR("http") && uri.scheme() != _XPLATSTR("https"))
    {
        throw std::invalid_argument("URI scheme must be 'http' or 'https'");
    }

    if (uri.host().empty())
    {
        throw std::invalid_argument("URI must contain a hostname.");
    }
}

namespace details
{

#if defined(_WIN32)
	extern const utility::char_t * get_with_body_err_msg = _XPLATSTR("A GET or HEAD request should not have an entity body.");
#endif
}
http_client::http_client(const uri &base_uri) : http_client(base_uri, http_client_config())
{}

http_client::http_client(const uri &base_uri, const http_client_config &client_config) :
	_client_config(http_client_config(client_config))
{
    if (base_uri.scheme().empty())
    {
        auto uribuilder = uri_builder(base_uri);
        uribuilder.set_scheme(_XPLATSTR("http"));
        uri uriWithScheme = uribuilder.to_uri();
        verify_uri(uriWithScheme);
		_base_uri = uriWithScheme;
    }
    else
    {
        verify_uri(base_uri);
		_base_uri = base_uri;
    }
}

http_client::~http_client() CPPREST_NOEXCEPT {}

const http_client_config & http_client::client_config() const
{
    return _client_config;
}

const uri & http_client::base_uri() const
{
    return _base_uri;
}

// Macros to help build string at compile time and avoid overhead.
#define STRINGIFY(x) _XPLATSTR(#x)
#define TOSTRING(x) STRINGIFY(x)
#define USERAGENT _XPLATSTR("cpprestsdk/") TOSTRING(CPPREST_VERSION_MAJOR) _XPLATSTR(".") TOSTRING(CPPREST_VERSION_MINOR) _XPLATSTR(".") TOSTRING(CPPREST_VERSION_REVISION)

pplx::task<http_response> http_client::request(http_request request, const pplx::cancellation_token &token)
{
    if (!request.headers().has(header_names::user_agent))
    {
        request.headers().add(header_names::user_agent, USERAGENT);
    }

    request._set_base_uri(base_uri());
    request._set_cancellation_token(token);
	request.set_client_config(client_config());
	request._set_base_uri(_base_uri);
	return request.get_response();//propagate
}


}}}