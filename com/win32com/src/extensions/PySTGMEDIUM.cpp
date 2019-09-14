// A Python object representing a windows STGMEDIUM structure.
#include "stdafx.h"
#include "PythonCOM.h"
#include "structmember.h"
#include "PyComTypeObjects.h"
// @doc This file contains autoduck documentation.
// @pymethod <o PySTGMEDIUM>|pythoncom|STGMEDIUM|Creates a new STGMEDIUM object
PyObject *Py_NewSTGMEDIUM(PyObject *self, PyObject *args)
{
    if (!PyArg_ParseTuple(args, ""))
        return NULL;
    return new PySTGMEDIUM();
}

PySTGMEDIUM *PyObject_FromSTGMEDIUM(STGMEDIUM *desc /* = NULL*/) { return new PySTGMEDIUM(desc); }

// @pymethod |PySTGMEDIUM|set|Sets the type and data of the object.
PyObject *PySet(PyObject *self, PyObject *args)
{
    int tymed;
    PyObject *ob;
    // @pyparm int|tymed||The type of the data
    // @pyparm object|data||
    if (!PyArg_ParseTuple(args, "iO:set", &tymed, &ob))
        return NULL;

    PySTGMEDIUM *ps = (PySTGMEDIUM *)self;
    ps->Close();  // ensure any old data clean
    switch (tymed) {
        case TYMED_GDI: {
            HBITMAP htmp;
            if (!PyWinObject_AsHANDLE(ob, (HANDLE *)&htmp))
                return NULL;
            ps->medium.hBitmap = htmp;
            break;
        }
        case TYMED_MFPICT: {
            HMETAFILEPICT htmp;
            if (!PyWinObject_AsHANDLE(ob, (HANDLE *)&htmp))
                return NULL;
            ps->medium.hMetaFilePict = htmp;
            break;
        }
        case TYMED_ENHMF: {
            HENHMETAFILE htmp;
            if (!PyWinObject_AsHANDLE(ob, (HANDLE *)&htmp))
                return NULL;
            ps->medium.hEnhMetaFile = htmp;
            break;
        }
        case TYMED_HGLOBAL: {
            const void *buf = NULL;
            Py_ssize_t cb = 0;
            // In py3k, unicode objects don't support the buffer
            // protocol, so explicitly check string types first.
            // We need to include the NULL for strings and unicode, as the
            // Windows clipboard functions will assume it is there for
            // text related formats (eg, CF_TEXT).
            if (PyString_Check(ob)) {
                cb = PyString_GET_SIZE(ob) + 1;  // for the NULL
                buf = (void *)PyString_AS_STRING(ob);
            }
            else if (PyUnicode_Check(ob)) {
                cb = PyUnicode_GET_DATA_SIZE(ob) + sizeof(Py_UNICODE);
                buf = (void *)PyUnicode_AS_UNICODE(ob);
            }
            else {
                if (PyObject_AsReadBuffer(ob, &buf, &cb) == -1)
                    return PyErr_Format(PyExc_TypeError, "tymed value of %d requires a string/unicode/buffer", tymed);
                // no extra nulls etc needed here.
            }
            ps->medium.hGlobal = GlobalAlloc(GMEM_FIXED, cb);
            if (!ps->medium.hGlobal)
                return PyErr_NoMemory();
            memcpy((void *)ps->medium.hGlobal, buf, cb);
            break;
        }
        case TYMED_FILE:
            if (!PyWinObject_AsTaskAllocatedWCHAR(ob, &ps->medium.lpszFileName, FALSE, NULL))
                return FALSE;
            break;
        case TYMED_ISTREAM:
            if (!PyCom_InterfaceFromPyInstanceOrObject(ob, IID_IStream, (void **)&ps->medium.pstm, FALSE /* bNoneOK */))
                return FALSE;
            break;
        case TYMED_ISTORAGE:
            if (!PyCom_InterfaceFromPyInstanceOrObject(ob, IID_IStorage, (void **)&ps->medium.pstg,
                                                       FALSE /* bNoneOK */))
                return FALSE;
            break;
        default:
            PyErr_Format(PyExc_ValueError, "Unknown tymed value '%d'", tymed);
            return NULL;
    }
    ps->medium.tymed = tymed;
    Py_INCREF(Py_None);
    return Py_None;
}

