#pragma once
#include "astreambuf.h"
#include "istreambuf_type_erasure.h"

namespace Concurrency::streams
{
	/*// Forward declarations
	template<typename _CharType> class basic_istream;
	template<typename _CharType> class basic_ostream;*/

	/// <summary>
	/// Reference-counted stream buffer.
	/// </summary>
	/// <typeparam name="_CharType">
	/// The data type of the basic element of the <c>streambuf.</c>
	/// </typeparam>
	/// <typeparam name="_CharType2">
	/// The data type of the basic element of the <c>streambuf.</c>
	/// </typeparam>
	template<typename _CharType>
	class streambuf_type_erasure : public details::streambuf_state_manager<_CharType>
	{
		typedef concurrency::streams::char_traits<_CharType> traits;
		typedef typename traits::int_type int_type;
		typedef typename traits::pos_type pos_type;
		typedef typename traits::off_type off_type;
		
	public:
		/// <summary>
		/// Ctor
		/// </summary>
		streambuf_type_erasure(_In_ std::shared_ptr<istreambuf_type_erasure> streambuf, std::ios_base::openmode mode)
			: details::streambuf_state_manager<_CharType>(mode), m_buffer(streambuf)
		{}

		/// <summary>
		/// Destructor
		/// </summary>
		virtual ~streambuf_type_erasure()
		{
			this->_close_read();
			this->_close_write();
		}

	private:
		//
		// The functions overridden below here are documented elsewhere.
		// See astreambuf.h for further information.
		//
		virtual bool can_seek() const { return this->is_open() && m_buffer->can_seek(); }
		virtual bool has_size() const { return this->can_seek(); }

		virtual size_t in_avail() const { return (size_t)m_buffer->in_avail(); }

		virtual size_t buffer_size(std::ios_base::openmode) const { return 0; }
		virtual void set_buffer_size(size_t, std::ios_base::openmode) { return; }

		virtual pplx::task<bool> _sync() { return pplx::task_from_result(m_buffer->pubsync() == 0); }

		virtual pplx::task<int_type> _putc(_CharType ch) { return pplx::task_from_result(m_buffer->sputn(&ch, sizeof(ch))<sizeof(ch) ? traits::eof() : traits::to_int_type(ch)); }
		virtual pplx::task<size_t> _putn(const _CharType *ptr, size_t size) { return pplx::task_from_result(((size_t)m_buffer->sputn(ptr, size*sizeof(*ptr)))/sizeof(*ptr)); }

		size_t _sgetn(_Out_writes_(size) _CharType *ptr, _In_ size_t size) const { return m_buffer->sgetn(ptr, size * sizeof(*ptr))/sizeof(*ptr); }
		virtual size_t _scopy(_Out_writes_(size) _CharType *, _In_ size_t size) { (void)(size); return (size_t)-1; }

		virtual pplx::task<size_t> _getn(_Out_writes_(size) _CharType *ptr, _In_ size_t size) { return pplx::task_from_result(((size_t)m_buffer->sbumpcn(ptr, size * sizeof(*ptr))) / sizeof(*ptr)); }

		virtual int_type _sbumpc()
		{
			_CharType Ch;
			_CharType *ptr = &Ch;
			auto sgr = m_buffer->sbumpcn(ptr, sizeof(*ptr));
			return sgr < sizeof(*ptr) ? traits::eof() : traits::to_int_type(Ch);
		}
		virtual int_type _sgetc()
		{
			_CharType Ch;
			_CharType *ptr = &Ch;
			auto sgr = m_buffer->sgetn(ptr, sizeof(*ptr));
			return sgr < sizeof(*ptr) ? traits::eof() : traits::to_int_type(Ch);
		}

		virtual pplx::task<int_type> _bumpc()
		{
			_CharType Ch;
			_CharType *ptr = &Ch;
			auto sgr = m_buffer->sbumpcn(ptr, sizeof(*ptr));
			return pplx::task_from_result<int_type>(sgr < sizeof(*ptr) ? traits::eof() : traits::to_int_type(Ch));
		}
		virtual pplx::task<int_type> _getc()
		{
			_CharType Ch;
			_CharType *ptr = &Ch;
			auto sgr = m_buffer->sgetn(ptr, sizeof(*ptr));
			return pplx::task_from_result<int_type>(sgr < sizeof(*ptr) ? traits::eof() : traits::to_int_type(Ch));
		}
		virtual pplx::task<int_type> _nextc()
		{
			_CharType Ch;
			_CharType *ptr = &Ch;
			auto sgr = m_buffer->snextcn(ptr, sizeof(*ptr));
			return pplx::task_from_result<int_type>(sgr < sizeof(*ptr) ? traits::eof() : traits::to_int_type(Ch));
		}
		virtual pplx::task<int_type> _ungetc()
		{
			throw std::logic_error("Not implemented " __FUNCSIG__);
			/*return pplx::task_from_result<int_type>(m_buffer->sungetc());*/
		}

