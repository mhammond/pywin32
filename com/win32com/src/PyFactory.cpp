/*
** Implementation for the CPyFactory class
*/

#include "stdafx.h"

#include <Import.h>		/* for PyImport_ImportModule() */

#include "PythonCOM.h"
#include "PyFactory.h"
#include "PythonCOMServer.h"

// Class Factories do not count against the DLLs total reference count.
static LONG factoryRefCount = 0;

extern void PyCom_LogF(const TCHAR *fmt, ...);
#define LogF PyCom_LogF

CPyFactory::CPyFactory(REFCLSID guidClassID) :
	m_guidClassID(guidClassID),
	m_cRef(1)
{
	InterlockedIncrement(&factoryRefCount);
}
CPyFactory::~CPyFactory()
{
	InterlockedDecrement(&factoryRefCount);
}

STDMETHODIMP CPyFactory::QueryInterface(REFIID iid, void **ppv)
{
	*ppv = NULL;

	if ( IsEqualIID(iid, IID_IUnknown) ||
		 IsEqualIID(iid, IID_IClassFactory) )
	{
		*ppv = this;
		AddRef();
		return S_OK;
	}

	return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CPyFactory::AddRef(void)
{
	return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) CPyFactory::Release(void)
{
	LONG cRef = InterlockedDecrement(&m_cRef);
	if ( cRef == 0 )
		delete this;
	return cRef;
}

STDMETHODIMP CPyFactory::CreateInstance(
	IUnknown *punkOuter,
	REFIID riid,
	void **ppv
	)
{
//	LogF("in CPyFactory::CreateInstance");

	if ( ppv == NULL )
		return E_POINTER;
	*ppv = NULL;

	if ( punkOuter != NULL )
		return CLASS_E_NOAGGREGATION;

	// Add a temporary reference to the main DLL, so that the Python
	// Init/Finalize semantics work correctly.
	// If we ignore Factory reference counts, there is a possibility
	// that the DLL global ref count will transition 1->0->1 during the
	// creation process.  To prevent this, we add an artificial lock
	// and remove it when done.
	HRESULT hr;
	PyCom_DLLAddRef();
	{ // scope to ensure CEnterLeave destructs before (possibly final) PyCom_DLLReleaseRef
		CEnterLeavePython celp;

		PyObject *pNewInstance = NULL;
		hr = CreateNewPythonInstance(m_guidClassID, riid, &pNewInstance);
		if ( FAILED(hr) )
		{
			LogF(_T("CPyFactory::CreateInstance failed to create instance. (%lx)"), hr);
		} 
		else 
		{
		   // CreateInstance now returns an object already all wrapped
		   // up (giving more flexibility to the Python programmer.
			if (!PyCom_InterfaceFromPyObject(pNewInstance, riid, ppv, FALSE)) {
				LogF(_T("CPyFactory::CreateInstance failed to get gateway to returned object"));
				hr = E_FAIL;
			}
		}
		Py_XDECREF(pNewInstance); // Dont need it any more.
	}
	PyCom_DLLReleaseRef();
	return hr;
}

STDMETHODIMP CPyFactory::LockServer(BOOL fLock)
{
	if ( fLock )
		PyCom_DLLAddRef();
	else
		PyCom_DLLReleaseRef();

	return S_OK;
}

// NOTE NOTE: CreateNewPythonInstance assumes that you have the Python thread lock 
// already acquired.
STDMETHODIMP CPyFactory::CreateNewPythonInstance(REFCLSID rclsid, REFCLSID rReqiid, PyObject **ppNewInstance)
{
	extern BOOL LoadGatewayModule(PyObject **);
	PyObject *pPyModule;

	if ( ppNewInstance == NULL )
		return E_INVALIDARG;

	if ( !LoadGatewayModule(&pPyModule) )
		return E_FAIL;

	PyObject *obiid = PyWinObject_FromIID(rclsid);
	PyObject *obReqiid = PyWinObject_FromIID(rReqiid);
	if ( !obiid || !obReqiid)
	{
		Py_XDECREF(pPyModule);
		Py_XDECREF(obiid);
		Py_XDECREF(obReqiid);
		PyErr_Clear(); // nothing Python can do!
		return E_OUTOFMEMORY;
	}

	*ppNewInstance = PyObject_CallMethod(pPyModule, "CreateInstance",
										 "OO", obiid, obReqiid);
	Py_DECREF(obiid);
	Py_DECREF(obReqiid);
	Py_DECREF(pPyModule);


	HRESULT hr = PyCom_SetCOMErrorFromPyException(IID_IClassFactory);
//	if ( !*ppNewInstance )PyRun_SimpleString("import traceback;traceback.print_exc()");
//	PyErr_Clear();	/* ### what to do with exceptions? ... */

	if ( !*ppNewInstance )
	{
		LogF(_T("ERROR: server.policy could not create an instance."));
	}

	return hr;
}

/*
** Load our C <-> Python gateway module if needed

  NOTE: Assumes the Python lock already acquired for us by our caller.
*/
BOOL LoadGatewayModule(PyObject **ppModule)
{
	PyObject *pPyModule = NULL;
	pPyModule = PyImport_ImportModule("win32com.server.policy");
 	if ( !pPyModule )
	{
		LogF(_T("PythonCOM Server - The policy module could not be loaded."));
		/* ### propagate the exception? */
		PyErr_Clear();
		return FALSE;
	}

	*ppModule = pPyModule;
	return TRUE;
}
void FreeGatewayModule(void)
{
/*****
	if ( g_pPyModule != NULL )
	{
		Py_DECREF(g_pPyModule);
		g_pPyModule = NULL;
	}
*****/
}
