#include "stdafx.h"
#include "..\..\include\cpprest\details\streambuf_from_Stream.h"

namespace Concurrency::streams::details
{
	streambuf_from_Stream::streambuf_from_Stream(System::IO::Stream^ source, std::optional<int64_t> contentLength, std::shared_ptr<void> holder):
		_source(source),
		_contentLength(contentLength),
		_holder(holder)
	{
	}

	streambuf_from_Stream::~streambuf_from_Stream()
	{
		CPPREST_BEGIN_MANAGED_TRY
		auto source = _source;
		_source = nullptr;
		source->~Stream();
		CPPREST_END_MANAGED_TRY
	}

	bool streambuf_from_Stream::can_seek()
	{
		CPPREST_BEGIN_MANAGED_TRY
		return _source->CanSeek;
		CPPREST_END_MANAGED_TRY
	}

	istreambuf_type_erasure::size_type streambuf_from_Stream::in_avail()
	{
		CPPREST_BEGIN_MANAGED_TRY
			if (_contentLength.has_value())
				return _contentLength.value();
			return _source->Length;
		CPPREST_END_MANAGED_TRY
	}

	int streambuf_from_Stream::pubsync()
	{
		CPPREST_BEGIN_MANAGED_TRY
		_source->Flush();
		return 0;
		CPPREST_END_MANAGED_TRY
	}

	istreambuf_type_erasure::size_type streambuf_from_Stream::sputn(const void* ptr, size_type size)
	{
		CPPREST_BEGIN_MANAGED_TRY
			array<System::Byte>^ byteArray = gcnew array<System::Byte>(static_cast<int>(size));
			System::Runtime::InteropServices::Marshal::Copy(System::IntPtr(const_cast<void*>(ptr)), byteArray, 0, byteArray->Length);
			_source->Write(byteArray, 0, byteArray->Length);
			return byteArray->Length;
		CPPREST_END_MANAGED_TRY
	}

	istreambuf_type_erasure::size_type streambuf_from_Stream::ReadWholeBuffer(array<System::Byte>^ buffer, size_type bufferOffset)
	{
		size_type oldReallyReaded = 0, reallyReaded = 0;
		do
		{
			oldReallyReaded = reallyReaded;
			reallyReaded += _source->Read(buffer, reallyReaded+bufferOffset, buffer->Length-reallyReaded-bufferOffset);
		} while (reallyReaded != oldReallyReaded);
		return reallyReaded;
	}

	istreambuf_type_erasure::size_type streambuf_from_Stream::sgetn(void* ptr, size_type size)
	{
		CPPREST_BEGIN_MANAGED_TRY
		//System::Diagnostics::Debug::WriteLine("sgetn");
		if (size <= 0)
		{
			//System::Diagnostics::Debug::WriteLine("size <= 0");
			return 0;
		}
		size_type realyReaded = 0;
		if(_last_read_buffer.operator->() == nullptr)
		{
			//System::Diagnostics::Debug::WriteLine("_last_read_buffer == 0");
			auto newBuffer = gcnew array<System::Byte>((size_t)size);
			realyReaded = ReadWholeBuffer(newBuffer, 0);
			System::Runtime::InteropServices::Marshal::Copy(newBuffer, 0, System::IntPtr(ptr), (size_t)size);
			_last_read_buffer = newBuffer;
		}
		else if (_last_read_buffer->Length >= size)
		{
			//System::Diagnostics::Debug::WriteLine("_last_read_buffer->Length >= size");
			System::Runtime::InteropServices::Marshal::Copy(_last_read_buffer, 0, System::IntPtr(ptr), (size_t)size);
			realyReaded = size;
		}
		else if (_last_read_buffer->Length < size)
		{
			//System::Diagnostics::Debug::WriteLine("_last_read_buffer->Length < size");
			auto newBuffer = gcnew array<System::Byte>(_last_read_buffer->Length + (size_t)size);
			System::Array::Copy(_last_read_buffer, newBuffer, _last_read_buffer->Length);
			realyReaded = ReadWholeBuffer(newBuffer, _last_read_buffer->Length);
			_last_read_buffer = newBuffer;
		}
		return realyReaded;
		CPPREST_END_MANAGED_TRY
	}

	void streambuf_from_Stream::CursorMoveForward(size_type size)
	{
		CPPREST_BEGIN_MANAGED_TRY
		//System::Diagnostics::Debug::WriteLine("CursorMoveForward");
		if (size <= 0)
		{
			//System::Diagnostics::Debug::WriteLine("size <= 0");
			return;
		}
		if (_last_read_buffer.operator->() == nullptr)
		{
			//System::Diagnostics::Debug::WriteLine("_last_read_buffer.operator->() == nullptr");
			throw std::logic_error(__FUNCSIG__);
		}
		if (_contentLength)
		{
			//System::Diagnostics::Debug::WriteLine("_contentLength");
			_contentLength = std::make_optional(_contentLength.value() - size);
		}
		if (_last_read_buffer->Length == size)
		{
			//System::Diagnostics::Debug::WriteLine("_last_read_buffer->Length == size");
			_last_read_buffer = nullptr;
		}
		else 
			if (_last_read_buffer->Length > size)
			{
				//System::Diagnostics::Debug::WriteLine("_last_read_buffer->Length > size");
				auto newBuffer = gcnew array<System::Byte>(_last_read_buffer->Length - (size_t)size);
				System::Array::Copy(_last_read_buffer, (int)size, newBuffer, 0, (int)(_last_read_buffer->Length - (size_t)size));
				_last_read_buffer = newBuffer;
			}
		CPPREST_END_MANAGED_TRY
	}

	istreambuf_type_erasure::size_type streambuf_from_Stream::sbumpcn(void* ptr, size_type size)
	{
		CPPREST_BEGIN_MANAGED_TRY
		//System::Diagnostics::Debug::WriteLine("sbumpcn");
		if (size <= 0)
		{
			//System::Diagnostics::Debug::WriteLine("size <= 0");
			return 0;
		}
		auto result = sgetn(ptr, size);
		if (_last_read_buffer->Length < size)
		{
			//System::Diagnostics::Debug::WriteLine("_last_read_buffer->Length < size");
			throw std::logic_error(__FUNCSIG__);
		}
		CursorMoveForward(size);
		return result;
		CPPREST_END_MANAGED_TRY
	}

	istreambuf_type_erasure::size_type streambuf_from_Stream::snextcn(void* ptr, size_type size)
	{
		CPPREST_BEGIN_MANAGED_TRY
		sgetn(ptr, size);
		CursorMoveForward(size);
		return sgetn(ptr, size);
		CPPREST_END_MANAGED_TRY
	}

	istreambuf_type_erasure::pos_type streambuf_from_Stream::pubseekoff(pos_type pos,
																		std::ios_base::seekdir cur,
	                                                                    std::ios_base::openmode mode)
	{
		throw std::logic_error{"Not implemented streambuf_from_Stream::pubseekoff"};
	}

	istreambuf_type_erasure::pos_type streambuf_from_Stream::pubseekpos(pos_type pos, std::ios_base::openmode mode)
	{
		throw std::logic_error{"Not implemented streambuf_from_Stream::pubseekpos"};
	}
}
