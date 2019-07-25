/***********************************************************

win2kras.cpp -- module for Windows 200 extensions to RAS

Note that this source file contains embedded documentation.
This documentation consists of marked up text inside the
C comments, and is prefixed with an '@' symbol.  The source
files are processed by a tool called "autoduck" which
generates Windows .hlp files.
@doc

******************************************************************/

#ifndef WINVER
#define WINVER 0x500
#endif

#include "pywintypes.h"
#include "ras.h"
#include "raserror.h"

typedef PyObject *(*PFNReturnRasError)(char *fnName, long err);

PFNReturnRasError pfnReturnRasError = NULL;

static PyObject *ReturnRasError(char *fnName, long err = 0)
{
    if (pfnReturnRasError == NULL)
        Py_FatalError("No ras pfn!");
    return (*pfnReturnRasError)(fnName, err);
}

class PyRASEAPUSERIDENTITY : public PyObject {
   public:
    PyRASEAPUSERIDENTITY(RASEAPUSERIDENTITY *);
    ~PyRASEAPUSERIDENTITY();

    /* Python support */
    static void deallocFunc(PyObject *ob);

    static PyObject *getattro(PyObject *self, PyObject *name);
    static PyTypeObject type;
    RASEAPUSERIDENTITY *m_identity;
};

#define PyRASEAPUSERIDENTITY_Check(ob) ((ob)->ob_type == &PyRASEAPUSERIDENTITY::type)

BOOL PyWinObject_AsRASEAPUSERIDENTITY(PyObject *ob, RASEAPUSERIDENTITY **ppRASEAPUSERIDENTITY, BOOL bNoneOK /*= TRUE*/)
{
    if (bNoneOK && ob == Py_None) {
        *ppRASEAPUSERIDENTITY = NULL;
    }
    else if (!PyRASEAPUSERIDENTITY_Check(ob)) {
        PyErr_SetString(PyExc_TypeError, "The object is not a PyRASEAPUSERIDENTITY object");
        return FALSE;
    }
    else {
        *ppRASEAPUSERIDENTITY = ((PyRASEAPUSERIDENTITY *)ob)->m_identity;
    }
    return TRUE;
}

PyObject *PyWinObject_FromRASEAPUSERIDENTITY(RASEAPUSERIDENTITY *pRASEAPUSERIDENTITY)
{
    if (pRASEAPUSERIDENTITY == NULL) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    return new PyRASEAPUSERIDENTITY(pRASEAPUSERIDENTITY);
}

PyTypeObject PyRASEAPUSERIDENTITY::type = {
    PYWIN_OBJECT_HEAD "PyRASEAPUSERIDENTITY",
    sizeof(PyRASEAPUSERIDENTITY),
    0,
    PyRASEAPUSERIDENTITY::deallocFunc,                               /* tp_dealloc */
    0,                                                               /* tp_print */
    0,                                                               /* tp_getattr */
    0,                                                               /* tp_setattr */
    0,                                                               /* tp_compare */
    0,                                                               /* tp_repr */
    0,                                                               /* tp_as_number */
    0,                                                               /* tp_as_sequence */
    0,                                                               /* tp_as_mapping */
    0,                                                               /* tp_hash */
    0,                                                               /* tp_call */
    0,                                                               /* tp_str */
    PyRASEAPUSERIDENTITY::getattro,                                  /*tp_getattro*/
    0,                                                               /*tp_setattro*/
    0,                                                               /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT,                                              // tp_flags;
    "An object that describes a Win32 RASDIALEXTENSIONS structure",  // tp_doc
    0,                                                               // tp_traverse;
    0,                                                               // tp_clear
    0,                                                               // tp_richcompare;
    0,                                                               // tp_weaklistoffset;
    0,                                                               // tp_iter
    0,                                                               // iternextfunc tp_iternext
    0,                                                               // tp_methods
    0,                                                               // tp_members
    0,                                                               // tp_getset;
    0,                                                               // tp_base;
    0,                                                               // tp_dict;
    0,                                                               // tp_descr_get;
    0,                                                               // tp_descr_set;
    0,                                                               // tp_dictoffset;
    0,                                                               // tp_init;
    0,                                                               // tp_alloc;
    0,                                                               // newfunc tp_new;
};

PyRASEAPUSERIDENTITY::PyRASEAPUSERIDENTITY(RASEAPUSERIDENTITY *identity)
{
    ob_type = &type;
    _Py_NewReference(this);
    m_identity = identity;
}

PyRASEAPUSERIDENTITY::~PyRASEAPUSERIDENTITY()
{
    if (m_identity) {
        // kinda-like an assert ;-)
        RasFreeEapUserIdentity(m_identity);
    }
}

PyObject *PyRASEAPUSERIDENTITY::getattro(PyObject *self, PyObject *obname)
{
    char *name = PYWIN_ATTR_CONVERT(obname);
    if (name == NULL)
        return NULL;
    PyRASEAPUSERIDENTITY *py = (PyRASEAPUSERIDENTITY *)self;
    if (strcmp(name, "szUserName") == 0 || strcmp(name, "userName") == 0)
        return PyWinObject_FromTCHAR(py->m_identity->szUserName);
    if (strcmp(name, "pbEapInfo") == 0 || strcmp(name, "eapInfo") == 0)
        return PyBuffer_FromMemory(py->m_identity->pbEapInfo, py->m_identity->dwSizeofEapInfo);
    return PyObject_GenericGetAttr(self, obname);
}

