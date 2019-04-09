#include "stdafx.h"
#include "..\..\include\cpprest/details/http_msg_native_base.h"

std::shared_ptr<web::http::details::HttpCppCliBridge>& web::http::details::http_msg_native_base::_get_httpCppCliBridge()
{
	return _httpCppCliBridge;
}

const std::shared_ptr<web::http::details::HttpCppCliBridge>& web::http::details::http_msg_native_base::_get_httpCppCliBridge() const
{
	return _httpCppCliBridge;
}
