/*
** Universal gateway module
*/

#include "stdafx.h"
#include "PythonCOM.h"
#include "PythonCOMServer.h"
#include "PythonCOMRegister.h"
#include "univgw_dataconv.h"


static PyObject *g_obRegisteredVTables = NULL;

// ### copied from PyGatewayBase.cpp
extern const GUID IID_IInternalUnwrapPythonObject;

typedef HRESULT (STDMETHODCALLTYPE * pfnGWMethod)(struct gw_object * _this);

typedef struct gw_vtbl
{
	DWORD		magic;
#define GW_VTBL_MAGIC	0x20534A47

	PyObject *	dispatcher;	// dispatcher from the COM2Py thunk definition
	
	// Python CObject that owns freeing this object.
	// This is here so that the method function pointer array
	// exists as long as the tear off interface does.
	// This is also here so that the tear off interfaces have the absolute
	// minimum state necessary.
	PyObject *  obVTable;   
	IID			iid;		// the IID of this interface
	UINT		cMethod;	// count of methods

	// the vtable (the actual methods)
#pragma warning ( disable : 4200 )
	pfnGWMethod	methods[];
#pragma warning ( default : 4200 )

} gw_vtbl;

typedef struct gw_object
{
	pfnGWMethod *					vtbl;
	IInternalUnwrapPythonObject *	punk;		// the identity interface
	LONG							cRef;

} gw_object;

#define GET_DEFN(gwo)	((gw_vtbl *)((char *)(gwo)->vtbl - offsetof(gw_vtbl, methods)))

static void set_error(REFIID riid, LPCOLESTR desc)
{
	ICreateErrorInfo *pICEI;
	HRESULT hr = CreateErrorInfo(&pICEI);
	if ( SUCCEEDED(hr) )
	{
		CComBSTR b(desc);

		pICEI->SetGUID(riid);
		pICEI->SetDescription(b);

		IErrorInfo *pIEI;
		Py_BEGIN_ALLOW_THREADS
		hr = pICEI->QueryInterface(IID_IErrorInfo, (LPVOID*) &pIEI);
		Py_END_ALLOW_THREADS
		if ( SUCCEEDED(hr) )
		{
			SetErrorInfo(0, pIEI);
			pIEI->Release();
		}
		pICEI->Release();			
	}
}

static HRESULT univgw_dispatch(DWORD index, gw_object * _this, va_list argPtr)
{
	PY_GATEWAY_METHOD;
	gw_vtbl * vtbl = GET_DEFN(_this);

	// get a pointer for the va_list and convert it to a (long) integer
	PyObject *obArgPtr = PyLong_FromVoidPtr((void *)VA_LIST_PTR(argPtr));

	// prep the rest of the arguments
	PyObject *obArgs = PyTuple_New(3);
	PyObject *obIndex = PyInt_FromLong(index);

	if ( obArgPtr == NULL || obArgs == NULL || obIndex == NULL )
	{
		set_error(vtbl->iid, L"could not create argument tuple");
		Py_XDECREF(obArgPtr);
		Py_XDECREF(obArgs);
		Py_XDECREF(obIndex);
		PyErr_Clear();
		return E_OUTOFMEMORY;	// ### select a different value?
	}

	// get the underlying Python object
	PyObject * instance;
	_this->punk->Unwrap(&instance);

	// obArgs takes our references
	PyTuple_SET_ITEM(obArgs, 0, instance);
	PyTuple_SET_ITEM(obArgs, 1, obIndex);
	PyTuple_SET_ITEM(obArgs, 2, obArgPtr);

	// call the provided method
	PyObject *result = PyEval_CallObjectWithKeywords(vtbl->dispatcher, obArgs, NULL);

	// done with the arguments and the contained objects
	Py_DECREF(obArgs);

	if ( result == NULL )
	  return PyCom_SetCOMErrorFromPyException(vtbl->iid);

	HRESULT hr;
	if ( result == Py_None )
	{
		hr = S_OK;
	}
	else
	{
		if ( !PyInt_Check(result) )
		{
			Py_DECREF(result);
			set_error(vtbl->iid, L"expected integer return value");
			return E_UNEXPECTED;	// ### select a different value?
		}

		hr = PyInt_AS_LONG(result);
	}

	Py_DECREF(result);

	// ### Greg> what to do for non-HRESULT return values?
	// ### Bill> If its not a float/double then
	// ###       then they'll see a 32bit sign-extended value.
	// ###       If its a float/double they're currently out of luck.
	// ###       The smart ones only declare int, HRESULT, or void
	// ###       functions in any event...
	// ### on X86s __stdcall return values go into:
	// ### char:     al
	// ### short:    ax
	// ### int:     eax
	// ### long:    eax
	// ### float:  ST(0)
	// ### double: ST(0)
	// ### HRESULT: eax
	// ### int64:   edx:eax
	// ### Where edx is the most significant 32bits.
	// ### All praise we don't have to deal with the Alpha calling convention...
	// ### If we want to handle multiple ABIs it might be wise to look into
	// ### using libfii.
	return hr;
}

