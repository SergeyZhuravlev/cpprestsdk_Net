# cpprestsdk_Net

Implementing pplx::task, http\https client, pplx::streams
Cpprestsdk_Net based on Microsoft/cpprestsdk 2.10.1 and Windows only .Net Framework 4.0.

Configured and builded for v141_xp x86 Windows XP sp3 Microsoft Visual Studio 2017.

Requred CPPREST_TARGET_XP and CPPREST_FORCE_PPLX and posible WINVER=0x0501;_WIN32_WINNT=0x0501 preporcessor definitions.
Requred CppRestNative.lib;CppRestNetProxy.lib;cpprest141_xp_2_10.lib libraries to link.