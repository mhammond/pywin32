//
// Data conversion
//

#include "stdafx.h"
#include "PythonCOM.h"
#include "PythonCOMServer.h"
#include "univgw_dataconv.h"

PyObject * dataconv_L64(PyObject *self, PyObject *args)
{
	void *pSrc;
	int size;

	if ( !PyArg_ParseTuple(args, "s#:L64", &pSrc, &size) )
		return NULL;
	if ( size != 8 )
	{
		PyErr_SetString(PyExc_ValueError, "argument must be 8 characters");
		return NULL;
	}

	return PyLong_FromLongLong(*(__int64 *)pSrc);
}

PyObject * dataconv_UL64(PyObject *self, PyObject *args)
{
	void *pSrc;
	int size;

	if ( !PyArg_ParseTuple(args, "s#:UL64", &pSrc, &size) )
		return NULL;
	if ( size != 8 )
	{
		PyErr_SetString(PyExc_ValueError, "argument must be 8 characters");
		return NULL;
	}

	return PyLong_FromUnsignedLongLong(*(unsigned __int64 *)pSrc);
}

PyObject * dataconv_strL64(PyObject *self, PyObject *args)
{
	__int64 val;

	if ( !PyArg_ParseTuple(args, "L:strL64", &val) )
		return NULL;

	return PyString_FromStringAndSize((char *)&val, sizeof(val));
}

PyObject * dataconv_strUL64(PyObject *self, PyObject *args)
{
	PyObject *ob;

	if ( !PyArg_ParseTuple(args, "O!:strUL64", &PyLong_Type, &ob) )
		return NULL;

	unsigned __int64 val = PyLong_AsUnsignedLongLong(ob);

	return PyString_FromStringAndSize((char *)&val, sizeof(val));
}

PyObject * dataconv_interface(PyObject *self, PyObject *args)
{
	PyObject *obPtr;
	PyObject *obIID;

	if ( !PyArg_ParseTuple(args, "OO:interface", &obPtr, &obIID) )
		return NULL;

	// determine the Py2COM thunk to wrap around the punk
	IID iid;
	if ( !PyWinObject_AsIID(obIID, &iid) )
		return NULL;

	IUnknown *punk = (IUnknown *)PyLong_AsVoidPtr(obPtr);
	if ( punk == NULL && PyErr_Occurred() )
		return NULL;

	// make sure to add a ref, which will be released with this object
	return PyCom_PyObjectFromIUnknown(punk, iid, TRUE);
}


static inline bool SizeOfVT(VARTYPE vt, int *pitem_size, int *pstack_size)
{
	int item_size;
	if (vt & VT_BYREF)
	{
		item_size = sizeof(void *);
	} else {

		switch (vt & VT_TYPEMASK)
		{
		case VT_BSTR:
		case VT_UNKNOWN:
		case VT_DISPATCH:
		case VT_ARRAY:
		case VT_PTR:
		case VT_LPSTR:
		case VT_LPWSTR:
		case VT_CARRAY:
		case VT_RECORD:
			item_size = sizeof(void *);
			break;
		case VT_I4:
		case VT_UI4:
			item_size = 4;
			break;
		case VT_INT:
		case VT_UINT:
			item_size = sizeof(INT);
			break;
		case VT_R4:
			item_size = 4;
			break;
		case VT_DATE:
			item_size = sizeof(DATE);
			break;
		case VT_R8:
			item_size = 8;
			break;
		case VT_CY:
			item_size = sizeof(CY);
			break;
		case VT_ERROR:
			item_size = sizeof(SCODE);
			break;
		case VT_BOOL:
			item_size = sizeof(VARIANT_BOOL);
			break;
		case VT_VARIANT:
			item_size = sizeof(VARIANT);
			break;
		case VT_I1:
		case VT_UI1:
			item_size = sizeof(char);
			break;
		case VT_I2:
		case VT_UI2:
			item_size = 2;
			break;
		case VT_I8:
		case VT_UI8:
			item_size = 8;
			break;
		case VT_HRESULT:
			item_size = sizeof(HRESULT);
			break;
		default:
			assert(FALSE);
			PyErr_SetString(PyExc_NotImplementedError, "unknown variant type");
			return FALSE;
		}
	}
#ifdef _M_IX86
	int stack_size = (item_size < 4) ? 4 : item_size;
#elif _M_X64
	// params > 64bits passed by address, and only VT_VARIANT is > 64bits.
	assert ((item_size <= 8) || ((vt & VT_TYPEMASK) == VT_VARIANT));
	if (item_size > 8) item_size = 8;
	// stack always 64 bits.
	int stack_size = 8;
#else
#error Unknown platform
#endif
	if (pitem_size) *pitem_size = item_size;
	if (pstack_size) *pstack_size = stack_size;
	return TRUE;
}	

