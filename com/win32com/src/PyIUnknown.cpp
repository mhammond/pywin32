// PyIUnknown

// @doc
#include "stdafx.h"
#include "PythonCOM.h"
#include "PythonCOMServer.h"

extern void PyCom_LogF(const char *fmt, ...);
#define LogF PyCom_LogF

char *PyIUnknown::szErrMsgObjectReleased = "The COM object has been released.";

static LONG cUnknowns=0;

LONG _PyCom_GetInterfaceCount(void)
{
	return cUnknowns;
}

PyIUnknown::PyIUnknown(IUnknown *punk)
{
	ob_type = &type;
	m_obj = punk;
	// refcnt of object managed by caller.
	InterlockedIncrement(&cUnknowns);
	PyCom_DLLAddRef();
}

PyIUnknown::~PyIUnknown()
{
	SafeRelease(this);	
	InterlockedDecrement(&cUnknowns);
	PyCom_DLLReleaseRef();
}
// @method string|PyIUnknown|__repr__|Called to create a representation of a PyIUnknown object
PyObject * PyIUnknown::repr()
{
	// @comm The repr of this object displays both the object's address, and its attached IUnknown's address
	TCHAR buf[80];
	wsprintf(buf, _T("<%hs at 0x%0lx with obj at 0x%0lx>"),ob_type->tp_name, (long)(PyObject *)this, (long)m_obj);
	return PyString_FromTCHAR(buf);
}

/*static void PyIUnknown::CleanupTrackList()
{
#ifdef _DEBUG
	int numInMap = m_obTrackList ? PyMapping_Length(m_obTrackList) : 0;
	LogF("Cleaning up %d COM objects...", numInMap);
	USES_CONVERSION;
	OLECHAR FAR *pythonOb = A2OLE("pythonObject");
#endif
	if (m_obTrackList) {
		AllocThreadState();
		PyObject *keys = PyMapping_Keys(m_obTrackList);
		if (keys) {
			int len = PySequence_Length(keys);
			for (int index=0;index<len;index++) {
				PyObject *intLook = PySequence_GetItem(keys, index);
				PyIUnknown *pLook = (PyIUnknown *)PyInt_AsLong(intLook);
				if (pLook) {
#ifdef NOPE_DEBUG
					const char *relDesc = pLook->m_obj ? "NOT RELEASED" : "released";
					LogF(" object <%s> at 0x%0lx, m_obj at 0x%0lx, ob_refcnt=%d, %s", pLook->ob_type->tp_name, pLook, pLook->m_obj, pLook->ob_refcnt, relDesc);
					if ( pLook->m_obj )
					{
						IDispatch *pdisp;
						HRESULT hr = pLook->m_obj->QueryInterface(IID_IDispatch, (LPVOID *)&pdisp);
						if ( SUCCEEDED(hr) )
						{
							DISPID dispid;
							hr = pdisp->GetIDsOfNames(IID_NULL, &pythonOb, 1, LOCALE_SYSTEM_DEFAULT, &dispid);
							if ( SUCCEEDED(hr) )
							{
								DISPPARAMS dispparams = { NULL, NULL, 0, 0 };
								VARIANT result;
								VariantInit(&result);
								hr = pdisp->Invoke(dispid, IID_NULL, LOCALE_SYSTEM_DEFAULT, DISPATCH_METHOD, &dispparams, &result, NULL, NULL);
								if ( SUCCEEDED(hr) && V_VT(&result) == VT_I4 )
								{
									PyObject *ob = (PyObject *)V_I4(&result);
									if ( PyInstance_Check(ob) )
									{
										LogF("   object is a Python class instance of: %s", PyString_AsString(((PyInstanceObject *)ob)->in_class->cl_name));
									}
									else
									{
										LogF("   object is a Python object of type: %s", ob->ob_type->tp_name);
									}
								}
							}

							/* successful QI; need to release it 
							pdisp->Release();
						}
					}
#endif // _DEBUG
//					SafeRelease(pLook);
				}
			}
		}
		Py_XDECREF(keys);
		// no need to actually remove each item from the map - just
		// remove ref to the map.
		Py_DECREF(m_obTrackList);
		m_obTrackList = NULL;
		FreeThreadState();
	}
#ifdef _DEBUG
	LogF("COM object cleanup complete.");
#endif
}
*/
/*static*/ IUnknown *PyIUnknown::GetI(PyObject *self)
{
	if (self==NULL) {
		PyCom_BuildInternalPyException("The Python object is invalid");
		return NULL;
	}
	PyIUnknown *pPyUnk = (PyIUnknown *)self;
	if (pPyUnk->m_obj==NULL) {
		PyCom_BuildInternalPyException(szErrMsgObjectReleased);
		return NULL;
	}
	return pPyUnk->m_obj;
}

/*static*/ void PyIUnknown::SafeRelease(PyIUnknown *ob)
{
	if (!ob)
		return;
	if (ob->m_obj)
	{
		// Safe for all objects which delete 
		// itself ignoring a reference count.
		PyThreadState *_save;
		__try
		{
			_save = PyEval_SaveThread();
			long rcnt = ob->m_obj->Release();
			PyEval_RestoreThread(_save);

#ifdef _DEBUG_LIFETIMES
			LogF(buf, "   SafeRelease(%ld) -> %s at 0x%0lx, IUnknown at 0x%0lx - Release() returned %ld",GetCurrentThreadId(), ob->ob_type->tp_name,ob, ob->m_obj,rcnt);
#endif
			ob->m_obj = NULL;
		}
		__except( EXCEPTION_EXECUTE_HANDLER )
		{
			LogF(_T("Exception occured:\n\tTry to release object at adress 0x%08x\n\twhich was already destroyed"), ob->m_obj);
			PyEval_RestoreThread(_save);
			ob->m_obj = NULL;
			return;
		}
	}
}

