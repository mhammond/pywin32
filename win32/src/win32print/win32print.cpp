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

typedef BOOL (WINAPI *EnumFormsfunc)(HANDLE,DWORD,LPBYTE,DWORD,LPDWORD,LPDWORD);
static EnumFormsfunc enumforms=NULL;
typedef BOOL (WINAPI *AddFormfunc)(HANDLE,DWORD,LPBYTE);
static AddFormfunc addform=NULL;
typedef BOOL (WINAPI *DeleteFormfunc)(HANDLE, LPWSTR);
static DeleteFormfunc deleteform=NULL;
typedef BOOL (WINAPI *GetFormfunc)(HANDLE,LPWSTR,DWORD,LPBYTE,DWORD,LPDWORD);
static GetFormfunc getform=NULL;
typedef BOOL (WINAPI *SetFormfunc)(HANDLE, LPWSTR, DWORD, LPBYTE);
static SetFormfunc setform=NULL;
typedef BOOL (WINAPI *AddJobfunc)(HANDLE,DWORD,LPBYTE,DWORD,LPDWORD);
static AddJobfunc addjob=NULL;
typedef BOOL (WINAPI *ScheduleJobfunc)(HANDLE, DWORD);
static ScheduleJobfunc schedulejob=NULL;

static PyObject *dummy_tuple=NULL;

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
	            &hprinter, // @pyparm int|hprinter||handle to printer (from <om win32print.OpenPrinter>)
	            &level,     // @pyparm int|level|1|type of docinfo structure (only docinfo level 1 supported)
	            &pDocName, &pOutputFile, &pDatatype // @pyparm data|tuple||A tuple corresponding to the level parameter.
	        ))
		return NULL;

	// @comm For level 1, the tuple is:
	// @tupleitem 0|string|docName|Specifies the name of the document.
	// @tupleitem 1|string|outputFile|Specifies the name of an output file. To print to a printer, set this to None.
	// @tupleitem 2|string|dataType|Identifies the type of data used to record the document, such 
	// as "raw" or "emf", used to record the print job. This member can be None. If it is not None,
	// the StartDoc function passes it to the printer driver. Note that the printer driver might 
	// ignore the requested data type. 

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
	            &hprinter  // @pyparm int|hprinter||handle to printer (from <om win32print.OpenPrinter>)
	        ))
		return NULL;

	if (!EndDocPrinter(hprinter))
		return PyWin_SetAPIError("EndDocPrinter");

	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod |win32print|AbortPrinter|Deletes spool file for a printer
