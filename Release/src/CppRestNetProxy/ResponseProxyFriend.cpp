#include "stdafx.h"
#include "..\..\include\cpprest\details\ResponseProxyFriend.h"
#include "..\..\include\cpprest\details\CpprestManagedTryCatch.h"
#include "..\..\include\cpprest\details\streambuf_from_Stream.h"
#include "..\..\include\cpprest\details\http_response_proxy.h"

web::http::details::ResponseProxyFriend::~ResponseProxyFriend()
{
	auto response = _response;
	_response = nullptr;
	delete response;
}

web::http::details::ResponseProxyFriend::ResponseProxyFriend(const std::weak_ptr<http_response_proxy>& response): _response(
	new std::weak_ptr<http_response_proxy>(response))
{
}

void web::http::details::ResponseProxyFriend::set_response_stream(System::IO::Stream^ responseStream,
                                                                  System::Int64 contentLength,
                                                                  System::String^ contentType)
{
	CPPREST_BEGIN_NATIVE_TRY
	if (_response == nullptr)
		throw gcnew System::NullReferenceException(__FUNCSIG__);
	std::shared_ptr<http_response_proxy> response(*_response);
	auto responseStreamBuf = std::make_shared<Concurrency::streams::details::streambuf_from_Stream>(
		responseStream, contentLength, response->_get_httpCppCliBridge());
	response->_set_response_body(responseStreamBuf, contentLength,
	                             msclr::interop::marshal_as<std::wstring>(contentType));
	CPPREST_END_NATIVE_TRY
}
