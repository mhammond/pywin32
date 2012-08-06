// PyIMoniker

// @doc
#include "stdafx.h"
#include "PythonCOM.h"
#include "PyIPersist.h"
#include "PyIPersistStream.h"
#include "PyIMoniker.h"
#include "PyIBindCtx.h"

PyIEnumMoniker::PyIEnumMoniker(IUnknown *pDisp) :
	PyIUnknown(pDisp)
{
	ob_type = &type;
}

PyIEnumMoniker::~PyIEnumMoniker()
{
}

/*static*/ IEnumMoniker *PyIEnumMoniker::GetI(PyObject *self)
{
	return (IEnumMoniker *)PyIUnknown::GetI(self);
}


// @pymethod <o PyIMoniker>|PyIEnumMoniker|Next|Retrieves a specified number of items in the enumeration sequence.
PyObject *PyIEnumMoniker::Next(PyObject *self, PyObject *args)
{
	long celt = 1;
	// @pyparm int|num|1|Number of items to retrieve.
	if ( !PyArg_ParseTuple(args, "|l:Next", &celt) )
		return NULL;

	IEnumMoniker *pMy = GetI(self);
	if (pMy==NULL) return NULL;

	IMoniker **rgVar = new IMoniker *[celt];
	if ( rgVar == NULL ) {
		PyErr_SetString(PyExc_MemoryError, "allocating result IMoniker *s");
		return NULL;
	}

	int i;
	ULONG celtFetched;
	PY_INTERFACE_PRECALL;
	HRESULT hr = pMy->Next(celt, rgVar, &celtFetched);
	PY_INTERFACE_POSTCALL;
	if ( FAILED(hr) )
	{
		delete [] rgVar;
		return PyCom_BuildPyException(hr);
	}

	PyObject *result = PyTuple_New(celtFetched);
	if (result) {
		for ( i = celtFetched; i--; )
		{
			PyObject *ob = PyCom_PyObjectFromIUnknown(rgVar[i], IID_IMoniker, FALSE);
			rgVar[i] = NULL;
			if ( ob == NULL ) {
				Py_DECREF(result);
				result = NULL;
				break;
			}
			PyTuple_SET_ITEM(result, i, ob);
		}
	}
	for ( i = celtFetched; i--; ) PYCOM_RELEASE(rgVar[i]);
	delete [] rgVar;
	return result;
	// @rdesc The result is a tuple of <o PyIID> objects, 
	// one for each element returned.  Note that if zero elements are returned, it is not considered
	// an error condition - an empty tuple is simply returned.
}
// @pymethod |PyIEnumMoniker|Skip|Skips over the next specified elementes.
PyObject *PyIEnumMoniker::Skip(PyObject *self, PyObject *args)
{
	ULONG num;
	// @pyparm int|num||The number of elements being requested.
	if (!PyArg_ParseTuple(args, "l:Skip", &num))
		return NULL;

	IEnumMoniker *pMy = GetI(self);
	if (pMy==NULL) return NULL;

	PY_INTERFACE_PRECALL;
	HRESULT hr = pMy->Skip(num);
	PY_INTERFACE_POSTCALL;
	if (FAILED(hr))
		return PyCom_BuildPyException(hr);
	Py_INCREF(Py_None);
	return Py_None;
}
// @pymethod |PyIEnumMoniker|Reset|Resets the enumeration sequence to the beginning.
PyObject *PyIEnumMoniker::Reset(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ":Reset"))
		return NULL;

	IEnumMoniker *pMy = GetI(self);
	if (pMy==NULL) return NULL;

	PY_INTERFACE_PRECALL;
	HRESULT hr = pMy->Reset();
	PY_INTERFACE_POSTCALL;
	if (FAILED(hr))
		return PyCom_BuildPyException(hr);
	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod <o PyIEnumMoniker>|PyIEnumMoniker|Clone|Creates another enumerator that contains the same enumeration state as the current one
PyObject *PyIEnumMoniker::Clone(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ":Clone"))
		return NULL;

	IEnumMoniker *pMy = GetI(self);
	if (pMy==NULL) return NULL;

	IEnumMoniker *pNew = NULL;
	PY_INTERFACE_PRECALL;
	HRESULT hr = pMy->Clone(&pNew);
	PY_INTERFACE_POSTCALL;
	if (FAILED(hr))
		return PyCom_BuildPyException(hr);
	return PyCom_PyObjectFromIUnknown(pNew, IID_IEnumMoniker, FALSE);
}

