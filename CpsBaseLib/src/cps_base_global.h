#ifdef WIN32
#ifdef CPSBaseLib_EXPORTS
#define CPSBASELIB_EXPORT __declspec(dllexport)
#else
#define CPSBASELIB_EXPORT __declspec(dllimport)
#endif
#else
#define CPSBASELIB_EXPORT
#endif // WIN32
