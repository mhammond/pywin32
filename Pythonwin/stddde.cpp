// stddde.cpp
// From KB Article ID: Q92829

// See pywin32 bug [ 1414160 ] DDE sets off DEP (in demos and pythonwin too)
// _CALLHACK_ appears to be the cause :(
// #define _CALLHACK_

#include "stdafxdde.h"

//
// Constants
//

#define DDE_TIMEOUT 60000  // a minute.

//
// Format lists
//

static WORD SysFormatList[] = {CF_TEXT, NULL};

//
// Structure used to hold a clipboard id and its text name
//

typedef struct _CFTAGNAME {
    WORD wFmt;
    char *pszName;
} CFTAGNAME, FAR *PCFTAGNAME;

//
// Standard format name lookup table
//

CFTAGNAME CFNames[] = {CF_TEXT,    SZCF_TEXT,    CF_BITMAP, SZCF_BITMAP, CF_METAFILEPICT, SZCF_METAFILEPICT,
                       CF_SYLK,    SZCF_SYLK,    CF_DIF,    SZCF_DIF,    CF_TIFF,         SZCF_TIFF,
                       CF_OEMTEXT, SZCF_OEMTEXT, CF_DIB,    SZCF_DIB,    CF_PALETTE,      SZCF_PALETTE,
                       CF_PENDATA, SZCF_PENDATA, CF_RIFF,   SZCF_RIFF,   CF_WAVE,         SZCF_WAVE,
                       NULL,       NULL};

////////////////////////////////////////////////////////////////////////////////////
//
// ********** The Barfy bit *********************
//
// We only support one server per app right now
// sooooo: here's the global we use to find it in the
// hateful DDE callback routine
// Let's see if I can get away with this

// CT BEGIN
// this should vanish with multiple servers
#ifdef _CALLHACK_
static CDDEServer *pTheServerList = NULL;
#else
static CDDEServer *pTheServer = NULL;
#endif
// CT END

////////////////////////////////////////////////////////////////////////////////////
//
// CDDECountedObject

IMPLEMENT_DYNCREATE(CDDECountedObject, CObject);

CDDECountedObject::CDDECountedObject() { m_iRefCount = 0; }

CDDECountedObject::~CDDECountedObject() { ASSERT(m_iRefCount == 0); }

int CDDECountedObject::AddRef()
{
    ASSERT(m_iRefCount < 1000);  // sanity check
    return ++m_iRefCount;
}

int CDDECountedObject::Release()
{
    int i = --m_iRefCount;
    ASSERT(m_iRefCount >= 0);
    if (m_iRefCount == 0) {
        delete this;
    }
    return i;
}

////////////////////////////////////////////////////////////////////////////////////
//
// CHSZ

IMPLEMENT_DYNCREATE(CHSZ, CObject);

CHSZ::CHSZ()
{
    m_hsz = NULL;
    m_dwDDEInstance = 0;
}

CHSZ::CHSZ(CDDEServer *pServer, const TCHAR *szName)
{
    m_dwDDEInstance = pServer->m_dwDDEInstance;
    m_hsz = ::DdeCreateStringHandle(m_dwDDEInstance, szName, DDE_STRING_CODEPAGE);
    ASSERT(m_hsz);
    /*
    #ifdef _DEBUG
        if (m_hsz==NULL) {
            char buf[128];
            wsprintf(buf, "DdeCreateStringHandle() failed with 0x%x\n", DdeGetLastError(m_dwDDEInstance));
            OutputDebugString(buf);
        }
    #endif
    */
}

void CHSZ::Create(CDDEServer *pServer, const TCHAR *szName)
{
    if (m_hsz) {
        ::DdeFreeStringHandle(pServer->m_dwDDEInstance, m_hsz);
    }
    m_dwDDEInstance = pServer->m_dwDDEInstance;
    m_hsz = ::DdeCreateStringHandle(m_dwDDEInstance, szName, DDE_STRING_CODEPAGE);
    ASSERT(m_hsz);
}

void CHSZ::Destroy()
{
    if (m_hsz) {
        ::DdeFreeStringHandle(m_dwDDEInstance, m_hsz);
        m_hsz = NULL;
    }
    m_dwDDEInstance = 0;
}

CHSZ::~CHSZ() { Destroy(); }

////////////////////////////////////////////////////////////////////////////////////
//
// CDDEItem

IMPLEMENT_DYNCREATE(CDDEItem, CObject);

CDDEItem::CDDEItem() { m_pTopic = NULL; }

CDDEItem::~CDDEItem() {}

void CDDEItem::Create(const TCHAR *pszName) { m_strName = pszName; }

BOOL CDDEItem::Request(UINT wFmt, CDDEAllocator &allocr) { return FALSE; }

// CT BEGIN
BOOL CDDEItem::NSRequest(const TCHAR *szItem, CDDEAllocator &allocr) { return FALSE; }
// CT END

BOOL CDDEItem::Poke(UINT wFmt, void *pData, DWORD dwSize) { return FALSE; }

// CT BEGIN
BOOL CDDEItem::Poke(void *pData, DWORD dwSize) { return FALSE; }

BOOL CDDEItem::NSPoke(const TCHAR *szItem, void *pData, DWORD dwSize) { return FALSE; }
// CT END

BOOL CDDEItem::IsSupportedFormat(WORD wFormat)
{
    WORD *pFmt = GetFormatList();
    if (!pFmt)
        return FALSE;
    while (*pFmt) {
        if (*pFmt == wFormat)
            return TRUE;
        pFmt++;
    }
    return FALSE;
}

BOOL CDDEItem::CanAdvise(UINT wFmt) { return IsSupportedFormat(wFmt); }

void CDDEItem::PostAdvise()
{
    if (m_pTopic == NULL)
        return;
    m_pTopic->PostAdvise(this);
}

