/***********************************************************

win32pdh - Performance Data Helpers API interface 

Note that this source file contains embedded documentation.
This documentation consists of marked up text inside the
C comments, and is prefixed with an '@' symbol.  The source
files are processed by a tool called "autoduck" which
generates Windows .hlp files.
@doc

******************************************************************/

#include "windows.h"
#include "pdh.h"
#include "pdhmsg.h"

// It appears PDH it not thread safe!
// Use a critical section to protect calls into it
CRITICAL_SECTION critSec;

#define PyW32_BEGIN_ALLOW_THREADS \
	PyThreadState *_save = PyEval_SaveThread(); \
	EnterCriticalSection(&critSec);

#define PyW32_END_ALLOW_THREADS \
	PyEval_RestoreThread(_save); \
	LeaveCriticalSection(&critSec);

#define PyW32_BLOCK_THREADS \
	Py_BLOCK_THREADS \
	LeaveCriticalSection(&critSec);



// Function pointer typedefs
typedef PDH_STATUS (WINAPI * FuncPdhEnumObjects)(
    LPCTSTR szReserved,	// reserved
    LPCTSTR szMachineName,	// machine name
    LPTSTR mszObjectList,	// buffer for objects
    LPDWORD pcchBufferLength,	// size of buffer
    DWORD dwDetailLevel,	// detail level
    BOOL bRefresh	// refresh flag for connected machines
   );

typedef PDH_STATUS (WINAPI * FuncPdhEnumObjectItems)(
    LPCTSTR szReserved,	// reserved
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
    LPVOID pReserved,	// reserved
    DWORD dwUserData,	// a value associated with this query
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
    DWORD dwUserData,	// user-defined value
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
   LPSTR mszExpandedPathList,	// names that match
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
  LPCSTR  szInstanceString,
  LPSTR   szInstanceName,
  LPDWORD pcchInstanceNameLength,
  LPSTR   szParentName,
  LPDWORD pcchParentNameLength,
  LPDWORD lpIndex
);

