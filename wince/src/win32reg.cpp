#include "Python.h"
#include "extapi.h"
#include <windows.h>
#include <tchar.h>
#include <math.h>


PyObject *error;

static PyObject *
ErrSet(DWORD n)
{	PyErr_SetObject(error, Py_BuildValue("i", n));
	return NULL;
}



static void
fixupMultiSZ(TCHAR **lpTab, BYTE *lpData, DWORD dwLen)
{	TCHAR *p;
	TCHAR *e;
	int c;

	e=(TCHAR *)(lpData + dwLen);
	for(p=(TCHAR *)lpData, c=0; p < e && *p != TEXT('\0'); p++, c++)
	{	lpTab[c]=p;
		for(; *p != TEXT('\0'); p++);
	}
}

static int
countMultiSZ(BYTE *lpData, DWORD dwLen)
{	TCHAR *p;
	TCHAR *e;
	int c;

	e=(TCHAR *)(lpData + dwLen);
	for (p=(TCHAR *)lpData, c=0; p < e && *p != TEXT('\0'); p++, c++)
		for (; p < e && *p != TEXT('\0'); p++);
	return c;
}

static int
Py2Reg(PyObject *ob, DWORD dwType, BYTE **ppData, DWORD *dwSize)
{	TCHAR *ptr, *dat;
	PyObject *p;
	int n, i, j;
	DWORD d;

	switch(dwType)
	{
		case REG_DWORD:
			if (!PyInt_Check(ob))
				return 0;
			*ppData=(BYTE *)LocalAlloc(LPTR, sizeof(DWORD));
			*dwSize=sizeof(DWORD);
			memcpy(*ppData, &PyInt_AS_LONG((PyIntObject *)ob), sizeof(DWORD));
			break;

		case REG_EXPAND_SZ:
		case REG_SZ:
			if (!PyString_Check(ob))
				return 0;
			*dwSize=(PyString_Size(ob) + 1) * sizeof(TCHAR);
			ptr=(TCHAR *)*ppData;
			ptr=TStringFromPyString(ob);
			break;

		case REG_MULTI_SZ:
			{
				if (!PyList_Check(ob))
					return 0;
				n=PyList_Size(ob);
				for(i=0; i < n; i++)
				{	p=PyList_GET_ITEM((PyListObject *)ob, i);
					if (!PyString_Check(p))
						return 0;
					d+=(PyString_GET_SIZE((PyStringObject *)p) + 1);
				}
			
				*dwSize=(d + 1) * sizeof(TCHAR);
				*ppData=(BYTE *)LocalAlloc(LPTR, *dwSize);
				ptr=(TCHAR *)*ppData;

				for(i=0; i < n; i++)
				{	p=PyList_GET_ITEM((PyListObject *)ob, i);
					dat=TStringFromPyString(p);
					j=PyString_GET_SIZE((PyStringObject *)p);
					memcpy(ptr, dat, j * sizeof(TCHAR));
					LocalFree(dat);
					ptr+=(j + 1);
				}
				break;
			}

		case REG_BINARY:
			if (!PyString_Check(ob))
				return 0;
			*dwSize=PyString_Size(ob);
			*ppData=(BYTE *)LocalAlloc(LPTR, *dwSize);
			memcpy(*ppData, PyString_AS_STRING((PyStringObject *)ob), *dwSize);
			break;

		default:
			return 0;
	}
	return 1;
}

