// Clones of the IExplorer* interfaces from the Vista ASK ShObjIdl.h
// This is necessary as VS6 can't use the Vista SDK, and many Python
// versions still use that compiler.

// Only need this if we don't have a Vista SDK available...
#if (WINVER < 0x600)

typedef ITEMIDLIST                  ITEMIDLIST_RELATIVE;
typedef ITEMIDLIST                  ITEMID_CHILD;
typedef ITEMIDLIST                  ITEMIDLIST_ABSOLUTE;
typedef ITEMIDLIST_ABSOLUTE         *PIDLIST_ABSOLUTE;
typedef const ITEMIDLIST_ABSOLUTE   *PCIDLIST_ABSOLUTE;
typedef const ITEMIDLIST_ABSOLUTE   *PCUIDLIST_ABSOLUTE;
typedef ITEMIDLIST_RELATIVE         *PIDLIST_RELATIVE;
typedef ITEMIDLIST_RELATIVE         *PUIDLIST_RELATIVE;
typedef const ITEMIDLIST_RELATIVE   *PCIDLIST_RELATIVE;
typedef const ITEMIDLIST_RELATIVE   *PCUIDLIST_RELATIVE;
typedef ITEMID_CHILD                *PITEMID_CHILD;
typedef const ITEMID_CHILD          *PCITEMID_CHILD;
typedef ITEMID_CHILD                *PUITEMID_CHILD;
typedef const ITEMID_CHILD          *PCUITEMID_CHILD;
typedef PCUITEMID_CHILD const       *PCUITEMID_CHILD_ARRAY;
typedef PCUIDLIST_RELATIVE const    *PCUIDLIST_RELATIVE_ARRAY;
typedef PCIDLIST_ABSOLUTE const     *PCIDLIST_ABSOLUTE_ARRAY;
typedef PCUIDLIST_ABSOLUTE const    *PCUIDLIST_ABSOLUTE_ARRAY;

typedef DWORD EXPLORER_BROWSER_OPTIONS;

typedef DWORD EXPLORER_BROWSER_FILL_FLAGS;

typedef DWORD GETPROPERTYSTOREFLAGS;

typedef SHCOLUMNID PROPERTYKEY;
typedef PROPERTYKEY *REFPROPERTYKEY;

#ifndef __IShellItemArray_FWD_DEFINED__
#define __IShellItemArray_FWD_DEFINED__
typedef interface IShellItemArray IShellItemArray;
#endif 	/* __IShellItemArray_FWD_DEFINED__ */

#ifndef __IEnumShellItems_FWD_DEFINED__
#define __IEnumShellItems_FWD_DEFINED__
typedef interface IEnumShellItems IEnumShellItems;
#endif 	/* __IEnumShellItems_FWD_DEFINED__ */

#ifndef __IExplorerBrowserEvents_FWD_DEFINED__
#define __IExplorerBrowserEvents_FWD_DEFINED__
typedef interface IExplorerBrowserEvents IExplorerBrowserEvents;
#endif 	/* __IExplorerBrowserEvents_FWD_DEFINED__ */


#ifndef __IExplorerBrowser_FWD_DEFINED__
#define __IExplorerBrowser_FWD_DEFINED__
typedef interface IExplorerBrowser IExplorerBrowser;
#endif 	/* __IExplorerBrowser_FWD_DEFINED__ */

#ifndef __IEnumExplorerCommand_FWD_DEFINED__
#define __IEnumExplorerCommand_FWD_DEFINED__
typedef interface IEnumExplorerCommand IEnumExplorerCommand;
#endif 	/* __IEnumExplorerCommand_FWD_DEFINED__ */


#ifndef __IExplorerCommandProvider_FWD_DEFINED__
#define __IExplorerCommandProvider_FWD_DEFINED__
typedef interface IExplorerCommandProvider IExplorerCommandProvider;
#endif 	/* __IExplorerCommandProvider_FWD_DEFINED__ */

