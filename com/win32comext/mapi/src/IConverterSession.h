#pragma once

// Class Identifiers
// {4e3a7680-b77a-11d0-9da5-00c04fd65685}
DEFINE_GUID(CLSID_IConverterSession, 0x4e3a7680, 0xb77a, 0x11d0, 0x9d, 0xa5, 0x0, 0xc0, 0x4f, 0xd6, 0x56, 0x85);

// Interface Identifiers
// {4b401570-b77b-11d0-9da5-00c04fd65685}
DEFINE_GUID(IID_IConverterSession, 0x4b401570, 0xb77b, 0x11d0, 0x9d, 0xa5, 0x0, 0xc0, 0x4f, 0xd6, 0x56, 0x85);

// Constants
#define CCSF_SMTP 0x0002              // the converter is being passed an SMTP message
#define CCSF_NOHEADERS 0x0004         // the converter should ignore the headers on the outside message
#define CCSF_USE_TNEF 0x0010          // the converter should embed TNEF in the MIME message
#define CCSF_INCLUDE_BCC 0x0020       // the converter should include Bcc recipients
#define CCSF_8BITHEADERS 0x0040       // the converter should allow 8 bit headers
#define CCSF_USE_RTF 0x0080           // the converter should do HTML->RTF conversion
#define CCSF_PLAIN_TEXT_ONLY 0x1000   // the converter should just send plain text
#define CCSF_NO_MSGID 0x4000          // don't include Message-Id field in outgoing messages
#define CCSF_EMBEDDED_MESSAGE 0x8000  // sent/unsent information is persisted in X-Unsent
#define CCSF_PRESERVE_SOURCE 0x40000  // don't modify the source message

// http://msdn2.microsoft.com/en-us/library/bb905202.aspx
interface IConverterSession : public IUnknown
{
   public:
    virtual HRESULT STDMETHODCALLTYPE SetAdrBook(LPADRBOOK pab);

    virtual HRESULT PlaceHolder1();
    virtual HRESULT PlaceHolder2();

    virtual HRESULT STDMETHODCALLTYPE MIMEToMAPI(LPSTREAM pstm, LPMESSAGE pmsg, LPCSTR pszSrcSrv, ULONG ulFlags);

    virtual HRESULT STDMETHODCALLTYPE MAPIToMIMEStm(LPMESSAGE pmsg, LPSTREAM pstm, ULONG ulFlags);

    virtual HRESULT PlaceHolder3();
    virtual HRESULT PlaceHolder4();
    virtual HRESULT PlaceHolder5();
    virtual HRESULT PlaceHolder6();
    virtual HRESULT PlaceHolder7();
    virtual HRESULT PlaceHolder8();
    virtual HRESULT PlaceHolder9();
};
