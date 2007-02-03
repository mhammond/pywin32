// @doc
#include "PyWinTypes.h"
#include "PyWinObjects.h"

#define CHECK_PFN(fname)if (pfn##fname==NULL) return PyErr_Format(PyExc_NotImplementedError,"%s is not available on this platform", #fname);
typedef HANDLE (WINAPI *CreateTransactionfunc)(LPSECURITY_ATTRIBUTES,LPGUID,DWORD,DWORD,DWORD,DWORD,LPWSTR);
static CreateTransactionfunc pfnCreateTransaction=NULL;
typedef BOOL (WINAPI *RollbackTransactionfunc)(HANDLE);
static RollbackTransactionfunc pfnRollbackTransaction=NULL;
typedef BOOL (WINAPI *RollbackTransactionAsyncfunc)(HANDLE);
static RollbackTransactionAsyncfunc pfnRollbackTransactionAsync=NULL;
typedef BOOL (WINAPI *CommitTransactionfunc)(HANDLE);
static CommitTransactionfunc pfnCommitTransaction=NULL;
typedef BOOL (WINAPI *CommitTransactionAsyncfunc)(HANDLE);
static CommitTransactionAsyncfunc pfnCommitTransactionAsync=NULL;

typedef BOOL (WINAPI *GetTransactionIdfunc)(HANDLE,LPGUID);
static GetTransactionIdfunc pfnGetTransactionId = NULL;
typedef BOOL (WINAPI *GetTransactionInformationfunc)(HANDLE,PDWORD,PDWORD,PDWORD,PDWORD,DWORD,LPWSTR);
static GetTransactionInformationfunc pfnGetTransactionInformation = NULL;
typedef BOOL (WINAPI *SetTransactionInformationfunc)(HANDLE,DWORD,DWORD,DWORD,LPWSTR);
static SetTransactionInformationfunc pfnSetTransactionInformation = NULL;
// static char *keywords[]={"TransactionHandle","IsolationLevel","IsolationFlags","Timeout","Description", NULL};
typedef HANDLE (WINAPI *OpenTransactionfunc)(DWORD,LPGUID);
static OpenTransactionfunc pfnOpenTransaction = NULL;
// static char *keywords[]={"DesiredAccess","TransactionId", NULL};
typedef HANDLE (WINAPI *OpenResourceManagerfunc)(DWORD,HANDLE,LPGUID);
static OpenResourceManagerfunc pfnOpenResourceManager = NULL;
// static char *keywords[]={"DesiredAccess","TmHandle","RmGuid", NULL};
typedef HANDLE (WINAPI *CreateTransactionManagerfunc)(LPSECURITY_ATTRIBUTES,LPWSTR,ULONG,ULONG);
static CreateTransactionManagerfunc pfnCreateTransactionManager = NULL;
// static char *keywords[]={"TransactionAttributes","LogFileName","CreateOptions","CommitStrength", NULL};
typedef HANDLE (WINAPI *CreateResourceManagerfunc)(LPSECURITY_ATTRIBUTES,LPGUID,DWORD,HANDLE,LPWSTR);
static CreateResourceManagerfunc pfnCreateResourceManager = NULL;
// static char *keywords[]={"ResourceManagerAttributes","ResourceManagerID","CreateOptions","TmHandle","Description", NULL};
typedef HANDLE (WINAPI *OpenEnlistmentfunc)(DWORD,HANDLE,LPGUID);
static OpenEnlistmentfunc pfnOpenEnlistment = NULL;
// static char *keywords[]={"DesiredAccess","ResourceManagerHandle","EnlistmentId", NULL};
typedef HANDLE (WINAPI *CreateEnlistmentfunc)(LPSECURITY_ATTRIBUTES,HANDLE,HANDLE,DWORD,DWORD,PVOID);
static CreateEnlistmentfunc pfnCreateEnlistment = NULL;
// static char *keywords[]={"EnlistmentrAttributes","ResourceManagerHandle","TransactionHandle","NotificationMask","CreateOptions","EnlistmentKey", NULL};
typedef HANDLE (WINAPI *OpenTransactionManagerfunc)(LPWSTR,ACCESS_MASK,ULONG);
static OpenTransactionManagerfunc pfnOpenTransactionManager = NULL;
// static char *keywords[]={"LogFileName","DesiredAccess","OpenOptions", NULL};

