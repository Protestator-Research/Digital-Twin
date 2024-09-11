#ifdef WIN32
#ifdef CppDigitalTwinLib_EXPORTS
#define CPPDIGITALTWINLIB_EXPORT __declspec(dllexport)
#else
#define CPPDIGITALTWINLIB_EXPORT __declspec(dllimport)
#endif
#else
#define CPPDIGITALTWINLIB_EXPORT
#endif // WIN32