//#define COMPILE_MOCKUP
#ifdef COMPILE_MOCKUP

STDMETHODIMP mockup(gw_object * _this)
{
	va_list args;
	va_start(args, _this);
	return univgw_dispatch(0x11223344, _this, args);
}

#endif // COMPILE_MOCKUP


static pfnGWMethod make_method(DWORD index, UINT argsize)
{
	unsigned char * code;

#ifdef _M_IX86

	static const unsigned char func_template[] = {
// ; 45   : STDMETHODIMP mockup(gw_object * _this)
// ; 46   : {
//  00000	55				push	 ebp
//  00001	8b ec			mov		 ebp, esp
//  00003	51				push	 ecx
		0x55, 0x8b, 0xec, 0x51,

// ; 47   : 	va_list args;
// ; 48   : 	va_start(args, _this);
//  00004	8d 45 0c		lea		 eax, DWORD PTR __this$[ebp+4]
//  00007	89 45 fc		mov		 DWORD PTR _args$[ebp], eax
		0x8d, 0x45, 0x0c, 0x89, 0x45, 0xfc,

// ; 49   : 	return univgw_dispatch(0x11223344, _this, args);
//  0000a	8b 4d fc		mov		 ecx, DWORD PTR _args$[ebp]
//  0000d	51				push	 ecx
//  0000e	8b 55 08		mov		 edx, DWORD PTR __this$[ebp]
//  00011	52				push	 edx
//  00012	68 44 33 22 11	push	 287454020		; 11223344H
//  00017	e8 00 00 00 00	call	 ?univgw_dispatch@@YAJKPAUgw_object@@PAD@Z ; univgw_dispatch
//  0001c	83 c4 0c	 	add		 esp, 12			; 0000000cH
		0x8b, 0x4d, 0xfc, 0x51, 0x8b, 0x55, 0x08, 0x52, 0x68,
		// offset = 19 (0x13)
		0x44, 0x33, 0x22, 0x11,		// replace these with <index>
		0xe8,
		// offset = 24 (0x18)
		0x00, 0x00, 0x00, 0x00,		// replace these with <univgw_dispatch>
		0x83, 0xc4, 0x0c,

//; 50   : }
//  0001f	8b e5			mov		 esp, ebp
//  00021	5d				pop		 ebp
//  00022	c2 04 00		ret		 4
		0x8b, 0xe5, 0x5d, 0xc2,
		// offset = 35 (0x23)
		0x04, 0x00,					// replace this with argsize
	};

	// make a copy of code and plug in the appropriate values.
	// NOTE: the call address is relative
	code = (unsigned char *)malloc(sizeof(func_template));
	memcpy(code, func_template, sizeof(func_template));
	*(long *)&code[19] = index;
	*(long *)&code[24] = (long)&univgw_dispatch - (long)&code[28];
	*(short *)&code[35] = argsize;

#else	// _M_IX86
#  error make_method not defined for this platform
#endif

	return (pfnGWMethod)code;
}