////////////////////////////////////////////////////////////////////////////////////
//
// CDDEStringItem

IMPLEMENT_DYNCREATE(CDDEStringItem, CDDEItem);

WORD *CDDEStringItem::GetFormatList()
{
    return SysFormatList;  // CF_TEXT
}

BOOL CDDEStringItem::Request(UINT wFmt, CDDEAllocator &allocr)
{
    ASSERT(wFmt == CF_TEXT);
    return allocr.Alloc(m_strData);
}

BOOL CDDEStringItem::Poke(UINT wFmt, void *pData, DWORD dwSize)
{
    ASSERT(wFmt == CF_TEXT);
    ASSERT(pData);
    m_strData = (TCHAR *)pData;
    OnPoke();
    return TRUE;
}

void CDDEStringItem::SetData(const TCHAR *pszData)
{
    ASSERT(pszData);
    m_strData = pszData;
    PostAdvise();
}

////////////////////////////////////////////////////////////////////////////////////
//
// CDDEItemList

IMPLEMENT_DYNCREATE(CDDEItemList, CObList);

CDDEItemList::CDDEItemList() {}

CDDEItemList::~CDDEItemList() {}

////////////////////////////////////////////////////////////////////////////////////
//
// CDDETopic

IMPLEMENT_DYNCREATE(CDDETopic, CObject);

CDDETopic::CDDETopic() {}

CDDETopic::~CDDETopic() {}

void CDDETopic::Create(const TCHAR *pszName) { m_strName = pszName; }

BOOL CDDETopic::AddItem(CDDEItem *pNewItem)
{
    ASSERT(pNewItem);

    //
    // See if we already have this item
    //

    POSITION pos = m_ItemList.Find(pNewItem);
    if (pos)
        return TRUE;  // already have it

    //
    // Add the new item
    //

    m_ItemList.AddTail(pNewItem);
    pNewItem->m_pTopic = this;

    return TRUE;
}

BOOL CDDETopic::Request(UINT wFmt, const TCHAR *pszItem, CDDEAllocator &allocr)
{
    //
    // See if we have this item
    //

    CDDEItem *pItem = FindItem(pszItem);
    if (!pItem)
        return FALSE;

    // CT BEGIN
    if (pItem->m_strName == "") {
        BOOL ret = NSRequest(pszItem, allocr);
        return ret;
    }
    // CT END
    return pItem->Request(wFmt, allocr);
}

// CT BEGIN
BOOL CDDETopic::NSRequest(const TCHAR *szItem, CDDEAllocator &allocr) { return FALSE; }
// CT END

BOOL CDDETopic::Poke(UINT wFmt, const TCHAR *pszItem, void *pData, DWORD dwSize)
{
    //
    // See if we have this item
    //

    CDDEItem *pItem = FindItem(pszItem);
    if (!pItem)
        return FALSE;

    // CT BEGIN
    if (pItem->m_strName == "") {
        BOOL ret = NSPoke(pszItem, pData, dwSize);
        return ret;
    }
    // CT END
    return pItem->Poke(wFmt, pData, dwSize);
}

// CT BEGIN

BOOL CDDETopic::Poke(const TCHAR *pszItem, void *pData, DWORD dwSize) { return Poke(CF_TEXT, pszItem, pData, dwSize); }

BOOL CDDETopic::NSPoke(const TCHAR *szItem, void *pData, DWORD dwSize) { return FALSE; }
// CT END

BOOL CDDETopic::Exec(void *pData, DWORD dwSize) { return FALSE; }

CDDEItem *CDDETopic::FindItem(const TCHAR *pszItem)
{
    POSITION pos = m_ItemList.GetHeadPosition();
    while (pos) {
        CDDEItem *pItem = m_ItemList.GetNext(pos);
        // CT BEGIN
        // NETSCAPE Hack
        if (pItem->m_strName == "") {
            return pItem;
        }
        // empty item matches all
        // CT END
        if (pItem->m_strName.CompareNoCase(pszItem) == 0)
            return pItem;
    }
    return NULL;
}

BOOL CDDETopic::CanAdvise(UINT wFmt, const TCHAR *pszItem)
{
    //
    // See if we have this item
    //

    CDDEItem *pItem = FindItem(pszItem);
    if (!pItem)
        return FALSE;

    return pItem->CanAdvise(wFmt);
}

void CDDETopic::PostAdvise(CDDEItem *pItem)
{
    ASSERT(m_pServer);
    ASSERT(pItem);
    m_pServer->PostAdvise(this, pItem);
}

////////////////////////////////////////////////////////////////////////////////////
//
// CDDETopicList

IMPLEMENT_DYNCREATE(CDDETopicList, CObList);

CDDETopicList::CDDETopicList() {}

CDDETopicList::~CDDETopicList() {}

////////////////////////////////////////////////////////////////////////////////////
//
// CDDEConv

IMPLEMENT_DYNCREATE(CDDEConv, CDDECountedObject);

CDDEConv::CDDEConv()
{
    m_pServer = NULL;
    m_hConv = NULL;
    m_hszTopic = NULL;
}

CDDEConv::CDDEConv(CDDEServer *pServer)
{
    m_pServer = pServer;
    m_hConv = NULL;
    m_hszTopic = NULL;
}

CDDEConv::CDDEConv(CDDEServer *pServer, HCONV hConv, HSZ hszTopic)
{
    m_pServer = pServer;
    m_hConv = hConv;
    m_hszTopic = hszTopic;
}

CDDEConv::~CDDEConv() { Terminate(); }

BOOL CDDEConv::Terminate()
{
    if (m_hConv) {
        //
        // Terminate this conversation
        //

        ::DdeDisconnect(m_hConv);

        //
        // Tell the server
        //

        ASSERT(m_pServer);
        m_pServer->RemoveConversation(m_hConv);

        m_hConv = NULL;

        return TRUE;
    }

    return FALSE;  // wasn't active
}