// @pymethod int|PyIUnknown|__cmp__|Implements COM rules for object identity.
int PyIUnknown::compare(PyObject *other)
{
	// @comm As per the COM rules for object identity, both objects are queried for IUnknown, and these values compared.
	// The only meaningful test is for equality - the result of other comparisons is undefined
	// (ie, determined by the object's relative addresses in memory.
	IUnknown *pUnkOther;
	IUnknown *pUnkThis;
	if (!PyCom_InterfaceFromPyObject(this, IID_IUnknown, (void **)&pUnkThis, FALSE))
		return -1;
	if (!PyCom_InterfaceFromPyObject(other, IID_IUnknown, (void **)&pUnkOther, FALSE)) {
		pUnkThis->Release();
		return -1;
	}
	int rc = pUnkThis==pUnkOther ? 0 :
		(pUnkThis < pUnkOther ? -1 : 1);
	pUnkThis->Release();
	pUnkOther->Release();
	return rc;
}

// @pymethod <o PyIUnknown>|PyIUnknown|QueryInterface|Queries an object for a specific interface.
PyObject *PyIUnknown::QueryInterface(PyObject *self, PyObject *args)
{
	PyObject *obiid;
	PyObject *obUseIID = NULL;
	// @pyparm IID|iid||The IID requested.
	// @pyparm IID|useIID|None|If provided and not None, will return an
	// interface for the specified IID if (and only if) a native interface can not be supported.
	// If the interface specified by iid is natively supported, this option is ignored.
	// @comm The useIID parameter is a very dangerous option, and should only
	// be used when you are sure you need it!
	// By specifying this parameter, you are telling the COM framework that regardless
	// of the true type of the result (as specified by iid), a Python wrapper
	// of type useIID will be created.  If iid does not derive from useIID,
	// then it is almost certain that using the object will cause an Access Violation.
	// <nl>For example, this option can be used to obtain a PyIUnknown object if
	// pythoncom does not natively support the interface. 
	// Another example might be to return an unsupported persistence interface as a
	// PyIPersist instance.<nl>
	// For backwards compatibility: the integer 0 implies None, and the
	// integer 1 implies IID_IUnknown.
	// @rdesc The result is always an object derived from PyIUnknown.
	// Any error (including E_NOINTERFACE) will generate a <o com_error> exception.
	if (!PyArg_ParseTuple(args, "O|O:QueryInterface", &obiid, &obUseIID ))
		return NULL;

	IID	iid;
	if (!PyWinObject_AsIID(obiid, &iid))
		return NULL;

	IID useIID;	/* used if obUseIID != NULL */

	if ( obUseIID != NULL )
	{
		if ( obUseIID == Py_None )
			obUseIID = NULL;
		else if ( PyInt_Check(obUseIID) )
		{
			if ( PyInt_AS_LONG((PyIntObject *)obUseIID) )
				useIID = IID_IUnknown;
			else
				obUseIID = NULL;
		}
		else if ( !PyWinObject_AsIID(obUseIID, &useIID) )
			return NULL;
	}

	IUnknown *pMyUnknown = GetI(self);
	if (pMyUnknown==NULL) return NULL;

	IUnknown *punk;
	PY_INTERFACE_PRECALL;
	HRESULT hr = pMyUnknown->QueryInterface(iid, (LPVOID*)&punk);
	PY_INTERFACE_POSTCALL;

	/* Note that this failure may include E_NOINTERFACE */
	if ( FAILED(hr) )
		return PyCom_BuildPyException(hr, pMyUnknown, IID_IUnknown);

	/* Return a type based on the IID (with no extra ref) */
	PyObject *rc = PyCom_PyObjectFromIUnknown(punk, iid, FALSE);

	/* we may have been asked to use a different interface */
	if ( rc == NULL && obUseIID != NULL )
	{
		PyErr_Clear();
		rc = PyCom_PyObjectFromIUnknown(punk, useIID, FALSE);
	}
	return rc;
}

// @object PyIUnknown|The base object for all PythonCOM objects.  Wraps a COM IUnknown object.
static struct PyMethodDef PyIUnknown_methods[] =
{
	{ "QueryInterface", PyIUnknown::QueryInterface, 1 }, // @pymeth QueryInterface|Queries the object for an interface.
	{NULL,  NULL}        
};
// @comm Note that there are no reference counting functions that are typically exposed via COM.
// This is because COM reference counts are automatically handled by PythonCOM - each interface
// object keeps exactly one COM reference, regardless of how many Python references.  When the
// Python object destructs due to its reference count hitting zero, the COM reference is then
// released.  It is not possible for force the closure of a PythonCOM object - the only
// way to ensure cleanup is to remove all Python references.

PyComTypeObject PyIUnknown::type("PyIUnknown",
                 NULL,
                 sizeof(PyIUnknown),
                 PyIUnknown_methods,
				 GET_PYCOM_CTOR(PyIUnknown));