static STDMETHODIMP univgw_QueryInterface(gw_object * _this, REFIID riid, void **ppv)
{
	// NOTE: ->iid can never be IID_IUnknown, so we don't have to worry
	// about COM equivalence rules here.

	if ( IsEqualIID(riid, GET_DEFN(_this)->iid) )
	{
		/*
		** Note that we don't need to cast since _this already points to a
		** properly-formed vtable-based object. Normally, a C++ object would
		** need the cast to select the proper vtable; we've got it already.
		*/
		*ppv = _this;
		++_this->cRef;
		return S_OK;
	}

	// delegate to the original interface
	return _this->punk->QueryInterface(riid, ppv);
}

static STDMETHODIMP_(ULONG) univgw_AddRef(gw_object * _this)
{
	return InterlockedIncrement(&_this->cRef);
}

static STDMETHODIMP_(ULONG) univgw_Release(gw_object * _this)
{
	LONG cRef = InterlockedDecrement(&_this->cRef);
	if ( cRef == 0 )
	{
		_this->punk->Release();
		CEnterLeavePython _celp;
		Py_DECREF(GET_DEFN(_this)->obVTable);
		free(_this);
		return 0;
	}
	return _this->cRef;
}

/* The IDispatch delegation when necessary */
static STDMETHODIMP univgw_GetIDsOfNames( gw_object * _this, REFIID riid, 
	OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, 
	DISPID FAR* rgDispId )
{
	return ((PyGatewayBase *)_this->punk)->GetIDsOfNames(riid, rgszNames, cNames, lcid, rgDispId);
}

static STDMETHODIMP univgw_GetTypeInfo( gw_object *_this, unsigned int iTInfo, LCID lcid, 
	ITypeInfo FAR* FAR* ppTInfo )
{
	return ((PyGatewayBase *)_this->punk)->GetTypeInfo(iTInfo, lcid, ppTInfo);
}

static STDMETHODIMP univgw_GetTypeInfoCount( gw_object *_this, unsigned int FAR* pctinfo )
{
	return ((PyGatewayBase *)_this->punk)->GetTypeInfoCount(pctinfo);
}

static STDMETHODIMP univgw_Invoke( gw_object *_this, DISPID dispIdMember, REFIID riid, LCID lcid, 
	WORD wFlags, DISPPARAMS FAR* pDispParams, 
	VARIANT FAR* pVarResult, EXCEPINFO FAR* pExcepInfo, 
	unsigned int FAR* puArgErr)
{
	return ((PyGatewayBase *)_this->punk)->Invoke(dispIdMember, riid, lcid, wFlags, pDispParams, pVarResult, pExcepInfo, puArgErr);
}


/* End of IDispatch delegation */

/* free the gw_vtbl object. also works on a partially constructed gw_vtbl. */
static void __cdecl free_vtbl(void * cobject)
{
	gw_vtbl * vtbl = (gw_vtbl *)cobject;

	Py_XDECREF(vtbl->dispatcher);

	// free the methods. 0..2 are the constant IUnknown methods
	for ( int i = vtbl->cMethod; --i > 2; )
		if ( vtbl->methods[i] != NULL )
			free(vtbl->methods[i]);
	free(vtbl);
}

