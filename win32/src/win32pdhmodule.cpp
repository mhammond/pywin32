/***********************************************************

win32pdh - Performance Data Helpers API interface 

Note that this source file contains embedded documentation.
This documentation consists of marked up text inside the
C comments, and is prefixed with an '@' symbol.  The source
files are processed by a tool called "autoduck" which
generates Windows .hlp files.
@doc

******************************************************************/

#include "PyWinTypes.h"
#include "pdh.h"
#include "pdhmsg.h"

/*
According to MSDN, Pdh calls are thread safe, although there was a bug
in Win2k that might make it appear to not be.  Plus, the PyW32* macros
weren't actually used most places, so would be no point in using them anywhere.

// It appears PDH it not thread safe!
// Use a critical section to protect calls into it
CRITICAL_SECTION critSec;

#define PyW32_BEGIN_ALLOW_THREADS \
	Py_BEGIN_ALLOW_THREADS \
	EnterCriticalSection(&critSec);

#define PyW32_END_ALLOW_THREADS \
	Py_END_ALLOW_THREADS \
	LeaveCriticalSection(&critSec);

#define PyW32_BLOCK_THREADS \
	Py_BLOCK_THREADS \
	LeaveCriticalSection(&critSec);
*/

// Function pointer typedefs
typedef PDH_STATUS (WINAPI * FuncPdhEnumObjects)(
    LPCTSTR szReserved,	// DataSource
    LPCTSTR szMachineName,	// machine name
    LPTSTR mszObjectList,	// buffer for objects
    LPDWORD pcchBufferLength,	// size of buffer
    DWORD dwDetailLevel,	// detail level
    BOOL bRefresh	// refresh flag for connected machines
   );

typedef PDH_STATUS (WINAPI * FuncPdhEnumObjectItems)(
    LPCTSTR szReserved,	// DataSource
    LPCTSTR szMachineName,	// machine name
    LPCTSTR szObjectName,	// object name
    LPTSTR mszCounterList,	// buffer for object's counters
    LPDWORD pcchCounterListLength,	// size of counter list buffer
    LPTSTR mszInstanceList,	// buffer for object's instances
    LPDWORD pcchInstanceListLength,	// size of instance list buffer
    DWORD dwDetailLevel,	// detail level
    DWORD dwFlags	// formatting flag
   );	

typedef PDH_STATUS (WINAPI * FuncPdhOpenQuery)(
    LPCTSTR szDataSource,	// DataSource
    DWORD_PTR dwUserData,	// a value associated with this query
    HQUERY *phQuery	// pointer to a buffer that will receive the query handle
   ) ;

typedef PDH_STATUS (WINAPI * FuncPdhCloseQuery)(
    HQUERY hQuery
   ) ;

typedef PDH_STATUS (WINAPI * FuncPdhRemoveCounter)(
    HCOUNTER hCounter
   ) ;

typedef PDH_STATUS (WINAPI * FuncPdhAddCounter)(
    HQUERY hQuery,	// handle to the query
    LPCTSTR szFullCounterPath,	// path of the counter
    DWORD_PTR dwUserData,	// user-defined value
    HCOUNTER *phCounter	// pointer to the counter handle buffer
   );	

typedef PDH_STATUS (WINAPI * FuncPdhMakeCounterPath)(
    PDH_COUNTER_PATH_ELEMENTS *pCounterPathElements,	// counter path elements
    LPTSTR szFullPathBuffer,	// path string buffer
    LPDWORD pcchBufferSize,	// size of buffer
    DWORD dwFlags	// reserved
   );	

typedef PDH_STATUS (WINAPI * FuncPdhGetCounterInfo)(
    HQUERY hCounter,	// handle of the counter
    BOOLEAN bRetrieveExplainText,	// TRUE to retrieve explain text
    LPDWORD pdwBufferSize,	// pointer to size of lpBuffer
    PPDH_COUNTER_INFO lpBuffer	// buffer for counter information
   );

typedef PDH_STATUS (WINAPI * FuncPdhGetFormattedCounterValue)(
    HCOUNTER hCounter,	// handle of the counter
    DWORD dwFormat,	// formatting flag
    LPDWORD lpdwType,	// counter type
    PPDH_FMT_COUNTERVALUE pValue	// counter value
   );	

typedef PDH_STATUS (WINAPI * FuncPdhCollectQueryData)(
    HQUERY hQuery
   );	

typedef PDH_STATUS (WINAPI * FuncPdhValidatePath)(
    LPCTSTR szFullCounterPath
   );	

typedef PDH_STATUS (WINAPI * FuncPdhExpandCounterPath)(
   LPCTSTR szWildCardPath,	// counter path to expand
   LPTSTR mszExpandedPathList,	// names that match
   LPDWORD pcchPathListLength	// size of buffer
   );

typedef PDH_STATUS (WINAPI * FuncPdhParseCounterPath)( 
  LPCTSTR szFullPathBuffer, // path string buffer
  PDH_COUNTER_PATH_ELEMENTS *pCounterPathElements, // counter path elements
  LPDWORD pdwBufferSize, // size of buffer
  DWORD dwFlags // reserved
  );

typedef PDH_STATUS (WINAPI *FuncPdhSetCounterScaleFactor) (
  HCOUNTER    hCounter,
  LONG        lFactor
);

typedef PDH_STATUS (WINAPI *FuncPdhParseInstanceName) (
  LPCTSTR  szInstanceString,
  LPTSTR   szInstanceName,
  LPDWORD pcchInstanceNameLength,
  LPTSTR   szParentName,
  LPDWORD pcchParentNameLength,
  LPDWORD lpIndex
);

typedef PDH_STATUS (WINAPI *FuncPdhBrowseCounters) (
  PPDH_BROWSE_DLG_CONFIG pBrowseDlgData
);

typedef PDH_STATUS (WINAPI *FuncPdhConnectMachine) (
  LPCTSTR szMachineName
);

typedef PDH_STATUS (WINAPI *FuncPdhLookupPerfIndexByName) (
  LPCTSTR szMachineName,
  LPCTSTR szCounterName,
  LPDWORD pdwIndex
);

typedef PDH_STATUS (WINAPI *FuncPdhLookupPerfNameByIndex) (
  LPCTSTR szMachineName,
  DWORD index,
  LPCTSTR szCounterName,
  LPDWORD pcchBuffer
);

#define CHECK_PDH_PTR(ptr) if((ptr)==NULL) { PyErr_Format(PyExc_RuntimeError, "The pdh.dll entry point function %s could not be loaded.", #ptr); return NULL;}

// The function pointers
FuncPdhEnumObjects pPdhEnumObjects = NULL;
FuncPdhEnumObjectItems pPdhEnumObjectItems = NULL;
FuncPdhOpenQuery pPdhOpenQuery = NULL;
FuncPdhCloseQuery pPdhCloseQuery = NULL;
FuncPdhRemoveCounter pPdhRemoveCounter = NULL;
FuncPdhAddCounter pPdhAddCounter = NULL;
FuncPdhAddCounter pPdhAddEnglishCounter = NULL;
FuncPdhMakeCounterPath pPdhMakeCounterPath = NULL;
FuncPdhGetCounterInfo pPdhGetCounterInfo = NULL;
FuncPdhGetFormattedCounterValue pPdhGetFormattedCounterValue = NULL;
FuncPdhCollectQueryData pPdhCollectQueryData = NULL;
FuncPdhValidatePath pPdhValidatePath = NULL;
FuncPdhExpandCounterPath pPdhExpandCounterPath = NULL;
FuncPdhParseCounterPath pPdhParseCounterPath = NULL;
FuncPdhSetCounterScaleFactor pPdhSetCounterScaleFactor = NULL;
FuncPdhParseInstanceName pPdhParseInstanceName = NULL;
FuncPdhBrowseCounters pPdhBrowseCounters = NULL;

