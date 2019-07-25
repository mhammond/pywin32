// SSPI objects for the win32security module, by Roger Upole
// $Id$

// Currently this file contains only object definitions - the SSPI methods
// themselves are defined in win32security.i.  In the future this may change.
// @doc - This file contains autoduck documentation
#include "PyWinTypes.h"
#include "structmember.h"
#include "PyWinObjects.h"
#include "PySecurityObjects.h"
#include "win32security_sspi.h"
#include "schannel.h"

////////////////////////////////////////////////////////////////////////
//
// PySecBufferDesc
//
////////////////////////////////////////////////////////////////////////
Py_ssize_t PySecBufferDesc_sq_length(PyObject *self)
{
    PSecBufferDesc psecbufferdesc = ((PySecBufferDesc *)self)->GetSecBufferDesc();
    return psecbufferdesc->cBuffers;
}

PyObject *PySecBufferDesc_sq_item(PyObject *self, Py_ssize_t i)
{
    PySecBufferDesc *This = (PySecBufferDesc *)self;
    PSecBufferDesc psecbufferdesc = This->GetSecBufferDesc();
    if ((ULONG)i >= psecbufferdesc->cBuffers) {
        PyErr_SetString(PyExc_IndexError, "Index specified larger than number of allocated buffers");
        return NULL;
    }
    Py_INCREF(This->obBuffers[i]);
    return This->obBuffers[i];
}

int PySecBufferDesc_sq_ass_item(PyObject *self, Py_ssize_t i, PyObject *ob)
{
    if (ob == NULL) {
        PyErr_SetString(PyExc_NotImplementedError, "Removing buffers not yet supported");
        return -1;
    }
    PSecBuffer psecbuffer;
    PySecBufferDesc *This = (PySecBufferDesc *)self;
    PSecBufferDesc psecbufferdesc = This->GetSecBufferDesc();
    if ((ULONG)i >= psecbufferdesc->cBuffers) {
        PyErr_Format(PyExc_IndexError, "PySecBufferDesc contains %d buffers", psecbufferdesc->cBuffers);
        return -1;
    }
    if (!PyWinObject_AsSecBuffer(ob, &psecbuffer, FALSE))
        return -1;
    Py_XDECREF(This->obBuffers[i]);
    Py_INCREF(ob);
    This->obBuffers[i] = ob;
    psecbufferdesc->pBuffers[i] = *psecbuffer;
    return 0;
}

PySequenceMethods PySecBufferDesc_sequencemethods = {
    PySecBufferDesc_sq_length,    // inquiry sq_length;
    NULL,                         // binaryfunc sq_concat;
    NULL,                         // intargfunc sq_repeat;
    PySecBufferDesc_sq_item,      // intargfunc sq_item;
    NULL,                         // intintargfunc sq_slice;
    PySecBufferDesc_sq_ass_item,  // intobjargproc sq_ass_item;;
    NULL,                         // intintobjargproc sq_ass_slice;
    NULL,                         // objobjproc sq_contains;
    NULL,                         // binaryfunc sq_inplace_concat;
    NULL                          // intargfunc sq_inplace_repeat;
};                                // ??? why isnt append included ???

// @object PySecBufferDesc|Sequence-like object that contains a group of buffers to be used with SSPI functions.
// @comm This object is created using win32security.PySecBufferDescType(Version), where Version is an int that
// defaults to SECBUFFER_VERSION if not passed in.
struct PyMethodDef PySecBufferDesc::methods[] = {
    {"append", PySecBufferDesc::append, 1},  // @pymeth append|Adds a <o PySecBuffer> to the list of buffers
    {NULL}};

#define OFF(e) offsetof(PySecBufferDesc, e)
struct PyMemberDef PySecBufferDesc::members[] = {
    {"Version", T_ULONG, OFF(secbufferdesc.ulVersion), 0, "Currently should always be SECBUFFER_VERSION"}, {NULL}};

PyTypeObject PySecBufferDescType = {
    PYWIN_OBJECT_HEAD "PySecBufferDesc",
    sizeof(PySecBufferDesc),
    0,
    PySecBufferDesc::deallocFunc,              // tp_dealloc
    0,                                         // tp_print
    0,                                         // tp_getattr
    0,                                         // tp_setattr
    0,                                         // tp_compare
    0,                                         // tp_repr
    0,                                         // PyNumberMethods *tp_as_number
    &PySecBufferDesc_sequencemethods,          // PySequenceMethods *tp_as_sequence
    0,                                         // PyMappingMethods *tp_as_mapping
    0,                                         // hashfunc tp_hash
    0,                                         // tp_call
    0,                                         // tp_str
    PyObject_GenericGetAttr,                   // tp_getattro
    PyObject_GenericSetAttr,                   // tp_setattro
    0,                                         // PyBufferProcs *tp_as_buffer
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,  // tp_flags
    0,                                         // tp_doc
    0,                                         // traverseproc tp_traverse
    0,                                         // tp_clear
    0,                                         // richcmpfunc tp_richcompare
    0,                                         // tp_weaklistoffset
    0,                                         // getiterfunc tp_iter
    0,                                         // iternextfunc tp_iternext
    PySecBufferDesc::methods,
    PySecBufferDesc::members,
    0,                       // tp_getset;
    0,                       // tp_base;
    0,                       // tp_dict;
    0,                       // tp_descr_get
    0,                       // tp_descr_set
    0,                       // tp_dictoffset
    0,                       // tp_init
    0,                       // tp_alloc
    PySecBufferDesc::tp_new  // newfunc tp_new;
};

// InitializeSecurityContext can allocate output buffers if flag ISC_REQ_ALLOCATE_MEMORY is set
// Untested !!!!!!
PySecBufferDesc::PySecBufferDesc(PSecBufferDesc psecbufferdesc)
{
    ob_type = &PySecBufferDescType;
    secbufferdesc = *psecbufferdesc;
    secbufferdesc.pBuffers = (PSecBuffer)malloc(psecbufferdesc->cBuffers * sizeof(SecBuffer));
    obBuffers = (PyObject **)malloc(psecbufferdesc->cBuffers * sizeof(PyObject *));
    if (obBuffers != NULL)
        for (ULONG i = 0; i < psecbufferdesc->cBuffers; i++)
            obBuffers[i] = PyWinObject_FromSecBuffer(&psecbufferdesc->pBuffers[i]);
    _Py_NewReference(this);
}

PySecBufferDesc::PySecBufferDesc(ULONG ulVersion)
{
    DWORD bufsize;
    ob_type = &PySecBufferDescType;
    secbufferdesc.ulVersion = ulVersion;
    secbufferdesc.cBuffers = 0;
    max_buffers = 5;

    bufsize = max_buffers * sizeof(PyObject *);
    obBuffers = (PyObject **)malloc(bufsize);
    if (obBuffers == NULL)
        PyErr_Format(PyExc_MemoryError, "PySecBufferDesc: unable to allocate %d PyObject pointers (%d bytes)",
                     max_buffers, bufsize);
    else
        ZeroMemory(obBuffers, bufsize);

    bufsize = max_buffers * sizeof(SecBuffer);
    secbufferdesc.pBuffers = (PSecBuffer)malloc(bufsize);
    if (obBuffers == NULL)
        PyErr_Format(PyExc_MemoryError, "PySecBufferDesc: unable to allocate %d SecBuffer's (%d bytes)", max_buffers,
                     bufsize);
    else
        ZeroMemory(secbufferdesc.pBuffers, bufsize);

    _Py_NewReference(this);
}

