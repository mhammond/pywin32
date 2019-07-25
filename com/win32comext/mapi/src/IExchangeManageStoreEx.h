#include <MAPIX.h>
#include <EdkMdb.h>

// 0x7CFF001E
#define PR_PROFILE_MDB_DN PROP_TAG(PT_STRING8, 0x7CFF)
// 0x7CFE000B
#define PR_FORCE_USE_ENTRYID_SERVER PROP_TAG(PT_BOOLEAN, 0x7CFE)

/*------------------------------------------------------------------------ * *
"IExchangeManageStoreEx" Interface Declaration
* * Used for store management functions.
* *-----------------------------------------------------------------------*/

#define EXCHANGE_IEXCHANGEMANAGESTOREEX_METHODS(IPURE) \
    MAPIMETHOD(CreateStoreEntryID2)                    \
    (THIS_ ULONG cValues, LPSPropValue lpPropArray, ULONG ulFlags, ULONG * lpcbEntryID, LPENTRYID * lppEntryID) IPURE;

#undef INTERFACE
#define INTERFACE IExchangeManageStoreEx
DECLARE_MAPI_INTERFACE_(IExchangeManageStoreEx, IUnknown){MAPI_IUNKNOWN_METHODS(PURE)
                                                              EXCHANGE_IEXCHANGEMANAGESTORE_METHODS(PURE)
                                                                  EXCHANGE_IEXCHANGEMANAGESTOREEX_METHODS(PURE)};
#undef IMPL
#define IMPL

DECLARE_MAPI_INTERFACE_PTR(IExchangeManageStoreEx, LPEXCHANGEMANAGESTOREEX);

DEFINE_GUID(IID_IExchangeManageStoreEx, 0x7fe3c629, 0x4d9a, 0x4510, 0xa4, 0x79, 0x56, 0x96, 0x2b, 0x24, 0x6d, 0xc6);
