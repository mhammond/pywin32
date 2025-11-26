/***********************************************************

win32ras.cpp -- module for interface into RAS

Note that this source file contains embedded documentation.
This documentation consists of marked up text inside the
C comments, and is prefixed with an '@' symbol.  The source
files are processed by a tool called "autoduck" which
generates Windows .hlp files.
@doc

******************************************************************/

#include "pywintypes.h"
#include "ras.h"
#include "raserror.h"

static PyObject *module_error;
static PyObject *obHandleMap = NULL;

/* error helper */

void SetError(char *msg, char *fnName = NULL, DWORD code = 0)
{
    PyObject *v = Py_BuildValue("(izs)", 0, fnName, msg);
    if (v != NULL) {
        PyErr_SetObject(module_error, v);
        Py_DECREF(v);
    }
}
PyObject *ReturnError(char *msg, char *fnName = NULL, DWORD code = 0)
{
    SetError(msg, fnName, code);
    return NULL;
}

PyObject *ReturnRasError(char *fnName, long err = 0)
{
    const int bufSize = 512;
    TCHAR buf[bufSize];
    DWORD errorCode = err == 0 ? GetLastError() : err;
    BOOL bHaveMessage = FALSE;
    if (errorCode) {
        bHaveMessage = RasGetErrorString(errorCode, buf, bufSize) == 0;
        if (!bHaveMessage) {
            bHaveMessage = (0 != FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, errorCode, 0, buf, bufSize, NULL));
        }
    }
    if (!bHaveMessage)
        _tcscpy(buf, _T("No error message is available"));
    /* strip trailing cr/lf */
    size_t end = _tcslen(buf) - 1;
    if (end > 1 && (buf[end - 1] == _T('\n') || buf[end - 1] == _T('\r')))
        buf[end - 1] = 0;
    else if (end > 0 && (buf[end] == _T('\n') || buf[end] == _T('\r')))
        buf[end] = 0;
    PyObject *v = Py_BuildValue("(iNN)", errorCode, PyWinCoreString_FromString(fnName), PyWinObject_FromTCHAR(buf));
    if (v != NULL) {
        PyErr_SetObject(module_error, v);
        Py_DECREF(v);
    }
    return NULL;
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

BOOL PyWinObject_AsRASEAPUSERIDENTITY(PyObject *ob, RASEAPUSERIDENTITY **ppRASEAPUSERIDENTITY, BOOL bNoneOK = TRUE)
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

////////////////////////////////////////////////////////////
//
// RASDIALEXTENSIONS support
//
class PyRASDIALEXTENSIONS : public PyObject {
   public:
    PyRASDIALEXTENSIONS(void);
    ~PyRASDIALEXTENSIONS();

    /* Python support */
    static void deallocFunc(PyObject *ob);

    static PyObject *getattro(PyObject *self, PyObject *obname);
    static int setattro(PyObject *self, PyObject *obname, PyObject *v);
    static PyTypeObject type;
    RASDIALEXTENSIONS m_ext;
    PyObject *m_pyeap;
};

#define PyRASDIALEXTENSIONS_Check(ob) ((ob)->ob_type == &PyRASDIALEXTENSIONS::type)

// @object RASDIALEXTENSIONS|An object that describes a Win32 RASDIALEXTENSIONS structure
BOOL PyWinObject_AsRASDIALEXTENSIONS(PyObject *ob, RASDIALEXTENSIONS **ppRASDIALEXTENSIONS, BOOL bNoneOK /*= TRUE*/)
{
    if (bNoneOK && ob == Py_None) {
        *ppRASDIALEXTENSIONS = NULL;
    }
    else if (!PyRASDIALEXTENSIONS_Check(ob)) {
        PyErr_SetString(PyExc_TypeError, "The object is not a PyRASDIALEXTENSIONS object");
        return FALSE;
    }
    else {
        *ppRASDIALEXTENSIONS = &((PyRASDIALEXTENSIONS *)ob)->m_ext;
    }
    return TRUE;
}

PyObject *PyWinObject_NewRASDIALEXTENSIONS(PyObject *self, PyObject *args)
{
    if (!PyArg_ParseTuple(args, ""))
        return NULL;
    return new PyRASDIALEXTENSIONS();
}