PySecBufferDesc::~PySecBufferDesc()
{
    if (secbufferdesc.pBuffers != NULL)
        free(secbufferdesc.pBuffers);
    for (ULONG buf_ind = 0; buf_ind < secbufferdesc.cBuffers; buf_ind++) Py_XDECREF(obBuffers[buf_ind]);
    if (obBuffers != NULL)
        free(obBuffers);
}

BOOL PySecBufferDesc_Check(PyObject *ob)
{
    if (ob->ob_type != &PySecBufferDescType) {
        PyErr_SetString(PyExc_TypeError, "Object must be a PySecBufferDesc");
        return FALSE;
    }
    return TRUE;
}

void PySecBufferDesc::deallocFunc(PyObject *ob) { delete (PySecBufferDesc *)ob; }

PSecBufferDesc PySecBufferDesc::GetSecBufferDesc(void) { return &secbufferdesc; }

PyObject *PySecBufferDesc::tp_new(PyTypeObject *typ, PyObject *args, PyObject *kwargs)
{
    ULONG ulVersion = SECBUFFER_VERSION;
    static char *keywords[] = {"Version", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|l:PySecBufferDescType", keywords, &ulVersion))
        return NULL;
    return new PySecBufferDesc(ulVersion);
}

BOOL PyWinObject_AsSecBufferDesc(PyObject *ob, PSecBufferDesc *ppSecBufferDesc, BOOL bNoneOk)
{
    if (ob == Py_None) {
        if (bNoneOk) {
            *ppSecBufferDesc = NULL;
            return TRUE;
        }
        PyErr_SetString(PyExc_ValueError, "PySecBufferDesc cannot be None in this context");
        return FALSE;
    }
    if (!PySecBufferDesc_Check(ob))
        return FALSE;
    *ppSecBufferDesc = ((PySecBufferDesc *)ob)->GetSecBufferDesc();
    // move any changes made to the individual PySecBuffer objects back into the SecBufferDesc array
    for (ULONG i = 0; i < (*ppSecBufferDesc)->cBuffers; i++)
        (*ppSecBufferDesc)->pBuffers[i] = *((PySecBuffer *)((PySecBufferDesc *)ob)->obBuffers[i])->GetSecBuffer();
    return TRUE;
}

// @pymethod |PySecBufferDesc|append|Adds a <o PySecBuffer> to the buffer configuration
PyObject *PySecBufferDesc::append(PyObject *self, PyObject *args)
{
    PyObject *ob;
    PSecBuffer psecbuffer;
    PSecBuffer pbufsave;
    PyObject **obbufsave;
    PySecBufferDesc *This = (PySecBufferDesc *)self;
    PSecBufferDesc psecbufferdesc = This->GetSecBufferDesc();
    if (!PyArg_ParseTuple(args, "O", &ob))
        return NULL;
    // @pyparm |buffer||<o PySecBuffer> object to be attached to the group of buffers
    if (!PyWinObject_AsSecBuffer(ob, &psecbuffer, FALSE))
        return NULL;
    // make sure consistent internal state can be restored if allocations fail
    pbufsave = psecbufferdesc->pBuffers;
    obbufsave = This->obBuffers;
    psecbufferdesc->cBuffers++;
    if (psecbufferdesc->cBuffers > This->max_buffers) {
        psecbufferdesc->pBuffers =
            (PSecBuffer)realloc(psecbufferdesc->pBuffers, psecbufferdesc->cBuffers * sizeof(SecBuffer));
        This->obBuffers = (PyObject **)realloc(This->obBuffers, psecbufferdesc->cBuffers * sizeof(PyObject *));
        if ((psecbufferdesc->pBuffers == NULL) || (This->obBuffers == NULL)) {
            PyErr_SetString(PyExc_MemoryError, "Unable to reallocate interal PySecBufferDesc structures");
            psecbufferdesc->cBuffers--;
            psecbufferdesc->pBuffers = pbufsave;
            This->obBuffers = obbufsave;
            return NULL;
        }
        This->max_buffers++;
    }
    // keep reference to PySecBuffers that contain the actual allocated buffers so they can be kept in sync
    psecbufferdesc->pBuffers[psecbufferdesc->cBuffers - 1] = *psecbuffer;
    This->obBuffers[psecbufferdesc->cBuffers - 1] = ob;
    Py_INCREF(ob);
    Py_INCREF(Py_None);
    return Py_None;
}

// propagate changes back to PySecBuffer objects that constitute the actual allocated buffers
void PySecBufferDesc::modify_in_place(void)
{
    for (ULONG i = 0; i < secbufferdesc.cBuffers; i++)
        *((PySecBuffer *)obBuffers[i])->GetSecBuffer() = secbufferdesc.pBuffers[i];
}

// ??? don't actually use this anywhere yet, but some protocols can allocate buffers for you (ISC_REQ_ALLOCATE_MEMORY)
// Check for this flag in ContextReq and an output SecBufferDesc with 0 buffers initially and non-zero after API call ?
// How to specify different deallocation (FreeContextBuffer) when object is destroyed ?
PyObject *PyWinObject_FromSecBufferDesc(PSecBufferDesc pSecBufferDesc)
{
    if (pSecBufferDesc == NULL) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    return new PySecBufferDesc(pSecBufferDesc);
}

////////////////////////////////////////////////////////////////////////
//
// PySecBuffer
//
////////////////////////////////////////////////////////////////////////

// @object PySecBuffer|Python object wrapping a SecBuffer structure
//  Created using win32security.PySecBufferType(type,size) where type is a SECBUFFER_* constant
struct PyMethodDef PySecBuffer::methods[] = {
    {"Clear", PySecBuffer::Clear, 1},  // @pymeth Clear|Resets all members of the structure
    {NULL}};

#undef OFF
#define OFF(e) offsetof(PySecBuffer, e)
struct PyMemberDef PySecBuffer::members[] = {
    // @prop int|BufferType|
    {"BufferType", T_ULONG, OFF(secbuffer.BufferType), 0,
     "Type of buffer, one of the SECBUFFER_* constants -  can also be combined with SECBUFFER_READONLY"},
    // @prop string|Buffer|
    {"Buffer", T_OBJECT, OFF(obdummy), 0, "Encoded data buffer"},
    // @prop int|BufferSize|
    {"BufferSize", T_ULONG, OFF(secbuffer.cbBuffer), 0, "Current size of data in buffer"},
    // @prop int|MaxBufferSize|
    {"MaxBufferSize", T_ULONG, OFF(maxbufsize), READONLY, "Maximum size of data buffer"},
    {NULL}};

PyTypeObject PySecBufferType = {
    PYWIN_OBJECT_HEAD "PySecBuffer",
    sizeof(PySecBuffer),
    0,
    PySecBuffer::deallocFunc,                  // tp_dealloc
    0,                                         // tp_print
    0,                                         // tp_getattr
    0,                                         // tp_setattr
    0,                                         // tp_compare
    0,                                         // tp_repr
    0,                                         // PyNumberMethods *tp_as_number
    0,                                         // PySequenceMethods *tp_as_sequence
    0,                                         // PyMappingMethods *tp_as_mapping
    0,                                         // hashfunc tp_hash
    0,                                         // tp_call
    0,                                         // tp_str
    PySecBuffer::getattro,                     // tp_getattro
    PySecBuffer::setattro,                     // tp_setattro
    0,                                         // PyBufferProcs *tp_as_buffer
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,  // tp_flags
    0,                                         // tp_doc
    0,                                         // traverseproc tp_traverse
    0,                                         // tp_clear
    0,                                         // richcmpfunc tp_richcompare
    0,                                         // tp_weaklistoffset
    0,                                         // getiterfunc tp_iter
    0,                                         // iternextfunc tp_iternext
    PySecBuffer::methods,
    PySecBuffer::members,
    0,                   // tp_getset;
    0,                   // tp_base;
    0,                   // tp_dict;
    0,                   // tp_descr_get
    0,                   // tp_descr_set
    0,                   // tp_dictoffset
    0,                   // tp_init
    0,                   // tp_alloc
    PySecBuffer::tp_new  // newfunc tp_new;
};

PySecBuffer::PySecBuffer(PSecBuffer psecbuffer)
{
    maxbufsize = secbuffer.cbBuffer;
    ob_type = &PySecBufferType;
    secbuffer = *psecbuffer;
    secbuffer.pvBuffer = malloc(psecbuffer->cbBuffer);
    if (secbuffer.pvBuffer == NULL)
        PyErr_Format(PyExc_MemoryError, "PySecBuffer::PySecBuffer - cannot allocate buffer of %d bytes",
                     psecbuffer->cbBuffer);
    else
        memcpy(secbuffer.pvBuffer, psecbuffer->pvBuffer, psecbuffer->cbBuffer);
    _Py_NewReference(this);
}

PySecBuffer::PySecBuffer(ULONG cbBuffer, ULONG BufferType)
{
    obdummy = NULL;
    maxbufsize = cbBuffer;
    ob_type = &PySecBufferType;
    secbuffer.cbBuffer = cbBuffer;
    secbuffer.BufferType = BufferType;
    secbuffer.pvBuffer = malloc(cbBuffer);
    // Any code that creates instances should check that buffer is not NULL !
    if (secbuffer.pvBuffer == NULL)
        PyErr_Format(PyExc_MemoryError, "PySecBuffer::PySecBuffer - cannot allocate buffer of %d bytes", cbBuffer);
    else
        ZeroMemory(secbuffer.pvBuffer, cbBuffer);
    _Py_NewReference(this);
}

PySecBuffer::~PySecBuffer()
{
    if (secbuffer.pvBuffer != NULL)
        free(secbuffer.pvBuffer);
}

BOOL PySecBuffer_Check(PyObject *ob)
{
    if (ob->ob_type != &PySecBufferType) {
        PyErr_SetString(PyExc_TypeError, "Object must be a PySecBuffer");
        return FALSE;
    }
    return TRUE;
}

void PySecBuffer::deallocFunc(PyObject *ob) { delete (PySecBuffer *)ob; }

PSecBuffer PySecBuffer::GetSecBuffer(void) { return &secbuffer; }

PyObject *PySecBuffer::getattro(PyObject *self, PyObject *obname)
{
    PSecBuffer psecbuffer = ((PySecBuffer *)self)->GetSecBuffer();
    char *name = PYWIN_ATTR_CONVERT(obname);
    if (name == NULL)
        return NULL;
    if (strcmp(name, "Buffer") == 0)
        return PyString_FromStringAndSize((char *)psecbuffer->pvBuffer, psecbuffer->cbBuffer);
    return PyObject_GenericGetAttr(self, obname);
}

int PySecBuffer::setattro(PyObject *self, PyObject *obname, PyObject *obvalue)
{
    PySecBuffer *This = (PySecBuffer *)self;
    char *name;
    void *value;
    DWORD valuelen;
    name = PYWIN_ATTR_CONVERT(obname);
    if (name == NULL)
        return -1;
    if (strcmp(name, "Buffer") == 0) {
        if (!PyWinObject_AsReadBuffer(obvalue, &value, &valuelen))
            return -1;
        PSecBuffer psecbuffer = This->GetSecBuffer();
        if (valuelen > This->maxbufsize) {
            PyErr_Format(PyExc_ValueError, "Data size (%d) greater than allocated buffer size (%d)", valuelen,
                         This->maxbufsize);
            return -1;
        }
        ZeroMemory(psecbuffer->pvBuffer, This->maxbufsize);
        memcpy(psecbuffer->pvBuffer, value, valuelen);
        // buffer length should be size of actual data, allocated size is kept in our own maxbufsize
        psecbuffer->cbBuffer = valuelen;
        return 0;
    }

    return PyObject_GenericSetAttr(self, obname, obvalue);
}

PyObject *PySecBuffer::tp_new(PyTypeObject *typ, PyObject *args, PyObject *kwargs)
{
    ULONG cbBuffer, BufferType;
    static char *keywords[] = {"BufferSize", "BufferType", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "ll", keywords, &cbBuffer, &BufferType))
        return NULL;
    return new PySecBuffer(cbBuffer, BufferType);
}