FuncPdhConnectMachine pPdhConnectMachine = NULL;
FuncPdhLookupPerfIndexByName pPdhLookupPerfIndexByName = NULL;
FuncPdhLookupPerfNameByIndex pPdhLookupPerfNameByIndex = NULL;

// TCHAR that frees itself
class TmpTCHAR
{
public:
	TCHAR *tmp;
	TmpTCHAR() { tmp=NULL; }
	TmpTCHAR(TCHAR *t) { tmp=t; }
	TCHAR * operator= (TCHAR *t){
		PyWinObject_FreeTCHAR(tmp);
		tmp=t;
		return t;
		}
	TCHAR ** operator& () {return &tmp;}
	boolean operator== (TCHAR *t) { return tmp==t; }
	operator TCHAR *() { return tmp; }
	~TmpTCHAR() { PyWinObject_FreeTCHAR(tmp); }
};

static PyObject *win32pdh_counter_error;

// Select whether to load ansi or unicode API functions
// Module is now always built as unicode, this can go away.
#ifdef UNICODE
#define A_OR_W "W"
#else
#define A_OR_W "A"
#endif

BOOL LoadPointers()
{
	HMODULE handle = LoadLibrary(_T("pdh.dll"));
	if (handle==NULL) {
//		PyErr_SetString(PyExc_RuntimeError, "The PDH DLL could not be located");
		return FALSE;
	}
	pPdhEnumObjects = (FuncPdhEnumObjects)GetProcAddress(handle, "PdhEnumObjects" A_OR_W);
	pPdhEnumObjectItems = (FuncPdhEnumObjectItems)GetProcAddress(handle, "PdhEnumObjectItems" A_OR_W);
	pPdhCloseQuery = (FuncPdhCloseQuery)GetProcAddress(handle, "PdhCloseQuery");
	pPdhRemoveCounter = (FuncPdhRemoveCounter)GetProcAddress(handle, "PdhRemoveCounter");
	pPdhOpenQuery = (FuncPdhOpenQuery)GetProcAddress(handle, "PdhOpenQuery" A_OR_W);
	pPdhAddCounter = (FuncPdhAddCounter)GetProcAddress(handle, "PdhAddCounter" A_OR_W);
	pPdhAddEnglishCounter = (FuncPdhAddCounter)GetProcAddress(handle, "PdhAddEnglishCounter" A_OR_W);
	pPdhMakeCounterPath = (FuncPdhMakeCounterPath)GetProcAddress(handle, "PdhMakeCounterPath" A_OR_W);
	pPdhGetCounterInfo = (FuncPdhGetCounterInfo)GetProcAddress(handle, "PdhGetCounterInfo" A_OR_W);
	pPdhGetFormattedCounterValue = (FuncPdhGetFormattedCounterValue)GetProcAddress(handle, "PdhGetFormattedCounterValue");
	pPdhCollectQueryData = (FuncPdhCollectQueryData)GetProcAddress(handle, "PdhCollectQueryData");
	pPdhValidatePath	= (FuncPdhValidatePath)GetProcAddress(handle, "PdhValidatePath" A_OR_W);
	pPdhExpandCounterPath	= (FuncPdhExpandCounterPath)GetProcAddress(handle, "PdhExpandCounterPath" A_OR_W);
	pPdhParseCounterPath = (FuncPdhParseCounterPath)GetProcAddress(handle, "PdhParseCounterPath" A_OR_W);
	pPdhSetCounterScaleFactor = (FuncPdhSetCounterScaleFactor)GetProcAddress(handle, "PdhSetCounterScaleFactor");
	pPdhParseInstanceName = (FuncPdhParseInstanceName)GetProcAddress(handle, "PdhParseInstanceName" A_OR_W);
	pPdhBrowseCounters = (FuncPdhBrowseCounters)GetProcAddress(handle, "PdhBrowseCounters" A_OR_W);
	pPdhConnectMachine = (FuncPdhConnectMachine)GetProcAddress(handle, "PdhConnectMachine" A_OR_W);
	pPdhLookupPerfNameByIndex = (FuncPdhLookupPerfNameByIndex)GetProcAddress(handle, "PdhLookupPerfNameByIndex" A_OR_W);
	pPdhLookupPerfIndexByName = (FuncPdhLookupPerfIndexByName)GetProcAddress(handle, "PdhLookupPerfIndexByName" A_OR_W);

	// Pdh error codes are in 2 different ranges
	PyWin_RegisterErrorMessageModule(PDH_CSTATUS_NO_MACHINE, PDH_CANNOT_SET_DEFAULT_REALTIME_DATASOURCE, handle);
	PyWin_RegisterErrorMessageModule(PDH_CSTATUS_NO_OBJECT, PDH_QUERY_PERF_DATA_TIMEOUT, handle);
	return TRUE;
}

BOOL CheckCounterStatusOK( DWORD status )
{
	if (status == 0)
		return TRUE;
	PyObject *v = PyInt_FromLong(status);
	PyErr_SetObject(win32pdh_counter_error, v);
	Py_DECREF(v);
	return FALSE;
}

// @pymethod tuple|win32pdh|EnumObjectItems|Enumerates an object's items
static PyObject *PyEnumObjectItems(PyObject *self, PyObject *args)
{
	DWORD detailLevel, flags = 0;
	PyObject *obDataSource, *obMachine, *obObject, *ret=NULL;
	if (!PyArg_ParseTuple(args, "OOOk|k:EnumObjectItems", 
	          &obDataSource, // @pyparm string|DataSource||Path of a performance log file, or None for machine counters
	          &obMachine, // @pyparm string|machine||The machine to use, or None
			  &obObject, // @pyparm string|object||The type of object
	          &detailLevel, // @pyparm int|detailLevel||The level of data required, win32pdh.PERF_DETAIL_*
	          &flags)) // @pyparm int|flags|0|Flags - must be zero
		return NULL;

    LPTSTR      szCounterListBuffer     = NULL;
    DWORD       dwCounterListSize       = 0;
    LPTSTR      szInstanceListBuffer    = NULL;
    DWORD       dwInstanceListSize      = 0;

	CHECK_PDH_PTR(pPdhEnumObjectItems);

	TmpTCHAR Machine, Object, DataSource;
	if (!PyWinObject_AsTCHAR(obDataSource, &DataSource, TRUE))
		return NULL;
	if (!PyWinObject_AsTCHAR(obMachine, &Machine, TRUE))
		return NULL;
	if (!PyWinObject_AsTCHAR(obObject, &Object, FALSE))
		return NULL;

	PDH_STATUS pdhStatus;

	Py_BEGIN_ALLOW_THREADS
	pdhStatus = (*pPdhEnumObjectItems) (
        DataSource,                   // Perf log file
        Machine,                   // local machine
        Object,        // object to enumerate
        szCounterListBuffer,    // pass in NULL buffers
        &dwCounterListSize,     // an 0 length to get
        szInstanceListBuffer,   // required size 
        &dwInstanceListSize,    // of the buffers in chars
        detailLevel,     // counter detail level
        flags); 
	Py_END_ALLOW_THREADS

	// it appears NT/2k will return 0, while XP will return
	// PDH_MORE_DATA
	if (pdhStatus != ERROR_SUCCESS && pdhStatus != PDH_MORE_DATA)
		return PyWin_SetAPIError("EnumObjectItems for buffer size", pdhStatus);

	// Allocate the buffers and try the call again.
	if (dwCounterListSize) {
		szCounterListBuffer = (LPTSTR)malloc (dwCounterListSize * sizeof (TCHAR));
		if (szCounterListBuffer==NULL) {
			PyErr_NoMemory();
			goto cleanup;
		}
	}

	if (dwInstanceListSize) {
		szInstanceListBuffer = (LPTSTR)malloc (dwInstanceListSize * sizeof (TCHAR));
		if (szInstanceListBuffer==NULL) {
			PyErr_NoMemory();
			goto cleanup;
		}
	}

	Py_BEGIN_ALLOW_THREADS
	pdhStatus = (*pPdhEnumObjectItems) (
	        DataSource,                   // Perf log file
	        Machine,                   // local machine
	        Object,        // object to enumerate
	        szCounterListBuffer,    // pass in NULL buffers
	        &dwCounterListSize,     // an 0 length to get
	        szInstanceListBuffer,   // required size 
	        &dwInstanceListSize,    // of the buffers in chars
	        detailLevel,     // counter detail level
	        flags); 
	Py_END_ALLOW_THREADS

	if (pdhStatus != ERROR_SUCCESS)
		PyWin_SetAPIError("EnumObjectItems for data", pdhStatus);
	else
		ret = Py_BuildValue("NN",
			szCounterListBuffer ? PyWinObject_FromMultipleString(szCounterListBuffer) : PyList_New(0),
			szInstanceListBuffer ? PyWinObject_FromMultipleString(szInstanceListBuffer) : PyList_New(0));

cleanup:
	if (szInstanceListBuffer)
		free(szInstanceListBuffer);
	if (szCounterListBuffer)
		free(szCounterListBuffer);
	return ret;
}

