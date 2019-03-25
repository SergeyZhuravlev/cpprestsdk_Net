#pragma once
#include "TriBool.h"

namespace cpprestsdk
{
	namespace details
	{
		class SystemCapability
		{
		private:
			static TriBool IsWindowsVistaOrGreater();

		public:
			static bool IsWinHttpSupportsTLS1_1AndTLS_1_2();
			//static bool IsWinHttpSupportsCheckServerCertificateRevocation();
		};
	}
}