typedef PDH_STATUS (WINAPI *FuncPdhBrowseCounters) (
  PPDH_BROWSE_DLG_CONFIG_A pBrowseDlgData
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

#define CHECK_PDH_PTR(ptr) if((ptr)==NULL) { PyErr_SetString(PyExc_RuntimeError, "The pdh.dll entry point functions could not be loaded."); return NULL;}

// The function pointers
FuncPdhEnumObjects pPdhEnumObjects = NULL;
FuncPdhEnumObjectItems pPdhEnumObjectItems = NULL;
FuncPdhOpenQuery pPdhOpenQuery = NULL;
FuncPdhCloseQuery pPdhCloseQuery = NULL;
FuncPdhRemoveCounter pPdhRemoveCounter = NULL;
FuncPdhAddCounter pPdhAddCounter = NULL;
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

#include "Python.h"
#include "malloc.h"
#include "PyWinTypes.h"

static PyObject *win32pdh_counter_error;

BOOL LoadPointers()
{
	HMODULE handle = LoadLibrary("pdh.dll");
	if (handle==NULL) {
//		PyErr_SetString(PyExc_RuntimeError, "The PDH DLL could not be located");
		return FALSE;
	}
	pPdhEnumObjects = (FuncPdhEnumObjects)GetProcAddress(handle, "PdhEnumObjectsA");
	pPdhEnumObjectItems = (FuncPdhEnumObjectItems)GetProcAddress(handle, "PdhEnumObjectItemsA");
	pPdhCloseQuery = (FuncPdhCloseQuery)GetProcAddress(handle, "PdhCloseQuery");
	pPdhRemoveCounter = (FuncPdhRemoveCounter)GetProcAddress(handle, "PdhRemoveCounter");
	pPdhOpenQuery = (FuncPdhOpenQuery)GetProcAddress(handle, "PdhOpenQuery");
	pPdhAddCounter = (FuncPdhAddCounter)GetProcAddress(handle, "PdhAddCounterA");
	pPdhMakeCounterPath = (FuncPdhMakeCounterPath)GetProcAddress(handle, "PdhMakeCounterPathA");
	pPdhGetCounterInfo = (FuncPdhGetCounterInfo)GetProcAddress(handle, "PdhGetCounterInfoA");
	pPdhGetFormattedCounterValue = (FuncPdhGetFormattedCounterValue)GetProcAddress(handle, "PdhGetFormattedCounterValue");
	pPdhCollectQueryData = (FuncPdhCollectQueryData)GetProcAddress(handle, "PdhCollectQueryData");
	pPdhValidatePath	= (FuncPdhValidatePath)GetProcAddress(handle, "PdhValidatePathA");
	pPdhExpandCounterPath	= (FuncPdhExpandCounterPath)GetProcAddress(handle, "PdhExpandCounterPathA");
	pPdhParseCounterPath = (FuncPdhParseCounterPath)GetProcAddress(handle, "PdhParseCounterPathA");
	pPdhSetCounterScaleFactor = (FuncPdhSetCounterScaleFactor)GetProcAddress(handle, "PdhSetCounterScaleFactor");
	pPdhParseInstanceName = (FuncPdhParseInstanceName)GetProcAddress(handle, "PdhParseInstanceNameA");
	pPdhBrowseCounters = (FuncPdhBrowseCounters)GetProcAddress(handle, "PdhBrowseCountersA");
	pPdhConnectMachine = (FuncPdhConnectMachine)GetProcAddress(handle, "PdhConnectMachineA");
	pPdhLookupPerfNameByIndex = (FuncPdhLookupPerfNameByIndex)GetProcAddress(handle, "PdhLookupPerfNameByIndexA");
	pPdhLookupPerfIndexByName = (FuncPdhLookupPerfIndexByName)GetProcAddress(handle, "PdhLookupPerfIndexByName");
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
	char *reserved;
	PyObject *obMachine, *obObject;
	if (!PyArg_ParseTuple(args, "zOOi|i:EnumObjectItems", 
	          &reserved, // @pyparm string|reserved||Should be None
	          &obMachine, // @pyparm string|machine||The machine to use, or None
	          &obObject, // @pyparm string|object||The type of object
	          &detailLevel, // @pyparm int|detailLevel||The level of data required.
	          &flags)) // @pyparm int|flags|0|Flags - must be zero
		return NULL;

    LPTSTR      szCounterListBuffer     = NULL;
    DWORD       dwCounterListSize       = 0;
    LPTSTR      szInstanceListBuffer    = NULL;
    DWORD       dwInstanceListSize      = 0;
    LPTSTR      szTemp          = NULL;

	CHECK_PDH_PTR(pPdhEnumObjectItems);

	TCHAR *strMachine, *strObject;
	if (!PyWinObject_AsTCHAR(obMachine, &strMachine, TRUE))
		return NULL;
	if (!PyWinObject_AsTCHAR(obObject, &strObject, FALSE)) {
		PyWinObject_FreeTCHAR(strMachine);
		return NULL;
	}


	PDH_STATUS pdhStatus;

	Py_BEGIN_ALLOW_THREADS

    pdhStatus = (*pPdhEnumObjectItems) (
        reserved,                   // reserved
        strMachine,                   // local machine
        strObject,        // object to enumerate
        szCounterListBuffer,    // pass in NULL buffers
        &dwCounterListSize,     // an 0 length to get
        szInstanceListBuffer,   // required size 
        &dwInstanceListSize,    // of the buffers in chars
        detailLevel,     // counter detail level
        flags); 
	Py_END_ALLOW_THREADS

	// it appears NT/2k will return 0, while XP will return
	// PDH_MORE_DATA
	if (pdhStatus != ERROR_SUCCESS && pdhStatus != PDH_MORE_DATA)  {
		PyWinObject_FreeTCHAR(strMachine);
		PyWinObject_FreeTCHAR(strObject);
		return PyWin_SetAPIError("EnumObjectItems for buffer size", pdhStatus);
	}

    // Allocate the buffers and try the call again.
	if (dwCounterListSize) {
		szCounterListBuffer = (LPTSTR)malloc (dwCounterListSize * sizeof (TCHAR));
		if (szCounterListBuffer==NULL) {
			PyErr_SetString(PyExc_MemoryError, "Allocating counter buffer");
			PyWinObject_FreeTCHAR(strMachine);
			PyWinObject_FreeTCHAR(strObject);
			return NULL;
		}
	} else
		szCounterListBuffer=NULL;

	if (dwInstanceListSize) {
		szInstanceListBuffer = (LPTSTR)malloc (dwInstanceListSize * sizeof (TCHAR));
		if (szInstanceListBuffer==NULL) {
			free(szCounterListBuffer);
			PyWinObject_FreeTCHAR(strMachine);
			PyWinObject_FreeTCHAR(strObject);
			PyErr_SetString(PyExc_MemoryError, "Allocating instance buffer");
			return NULL;
		}
	} else
		szInstanceListBuffer = NULL;

	Py_BEGIN_ALLOW_THREADS
	pdhStatus = (*pPdhEnumObjectItems) (
	        reserved,                   // reserved
	        strMachine,                   // local machine
	        strObject,        // object to enumerate
	        szCounterListBuffer,    // pass in NULL buffers
	        &dwCounterListSize,     // an 0 length to get
	        szInstanceListBuffer,   // required size 
	        &dwInstanceListSize,    // of the buffers in chars
	        detailLevel,     // counter detail level
	        flags); 
	Py_END_ALLOW_THREADS
	PyWinObject_FreeTCHAR(strMachine);
	PyWinObject_FreeTCHAR(strObject);

    if (pdhStatus != ERROR_SUCCESS) {
		free(szCounterListBuffer);
		free(szInstanceListBuffer);
		return PyWin_SetAPIError("EnumObjectItems for data", pdhStatus);
    }

	PyObject *retCounter = PyList_New(0);
	if (szCounterListBuffer)
		for (szTemp = szCounterListBuffer;
			*szTemp != 0;
			szTemp += lstrlen(szTemp) + 1) {
				PyObject *obTemp = PyString_FromString(szTemp);
				PyList_Append(retCounter, obTemp);
				Py_XDECREF(obTemp);
		}
	PyObject *retInstance = PyList_New(0);
	if (szInstanceListBuffer)
		for (szTemp = szInstanceListBuffer;
			*szTemp != 0;
			szTemp += lstrlen(szTemp) + 1) {
				PyObject *obTemp = PyString_FromString(szTemp);
				PyList_Append(retInstance, obTemp);
				Py_XDECREF(obTemp);
		}
	PyObject *rc = Py_BuildValue("OO", retCounter, retInstance);
	Py_XDECREF(retCounter);
	Py_XDECREF(retInstance);
	free(szInstanceListBuffer);
	free(szCounterListBuffer);
	return rc;
}

// @pymethod list|win32pdh|EnumObjects|Enumerates objects
static PyObject *PyEnumObjects(PyObject *self, PyObject *args)
{
	DWORD detailLevel, refresh=1;
	char *reserved;
	PyObject *obMachine;
	if (!PyArg_ParseTuple(args, "zOi|i:EnumObjects", 
	          &reserved, // @pyparm string|reserved||Should be None
	          &obMachine, // @pyparm string|machine||The machine to use, or None
	          &detailLevel, // @pyparm int|detailLevel||The level of data required.
	          &refresh)) // @pyparm int|refresh|1|Should the list be refreshed.
		return NULL;

    LPTSTR      szObjectListBuffer     = NULL;
    DWORD       dwObjectListSize       = 0;
    LPTSTR      szTemp          = NULL;

	CHECK_PDH_PTR(pPdhEnumObjects);

	TCHAR *strMachine;
	if (!PyWinObject_AsTCHAR(obMachine, &strMachine, TRUE))
		return NULL;


	PDH_STATUS pdhStatus;

	Py_BEGIN_ALLOW_THREADS

    pdhStatus = (*pPdhEnumObjects) (
        reserved,                   // reserved
        strMachine,                   // local machine
        szObjectListBuffer,    // pass in NULL buffers
        &dwObjectListSize,     // an 0 length to get
        detailLevel,     // counter detail level
        refresh); 
	Py_END_ALLOW_THREADS

	// it appears NT/2k will return 0, while XP will return
	// PDH_MORE_DATA
	if (pdhStatus != ERROR_SUCCESS && pdhStatus != PDH_MORE_DATA)  {
		PyWinObject_FreeTCHAR(strMachine);
		return PyWin_SetAPIError("EnumObjects for buffer size", pdhStatus);
	}

    // Allocate the buffers and try the call again.
	if (dwObjectListSize) {
		szObjectListBuffer = (LPTSTR)malloc (dwObjectListSize * sizeof (TCHAR));
		if (szObjectListBuffer==NULL) {
			PyErr_SetString(PyExc_MemoryError, "Allocating object buffer");
			PyWinObject_FreeTCHAR(strMachine);
			return NULL;
		}
	} else
		szObjectListBuffer=NULL;


	Py_BEGIN_ALLOW_THREADS
	pdhStatus = (*pPdhEnumObjects) (
	        reserved,                   // reserved
	        strMachine,                   // local machine
	        szObjectListBuffer,    // pass in NULL buffers
	        &dwObjectListSize,     // an 0 length to get
	        detailLevel,     // counter detail level
	        0); 
	Py_END_ALLOW_THREADS
	PyWinObject_FreeTCHAR(strMachine);

    if (pdhStatus != ERROR_SUCCESS) {
		free(szObjectListBuffer);
		return PyWin_SetAPIError("EnumObjects for data", pdhStatus);
    }

	PyObject *retObject = PyList_New(0);
	if (szObjectListBuffer)
		for (szTemp = szObjectListBuffer;
			*szTemp != 0;
			szTemp += lstrlen(szTemp) + 1) {
				PyObject *obTemp = PyString_FromString(szTemp);
				PyList_Append(retObject, obTemp);
				Py_XDECREF(obTemp);
		}
	free(szObjectListBuffer);
	Py_INCREF(retObject);
	return retObject;
}

// @pymethod int|win32pdh|AddCounter|Adds a new counter
static PyObject *PyAddCounter(PyObject *self, PyObject *args)
{
	HQUERY hQuery;
	PyObject *obPath;
	DWORD userData = 0;
	if (!PyArg_ParseTuple(args, "iO|i:AddCounter", 
	          &hQuery, // @pyparm int|hQuery||Handle to an open query.
	          &obPath, // @pyparm string|path||Full path to the performance data
	          &userData)) // @pyparm int|userData|0|User data associated with the counter.
		return NULL;

	TCHAR *szPath;
	if (!PyWinObject_AsTCHAR(obPath, &szPath, FALSE))
		return NULL;
	HCOUNTER hCounter;
	CHECK_PDH_PTR(pPdhAddCounter);
	PyW32_BEGIN_ALLOW_THREADS
    PDH_STATUS pdhStatus = (*pPdhAddCounter) (
        hQuery,
        szPath,
	    userData,
	    &hCounter);

	PyW32_END_ALLOW_THREADS;
	PyWinObject_FreeTCHAR(szPath);
    if (pdhStatus != ERROR_SUCCESS) 
		return PyWin_SetAPIError("AddCounter", pdhStatus);
	// @comm See also <om win32pdh.RemoveCounter>
	return PyInt_FromLong((long)hCounter);
}

// @pymethod |win32pdh|RemoveCounter|Removes a previously opened counter
static PyObject *PyRemoveCounter(PyObject *self, PyObject *args)
{
	HQUERY handle;
	if (!PyArg_ParseTuple(args, "i:RemoveCounter", 
	          &handle)) // @pyparm int|handle||Handle to an open counter.
		return NULL;
	// @comm See also <om win32pdh.AddCounter>
	CHECK_PDH_PTR(pPdhRemoveCounter);
	PyW32_BEGIN_ALLOW_THREADS
    PDH_STATUS pdhStatus = (*pPdhRemoveCounter) (handle);
	PyW32_END_ALLOW_THREADS

    if (pdhStatus != ERROR_SUCCESS) 
		return PyWin_SetAPIError("RemoveCounter", pdhStatus);
	Py_INCREF(Py_None);
	return Py_None;
}


// @pymethod int|win32pdh|OpenQuery|Opens a new query
static PyObject *PyOpenQuery(PyObject *self, PyObject *args)
{
	DWORD userData = 0;
	char *reserved = NULL;
	if (!PyArg_ParseTuple(args, "|zi:OpenQuery", 
	          &reserved, // @pyparm object|reserved|None|Must be None
	          &userData)) // @pyparm int|userData|0|User data associated with the query.
		return NULL;

	HQUERY hQuery;
	CHECK_PDH_PTR(pPdhOpenQuery);
	PDH_STATUS pdhStatus;
	Py_BEGIN_ALLOW_THREADS
    pdhStatus = (*pPdhOpenQuery) (
        (void *)reserved,
	    userData,
	    &hQuery);
	Py_END_ALLOW_THREADS

    if (pdhStatus != ERROR_SUCCESS) 
		return PyWin_SetAPIError("OpenQuery", pdhStatus);
	return PyInt_FromLong((long)hQuery);
	// @comm See also <om win32pdh.CloseQuery>
}

// @pymethod |win32pdh|CloseQuery|Closes a query
static PyObject *PyCloseQuery(PyObject *self, PyObject *args)
{
	HQUERY handle;
	if (!PyArg_ParseTuple(args, "i:CloseQuery", 
	          &handle)) // @pyparm int|handle||Handle to an open query.
		return NULL;
	// @comm See also <om win32pdh.OpenQuery>
	CHECK_PDH_PTR(pPdhCloseQuery);
	PyW32_BEGIN_ALLOW_THREADS
    PDH_STATUS pdhStatus = (*pPdhCloseQuery) (handle);
	PyW32_END_ALLOW_THREADS

    if (pdhStatus != ERROR_SUCCESS) 
		return PyWin_SetAPIError("CloseQuery", pdhStatus);
	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod |win32pdh|MakeCounterPath|Makes a fully resolved counter path
static PyObject *PyMakeCounterPath(PyObject *self, PyObject *args)
{
    PyObject *rc = NULL;
	char szResult[512];
	DWORD bufSize = sizeof(szResult);
	DWORD flags = 0;
	DWORD dwInstanceIndex;
	PyObject *obMachineName, *obObjectName, *obInstanceName, *obParentInstance, *obCounterName;
	// @pyparm (machineName, objectName, instanceName, parentInstance, instanceIndex, counterName)|elements||The elements to use to create the path.
	// @pyparm int|flags||reserved.
	if (!PyArg_ParseTuple(args, "(OOOOlO)|l:MakeCounterPath", 
			&obMachineName,
			&obObjectName,
			&obInstanceName,
			&obParentInstance,
			&dwInstanceIndex,
			&obCounterName,
			&flags))
		return NULL;
	// First call to get buffer size

	PDH_COUNTER_PATH_ELEMENTS cpe;
	memset(&cpe, 0, sizeof(cpe));
	cpe.dwInstanceIndex = dwInstanceIndex;
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

	CHECK_PDH_PTR(pPdhMakeCounterPath);
	// OK - I give up in absolute disgust.  Will no longer make any
	// attempts to determine bufffer size.  Will hard-code.
/******
	DWORD bufSize = 0;
    PDH_STATUS pdhStatus = (*pPdhMakeCounterPath) (&cpe, NULL, &bufSize, flags);
    if (pdhStatus != ERROR_SUCCESS) 
		return PyWin_SetAPIError("MakeCounterPath for size", pdhStatus);

	// No fing idea why this is returning 1/2 the size needed!
	// bufSize is doc'd as returning number of BYTES, not characters.
	// Also docd as having term NULL - but occasionally Im getting wierd data...
	bufSize++;
	bufSize *= 2;
	char *szResult = (char *)malloc(bufSize);
	if (szResult==NULL) {
		PyErr_SetString(PyExc_MemoryError, "Allocating result buffer");
		return NULL;
	}
*****/
	PDH_STATUS pdhStatus;
	Py_BEGIN_ALLOW_THREADS
    pdhStatus = (*pPdhMakeCounterPath) (&cpe, szResult, &bufSize, flags);
	Py_END_ALLOW_THREADS
    if (pdhStatus != ERROR_SUCCESS)
    	rc = PyWin_SetAPIError("MakeCounterPath for data", pdhStatus);
    else
    	rc = PyString_FromString(szResult);
done:

//    free(szResult);
	PyWinObject_FreeTCHAR(cpe.szMachineName);
	PyWinObject_FreeTCHAR(cpe.szObjectName);
	PyWinObject_FreeTCHAR(cpe.szInstanceName);
	PyWinObject_FreeTCHAR(cpe.szParentInstance);
	PyWinObject_FreeTCHAR(cpe.szCounterName);
    return rc;
}

// @pymethod |win32pdh|GetCounterInfo|Retrieves information about a counter, such as data size, counter type, path, and user-supplied data values.
static PyObject *PyGetCounterInfo(PyObject *self, PyObject *args)
{
	HCOUNTER handle;
	BOOL bExplainText = TRUE;	
	if (!PyArg_ParseTuple(args, "i|i:GetCounterInfo", 
			&handle, // @pyparm int|handle||The handle of the item to query
			&bExplainText)) // @pyparm int|bRetrieveExplainText||Should explain text be retrieved?
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
		   	rc = Py_BuildValue("iiiiiiz(zzzziz)z",     
	                      pInfo->dwType,
	                      pInfo->CVersion,
	                      pInfo->lScale,
	                      pInfo->lDefaultScale,
	                      pInfo->dwUserData,
	                      pInfo->dwQueryUserData,
	                      pInfo->szFullPath,
	                      
	                     pInfo->szMachineName,
		                 pInfo->szObjectName,
		                 pInfo->szInstanceName,
		                 pInfo->szParentInstance,
		                 pInfo->dwInstanceIndex,
		                 pInfo->szCounterName,
            
		                 pInfo->szExplainText);
    }
    free(pInfo);
    return rc;
}


// @pymethod (int,object)|win32pdh|GetFormattedCounterValue|Retrieves a formatted counter value
static PyObject *PyGetFormattedCounterValue(PyObject *self, PyObject *args)
{
	HCOUNTER handle;
	DWORD format;
	if (!PyArg_ParseTuple(args, "ii:GetFormattedCounterValue", 
			&handle, // @pyparm int|handle||Handle to the counter
			&format)) // @pyparm int|format||Format of result.  Can be PDH_FMT_DOUBLE, PDH_FMT_LARGE, PDH_FMT_LONG and or'd with PDH_FMT_NOSCALE, PDH_FMT_1000

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
	// XXX - need long int support
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
	if (!PyArg_ParseTuple(args, "i:CollectQueryData", 
	          &hQuery)) // @pyparm int|hQuery||Handle to an open query.
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
	PyObject *obPath;
	if (!PyArg_ParseTuple(args, "O:ExpandCounterPath", 
	          &obPath)) // @pyparm string|wildCardPath||The counter path to expand.
		return NULL;

	TCHAR *path;
	CHECK_PDH_PTR(pPdhExpandCounterPath);

	if (!PyWinObject_AsTCHAR(obPath, &path, FALSE))
		return NULL;

	DWORD dwSize = 4096;
/*    PDH_STATUS pdhStatus = (*pPdhExpandCounterPath) (path, NULL, &dwSize);
    if (pdhStatus != ERROR_SUCCESS) {
    	printf("dwSize is %d", dwSize);
		return PyWin_SetAPIError("ExpandCounterPath for size", pdhStatus);
    }
*/
	// @comm The result is limited to 4096 bytes!
	char *buf = (char *)malloc(dwSize * sizeof(TCHAR));
    PDH_STATUS pdhStatus = (*pPdhExpandCounterPath) (path, buf, &dwSize);
    PyObject *rc;
    if (pdhStatus != ERROR_SUCCESS) 
		rc = PyWin_SetAPIError("ExpandCounterPath for data", pdhStatus);
	else {
		rc = PyList_New(0);
		for (char *szTemp = buf;*szTemp != 0;szTemp += lstrlen(szTemp) + 1) {
			PyObject *obTemp = PyString_FromString(szTemp);
			PyList_Append(rc, obTemp);
			Py_XDECREF(obTemp);
		}
	}
	PyWinObject_FreeTCHAR(path);
	free(buf);
	return rc;
	// @comm The counter path format is assumed to be:
	// <nl>\\machine\object(parent/instance#index)\countername
	// <nl>and the parent, instance, index, and countername elements
	// may contain either a valid name or a wild card character.
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
	TCHAR *path;
	if (!PyWinObject_AsTCHAR(obPath, &path, FALSE))
		return NULL;

	CHECK_PDH_PTR(pPdhParseCounterPath);
	DWORD size=0;
    PDH_STATUS pdhStatus = (*pPdhParseCounterPath) (path, NULL, &size, flags);

    if (size==0) {
		return PyWin_SetAPIError("ParseCounterPath for size", pdhStatus);
	}
	void *pBuf = malloc(size);
	if (pBuf==NULL) {
		PyErr_SetString(PyExc_MemoryError, "allocating buffer for PDH_COUNTER_PATH_ELEMENTS(+strings)");
		PyWinObject_FreeTCHAR(path);
		return NULL;
	}

	PDH_COUNTER_PATH_ELEMENTS *pCPE = (PDH_COUNTER_PATH_ELEMENTS *)pBuf;
	PyW32_BEGIN_ALLOW_THREADS
    pdhStatus = (*pPdhParseCounterPath) (path, pCPE, &size, flags);
	PyW32_END_ALLOW_THREADS
	PyWinObject_FreeTCHAR(path);

	PyObject *rc;
	if (pdhStatus != 0) {
		rc = PyWin_SetAPIError("ParseCounterPath", pdhStatus);
	} else {
		rc = Py_BuildValue("zzzziz", 
			pCPE->szMachineName,
			pCPE->szObjectName,
			pCPE->szInstanceName,
			pCPE->szParentInstance,
			pCPE->dwInstanceIndex,
			pCPE->szCounterName);
	}
	free(pBuf);
	return rc;
}

// @pymethod (name, parent, instance)|win32pdh|ParseInstanceName|Parses the elements of the instance name
static PyObject *PyParseInstanceName(PyObject *self, PyObject *args)
{
	PyObject *obiname;
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
	PyW32_BEGIN_ALLOW_THREADS
    PDH_STATUS pdhStatus = (*pPdhParseInstanceName) (iname, szName, &nameSize, szParent, &parentSize, &dwInstance);
	PyW32_END_ALLOW_THREADS
	PyWinObject_FreeTCHAR(iname);
	if (pdhStatus != 0)
		return PyWin_SetAPIError("ParseInstanceName", pdhStatus);
	return Py_BuildValue("ssi", szName, szParent, dwInstance);
}

// @pymethod |win32pdh|SetCounterScaleFactor|Sets the scale factor that is applied to the calculated value of the specified counter when you request the formatted counter value.
static PyObject *PySetCounterScaleFactor(PyObject *self, PyObject *args)
{
	HCOUNTER hCounter;
	LONG lFactor;
	if (!PyArg_ParseTuple(args, "il:SetCounterScaleFactor", 
	          &hCounter, // @pyparm int|hCounter||Handle to the counter.
			  &lFactor)) // @pyparm int|factor||power of ten used to multiply value.
		return NULL;

	CHECK_PDH_PTR(pPdhSetCounterScaleFactor);
	PyW32_BEGIN_ALLOW_THREADS
    PDH_STATUS pdhStatus = (*pPdhSetCounterScaleFactor) (hCounter, lFactor);
	PyW32_END_ALLOW_THREADS
	if (pdhStatus != 0)
		return PyWin_SetAPIError("SetCounterScaleFactor", pdhStatus);
	Py_INCREF(Py_None);
	return Py_None;
}

typedef struct {
	PyObject *func;
	PDH_BROWSE_DLG_CONFIG *pcfg;
} MY_DLG_CONFIG;


PDH_STATUS __stdcall CounterPathCallback(DWORD dwArg)
{
	MY_DLG_CONFIG *pMy = (MY_DLG_CONFIG *)dwArg;
	DWORD rc = PDH_INVALID_DATA;
	CEnterLeavePython _celp;
	PyObject *args = Py_BuildValue("(s)", pMy->pcfg->szReturnPathBuffer);
	PyObject *result = PyEval_CallObject(pMy->func, args);
	Py_XDECREF(args);
	if (result==NULL) { // What to do with exceptions?
		PyErr_Clear();
	} else if (PyInt_Check(result)) {
		rc = PyInt_AsLong(result);
	} // else do what?
	Py_XDECREF(result);
	return rc;
}

#define GET_IT(r, ob, i) { \
	PyObject *subItem = PySequence_GetItem(ob, i); \
	if (subItem==NULL) return NULL; \
	if (!PyInt_Check(subItem)) { \
		Py_DECREF(subItem); \
		PyErr_SetString(PyExc_TypeError, "Must be sequence of integers"); \
		return NULL; \
	} \
	r = PyInt_AsLong(subItem); \
	Py_DECREF(subItem); \
}