// @pymethod list|win32pdh|EnumObjects|Enumerates objects
static PyObject *PyEnumObjects(PyObject *self, PyObject *args)
{
	DWORD detailLevel, refresh=1;
	PyObject *obMachine, *obDataSource, *ret=NULL;
	if (!PyArg_ParseTuple(args, "OOi|i:EnumObjects", 
	          &obDataSource, // @pyparm string|DataSource||Path to a performance log file, or None for machine counters
	          &obMachine, // @pyparm string|machine||The machine to use, or None
	          &detailLevel, // @pyparm int|detailLevel||The level of data required.
	          &refresh)) // @pyparm int|refresh|1|Should the list be refreshed.
		return NULL;

    LPTSTR      szObjectListBuffer     = NULL;
    DWORD       dwObjectListSize       = 0;

	CHECK_PDH_PTR(pPdhEnumObjects);
	TmpTCHAR DataSource, Machine;
	if (!PyWinObject_AsTCHAR(obDataSource, &DataSource, TRUE))
		return NULL;
	if (!PyWinObject_AsTCHAR(obMachine, &Machine, TRUE))
		return NULL;

	PDH_STATUS pdhStatus;

	Py_BEGIN_ALLOW_THREADS
	pdhStatus = (*pPdhEnumObjects) (
        DataSource,                   // perf log file
        Machine,                   // local machine
        szObjectListBuffer,    // pass in NULL buffers
        &dwObjectListSize,     // an 0 length to get
        detailLevel,     // counter detail level
        refresh); 
	Py_END_ALLOW_THREADS

	// it appears NT/2k will return 0, while XP will return
	// PDH_MORE_DATA
	if (pdhStatus != ERROR_SUCCESS && pdhStatus != PDH_MORE_DATA)
		return PyWin_SetAPIError("EnumObjects for buffer size", pdhStatus);

    // Allocate the buffers and try the call again.
	if (dwObjectListSize) {
		szObjectListBuffer = (LPTSTR)malloc (dwObjectListSize * sizeof (TCHAR));
		if (szObjectListBuffer==NULL) {
			PyErr_NoMemory();
			return NULL;
		}
	}

	Py_BEGIN_ALLOW_THREADS
	pdhStatus = (*pPdhEnumObjects) (
	        DataSource,                   // Perf log file
	        Machine,                   // local machine
	        szObjectListBuffer,    // pass in NULL buffers
	        &dwObjectListSize,     // an 0 length to get
	        detailLevel,     // counter detail level
	        0); 
	Py_END_ALLOW_THREADS

	if (pdhStatus != ERROR_SUCCESS)
		PyWin_SetAPIError("EnumObjects for data", pdhStatus);
	else
		ret = szObjectListBuffer ? PyWinObject_FromMultipleString(szObjectListBuffer) : PyList_New(0);

	if (szObjectListBuffer)
		free(szObjectListBuffer);
	return ret;
}

// @pymethod int|win32pdh|AddCounter|Adds a new counter
static PyObject *PyAddCounter(PyObject *self, PyObject *args)
{
	HQUERY hQuery;
	PyObject *obhQuery;
	PyObject *obPath;
	PyObject *obuserData = Py_None;	// Might make more sense to use actual PyObject for userData
	DWORD_PTR userData = 0;
	CHECK_PDH_PTR(pPdhAddCounter);
	PDH_STATUS pdhStatus;
	if (!PyArg_ParseTuple(args, "OO|O:AddCounter", 
	          &obhQuery, // @pyparm int|hQuery||Handle to an open query.
	          &obPath, // @pyparm string|path||Full path to the performance data
	          &obuserData)) // @pyparm int|userData|0|User data associated with the counter.
		return NULL;
	if (!PyWinObject_AsHANDLE(obhQuery, &hQuery))
		return NULL;
	if (obuserData != Py_None)
		if (!PyWinLong_AsDWORD_PTR(obuserData, &userData))
			return NULL;
	TCHAR *szPath;
	if (!PyWinObject_AsTCHAR(obPath, &szPath, FALSE))
		return NULL;
	HCOUNTER hCounter;

	Py_BEGIN_ALLOW_THREADS
	pdhStatus = (*pPdhAddCounter) (
        hQuery,
        szPath,
	    userData,
	    &hCounter);

	Py_END_ALLOW_THREADS;
	PyWinObject_FreeTCHAR(szPath);
	if (pdhStatus != ERROR_SUCCESS) 
		return PyWin_SetAPIError("AddCounter", pdhStatus);
	// @comm See also <om win32pdh.RemoveCounter>
	return PyWinLong_FromHANDLE(hCounter);
}

// @pymethod int|win32pdh|AddEnglishCounter|Adds a counter to a query by its English name
// @comm Available on Vista and later
// @rdesc Returns a handle to the counter
static PyObject *PyAddEnglishCounter(PyObject *self, PyObject *args)
{
	HQUERY hQuery;
	PyObject *obhQuery;
	PyObject *obPath;
	PyObject *obuserData = Py_None;	// Might make more sense to use actual PyObject for userData
	DWORD_PTR userData = 0;
	CHECK_PDH_PTR(pPdhAddEnglishCounter);
	PDH_STATUS pdhStatus;
	if (!PyArg_ParseTuple(args, "OO|O:AddEnglishCounter", 
	          &obhQuery, // @pyparm int|hQuery||Handle to an open query.
	          &obPath, // @pyparm string|path||Full counter path with standard English names.
	          &obuserData)) // @pyparm int|userData|0|User data associated with the counter.
		return NULL;
	if (!PyWinObject_AsHANDLE(obhQuery, &hQuery))
		return NULL;
	if (obuserData != Py_None)
		if (!PyWinLong_AsDWORD_PTR(obuserData, &userData))
			return NULL;
	TCHAR *szPath;
	if (!PyWinObject_AsTCHAR(obPath, &szPath, FALSE))
		return NULL;
	HCOUNTER hCounter;

	Py_BEGIN_ALLOW_THREADS
	pdhStatus = (*pPdhAddEnglishCounter) (
        hQuery,
        szPath,
	    userData,
	    &hCounter);

	Py_END_ALLOW_THREADS;
	PyWinObject_FreeTCHAR(szPath);
	if (pdhStatus != ERROR_SUCCESS) 
		return PyWin_SetAPIError("AddEnglishCounter", pdhStatus);
	// @comm See also <om win32pdh.RemoveCounter>
	return PyWinLong_FromHANDLE(hCounter);
}