#ifndef __IExplorerBrowserEvents_INTERFACE_DEFINED__
#define __IExplorerBrowserEvents_INTERFACE_DEFINED__

/* interface IExplorerBrowserEvents */
/* [local][object][uuid] */ 


EXTERN_C const IID IID_IExplorerBrowserEvents;


MIDL_INTERFACE("361bbdc7-e6ee-4e13-be58-58e2240c810f")
IExplorerBrowserEvents : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE OnNavigationPending( 
        /* [in] */ PCIDLIST_ABSOLUTE pidlFolder) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE OnViewCreated( 
        /* [in] */ IShellView *psv) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE OnNavigationComplete( 
        /* [in] */ PCIDLIST_ABSOLUTE pidlFolder) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE OnNavigationFailed( 
        /* [in] */ PCIDLIST_ABSOLUTE pidlFolder) = 0;
    
};
#endif // __IExplorerBrowserEvents_INTERFACE_DEFINED__

#ifndef __IExplorerBrowser_INTERFACE_DEFINED__
#define __IExplorerBrowser_INTERFACE_DEFINED__

EXTERN_C const IID IID_IExplorerBrowser;

MIDL_INTERFACE("dfd3b6b5-c10c-4be9-85f6-a66969f402f6")
IExplorerBrowser : public IUnknown
{
public:
    virtual /* [local] */ HRESULT STDMETHODCALLTYPE Initialize( 
        /* [in] */ HWND hwndParent,
        /* [in] */ RECT *prc,
        /* [unique][in] */ FOLDERSETTINGS *pfs) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE Destroy( void) = 0;
    
    virtual /* [local] */ HRESULT STDMETHODCALLTYPE SetRect( 
        /* [unique][out][in] */ HDWP *phdwp,
        /* [in] */ RECT rcBrowser) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE SetPropertyBag( 
        /* [string][in] */ LPCWSTR pszPropertyBag) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE SetEmptyText( 
        /* [string][in] */ LPCWSTR pszEmptyText) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE SetFolderSettings( 
        /* [in] */ FOLDERSETTINGS *pfs) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE Advise( 
        /* [in] */ IExplorerBrowserEvents *psbe,
        /* [out] */ DWORD *pdwCookie) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE Unadvise( 
        /* [in] */ DWORD dwCookie) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE SetOptions( 
        /* [in] */ EXPLORER_BROWSER_OPTIONS dwFlag) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE GetOptions( 
        /* [out] */ EXPLORER_BROWSER_OPTIONS *pdwFlag) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE BrowseToIDList( 
        /* [in] */ PCUIDLIST_RELATIVE pidl,
        /* [in] */ UINT uFlags) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE BrowseToObject( 
        /* [in] */ IUnknown *punk,
        /* [in] */ UINT uFlags) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE FillFromObject( 
        /* [unique][in] */ IUnknown *punk,
        /* [in] */ EXPLORER_BROWSER_FILL_FLAGS dwFlags) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE RemoveAll( void) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE GetCurrentView( 
        /* [in] */ REFIID riid,
        /* [iid_is][out] */ void **ppv) = 0;
    
};
#endif // __IExplorerBrowser_INTERFACE_DEFINED__

#ifndef __IExplorerCommand_INTERFACE_DEFINED__
#define __IExplorerCommand_INTERFACE_DEFINED__

typedef DWORD EXPCMDSTATE;
typedef DWORD EXPCMDFLAGS;

EXTERN_C const IID IID_IExplorerCommand;