static PyObject * univgw_CreateVTable(PyObject *self, PyObject *args)
{
	PyObject *obDef;
	int isDispatch = 0;
	if ( !PyArg_ParseTuple(args, "O|i:CreateVTable", &obDef, &isDispatch) )
		return NULL;

	PyObject *obIID = PyObject_CallMethod(obDef, "iid", NULL);
	if ( obIID == NULL )
		return NULL;
	IID iid;
	if ( !PyWinObject_AsIID(obIID, &iid) )
	{
		Py_DECREF(obIID);
		return NULL;
	}
	Py_DECREF(obIID);

	/*
	** It doesn't make sense to create one of these for IUnknown, since
	** we need to use the original PyGatewayBase object as the base
	** object for all gateways (for COM equivalence testing)
	**
	** NOTE: it isn't worth it to try to optimize the other interfaces
	** on PyGatewayBase
	*/
	if ( iid == IID_IUnknown )
	{
		PyErr_SetString(PyExc_ValueError, "tear-off not allowed for IUnknown");
		return NULL;
	}

	PyObject * methods = PyObject_CallMethod(obDef, "vtbl_argsizes", NULL);
	if ( methods == NULL )
		return NULL;

	int count = PyObject_Length(methods);
	if ( count == -1 )
	{
		Py_DECREF(methods);
		return NULL;
	}
	int numReservedVtables = 3;	// the methods list should not specify IUnknown methods
	
	if (isDispatch) // or IDispatch if this interface uses it.
		numReservedVtables+= 4;

	count += numReservedVtables;

	// compute the size of the structure plus the method pointers
	size_t size = sizeof(gw_vtbl) + count * sizeof(pfnGWMethod);
	gw_vtbl * vtbl = (gw_vtbl *)malloc(size);
	if ( vtbl == NULL )
	{
		Py_DECREF(methods);
		PyErr_NoMemory();
		return NULL;
	}
	memset(vtbl, 0, size);

	vtbl->magic = GW_VTBL_MAGIC;
	Py_INCREF(obDef);
	vtbl->iid = iid;
	vtbl->cMethod = count;
	vtbl->obVTable = NULL;

	vtbl->dispatcher = PyObject_GetAttrString(obDef, "dispatch");
	if ( vtbl->dispatcher == NULL )
		goto error;

	vtbl->methods[0] = (pfnGWMethod)univgw_QueryInterface;
	vtbl->methods[1] = (pfnGWMethod)univgw_AddRef;
	vtbl->methods[2] = (pfnGWMethod)univgw_Release;

	if (isDispatch) {
		vtbl->methods[3] = (pfnGWMethod)univgw_GetIDsOfNames;
		vtbl->methods[4] = (pfnGWMethod)univgw_GetTypeInfo;
		vtbl->methods[5] = (pfnGWMethod)univgw_GetTypeInfoCount;
		vtbl->methods[6] = (pfnGWMethod)univgw_Invoke;
	}

	// add the methods. NOTE: 0..2 are the constant IUnknown methods
	int i;
	for ( i = vtbl->cMethod - numReservedVtables; i--; )
	{
		PyObject * obArgSize = PySequence_GetItem(methods, i);
		if ( obArgSize == NULL )
			goto error;

		int argSize = PyInt_AsLong(obArgSize);
		Py_DECREF(obArgSize);
		if ( argSize == -1 && PyErr_Occurred() )
			goto error;

		// dynamically construct a function with the provided argument
		// size; reserve additional space for the _this argument.
		pfnGWMethod meth = make_method(i, argSize + sizeof(gw_object *));
		if ( meth == NULL )
		{
			(void)PyErr_NoMemory();
			goto error;
		}

		vtbl->methods[i + numReservedVtables] = meth;
	}
	Py_DECREF(methods);

	PyObject *result;
	result = PyCObject_FromVoidPtr(vtbl, free_vtbl);
	if ( result == NULL )
	{
		free_vtbl(vtbl);
		return NULL;
	}
	
	// Stick the CObject into the vtable itself
	// so that the tear off interfaces can INCREF/DECREF it at
	// their own whim.
	vtbl->obVTable = result;
	Py_INCREF(result); // the vtable's reference.
	return result;

  error:
	Py_DECREF(methods);
	free_vtbl(vtbl);
	return NULL;
}

