#ifdef WIN32
#ifdef PhysicalTwinCommunication_EXPORTS
#define CPPPHYSICALTWINCOMMUNICATION_EXPORT __declspec(dllexport)
#else
#define CPPPHYSICALTWINCOMMUNICATION_EXPORT __declspec(dllimport)
#endif
#else
#define CPPPHYSICALTWINCOMMUNICATION_EXPORT
#endif // WIN32