/*static*/ void PyRASEAPUSERIDENTITY::deallocFunc(PyObject *ob) { delete (PyRASEAPUSERIDENTITY *)ob; }

// @pymethod |win2kras|PyRasGetEapUserIdentity|Sets the dial parameters for the specified entry.
static PyObject *PyRasGetEapUserIdentity(PyObject *self, PyObject *args)
{
    TCHAR *phoneBook = NULL, *entry = NULL;
    PyObject *obphoneBook, *obentry;
    int flags;
    HWND hwnd = NULL;
    PyObject *ret = NULL;
    if (!PyArg_ParseTuple(
            args, "OOi|O&:GetEapUserIdentity",
            &obphoneBook,  // @pyparm string|phoneBook||string containing the full path of the phone-book (PBK) file. If
                           // this parameter is None, the function will use the system phone book.
            &obentry,      // @pyparm string|entry||string containing an existing entry name.
            &flags,  // @pyparm int|flags||Specifies zero or more of the following flags that qualify the authentication
                     // process.
                     // @flagh Flag|Description
                     // @flag RASEAPF_NonInteractive|Specifies that the authentication protocol should not bring up a
                     // graphical user-interface. If this flag is not present, it is okay for the protocol to display a
                     // user interface.
                     // @flag RASEAPF_Logon|Specifies that the user data is obtained from Winlogon.
                     // @flag RASEAPF_Preview|Specifies that the user should be prompted for identity information before
                     // dialing.
            PyWinObject_AsHANDLE,
            &hwnd))  // @pyparm <o PyHANDLE>|hwnd|None|Handle to the parent window for the UI dialog.
        return NULL;

    if (PyWinObject_AsTCHAR(obphoneBook, &phoneBook, TRUE) && PyWinObject_AsTCHAR(obentry, &entry, FALSE)) {
        // @pyseeapi RasGetEapUserIdentity
        DWORD rc;
        RASEAPUSERIDENTITY *identity;
        Py_BEGIN_ALLOW_THREADS rc = RasGetEapUserIdentity(phoneBook, entry, flags, hwnd, &identity);
        Py_END_ALLOW_THREADS if (rc != 0) ReturnRasError("RasGetEapUserIdentity", rc);
        else ret = PyWinObject_FromRASEAPUSERIDENTITY(identity);
    }
    PyWinObject_FreeTCHAR(phoneBook);
    PyWinObject_FreeTCHAR(entry);
    return ret;
}

/* List of functions exported by this module */
// @module win2kras|A module encapsulating the Windows 2000 extensions to the Remote Access Service (RAS) API.
static struct PyMethodDef win2kras_functions[] = {
    {"GetEapUserIdentity", PyRasGetEapUserIdentity,
     METH_VARARGS},  // @pymeth RasGetEapUserIdentity|Retrieves identity information for the current user. Use this
                     // information to call RasDial with a phone-book entry that requires Extensible Authentication
                     // Protocol (EAP).
    {NULL, NULL}};

#define ADD_CONSTANT(tok)                                \
    if (rc = PyModule_AddIntConstant(module, #tok, tok)) \
    return rc

static int AddConstants(PyObject *module)
{
    int rc;
    ADD_CONSTANT(RASEAPF_NonInteractive);  // @const win2kras|RASEAPF_NonInteractive|Specifies that the authentication
                                           // protocol should not bring up a graphical user-interface. If this flag is
                                           // not present, it is okay for the protocol to display a user interface.
    ADD_CONSTANT(
        RASEAPF_Logon);  // @const win2kras|RASEAPF_Logon|Specifies that the user data is obtained from Winlogon.
    ADD_CONSTANT(RASEAPF_Preview);  // @const win2kras|RASEAPF_Preview|Specifies that the user should be prompted for
                                    // identity information before dialing.
    return 0;
}

PYWIN_MODULE_INIT_FUNC(win2kras)
{
    PYWIN_MODULE_INIT_PREPARE(
        win2kras, win2kras_functions,
        "A module encapsulating the Windows 2000 extensions to the Remote Access Service (RAS) API.");

    if (PyType_Ready(&PyRASEAPUSERIDENTITY::type) == -1)
        PYWIN_MODULE_INIT_RETURN_ERROR;
    if (AddConstants(module) != 0)
        PYWIN_MODULE_INIT_RETURN_ERROR;

#ifdef _DEBUG
    const TCHAR *modName = _T("win32ras_d.pyd");
#else
    const TCHAR *modName = _T("win32ras.pyd");
#endif
    // We insist on win32ras being imported - but the least we
    // can do is attempt the import ourselves!
    HMODULE hmod = GetModuleHandle(modName);
    if (hmod == NULL) {
        PyObject *tempMod = PyImport_ImportModule("win32ras");
        Py_XDECREF(tempMod);
        hmod = GetModuleHandle(modName);
    }
    if (hmod == NULL) {
        PyErr_SetString(PyExc_RuntimeError, "You must import 'win32ras' before importing this module");
        PYWIN_MODULE_INIT_RETURN_ERROR;
    }
    FARPROC fp = GetProcAddress(hmod, "ReturnRasError");
    if (fp == NULL) {
        PyErr_SetString(PyExc_RuntimeError, "Could not locate 'ReturnRasError' in 'win32ras'");
        PYWIN_MODULE_INIT_RETURN_ERROR;
    }
    pfnReturnRasError = (PFNReturnRasError)fp;

    PYWIN_MODULE_INIT_RETURN_SUCCESS;
}
