#pragma once
#include <exception>
#include <string>
#include "..\..\include\cpprest\CppRestNativeExport.h"

namespace pplx
{
	/// <summary>
///     This class describes an exception thrown by the PPL tasks layer in order to force the current task
///     to cancel. It is also thrown by the <c>get()</c> method on <see cref="task Class">task</see>, for a
///     canceled task.
/// </summary>
/// <seealso cref="task::get Method"/>
/// <seealso cref="cancel_current_task Method"/>
/**/
	class CPPRESTNATIVE_API task_canceled : public std::exception
	{
	private:
		std::string _message;

	public:
		/// <summary>
		///     Constructs a <c>task_canceled</c> object.
		/// </summary>
		/// <param name="_Message">
		///     A descriptive message of the error.
		/// </param>
		/**/
		explicit task_canceled(_In_z_ const char* _Message) throw();

		/// <summary>
		///     Constructs a <c>task_canceled</c> object.
		/// </summary>
		/**/
		task_canceled() throw();
		~task_canceled() throw ();
		const char* what() const override;
	};
}
