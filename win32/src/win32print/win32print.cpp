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
#include <stdarg.h>


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


// @pymethod None|win32print|AddPrinterConnection|Connects to remote printer
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


// @pymethod None|win32print|DeletePrinterConnection|Removes connection to remote printer
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

// @pymethod string|win32print|GetDefaultPrinter|Returns the default printer.
static PyObject *PyGetDefaultPrinter(PyObject *self, PyObject *args)
{
	char *printer, *s;
	int printer_size= 100;

	/* Windows < 2000 does not have a GetDefaultPrinter so the default printer
	   must be retrieved from registry */

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


// @pymethod None|win32print|SetDefaultPrinter|Sets the default printer.
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



// @pymethod tuple|win32print|EnumPrinters|Enumerates printers, print servers, domains and print providers.
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
	        // rdesc The result is a tuple of tuples; one for each printer enumerated.
	        // Each individual element is a tuple of (flags, description, name, comment)
	for (i= 0; i < printersreturned; i++)
	{
		PRINTER_INFO_1 *info;
		info= (PRINTER_INFO_1 *)(buf + i * sizeof(PRINTER_INFO_1));
		PyTuple_SetItem(ret, i, Py_BuildValue("isss", (int)info->Flags, info->pDescription, info->pName, info->pComment));
	}
	free(buf);
	return ret;
}


// @pymethod int|win32print|StartDocPrinter|Notifies the print spooler that a document is to be spooled for printing. To be used before using WritePrinter. Returns the Jobid of the started job.
static PyObject *PyStartDocPrinter(PyObject *self, PyObject *args)
{
	HANDLE hprinter;
	DWORD level= 1;
	char *pDocName, *pOutputFile, *pDatatype;
	DOC_INFO_1 info;
	DWORD JobID;

	if (!PyArg_ParseTuple(args, "ii(szz):StartDocPrinter",
	            &hprinter, // @pyparm int|hprinter||handle to printer (from OpenPrinter)
	            &level,     // @pyparm int|level|1|type of docinfo structure (only docinfo level 1 supported)
	            &pDocName, &pOutputFile, &pDatatype // @pyparm (string, string, string)|(DocName, OutputFile, DataType)||Sequence of document name, output file, data type
	        ))
		return NULL;

	if (level != 1)
	{
		PyErr_SetString(PyExc_ValueError, "This information level is not supported");
		return NULL;
	}

	info.pDocName= pDocName;
	info.pOutputFile= pOutputFile;
	info.pDatatype= pDatatype;

	if (0 == (JobID= StartDocPrinter(hprinter, level, (LPBYTE)&info)))
		return PyWin_SetAPIError("StartDocPrinter");

	PyObject *ret= Py_BuildValue("i", JobID);
	return ret;
}


// @pymethod None|win32print|EndDocPrinter|The EndDocPrinter function ends a print job for the specified printer. To be used after using WritePrinter.
static PyObject *PyEndDocPrinter(PyObject *self, PyObject *args)
{
	HANDLE hprinter;

	if (!PyArg_ParseTuple(args, "i:EndDocPrinter",
	            &hprinter  // @pyparm int|hprinter||handle to printer (from OpenPrinter)
	        ))
		return NULL;

	if (!EndDocPrinter(hprinter))
		return PyWin_SetAPIError("EndDocPrinter");

	Py_INCREF(Py_None);
	return Py_None;
}


// @pymethod int|win32print|WritePrinter|Copies the specified bytes to the specified printer. Suitable for copying raw Postscript or HPGL files to a printer. StartDocPrinter and EndDocPrinter should be called before and after. Returns number of bytes written to printer.
static PyObject *PyWritePrinter(PyObject *self, PyObject *args)
{
	HANDLE hprinter;
	LPVOID buf;
	DWORD buf_size;
	DWORD bufwritten_size;

	if (!PyArg_ParseTuple(args, "is#:WritePrinter",
	            &hprinter,  // @pyparm int|hprinter||Handle to printer (from OpenPrinter)
	            &buf,       // @pyparm string|buf||String to send to printer. Embedded NULL bytes are allowed.
	            &buf_size
	        ))
		return NULL;

	if (!WritePrinter(hprinter, buf, buf_size, &bufwritten_size))
		return PyWin_SetAPIError("WritePrinter");

	PyObject *ret= Py_BuildValue("i", bufwritten_size);
	return ret;
}