// @pymethod |win32pdh|RemoveCounter|Removes a previously opened counter
static PyObject *PyRemoveCounter(PyObject *self, PyObject *args)
{
	HQUERY handle;
	PyObject *obhandle;
	PDH_STATUS pdhStatus;
	if (!PyArg_ParseTuple(args, "O:RemoveCounter", 
	          &obhandle)) // @pyparm int|handle||Handle to an open counter.
		return NULL;
	if (!PyWinObject_AsHANDLE(obhandle, &handle))
		return NULL;
	// @comm See also <om win32pdh.AddCounter>
	CHECK_PDH_PTR(pPdhRemoveCounter);
	Py_BEGIN_ALLOW_THREADS
	pdhStatus = (*pPdhRemoveCounter) (handle);
	Py_END_ALLOW_THREADS

    if (pdhStatus != ERROR_SUCCESS) 
		return PyWin_SetAPIError("RemoveCounter", pdhStatus);
	Py_INCREF(Py_None);
	return Py_None;
}


// @pymethod int|win32pdh|OpenQuery|Opens a new query
static PyObject *PyOpenQuery(PyObject *self, PyObject *args)
{
	DWORD_PTR userData = 0;
	TCHAR *DataSource = NULL;
	PyObject *obDataSource = Py_None, *obuserData = Py_None;
	if (!PyArg_ParseTuple(args, "|OO:OpenQuery", 
	          &obDataSource, // @pyparm str|DataSource|None|Name of a performaance log file, or None for live data
	          &obuserData)) // @pyparm int|userData|0|User data associated with the query.
		return NULL;

	HQUERY hQuery;
	CHECK_PDH_PTR(pPdhOpenQuery);
	PDH_STATUS pdhStatus;
	if (obuserData != Py_None)
		if (!PyWinLong_AsDWORD_PTR(obuserData, &userData))
			return NULL;
	if (!PyWinObject_AsTCHAR(obDataSource, &DataSource, TRUE))
		return NULL;

	Py_BEGIN_ALLOW_THREADS
	pdhStatus = (*pPdhOpenQuery) (
        DataSource,
	    userData,
	    &hQuery);
	Py_END_ALLOW_THREADS

	PyWinObject_FreeTCHAR(DataSource);
    if (pdhStatus != ERROR_SUCCESS) 
		return PyWin_SetAPIError("OpenQuery", pdhStatus);
	return PyWinLong_FromHANDLE(hQuery);
	// @comm See also <om win32pdh.CloseQuery>
}

