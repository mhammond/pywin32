// @doc
#include "PyWinTypes.h"
#include "PyWinObjects.h"
#include "ktmw32.h"

// @pymethod <o PyHANDLE>|win32transaction|CreateTransaction|Creates a transaction
// @pyseeapi CreateTransaction
static PyObject *PyCreateTransaction(PyObject *self, PyObject *args, PyObject *kwargs)
{
    WCHAR *description = NULL;
    PyObject *obsa = Py_None, *obuow = Py_None, *obdescription = Py_None;
    DWORD createoptions = 0, isolationlevel = 0, isolationflags = 0, timeout = 0;
    PSECURITY_ATTRIBUTES psa = NULL;
    GUID *uow = NULL;
    HANDLE hret;
    static char *keywords[] = {"TransactionAttributes", "UOW",     "CreateOptions", "IsolationLevel",
                               "IsolationFlags",        "Timeout", "Description",   NULL};

    if (!PyArg_ParseTupleAndKeywords(
            args, kwargs, "|OOkkkkO:CreateTransaction", keywords,
            &obsa,   // @pyparm <o PySECURITY_ATTRIBUTES>|TransactionAttributes|None|Security and inheritance for the
                     // transaction, can be None
            &obuow,  // @pyparm <o PyIID>|UOW|None|Reserved, use only None
            &createoptions,   // @pyparm int|CreateOptions|0|TRANSACTION_DO_NOT_PROMOTE is only defined flag
            &isolationlevel,  // @pyparm int|IsolationLevel|0|Reserved, use only 0
            &isolationflags,  // @pyparm int|IsolationFlags|0|Reserved, use only 0
            &timeout,         // @pyparm int|Timeout|0|Abort timeout in milliseconds
            &obdescription))  // @pyparm <o PyUnicode>|Description|None|Text description of transaction, can be None
        return NULL;
    if (!PyWinObject_AsSECURITY_ATTRIBUTES(obsa, &psa, TRUE))
        return NULL;
    if (obuow != Py_None) {
        PyErr_SetString(PyExc_TypeError, "UOW must be None");
        return NULL;
    }
    if (!PyWinObject_AsWCHAR(obdescription, &description, TRUE))
        return NULL;
    Py_BEGIN_ALLOW_THREADS hret =
        CreateTransaction(psa, uow, createoptions, isolationlevel, isolationflags, timeout, description);
    Py_END_ALLOW_THREADS PyWinObject_FreeWCHAR(description);
    if (hret == INVALID_HANDLE_VALUE)
        return PyWin_SetAPIError("CreateTransaction");
    return PyWinObject_FromHANDLE(hret);
}

// @pymethod |win32transaction|RollbackTransaction|Rolls back a transaction
// @pyseeapi RollbackTransaction
static PyObject *PyRollbackTransaction(PyObject *self, PyObject *args, PyObject *kwargs)
{
    PyObject *obtrans;
    HANDLE htrans;
    BOOL ret;
    static char *keywords[] = {"TransactionHandle", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O:RollbackTransaction", keywords,
                                     &obtrans))  // @pyparm <o PyHANDLE>|TransactionHandle||Handle to a transaction
        return NULL;
    if (!PyWinObject_AsHANDLE(obtrans, &htrans))
        return NULL;
    Py_BEGIN_ALLOW_THREADS ret = RollbackTransaction(htrans);
    Py_END_ALLOW_THREADS if (!ret) return PyWin_SetAPIError("RollbackTransaction");
    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod |win32transaction|RollbackTransactionAsync|Rolls back a transaction asynchronously
// @pyseeapi RollbackTransactionAsync
static PyObject *PyRollbackTransactionAsync(PyObject *self, PyObject *args, PyObject *kwargs)
{
    PyObject *obtrans;
    HANDLE htrans;
    BOOL ret;
    static char *keywords[] = {"TransactionHandle", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O:RollbackTransactionAsync", keywords,
                                     &obtrans))  // @pyparm <o PyHANDLE>|TransactionHandle||Handle to a transaction
        return NULL;
    if (!PyWinObject_AsHANDLE(obtrans, &htrans))
        return NULL;
    Py_BEGIN_ALLOW_THREADS ret = RollbackTransactionAsync(htrans);
    Py_END_ALLOW_THREADS if (!ret) return PyWin_SetAPIError("RollbackTransactionAsync");
    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod |win32transaction|CommitTransaction|Commits a transaction
// @pyseeapi CommitTransaction
static PyObject *PyCommitTransaction(PyObject *self, PyObject *args, PyObject *kwargs)
{
    PyObject *obtrans;
    HANDLE htrans;
    BOOL ret;
    static char *keywords[] = {"TransactionHandle", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O:CommitTransaction", keywords,
                                     &obtrans))  // @pyparm <o PyHANDLE>|TransactionHandle||Handle to a transaction
        return NULL;
    if (!PyWinObject_AsHANDLE(obtrans, &htrans))
        return NULL;
    Py_BEGIN_ALLOW_THREADS ret = CommitTransaction(htrans);
    Py_END_ALLOW_THREADS if (!ret) return PyWin_SetAPIError("CommitTransaction");
    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod |win32transaction|CommitTransactionAsync|Commits a transaction asynchronously
