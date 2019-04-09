#include "stdafx.h"
#include "..\..\include\cpprest\details\http_request_proxy.h"
#include "..\..\include\cpprest\details\HttpCppCliBridge.h"
#include <msclr/marshal.h>
#include <msclr\marshal_cppstd.h>
#include "..\..\include\cpprest\http_client_config.h"
#include "..\..\include\cpprest\details\internal_http_helpers.h"
#include "..\..\include\cpprest\details\CpprestManagedTryCatch.h"
#include "..\..\include\cpprest\istreambuf_type_erasure.h"
#include "..\..\include\cpprest\details\streambuf_from_Stream.h"
#include "..\..\include\cpprest\details\RequestProxyFriend.h"
#include "..\..\include\cpprest\details\ResponseProxyFriend.h"

namespace web::http::details
{
	http_request_proxy::http_request_proxy()
	{
		_get_httpCppCliBridge() = std::make_shared<HttpCppCliBridge>();
		auto cts = gcnew System::Threading::CancellationTokenSource();
		_get_httpCppCliBridge()->cancellationTokenSource = cts;
		_get_httpCppCliBridge()->cancellationToken = cts->Token;
	}

	http_request_proxy::~http_request_proxy()
	{
	}

	std::shared_ptr<http::details::http_response_proxy> http_request_proxy::get_response_cli()
	{
		CPPREST_BEGIN_MANAGED_TRY
		auto response = processRequest();
		_response = response;
		return response;
		CPPREST_END_MANAGED_TRY
	}

	namespace
	{
		System::Uri^ GetUri(const uri& uri)
		{
			return gcnew System::Uri(msclr::interop::marshal_as<System::String^>(uri.to_string()));
		}

		CppRest::ProxyMode GetProxyMode(const web_proxy& proxy)
		{
			auto mode = proxy.internal_mode();
			switch(mode)
			{
				case web_proxy::use_default_:
				case web_proxy::use_auto_discovery:
				case web_proxy::ie_settings_:
					return CppRest::ProxyMode::IE;
				case web_proxy::disabled_:
					return CppRest::ProxyMode::Disabled;
				case web_proxy::user_provided_:
					return CppRest::ProxyMode::Manual;
			default: 
				throw std::runtime_error("Unknown proxy mode "+ std::to_string(mode));
			}
		}

		CppRest::Credentials^ GetCredentials(const credentials& cred)
		{
			auto nativeCredential = cred;
			if (!nativeCredential.is_set())
				return nullptr;
			auto proxyCredentials = gcnew CppRest::Credentials();
			proxyCredentials->ProxyPassword = msclr::interop::marshal_as<System::String^>(nativeCredential.password());
			proxyCredentials->ProxyDomainWithLogin = msclr::interop::marshal_as<System::String^>(nativeCredential.username());
			return proxyCredentials;
		}

		System::Net::ICredentials^ GetNetworkCredentials(const credentials& cred)
		{
			auto nativeCredential = cred;
			if (!nativeCredential.is_set())
				return nullptr;
			auto credentials = gcnew System::Net::NetworkCredential(
				msclr::interop::marshal_as<System::String^>(nativeCredential.username()), 
				msclr::interop::marshal_as<System::String^>(nativeCredential.password()));
			return credentials;
		}

		System::String^ GetProxyAddress(const web_proxy& proxy)
		{
			auto address = proxy.address();
			if (address.is_empty())
				return nullptr;
			return msclr::interop::marshal_as<System::String^>(address.to_string());
		}

		void FillClientCerts(const web::http::client::http_client_config::certificates_t& certs, System::Collections::Generic::ICollection<System::Collections::Generic::KeyValuePair<System::String^, System::Security::Cryptography::X509Certificates::StoreLocation>>^ certsResult)
		{
			for (auto&& cert : certs)
			{
				auto thumb = msclr::interop::marshal_as<System::String^>(cert.first);
				certsResult->Add(System::Collections::Generic::KeyValuePair<System::String^, System::Security::Cryptography::X509Certificates::StoreLocation>(thumb, cert.second ? System::Security::Cryptography::X509Certificates::StoreLocation::CurrentUser : System::Security::Cryptography::X509Certificates::StoreLocation::LocalMachine));
			}
		}