BOOL CDDEConv::ConnectTo(const TCHAR *pszService, const TCHAR *pszTopic)
{
    ASSERT(pszService);
    ASSERT(pszTopic);
    ASSERT(m_pServer);
    // CT BEGIN
    // this case was not handled:
    if (m_hConv) {
        int lev = this->AddRef();  // keep me alive for re-use
        Terminate();               // removes one ref
    }

    // CT END
    ASSERT(!m_hConv);

    CHSZ hszService(m_pServer, pszService);
    CHSZ hszTopic(m_pServer, pszTopic);

    //
    // Try to connect
    //

    m_hConv = ::DdeConnect(m_pServer->m_dwDDEInstance, hszService, hszTopic, NULL);

    if (!m_hConv) {
        DWORD dwErr = GetLastError();
        m_pServer->Status(_T("Failed to connect to %s|%s. Error %u"), (const TCHAR *)pszService,
                          (const TCHAR *)pszTopic, dwErr);
        return FALSE;
    }

    //
    // Add this conversation to the server list
    //

    m_pServer->AddConversation(this);
    return TRUE;
}

BOOL CDDEConv::AdviseData(UINT wFmt, const TCHAR *pszTopic, const TCHAR *pszItem, void *pData, DWORD dwSize)
{
    return FALSE;
}

BOOL CDDEConv::Request(const TCHAR *pszItem, CString &ret)
{
    ASSERT(m_pServer);
    ASSERT(pszItem);

    CHSZ hszItem(m_pServer, pszItem);
    HDDEDATA hData = ::DdeClientTransaction(NULL, 0, m_hConv, hszItem,
#if defined(UNICODE)
                                            CF_UNICODETEXT,
#else
                                            CF_TEXT,
#endif
                                            XTYP_REQUEST, DDE_TIMEOUT, NULL);

    if (!hData) {
        return FALSE;
    }

    //
    // Copy the result data
    //
    DWORD dwSize;
    BYTE *pData = ::DdeAccessData(hData, &dwSize);
    DWORD nChars = (dwSize / sizeof(TCHAR)) - 1;
    ret = CString((TCHAR *)pData, nChars);
    ::DdeUnaccessData(hData);
    // MSDN sez 'When an application has finished using the data handle
    // returned by DdeClientTransaction, the application should free the
    // handle by calling the DdeFreeDataHandle function.' - which would
    // be about now!
    ::DdeFreeDataHandle(hData);
    return TRUE;
}

BOOL CDDEConv::Advise(const TCHAR *pszItem)
{
    ASSERT(m_pServer);
    ASSERT(pszItem);

    CHSZ hszItem(m_pServer, pszItem);
    HDDEDATA hData = ::DdeClientTransaction(NULL, 0, m_hConv, hszItem, CF_TEXT, XTYP_ADVSTART, DDE_TIMEOUT, NULL);

    if (!hData) {
        // Failed
        return FALSE;
    }
    return TRUE;
}

BOOL CDDEConv::Exec(const TCHAR *pszCmd)
{
    //
    // Send the command
    //

    HDDEDATA hData = ::DdeClientTransaction((BYTE *)pszCmd, (_tcslen(pszCmd) + 1) * sizeof(TCHAR), m_hConv, 0, CF_TEXT,
                                            XTYP_EXECUTE, DDE_TIMEOUT, NULL);

    if (!hData) {
        // Failed
        return FALSE;
    }
    return TRUE;
}

BOOL CDDEConv::Poke(UINT wFmt, const TCHAR *pszItem, void *pData, DWORD dwSize)
{
    //
    // Send the command
    //

    CHSZ hszItem(m_pServer, pszItem);
    HDDEDATA hData =
        ::DdeClientTransaction((BYTE *)pData, dwSize, m_hConv, hszItem, wFmt, XTYP_POKE, DDE_TIMEOUT, NULL);

    if (!hData) {
        // Failed
        return FALSE;
    }
    return TRUE;
}

// CT BEGIN
BOOL CDDEConv::Poke(const TCHAR *pszItem, void *pData, DWORD dwSize)
{
    //
    // format-less version for Netscape defaults to text
    //
    return Poke(CF_TEXT, pszItem, pData, dwSize);
}
// CT END

////////////////////////////////////////////////////////////////////////////////////
//
// CDDEConvList

IMPLEMENT_DYNCREATE(CDDEConvList, CObList);

CDDEConvList::CDDEConvList() {}

CDDEConvList::~CDDEConvList() {}

////////////////////////////////////////////////////////////////////////////////////
//
// Topics and items to support the 'system' topic

//
// Generic system topic items
//

IMPLEMENT_DYNCREATE(CDDESystemItem, CDDEItem);

WORD *CDDESystemItem::GetFormatList() { return SysFormatList; }

//
// Specific system topic items
//

IMPLEMENT_DYNCREATE(CDDESystemItem_TopicList, CDDESystemItem);

BOOL CDDESystemItem_TopicList::Request(UINT wFmt, CDDEAllocator &allocr)
{
    //
    // Return the list of topics for this service
    //

    static CString strTopics;
    strTopics = "";
    ASSERT(m_pTopic);
    CDDEServer *pServer = m_pTopic->m_pServer;
    ASSERT(pServer);
    POSITION pos = pServer->m_TopicList.GetHeadPosition();
    int items = 0;
    while (pos) {
        CDDETopic *pTopic = pServer->m_TopicList.GetNext(pos);

        //
        // put in a tab delimiter unless this is the first item
        //

        if (items != 0)
            strTopics += SZ_TAB;

        //
        // Copy the string name of the item
        //

        strTopics += pTopic->m_strName;

        items++;
    }

    //
    // Set up the return info
    //
    return allocr.Alloc(strTopics);
}

IMPLEMENT_DYNCREATE(CDDESystemItem_ItemList, CDDESystemItem);