// @pymethod |PySecBuffer|Clear|Resets the buffer to all NULL's, and set the current size to maximum
PyObject *PySecBuffer::Clear(PyObject *self, PyObject *args)
{
    if (!PyArg_ParseTuple(args, ":Clear"))
        return NULL;
    PySecBuffer *This = (PySecBuffer *)self;
    PSecBuffer psecbuffer = This->GetSecBuffer();
    psecbuffer->cbBuffer = This->maxbufsize;
    ZeroMemory(psecbuffer->pvBuffer, psecbuffer->cbBuffer);
    Py_INCREF(Py_None);
    return Py_None;
}

BOOL PyWinObject_AsSecBuffer(PyObject *ob, PSecBuffer *psecbuffer, BOOL bNoneOk)
{
    if (!PySecBuffer_Check(ob))
        return FALSE;
    *psecbuffer = ((PySecBuffer *)ob)->GetSecBuffer();
    return TRUE;
}

PyObject *PyWinObject_FromSecBuffer(PSecBuffer psecbuffer) { return new PySecBuffer(psecbuffer); }

////////////////////////////////////////////////////////////////////////
//
// PyCtxtHandle
//
////////////////////////////////////////////////////////////////////////
// @object PyCtxtHandle|Security context handle, as used with sspi functions
// @comm Create using win32security.PyCtxtHandleType().  The handle must be initialized by passing it to
// <om win32security.InitializeSecurityContext> or <om win32security.AcceptSecurityContext>
struct PyMethodDef PyCtxtHandle::methods[] = {
    {"Detach", PyCtxtHandle::Detach,
     1},  // @pymeth Detach|Disassociates object from handle and returns integer value of handle
    {"CompleteAuthToken", PyCtxtHandle::CompleteAuthToken,
     1},  //@pymeth CompleteAuthToken|Completes the authentication token
    {"QueryContextAttributes", PyCtxtHandle::QueryContextAttributes,
     1},  // @pymeth QueryContextAttributes|Retrieves info about a security context
    {"DeleteSecurityContext", PyCtxtHandle::DeleteSecurityContext,
     1},  // @pymeth DeleteSecurityContext|Frees the security context and invalidates the handle
    {"QuerySecurityContextToken", PyCtxtHandle::QuerySecurityContextToken,
     1},  // @pymeth QuerySecurityContextToken|Returns the access token for a security context
    {"MakeSignature", PyCtxtHandle::MakeSignature, 1},  // @pymeth MakeSignature|Generates a signature for a message
    {"VerifySignature", PyCtxtHandle::VerifySignature,
     1},  // @pymeth VerifySignature|Verifies  a signature created using <om PyCtxtHandle.MakeSignature>
    {"EncryptMessage", PyCtxtHandle::EncryptMessage,
     1},  // @pymeth EncryptMessage|Encrypts data with security context's session key
    {"DecryptMessage", PyCtxtHandle::DecryptMessage,
     1},  // @pymeth DecryptMessage|Decrypts data encrypted by <om PyCtxtHandle.EncryptMessage>
    {"ImpersonateSecurityContext", PyCtxtHandle::ImpersonateSecurityContext,
     1},  // @pymeth ImpersonateSecurityContext|Causes a server to act in the security context of an authenticated
          // client
    {"RevertSecurityContext", PyCtxtHandle::RevertSecurityContext,
     1},  // @pymeth RevertSecurityContext|Stops impersonation of a client initiated by <om
          // PyCtxtHandle::ImpersonateSecurityContext>
    {NULL}};