// @pymethod <o PyHANDLE>|win32transaction|CreateTransaction|Creates a transaction
// @pyseeapi CreateTransaction
static PyObject *PyCreateTransaction(PyObject *self, PyObject *args, PyObject *kwargs)
{
	CHECK_PFN(CreateTransaction);
	WCHAR *description=NULL;
	PyObject *obsa=Py_None, *obuow=Py_None, *obdescription=Py_None;
	DWORD createoptions=0, isolationlevel=0, isolationflags=0, timeout=0;
	PSECURITY_ATTRIBUTES psa=NULL;
	GUID *uow=NULL;
	HANDLE hret;
	static char *keywords[]={"TransactionAttributes","UOW","CreateOptions","IsolationLevel",
		"IsolationFlags","Timeout","Description", NULL};

	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|OOkkkkO:CreateTransaction", keywords,
		&obsa,				// @pyparm <o PySECURITY_ATTRIBUTES>|TransactionAttributes|None|Security and inheritance for the transaction, can be None
		&obuow,				// @pyparm <o PyIID>|UOW|None|Reserved, use only None
		&createoptions,		// @pyparm int|CreateOptions|0|TRANSACTION_DO_NOT_PROMOTE is only defined flag
		&isolationlevel,	// @pyparm int|IsolationLevel|0|Reserved, use only 0
		&isolationflags,	// @pyparm int|IsolationFlags|0|Reserved, use only 0
		&timeout,			// @pyparm int|Timeout|0|Abort timeout in milliseconds
		&obdescription))	// @pyparm <o PyUnicode>|Description|None|Text description of transaction, can be None
		return NULL;
	if (!PyWinObject_AsSECURITY_ATTRIBUTES(obsa, &psa, TRUE))
		return NULL;
	if (obuow!=Py_None){
		PyErr_SetString(PyExc_TypeError,"UOW must be None");
		return NULL;
		}
	if (!PyWinObject_AsWCHAR(obdescription, &description, TRUE))
		return NULL;
	Py_BEGIN_ALLOW_THREADS
	hret=(*pfnCreateTransaction)(psa, uow, createoptions, isolationlevel, isolationflags, timeout, description);
	Py_END_ALLOW_THREADS
	PyWinObject_FreeWCHAR(description);
	if (hret==INVALID_HANDLE_VALUE)
		return PyWin_SetAPIError("CreateTransaction");
	return PyWinObject_FromHANDLE(hret);
}

// @pymethod |win32transaction|RollbackTransaction|Rolls back a transaction
// @pyseeapi RollbackTransaction
static PyObject *PyRollbackTransaction(PyObject *self, PyObject *args, PyObject *kwargs)
{
	CHECK_PFN(RollbackTransaction);
	PyObject *obtrans;
	HANDLE htrans;
	BOOL ret;
	static char *keywords[]={"TransactionHandle", NULL};
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O:RollbackTransaction", keywords,
		&obtrans))	// @pyparm <o PyHANDLE>|TransactionHandle||Handle to a transaction
		return NULL;
	if (!PyWinObject_AsHANDLE(obtrans, &htrans, FALSE))
		return NULL;
	Py_BEGIN_ALLOW_THREADS
	ret=(*pfnRollbackTransaction)(htrans);
	Py_END_ALLOW_THREADS
	if (!ret)
		return PyWin_SetAPIError("RollbackTransaction");
	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod |win32transaction|RollbackTransactionAsync|Rolls back a transaction asynchronously
// @pyseeapi RollbackTransactionAsync
static PyObject *PyRollbackTransactionAsync(PyObject *self, PyObject *args, PyObject *kwargs)
{
	CHECK_PFN(RollbackTransactionAsync);
	PyObject *obtrans;
	HANDLE htrans;
	BOOL ret;
	static char *keywords[]={"TransactionHandle", NULL};
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O:RollbackTransactionAsync", keywords,
		&obtrans))	// @pyparm <o PyHANDLE>|TransactionHandle||Handle to a transaction
		return NULL;
	if (!PyWinObject_AsHANDLE(obtrans, &htrans, FALSE))
		return NULL;
	Py_BEGIN_ALLOW_THREADS
	ret=(*pfnRollbackTransactionAsync)(htrans);
	Py_END_ALLOW_THREADS
	if (!ret)
		return PyWin_SetAPIError("RollbackTransactionAsync");
	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod |win32transaction|CommitTransaction|Commits a transaction
