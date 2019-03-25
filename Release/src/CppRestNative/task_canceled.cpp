#include "stdafx.h"
#include <pplx/task_canceled.h>

pplx::task_canceled::task_canceled(const char* _Message) throw(): _message(_Message)
{
}

pplx::task_canceled::task_canceled() throw(): exception()
{
}

pplx::task_canceled::~task_canceled() throw()
{
}

const char* pplx::task_canceled::what() const
{
	return _message.c_str();
}