// convert a job structure to python. only works for level 1
PyObject *JobtoPy(DWORD level, LPBYTE buf)
{
	if (level != 1)
		return NULL;

	JOB_INFO_1 *job1= (JOB_INFO_1 *)buf;
	SYSTEMTIME localSubmitted;

	SystemTimeToTzSpecificLocalTime(NULL, &(job1->Submitted), &localSubmitted);
	PyObject *pylocalsubmitted= new PyTime(localSubmitted);
	PyObject *ret= Py_BuildValue("{s:i, s:s, s:s, s:s, s:s, s:s, s:s, s:i, s:i, s:i, s:i, s:i, s:O}",
	        "JobID", job1->JobId,
	        "pPrinterName", job1->pPrinterName,
	        "pMachineName", job1->pMachineName,
	        "pUserName", job1->pUserName,
	        "pDocument", job1->pDocument,
	        "pDatatype", job1->pDatatype,
	        "pStatus", job1->pStatus,
	        "Status", job1->Status,
	        "Priority", job1->Priority,
	        "Position", job1->Position,
	        "TotalPages", job1->TotalPages,
	        "PagesPrinted", job1->PagesPrinted,
	        "Submitted", pylocalsubmitted);
	Py_XDECREF(pylocalsubmitted);
	return ret;
}

// @pymethod tuple|win32print|EnumJobs|Enumerates print jobs on specified printer.
static PyObject *PyEnumJobs(PyObject *self, PyObject *args)
{
	HANDLE hprinter;
	DWORD firstjob;
	DWORD nojobs;
	DWORD level= 1;
	LPBYTE buf;
	DWORD buf_size;
	DWORD bufneeded_size;
	DWORD jobsreturned;

	if (!PyArg_ParseTuple(args, "iiii:EnumJobs",
	          &hprinter,   // @pyparm int|hPrinter||Handle of printer.
	          &firstjob,   // @pyparm int|FirstJob||location of first job in print queue to enumerate.
	          &nojobs,     // @pyparm int|NoJobs||Number of jobs to enumerate.
	          &level       // @pyparm int|Level|1|Level of information to return (only JOB_INFO_1 is supported).
	          ))
		return NULL;
	if (level != 1)
	{
		PyErr_SetString(PyExc_ValueError, "This information level is not supported");
		return NULL;
	}
	EnumJobs(hprinter, firstjob, nojobs, level, NULL, 0, &bufneeded_size, &jobsreturned);
	if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
		return PyWin_SetAPIError("EnumJobs");
	buf_size= bufneeded_size;
	if (NULL == (buf= (LPBYTE)malloc(buf_size)))
	{
		PyErr_SetString(PyExc_MemoryError, "Malloc failed.");
		return NULL;
	}
	if (!EnumJobs(hprinter, firstjob, nojobs, level, buf, buf_size, &bufneeded_size, &jobsreturned))
	{
		free(buf);
		return PyWin_SetAPIError("EnumJobs");
	}

	DWORD i;
	PyObject *ret = PyTuple_New(jobsreturned);
	for (i= 0; i < jobsreturned; i++)
	{
		PyTuple_SetItem(ret, i, JobtoPy(1, (buf + i * sizeof(JOB_INFO_1))));
	}
	free(buf);
	return ret;
}


// @pymethod dictionary|win32print|GetJob|Returns dictionary of information about a specified print job.
static PyObject *PyGetJob(PyObject *self, PyObject *args)
{
	HANDLE hprinter;
	DWORD jobid;
	DWORD level= 1;
	JOB_INFO_1 *buf;
	DWORD buf_size;
	DWORD bufneeded_size;

	if (!PyArg_ParseTuple(args, "ii|i:GetJob",
	          &hprinter,// @pyparm int|hPrinter||Handle of printer.
	          &jobid,   // @pyparm int|JobID||Job Identifier.
	          &level   // @pyparm int|Level|1|Level of information to return (only JOB_INFO_1 is supported).
	          ))
		return NULL;
	if (level != 1)
	{
		PyErr_SetString(PyExc_ValueError, "This information level is not supported");
		return NULL;
	}
	GetJob(hprinter, jobid, level, NULL, 0, &bufneeded_size);
	if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
		return PyWin_SetAPIError("GetJob");
	buf_size= bufneeded_size;
	if (NULL == (buf= (JOB_INFO_1 *)malloc(buf_size)))
	{
		PyErr_SetString(PyExc_MemoryError, "Malloc failed.");
		return NULL;
	}
	if (!GetJob(hprinter, jobid, level, (LPBYTE)buf, buf_size, &bufneeded_size))
	{
		free(buf);
		return PyWin_SetAPIError("GetJob");
	}
	PyObject *ret= JobtoPy(1, (LPBYTE)buf);
	free(buf);
	return ret;
}


