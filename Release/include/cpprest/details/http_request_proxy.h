#pragma once
#include <cpprest\CppRestProxyExport.h>
#include <memory>
#include "http_request_base.h"
#include <cpprest\details\http_msg_native_base.h>
#include "cpprest/base_uri.h"
#include "cpprest/http_client_config.h"
#include "cpprest/http_headers.h"
#include "http_response_proxy.h"
#include "cpprest/istreambuf_type_erasure.h"
#include <optional>

namespace web::http::details
{
	class _http_request;
//#include "warnings\dll_export_warnings_disable.h"
	class http_request_proxy : public http_request_base, public std::enable_shared_from_this<http_request_proxy>
	{
	public:
		virtual void _request_stream_writer(std::shared_ptr<Concurrency::streams::istreambuf_type_erasure> streambuf, std::optional<Concurrency::streams::istreambuf_type_erasure::size_type> contentSize) = 0;
		virtual void _response_explicit_stream_writer(std::shared_ptr<Concurrency::streams::istreambuf_type_erasure> streambuf) = 0;
		virtual std::optional<int64_t> _request_stream_length() = 0;
		virtual bool _has_explicit_response_stream() = 0;

	protected:
		CPPRESTPROXY_API http_request_proxy();
		virtual CPPRESTPROXY_API ~http_request_proxy();
		std::shared_ptr<http::details::http_response_proxy> CPPRESTPROXY_API get_response_cli();

		virtual std::shared_ptr<details::_http_request> _request_impl_from_this() = 0;

		virtual method &method() = 0;
		virtual uri absolute_uri() const = 0;
		virtual web::http::client::http_client_config &client_config() = 0;
		virtual const web::http::client::http_client_config &client_config() const = 0;
		virtual http_headers &headers() = 0;
		virtual bool has_request_body() = 0;
		void CPPRESTPROXY_API abort_request();

		virtual std::shared_ptr<http::details::http_response_proxy> make_empty_response() = 0;

	private:

		std::shared_ptr<http::details::http_response_proxy> processRequest();
		std::shared_ptr<http::details::http_response_proxy> processResponse();

		std::weak_ptr<http::details::http_response_proxy> _response;
	};
//#include "warnings\dll_export_warnings_restore.h"
}