// @pymethod |win32pdh|CloseQuery|Closes a query
static PyObject *PyCloseQuery(PyObject *self, PyObject *args)
{
	HQUERY handle;
	PyObject *obhandle;
	PDH_STATUS pdhStatus;
	if (!PyArg_ParseTuple(args, "O:CloseQuery", 
			&obhandle)) // @pyparm int|handle||Handle to an open query.
		return NULL;
	if (!PyWinObject_AsHANDLE(obhandle, &handle))
		return NULL;
	// @comm See also <om win32pdh.OpenQuery>
	CHECK_PDH_PTR(pPdhCloseQuery);
	Py_BEGIN_ALLOW_THREADS
	pdhStatus = (*pPdhCloseQuery) (handle);
	Py_END_ALLOW_THREADS

	if (pdhStatus != ERROR_SUCCESS) 
		return PyWin_SetAPIError("CloseQuery", pdhStatus);
	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod |win32pdh|MakeCounterPath|Makes a fully resolved counter path
static PyObject *PyMakeCounterPath(PyObject *self, PyObject *args)
{
	CHECK_PDH_PTR(pPdhMakeCounterPath);
	PyObject *ret = NULL;
	TCHAR *szResult=NULL;
	DWORD bufSize = PDH_MAX_COUNTER_PATH;
	DWORD flags = 0;

	PyObject *obMachineName, *obObjectName, *obInstanceName, *obParentInstance, *obCounterName;
	PDH_COUNTER_PATH_ELEMENTS cpe;
	memset(&cpe, 0, sizeof(cpe));

	// @pyparm (machineName, objectName, instanceName, parentInstance, instanceIndex, counterName)|elements||The elements to use to create the path.
	// @pyparm int|flags|0|PDH_PATH_WBEM_RESULT, PDH_PATH_WBEM_INPUT, or 0
	if (!PyArg_ParseTuple(args, "(OOOOkO)|l:MakeCounterPath", 
			&obMachineName,
			&obObjectName,
			&obInstanceName,
			&obParentInstance,
			&cpe.dwInstanceIndex,
			&obCounterName,
			&flags))
		return NULL;

	if (!PyWinObject_AsTCHAR(obMachineName, &cpe.szMachineName, TRUE))
		goto done;
	if (!PyWinObject_AsTCHAR(obObjectName, &cpe.szObjectName, FALSE))
		goto done;
	if (!PyWinObject_AsTCHAR(obInstanceName, &cpe.szInstanceName, TRUE))
		goto done;
	if (!PyWinObject_AsTCHAR(obParentInstance, &cpe.szParentInstance, TRUE))
		goto done;
	if (!PyWinObject_AsTCHAR(obCounterName, &cpe.szCounterName, FALSE))
		goto done;
	szResult = (TCHAR *)malloc(bufSize * sizeof(TCHAR));
	if (szResult == NULL){
		PyErr_NoMemory();
		goto done;
		}

	PDH_STATUS pdhStatus;
	Py_BEGIN_ALLOW_THREADS
	pdhStatus = (*pPdhMakeCounterPath) (&cpe, szResult, &bufSize, flags);
	Py_END_ALLOW_THREADS
    if (pdhStatus != ERROR_SUCCESS)
    	PyWin_SetAPIError("PdhMakeCounterPath", pdhStatus);
    else
    	ret = PyWinObject_FromTCHAR(szResult);

done:
	if (szResult)
		free(szResult);
	PyWinObject_FreeTCHAR(cpe.szMachineName);
	PyWinObject_FreeTCHAR(cpe.szObjectName);
	PyWinObject_FreeTCHAR(cpe.szInstanceName);
	PyWinObject_FreeTCHAR(cpe.szParentInstance);
	PyWinObject_FreeTCHAR(cpe.szCounterName);
    return ret;
}

// @pymethod |win32pdh|GetCounterInfo|Retrieves information about a counter, such as data size, counter type, path, and user-supplied data values.
static PyObject *PyGetCounterInfo(PyObject *self, PyObject *args)
{
	HCOUNTER handle;
	PyObject *obhandle;
	BOOL bExplainText = TRUE;	
	if (!PyArg_ParseTuple(args, "O|i:GetCounterInfo", 
			&obhandle, // @pyparm int|handle||The handle of the item to query
			&bExplainText)) // @pyparm int|bRetrieveExplainText||Should explain text be retrieved?
		return NULL;
	if (!PyWinObject_AsHANDLE(obhandle, &handle))
		return NULL;
	// First call to get buffer size
	DWORD bufSize = 0;
	CHECK_PDH_PTR(pPdhGetCounterInfo);
	PDH_STATUS pdhStatus;
	Py_BEGIN_ALLOW_THREADS
	pdhStatus = (*pPdhGetCounterInfo) (handle, bExplainText, &bufSize, NULL);
	Py_END_ALLOW_THREADS
	// as usual, pre-xp returns ERROR_SUCCESS, xp returns PDH_MORE_DATA
	if (pdhStatus != ERROR_SUCCESS && pdhStatus != PDH_MORE_DATA) 
		return PyWin_SetAPIError("GetCounterInfo for size", pdhStatus);
		
	PPDH_COUNTER_INFO pInfo = (PPDH_COUNTER_INFO)malloc(bufSize);
	if (pInfo==NULL) {
		PyErr_SetString(PyExc_MemoryError, "Allocating result buffer");
		return NULL;
	}
	pInfo->dwLength = bufSize;

	Py_BEGIN_ALLOW_THREADS
	pdhStatus = (*pPdhGetCounterInfo)(handle, bExplainText, &bufSize, pInfo);
	Py_END_ALLOW_THREADS
	PyObject *rc;
	if (pdhStatus != ERROR_SUCCESS)
		rc = PyWin_SetAPIError("GetCounterInfo for data", pdhStatus);
	else {
		if (!CheckCounterStatusOK(pInfo->CStatus))
			rc = NULL;
		else 
			rc = Py_BuildValue("iiiiNNN(NNNNiN)N",
				pInfo->dwType,
				pInfo->CVersion,
				// ??? CStatus is missing ???
				pInfo->lScale,
				pInfo->lDefaultScale,
				PyWinObject_FromDWORD_PTR(pInfo->dwUserData),
				PyWinObject_FromDWORD_PTR(pInfo->dwQueryUserData),
				PyWinObject_FromTCHAR(pInfo->szFullPath),
						  
				PyWinObject_FromTCHAR(pInfo->szMachineName),
				PyWinObject_FromTCHAR(pInfo->szObjectName),
				PyWinObject_FromTCHAR(pInfo->szInstanceName),
				PyWinObject_FromTCHAR(pInfo->szParentInstance),
				pInfo->dwInstanceIndex,
				PyWinObject_FromTCHAR(pInfo->szCounterName),
			
				PyWinObject_FromTCHAR(pInfo->szExplainText));
    }
    free(pInfo);
    return rc;
}


// @pymethod (int,object)|win32pdh|GetFormattedCounterValue|Retrieves a formatted counter value
static PyObject *PyGetFormattedCounterValue(PyObject *self, PyObject *args)
{
	HCOUNTER handle;
	PyObject *obhandle;
	DWORD format;
	if (!PyArg_ParseTuple(args, "Oi:GetFormattedCounterValue", 
			&obhandle, // @pyparm int|handle||Handle to the counter
			&format)) // @pyparm int|format||Format of result.  Can be PDH_FMT_DOUBLE, PDH_FMT_LARGE, PDH_FMT_LONG and or'd with PDH_FMT_NOSCALE, PDH_FMT_1000

		return NULL;
	if (!PyWinObject_AsHANDLE(obhandle, &handle))
		return NULL;
	DWORD type;
	PDH_FMT_COUNTERVALUE result;
	CHECK_PDH_PTR(pPdhGetFormattedCounterValue);
	PDH_STATUS pdhStatus;
	Py_BEGIN_ALLOW_THREADS
	pdhStatus = (*pPdhGetFormattedCounterValue) (handle, format, &type, &result);
	Py_END_ALLOW_THREADS
    if (pdhStatus != ERROR_SUCCESS) 
		return PyWin_SetAPIError("GetFormattedCounterValue", pdhStatus);
	if (!CheckCounterStatusOK(result.CStatus))
		return NULL;

	PyObject *rc;
	if (format & PDH_FMT_DOUBLE)
		rc = PyFloat_FromDouble(result.doubleValue);
	else if (format & PDH_FMT_LONG)
		rc = PyInt_FromLong(result.longValue);
	else if (format & PDH_FMT_LARGE)
		rc = PyLong_FromLongLong(result.largeValue);
	else {
		PyErr_SetString(PyExc_ValueError, "Dont know how to convert the result");
		rc = NULL;
	}
	PyObject *realrc = Py_BuildValue("iO", type, rc);
	Py_XDECREF(rc);
	return realrc;
}

// @pymethod |win32pdh|CollectQueryData|Collects the current raw data value for all counters in the specified query and updates the status code of each counter.
static PyObject *PyCollectQueryData(PyObject *self, PyObject *args)
{
	HQUERY hQuery;
	PyObject *obhQuery;
	if (!PyArg_ParseTuple(args, "O:CollectQueryData", 
	          &obhQuery)) // @pyparm int|hQuery||Handle to an open query.
		return NULL;
	if (!PyWinObject_AsHANDLE(obhQuery, &hQuery))
		return NULL;
	CHECK_PDH_PTR(pPdhCollectQueryData);
	PDH_STATUS pdhStatus;
	Py_BEGIN_ALLOW_THREADS
	pdhStatus = (*pPdhCollectQueryData) (hQuery);
	Py_END_ALLOW_THREADS

    if (pdhStatus != ERROR_SUCCESS) 
		return PyWin_SetAPIError("CollectQueryData", pdhStatus);
	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod int|win32pdh|ValidatePath|Validates that the specified counter is present on the machine specified in the counter path.
static PyObject *PyValidatePath(PyObject *self, PyObject *args)
{
	PyObject *obPath;
	if (!PyArg_ParseTuple(args, "O:ValidatePath", 
	          &obPath)) // @pyparm string|path||The counter path to validate.
		return NULL;

	CHECK_PDH_PTR(pPdhValidatePath);

	TCHAR *path;
	if (!PyWinObject_AsTCHAR(obPath, &path, FALSE))
		return NULL;

    PDH_STATUS pdhStatus = (*pPdhValidatePath) (path);

	PyWinObject_FreeTCHAR(path);

	return PyInt_FromLong(pdhStatus);
	// @comm This method returns an integer result code.  No exception is
	// ever thrown.  Zero result indicates success.
}

// @pymethod [string,]|win32pdh|ExpandCounterPath|Examines the specified machine (or local machine if none is specified) for counters and instances of counters that match the wild card strings in the counter path.
static PyObject *PyExpandCounterPath(PyObject *self, PyObject *args)
{
	/* There is a large memory leak evident in the underlying API function on WinXP.
	{
		TCHAR buf[4096];
		DWORD dwSize=4096;
		for (int i=0;i<10000;i++){
			PDH_STATUS pdhStatus=(*pPdhExpandCounterPath)(_T("\\\\yourmachinename\\Memory\\*"), buf, &dwSize);
		}
	}
	*/
	PyObject *obPath;
	if (!PyArg_ParseTuple(args, "O:ExpandCounterPath", 
	          &obPath)) // @pyparm string|wildCardPath||The counter path to expand.
		return NULL;

	TmpTCHAR path;
	CHECK_PDH_PTR(pPdhExpandCounterPath);

	if (!PyWinObject_AsTCHAR(obPath, &path, FALSE))
		return NULL;

	DWORD dwSize = 0;
	PDH_STATUS pdhStatus;
	Py_BEGIN_ALLOW_THREADS
	pdhStatus = (*pPdhExpandCounterPath) (path, NULL, &dwSize);
	Py_END_ALLOW_THREADS
	
	if (dwSize == 0)
		return PyWin_SetAPIError("ExpandCounterPath for size", pdhStatus);
	dwSize++;
	TCHAR *buf = (TCHAR *)malloc(dwSize * sizeof(TCHAR));
	if (buf==NULL){
		PyErr_NoMemory();
		return NULL;
		}

	Py_BEGIN_ALLOW_THREADS
	pdhStatus = (*pPdhExpandCounterPath) (path, buf, &dwSize);
	Py_END_ALLOW_THREADS
	PyObject *rc;
	if (pdhStatus != ERROR_SUCCESS) 
		rc = PyWin_SetAPIError("ExpandCounterPath for data", pdhStatus);
	else
		rc = PyWinObject_FromMultipleString(buf);
	free(buf);
	return rc;
	// @comm The counter path format is assumed to be:
	// <nl>\\machine\object(parent/instance#index)\countername
	// <nl>and the parent, instance, index, and countername elements
	// may contain either a valid name or a wild card character.
	// @comm The API function leaks memory on Windows XP.
}

// @pymethod (machineName, objectName, instanceName, parentInstance, instanceIndex, counterName)|win32pdh|ParseCounterPath|Parses the elements of the counter path.
static PyObject *PyParseCounterPath(PyObject *self, PyObject *args)
{
	PyObject *obPath;
	int flags = 0;
	if (!PyArg_ParseTuple(args, "O|i:ParseCounterPath", 
	          &obPath,   // @pyparm string|path||The counter path to parse.
			  &flags)) // @pyparm int|flags|0|Reserved - must be zero.
		return NULL;
	TmpTCHAR path;
	if (!PyWinObject_AsTCHAR(obPath, &path, FALSE))
		return NULL;

	CHECK_PDH_PTR(pPdhParseCounterPath);
	DWORD size=0;
	PDH_STATUS pdhStatus;

	Py_BEGIN_ALLOW_THREADS
	pdhStatus = (*pPdhParseCounterPath) (path, NULL, &size, flags);
	Py_END_ALLOW_THREADS
    if (size==0)
		return PyWin_SetAPIError("ParseCounterPath for size", pdhStatus);
	void *pBuf = malloc(size);
	if (pBuf==NULL) {
		PyErr_SetString(PyExc_MemoryError, "allocating buffer for PDH_COUNTER_PATH_ELEMENTS(+strings)");
		return NULL;
	}

	PDH_COUNTER_PATH_ELEMENTS *pCPE = (PDH_COUNTER_PATH_ELEMENTS *)pBuf;
	Py_BEGIN_ALLOW_THREADS
	pdhStatus = (*pPdhParseCounterPath) (path, pCPE, &size, flags);
	Py_END_ALLOW_THREADS

	PyObject *rc;
	if (pdhStatus != 0) {
		rc = PyWin_SetAPIError("ParseCounterPath", pdhStatus);
	} else {
		rc = Py_BuildValue("NNNNkN", 
			PyWinObject_FromTCHAR(pCPE->szMachineName),
			PyWinObject_FromTCHAR(pCPE->szObjectName),
			PyWinObject_FromTCHAR(pCPE->szInstanceName),
			PyWinObject_FromTCHAR(pCPE->szParentInstance),
			pCPE->dwInstanceIndex,
			PyWinObject_FromTCHAR(pCPE->szCounterName));
	}
	free(pBuf);
	return rc;
}

// @pymethod (name, parent, instance)|win32pdh|ParseInstanceName|Parses the elements of the instance name
static PyObject *PyParseInstanceName(PyObject *self, PyObject *args)
{
	PyObject *obiname;
	PDH_STATUS pdhStatus;
	if (!PyArg_ParseTuple(args, "O:ParseInstanceName", 
	          &obiname))// @pyparm string|instanceName||The instance name to parse.
		return NULL;
	TCHAR *iname;
	if (!PyWinObject_AsTCHAR(obiname, &iname, FALSE))
		return NULL;

	CHECK_PDH_PTR(pPdhParseInstanceName);
	TCHAR szName[_MAX_PATH], szParent[_MAX_PATH];
	DWORD nameSize = _MAX_PATH, parentSize = _MAX_PATH;
	DWORD dwInstance;
	Py_BEGIN_ALLOW_THREADS
    pdhStatus = (*pPdhParseInstanceName) (iname, szName, &nameSize, szParent, &parentSize, &dwInstance);
	Py_END_ALLOW_THREADS
	PyWinObject_FreeTCHAR(iname);
	if (pdhStatus != 0)
		return PyWin_SetAPIError("ParseInstanceName", pdhStatus);
	return Py_BuildValue("NNk",
		PyWinObject_FromTCHAR(szName),
		PyWinObject_FromTCHAR(szParent),
		dwInstance);
}

// @pymethod |win32pdh|SetCounterScaleFactor|Sets the scale factor that is applied to the calculated value of the specified counter when you request the formatted counter value.
static PyObject *PySetCounterScaleFactor(PyObject *self, PyObject *args)
{
	HCOUNTER hCounter;
	PDH_STATUS pdhStatus;
	PyObject *obhCounter;
	LONG lFactor;
	if (!PyArg_ParseTuple(args, "Ol:SetCounterScaleFactor", 
	          &obhCounter, // @pyparm int|hCounter||Handle to the counter.
			  &lFactor)) // @pyparm int|factor||power of ten used to multiply value.
		return NULL;
	if (!PyWinObject_AsHANDLE(obhCounter, &hCounter))
		return NULL;
	CHECK_PDH_PTR(pPdhSetCounterScaleFactor);
	Py_BEGIN_ALLOW_THREADS
    pdhStatus = (*pPdhSetCounterScaleFactor) (hCounter, lFactor);
	Py_END_ALLOW_THREADS
	if (pdhStatus != 0)
		return PyWin_SetAPIError("SetCounterScaleFactor", pdhStatus);
	Py_INCREF(Py_None);
	return Py_None;
}

typedef struct {
	PyObject *func;
	PDH_BROWSE_DLG_CONFIG cfg;
	BOOL bReturnMultiple;
	PyObject *callbackarg;
} MY_DLG_CONFIG;


PDH_STATUS __stdcall PyCounterPathCallback(DWORD_PTR dwArg)
{
	MY_DLG_CONFIG *pMy = (MY_DLG_CONFIG *)dwArg;
	// Resize buffer and retry if status indicates so
	if (pMy->cfg.CallBackStatus == PDH_MORE_DATA){
		DWORD newsize = pMy->cfg.cchReturnPathLength * 2;
		TCHAR *newbuf=(TCHAR *)malloc(newsize * sizeof(TCHAR));
		if (newbuf==NULL)
			return ERROR_OUTOFMEMORY;
		free(pMy->cfg.szReturnPathBuffer);
		pMy->cfg.szReturnPathBuffer=newbuf;
		pMy->cfg.cchReturnPathLength=newsize;
		return PDH_RETRY;
		}
	else if (pMy->cfg.CallBackStatus != ERROR_SUCCESS)
		return pMy->cfg.CallBackStatus;	// ??? Maybe print error here also ???

	CEnterLeavePython _celp;
	/* Buffer is actually MULTI_SZ format, may contain multiple counter paths.
		Previously, only first path was passed to callback, now controlled by
		ReturnMultiple arg to BrowseCounters.
		CallBackArg also was not present, so pass only counter path(s) when it's not given
		to avoid breaking older callback functions that only expect a single arg.
	*/
	PyObject *args=NULL, *result=NULL;
	DWORD rc;
	if (pMy->callbackarg)
		args = Py_BuildValue("(NO)", 
			pMy->bReturnMultiple ? PyWinObject_FromMultipleString(pMy->cfg.szReturnPathBuffer)
								 : PyWinObject_FromTCHAR(pMy->cfg.szReturnPathBuffer),
			pMy->callbackarg);
	else
		args = Py_BuildValue("(N)", 
			pMy->bReturnMultiple ? PyWinObject_FromMultipleString(pMy->cfg.szReturnPathBuffer)
								 : PyWinObject_FromTCHAR(pMy->cfg.szReturnPathBuffer));
	if (args == NULL){
		PyErr_Print();	// Called asynchronously, best we can do is print the exception
		rc = ERROR_OUTOFMEMORY;
		}
	else{
		result = PyEval_CallObject(pMy->func, args);
		if (result==NULL) {
			PyErr_Print();	// *Don't* leave exception hanging
			rc = PDH_INVALID_DATA;
			}
		// Previous implementation did not require a return value, so assume success if None
		else if (result==Py_None)
			rc = ERROR_SUCCESS;
		else{
			rc = PyInt_AsLong(result);
			if (rc == -1 && PyErr_Occurred()){
				PyErr_Print();	// *Don't* leave exception hanging
				rc = PDH_INVALID_DATA;
				}
			}
		}
	Py_XDECREF(args);
	Py_XDECREF(result);
	return rc;
}

#define SET_BOOL(r, i) { \
	if (i<seqLen){ \
		PyObject *subItem = PyTuple_GET_ITEM(flags_tuple, i); \
		myCfg.cfg.##r = PyObject_IsTrue(subItem); \
	} \
}

// @pymethod string|win32pdh|BrowseCounters|Displays the counter browsing dialog box so that the user can select the counters to be returned to the caller. 
static PyObject *PyBrowseCounters(PyObject *self, PyObject *args, PyObject *kwargs)
{
	PyObject *obFlags, *obhwnd;
	PyObject *obDialogBoxCaption=Py_None, *obInitialPath=Py_None, *obDataSource=Py_None;
	PyObject *ret=NULL;
	MY_DLG_CONFIG myCfg;
	TCHAR *InitialPath=NULL;
	DWORD cchInitialPath;
	CHECK_PDH_PTR(pPdhBrowseCounters);
	PDH_STATUS pdhStatus;
	static char *keywords[] = {"Flags", "hWndOwner", "CallBack", "DefaultDetailLevel", "DialogBoxCaption",
		"InitialPath", "DataSource", "ReturnMultiple", "CallBackArg", NULL};
  
	// Note - this has set caption and others to default of zero.
	memset(&myCfg, 0, sizeof(myCfg));
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "OOOk|OOOlO:BrowseCounters", keywords, 
	          &obFlags, // @pyparm (boolean, ...)|Flags||Sequence of boolean flags, or None. All default to False.
						// (bIncludeInstanceIndex, bSingleCounterPerAdd, bSingleCounterPerDialog, bLocalCountersOnly, bWildCardInstances,
						// bHideDetailBox, bInitializePath, bDisableMachineSelection, bIncludeCostlyObjects, bShowObjectBrowser)
			  &obhwnd, // @pyparm <o PyHANDLE>|hWndOwner||Parent for the dialog.
			  &myCfg.func, // @pyparm object|CallBack||A callable object to function as the callback.
			  &myCfg.cfg.dwDefaultDetailLevel, // @pyparm int|DefaultDetailLevel||The default detail level to show on startup in the Detail Level combo box. If the Detail Level combo box is not shown, this is the detail level to use in filtering the displayed performance counters and objects. 
			  &obDialogBoxCaption, // @pyparm string|DialogBoxCaption|None|The dialog caption, or None for default.
			  &obInitialPath,	// @pyparm str|InitialPath|None|Counter to be selected initially, or None for default
			  &obDataSource,	// @pyparm str|DataSource|None|Name of a performance log file, or None for live counters
			  &myCfg.bReturnMultiple, // @pyparm boolean|ReturnMultiple|False|Return all selected counter paths as a sequence of strings.
									// Previously, this function only returned a single path even when multiple counters were selected.
			  &myCfg.callbackarg))	// @pyparm object|CallBackArg|None|Extra argument to be passed to callback function.  For backward compatibility,
									// the callback will only receive a single argument if this is not given.
			  return NULL;
	if (!PyWinObject_AsHANDLE(obhwnd, (HANDLE *)&myCfg.cfg.hWndOwner))
		return NULL;
	if (!PyCallable_Check(myCfg.func)) {
		PyErr_SetString(PyExc_TypeError, "The callback object must be a callable object");
		return NULL;
	}

	if (obFlags!=Py_None) {
		DWORD seqLen;
		PyObject *flags_tuple = PyWinSequence_Tuple(obFlags, &seqLen);
		if (flags_tuple == NULL)
			return NULL;
		SET_BOOL(bIncludeInstanceIndex, 0);
		SET_BOOL(bSingleCounterPerAdd, 1);
		SET_BOOL(bSingleCounterPerDialog, 2);
		SET_BOOL(bLocalCountersOnly, 3);
		SET_BOOL(bWildCardInstances, 4);
		SET_BOOL(bHideDetailBox, 5);
		SET_BOOL(bInitializePath, 6);
		SET_BOOL(bDisableMachineSelection, 7);
		SET_BOOL(bIncludeCostlyObjects, 8);
		SET_BOOL(bShowObjectBrowser, 9);
		Py_DECREF(flags_tuple);
	}

	myCfg.cfg.dwCallBackArg = (DWORD_PTR)&myCfg;
	myCfg.cfg.pCallBack = PyCounterPathCallback;

	// Initialize the return buffer if starting path is passed in. (bInitializePath will also be set)
	if (!PyWinObject_AsTCHAR(obInitialPath, &InitialPath, TRUE, &cchInitialPath))
		return NULL;	// Last exit without cleanup
	myCfg.cfg.cchReturnPathLength = max(cchInitialPath+1, 1024);
	myCfg.cfg.szReturnPathBuffer = (TCHAR *)malloc(myCfg.cfg.cchReturnPathLength * sizeof(TCHAR));
	if (myCfg.cfg.szReturnPathBuffer == NULL){
		PyErr_NoMemory();
		goto cleanup;
		}
	memset(myCfg.cfg.szReturnPathBuffer,  0, myCfg.cfg.cchReturnPathLength * sizeof(TCHAR));
	if (InitialPath){
		_tcsncpy(myCfg.cfg.szReturnPathBuffer, InitialPath, cchInitialPath);
		myCfg.cfg.bInitializePath = TRUE;
		}
	if (!PyWinObject_AsTCHAR(obDialogBoxCaption, &myCfg.cfg.szDialogBoxCaption, TRUE))
		goto cleanup;
	if (!PyWinObject_AsTCHAR(obDataSource, &myCfg.cfg.szDataSource, TRUE))
		goto cleanup;
	
	Py_BEGIN_ALLOW_THREADS
	pdhStatus = (*pPdhBrowseCounters) (&myCfg.cfg);
	Py_END_ALLOW_THREADS

	if (pdhStatus != 0 && pdhStatus != PDH_DIALOG_CANCELLED)
		PyWin_SetAPIError("PdhBrowseCounters", pdhStatus);
	else
		ret = myCfg.bReturnMultiple ? PyWinObject_FromMultipleString(myCfg.cfg.szReturnPathBuffer)
									 :PyWinObject_FromTCHAR(myCfg.cfg.szReturnPathBuffer);

