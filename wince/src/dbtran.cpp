#include <windows.h>
#include <tchar.h>
#include <addrmapi.h>
#include "Python.h"
#include "PyWinTypes.h"

#if 0
const __int64 FT_EPOCH=116444736000000000;
const __int64 FT_TICKS=10000000;


// PyTimeToFileTime: convert a python float to a FILETIME

BOOL PyTimeToFileTime(PyObject *ob, FILETIME *p)
{	__int64 iTime=0;
	double d=0;

	d=PyFloat_AsDouble(ob);
	iTime=(__int64)(d * FT_TICKS) + FT_EPOCH;
	p->dwHighDateTime=(DWORD)((iTime >> 32) & 0x00000000FFFFFFFF);
	p->dwLowDateTime =(DWORD)(iTime & 0x00000000FFFFFFFF);
	return TRUE;
}


// FileTimeToPyTime: convert a FILETIME to python float

PyObject *FileTimeToPyTime(FILETIME *p)
{	__int64  x;

	x=((__int64)p->dwHighDateTime << 32) + ((unsigned)p->dwLowDateTime);
	x-=(0x19db1ded53ea710L);
	x/=10000000L;
	return PyFloat_FromDouble((double)x);
}
#endif

// PyToSortOrderSpec: Convert an array of python tuples of the 
// format (propid, flags) to an array of SORTORDERSPEC structs.

BOOL PyToSortOrderSpec(PyObject *ob, SORTORDERSPEC **p)
{	PyObject *obItem=NULL;
	SORTORDERSPEC *pss=NULL;
	DWORD dwLen=0;
	DWORD dwNum=0;
	DWORD dwVal=0;

	if (!PySequence_Check(ob)) {
		PyErr_SetString(PyExc_TypeError, "SORTORDERSPEC must be a sequence of tuples");
		return FALSE;
	}
	dwLen=PyObject_Length(ob);
	if (dwLen > PEGDB_MAXSORTORDER) {
		PyErr_SetString(PyExc_ValueError, "Too many items in SORTORDERSPEC");
		return FALSE;
	}
	*p=(SORTORDERSPEC *)LocalAlloc(LPTR, sizeof(SORTORDERSPEC) * dwLen);
	pss=*p;
	for (dwNum=0; dwNum < dwLen; dwNum++) {
		obItem=PySequence_GetItem(ob, dwNum);
		if (!PySequence_Check(obItem)) {
			LocalFree(*p);
			PyErr_SetString(PyExc_TypeError, "SORTORDERSPEC must be a sequence of tuples");
			return FALSE;
		}
		dwVal=(DWORD)PyInt_AsLong(PySequence_GetItem(obItem, 0));
		if (dwVal == -1) {
			LocalFree(*p);
			PyErr_SetString(PyExc_TypeError, "SORTORDERSPEC items must be ints");
			return FALSE;
		}
		pss->propid=(PEGPROPID)dwVal;

		dwVal=(DWORD)PyInt_AsLong(PySequence_GetItem(obItem, 1));
		if (dwVal == -1) {
			LocalFree(*p);
			PyErr_SetString(PyExc_TypeError, "SORTORDERSPEC items must be ints");
			return FALSE;
		}
		pss->dwFlags=dwVal;
		pss++;
	}
	return TRUE;
}


// SortOrderSpecToPy: Convert an array of SORTORDERSPEC structures 
// to a list of python tuples of the format (propid, flags).

PyObject *SortOrderSpecToPy(SORTORDERSPEC *p, DWORD dwLen)
{	PyObject *obList=NULL;
	PyObject *obItem=NULL;
	DWORD dwNum=0;

	obList=PyList_New(0);
	for (dwNum=0; dwNum < dwLen; dwNum++) {
		obItem=Py_BuildValue("(ii)", p[dwNum].propid, p[dwNum].dwFlags);
		PyList_Append(obList, obItem);
		Py_XDECREF(obItem);
	}
	return obList;
}


// PropValToPy: Convert a CEPROPVAL structure to a
// python tuple of the format (propid, value).

