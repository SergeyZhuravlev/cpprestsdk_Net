#pragma once
#include "cpprest/http_headers.h"
#include <msclr/marshal_cppstd.h>
#include <msclr/marshal.h>

namespace web::http::details
{
	public ref class HeadersMarshaller
	{
	public:
		CppRestNetImpl::HeaderRangeValue^ GetRangeHeaders(const http_headers& pairs);
		System::Net::WebHeaderCollection^ GetHeaders(const http_headers& pairs);
		void FillFromHeaders(const http_headers& pairs, System::Net::HttpWebRequest^ request);
		http_headers getHeaders(System::Net::WebHeaderCollection^ pairs);

	private:
		bool IgnoredHttpHeader(http_headers::const_reference pair);
		void WebRequestFiller(System::Net::HttpWebRequest^ request, System::String^ key, System::String^ value);

		static System::Collections::Generic::ISet<System::String^>^ _ignoredHttpHeaders = IgnoredHttpHeadersInit();
		static System::Collections::Generic::ISet<System::String^>^ IgnoredHttpHeadersInit();
	};
}
