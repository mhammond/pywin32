/***********************************************************

win32printmodule.cpp -- module for interface into printer API


Note that this source file contains embedded documentation.
This documentation consists of marked up text inside the
C comments, and is prefixed with an '@' symbol.  The source
files are processed by a tool called "autoduck" which
generates Windows .hlp files.
@doc

******************************************************************/

#include "windows.h"

#include "Python.h"
#include "PyWinTypes.h"
#include "PyWinObjects.h"


// Printer stuff.
// @pymethod int|win32print|OpenPrinter|Retrieves a handle to a printer.
static PyObject *PyOpenPrinter(PyObject *self, PyObject *args)
{
	char *printer;
	if (!PyArg_ParseTuple(args, "s:OpenPrinter", 
	          &printer)) // @pyparm string|printer||printer or print server name.
		return NULL;
	HANDLE handle;
	if (!OpenPrinter(printer, &handle, NULL))
		return PyWin_SetAPIError("OpenPrinter");
	return Py_BuildValue("i", (int)handle);
}

// @pymethod |win32print|ClosePrinter|Closes a handle to a printer.
static PyObject *PyClosePrinter(PyObject *self, PyObject *args)
{
	int handle;
	if (!PyArg_ParseTuple(args, "i:ClosePrinter", 
	          &handle)) // @pyparm int|handle||handle to printer object
		return NULL;
	if (!ClosePrinter((HANDLE)handle))
		return PyWin_SetAPIError("ClosePrinter");
	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod tuple|win32print|GetPrinter|Retrieves information about a printer
static PyObject *PyGetPrinter(PyObject *self, PyObject *args)
{
	int handle;
	DWORD needed;
	if (!PyArg_ParseTuple(args, "i:GetPrinter", 
	          &handle)) // @pyparm int|handle||handle to printer object
		return NULL;
	// first allocate memory.
	GetPrinter((HANDLE)handle, 2, NULL, 0, &needed );
	if (GetLastError()!=ERROR_INSUFFICIENT_BUFFER)
		return PyWin_SetAPIError("GetPrinter");
	PRINTER_INFO_2 *pInfo = (PRINTER_INFO_2 *)malloc(needed);
	if (pInfo==NULL)
		PyWin_SetAPIError("No memory for printer information");
	if (!GetPrinter((HANDLE)handle, 2, (LPBYTE)pInfo, needed, &needed )) {
		free(pInfo);
		return PyWin_SetAPIError("GetPrinter");
	}
	PyObject *rc = Py_BuildValue("ssssssszssssziiiiiiii",
		    pInfo->pServerName, pInfo->pPrinterName, 	pInfo->pShareName, pInfo->pPortName,
			pInfo->pDriverName, pInfo->pComment, pInfo->pLocation, NULL, pInfo->pSepFile,
			pInfo->pPrintProcessor, pInfo->pDatatype, pInfo->pParameters, NULL,
			pInfo->Attributes, pInfo->Priority, pInfo->DefaultPriority, pInfo->StartTime, pInfo->UntilTime,
			pInfo->Status, pInfo->cJobs, pInfo->AveragePPM);
	free(pInfo);
	return rc;
}


// @pymethod int|win32print|AddPrinterConnection|Connects to remote printer
static PyObject *PyAddPrinterConnection(PyObject *self, PyObject *args)
{
	char *printer;
	if (!PyArg_ParseTuple(args, "s:AddPrinterConnection", 
	          &printer)) // @pyparm string|printer||printer to connect to (eg: \\server\printer).
		return NULL;
	if (!AddPrinterConnection(printer))
		return PyWin_SetAPIError("AddPrinterConnection");
	Py_INCREF(Py_None);
	return Py_None;
}


// @pymethod int|win32print|DeletePrinterConnection|Removes connection to remote printer
static PyObject *PyDeletePrinterConnection(PyObject *self, PyObject *args)
{
	char *printer;
	if (!PyArg_ParseTuple(args, "s:DeletePrinterConnection", 
	          &printer)) // @pyparm string|printer||printer to disconnect from (eg: \\server\printer).
		return NULL;
	if (!DeletePrinterConnection(printer))
		return PyWin_SetAPIError("DeletePrinterConnection");
	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod int|win32print|GetDefaultPrinter|Returns the default printer.
static PyObject *PyGetDefaultPrinter(PyObject *self, PyObject *args)
{
	char *printer, *s;
	int printer_size= 100;

	/* Windows < 2000 does not have a GetDefaultPrinter so the default printer
	   must be retreived from registry */

	if (NULL == (printer= (char *)malloc(printer_size)))
	{
		PyErr_SetString(PyExc_MemoryError, "Malloc failed.");
		return NULL;
	}
	if (0 == GetProfileString("Windows", "Device", "", printer, printer_size))
	{
		PyErr_SetString(PyExc_RuntimeError, "The default printer was not found.");
		return NULL;
	}
	if (NULL == (s= strchr(printer, ',')))
	{
		PyErr_SetString(PyExc_RuntimeError, "The returned printer is malformed.");
		return NULL;
	}
	*s= 0;
	PyObject *ret= Py_BuildValue("s", printer);
	free(printer);
	return ret;
}


// @pymethod int|win32print|SetDefaultPrinter|Sets the default printer.
static PyObject *PySetDefaultPrinter(PyObject *self, PyObject *args)
{
	char *printer, *info, *dprinter;
	int info_size= 100;

	/* Windows < 2000 does not have a SetDefaultPrinter so the default printer
	   must be set in the registry */

	if (!PyArg_ParseTuple(args, "s:SetDefaultPrinter", 
	        &printer)) // @pyparm string|printer||printer to set as default
		return NULL;
	
	if (NULL == (info= (char *)malloc(info_size)))
	{
		PyErr_SetString(PyExc_MemoryError, "Malloc failed.");
		return NULL;
	}
	if (0 == GetProfileString("Devices", printer, "", info, info_size))
	{
		free(info);
		PyErr_SetString(PyExc_RuntimeError, "The printer was not found.");
		return NULL;
	}
	if (NULL == (dprinter= (char *)malloc(strlen(printer) + strlen(info) + 3)))
	{
		free(info);
		PyErr_SetString(PyExc_MemoryError, "Malloc failed.");
		return NULL;
	}
	strcpy(dprinter, printer);
	strcat(dprinter, ",");
	strcat(dprinter, info);
	WriteProfileString("Windows", "device", dprinter);
	SendNotifyMessage(HWND_BROADCAST,WM_SETTINGCHANGE,0,0);
	free(dprinter);
	free(info);
	Py_INCREF(Py_None);
	return Py_None;
}



// @pymethod int|win32print|EnumPrinters|Enumerates printers, print servers, domains and print providers.
static PyObject *PyEnumPrinters(PyObject *self, PyObject *args)
{
	DWORD flags;
	DWORD level= 1;
	BYTE *buf;
	DWORD bufsize;
	DWORD bufneeded;
	DWORD printersreturned;
	char *name= NULL;
	DWORD i;	

	if (!PyArg_ParseTuple(args, "i|zi:EnumPrinters", 
					&flags,   // @pyparm int|flag|| types of printer objects to enumerate (PRINTER_ENUM_*).
					&name,    // @pyparm string|name|None|name of printer object.
					&level))  // @pyparm int|level|1|type of printer info structure (only PRINTER_INFO_1 is supported)			      
		return NULL;
	if (level != 1)
	{
		PyErr_SetString(PyExc_ValueError, "This information level is not supported");
		return NULL;
	}
	EnumPrinters(flags, name, level, NULL, 0, &bufneeded, &printersreturned);
	if (GetLastError()!=ERROR_INSUFFICIENT_BUFFER)
		return PyWin_SetAPIError("EnumPrinters");
	bufsize= bufneeded;
	if (NULL == (buf= (BYTE *)malloc(bufsize)))
	{
		PyErr_SetString(PyExc_MemoryError, "Malloc failed.");
		return NULL;
	}

	if (!EnumPrinters(flags, name, level, buf, bufsize, &bufneeded, &printersreturned))
	{
		free(buf);
		return PyWin_SetAPIError("EnumPrinters");
	}

	PyObject *ret = PyTuple_New(printersreturned);
	for (i= 0; i < printersreturned; i++)
	{
		PRINTER_INFO_1 *info;
		info= (PRINTER_INFO_1 *)(buf + i * sizeof(PRINTER_INFO_1));
		PyTuple_SetItem(ret, i, Py_BuildValue("isss", (int)info->Flags, info->pDescription, info->pName, info->pComment));
	}
	free(buf);
	return ret;
}





/* List of functions exported by this module */
// @module win32print|A module, encapsulating the Windows Win32 API.
static struct PyMethodDef win32print_functions[] = {
	{"OpenPrinter",				PyOpenPrinter, 1}, // @pymeth OpenPrinter|Retrieves a handle to a printer.
	{"GetPrinter",				PyGetPrinter       ,1}, // @pymeth GetPrinter|Retrieves information about a printer
	{"ClosePrinter",			PyClosePrinter,     1}, // @pymeth ClosePrinter|Closes a handle to a printer.
	{"AddPrinterConnection",	PyAddPrinterConnection, 1}, // @pymeth AddPrinterConnection|Connects to a network printer.
	{"DeletePrinterConnection",	PyDeletePrinterConnection, 1}, // @pymeth DeletePrinterConnection|Disconnects from a network printer.
	{"EnumPrinters",			PyEnumPrinters, 1}, // @pymeth EnumPrinters|Enumerates printers, print servers, domains and print providers.
	{"GetDefaultPrinter",		PyGetDefaultPrinter, 1}, // @pymeth GetDefaultPrinter|Returns the default printer.
	{"SetDefaultPrinter",		PySetDefaultPrinter, 1}, // @pymeth SetDefaultPrinter|Sets the default printer.
	{ NULL }
};


static void AddConstant(PyObject *dict, char *name, long val)
{
  PyObject *nv = PyInt_FromLong(val);
  PyDict_SetItemString(dict, name, nv );
  Py_XDECREF(nv);
}


extern "C" __declspec(dllexport) void
initwin32print(void)
{
  PyObject *module, *dict;
  module = Py_InitModule("win32print", win32print_functions);
  dict = PyModule_GetDict(module);
  AddConstant(dict, "PRINTER_INFO_1", 1);
  AddConstant(dict, "PRINTER_ENUM_LOCAL", PRINTER_ENUM_LOCAL);
  AddConstant(dict, "PRINTER_ENUM_NAME", PRINTER_ENUM_NAME);
  AddConstant(dict, "PRINTER_ENUM_SHARED", PRINTER_ENUM_SHARED);
  AddConstant(dict, "PRINTER_ENUM_DEFAULT", PRINTER_ENUM_DEFAULT);
  AddConstant(dict, "PRINTER_ENUM_CONNECTIONS", PRINTER_ENUM_CONNECTIONS);
  AddConstant(dict, "PRINTER_ENUM_NETWORK", PRINTER_ENUM_NETWORK);
  AddConstant(dict, "PRINTER_ENUM_REMOTE", PRINTER_ENUM_REMOTE);
  AddConstant(dict, "PRINTER_ENUM_EXPAND", PRINTER_ENUM_EXPAND);
  AddConstant(dict, "PRINTER_ENUM_CONTAINER", PRINTER_ENUM_CONTAINER);
  AddConstant(dict, "PRINTER_ENUM_ICON1", PRINTER_ENUM_ICON1);
  AddConstant(dict, "PRINTER_ENUM_ICON2", PRINTER_ENUM_ICON2);
  AddConstant(dict, "PRINTER_ENUM_ICON3", PRINTER_ENUM_ICON3);
  AddConstant(dict, "PRINTER_ENUM_ICON4", PRINTER_ENUM_ICON4);
  AddConstant(dict, "PRINTER_ENUM_ICON5", PRINTER_ENUM_ICON5);
  AddConstant(dict, "PRINTER_ENUM_ICON6", PRINTER_ENUM_ICON6);
  AddConstant(dict, "PRINTER_ENUM_ICON7", PRINTER_ENUM_ICON7);
  AddConstant(dict, "PRINTER_ENUM_ICON8", PRINTER_ENUM_ICON8);
}