static PyObject *PyAbortPrinter(PyObject *self, PyObject *args)
{
	 // @pyparm int|hprinter||Handle to printer as returned by <om win32print.OpenPrinter>
	HANDLE hprinter;
	if (!PyArg_ParseTuple(args, "i:AbortPrinter",&hprinter))
		return NULL;
	if (!AbortPrinter(hprinter))
		return PyWin_SetAPIError("AbortPrinter");
	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod |win32print|StartPagePrinter|Notifies the print spooler that a page is to be printed on specified printer
static PyObject *PyStartPagePrinter(PyObject *self, PyObject *args)
{
	// @pyparm int|hprinter||Printer handle as returned by <om win32print.OpenPrinter>
	HANDLE hprinter;
	if (!PyArg_ParseTuple(args, "l:StartPagePrinter", &hprinter))
		return NULL;
	if (!StartPagePrinter(hprinter))
		return PyWin_SetAPIError("StartPagePrinter");
	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod |win32print|EndPagePrinter|Ends a page in a print job
static PyObject *PyEndPagePrinter(PyObject *self, PyObject *args)
{
	// @pyparm int|hprinter||Printer handle as returned by <om win32print.OpenPrinter>
	HANDLE hprinter;
	if (!PyArg_ParseTuple(args, "l:EndPagePrinter", &hprinter))
		return NULL;
	if (!EndPagePrinter(hprinter))
		return PyWin_SetAPIError("EndPagePrinter");
	Py_INCREF(Py_None);
	return Py_None;
}

// @object DOCINFO|A tuple of information representing a DOCINFO struct
// @prop string/<o PyUnicode>|DocName|Name of document
// @prop string/<o PyUnicode>|Output|Name of output file when printing to file. Use None for normal printing.
// @prop string/<o PyUnicode>|DataType|Type of data to be sent to printer, eg RAW, EMF, TEXT. Use None for printer default.
// @prop int|Type|Flag specifying mode of operation.  Can be DI_APPBANDING, DI_ROPS_READ_DESTINATION, or 0
BOOL PyWinObject_AsDOCINFO(PyObject *obdocinfo, DOCINFO *di)
{
	if (!PyTuple_Check(obdocinfo)){
		PyErr_SetString(PyExc_TypeError,"DOCINFO must be a tuple");
		return FALSE;
		}
	di->cbSize=sizeof(DOCINFO);
	if (!PyArg_ParseTuple(obdocinfo, "zzzl", &di->lpszDocName, &di->lpszOutput, &di->lpszDatatype, &di->fwType))
		return FALSE;
	return TRUE;
}

// @pymethod int|win32print|StartDoc|Starts spooling a print job on a printer device context
static PyObject *PyStartDoc(PyObject *self, PyObject *args)
{
	// @pyparm int|hdc||Printer device context handle as returned by <om win32gui.CreateDC>
	// @pyparm tuple|docinfo||<o DOCINFO> tuple specifying print job parameters
	// @rdesc On success, returns the job id of the print job
	HDC hdc;
	DOCINFO docinfo;
	int jobid;
	PyObject *obdocinfo;
	if (!PyArg_ParseTuple(args, "lO:StartDoc", &hdc, &obdocinfo))
		return NULL;
	if (!PyWinObject_AsDOCINFO(obdocinfo, &docinfo))
		return NULL;
	jobid=StartDoc(hdc, &docinfo);
	if (jobid > 0)
		return Py_BuildValue("l",jobid);
	return PyWin_SetAPIError("StartDoc");
}

// @pymethod |win32print|EndDoc|Stops spooling a print job on a printer device context
static PyObject *PyEndDoc(PyObject *self, PyObject *args)
{
	// @pyparm int|hdc||Printer device context handle as returned by <om win32gui.CreateDC>
	HDC hdc;
	int err;
	if (!PyArg_ParseTuple(args, "l:EndDoc", &hdc))
		return NULL;
	err=EndDoc(hdc);
	if (err > 0){
		Py_INCREF(Py_None);
		return Py_None;
		}
	return PyWin_SetAPIError("EndDoc");
}

// @pymethod |win32print|AbortDoc|Cancels a print job
static PyObject *PyAbortDoc(PyObject *self, PyObject *args)
{
	// @pyparm int|hdc||Printer device context handle as returned by <om win32gui.CreateDC>
	HDC hdc;
	int err;
	if (!PyArg_ParseTuple(args, "l:AbortDoc", &hdc))
		return NULL;
	err=AbortDoc(hdc);
	if (err > 0){
		Py_INCREF(Py_None);
		return Py_None;
		}
	return PyWin_SetAPIError("AbortDoc");
}

// @pymethod |win32print|StartPage|Starts a page on a printer device context
static PyObject *PyStartPage(PyObject *self, PyObject *args)
{
	// @pyparm int|hdc||Printer device context handle as returned by <om win32gui.CreateDC>
	HDC hdc;
	int err;
	if (!PyArg_ParseTuple(args, "l:StartPage", &hdc))
		return NULL;
	err=StartPage(hdc);
	if (err > 0){
		Py_INCREF(Py_None);
		return Py_None;
		}
	return PyWin_SetAPIError("StartPage");
}

// @pymethod |win32print|EndPage|Ends a page on a printer device context
static PyObject *PyEndPage(PyObject *self, PyObject *args)
{
	// @pyparm int|hdc||Printer device context handle as returned by <om win32gui.CreateDC>
	HDC hdc;
	int err;
	if (!PyArg_ParseTuple(args, "l:EndPage", &hdc))
		return NULL;
	err=EndPage(hdc);
	if (err > 0){
		Py_INCREF(Py_None);
		return Py_None;
		}
	return PyWin_SetAPIError("EndPage");
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

// @pymethod int|win32print|DocumentProperties|Changes printer configuration for a printer
// @comm If DM_IN_PROMPT is specified, return value will be IDOK or IDCANCEL
static PyObject *PyDocumentProperties(PyObject *self, PyObject *args)
{
	long ret;
	HANDLE hprinter;
	HWND hwnd;
	char *devicename;
	PDEVMODE dmoutput, dminput;
	PyObject *obdmoutput, *obdminput;
	DWORD mode;
	// @pyparm int|HWnd||Parent window handle to use if DM_IN_PROMPT is specified to display printer dialog
	// @pyparm int|hPrinter||Printer handle as returned by OpenPrinter
	// @pyparm string|DeviceName||Name of printer
	// @pyparm <o PyDEVMODE>|DevModeOutput||PyDEVMODE object that receives modified info, can be None if DM_OUT_BUFFER not specified
	// @pyparm <o PyDEVMODE>|DevModeInput||PyDEVMODE that specifies initial configuration, can be None if DM_IN_BUFFER not specified
	// @pyparm int|Mode||A combination of DM_IN_BUFFER, DM_OUT_BUFFER, and DM_IN_PROMPT - pass 0 to retrieve driver data size
	if (!PyArg_ParseTuple(args,"llsOOl:DocumentProperties", &hwnd, &hprinter, &devicename, &obdmoutput, &obdminput, &mode))
		return NULL;
	if (!PyWinObject_AsDEVMODE(obdmoutput, &dmoutput, TRUE))
		return NULL;
	if (!PyWinObject_AsDEVMODE(obdminput, &dminput, TRUE))
		return NULL;
	ret=DocumentProperties(hwnd, hprinter, devicename, dmoutput, dminput, mode);
	if (ret < 0){
		PyWin_SetAPIError("DocumentProperties");
		return NULL;
		}
	if (obdmoutput!=Py_None)
		((PyDEVMODE *)obdmoutput)->modify_in_place();
	return PyInt_FromLong(ret);
}

// @pymethod (string,...)|win32print|EnumPrintProcessors|List printer processors for specified server and environment
static PyObject *PyEnumPrintProcessors(PyObject *self, PyObject *args)
{
	PRINTPROCESSOR_INFO_1 *info=NULL; // currently only level that exists
	LPBYTE buf=NULL;
	char *servername=NULL, *environment=NULL;
	DWORD level=1, bufsize=0, bytes_needed, return_cnt;
	PyObject *ret, *tuple_item;
	// @pyparm string|Server|None|Name of print server, use None for local machine
	// @pyparm string|Environment|None|Environment - eg 'Windows NT x86' - use None for current client environment
	if (!PyArg_ParseTuple(args,"|zz:EnumPrintProcessors", &servername, &environment))
		return NULL;

	EnumPrintProcessors(servername, environment, level, buf, bufsize, &bytes_needed, &return_cnt);
	if (bytes_needed==0){
		PyWin_SetAPIError("EnumPrintProcessors");
		return NULL;
		}
	buf=(LPBYTE)malloc(bytes_needed);
	if (buf==NULL){
		PyErr_Format(PyExc_MemoryError,"EnumPrintProcessors: unable to allocate buffer of size %d", bytes_needed);
		return NULL;
		}
	bufsize=bytes_needed;
	if (!EnumPrintProcessors(servername, environment, level, buf, bufsize, &bytes_needed, &return_cnt))
		PyWin_SetAPIError("EnumPrintProcessors");
	else{
		ret=PyTuple_New(return_cnt);
		if (ret!=NULL){
			info=(PRINTPROCESSOR_INFO_1 *)buf;
			for (DWORD buf_ind=0; buf_ind<return_cnt; buf_ind++){
				tuple_item=PyString_FromString(info->pName);
				if (tuple_item==NULL){
					Py_DECREF(ret);
					ret=NULL;
					break;
					}
				PyTuple_SetItem(ret,buf_ind,tuple_item);
				info++;
				}
			}
		}
	free(buf);
	return ret;
}

// @pymethod (<o PyUnicode>,...)|win32print|EnumPrintProcessorDatatypes|List data types that specified print provider recognizes
static PyObject *PyEnumPrintProcessorDatatypes(PyObject *self, PyObject *args)
{
	DATATYPES_INFO_1W *di1;
	LPBYTE buf=NULL;
	WCHAR *servername=NULL, *processorname=NULL;
	PyObject *observername, *obprocessorname;
	DWORD level=1, bufsize=0, bytes_needed, return_cnt, buf_ind;
	PyObject *ret=NULL, *tuple_item;
	// @pyparm string/<o PyUnicode>|ServerName||Name of print server, use None for local machine
	// @pyparm string/<o PyUnicode>|PrintProcessorName||Name of print processor
	if (!PyArg_ParseTuple(args,"OO:EnumPrintProcessorDatatypes", &observername, &obprocessorname))
		return NULL;
	if (!PyWinObject_AsWCHAR(observername, &servername, TRUE))
		goto done;
	if (!PyWinObject_AsWCHAR(obprocessorname, &processorname, FALSE))
		goto done;
	EnumPrintProcessorDatatypesW(servername, processorname, level, buf, bufsize, &bytes_needed, &return_cnt);
	if (bytes_needed==0){
		PyWin_SetAPIError("EnumPrintProcessorDatatypes");
		goto done;
		}
	buf=(LPBYTE)malloc(bytes_needed);
	if (buf==NULL){
		PyErr_Format(PyExc_MemoryError,"EnumPrintProcessorDatatypes: unable to allocate buffer of size %d", bytes_needed);
		goto done;
		}
	bufsize=bytes_needed;
	if (!EnumPrintProcessorDatatypesW(servername, processorname, level, buf, bufsize, &bytes_needed, &return_cnt)){
		PyWin_SetAPIError("EnumPrintProcessorDatatypes");
		goto done;
		}
	ret=PyTuple_New(return_cnt);
	if (ret==NULL)
		goto done;
	di1=(DATATYPES_INFO_1W *)buf;
	for (buf_ind=0; buf_ind<return_cnt; buf_ind++){
		tuple_item=PyWinObject_FromWCHAR(di1->pName);
		if (tuple_item==NULL){
			Py_DECREF(ret);
			ret=NULL;
			break;
			}
		PyTuple_SetItem(ret,buf_ind,tuple_item);
		di1++;
		}
done:
	if (servername!=NULL)
		PyWinObject_FreeWCHAR(servername);
	if (processorname!=NULL)
		PyWinObject_FreeWCHAR(processorname);
	if (buf!=NULL)
		free(buf);
	return ret;
}

// @pymethod (dict,...)|win32print|EnumPrinterDrivers|Lists installed printer drivers
static PyObject *PyEnumPrinterDrivers(PyObject *self, PyObject *args)
{
	DWORD level=1, bufsize=0, bytes_needed, return_cnt, i;
	LPBYTE buf=NULL;
	DRIVER_INFO_1W *di1;
	DRIVER_INFO_2W *di2;
	DRIVER_INFO_3W *di3;
	DRIVER_INFO_4W *di4;
	DRIVER_INFO_5W *di5;
	DRIVER_INFO_6W *di6;
	PyObject *ret=NULL, *tuple_item;
	PyObject *observername=Py_None, *obenvironment=Py_None;
	WCHAR *servername=NULL, *environment=NULL;
	// @pyparm string/unicode|Server|None|Name of print server, use None for local machine
	// @pyparm string/unicode|Environment|None|Environment - eg 'Windows NT x86' - use None for current client environment
	// @pyparm int|Level|1|Level of information to return, 1-6 (not all levels are supported on all platforms)
	// @rdesc Returns a sequence of dictionaries representing DRIVER_INFO_* structures
	// @comm On Win2k and up, 'all' can be passed for environment
	if (!PyArg_ParseTuple(args,"|OOl:EnumPrinterDrivers", &observername, &obenvironment, &level))
		return NULL;
	if (!PyWinObject_AsWCHAR(observername, &servername, TRUE))
		goto done;
	if (!PyWinObject_AsWCHAR(obenvironment, &environment, TRUE))
		goto done;

	EnumPrinterDriversW(servername, environment, level, buf, bufsize, &bytes_needed, &return_cnt);
	if (bytes_needed==0){
		PyWin_SetAPIError("EnumPrinterDrivers");
		goto done;
		}
	buf=(LPBYTE)malloc(bytes_needed);
	if (buf==NULL){
		PyErr_Format(PyExc_MemoryError,"EnumPrinterDrivers: unable to allocate buffer of size %d", bytes_needed);
		goto done;
		}
	bufsize=bytes_needed;
	if (!EnumPrinterDriversW(servername, environment, level, buf, bufsize, &bytes_needed, &return_cnt)){
		PyWin_SetAPIError("EnumPrintProcessors");
		goto done;
		}
	ret=PyTuple_New(return_cnt);
	if (ret==NULL)
		goto done;
	switch (level)
		case 1:{
			di1=(DRIVER_INFO_1W *)buf;
			for (i=0; i<return_cnt; i++){
				tuple_item=Py_BuildValue("{s:u}","Name",di1->pName);
				if (tuple_item==NULL){
					Py_DECREF(ret);
					ret=NULL;
					break;
					}
				PyTuple_SetItem(ret, i, tuple_item);
				di1++;
				}
			break;
		case 2:
			di2=(DRIVER_INFO_2W *)buf;
			for (i=0; i<return_cnt; i++){
				tuple_item=Py_BuildValue("{s:l,s:u,s:u,s:u,s:u,s:u}",
					"Version",di2->cVersion,
					"Name",di2->pName,
					"Environment",di2->pEnvironment,
					"DriverPath",di2->pDriverPath,
					"DataFile",di2->pDataFile,
					"ConfigFile",di2->pConfigFile);
				if (tuple_item==NULL){
					Py_DECREF(ret);
					ret=NULL;
					break;
					}
				PyTuple_SetItem(ret, i, tuple_item);
				di2++;
				}
			break;
		case 3:
			di3=(DRIVER_INFO_3W *)buf;
			for (i=0; i<return_cnt; i++){
				tuple_item=Py_BuildValue("{s:l,s:u,s:u,s:u,s:u,s:u,s:u,s:u,s:u,s:u}",
					"Version",di3->cVersion,
					"Name",di3->pName,
					"Environment",di3->pEnvironment,
					"DriverPath",di3->pDriverPath,
					"DataFile",di3->pDataFile,
					"ConfigFile",di3->pConfigFile,
					"HelpFile", di3->pHelpFile,
					"DependentFiles",di3->pDependentFiles,
					"MonitorName",di3->pMonitorName,
					"DefaultDataType",di3->pDefaultDataType);
				if (tuple_item==NULL){
					Py_DECREF(ret);
					ret=NULL;
					break;
					}
				PyTuple_SetItem(ret, i, tuple_item);
				di3++;
				}
			break;
		case 4:
			di4=(DRIVER_INFO_4W *)buf;
			for (i=0; i<return_cnt; i++){
				tuple_item=Py_BuildValue("{s:l,s:u,s:u,s:u,s:u,s:u,s:u,s:u,s:u,s:u,s:u}",
					"Version",di4->cVersion,
					"Name",di4->pName,
					"Environment",di4->pEnvironment,
					"DriverPath",di4->pDriverPath,
					"DataFile",di4->pDataFile,
					"ConfigFile",di4->pConfigFile,
					"HelpFile", di4->pHelpFile,
					"DependentFiles",di4->pDependentFiles,
					"MonitorName",di4->pMonitorName,
					"DefaultDataType",di4->pDefaultDataType,
					"PreviousNames",di4->pszzPreviousNames);
				if (tuple_item==NULL){
					Py_DECREF(ret);
					ret=NULL;
					break;
					}
				PyTuple_SetItem(ret, i, tuple_item);
				di4++;
				}
			break;
		case 5:
			di5=(DRIVER_INFO_5W *)buf;
			for (i=0; i<return_cnt; i++){
				tuple_item=Py_BuildValue("{s:l,s:u,s:u,s:u,s:u,s:u,s:l,s:l,s:l}",
					"Version",di5->cVersion,
					"Name",di5->pName,
					"Environment",di5->pEnvironment,
					"DriverPath",di5->pDriverPath,
					"DataFile",di5->pDataFile,
					"ConfigFile",di5->pConfigFile,
					"DriverAttributes", di5->dwDriverAttributes,
					"DriverVersion",di5->dwDriverVersion,
					"ConfigVersion",di5->dwConfigVersion);
				if (tuple_item==NULL){
					Py_DECREF(ret);
					ret=NULL;
					break;
					}
				PyTuple_SetItem(ret, i, tuple_item);
				di5++;
				}
			break;
		case 6:
			di6=(DRIVER_INFO_6W *)buf;
			for (i=0; i<return_cnt; i++){
				tuple_item=Py_BuildValue("{s:l,s:u,s:u,s:u,s:u,s:u,s:u,s:u,s:u,s:u,s:u,s:O&,s:L,s:u,s:u,s:u}",
					"Version",di6->cVersion,
					"Name",di6->pName,
					"Environment",di6->pEnvironment,
					"DriverPath",di6->pDriverPath,
					"DataFile",di6->pDataFile,
					"ConfigFile",di6->pConfigFile,
					"HelpFile", di6->pHelpFile,
					"DependentFiles",di6->pDependentFiles,
					"MonitorName",di6->pMonitorName,
					"DefaultDataType",di6->pDefaultDataType,
					"PreviousNames",di6->pszzPreviousNames,
					"DriverDate",PyWinObject_FromFILETIME,&di6->ftDriverDate,
					"DriverVersion",di6->dwlDriverVersion,
					"MfgName",di6->pszMfgName,
					"OEMUrl",di6->pszOEMUrl,
					"Provider",di6->pszProvider
					);
				if (tuple_item==NULL){
					Py_DECREF(ret);
					ret=NULL;
					break;
					}
				PyTuple_SetItem(ret, i, tuple_item);
				di6++;
				}
			break;
		default:
			PyErr_Format(PyExc_ValueError,"EnumPrinterDrivers: Level %d is not supported", level);
			Py_DECREF(ret);
			ret=NULL;
		}
done:
	if (buf!=NULL)
		free(buf);
	if (servername!=NULL)
		PyWinObject_FreeWCHAR(servername);
	if (environment!=NULL)
		PyWinObject_FreeWCHAR(environment);
	return ret;
}

PyObject *PyWin_Object_FromFORM_INFO_1(FORM_INFO_1W *fi1)
{
	if (fi1==NULL){
		Py_INCREF(Py_None);
		return Py_None;
		}
	return Py_BuildValue("{s:l,s:u,s:{s:l,s:l},s:{s:l,s:l,s:l,s:l}}",
		"Flags", fi1->Flags,
		"Name", fi1->pName,
		"Size", 
			"cx", fi1->Size.cx, "cy", fi1->Size.cy,
		"ImageableArea", 
			"left", fi1->ImageableArea.left, "top", fi1->ImageableArea.top,
			"right", fi1->ImageableArea.right, "bottom", fi1->ImageableArea.bottom);
}

// @pymethod (<o FORM_INFO_1>,...)|win32print|EnumForms|Lists forms for a printer
static PyObject *PyEnumForms(PyObject *self, PyObject *args)
{
	// @pyparm int|hprinter||Printer handle as returned by <om win32print.OpenPrinter>
	// @rdesc Returns a sequence of dictionaries representing FORM_INFO_1 structures
	PyObject *ret=NULL, *tuple_item;
	HANDLE hprinter;
	DWORD level=1, bufsize=0, bytes_needed=0, return_cnt, buf_ind;
	FORM_INFO_1W *fi1;
	LPBYTE buf=NULL;
	if (enumforms==NULL){
		PyErr_SetString(PyExc_NotImplementedError,"EnumForms does not exist on this version of Windows");
		return NULL;
		}
	if (!PyArg_ParseTuple(args,"l:EnumForms",&hprinter))
		return NULL;
	(*enumforms)(hprinter, level, buf, bufsize, &bytes_needed, &return_cnt);
	if (bytes_needed==0){
		PyWin_SetAPIError("EnumForms");
		goto done;
		}
	buf=(LPBYTE)malloc(bytes_needed);
	if (buf==NULL){
		PyErr_Format(PyExc_MemoryError,"EnumForms: unable to allocate buffer of size %d", bytes_needed);
		goto done;
		}
	bufsize=bytes_needed;
	if (!(*enumforms)(hprinter, level, buf, bufsize, &bytes_needed, &return_cnt)){
		PyWin_SetAPIError("EnumPrintProcessors");
		goto done;
		}
	ret=PyTuple_New(return_cnt);
	if (ret==NULL)
		goto done;
	fi1=(FORM_INFO_1W *)buf;
	for (buf_ind=0; buf_ind<return_cnt; buf_ind++){
		tuple_item=PyWin_Object_FromFORM_INFO_1(fi1);
		if (tuple_item==NULL){
			Py_DECREF(ret);
			ret=NULL;
			break;
			}
		PyTuple_SetItem(ret,buf_ind,tuple_item);
		fi1++;
		}
done:
	if (buf!=NULL)
		free(buf);
	return ret;
}

BOOL PyWinObject_AsRECTL(PyObject *obrectl, RECTL *rectl)
{
	static char *rectl_keys[]={"left","top","right","bottom",0};
	static char* err_msg="RECTL must be a dictionary containing {left:int, top:int, right:int, bottom:int}";
	if (obrectl->ob_type!=&PyDict_Type){
		PyErr_SetString(PyExc_TypeError,err_msg);
		return FALSE;
		}
	if (PyArg_ParseTupleAndKeywords(dummy_tuple, obrectl, "llll", rectl_keys,
		&rectl->left, &rectl->top, &rectl->right, &rectl->bottom))
		return TRUE;

	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, err_msg);
	return FALSE;

}

BOOL PyWinObject_AsSIZEL(PyObject *obsizel, SIZEL *sizel)
{
	static char *sizel_keys[]={"cx","cy",0};
	static char* err_msg="SIZEL must be a dictionary containing {cx:int, cy:int}";
	if (obsizel->ob_type!=&PyDict_Type){
		PyErr_SetString(PyExc_TypeError,err_msg);
		return FALSE;
		}
	if (PyArg_ParseTupleAndKeywords(dummy_tuple, obsizel, "ll", sizel_keys, &sizel->cx, &sizel->cy))
		return TRUE;

	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, err_msg);
	return FALSE;
}

// @object FORM_INFO_1|A dictionary containing FORM_INFO_1W data
// @prop int|Flags|FORM_USER, FORM_BUILTIN, or FORM_PRINTER
// @prop <o PyUnicode>|Name|Name of form
// @prop dict|Size|A dictionary representing a SIZEL structure {'cx':int,'cy':int}
// @prop dict|ImageableArea|A dictionary representing a RECTL structure {'left':int, 'top':int, 'right':int, 'bottom':int}

BOOL PyWinObject_AsFORM_INFO_1(PyObject *obform, FORM_INFO_1W *fi1)
{
	static char *form_keys[]={"Flags","Name","Size","ImageableArea",0};
	static char* err_msg="FORM_INFO_1 must be a dictionary containing {Flags:int, Name:unicode, Size:dict, ImageableArea:dict}";
	if (obform->ob_type!=&PyDict_Type){
		PyErr_SetString(PyExc_TypeError,err_msg);
		return FALSE;
		}
	return PyArg_ParseTupleAndKeywords(dummy_tuple, obform, "luO&O&:FORM_INFO_1", form_keys, &fi1->Flags, &fi1->pName, 
		PyWinObject_AsSIZEL, &fi1->Size, PyWinObject_AsRECTL, &fi1->ImageableArea);
}

// @pymethod |win32print|AddForm|Adds a form for a printer
static PyObject *PyAddForm(PyObject *self, PyObject *args)
{
	// @pyparm int|hprinter||Printer handle as returned by <om win32print.OpenPrinter>
	// @pyparm dict|Form||<o FORM_INFO_1> dictionary
	// @rdesc Returns None on success, throws an exception otherwise
	FORM_INFO_1W fi1;
	HANDLE hprinter;
	if (addform==NULL){
		PyErr_SetString(PyExc_NotImplementedError,"AddForm does not exist on this version of Windows");
		return NULL;
		}
	if (!PyArg_ParseTuple(args, "lO&:AddForm", &hprinter, PyWinObject_AsFORM_INFO_1, &fi1))
		return NULL;
	if (!(*addform)(hprinter, 1, (LPBYTE)&fi1))
		return PyWin_SetAPIError("AddForm");
	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod |win32print|DeleteForm|Deletes a form defined for a printer
static PyObject *PyDeleteForm(PyObject *self, PyObject *args)
{
	// @pyparm int|hprinter||Printer handle as returned by <om win32print.OpenPrinter>
	// @pyparm <o PyUnicode>|FormName||Name of form to be deleted
	// @rdesc Returns None on success, throws an exception otherwise
	HANDLE hprinter;
	WCHAR *formname;
	if (deleteform==NULL){
		PyErr_SetString(PyExc_NotImplementedError,"DeleteForm does not exist on this version of Windows");
		return NULL;
		}

	if (!PyArg_ParseTuple(args, "lu:DeleteForm", &hprinter, &formname))
		return NULL;
	if (!(*deleteform)(hprinter, formname))
		return PyWin_SetAPIError("DeleteForm");
	Py_INCREF(Py_None);
	return Py_None;
}
// @pymethod |win32print|GetForm|Retrieves information about a form defined for a printer
static PyObject *PyGetForm(PyObject *self, PyObject *args)
{
	// @pyparm int|hprinter||Printer handle as returned by <om win32print.OpenPrinter>
	// @pyparm <o PyUnicode>|FormName||Name of form for which to retrieve info
	// @rdesc Returns a <o FORM_INFO_1> dict
	HANDLE hprinter;
	WCHAR *formname;
	DWORD level=1, bufsize=0, bytes_needed=0;
	FORM_INFO_1W *fi1=NULL;
	LPBYTE buf=NULL;
	PyObject *ret=NULL;

	if (getform==NULL){
		PyErr_SetString(PyExc_NotImplementedError,"GetForm does not exist on this version of Windows");
		return NULL;
		}
	if (!PyArg_ParseTuple(args,"lu:GetForm", &hprinter, &formname))
		return NULL;
	(*getform)(hprinter, formname, level, buf, bufsize, &bytes_needed);
	if (bytes_needed==0)
		return PyWin_SetAPIError("GetForm");
	buf=(LPBYTE)malloc(bytes_needed);
	if (buf==NULL)
		return PyErr_Format(PyExc_MemoryError,"GetForm: Unable to allocate %d bytes",bytes_needed);
	bufsize=bytes_needed;
	if (!(*getform)(hprinter, formname, level, buf, bufsize, &bytes_needed))
		PyWin_SetAPIError("GetForm");
	else{
		fi1=(FORM_INFO_1W *)buf;
		ret=PyWin_Object_FromFORM_INFO_1(fi1);
		}
	free(buf);
	return ret;
}

// @pymethod |win32print|SetForm|Change information for a form
static PyObject *PySetForm(PyObject *self, PyObject *args)
{
	// @pyparm int|hprinter||Printer handle as returned by <om win32print.OpenPrinter>
	// @pyparm <o PyUnicode>|FormName||Name of form
	// @pyparm dict|Form||<o FORM_INFO_1> dictionary
	// @rdesc Returns None on success
	FORM_INFO_1W fi1;
	HANDLE hprinter;
	WCHAR *formname;
	if (setform==NULL){
		PyErr_SetString(PyExc_NotImplementedError,"SetForm does not exist on this version of Windows");
		return NULL;
		}
	if (!PyArg_ParseTuple(args, "luO&:SetForm", &hprinter, &formname, PyWinObject_AsFORM_INFO_1, &fi1))
		return NULL;
	if (!(*setform)(hprinter, formname, 1, (LPBYTE)&fi1))
		return PyWin_SetAPIError("SetForm");
	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod |win32print|AddJob|Add a job to be spooled to a printer queue
static PyObject *PyAddJob(PyObject *self, PyObject *args)
{
	// @rdesc Returns the file name to which data should be written and the job id of the new job
	// @pyparm int|hprinter||Printer handle as returned by <om win32print.OpenPrinter>
	HANDLE hprinter;
	DWORD level=1, bufsize, bytes_needed;
	LPBYTE buf=NULL;
	PyObject *ret=NULL;
	BOOL bsuccess;
	if (addjob==NULL){
		PyErr_SetString(PyExc_NotImplementedError,"AddJob does not exist on this version of Windows");
		return NULL;
		}

	if (!PyArg_ParseTuple(args,"l:AddJob", &hprinter))
		return NULL;
	bufsize=sizeof(ADDJOB_INFO_1)+ (MAX_PATH*sizeof(WCHAR));
	buf=(LPBYTE)malloc(bufsize);
	if (buf==NULL)
		return PyErr_Format(PyExc_MemoryError,"AddJob: unable to allocate %d bytes",bufsize);
	bsuccess=(*addjob)(hprinter, level, buf, bufsize, &bytes_needed);
	if (!bsuccess)
		if (bytes_needed > bufsize){
			free(buf);
			buf=(LPBYTE)malloc(bytes_needed);
			if (buf==NULL)
				return PyErr_Format(PyExc_MemoryError,"AddJob: unable to allocate %d bytes",bytes_needed);
			bufsize=bytes_needed;
			bsuccess=(*addjob)(hprinter, level, buf, bufsize, &bytes_needed);
			}
	if (!bsuccess)
		PyWin_SetAPIError("AddJob");
	else
		ret=Py_BuildValue("ul",((ADDJOB_INFO_1 *)buf)->Path,((ADDJOB_INFO_1 *)buf)->JobId);
	if (buf!=NULL)
		free(buf);
	return ret;
}

// @pymethod |win32print|ScheduleJob|Schedules a spooled job to be printed
static PyObject *PyScheduleJob(PyObject *self, PyObject *args)
{
	// @pyparm int|hprinter||Printer handle as returned by <om win32print.OpenPrinter>
	// @pyparm int|JobId||Job Id as returned by <om win32print.AddJob>
	HANDLE hprinter;
	DWORD jobid;
	if (schedulejob==NULL){
		PyErr_SetString(PyExc_NotImplementedError,"ScheduleJob does not exist on this version of Windows");
		return NULL;
		}

	if (!PyArg_ParseTuple(args,"ll:ScheduleJob", &hprinter, &jobid))
		return NULL;
	if (!(*schedulejob)(hprinter, jobid)){
		PyWin_SetAPIError("ScheduleJob");
		return NULL;
		}
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
	{"StartDocPrinter",			PyStartDocPrinter, 1},   // @pymeth StartDocPrinter|Notifies the print spooler that a document is to be spooled for printing. Returns the Jobid of the started job.
	{"EndDocPrinter",			PyEndDocPrinter, 1},   // @pymeth EndDocPrinter|The EndDocPrinter function ends a print job for the specified printer.
	{"AbortPrinter",			PyAbortPrinter, 1},   // @pymeth AbortPrinter|Deletes spool file for printer
	{"StartPagePrinter",		PyStartPagePrinter, 1}, // @pymeth StartPagePrinter|Notifies the print spooler that a page is to be printed on specified printer
	{"EndPagePrinter",			PyEndPagePrinter, 1}, // @pymeth EndPagePrinter|Ends a page in a print job
	{"StartDoc",     PyStartDoc, 1},   // @pymeth StartDoc|Starts spooling a print job on a printer device context
	{"EndDoc",     PyEndDoc, 1},   // @pymeth EndDoc|Stops spooling a print job on a printer device context
	{"AbortDoc",     PyAbortDoc, 1},   // @pymeth AbortDoc|Cancels print job on a printer device context
	{"StartPage",     PyStartPage, 1},   // @pymeth StartPage|Starts a page on a printer device context
	{"EndPage",     PyEndPage, 1},   // @pymeth EndPage|Ends a page on a printer device context
	{"WritePrinter",      PyWritePrinter, 1},   // @pymeth WritePrinter|Copies the specified bytes to the specified printer. StartDocPrinter and EndDocPrinter should be called before and after. Returns number of bytes written to printer.
	{"EnumJobs",        PyEnumJobs, 1},   // @pymeth EnumJobs|Enumerates print jobs on specified printer.
	{"GetJob",          PyGetJob, 1},   // @pymeth GetJob|Returns dictionary of information about a specified print job.
	{"SetJob",          PySetJob, 1},   // @pymeth SetJob|Pause, cancel, resume, set priority levels on a print job.
	{"DocumentProperties", PyDocumentProperties, 1}, //@pymeth DocumentProperties|Changes printer configuration
	{"EnumPrintProcessors", PyEnumPrintProcessors, 1}, //@pymeth EnumPrintProcessors|List printer providers for specified server and environment
	{"EnumPrintProcessorDatatypes", PyEnumPrintProcessorDatatypes, 1}, //@pymeth EnumPrintProcessorDatatypes|Lists data types that specified print provider supports
	{"EnumPrinterDrivers", PyEnumPrinterDrivers, 1}, //@pymeth EnumPrinterDrivers|Lists installed printer drivers
	{"EnumForms", PyEnumForms, 1}, //@pymeth EnumForms|Lists forms for a printer
	{"AddForm", PyAddForm, 1}, //@pymeth AddForm|Adds a form for a printer
	{"DeleteForm", PyDeleteForm, 1}, //@pymeth DeleteForm|Deletes a form defined for a printer
	{"GetForm", PyGetForm, 1}, //@pymeth GetForm|Retrieves information about a defined form
	{"SetForm", PySetForm, 1}, //@pymeth SetForm|Change information for a form
	{"AddJob", PyAddJob, 1}, //@pymeth AddJob|Adds a job to be spooled to a printer queue
	{"ScheduleJob", PyScheduleJob, 1}, //@pymeth ScheduleJob|Schedules a spooled job to be printed
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
  PyWinGlobals_Ensure();
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
  AddConstant(dict, "DI_APPBANDING", DI_APPBANDING);
  AddConstant(dict, "DI_ROPS_READ_DESTINATION", DI_ROPS_READ_DESTINATION);
  AddConstant(dict, "FORM_USER", FORM_USER);
  AddConstant(dict, "FORM_BUILTIN", FORM_BUILTIN);
  AddConstant(dict, "FORM_PRINTER", FORM_PRINTER);

  FARPROC fp;
  HMODULE hmodule=LoadLibrary("winspool.drv");
  if (hmodule!=NULL){
	fp=GetProcAddress(hmodule,"EnumFormsW");
	if (fp!=NULL)
		enumforms=(EnumFormsfunc)fp;
	fp=GetProcAddress(hmodule,"AddFormW");
	if (fp!=NULL)
		addform=(AddFormfunc)fp;
	fp=GetProcAddress(hmodule,"DeleteFormW");
	if (fp!=NULL)
		deleteform=(DeleteFormfunc)fp;
	fp=GetProcAddress(hmodule,"GetFormW");
	if (fp!=NULL)
		getform=(GetFormfunc)fp;
	fp=GetProcAddress(hmodule,"SetFormW");
	if (fp!=NULL)
		setform=(SetFormfunc)fp;
	fp=GetProcAddress(hmodule,"AddJobW");
	if (fp!=NULL)
		addjob=(AddJobfunc)fp;
	fp=GetProcAddress(hmodule,"ScheduleJob");
	if (fp!=NULL)
		schedulejob=(ScheduleJobfunc)fp;
  }
  dummy_tuple=PyTuple_New(0);
}
