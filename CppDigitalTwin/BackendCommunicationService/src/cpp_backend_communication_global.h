#ifdef WIN32
#ifdef CppBackendCommunication_EXPORTS
#define CPSBACKENDCOMMUNICATION_EXPORT __declspec(dllexport)
#else
#define CPSBACKENDCOMMUNICATION_EXPORT __declspec(dllimport)
#endif
#else
#define CPSBACKENDCOMMUNICATION_EXPORT
#endif // WIN32