// Convert a python dictionary to a job structure. Only works for level 1
// There has got to be an easier way to do this...
// Returned buffer must be freed.
LPBYTE PytoJob(DWORD level, PyObject *pyjobinfo)
{
	PyObject* temp;
	char *err= NULL;

	if (level != 1)
		return NULL;

	JOB_INFO_1 *job1;
	if (!PyDict_Check (pyjobinfo))
	{
		PyErr_SetString(PyExc_ValueError, "JOB_INFO must be a dictionary.");
		return NULL;
	}
	if (NULL == (job1= (JOB_INFO_1 *)malloc(sizeof(JOB_INFO_1))))
	{
		PyErr_SetString(PyExc_MemoryError, "Malloc failed.");
		return NULL;
	}
	if (NULL != (temp= PyDict_GetItemString(pyjobinfo, "JobID")) && PyInt_Check(temp))
		job1->JobId= PyInt_AsLong(temp);
	else
		err= "JobID invalid";
	if (NULL != (temp= PyDict_GetItemString(pyjobinfo, "pPrinterName")) && PyString_Check(temp))
		job1->pPrinterName= PyString_AsString(temp);
	else
		err= "pPrinterName invalid";
	if (NULL != (temp= PyDict_GetItemString(pyjobinfo, "pMachineName")) && PyString_Check(temp))
		job1->pMachineName= PyString_AsString(temp);
	else
		err= "pMachineName invalid";
	if (NULL != (temp= PyDict_GetItemString(pyjobinfo, "pUserName")) && PyString_Check(temp))
		job1->pUserName= PyString_AsString(temp);
	else
		err= "pUsername invalid";
	if (NULL != (temp= PyDict_GetItemString(pyjobinfo, "pDocument")) && PyString_Check(temp))
		job1->pDocument= PyString_AsString(temp);
	else
		err= "pDocument invalid";
	if (NULL != (temp= PyDict_GetItemString(pyjobinfo, "pDatatype")) && PyString_Check(temp))
		job1->pDatatype= PyString_AsString(temp);
	else
		err= "pDatatype invalid";
	if (NULL != (temp= PyDict_GetItemString(pyjobinfo, "pStatus")))
		if (PyString_Check(temp))
			job1->pStatus= PyString_AsString(temp);
		else
			job1->pStatus= NULL;
	else
		err= "pStatus invalid";
	if (NULL != (temp= PyDict_GetItemString(pyjobinfo, "Status")) && PyInt_Check(temp))
		job1->Status= PyInt_AsLong(temp);
	else
		err= "Status invalid";
	if (NULL != (temp= PyDict_GetItemString(pyjobinfo, "Priority")) && PyInt_Check(temp))
		job1->Priority= PyInt_AsLong (temp);
	else
		err= "Priority invalid";
	if (NULL != (temp= PyDict_GetItemString(pyjobinfo, "Position")) && PyInt_Check(temp))
		job1->Position= PyInt_AsLong (temp);
	else
		err= "Position invalid";
	if (NULL != (temp= PyDict_GetItemString(pyjobinfo, "TotalPages")) && PyInt_Check(temp))
		job1->TotalPages= PyInt_AsLong (temp);
	else
		err= "TotalPages invalid";
	if (NULL != (temp= PyDict_GetItemString(pyjobinfo, "PagesPrinted")) && PyInt_Check(temp))
		job1->PagesPrinted= PyInt_AsLong(temp);
	else
		err= "PagesPrinted invalid";
	if (err != NULL)
	{
		free(job1);
		PyErr_SetString(PyExc_ValueError, err);
		return NULL;
	}
	return (LPBYTE)job1;
}