BOOL CDDESystemItem_ItemList::Request(UINT wFmt, CDDEAllocator &allocr)
{
    //
    // Return the list of items in this topic
    //

    static CString strItems;
    strItems = "";
    ASSERT(m_pTopic);
    POSITION pos = m_pTopic->m_ItemList.GetHeadPosition();
    int items = 0;
    while (pos) {
        CDDEItem *pItem = m_pTopic->m_ItemList.GetNext(pos);

        //
        // put in a tab delimiter unless this is the first item
        //

        if (items != 0)
            strItems += SZ_TAB;

        //
        // Copy the string name of the item
        //

        strItems += pItem->m_strName;

        items++;
    }

    //
    // Set up the return info
    //
    return allocr.Alloc(strItems);
}

IMPLEMENT_DYNCREATE(CDDESystemItem_FormatList, CDDESystemItem);

BOOL CDDESystemItem_FormatList::Request(UINT wFmt, CDDEAllocator &allocr)
{
    //
    // Return the list of formats in this topic
    //

    static CString strFormats;
    strFormats = "";
    ASSERT(m_pTopic);
    POSITION pos = m_pTopic->m_ItemList.GetHeadPosition();
    int iFormats = 0;
    WORD wFmtList[100];
    while (pos) {
        CDDEItem *pItem = m_pTopic->m_ItemList.GetNext(pos);

        //
        // get the format list for this item
        //

        WORD *pItemFmts = pItem->GetFormatList();
        if (pItemFmts) {
            //
            // Add each format to the list if we don't have it already
            //

            while (*pItemFmts) {
                //
                // See if we have it
                //

                int i;
                for (i = 0; i < iFormats; i++) {
                    if (wFmtList[i] == *pItemFmts)
                        break;  // have it already
                }

                if (i == iFormats) {
                    //
                    // This is a new one
                    //

                    wFmtList[iFormats] = *pItemFmts;

                    //
                    // Add the string name to the list
                    //

                    //
                    // put in a tab delimiter unless this is the first item
                    //

                    if (iFormats != 0)
                        strFormats += SZ_TAB;

                    //
                    // Copy the string name of the item
                    //

                    strFormats += ::GetFormatName(*pItemFmts);

                    iFormats++;
                }

                pItemFmts++;
            }
        }
    }

    //
    // Set up the return info
    //
    return allocr.Alloc(strFormats);
}

IMPLEMENT_DYNCREATE(CDDEServerSystemTopic, CDDETopic);

BOOL CDDEServerSystemTopic::Request(UINT wFmt, const TCHAR *pszItem, CDDEAllocator &allocr)
{
    m_pServer->Status(_T("System topic request: %s"), pszItem);
    return CDDETopic::Request(wFmt, pszItem, allocr);
}

////////////////////////////////////////////////////////////////////////////////////
//
// CDDEServer

IMPLEMENT_DYNCREATE(CDDEServer, CObject);

CDDEServer::CDDEServer()
{
    m_bInitialized = FALSE;
    m_strServiceName = AfxGetAppName();
    m_dwDDEInstance = 0;
    m_pSystemTopic = NULL;
}

CDDEServer::~CDDEServer() { Shutdown(); }

void CDDEServer::Shutdown()
{
    if (m_bInitialized) {
        //
        // Terminate all conversations
        //

        POSITION pos = m_ConvList.GetHeadPosition();
        while (pos) {
            CDDEConv *pConv = m_ConvList.GetNext(pos);
            ASSERT(pConv);
            pConv->Terminate();
        }

        //
        // Unregister the service name
        //

        ::DdeNameService(m_dwDDEInstance, m_hszServiceName, NULL, DNS_UNREGISTER);

        //
        // Release DDEML
        //

        ::DdeUninitialize(m_dwDDEInstance);
        m_dwDDEInstance = 0;
        m_bInitialized = FALSE;

// CT BEGIN
#ifdef _CALLHACK_
        CDDEServer *pOther = pTheServerList;
        while (pOther->pServers != this) pOther = pOther->pServers;
        pOther->pServers = this->pServers;
        if (pTheServerList == this)
            pTheServerList = pOther;
        if (pTheServerList == this)
            pTheServerList = NULL;
#else
        ASSERT(pTheServer == this);
        pTheServer = NULL;
#endif
        // CT END
        delete m_pSystemTopic;
        m_pSystemTopic = NULL;
    }
}

//////////////////////////////////////////////////////////////////////
//
// Support for an unlimited number of DDE servers
//
// crude hack by C. Tismer 980913
//

//
// this is just a fake to have something real to address.
//
void CALLBACK _target_proc(int BARF)
{
    //	printf ("pointer=%x\n", BARF) ;
}

//
// this naked function is a template which is used to generate the callback stub.
// the idea is to push another parameter beneath the callback parameters.
// since the callback uses __stdcall, we just have to insert an entry above SP.
// The real function can then be called by a simple jump.
//
// Gaah - it took me two days to realize that I need "offset" here
//
#ifdef _CALLHACK_
__declspec(naked) void _template()
{
    __asm {
		pop eax  // save the return addr
		push offset _template  // stuff the parameter in
		push eax  // restore retaddr
		push offset _target_proc  // continue with the real one
		ret far  // absolute jump is better; "jmp" would be relative
    }
}

#pragma pack(push, 1)
typedef struct tagJMPENTRY {
    byte jmp;
    void *reladr;
} JMPENTRY;

typedef struct tagCBTEMPLATE {
    BYTE pop;
    BYTE push_self;
    DWORD _self_;
    BYTE push_eax;
    BYTE push_target;
    DWORD _target_;
    BYTE retf;
} CBTEMPLATE;
#pragma pack(pop)