// @pyseeapi CommitTransaction
static PyObject *PyCommitTransaction(PyObject *self, PyObject *args, PyObject *kwargs)
{
	CHECK_PFN(CommitTransaction);
	PyObject *obtrans;
	HANDLE htrans;
	BOOL ret;
	static char *keywords[]={"TransactionHandle", NULL};
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O:CommitTransaction", keywords,
		&obtrans))	// @pyparm <o PyHANDLE>|TransactionHandle||Handle to a transaction
		return NULL;
	if (!PyWinObject_AsHANDLE(obtrans, &htrans, FALSE))
		return NULL;
	Py_BEGIN_ALLOW_THREADS
	ret=(*pfnCommitTransaction)(htrans);
	Py_END_ALLOW_THREADS
	if (!ret)
		return PyWin_SetAPIError("CommitTransaction");
	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod |win32transaction|CommitTransactionAsync|Commits a transaction asynchronously
// @pyseeapi CommitTransactionAsync
static PyObject *PyCommitTransactionAsync(PyObject *self, PyObject *args, PyObject *kwargs)
{
	CHECK_PFN(CommitTransactionAsync);
	PyObject *obtrans;
	HANDLE htrans;
	BOOL ret;
	static char *keywords[]={"TransactionHandle", NULL};
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O:CommitTransactionAsync", keywords,
		&obtrans))	// @pyparm <o PyHANDLE>|TransactionHandle||Handle to a transaction
		return NULL;
	if (!PyWinObject_AsHANDLE(obtrans, &htrans, FALSE))
		return NULL;
	Py_BEGIN_ALLOW_THREADS
	ret=(*pfnCommitTransactionAsync)(htrans);
	Py_END_ALLOW_THREADS
	if (!ret)
		return PyWin_SetAPIError("CommitTransactionAsync");
	Py_INCREF(Py_None);
	return Py_None;
}


// @module win32transaction|Module wrapping Kernal Transaction Manager functions, as used with
//	transacted NTFS and transacted registry functions.
// @comm These functions are only available on Vista and later.
// @comm All functions accept keyword arguments.
static PyMethodDef win32transaction_functions[] = {
	// @pymeth CreateTransaction|Creates a transaction
	{ "CreateTransaction", (PyCFunction)PyCreateTransaction, METH_KEYWORDS|METH_VARARGS},
	// @pymeth RollbackTransaction|Rolls back a transaction
	{ "RollbackTransaction", (PyCFunction)PyRollbackTransaction, METH_KEYWORDS|METH_VARARGS},
	// @pymeth RollbackTransactionAsync|Rolls back a transaction asynchronously
	{ "RollbackTransactionAsync", (PyCFunction)PyRollbackTransactionAsync, METH_KEYWORDS|METH_VARARGS},
	// @pymeth CommitTransaction|Commits a transaction
	{ "CommitTransaction", (PyCFunction)PyCommitTransaction, METH_KEYWORDS|METH_VARARGS},
	// @pymeth CommitTransactionAsync|Commits a transaction asynchronously
	{ "CommitTransactionAsync", (PyCFunction)PyCommitTransactionAsync, METH_KEYWORDS|METH_VARARGS},
	{ NULL, NULL }
};