// @object PyIEnumMoniker|A Python interface to IEnumMoniker
static struct PyMethodDef PyIEnumMoniker_methods[] =
{
	{"Next",          PyIEnumMoniker::Next,  1}, // @pymeth Next|Retrieves a specified number of items in the enumeration sequence.
	{"Skip",          PyIEnumMoniker::Skip,  1}, // @pymeth Skip|Skips over the next specified elementes.
	{"Reset",          PyIEnumMoniker::Reset,  1}, // @pymeth Reset|Resets the enumeration sequence to the beginning.
	{"Clone",          PyIEnumMoniker::Clone,  1}, // @pymeth Clone|Creates another enumerator that contains the same enumeration state as the current one.
	{NULL,  NULL}        
};

PyComEnumTypeObject PyIEnumMoniker::type("PyIEnumMoniker",
                 &PyIUnknown::type, // @base PyIEnumMoniker|PyIUnknown
                 sizeof(PyIEnumMoniker),
                 PyIEnumMoniker_methods,
				 GET_PYCOM_CTOR(PyIEnumMoniker));

//////////////////////////////////////////////////////////////////////////////////////
PyIMoniker::PyIMoniker(IUnknown *pDisp) :
	PyIPersistStream(pDisp)
{
	ob_type = &type;
}

PyIMoniker::~PyIMoniker()
{
}

/*static*/ IMoniker *PyIMoniker::GetI(PyObject *self)
{
	return (IMoniker *)PyIPersistStream::GetI(self);
}

// @pymethod <o PyIUnknown>|PyIMoniker|BindToObject|Uses the moniker to bind to the object it identifies.
PyObject *PyIMoniker::BindToObject(PyObject *self, PyObject *args)
{
	// @pyparm <o PyIBindCtx>|bindCtx||bind context object to be used.
	// @pyparm <o PyIMoniker>|moniker||If the moniker is part of a composite moniker, otherwise None
	// @pyparm <o IID>|iidResult||IID of the result object.
	PyObject *obBindCtx;
	PyObject *obMoniker;
	PyObject *obIID;

	if (!PyArg_ParseTuple(args, "OOO:BindToObject", &obBindCtx, &obMoniker, &obIID))
		return NULL;

	IMoniker *pMy = GetI(self);
	if (pMy==NULL) return NULL;

	IBindCtx *pBindCtx;
	if (!PyCom_InterfaceFromPyInstanceOrObject(obBindCtx, IID_IBindCtx, (void **)&pBindCtx, FALSE))
		return NULL;
	
	IMoniker *pMonLeft = NULL;
	if (obMoniker!=Py_None) {
		if (!PyCom_InterfaceFromPyInstanceOrObject(obMoniker, IID_IMoniker, (void **)&pMonLeft, FALSE)) {
			PYCOM_RELEASE(pBindCtx);
			return NULL;
		}
	}
	IID iid;
	if (!PyWinObject_AsIID(obIID, &iid)) {
		PY_INTERFACE_PRECALL;
		pBindCtx->Release();
		if (pMonLeft) pMonLeft->Release();
		PY_INTERFACE_POSTCALL;
		return NULL;
	}

	void *pResult = NULL;
	PY_INTERFACE_PRECALL;
	HRESULT hr = pMy->BindToObject(pBindCtx, pMonLeft, iid, &pResult );
	pBindCtx->Release();
	if (pMonLeft) pMonLeft->Release();
	PY_INTERFACE_POSTCALL;

	if (S_OK!=hr) // S_OK only acceptable
		return PyCom_BuildPyException(hr, pMy, IID_IMoniker);
	return PyCom_PyObjectFromIUnknown((IUnknown *)pResult, iid, FALSE );
}

// @pymethod <o PyIUnknown>|PyIMoniker|BindToStorage|Retrieves an interface object to the storage that contains the object identified by the moniker.
PyObject *PyIMoniker::BindToStorage(PyObject *self, PyObject *args)
{
	// @pyparm <o PyIBindCtx>|bindCtx||bind context object to be used.
	// @pyparm <o PyIMoniker>|moniker||If the moniker is part of a composite moniker, otherwise None
	// @pyparm <o IID>|iidResult||IID of the result object.
	PyObject *obBindCtx;
	PyObject *obMoniker;
	PyObject *obIID;

	if (!PyArg_ParseTuple(args, "OOO:BindToStorage", &obBindCtx, &obMoniker, &obIID))
		return NULL;

	IMoniker *pMy = GetI(self);
	if (pMy==NULL) return NULL;

	IBindCtx *pBindCtx;
	if (!PyCom_InterfaceFromPyInstanceOrObject(obBindCtx, IID_IBindCtx, (void **)&pBindCtx, FALSE))
		return NULL;
	
	IMoniker *pMonLeft = NULL;
	if (obMoniker!=Py_None) {
		if (!PyCom_InterfaceFromPyInstanceOrObject(obMoniker, IID_IMoniker, (void **)&pMonLeft, FALSE)) {
			PYCOM_RELEASE(pBindCtx);
			return NULL;
		}
	}
	IID iid;
	if (!PyWinObject_AsIID(obIID, &iid)) {
		PY_INTERFACE_PRECALL;
		pBindCtx->Release();
		if (pMonLeft) pMonLeft->Release();
		PY_INTERFACE_POSTCALL;

		return NULL;
	}

	void *pResult = NULL;
	PY_INTERFACE_PRECALL;
	HRESULT hr = pMy->BindToStorage(pBindCtx, pMonLeft, iid, &pResult );
	pBindCtx->Release();
	if (pMonLeft) pMonLeft->Release();
	PY_INTERFACE_POSTCALL;

	if (S_OK!=hr) // S_OK only acceptable
		return PyCom_BuildPyException(hr, pMy, IID_IMoniker);
	return PyCom_PyObjectFromIUnknown((IUnknown *)pResult, iid, FALSE );
}

