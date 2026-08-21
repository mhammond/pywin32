// Defines the Active Script Debugging GUID constants (IID_IDebug*,
// CLSID_*DebugManager*, IID_IActiveScriptDebug*, ...) that mingw-w64's uuid
// import library omits but MSVC's uuid.lib supplies.
//
// This must be its own translation unit: INITGUID makes <combaseapi.h> skip
// <cguid.h> (dropping GUID_NULL / IID_NULL) and disturbs other GUID headers, so
// it cannot be set in TUs that use those symbols. Here it is harmless -- this
// TU only *defines* GUIDs (it pulls in only <activdbg.h>, not PythonCOM.h). The
// definitions are DECLSPEC_SELECTANY, so they merge; the other TUs reference
// them extern. Mirrors com/win32com/src/MinGWGUIDs.cpp.
//
// Empty on MSVC, where uuid.lib already supplies these constants.
#ifdef __MINGW32__
#define INITGUID
#include <activdbg.h>
#endif
