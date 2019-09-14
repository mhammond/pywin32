// Python-ActiveX Scripting header file.

// Copyright 1996-1997 Mark Hammond (MHammond@skippinet.com.au)

#ifdef BUILD_FREEZE
#define PYAXSCRIPT_EXPORT
#else
#ifdef PY_BUILD_AXSCRIPT
#define PYAXSCRIPT_EXPORT __declspec(dllexport)
#else
#define PYAXSCRIPT_EXPORT __declspec(dllimport)
#endif
#endif

// Disable an OK warning...
#pragma warning(disable : 4275)
// warning C4275: non dll-interface struct '???' used as base for dll-interface class 'PyGatewayBase'

// Client side

class PYAXSCRIPT_EXPORT PyIActiveScript : public PyIUnknown {
   public:
    MAKE_PYCOM_CTOR_ERRORINFO(PyIActiveScript, IID_IActiveScript);
    static IActiveScript *GetI(PyObject *self);
    static PyComTypeObject type;

    // The Python methods
    static PyObject *SetScriptSite(PyObject *self, PyObject *args);
    static PyObject *GetScriptSite(PyObject *self, PyObject *args);
    static PyObject *SetScriptState(PyObject *self, PyObject *args);
    static PyObject *GetScriptState(PyObject *self, PyObject *args);
    static PyObject *Close(PyObject *self, PyObject *args);
    static PyObject *AddNamedItem(PyObject *self, PyObject *args);
    static PyObject *AddTypeLib(PyObject *self, PyObject *args);
    static PyObject *GetScriptDispatch(PyObject *self, PyObject *args);
    static PyObject *GetCurrentScriptThreadID(PyObject *self, PyObject *args);
    static PyObject *GetScriptThreadID(PyObject *self, PyObject *args);
    static PyObject *GetScriptThreadState(PyObject *self, PyObject *args);
    static PyObject *InterruptScriptThread(PyObject *self, PyObject *args);
    static PyObject *Clone(PyObject *self, PyObject *args);

   protected:
    PyIActiveScript(IUnknown *pdisp);
    ~PyIActiveScript();
};

class PYAXSCRIPT_EXPORT PyIActiveScriptParse : public PyIUnknown {
   public:
    MAKE_PYCOM_CTOR_ERRORINFO(PyIActiveScriptParse, IID_IActiveScriptParse);
    static IActiveScriptParse *GetI(PyObject *self);
    static PyComTypeObject type;

    // The Python methods
    static PyObject *InitNew(PyObject *self, PyObject *args);
    static PyObject *AddScriptlet(PyObject *self, PyObject *args);
    static PyObject *ParseScriptText(PyObject *self, PyObject *args);

   protected:
    PyIActiveScriptParse(IUnknown *pdisp);
    ~PyIActiveScriptParse();
};

class PYAXSCRIPT_EXPORT PyIActiveScriptSite : public PyIUnknown {
   public:
    MAKE_PYCOM_CTOR_ERRORINFO(PyIActiveScriptSite, IID_IActiveScriptSite);
    static IActiveScriptSite *GetI(PyObject *self);
    static PyComTypeObject type;

    // The Python methods
    static PyObject *GetLCID(PyObject *self, PyObject *args);
    static PyObject *GetItemInfo(PyObject *self, PyObject *args);
    static PyObject *GetDocVersionString(PyObject *self, PyObject *args);
    static PyObject *OnStateChange(PyObject *self, PyObject *args);
    static PyObject *OnEnterScript(PyObject *self, PyObject *args);
    static PyObject *OnLeaveScript(PyObject *self, PyObject *args);
    static PyObject *OnScriptError(PyObject *self, PyObject *args);
    static PyObject *OnScriptTerminate(PyObject *self, PyObject *args);

   protected:
    PyIActiveScriptSite(IUnknown *pdisp);
    ~PyIActiveScriptSite();
};

///////////////////////////////////////////////
// Server side
class PYAXSCRIPT_EXPORT PyGActiveScript : public PyGatewayBase, public IActiveScript {
   protected:
    PyGActiveScript(PyObject *instance) : PyGatewayBase(instance) { ; }
    /*
        public:
            static HRESULT PyGActiveScript::PyGatewayConstruct(PyObject *pPyInstance, void **ppResult, REFIID iid) {
                if (ppResult==NULL) return E_INVALIDARG;
                PyGActiveScript *newob = new PyGActiveScript(pPyInstance);
                *ppResult = (IActiveScript *)newob;
                char buf[128];
                wsprintf(buf, "New Object is %x, as IDispatch* is %x, as IUnknown* is %x, result is %x\n", newob,
    (IDispatch *)newob, (IUnknown *)newob, *ppResult); OutputDebugString(buf); return *ppResult ? S_OK : E_OUTOFMEMORY;
            }
        protected:
    #ifdef GW_USE_VIRTUAL
            virtual IID GetIID(void) { return IID_IActiveScript; }
            virtual void *ThisAsIID(IID iid)
            {
                if (iid==IID_IActiveScript)
                    return (IActiveScript *)this;
                else
                    return (IDispatch *)this; // Assumption is must be IDispatch or IUnknown
            }
    #endif
    */
    PYGATEWAY_MAKE_SUPPORT(PyGActiveScript, IActiveScript, IID_IActiveScript)