MIDL_INTERFACE("a08ce4d0-fa25-44ab-b57c-c7b1c323e0b9")
IExplorerCommand : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE GetTitle( 
        /* [unique][in] */ IShellItemArray *psiItemArray,
        /* [string][out] */ LPWSTR *ppszName) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE GetIcon( 
        /* [unique][in] */ IShellItemArray *psiItemArray,
        /* [string][out] */ LPWSTR *ppszIcon) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE GetToolTip( 
        /* [unique][in] */ IShellItemArray *psiItemArray,
        /* [string][out] */ LPWSTR *ppszInfotip) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE GetCanonicalName( 
        /* [out] */ GUID *pguidCommandName) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE GetState( 
        /* [in] */ IShellItemArray *psiItemArray,
        /* [in] */ BOOL fOkToBeSlow,
        /* [out] */ EXPCMDSTATE *pCmdState) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE Invoke( 
        /* [in] */ IShellItemArray *psiItemArray,
        /* [unique][in] */ IBindCtx *pbc) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE GetFlags( 
        /* [out] */ EXPCMDFLAGS *pFlags) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE EnumSubCommands( 
        /* [out] */ IEnumExplorerCommand **ppEnum) = 0;
    
};
#endif __IExplorerCommand_INTERFACE_DEFINED__

#ifndef __IEnumExplorerCommand_INTERFACE_DEFINED__
#define __IEnumExplorerCommand_INTERFACE_DEFINED__

EXTERN_C const IID IID_IEnumExplorerCommand;

MIDL_INTERFACE("a88826f8-186f-4987-aade-ea0cef8fbfe8")
IEnumExplorerCommand : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE Next( 
        /* [in] */ ULONG celt,
        /* [length_is][size_is][out] */ IExplorerCommand **pUICommand,
        /* [out] */ ULONG *pceltFetched) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE Skip( 
        /* [in] */ ULONG celt) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE Clone( 
        /* [out] */ IEnumExplorerCommand **ppenum) = 0;
    
};
#endif // __IEnumExplorerCommand_INTERFACE_DEFINED__

#ifndef __IShellItemArray_INTERFACE_DEFINED__
#define __IShellItemArray_INTERFACE_DEFINED__

typedef DWORD SIATTRIBFLAGS;


EXTERN_C const IID IID_IShellItemArray;

MIDL_INTERFACE("b63ea76d-1f85-456f-a19c-48159efa858b")
IShellItemArray : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE BindToHandler( 
        /* [unique][in] */ IBindCtx *pbc,
        /* [in] */ REFGUID rbhid,
        /* [in] */ REFIID riid,
        /* [iid_is][out] */ void **ppvOut) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE GetPropertyStore( 
        /* [in] */ GETPROPERTYSTOREFLAGS flags,
        /* [in] */ REFIID riid,
        /* [iid_is][out] */ void **ppv) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE GetPropertyDescriptionList( 
        /* [in] */ REFPROPERTYKEY keyType,
        /* [in] */ REFIID riid,
        /* [iid_is][out] */ void **ppv) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE GetAttributes( 
        /* [in] */ SIATTRIBFLAGS dwAttribFlags,
        /* [in] */ SFGAOF sfgaoMask,
        /* [out] */ SFGAOF *psfgaoAttribs) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE GetCount( 
        /* [out] */ DWORD *pdwNumItems) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE GetItemAt( 
        /* [in] */ DWORD dwIndex,
        /* [out] */ IShellItem **ppsi) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE EnumItems( 
        /* [out] */ IEnumShellItems **ppenumShellItems) = 0;
};
#endif // __IShellItemArray_INTERFACE_DEFINED__

#ifndef __IEnumShellItems_INTERFACE_DEFINED__
#define __IEnumShellItems_INTERFACE_DEFINED__

EXTERN_C const IID IID_IEnumShellItems;
    
MIDL_INTERFACE("70629033-e363-4a28-a567-0db78006e6d7")
IEnumShellItems : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE Next( 
        /* [in] */ ULONG celt,
        /* [length_is][size_is][out] */ IShellItem **rgelt,
        /* [out] */ ULONG *pceltFetched) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE Skip( 
        /* [in] */ ULONG celt) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE Clone( 
        /* [out] */ IEnumShellItems **ppenum) = 0;
};
#endif // __IEnumShellItems_INTERFACE_DEFINED__

#endif // WINVER