// @object PySTGMEDIUM|A STGMEDIUM object represents a COM STGMEDIUM structure.
struct PyMethodDef PySTGMEDIUM::methods[] = {{"set", PySet, 1},  // @pymeth set|Sets the type and data of the object
                                             {NULL}};

PyTypeObject PySTGMEDIUM::Type = {
    PYWIN_OBJECT_HEAD "PySTGMEDIUM",
    sizeof(PySTGMEDIUM),
    0,
    PySTGMEDIUM::deallocFunc, /* tp_dealloc */
    0,                        /* tp_print */
    0,                        /* tp_getattr */
    0,                        /* tp_setattr */
    0,                        /* tp_compare */
    0,                        /* tp_repr */
    0,                        /* tp_as_number */
    0,                        /* tp_as_sequence */
    0,                        /* tp_as_mapping */
    0,                        /* tp_hash */
    0,                        /* tp_call */
    0,                        /* tp_str */
    PySTGMEDIUM::getattro,    /* tp_getattro */
    0,                        /* tp_setattro */
    0,                        /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,       /* tp_flags */
    0,                        /* tp_doc */
    0,                        /* tp_traverse */
    0,                        /* tp_clear */
    0,                        /* tp_richcompare */
    0,                        /* tp_weaklistoffset */
    0,                        /* tp_iter */
    0,                        /* tp_iternext */
    PySTGMEDIUM::methods,     /* tp_methods */
    0,                        /* tp_members */
    0,                        /* tp_getset */
    0,                        /* tp_base */
    0,                        /* tp_dict */
    0,                        /* tp_descr_get */
    0,                        /* tp_descr_set */
    0,                        /* tp_dictoffset */
    0,                        /* tp_init */
    0,                        /* tp_alloc */
    0,                        /* tp_new */
};

#define OFF(e) offsetof(PySTGMEDIUM, e)

PySTGMEDIUM::PySTGMEDIUM(STGMEDIUM *pm)
{
    ob_type = &PySTGMEDIUM::Type;
    _Py_NewReference(this);
    if (pm)
        memcpy(&medium, pm, sizeof(medium));
    else
        memset(&medium, 0, sizeof(medium));
}

PySTGMEDIUM::~PySTGMEDIUM() { Close(); }

void PySTGMEDIUM::DropOwnership() { memset(&medium, 0, sizeof(medium)); }

BOOL PySTGMEDIUM::CopyTo(STGMEDIUM *pDest)
{
    // caller is responsible for ensuring this is clean.
    assert(pDest->tymed == 0 && pDest->pUnkForRelease == 0 && pDest->hGlobal == 0);
    switch (medium.tymed) {
        case TYMED_GDI:
            // Receiving app that is performing Paste operation takes ownership of the handle and
            //	is responsible for freeing it (usually by calling ReleaseStgMedium)
            pDest->hBitmap = medium.hBitmap;
            break;
        case TYMED_MFPICT:
            pDest->hMetaFilePict = medium.hMetaFilePict;
            break;
        case TYMED_ENHMF:
            pDest->hEnhMetaFile = medium.hEnhMetaFile;
            break;
        case TYMED_HGLOBAL: {
            SIZE_T cb = GlobalSize(medium.hGlobal);
            pDest->hGlobal = GlobalAlloc(GMEM_FIXED, cb);
            if (!pDest->hGlobal) {
                PyErr_NoMemory();
                return FALSE;
            }
            memcpy(pDest->hGlobal, medium.hGlobal, cb);
            break;
        }
        case TYMED_FILE:
            if (medium.lpszFileName) {
                size_t cch = wcslen(medium.lpszFileName) + 1;
                if (!(pDest->lpszFileName = (WCHAR *)CoTaskMemAlloc(sizeof(WCHAR) * cch))) {
                    PyErr_NoMemory();
                    return FALSE;
                }
                wcsncpy(pDest->lpszFileName, medium.lpszFileName, cch);
            }
            break;
        case TYMED_ISTREAM:
            pDest->pstm = medium.pstm;
            if (pDest->pstm)
                pDest->pstm->AddRef();
            break;
        case TYMED_ISTORAGE:
            pDest->pstg = medium.pstg;
            if (pDest->pstg)
                pDest->pstg->AddRef();
            break;
        case TYMED_NULL:
            // nothing to do
            break;
        default:
            PyErr_Format(PyExc_ValueError, "Unknown tymed value '%d'", medium.tymed);
            return FALSE;
    }
    pDest->tymed = medium.tymed;
    return TRUE;
}
void PySTGMEDIUM::Close()
{
    if (medium.tymed) {
        ReleaseStgMedium(&medium);
        memset(&medium, 0, sizeof(medium));
        assert(!medium.tymed);
    }
}