PyObject *dataconv_SizeOfVT(PyObject *self, PyObject *args)
{
	int vt;
	if (!PyArg_ParseTuple(args, "i:SizeOfVT", &vt))
		return NULL;

	int item_size = 0;
	int stack_size = 0;
	if (!SizeOfVT((VARTYPE)vt, &item_size, &stack_size))
		return NULL;
	if (item_size <= 0) {
		PyErr_Format(PyExc_ValueError, "The value %d (0x%x) is an invalid variant type", vt, vt);
		return NULL;
	}
	return Py_BuildValue("ii", item_size, stack_size);
}

#define VALID_BYREF_MISSING(obUse) (obUse==Py_None || obUse->ob_type == &PyOleEmptyType)


PyObject * dataconv_WriteFromOutTuple(PyObject *self, PyObject *args)
{
	PyObject *obArgTypes;
	PyObject *obArgType;
	PyObject *obRetValues;
	PyObject *obPtr;
	PyObject *obOutValue;
	VARTYPE vtArgType;
	BYTE *pbArgs;
	BYTE *pbArg;
	Py_ssize_t cArgs;
	UINT uiIndirectionLevel = 0;
	Py_ssize_t i;
	
	if (!PyArg_ParseTuple(args, "OOO:WriteFromOutTuple", &obRetValues, &obArgTypes, &obPtr))
		return NULL;

	pbArgs = (BYTE *)PyLong_AsVoidPtr(obPtr);
	assert(pbArgs);
	if (!pbArgs)
		return NULL;

	// Nothing to do, oh darn.
	if (obRetValues == Py_None || obArgTypes == Py_None)
	{
		Py_INCREF(Py_None);
		return Py_None;
	}

	if (!PyTuple_Check(obArgTypes))
	{
		PyErr_SetString(PyExc_TypeError, "OLE type description - expecting a tuple");
		return NULL;
	}

	cArgs = PyTuple_Size(obArgTypes);
	if (!PyTuple_Check(obRetValues) && (UINT)PyTuple_Size(obRetValues) != cArgs)
	{
		PyErr_Format(PyExc_TypeError, "Expecting a tuple of length %d or None.", cArgs);
		return NULL;
	}
	
	for(i = 0 ; i < cArgs; i++)
	{
		obArgType = PyTuple_GET_ITEM(PyTuple_GET_ITEM(obArgTypes, i), 0);
		vtArgType = (VARTYPE)PyInt_AS_LONG(obArgType);


		// The following types aren't supported:
		// SAFEARRAY *: This requires support for SAFEARRAYs as a
		//              Python extensions type so we can update the SAFEARRAY
		//              in place.
		// VT_LPWSTR:   This just hasn't been written yet.
		// VT_LPSTR:    This just hasn't been written yet.
		// VT_LPWSTR | VT_BYREF:
		// VT_LPSTR  | VT_BYREF:
		//              These can't be supported since we don't know the correct
		//              memory allocation policy.

		// Find the start of the argument.
		pbArg = pbArgs + PyInt_AS_LONG(PyTuple_GET_ITEM(PyTuple_GET_ITEM(obArgTypes, i), 1));
		obOutValue = PyTuple_GET_ITEM(obRetValues, i);
	
		if (vtArgType & VT_ARRAY)
		{
			VARENUM rawVT = (VARENUM)(vtArgType & VT_TYPEMASK);
			if (vtArgType & VT_BYREF)
			{
				SAFEARRAY **ppsa = *(SAFEARRAY ***)pbArg;
				SAFEARRAY *psa;
				if (!VALID_BYREF_MISSING(obOutValue))
				{
					if (!PyCom_SAFEARRAYFromPyObject(obOutValue, ppsa, rawVT))
					{
						goto Error;
					}
				}
				else
				{
					SAFEARRAYBOUND rgsabound[1];
					rgsabound[0].lLbound = 0;
					rgsabound[0].cElements = 1;
					psa = SafeArrayCreate(rawVT, 1, rgsabound);
					*ppsa = psa;
				}
			}
			else
			{
				// We can't convert this in place... Ack...
				PyErr_SetString(
					PyExc_TypeError,
					"Inplace SAFEARRAY mucking isn't allowed, doh!");
				goto Error;
				
				SAFEARRAY *psa = *(SAFEARRAY **)pbArg;
				// Here we're updating an existing SafeArray.
				// so we need to handle it very carefully....
				SafeArrayDestroy(psa);
				if (!PyCom_SAFEARRAYFromPyObject(obOutValue, &psa, rawVT))
					return NULL;
			}
		}
			
		// All done with safe array handling.

		PyObject *obUse = NULL;

		switch (vtArgType) {
		case VT_VARIANT | VT_BYREF:
		{
			VARIANT *pvar = *(VARIANT **)pbArg;
			VariantClear(pvar);
			if (!VALID_BYREF_MISSING(obOutValue)) {
				PyCom_VariantFromPyObject(obOutValue, pvar);
			}
			else
			{
				V_VT(pvar) = VT_NULL;
			}
			break;
		}
		case VT_BSTR:
		{
			// This is the normal "BSTR" case, we can't
			// allocate a new BSTR we have to back patch the one
			// thats already there...
			BSTR bstr = *(BSTR *)pbArg;
			BSTR bstrT;
			UINT cch = SysStringLen(bstr);
			if ( PyString_Check(obOutValue) || PyUnicode_Check(obOutValue) )
			{
				if ( !PyWinObject_AsBstr(obOutValue, &bstrT) )
				{
					goto Error;
				}
			}
			else
			{
				// Use str(object) instead!
				obUse = PyObject_Str(obOutValue);
				if (obUse == NULL)
				{
					goto Error;
				}
				if ( !PyWinObject_AsBstr(obUse, &bstrT) )
				{
					goto Error;
				}
			}
			
			if (SysStringLen(bstrT) > cch)
			{
				PyErr_Format(
					PyExc_ValueError,
					"Return value[%d] with type BSTR was "
					"longer than the input value: %d",
					i,
					cch);
				goto Error;
			}
				
			// Ok, now we know theres enough room in the source BSTR to
			// modify the sucker in place.
			wcscpy(bstr, bstrT);

			// Free the temp BSTR.
			SysFreeString(bstrT);
			break;
		}
		case VT_BSTR | VT_BYREF:
		{
			BSTR *pbstr = *(BSTR **)pbArg;
			BSTR bstrT = NULL;
			SysFreeString(*pbstr);

			*pbstr = NULL;
			
			if ( PyString_Check(obOutValue) || PyUnicode_Check(obOutValue) )
			{
				if ( !PyWinObject_AsBstr(obOutValue, &bstrT) )
				{
					goto Error;
				}
			}
			else
			{
				// Use str(object) instead!
				obUse = PyObject_Str(obOutValue);
				if (obUse == NULL)
				{
					goto Error;
				}
				if (!PyWinObject_AsBstr(obUse, &bstrT) )
				{
					goto Error;
				}
			}
			*pbstr = bstrT;
			break;
		}
		case VT_ERROR | VT_BYREF:
		case VT_HRESULT | VT_BYREF:
		case VT_I4 | VT_BYREF:
		{
			INT *pi = *(INT **)pbArg;
			obUse = PyNumber_Int(obOutValue);
			if (obUse == NULL)
			{
				goto Error;
			}
			*pi = PyInt_AsLong(obUse);
			if (*pi == (UINT)-1 && PyErr_Occurred())
				goto Error;
			break;
		}
		case VT_UI4 | VT_BYREF:
		{
			UINT *pui = *(UINT **)pbArg;
			// special care here as we could be > sys.maxint,
			// in which case we must work with longs.
			// Avoiding PyInt_AsUnsignedLongMask as it doesn't
			// exist in 2.2.
			if (PyLong_Check(obOutValue)) {
				*pui = PyLong_AsUnsignedLong(obOutValue);
			} else {
				// just do the generic "number" thing.
				obUse = PyNumber_Int(obOutValue);
				if (obUse == NULL)
				{
					goto Error;
				}
				*pui = (UINT)PyInt_AsLong(obUse);
			}
			if (*pui == (UINT)-1 && PyErr_Occurred())
				goto Error;
			break;
		}
		case VT_I2 | VT_BYREF:
		{
			short *ps = *(short **)pbArg;
			obUse = PyNumber_Int(obOutValue);
			if (obUse == NULL)
			{
				goto Error;
			}
			*ps = (short)PyInt_AsLong(obUse);
			if (*ps == (UINT)-1 && PyErr_Occurred())
				goto Error;
			break;
		}
		case VT_UI2 | VT_BYREF:
		{
			unsigned short *pus = *(unsigned short **)pbArg;
			obUse = PyNumber_Int(obOutValue);
			if (obUse == NULL)
			{
				goto Error;
			}
			*pus = (unsigned short)PyInt_AsLong(obUse);
			if (*pus == (UINT)-1 && PyErr_Occurred())
				goto Error;
			break;
		}
		case VT_I1 | VT_BYREF:
		{
			signed char *pb = *(signed char **)pbArg;
			obUse = PyNumber_Int(obOutValue);
			if (obUse == NULL)
			{
				goto Error;
			}
			*pb = (signed char)PyInt_AsLong(obUse);
			if (*pb == (UINT)-1 && PyErr_Occurred())
				goto Error;
			break;
		}
		case VT_UI1 | VT_BYREF:
		{
			BYTE *pb = *(BYTE **)pbArg;
			BYTE *pbOutBuffer = NULL;
			if (PyString_Check(obOutValue))
			{
				pbOutBuffer = (BYTE *)PyString_AS_STRING(obOutValue);
				Py_ssize_t cb = PyString_GET_SIZE(obOutValue);
				memcpy(pb, pbOutBuffer, cb);
			}
			// keep this after string check since string can act as buffers
			else if (obOutValue->ob_type->tp_as_buffer)
			{
				DWORD cb;
				if (!PyWinObject_AsReadBuffer(obOutValue, (void **)&pbOutBuffer, &cb))
					goto Error;
				memcpy(pb, pbOutBuffer, cb);
			}
			else
			{
				obUse = PyNumber_Int(obOutValue);
				if (obUse == NULL)
				{
					goto Error;
				}
				*pb = (BYTE)PyInt_AsLong(obUse);
				if (*pb == (UINT)-1 && PyErr_Occurred())
					goto Error;
			}
			break;
		}
		case VT_BOOL | VT_BYREF:
		{
			VARIANT_BOOL *pbool = *(VARIANT_BOOL **)pbArg;
			obUse = PyNumber_Int(obOutValue);
			if (obUse == NULL)
			{
				goto Error;
			}
			*pbool = PyInt_AsLong(obUse) ? VARIANT_TRUE : VARIANT_FALSE;
			if (*pbool == (UINT)-1 && PyErr_Occurred())
				goto Error;
			break;
		}
		case VT_R8 | VT_BYREF:
		{
			double *pdbl = *(double **)pbArg;
			obUse = PyNumber_Float(obOutValue);
			if (obUse == NULL)
			{
				goto Error;
			}
			*pdbl = PyFloat_AsDouble(obUse);
			break;
		}
		case VT_R4 | VT_BYREF:
		{
			float *pfloat = *(float **)pbArg;
			obUse = PyNumber_Float(obOutValue);
			if (obUse == NULL)
			{
				goto Error;
			}
			*pfloat = (float)PyFloat_AsDouble(obUse);
			break;
		}
		case VT_DISPATCH | VT_BYREF:
		{
			PyObject *obIID = PyTuple_GET_ITEM(PyTuple_GET_ITEM(obArgTypes, i), 3);
			IID iid = IID_IDispatch;
			if (obIID != NULL && obIID!=Py_None)
				PyWinObject_AsIID(obIID, &iid);
			IDispatch **pdisp = *(IDispatch ***)pbArg;
			if (!PyCom_InterfaceFromPyInstanceOrObject(
				obOutValue,
				iid,
				(void **)pdisp,
				TRUE))
			{
				goto Error;
			}
			// COM Reference added by InterfaceFrom...
			break;
		}
		case VT_UNKNOWN | VT_BYREF:
		{
			PyObject *obIID = PyTuple_GET_ITEM(PyTuple_GET_ITEM(obArgTypes, i), 3);
			IID iid = IID_IUnknown;
			if (obIID != NULL && obIID!=Py_None)
				PyWinObject_AsIID(obIID, &iid);
			IUnknown **punk = *(IUnknown ***)pbArg;
			if (!PyCom_InterfaceFromPyInstanceOrObject(
				obOutValue,
				iid,
				(void **)punk, TRUE))
			{
				goto Error;
			}
			// COM Reference added by InterfaceFrom...
			break;
		}
		case VT_DATE | VT_BYREF:
		{
			DATE *pdbl = *(DATE **)pbArg;
			if ( !PyWinObject_AsDATE(obOutValue, pdbl) )
			{
				goto Error;
			}
			break;
		}
		case VT_CY | VT_BYREF:
		{
			CY *pcy = *(CY **)pbArg;
			if (!PyObject_AsCurrency(obOutValue, pcy))
				goto Error;
			break;
		}
		case VT_I8 | VT_BYREF:
		{
			LARGE_INTEGER *pi64 = *(LARGE_INTEGER **)pbArg;
			if (!PyWinObject_AsLARGE_INTEGER(obOutValue, pi64))
			{
				goto Error;
			}
			break;
		}
		case VT_UI8 | VT_BYREF:
		{
			ULARGE_INTEGER *pui64 = *(ULARGE_INTEGER **)pbArg;
			if (!PyWinObject_AsULARGE_INTEGER(obOutValue, pui64))
			{
				goto Error;
			}
			break;
		}
		default:
			// could try default, but this error indicates we need to
			// beef up the VARIANT support, rather than default.
			PyErr_Format(PyExc_TypeError, "The VARIANT type is unknown (0x%x).",
			             vtArgType);
			goto Error;
		}
		
		Py_XDECREF(obUse);
	}

	Py_INCREF(Py_None);
	return Py_None;
Error:
	return NULL;
}