PyTypeObject PyCtxtHandleType = {
    PYWIN_OBJECT_HEAD "PyCtxtHandle",
    sizeof(PyCtxtHandle),
    0,
    PyCtxtHandle::deallocFunc,                 // tp_dealloc
    0,                                         // tp_print
    0,                                         // tp_getattr
    0,                                         // tp_setattr
    0,                                         // tp_compare
    0,                                         // tp_repr
    0,                                         // PyNumberMethods *tp_as_number
    0,                                         // PySequenceMethods *tp_as_sequence
    0,                                         // PyMappingMethods *tp_as_mapping
    0,                                         // hashfunc tp_hash
    0,                                         // tp_call
    0,                                         // tp_str
    PyObject_GenericGetAttr,                   // tp_getattro
    PyObject_GenericSetAttr,                   // tp_setattro
    0,                                         // PyBufferProcs *tp_as_buffer
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,  // tp_flags
    0,                                         // tp_doc
    0,                                         // traverseproc tp_traverse
    0,                                         // tp_clear
    0,                                         // richcmpfunc tp_richcompare
    0,                                         // tp_weaklistoffset
    0,                                         // getiterfunc tp_iter
    0,                                         // iternextfunc tp_iternext
    PyCtxtHandle::methods,
    NULL,
    0,                    // tp_getset;
    0,                    // tp_base;
    0,                    // tp_dict;
    0,                    // tp_descr_get
    0,                    // tp_descr_set
    0,                    // tp_dictoffset
    0,                    // tp_init
    0,                    // tp_alloc
    PyCtxtHandle::tp_new  // newfunc tp_new;
};

PyCtxtHandle::PyCtxtHandle(PCtxtHandle pctxthandle)
{
    ob_type = &PyCtxtHandleType;
    ctxthandle = *pctxthandle;
    _Py_NewReference(this);
}

PyCtxtHandle::PyCtxtHandle(void)
{
    ob_type = &PyCtxtHandleType;
    SecInvalidateHandle(&ctxthandle);
    _Py_NewReference(this);
}

PyCtxtHandle::~PyCtxtHandle()
{
    if (SecIsValidHandle(&ctxthandle))
        (*psecurityfunctiontable->DeleteSecurityContext)(&ctxthandle);
}

BOOL PyCtxtHandle_Check(PyObject *ob)
{
    if (ob->ob_type != &PyCtxtHandleType) {
        PyErr_SetString(PyExc_TypeError, "Object must be a PyCtxtHandle");
        return FALSE;
    }
    return TRUE;
}

void PyCtxtHandle::deallocFunc(PyObject *ob) { delete (PyCtxtHandle *)ob; }

PCtxtHandle PyCtxtHandle::GetCtxtHandle(void) { return &ctxthandle; }

PyObject *PyCtxtHandle::tp_new(PyTypeObject *typ, PyObject *args, PyObject *kwargs) { return new PyCtxtHandle(); }

BOOL PyWinObject_AsCtxtHandle(PyObject *ob, PCtxtHandle *pctxthandle, BOOL bNoneOk)
{
    if (ob == Py_None) {
        if (bNoneOk) {
            *pctxthandle = NULL;
            return TRUE;
        }
        PyErr_SetString(PyExc_ValueError, "Context handle cannot be NULL");
        return FALSE;
    }
    if (!PyCtxtHandle_Check(ob))
        return FALSE;
    *pctxthandle = ((PyCtxtHandle *)ob)->GetCtxtHandle();
    return TRUE;
}

PyObject *PyWinObject_FromCtxtHandle(PCtxtHandle pctxthandle) { return new PyCtxtHandle(pctxthandle); }

