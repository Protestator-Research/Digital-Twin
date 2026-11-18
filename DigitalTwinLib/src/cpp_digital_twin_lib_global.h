#pragma once
#if defined(_WIN32) || defined(WIN32)
#ifdef DigitalTwinLib_EXPORTS
#define CPPDIGITALTWINLIB_EXPORT __declspec(dllexport)
#else
#define CPPDIGITALTWINLIB_EXPORT __declspec(dllimport)
#endif
#else
#define CPPDIGITALTWINLIB_EXPORT
#endif // WIN32