PyObject *PropValToPy(PEGPROPVAL *pVal)
{
	PyObject *ob=NULL;
	switch (LOWORD(pVal->propid)) {
		case CEVT_I2:
			ob = PyInt_FromLong(pVal->val.iVal);
			break;
		case CEVT_UI2:
			ob = PyInt_FromLong(pVal->val.uiVal);
			break;
		case CEVT_I4:
			ob = PyInt_FromLong(pVal->val.lVal);
			break;
		case CEVT_UI4:
			ob = PyInt_FromLong(pVal->val.ulVal);
			break;

		case CEVT_FILETIME:
			ob = PyWinObject_FromFILETIME(pVal->val.filetime);
			break;

		case CEVT_LPWSTR:
			ob = PyWinObject_FromTCHAR(pVal->val.lpwstr);
			break;

		case CEVT_BLOB:
			ob = PyString_FromStringAndSize((char *)pVal->val.blob.lpb, pVal->val.blob.dwCount);
			break;
		default:
			PyErr_SetString(PyExc_TypeError, "Unexpected value type");
			ob = NULL;
	}
	if (ob==NULL) return NULL;
	PyObject *rc = Py_BuildValue("(iO)", pVal->propid, ob);
	Py_DECREF(ob);
	return rc;
}


// PropValsToPy: Convert an array of CEPROPVAL structures to
// python tuples of the format (propid, value).

PyObject *PropValsToPy(CEPROPVAL *p, DWORD dwLen)
{	PyObject *obList=NULL;
	PyObject *obItem=NULL;
	PEGPROPVAL *ppv=NULL;
	DWORD dwNum=0;

	ppv=p;
	obList=PyList_New(0);
	for (dwNum=0; dwNum < dwLen; dwNum++) {
		obItem=PropValToPy(ppv);
		if (obItem==NULL) {
			Py_DECREF(obList);
			return NULL;
		}
		PyList_Append(obList, obItem);
		Py_DECREF(obItem);
		ppv++;
	}
	return obList;
}


// PyToPropVal: Convert a python tuple of the format
// (propid, value) to a PEGPROPVAL structure.

BOOL PyToPropVal(PyObject *ob, PEGPROPVAL *p)
{	PyObject *obId=NULL;
	PyObject *obVal=NULL;
	PEGBLOB *lpBlob=NULL;
	FILETIME ft;
	double d=0;
	BYTE *lpByte=NULL;
	DWORD dwSize=0;

	if ((!PyTuple_Check(ob)) || (PyTuple_Size(ob) != 2)) {
		PyErr_SetString(PyExc_ValueError, "Value must be a 2 tuple");
		return FALSE;
	}
	obId =PyTuple_GET_ITEM(ob, 0);
	obVal=PyTuple_GET_ITEM(ob, 1);

	memset(p, 0, sizeof(PEGPROPVAL));
	p->propid=(PEGPROPID)PyInt_AsLong(obId);

	switch (LOWORD(p->propid)) {
		case CEVT_I2:
			if (!PyInt_Check(obVal)) {
				PyErr_SetString(PyExc_TypeError, "Integer expected");
				return FALSE;
			}
			p->val.iVal=(short)PyInt_AsLong(obVal);
			break;

		case CEVT_UI2:
			if (!PyInt_Check(obVal)) {
				PyErr_SetString(PyExc_TypeError, "Integer expected");
				return FALSE;
			}
			p->val.uiVal=(USHORT)PyInt_AsLong(obVal);
			break;

		case CEVT_I4:
			if (!PyInt_Check(obVal)) {
				PyErr_SetString(PyExc_TypeError, "Integer expected");
				return FALSE;
			}
			p->val.lVal=(LONG)PyInt_AsLong(obVal);
			break;

		case CEVT_UI4:
			if (!PyInt_Check(obVal)) {
				PyErr_SetString(PyExc_TypeError, "Integer expected");
				return FALSE;
			}
			p->val.ulVal=(ULONG)PyInt_AsLong(obVal);
			break;

		case CEVT_FILETIME:
			if (!PyWinObject_AsFILETIME(obVal, &ft)) {
				return FALSE;
			}
			p->val.filetime=ft;
			break;

		case CEVT_LPWSTR: {
			if (!PyString_Check(obVal)) {
				PyErr_SetString(PyExc_TypeError, "String expected");
				return FALSE;
			}
			TCHAR *szVal;
			if (!PyWinObject_AsTCHAR(obVal, &szVal, FALSE))
				return FALSE;
			int len = _tcslen(szVal);
			p->val.lpwstr=(TCHAR *)LocalAlloc(LPTR, sizeof(TCHAR) * (len+1));
			if (p->val.lpwstr==NULL) {
				PyErr_SetString(PyExc_MemoryError, "Allocating string");
				PyWinObject_FreeTCHAR(szVal);
				return FALSE;
			}
			_tcscpy(p->val.lpwstr, szVal);
			PyWinObject_FreeTCHAR(szVal);
			break;
			}

		case CEVT_BLOB:
			if (!PyString_Check(obVal)) {
				PyErr_SetString(PyExc_TypeError, "String expected");
				return FALSE;
			}
			dwSize=PyString_Size(obVal);
			lpByte=(BYTE *)LocalAlloc(LPTR, dwSize);
//			lpBlob=(PEGBLOB *)LocalAlloc(LPTR, sizeof(PEGBLOB));
//			lpBlob->dwCount=dwSize;
//			lpBlob->lpb=lpByte;
//			p->val.blob=*lpBlob;
			p->val.blob.dwCount=dwSize;
			p->val.blob.lpb=lpByte;
			break;
	}
	return TRUE;
}