// @pymethod string|win32pdh|BrowseCounters|Displays the counter browsing dialog box so that the user can select the counters to be returned to the caller. 
static PyObject *PyBrowseCounters(PyObject *self, PyObject *args)
{
	PyObject *obFlags;
	PDH_BROWSE_DLG_CONFIG cfg;
	PDH_BROWSE_DLG_CONFIG *pcfg = &cfg;
	MY_DLG_CONFIG myCfg;
	// Note - this has set caption and others to default of zero.
	memset(&cfg, 0, sizeof(cfg));
	if (!PyArg_ParseTuple(args, "OiOi|z:BrowseCounters", 
	          &obFlags, // @pyparm tuple|flags||Tuple describing the bitmasks, or None.
			  &pcfg->hWndOwner, // @pyparm int|hWnd||parent for the dialog.
			  &myCfg.func, // @pyparm object|callback||A callable object to function as the callback.
			  &pcfg->dwDefaultDetailLevel, // @pyparm int|defDetailLevel||The default detail level to show on startup in the Detail Level combo box. If the Detail Level combo box is not shown, this is the detail level to use in filtering the displayed performance counters and objects. 
			  &pcfg->szDialogBoxCaption)) // @pyparm string|dlgCaption||The dialog coption, or None for default.
		return NULL;

	if (!PyCallable_Check(myCfg.func)) {
		PyErr_SetString(PyExc_TypeError, "The callback object must be a callable object");
		return NULL;
	}
	myCfg.pcfg = pcfg;
	if (obFlags!=Py_None) {
		if (!PySequence_Check(obFlags)) {
			PyErr_SetString(PyExc_TypeError, "Flags must be None, or sequence of integers");
			return NULL;
		}
		int seqLen = PySequence_Length(obFlags);
		if (seqLen>0) GET_IT(pcfg->bIncludeInstanceIndex, obFlags, 0);
		if (seqLen>1) GET_IT(pcfg->bSingleCounterPerAdd, obFlags, 1);
		if (seqLen>2) GET_IT(pcfg->bSingleCounterPerDialog, obFlags, 2);
		if (seqLen>3) GET_IT(pcfg->bLocalCountersOnly, obFlags, 3);
		if (seqLen>4) GET_IT(pcfg->bWildCardInstances, obFlags, 4);
		if (seqLen>5) GET_IT(pcfg->bHideDetailBox, obFlags, 5);
		if (seqLen>6) GET_IT(pcfg->bInitializePath, obFlags, 6);
		
	}

	pcfg->dwCallBackArg = (DWORD)&myCfg;
	pcfg->pCallBack = CounterPathCallback;

	pcfg->szReturnPathBuffer = (char *)malloc(1024);
	pcfg->cchReturnPathLength = 1024;


	CHECK_PDH_PTR(pPdhBrowseCounters);
	PyW32_BEGIN_ALLOW_THREADS
    PDH_STATUS pdhStatus = (*pPdhBrowseCounters) (pcfg);
	PyW32_END_ALLOW_THREADS

	PyObject *rc;
	if (pdhStatus != 0 && pdhStatus != PDH_DIALOG_CANCELLED) {
		PyWin_SetAPIError("pPdhBrowseCounters", pdhStatus);
		rc = NULL;
	}
	else
		rc = PyString_FromString(pcfg->szReturnPathBuffer);

	// Note - myCfg does not own any references
	free(pcfg->szReturnPathBuffer);

	return rc;
}

