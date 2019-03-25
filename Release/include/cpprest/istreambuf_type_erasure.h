#pragma once
#include "CppRestNativeExport.h"
#include <cstdint>
#include <ios>

namespace Concurrency::streams
{
	struct istreambuf_type_erasure
	{
		using pos_type = int64_t;
		using size_type = int64_t;
		virtual bool can_seek() = 0;
		virtual size_type in_avail() = 0;
		virtual int pubsync() = 0;
		virtual size_type sputn(const void* ptr, size_type size) = 0;
		virtual size_type sgetn(void* ptr, size_type size) = 0;
		virtual size_type sbumpcn(void* ptr, size_type size) = 0;
		virtual size_type snextcn(void* ptr, size_type size) = 0;
		virtual pos_type pubseekoff(pos_type pos, std::ios_base::seekdir cur, std::ios_base::openmode mode) = 0;
		virtual pos_type pubseekpos(pos_type pos, std::ios_base::openmode mode) = 0;
		virtual CPPRESTNATIVE_API ~istreambuf_type_erasure();
	};
}
