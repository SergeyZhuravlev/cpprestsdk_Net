#include "stdafx.h"
#include "cpprest/details/RequestProxyFriend.h"
#include "cpprest/details/CpprestManagedTryCatch.h"
#include "cpprest/details/HttpCppCliBridge.h"
#include "cpprest/details/streambuf_from_Stream.h"
#include "cpprest/details/http_request_proxy.h"
#include "cpprestsdk_Net/Release/include/cpprest/details/http_request_proxy.h"

web::http::details::RequestProxyFriend::~RequestProxyFriend()
{
	auto request = _request;
	_request = nullptr;
	delete request;
}

void web::http::details::RequestProxyFriend::set_response_holder(CppRestNetImpl::ResponseImpl^ response)
{
	std::shared_ptr<http_request_proxy> request(*_request);
	request->_get_httpCppCliBridge()->responseImpl = response;
}

web::http::details::RequestProxyFriend::RequestProxyFriend(const std::weak_ptr<http_request_proxy>& request): _request(
	new std::weak_ptr<http_request_proxy>(request))
{
}

void web::http::details::RequestProxyFriend::_request_stream_writer(System::IO::Stream^ requestStream,
                                                                    System::Nullable<System::Int64> contentLength)
{
	CPPREST_BEGIN_NATIVE_TRY
		if (_request == nullptr)
			throw gcnew System::NullReferenceException(__FUNCSIG__);
		std::shared_ptr<http_request_proxy> request(*_request);
		auto requestStreamBuf = std::make_shared<Concurrency::streams::details::streambuf_from_Stream>(
			requestStream, std::nullopt);
		auto contentLength_ = contentLength.HasValue ? std::make_optional(contentLength.Value) : std::nullopt;
		request->_request_stream_writer(requestStreamBuf, contentLength_);
	CPPREST_END_NATIVE_TRY
}

System::Nullable<System::Int64> web::http::details::RequestProxyFriend::_request_stream_length()
{
	CPPREST_BEGIN_NATIVE_TRY
		if (_request == nullptr)
			throw gcnew System::NullReferenceException(__FUNCSIG__);
		std::shared_ptr<http_request_proxy> request(*_request);
		auto result = request->_request_stream_length();
		if (result)
			return System::Nullable<System::Int64>(*result);
		System::Nullable<System::Int64> null;
		return null;
	CPPREST_END_NATIVE_TRY
}

bool web::http::details::RequestProxyFriend::_response_explicit_stream_writer(System::IO::Stream^ responseStream,
                                                                              System::Int64 contentLength)
{
	CPPREST_BEGIN_NATIVE_TRY
		if (_request == nullptr)
			throw gcnew System::NullReferenceException(__FUNCSIG__);
		std::shared_ptr<web::http::details::http_request_proxy> request(*_request);
		if (!request->_has_explicit_response_stream())
			return false;
		auto responseStreamBuf = std::make_shared<Concurrency::streams::details::streambuf_from_Stream>(
			responseStream, contentLength);
		request->_response_explicit_stream_writer(responseStreamBuf);
		return true;
	CPPREST_END_NATIVE_TRY
}
