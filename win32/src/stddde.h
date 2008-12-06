// stddde.h

#ifndef _STDDDE_
#define _STDDDE_

#include <ddeml.h>

//
// String names for standard Windows Clipboard formats
//

#define SZCF_TEXT           "TEXT"        
#define SZCF_BITMAP         "BITMAP"      
#define SZCF_METAFILEPICT   "METAFILEPICT"
#define SZCF_SYLK           "SYLK"        
#define SZCF_DIF            "DIF"         
#define SZCF_TIFF           "TIFF"        
#define SZCF_OEMTEXT        "OEMTEXT"     
#define SZCF_DIB            "DIB"         
#define SZCF_PALETTE        "PALETTE"     
#define SZCF_PENDATA        "PENDATA"     
#define SZCF_RIFF           "RIFF"     
#define SZCF_WAVE           "WAVE"     
#define SZCF_UNICODETEXT    "UNICODETEXT" 
#define SZCF_ENHMETAFILE    "ENHMETAFILE" 

//
// String names for some standard DDE strings not
// defined in DDEML.H
//

#define SZ_READY            "Ready"
#define SZ_BUSY             "Busy"
#define SZ_TAB              "\t"
#define SZ_RESULT           "Result"
#define SZ_PROTOCOLS        "Protocols"
#define SZ_EXECUTECONTROL1  "Execute Control 1"

//
// Helpers
//

static CString GetFormatName(WORD wFmt);

// A class used for memory allocation; designed to be created on the stack,
// then passed around.
class CDDEAllocator
{
public:
    CDDEAllocator(DWORD instance, HSZ hszItem, UINT wFmt, HDDEDATA *hret) :
            m_instance(instance), m_hszItem(hszItem), m_wFmt(wFmt), m_hret(hret) {;}
    BOOL Alloc(CString &cs) {
        // XXX - should we check wFmt is CF_TEXT vs CF_UNICODETEXT??
        return Alloc((LPBYTE)(const TCHAR *)cs, (cs.GetLength() + 1) * sizeof(TCHAR));
    }
    BOOL Alloc(LPBYTE p, DWORD cb) {
        // XXX - should we check wFmt is CF_TEXT vs CF_UNICODETEXT??
        *m_hret = ::DdeCreateDataHandle(m_instance,
                                        p,
                                        cb,
                                        0,
                                        m_hszItem,
                                        m_wFmt,
                                        0);
        return TRUE;
    }
protected:
    UINT m_wFmt;
    DWORD m_instance;
    HSZ m_hszItem;
    HDDEDATA *m_hret;

};

//
// Generic counted object class
//

class CDDECountedObject : public CObject
{
    DECLARE_DYNCREATE(CDDECountedObject);
public:
    CDDECountedObject();
    virtual ~CDDECountedObject();
    int AddRef();
    int Release();

private:
    int m_iRefCount;
};
    
//
// String handle class
//

class CDDEServer;

class CHSZ : public CObject
{
    DECLARE_DYNCREATE(CHSZ);
public:
    CHSZ();
    CHSZ(CDDEServer* pServer, const TCHAR* szName);
    virtual ~CHSZ();
    void Create(CDDEServer* pServer, const TCHAR* szName);
	void Destroy();
    operator HSZ() {return m_hsz;}

    HSZ m_hsz;

protected:
    DWORD m_dwDDEInstance;
};

//
// DDE item class
//

class CDDETopic;

class CDDEItem : public CObject
{
    DECLARE_DYNCREATE(CDDEItem);
public:
    CDDEItem();
    virtual ~CDDEItem();
    void Create(const TCHAR* pszName);
    void PostAdvise();
    virtual BOOL Request(UINT wFmt, CDDEAllocator &allocr);
//CT BEGIN
    virtual BOOL NSRequest(const TCHAR* szItem, CDDEAllocator &allocr);
    virtual BOOL NSPoke(const TCHAR* szItem, void* pData, DWORD dwSize);
    virtual BOOL Poke(void* pData, DWORD dwSize);
//CT END
    virtual BOOL Poke(UINT wFmt, void* pData, DWORD dwSize);
    virtual BOOL IsSupportedFormat(WORD wFormat);
    virtual WORD* GetFormatList()
        {return NULL;}
    virtual BOOL CanAdvise(UINT wFmt);

    CString m_strName;          // name of this item
    CDDETopic* m_pTopic;        // pointer to the topic it belongs to

protected:
};

//
// String item class
//

class CDDEStringItem : public CDDEItem
{
    DECLARE_DYNCREATE(CDDEStringItem);
public:
    virtual void OnPoke(){;}
    virtual void SetData(const TCHAR* pszData);
    virtual const TCHAR* GetData()
        {return (const TCHAR*)m_strData;}
    operator const TCHAR*()
        {return (const TCHAR*)m_strData;}

protected:
    virtual BOOL Request(UINT wFmt, CDDEAllocator &allocr);
    virtual BOOL Poke(UINT wFmt, void* pData, DWORD dwSize);
    virtual WORD* GetFormatList();

