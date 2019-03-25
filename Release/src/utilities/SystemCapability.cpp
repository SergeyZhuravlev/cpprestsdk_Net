#include "stdafx.h"
#include "Windows.h"
#include "..\..\include\cpprest\details\SystemCapability.h"


namespace cpprestsdk
{
	namespace details
	{
		TriBool SystemCapability::IsWindowsVistaOrGreater()
		{
			OSVERSIONINFO osvi;

			ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
			osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

			if (!GetVersionEx(&osvi))
				return TriBool::Unknown;

			/*bIsWindowsXPorLater =
				((osvi.dwMajorVersion > 5) ||
				((osvi.dwMajorVersion == 5) && (osvi.dwMinorVersion >= 1)));*/
			return (osvi.dwMajorVersion > 5 ? TriBool::True : TriBool::False);
		}

		bool SystemCapability::IsWinHttpSupportsTLS1_1AndTLS_1_2()
		{
			return get_value_or(IsWindowsVistaOrGreater(), true);
		}

		/*bool SystemCapability::IsWinHttpSupportsCheckServerCertificateRevocation()
		{
			return get_value_or(IsWindowsVistaOrGreater(), true);
		}*/
	}
}