// @pyseeapi CommitTransactionAsync
static PyObject *PyCommitTransactionAsync(PyObject *self, PyObject *args, PyObject *kwargs)
{
    PyObject *obtrans;
    HANDLE htrans;
    BOOL ret;
    static char *keywords[] = {"TransactionHandle", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O:CommitTransactionAsync", keywords,
                                     &obtrans))  // @pyparm <o PyHANDLE>|TransactionHandle||Handle to a transaction
        return NULL;
    if (!PyWinObject_AsHANDLE(obtrans, &htrans))
        return NULL;
    Py_BEGIN_ALLOW_THREADS ret = CommitTransactionAsync(htrans);
    Py_END_ALLOW_THREADS if (!ret) return PyWin_SetAPIError("CommitTransactionAsync");
    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod <o PyIID>|win32transaction|GetTransactionId|Returns the transaction's GUID
static PyObject *PyGetTransactionId(PyObject *self, PyObject *args, PyObject *kwargs)
{
    PyObject *obtrans;
    HANDLE htrans;
    BOOL ret;
    GUID guid;
    static char *keywords[] = {"TransactionHandle", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O:GetTransactionId", keywords,
                                     &obtrans))  // @pyparm <o PyHANDLE>|TransactionHandle||Handle to a transaction
        return NULL;
    if (!PyWinObject_AsHANDLE(obtrans, &htrans))
        return NULL;
    Py_BEGIN_ALLOW_THREADS ret = GetTransactionId(htrans, &guid);
    Py_END_ALLOW_THREADS if (!ret) return PyWin_SetAPIError("GetTransactionId");
    return PyWinObject_FromIID(guid);
}

// @pymethod <o PyHANDLE>|win32transaction|OpenTransaction|Creates a handle to an existing transaction
static PyObject *PyOpenTransaction(PyObject *self, PyObject *args, PyObject *kwargs)
{
    PyObject *obguid;
    HANDLE htrans;
    DWORD access;
    GUID guid;
    static char *keywords[] = {"DesiredAccess", "TransactionId", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "kO:OpenTransaction", keywords,
                                     &access,   // @pyparm int|DesiredAccess||Combination of TRANSACTION_* access rights
                                     &obguid))  // @pyparm <o PyIID>|TransactionId||GUID identifying the transaction
        return NULL;
    if (!PyWinObject_AsIID(obguid, &guid))
        return NULL;
    Py_BEGIN_ALLOW_THREADS htrans = OpenTransaction(access, &guid);
    Py_END_ALLOW_THREADS if (htrans == INVALID_HANDLE_VALUE) return PyWin_SetAPIError("OpenTransaction");
    return PyWinObject_FromHANDLE(htrans);
}

// @module win32transaction|Module wrapping Kernal Transaction Manager functions, as used with
//	transacted NTFS and transacted registry functions.
// @comm All functions accept keyword arguments.
static PyMethodDef win32transaction_functions[] = {
    // @pymeth CreateTransaction|Creates a transaction
    {"CreateTransaction", (PyCFunction)PyCreateTransaction, METH_KEYWORDS | METH_VARARGS},
    // @pymeth RollbackTransaction|Rolls back a transaction
    {"RollbackTransaction", (PyCFunction)PyRollbackTransaction, METH_KEYWORDS | METH_VARARGS},
    // @pymeth RollbackTransactionAsync|Rolls back a transaction asynchronously
    {"RollbackTransactionAsync", (PyCFunction)PyRollbackTransactionAsync, METH_KEYWORDS | METH_VARARGS},
    // @pymeth CommitTransaction|Commits a transaction
    {"CommitTransaction", (PyCFunction)PyCommitTransaction, METH_KEYWORDS | METH_VARARGS},
    // @pymeth CommitTransactionAsync|Commits a transaction asynchronously
    {"CommitTransactionAsync", (PyCFunction)PyCommitTransactionAsync, METH_KEYWORDS | METH_VARARGS},
    // @pymeth GetTransactionId|Returns the transaction's GUID
    {"GetTransactionId", (PyCFunction)PyGetTransactionId, METH_KEYWORDS | METH_VARARGS},
    // @pymeth OpenTransaction|Creates a handle to an existing transaction
    {"OpenTransaction", (PyCFunction)PyOpenTransaction, METH_KEYWORDS | METH_VARARGS},
    {NULL, NULL}};

PYWIN_MODULE_INIT_FUNC(win32transaction)
{
    PYWIN_MODULE_INIT_PREPARE(win32transaction, win32transaction_functions,
                              "Module wrapping Kernal Transaction Manager functions, as used with"
                              " transacted NTFS and transacted registry functions.");

    Py_INCREF(PyWinExc_ApiError);
    PyDict_SetItemString(dict, "error", PyWinExc_ApiError);

    PYWIN_MODULE_INIT_RETURN_SUCCESS;
}