extern "C" __declspec(dllexport) void
initwin32transaction(void)
{
	PyObject *dict, *mod;
	PyWinGlobals_Ensure();
	mod = Py_InitModule("win32transaction", win32transaction_functions);

	// Load dll and function pointers to avoid dependency on newer libraries and headers
	HMODULE hmodule=GetModuleHandle(L"Ktmw32.dll");
	if (hmodule==NULL)
		hmodule=LoadLibrary(L"Ktmw32.dll");
	if (hmodule){
		pfnCreateTransaction=(CreateTransactionfunc)GetProcAddress(hmodule, "CreateTransaction");
		pfnRollbackTransaction=(RollbackTransactionfunc)GetProcAddress(hmodule, "RollbackTransaction");
		pfnRollbackTransactionAsync=(RollbackTransactionAsyncfunc)GetProcAddress(hmodule, "RollbackTransactionAsync");
		pfnCommitTransaction=(CommitTransactionfunc)GetProcAddress(hmodule, "CommitTransaction");
		pfnCommitTransactionAsync=(CommitTransactionAsyncfunc)GetProcAddress(hmodule, "CommitTransactionAsync");
		pfnGetTransactionId=(GetTransactionIdfunc)GetProcAddress(hmodule, "GetTransactionId");
		pfnGetTransactionInformation=(GetTransactionInformationfunc)GetProcAddress(hmodule, "GetTransactionInformation");
		pfnSetTransactionInformation=(SetTransactionInformationfunc)GetProcAddress(hmodule, "SetTransactionInformation");
		pfnOpenTransaction=(OpenTransactionfunc)GetProcAddress(hmodule, "OpenTransaction");
		pfnOpenResourceManager=(OpenResourceManagerfunc)GetProcAddress(hmodule, "OpenResourceManager");
		pfnCreateTransactionManager=(CreateTransactionManagerfunc)GetProcAddress(hmodule, "CreateTransactionManager");
		pfnCreateResourceManager=(CreateResourceManagerfunc)GetProcAddress(hmodule, "CreateResourceManager");
		pfnOpenEnlistment=(OpenEnlistmentfunc)GetProcAddress(hmodule, "OpenEnlistment");
		pfnCreateEnlistment=(CreateEnlistmentfunc)GetProcAddress(hmodule, "CreateEnlistment");
		pfnOpenTransactionManager=(OpenTransactionManagerfunc)GetProcAddress(hmodule, "OpenTransactionManager");
		}

	dict = PyModule_GetDict(mod);
	Py_INCREF(PyWinExc_ApiError);
	PyDict_SetItemString(dict, "error", PyWinExc_ApiError);

}

/* Transaction access rights used with OpenTransaction:
TRANSACTION_QUERY_INFORMATION
TRANSACTION_SET_INFORMATION
TRANSACTION_ENLIST
TRANSACTION_COMMIT
TRANSACTION_ROLLBACK 
TRANSACTION_GENERIC_READ
TRANSACTION_GENERIC_WRITE
TRANSACTION_GENERIC_EXECUTE
TRANSACTION_ALL_ACCESS
TRANSACTION_RESOURCE_MANAGER_RIGHTS
TRANSACTION_PROPAGATE
*/

/*
Transaction manager access rights used with OpenTransactionManager
TRANSACTIONMANAGER_QUERY_INFORMATION
TRANSACTIONMANAGER_SET_INFORMATION
TRANSACTIONMANAGER_RECOVER
TRANSACTIONMANAGER_RENAME
TRANSACTIONMANAGER_CREATE_RM
TRANSACTIONMANAGER_GENERIC_READ
TRANSACTIONMANAGER_GENERIC_WRITE
TRANSACTIONMANAGER_GENERIC_EXECUTE 
TRANSACTIONMANAGER_ALL_ACCESS
*/

/*
Resource manager rights:
RESOURCEMANAGER_QUERY_INFORMATION
RESOURCEMANAGER_SET_INFORMATION
RESOURCEMANAGER_RECOVER
RESOURCEMANAGER_ENLIST
RESOURCEMANAGER_GET_NOTIFICATION
RESOURCEMANAGER_GENERIC_READ
RESOURCEMANAGER_GENERIC_WRITE
RESOURCEMANAGER_GENERIC_EXECUTE
RESOURCEMANAGER_ALL_ACCESS
*/

/* NOTIFICATION_MASK enum:
TRANSACTION_NOTIFY_MASK
TRANSACTION_NOTIFY_PREPREPARE
TRANSACTION_NOTIFY_PREPARE
TRANSACTION_NOTIFY_COMMIT
TRANSACTION_NOTIFY_ROLLBACK
TRANSACTION_NOTIFY_PREPREPARE_COMPLETE
TRANSACTION_NOTIFY_PREPARE_COMPLETE
TRANSACTION_NOTIFY_COMMIT_COMPLETE
TRANSACTION_NOTIFY_ROLLBACK_COMPLETE
TRANSACTION_NOTIFY_RECOVER
TRANSACTION_NOTIFY_SINGLE_PHASE_COMMIT
TRANSACTION_NOTIFY_DELEGATE_COMMIT
TRANSACTION_NOTIFY_RECOVER_QUERY
TRANSACTION_NOTIFY_ENLIST_PREPREPARE
TRANSACTION_NOTIFY_LAST_RECOVER
TRANSACTION_NOTIFY_INDOUBT
TRANSACTION_NOTIFY_TM_ONLINE
TRANSACTION_NOTIFY_REQUEST_OUTCOME
*/