    CString m_strData;
};

//
// Item list class
//

class CDDEItemList : public CObList
{
    DECLARE_DYNCREATE(CDDEItemList);
public:
    CDDEItemList();
    virtual ~CDDEItemList();
    CDDEItem* GetNext(POSITION& rPosition) const
        {return (CDDEItem*)CObList::GetNext(rPosition);}

};

//
// Topic class
//

class CDDEServer;

class CDDETopic : public CObject
{
    DECLARE_DYNCREATE(CDDETopic);
public:
    CDDETopic();
    virtual ~CDDETopic();
    void Create(const TCHAR* pszName);
    BOOL AddItem(CDDEItem* pItem);
    virtual BOOL Request(UINT wFmt, const TCHAR* pszItem, CDDEAllocator &allocr);
//CT BEGIN
    virtual BOOL NSRequest(const TCHAR *szItem, CDDEAllocator &allocr);
    // Note: If poke ever needs to return data, it should do like NSRequest,
    // otherwise memory management becomes impossible.
    virtual BOOL NSPoke(const TCHAR * szItem, void* pData, DWORD dwSize);
    virtual BOOL Poke(const TCHAR* pszItem,
                      void* pData, DWORD dwSize);
//CT END
    virtual BOOL Poke(UINT wFmt, const TCHAR* pszItem,
                      void* pData, DWORD dwSize);
    virtual BOOL Exec(void* pData, DWORD dwSize);
    virtual CDDEItem* FindItem(const TCHAR* pszItem);
    virtual BOOL CanAdvise(UINT wFmt, const TCHAR* pszItem);
    void PostAdvise(CDDEItem* pItem);

    CString m_strName;          // name of this topic
    CDDEServer* m_pServer;      // ptr to the server which owns this topic
    CDDEItemList m_ItemList;    // List of items for this topic

protected:
};

//
// Topic list class
//

class CDDETopicList : public CObList
{
    DECLARE_DYNCREATE(CDDETopicList);
public:
    CDDETopicList();
    virtual ~CDDETopicList();
    CDDETopic* GetNext(POSITION& rPosition) const
        {return (CDDETopic*)CObList::GetNext(rPosition);}

protected:

};

//
// Conversation class
//

class CDDEConv : public CDDECountedObject
{
    DECLARE_DYNCREATE(CDDEConv);
public:
    CDDEConv();
    CDDEConv(CDDEServer* pServer);
    CDDEConv(CDDEServer* pServer, HCONV hConv, HSZ hszTopic);
    BOOL Connected() {return (m_hConv != NULL);}
    virtual ~CDDEConv();
    virtual BOOL ConnectTo(const TCHAR* pszService, const TCHAR* pszTopic);
    virtual BOOL Terminate();
    virtual BOOL AdviseData(UINT wFmt, const TCHAR* pszTopic, const TCHAR* pszItem,
                            void* pData, DWORD dwSize);
    virtual BOOL Request(const TCHAR* pszItem, CString &ret);
    virtual BOOL Advise(const TCHAR* pszItem);
    virtual BOOL Exec(const TCHAR* pszCmd);
    virtual BOOL Poke(UINT wFmt, const TCHAR* pszItem, void* pData, DWORD dwSize);
//CT BEGIN
    virtual BOOL Poke(const TCHAR* pszItem, void* pData, DWORD dwSize);
//CT END
    CDDEServer* m_pServer;
    HCONV   m_hConv;            // Conversation handle
    HSZ     m_hszTopic;         // Topic name

};

//
// Conversation list class
//

class CDDEConvList : public CObList
{
    DECLARE_DYNCREATE(CDDEConvList);
public:
    CDDEConvList();
    virtual ~CDDEConvList();
    CDDEConv* GetNext(POSITION& rPosition) const
        {return (CDDEConv*)CObList::GetNext(rPosition);}

    
protected:

};

//
// Topics and items used to support the 'system' topic in the server
//

class CDDESystemItem : public CDDEItem
{
    DECLARE_DYNCREATE(CDDESystemItem);
protected:
    virtual WORD* GetFormatList();
};

class CDDESystemItem_TopicList : public CDDESystemItem
{
    DECLARE_DYNCREATE(CDDESystemItem_TopicList);
protected:
    virtual BOOL Request(UINT wFmt, CDDEAllocator &allocr);
};

class CDDESystemItem_ItemList : public CDDESystemItem
{
    DECLARE_DYNCREATE(CDDESystemItem_ItemList);
protected:
    virtual BOOL Request(UINT wFmt, CDDEAllocator &allocr);
};

class CDDESystemItem_FormatList : public CDDESystemItem
{
    DECLARE_DYNCREATE(CDDESystemItem_FormatList);
protected:
    virtual BOOL Request(UINT wFmt, CDDEAllocator &allocr);
};

