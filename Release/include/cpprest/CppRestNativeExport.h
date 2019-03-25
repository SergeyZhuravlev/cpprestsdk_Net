#pragma once
#ifdef CPPRESTNATIVE_EXPORT
#define CPPRESTNATIVE_API __declspec(dllexport)
#else
#define CPPRESTNATIVE_API __declspec(dllimport)
#endif