		virtual pos_type getpos(std::ios_base::openmode mode) const { return m_buffer->pubseekoff(0, std::ios_base::cur, mode)/sizeof(_CharType); }
		virtual pos_type seekpos(pos_type pos, std::ios_base::openmode mode) { return m_buffer->pubseekpos(pos*sizeof(_CharType), mode); }
		virtual pos_type seekoff(off_type off, std::ios_base::seekdir dir, std::ios_base::openmode mode) { return m_buffer->pubseekoff(off, dir, mode)/sizeof(_CharType); }

		virtual _CharType* _alloc(size_t) { return nullptr; }
		virtual void _commit(size_t) { throw std::logic_error("Not implemented " __FUNCSIG__); }

		virtual bool acquire(_CharType*& ptr, size_t& count) 
		{
			ptr = nullptr;
			count = 0;
			return false;
		}
		virtual void release(_CharType *, size_t) { throw std::logic_error("Not implemented " __FUNCSIG__); }

		template<typename CharType> friend class concurrency::streams::stdio_ostream;
		template<typename CharType> friend class concurrency::streams::stdio_istream;

		std::shared_ptr<istreambuf_type_erasure> m_buffer;
	};
	
	/*: public details::basic_streambuf<_CharType>
	{
	public:
		typedef typename details::basic_streambuf<_CharType>::traits    traits;
		typedef typename details::basic_streambuf<_CharType>::int_type  int_type;
		typedef typename details::basic_streambuf<_CharType>::pos_type  pos_type;
		typedef typename details::basic_streambuf<_CharType>::off_type  off_type;
		typedef typename details::basic_streambuf<_CharType>::char_type char_type;

		template <typename _CharType2> friend class streambuf;

		/// <summary>
		/// Constructor.
		/// </summary>
		/// <param name="ptr">A pointer to the concrete stream buffer implementation.</param>
		streambuf_type_erasure(_In_ const std::shared_ptr<details::istreambuf_type_erasure> &ptr) : m_buffer(ptr) {}

		/// <summary>
		/// Default constructor.
		/// </summary>
		streambuf_type_erasure() { }

		/// <summary>
		/// Converter Constructor.
		/// </summary>
		/// <typeparam name="AlterCharType">
		/// The data type of the basic element of the source <c>streambuf</c>.
		/// </typeparam>
		/// <param name="other">The source buffer to be converted.</param>
		template <typename AlterCharType>
		streambuf_type_erasure(const streambuf_type_erasure<AlterCharType> &other) :
			m_buffer(other.m_buffer)
		{
			static_assert(std::is_same<pos_type, typename details::basic_streambuf<AlterCharType>::pos_type>::value
				&& std::is_same<off_type, typename details::basic_streambuf<AlterCharType>::off_type>::value
				&& std::is_integral<_CharType>::value && std::is_integral<AlterCharType>::value
				&& std::is_integral<int_type>::value && std::is_integral<typename details::basic_streambuf<AlterCharType>::int_type>::value
				&& sizeof(_CharType) == sizeof(AlterCharType)
				&& sizeof(int_type) == sizeof(typename details::basic_streambuf<AlterCharType>::int_type),
				"incompatible stream character types");

			if (other.m_buffer->char_type_size() != sizeof(_CharType))
				throw new std::logic_error("incompatible stream character types");
		}

		/// <summary>
		/// Constructs an input stream head for this stream buffer.
		/// </summary>
		/// <returns><c>basic_istream</c>.</returns>
		concurrency::streams::basic_istream<_CharType> create_istream() const
		{
			if (!can_read()) throw std::runtime_error("stream buffer not set up for input of data");
			return concurrency::streams::basic_istream<_CharType>(*this);
		}

		/// <summary>
		/// Constructs an output stream for this stream buffer.
		/// </summary>
		/// <returns>basic_ostream</returns>
		concurrency::streams::basic_ostream<_CharType> create_ostream() const
		{
			if (!can_write()) throw std::runtime_error("stream buffer not set up for output of data");
			return concurrency::streams::basic_ostream<_CharType>(*this);
		}

		/// <summary>
		/// Checks if the stream buffer has been initialized or not.
		/// </summary>
		operator bool() const { return (bool)m_buffer; }

		/// <summary>
		/// Destructor
		/// </summary>
		virtual ~streambuf_type_erasure() { }

		const std::shared_ptr<details::istreambuf_type_erasure> & get_base() const
		{
			if (!m_buffer)
			{
				throw std::invalid_argument("Invalid streambuf object");
			}

			return m_buffer;
		}

		/// <summary>
		/// <c>can_read</c> is used to determine whether a stream buffer will support read operations (get).
		/// </summary>
		virtual bool can_read() const { return get_base()->can_read(); }

		/// <summary>
		/// <c>can_write</c> is used to determine whether a stream buffer will support write operations (put).
		/// </summary>
		virtual bool can_write() const { return  get_base()->can_write(); }

		/// <summary>
		/// <c>can_seek</c> is used to determine whether a stream buffer supports seeking.
		/// </summary>
		/// <returns>True if seeking is supported, false otherwise.</returns>
		virtual bool can_seek() const { return get_base()->can_seek(); }

		/// <summary>
		/// <c>has_size</c> is used to determine whether a stream buffer supports size().
		/// </summary>
		/// <returns>True if the <c>size</c> API is supported, false otherwise.</returns>
		virtual bool has_size() const { return get_base()->has_size(); }

		/// <summary>
		/// Gets the total number of characters in the stream buffer, if known. Calls to <c>has_size</c> will determine whether
		/// the result of <c>size</c> can be relied on.
		/// </summary>
		/// <returns>The total number of characters in the stream buffer.</returns>
		virtual utility::size64_t size() const { return get_base()->size(); }

		/// <summary>
		/// Gets the stream buffer size, if one has been set.
		/// </summary>
		/// <param name="direction">The direction of buffering (in or out)</param>
		/// <returns>The size of the internal buffer (for the given direction).</returns>
		/// <remarks>An implementation that does not support buffering will always return 0.</remarks>
		virtual size_t buffer_size(std::ios_base::openmode direction = std::ios_base::in) const { return get_base()->buffer_size(direction); }

		/// <summary>
		/// Sets the stream buffer implementation to buffer or not buffer.
		/// </summary>
		/// <param name="size">The size to use for internal buffering, 0 if no buffering should be done.</param>
		/// <param name="direction">The direction of buffering (in or out)</param>
		/// <remarks>An implementation that does not support buffering will silently ignore calls to this function and it will not have any effect on what is returned by subsequent calls to <see cref="::buffer_size method" />.</remarks>
		virtual void set_buffer_size(size_t size, std::ios_base::openmode direction = std::ios_base::in) { get_base()->set_buffer_size(size, direction); }

		/// <summary>
		/// For any input stream, <c>in_avail</c> returns the number of characters that are immediately available
		/// to be consumed without blocking. May be used in conjunction with <cref="::sbumpc method"/> to read data without
		/// incurring the overhead of using tasks.
		/// </summary>
		/// <returns>Number of characters that are ready to read.</returns>
		virtual size_t in_avail() const { return get_base()->in_avail(); }

		/// <summary>
		/// Checks if the stream buffer is open.
		/// </summary>
		/// <remarks>No separation is made between open for reading and open for writing.</remarks>
		/// <returns>True if the stream buffer is open for reading or writing, false otherwise.</returns>
		virtual bool is_open() const { return get_base()->is_open(); }

		/// <summary>
		/// <c>is_eof</c> is used to determine whether a read head has reached the end of the buffer.
		/// </summary>
		/// <returns>True if at the end of the buffer, false otherwise.</returns>
		virtual bool is_eof() const { return get_base()->is_eof(); }

		/// <summary>
		/// Closes the stream buffer, preventing further read or write operations.
		/// </summary>
		/// <param name="mode">The I/O mode (in or out) to close for.</param>
		virtual pplx::task<void> close(std::ios_base::openmode mode = (std::ios_base::in | std::ios_base::out))
		{
			// We preserve the check here to workaround a Dev10 compiler crash
			auto buffer = get_base();
			return buffer ? buffer->close(mode) : pplx::task_from_result();
		}

		/// <summary>
		/// Closes the stream buffer with an exception.
		/// </summary>
		/// <param name="mode">The I/O mode (in or out) to close for.</param>
		/// <param name="eptr">Pointer to the exception.</param>
		virtual pplx::task<void> close(std::ios_base::openmode mode, std::exception_ptr eptr)
		{
			// We preserve the check here to workaround a Dev10 compiler crash
			auto buffer = get_base();
			return buffer ? buffer->close(mode, eptr) : pplx::task_from_result();
		}

		/// <summary>
		/// Writes a single character to the stream.
		/// </summary>
		/// <param name="ch">The character to write</param>
		/// <returns>The value of the character. EOF if the write operation fails</returns>
		virtual pplx::task<int_type> putc(_CharType ch)
		{
			return get_base()->putc(ch);
		}

		/// <summary>
		/// Allocates a contiguous memory block and returns it.
		/// </summary>
		/// <param name="count">The number of characters to allocate.</param>
		/// <returns>A pointer to a block to write to, null if the stream buffer implementation does not support alloc/commit.</returns>
		virtual _CharType* alloc(size_t count)
		{
			return get_base()->alloc(count);
		}

		/// <summary>
		/// Submits a block already allocated by the stream buffer.
		/// </summary>
		/// <param name="count">The number of characters to be committed.</param>
		virtual void commit(size_t count)
		{
			get_base()->commit(count);
		}

		/// <summary>
		/// Gets a pointer to the next already allocated contiguous block of data.
		/// </summary>
		/// <param name="ptr">A reference to a pointer variable that will hold the address of the block on success.</param>
		/// <param name="count">The number of contiguous characters available at the address in 'ptr.'</param>
		/// <returns><c>true</c> if the operation succeeded, <c>false</c> otherwise.</returns>
		/// <remarks>
		/// A return of false does not necessarily indicate that a subsequent read operation would fail, only that
		/// there is no block to return immediately or that the stream buffer does not support the operation.
		/// The stream buffer may not de-allocate the block until <see cref="::release method" /> is called.
		/// If the end of the stream is reached, the function will return <c>true</c>, a null pointer, and a count of zero;
		/// a subsequent read will not succeed.
		/// </remarks>
		virtual bool acquire(_Out_ _CharType*& ptr, _Out_ size_t& count)
		{
			ptr = nullptr;
			count = 0;
			return get_base()->acquire(ptr, count);
		}

		/// <summary>
		/// Releases a block of data acquired using <see cref="::acquire method"/>. This frees the stream buffer to de-allocate the
		/// memory, if it so desires. Move the read position ahead by the count.
		/// </summary>
		/// <param name="ptr">A pointer to the block of data to be released.</param>
		/// <param name="count">The number of characters that were read.</param>
		virtual void release(_Out_writes_(count) _CharType *ptr, _In_ size_t count)
		{
			get_base()->release(ptr, count);
		}

		/// <summary>
		/// Writes a number of characters to the stream.
		/// </summary>
		/// <param name="ptr">A pointer to the block of data to be written.</param>
		/// <param name="count">The number of characters to write.</param>
		/// <returns>The number of characters actually written, either 'count' or 0.</returns>
		CASABLANCA_DEPRECATED("This API in some cases performs a copy. It is deprecated and will be removed in a future release. Use putn_nocopy instead.")
			virtual pplx::task<size_t> putn(const _CharType *ptr, size_t count)
		{
			return get_base()->putn(ptr, count);
		}

		/// <summary>
		/// Writes a number of characters to the stream. Note: callers must make sure the data to be written is valid until
		/// the returned task completes.
		/// </summary>
		/// <param name="ptr">A pointer to the block of data to be written.</param>
		/// <param name="count">The number of characters to write.</param>
		/// <returns>The number of characters actually written, either 'count' or 0.</returns>
		virtual pplx::task<size_t> putn_nocopy(const _CharType *ptr, size_t count)
		{
			return get_base()->putn_nocopy(ptr, count);
		}

		/// <summary>
		/// Reads a single character from the stream and advances the read position.
		/// </summary>
		/// <returns>The value of the character. EOF if the read fails.</returns>
		virtual pplx::task<int_type> bumpc()
		{
			return get_base()->bumpc();
		}

		/// <summary>
		/// Reads a single character from the stream and advances the read position.
		/// </summary>
		/// <returns>The value of the character. <c>-1</c> if the read fails. <c>-2</c> if an asynchronous read is required</returns>
		/// <remarks>This is a synchronous operation, but is guaranteed to never block.</remarks>
		virtual typename details::basic_streambuf<_CharType>::int_type sbumpc()
		{
			return get_base()->sbumpc();
		}

		/// <summary>
		/// Reads a single character from the stream without advancing the read position.
		/// </summary>
		/// <returns>The value of the byte. EOF if the read fails.</returns>
		virtual pplx::task<int_type> getc()
		{
			return get_base()->getc();
		}

		/// <summary>
		/// Reads a single character from the stream without advancing the read position.
		/// </summary>
		/// <returns>The value of the character. EOF if the read fails. <see cref="::requires_async method" /> if an asynchronous read is required</returns>
		/// <remarks>This is a synchronous operation, but is guaranteed to never block.</remarks>
		virtual typename details::basic_streambuf<_CharType>::int_type sgetc()
		{
			return get_base()->sgetc();
		}

		/// <summary>
		/// Advances the read position, then returns the next character without advancing again.
		/// </summary>
		/// <returns>The value of the character. EOF if the read fails.</returns>
		pplx::task<int_type> nextc()
		{
			return get_base()->nextc();
		}

		/// <summary>
		/// Retreats the read position, then returns the current character without advancing.
		/// </summary>
		/// <returns>The value of the character. EOF if the read fails. <see cref="::requires_async method" /> if an asynchronous read is required</returns>
		pplx::task<int_type> ungetc()
		{
			return get_base()->ungetc();
		}

		/// <summary>
		/// Reads up to a given number of characters from the stream.
		/// </summary>
		/// <param name="ptr">The address of the target memory area.</param>
		/// <param name="count">The maximum number of characters to read.</param>
		/// <returns>The number of characters read. O if the end of the stream is reached.</returns>
		virtual pplx::task<size_t> getn(_Out_writes_(count) _CharType *ptr, _In_ size_t count)
		{
			return get_base()->getn(ptr, count);
		}

		/// <summary>
		/// Copies up to a given number of characters from the stream, synchronously.
		/// </summary>
		/// <param name="ptr">The address of the target memory area.</param>
		/// <param name="count">The maximum number of characters to read.</param>
		/// <returns>The number of characters copied. O if the end of the stream is reached or an asynchronous read is required.</returns>
		/// <remarks>This is a synchronous operation, but is guaranteed to never block.</remarks>
		virtual size_t scopy(_Out_writes_(count) _CharType *ptr, _In_ size_t count)
		{
			return get_base()->scopy(ptr, count);
		}

		/// <summary>
		/// Gets the current read or write position in the stream.
		/// </summary>
		/// <param name="direction">The I/O direction to seek (see remarks)</param>
		/// <returns>The current position. EOF if the operation fails.</returns>
		/// <remarks>Some streams may have separate write and read cursors.
		///          For such streams, the direction parameter defines whether to move the read or the write cursor.</remarks>
		virtual typename details::basic_streambuf<_CharType>::pos_type getpos(std::ios_base::openmode direction) const
		{
			return get_base()->getpos(direction);
		}

		/// <summary>
		/// Seeks to the given position.
		/// </summary>
		/// <param name="pos">The offset from the beginning of the stream.</param>
		/// <param name="direction">The I/O direction to seek (see remarks).</param>
		/// <returns>The position. EOF if the operation fails.</returns>
		/// <remarks>Some streams may have separate write and read cursors. For such streams, the direction parameter defines whether to move the read or the write cursor.</remarks>
		virtual typename details::basic_streambuf<_CharType>::pos_type seekpos(typename details::basic_streambuf<_CharType>::pos_type pos, std::ios_base::openmode direction)
		{
			return get_base()->seekpos(pos, direction);
		}

		/// <summary>
		/// Seeks to a position given by a relative offset.
		/// </summary>
		/// <param name="offset">The relative position to seek to</param>
		/// <param name="way">The starting point (beginning, end, current) for the seek.</param>
		/// <param name="mode">The I/O direction to seek (see remarks)</param>
		/// <returns>The position. EOF if the operation fails.</returns>
		/// <remarks>Some streams may have separate write and read cursors.
		///          For such streams, the mode parameter defines whether to move the read or the write cursor.</remarks>
		virtual typename details::basic_streambuf<_CharType>::pos_type seekoff(typename details::basic_streambuf<_CharType>::off_type offset, std::ios_base::seekdir way, std::ios_base::openmode mode)
		{
			return get_base()->seekoff(offset, way, mode);
		}

		/// <summary>
		/// For output streams, flush any internally buffered data to the underlying medium.
		/// </summary>
		/// <returns><c>true</c> if the flush succeeds, <c>false</c> if not</returns>
		virtual pplx::task<void> sync()
		{
			return get_base()->sync();
		}

		/// <summary>
		/// Retrieves the stream buffer exception_ptr if it has been set.
		/// </summary>
		/// <returns>Pointer to the exception, if it has been set; otherwise, <c>nullptr</c> will be returned</returns>
		virtual std::exception_ptr exception() const
		{
			return get_base()->exception();
		}

	private:
		std::shared_ptr<details::basic_streambuf<_CharType>> m_buffer;

	};*/
}
