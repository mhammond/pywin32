//
// @doc

#include "windows.h"
#include "Python.h"
#include "structmember.h"
#include "PyWinTypes.h"
#include "PyWinObjects.h"

// @pymethod <o PyHANDLE>|pywintypes|HANDLE|Creates a new HANDLE object
PyObject *PyWinMethod_NewHANDLE(PyObject *self, PyObject *args)
{
	HANDLE hInit;
	if (!PyArg_ParseTuple(args, "|i:HANDLE", &hInit))
		return NULL;
	return new PyHANDLE(hInit);
}

BOOL PyWinObject_AsHANDLE(PyObject *ob, HANDLE *pHANDLE, BOOL bNoneOK /*= TRUE*/)
{
	if (ob==Py_None) {
		if (!bNoneOK) {
			PyErr_SetString(PyExc_TypeError, "None is not a valid HANDLE in this context");
			return FALSE;
		}
		*pHANDLE = (HANDLE)0;
	} else if (PyHANDLE_Check(ob)) {
		PyHANDLE *pH = (PyHANDLE *)ob;
		*pHANDLE = (HANDLE)(*pH);
	} else if (PyInt_Check(ob)) { // Support integer objects for b/w compat.
		*pHANDLE = (HANDLE)PyInt_AsLong(ob);
	} else {
		PyErr_SetString(PyExc_TypeError, "The object is not a PyHANDLE object");
		return FALSE;
	}
	return TRUE;
}

PyObject *PyWinObject_FromHANDLE(HANDLE h)
{
	return new PyHANDLE(h);
}

BOOL PyWinObject_CloseHANDLE(PyObject *obHandle)
{
	BOOL ok;
	if (PyHANDLE_Check(obHandle))
		// Python error already set.
		ok = ((PyHANDLE *)obHandle)->Close();
	else if PyInt_Check(obHandle) {
		ok = ::CloseHandle((HANDLE)PyInt_AsLong(obHandle));
		if (!ok)
			PyWin_SetAPIError("CloseHandle");
	} else {
		PyErr_SetString(PyExc_TypeError, "A handle must be a HANDLE object or an integer");
		return FALSE;
	}
	return ok;
}

// @pymethod |PyHANDLE|Close|Closes the underlying Win32 handle.
// @comm If the handle is already closed, no error is raised.
PyObject *PyHANDLE::Close(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ":Close"))
		return NULL;
	if (!((PyHANDLE *)self)->Close())
		return NULL;
	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod int|PyHANDLE|Detach|Detaches the Win32 handle from the handle object.
// @rdesc The result is the value of the handle before it is detached.  If the
// handle is already detached, this will return zero.
// @comm After calling this function, the handle is effectively invalidated,
// but the handle is not closed.  You would call this function when you
// need the underlying win32 handle to exist beyond the lifetime of the
// handle object.
PyObject *PyHANDLE::Detach(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ":Detach"))
		return NULL;
	PyHANDLE *pThis = (PyHANDLE *)self;
	long ret = (long)pThis->m_handle;
	pThis->m_handle = 0;
	return PyInt_FromLong(ret);
}

// @object PyHANDLE|A Python object, representing a win32 HANDLE.
// @comm This object wraps a win32 HANDLE object, automatically closing it when the object
// is destroyed.  To guarantee cleanup, you can call either <om PyHANDLE.Close>, or 
// <om win32api.CloseHandle>.
// <nl>Most functions which accept a handle object also accept an integer - however,
// use of the handle object is encouraged.

static struct PyMethodDef PyHANDLE_methods[] = {
	{"Close",     PyHANDLE::Close, 1}, 	// @pymeth Close|Closes the handle
	{"Detach",     PyHANDLE::Detach, 1}, 	// @pymeth Detach|Detaches the Win32 handle from the handle object.
	{NULL}
};

static PyNumberMethods PyHANDLE_NumberMethods =
{
	PyHANDLE::binaryFailureFunc,	/* nb_add */
	PyHANDLE::binaryFailureFunc,	/* nb_subtract */
	PyHANDLE::binaryFailureFunc,	/* nb_multiply */
	PyHANDLE::binaryFailureFunc,	/* nb_divide */
	PyHANDLE::binaryFailureFunc,	/* nb_remainder */
	PyHANDLE::binaryFailureFunc,	/* nb_divmod */
	PyHANDLE::ternaryFailureFunc,	/* nb_power */
	PyHANDLE::unaryFailureFunc,	/* nb_negative */
	PyHANDLE::unaryFailureFunc,	/* nb_positive */
	PyHANDLE::unaryFailureFunc,	/* nb_absolute */
	// @pymeth  __nonzero__|Used for detecting true/false.
	PyHANDLE::nonzeroFunc,
	PyHANDLE::unaryFailureFunc,	/* nb_invert */
	PyHANDLE::binaryFailureFunc,	/* nb_lshift */
	PyHANDLE::binaryFailureFunc,	/* nb_rshift */
	PyHANDLE::binaryFailureFunc,	/* nb_and */
	PyHANDLE::binaryFailureFunc,	/* nb_xor */
	PyHANDLE::binaryFailureFunc,	/* nb_or */
	0,							/* nb_coerce (allowed to be zero) */
	PyHANDLE::intFunc,			/* nb_int */
	PyHANDLE::unaryFailureFunc,	/* nb_long */
	PyHANDLE::unaryFailureFunc,	/* nb_float */
	PyHANDLE::unaryFailureFunc,	/* nb_oct */
	PyHANDLE::unaryFailureFunc,	/* nb_hex */
};
// @pymeth __int__|Used when an integer representation of the handle object is required.