// @pymethod |PyCtxtHandle|MakeSignature|Creates a crytographic hash of a message using session key of the security
// context
PyObject *PyCtxtHandle::MakeSignature(PyObject *self, PyObject *args)
{
    // @pyparm int|fqop||Flags that indicate quality of protection desired, specific to each security package
    // @pyparm <o PySecBufferDesc>|Message||Buffer set that includes buffers for input data and output signature
    // @pyparm int|MessageSeqNo||A sequential number used by some packages to verify that no extraneous messages have
    // been received
    // @rdesc Returns None on success, and output buffer in Message will contain the signature
    // @comm The buffer configuration is dependent on the security package.  Usually there is one input buffer of
    //  type SECBUFFER_DATA and an output buffer of type SECBUFFER_TOKEN
    SECURITY_STATUS err;
    PyObject *obdesc;
    PSecBufferDesc psecbufferdesc;
    ULONG fqop, seq_no;
    CHECK_SECURITYFUNCTIONTABLE(MakeSignature);

    if (!PyArg_ParseTuple(args, "lOl:MakeSignature", &fqop, &obdesc, &seq_no))
        return NULL;
    if (!PyWinObject_AsSecBufferDesc(obdesc, &psecbufferdesc, FALSE))
        return NULL;
    PCtxtHandle pctxt = ((PyCtxtHandle *)self)->GetCtxtHandle();
    Py_BEGIN_ALLOW_THREADS err = (*psecurityfunctiontable->MakeSignature)(pctxt, fqop, psecbufferdesc, seq_no);
    Py_END_ALLOW_THREADS if (err < 0)
    {
        PyWin_SetAPIError("MakeSignature", err);
        return NULL;
    }
    // copy changes in buffers back to individual PySecBuffer objects
    ((PySecBufferDesc *)obdesc)->modify_in_place();
    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod |PyCtxtHandle|VerifySignature|Verifies a signature created using <om PyCtxtHandle.MakeSignature>
PyObject *PyCtxtHandle::VerifySignature(PyObject *self, PyObject *args)
{
    // @pyparm <o PySecBufferDesc>|Message||SecBufferDesc that contains data buffer and signature buffer
    // @pyparm int|MessageSeqNo||A sequential number used by some packages to verify that no extraneous messages have
    // been received
    // @rdesc Returns quality of protection flags used to create signature
    // @comm The buffer configuration is dependent on the security package.  Usually there is a data buffer of type
    // SECBUFFER_DATA
    //   and a signature buffer of type SECBUFFER_TOKEN

    SECURITY_STATUS err;
    PyObject *obdesc;
    PSecBufferDesc psecbufferdesc;
    ULONG fqop, seq_no;
    CHECK_SECURITYFUNCTIONTABLE(VerifySignature);
    if (!PyArg_ParseTuple(args, "Ol:VerifySignature", &obdesc, &seq_no))
        return NULL;

    if (!PyWinObject_AsSecBufferDesc(obdesc, &psecbufferdesc, FALSE))
        return NULL;
    PCtxtHandle pctxt = ((PyCtxtHandle *)self)->GetCtxtHandle();
    err = (*psecurityfunctiontable->VerifySignature)(pctxt, psecbufferdesc, seq_no, &fqop);
    if (err == SEC_E_OK)
        return Py_BuildValue("l", fqop);
    PyWin_SetAPIError("VerifySignature", err);
    return NULL;
}

// @pymethod |PyCtxtHandle|EncryptMessage|Encrypts data with session key of security context
PyObject *PyCtxtHandle::EncryptMessage(PyObject *self, PyObject *args)
{
    // @pyparm int|fqop||Flags that indicate quality of protection desired, specific to each security package
    // @pyparm <o PySecBufferDesc>|Message||<o PySecBufferDesc> that contains data buffer(s) to be encrypted
    // @pyparm int|MessageSeqNo||A sequential number used by some packages to verify that no extraneous messages have
    // been received
    // @rdesc Returns None on success, and buffer(s) will contain encrypted data
    // @comm The buffer configuration is dependent on the security package.  Usually there is one input buffer
    //  of type SECBUFFER_DATA to be encrypted in-place and another empty buffer of type SECBUFFER_PADDING or
    //  SECBUFFER_TOKEN to receive signature or padding data
    SECURITY_STATUS err;
    PyObject *obdesc;
    PSecBufferDesc psecbufferdesc;
    ULONG fqop, seq_no;
    CHECK_SECURITYFUNCTIONTABLE(EncryptMessage);

    if (!PyArg_ParseTuple(args, "lOl:EncryptMessage", &fqop, &obdesc, &seq_no))
        return NULL;

    if (!PyWinObject_AsSecBufferDesc(obdesc, &psecbufferdesc, FALSE))
        return NULL;
    PCtxtHandle pctxt = ((PyCtxtHandle *)self)->GetCtxtHandle();
    Py_BEGIN_ALLOW_THREADS err = (*psecurityfunctiontable->EncryptMessage)(pctxt, fqop, psecbufferdesc, seq_no);
    Py_END_ALLOW_THREADS if (err < 0)
    {
        PyWin_SetAPIError("EncryptMessage", err);
        return NULL;
    }
    // copy changes in buffers back to individual PySecBuffer objects
    ((PySecBufferDesc *)obdesc)->modify_in_place();
    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod |PyCtxtHandle|DecryptMessage|Decrypts data produced by <om PyCtxtHandle.EncryptMessage>
PyObject *PyCtxtHandle::DecryptMessage(PyObject *self, PyObject *args)
{
    // @pyparm <o PySecBufferDesc>|Message||<o PySecBufferDesc> containing data buffers to be decrypted
    // @pyparm int|MessageSeqNo||A sequential number used by some packages to verify that no extraneous messages have
    // been received
    // @rdesc Returns flags specfic to security package indicating quality of protection
    // @comm The buffer configuration is dependent on the security package.  Usually there is one buffer
    //  of type SECBUFFER_DATA which is modified in place and a second buffer of type SECBUFFER_TOKEN or
    //  SECBUFFER_PADDING containing signature, padding, or other extra data from encryption process that doesn't fit
    //  in first buffer
    SECURITY_STATUS err;
    PyObject *obdesc;
    PSecBufferDesc psecbufferdesc;
    ULONG fqop, seq_no;
    CHECK_SECURITYFUNCTIONTABLE(DecryptMessage);
    if (!PyArg_ParseTuple(args, "Ol:DecryptMessage", &obdesc, &seq_no))
        return NULL;

    if (!PyWinObject_AsSecBufferDesc(obdesc, &psecbufferdesc, FALSE))
        return NULL;
    PCtxtHandle pctxt = ((PyCtxtHandle *)self)->GetCtxtHandle();
    Py_BEGIN_ALLOW_THREADS err = (*psecurityfunctiontable->DecryptMessage)(pctxt, psecbufferdesc, seq_no, &fqop);
    Py_END_ALLOW_THREADS((PySecBufferDesc *)obdesc)->modify_in_place();
    if (err == SEC_E_OK)
        return Py_BuildValue("l", fqop);
    PyWin_SetAPIError("DecryptMessage", err);
    return NULL;
}

// @pymethod long|PyCtxtHandle|Detach|Disassociates object from handle and returns integer value of handle
// @comm Use when the security context needs to persist beyond the lifetime of the Python object
PyObject *PyCtxtHandle::Detach(PyObject *self, PyObject *args)
{
    if (!PyArg_ParseTuple(args, ":Detach"))
        return NULL;
    PyCtxtHandle *This = (PyCtxtHandle *)self;
    PCtxtHandle pctxthandle = This->GetCtxtHandle();
    PyObject *ret = PyWinObject_FromSecHandle(pctxthandle);
    if (ret != NULL)
        SecInvalidateHandle(pctxthandle);
    return ret;
}

// @pymethod |PyCtxtHandle|DeleteSecurityContext|Frees the security context and invalidates the handle
PyObject *PyCtxtHandle::DeleteSecurityContext(PyObject *self, PyObject *args)
{
    CHECK_SECURITYFUNCTIONTABLE(DeleteSecurityContext);
    if (!PyArg_ParseTuple(args, ":DeleteSecurityContext"))
        return NULL;
    PyCtxtHandle *This = (PyCtxtHandle *)self;
    PCtxtHandle pctxt = This->GetCtxtHandle();
    SECURITY_STATUS err = (*psecurityfunctiontable->DeleteSecurityContext)(pctxt);
    if (err != SEC_E_OK) {
        PyWin_SetAPIError("DeleteSecurityContext", err);
        return NULL;
    }
    SecInvalidateHandle(pctxt);
    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod |PyCtxtHandle|CompleteAuthToken|Completes the authentication token
// @comm This method should be invoked on a context handle if the InitializeSecurityContext call that created it
//   returned SEC_I_COMPLETE_NEEDED or SEC_I_COMPLETE_AND_CONTINUE
PyObject *PyCtxtHandle::CompleteAuthToken(PyObject *self, PyObject *args)
{
    PCtxtHandle pctxt;
    PSecBufferDesc psecbufferdesc;
    PyObject *obsecbufferdesc;
    SECURITY_STATUS err;
    CHECK_SECURITYFUNCTIONTABLE(CompleteAuthToken);
    // @pyparm <o PySecBufferDesc>|Token||The buffer that contains the token buffer used when the context was
    // initialized
    if (!PyArg_ParseTuple(args, "O:CompleteAuthToken", &obsecbufferdesc))
        return NULL;
    if (!PyWinObject_AsSecBufferDesc(obsecbufferdesc, &psecbufferdesc, FALSE))
        return NULL;
    pctxt = ((PyCtxtHandle *)self)->GetCtxtHandle();
    Py_BEGIN_ALLOW_THREADS err = (*psecurityfunctiontable->CompleteAuthToken)(pctxt, psecbufferdesc);
    Py_END_ALLOW_THREADS if (err == SEC_E_OK)
    {
        Py_INCREF(Py_None);
        return Py_None;
    }
    PyWin_SetAPIError("CompleteAuthToken", err);
    return NULL;
}

// @pymethod |PyCtxtHandle|QueryContextAttributes|Retrieves info about a security context
PyObject *PyCtxtHandle::QueryContextAttributes(PyObject *self, PyObject *args)
{
    SECURITY_STATUS err;
    PCtxtHandle pctxt;
    BYTE buf[256];
    ZeroMemory(&buf, 256);
    ULONG attr;
    PyObject *ret = NULL;
    CHECK_SECURITYFUNCTIONTABLE(QueryContextAttributesW);
    // @pyparm int|Attribute||SECPKG_ATTR_* constant
    if (!PyArg_ParseTuple(args, "l:QueryContextAttributes", &attr))
        return NULL;

    pctxt = ((PyCtxtHandle *)self)->GetCtxtHandle();
    Py_BEGIN_ALLOW_THREADS err = (*psecurityfunctiontable->QueryContextAttributesW)(pctxt, attr, &buf);
    Py_END_ALLOW_THREADS if (err != SEC_E_OK)
    {
        PyWin_SetAPIError("QueryContextAttributes", err);
        return NULL;
    }
    // @comm Not all attributes are available for every security package
    // @flagh Attribute|Return type
    switch (attr) {
        // @flag SECPKG_ATTR_ACCESS_TOKEN|<o PyHANDLE> - returns a handle to the context's access token
        case SECPKG_ATTR_ACCESS_TOKEN:
            ret = PyWinObject_FromHANDLE(((PSecPkgContext_AccessToken)&buf)->AccessToken);
            break;
        // @flag SECPKG_ATTR_AUTHORITY|<o PyUnicode> - returns the name of the authenticating entity
        case SECPKG_ATTR_AUTHORITY:
            ret = PyWinObject_FromWCHAR(((PSecPkgContext_AuthorityW)&buf)->sAuthorityName);
            (*psecurityfunctiontable->FreeContextBuffer)(((PSecPkgContext_AuthorityW)&buf)->sAuthorityName);
            break;
        // @flag SECPKG_ATTR_CIPHER_STRENGTHS|(int,int) - returns the mininum and maximum cipher strengths allowed
        case SECPKG_ATTR_CIPHER_STRENGTHS:
            PSecPkgCred_CipherStrengths cs;
            cs = (PSecPkgCred_CipherStrengths)&buf;
            ret = Py_BuildValue("ll", cs->dwMinimumCipherStrength, cs->dwMaximumCipherStrength);
            break;
        // @flag SECPKG_ATTR_CONNECTION_INFO|Returns a dictionary of connection info representing a
        // SecPkgContext_ConnectionInfo struct
        case SECPKG_ATTR_CONNECTION_INFO:
            PSecPkgContext_ConnectionInfo ci;
            ci = (PSecPkgContext_ConnectionInfo)&buf;
            ret = Py_BuildValue("{s:l,s:l,s:l,s:l,s:l,s:l,s:l}", "Protocol", ci->dwProtocol, "Cipher", ci->aiCipher,
                                "CipherStrength", ci->dwCipherStrength, "Hash", ci->aiHash, "HashStrength",
                                ci->dwHashStrength, "Exch", ci->aiExch, "ExchStrength", ci->dwExchStrength);
            break;
        // @flag SECPKG_ATTR_SESSION_KEY|string - returns the session key for the context
        case SECPKG_ATTR_SESSION_KEY:
            PSecPkgContext_SessionKey sk;
            sk = (PSecPkgContext_SessionKey)&buf;
            ret = PyString_FromStringAndSize((const char *)sk->SessionKey, sk->SessionKeyLength);
            (*psecurityfunctiontable->FreeContextBuffer)(sk->SessionKey);
            break;
        // @flag SECPKG_ATTR_ISSUER_LIST_EX|(int, string) - Returns names of trusted certificate issuers
        case SECPKG_ATTR_ISSUER_LIST_EX:
            PSecPkgContext_IssuerListInfoEx li;
            li = (PSecPkgContext_IssuerListInfoEx)&buf;
            ret = Py_BuildValue("lN", li->cIssuers,
                                PyString_FromStringAndSize((char *)li->aIssuers->pbData, li->aIssuers->cbData));
            (*psecurityfunctiontable->FreeContextBuffer)(li->aIssuers);
            break;
        // @flag SECPKG_ATTR_FLAGS|int - returns flags negotiated when context was established
        case SECPKG_ATTR_FLAGS:
            ret = PyLong_FromUnsignedLong(((PSecPkgContext_Flags)&buf)->Flags);
            break;
        // @flag SECPKG_ATTR_PACKAGE_INFO|dict - returns dictionary containing info for context's security package
        case SECPKG_ATTR_PACKAGE_INFO:
            PSecPkgContext_PackageInfoW pi;
            pi = (PSecPkgContext_PackageInfoW)&buf;
            ret = PyWinObject_FromSecPkgInfo(pi->PackageInfo);
            (*psecurityfunctiontable->FreeContextBuffer)(pi->PackageInfo);
            break;
        // @flag SECPKG_ATTR_NEGOTIATION_INFO|(int, dict) - returns state of negotiation (SECPKG_NEGOTIATION_COMPLETE,
        //  SECPKG_NEGOTIATION_OPTIMISTIC,SECPKG_NEGOTIATION_IN_PROGRESS) and info for negotiated package
        case SECPKG_ATTR_NEGOTIATION_INFO:
            PSecPkgContext_NegotiationInfoW ni;
            ni = (PSecPkgContext_NegotiationInfoW)&buf;
            ret = Py_BuildValue("lN", ni->NegotiationState, PyWinObject_FromSecPkgInfo(ni->PackageInfo));
            (*psecurityfunctiontable->FreeContextBuffer)(ni->PackageInfo);
            break;
        // @flag SECPKG_ATTR_NAMES|<o PyUnicode> - returns the user name for the context
        case SECPKG_ATTR_NAMES:
            ret = PyWinObject_FromWCHAR(((PSecPkgContext_NamesW)&buf)->sUserName);
            (*psecurityfunctiontable->FreeContextBuffer)(((PSecPkgContext_NamesW)&buf)->sUserName);
            break;
        // @flag SECPKG_ATTR_SIZES|dict containing buffer sizes to be used with the context
        case SECPKG_ATTR_SIZES:
            PSecPkgContext_Sizes sz;
            sz = (PSecPkgContext_Sizes)&buf;
            ret = Py_BuildValue("{s:l,s:l,s:l,s:l}", "MaxToken", sz->cbMaxToken, "MaxSignature", sz->cbMaxSignature,
                                "BlockSize", sz->cbBlockSize, "SecurityTrailer", sz->cbSecurityTrailer);
            break;
        // @flag SECPKG_ATTR_PASSWORD_EXPIRY|<o PyTime> - returns time password expires
        case SECPKG_ATTR_PASSWORD_EXPIRY:
            PSecPkgContext_PasswordExpiry pe;
            pe = (PSecPkgContext_PasswordExpiry)&buf;
            ret = PyWinObject_FromTimeStamp(pe->tsPasswordExpires);
            break;
        // @flag SECPKG_ATTR_LIFESPAN|(<o PyTime>,<o PyTime>) - returns time period during which context is valid
        case SECPKG_ATTR_LIFESPAN:
            PSecPkgContext_Lifespan ls;
            ls = (PSecPkgContext_Lifespan)&buf;
            ret = Py_BuildValue("NN", PyWinObject_FromTimeStamp(ls->tsStart), PyWinObject_FromTimeStamp(ls->tsExpiry));
            break;
        // @flag SECPKG_ATTR_NATIVE_NAMES|(<o PyUnicode>,<o PyUnicode>) - returns client and server names
        case SECPKG_ATTR_NATIVE_NAMES:
            PSecPkgContext_NativeNamesW nn;
            nn = (PSecPkgContext_NativeNamesW)&buf;
            ret = Py_BuildValue("NN", PyWinObject_FromWCHAR(nn->sClientName), PyWinObject_FromWCHAR(nn->sServerName));
            (*psecurityfunctiontable->FreeContextBuffer)(nn->sClientName);
            (*psecurityfunctiontable->FreeContextBuffer)(nn->sServerName);
            break;
        // @flag SECPKG_ATTR_TARGET_INFORMATION|string - returns the target for the context
        case SECPKG_ATTR_TARGET_INFORMATION:
            PSecPkgContext_TargetInformation ti;
            ti = (PSecPkgContext_TargetInformation)&buf;
            ret = PyString_FromStringAndSize((const char *)ti->MarshalledTargetInfo, ti->MarshalledTargetInfoLength);
            (*psecurityfunctiontable->FreeContextBuffer)(ti->MarshalledTargetInfo);
            break;
        // @flag SECPKG_ATTR_STREAM_SIZES|dict (see SecPkgContext_StreamSizes) containing message buffer sizes
        case SECPKG_ATTR_STREAM_SIZES:
            PSecPkgContext_StreamSizes ss;
            ss = (PSecPkgContext_StreamSizes)buf;
            ret = Py_BuildValue("{s:l,s:l,s:l,s:l,s:l}", "Header", ss->cbHeader, "Trailer", ss->cbTrailer,
                                "MaximumMessage", ss->cbMaximumMessage, "Buffers", ss->cBuffers, "BlockSize",
                                ss->cbBlockSize);
            break;
        // @flag SECPKG_ATTR_KEY_INFO|dict (see SecPkgContext_KeyInfo) containing encryption key parameters
        case SECPKG_ATTR_KEY_INFO:
            PSecPkgContext_KeyInfo ki;
            ki = (PSecPkgContext_KeyInfo)&buf;
            ret = Py_BuildValue("{s:u,s:u,s:l,s:l,s:l}", "SignatureAlgorithmName", ki->sSignatureAlgorithmName,
                                "EncryptAlgorithmName", ki->sEncryptAlgorithmName, "KeySize", ki->KeySize,
                                "SignatureAlgorithm", ki->SignatureAlgorithm, "EncryptAlgorithm", ki->EncryptAlgorithm);
            (*psecurityfunctiontable->FreeContextBuffer)(ki->sSignatureAlgorithmName);
            (*psecurityfunctiontable->FreeContextBuffer)(ki->sEncryptAlgorithmName);
            break;
        // @flag SECPKG_ATTR_DCE_INFO|not supported yet
        case SECPKG_ATTR_DCE_INFO:  // SecPkgContext_DceInfo
        // @flag SECPKG_ATTR_LOCAL_CERT_CONTEXT|not supported yet
        case SECPKG_ATTR_LOCAL_CERT_CONTEXT:  // PCCERT_CONTEXT
        // @flag SECPKG_ATTR_REMOTE_CERT_CONTEXT|not supported yet
        case SECPKG_ATTR_REMOTE_CERT_CONTEXT:  // PCCERT_CONTEXT
        // @flag SECPKG_ATTR_ROOT_STORE|not supported yet
        case SECPKG_ATTR_ROOT_STORE:  // HCERTCONTEXT
        // @flag SECPKG_ATTR_SUPPORTED_ALGS|not supported yet
        case SECPKG_ATTR_SUPPORTED_ALGS:  // SecPkgCred_SupportedAlgs
        // @flag SECPKG_ATTR_SUPPORTED_PROTOCOLS|not supported yet
        case SECPKG_ATTR_SUPPORTED_PROTOCOLS:  // SecPkgCred_SupportedProtocols
        default:
            PyErr_SetString(PyExc_NotImplementedError, "Attribute is not supported yet");
    }
    return ret;
}

// @pymethod <o PyHandle>|PyCtxtHandle|QuerySecurityContextToken|Returns the access token for a security context
PyObject *PyCtxtHandle::QuerySecurityContextToken(PyObject *self, PyObject *args)
{
    SECURITY_STATUS err;
    PCtxtHandle pctxt;
    HANDLE htoken;
    CHECK_SECURITYFUNCTIONTABLE(QuerySecurityContextToken);
    if (!PyArg_ParseTuple(args, ":QuerySecurityContextToken"))
        return NULL;
    pctxt = ((PyCtxtHandle *)self)->GetCtxtHandle();
    Py_BEGIN_ALLOW_THREADS err = (*psecurityfunctiontable->QuerySecurityContextToken)(pctxt, &htoken);
    Py_END_ALLOW_THREADS if (err == SEC_E_OK) return PyWinObject_FromHANDLE(htoken);
    PyWin_SetAPIError("QuerySecurityContextToken", err);
    return NULL;
}

// @pymethod |PyCtxtHandle|ImpersonateSecurityContext|Impersonates a client security context
PyObject *PyCtxtHandle::ImpersonateSecurityContext(PyObject *self, PyObject *args)
{
    CHECK_SECURITYFUNCTIONTABLE(ImpersonateSecurityContext);
    PCtxtHandle pctxt;
    SECURITY_STATUS err;
    if (!PyArg_ParseTuple(args, ":ImpersonateSecurityContext"))
        return NULL;
    pctxt = ((PyCtxtHandle *)self)->GetCtxtHandle();
    Py_BEGIN_ALLOW_THREADS err = (*psecurityfunctiontable->ImpersonateSecurityContext)(pctxt);
    Py_END_ALLOW_THREADS if (err == SEC_E_OK)
    {
        Py_INCREF(Py_None);
        return Py_None;
    }
    PyWin_SetAPIError("ImpersonateSecurityContext", err);
    return NULL;
}

// @pymethod |PyCtxtHandle|RevertSecurityContext|Stops impersonation of client context (see <om
// PyCtxtHandle::ImpersonateSecurityContext>)
PyObject *PyCtxtHandle::RevertSecurityContext(PyObject *self, PyObject *args)
{
    CHECK_SECURITYFUNCTIONTABLE(RevertSecurityContext);
    PCtxtHandle pctxt;
    SECURITY_STATUS err;
    if (!PyArg_ParseTuple(args, ":RevertSecurityContext"))
        return NULL;
    pctxt = ((PyCtxtHandle *)self)->GetCtxtHandle();
    Py_BEGIN_ALLOW_THREADS err = (*psecurityfunctiontable->RevertSecurityContext)(pctxt);
    Py_END_ALLOW_THREADS if (err == SEC_E_OK)
    {
        Py_INCREF(Py_None);
        return Py_None;
    }
    PyWin_SetAPIError("RevertSecurityContext", err);
    return NULL;
}

////////////////////////////////////////////////////////////////////////
//
// PyCredHandle
//
////////////////////////////////////////////////////////////////////////
PyObject *PyWinObject_FromSecPkgInfo(PSecPkgInfoW psecpkginfo)
{
    return Py_BuildValue("{s:l,s:l,s:l,s:l,s:u,s:u}", "Capabilities", psecpkginfo->fCapabilities, "Version",
                         psecpkginfo->wVersion, "RPCID", psecpkginfo->wRPCID, "MaxToken", psecpkginfo->cbMaxToken,
                         "Name", psecpkginfo->Name, "Comment", psecpkginfo->Comment);
}

// @object PyCredHandle|Handle to a set of logon credentials, used with sspi authentication functions
// @comm This object is usually created using <om win32security.AcquireCredentialsHandle>.
// An uninitialized handle can also be created using win32security.PyCredHandleType()
struct PyMethodDef PyCredHandle::methods[] = {
    {"Detach", PyCredHandle::Detach,
     1},  // @pymeth Detach|Disassociates object from handle and returns integer value of handle (prevents automatic
          // freeing of credentials when object is deallocated),
    {"FreeCredentialsHandle", PyCredHandle::FreeCredentialsHandle,
     1},  // @pymeth FreeCredentialsHandle|Releases the credentials handle
    {"QueryCredentialsAttributes", PyCredHandle::QueryCredentialsAttributes,
     1},  // @pymeth QueryCredentialsAttributes|Returns information about the credentials
    {NULL}};

PyTypeObject PyCredHandleType = {
    PYWIN_OBJECT_HEAD "PyCredHandle",
    sizeof(PyCredHandle),
    0,
    PyCredHandle::deallocFunc,                 // tp_dealloc
    0,                                         // tp_print
    0,                                         // tp_getattr
    0,                                         // tp_setattr
    0,                                         // tp_compare
    0,                                         // tp_repr
    0,                                         // PyNumberMethods *tp_as_number
    0,                                         // PySequenceMethods *tp_as_sequence
    0,                                         // PyMappingMethods *tp_as_mapping
    0,                                         // hashfunc tp_hash
    0,                                         // tp_call
    0,                                         // tp_str
    PyObject_GenericGetAttr,                   // tp_getattro
    PyObject_GenericSetAttr,                   // tp_setattro
    0,                                         // PyBufferProcs *tp_as_buffer
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,  // tp_flags
    0,                                         // tp_doc
    0,                                         // traverseproc tp_traverse
    0,                                         // tp_clear
    0,                                         // richcmpfunc tp_richcompare
    0,                                         // tp_weaklistoffset
    0,                                         // getiterfunc tp_iter
    0,                                         // iternextfunc tp_iternext
    PyCredHandle::methods,
    NULL,
    0,                    // tp_getset;
    0,                    // tp_base;
    0,                    // tp_dict;
    0,                    // tp_descr_get
    0,                    // tp_descr_set
    0,                    // tp_dictoffset
    0,                    // tp_init
    0,                    // tp_alloc
    PyCredHandle::tp_new  // newfunc tp_new;
};

PyCredHandle::PyCredHandle(PCredHandle pcredhandle)
{
    ob_type = &PyCredHandleType;
    credhandle = *pcredhandle;
    _Py_NewReference(this);
}

PyCredHandle::PyCredHandle(void)
{
    ob_type = &PyCredHandleType;
    SecInvalidateHandle(&credhandle);
    _Py_NewReference(this);
}

PyCredHandle::~PyCredHandle()
{
    if (SecIsValidHandle(&credhandle))
        (*psecurityfunctiontable->FreeCredentialsHandle)(&credhandle);
}

BOOL PyCredHandle_Check(PyObject *ob)
{
    if (ob->ob_type != &PyCredHandleType) {
        PyErr_SetString(PyExc_TypeError, "Object must be a PyCredHandle");
        return FALSE;
    }
    return TRUE;
}

void PyCredHandle::deallocFunc(PyObject *ob) { delete (PyCredHandle *)ob; }

PCredHandle PyCredHandle::GetCredHandle(void) { return &credhandle; }

PyObject *PyCredHandle::tp_new(PyTypeObject *typ, PyObject *args, PyObject *kwargs) { return new PyCredHandle(); }

BOOL PyWinObject_AsCredHandle(PyObject *ob, PCredHandle *pcredhandle, BOOL bNoneOk)
{
    if (ob == Py_None) {
        if (bNoneOk) {
            *pcredhandle = NULL;
            return TRUE;
        }
        PyErr_SetString(PyExc_ValueError, "Credentials handle cannot be NULL");
        return FALSE;
    }
    if (!PyCredHandle_Check(ob))
        return FALSE;
    *pcredhandle = ((PyCredHandle *)ob)->GetCredHandle();
    return TRUE;
}

PyObject *PyWinObject_FromCredHandle(PCredHandle pcredhandle) { return new PyCredHandle(pcredhandle); }

// @pymethod long|PyCredHandle|Detach|Disassociates object from handle and returns integer value of handle,
PyObject *PyCredHandle::Detach(PyObject *self, PyObject *args)
{
    if (!PyArg_ParseTuple(args, ":Detach"))
        return NULL;
    PyCredHandle *This = (PyCredHandle *)self;
    PCredHandle pcredhandle = This->GetCredHandle();
    PyObject *ret = PyWinObject_FromSecHandle(pcredhandle);
    if (ret != NULL)
        SecInvalidateHandle(pcredhandle);
    return ret;
}

// @pymethod |PyCredHandle|FreeCredentialsHandle|Releases the credentials handle and makes object unusable
PyObject *PyCredHandle::FreeCredentialsHandle(PyObject *self, PyObject *args)
{
    CHECK_SECURITYFUNCTIONTABLE(FreeCredentialsHandle);
    if (!PyArg_ParseTuple(args, ":FreeCredentialsHandle"))
        return NULL;
    PyCredHandle *This = (PyCredHandle *)self;
    PCredHandle pcredhandle = This->GetCredHandle();
    SECURITY_STATUS err = (*psecurityfunctiontable->FreeCredentialsHandle)(pcredhandle);
    if (err != SEC_E_OK) {
        PyWin_SetAPIError("FreeCredentialsHandle", err);
        return NULL;
    }
    SecInvalidateHandle(pcredhandle);
    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod |PyCredHandle|QueryCredentialsAttributes|Returns information about the credentials
// @rdesc Type of returned values is dependent on Attribute
PyObject *PyCredHandle::QueryCredentialsAttributes(PyObject *self, PyObject *args)
{
    // @pyparm int|Attribute||SECPKG_* constant specifying which type of information to return
    // @comm Only SECPKG_CRED_ATTR_NAMES currently supported
    ULONG attr;
    SECURITY_STATUS err;
    PyObject *ret = NULL;
    BYTE buf[32];
    CHECK_SECURITYFUNCTIONTABLE(QueryCredentialsAttributesW);
    PyCredHandle *This = (PyCredHandle *)self;
    PCredHandle pcredhandle = This->GetCredHandle();
    if (!PyArg_ParseTuple(args, "l:QueryCredentialsAttributes", &attr))
        return NULL;
    Py_BEGIN_ALLOW_THREADS err = (*psecurityfunctiontable->QueryCredentialsAttributesW)(pcredhandle, attr, &buf);
    Py_END_ALLOW_THREADS if (err != SEC_E_OK)
    {
        PyWin_SetAPIError("QueryCredentialsAttributes", err);
        return NULL;
    }
    // @flagh Attribute|Return type
    switch (attr) {
        // @flag SECPKG_CRED_ATTR_NAMES|<o PyUnicode> - returns username that credentials represent
        case SECPKG_CRED_ATTR_NAMES:
            PSecPkgCredentials_NamesW cn;
            cn = (PSecPkgCredentials_NamesW)&buf;
            ret = PyWinObject_FromWCHAR(cn->sUserName);
            (*psecurityfunctiontable->FreeContextBuffer)(cn->sUserName);
            break;
        // @flag SECPKG_ATTR_SUPPORTED_ALGS|Not supported yet
        case SECPKG_ATTR_SUPPORTED_ALGS:  // SecPkgCred_SupportedAlgs:
        // @flag SECPKG_ATTR_CIPHER_STRENGTHS|Not supported yet
        case SECPKG_ATTR_CIPHER_STRENGTHS:  // SecPkgCred_CipherStrengths:
        // @flag SECPKG_ATTR_SUPPORTED_PROTOCOLS|Not supported yet
        case SECPKG_ATTR_SUPPORTED_PROTOCOLS:  // SecPkgCred_SupportedProtocols:
        default:
            PyErr_SetString(PyExc_NotImplementedError, "Attribute is not supported yet");
    }
    return ret;
}
