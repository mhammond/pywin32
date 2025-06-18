#define INITGUID

#define USES_IID_IExchangeFolderACLs

#ifndef BUILD_FREEZE
/* In a frozen environemnt, these are likely to be picked
up by the MAPI module */

#define USES_IID_IMsgStore
#define USES_IID_IMAPISession
#define USES_IID_IAttachment
#define USES_IID_IProfSect
#define USES_IID_IMAPIStatus
#define USES_IID_IMailUser
#define USES_IID_IDistList
#define USES_IID_IABContainer
#define USES_IID_IProfSect
#define USES_IID_IMessage
#define USES_IID_IMAPIFolder
#define USES_IID_IAddrBook
#define USES_IID_IMAPIProp
#define USES_IID_IMAPIPropData
#define USES_IID_IMAPIContainer

#endif /* BUILD_FREEZE */
#include "windows.h"
#include "mapiguid.h"
#include "edkguid.h"