PYWINTYPES_EXPORT PyTypeObject PyHANDLEType =
{
	PyObject_HEAD_INIT(&PyType_Type)
	0,
	"PyHANDLE",
	sizeof(PyHANDLE),
	0,
	PyHANDLE::deallocFunc,		/* tp_dealloc */
	// @pymeth __print__|Used when the object is printed.
	PyHANDLE::printFunc,		/* tp_print */
	PyHANDLE::getattr,				/* tp_getattr */
	0, // PyHANDLE::setattr,				/* tp_setattr */
	// @pymeth __cmp__|Used when HANDLE objects are compared.
	PyHANDLE::compareFunc,	/* tp_compare */
	0,						/* tp_repr */
	&PyHANDLE_NumberMethods,	/* tp_as_number */
	0,	/* tp_as_sequence */
	0,						/* tp_as_mapping */
	// @pymeth __hash__|Used when the hash value of an object is required
	PyHANDLE::hashFunc,		/* tp_hash */
	0,						/* tp_call */
	// @pymeth __str__|Used when a string representation is required
	PyHANDLE::strFunc,		/* tp_str */
};

#define OFF(e) offsetof(PyHANDLE, e)

/*static*/ struct memberlist PyHANDLE::memberlist[] = {
	{"handle",      T_INT,      OFF(m_handle)}, // @prop integer|handle|The win32 handle itself, as an integer.
	{NULL}	/* Sentinel */
};

PyHANDLE::PyHANDLE(HANDLE hInit)
{
	ob_type = &PyHANDLEType;
	_Py_NewReference(this);
	m_handle = hInit;
}

PyHANDLE::~PyHANDLE(void)
{
	// can not call Close here, as it is a virtual, and therefore
	// will not correctly call a derived class.
}

BOOL PyHANDLE::Close(void)
{
	BOOL rc = m_handle ? CloseHandle(m_handle) : TRUE;
	m_handle = 0;
	if (!rc)
		PyWin_SetAPIError("CloseHandle");
	return rc;
}

// @pymethod |PyHANDLE|__nonzero__|Used for detecting true/false.
// @rdesc The result is 1 if the attached handle is non zero, else 0.
/*static*/ int PyHANDLE::nonzeroFunc(PyObject *ob)
{
	return ((PyHANDLE *)ob)->m_handle != 0;
}

int PyHANDLE::compare(PyObject *ob)
{
	
	return  m_handle == ((PyHANDLE *)ob)->m_handle ? 0 :
		(m_handle < ((PyHANDLE *)ob)->m_handle ? -1 : 1);
}

long PyHANDLE::asLong(void)
{
	return (long)m_handle;
}

// @pymethod |PyHANDLE|__int__|Used when the handle as an integer is required.
// @comm To get the underling win32 handle from a PyHANDLE object, use int(handleObject)
PyObject * PyHANDLE::intFunc(PyObject *ob)
{
	long result = ((PyHANDLE *)ob)->asLong();
	if ( result == -1 )
		return NULL;
	return PyInt_FromLong(result);
}

// @pymethod |PyHANDLE|__print__|Used when the HANDLE object is printed.
int PyHANDLE::printFunc(PyObject *ob, FILE *fp, int flags)
{
	return ((PyHANDLE *)ob)->print(fp, flags);
}

// @pymethod |PyHANDLE|__str__|Used when a string representation of the handle object is required.
 PyObject * PyHANDLE::strFunc(PyObject *ob)
{
	return ((PyHANDLE *)ob)->asStr();
}


// @pymethod int|PyHANDLE|__cmp__|Used when objects are compared.
int PyHANDLE::compareFunc(PyObject *ob1, PyObject *ob2)
{
	return ((PyHANDLE *)ob1)->compare(ob2);
}

// @pymethod int|PyHANDLE|__hash__|Used when the hash value of a HANDLE object is required
long PyHANDLE::hashFunc(PyObject *ob)
{
	return ((PyHANDLE *)ob)->hash();
}


