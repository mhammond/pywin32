#include <windows.h>
#include <tchar.h>
#include "Python.h"
#include "PyWinTypes.h"

extern PyObject *SortOrderSpecToPy(SORTORDERSPEC *, DWORD);
extern BOOL PyToSortOrderSpec(PyObject *, SORTORDERSPEC **);
extern PyObject *PropValToPy(PEGPROPVAL *);
extern PyObject *PropValsToPy(PEGPROPVAL *, DWORD);
extern BOOL PyToPropVal(PyObject *, PEGPROPVAL *);
extern BOOL PyToPropVals(PyObject *, PEGPROPVAL **);
extern BOOL FreePropVals(PEGPROPVAL *);


static PyObject *
PyCreateDatabase(PyObject *self, PyObject *args)
{	SORTORDERSPEC *p=NULL;
	PyObject *obSort=NULL;
	TCHAR *lpName=NULL;
	PyObject *obName;
	DWORD dwType=0;
	WORD  nSort=0;
	PEGOID dbOid=NULL;

	if (!PyArg_ParseTuple(args, "OiO:CreateDatabase", &obName, &dwType, &obSort))
		return NULL;
	if (!PyWinObject_AsTCHAR(obName, &lpName, FALSE))
		return NULL;
	if (_tcslen(lpName) > PEGDB_MAXDBASENAMELEN) {
		PyWinObject_FreeTCHAR(lpName);
		PyErr_SetString(PyExc_ValueError, "Db name too long");
		return NULL;
	}
	if (!PyToSortOrderSpec(obSort, &p)) {
		PyWinObject_FreeTCHAR(lpName);
		LocalFree(p);
		return NULL;
	}
	nSort=PyObject_Length(obSort);
	dbOid=PegCreateDatabase(lpName, dwType, nSort, p);
	PyWinObject_FreeTCHAR(lpName);
	LocalFree(p);
	if (dbOid==NULL) {
		return PyWin_SetAPIError("PegCreateDatabase");
	}
	return Py_BuildValue("i", dbOid);
}

static PyObject *
PyOpenDatabase(PyObject *self, PyObject *args)
{	TCHAR *lpName=NULL;
	HANDLE hDb=NULL;
	DWORD dwFlags=0;
	PEGOID dbOid=NULL;
	PEGPROPID cpSort=0;
	PyObject *obName;

	if (!PyArg_ParseTuple(args, "iOii:OpenDatabase", &dbOid, &obName, &cpSort, &dwFlags))
		return NULL;
	if (!PyWinObject_AsTCHAR(obName, &lpName, FALSE))
		return NULL;
	hDb=PegOpenDatabase(&dbOid, lpName, cpSort, dwFlags, NULL);
	PyWinObject_FreeTCHAR(lpName);
	if (hDb==INVALID_HANDLE_VALUE) {
		return PyWin_SetAPIError("PegOpenDatabase");
	}
	return Py_BuildValue("(ii)", hDb, dbOid);
}

static PyObject *
PyFindDatabases(PyObject *self, PyObject *args)
{	PyObject *obList=NULL;
	PyObject *obSort=NULL;
	PyObject *obTime=NULL;
	PyObject *obInfo=NULL;
	PyObject *obName=NULL;
	PEGOIDINFO oidInfo;
	PEGOID dbOid=NULL;
	HANDLE hEnum=NULL;
	DWORD dwType=0;
	DWORD dwFlags=0;

	if (!PyArg_ParseTuple(args, "|i:FindDatabases", &dwType))
		return NULL;
	hEnum=PegFindFirstDatabase(dwType);
	if (hEnum==INVALID_HANDLE_VALUE) {
		return PyWin_SetAPIError("PegFindFirstDatabase");
	}
	memset(&oidInfo, 0, sizeof(PEGOIDINFO));
	obList=PyList_New(0);
	while (1) {
		dbOid=PegFindNextDatabase(hEnum);
		if ((dbOid==0) || (dbOid==ERROR_NO_MORE_ITEMS)) {
			break;
		}
		if (!PegOidGetInfo(dbOid, &oidInfo)) {
			CloseHandle(hEnum);
			Py_DECREF(obList);
			return PyWin_SetAPIError("PegOidGetInfo");
		}
		obTime=PyWinObject_FromFILETIME(oidInfo.infDatabase.ftLastModified);
		obSort=SortOrderSpecToPy(&oidInfo.infDatabase.rgSortSpecs[0], (DWORD)oidInfo.infDatabase.wNumSortOrder);
		obName=PyWinObject_FromTCHAR(oidInfo.infDatabase.szDbaseName);
		obInfo=Py_BuildValue("(iiOiiiiOO)",
							dbOid,
							oidInfo.infDatabase.dwFlags,
							obName,
							oidInfo.infDatabase.dwDbaseType,
							(DWORD)oidInfo.infDatabase.wNumRecords,
							(DWORD)oidInfo.infDatabase.wNumSortOrder,
							oidInfo.infDatabase.dwSize,
							obTime,
							obSort
							);
		PyList_Append(obList, obInfo);
		Py_XDECREF(obName);
		Py_XDECREF(obTime);
		Py_XDECREF(obSort);
		Py_XDECREF(obInfo);
	}
	CloseHandle(hEnum);
	return obList;
}

