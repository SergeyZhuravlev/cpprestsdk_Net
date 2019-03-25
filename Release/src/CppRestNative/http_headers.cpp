#include "stdafx.h"
#include "cpprest\http_headers.h"
#include "cpprest\details\http_helpers.h"

namespace web {
	namespace http
	{

		//#define CRLF _XPLATSTR("\r\n")

		utility::string_t http_headers::content_type() const
		{
			utility::string_t result;
			match(http::header_names::content_type, result);
			return result;
		}

		void http_headers::set_content_type(utility::string_t type)
		{
			m_headers[http::header_names::content_type] = std::move(type);
		}

		utility::string_t http_headers::cache_control() const
		{
			utility::string_t result;
			match(http::header_names::cache_control, result);
			return result;
		}

		void http_headers::set_cache_control(utility::string_t control)
		{
			add(http::header_names::cache_control, std::move(control));
		}

		utility::string_t http_headers::date() const
		{
			utility::string_t result;
			match(http::header_names::date, result);
			return result;
		}

		void http_headers::set_date(const utility::datetime& date)
		{
			m_headers[http::header_names::date] = date.to_string(utility::datetime::RFC_1123);
		}

		utility::size64_t http_headers::content_length() const
		{
			utility::size64_t length = 0;
			match(http::header_names::content_length, length);
			return length;
		}

		void http_headers::set_content_length(utility::size64_t length)
		{
			m_headers[http::header_names::content_length] = utility::conversions::details::to_string_t(length);
		}

	}
}