//
// PyIID.cpp -- IID type for Python
//
// @doc

#include "windows.h"
#include "Python.h"
#include "PyWinTypes.h"
#include "PyWinObjects.h"

#ifndef  NO_PYWINTYPES_IID
// @pymethod <o PyIID>|pywintypes|IID|Creates a new IID object
PyObject *PyWinMethod_NewIID(PyObject *self, PyObject *args)
{
	BSTR bstrIID;
	PyObject *obIID;
	IID iid;

	// @pyparm string/Unicode|iidString||A string representation of an IID, or a ProgID.
	if ( !PyArg_ParseTuple(args, "O", &obIID) )
		return NULL;
	// Already an IID? Return self.
	if ( PyIID_Check(obIID) ) {
		Py_INCREF(obIID);
		return obIID;
	}
	if (!PyWinObject_AsBstr(obIID, &bstrIID))
		return NULL;

	HRESULT hr = CLSIDFromString(bstrIID, &iid);
	if ( FAILED(hr) )
	{
#ifndef MS_WINCE
		hr = CLSIDFromProgID(bstrIID, &iid);
		if ( FAILED(hr) )
		{
#endif
			PyWinObject_FreeBstr(bstrIID);
			PyWin_SetBasicCOMError(hr);
			return NULL;
#ifndef MS_WINCE
		}
#endif
	}
	PyWinObject_FreeBstr(bstrIID);
	/* iid -> PyObject */
	return PyWinObject_FromIID(iid);
}

static HRESULT myCLSIDFromString(OLECHAR *str, CLSID *clsid)
{
	HRESULT hr = CLSIDFromString(str, clsid);
#ifdef MS_WINCE
	return hr;
#else
	if ( SUCCEEDED(hr) )
		return hr;
	return CLSIDFromProgID(str, clsid);
#endif
}

BOOL PyWinObject_AsIID(PyObject *obCLSID, CLSID *clsid)
{
	BSTR bstrCLSID;
	if ( PyIID_Check(obCLSID) ) {
		*clsid = ((PyIID *)obCLSID)->m_iid;
	} else if (PyWinObject_AsBstr(obCLSID, &bstrCLSID, FALSE)) {
		HRESULT hr = myCLSIDFromString(bstrCLSID, clsid);
		PyWinObject_FreeBstr(bstrCLSID);
		if ( FAILED(hr) )
		{
			PyWin_SetBasicCOMError(hr);
			return FALSE;
		}
	} else {
		PyErr_Clear();
		PyErr_SetString(PyExc_TypeError, "Only strings and iids can be converted to a CLSID.");
		return FALSE;
	}
	return TRUE;
}

PyObject *PyWinObject_FromIID(const IID &riid)
{
	// Later we could cache common IIDs - say IUnknown, IDispatch and NULL?
	PyObject *rc = new PyIID(riid);
	if (rc==NULL)
		PyErr_SetString(PyExc_MemoryError, "allocating new PyIID object");
	return rc;
}

PyObject *PyWinStringObject_FromIID(const IID &riid)
{
	OLECHAR oleRes[128];
	if (StringFromGUID2(riid, oleRes, sizeof(oleRes))==0) {
		// Should never happen - 128 should be heaps big enough.
		PyErr_SetString(PyExc_ValueError, "The string is too long");
		return NULL;
	}
	char *szResult;
	if (!PyWin_WCHAR_AsString(oleRes, -1, &szResult))
		return NULL;
	PyObject *rc = PyString_FromString(szResult);
	PyWinObject_FreeString(szResult);
	return rc;
}

PyObject *PyWinUnicodeObject_FromIID(const IID &riid)
{
	OLECHAR oleRes[128];
	if (StringFromGUID2(riid, oleRes, sizeof(oleRes))==0) {
		// Should never happen - 128 should be heaps big enough.
		PyErr_SetString(PyExc_ValueError, "The string is too long");
		return NULL;
	}
	return PyWinObject_FromOLECHAR(oleRes);
}