static PyObject *
Reg2Py(BYTE *lpData, DWORD dwSize, DWORD dwType)
{	TCHAR *ppData;

	switch(dwType)
	{
		case REG_DWORD:
			return PyInt_FromLong(*(LONG *)lpData);

		case REG_EXPAND_SZ:
		case REG_SZ:
			if (((TCHAR *)lpData)[(dwSize/sizeof(TCHAR)) - 1]==TEXT('\0'))
				dwSize-=sizeof(TCHAR);
			return PyString_FromTStringAndSize((TCHAR *)lpData, (dwSize/sizeof(TCHAR)));

		case REG_MULTI_SZ:
		{	PyObject *ob;
			TCHAR **lpTab;
			int c, i;
			c=countMultiSZ(lpData, dwSize);
			lpTab=(TCHAR **)LocalAlloc(LPTR, (sizeof(TCHAR *) * c));
			fixupMultiSZ(lpTab, lpData, dwSize);
			ob=PyList_New(c);
			for(i=0; i < c; i++)
			{	PyList_SetItem(ob, i, PyString_FromTString((TCHAR *)lpTab[i]));
			}
			return ob;
		}

		case REG_BINARY:
			return Py_BuildValue("s#", (CHAR *)lpData, dwSize);

		default:
			return NULL;
	}
	return NULL;
}

