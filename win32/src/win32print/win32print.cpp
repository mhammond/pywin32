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

#define CHECK_PFN(fname)if (pfn##fname==NULL) return PyErr_Format(PyExc_NotImplementedError,"%s is not available on this platform", #fname);

typedef BOOL (WINAPI *EnumFormsfunc)(HANDLE,DWORD,LPBYTE,DWORD,LPDWORD,LPDWORD);
static EnumFormsfunc pfnEnumForms=NULL;
typedef BOOL (WINAPI *AddFormfunc)(HANDLE,DWORD,LPBYTE);
static AddFormfunc pfnAddForm=NULL;
typedef BOOL (WINAPI *DeleteFormfunc)(HANDLE, LPWSTR);
static DeleteFormfunc pfnDeleteForm=NULL;
typedef BOOL (WINAPI *GetFormfunc)(HANDLE,LPWSTR,DWORD,LPBYTE,DWORD,LPDWORD);
static GetFormfunc pfnGetForm=NULL;
typedef BOOL (WINAPI *SetFormfunc)(HANDLE, LPWSTR, DWORD, LPBYTE);
static SetFormfunc pfnSetForm=NULL;
typedef BOOL (WINAPI *AddJobfunc)(HANDLE,DWORD,LPBYTE,DWORD,LPDWORD);
static AddJobfunc pfnAddJob=NULL;
typedef BOOL (WINAPI *ScheduleJobfunc)(HANDLE, DWORD);
static ScheduleJobfunc pfnScheduleJob=NULL;
typedef BOOL (WINAPI * EnumPortsfunc)(LPWSTR,DWORD,LPBYTE,DWORD,LPDWORD,LPDWORD);
static EnumPortsfunc pfnEnumPorts=NULL;
static EnumPortsfunc pfnEnumMonitors=NULL; // same args as EnumPorts
typedef BOOL (WINAPI *GetPrintProcessorDirectoryfunc)(LPWSTR,LPWSTR,DWORD,LPBYTE,DWORD,LPDWORD);
static GetPrintProcessorDirectoryfunc pfnGetPrintProcessorDirectory=NULL;
static GetPrintProcessorDirectoryfunc pfnGetPrinterDriverDirectory=NULL;  // same as GetPrintProcessorDirectory

static PyObject *dummy_tuple=NULL;