    // IActiveScript
    STDMETHOD(SetScriptSite)
    (
        /* [in]  */ IActiveScriptSite *pioss);

    STDMETHOD(GetScriptSite)
    (
        /* [in]  */ REFIID iid,
        /* [out] */ VOID **ppvSiteObject);

    STDMETHOD(SetScriptState)
    (THIS_
         /* [in]  */ SCRIPTSTATE ss);

    STDMETHOD(GetScriptState)
    (THIS_
         /* [out] */ SCRIPTSTATE *pssState);

    STDMETHOD(Close)(void);

    STDMETHOD(AddNamedItem)
    (
        /* [in]  */ LPCOLESTR pstrName,
        /* [in]  */ DWORD dwFlags);

    STDMETHOD(AddTypeLib)
    (
        /* [in]  */ REFGUID rguidTypeLib,
        /* [in]  */ DWORD dwMajor,
        /* [in]  */ DWORD dwMinor,
        /* [in]  */ DWORD dwFlags);

    STDMETHOD(GetScriptDispatch)
    (
        /* [in]  */ LPCOLESTR pstrItemName,
        /* [out] */ IDispatch **ppdisp);

    STDMETHOD(GetCurrentScriptThreadID)
    (
        /* [out] */ SCRIPTTHREADID *pstidThread);

    STDMETHOD(GetScriptThreadID)
    (
        /* [in]  */ DWORD dwWin32ThreadId,
        /* [out] */ SCRIPTTHREADID *pstidThread);

    STDMETHOD(GetScriptThreadState)
    (
        /* [in]  */ SCRIPTTHREADID stidThread,
        /* [out] */ SCRIPTTHREADSTATE *pstsState);

    STDMETHOD(InterruptScriptThread)
    (
        /* [in]  */ SCRIPTTHREADID stidThread,
        /* [in]  */ const EXCEPINFO *pexcepinfo,
        /* [in]  */ DWORD dwFlags);

    STDMETHOD(Clone)
    (
        /* [out] */ IActiveScript **ppscript);
};

class PYAXSCRIPT_EXPORT PyGActiveScriptParse : public PyGatewayBase, public IActiveScriptParse {
    PyGActiveScriptParse(PyObject *instance) : PyGatewayBase(instance) { ; }
    PYGATEWAY_MAKE_SUPPORT(PyGActiveScriptParse, IActiveScriptParse, IID_IActiveScriptParse)
    // IActiveScriptParse
    STDMETHOD(InitNew)(void);

    STDMETHOD(AddScriptlet)
    (
        /* [in] */ LPCOLESTR pstrDefaultName,
        /* [in] */ LPCOLESTR pstrCode,
        /* [in] */ LPCOLESTR pstrItemName,
        /* [in] */ LPCOLESTR pstrSubItemName,
        /* [in] */ LPCOLESTR pstrEventName,
        /* [in] */ LPCOLESTR pstrDelimiter,
        /* [in] */ DWORD_PTR dwSourceContextCookie,
        /* [in] */ ULONG ulStartingLineNumber,
        /* [in] */ DWORD dwFlags,
        /* [out] */ BSTR __RPC_FAR *pbstrName,
        /* [out] */ EXCEPINFO __RPC_FAR *pexcepinfo);

    STDMETHOD(ParseScriptText)
    (
        /* [in] */ LPCOLESTR pstrCode,
        /* [in] */ LPCOLESTR pstrItemName,
        /* [in] */ IUnknown __RPC_FAR *punkContext,
        /* [in] */ LPCOLESTR pstrDelimiter,
        /* [in] */ DWORD_PTR dwSourceContextCookie,
        /* [in] */ ULONG ulStartingLineNumber,
        /* [in] */ DWORD dwFlags,
        /* [out] */ VARIANT __RPC_FAR *pvarResult,
        /* [out] */ EXCEPINFO __RPC_FAR *pexcepinfo);
};

///////////////////////////////////////////////
// Server side
class PYAXSCRIPT_EXPORT PyGActiveScriptSite : public PyGatewayBase, public IActiveScriptSite {
   protected:
    PyGActiveScriptSite(PyObject *instance) : PyGatewayBase(instance) { ; }
    PYGATEWAY_MAKE_SUPPORT(PyGActiveScriptSite, IActiveScriptSite, IID_IActiveScriptSite)

    // IActiveScriptSite
    STDMETHOD(GetLCID)
    (
        /* [out] */ LCID FAR *plcid);

    STDMETHOD(GetItemInfo)
    (
        /* [in] */ LPCOLESTR pstrName,
        /* [in] */ DWORD dwReturnMask,
        /* [out] */ IUnknown FAR *FAR *ppiunkItem,
        /* [out] */ ITypeInfo FAR *FAR *ppti);

    STDMETHOD(GetDocVersionString)
    (
        /* [out] */ BSTR FAR *pbstrVersion);

    STDMETHOD(OnScriptTerminate)
    (
        /* [in] */ const VARIANT FAR *pvarResult,
        /* [in] */ const EXCEPINFO FAR *pexcepinfo);

    STDMETHOD(OnStateChange)
    (
        /* [in] */ SCRIPTSTATE ssScriptState);

    STDMETHOD(OnScriptError)
    (
        /* [in] */ IActiveScriptError FAR *pscripterror);

    STDMETHOD(OnEnterScript)(void);

    STDMETHOD(OnLeaveScript)(void);
};