// @pymethod string|win32pdh|ConnectMachine|connects to the specified machine, and creates and initializes a machine entry in the PDH DLL.
static PyObject *PyConnectMachine(PyObject *self, PyObject *args)
{
	PyObject *obPath;
	if (!PyArg_ParseTuple(args, "O:ConnectMachine", 
	          &obPath))   // @pyparm string|machineName||The machine name.
		return NULL;
	TCHAR *path;
	if (!PyWinObject_AsTCHAR(obPath, &path, FALSE))
		return NULL;

	CHECK_PDH_PTR(pPdhConnectMachine);
	PyW32_BEGIN_ALLOW_THREADS
	PDH_STATUS pdhStatus = (*pPdhConnectMachine) (path);
	PyW32_END_ALLOW_THREADS
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
	if (!PyArg_ParseTuple(args, "OO:LookupPerfIndexByName", 
	          &obmname,// @pyparm string|machineName||The name of the machine where the specified counter is located. The machine name can be specified by the DNS name or the IP address. 
	          &obiname))// @pyparm string|instanceName||The full name of the counter.
		return NULL;
	TCHAR *mname;
	if (!PyWinObject_AsTCHAR(obmname, &mname, TRUE))
		return NULL;
	TCHAR *iname;
	if (!PyWinObject_AsTCHAR(obiname, &iname, FALSE)) {
		PyWinObject_FreeTCHAR(mname);
		return NULL;
	}

	CHECK_PDH_PTR(pPdhLookupPerfIndexByName);
	DWORD dwIndex;
	PyW32_BEGIN_ALLOW_THREADS
	PDH_STATUS pdhStatus = (*pPdhLookupPerfIndexByName) (mname, iname, &dwIndex);
	PyW32_END_ALLOW_THREADS
	PyWinObject_FreeTCHAR(mname);
	PyWinObject_FreeTCHAR(iname);
	if (pdhStatus != 0)
		return PyWin_SetAPIError("LookupPerfIndexByName", pdhStatus);
	return PyInt_FromLong(dwIndex);
}