class CDDEServerSystemTopic : public CDDETopic
{
    DECLARE_DYNCREATE(CDDEServerSystemTopic);
protected:
    virtual BOOL Request(UINT wFmt, const TCHAR* pszItem,
                         CDDEAllocator &allocr);

};


//
// Server class
// Note: this class is for a server which supports only one service
//


class CDDEServer : public CObject
{
    DECLARE_DYNCREATE(CDDEServer);
public:
    CDDEServer();
    virtual ~CDDEServer();
    BOOL Create(const TCHAR* pszServiceName,
                DWORD dwFilterFlags = 0,
                DWORD* pdwDDEInst = NULL);
	virtual CDDEServerSystemTopic *CreateSystemTopic() {return new CDDEServerSystemTopic();}
    void Shutdown();
    virtual BOOL OnCreate() {return TRUE;}
    virtual UINT GetLastError()
        {return ::DdeGetLastError(m_dwDDEInstance);}
    virtual HDDEDATA CustomCallback(WORD wType,
                                    WORD wFmt,
                                    HCONV hConv,
                                    HSZ hsz1,
                                    HSZ hsz2,
                                    HDDEDATA hData,
                                    DWORD dwData1,
                                    DWORD dwData2)
        {return NULL;}

    virtual BOOL Request(UINT wFmt, const TCHAR* pszTopic, const TCHAR* pszItem,
                         CDDEAllocator &allocr);
    virtual BOOL Poke(UINT wFmt, const TCHAR* pszTopic, const TCHAR* pszItem,
                      void* pData, DWORD dwSize);
    virtual BOOL AdviseData(UINT wFmt, HCONV hConv, const TCHAR* pszTopic, const TCHAR* pszItem,
                      void* pData, DWORD dwSize);
    virtual BOOL Exec(const TCHAR* pszTopic, void* pData, DWORD dwSize);
    virtual void Status(const TCHAR* pszFormat, ...) {;}
    virtual BOOL AddTopic(CDDETopic* pTopic);
    CString StringFromHsz(HSZ hsz);
    virtual BOOL CanAdvise(UINT wFmt, const TCHAR* pszTopic, const TCHAR* pszItem);
    void PostAdvise(CDDETopic* pTopic, CDDEItem* pItem);
    CDDEConv*  AddConversation(HCONV hConv, HSZ hszTopic);
    CDDEConv* AddConversation(CDDEConv* pNewConv);
    BOOL RemoveConversation(HCONV hConv);
    CDDEConv*  FindConversation(HCONV hConv);

    DWORD       m_dwDDEInstance;        // DDE Instance handle
    CDDETopicList m_TopicList;          // topic list

protected:
    BOOL        m_bInitialized;         // TRUE after DDE init complete
    CString     m_strServiceName;       // Service name
    CHSZ        m_hszServiceName;       // String handle for service name
    CDDEConvList m_ConvList;            // Conversation list

    HDDEDATA DoWildConnect(HSZ hszTopic);
    BOOL DoCallback(WORD wType,
                WORD wFmt,
                HCONV hConv,
                HSZ hsz1,
                HSZ hsz2,
                HDDEDATA hData,
                HDDEDATA *phReturnData);
    CDDETopic* FindTopic(const TCHAR* pszTopic);

private:
//CT BEGIN
	// this is the old version which
	// is kept intact.
    static HDDEDATA CALLBACK StdDDECallback(WORD wType,
                                            WORD wFmt,
                                            HCONV hConv,
                                            HSZ hsz1,
                                            HSZ hsz2,
                                            HDDEDATA hData,
                                            DWORD dwData1,
                                            DWORD dwData2);

	/* 
		this is the prototype for the new callback
		with an extra parameter to hold the server.
		This parameter is stuffed in by a hairy trick
		which is implemented in stddde.cpp.
		Without this trick, the old routine maps to it.
	*/
    static HDDEDATA CALLBACK DynDDECallback(
											CDDEServer *pServ,
											WORD wType,
                                            WORD wFmt,
                                            HCONV hConv,
                                            HSZ hsz1,
                                            HSZ hsz2,
                                            HDDEDATA hData,
                                            DWORD dwData1,
                                            DWORD dwData2);

	// this buffer takes the compiled dynamic
	// callback stub.
	byte DynamicCallback[20] ;
	// this builds a ring of servers
	CDDEServer *pServers ;
//CT END 

    CDDEServerSystemTopic* m_pSystemTopic;
    CDDESystemItem_TopicList m_SystemItemTopics;
    CDDESystemItem_ItemList m_SystemItemSysItems;
    CDDESystemItem_ItemList m_SystemItemItems;
    CDDESystemItem_FormatList m_SystemItemFormats;
};          

#ifdef UNICODE
#define DDE_STRING_CODEPAGE CP_WINUNICODE
#else
#define DDE_STRING_CODEPAGE CP_WINANSI
#endif

#endif // _STDDDE_