// Does all of the heavy lifting...
// Returns the created vtable pointer.
static IUnknown *CreateTearOff
(
	PyObject *obInstance,
	PyGatewayBase *gatewayBase,
	PyObject *obVTable
)
{
	gw_vtbl * vtbl = (gw_vtbl *)PyCObject_AsVoidPtr(obVTable);

	if ( vtbl->magic != GW_VTBL_MAGIC )
	{
		PyErr_SetString(PyExc_ValueError, "argument does not contain a vtable");
		return NULL;
	}

	// construct a C++ object (a block of mem with a vtbl)
	gw_object * punk = (gw_object *)malloc(sizeof(gw_object));
	if ( punk == NULL )
	{
		PyErr_NoMemory();
		return NULL;
	}
	punk->vtbl = vtbl->methods;
	punk->punk = (IInternalUnwrapPythonObject *)gatewayBase;
	punk->punk->AddRef();
	// we start with one reference (the object we return)
	punk->cRef = 1;
	// Make sure the vtbl doesn't go away before we do.
	Py_INCREF(vtbl->obVTable);

	return (IUnknown *)punk;	
}

static PyObject * univgw_CreateTearOff(PyObject *self, PyObject *args)
{
	PyObject *obInstance;
	PyObject *obVTable;
	PyObject *obIID = NULL;
	IUnknown *punk = NULL;
	if ( !PyArg_ParseTuple(args, "OO|O:CreateTearOff", &obInstance, &obVTable, &obIID) )
		return NULL;

	IID iidInterface = IID_IUnknown;	// what PyI* to wrap with
	if ( obIID && obIID != Py_None )
		if ( !PyWinObject_AsIID(obIID, &iidInterface) )
			return NULL;

	// obVTable must be a CObject containing our vtbl ptr
	if ( !PyCObject_Check(obVTable) )
	{
		PyErr_SetString(PyExc_ValueError, "argument is not a CObject/vtable");
		return NULL;
	}

	// Do all of the grunt work.
	punk = CreateTearOff(obInstance, NULL, obVTable);
	if (!punk)
	{
		Py_DECREF(obVTable);
		return NULL;
	}

	// Convert to a PyObject.
	return PyCom_PyObjectFromIUnknown((IUnknown *)punk, iidInterface, FALSE);
}

// This is the function that gets called for creating
// registered PythonCOM gateway objects.
static HRESULT CreateRegisteredTearOff(PyObject *pPyInstance, PyGatewayBase *base, void **ppResult, REFIID iid)
{
	if (ppResult == NULL ||
		pPyInstance == NULL)
	{
		return E_POINTER;
	}

	// Lookup vtable using iid.
	PyObject *obIID = PyWinObject_FromIID(iid);
	PyObject *obVTable = PyDict_GetItem(g_obRegisteredVTables, obIID);
	if (!obVTable)
	{
		OLECHAR oleRes[128];
		StringFromGUID2(iid, oleRes, sizeof(oleRes));
		printf("Couldn't find IID %S\n", oleRes);
		// This should never happen....
		_ASSERTE(FALSE);
		return E_NOINTERFACE;
	}

	// obVTable must be a CObject containing our vtbl ptr
	if ( !PyCObject_Check(obVTable) )
	{
		Py_DECREF(obVTable);
		_ASSERTE(FALSE);
		return E_NOINTERFACE;
	}

	// Do all of the grunt work.
	*ppResult = CreateTearOff(pPyInstance, base, obVTable);
	Py_DECREF(obVTable);
	if (*ppResult == NULL)
		return E_FAIL;
	return S_OK;
}

