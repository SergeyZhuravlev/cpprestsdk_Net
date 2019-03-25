#pragma once
#include <cpprest\CppRestProxyExport.h>
#include "http_response_base.h"
#include "cpprest/http_headers.h"
#include "cpprest/istreambuf_type_erasure.h"

namespace web::http::details
{
//#include "warnings\dll_export_warnings_disable.h"
	class http_response_proxy : public http_response_base, public std::enable_shared_from_this<http_response_proxy>
	{
	public:
		virtual void set_status_code(http::status_code code) = 0;
		virtual void set_reason_phrase(const http::reason_phrase &reason) = 0;
		virtual void _set_response_body(const std::shared_ptr<Concurrency::streams::istreambuf_type_erasure> &streambuf, utility::size64_t contentLength, const utf16string &contentType) = 0;
		virtual void _set_content_ready(uint64_t contentSize) = 0;
		virtual http::http_headers& headers() = 0;

		CPPRESTPROXY_API ~http_response_proxy();
	};
//#include "warnings\dll_export_warnings_restore.h"
}