// @pymethod string|win32pdh|LookupPerfNameByIndex|Returns the performance object name corresponding to the specified index.
static PyObject *PyLookupPerfNameByIndex(PyObject *self, PyObject *args)
{
	PyObject *obmname;
	int index;
	if (!PyArg_ParseTuple(args, "Oi:LookupPerfIndexByName", 
	          &obmname,// @pyparm string|machineName||The name of the machine where the specified counter is located. The machine name can be specified by the DNS name or the IP address. 
	          &index))// @pyparm int|index||The index of the performance object.
		return NULL;
	TCHAR *mname;
	if (!PyWinObject_AsTCHAR(obmname, &mname, TRUE))
		return NULL;

	TCHAR buffer[512];
	DWORD buf_size = sizeof(buffer)/sizeof(buffer[0]);
	CHECK_PDH_PTR(pPdhLookupPerfNameByIndex);
	PyW32_BEGIN_ALLOW_THREADS
	PDH_STATUS pdhStatus = (*pPdhLookupPerfNameByIndex) (mname, index, buffer, &buf_size);
	PyW32_END_ALLOW_THREADS
	PyWinObject_FreeTCHAR(mname);
	if (pdhStatus != 0)
		return PyWin_SetAPIError("LookupPerfNameByIndex", pdhStatus);
	return PyWinObject_FromTCHAR(buffer);
}