// FreePropVals: Free an array of PROPVALS and any var
// length data allocated by PyToPropVals

BOOL FreePropVals(PEGPROPVAL *p)
{	PEGPROPVAL *pp=p;
	WORD wType=0;

	while(pp->propid != 0) {
		wType=LOWORD(pp->propid);
		if (wType==PEGVT_LPWSTR) {
			LocalFree(pp->val.lpwstr);
		}
		if (wType==PEGVT_BLOB) {
			LocalFree(p->val.blob.lpb);
//			LocalFree(p->val.blob);
		}
		pp++;
	}
	LocalFree(p);
	return TRUE;
}


// PyToPropVals: Turn a sequence of (propid, value) tuples into 
// an array of PEGPROPVALs which must be freed by FreePropVals.

BOOL PyToPropVals(PyObject *ob, PEGPROPVAL **p)
{	PEGPROPVAL *lppv=NULL;
	PyObject *obItem=NULL;
	DWORD dwLen=0;
	DWORD dwNum=0;

	if ((!PySequence_Check(ob)) || (PyObject_Length(ob) < 1)) {
		PyErr_SetString(PyExc_TypeError, "PROPVALS must be a sequence of tuples");
		return FALSE;
	}
	dwLen=PyObject_Length(ob);
	*p=(PEGPROPVAL *)LocalAlloc(LPTR, sizeof(PEGPROPVAL) * (dwLen + 1));
	lppv=*p;
	for (dwNum=0; dwNum < dwLen; dwNum++) {
		obItem=PySequence_GetItem(ob, dwNum);
		if (!PyToPropVal(obItem, lppv)) {
			FreePropVals(*p);
			return FALSE;
		}
		lppv++;
	}
	return TRUE;
}


// PyToDbaseInfo: Convert a python tuple to PEGDBASEINFO

BOOL PyToDbaseInfo(PyObject *ob, PEGDBASEINFO *p)
{	PyObject *obItem=NULL;
	SORTORDERSPEC *pss=NULL;
	SORTORDERSPEC *psi=NULL;
	DWORD n=0;

	if ((!PySequence_Check(ob)) || (PyObject_Length(ob) != 8)) {
		PyErr_SetString(PyExc_TypeError, "PEGDBASEINFO must be an 8-tuple");
		return FALSE;
	}
	memset(p, 0, sizeof(PEGDBASEINFO));
	obItem=PyTuple_GET_ITEM(ob, 0);
	p->dwFlags=(DWORD)PyInt_AsLong(obItem);
	obItem=PyTuple_GET_ITEM(ob, 1);
	wsprintf(p->szDbaseName, TEXT("%hs"), PyString_AsString(obItem));
	obItem=PyTuple_GET_ITEM(ob, 2);
	p->dwDbaseType=(DWORD)PyInt_AsLong(obItem);
	obItem=PyTuple_GET_ITEM(ob, 3);
	p->wNumRecords=(WORD)PyInt_AsLong(obItem);
	obItem=PyTuple_GET_ITEM(ob, 4);
	p->wNumSortOrder=(WORD)PyInt_AsLong(obItem);
	obItem=PyTuple_GET_ITEM(ob, 5);
	p->dwSize=(DWORD)PyInt_AsLong(obItem);
	obItem=PyTuple_GET_ITEM(ob, 6);
	PyWinObject_AsFILETIME(obItem, &(p->ftLastModified));
	obItem=PyTuple_GET_ITEM(ob, 7);
	if (!PyToSortOrderSpec(obItem, &pss)) {
		LocalFree(p);
		return FALSE;
	}
	psi=p->rgSortSpecs;
	for (n=0; n < (DWORD)PyObject_Length(obItem); n++) {
		psi[n].propid=pss[n].propid;
		psi[n].dwFlags=pss[n].dwFlags;
	}
	LocalFree(pss);
	return TRUE;
}