static PyObject * univgw_RegisterVTable(PyObject *self, PyObject *args)
{
	IID iid;
	PyObject *obVTable;
	PyObject *obIID;
	char *pszInterfaceNm = NULL;
	PyObject *ret = NULL;

	if ( !PyArg_ParseTuple(args, "OOs:RegisterVTable", &obVTable, &obIID, &pszInterfaceNm) )
		return NULL;

	// obVTable must be a CObject containing our vtbl ptr
	if ( !PyCObject_Check(obVTable) )
	{
		PyErr_SetString(PyExc_ValueError, "argument is not a CObject/vtable");
		return NULL;
	}

	if (!PyWinObject_AsIID(obIID, &iid))
	{
		PyErr_SetString(PyExc_ValueError, "argument is not an IID");
		return NULL;
	}
	// obIID may be a string, but we need it to be a real PyIID.
	PyObject *keyObject = PyWinObject_FromIID(iid);

	if (0 != PyDict_SetItem(
		g_obRegisteredVTables,
		keyObject,
		obVTable))
	{

		goto done;
	}

	if (!PyCom_IsGatewayRegistered(iid))
	{
		HRESULT hr = PyCom_RegisterGatewayObject(iid, CreateRegisteredTearOff, pszInterfaceNm);
		if (FAILED(hr))
			goto done;
	}
	
	Py_INCREF(Py_None);
	ret = Py_None;
done:
	Py_DECREF(keyObject);
	return ret;
}

static PyObject * univgw_ReadMemory(PyObject *self, PyObject *args)
{
	PyObject *obPtr;
	int size;
	if ( !PyArg_ParseTuple(args, "Oi:ReadMemory", &obPtr, &size) )
		return NULL;

	void *p = PyLong_AsVoidPtr(obPtr);
	if ( p == NULL && PyErr_Occurred() )
	{
		// reset the error to something meaningful
		PyErr_SetString(PyExc_ValueError, "argument is not an integer");
		return NULL;
	}

	return PyString_FromStringAndSize((char *)p, size);
}

static PyObject * univgw_WriteMemory(PyObject *self, PyObject *args)
{
	PyObject *obPtr;
	void *pSrc;
	int size;
	if ( !PyArg_ParseTuple(args, "Os#:WriteMemory", &obPtr, &pSrc, &size) )
		return NULL;

	void *pDst = PyLong_AsVoidPtr(obPtr);
	if ( pDst == NULL && PyErr_Occurred() )
	{
		// reset the error to something meaningful
		PyErr_SetString(PyExc_ValueError, "argument is not an integer");
		return NULL;
	}

	memcpy(pDst, pSrc, size);

	Py_INCREF(Py_None);
	return Py_None;
}

static struct PyMethodDef univgw_functions[] =
{
	{ "CreateVTable", univgw_CreateVTable, 1 },
	{ "CreateTearOff", univgw_CreateTearOff, 1 },
	{ "ReadMemory", univgw_ReadMemory, 1 },
	{ "WriteMemory", univgw_WriteMemory, 1 },
	{ "RegisterVTable", univgw_RegisterVTable, 1},

	{ "L64", dataconv_L64, 1 },
	{ "UL64", dataconv_UL64, 1 },
	{ "strL64", dataconv_strL64, 1 },
	{ "strUL64", dataconv_strUL64, 1 },
	{ "interface", dataconv_interface, 1},
	{ "SizeOfVT", dataconv_SizeOfVT, 1},
	{ "WriteFromOutTuple", dataconv_WriteFromOutTuple, 1},
	{ "ReadFromInTuple", dataconv_ReadFromInTuple, 1},

	{ NULL } /* sentinel */
};

BOOL initunivgw(PyObject *parentDict)
{
//	HRESULT hr;

	PyObject *module = Py_InitModule("pythoncom.__univgw", univgw_functions);
	if (!module) /* Eeek - some serious error! */
		return FALSE;

//	PyObject *dict = PyModule_GetDict(module);
//	if (!dict) return; /* Another serious error!*/

	g_obRegisteredVTables = PyDict_New();

	PyDict_SetItemString(parentDict, "_univgw", module);

	return TRUE;
}