/* List of functions exported by this module */
// @module win32pdh|A module, encapsulating the Windows Performance Data Helpers API
static struct PyMethodDef win32pdh_functions[] = {
	{"AddCounter",               PyAddCounter,           1}, // @pymeth AddCounter|Adds a new counter
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
	{"BrowseCounters",           PyBrowseCounters,        1}, // @pymeth BrowseCounters|Displays the counter browsing dialog box so that the user can select the counters to be returned to the caller. 
	{"ConnectMachine",           PyConnectMachine,        1}, // @pymeth ConnectMachine|connects to the specified machine, and creates and initializes a machine entry in the PDH DLL.
	{"LookupPerfIndexByName",    PyLookupPerfIndexByName, 1}, // @pymeth LookupPerfIndexByName|Returns the counter index corresponding to the specified counter name.
	{"LookupPerfNameByIndex",    PyLookupPerfNameByIndex, 1}, // @pymeth LookupPerfNameByIndex|Returns the performance object name corresponding to the specified index.
	{NULL}
};

/* Initialize this module. */
int AddConstant(PyObject *dict, char *key, long value)
{
	PyObject *okey = PyString_FromString(key);
	PyObject *oval = PyLong_FromLong(value);
	if (!okey || !oval) {
		Py_XDECREF(okey);
		Py_XDECREF(oval);
		return 1;
	}
	int rc = PyDict_SetItem(dict,okey, oval);
	Py_XDECREF(okey);
	Py_XDECREF(oval);
	return rc;
}
#define ADD_CONSTANT(tok) AddConstant(dict,#tok, tok)


extern"C" __declspec(dllexport) void
initwin32pdh(void)
{
	InitializeCriticalSection(&critSec);
	PyObject *dict, *module;
	module = Py_InitModule("win32pdh", win32pdh_functions);
	if (!module) /* Eeek - some serious error! */
		return;
	dict = PyModule_GetDict(module);
	if (!dict) return;
	PyWinGlobals_Ensure();
	Py_INCREF(PyWinExc_ApiError);
	PyDict_SetItemString(dict, "error", PyWinExc_ApiError);
	win32pdh_counter_error = PyString_FromString("win32pdh counter status error");
	PyDict_SetItemString(dict, "counter status error", win32pdh_counter_error);
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
//	ADD_CONSTANT();
}