long PyHANDLE::hash(void)
{
	// Just use the address.
	return (long)this;
}

int PyHANDLE::print(FILE *fp, int flags)
{
	TCHAR resBuf[160];
	wsprintf(resBuf, _T("<%hs at %ld (%ld)>"), GetTypeName(), (long)this, (long)m_handle);
    // ### ACK! Python uses a non-debug runtime. We can't use stream
	// ### functions when in DEBUG mode!!  (we link against a different
	// ### runtime library)  Hack it by getting Python to do the print!
	//
	// ### - Double Ack - Always use the hack!
//#ifdef _DEBUG
	PyObject *ob = PyString_FromTCHAR(resBuf);
	PyObject_Print(ob, fp, flags|Py_PRINT_RAW);
	Py_DECREF(ob);
/***
#else
	fputs(resBuf, fp);
#endif
***/
	return 0;
}

PyObject * PyHANDLE::asStr(void)
{
	TCHAR resBuf[160];
	wsprintf(resBuf, _T("<%s:%ld>"), GetTypeName(), (long)m_handle);
	return PyString_FromTCHAR(resBuf);
}

PyObject *PyHANDLE::getattr(PyObject *self, char *name)
{
	PyObject *res;

	res = Py_FindMethod(PyHANDLE_methods, self, name);
	if (res != NULL)
		return res;
	PyErr_Clear();
	if (strcmp(name, "handle")==0)
		return PyInt_FromLong((long)((PyHANDLE *)self)->m_handle);
	return PyMember_Get((char *)self, memberlist, name);
}

/*int PyHANDLE::setattr(PyObject *self, char *name, PyObject *v)
{
	if (v == NULL) {
		PyErr_SetString(PyExc_AttributeError, "can't delete HANDLE attributes");
		return -1;
	}
	return PyMember_Set((char *)self, memberlist, name, v);
} */

char *failMsg = "bad operand type";
/*static*/ PyObject *PyHANDLE::unaryFailureFunc(PyObject *ob)
{
	PyErr_SetString(PyExc_TypeError, failMsg);
	return NULL;
}
/*static*/ PyObject *PyHANDLE::binaryFailureFunc(PyObject *ob1, PyObject *ob2)
{
	PyErr_SetString(PyExc_TypeError, failMsg);
	return NULL;
}
/*static*/ PyObject *PyHANDLE::ternaryFailureFunc(PyObject *ob1, PyObject *ob2, PyObject *ob3)
{
	PyErr_SetString(PyExc_TypeError, failMsg);
	return NULL;
}

/*static*/ void PyHANDLE::deallocFunc(PyObject *ob)
{
	// Call virtual method Close
	((PyHANDLE *)ob)->Close();
    PyErr_Clear(); // can not leave pending exceptions in destructors.
	delete (PyHANDLE *)ob;
}


// A Registry handle.
// @object PyHKEY|A Python object, representing a win32 HKEY (a HANDLE> to a registry key).
// See the <o PyHANDLE> object for more details
BOOL PyWinObject_AsHKEY(PyObject *ob, HKEY *pRes, BOOL bNoneOK)
{
	return PyWinObject_AsHANDLE(ob, (HANDLE *)pRes, bNoneOK);
}
PyObject *PyWinObject_FromHKEY(HKEY h)
{
	return new PyHKEY(h);
}
// @pymethod <o PyHKEY>|pywintypes|HKEY|Creates a new HKEY object
PyObject *PyWinMethod_NewHKEY(PyObject *self, PyObject *args)
{
	HANDLE hInit;
	if (!PyArg_ParseTuple(args, "|i:HANDLERegistry", &hInit))
		return NULL;
	return new PyHKEY(hInit);
}

BOOL PyWinObject_CloseHKEY(PyObject *obHandle)
{
	BOOL ok;
	if (PyHANDLE_Check(obHandle))
		// Python error already set.
		ok = ((PyHKEY *)obHandle)->Close();
	else if PyInt_Check(obHandle) {
		long rc = ::RegCloseKey((HKEY)PyInt_AsLong(obHandle));
		ok = (rc==ERROR_SUCCESS);
		if (!ok)
			PyWin_SetAPIError("RegCloseKey", rc);
	} else {
		PyErr_SetString(PyExc_TypeError, "A handle must be a HKEY object or an integer");
		return FALSE;
	}
	return ok;
}

// The non-static member functions
BOOL PyHKEY::Close(void)
{
	LONG rc = m_handle ? RegCloseKey((HKEY)m_handle) : ERROR_SUCCESS;
	m_handle = 0;
	if (rc!= ERROR_SUCCESS)
		PyWin_SetAPIError("RegCloseKey", rc);
	return rc==ERROR_SUCCESS;
}