static PyObject *
PyRegCloseKey(PyObject *self, PyObject *args)
{	HKEY hKey;

	if (!PyArg_ParseTuple(args, "i:RegCloseKey", &hKey))
		return NULL;
	if (RegCloseKey(hKey) != ERROR_SUCCESS)
		return ErrSet(GetLastError());
	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *
PyRegCreateKeyEx(PyObject *self, PyObject *args)
{	PyObject *obName;
	PyObject *obType;
	TCHAR *szName;
	TCHAR *szType;
	DWORD dwDisp;
	HKEY  hKey;
	HKEY  rKey;

	if (!PyArg_ParseTuple(args, "iOO:RegCreateKeyEx", &hKey, &obName, &obType))
		return NULL;
	szName=TStringFromPyString(obName);
	szType=TStringFromPyString(obType);
	if (RegCreateKeyEx(hKey, szName, 0, szType, 0, 0, NULL, &rKey, &dwDisp) != ERROR_SUCCESS)
	{	LocalFree(szName);
		LocalFree(szType);
		return ErrSet(GetLastError());
	}
	LocalFree(szName);
	LocalFree(szType);
	return PyInt_FromLong((LONG)rKey);
}

static PyObject *
PyRegDeleteKey(PyObject *self, PyObject *args)
{	PyObject *obName;
	TCHAR *szName;
	HKEY  hKey;

	if (!PyArg_ParseTuple(args, "iO:RegDeleteKey", &hKey, &obName))
		return NULL;
	szName=TStringFromPyString(obName);
	if (RegDeleteKey(hKey, szName) != ERROR_SUCCESS)
	{	LocalFree(szName);
		return ErrSet(GetLastError());
	}
	LocalFree(szName);
	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *
PyRegDeleteValue(PyObject *self, PyObject *args)
{	PyObject *obName;
	TCHAR *szName;
	HKEY hKey;

	if (!PyArg_ParseTuple(args, "iO:RegDeleteValue", &hKey, &obName))
		return NULL;
	szName=TStringFromPyString(obName);
	if (RegDeleteValue(hKey, szName) != ERROR_SUCCESS)
	{	LocalFree(szName);
		return NULL;
	}
	LocalFree(szName);
	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *
PyRegEnumKeyEx(PyObject *self, PyObject *args)
{	PyObject *ob;
	TCHAR *szName;
	DWORD dwIdx;
	DWORD dwLen;
	DWORD dwIgn;
	HKEY  hKey;
	FILETIME ft;

	if (!PyArg_ParseTuple(args, "ii:RegEnumKeyEx", &hKey, &dwIdx))
		return NULL;
	if (RegQueryInfoKey(hKey, NULL, NULL, NULL, &dwIgn, &dwLen, &dwIgn, &dwIgn, &dwIgn, &dwIgn, NULL, NULL) != ERROR_SUCCESS)
		return ErrSet(GetLastError());
	szName=(TCHAR *)LocalAlloc(LPTR, (dwLen * sizeof(TCHAR)));
	if (RegEnumKeyEx(hKey, dwIdx, szName, &dwLen, NULL, NULL, NULL, &ft) != ERROR_SUCCESS)
		return ErrSet(GetLastError());
	ob=PyString_FromTStringAndSize(szName, dwLen);
	LocalFree(szName);
	return ob;
}

static PyObject *
PyRegEnumValue(PyObject *self, PyObject *args)
{	PyObject *obName;
	PyObject *obValue;
	TCHAR *szName;
	BYTE  *lpValue;
	DWORD dwKeys;
	DWORD dwName;
	DWORD dwValue;
	DWORD dwType;
	DWORD dwIdx;
	DWORD dwIgn;
	HKEY  hKey;

	if (!PyArg_ParseTuple(args, "ii:RegEnumValue", &hKey, &dwIdx))
		return NULL;
	if (RegQueryInfoKey(hKey, NULL, NULL, NULL, &dwIgn, &dwIgn, &dwIgn, &dwIgn, &dwName, &dwValue, NULL, NULL) != ERROR_SUCCESS)
		return ErrSet(GetLastError());
	++dwName;
	++dwValue;
	szName=(TCHAR *)LocalAlloc(LPTR, (dwName * sizeof(TCHAR)));
	lpValue=(BYTE *)LocalAlloc(LPTR, (dwValue * sizeof(BYTE)));
	if (RegEnumValue(hKey, dwIdx, szName, &dwName, NULL, &dwType, lpValue, &dwValue) != ERROR_SUCCESS)
	{	LocalFree(szName);
		LocalFree(lpValue);
		return ErrSet(GetLastError());
	}
	if (RegQueryValueEx((HKEY)hKey, szName, NULL, NULL, NULL, &dwValue) != ERROR_SUCCESS)
	{	LocalFree(szName);
		LocalFree(lpValue);
		return ErrSet(GetLastError());
	}
	obName =PyString_FromTString(szName);
	obValue=Reg2Py(lpValue, dwValue, dwType);
	LocalFree(szName);
	LocalFree(lpValue);
	return Py_BuildValue("OOi", obName, obValue, dwType);
}

static PyObject *
PyRegOpenKeyEx(PyObject *self, PyObject *args)
{	PyObject *obName;
	TCHAR *szName;
	HKEY  hKey;
	HKEY  rKey;

	if (!PyArg_ParseTuple(args, "iO:RegOpenKeyEx", &hKey, &obName))
		return NULL;
	szName=TStringFromPyString(obName);
	if (RegOpenKeyEx(hKey, szName, 0, NULL, &rKey) != ERROR_SUCCESS)
	{	LocalFree(szName);
		return ErrSet(GetLastError());
	}
	LocalFree(szName);
	return PyInt_FromLong((LONG)rKey);
}

static PyObject *
PyRegQueryInfoKey(PyObject *self, PyObject *args)
{	FILETIME ft;
	DWORD dwcKeys;
	DWORD dwcVals;
	DWORD dwlKey;
	DWORD dwlClass;
	HKEY  hKey;
	double d;

	if (!PyArg_ParseTuple(args, "i:RegQueryInfoKey", &hKey))
		return NULL;
	if (RegQueryInfoKey((HKEY)hKey, NULL, NULL, NULL, &dwcKeys, &dwlKey, &dwlClass, &dwcVals, NULL, NULL, NULL, &ft) != ERROR_SUCCESS)
		return ErrSet(GetLastError());
	d=ft.dwLowDateTime;
 	d=d + pow(2.0, 32.0) * ft.dwHighDateTime;
 	return Py_BuildValue("iiO", dwcKeys, dwcVals, PyLong_FromDouble(d));
}

static PyObject *
PyRegQueryValueEx(PyObject *self, PyObject *args)
{	PyObject *obName;
	PyObject *ob;
	TCHAR *szName;
	BYTE  *lpData;
	DWORD dwType;
	DWORD dwSize;
	HKEY  hKey;

	if (!PyArg_ParseTuple(args, "iO:RegQueryValueEx", &hKey, &obName))
		return NULL;
	szName=TStringFromPyString(obName);
	if (RegQueryValueEx(hKey, szName, NULL, NULL, NULL, &dwSize) != ERROR_SUCCESS)
	{	LocalFree(szName);
		return ErrSet(GetLastError());
	}
	lpData=(BYTE *)LocalAlloc(LPTR, (dwSize * sizeof(BYTE)));
	LONG rc
	if ((rc=RegQueryValueEx(hKey, szName, NULL, &dwType, lpData, &dwSize)) != ERROR_SUCCESS)
	{	LocalFree(szName);
		LocalFree(lpData);
		return ErrSet(rc);
	}
	ob=Reg2Py(lpData, dwSize, dwType);
	LocalFree(szName);
	LocalFree(lpData);
	return Py_BuildValue("Oi", ob, dwType);
}

static PyObject *
PyRegSetValueEx(PyObject *self, PyObject *args)
{	PyObject *obName;
	PyObject *obVal;
	TCHAR *szName;
	BYTE *lpData;
	DWORD dwLen;
	DWORD dwType;
	HKEY  hKey;

	if (!PyArg_ParseTuple(args, "iOiO:RegSetValueEx", &hKey, &obName, &dwType, &obVal))
		return NULL;
	if (!Py2Reg(obVal, dwType, &lpData, &dwLen))
	{	LocalFree(lpData);
		return ErrSet(GetLastError());
	}
	szName=TStringFromPyString(obName);
	if (RegSetValueEx(hKey, szName, NULL, dwType, lpData, dwLen) != ERROR_SUCCESS)
	{	LocalFree(lpData);
		LocalFree(szName);
		return ErrSet(GetLastError());
	}
	LocalFree(lpData);
	LocalFree(szName);
	Py_INCREF(Py_None);
	return Py_None;
}



static PyMethodDef win32reg_methods[]=
{	{"RegCloseKey", 	PyRegCloseKey, 1},
	{"RegCreateKeyEx", 	PyRegCreateKeyEx, 1},
	{"RegDeleteKey", 	PyRegDeleteKey, 1},
	{"RegDeleteValue", 	PyRegDeleteValue, 1},
	{"RegEnumKeyEx", 	PyRegEnumKeyEx, 1},
	{"RegEnumValue", 	PyRegEnumValue, 1},
	{"RegOpenKeyEx",	PyRegOpenKeyEx, 1},
	{"RegQueryInfoKey", PyRegQueryInfoKey, 1},
	{"RegQueryValueEx", PyRegQueryValueEx, 1},
	{"RegSetValueEx",	PyRegSetValueEx, 1},
	{NULL, NULL}
};



#define CONST_LONG(n) PyDict_SetItemString(d, #n, PyInt_FromLong((LONG)n))

extern "C" __declspec(dllexport) void initwin32reg(void)
{	PyObject *m=NULL;
	PyObject *d=NULL;

	m=Py_InitModule4("win32reg", win32reg_methods, "", (PyObject*)NULL, PYTHON_API_VERSION);
	if (!m) return;
	d=PyModule_GetDict(m);
	if (!d) return;
	error=PyString_FromString("error");
	PyDict_SetItemString(d, "error", error);

	CONST_LONG(HKEY_CLASSES_ROOT);
	CONST_LONG(HKEY_CURRENT_USER);
	CONST_LONG(HKEY_LOCAL_MACHINE);
	CONST_LONG(HKEY_USERS);
	CONST_LONG(REG_BINARY);
	CONST_LONG(REG_DWORD);
	CONST_LONG(REG_EXPAND_SZ);
	CONST_LONG(REG_LINK);
	CONST_LONG(REG_MULTI_SZ);
	CONST_LONG(REG_NONE);
	CONST_LONG(REG_SZ);
}
