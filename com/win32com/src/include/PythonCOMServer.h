#ifndef __PYTHONCOMSERVER_H__
#define __PYTHONCOMSERVER_H__

// PythonCOMServer.h :Server side COM support

#include <Python.h>

#define DLLAcquireGlobalLock	PyWin_AcquireGlobalLock
#define DLLReleaseGlobalLock	PyWin_ReleaseGlobalLock

void PYCOM_EXPORT PyCom_DLLAddRef(void);
void PYCOM_EXPORT PyCom_DLLReleaseRef(void);

// Use this macro at the start of all gateway methods.
#define PY_GATEWAY_METHOD CEnterLeavePython _celp

// Gateway constructors.
// Each gateway must be able to be created from a "gateway constructor".  This
// is simply a function that takes a Python instance as as argument, and returns
// a gateway object of the correct type.  The MAKE_PYGATEWAY_CTOR is a helper that
// will embed such a constructor in the class - however, this is not necessary - 
// _any_ function of the correct signature can be used.

typedef HRESULT (* pfnPyGatewayConstructor)(PyObject *PythonInstance, void **ppResult, REFIID iid);
HRESULT PyCom_MakeRegisteredGatewayObject(REFIID iid, PyObject *instance, void **ppv);

// A version of the above which support classes being derived from
// other than IUnknown
#define PYGATEWAY_MAKE_SUPPORT2(classname, IInterface, theIID, gatewaybaseclass) \
	public: \
		static HRESULT classname::PyGatewayConstruct(PyObject *pPyInstance, void **ppResult, REFIID iid) { \
			if (ppResult==NULL) return E_INVALIDARG; \
			*ppResult = (new classname(pPyInstance))->ThisAsIID(iid);  \
			return *ppResult ? S_OK : E_OUTOFMEMORY; } \
	protected: \
		virtual IID GetIID(void) { return theIID; } \
		virtual void *ThisAsIID(IID iid) {if (this==NULL) return NULL;if (iid==theIID) return (IInterface *)this; else return gatewaybaseclass::ThisAsIID(iid);} \
		STDMETHOD_(ULONG,AddRef)(void) {return gatewaybaseclass::AddRef();} \
		STDMETHOD_(ULONG,Release)(void) {return gatewaybaseclass::Release();} \
		STDMETHOD(QueryInterface)(REFIID iid, void ** obj) {return gatewaybaseclass::QueryInterface(iid, obj);};

// This is the "old" version to use, or use it if you derive
// directly from PyGatewayBase
#define PYGATEWAY_MAKE_SUPPORT(classname, IInterface, theIID) \
	PYGATEWAY_MAKE_SUPPORT2(classname, IInterface, theIID, PyGatewayBase)


#define GET_PYGATEWAY_CTOR(classname) classname::PyGatewayConstruct

// Disable an OK warning...
#pragma warning( disable : 4275 )
// warning C4275: non dll-interface struct 'IDispatch' used as base for dll-interface class 'PyGatewayBase'

// Helper interface for fetching a Python object from a gateway

extern const GUID IID_IInternalUnwrapPythonObject;

interface IInternalUnwrapPythonObject : public IUnknown
{
public:
    STDMETHOD(Unwrap)( PyObject **ppPyObject ) = 0;
};

/////////////////////////////////////////////////////////////////////////////
// PyGatewayBase
//
// Base class for all gateways.
//
class PYCOM_EXPORT PyGatewayBase : 
#ifndef NO_PYCOM_IDISPATCHEX
	public IDispatchEx, // IDispatch comes along for the ride!
#else
	public IDispatch,   // No IDispatchEx - must explicitely use IDispatch
#endif
	public ISupportErrorInfo,
	public IInternalUnwrapPythonObject
{
protected:
	PyGatewayBase(PyObject *instance);
	~ PyGatewayBase();

	// Invoke the Python method (via the policy object)
	STDMETHOD(InvokeViaPolicy)(
		const char *szMethodName,
		PyObject **ppResult = NULL,
		const char *szFormat = NULL,
		...);

public:
	// IUnknown
	STDMETHOD_(ULONG,AddRef)(void);
	STDMETHOD_(ULONG,Release)(void);
	STDMETHOD(QueryInterface)(REFIID iid, void ** obj);

	// IDispatch
	STDMETHOD(GetTypeInfoCount)(UINT FAR* pctInfo);
	STDMETHOD(GetTypeInfo)(UINT itinfo, LCID lcid, ITypeInfo FAR* FAR* pptInfo);
	STDMETHOD(GetIDsOfNames)(REFIID refiid,	OLECHAR FAR* FAR* rgszNames, UINT cNames, LCID lcid, DISPID FAR* rgdispid);
	STDMETHOD(Invoke)(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR* params, VARIANT FAR* pVarResult, EXCEPINFO FAR* pexcepinfo, UINT FAR* puArgErr);

	// IDispatchEx
#ifndef NO_PYCOM_IDISPATCHEX
	STDMETHOD(GetDispID)(BSTR bstrName, DWORD grfdex, DISPID *pid);
	STDMETHOD(InvokeEx)(DISPID id, LCID lcid, WORD wFlags, DISPPARAMS *pdp, VARIANT *pvarRes, EXCEPINFO *pei, IServiceProvider *pspCaller);
	STDMETHOD(DeleteMemberByName)(BSTR bstr, DWORD grfdex);
	STDMETHOD(DeleteMemberByDispID)(DISPID id);
	STDMETHOD(GetMemberProperties)(DISPID id, DWORD grfdexFetch, DWORD *pgrfdex);
	STDMETHOD(GetMemberName)(DISPID id, BSTR *pbstrName);
	STDMETHOD(GetNextDispID)(DWORD grfdex, DISPID id, DISPID *pid);
	STDMETHOD(GetNameSpaceParent)(IUnknown **ppunk);
#endif // NO_PYCOM_IDISPATCHEX
	// ISupportErrorInfo
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

	// IInternalUnwrapPythonObject
	STDMETHOD(Unwrap)(PyObject **ppPyObject);

	// Basically just PYGATEWAY_MAKE_SUPPORT(PyGatewayBase, IDispatch, IID_IDispatch);
	// but with special handling as its the base class.
	static HRESULT PyGatewayBase::PyGatewayConstruct(PyObject *pPyInstance, void **ppResult, REFIID iid)
	{
		if (ppResult==NULL) return E_INVALIDARG;
		*ppResult = (IDispatch *)(PyGatewayBase *)new PyGatewayBase(pPyInstance);
		return *ppResult ? S_OK : E_OUTOFMEMORY;
	}
	// Currently this is used only for ISupportErrorInfo,
	// so hopefully this will never be called in this base class.
	// (however, this is not a rule, so we wont assert or anything!)
	virtual IID GetIID(void) { return IID_IUnknown; }
	virtual void *ThisAsIID(IID iid);
	// End of PYGATEWAY_MAKE_SUPPORT
	PyObject * m_pPyObject;
private:
	PyGatewayBase *m_pBaseObject;
	LONG m_cRef;
};

#pragma warning(default : 4275 )

// B/W compat hack for gateways.
#define PyCom_HandlePythonFailureToCOM() PyCom_SetCOMErrorFromPyException(GetIID())


#endif /* __PYTHONCOMSERVER_H__ */
