// Defines COM GUID constants that mingw-w64's uuid import library omits but
// MSVC's uuid.lib supplies (notably IID_IDispatchEx from <dispex.h>).
//
// This must be its own translation unit: INITGUID has side effects we cannot
// tolerate in the rest of pythoncom. It makes <combaseapi.h> skip <cguid.h>
// (dropping GUID_NULL / IID_NULL, which most TUs use) and flips the conditional
// declarations in MAPIGuid.h. Here it is harmless -- this TU only *defines*
// GUIDs, it never uses them. The definitions are DECLSPEC_SELECTANY, so they
// merge; the other TUs reference them extern. Mirrors the mapiguids.cpp /
// exchangeguids.cpp pattern used by the mapi extension.
//
// Empty on MSVC, where uuid.lib already supplies these constants.
#ifdef __MINGW32__
#define INITGUID
#include <dispex.h>  // IID_IDispatchEx
#endif