cleanup:
	// Note - myCfg does not own any references
	if (myCfg.cfg.szReturnPathBuffer)
		free(myCfg.cfg.szReturnPathBuffer);
	PyWinObject_FreeTCHAR(myCfg.cfg.szDialogBoxCaption);
	PyWinObject_FreeTCHAR(myCfg.cfg.szDataSource);
	PyWinObject_FreeTCHAR(InitialPath);
	return ret;
}

// @pymethod string|win32pdh|ConnectMachine|connects to the specified machine, and creates and initializes a machine entry in the PDH DLL.
static PyObject *PyConnectMachine(PyObject *self, PyObject *args)
{
	PyObject *obPath;
	PDH_STATUS pdhStatus;
	if (!PyArg_ParseTuple(args, "O:ConnectMachine", 
	          &obPath))   // @pyparm string|machineName||The machine name.
		return NULL;
	TCHAR *path;
	if (!PyWinObject_AsTCHAR(obPath, &path, TRUE))
		return NULL;

	CHECK_PDH_PTR(pPdhConnectMachine);
	Py_BEGIN_ALLOW_THREADS
	pdhStatus = (*pPdhConnectMachine) (path);
	Py_END_ALLOW_THREADS
	PyWinObject_FreeTCHAR(path);

	PyObject *rc;
	if (pdhStatus != 0) {
		rc = PyWin_SetAPIError("ConnectMachine", pdhStatus);
	} else {
		rc = Py_None;
		Py_INCREF(rc);
	}
	return rc;
}