// @pymethod string|PyIMoniker|GetDisplayName|Gets the display name , which is a user-readable representation of this moniker.
PyObject *PyIMoniker::GetDisplayName(PyObject *self, PyObject *args)
{
	// @pyparm <o PyIBindCtx>|bindCtx||bind context object to be used.
	// @pyparm <o PyIMoniker>|moniker||If the moniker is part of a composite moniker, otherwise None
	PyObject *obBindCtx;
	PyObject *obMoniker;

	if (!PyArg_ParseTuple(args, "OO:GetDisplayName", &obBindCtx, &obMoniker))
		return NULL;

	IMoniker *pMy = GetI(self);
	if (pMy==NULL) return NULL;

	IBindCtx *pBindCtx;
	if (!PyCom_InterfaceFromPyInstanceOrObject(obBindCtx, IID_IBindCtx, (void **)&pBindCtx, FALSE))
		return NULL;
	
	IMoniker *pMonLeft = NULL;
	if (obMoniker!=Py_None) {
		if (!PyCom_InterfaceFromPyInstanceOrObject(obMoniker, IID_IMoniker, (void **)&pMonLeft, FALSE)) {
			PYCOM_RELEASE(pBindCtx);
			return NULL;
		}
	}
	LPOLESTR result;
	PY_INTERFACE_PRECALL;
	HRESULT hr = pMy->GetDisplayName(pBindCtx, pMonLeft, &result );
	pBindCtx->Release();
	if (pMonLeft) pMonLeft->Release();
	PY_INTERFACE_POSTCALL;
	if (S_OK!=hr) // S_OK only acceptable
		return PyCom_BuildPyException(hr, pMy, IID_IMoniker);
	PyObject *obResult = PyWinObject_FromWCHAR(result);
	CoTaskMemFree(result);
	return obResult;
}


// @pymethod <o PyIMoniker>|PyIMoniker|ComposeWith|Combines the current moniker with another moniker, creating a new composite moniker.
PyObject *PyIMoniker::ComposeWith(PyObject *self, PyObject *args)
{
	// @pyparm <o PyIMoniker>|mkRight||The IMoniker interface on the moniker to compose onto the end of this moniker.
	// @pyparm int|fOnlyIfNotGeneric||If TRUE, the caller requires a non-generic composition, so the operation should proceed only if pmkRight is a moniker class that this moniker can compose with in some way other than forming a generic composite. If FALSE, the method can create a generic composite if necessary.
	PyObject *obmkRight;
	int bOnlyIfNotGeneric;
	if (!PyArg_ParseTuple(args, "Oi:ComposeWith", &obmkRight, &bOnlyIfNotGeneric))
		return NULL;

	IMoniker *pMy = GetI(self);
	if (pMy==NULL) return NULL;

	IMoniker *pmkRight;
	if (!PyCom_InterfaceFromPyInstanceOrObject(obmkRight, IID_IMoniker, (void **)&pmkRight, FALSE))
		return NULL;

	IMoniker *pResult = NULL;

	PY_INTERFACE_PRECALL;
	HRESULT hr = pMy->ComposeWith(pmkRight, bOnlyIfNotGeneric, &pResult);
	pmkRight->Release();
	PY_INTERFACE_POSTCALL;
	if (S_OK!=hr) // S_OK only acceptable
		return PyCom_BuildPyException(hr, pMy, IID_IMoniker);
	return PyCom_PyObjectFromIUnknown(pResult, IID_IMoniker, FALSE );
}

