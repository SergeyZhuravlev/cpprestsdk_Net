#include "stdafx.h"
#include <cpprest\details\HttpCppCliBridge.h>

web::http::details::HttpCppCliBridge::HttpCppCliBridge(): headersMarshaller(gcnew HeadersMarshaller())
{
}

web::http::details::HttpCppCliBridge::~HttpCppCliBridge()
{
	delete cancellationTokenSource.operator->();
	delete responseImpl.operator->();
}
