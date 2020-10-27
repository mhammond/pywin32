/*
** Universal gateway module
** Written by Bill Tutt and Greg Stein.
*/

#include "stdafx.h"
#include "stddef.h"
#include "PythonCOM.h"
#include "PythonCOMServer.h"
#include "PythonCOMRegister.h"
#include "univgw_dataconv.h"

static PyObject *g_obRegisteredVTables = NULL;

// ### copied from PyGatewayBase.cpp
extern const GUID IID_IInternalUnwrapPythonObject;

typedef HRESULT(STDMETHODCALLTYPE *pfnGWMethod)(struct gw_object *_this);

typedef struct gw_vtbl {
    DWORD magic;
#define GW_VTBL_MAGIC 0x20534A47

    PyObject *dispatcher;  // dispatcher from the COM2Py thunk definition

    IID iid;                // the IID of this interface
    UINT cMethod;           // count of methods
    UINT cReservedMethods;  // number of reserved methods; 3 for IUnknown, 7 for IDispatch.

    // the vtable (the actual methods)
#pragma warning(disable : 4200)
    pfnGWMethod methods[];
#pragma warning(default : 4200)

} gw_vtbl;

typedef struct gw_object {
    pfnGWMethod *vtbl;
    PyObject *obVTable;                 // reference to vtable
    IInternalUnwrapPythonObject *punk;  // the identity interface
    LONG cRef;

} gw_object;

#define GET_DEFN(gwo) ((gw_vtbl *)((char *)(gwo)->vtbl - offsetof(gw_vtbl, methods)))

static void set_error(REFIID riid, LPCOLESTR desc)
{
    ICreateErrorInfo *pICEI;
    HRESULT hr = CreateErrorInfo(&pICEI);
    if (SUCCEEDED(hr)) {
        BSTR b = SysAllocString(desc);

        pICEI->SetGUID(riid);
        pICEI->SetDescription(b);

        IErrorInfo *pIEI;
        Py_BEGIN_ALLOW_THREADS hr = pICEI->QueryInterface(IID_IErrorInfo, (LPVOID *)&pIEI);
        Py_END_ALLOW_THREADS if (SUCCEEDED(hr))
        {
            SetErrorInfo(0, pIEI);
            pIEI->Release();
        }
        pICEI->Release();
        SysFreeString(b);
    }
}