void *_unindirect(void *ptr)
{
    JMPENTRY *look;
    look = (JMPENTRY *)ptr;
    //
    // in debug mode, we have to follow a jump table entry.
    //
    if (look->jmp == 0xe9)
        ptr = (byte *)ptr + (int)look->reladr + sizeof(JMPENTRY);
    return ptr;
}

BOOL make_template(CBTEMPLATE *code, DWORD param, void *target)
{
    //""" returns true if the compiled code seems suitable.
    // false, if something is unforeseen.
    //"""
    void *ptr = (void *)_template;
    ptr = _unindirect(ptr);
    CBTEMPLATE *cbptr = (CBTEMPLATE *)ptr;
    //
    // check if our source model matches, for sanity.
    //
    if (!(cbptr->_self_ == (DWORD)_template && cbptr->_target_ == (DWORD)_target_proc))
        return false;
    //
    // Now that we located our code, we make a copy to the dynamic instance.
    //
    memmove(code, ptr, sizeof(*code));
    //
    // check again if our target model matches, for sanity.
    //
    if (!(code->_self_ == (DWORD)_template && code->_target_ == (DWORD)_target_proc))
        return false;
    //
    // now we believe in the hack and insert our parameters.
    //
    code->_self_ = param;
    code->_target_ = (DWORD)_unindirect(target);
    return true;
}

// END of crude hack.
//
//////////////////////////////////////////////////////////////////////
#endif  // _CALLHACK_

BOOL CDDEServer::Create(const TCHAR *pszServiceName, DWORD dwFilterFlags /* = 0 */, DWORD *pdwDDEInst /* = NULL */)
{
    //
    // make sure we are alone in the world
    //

// CT BEGIN
#ifdef _CALLHACK_
    if (pTheServerList != NULL) {
        // insert me into the ring
        CDDEServer *pOther = pTheServerList;
        while (pOther->pServers != pTheServerList) pOther = pOther->pServers;
        this->pServers = pTheServerList;
        pOther->pServers = this;
    }
    else {
        pTheServerList = this;
        this->pServers = this;
    }
#else
    if (pTheServer != NULL) {
        TRACE("Already got a server!\n");
        ASSERT(0);
        return FALSE;
    }
    else {
        pTheServer = this;
    }
#endif
    // as a change, we do no longer use the one pTheServer variable
    // but build a ring of servers which is found at pTheServer.
#ifdef _CALLHACK_
    if (sizeof(DynamicCallback) < sizeof(CBTEMPLATE)) {
        TRACE("Please look into the DynamicCallback structure!\n");
        ASSERT(0);
        return false;
    }
    CBTEMPLATE *thisDynamicCallback = (CBTEMPLATE *)&this->DynamicCallback;
    if (!(make_template(thisDynamicCallback, (DWORD)this, DynDDECallback)))
        return false;
#endif _CALLHACK_
    // CT END
    //
    // Make sure the application hasn't requested any filter options
    // which will prevent us from working correctly.
    //
    /* Not for Python tho!
        dwFilterFlags &= !(CBF_FAIL_CONNECTIONS
                          |CBF_SKIP_CONNECT_CONFIRMS
                          | CBF_SKIP_DISCONNECTS
                          | CBF_FAIL_SELFCONNECTIONS);
    */
    //
    // Initialize DDEML.  Note that DDEML doesn't make any callbacks
    // during initialization so we don't need to worry about the
    // custom callback yet.
    //

    UINT uiResult;

// CT BEGIN
#ifdef _CALLHACK_
    uiResult = ::DdeInitialize(&m_dwDDEInstance, (PFNCALLBACK)(thisDynamicCallback), dwFilterFlags, 0);
#else
    uiResult = ::DdeInitialize(&m_dwDDEInstance, (PFNCALLBACK)&StdDDECallback, dwFilterFlags, 0);
#endif
    // CT END

    if (uiResult != DMLERR_NO_ERROR)
        return FALSE;

    // MH - Added from DDEINST.C sample.
    // Start a critical section. This fixes a problem where the DDEML
    // Can hang under threaded conditions.
    CRITICAL_SECTION lpCritical;
    InitializeCriticalSection(&lpCritical);
    EnterCriticalSection(&lpCritical);

    //
    // Return the DDE instance id if it was requested
    //

    if (pdwDDEInst) {
        *pdwDDEInst = m_dwDDEInstance;
    }

    //
    // Copy the service name and create a DDE name handle for it
    //

    m_strServiceName = pszServiceName;
    m_hszServiceName.Create(this, m_strServiceName);

    //
    // Add all the system topic to the service tree
    //

    //
    // Create a system topic
    //
    ASSERT(m_pSystemTopic == NULL);  // only create once.
    m_pSystemTopic = CreateSystemTopic();
    ASSERT(m_pSystemTopic);  // only create once.

    m_pSystemTopic->Create(SZDDESYS_TOPIC);
    AddTopic(m_pSystemTopic);

    //
    // Create some system topic items
    //

    m_SystemItemTopics.Create(SZDDESYS_ITEM_TOPICS);
    m_pSystemTopic->AddItem(&m_SystemItemTopics);

    m_SystemItemSysItems.Create(SZDDESYS_ITEM_SYSITEMS);
    m_pSystemTopic->AddItem(&m_SystemItemSysItems);

    m_SystemItemItems.Create(SZDDE_ITEM_ITEMLIST);
    m_pSystemTopic->AddItem(&m_SystemItemItems);

    m_SystemItemFormats.Create(SZDDESYS_ITEM_FORMATS);
    m_pSystemTopic->AddItem(&m_SystemItemFormats);

    //
    // Register the name of our service
    //

    ::DdeNameService(m_dwDDEInstance, m_hszServiceName, NULL, DNS_REGISTER);

    m_bInitialized = TRUE;

    //
    // See if any derived class wants to add anything
    //

    return OnCreate();
}

//
// Callback function
// Note: this is a static
//

