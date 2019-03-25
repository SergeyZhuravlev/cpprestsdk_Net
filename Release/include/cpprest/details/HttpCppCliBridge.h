#pragma once
#include <msclr/gcroot.h>
#include "HeadersMarshaller.h"

namespace web::http::details
{
	class HttpCppCliBridge
	{
	public:
		msclr::gcroot<CppRestNetImpl::RequestImpl^> requestImpl;
		msclr::gcroot<CppRestNetImpl::ResponseImpl^> responseImpl;
		msclr::gcroot<System::Threading::CancellationTokenSource^> cancellationTokenSource;
		msclr::gcroot<System::Threading::CancellationToken> cancellationToken;
		msclr::gcroot<HeadersMarshaller^> headersMarshaller;

		HttpCppCliBridge();
		virtual ~HttpCppCliBridge();
	};
}