// @pymethod int|win32pdh|LookupPerfIndexByName|Returns the counter index corresponding to the specified counter name.
static PyObject *PyLookupPerfIndexByName(PyObject *self, PyObject *args)
{
	PyObject *obiname, *obmname;
	PDH_STATUS pdhStatus;
	if (!PyArg_ParseTuple(args, "OO:LookupPerfIndexByName", 
	          &obmname,// @pyparm string|machineName||The name of the machine where the specified counter is located. The machine name can be specified by the DNS name or the IP address. 
	          &obiname))// @pyparm string|instanceName||The full name of the counter.
		return NULL;
	TmpTCHAR mname, iname;
	if (!PyWinObject_AsTCHAR(obmname, &mname, TRUE))
		return NULL;
	if (!PyWinObject_AsTCHAR(obiname, &iname, FALSE))
		return NULL;

	CHECK_PDH_PTR(pPdhLookupPerfIndexByName);
	DWORD dwIndex;
	Py_BEGIN_ALLOW_THREADS
	pdhStatus = (*pPdhLookupPerfIndexByName) (mname, iname, &dwIndex);
	Py_END_ALLOW_THREADS

	if (pdhStatus != 0)
		return PyWin_SetAPIError("LookupPerfIndexByName", pdhStatus);
	return PyInt_FromLong(dwIndex);
}