PyTypeObject PyRASDIALEXTENSIONS::type = {
    PYWIN_OBJECT_HEAD "PyRASDIALEXTENSIONS",
    sizeof(PyRASDIALEXTENSIONS),
    0,
    PyRASDIALEXTENSIONS::deallocFunc,                                /* tp_dealloc */
    0,                                                               /* tp_print */
    0,                                                               /* tp_getattr */
    0,                                                               /* tp_setattr */
    0,                                                               /* tp_compare */
    0,                                                               /* tp_repr */
    0,                                                               /* tp_as_number */
    0,                                                               /* tp_as_sequence */
    0,                                                               /* tp_as_mapping */
    0,                                                               /* tp_hahs */
    0,                                                               /* tp_call */
    0,                                                               /* tp_str */
    PyRASDIALEXTENSIONS::getattro,                                   /* tp_getattro */
    PyRASDIALEXTENSIONS::setattro,                                   /* tp_setattro */
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

PyRASDIALEXTENSIONS::PyRASDIALEXTENSIONS()
{
    ob_type = &type;
    _Py_NewReference(this);
    m_pyeap = Py_None;
    Py_INCREF(Py_None);
    memset(&m_ext, 0, sizeof(m_ext));
}

PyRASDIALEXTENSIONS::~PyRASDIALEXTENSIONS() { Py_DECREF(m_pyeap); }

PyObject *PyRASDIALEXTENSIONS::getattro(PyObject *self, PyObject *obname)
{
    char *name = PYWIN_ATTR_CONVERT(obname);
    if (name == NULL)
        return NULL;
    PyRASDIALEXTENSIONS *py = (PyRASDIALEXTENSIONS *)self;
    // @prop integer|dwfOptions|(fOptions may also be used)
    if (strcmp(name, "dwfOptions") == 0 || strcmp(name, "fOptions") == 0)
        return PyLong_FromLong(py->m_ext.dwfOptions);
    // @prop integer|hwndParent|
    if (strcmp(name, "hwndParent") == 0)
        return PyLong_FromVoidPtr(py->m_ext.hwndParent);
    // @prop integer|reserved|
    if (strcmp(name, "reserved") == 0)
        return PyWinObject_FromULONG_PTR(py->m_ext.reserved);
    // @prop integer|reserved1|
    if (strcmp(name, "reserved1") == 0)
        return PyWinObject_FromULONG_PTR(py->m_ext.reserved1);
    // @prop <o RASEAPINFO>|RasEapInfo|
    if (strcmp(name, "RasEapInfo") == 0) {
        Py_INCREF(py->m_pyeap);
        return py->m_pyeap;
    }
    return PyObject_GenericGetAttr(self, obname);
}

int PyRASDIALEXTENSIONS::setattro(PyObject *self, PyObject *obname, PyObject *val)
{
    if (val == NULL) {
        PyErr_SetString(PyExc_AttributeError, "can't delete OVERLAPPED attributes");
        return -1;
    }
    char *name = PYWIN_ATTR_CONVERT(obname);
    if (name == NULL)
        return -1;
    PyRASDIALEXTENSIONS *py = (PyRASDIALEXTENSIONS *)self;
    if (strcmp(name, "dwfOptions") == 0 || strcmp(name, "fOptions") == 0) {
        int i = PyLong_AsLong(val);
        if (i == -1 && PyErr_Occurred())
            return -1;
        py->m_ext.dwfOptions = i;
        return 0;
    }
    if (strcmp(name, "hwndParent") == 0) {
        HANDLE h;
        if (!PyWinObject_AsHANDLE(val, &h))
            return -1;
        py->m_ext.hwndParent = (HWND)h;
        return 0;
    }
    if (strcmp(name, "reserved") == 0) {
        long v = PyLong_AsLong(val);
        if (v == -1 && PyErr_Occurred())
            return -1;
        py->m_ext.reserved = v;
        return 0;
    }
    if (strcmp(name, "reserved1") == 0) {
        long v = PyLong_AsLong(val);
        if (v == -1 && PyErr_Occurred())
            return -1;
        py->m_ext.reserved1 = v;
        return 0;
    }
    if (strcmp(name, "RasEapInfo") == 0) {
        RASEAPUSERIDENTITY *temp;
        if (!PyWinObject_AsRASEAPUSERIDENTITY(val, &temp))
            return -1;
        py->m_ext.RasEapInfo.dwSizeofEapInfo = temp->dwSizeofEapInfo;
        py->m_ext.RasEapInfo.pbEapInfo = temp->pbEapInfo;
        Py_DECREF(py->m_pyeap);
        py->m_pyeap = val;
        Py_INCREF(val);
        return 0;
    }
    return PyObject_GenericSetAttr(self, obname, val);
}

/*static*/ void PyRASDIALEXTENSIONS::deallocFunc(PyObject *ob) { delete (PyRASDIALEXTENSIONS *)ob; }

////////////////////////////////////////////////////////////
//
// RASDIALPARAMS support
//

// @object RASDIALPARAMS|A tuple that describes a Win32 RASDIALPARAMS structure
// @comm When used as a parameter, RASDIALPARAMS must be a sequence, of up to
// 6 items long.  All items must be strings - None is not allowed.
// <nl>When this is returned from a RAS function, all six fields will exist.
// <nl>RAS will often accept an empty string to mean "default" - ie, passing
// an empty string to phoneNumber uses the stored phone number.
// @tupleitem 0|string|entryName|name of RAS entry.
// @tupleitem 1|string|phoneNumber|phone number to be used.
// @tupleitem 2|string|callBackNumber|phone number to be used if callback is enabled.
// @tupleitem 3|string|userName|username to log on with.
// @tupleitem 4|string|password|password to use
// @tupleitem 5|string|domain|Network domain to log on to.
// @ex An example with win32ras.Dial|handle = win32ras.Dial(None, None, ("Entry Name",), None)
BOOL PyObjectToRasDialParams(PyObject *ob, RASDIALPARAMS *p)
{
    char *fnName = "<RasDialParams conversion>";
    ZeroMemory(p, sizeof(*p));
    p->dwSize = sizeof(RASDIALPARAMS);
    PyObject *t = PySequence_Tuple(ob);
    if (t == NULL)
        return NULL;

    TCHAR *dest, *src;
    Py_ssize_t size = PyTuple_GET_SIZE(ob);
    DWORD dest_size, src_size;
    BOOL ret = TRUE;
    for (Py_ssize_t num = 0; num < size; num++) {
        switch (num) {
#define GET_BUF_AND_SIZE(name) \
    dest = p->name;            \
    dest_size = sizeof(p->name) / sizeof(p->name[0])
            case 0:
                GET_BUF_AND_SIZE(szEntryName);
                break;
            case 1:
                GET_BUF_AND_SIZE(szPhoneNumber);
                break;
            case 2:
                GET_BUF_AND_SIZE(szCallbackNumber);
                break;
            case 3:
                GET_BUF_AND_SIZE(szUserName);
                break;
            case 4:
                GET_BUF_AND_SIZE(szPassword);
                break;
            case 5:
                GET_BUF_AND_SIZE(szDomain);
                break;
            default:
                SetError("The RasDialParams sequence length must be less than 6", fnName);
                return FALSE;
        }
        PyObject *sub = PyTuple_GET_ITEM(t, num);
        ret = PyWinObject_AsTCHAR(sub, &src, FALSE, &src_size);
        if (!ret)
            break;
        // check it fits in the dest buffer.
        if (src_size >= dest_size) {
            PyErr_Format(PyExc_ValueError, "%s: String size (%d) greater than acceptable size (%d)", fnName, src_size,
                         dest_size - 1);
            ret = FALSE;
        }
        else
            _tcsncpy(dest, src, src_size);
        PyWinObject_FreeTCHAR(src);
        if (!ret)
            break;
    }
    Py_DECREF(t);
    return ret;
}

/////////////////////////////////////////////////////////////////////
//
// the RAS callback function.  This looks up a Python handler,
// and defers the call to it.
//
// @method |win32ras|RasDialFunc1|A placeholder for a RAS callback.
// @comm Certain RAS function require a callback function to be passed.
// This description describes the signature of the function you pass
// to these functions.
VOID CALLBACK PyRasDialFunc1(HRASCONN hrasconn,      // handle to RAS connection
                             UINT unMsg,             // type of event that has occurred
                             RASCONNSTATE rascs,     // connection state about to be entered
                             DWORD dwError,          // error that may have occurred
                             DWORD dwExtendedError)  // extended error information for some errors
{
    CEnterLeavePython _celp;
    char *fnName = "<RAS Callback handler>";
    PyObject *handler = NULL;
    if (obHandleMap) {
        // NOTE:  As we hold the thread lock, assume noone else can mod this dict.
        PyObject *key = PyWinLong_FromVoidPtr(hrasconn);
        if (key == NULL)
            return;
        handler = PyDict_GetItem(obHandleMap, key);
        // If handler is NULL, check if None is in the map, and if so,
        // use and replace it.
        if (handler == NULL) {
            handler = PyDict_GetItem(obHandleMap, Py_None);
            if (handler) {
                PyDict_SetItem(obHandleMap, key, handler);
                PyDict_DelItem(obHandleMap, Py_None);
            }
        }
        Py_DECREF(key);
    }
    if (handler == NULL) {
        SetError("Warning - RAS callback has no handler!", fnName);
        PyErr_Print();
        return;
    }
    // @pyparm int|hrascon||The handle to the RAS session.
    // @pyparm int|msg||A message code identifying the reason for the callback.
    // @pyparm int|rascs||Connection state about to be entered.
    // @pyparm int|error||The error state of the connection
    // @pyparm int|extendedError||
    PyObject *args = Py_BuildValue("Niiii", PyWinLong_FromHANDLE(hrasconn), unMsg, rascs, dwError, dwExtendedError);
    if (args == NULL)
        return;
    PyObject *res = PyObject_CallObject(handler, args);
    Py_DECREF(args);
    if (res == NULL) {
        PyErr_Print();
        SetError("RAS callback failed!", fnName);
        PyErr_Print();
        return;
    }
    Py_DECREF(res);
}

// @pymethod |win32ras|CreatePhonebookEntry|Creates a new phonebook entry.  The function displays a dialog box into
// which the user can enter information about the entry
static PyObject *PyRasCreatePhonebookEntry(PyObject *self, PyObject *args)
{
    DWORD rc;
    LPTSTR fileName = NULL;
    PyObject *obhwnd;
    if (!PyArg_ParseTuple(args, "O|s:CreatePhoneBookEntry",
                          &obhwnd,     // @pyparm int|hWnd||Handle to the parent window of the dialog box.
                          &fileName))  // @pyparm string|fileName|None|Specifies the filename of the phonebook entry.
                                       // Currently this is ignored.
        return NULL;
    HWND hwnd;
    if (!PyWinObject_AsHANDLE(obhwnd, (HANDLE *)&hwnd))
        return NULL;
    if (hwnd != 0 && !IsWindow(hwnd))
        return ReturnError("The first parameter must be a valid window handle",
                           "<CreatePhonebookEntry param conversion>");
    if ((rc = RasCreatePhonebookEntry(hwnd, fileName)))
        return ReturnRasError("RasCreatePhonebookEntry", rc);  // @pyseeapi RasCreatePhonebookEntry
    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod int, int|win32ras|Dial|Establishes a RAS connection to a RAS server.
static PyObject *PyRasDial(PyObject *self, PyObject *args)
{
    DWORD rc;
    PyObject *obExtensions;
    PyObject *obParams;
    PyObject *obCallback;
    RASDIALPARAMS dialParams;
    RASDIALEXTENSIONS *dialExts;
    LPTSTR fileName;
    HRASCONN hRas = (HRASCONN)0;

    if (!PyArg_ParseTuple(
            args, "OzOO:Dial",
            &obExtensions,  // @pyparm <o PyRASDIALEXTENSIONS>|dialExtensions||An object providing the RASDIALEXTENSIONS
                            // information, or None
            &fileName,      // @pyparm string|fileName||Specifies the filename of the phonebook entry, or None.
            &obParams,      // @pyparm <o RASDIALPARAMS>|RasDialParams||A tuple describing a RASDIALPARAMS structure.
            &obCallback))   // @pyparm method or hwnd|callback||The method to be called when RAS events occur, or None.
                            // If not None, the function must have the signature of <om win32ras.RasDialFunc1>
        return NULL;
    if (!PyObjectToRasDialParams(obParams, &dialParams))
        return NULL;

    if (!PyWinObject_AsRASDIALEXTENSIONS(obExtensions, &dialExts, TRUE))
        return NULL;

    DWORD notType = 0;
    LPVOID pNotification;
    if (obCallback == Py_None) {
        pNotification = NULL;
    }
    else if (PyCallable_Check(obCallback)) {
        pNotification = PyRasDialFunc1;
        notType = 1;
    }
    else if (PyLong_Check(obCallback)) {
        if (!PyWinLong_AsVoidPtr(obCallback, &pNotification))
            return NULL;
        notType = 0xFFFFFFFF;
    }
    else
        return ReturnError("The callback object must be an integer handle, None, or a callable object",
                           "<Dial param parsing>");
    // If we have a callback, store it in our map with None as the key.
    // The callback routine will patch this once it knows the true key.
    // Before we do, we must check None is not already there
    if (notType == 1) {
        if (obHandleMap == NULL && (obHandleMap = PyDict_New()) == NULL)
            return NULL;
        if (PyMapping_HasKey(obHandleMap, Py_None)) {
            PyErr_SetString(PyExc_RuntimeError, "Another RAS callback is in the process of starting");
            return NULL;
        }
        PyDict_SetItem(obHandleMap, Py_None, obCallback);
    }

    // @pyseeapi RasDial
    Py_BEGIN_ALLOW_THREADS rc = RasDial(dialExts, fileName, &dialParams, notType, pNotification, &hRas);
    Py_END_ALLOW_THREADS if (hRas == 0 && notType == 1)
    {
        PyDict_DelItem(obHandleMap, Py_None);
        PyErr_Clear();
    }
    return Py_BuildValue("Ni", PyWinLong_FromHANDLE(hRas), rc);
    // @rdesc The return value is (handle, retCode).
    // <nl>It is possible for a valid handle to be returned even on failure.
    // <nl>If the returned handle is = 0, then it can be assumed invalid.
    // @comm Note - this handle must be closed using <om win32ras.HangUp>, or
    // else the RAS port will remain open, even after the program has terminated.
    // Your operating system may need rebooting to clean up otherwise!
}

// @pymethod |win32ras|EditPhonebookEntry|Creates a new phonebook entry.  The function displays a dialog box into which
// the user can enter information about the entry
static PyObject *PyRasEditPhonebookEntry(PyObject *self, PyObject *args)
{
    DWORD rc;
    LPTSTR fileName;
    LPTSTR entryName;
    PyObject *obhwnd;
    if (!PyArg_ParseTuple(
            args, "Ozs:EditPhoneBookEntry",
            &obhwnd,      // @pyparm int|hWnd||Handle to the parent window of the dialog box.
            &fileName,    // @pyparm string|fileName||Specifies the filename of the phonebook entry, or None.  Currently
                          // this is ignored.
            &entryName))  // @pyparm string|entryName|None|Specifies the name of the phonebook entry to edit
        return NULL;
    HWND hwnd;
    if (!PyWinObject_AsHANDLE(obhwnd, (HANDLE *)&hwnd))
        return NULL;
    if (hwnd != 0 && !IsWindow(hwnd))
        return ReturnError("The first parameter must be a valid window handle", "<EditPhonebookEntry param parsing>");
    Py_BEGIN_ALLOW_THREADS rc = RasEditPhonebookEntry(hwnd, fileName, entryName);
    Py_END_ALLOW_THREADS if (rc) return ReturnRasError("RasEditPhonebookEntry", rc);  // @pyseeapi RasEditPhonebookEntry
    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod list|win32ras|EnumConnections|Returns a list of tuples, one for each active connection.
static PyObject *PyRasEnumConnections(PyObject *self, PyObject *args)
{
    DWORD rc;
    DWORD bufSize;
    DWORD noConns = 0;
    RASCONN tc;
    if (!PyArg_ParseTuple(args, ":EnumConnections"))
        return NULL;
    RASCONN *pCon = NULL;
    // make dummy call to determine buffer size.
    tc.dwSize = bufSize = sizeof(RASCONN);
    Py_BEGIN_ALLOW_THREADS rc = RasEnumConnections(&tc, &bufSize, &noConns);
    Py_END_ALLOW_THREADS if (rc != 0 && rc != ERROR_BUFFER_TOO_SMALL) return ReturnRasError("RasEnumConnections(NULL)",
                                                                                            rc);
    if (rc == ERROR_BUFFER_TOO_SMALL) {
        if (bufSize == 0)
            return ReturnRasError("RasEnumConnections buffer size is invalid");
        pCon = (RASCONN *)malloc(bufSize);
        if (pCon == NULL) {
            PyErr_SetString(PyExc_MemoryError, "Allocating buffer for RAS connections");
            return NULL;
        }
        // @pyseeapi RasEnumConnections
        pCon[0].dwSize = sizeof(RASCONN);
        Py_BEGIN_ALLOW_THREADS rc = RasEnumConnections(pCon, &bufSize, &noConns);
        Py_END_ALLOW_THREADS if (rc != 0) return ReturnRasError("RasEnumConnections", rc);
    }
    else {
        pCon = &tc;
    }
    PyObject *ret = PyList_New(noConns);
    if (ret == NULL)
        return NULL;

    for (DWORD i = 0; i < noConns; i++) {
        PyObject *item =
            Py_BuildValue("(NNNN)", PyWinLong_FromHANDLE(pCon[i].hrasconn), PyWinObject_FromTCHAR(pCon[i].szEntryName),
                          PyWinObject_FromTCHAR(pCon[i].szDeviceType), PyWinObject_FromTCHAR(pCon[i].szDeviceName));
        if (item == NULL) {
            Py_DECREF(ret);
            ret = NULL;
            break;
        }
        PyList_SET_ITEM(ret, i, item);
    }
    // @rdesc Each tuple is of format (handle, entryName, deviceType, deviceName)
    if (pCon && pCon != &tc)
        free(pCon);
    return ret;
}

// @pymethod |win32ras|EnumEntries|Returns a list of tuples, one for each phonebook entry.
static PyObject *PyRasEnumEntries(PyObject *self, PyObject *args)
{
    DWORD rc;
    DWORD bufSize = 3 * sizeof(RASENTRYNAME);
    RASENTRYNAME *buf = NULL;
    DWORD noConns = 0, i;
    TCHAR *reserved = NULL;
    TCHAR *bookName = NULL;
    PyObject *obreserved = Py_None, *obbookName = Py_None, *ret = NULL;

    if (!PyArg_ParseTuple(args, "|OO:EnumEntries",
                          &obreserved,   // @pyparm string|reserved|None|Reserved - must be None
                          &obbookName))  // @pyparm string|fileName|None|The name of the phonebook file, or None.
        return NULL;
    if (!PyWinObject_AsTCHAR(obreserved, &reserved, TRUE) || !PyWinObject_AsTCHAR(obbookName, &bookName, TRUE))
        goto cleanup;

    while (true) {
        if (buf)
            free(buf);
        buf = (RASENTRYNAME *)malloc(bufSize);
        if (buf == NULL) {
            PyErr_NoMemory();
            goto cleanup;
        }
        // ??? Not sure if this is needed, only sets the size of first struct in buf ???
        buf->dwSize = sizeof(RASENTRYNAME);
        Py_BEGIN_ALLOW_THREADS rc = RasEnumEntries(reserved, bookName, buf, &bufSize, &noConns);
        Py_END_ALLOW_THREADS if (rc == 0) break;
        if (rc == ERROR_BUFFER_TOO_SMALL)
            continue;
        ReturnRasError("RasEnumEntries", rc);
        goto cleanup;
    }

    ret = PyTuple_New(noConns);
    if (!ret)
        goto cleanup;
    for (i = 0; i < noConns; i++) {
        PyObject *item = PyWinObject_FromTCHAR(buf[i].szEntryName);
        // ??? This struct now has some extra data ???
        if (item == NULL) {
            Py_DECREF(ret);
            ret = NULL;
            break;
        }
        PyTuple_SET_ITEM(ret, i, item);
    }

cleanup:
    if (buf)
        free(buf);
    PyWinObject_FreeTCHAR(reserved);
    PyWinObject_FreeTCHAR(bookName);
    return ret;
}

// @pymethod (int, int, string, string)|win32ras|GetConnectStatus|Returns a tuple with connection information.
static PyObject *PyRasGetConnectStatus(PyObject *self, PyObject *args)
{
    DWORD rc;
    PyObject *obras;
    if (!PyArg_ParseTuple(args, "O:GetConnectStatus",
                          &obras))  // @pyparm int|hrasconn||Handle to the RAS session.
        return NULL;
    HRASCONN hras;
    if (!PyWinObject_AsHANDLE(obras, (HANDLE *)&hras))
        return NULL;
    RASCONNSTATUS cs;
    // @pyseeapi RasGetConnectStatus
    cs.dwSize = sizeof(RASCONNSTATUS);
    if ((rc = RasGetConnectStatus(hras, &cs)))
        return ReturnRasError("RasGetConnectStatus", rc);  // @pyseeapi RasGetConnectStatus
    return Py_BuildValue("(iiss)", cs.rasconnstate, cs.dwError, cs.szDeviceType, cs.szDeviceName);
}

// @pymethod (s,s,s,s,s,s),i|win32ras|GetEntryDialParams|Returns a tuple with the most recently set dial parameters for
// the specified entry.
static PyObject *PyRasGetEntryDialParams(PyObject *self, PyObject *args)
{
    TCHAR *fileName = NULL;
    TCHAR *entryName = NULL;
    PyObject *obfileName, *obentryName, *ret = NULL;
    DWORD rc;
    if (!PyArg_ParseTuple(args, "OO:GetEntryDialParams",
                          &obfileName,    // @pyparm string|fileName||The filename of the phonebook, or None.
                          &obentryName))  // @pyparm string|entryName||The name of the entry to retrieve the params for.
        return NULL;

    if (PyWinObject_AsTCHAR(obfileName, &fileName, TRUE) && PyWinObject_AsTCHAR(obentryName, &entryName, FALSE)) {
        RASDIALPARAMS dp;
        BOOL bPass;
        dp.dwSize = sizeof(RASDIALPARAMS);
        _tcsncpy(dp.szEntryName, entryName, RAS_MaxEntryName + 1);
        dp.szEntryName[RAS_MaxEntryName] = '\0';
        // @pyseeapi RasGetEntryDialParams
        if ((rc = RasGetEntryDialParams(fileName, &dp, &bPass)))
            ReturnRasError("RasGetEntryDialParams", rc);  // @pyseeapi RasGetConnectStatus
        else
            ret = Py_BuildValue("(NNNNNN),N", PyWinObject_FromTCHAR(dp.szEntryName),
                                PyWinObject_FromTCHAR(dp.szPhoneNumber), PyWinObject_FromTCHAR(dp.szCallbackNumber),
                                PyWinObject_FromTCHAR(dp.szUserName), PyWinObject_FromTCHAR(dp.szPassword),
                                PyWinObject_FromTCHAR(dp.szDomain), PyBool_FromLong(bPass));
    }
    PyWinObject_FreeTCHAR(fileName);
    PyWinObject_FreeTCHAR(entryName);
    return ret;
    // @rdesc The return value is a tuple describing the params retrieved, plus a BOOL integer
    // indicating if the password was also retrieved.
}

// @pymethod string|win32ras|GetErrorString|Returns an error string for a RAS error code.
static PyObject *PyRasGetErrorString(PyObject *self, PyObject *args)
{
    DWORD error;
    DWORD rc;
    if (!PyArg_ParseTuple(args, "i:GetErrorString",
                          &error))  // @pyparm int|error||The error value being queried.
        return NULL;

    TCHAR buf[512];
    // @pyseeapi RasGetErrorString
    if (rc = RasGetErrorString(error, buf, sizeof(buf) / sizeof(buf[0])))
        return ReturnRasError("RasGetErrorString");
    return PyWinObject_FromTCHAR(buf);
}

// @pymethod |win32ras|HangUp|Terminates a remote access session.
static PyObject *PyRasHangUp(PyObject *self, PyObject *args)
{
    DWORD rc;
    HRASCONN hras;
    if (!PyArg_ParseTuple(args, "O&:HangUp", PyWinObject_AsHANDLE,
                          &hras))  // @pyparm int|hras||The handle to the RAS connection to be terminated.
        return NULL;

    // @pyseeapi RasHangUp
    if (rc = RasHangUp(hras))
        return ReturnRasError("RasHangup");
    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod |win32ras|IsHandleValid|Indicates if the given RAS handle is valid.
static PyObject *PyRasIsHandleValid(PyObject *self, PyObject *args)
{
    HRASCONN hras;
    if (!PyArg_ParseTuple(args, "O&:IsHandleValid", PyWinObject_AsHANDLE,
                          &hras))  // @pyparm int|hras||The handle to the RAS connection being checked.
        return NULL;
    BOOL bRet = (hras >= 0);
    return PyBool_FromLong(bRet);
}

// @pymethod |win32ras|SetEntryDialParams|Sets the dial parameters for the specified entry.
static PyObject *PyRasSetEntryDialParams(PyObject *self, PyObject *args)
{
    TCHAR *fileName;
    PyObject *obfileName, *obParams;
    RASDIALPARAMS dialParams;
    DWORD rc;
    BOOL bRemPass;
    if (!PyArg_ParseTuple(
            args, "OOi:SetEntryDialParams",
            &obfileName,  // @pyparm string|fileName||The filename of the phonebook, or None.
            &obParams,    // @pyparm (tuple)|RasDialParams||A tuple describing a RASDIALPARAMS structure.
            &bRemPass))   // @pyparm int|bSavePassword||Indicates whether to remove password from entry's parameters.
        return NULL;

    if (!PyObjectToRasDialParams(obParams, &dialParams))
        return NULL;
    if (!PyWinObject_AsTCHAR(obfileName, &fileName, TRUE))
        return NULL;
    // @pyseeapi RasSetEntryDialParams
    rc = RasSetEntryDialParams(fileName, &dialParams, bRemPass);
    PyWinObject_FreeTCHAR(fileName);
    if (rc)
        return ReturnRasError("SetEntryDialParams", rc);  // @pyseeapi RasGetConnectStatus
    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod |win32ras|PyRasGetEapUserIdentity|Sets the dial parameters for the specified entry.
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
// @module win32ras|A module encapsulating the Windows Remote Access Service (RAS) API.
static struct PyMethodDef win32ras_functions[] = {
    {"CreatePhonebookEntry", PyRasCreatePhonebookEntry,
     METH_VARARGS},  // @pymeth CreatePhonebookEntry|Creates a new phonebook entry.  The function displays a dialog box
                     // into which the user can enter information about the entry.
    {"Dial", PyRasDial, METH_VARARGS},  // @pymeth Dial|Establishes a RAS connection to a RAS server.
    {"EditPhonebookEntry", PyRasEditPhonebookEntry,
     METH_VARARGS},  // @pymeth EditPhonebookEntry|Creates a new phonebook entry.  The function displays a dialog box
                     // into which the user can enter information about the entry
    {"EnumConnections", PyRasEnumConnections,
     METH_VARARGS},  // @pymeth EnumConnections|Returns a list of tuples, one for each active connection.
    {"EnumEntries", PyRasEnumEntries,
     METH_VARARGS},  // @pymeth EnumEntries|Returns a list of tuples, one for each phonebook entry.
    {"GetConnectStatus", PyRasGetConnectStatus,
     METH_VARARGS},  // @pymeth GetConnectStatus|Returns a tuple with connection information.
    {"GetEapUserIdentity", PyRasGetEapUserIdentity,
     METH_VARARGS},  // @pymeth RasGetEapUserIdentity|Retrieves identity information for the current user. Use this
                     // information to call RasDial with a phone-book entry that requires Extensible Authentication
                     // Protocol (EAP).
    {"GetEntryDialParams", PyRasGetEntryDialParams,
     METH_VARARGS},  // @pymeth GetEntryDialParams|Returns a tuple with the most recently set dial parameters for the
                     // specified entry.
    {"GetErrorString", PyRasGetErrorString,
     METH_VARARGS},                         // @pymeth GetErrorString|Returns an error string for a RAS error code.
    {"HangUp", PyRasHangUp, METH_VARARGS},  // @pymeth HangUp|Terminates a remote access session.
    {"IsHandleValid", PyRasIsHandleValid,
     METH_VARARGS},  // @pymeth IsHandleValid|Indicates if the given RAS handle is valid.
    {"SetEntryDialParams", PyRasSetEntryDialParams,
     METH_VARARGS},  // @pymeth SetEntryDialParams|Sets the dial parameters for the specified entry.
    {"RASDIALEXTENSIONS", PyWinObject_NewRASDIALEXTENSIONS,
     METH_VARARGS},  // @pymeth RASDIALEXTENSIONS|Creates a new <o RASDIALEXTENSIONS> object
    {NULL, NULL}};

#define ADD_CONSTANT(tok)                                \
    if (rc = PyModule_AddIntConstant(module, #tok, tok)) \
    return rc
#define ADD_ENUM(parta, partb)                                                 \
    if (rc = PyModule_AddIntConstant(module, #parta "_" #partb, parta::partb)) \
    return rc
#define ADD_ENUM3(parta, partb, partc)                                                           \
    if (rc = PyModule_AddIntConstant(module, #parta "_" #partb "_" #partc, parta::partb::partc)) \
    return rc

static int AddConstants(PyObject *module)
{
    int rc;
    ADD_CONSTANT(RASCS_OpenPort);             // @const win32ras|RASCS_OpenPort|Constant for RAS state.
    ADD_CONSTANT(RASCS_PortOpened);           // @const win32ras|RASCS_PortOpened|Constant for RAS state.
    ADD_CONSTANT(RASCS_ConnectDevice);        // @const win32ras|RASCS_ConnectDevice|Constant for RAS state.
    ADD_CONSTANT(RASCS_DeviceConnected);      // @const win32ras|RASCS_DeviceConnected|Constant for RAS state.
    ADD_CONSTANT(RASCS_AllDevicesConnected);  // @const win32ras|RASCS_AllDevicesConnected|Constant for RAS state.
    ADD_CONSTANT(RASCS_Authenticate);         // @const win32ras|RASCS_Authenticate|Constant for RAS state.
    ADD_CONSTANT(RASCS_AuthNotify);           // @const win32ras|RASCS_AuthNotify|Constant for RAS state.
    ADD_CONSTANT(RASCS_AuthRetry);            // @const win32ras|RASCS_AuthRetry|Constant for RAS state.
    ADD_CONSTANT(RASCS_AuthCallback);         // @const win32ras|RASCS_AuthCallback|Constant for RAS state.
    ADD_CONSTANT(RASCS_AuthChangePassword);   // @const win32ras|RASCS_AuthChangePassword|Constant for RAS state.
    ADD_CONSTANT(RASCS_AuthProject);          // @const win32ras|RASCS_AuthProject|Constant for RAS state.
    ADD_CONSTANT(RASCS_AuthLinkSpeed);        // @const win32ras|RASCS_AuthLinkSpeed|Constant for RAS state.
    ADD_CONSTANT(RASCS_AuthAck);              // @const win32ras|RASCS_AuthAck|Constant for RAS state.
    ADD_CONSTANT(RASCS_ReAuthenticate);       // @const win32ras|RASCS_ReAuthenticate|Constant for RAS state.
    ADD_CONSTANT(RASCS_Authenticated);        // @const win32ras|RASCS_Authenticated|Constant for RAS state.
    ADD_CONSTANT(RASCS_PrepareForCallback);   // @const win32ras|RASCS_PrepareForCallback|Constant for RAS state.
    ADD_CONSTANT(RASCS_WaitForModemReset);    // @const win32ras|RASCS_WaitForModemReset|Constant for RAS state.
    ADD_CONSTANT(RASCS_WaitForCallback);      // @const win32ras|RASCS_WaitForCallback|Constant for RAS state.
    ADD_CONSTANT(RASCS_Projected);            // @const win32ras|RASCS_Projected|Constant for RAS state.
    ADD_CONSTANT(RASCS_StartAuthentication);  // @const win32ras|RASCS_StartAuthentication|Constant for RAS state.
    ADD_CONSTANT(RASCS_CallbackComplete);     // @const win32ras|RASCS_CallbackComplete|Constant for RAS state.
    ADD_CONSTANT(RASCS_LogonNetwork);         // @const win32ras|RASCS_LogonNetwork|Constant for RAS state.
    ADD_CONSTANT(RASCS_Interactive);          // @const win32ras|RASCS_Interactive|Constant for RAS state.
    ADD_CONSTANT(RASCS_RetryAuthentication);  // @const win32ras|RASCS_RetryAuthentication|Constant for RAS state.
    ADD_CONSTANT(RASCS_CallbackSetByCaller);  // @const win32ras|RASCS_CallbackSetByCaller|Constant for RAS state.
    ADD_CONSTANT(RASCS_PasswordExpired);      // @const win32ras|RASCS_PasswordExpired|Constant for RAS state.
    ADD_CONSTANT(RASCS_Connected);            // @const win32ras|RASCS_Connected|Constant for RAS state.
    ADD_CONSTANT(RASCS_Disconnected);         // @const win32ras|RASCS_Disconnected|Constant for RAS state.

    ADD_CONSTANT(RASEAPF_NonInteractive);  // @const win32ras|RASEAPF_NonInteractive|Specifies that the authentication
                                           // protocol should not bring up a graphical user-interface. If this flag is
                                           // not present, it is okay for the protocol to display a user interface.
    ADD_CONSTANT(
        RASEAPF_Logon);  // @const win32ras|RASEAPF_Logon|Specifies that the user data is obtained from Winlogon.
    ADD_CONSTANT(RASEAPF_Preview);  // @const win32ras|RASEAPF_Preview|Specifies that the user should be prompted for
                                    // identity information before dialing.

    return 0;
}

PYWIN_MODULE_INIT_FUNC(win32ras)
{
    PYWIN_MODULE_INIT_PREPARE(win32ras, win32ras_functions,
                              "A module encapsulating the Windows Remote Access Service (RAS) API.");

    module_error = PyWinExc_ApiError;
    Py_INCREF(module_error);
    PyDict_SetItemString(dict, "error", module_error);
    if (PyType_Ready(&PyRASDIALEXTENSIONS::type) == -1)
        PYWIN_MODULE_INIT_RETURN_ERROR;
    if (PyType_Ready(&PyRASEAPUSERIDENTITY::type) == -1)
        PYWIN_MODULE_INIT_RETURN_ERROR;
    if (AddConstants(module) != 0)
        PYWIN_MODULE_INIT_RETURN_ERROR;

    PYWIN_MODULE_INIT_RETURN_SUCCESS;
}
