#pragma once
#pragma once
#ifdef CPPRESTPROXY_EXPORT
#define CPPRESTPROXY_API __declspec(dllexport)
#else
#define CPPRESTPROXY_API __declspec(dllimport)
#endif