// @pymethod string|win32pdh|LookupPerfNameByIndex|Returns the performance object name corresponding to the specified index.
static PyObject *PyLookupPerfNameByIndex(PyObject *self, PyObject *args)
{
	CHECK_PDH_PTR(pPdhLookupPerfNameByIndex);
	PyObject *obmname;
	DWORD index;
	if (!PyArg_ParseTuple(args, "Ok:LookupPerfIndexByName", 
	          &obmname,// @pyparm string|machineName||The name of the machine where the specified counter is located. The machine name can be specified by the DNS name or the IP address. 
	          &index))// @pyparm int|index||The index of the performance object.
		return NULL;
	TmpTCHAR mname;
	if (!PyWinObject_AsTCHAR(obmname, &mname, TRUE))
		return NULL;

	/* Determining required buffer size is again painful.
		MSDN says buf_size should receive required size when buffer is too small, but it does not on XP.
		Also on XP it returns PDH_INSUFFICIENT_BUFFER instead of PDH_MORE_DATA.
		If you pass it a NULL buffer on XP, it returns PDH_INVALID_ARGUMENT, so need to always
		preallocate some buffer space.  To account for different platforms, check for either
		PDH_MORE_DATA or PDH_INSUFFICIENT_BUFFER, and just keep doubling the allocation if
		required buffer size is not returned. 
	*/
	TCHAR *buffer = NULL;
	DWORD buf_size = 128, init_buf_size;
	PDH_STATUS pdhStatus;
	PyObject *ret;
	while (true){
		if (buffer)
			free(buffer);
		init_buf_size = buf_size;
		buffer=(TCHAR *)malloc(buf_size * sizeof(TCHAR));
		if (buffer == NULL){
			PyErr_NoMemory();
			return NULL;
			}
		Py_BEGIN_ALLOW_THREADS
		pdhStatus = (*pPdhLookupPerfNameByIndex) (mname, index, buffer, &buf_size);
		Py_END_ALLOW_THREADS
		if (pdhStatus == ERROR_SUCCESS){
			ret = PyWinObject_FromTCHAR(buffer);
			break;
			}
		if (pdhStatus != PDH_MORE_DATA && pdhStatus != PDH_INSUFFICIENT_BUFFER){
			ret = PyWin_SetAPIError("LookupPerfNameByIndex", pdhStatus);
			break;
			}
		if (buf_size <= init_buf_size)
			buf_size *=2;
		}
	free(buffer);
	return ret;
}

/* List of functions exported by this module */
// @module win32pdh|A module, encapsulating the Windows Performance Data Helpers API
static struct PyMethodDef win32pdh_functions[] = {
	{"AddCounter",               PyAddCounter,           1}, // @pymeth AddCounter|Adds a new counter
	{"AddEnglishCounter",        PyAddEnglishCounter,    1}, // @pymeth AddEnglishCounter|Adds a counter to a query by its English name
	{"RemoveCounter",            PyRemoveCounter,        1}, // @pymeth RemoveCounter|Removes an open counter.
	{"EnumObjectItems",          PyEnumObjectItems,      1}, // @pymeth EnumObjectItems|Enumerates an object's items
 	{"EnumObjects",		         PyEnumObjects,          1}, // @pymeth EnumObjects|Enumerates objects
	{"OpenQuery",                PyOpenQuery,            1}, // @pymeth OpenQuery|Opens a new query
	{"CloseQuery",               PyCloseQuery,           1}, // @pymeth CloseQuery|Closes an open query.
	{"MakeCounterPath",          PyMakeCounterPath,      1}, // @pymeth MakeCounterPath|Makes a fully resolved counter path
	{"GetCounterInfo",           PyGetCounterInfo,       1}, // @pymeth GetCounterInfo|Retrieves information about a counter, such as data size, counter type, path, and user-supplied data values.
	{"GetFormattedCounterValue", PyGetFormattedCounterValue,      1}, // @pymeth GetFormattedCounterValue|Retrieves a formatted counter value
	{"CollectQueryData",         PyCollectQueryData,     1}, // @pymeth CollectQueryData|Collects the current raw data value for all counters in the specified query and updates the status code of each counter.
	{"ValidatePath",             PyValidatePath,     1}, // @pymeth ValidatePath|Validates that the specified counter is present on the machine specified in the counter path.
	{"ExpandCounterPath",        PyExpandCounterPath,     1}, // @pymeth ExpandCounterPath|Examines the specified machine (or local machine if none is specified) for counters and instances of counters that match the wild card strings in the counter path.
	{"ParseCounterPath",         PyParseCounterPath,      1}, // @pymeth ParseCounterPath|Parses the elements of the counter path.
	{"ParseInstanceName",        PyParseInstanceName,     1}, // @pymeth ParseInstanceName|Parses the elements of the instance name
	{"SetCounterScaleFactor",	 PySetCounterScaleFactor, 1}, // @pymeth SetCounterScaleFactor|Sets the scale factor that is applied to the calculated value of the specified counter when you request the formatted counter value.
	{"BrowseCounters",           (PyCFunction)PyBrowseCounters, METH_VARARGS|METH_KEYWORDS}, // @pymeth BrowseCounters|Displays the counter browsing dialog box so that the user can select the counters to be returned to the caller. 
	{"ConnectMachine",           PyConnectMachine,        1}, // @pymeth ConnectMachine|connects to the specified machine, and creates and initializes a machine entry in the PDH DLL.
	{"LookupPerfIndexByName",    PyLookupPerfIndexByName, 1}, // @pymeth LookupPerfIndexByName|Returns the counter index corresponding to the specified counter name.
	{"LookupPerfNameByIndex",    PyLookupPerfNameByIndex, 1}, // @pymeth LookupPerfNameByIndex|Returns the performance object name corresponding to the specified index.
	{NULL}
};


#define ADD_CONSTANT(tok) PyModule_AddIntConstant(module, #tok, tok)

PYWIN_MODULE_INIT_FUNC(win32pdh)
{
	PYWIN_MODULE_INIT_PREPARE(win32pdh, win32pdh_functions,
	                          "A module, encapsulating the Windows Performance Data Helpers API");
	// InitializeCriticalSection(&critSec);

	PyDict_SetItemString(dict, "error", PyWinExc_ApiError);
	win32pdh_counter_error = PyErr_NewException("win32pdh.counter_status_error", NULL, NULL);
	PyDict_SetItemString(dict, "counter_status_error", win32pdh_counter_error);
	LoadPointers(); // Setting an error in this function will cause Python to spew.
  
	ADD_CONSTANT(PDH_VERSION);
 
	ADD_CONSTANT(PDH_FMT_RAW);
	ADD_CONSTANT(PDH_FMT_ANSI);
	ADD_CONSTANT(PDH_FMT_UNICODE);
	ADD_CONSTANT(PDH_FMT_LONG);
	ADD_CONSTANT(PDH_FMT_DOUBLE);
	ADD_CONSTANT(PDH_FMT_LARGE);
	ADD_CONSTANT(PDH_FMT_NOSCALE);
	ADD_CONSTANT(PDH_FMT_1000);
	ADD_CONSTANT(PDH_FMT_NODATA);
	
	ADD_CONSTANT(PDH_MAX_SCALE);
	ADD_CONSTANT(PDH_MIN_SCALE);
	
	ADD_CONSTANT(PERF_DETAIL_NOVICE);
	ADD_CONSTANT(PERF_DETAIL_ADVANCED);
	ADD_CONSTANT(PERF_DETAIL_EXPERT);
	ADD_CONSTANT(PERF_DETAIL_WIZARD);
	ADD_CONSTANT(PDH_PATH_WBEM_RESULT);
	ADD_CONSTANT(PDH_PATH_WBEM_INPUT);
//	ADD_CONSTANT();
	PYWIN_MODULE_INIT_RETURN_SUCCESS;
}
