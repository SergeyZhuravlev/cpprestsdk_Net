#include "stdafx.h"
#include "..\..\include\cpprest\details\HeadersMarshaller.h"

CppRestNetImpl::HeaderRangeValue^ web::http::details::HeadersMarshaller::GetRangeHeaders(const web::http::http_headers& pairs)
{
	for (auto&& header : pairs)
	{
		auto key = msclr::interop::marshal_as<System::String^>(header.first);
		auto value = msclr::interop::marshal_as<System::String^>(header.second);
		if (key->Equals(CppRestNetImpl::CppRestConsts::RangeHeader))
			return gcnew CppRestNetImpl::HeaderRangeValue(key, value);
	}
	return nullptr;
}

System::Net::WebHeaderCollection^ web::http::details::HeadersMarshaller::GetHeaders(const web::http::http_headers& pairs)
{
	auto result = gcnew System::Net::WebHeaderCollection();
	for (auto&& header : pairs)
	{
		auto key = msclr::interop::marshal_as<System::String^>(header.first);
		auto value = msclr::interop::marshal_as<System::String^>(header.second);
		if (!IgnoredHttpHeader(header))
			result->Add(key, value);
	}
	return result;
}

void web::http::details::HeadersMarshaller::FillFromHeaders(const http_headers& pairs, System::Net::HttpWebRequest^ request)
{
	for (auto&& header : pairs)
	{
		auto key = msclr::interop::marshal_as<System::String^>(header.first);
		auto value = msclr::interop::marshal_as<System::String^>(header.second);
		WebRequestFiller(request, key, value);
	}
}

bool web::http::details::HeadersMarshaller::IgnoredHttpHeader(http_headers::const_reference pair)
{
	auto keyName = msclr::interop::marshal_as<System::String^>(pair.first);
	return _ignoredHttpHeaders->Contains(keyName);
}

void web::http::details::HeadersMarshaller::WebRequestFiller(System::Net::HttpWebRequest^ request, System::String^ key, System::String^ value)
{
	if (CppRestNetImpl::CppRestConsts::UserAgentHeader->Equals(key))
		request->UserAgent = value;
	if (CppRestNetImpl::CppRestConsts::ContentLengthHeader->Equals(key))
		request->ContentLength = System::Int64::Parse(value);
	if (CppRestNetImpl::CppRestConsts::ContentTypeHeader->Equals(key))
		request->ContentType = value;
	if (CppRestNetImpl::CppRestConsts::AcceptHeader->Equals(key))
		request->Accept = value;
	if (CppRestNetImpl::CppRestConsts::ConnectionHeader->Equals(key))
		request->Connection = value;
	if (CppRestNetImpl::CppRestConsts::DateHeader->Equals(key))
		request->Date = System::DateTime::Parse(value);
	if (CppRestNetImpl::CppRestConsts::ExpectHeader->Equals(key))
		request->Expect = value;
	if (CppRestNetImpl::CppRestConsts::HostHeader->Equals(key))
		request->Host = value;
	if (CppRestNetImpl::CppRestConsts::IfModifiedSinceHeader->Equals(key))
		request->IfModifiedSince = System::DateTime::Parse(value);
	if (CppRestNetImpl::CppRestConsts::RefererHeader->Equals(key))
		request->Referer = value;
	if (CppRestNetImpl::CppRestConsts::TransferEncodingHeader->Equals(key))
		request->TransferEncoding = value;
}

web::http::http_headers web::http::details::HeadersMarshaller::getHeaders(System::Net::WebHeaderCollection^ pairs)
{
	http_headers result;
	for each (auto key in pairs->AllKeys)
	{
		auto value = pairs->Get(key);
		result.add(
			msclr::interop::marshal_as<utility::string_t>(key),
			msclr::interop::marshal_as<utility::string_t>(value));
	}
	return result;
}

System::Collections::Generic::ISet<System::String^>^ web::http::details::HeadersMarshaller::IgnoredHttpHeadersInit()
{
	auto ignoredHeaders = gcnew System::Collections::Generic::HashSet<System::String^>();
	ignoredHeaders->Add(CppRestNetImpl::CppRestConsts::UserAgentHeader);
	ignoredHeaders->Add(CppRestNetImpl::CppRestConsts::ContentLengthHeader);
	ignoredHeaders->Add(CppRestNetImpl::CppRestConsts::ContentTypeHeader);
	ignoredHeaders->Add(CppRestNetImpl::CppRestConsts::RangeHeader);
	ignoredHeaders->Add(CppRestNetImpl::CppRestConsts::AcceptHeader);
	ignoredHeaders->Add(CppRestNetImpl::CppRestConsts::ConnectionHeader);
	ignoredHeaders->Add(CppRestNetImpl::CppRestConsts::DateHeader);
	ignoredHeaders->Add(CppRestNetImpl::CppRestConsts::ExpectHeader);
	ignoredHeaders->Add(CppRestNetImpl::CppRestConsts::HostHeader);
	ignoredHeaders->Add(CppRestNetImpl::CppRestConsts::IfModifiedSinceHeader);
	ignoredHeaders->Add(CppRestNetImpl::CppRestConsts::RefererHeader);
	ignoredHeaders->Add(CppRestNetImpl::CppRestConsts::TransferEncodingHeader);
	return ignoredHeaders;
}