// @pymethod <o PyIEnumMoniker>|PyIMoniker|Enum|Supplies an enumerator that can enumerate the components of a composite moniker.
PyObject *PyIMoniker::Enum(PyObject *self, PyObject *args)
{
	// @pyparm int|fForward|True|If TRUE, enumerates the monikers from left to right. If FALSE, enumerates from right to left.
	int fForward = TRUE;
	if (!PyArg_ParseTuple(args, "|i:Enum", &fForward))
		return NULL;

	IMoniker *pMy = GetI(self);
	if (pMy==NULL) return NULL;

	IEnumMoniker *pResult = NULL;
	HRESULT hr = pMy->Enum(fForward, &pResult);
	if (S_OK!=hr)
		return PyCom_BuildPyException(hr, pMy, IID_IMoniker);
	return PyCom_PyObjectFromIUnknown(pResult, IID_IEnumMoniker, FALSE );
}

// @pymethod int|PyIMoniker|IsEqual|Compares this moniker with a specified moniker and indicates whether they are identical. 
PyObject *PyIMoniker::IsEqual(PyObject *self, PyObject *args)
{
	PyObject *obOther;
	// @pyparm <o PyIMoniker>|other||The moniker to compare
	if (!PyArg_ParseTuple(args, "O:IsEqual", &obOther))
		return NULL;

	IMoniker *pMy = GetI(self);
	if (pMy==NULL) return NULL;

	IMoniker *pOther;
	if (!PyCom_InterfaceFromPyInstanceOrObject(obOther, IID_IMoniker, (void **)&pOther, FALSE))
		return NULL;

	PY_INTERFACE_PRECALL;
	HRESULT hr = pMy->IsEqual(pOther);
	pOther->Release();
	PY_INTERFACE_POSTCALL;
	if (FAILED(hr))
		return PyCom_BuildPyException(hr, pMy, IID_IMoniker);
	return PyInt_FromLong(hr);
}

// @pymethod int|PyIMoniker|IsSystemMoniker|Indicates whether this moniker is of one of the system-supplied moniker classes.
PyObject *PyIMoniker::IsSystemMoniker(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ":IsSystemMoniker"))
		return NULL;

	IMoniker *pMy = GetI(self);
	if (pMy==NULL) return NULL;

	DWORD mksys;
	PY_INTERFACE_PRECALL;
	HRESULT hr = pMy->IsSystemMoniker(&mksys);
	PY_INTERFACE_POSTCALL;
	if (FAILED(hr))
		return PyCom_BuildPyException(hr, pMy, IID_IMoniker);
	return PyInt_FromLong(mksys);
}

// @pymethod int|PyIMoniker|Hash|Calculates a 32-bit integer using the internal state of the moniker. 
PyObject *PyIMoniker::Hash(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ":Hash" ))
		return NULL;

	IMoniker *pMy = GetI(self);
	if (pMy==NULL) return NULL;
	DWORD result;
	PY_INTERFACE_PRECALL;
	HRESULT hr = pMy->Hash(&result);
	PY_INTERFACE_POSTCALL;
	if (FAILED(hr))
		return PyCom_BuildPyException(hr, pMy, IID_IMoniker);
	return PyInt_FromLong(result);
}

// @object PyIMoniker|A Python interface to IMoniker
static struct PyMethodDef PyIMoniker_methods[] =
{
	{"BindToObject",   PyIMoniker::BindToObject,  1}, // @pymeth BindToObject|Uses the moniker to bind to the object it identifies.
	{"BindToStorage",   PyIMoniker::BindToStorage,  1}, // @pymeth BindToStorage|Retrieves an interface object to the storage that contains the object identified by the moniker.
	{"GetDisplayName",    PyIMoniker::GetDisplayName,  1}, // @pymeth GetDisplayName|Gets the display name , which is a user-readable representation of this moniker.
	{"ComposeWith",    PyIMoniker::ComposeWith,  1}, // @pymeth ComposeWith|Combines the current moniker with another moniker, creating a new composite moniker.
	{"Enum",           PyIMoniker::Enum,  1}, // @pymeth Enum|Supplies an enumerator that can enumerate the components of a composite moniker.
	{"IsEqual",        PyIMoniker::IsEqual,  1}, // @pymeth IsEqual|Compares this moniker with a specified moniker and indicates whether they are identical. 
	{"IsSystemMoniker",PyIMoniker::IsSystemMoniker,  1}, // @pymeth IsSystemMoniker|Indicates whether this moniker is of one of the system-supplied moniker classes.
	{"Hash",        PyIMoniker::Hash,  1}, // @pymeth Hash|Calculates a 32-bit integer using the internal state of the moniker. 
	{NULL,  NULL}        
};

PyComEnumProviderTypeObject PyIMoniker::type("PyIMoniker",
                 &PyIPersistStream::type,  // @base PyIMoniker|PyIPersistStream
                 sizeof(PyIMoniker),
                 PyIMoniker_methods,
                 GET_PYCOM_CTOR(PyIMoniker),
                 "Enum");