PyObject * dataconv_ReadFromInTuple(PyObject *self, PyObject *args)
{
	PyObject *obArgTypes;
	PyObject *obArgType;
	PyObject *obPtr;
	BYTE *pb;
	BYTE *pbArg;
	Py_ssize_t cArgs, i;
	PyObject *obArgs = NULL;
	PyObject *obArg;
	VARTYPE vtArgType;
	UINT cb;
	VARIANT var;
	BOOL bIsByRef;
	

	if (!PyArg_ParseTuple(args, "OO:ReadFromInTuple", &obArgTypes, &obPtr))
		return NULL;
	
	pbArg = (BYTE *)PyLong_AsVoidPtr(obPtr);
	assert(pbArg);
	if (!pbArg)
		return NULL;

	pb = pbArg;

	if (!PyTuple_Check(obArgTypes))
	{
		PyErr_SetString(PyExc_TypeError, "OLE type description - expecting a tuple");
		return NULL;
	}
	
	cArgs = PyTuple_Size(obArgTypes);
	obArgs = PyTuple_New(cArgs);
	if (!obArgs)
		return NULL;

	for(i = 0 ; i < cArgs; i++)
	{
		// (<type tuple>, argPtr offset, arg size)
		if (PyTuple_Size(PyTuple_GET_ITEM(obArgTypes, i)) != 3)
		{
			PyErr_SetString(PyExc_TypeError, "OLE type description - expecting an arg desc tuple of size 3");
			goto Error;
		}
		
		obArgType = PyTuple_GET_ITEM(PyTuple_GET_ITEM(obArgTypes, i), 0);

		// Position pb to point to the current argument.
		pb = pbArg + PyInt_AS_LONG(PyTuple_GET_ITEM(PyTuple_GET_ITEM(obArgTypes, i), 1));
		vtArgType = (VARTYPE)PyInt_AS_LONG(obArgType);
#ifdef _M_IX86
		bIsByRef = vtArgType & VT_BYREF;
#elif _M_X64
		// params > 64bits always passed by address - and the only
		// arg we support > 64 bits is a VARIANT structure.
		bIsByRef = (vtArgType==VT_VARIANT) || (vtArgType & VT_BYREF);
#else
#error Unknown platform
#endif
		VARTYPE vtConversionType = vtArgType & VT_TYPEMASK;
		if (vtArgType & VT_ARRAY) {
			SAFEARRAY FAR *psa = *((SAFEARRAY **)pb);
			if (psa==NULL) { // A NULL array
				Py_INCREF(Py_None);
				obArg = Py_None;
			} else {
				if (vtArgType & VT_BYREF) // one more level of indirection
					psa = *((SAFEARRAY FAR **)psa);
				if (psa==NULL) { // A NULL array
					Py_INCREF(Py_None);
					obArg = Py_None;
				} else 
					obArg = PyCom_PyObjectFromSAFEARRAY(psa, (VARENUM)vtConversionType);
			}
		} else {
			switch (vtConversionType)
			{
			// If they can fit in a VARIANT, cheat and make that code do all of the work...
			case VT_I2:
			case VT_I4:
			case VT_R4:
			case VT_R8:
			case VT_CY:
			case VT_DATE:
			case VT_BSTR:
			case VT_ERROR:
			case VT_BOOL:
			case VT_I1:
			case VT_UI1:
			case VT_UI2:
			case VT_UI4:
			case VT_INT:
			case VT_UINT:
			case VT_UNKNOWN:
			case VT_DISPATCH:
			case VT_HRESULT:
				VariantInit(&var);
				if (vtConversionType == VT_HRESULT ||
					vtConversionType == VT_INT)
				{
					// Preserve VT_BYREF or VT_ARRAY
					vtArgType = VT_I4 | (vtArgType & VT_TYPEMASK);
				}
				if (vtArgType == VT_UINT) 
				{
					// Preserve VT_BYREF or VT_ARRAY
					vtArgType = VT_UI4 | (vtArgType & VT_TYPEMASK);
				}
				V_VT(&var) = vtArgType;
				// Copy the data into the variant...
				if (!SizeOfVT(V_VT(&var), (int *)&cb, NULL))
					goto Error;
				memcpy(&V_I4(&var), pb, cb);
				// Convert it into a PyObject:
				obArg = PyCom_PyObjectFromVariant(&var);
				break;
			case VT_VARIANT:
				// A _real_ variant.
				if (bIsByRef)
					obArg = PyCom_PyObjectFromVariant(*(VARIANT**)pb);
				else
					obArg = PyCom_PyObjectFromVariant((VARIANT*)pb);
				break;
			case VT_LPSTR:
				obArg = PyString_FromString(*(CHAR **)pb);
				break;
			case VT_LPWSTR:
				obArg = PyWinObject_FromOLECHAR(*(OLECHAR **)pb);
				break;
			// Special cases:
			case VT_UI8:
				if (bIsByRef)
				{
					obArg = PyWinObject_FromULARGE_INTEGER(*(ULARGE_INTEGER *)pb);
				}
				else
				{
					obArg = PyWinObject_FromULARGE_INTEGER(**(ULARGE_INTEGER **)pb);
				}
				break;
			case VT_I8:
				if (bIsByRef)
				{
					obArg = PyWinObject_FromLARGE_INTEGER(*(LARGE_INTEGER *)pb);
				}
				else
				{
					obArg = PyWinObject_FromLARGE_INTEGER(**(LARGE_INTEGER **)pb);
				}
				break;
			// Pointers to unhandled arguments:
			// neither of these will be VT_BYREF'd.
			case VT_RECORD:
			case VT_PTR:
				obArg = PyLong_FromVoidPtr((void *)pb);
				break;
			// None of these should ever happen:
			case VT_USERDEFINED:
			// Should have been coerced into VT_PTR.
			case VT_CARRAY:
			default:
				obArg = NULL;
				PyErr_SetString(PyExc_TypeError, "Unknown/bad type description type!");
				// barf here, we don't wtf they were thinking...
				break;
			} // switch
		} // if ARRAY

		if (obArg == NULL)
		{
			goto Error;
		}
		PyTuple_SET_ITEM(obArgs, i, obArg);
	}

	return obArgs;
	
Error:
	Py_XDECREF(obArgs);
	return NULL;
}
