#pragma once
#if defined(_WIN32) || defined(WIN32)
#ifdef DigitalTwinStore_EXPORTS
#define CPPDIGITALTWINSTORE_EXPORT __declspec(dllexport)
#else
#define CPPDIGITALTWINSTORE_EXPORT __declspec(dllimport)
#endif
#else
#define CPPDIGITALTWINSTORE_EXPORT
#endif // WIN32