		CppRestNetImpl::IRequestSettings^ GetSettings(const web::http::client::http_client_config& config)
		{
			auto settings = gcnew CppRestNetImpl::RequestSettings();
			settings->AllTimeouts = System::TimeSpan::FromMilliseconds(static_cast<double>(config.timeout<std::chrono::milliseconds>().count()));
			const auto nativeProxy = config.proxy();
			settings->ProxyInfo = gcnew CppRest::ProxyInfo(GetProxyMode(nativeProxy), GetCredentials(nativeProxy.credentials()), GetProxyAddress(nativeProxy));
			FillClientCerts(config.get_client_certificates(), settings->ClientCertificates);
			settings->Credentials = GetNetworkCredentials(config.credentials());
			return settings;
		}
	}

	void http_request_proxy::abort_request()
	{
		CPPREST_BEGIN_MANAGED_TRY
		auto tokenSource = _get_httpCppCliBridge()->cancellationTokenSource.operator->();
		tokenSource->Cancel();
		CPPREST_END_MANAGED_TRY
	}

	std::shared_ptr<http::details::http_response_proxy> http_request_proxy::processRequest()
	{
		auto token = _get_httpCppCliBridge()->cancellationToken.operator->();
		System::Action<System::IO::Stream^, System::Nullable<System::Int64>>^ requestStreamWriter = nullptr;
		System::Func<System::Nullable<System::Int64>>^ requestStreamLength = nullptr;
		System::Func<System::IO::Stream^, System::Int64, System::Boolean>^ responseExplicitStreamWriter = nullptr;
		RequestProxyFriend requestProxyFriend(this->weak_from_this());
		auto responseHolderSetter = gcnew System::Action<CppRestNetImpl::ResponseImpl^>(%requestProxyFriend, &RequestProxyFriend::set_response_holder);
		if (has_request_body())
		{
			requestStreamLength = gcnew System::Func<System::Nullable<System::Int64>>(%requestProxyFriend, &RequestProxyFriend::_request_stream_length);
			requestStreamWriter = gcnew System::Action<System::IO::Stream^, System::Nullable<System::Int64>>(%requestProxyFriend, &RequestProxyFriend::_request_stream_writer);
		}
		responseExplicitStreamWriter = gcnew System::Func<System::IO::Stream^, System::Int64, System::Boolean>(%requestProxyFriend, &RequestProxyFriend::_response_explicit_stream_writer);
		auto requestImpl = gcnew CppRestNetImpl::RequestImpl(
			GetUri(absolute_uri())
			, responseHolderSetter
			, GetSettings(client_config())
			, msclr::interop::marshal_as<System::String^>(method())
			, _get_httpCppCliBridge()->headersMarshaller->GetHeaders(headers())
			, _get_httpCppCliBridge()->headersMarshaller->GetRangeHeaders(headers())
			/*, System::Nullable<long long>()
			, nullptr*/
			, requestStreamWriter
			, requestStreamLength
			, responseExplicitStreamWriter
			, token);
		_get_httpCppCliBridge()->headersMarshaller->FillFromHeaders(headers(), requestImpl->InternalRequest);
		_get_httpCppCliBridge()->requestImpl = requestImpl;
		auto response = processResponse();
		ResponseProxyFriend responseProxyFriend(response->weak_from_this());
		auto explicitResponseStreamContentLength = requestImpl->ExplicitResponseStreamContentLength;
		if(explicitResponseStreamContentLength.HasValue)
			response->_set_content_ready(explicitResponseStreamContentLength.Value);
		auto responseStreamSetter = gcnew System::Action<System::IO::Stream^, System::Int64, System::String^>(%responseProxyFriend, &ResponseProxyFriend::set_response_stream);
		requestImpl->SetResponseBody(responseStreamSetter);
		return response;
	}

	std::shared_ptr<http::details::http_response_proxy> http_request_proxy::processResponse()
	{
		auto requestImpl = _get_httpCppCliBridge()->requestImpl;
		auto response = make_empty_response();
		response->_get_httpCppCliBridge() = this->_get_httpCppCliBridge();
		RequestProxyFriend requestProxyFriend(this->weak_from_this());
		auto responseImpl = requestImpl->Response;
		if (responseImpl->StatusCode.HasValue)
		{
			response->set_status_code(static_cast<int>(responseImpl->StatusCode.Value));
			response->set_reason_phrase(msclr::interop::marshal_as<http::reason_phrase>(responseImpl->StatusDescription) /*get_default_reason_phrase(static_cast<int>(responseImpl->StatusCode.Value))*/);
		}
		/*else if(responseImpl->NetErrorMessage != nullptr)
				response->set_reason_phrase(msclr::interop::marshal_as<http::reason_phrase>(responseImpl->NetErrorMessage));*/
		response->headers() = _get_httpCppCliBridge()->headersMarshaller->getHeaders(responseImpl->Headers);
		return response;
	}
}