PyObject *PySTGMEDIUM::getattro(PyObject *self, PyObject *obname)
{
    char *name = PYWIN_ATTR_CONVERT(obname);
    if (name == NULL)
        return NULL;

    PySTGMEDIUM *ps = (PySTGMEDIUM *)self;
    // @prop int|tymed|An integer indicating the type of data in the stgmedium
    if (strcmp(name, "tymed") == 0)
        return PyInt_FromLong(ps->medium.tymed);
    // @prop object|data|The data in the stgmedium.
    // The result depends on the value of the 'tymed' property of the <o PySTGMEDIUM> object.
    // @flagh tymed|Result Type
    if (strcmp(name, "data") == 0) {
        switch (ps->medium.tymed) {
            // @flag TYMED_GDI|An integer GDI handle
            case TYMED_GDI:
                return PyLong_FromVoidPtr(ps->medium.hBitmap);
            // @flag TYMED_MFPICT|An integer METAFILE handle
            case TYMED_MFPICT:
                return PyLong_FromVoidPtr(ps->medium.hMetaFilePict);
            // @flag TYMED_ENHMF|An integer ENHMETAFILE handle
            case TYMED_ENHMF:
                return PyLong_FromVoidPtr(ps->medium.hEnhMetaFile);
            // @flag TYMED_HGLOBAL|A string with the bytes of the global memory object.
            case TYMED_HGLOBAL: {
                PyObject *ret;
                void *p = GlobalLock(ps->medium.hGlobal);
                if (p) {
                    ret = PyString_FromStringAndSize((char *)p, GlobalSize(ps->medium.hGlobal));
                    GlobalUnlock(ps->medium.hGlobal);
                }
                else {
                    ret = Py_None;
                    Py_INCREF(Py_None);
                }
                return ret;
            }
            // @flag TYMED_FILE|A string/unicode filename
            case TYMED_FILE:
                return PyWinObject_FromWCHAR(ps->medium.lpszFileName);
            // @flag TYMED_ISTREAM|A <o PyIStream> object
            case TYMED_ISTREAM:
                // @flag TYMED_ISTORAGE|A <o PyIStorage> object
                return PyCom_PyObjectFromIUnknown(ps->medium.pstm, IID_IStream, TRUE);
            case TYMED_ISTORAGE:
                return PyCom_PyObjectFromIUnknown(ps->medium.pstg, IID_IStorage, TRUE);
            case TYMED_NULL:
                PyErr_SetString(PyExc_ValueError, "This STGMEDIUM has no data");
                return NULL;
            default:
                PyErr_SetString(PyExc_RuntimeError, "Unknown tymed");
                return NULL;
        }
    }
    // @prop int|data_handle|The raw 'integer' representation of the data.
    // For TYMED_HGLOBAL, this is the handle rather than the string data.
    // For the string and interface types, this is an integer holding the pointer.
    if (strcmp(name, "data_handle") == 0) {
        switch (ps->medium.tymed) {
            case TYMED_GDI:
                return PyLong_FromVoidPtr(ps->medium.hBitmap);
            case TYMED_MFPICT:
                return PyLong_FromVoidPtr(ps->medium.hMetaFilePict);
            case TYMED_ENHMF:
                return PyLong_FromVoidPtr(ps->medium.hEnhMetaFile);
            case TYMED_HGLOBAL:
                return PyLong_FromVoidPtr(ps->medium.hGlobal);
            // and may as well hand out the pointers for these.
            // We are all consenting adults :)
            case TYMED_FILE:
                return PyLong_FromVoidPtr(ps->medium.lpszFileName);
            case TYMED_ISTREAM:
                return PyLong_FromVoidPtr(ps->medium.pstm);
            case TYMED_ISTORAGE:
                return PyLong_FromVoidPtr(ps->medium.pstg);
            case TYMED_NULL:
                PyErr_SetString(PyExc_ValueError, "This STGMEDIUM has no data");
                return NULL;
            default:
                PyErr_SetString(PyExc_RuntimeError, "Unknown tymed");
                return NULL;
        }
    }
    return PyObject_GenericGetAttr(self, obname);
}

/*static*/ void PySTGMEDIUM::deallocFunc(PyObject *ob) { delete (PySTGMEDIUM *)ob; }