static PyObject *
PyDeleteDatabase(PyObject *self, PyObject *args)
{	PEGOID dbOid=NULL;

	if (!PyArg_ParseTuple(args, "i:DeleteDatabase", &dbOid))
		return NULL;
	if (!PegDeleteDatabase(dbOid)) {
		return PyWin_SetAPIError("PegDeleteDatabase");
	}
	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *
PyDeleteRecord(PyObject *self, PyObject *args)
{	HANDLE hDb=INVALID_HANDLE_VALUE;
	PEGOID recOid=NULL;

	if (!PyArg_ParseTuple(args, "ii:DeleteRecord", &hDb, &recOid))
		return NULL;
	if (!PegDeleteRecord(hDb, recOid)) {
		return PyWin_SetAPIError("PegDeleteRecord");
	}
	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *
PySeekDatabase(PyObject *self, PyObject *args)
{	PyObject *obValue=NULL;
	HANDLE hDb=NULL;
	DWORD dwSeekType=0;
	DWORD dwValue=0;
	DWORD dwIndex=0;
	PEGOID recOid=NULL;
	PEGPROPVAL pv;

	if (!PyArg_ParseTuple(args, "iiO:SeekDatabase", &hDb, &dwSeekType, &obValue))
		return NULL;

	switch(dwSeekType) {

		case PEGDB_SEEK_PEGOID:
		case PEGDB_SEEK_VALUENEXTEQUAL:
		case PEGDB_SEEK_BEGINNING:
		case PEGDB_SEEK_CURRENT:
		case PEGDB_SEEK_END:
			dwValue=(DWORD)PyInt_AsLong(obValue);
			break;

		case PEGDB_SEEK_VALUESMALLER:
		case PEGDB_SEEK_VALUEFIRSTEQUAL:
		case PEGDB_SEEK_VALUEGREATER:
			if (!PyToPropVal(obValue, &pv)) {
				return NULL;
			}
			dwValue=(DWORD)&pv;
			break;
	}
	recOid=PegSeekDatabase(hDb, dwSeekType, dwValue, &dwIndex);
	if (recOid==0) {
		Py_INCREF(Py_None);
		return Py_None;
	}
	return Py_BuildValue("(ii)", recOid, dwIndex);
}

static PyObject *
PyReadRecordProps(PyObject *self, PyObject *args)
{	PyObject *obProps=NULL;
	PyObject *obValue=NULL;
	PEGOID recOid=NULL;
	HANDLE hDb=NULL;
	WORD cProps=0;
	BYTE *lpBuf=NULL;
	DWORD cbBuf=0;

	if (!PyArg_ParseTuple(args, "i:ReadRecordProps", &hDb))
		return NULL;
	recOid=PegReadRecordProps(hDb, PEGDB_ALLOWREALLOC, &cProps, NULL, &lpBuf, &cbBuf);
	if (recOid==0) {
		LocalFree(lpBuf);
		Py_INCREF(Py_None);
		return Py_None;
	}
	obProps=PropValsToPy((PEGPROPVAL *)lpBuf, (DWORD)cProps);
	LocalFree(lpBuf);
	obValue=Py_BuildValue("(iO)", recOid, obProps);
	Py_XDECREF(obProps);
	return obValue;
}

static PyObject *
PyWriteRecordProps(PyObject *self, PyObject *args)
{	PyObject *obProps=NULL;
	PEGPROPVAL *ppv=NULL;
	PEGOID obOid=NULL;
	PEGOID rcOid=NULL;
	HANDLE hDb=NULL;
	WORD cProps=0;

	if (!PyArg_ParseTuple(args, "iiO:WriteRecordProps", &hDb, &rcOid, &obProps))
		return NULL;
	if (!PyToPropVals(obProps, &ppv))
		return NULL;
	cProps=(WORD)PyObject_Length(obProps);
	obOid=PegWriteRecordProps(hDb, rcOid, cProps, ppv);
	FreePropVals(ppv);
	if (obOid==0)
		return PyWin_SetAPIError("PegWriteRecordProps");
	return Py_BuildValue("i", obOid);
}

static PyObject *
PyOidGetInfo(PyObject *self, PyObject *args)
{	PyObject *obInfo=NULL;
	PyObject *obTime=NULL;
	PyObject *obSort=NULL;
	PyObject *obName=NULL;
	PEGOIDINFO oidInfo;
	PEGOID dbOid=NULL;

	if (!PyArg_ParseTuple(args, "i:OidGetInfo", &dbOid))
		return NULL;
	memset(&oidInfo, 0, sizeof(PEGOIDINFO));
	if (!PegOidGetInfo(dbOid, &oidInfo)) {
			return PyWin_SetAPIError("PegOidGetInfo");
	}
	obTime=PyWinObject_FromFILETIME(oidInfo.infDatabase.ftLastModified);
	obSort=SortOrderSpecToPy(&oidInfo.infDatabase.rgSortSpecs[0], (DWORD)oidInfo.infDatabase.wNumSortOrder);
	obName=PyWinObject_FromTCHAR(oidInfo.infDatabase.szDbaseName);
	obInfo=Py_BuildValue("(iiOiiiiOO)",
						dbOid,
						oidInfo.infDatabase.dwFlags,
						obName,
						oidInfo.infDatabase.dwDbaseType,
						(DWORD)oidInfo.infDatabase.wNumRecords,
						(DWORD)oidInfo.infDatabase.wNumSortOrder,
						oidInfo.infDatabase.dwSize,
						obTime,
						obSort
						);
	Py_XDECREF(obName);
	Py_XDECREF(obTime);
	Py_XDECREF(obSort);
	return obInfo;
}
/*
static PyObject *
PySetDatabaseInfo(PyObject *self, PyObject *args)
{	PyObject *obInfo=NULL;
	PEGDBASEINFO dbInfo;
	PEGOID dbOid=NULL;

	if (!PyArg_ParseTuple(args, "iO:SetDatabaseInfo", &dbOid, &obInfo))
		return NULL;
	if (!PyToDbaseInfo(obInfo, &dbInfo)) {
		return NULL;
	}
	if (!PegSetDatabaseInfo(dbOid, &dbInfo)) {
			return PyWin_SetAPIError("PegSetDatabaseInfo");
	}
	Py_INCREF(Py_None);
	return Py_None;
}
*/
static PyObject *
PyPROP_TAG(PyObject *self, PyObject *args)
{	ULONG ulPropType=0;
	ULONG ulPropId=0;

	if (!PyArg_ParseTuple(args, "ii:PROP_TAG", &ulPropType, &ulPropId))
		return NULL;
	return PyInt_FromLong(((ulPropId << 16) | ulPropType));
}

static PyObject *
PyPROP_TYPE(PyObject *self, PyObject *args)
{	ULONG ulPropTag=0;

	if (!PyArg_ParseTuple(args, "i:PROP_TYPE", &ulPropTag))
		return NULL;

	return PyInt_FromLong((ulPropTag & (ULONG)0x0000FFFF));
}

static PyObject *
PyPROP_ID(PyObject *self, PyObject *args)
{	ULONG ulPropTag=0;

	if (!PyArg_ParseTuple(args, "i:PROP_ID", &ulPropTag))
		return NULL;
	return PyInt_FromLong((ulPropTag >> 16));
}



/*
CeCreateDatabase(dbName, dbType, dbSortOrder)
CeOpenDatabase(dbOid, dbName, dbSortProperty, dwFlags)
CeFindDatabases([dbType])
CeDeleteDatabase(dbOid)
CeDeleteRecord(hDatabase, obRecordOid)
CeSeekDatabase(hDatabase, dwSeekType, obValue)
CeReadRecordProps(hDatabase)
CeWriteRecordProps(hDatabase, recOid, obPropVals)
CeOidGetInfo(obOid)
CeSetDatabaseInfo
PROP_TAG(prop_type, prop_id)
PROP_TYPE(prop_tag)
PROP_ID(prop_id)
*/

static PyMethodDef win32db_methods[]=
{	{"CreateDatabase", 		PyCreateDatabase, 1},
	{"OpenDatabase", 		PyOpenDatabase, 1},
	{"FindDatabases", 		PyFindDatabases, 1},
	{"DeleteDatabase", 		PyDeleteDatabase, 1},
	{"DeleteRecord", 		PyDeleteRecord, 1},
	{"SeekDatabase", 		PySeekDatabase, 1},
	{"ReadRecordProps", 	PyReadRecordProps, 1},
	{"WriteRecordProps", 	PyWriteRecordProps, 1},
	{"OidGetInfo", 			PyOidGetInfo, 1},
//	{"SetDatabaseInfo", 	PySetDatabaseInfo, 1},
	{"PROP_TAG",			PyPROP_TAG, 1},
	{"PROP_TYPE",			PyPROP_TYPE, 1},
	{"PROP_ID",				PyPROP_ID, 1},
	{NULL, NULL}
};



#define CONST_LONG(n) PyDict_SetItemString(d, #n, PyInt_FromLong((LONG)n))

extern "C" __declspec(dllexport) void initwin32db(void)
{	PyObject *m=NULL;
	PyObject *d=NULL;
	PyWinGlobals_Ensure();

	m=Py_InitModule4("win32db", win32db_methods, "", (PyObject*)NULL, PYTHON_API_VERSION);
	d=PyModule_GetDict(m);
	PyDict_SetItemString(d, "error", PyWinExc_ApiError);


	CONST_LONG(PEGVT_I2);
	CONST_LONG(PEGVT_UI2);
	CONST_LONG(PEGVT_I4);
	CONST_LONG(PEGVT_UI4);
	CONST_LONG(PEGVT_FILETIME);
	CONST_LONG(PEGVT_LPWSTR);
	CONST_LONG(PEGVT_BLOB);

	CONST_LONG(OBJTYPE_INVALID);
	CONST_LONG(OBJTYPE_FILE);
	CONST_LONG(OBJTYPE_DIRECTORY);
	CONST_LONG(OBJTYPE_DATABASE);
	CONST_LONG(OBJTYPE_RECORD);

	CONST_LONG(PEGDB_SORT_DESCENDING);
	CONST_LONG(PEGDB_SORT_CASEINSENSITIVE);
	CONST_LONG(PEGDB_SORT_UNKNOWNFIRST);
	CONST_LONG(PEGDB_SORT_GENERICORDER);
	CONST_LONG(PEGDB_MAXDBASENAMELEN);
	CONST_LONG(PEGDB_MAXSORTORDER);
	CONST_LONG(PEGDB_VALIDNAME);
	CONST_LONG(PEGDB_VALIDTYPE);
	CONST_LONG(PEGDB_VALIDSORTSPEC);
	CONST_LONG(PEGDB_VALIDMODTIME);
	CONST_LONG(PEGDB_AUTOINCREMENT);
	CONST_LONG(PEGDB_SEEK_PEGOID);
	CONST_LONG(PEGDB_SEEK_BEGINNING);
	CONST_LONG(PEGDB_SEEK_END);
	CONST_LONG(PEGDB_SEEK_CURRENT);
	CONST_LONG(PEGDB_SEEK_VALUESMALLER);
	CONST_LONG(PEGDB_SEEK_VALUEFIRSTEQUAL);
	CONST_LONG(PEGDB_SEEK_VALUEGREATER);
	CONST_LONG(PEGDB_SEEK_VALUENEXTEQUAL);
	CONST_LONG(PEGDB_PROPNOTFOUND);
	CONST_LONG(PEGDB_PROPDELETE);
	CONST_LONG(PEGDB_MAXDATABLOCKSIZE);
	CONST_LONG(PEGDB_MAXPROPDATASIZE);
	CONST_LONG(PEGDB_MAXRECORDSIZE);
	CONST_LONG(PEGDB_ALLOWREALLOC);
}
