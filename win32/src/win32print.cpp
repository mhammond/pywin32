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

/* List of functions exported by this module */
// @module win32print|A module, encapsulating the Windows Win32 API.
static struct PyMethodDef win32print_functions[] = {
	{"OpenPrinter",         PyOpenPrinter, 1}, // @pymeth OpenPrinter|Retrieves a handle to a printer.
	{"GetPrinter",			PyGetPrinter       ,1}, // @pymeth GetPrinter|Retrieves information about a printer
	{"ClosePrinter",		PyClosePrinter,     1}, // @pymeth ClosePrinter|Closes a handle to a printer.
	{ NULL }
};

extern "C" __declspec(dllexport) void
initwin32print(void)
{
  PyObject *module;
  module = Py_InitModule("win32print", win32print_functions);
}