HDDEDATA CALLBACK CDDEServer::StdDDECallback(WORD wType, WORD wFmt, HCONV hConv, HSZ hsz1, HSZ hsz2, HDDEDATA hData,
                                             DWORD dwData1, DWORD dwData2)
{
    // CT BEGIN
    //  HDDEDATA hDdeData = NULL;
    //  UINT ui = 0;
    //  DWORD dwErr = 0;

    //
    // get a pointer to the server
    //
#ifdef _CALLHACK_
    // should not arive here
    ASSERT(0);
    return NULL;
#else
    CDDEServer *pServ = pTheServer;  // BARF BARF BARF
    ASSERT(pServ);
    //    pServ->Status("Callback %4.4XH", wType);

    // CT: mapped this one to the new callback, for easier testing.
    return DynDDECallback(pServ, wType, wFmt, hConv, hsz1, hsz2, hData, dwData1, dwData2);
#endif
}

HDDEDATA CALLBACK CDDEServer::DynDDECallback(CDDEServer *pServ, WORD wType, WORD wFmt, HCONV hConv, HSZ hsz1, HSZ hsz2,
                                             HDDEDATA hData, DWORD dwData1, DWORD dwData2)
{
    HDDEDATA hDdeData = NULL;
    UINT ui = 0;
    DWORD dwErr = 0;

    //
    // get a pointer to the server
    //

    //  CDDEServer* pServ = pTheServer; // BARF BARF BARF
    // CT END
    ASSERT(pServ);
    //    pServ->Status("Callback %4.4XH", wType);

    switch (wType) {
        case XTYP_CONNECT_CONFIRM:

            //
            // Add a new conversation to the list
            //

            pServ->Status(_T("Connect to %s"), (const TCHAR *)pServ->StringFromHsz(hsz1));
            pServ->AddConversation(hConv, hsz1);
            break;

        case XTYP_DISCONNECT:

            //
            // get some info on why it disconnected
            //

            CONVINFO ci;
            memset(&ci, 0, sizeof(ci));
            ci.cb = sizeof(ci);
            ui = ::DdeQueryConvInfo(hConv, wType, &ci);
            dwErr = pServ->GetLastError();

            //
            // Remove a conversation from the list
            //

            pServ->Status(_T("Disconnect"));
            pServ->RemoveConversation(hConv);
            break;

        case XTYP_WILDCONNECT:

            //
            // We only support wild connects to either a NULL service
            // name or to the name of our own service.
            //

            if ((hsz2 == NULL) || !::DdeCmpStringHandles(hsz2, pServ->m_hszServiceName)) {
                pServ->Status(_T("Wild connect to %s"), (const TCHAR *)pServ->StringFromHsz(hsz1));
                return pServ->DoWildConnect(hsz1);
            }
            break;

            //
            // For all other messages we see if we want them here
            // and if not, they get passed on to the user callback
            // if one is defined.
            //

        case XTYP_ADVSTART:
        case XTYP_CONNECT:
        case XTYP_EXECUTE:
        case XTYP_REQUEST:
        case XTYP_ADVREQ:
        case XTYP_ADVDATA:
        case XTYP_POKE:

            //
            // Try and process them here first.
            //

            if (pServ->DoCallback(wType, wFmt, hConv, hsz1, hsz2, hData, &hDdeData)) {
                return hDdeData;
            }

            //
            // Fall Through to allow the custom callback a chance
            //

        default:

            return pServ->CustomCallback(wType, wFmt, hConv, hsz1, hsz2, hData, dwData1, dwData2);
    }

    return (HDDEDATA)NULL;
}

CDDEConv *CDDEServer::AddConversation(HCONV hConv, HSZ hszTopic)
{
    //
    // create a new conversation object
    //

    CDDEConv *pConv = new CDDEConv(this, hConv, hszTopic);
    ASSERT(pConv);
    pConv->AddRef();

    //
    // Add it into the list
    //

    m_ConvList.AddTail(pConv);

    return pConv;
}

CDDEConv *CDDEServer::AddConversation(CDDEConv *pNewConv)
{
    ASSERT(pNewConv);
    pNewConv->AddRef();

    //
    // Add it into the list
    //

    m_ConvList.AddTail(pNewConv);

    return pNewConv;
}

BOOL CDDEServer::RemoveConversation(HCONV hConv)
{
    //
    // Try to find the conversation in the list
    //

    CDDEConv *pConv = NULL;
    POSITION pos = m_ConvList.GetHeadPosition();
    while (pos) {
        pConv = m_ConvList.GetNext(pos);
        if (pConv->m_hConv == hConv) {
            m_ConvList.RemoveAt(m_ConvList.Find(pConv));
            pConv->Release();
            return TRUE;
        }
    }

    //
    // Not in the list
    //

    return FALSE;
}