// @pymethod None|win32print|SetJob|Pause, cancel, resume, set priority levels on a print job.
static PyObject *PySetJob(PyObject *self, PyObject *args)
{
	HANDLE hprinter;
	DWORD jobid;
	DWORD level= 1;
	PyObject *pyjobinfo;
	DWORD command;
	LPBYTE buf;

	if (!PyArg_ParseTuple(args, "iiiOi:GetJob",
	    &hprinter,// @pyparm int|hPrinter||Handle of printer.
	    &jobid,   // @pyparm int|JobID||Job Identifier.
	    &level,   // @pyparm int|Level|1|Level of information to return (only 0 and JOB_INFO_1 are supported).
	    &pyjobinfo, // @pyparm dict|JobInfo||JOB_INFO_1 Dictionary (can be None if Level is 0). Position should be JOB_POSITION_UNSPECIFIED.
	    &command  // @pyparm int|Command||Job command value (JOB_CONTROL_*).
	    ))
		return NULL;
	if (level != 1 && level != 0)
	{
		PyErr_SetString(PyExc_ValueError, "This information level is not supported");
		return NULL;
	}
	if (pyjobinfo == Py_None)
		buf= NULL;
	else
	{
		if (NULL == (buf= PytoJob(1, pyjobinfo)))
			return NULL;
	}
	if (!SetJob(hprinter, jobid, level, buf, command))
	{
		if (buf)
			free(buf);
		return PyWin_SetAPIError("SetJob");
	}
	if (buf)
		free(buf);
	Py_INCREF(Py_None);
	return Py_None;
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
	{"StartDocPrinter",     PyStartDocPrinter, 1},   // @pymeth StartDocPrinter|Notifies the print spooler that a document is to be spooled for printing. Returns the Jobid of the started job.
	{"EndDocPrinter",     PyEndDocPrinter, 1},   // @pymeth EndDocPrinter|The EndDocPrinter function ends a print job for the specified printer.
	{"WritePrinter",      PyWritePrinter, 1},   // @pymeth WritePrinter|Copies the specified bytes to the specified printer. StartDocPrinter and EndDocPrinter should be called before and after. Returns number of bytes written to printer.
	{"EnumJobs",        PyEnumJobs, 1},   // @pymeth EnumJobs|Enumerates print jobs on specified printer.
	{"GetJob",          PyGetJob, 1},   // @pymeth GetJob|Returns dictionary of information about a specified print job.
	{"SetJob",          PySetJob, 1},   // @pymeth SetJob|Pause, cancel, resume, set priority levels on a print job.
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
  if (!module) return;
  dict = PyModule_GetDict(module);
  if (!dict) return;
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
  AddConstant(dict, "JOB_STATUS_DELETING", JOB_STATUS_DELETING);
  AddConstant(dict, "JOB_STATUS_ERROR", JOB_STATUS_ERROR);
  AddConstant(dict, "JOB_STATUS_OFFLINE", JOB_STATUS_OFFLINE);
  AddConstant(dict, "JOB_STATUS_PAPEROUT", JOB_STATUS_PAPEROUT);
  AddConstant(dict, "JOB_STATUS_PAUSED", JOB_STATUS_PAUSED);
  AddConstant(dict, "JOB_STATUS_PRINTED", JOB_STATUS_PRINTED);
  AddConstant(dict, "JOB_STATUS_PRINTING", JOB_STATUS_PRINTING);
  AddConstant(dict, "JOB_STATUS_SPOOLING", JOB_STATUS_SPOOLING);
  AddConstant(dict, "MIN_PRIORITY", MIN_PRIORITY);
  AddConstant(dict, "MAX_PRIORITY", MAX_PRIORITY);
  AddConstant(dict, "DEF_PRIORITY", DEF_PRIORITY);
  AddConstant(dict, "JOB_INFO_1", 1);
  AddConstant(dict, "JOB_CONTROL_CANCEL", JOB_CONTROL_CANCEL);
  AddConstant(dict, "JOB_CONTROL_PAUSE", JOB_CONTROL_PAUSE);
  AddConstant(dict, "JOB_CONTROL_RESTART", JOB_CONTROL_RESTART);
  AddConstant(dict, "JOB_CONTROL_RESUME", JOB_CONTROL_RESUME);
  AddConstant(dict, "JOB_POSITION_UNSPECIFIED", JOB_POSITION_UNSPECIFIED);
}
