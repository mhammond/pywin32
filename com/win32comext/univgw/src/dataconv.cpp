//
// Data conversion
//

#include "stdafx.h"

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


static inline int SizeOfVT(VARTYPE vt)
{
	if (vt & VT_BYREF)
	{
		return sizeof(void *);
	}

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
		return sizeof(void *);
	case VT_I4:
	case VT_UI4:
		return 4;
	case VT_INT:
	case VT_UINT:
		return sizeof(INT);
	case VT_R4:
		return 4;
	case VT_DATE:
		return sizeof(DATE);
	case VT_R8:
		return 8;
	case VT_CY:
		return sizeof(CY);
	case VT_ERROR:
		return sizeof(SCODE);
	case VT_BOOL:
		return sizeof(VARIANT_BOOL);
	case VT_VARIANT:
		return sizeof(VARIANT);
	case VT_I1:
	case VT_UI1:
		return sizeof(char);
	case VT_I2:
	case VT_UI2:
		return 2;
	case VT_I8:
	case VT_UI8:
		return 8;
	case VT_HRESULT:
		return sizeof(HRESULT);
	default:
		_ASSERTE(FALSE);
		return 0;
	}
}	

PyObject *dataconv_SizeOfVT(PyObject *self, PyObject *args)
{
	PyObject *obVT;
	if (!PyArg_ParseTuple(args, "O:SizeOfVT", &obVT))
		return NULL;

	return PyInt_FromLong(SizeOfVT((VARTYPE)PyInt_AS_LONG(obVT)));
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
	UINT cArgs;
	UINT uiIndirectionLevel = 0;
	UINT i;
	VARTYPE vtConversionType = VT_EMPTY;
	
	if (!PyArg_ParseTuple(args, "OOO:WriteFromOutTuple", &obRetValues, &obArgTypes, &obPtr))
		return NULL;

	pbArgs = (BYTE *)PyLong_AsVoidPtr(obPtr);
	_ASSERTE(pbArgs);
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
		if (!PyTuple_Check(obArgType))
		{
			PyErr_SetString(PyExc_TypeError, "All out arguments must have at least one level of indirection!");
			goto Error;
		}
		uiIndirectionLevel = 0;
		while (PyTuple_Check(obArgType))
		{
			if (PyTuple_Size(obArgType) != 2)
			{
				PyErr_SetString(PyExc_TypeError, "OLE type description - expecting a sub-type tuple of size 2");
				goto Error;
			}
			vtArgType = (VARTYPE)PyInt_AS_LONG(PyTuple_GET_ITEM(obArgType, 0));
			switch (vtArgType)
			{
			case VT_PTR:
				uiIndirectionLevel++;
				break;
			case VT_CARRAY:
				// Force treatment as VT_PTR.
				uiIndirectionLevel += 2;
				break;
			case VT_SAFEARRAY:
				vtConversionType |= VT_ARRAY;
				break;
			default:
				PyErr_SetString(PyExc_TypeError, "COM type description - unknown indirection type.");
				goto Error;
			}
			   
			obArgType = PyTuple_GET_ITEM(obArgType, 1);
		}
		vtArgType = (VARTYPE)PyInt_AS_LONG(obArgType);

		// non-BSTR strings aren't supported at all...
		// only because we don't know how much memory we're allowed to overwrite...
		if (vtArgType & VT_TYPEMASK == VT_LPSTR ||
			vtArgType & VT_TYPEMASK == VT_LPWSTR)
		{
			uiIndirectionLevel +=2;
		}
		
		// If the indirection is > 1 we passed in a pointer to the argument
		// on the stack. They made any necessary changes, so skip it.
		//
		if (uiIndirectionLevel > 1)
		{
			continue;
		}
		
		// If the indirection level == 0 then we don't how to handle that either.
		// How do you handle an "[in, out] IDispatch *pdisp" sanely?
		// If we converted the arg for them, they're in trouble now,
		// since we can't handle this lame case.
		// Should I issue a warning when generating the type tuples for
		// cases like this?
		if (uiIndirectionLevel == 0)
		{
			// Throw an error only if we can convert the type and it
			// has a hidden indirection level.
			switch (vtConversionType)
			{
			case VT_UNKNOWN:
			case VT_DISPATCH:
				// What else could there be??
			default:
				_ASSERTE(FALSE);
				break;
			}
			// Skip the arg, they could have tweaked it themselves by now.
			continue;
		}

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

		vtConversionType |= vtArgType | VT_BYREF;

		// Find the start of the argument.
		pbArg = pbArgs + PyInt_AS_LONG(PyTuple_GET_ITEM(PyTuple_GET_ITEM(obArgTypes, i), 1));
		obOutValue = PyTuple_GET_ITEM(obRetValues, i);
	
		if (vtConversionType & VT_ARRAY)
		{
			VARENUM rawVT = (VARENUM)(vtConversionType & VT_TYPEMASK);
			if (vtConversionType & VT_BYREF)
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

		switch (vtConversionType) {
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
			break;
		}
		case VT_UI4 | VT_BYREF:
		{
			UINT *pui = *(UINT **)pbArg;
			obUse = PyNumber_Int(obOutValue);
			if (obUse == NULL)
			{
				goto Error;
			}
		    *pui = (UINT)PyInt_AsLong(obUse);
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
			break;
		}
		case VT_UI1 | VT_BYREF:
		{
			BYTE *pb = *(BYTE **)pbArg;
			BYTE *pbOutBuffer = NULL;
			UINT cb = 0;
			if (PyBuffer_Check(obOutValue))
			{
				cb = obOutValue->ob_type->tp_as_buffer->bf_getreadbuffer(obOutValue, 0, (void **)&pbOutBuffer);
				memcpy(pb, pbOutBuffer, cb);
			}
			else if (PyString_Check(obOutValue))
			{
				pbOutBuffer = (BYTE *)PyString_AS_STRING(obOutValue);
				cb = PyString_GET_SIZE(obOutValue);
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
			*pbool = (VARIANT_BOOL)PyInt_AsLong(obUse);
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
			IDispatch **pdisp = *(IDispatch ***)pbArg;
			if (!PyCom_InterfaceFromPyInstanceOrObject(
				obOutValue,
				IID_IDispatch,
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
			IUnknown **punk = *(IUnknown ***)pbArg;
			if (!PyCom_InterfaceFromPyInstanceOrObject(
				obOutValue,
				IID_IUnknown,
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
			if ( !PyTime_Check(obOutValue) )
			{
				goto Error;
			}
			if ( !PyWinObject_AsDATE(obOutValue, pdbl) )
			{
				goto Error;
			}
			break;
		}
		case VT_CY | VT_BYREF:
		{
			CY *pcy = *(CY **)pbArg;
			if (!PyTuple_Check(obOutValue) || PyTuple_Size(obOutValue) != 2 ||
				!PyLong_Check(PyTuple_GET_ITEM(obOutValue, 0)) ||
				!PyLong_Check(PyTuple_GET_ITEM(obOutValue, 1)))
			{
				PyErr_Format(
					PyExc_TypeError,
					"Return value[%d] is a VT_CY which requires a tuple "
					"containing two Python longs.", i);
				goto Error;
			}
			pcy->Hi = PyLong_AsLong(PyTuple_GET_ITEM(obOutValue, 0));
			pcy->Lo = PyLong_AsLong(PyTuple_GET_ITEM(obOutValue, 1));
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
			PyErr_SetString(PyExc_TypeError, "The VARIANT type is unknown.");
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
	UINT cArgs, i;
	PyObject *obArgs = NULL;
	PyObject *obArg;
	VARTYPE vtArgType;
	VARTYPE vtConversionType;
	UINT uiIndirectionLevel;
	UINT cb;
	VARIANT var;
	

	if (!PyArg_ParseTuple(args, "OO:ReadFromInTuple", &obArgTypes, &obPtr))
		return NULL;
	
	pbArg = (BYTE *)PyLong_AsVoidPtr(obPtr);
	_ASSERTE(pbArg);
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
			PyErr_SetString(PyExc_TypeError, "OLE type description - expecting a arg desc tuple of size 3");
			goto Error;
		}
		
		obArgType = PyTuple_GET_ITEM(PyTuple_GET_ITEM(obArgTypes, i), 0);

		// Position pb to point to the current argument.
		pb = pbArg + PyInt_AS_LONG(PyTuple_GET_ITEM(PyTuple_GET_ITEM(obArgTypes, i), 1));
		uiIndirectionLevel = 0;
		vtConversionType = VT_EMPTY;
		if (PyTuple_Check(obArgType))
		{
			// Handle any indirections...
			while (PyTuple_Check(obArgType)) {
				if (PyTuple_Size(obArgType)!=2)
				{
					PyErr_SetString(PyExc_TypeError, "OLE type description - expecting a sub-type tuple of size 2");
					goto Error;
				}
				vtArgType = (VARTYPE)PyInt_AS_LONG(PyTuple_GET_ITEM(obArgType, 0));
				switch (vtArgType)
				{
				case VT_PTR:
					uiIndirectionLevel++;
					break;
				case VT_CARRAY:
					// Force treatment as VT_PTR.
					uiIndirectionLevel += 2;
					break;
				case VT_SAFEARRAY:
					vtConversionType |= VT_ARRAY;
					break;
				default:
					PyErr_SetString(PyExc_TypeError, "COM type description - unknown indirection type.");
					goto Error;
				}
				obArgType = PyTuple_GET_ITEM(obArgType, 1);
			}
		}
		
		vtArgType = (VARTYPE)PyInt_AS_LONG(obArgType);
		
		// non-BSTR strings are only supported in their normal case.
		if (vtArgType & VT_TYPEMASK == VT_LPSTR ||
			vtArgType & VT_TYPEMASK == VT_LPWSTR)
		{
			uiIndirectionLevel++;
		}
		
		// If our indirection level is over 1, then we don't know what to do.
		// They can figure out wtf is going on.
		if (uiIndirectionLevel > 1)
		{
			vtConversionType = VT_PTR;	
		}
		else
		{
			if (uiIndirectionLevel == 1)
			{
				vtConversionType |= VT_BYREF;
			}
			vtConversionType |= vtArgType;
		}

		switch (vtConversionType & VT_TYPEMASK)
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
		case VT_VARIANT:
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
				vtConversionType = VT_I4 | (vtConversionType & VT_TYPEMASK);
			}
			if (vtConversionType == VT_UINT) 
			{
				// Preserve VT_BYREF or VT_ARRAY
				vtConversionType = VT_UI4 | (vtConversionType & VT_TYPEMASK);
			}
			var.vt = vtConversionType;
			// Copy the data into the variant...
			cb = SizeOfVT(var.vt);
			memcpy(&var.lVal, pb, cb);
			// Convert it into a PyObject:
			obArg = PyCom_PyObjectFromVariant(&var);
			break;
		// Strings are always VT_BYREF'd due to the above
		// uiIndirectionLevel calculation.
		case VT_LPSTR:
			obArg = PyString_FromString(*(CHAR **)pb);
			break;
		case VT_LPWSTR:
			obArg = PyWinObject_FromOLECHAR(*(OLECHAR **)pb);
			break;
		// Special cases:
		case VT_UI8:
			if (uiIndirectionLevel == 0)
			{
				obArg = PyWinObject_FromULARGE_INTEGER(*(ULARGE_INTEGER *)pb);
			}
			else
			{
				obArg = PyWinObject_FromULARGE_INTEGER(**(ULARGE_INTEGER **)pb);
			}
			break;
		case VT_I8:
			if (uiIndirectionLevel == 0)
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
		}

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
