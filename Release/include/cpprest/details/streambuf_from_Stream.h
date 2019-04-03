#pragma once
#include "cpprest/istreambuf_type_erasure.h"
#include <optional>
#include <msclr/gcroot.h>
#include "cpprest/details/CpprestManagedTryCatch.h"

namespace Concurrency::streams::details
{
	//������ ��� ������������� �� ��������� ������� ����� ������ ������
	//��������� ������� in_avail ����� ��� ������������, �� ����� �� ��� ����� � cpprest. ����������� �� ����� �� ������������ ������
	//streambuf_from_Stream ����� � � ���� � � ����� � Stream
	class streambuf_from_Stream: public Concurrency::streams::istreambuf_type_erasure, public std::enable_shared_from_this<streambuf_from_Stream>
	{
	public:
		streambuf_from_Stream(System::IO::Stream^ source, std::optional<int64_t> contentLength, std::shared_ptr<void> holder = nullptr);

		bool can_seek() override;

		size_type in_avail() override;

		int pubsync() override;

		size_type sputn(const void* ptr, size_type size) override;

		size_type sgetn(void* ptr, size_type size) override;

		size_type sbumpcn(void* ptr, size_type size) override;

		size_type snextcn(void* ptr, size_type size) override;

		pos_type pubseekoff(pos_type pos, std::ios_base::seekdir cur, std::ios_base::openmode mode) override;

		pos_type pubseekpos(pos_type pos, std::ios_base::openmode mode) override;

		~streambuf_from_Stream() override;

	private:
		size_type ReadWholeBuffer(array<System::Byte>^ buffer, size_type bufferOffset);
		void CursorMoveForward(size_type size);

		std::optional<size_type> _contentLength;
		msclr::gcroot<System::IO::Stream^> _source;
		msclr::gcroot<array<System::Byte>^> _last_read_buffer;
		std::shared_ptr<void> _holder;
	};
}