HDDEDATA CDDEServer::DoWildConnect(HSZ hszTopic)
{
    //
    // See how many topics we will be returning
    //

    int iTopics = 0;
    CString strTopic = "<null>";
    if (hszTopic == NULL) {
        //
        // Count all the topics we have
        //

        iTopics = m_TopicList.GetCount();
    }
    else {
        //
        // See if we have this topic in our list
        //

        strTopic = StringFromHsz(hszTopic);
        CDDETopic *pTopic = FindTopic(strTopic);
        if (pTopic) {
            iTopics++;
        }
    }

    //
    // If we have no match or no topics at all, just return
    // NULL now to refuse the connect
    //

    if (!iTopics) {
        Status(_T("Wild connect to %s refused"), (const TCHAR *)strTopic);
        return (HDDEDATA)NULL;
    }

    //
    // Allocate a chunk of DDE data big enough for all the HSZPAIRS
    // we'll be sending back plus space for a NULL entry on the end
    //

    HDDEDATA hData = ::DdeCreateDataHandle(m_dwDDEInstance, NULL, (iTopics + 1) * sizeof(HSZPAIR), 0, NULL, 0, 0);

    //
    // Check we actually got it.
    //

    if (!hData)
        return (HDDEDATA)NULL;

    HSZPAIR *pHszPair = (PHSZPAIR)DdeAccessData(hData, NULL);

    //
    // Copy the topic data
    //

    if (hszTopic == NULL) {
        //
        // Copy all the topics we have (includes the system topic)
        //

        POSITION pos = m_TopicList.GetHeadPosition();
        while (pos) {
            CDDETopic *pTopic = m_TopicList.GetNext(pos);
            pHszPair->hszSvc =
                ::DdeCreateStringHandle(m_dwDDEInstance, (TCHAR *)(const TCHAR *)m_strServiceName, DDE_STRING_CODEPAGE);
            pHszPair->hszTopic = ::DdeCreateStringHandle(m_dwDDEInstance, (TCHAR *)(const TCHAR *)pTopic->m_strName,
                                                         DDE_STRING_CODEPAGE);

            pHszPair++;
        }
    }
    else {
        //
        // Just copy the one topic asked for
        //

        pHszPair->hszSvc = m_hszServiceName;
        pHszPair->hszTopic = hszTopic;

        pHszPair++;
    }

    //
    // Put the terminator on the end
    //

    pHszPair->hszSvc = NULL;
    pHszPair->hszTopic = NULL;

    //
    // Finished with the data block
    //

    ::DdeUnaccessData(hData);

    //
    // Return the block handle
    //

    return hData;
}

CDDETopic *CDDEServer::FindTopic(const TCHAR *pszTopic)
{
    POSITION pos = m_TopicList.GetHeadPosition();
    while (pos) {
        CDDETopic *pTopic = m_TopicList.GetNext(pos);
        if (pTopic->m_strName.CompareNoCase(pszTopic) == 0)
            return pTopic;
    }
    return NULL;
}

BOOL CDDEServer::DoCallback(WORD wType, WORD wFmt, HCONV hConv, HSZ hszTopic, HSZ hszItem, HDDEDATA hData,
                            HDDEDATA *phReturnData)
{
    //
    // See if we know the topic
    //

    CString strTopic = StringFromHsz(hszTopic);

    //
    // See if this is an execute request
    //

    if (wType == XTYP_EXECUTE) {
        //
        // Call the exec function to process it
        //

        Status(_T("Exec"));
        DWORD dwLength = 0;
        void *pData = ::DdeAccessData(hData, &dwLength);
        BOOL b = Exec(strTopic, pData, dwLength);
        ::DdeUnaccessData(hData);

        if (b) {
            *phReturnData = (HDDEDATA)DDE_FACK;
            return TRUE;  // MH - Say we processed it
        }

        //
        // Either no handler or it didn't get handled by the function
        //

        Status(_T("Exec failed"));
        *phReturnData = (HDDEDATA)DDE_FNOTPROCESSED;
        return FALSE;
    }

    //
    // See if this is a connect request. Accept it if it is.
    //

    if (wType == XTYP_CONNECT) {
        if (!FindTopic(strTopic))
            return FALSE;  // unknown topic
        *phReturnData = (HDDEDATA)TRUE;
        return TRUE;
    }

    //
    // For any other transaction we need to be sure this is an
    // item we support and in some cases, that the format requested
    // is supported for that item.
    //

    CString strItem = StringFromHsz(hszItem);

    //
    // Now just do whatever is required for each specific transaction
    //

    BOOL b = FALSE;
    DWORD dwLength = 0;
    void *pData = NULL;

    switch (wType) {
        case XTYP_ADVSTART:

            //
            // Confirm that the supported topic/item pair is OK and
            // that the format is supported

            if (!CanAdvise(wFmt, strTopic, strItem)) {
                Status(_T("Can't advise on %s|%s"), (const TCHAR *)strTopic, (const TCHAR *)strItem);
                return FALSE;
            }

            //
            // Start an advise request.  Topic/item and format are ok.
            //

            *phReturnData = (HDDEDATA)TRUE;
            break;

        case XTYP_POKE:

            //
            // Some data for one of our items.
            //

            pData = ::DdeAccessData(hData, &dwLength);
            b = Poke(wFmt, strTopic, strItem, pData, dwLength);
            ::DdeUnaccessData(hData);

            if (!b) {
                //
                // Nobody took the data.
                // Maybe its not a supported item or format
                //

                Status(_T("Poke %s|%s failed"), (const TCHAR *)strTopic, (const TCHAR *)strItem);
                return FALSE;
            }

            //
            // Data at the server has changed.  See if we
            // did this ourself (from a poke) or if it's from
            // someone else.  If it came from elsewhere then post
            // an advise notice of the change.
            //

            CONVINFO ci;
            ci.cb = sizeof(CONVINFO);
            if (::DdeQueryConvInfo(hConv, (DWORD)QID_SYNC, &ci)) {
                if (!(ci.wStatus & ST_ISSELF)) {
                    //
                    // It didn't come from us
                    //

                    ::DdePostAdvise(m_dwDDEInstance, hszTopic, hszItem);
                }
            }

            *phReturnData = (HDDEDATA)DDE_FACK;  // say we took it
            break;

        case XTYP_ADVDATA:

            //
            // A server topic/item has changed value
            //

            pData = ::DdeAccessData(hData, &dwLength);
            b = AdviseData(wFmt, hConv, strTopic, strItem, pData, dwLength);
            ::DdeUnaccessData(hData);

            if (!b) {
                //
                // Nobody took the data.
                // Maybe its not of interrest
                //

                Status(_T("AdviseData %s|%s failed"), (const TCHAR *)strTopic, (const TCHAR *)strItem);
                *phReturnData = (HDDEDATA)DDE_FNOTPROCESSED;
            }
            else {
                *phReturnData = (HDDEDATA)DDE_FACK;  // say we took it
            }
            break;

        case XTYP_ADVREQ:
        case XTYP_REQUEST:

            //
            // Attempt to start an advise or get the data on a topic/item
            // See if we have a request function for this item or
            // a generic one for the topic
            //
            {  // scope for locals.

                CDDEAllocator allocr(m_dwDDEInstance, hszItem, wFmt, phReturnData);
                Status(_T("Request %s|%s"), (const TCHAR *)strTopic, (const TCHAR *)strItem);
                dwLength = 0;
                if (!Request(wFmt, strTopic, strItem, allocr)) {
                    //
                    // Nobody accepted the request
                    // Maybe unsupported topic/item or bad format
                    //

                    Status(_T("Request %s|%s failed"), (LPCTSTR)strTopic, (LPCTSTR)strItem);
                    *phReturnData = NULL;
                    return FALSE;
                }

            }  // end locals scope
            // Data already setup via 'allocr' param, so we are done.
            break;

        default:
            break;
    }

    //
    // Say we processed the transaction in some way
    //

    return TRUE;
}

