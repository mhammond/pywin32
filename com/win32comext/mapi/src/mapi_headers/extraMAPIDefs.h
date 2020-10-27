#pragma once

// https://blogs.msdn.microsoft.com/stephen_griffin/2011/10/13/the-elusive-0x81002746-error/
// https://github.com/stephenegriffin/mfcmapi
#define MAIL_E_NAMENOTFOUND MAKE_SCODE(SEVERITY_ERROR, 0x0100, 10054)
#define MAPI_E_STORE_FULL MAKE_MAPI_E(0x60C)
#define MAPI_E_LOCKID_LIMIT MAKE_MAPI_E(0x60D)
#define MAPI_E_NAMED_PROP_QUOTA_EXCEEDED MAKE_MAPI_E(0x900)
#define MAPI_E_PROFILE_DELETED MAKE_MAPI_E(0x204)
#define MAPI_E_RECONNECTED MAKE_MAPI_E(0x125)
#define MAPI_E_OFFLINE MAKE_MAPI_E(0x126)

#define MAPI_FORCE_ACCESS 0x00080000
