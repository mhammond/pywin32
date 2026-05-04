#ifndef IMSCAPABILITIESGUID_H
#ifdef INITGUID
#include <mapiguid.h>
#define IMSCAPABILITIESGUID_H
#endif /* INITGUID */

// {00020393-0000-0000-C000-000000000046}
#if !defined(INITGUID) || defined(USES_IID_IMSCapabilities)
DEFINE_OLEGUID(IID_IMSCapabilities, 0x00020393, 0, 0);
#endif

#endif /* IMSCAPABILITIESGUID_H */

#ifndef IMSCAPABILITIES_H
#define IMSCAPABILITIES_H

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef BEGIN_INTERFACE
#define BEGIN_INTERFACE
#endif

// IMSCapabilities - advertises capabilities of the given store provider

/* Selector values for GetCapabilities() */
enum class MSCAP_SELECTOR
{
	MSCAP_SEL_RESERVED1   = 0,
	MSCAP_SEL_RESERVED2   = 1,
	MSCAP_SEL_FOLDER      = 2,
	MSCAP_SEL_RESERVED3   = 3,
	MSCAP_SEL_RESTRICTION = 4,
};

/* Return values for GetCapabilities */
/* Values based on selector used to query */

// MSCAP_SEL_FOLDER
// Support for folder homepages in non-default stores
#define MSCAP_SECURE_FOLDER_HOMEPAGES ((ULONG) 0x00000001)

// MSCAP_SEL_RESTRICTION
// Support for RES_ANNOTATION restrictions
#define MSCAP_RES_ANNOTATION ((ULONG) 0x00000001)

#define MAPI_IMSCAPABILITIES_METHODS(IPURE) \
	MAPIMETHOD_(ULONG, GetCapabilities) (THIS_ MSCAP_SELECTOR mscapSelector);

#undef INTERFACE
#define INTERFACE IMSCapabilities
DECLARE_MAPI_INTERFACE_(IMSCapabilities, IUnknown)
{
	MAPI_IUNKNOWN_METHODS(PURE)
	MAPI_IMSCAPABILITIES_METHODS(PURE)
};

DECLARE_MAPI_INTERFACE_PTR(IMSCapabilities, LPMSCAPABILITIES);

#ifdef __cplusplus
} /*	extern "C" */
#endif

#endif /* IMSCAPABILITIES_H */