BOOL CDDEServer::AddTopic(CDDETopic *pNewTopic)
{
    ASSERT(pNewTopic);

    //
    // See if we already have this topic
    //

    POSITION pos = m_TopicList.Find(pNewTopic);
    if (pos)
        return TRUE;  // already have it

    //
    // Add the new topic
    //

    m_TopicList.AddTail(pNewTopic);
    pNewTopic->m_pServer = this;

    pNewTopic->AddItem(&m_SystemItemItems);
    pNewTopic->AddItem(&m_SystemItemFormats);

    return TRUE;
}

CString CDDEServer::StringFromHsz(HSZ hsz)
{
    CString str = "<null>";

    //
    // Get the length of the string
    //

    DWORD dwLen = ::DdeQueryString(m_dwDDEInstance, hsz, NULL, 0, DDE_STRING_CODEPAGE);

    if (dwLen == 0)
        return str;

    //
    // get the text
    //

    TCHAR *pBuf = str.GetBufferSetLength(dwLen + 1);
    ASSERT(pBuf);

    //
    // Get the string text
    //

    DWORD dw = ::DdeQueryString(m_dwDDEInstance, hsz, pBuf, dwLen + 1, DDE_STRING_CODEPAGE);

    //
    // Tidy up
    //

    str.ReleaseBuffer();

    if (dw == 0)
        str = "<error>";

    return str;
}

BOOL CDDEServer::Request(UINT wFmt, const TCHAR *pszTopic, const TCHAR *pszItem, CDDEAllocator &allocr)
{
    //
    // See if we have a topic that matches
    //

    CDDETopic *pTopic = FindTopic(pszTopic);
    if (!pTopic)
        return FALSE;

    return pTopic->Request(wFmt, pszItem, allocr);
}

BOOL CDDEServer::Poke(UINT wFmt, const TCHAR *pszTopic, const TCHAR *pszItem, void *pData, DWORD dwSize)
{
    //
    // See if we have a topic that matches
    //

    CDDETopic *pTopic = FindTopic(pszTopic);
    if (!pTopic)
        return FALSE;

    return pTopic->Poke(wFmt, pszItem, pData, dwSize);
}

BOOL CDDEServer::Exec(const TCHAR *pszTopic, void *pData, DWORD dwSize)
{
    //
    // See if we have a topic that matches
    //

    CDDETopic *pTopic = FindTopic(pszTopic);
    if (!pTopic)
        return FALSE;

    return pTopic->Exec(pData, dwSize);
}

BOOL CDDEServer::CanAdvise(UINT wFmt, const TCHAR *pszTopic, const TCHAR *pszItem)
{
    //
    // See if we have a topic that matches
    //

    CDDETopic *pTopic = FindTopic(pszTopic);
    if (!pTopic)
        return FALSE;

    return pTopic->CanAdvise(wFmt, pszItem);
}

void CDDEServer::PostAdvise(CDDETopic *pTopic, CDDEItem *pItem)
{
    ASSERT(pTopic);
    ASSERT(pItem);

    ::DdePostAdvise(
        m_dwDDEInstance,
        ::DdeCreateStringHandle(m_dwDDEInstance, (TCHAR *)(const TCHAR *)pTopic->m_strName, DDE_STRING_CODEPAGE),
        ::DdeCreateStringHandle(m_dwDDEInstance, (TCHAR *)(const TCHAR *)pItem->m_strName, DDE_STRING_CODEPAGE));
}

CString GetFormatName(WORD wFmt)
{
    CString strName = "";
    PCFTAGNAME pCTN;

    //
    // Try for a standard one first
    //

    pCTN = CFNames;
    while (pCTN->wFmt) {
        if (pCTN->wFmt == wFmt) {
            strName = pCTN->pszName;
            return strName;
        }
        pCTN++;
    }

    //
    // See if it's a registered one
    //

    TCHAR buf[256];
    if (::GetClipboardFormatName(wFmt, buf, sizeof(buf) / sizeof(buf[0]))) {
        strName = buf;
    }

    return strName;
}

CDDEConv *CDDEServer::FindConversation(HCONV hConv)
{
    POSITION pos = m_ConvList.GetHeadPosition();
    while (pos) {
        CDDEConv *pConv = m_ConvList.GetNext(pos);
        ASSERT(pConv);
        if (pConv->m_hConv == hConv)
            return pConv;
    }
    return NULL;
}

BOOL CDDEServer::AdviseData(UINT wFmt, HCONV hConv, const TCHAR *pszTopic, const TCHAR *pszItem, void *pData,
                            DWORD dwSize)
{
    //
    // See if we know this conversation
    //

    CDDEConv *pConv = FindConversation(hConv);
    if (!pConv)
        return FALSE;

    return pConv->AdviseData(wFmt, pszTopic, pszItem, pData, dwSize);
}