// @object PyIID|A Python object, representing an IID/CLSID.
// <nl>All pythoncom functions that return a CLSID/IID will return one of these
// objects.  However, in almost all cases, functions that expect a CLSID/IID
// as a param will accept either a string object, or a native PyIID object.
PYWINTYPES_EXPORT PyTypeObject PyIIDType =
{
	PyObject_HEAD_INIT(&PyType_Type)
	0,
	"PyIID",
	sizeof(PyIID),
	0,
	PyIID::deallocFunc,		/* tp_dealloc */
	// @pymeth __print__|Used when the IID object is printed.
	PyIID::printFunc,		/* tp_print */
	0,						/* tp_getattr */
	0,						/* tp_setattr */
	// @pymeth __cmp__|Used when IID objects are compared.
	PyIID::compareFunc,		/* tp_compare */
	0,						/* tp_repr */
	0,						/* tp_as_number */
	0,						/* tp_as_sequence */
	0,						/* tp_as_mapping */
	// @pymeth __hash__|Used when the hash value of an IID object is required
	PyIID::hashFunc,		/* tp_hash */
	0,						/* tp_call */
	// @pymeth __str__|Used whenever a string representation of the IID is required.
	PyIID::strFunc,			/* tp_str */
};

PyIID::PyIID(REFIID riid)
{
	ob_type = &PyIIDType;
	_Py_NewReference(this);
	m_iid = riid;
}

int PyIID::IsEqual(REFIID riid)
{
	return IsEqualIID(m_iid, riid);
}

int PyIID::IsEqual(PyObject *ob)
{
	if ( ob->ob_type != &PyIIDType )
		return 0;
	return IsEqualIID(m_iid, ((PyIID *)ob)->m_iid);
}

int PyIID::IsEqual(PyIID &iid)
{
	return IsEqualIID(m_iid, iid.m_iid);
}

int PyIID::compare(PyObject *ob)
{
	return memcmp(&m_iid, &((PyIID *)ob)->m_iid, sizeof(m_iid));
}

int PyIID::print(FILE *fp, int flags)
{
	OLECHAR oleRes[128];
	StringFromGUID2(m_iid, oleRes, sizeof(oleRes));
//	USES_CONVERSION;

	TCHAR buf[128];
	wsprintf(buf, _T("<iid:%ws>"), oleRes);

	//
    // ### ACK! Python uses a non-debug runtime. We can't use stream
	// ### functions when in DEBUG mode!!  (we link against a different
	// ### runtime library)  Hack it by getting Python to do the print!
	//
	// ### - Double Ack - Always use the hack!
// #ifdef _DEBUG
	PyObject *ob = PyString_FromTCHAR(buf);
	PyObject_Print(ob, fp, flags|Py_PRINT_RAW);
	Py_DECREF(ob);
//#else
///	fputs(buf, fp);
//#endif

	return 0;
}

long PyIID::hash(void)
{
	DWORD n[4];

	memcpy(n, &m_iid, sizeof(n));
	n[0] += n[1] + n[2] + n[3];
	if ( n[0] == -1 )
		return -2;
	return n[0];
}

PyObject *PyIID::str(void)
{
	return PyWinStringObject_FromIID(m_iid);
}

/*static*/ void PyIID::deallocFunc(PyObject *ob)
{
	delete (PyIID *)ob;
}

// @pymethod |PyIID|__print__|Used when the IID object is printed.
int PyIID::printFunc(PyObject *ob, FILE *fp, int flags)
{
	return ((PyIID *)ob)->print(fp, flags);
}
// @pymethod int|PyIID|__cmp__|Used when IID objects are compared.
int PyIID::compareFunc(PyObject *ob1, PyObject *ob2)
{
	return ((PyIID *)ob1)->compare(ob2);
}
// @pymethod int|PyIID|__hash__|Used when the hash value of an IID object is required
long PyIID::hashFunc(PyObject *ob)
{
	return ((PyIID *)ob)->hash();
}
// @pymethod string|PyIID|__str__|Used whenever a string representation of the IID is required.
PyObject * PyIID::strFunc(PyObject *ob)
{
	return ((PyIID *)ob)->str();
}
#endif // NO_PYWINTYPES_IID