static HRESULT univgw_dispatch(DWORD index, gw_object *_this, va_list argPtr)
{
    PY_GATEWAY_METHOD;
    gw_vtbl *vtbl = GET_DEFN(_this);

    // get a pointer for the va_list and convert it to a (long) integer
    PyObject *obArgPtr = PyLong_FromVoidPtr((void *)VA_LIST_PTR(argPtr));

    // prep the rest of the arguments
    PyObject *obArgs = PyTuple_New(3);
    PyObject *obIndex = PyInt_FromLong(index);

    if (obArgPtr == NULL || obArgs == NULL || obIndex == NULL) {
        set_error(vtbl->iid, L"could not create argument tuple");
        Py_XDECREF(obArgPtr);
        Py_XDECREF(obArgs);
        Py_XDECREF(obIndex);
        PyErr_Clear();
        return E_OUTOFMEMORY;  // ### select a different value?
    }

    // get the underlying Python object
    PyObject *instance;
    _this->punk->Unwrap(&instance);

    // obArgs takes our references
    PyTuple_SET_ITEM(obArgs, 0, instance);
    PyTuple_SET_ITEM(obArgs, 1, obIndex);
    PyTuple_SET_ITEM(obArgs, 2, obArgPtr);

    // call the provided method
    PyObject *result = PyEval_CallObjectWithKeywords(vtbl->dispatcher, obArgs, NULL);

    // done with the arguments and the contained objects
    Py_DECREF(obArgs);

    if (result == NULL) {
        PyCom_LoggerException(NULL, "Failed to call the universal dispatcher");
        return PyCom_SetCOMErrorFromPyException(vtbl->iid);
    }

    HRESULT hr;
    if (result == Py_None) {
        hr = S_OK;
    }
    else {
        if (!PyInt_Check(result)) {
            Py_DECREF(result);
            set_error(vtbl->iid, L"expected integer return value");
            return E_UNEXPECTED;  // ### select a different value?
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

STDMETHODIMP mockup(gw_object *_this)
{
    va_list args;
    va_start(args, _this);
    return univgw_dispatch(0x11223344, _this, args);
}

#endif  // COMPILE_MOCKUP

static pfnGWMethod make_method(DWORD index, UINT argsize, UINT argc)
{
    unsigned char *code;

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
        0x44, 0x33, 0x22, 0x11,  // replace these with <index>
        0xe8,
        // offset = 24 (0x18)
        0x00, 0x00, 0x00, 0x00,  // replace these with <univgw_dispatch>
        0x83, 0xc4, 0x0c,

        //; 50   : }
        //  0001f	8b e5			mov		 esp, ebp
        //  00021	5d				pop		 ebp
        //  00022	c2 04 00		ret		 4
        0x8b, 0xe5, 0x5d, 0xc2,
        // offset = 35 (0x23)
        0x04, 0x00,  // replace this with argsize
    };

    // make a copy of code and plug in the appropriate values.
    // NOTE: the call address is relative, and that the memory we allocate
    // must be marked as 'executable' or DEP will kill us.  To be good
    // citizens we leave the final page 'executable' but read-only.
    code = (unsigned char *)VirtualAlloc(NULL, sizeof(func_template), MEM_COMMIT, PAGE_READWRITE);
    if (code == NULL) {
        PyErr_NoMemory();
        return NULL;  // caller sets memory error
    }
    memcpy(code, func_template, sizeof(func_template));
    *(long *)&code[19] = index;
    *(long *)&code[24] = (long)&univgw_dispatch - (long)&code[28];
    *(short *)&code[35] = argsize;

    DWORD oldprotect;
    if (!VirtualProtect(code, sizeof(func_template), PAGE_EXECUTE, &oldprotect)) {
        VirtualFree(code, 0, MEM_RELEASE);
        PyErr_SetString(PyExc_RuntimeError, "failed to set memory attributes to executable");
        return NULL;
    }
#elif _M_X64
    static const unsigned char wrapper[] = {
        0x48, 0x89, 0x54, 0x24, 0x10, /* mov [rsp + 16], rdx */
        0x4c, 0x89, 0x44, 0x24, 0x18, /* mov [rsp + 24], r8  */
        0x4c, 0x89, 0x4c, 0x24, 0x20, /* mov [rsp + 32], r9 */

        0x48, 0x89, 0xca,                         /* mov rdx, rcx */
        0x4c, 0x8d, 0x44, 0x24, 0x10,             /* lea r8, [rsp + 16] */
        0x48, 0x83, 0xec, 0x28,                   /* sub rsp, 40 - we have to keep stack 16-byte aligned */
        0x48, 0xc7, 0xc1, 0x00, 0x00, 0x00, 0x00, /* mov rcx, imm32 */
        0x48, 0xb8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* mov rax, imm64, target address */
        0xff, 0xd0,                                                 /* call rax */
        0x48, 0x83, 0xc4, 0x28,                                     /* add rsp, 40 */
        0xc3                                                        /* ret */
    };

    code = (unsigned char *)VirtualAlloc(NULL, sizeof(wrapper), MEM_COMMIT, PAGE_READWRITE);
    if (code == NULL) {
        PyErr_NoMemory();
        return NULL;  // caller sets memory error
    }
    memcpy(code, &wrapper[0], sizeof(wrapper));

    for (int i = 0; i < 3; i++) {
        if (i < argc)
            continue;
        for (int j = 0; j < 5; j++) code[i * 5 + j] = 0x90;
    }

    *(int *)(code + 30) = index;
    *(void **)(code + 36) = &univgw_dispatch;

    DWORD oldprotect;
    if (!VirtualProtect(code, sizeof(wrapper), PAGE_EXECUTE, &oldprotect)) {
        VirtualFree(code, 0, MEM_RELEASE);
        PyErr_SetString(PyExc_RuntimeError, "failed to set memory attributes to executable");
        return NULL;
    }
#else  // other arches
    /* The MAINWIN toolkit allows us to build this on Linux!!! */
#pragma message("XXXXXXXXX - win32com.universal wont work on this platform - need make_method")
#endif

    return (pfnGWMethod)code;
}

static STDMETHODIMP univgw_QueryInterface(gw_object *_this, REFIID riid, void **ppv)
{
    // NOTE: ->iid can never be IID_IUnknown, so we don't have to worry
    // about COM equivalence rules here.

    if (IsEqualIID(riid, GET_DEFN(_this)->iid)) {
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

static STDMETHODIMP_(ULONG) univgw_AddRef(gw_object *_this) { return InterlockedIncrement(&_this->cRef); }

static STDMETHODIMP_(ULONG) univgw_Release(gw_object *_this)
{
    LONG cRef = InterlockedDecrement(&_this->cRef);
    if (cRef == 0) {
        _this->punk->Release();
        CEnterLeavePython _celp;
        Py_DECREF(_this->obVTable);
        free(_this);
        return 0;
    }
    return _this->cRef;
}

/* The IDispatch delegation when necessary */
static STDMETHODIMP univgw_GetIDsOfNames(gw_object *_this, REFIID riid, OLECHAR FAR *FAR *rgszNames,
                                         unsigned int cNames, LCID lcid, DISPID FAR *rgDispId)
{
    return ((PyGatewayBase *)_this->punk)->GetIDsOfNames(riid, rgszNames, cNames, lcid, rgDispId);
}

static STDMETHODIMP univgw_GetTypeInfo(gw_object *_this, unsigned int iTInfo, LCID lcid, ITypeInfo FAR *FAR *ppTInfo)
{
    return ((PyGatewayBase *)_this->punk)->GetTypeInfo(iTInfo, lcid, ppTInfo);
}

static STDMETHODIMP univgw_GetTypeInfoCount(gw_object *_this, unsigned int FAR *pctinfo)
{
    return ((PyGatewayBase *)_this->punk)->GetTypeInfoCount(pctinfo);
}

static STDMETHODIMP univgw_Invoke(gw_object *_this, DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags,
                                  DISPPARAMS FAR *pDispParams, VARIANT FAR *pVarResult, EXCEPINFO FAR *pExcepInfo,
                                  unsigned int FAR *puArgErr)
{
    return ((PyGatewayBase *)_this->punk)
        ->Invoke(dispIdMember, riid, lcid, wFlags, pDispParams, pVarResult, pExcepInfo, puArgErr);
}

/* End of IDispatch delegation */

/* free the gw_vtbl object. also works on a partially constructed gw_vtbl. */
static void free_vtbl(gw_vtbl *vtbl)
{
    assert(vtbl);
    if (!vtbl)
        return;
    assert(vtbl->magic == GW_VTBL_MAGIC);
    Py_XDECREF(vtbl->dispatcher);

    // free the methods. 0..2 are the constant IUnknown methods
    for (int i = vtbl->cMethod; i-- > (int)vtbl->cReservedMethods;)
        if (vtbl->methods[i] != NULL)
            VirtualFree((void *)vtbl->methods[i], 0, MEM_RELEASE);
    VirtualFree(vtbl, 0, MEM_RELEASE);
}

#if PY_VERSION_HEX > 0x03010000
// Use the new capsule API
const char *capsule_name = "win32com universal gateway";

static void __cdecl do_free_vtbl(PyObject *ob) { free_vtbl((gw_vtbl *)PyCapsule_GetPointer(ob, capsule_name)); }

static PyObject *PyVTable_Create(void *vtbl) { return PyCapsule_New(vtbl, capsule_name, do_free_vtbl); }
static gw_vtbl *PyVTable_Get(PyObject *ob) { return (gw_vtbl *)PyCapsule_GetPointer(ob, capsule_name); }

static bool PyVTable_Check(PyObject *ob) { return PyCapsule_IsValid(ob, capsule_name) != 0; }
#else
// Use the old CObject API.
static void __cdecl do_free_vtbl(void *cobject)
{
    gw_vtbl *vtbl = (gw_vtbl *)cobject;
    free_vtbl(vtbl);
}

static PyObject *PyVTable_Create(void *vtbl) { return PyCObject_FromVoidPtr(vtbl, do_free_vtbl); }

static gw_vtbl *PyVTable_Get(PyObject *ob) { return (gw_vtbl *)PyCObject_AsVoidPtr(ob); }

static bool PyVTable_Check(PyObject *ob) { return PyCObject_Check(ob) != 0; }
#endif

static PyObject *univgw_CreateVTable(PyObject *self, PyObject *args)
{
    PyObject *obDef;
    int isDispatch = 0;
    if (!PyArg_ParseTuple(args, "O|i:CreateVTable", &obDef, &isDispatch))
        return NULL;

    PyObject *obIID = PyObject_CallMethod(obDef, "iid", NULL);
    if (obIID == NULL)
        return NULL;
    IID iid;
    if (!PyWinObject_AsIID(obIID, &iid)) {
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
    if (iid == IID_IUnknown) {
        PyErr_SetString(PyExc_ValueError, "tear-off not allowed for IUnknown");
        return NULL;
    }

    PyObject *methods = PyObject_CallMethod(obDef, "vtbl_argsizes", NULL);
    if (methods == NULL)
        return NULL;

    int count = PyObject_Length(methods);
    if (count == -1) {
        Py_DECREF(methods);
        return NULL;
    }
    PyObject *methodsArgc = PyObject_CallMethod(obDef, "vtbl_argcounts", NULL);
    if (methodsArgc == NULL)
        return NULL;

    int numReservedVtables = 3;  // the methods list should not specify IUnknown methods

    if (isDispatch)  // or IDispatch if this interface uses it.
        numReservedVtables += 4;

    count += numReservedVtables;

    // compute the size of the structure plus the method pointers
    size_t size = sizeof(gw_vtbl) + count * sizeof(pfnGWMethod);
    // NOTE: we are allocating a function pointer, so the memory we
    // allocate must be marked as 'executable' or DEP will kill us. To be
    // good citizens we leave the final page 'executable' but read-only.
    gw_vtbl *vtbl = (gw_vtbl *)VirtualAlloc(NULL, size, MEM_COMMIT, PAGE_READWRITE);
    if (vtbl == NULL) {
        Py_DECREF(methods);
        PyErr_NoMemory();
        return NULL;
    }
    // memset(vtbl, 0, size); - reset by VirtualAlloc

    vtbl->magic = GW_VTBL_MAGIC;
    vtbl->iid = iid;
    vtbl->cMethod = count;
    vtbl->cReservedMethods = numReservedVtables;

    vtbl->dispatcher = PyObject_GetAttrString(obDef, "dispatch");
    if (vtbl->dispatcher == NULL)
        goto error;

    vtbl->methods[0] = (pfnGWMethod)univgw_QueryInterface;
    vtbl->methods[1] = (pfnGWMethod)univgw_AddRef;
    vtbl->methods[2] = (pfnGWMethod)univgw_Release;

    if (isDispatch) {
        vtbl->methods[3] = (pfnGWMethod)univgw_GetTypeInfoCount;
        vtbl->methods[4] = (pfnGWMethod)univgw_GetTypeInfo;
        vtbl->methods[5] = (pfnGWMethod)univgw_GetIDsOfNames;
        vtbl->methods[6] = (pfnGWMethod)univgw_Invoke;
    }

    // add the methods. NOTE: 0..2 are the constant IUnknown methods
    int i;
    for (i = vtbl->cMethod - numReservedVtables; i--;) {
        PyObject *obArgSize = PySequence_GetItem(methods, i);
        if (obArgSize == NULL)
            goto error;

        PyObject *obArgCount = PySequence_GetItem(methodsArgc, i);
        if (obArgCount == NULL)
            goto error;

        int argSize = PyInt_AsLong(obArgSize);
        Py_DECREF(obArgSize);
        if (argSize == -1 && PyErr_Occurred())
            goto error;

        int argCount = PyInt_AsLong(obArgCount);
        Py_DECREF(obArgCount);
        if (argCount == -1 && PyErr_Occurred())
            goto error;
        // dynamically construct a function with the provided argument
        // size; reserve additional space for the _this argument.
        pfnGWMethod meth = make_method(i, argSize + sizeof(void *), argCount + 1);
        if (meth == NULL)
            goto error;

        vtbl->methods[i + numReservedVtables] = meth;
    }
    Py_DECREF(methods);
    Py_DECREF(methodsArgc);

    DWORD oldprotect;
    if (!VirtualProtect(vtbl, size, PAGE_EXECUTE, &oldprotect)) {
        free_vtbl(vtbl);
        PyErr_SetString(PyExc_RuntimeError, "failed to set memory attributes to executable");
        goto error;
    }

    PyObject *result;
    result = PyVTable_Create(vtbl);
    if (result == NULL) {
        free_vtbl(vtbl);
        return NULL;
    }
    return result;

error:
    Py_DECREF(methods);
    free_vtbl(vtbl);
    return NULL;
}

// Does all of the heavy lifting...
// Returns the created vtable pointer.
static IUnknown *CreateTearOff(PyObject *obInstance, PyGatewayBase *gatewayBase, PyObject *obVTable)
{
    gw_vtbl *vtbl = PyVTable_Get(obVTable);
    if (!vtbl)
        return NULL;

    if (vtbl->magic != GW_VTBL_MAGIC) {
        PyErr_SetString(PyExc_ValueError, "argument does not contain a vtable");
        return NULL;
    }

    // construct a C++ object (a block of mem with a vtbl)
    gw_object *punk = (gw_object *)malloc(sizeof(gw_object));
    if (punk == NULL) {
        PyErr_NoMemory();
        return NULL;
    }
    punk->vtbl = vtbl->methods;
    punk->punk = (IInternalUnwrapPythonObject *)gatewayBase;
    punk->punk->AddRef();
    punk->obVTable = obVTable;
    Py_INCREF(obVTable);
    // we start with one reference (the object we return)
    punk->cRef = 1;
    return (IUnknown *)punk;
}

static PyObject *univgw_CreateTearOff(PyObject *self, PyObject *args)
{
    PyObject *obInstance;
    PyObject *obVTable;
    PyObject *obIID = NULL;
    IUnknown *punk = NULL;
    if (!PyArg_ParseTuple(args, "OO|O:CreateTearOff", &obInstance, &obVTable, &obIID))
        return NULL;

    IID iidInterface = IID_IUnknown;  // what PyI* to wrap with
    if (obIID && obIID != Py_None)
        if (!PyWinObject_AsIID(obIID, &iidInterface))
            return NULL;

    // obVTable must be a CObject containing our vtbl ptr
    if (!PyVTable_Check(obVTable)) {
        PyErr_SetString(PyExc_ValueError, "argument is not a CObject/vtable");
        return NULL;
    }

    PyGatewayBase *base = NULL;
    PY_INTERFACE_PRECALL;
    HRESULT hr = PyCom_MakeRegisteredGatewayObject(IID_IUnknown, obInstance, NULL, (void **)&base);
    PY_INTERFACE_POSTCALL;
    if (FAILED(hr)) {
        PyCom_BuildPyException(hr);
        return NULL;
    }

    // Do all of the grunt work.
    punk = CreateTearOff(obInstance, base, obVTable);
    PYCOM_RELEASE(base)
    if (!punk)
        return NULL;

    // Convert to a PyObject.
    return PyCom_PyObjectFromIUnknown((IUnknown *)punk, iidInterface, FALSE);
}

// This is the function that gets called for creating
// registered PythonCOM gateway objects.
static HRESULT CreateRegisteredTearOff(PyObject *pPyInstance, PyGatewayBase *base, void **ppResult, REFIID iid)
{
    if (ppResult == NULL || pPyInstance == NULL) {
        return E_POINTER;
    }

    // Lookup vtable using iid.
    PyObject *obIID = PyWinObject_FromIID(iid);
    if (!obIID)
        return PyCom_SetCOMErrorFromPyException(iid);
    PyObject *obVTable = PyDict_GetItem(g_obRegisteredVTables, obIID);  // NOTE: NO reference added to obVTable
    Py_DECREF(obIID);
    if (!obVTable) {
        OLECHAR oleRes[128];
        StringFromGUID2(iid, oleRes, sizeof(oleRes));
        printf("Couldn't find IID %S\n", oleRes);
        // This should never happen....
        assert(FALSE);
        return E_NOINTERFACE;
    }

    // obVTable must be a CObject containing our vtbl ptr
    if (!PyVTable_Check(obVTable)) {
        assert(FALSE);
        return E_NOINTERFACE;
    }
    // If the base object is NULL, we must create one now.
    // Our base _must_ be a PyGateway to keep identity rules etc.
    BOOL bCreatedBase = FALSE;
    if (base == NULL) {
        PY_INTERFACE_PRECALL;
        HRESULT hr = PyCom_MakeRegisteredGatewayObject(IID_IUnknown, pPyInstance, NULL, (void **)&base);
        PY_INTERFACE_POSTCALL;
        if (FAILED(hr))
            return hr;
        bCreatedBase = TRUE;
    }

    // Do all of the grunt work.
    *ppResult = CreateTearOff(pPyInstance, base, obVTable);
    if (bCreatedBase) {
        PYCOM_RELEASE(base);
    }
    if (*ppResult == NULL)
        return E_FAIL;
    return S_OK;
}

static PyObject *univgw_RegisterVTable(PyObject *self, PyObject *args)
{
    IID iid;
    PyObject *obVTable;
    PyObject *obIID;
    char *pszInterfaceNm = NULL;
    PyObject *ret = NULL;

    if (!PyArg_ParseTuple(args, "OOs:RegisterVTable", &obVTable, &obIID, &pszInterfaceNm))
        return NULL;

    // obVTable must be a CObject containing our vtbl ptr
    if (!PyVTable_Check(obVTable)) {
        PyErr_SetString(PyExc_ValueError, "argument is not a CObject/vtable");
        return NULL;
    }

    if (!PyWinObject_AsIID(obIID, &iid)) {
        PyErr_SetString(PyExc_ValueError, "argument is not an IID");
        return NULL;
    }
    // obIID may be a string, but we need it to be a real PyIID.
    PyObject *keyObject = PyWinObject_FromIID(iid);

    if (0 != PyDict_SetItem(g_obRegisteredVTables, keyObject, obVTable)) {
        goto done;
    }

    if (!PyCom_IsGatewayRegistered(iid)) {
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

static PyObject *univgw_ReadMemory(PyObject *self, PyObject *args)
{
    PyObject *obPtr;
    int size;
    if (!PyArg_ParseTuple(args, "Oi:ReadMemory", &obPtr, &size))
        return NULL;

    void *p = PyLong_AsVoidPtr(obPtr);
    if (p == NULL && PyErr_Occurred()) {
        // reset the error to something meaningful
        PyErr_SetString(PyExc_ValueError, "argument is not an integer");
        return NULL;
    }

    return PyString_FromStringAndSize((char *)p, size);
}

static PyObject *univgw_WriteMemory(PyObject *self, PyObject *args)
{
    PyObject *obPtr;
    void *pSrc;
    int size;
    if (!PyArg_ParseTuple(args, "Os#:WriteMemory", &obPtr, &pSrc, &size))
        return NULL;

    void *pDst = PyLong_AsVoidPtr(obPtr);
    if (pDst == NULL && PyErr_Occurred()) {
        // reset the error to something meaningful
        PyErr_SetString(PyExc_ValueError, "argument is not an integer");
        return NULL;
    }

    memcpy(pDst, pSrc, size);

    Py_INCREF(Py_None);
    return Py_None;
}

static struct PyMethodDef univgw_functions[] = {
    {"CreateVTable", univgw_CreateVTable, 1},
    {"CreateTearOff", univgw_CreateTearOff, 1},
    {"ReadMemory", univgw_ReadMemory, 1},
    {"WriteMemory", univgw_WriteMemory, 1},
    {"RegisterVTable", univgw_RegisterVTable, 1},

    {"L64", dataconv_L64, 1},
    {"UL64", dataconv_UL64, 1},
    {"strL64", dataconv_strL64, 1},
    {"strUL64", dataconv_strUL64, 1},
    {"interface", dataconv_interface, 1},
    {"SizeOfVT", dataconv_SizeOfVT, 1},
    {"WriteFromOutTuple", dataconv_WriteFromOutTuple, 1},
    {"ReadFromInTuple", dataconv_ReadFromInTuple, 1},

    {NULL} /* sentinel */
};

BOOL initunivgw(PyObject *parentDict)
{
    //	HRESULT hr;

    PyObject *module;

#if (PY_VERSION_HEX < 0x03000000)
    module = Py_InitModule("pythoncom.__univgw", univgw_functions);
#else
    static PyModuleDef univgw_def = {PyModuleDef_HEAD_INIT, "pythoncom.__univgw", "Univeral gateway", -1,
                                     univgw_functions};
    module = PyModule_Create(&univgw_def);
#endif
    if (!module) /* Eeek - some serious error! */
        return FALSE;

    //	PyObject *dict = PyModule_GetDict(module);
    //	if (!dict) return; /* Another serious error!*/

    g_obRegisteredVTables = PyDict_New();

    PyDict_SetItemString(parentDict, "_univgw", module);

    return TRUE;
}