// @object PRINTER_DEFAULTS|A dictionary representing a PRINTER_DEFAULTS structure
// @prop string|pDatatype|Data type to be used for print jobs, see <om win32print.EnumPrintProcessorDatatypes>, can be None
// @prop <o PyDEVMODE>|pDevMode|A PyDEVMODE that specifies default printer parameters, can be None 
// @prop int|DesiredAccess|An ACCESS_MASK specifying what level of access is needed, eg PRINTER_ACCESS_ADMINISTER, PRINTER_ACCESS_USE 
BOOL PyWinObject_AsPRINTER_DEFAULTS(PyObject *obdefaults, PPRINTER_DEFAULTS pdefaults)
{
	static char *printer_default_keys[]={"pDataType","pDevMode","DesiredAccess",NULL};
	static char *printer_default_format="zOl";
	PyObject *obdevmode;
	if (!PyDict_Check(obdefaults)){
		PyErr_SetString(PyExc_TypeError, "PRINTER_DEFAULTS must be a dictionary");
		return FALSE;
		}
	ZeroMemory(pdefaults,sizeof(PRINTER_DEFAULTS));
	return PyArg_ParseTupleAndKeywords(dummy_tuple,obdefaults,printer_default_format,printer_default_keys,
		&pdefaults->pDatatype, &obdevmode, &pdefaults->DesiredAccess)
		&&PyWinObject_AsDEVMODE(obdevmode, &pdefaults->pDevMode, TRUE);
}
// Printer stuff.
// @pymethod int|win32print|OpenPrinter|Retrieves a handle to a printer.
static PyObject *PyOpenPrinter(PyObject *self, PyObject *args)
{
	char *printer;
	HANDLE handle;
	PRINTER_DEFAULTS printer_defaults;
	PRINTER_DEFAULTS *pprinter_defaults=NULL;
	PyObject *obdefaults=Py_None;
	if (!PyArg_ParseTuple(args, "s|O:OpenPrinter", 
		&printer,     // @pyparm string|printer||printer or print server name.
		&obdefaults)) // @pyparm dict|Defaults|None|<o PRINTER_DEFAULTS> dict, or None
		return NULL;
	if (obdefaults!=Py_None){
		if (!PyWinObject_AsPRINTER_DEFAULTS(obdefaults, &printer_defaults))
			return NULL;
		pprinter_defaults=&printer_defaults;
		}
	if (!OpenPrinter(printer, &handle, pprinter_defaults))
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

// @pymethod dict|win32print|GetPrinter|Retrieves information about a printer
// @rdesc Returns a dictionary containing PRINTER_INFO_* data for level, or
//  returns a tuple of PRINTER_INFO_2 data if no level is passed in.
static PyObject *PyGetPrinter(PyObject *self, PyObject *args)
{
	int handle;
	DWORD needed, level;
	BOOL backward_compat;
	LPBYTE buf=NULL;
	PyObject *rc=NULL;
	// @comm Original implementation used level 2 only and returned a tuple
	// Pass single arg as indicator to use old behaviour for backward compatibility
	if (PyArg_ParseTuple(args, "i:GetPrinter", 
		&handle)){ // @pyparm int|handle||handle to printer object as returned by <om win32print.OpenPrinter>
		backward_compat=TRUE;
		level=2;
		}
	else{
		PyErr_Clear();
		if (!PyArg_ParseTuple(args, "ii:GetPrinter", &handle, &level)) // @pyparm int|Level|2|Level of data returned (1,2,3,4,5,7,8,9)
			return NULL;
		backward_compat=FALSE;
		}
	// first allocate memory.
	GetPrinter((HANDLE)handle, level, NULL, 0, &needed );
	if (GetLastError()!=ERROR_INSUFFICIENT_BUFFER)
		return PyWin_SetAPIError("GetPrinter");
	buf=(LPBYTE)malloc(needed);
	if (buf==NULL)
		return PyErr_Format(PyExc_MemoryError,"GetPrinter: Unable to allocate buffer of %d bytes", needed);
	if (!GetPrinter((HANDLE)handle, level, buf, needed, &needed )) {
		free(buf);
		return PyWin_SetAPIError("GetPrinter");
	}
	switch (level){
		case 1:
			PRINTER_INFO_1 *pi1;
			pi1=(PRINTER_INFO_1 *)buf;
			rc=Py_BuildValue("{s:l,s:s,s:s,s:s}",
				"Flags",pi1->Flags, "pDescription",pi1->pDescription,
				"pName",pi1->pName, "pComment",pi1->pComment); 
			break;
		case 2:
			PRINTER_INFO_2 *pi2;
			pi2=(PRINTER_INFO_2 *)buf;
			if (backward_compat)
				rc = Py_BuildValue("ssssssszssssziiiiiiii",
					pi2->pServerName, pi2->pPrinterName, 	pi2->pShareName, pi2->pPortName,
					pi2->pDriverName, pi2->pComment, pi2->pLocation, NULL, pi2->pSepFile,
					pi2->pPrintProcessor, pi2->pDatatype, pi2->pParameters, NULL,
					pi2->Attributes, pi2->Priority, pi2->DefaultPriority, pi2->StartTime, pi2->UntilTime,
					pi2->Status, pi2->cJobs, pi2->AveragePPM);
			else
				rc = Py_BuildValue("{s:s,s:s,s:s,s:s,s:s,s:s,s:s,s:O&,s:s,s:s,s:s,s:s,s:O&,s:i,s:i,s:i,s:i,s:i,s:i,s:i,s:i}",
					"pServerName",pi2->pServerName, "pPrinterName",pi2->pPrinterName,
					"pShareName",pi2->pShareName, "pPortName",pi2->pPortName,
					"pDriverName",pi2->pDriverName, "pComment",pi2->pComment,
					"pLocation",pi2->pLocation, "pDevMode",PyWinObject_FromDEVMODE,pi2->pDevMode,
					"pSepFile", pi2->pSepFile, "pPrintProcessor",pi2->pPrintProcessor,
					"pDatatype",pi2->pDatatype, "pParameters",pi2->pParameters,
					"pSecurityDescriptor",PyWinObject_FromSECURITY_DESCRIPTOR,pi2->pSecurityDescriptor,
					"Attributes",pi2->Attributes, "Priority",pi2->Priority,
					"DefaultPriority",pi2->DefaultPriority,
					"StartTime",pi2->StartTime, "UntilTime",pi2->UntilTime,
					"Status",pi2->Status, "cJobs",pi2->cJobs, "AveragePPM",pi2->AveragePPM);
			break;
		case 3:
			PRINTER_INFO_3 *pi3;
			pi3=(PRINTER_INFO_3 *)buf;
			rc = Py_BuildValue("{s:O&}","pSecurityDescriptor",PyWinObject_FromSECURITY_DESCRIPTOR,pi3->pSecurityDescriptor);
			break;
		case 4:
			PRINTER_INFO_4 *pi4;
			pi4=(PRINTER_INFO_4 *)buf;
			rc = Py_BuildValue("{s:s,s:s,s:l}",
				"pPrinterName",pi4->pPrinterName,
				"pServerName",pi4->pServerName, 
				"Attributes",pi4->Attributes);
			break;
		case 5:
			PRINTER_INFO_5 *pi5;
			pi5=(PRINTER_INFO_5 *)buf;
			rc = Py_BuildValue("{s:s,s:s,s:l,s:l,s:l}",
				"pPrinterName",pi5->pPrinterName,
				"pPortName",pi5->pPortName,
				"Attributes",pi5->Attributes,
				"DeviceNotSelectedTimeout",pi5->DeviceNotSelectedTimeout,
				"TransmissionRetryTimeout",pi5->TransmissionRetryTimeout);
			break;
		case 7:
			PRINTER_INFO_7 *pi7;
			pi7=(PRINTER_INFO_7 *)buf;
			rc=Py_BuildValue("{s:s,s:l}","ObjectGUID",pi7->pszObjectGUID, "Action",pi7->dwAction);
			break;
		case 8:   // global printer defaults
			PRINTER_INFO_8 *pi8;
			pi8=(PRINTER_INFO_8 *)buf;
			rc=Py_BuildValue("{s:O&}","pDevMode", PyWinObject_FromDEVMODE, pi8->pDevMode);
			break;
		case 9:  // per user printer defaults
			PRINTER_INFO_9 *pi9;
			pi9=(PRINTER_INFO_9 *)buf;
			rc=Py_BuildValue("{s:O&}","pDevMode", PyWinObject_FromDEVMODE, pi9->pDevMode);
			break;
		default:
			PyErr_Format(PyExc_NotImplementedError,"Level %d is not supported",level);
		}
	free(buf);
	return rc;
}

BOOL PyWinObject_AsPRINTER_INFO(DWORD level, PyObject *obinfo, LPBYTE *pbuf)
{
	static char *pi2_keys[]={"pServerName","pPrinterName","pShareName","pPortName",
		"pDriverName","pComment","pLocation","pDevMode","pSepFile","pPrintProcessor",
		"pDatatype","pParameters","pSecurityDescriptor","Attributes","Priority",
		"DefaultPriority","StartTime","UntilTime","Status","cJobs","AveragePPM", NULL};
	static char *pi2_format="zzzzzzzOzzzzOllllllll:PRINTER_INFO_2";

	static char *pi3_keys[]={"pSecurityDescriptor", NULL};
	static char *pi3_format="O:PRINTER_INFO_3";

	static char *pi4_keys[]={"pPrinterName","pServerName","Attributes", NULL};
	static char *pi4_format="zzl:PRINTER_INFO_4";

	static char *pi5_keys[]={"pPrinterName","pPortName","Attributes",
		"DeviceNotSelectedTimeout","TransmissionRetryTimeout", NULL};
	static char *pi5_format="zzlll:PRINTER_INFO_5";

	static char *pi7_keys[]={"ObjectGUID","Action", NULL};
	static char *pi7_format="zl:PRINTER_INFO_7";
	
	static char *pi8_keys[]={"pDevMode", NULL};
	static char *pi8_format="O:PRINTER_INFO_8";

	PyObject *obdevmode, *obsecurity_descriptor;
	BOOL ret=FALSE;
	size_t bufsize;

	*pbuf=NULL;
	if (level==0)
		if (obinfo==Py_None)
			return TRUE;
		else{
			*pbuf = (LPBYTE)PyInt_AsLong(obinfo);
			if ((*pbuf==(LPBYTE)-1)&&PyErr_Occurred()){
				PyErr_Clear();
				PyErr_SetString(PyExc_TypeError,"Info must be None or a PRINTER_STATUS_* integer when level is 0.");
				return FALSE;
				}
			return TRUE;
			}

	if (!PyDict_Check (obinfo)){
		PyErr_Format(PyExc_TypeError, "PRINTER_INFO_%d must be a dictionary", level);
		return FALSE;
		}
	switch(level){
		case 2:
			PRINTER_INFO_2 *pi2;
			bufsize=sizeof(PRINTER_INFO_2);
			if (NULL == (*pbuf= (LPBYTE)malloc(bufsize))){
				PyErr_Format(PyExc_MemoryError, "Malloc failed for %d bytes", bufsize);
				break;
				}
			ZeroMemory(*pbuf,bufsize);
			pi2=(PRINTER_INFO_2 *)*pbuf;

			if (PyArg_ParseTupleAndKeywords(dummy_tuple, obinfo, pi2_format, pi2_keys,
				&pi2->pServerName, &pi2->pPrinterName, &pi2->pShareName, &pi2->pPortName,
				&pi2->pDriverName, &pi2->pComment, &pi2->pLocation, &obdevmode,
				&pi2->pSepFile, &pi2->pPrintProcessor, &pi2->pDatatype, &pi2->pParameters,
				&obsecurity_descriptor, &pi2->Attributes, &pi2->Priority, &pi2->DefaultPriority,
				&pi2->StartTime, &pi2->UntilTime, &pi2->Status, &pi2->cJobs, &pi2->AveragePPM)
				&&PyWinObject_AsDEVMODE(obdevmode, &pi2->pDevMode,FALSE)
				&&PyWinObject_AsSECURITY_DESCRIPTOR(obsecurity_descriptor, &pi2->pSecurityDescriptor, TRUE))
				ret=TRUE;
			break;
		case 3:
			PRINTER_INFO_3 *pi3;
			bufsize=sizeof(PRINTER_INFO_3);
			if (NULL == (*pbuf=(LPBYTE)malloc(bufsize))){
				PyErr_Format(PyExc_MemoryError, "Malloc failed for %d bytes", bufsize);
				break;
				}
			ZeroMemory(*pbuf,bufsize);
			pi3=(PRINTER_INFO_3 *)*pbuf;
			ret=PyArg_ParseTupleAndKeywords(dummy_tuple, obinfo, pi3_format, pi3_keys, &obsecurity_descriptor)
				&&PyWinObject_AsSECURITY_DESCRIPTOR(obsecurity_descriptor, &pi3->pSecurityDescriptor, FALSE);
			break;
		case 4:
			PRINTER_INFO_4 *pi4;
			bufsize=sizeof(PRINTER_INFO_4);
			if (NULL == (*pbuf=(LPBYTE)malloc(bufsize))){
				PyErr_Format(PyExc_MemoryError, "Malloc failed for %d bytes", bufsize);
				break;
				}
			ZeroMemory(*pbuf,bufsize);
			pi4=(PRINTER_INFO_4 *)*pbuf;
			ret=PyArg_ParseTupleAndKeywords(dummy_tuple, obinfo, pi4_format, pi4_keys,
				&pi4->pPrinterName, &pi4->pServerName, &pi4->Attributes);
			break;
		case 5:
			PRINTER_INFO_5 *pi5;
			bufsize=sizeof(PRINTER_INFO_5);
			if (NULL == (*pbuf=(LPBYTE)malloc(bufsize))){
				PyErr_Format(PyExc_MemoryError, "Malloc failed for %d bytes", bufsize);
				break;
				}
			ZeroMemory(*pbuf,bufsize);
			pi5=(PRINTER_INFO_5 *)*pbuf;
			ret=PyArg_ParseTupleAndKeywords(dummy_tuple, obinfo, pi5_format, pi5_keys,
				&pi5->pPrinterName, &pi5->pPortName, &pi5->Attributes,
				&pi5->DeviceNotSelectedTimeout, &pi5->TransmissionRetryTimeout);
			break;
		case 7:
			PRINTER_INFO_7 *pi7;
			bufsize=sizeof(PRINTER_INFO_7);
			if (NULL == (*pbuf=(LPBYTE)malloc(bufsize))){
				PyErr_Format(PyExc_MemoryError, "Malloc failed for %d bytes", bufsize);
				break;
				}
			ZeroMemory(*pbuf,bufsize);
			pi7=(PRINTER_INFO_7 *)*pbuf;
			ret=PyArg_ParseTupleAndKeywords(dummy_tuple, obinfo, pi7_format, pi7_keys,
				&pi7->pszObjectGUID, &pi7->dwAction);
			break;
		case 8:
		case 9:   //identical structs, 8 is for global defaults and 9 is for user defaults
			PRINTER_INFO_8 *pi8;
			bufsize=sizeof(PRINTER_INFO_8);
			if (NULL == (*pbuf=(LPBYTE)malloc(bufsize))){
				PyErr_Format(PyExc_MemoryError, "Malloc failed for %d bytes", bufsize);
				break;
				}
			ZeroMemory(*pbuf,bufsize);
			pi8=(PRINTER_INFO_8 *)*pbuf;
			ret=PyArg_ParseTupleAndKeywords(dummy_tuple, obinfo, pi8_format, pi8_keys,&obdevmode)
				&&PyWinObject_AsDEVMODE(obdevmode,&pi8->pDevMode,FALSE);
			break;
		default:
			PyErr_Format(PyExc_NotImplementedError,"Information level %d is not supported", level);
		}
	if (!ret){
		if ((*pbuf!=NULL) && (level!=0))
			free(*pbuf);
		*pbuf=NULL;
		}
	return ret;
}

// @pymethod |win32print|SetPrinter|Change printer configuration and status
static PyObject *PySetPrinter(PyObject *self, PyObject *args)
{
	HANDLE hprinter;
	LPBYTE buf=NULL;
	DWORD level, command;
	PyObject *obinfo=NULL, *ret=NULL;
	// @pyparm int|hPrinter||Printer handle as returned by <om win32print.OpenPrinter>
	// @pyparm int|Level||Level of data contained in pPrinter
	// @pyparm dict|pPrinter||PRINTER_INFO_* dict as returned by <om win32print.GetPrinter>, can be None if level is 0
	// @pyparm int|Command||Command to send to printer - one of the PRINTER_CONTROL_* constants, or 0
	// @comm If Level is 0 and Command is PRINTER_CONTROL_SET_STATUS, pPrinter should be an integer,
	// and is interpreted as the new printer status to set (one of the PRINTER_STATUS_* constants). 
	if (!PyArg_ParseTuple(args, "llOl:SetPrinter", 
		&hprinter, &level, &obinfo, &command))
		return NULL;
	if (!PyWinObject_AsPRINTER_INFO(level, obinfo, &buf))
		return NULL;
	if (!SetPrinter(hprinter, level, buf, command))
		PyWin_SetAPIError("SetPrinter");
	else{
		Py_INCREF(Py_None);
		ret=Py_None;
		}
	if ((level!=0)&&(buf!=NULL))
		free(buf);
	return ret;
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
	JOB_INFO_1 *job1;
	JOB_INFO_2 *job2;
	JOB_INFO_3 *job3;
	SYSTEMTIME localSubmitted;
	PyObject *pylocalsubmitted, *ret;
	switch (level){
		case 1:{
			job1= (JOB_INFO_1 *)buf;
			SystemTimeToTzSpecificLocalTime(NULL, &(job1->Submitted), &localSubmitted);
			pylocalsubmitted= new PyTime(localSubmitted);
			ret= Py_BuildValue("{s:i, s:s, s:s, s:s, s:s, s:s, s:s, s:i, s:i, s:i, s:i, s:i, s:O}",
					"JobId", job1->JobId,
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
		case 2:{
			job2=(JOB_INFO_2 *)buf;
			SystemTimeToTzSpecificLocalTime(NULL, &(job2->Submitted), &localSubmitted);
			pylocalsubmitted= new PyTime(localSubmitted);
			ret= Py_BuildValue("{s:i, s:s, s:s, s:s, s:s, s:s, s:s, s:s, s:s, s:s, s:O&, s:s, s:O&, s:i, s:i, s:i, s:i, s:i, s:i, s:i, s:O, s:i, s:i}",
					"JobId", job2->JobId,
					"pPrinterName", job2->pPrinterName,
					"pMachineName", job2->pMachineName,
					"pUserName", job2->pUserName,
					"pDocument", job2->pDocument,
					"pNotifyName", job2->pNotifyName,
					"pDatatype", job2->pDatatype,
					"pPrintProcessor", job2->pPrintProcessor,
					"pParameters", job2->pParameters,
					"pDriverName", job2->pDriverName,
					"pDevMode", PyWinObject_FromDEVMODE, job2->pDevMode,
					"pStatus", job2->pStatus,
					"pSecurityDescriptor", PyWinObject_FromSECURITY_DESCRIPTOR, job2->pSecurityDescriptor,
					"Status", job2->Status,
					"Priority", job2->Priority,
					"Position", job2->Position,
					"StartTime", job2->StartTime,
					"UntilTime", job2->UntilTime,
					"TotalPages", job2->TotalPages,
					"Size", job2->Size,
					"Submitted", pylocalsubmitted,
					"Time", job2->Time,
					"PagesPrinted", job2->PagesPrinted);
			Py_XDECREF(pylocalsubmitted);
			return ret;
			}
	   	case 3:{
			job3=(JOB_INFO_3 *)buf;
			ret=Py_BuildValue("{s:l, s:l, s:l}",
				"JobId", job3->JobId,
				"NextJobId",job3->NextJobId,
				"Reserved",job3->Reserved);
			return ret;
			}
		default:
			return PyErr_Format(PyExc_NotImplementedError,"Job info level %d is not yet supported", level);
		}
}

// @pymethod tuple|win32print|EnumJobs|Enumerates print jobs on specified printer.
// @rdesc Returns a sequence of dictionaries representing JOB_INFO_* structures, depending on level
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
	size_t job_info_offset[]={sizeof(JOB_INFO_1),sizeof(JOB_INFO_2),sizeof(JOB_INFO_3)};
	if (!PyArg_ParseTuple(args, "iii|i:EnumJobs",
	          &hprinter,   // @pyparm int|hPrinter||Handle of printer.
	          &firstjob,   // @pyparm int|FirstJob||location of first job in print queue to enumerate.
	          &nojobs,     // @pyparm int|NoJobs||Number of jobs to enumerate.
	          &level       // @pyparm int|Level|1|Level of information to return (JOB_INFO_1, JOB_INFO_2, JOB_INFO_3 supported).
	          ))
		return NULL;
	if ((level < 1)||(level > 3))
		return PyErr_Format(PyExc_ValueError, "Information level %d is not supported", level);
	EnumJobs(hprinter, firstjob, nojobs, level, NULL, 0, &bufneeded_size, &jobsreturned);
	if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
		return PyWin_SetAPIError("EnumJobs");
	buf_size= bufneeded_size;
	if (NULL == (buf= (LPBYTE)malloc(buf_size)))
		return PyErr_Format(PyExc_MemoryError, "Malloc failed for %d bytes", buf_size);
	if (!EnumJobs(hprinter, firstjob, nojobs, level, buf, buf_size, &bufneeded_size, &jobsreturned))
	{
		free(buf);
		return PyWin_SetAPIError("EnumJobs");
	}

	DWORD i;
	PyObject *job_info;
	PyObject *ret = PyTuple_New(jobsreturned);
	if (ret!=NULL)
		for (i= 0; i < jobsreturned; i++)
		{
			job_info=JobtoPy(level, (buf + i * job_info_offset[level-1]));
			if (job_info == NULL){
				Py_DECREF(ret);
				ret=NULL;
				break;
			}
			PyTuple_SetItem(ret, i, job_info);
		}
	free(buf);
	return ret;
}


// @pymethod dictionary|win32print|GetJob|Returns dictionary of information about a specified print job.
// @rdesc Returns a dict representing a JOB_INFO_* struct, depending on level
static PyObject *PyGetJob(PyObject *self, PyObject *args)
{
	HANDLE hprinter;
	DWORD jobid;
	DWORD level= 1;
	LPBYTE buf;
	DWORD buf_size;
	DWORD bufneeded_size;

	if (!PyArg_ParseTuple(args, "ii|i:GetJob",
	          &hprinter,// @pyparm int|hPrinter||Handle of printer.
	          &jobid,   // @pyparm int|JobID||Job Identifier.
	          &level   // @pyparm int|Level|1|Level of information to return (JOB_INFO_1, JOB_INFO_2, JOB_INFO_3 supported).
	          ))
		return NULL;
	if ((level < 1)||(level > 3))
		return PyErr_Format(PyExc_ValueError, "Information level %d is not supported", level);

	GetJob(hprinter, jobid, level, NULL, 0, &bufneeded_size);
	if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
		return PyWin_SetAPIError("GetJob");
	buf_size= bufneeded_size;
	if (NULL == (buf= (LPBYTE)malloc(buf_size)))
	{
		PyErr_SetString(PyExc_MemoryError, "Malloc failed.");
		return NULL;
	}
	if (!GetJob(hprinter, jobid, level, buf, buf_size, &bufneeded_size))
	{
		free(buf);
		return PyWin_SetAPIError("GetJob");
	}
	PyObject *ret= JobtoPy(level, buf);
	free(buf);
	return ret;
}


// Convert a python dictionary to a JOB_INFO_* structure.
// Returned buffer must be freed.
BOOL PytoJob(DWORD level, PyObject *pyjobinfo, LPBYTE *pbuf)
{
	static char *job1_keys[]={"JobId","pPrinterName","pMachineName","pUserName","pDocument","pDatatype",
		"pStatus","Status","Priority","Position","TotalPages","PagesPrinted","Submitted", NULL};
	static char *job1_format="lzzzzzzlllll|O:JOB_INFO_1";

	static char *job2_keys[]={"JobId","pPrinterName","pMachineName","pUserName","pDocument","pNotifyName",
		"pDatatype","pPrintProcessor","pParameters","pDriverName","pDevMode","pStatus","pSecurityDescriptor",
		"Status","Priority","Position","StartTime","UntilTime","TotalPages","Size",
		"Submitted","Time","PagesPrinted", NULL};
	static char *job2_format="lzzzzzzzzzOzOlllllllOll:JOB_INFO_2";

	static char *job3_keys[]={"JobId","NextJobId","Reserved", NULL};
	static char *job3_format="ll|l:JOB_INFO_3";

	PyObject *obdevmode, *obsecurity_descriptor, *obsubmitted=Py_None;
	BOOL ret=FALSE;

	*pbuf=NULL;
	switch(level){
		case 0:
			if (pyjobinfo==Py_None)
				ret=TRUE;
			else
				PyErr_SetString(PyExc_TypeError,"Info must be None when level is 0.");
			break;
		case 1:
			if (!PyDict_Check (pyjobinfo)){
				PyErr_SetString(PyExc_TypeError, "JOB_INFO_1 must be a dictionary");
				break;
				}
			JOB_INFO_1 *job1;
			if (NULL == (*pbuf= (LPBYTE)malloc(sizeof(JOB_INFO_1)))){
				PyErr_Format(PyExc_MemoryError, "Malloc failed for %d bytes", sizeof(JOB_INFO_1));
				break;
				}
			job1=(JOB_INFO_1 *)*pbuf;
			ZeroMemory(job1,sizeof(JOB_INFO_1));
			if (PyArg_ParseTupleAndKeywords(dummy_tuple, pyjobinfo, job1_format, job1_keys,
				&job1->JobId, &job1->pPrinterName, &job1->pMachineName, &job1->pUserName, &job1->pDocument,
				&job1->pDatatype, &job1->pStatus, &job1->Status, &job1->Priority, &job1->Position,
				&job1->TotalPages, &job1->PagesPrinted, &obsubmitted)
				&&((obsubmitted==Py_None)||PyWinObject_AsSYSTEMTIME(obsubmitted, &job1->Submitted)))
				ret=TRUE;
			break;
		case 2:
			if (!PyDict_Check (pyjobinfo)){
				PyErr_SetString(PyExc_TypeError, "JOB_INFO_2 must be a dictionary");
				break;
				}
			JOB_INFO_2 *job2;
			if (NULL == (*pbuf=(LPBYTE)malloc(sizeof(JOB_INFO_2)))){
				PyErr_Format(PyExc_MemoryError, "Malloc failed for %d bytes", sizeof(JOB_INFO_2));
				break;
				}
			job2=(JOB_INFO_2 *)*pbuf;
			ZeroMemory(job2,sizeof(JOB_INFO_2));
			if (PyArg_ParseTupleAndKeywords(dummy_tuple, pyjobinfo, job2_format, job2_keys,
					&job2->JobId, &job2->pPrinterName, &job2->pMachineName, &job2->pUserName, &job2->pDocument,
					&job2->pNotifyName, &job2->pDatatype, &job2->pPrintProcessor, &job2->pParameters,
					&job2->pDriverName, &obdevmode, &job2->pStatus, &obsecurity_descriptor, &job2->Status,
					&job2->Priority, &job2->Position, &job2->StartTime, &job2->UntilTime,
					&job2->TotalPages, &job2->Size, &obsubmitted, &job2->Time, &job2->PagesPrinted)
				&&PyWinObject_AsDEVMODE(obdevmode, &job2->pDevMode, TRUE)
				&&PyWinObject_AsSECURITY_DESCRIPTOR(obsecurity_descriptor, &job2->pSecurityDescriptor, TRUE)
				&&((obsubmitted==Py_None)||PyWinObject_AsSYSTEMTIME(obsubmitted, &job2->Submitted)))
				ret=TRUE;
			break;
		case 3:
			if (!PyDict_Check (pyjobinfo)){
				PyErr_SetString(PyExc_TypeError, "JOB_INFO_3 must be a dictionary");
				break;
				}
			JOB_INFO_3 *job3;
			if (NULL == (*pbuf=(LPBYTE)malloc(sizeof(JOB_INFO_3)))){
				PyErr_Format(PyExc_MemoryError, "Malloc failed for %d bytes", sizeof(JOB_INFO_3));
				break;
				}
			job3=(JOB_INFO_3 *)*pbuf;
			ZeroMemory(job3,sizeof(JOB_INFO_3));
			ret=PyArg_ParseTupleAndKeywords(dummy_tuple, pyjobinfo, job3_format, job3_keys,
				&job3->JobId, &job3->NextJobId, &job3->Reserved);
			break;
		default:
			PyErr_Format(PyExc_NotImplementedError,"Information level %d is not supported", level);
		}
	if (!ret)
		if (*pbuf!=NULL)
			free(*pbuf);
	return ret;
}


// @pymethod None|win32print|SetJob|Pause, cancel, resume, set priority levels on a print job.
// @comm If printer is not opened with at least PRINTER_ACCESS_ADMINISTER access, 'Position' member of
// JOB_INFO_1 and JOB_INFO_2 must be set to JOB_POSITION_UNSPECIFIED
static PyObject *PySetJob(PyObject *self, PyObject *args)
{
	HANDLE hprinter;
	DWORD jobid;
	DWORD level= 1;
	PyObject *pyjobinfo;
	DWORD command;
	LPBYTE buf;

	if (!PyArg_ParseTuple(args, "iiiOi:SetJob",
	    &hprinter,// @pyparm int|hPrinter||Handle of printer.
	    &jobid,   // @pyparm int|JobID||Job Identifier.
	    &level,   // @pyparm int|Level|1|Level of information in JobInfo dict (0, 1, 2, and 3 are supported).
	    &pyjobinfo, // @pyparm dict|JobInfo||JOB_INFO_* Dictionary as returned by <om win32print.GetJob> or <om win32print.EnumJobs> (can be None if Level is 0).
	    &command  // @pyparm int|Command||Job command value (JOB_CONTROL_*).
	    ))
		return NULL;
	if (!PytoJob(level, pyjobinfo, &buf))
		return NULL;

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

PyObject * PyWinObject_FromWCHARMultiple(WCHAR *multistring)
{
	PyObject *obelement, *ret=NULL;
	// takes a consecutive sequence of NULL terminated unicode strings,
	// terminated by an additional NULL and returns a list
	int elementlen;
	if (multistring==NULL){
		Py_INCREF(Py_None);
		return Py_None;
		}
	ret=PyList_New(0);
	if (ret==NULL)
		return NULL;
	elementlen=wcslen(multistring);
	do{
		obelement=PyWinObject_FromWCHAR(multistring, elementlen);
		if ((obelement==NULL)||(PyList_Append(ret,obelement)==-1)){
			Py_DECREF(ret);
			return NULL;
			}
		Py_DECREF(obelement);
		multistring+=elementlen+1;
		elementlen=wcslen(multistring);
		}
	while (elementlen>0);
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
				tuple_item=Py_BuildValue("{s:l,s:u,s:u,s:u,s:u,s:u,s:u,s:O&,s:u,s:u}",
					"Version",di3->cVersion,
					"Name",di3->pName,
					"Environment",di3->pEnvironment,
					"DriverPath",di3->pDriverPath,
					"DataFile",di3->pDataFile,
					"ConfigFile",di3->pConfigFile,
					"HelpFile", di3->pHelpFile,
					"DependentFiles",PyWinObject_FromWCHARMultiple,di3->pDependentFiles,
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
	CHECK_PFN(EnumForms);

	if (!PyArg_ParseTuple(args,"l:EnumForms",&hprinter))
		return NULL;
	(*pfnEnumForms)(hprinter, level, buf, bufsize, &bytes_needed, &return_cnt);
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
	if (!(*pfnEnumForms)(hprinter, level, buf, bufsize, &bytes_needed, &return_cnt)){
		PyWin_SetAPIError("EnumForms");
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
	CHECK_PFN(AddForm);

	if (!PyArg_ParseTuple(args, "lO&:AddForm", &hprinter, PyWinObject_AsFORM_INFO_1, &fi1))
		return NULL;
	if (!(*pfnAddForm)(hprinter, 1, (LPBYTE)&fi1))
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
	CHECK_PFN(DeleteForm);

	if (!PyArg_ParseTuple(args, "lu:DeleteForm", &hprinter, &formname))
		return NULL;
	if (!(*pfnDeleteForm)(hprinter, formname))
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
	CHECK_PFN(GetForm);

	if (!PyArg_ParseTuple(args,"lu:GetForm", &hprinter, &formname))
		return NULL;
	(*pfnGetForm)(hprinter, formname, level, buf, bufsize, &bytes_needed);
	if (bytes_needed==0)
		return PyWin_SetAPIError("GetForm");
	buf=(LPBYTE)malloc(bytes_needed);
	if (buf==NULL)
		return PyErr_Format(PyExc_MemoryError,"GetForm: Unable to allocate %d bytes",bytes_needed);
	bufsize=bytes_needed;
	if (!(*pfnGetForm)(hprinter, formname, level, buf, bufsize, &bytes_needed))
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
	CHECK_PFN(SetForm);

	if (!PyArg_ParseTuple(args, "luO&:SetForm", &hprinter, &formname, PyWinObject_AsFORM_INFO_1, &fi1))
		return NULL;
	if (!(*pfnSetForm)(hprinter, formname, 1, (LPBYTE)&fi1))
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
	CHECK_PFN(AddJob);

	if (!PyArg_ParseTuple(args,"l:AddJob", &hprinter))
		return NULL;
	bufsize=sizeof(ADDJOB_INFO_1)+ (MAX_PATH*sizeof(WCHAR));
	buf=(LPBYTE)malloc(bufsize);
	if (buf==NULL)
		return PyErr_Format(PyExc_MemoryError,"AddJob: unable to allocate %d bytes",bufsize);
	bsuccess=(*pfnAddJob)(hprinter, level, buf, bufsize, &bytes_needed);
	if (!bsuccess)
		if (bytes_needed > bufsize){
			free(buf);
			buf=(LPBYTE)malloc(bytes_needed);
			if (buf==NULL)
				return PyErr_Format(PyExc_MemoryError,"AddJob: unable to allocate %d bytes",bytes_needed);
			bufsize=bytes_needed;
			bsuccess=(*pfnAddJob)(hprinter, level, buf, bufsize, &bytes_needed);
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
	CHECK_PFN(ScheduleJob);

	if (!PyArg_ParseTuple(args,"ll:ScheduleJob", &hprinter, &jobid))
		return NULL;
	if (!(*pfnScheduleJob)(hprinter, jobid)){
		PyWin_SetAPIError("ScheduleJob");
		return NULL;
		}
	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod |win32print|DeviceCapabilities|Queries a printer for its capabilities
static PyObject *PyDeviceCapabilities(PyObject *self, PyObject *args)
{
	// @pyparm string|Device||Name of printer
	// @pyparm string|Port||Port that printer is using
	// @pyparm int|Capability||Type of capability to return - DC_* constant
	// @pyparm <o PyDEVMODE>|DEVMODE|None|If present, function returns values from it, otherwise the printer defaults are used
	char *device, *port;
	WORD capability;
	LPTSTR buf=NULL;
	PDEVMODE pdevmode;
	PyObject *obdevmode=Py_None, *ret=NULL, *tuple_item;
	DWORD result, bufsize, bufindex;
	static size_t papernamesize=64; // same for DC_PAPERNAMES, DC_MEDIATYPENAMES, DC_MEDIAREADY, DC_FILEDEPENDENCIES
	static size_t binnamesize=24; // DC_BINNAMES
	static size_t personalitysize=32; // DC_PERSONALITY
	size_t retsize;

	if (!PyArg_ParseTuple(args,"ssh|O:DeviceCapabilities", &device, &port, &capability, &obdevmode))
		return NULL;
	if (!PyWinObject_AsDEVMODE(obdevmode, &pdevmode, TRUE))
		return NULL;
	result=DeviceCapabilities(device,port,capability,buf,pdevmode);
	if (result==-1){
		PyWin_SetAPIError("DeviceCapabilities");
		return NULL;
		}
	// @flagh Capability|Returned value
	switch (capability){
		// none of these use the output pointer, just the returned DWORD
		case DC_BINADJUST:
		case DC_COLLATE:
		case DC_COPIES:
		case DC_COLORDEVICE:
		case DC_DUPLEX:
		case DC_DRIVER:
		case DC_EMF_COMPLIANT:
		case DC_EXTRA:
		case DC_FIELDS:
		case DC_NUP:
		case DC_ORIENTATION:
		case DC_PRINTRATE:
		case DC_PRINTRATEPPM:
		case DC_PRINTRATEUNIT:
		case DC_PRINTERMEM:
		case DC_SIZE:
		case DC_STAPLE:
		case DC_TRUETYPE:
		case DC_VERSION:
			ret=Py_BuildValue("l",result);
			break;
		// @flag DC_MINEXTENT|Dictionary containing minimum paper width and height
		// @flag DC_MAXEXTENT|Dictionary containing maximum paper width and height
		case DC_MINEXTENT:
		case DC_MAXEXTENT:
			ret=Py_BuildValue("{s:h,s:h}","Width",LOWORD(result),"Length",HIWORD(result));
			break;
		// @flag DC_ENUMRESOLUTIONS|Sequence of dictionaries containing x and y resolutions in DPI
		case DC_ENUMRESOLUTIONS:{	
			// output is pairs of LONGs, result indicates number of pairs
			PLONG presolutions;
			LONG xres, yres;
			bufsize=result*2*sizeof(LONG);
			buf=(LPTSTR)malloc(bufsize);
			if (buf==NULL){
				PyErr_Format(PyExc_MemoryError,"DeviceCapabilites: Unable to allocate %d bytes",bufsize);
				break;
				}
			result=DeviceCapabilities(device,port,capability,buf,pdevmode);
			if (result==-1)
				break;
			ret=PyTuple_New(result);
			if (ret==NULL)
				break;
			presolutions=(PLONG)buf;
			for (bufindex=0;bufindex<result;bufindex++){
				xres=*presolutions++;
				yres=*presolutions++;
				tuple_item=Py_BuildValue("{s:l,s:l}", "xdpi", xres, "ydpi", yres);
				if (tuple_item==NULL){
					Py_DECREF(ret);
					ret=NULL;
					break;
					}
				PyTuple_SET_ITEM(ret,bufindex,tuple_item);
				}
			break;
			}
		// @flag DC_PAPERS|Returns a sequence of ints, DMPAPER_* constants
		// @flag DC_BINS|Returns a sequence of ints, DMBIN_* constants
		case DC_PAPERS:
		case DC_BINS:{
			// output is an array of WORDs
			WORD *pword;
			retsize=sizeof(WORD);
			bufsize=result*retsize;
			buf=(LPTSTR)malloc(bufsize);
			if (buf==NULL){
				PyErr_Format(PyExc_MemoryError,"DeviceCapabilites: Unable to allocate %d bytes",bufsize);
				break;
				}
			result=DeviceCapabilities(device,port,capability,buf,pdevmode);
			if (result==-1)
				break;
			ret=PyTuple_New(result);
			if (ret==NULL)
				break;
			pword=(WORD *)buf;
			for (bufindex=0;bufindex<result;bufindex++){
				tuple_item=Py_BuildValue("h", *pword++);
				if (tuple_item==NULL){
					Py_DECREF(ret);
					ret=NULL;
					break;
					}
				PyTuple_SET_ITEM(ret,bufindex,tuple_item);
				}
			break;
			}
		// @flag DC_MEDIATYPES|Sequence of ints, DMMEDIA_* constants
		case DC_MEDIATYPES:{ 
			DWORD *pdword;
			retsize=sizeof(DWORD);
			bufsize=result*retsize;
			buf=(LPTSTR)malloc(bufsize);
			if (buf==NULL){
				PyErr_Format(PyExc_MemoryError,"DeviceCapabilites: Unable to allocate %d bytes",bufsize);
				break;
				}
			result=DeviceCapabilities(device,port,capability,buf,pdevmode);
			if (result==-1)
				break;
			ret=PyTuple_New(result);
			if (ret==NULL)
				break;
			pdword=(DWORD *)buf;
			for (bufindex=0;bufindex<result;bufindex++){
				tuple_item=Py_BuildValue("l", *pdword++);
				if (tuple_item==NULL){
					Py_DECREF(ret);
					ret=NULL;
					break;
					}
				PyTuple_SET_ITEM(ret,bufindex,tuple_item);
				}
			break;
			}
		// @flag DC_PAPERNAMES|Sequence of strings
		// @flag DC_MEDIATYPENAMES|Sequence of strings
		// @flag DC_MEDIAREADY|Sequence of strings
		// @flag DC_FILEDEPENDENCIES|Sequence of strings
		// @flag DC_PERSONALITY|Sequence of strings
		// @flag DC_BINNAMES|Sequence of strings
		case DC_PAPERNAMES:
		case DC_MEDIATYPENAMES:
		case DC_MEDIAREADY:
		case DC_FILEDEPENDENCIES: 	// first 4 return array of 64-char strings
		case DC_PERSONALITY:	// returns 32-char strings
		case DC_BINNAMES:{		// returns array of 24-char strings
			char *retname;
			if (capability==DC_BINNAMES)
				retsize=binnamesize;
			else if (capability==DC_PERSONALITY)
				retsize=personalitysize;
			else
				retsize=papernamesize;
			bufsize=result*retsize*sizeof(char);
			buf=(LPTSTR)malloc(bufsize);
			if (buf==NULL){
				PyErr_Format(PyExc_MemoryError,"DeviceCapabilites: Unable to allocate %d bytes",bufsize);
				break;
				}
			ZeroMemory(buf,bufsize);
			result=DeviceCapabilities(device,port,capability,buf,pdevmode);
			if (result==-1)
				break;
			ret=PyTuple_New(result);
			if (ret==NULL)
				break;
			retname=(char *)buf;
			for (bufindex=0;bufindex<result;bufindex++){
				if (*(retname+retsize-1)==0)
					tuple_item=PyString_FromString(retname);
				else  // won't be null-terminated if string occupies entire space
					tuple_item=PyString_FromStringAndSize(retname,retsize);
				if (tuple_item==NULL){
					Py_DECREF(ret);
					ret=NULL;
					break;
					}
				PyTuple_SET_ITEM(ret,bufindex,tuple_item);
				retname+=retsize;
				}
			break;
			}
		// @flag DC_PAPERSIZE|Sequence of dicts containing paper sizes, in 1/10 millimeter units
		// @flag All others|Output is an int
		case DC_PAPERSIZE:{
			// output is an array of POINTs
			POINT *ppoint;
			retsize=sizeof(POINT);
			bufsize=result*retsize;
			buf=(LPTSTR)malloc(bufsize);
			if (buf==NULL){
				PyErr_Format(PyExc_MemoryError,"DeviceCapabilites: Unable to allocate %d bytes",bufsize);
				break;
				}
			result=DeviceCapabilities(device,port,capability,buf,pdevmode);
			if (result==-1)
				break;
			ret=PyTuple_New(result);
			if (ret==NULL)
				break;
			ppoint=(POINT *)buf;
			for (bufindex=0;bufindex<result;bufindex++){
				tuple_item=Py_BuildValue("{s:l,s:l}", "x",ppoint->x, "y",ppoint->y);
				if (tuple_item==NULL){
					Py_DECREF(ret);
					ret=NULL;
					break;
					}
				PyTuple_SET_ITEM(ret,bufindex,tuple_item);
				ppoint++;
				}
			break;
			}
		// last 3 are 95/98/Me only
		case DC_DATATYPE_PRODUCED:
		case DC_MANUFACTURER:
		case DC_MODEL:
		default:
			PyErr_Format(PyExc_NotImplementedError,"Type %d is not supported", capability);
		}

	if (result==-1)
		PyWin_SetAPIError("DeviceCapabilities");
	if (buf!=NULL)
		free(buf);
	return ret;
}

// @pymethod (dict,...)|win32print|EnumMonitors|Lists installed printer port monitors
static PyObject *PyEnumMonitors(PyObject *self, PyObject *args)
{
	// @pyparm str/<o PyUnicode>|Name||Name of server, use None for local machine
	// @pyparm int|Level||Level of information to return, 1 and 2 supported
	// @rdesc Returns a sequence of dicts representing MONITOR_INFO_* structures depending on level
	PyObject *ret=NULL, *tuple_item, *observer_name;
	WCHAR *server_name=NULL;
	DWORD level, bufsize=0, bytes_needed=0, return_cnt, buf_ind;
	LPBYTE buf=NULL;
	CHECK_PFN(EnumMonitors);

	if (!PyArg_ParseTuple(args,"Ol:EnumMonitors", &observer_name, &level))
		return NULL;
	if (!PyWinObject_AsWCHAR(observer_name, &server_name, TRUE))
		return NULL;
	(*pfnEnumMonitors)(server_name, level, buf, bufsize, &bytes_needed, &return_cnt);
	if (bytes_needed==0){
		PyWin_SetAPIError("EnumMonitors");
		goto done;
		}
	buf=(LPBYTE)malloc(bytes_needed);
	if (buf==NULL){
		PyErr_Format(PyExc_MemoryError,"EnumMonitors: unable to allocate buffer of size %d", bytes_needed);
		goto done;
		}
	bufsize=bytes_needed;
	if (!(*pfnEnumMonitors)(server_name, level, buf, bufsize, &bytes_needed, &return_cnt)){
		PyWin_SetAPIError("EnumMonitors");
		goto done;
		}
	ret=PyTuple_New(return_cnt);
	if (ret==NULL)
		goto done;
	switch (level){
		case 1:{
			MONITOR_INFO_1W *mi1;
			mi1=(MONITOR_INFO_1W *)buf;
			for (buf_ind=0; buf_ind<return_cnt; buf_ind++){
				tuple_item=Py_BuildValue("{s:u}","Name",mi1->pName);
				if (tuple_item==NULL){
					Py_DECREF(ret);
					ret=NULL;
					break;
					}
				PyTuple_SetItem(ret,buf_ind,tuple_item);
				mi1++;
				}
			break;
			}
		case 2:{
			MONITOR_INFO_2W *mi2;
			mi2=(MONITOR_INFO_2W *)buf;
			for (buf_ind=0; buf_ind<return_cnt; buf_ind++){
				tuple_item=Py_BuildValue("{s:u,s:u,s:u}", "Name",mi2->pName,
					"Environment",mi2->pEnvironment, "DLLName",mi2->pDLLName);
				if (tuple_item==NULL){
					Py_DECREF(ret);
					ret=NULL;
					break;
					}
				PyTuple_SetItem(ret,buf_ind,tuple_item);
				mi2++;
				}
			break;
			}
		default:
			PyErr_Format(PyExc_NotImplementedError,"EnumMonitors: Level %d is not supported", level);
		}
done:
	if (server_name!=NULL)
		PyWinObject_FreeWCHAR(server_name);
	if (buf!=NULL)
		free(buf);
	return ret;
}

// @pymethod (dict,...)|win32print|EnumPorts|Lists printer port on a server
static PyObject *PyEnumPorts(PyObject *self, PyObject *args)
{
	// @pyparm str/<o PyUnicode>|Name||Name of server, use None for local machine
	// @pyparm int|Level||Level of information to return, 1 and 2 supported
	// @rdesc Returns a sequence of dicts representing PORT_INFO_* structures depending on level
	PyObject *ret=NULL, *tuple_item, *observer_name;
	WCHAR *server_name=NULL;
	DWORD level, bufsize=0, bytes_needed=0, return_cnt, buf_ind;
	LPBYTE buf=NULL;
	CHECK_PFN(EnumPorts);

	if (!PyArg_ParseTuple(args,"Ol:EnumPorts", &observer_name, &level))
		return NULL;
	if (!PyWinObject_AsWCHAR(observer_name, &server_name, TRUE))
		return NULL;
	(*pfnEnumPorts)(server_name, level, buf, bufsize, &bytes_needed, &return_cnt);
	if (bytes_needed==0){
		PyWin_SetAPIError("EnumPorts");
		goto done;
		}
	buf=(LPBYTE)malloc(bytes_needed);
	if (buf==NULL){
		PyErr_Format(PyExc_MemoryError,"EnumPorts: unable to allocate buffer of size %d", bytes_needed);
		goto done;
		}
	bufsize=bytes_needed;
	if (!(*pfnEnumPorts)(server_name, level, buf, bufsize, &bytes_needed, &return_cnt)){
		PyWin_SetAPIError("EnumPorts");
		goto done;
		}
	ret=PyTuple_New(return_cnt);
	if (ret==NULL)
		goto done;
	switch (level){
		case 1:{
			PORT_INFO_1W *pi1;
			pi1=(PORT_INFO_1W *)buf;
			for (buf_ind=0; buf_ind<return_cnt; buf_ind++){
				tuple_item=Py_BuildValue("{s:u}","Name",pi1->pName);
				if (tuple_item==NULL){
					Py_DECREF(ret);
					ret=NULL;
					break;
					}
				PyTuple_SetItem(ret,buf_ind,tuple_item);
				pi1++;
				}
			break;
			}
		case 2:{
			PORT_INFO_2W *pi2;
			pi2=(PORT_INFO_2W *)buf;
			for (buf_ind=0; buf_ind<return_cnt; buf_ind++){
				tuple_item=Py_BuildValue("{s:u,s:u,s:u,s:l,s:l}", "Name",pi2->pPortName,
					"MonitorName",pi2->pMonitorName, "Description",pi2->pDescription,
					"PortType",pi2->fPortType, "Reserved",pi2->Reserved);
				if (tuple_item==NULL){
					Py_DECREF(ret);
					ret=NULL;
					break;
					}
				PyTuple_SetItem(ret,buf_ind,tuple_item);
				pi2++;
				}
			break;
			}
		default:
			PyErr_Format(PyExc_NotImplementedError,"EnumPorts: Level %d is not supported", level);
		}
done:
	if (server_name!=NULL)
		PyWinObject_FreeWCHAR(server_name);
	if (buf!=NULL)
		free(buf);
	return ret;
}

// @pymethod <o PyUnicode>|win32print|GetPrintProcessorDirectory|Returns the directory where print processor files reside
static PyObject *PyGetPrintProcessorDirectory(PyObject *self, PyObject *args)
{
	// @pyparm str/<o PyUnicode>|Name||Name of server, use None for local machine
	// @pyparm str/<o PyUnicode>|Environment||Environment - eg 'Windows NT x86' - use None for current client environment
	PyObject *ret=NULL, *observer_name=Py_None, *obenvironment=Py_None;
	WCHAR *server_name=NULL, *environment=NULL;
	DWORD level=1, bufsize=0, bytes_needed=0, bytes_returned=0;
	LPBYTE buf=NULL;
	CHECK_PFN(GetPrintProcessorDirectory);

	if (!PyArg_ParseTuple(args,"|OO:GetPrintProcessorDirectory", &observer_name, &obenvironment))
		return NULL;
	if (!PyWinObject_AsWCHAR(observer_name, &server_name, TRUE))
		return NULL;
	if (!PyWinObject_AsWCHAR(obenvironment, &environment, TRUE))
		return NULL;

	(*pfnGetPrintProcessorDirectory)(server_name, environment, level, buf, bufsize, &bytes_needed);
	if (bytes_needed==0){
		PyWin_SetAPIError("GetPrintProcessorDirectory");
		goto done;
		}
	buf=(LPBYTE)malloc(bytes_needed);
	if (buf==NULL){
		PyErr_Format(PyExc_MemoryError,"GetPrintProcessorDirectory: unable to allocate buffer of size %d", bytes_needed);
		goto done;
		}
	bufsize=bytes_needed;
	if (!(*pfnGetPrintProcessorDirectory)(server_name, environment, level, buf, bufsize, &bytes_needed))
		PyWin_SetAPIError("GetPrintProcessorDirectory");
	else
		ret=PyWinObject_FromWCHAR((WCHAR *)buf);
done:
	if (server_name!=NULL)
		PyWinObject_FreeWCHAR(server_name);
	if (environment!=NULL)
		PyWinObject_FreeWCHAR(environment);
	if (buf!=NULL)
		free(buf);
	return ret;
}

// @pymethod <o PyUnicode>|win32print|GetPrinterDriverDirectory|Returns the directory where printer drivers are installed
static PyObject *PyGetPrinterDriverDirectory(PyObject *self, PyObject *args)
{
	// @pyparm str/<o PyUnicode>|Name||Name of server, use None for local machine
	// @pyparm str/<o PyUnicode>|Environment||Environment - eg 'Windows NT x86' - use None for current client environment
	PyObject *ret=NULL, *observer_name=Py_None, *obenvironment=Py_None;
	WCHAR *server_name=NULL, *environment=NULL;
	DWORD level=1, bufsize=0, bytes_needed=0, bytes_returned=0;
	LPBYTE buf=NULL;
	CHECK_PFN(GetPrinterDriverDirectory);

	if (!PyArg_ParseTuple(args,"|OO:GetPrinterDriverDirectory", &observer_name, &obenvironment))
		return NULL;
	if (!PyWinObject_AsWCHAR(observer_name, &server_name, TRUE))
		return NULL;
	if (!PyWinObject_AsWCHAR(obenvironment, &environment, TRUE))
		return NULL;

	(*pfnGetPrinterDriverDirectory)(server_name, environment, level, buf, bufsize, &bytes_needed);
	if (bytes_needed==0){
		PyWin_SetAPIError("GetPrinterDriverDirectory");
		goto done;
		}
	buf=(LPBYTE)malloc(bytes_needed);
	if (buf==NULL){
		PyErr_Format(PyExc_MemoryError,"GetPrinterDriverDirectory: unable to allocate buffer of size %d", bytes_needed);
		goto done;
		}
	bufsize=bytes_needed;
	if (!(*pfnGetPrinterDriverDirectory)(server_name, environment, level, buf, bufsize, &bytes_needed))
		PyWin_SetAPIError("GetPrinterDriverDirectory");
	else
		ret=PyWinObject_FromWCHAR((WCHAR *)buf);
done:
	if (server_name!=NULL)
		PyWinObject_FreeWCHAR(server_name);
	if (environment!=NULL)
		PyWinObject_FreeWCHAR(environment);
	if (buf!=NULL)
		free(buf);
	return ret;
}

// @pymethod int|win32print|AddPrinter|Installs a printer on a server
// @rdesc Returns a handle to the new printer
static PyObject *PyAddPrinter(PyObject *self, PyObject *args)
{
	HANDLE hprinter;
	LPBYTE buf=NULL;
	DWORD level;
	PyObject *obinfo;
	char *server_name;
	// @pyparm string|Name||Name of server on which to install printer, None indicates local machine
	// @pyparm int|Level||Level of data contained in pPrinter, only level 2 currently supported
	// @pyparm dict|pPrinter||PRINTER_INFO_2 dict as returned by <om win32print.GetPrinter>
	// @comm pPrinterName, pPortName, pDriverName, and pPrintProcessor are required
	if (!PyArg_ParseTuple(args, "zlO:AddPrinter", &server_name, &level, &obinfo))
		return NULL;
	if (level!=2){
		PyErr_SetString(PyExc_ValueError,"AddPrinter only accepts level 2");
		return NULL;
		}
	if (!PyWinObject_AsPRINTER_INFO(level, obinfo, &buf))
		return NULL;
	hprinter=AddPrinter(server_name, level, buf);
	if (buf!=NULL)
		free(buf);
	if (hprinter==NULL){
		PyWin_SetAPIError("AddPrinter");
		return NULL;
		}
	return Py_BuildValue("l",hprinter);
}

// @pymethod |win32print|DeletePrinter|Deletes an existing printer
// @comm Printer handle must be opened for PRINTER_ACCESS_ADMINISTER
// If there are any pending print jobs for the printer, actual deletion does not happen until they are done
static PyObject *PyDeletePrinter(PyObject *self, PyObject *args)
{
	// @pyparm int|hPrinter||Handle to printer as returned by <om win32print.OpenPrinter> or <om win32print.AddPrinter>
	HANDLE hprinter;
	if (!PyArg_ParseTuple(args, "l:DeletePrinter", &hprinter))
		return NULL;
	if (!DeletePrinter(hprinter)){
		PyWin_SetAPIError("DeletePrinter");
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
	{"SetPrinter",				PySetPrinter, 1}, // @pymeth SetPrinter|Changes printer configuration and status
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
	{"DeviceCapabilities", PyDeviceCapabilities, 1}, //@pymeth DeviceCapabilities|Queries a printer for its capabilities
	{"EnumMonitors", PyEnumMonitors, 1}, //@pymeth EnumMonitors|Lists installed printer port monitors
	{"EnumPorts", PyEnumPorts, 1}, //@pymeth EnumPorts|Lists printer ports on a server
	{"GetPrintProcessorDirectory", PyGetPrintProcessorDirectory, 1}, //@pymeth GetPrintProcessorDirectory|Returns the directory where print processor files reside
	{"GetPrinterDriverDirectory", PyGetPrinterDriverDirectory, 1}, //@pymeth GetPrinterDriverDirectory|Returns the directory where printer drivers are installed
	{"AddPrinter", PyAddPrinter, 1}, //@pymeth AddPrinter|Adds a new printer on a server
	{"DeletePrinter", PyDeletePrinter, 1}, //@pymeth DeletePrinter|Deletes an existing printer
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

  // Printer, print server, and print job access rights
  AddConstant(dict, "SERVER_ACCESS_ADMINISTER",SERVER_ACCESS_ADMINISTER);
  AddConstant(dict, "SERVER_ACCESS_ENUMERATE",SERVER_ACCESS_ENUMERATE);
  AddConstant(dict, "PRINTER_ACCESS_ADMINISTER",PRINTER_ACCESS_ADMINISTER);
  AddConstant(dict, "PRINTER_ACCESS_USE",PRINTER_ACCESS_USE);
  AddConstant(dict, "JOB_ACCESS_ADMINISTER",JOB_ACCESS_ADMINISTER);
  AddConstant(dict, "JOB_ACCESS_READ",JOB_ACCESS_READ);
  AddConstant(dict, "SERVER_ALL_ACCESS",SERVER_ALL_ACCESS);
  AddConstant(dict, "SERVER_READ",SERVER_READ);
  AddConstant(dict, "SERVER_WRITE",SERVER_WRITE);
  AddConstant(dict, "SERVER_EXECUTE",SERVER_EXECUTE);
  AddConstant(dict, "PRINTER_ALL_ACCESS",PRINTER_ALL_ACCESS);
  AddConstant(dict, "PRINTER_READ",PRINTER_READ);
  AddConstant(dict, "PRINTER_WRITE",PRINTER_WRITE);
  AddConstant(dict, "PRINTER_EXECUTE",PRINTER_EXECUTE);
  AddConstant(dict, "JOB_ALL_ACCESS",JOB_ALL_ACCESS);
  AddConstant(dict, "JOB_READ",JOB_READ);
  AddConstant(dict, "JOB_WRITE",JOB_WRITE);
  AddConstant(dict, "JOB_EXECUTE",JOB_EXECUTE);

  // Command values for SetPrinter
  AddConstant(dict, "PRINTER_CONTROL_PAUSE",PRINTER_CONTROL_PAUSE);
  AddConstant(dict, "PRINTER_CONTROL_PURGE",PRINTER_CONTROL_PURGE);
  AddConstant(dict, "PRINTER_CONTROL_SET_STATUS",PRINTER_CONTROL_SET_STATUS);
  AddConstant(dict, "PRINTER_CONTROL_RESUME",PRINTER_CONTROL_RESUME);

  // printer status constants
  AddConstant(dict, "PRINTER_STATUS_PAUSED",PRINTER_STATUS_PAUSED);
  AddConstant(dict, "PRINTER_STATUS_ERROR",PRINTER_STATUS_ERROR);
  AddConstant(dict, "PRINTER_STATUS_PENDING_DELETION",PRINTER_STATUS_PENDING_DELETION);
  AddConstant(dict, "PRINTER_STATUS_PAPER_JAM",PRINTER_STATUS_PAPER_JAM);
  AddConstant(dict, "PRINTER_STATUS_PAPER_OUT",PRINTER_STATUS_PAPER_OUT);
  AddConstant(dict, "PRINTER_STATUS_MANUAL_FEED",PRINTER_STATUS_MANUAL_FEED);
  AddConstant(dict, "PRINTER_STATUS_PAPER_PROBLEM",PRINTER_STATUS_PAPER_PROBLEM);
  AddConstant(dict, "PRINTER_STATUS_OFFLINE",PRINTER_STATUS_OFFLINE);
  AddConstant(dict, "PRINTER_STATUS_IO_ACTIVE",PRINTER_STATUS_IO_ACTIVE);
  AddConstant(dict, "PRINTER_STATUS_BUSY",PRINTER_STATUS_BUSY);
  AddConstant(dict, "PRINTER_STATUS_PRINTING",PRINTER_STATUS_PRINTING);
  AddConstant(dict, "PRINTER_STATUS_OUTPUT_BIN_FULL",PRINTER_STATUS_OUTPUT_BIN_FULL);
  AddConstant(dict, "PRINTER_STATUS_NOT_AVAILABLE",PRINTER_STATUS_NOT_AVAILABLE);
  AddConstant(dict, "PRINTER_STATUS_WAITING",PRINTER_STATUS_WAITING);
  AddConstant(dict, "PRINTER_STATUS_PROCESSING",PRINTER_STATUS_PROCESSING);
  AddConstant(dict, "PRINTER_STATUS_INITIALIZING",PRINTER_STATUS_INITIALIZING);
  AddConstant(dict, "PRINTER_STATUS_WARMING_UP",PRINTER_STATUS_WARMING_UP);
  AddConstant(dict, "PRINTER_STATUS_TONER_LOW",PRINTER_STATUS_TONER_LOW);
  AddConstant(dict, "PRINTER_STATUS_NO_TONER",PRINTER_STATUS_NO_TONER);
  AddConstant(dict, "PRINTER_STATUS_PAGE_PUNT",PRINTER_STATUS_PAGE_PUNT);
  AddConstant(dict, "PRINTER_STATUS_USER_INTERVENTION",PRINTER_STATUS_USER_INTERVENTION);
  AddConstant(dict, "PRINTER_STATUS_OUT_OF_MEMORY",PRINTER_STATUS_OUT_OF_MEMORY);
  AddConstant(dict, "PRINTER_STATUS_DOOR_OPEN",PRINTER_STATUS_DOOR_OPEN);
  AddConstant(dict, "PRINTER_STATUS_SERVER_UNKNOWN",PRINTER_STATUS_SERVER_UNKNOWN);
  AddConstant(dict, "PRINTER_STATUS_POWER_SAVE",PRINTER_STATUS_POWER_SAVE);

  // attribute flags for PRINTER_INFO_2
  AddConstant(dict, "PRINTER_ATTRIBUTE_QUEUED",PRINTER_ATTRIBUTE_QUEUED);
  AddConstant(dict, "PRINTER_ATTRIBUTE_DIRECT",PRINTER_ATTRIBUTE_DIRECT);
  AddConstant(dict, "PRINTER_ATTRIBUTE_DEFAULT",PRINTER_ATTRIBUTE_DEFAULT);
  AddConstant(dict, "PRINTER_ATTRIBUTE_SHARED",PRINTER_ATTRIBUTE_SHARED);
  AddConstant(dict, "PRINTER_ATTRIBUTE_NETWORK",PRINTER_ATTRIBUTE_NETWORK);
  AddConstant(dict, "PRINTER_ATTRIBUTE_HIDDEN",PRINTER_ATTRIBUTE_HIDDEN);
  AddConstant(dict, "PRINTER_ATTRIBUTE_LOCAL",PRINTER_ATTRIBUTE_LOCAL);
  AddConstant(dict, "PRINTER_ATTRIBUTE_ENABLE_DEVQ",PRINTER_ATTRIBUTE_ENABLE_DEVQ);
  AddConstant(dict, "PRINTER_ATTRIBUTE_KEEPPRINTEDJOBS",PRINTER_ATTRIBUTE_KEEPPRINTEDJOBS);
  AddConstant(dict, "PRINTER_ATTRIBUTE_DO_COMPLETE_FIRST",PRINTER_ATTRIBUTE_DO_COMPLETE_FIRST);
  AddConstant(dict, "PRINTER_ATTRIBUTE_WORK_OFFLINE",PRINTER_ATTRIBUTE_WORK_OFFLINE);
  AddConstant(dict, "PRINTER_ATTRIBUTE_ENABLE_BIDI",PRINTER_ATTRIBUTE_ENABLE_BIDI);
  AddConstant(dict, "PRINTER_ATTRIBUTE_RAW_ONLY",PRINTER_ATTRIBUTE_RAW_ONLY);
  AddConstant(dict, "PRINTER_ATTRIBUTE_PUBLISHED",PRINTER_ATTRIBUTE_PUBLISHED);
  AddConstant(dict, "PRINTER_ATTRIBUTE_FAX",PRINTER_ATTRIBUTE_FAX);
  AddConstant(dict, "PRINTER_ATTRIBUTE_TS",PRINTER_ATTRIBUTE_TS);

  // directory service contants for Action member of PRINTER_INFO_7
  AddConstant(dict, "DSPRINT_PUBLISH",DSPRINT_PUBLISH);
  AddConstant(dict, "DSPRINT_UNPUBLISH",DSPRINT_UNPUBLISH);
  AddConstant(dict, "DSPRINT_UPDATE",DSPRINT_UPDATE);
  AddConstant(dict, "DSPRINT_PENDING",DSPRINT_PENDING);
  AddConstant(dict, "DSPRINT_REPUBLISH",DSPRINT_REPUBLISH);

  // port types from PORT_INFO_2
  AddConstant(dict, "PORT_TYPE_WRITE",PORT_TYPE_WRITE);
  AddConstant(dict, "PORT_TYPE_READ",PORT_TYPE_READ);
  AddConstant(dict, "PORT_TYPE_REDIRECTED",PORT_TYPE_REDIRECTED);
  AddConstant(dict, "PORT_TYPE_NET_ATTACHED",PORT_TYPE_NET_ATTACHED);

  HMODULE hmodule=LoadLibrary("winspool.drv");
  if (hmodule!=NULL){
	pfnEnumForms=(EnumFormsfunc)GetProcAddress(hmodule,"EnumFormsW");
	pfnAddForm=(AddFormfunc)GetProcAddress(hmodule,"AddFormW");
	pfnDeleteForm=(DeleteFormfunc)GetProcAddress(hmodule,"DeleteFormW");
	pfnGetForm=(GetFormfunc)GetProcAddress(hmodule,"GetFormW");
	pfnSetForm=(SetFormfunc)GetProcAddress(hmodule,"SetFormW");
	pfnAddJob=(AddJobfunc)GetProcAddress(hmodule,"AddJobW");
	pfnScheduleJob=(ScheduleJobfunc)GetProcAddress(hmodule,"ScheduleJob");
	pfnEnumPorts=(EnumPortsfunc)GetProcAddress(hmodule,"EnumPortsW");
	pfnEnumMonitors=(EnumPortsfunc)GetProcAddress(hmodule,"EnumMonitorsW");
	pfnGetPrintProcessorDirectory=(GetPrintProcessorDirectoryfunc)GetProcAddress(hmodule,"GetPrintProcessorDirectoryW");
	pfnGetPrinterDriverDirectory=(GetPrintProcessorDirectoryfunc)GetProcAddress(hmodule,"GetPrinterDriverDirectoryW");
  }
  dummy_tuple=PyTuple_New(0);
}

