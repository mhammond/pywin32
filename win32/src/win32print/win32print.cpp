/***********************************************************

win32printmodule.cpp -- module for interface into printer API


Note that this source file contains embedded documentation.
This documentation consists of marked up text inside the
C comments, and is prefixed with an '@' symbol.  The source
files are processed by a tool called "autoduck" which
generates Windows .hlp files.
@doc

******************************************************************/

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
typedef BOOL (WINAPI *DeletePrinterDriverExfunc)(LPWSTR, LPWSTR, LPWSTR, DWORD, DWORD);
static DeletePrinterDriverExfunc pfnDeletePrinterDriverEx=NULL;
typedef BOOL (WINAPI *FlushPrinterfunc)(HANDLE, LPVOID, DWORD, LPDWORD, DWORD);
static FlushPrinterfunc pfnFlushPrinter=NULL;
typedef BOOL (WINAPI *GetDefaultPrinterfunc)(LPWSTR, LPDWORD);
static GetDefaultPrinterfunc pfnGetDefaultPrinter=NULL;
typedef BOOL (WINAPI *SetDefaultPrinterfunc)(LPWSTR);
static SetDefaultPrinterfunc pfnSetDefaultPrinter=NULL;

static PyObject *dummy_tuple=NULL;

// To be used in PyArg_ParseTuple with O& format 
BOOL PyWinObject_AsPrinterHANDLE(PyObject *obhprinter, HANDLE *phprinter){
	return PyWinObject_AsHANDLE(obhprinter, phprinter);
}

// @object PyPrinterHANDLE|Handle to a printer or print server.
//	<nl>Created using <om win32print.OpenPrinter> or <om win32print.AddPrinter>
//	<nl>Inherits all methods and properties of <o PyHANDLE>.
//	<nl>When object is destroyed, handle is released using ClosePrinter.
class PyPrinterHANDLE: public PyHANDLE
{
public:
	PyPrinterHANDLE(HANDLE hInit) : PyHANDLE(hInit) {}
	virtual BOOL Close(void){
		BOOL ret=ClosePrinter(m_handle);
		if (!ret)
			PyWin_SetAPIError("ClosePrinter");
		m_handle = 0;
		return ret;
		}
	virtual const char *GetTypeName(){
		return "PyPrinterHANDLE";
		}
};

PyObject *PyWinObject_FromPrinterHANDLE(HANDLE hprinter)
{
	PyObject *ret=new PyPrinterHANDLE(hprinter);
	if (ret==NULL)
		PyErr_NoMemory();
	return ret;
}

void PyWinObject_FreePRINTER_DEFAULTS(PPRINTER_DEFAULTS pdefaults)
{
	PyWinObject_FreeTCHAR(pdefaults->pDatatype);
}

// @object PRINTER_DEFAULTS|A dictionary representing a PRINTER_DEFAULTS structure
// @prop string|pDatatype|Data type to be used for print jobs, see <om win32print.EnumPrintProcessorDatatypes>, optional, can be None
// @prop <o PyDEVMODE>|pDevMode|A PyDEVMODE that specifies default printer parameters, optional, can be None 
// @prop int|DesiredAccess|An ACCESS_MASK specifying what level of access is needed, eg PRINTER_ACCESS_ADMINISTER, PRINTER_ACCESS_USE 
BOOL PyWinObject_AsPRINTER_DEFAULTS(PyObject *obdefaults, PPRINTER_DEFAULTS pdefaults)
{
	static char *printer_default_keys[]={"DesiredAccess","pDataType","pDevMode",NULL};
	static char *printer_default_format="k|OO";
	ZeroMemory(pdefaults,sizeof(PRINTER_DEFAULTS));
	PyObject *obDataType=Py_None, *obdevmode=Py_None;
	if (!PyDict_Check(obdefaults)){
		PyErr_SetString(PyExc_TypeError, "PRINTER_DEFAULTS must be a dictionary");
		return FALSE;
		}
	return PyArg_ParseTupleAndKeywords(dummy_tuple,obdefaults,printer_default_format,printer_default_keys,
		&pdefaults->DesiredAccess, &pdefaults->pDatatype, &obdevmode)
		&&PyWinObject_AsDEVMODE(obdevmode, &pdefaults->pDevMode, TRUE)
		&&PyWinObject_AsTCHAR(obDataType, &pdefaults->pDatatype, TRUE);
}

// Printer stuff.
// @pymethod <o PyPrinterHANDLE>|win32print|OpenPrinter|Retrieves a handle to a printer.
static PyObject *PyOpenPrinter(PyObject *self, PyObject *args)
{
	TCHAR *printer;
	HANDLE handle;
	PRINTER_DEFAULTS printer_defaults = {NULL, NULL, 0};
	PRINTER_DEFAULTS *pprinter_defaults=NULL;
	PyObject *obprinter, *obdefaults=Py_None, *ret=NULL;
	if (!PyArg_ParseTuple(args, "O|O:OpenPrinter",
		&obprinter,     // @pyparm string|printer||Printer or print server name.  Use None to open local print server.
		&obdefaults)) // @pyparm dict|Defaults|None|<o PRINTER_DEFAULTS> dict, or None
		return NULL;
	if (obdefaults!=Py_None){
		if (!PyWinObject_AsPRINTER_DEFAULTS(obdefaults, &printer_defaults))
			return NULL;
		pprinter_defaults=&printer_defaults;
		}
	if (PyWinObject_AsTCHAR(obprinter, &printer, TRUE)){
		BOOL bsuccess;
		Py_BEGIN_ALLOW_THREADS
		bsuccess = OpenPrinter(printer, &handle, pprinter_defaults);
		Py_END_ALLOW_THREADS
		if (bsuccess)
			ret=PyWinObject_FromPrinterHANDLE(handle);
		else
			PyWin_SetAPIError("OpenPrinter");
		}
	PyWinObject_FreePRINTER_DEFAULTS(&printer_defaults);
	PyWinObject_FreeTCHAR(printer);
	return ret;
}

// @pymethod |win32print|ClosePrinter|Closes a handle to a printer.
static PyObject *PyClosePrinter(PyObject *self, PyObject *args)
{
	PyObject *obhprinter;
	if (!PyArg_ParseTuple(args, "O:ClosePrinter",
		&obhprinter))	// @pyparm <o PyPrinterHANDLE>|hPrinter||handle to printer object
		return NULL;

	/* If the object is a PyPrinterHANDLE, its Close method must be called to ensure that the m_handle member is cleared.
		A second handle with the same value can be created as soon as the first handle is closed here, and if
		this happens between the time this function is executed and the first object is deref'ed, the original object's
		destruction would close a valid handle contained in the second object. */
	if (PyHANDLE_Check(obhprinter)){
		// Make sure we can't Close any other type of handle
		const char *handletype=((PyHANDLE *)obhprinter)->GetTypeName();
		if (strcmp(handletype, "PyPrinterHANDLE")!=0)
			return PyErr_Format(PyExc_TypeError, "ClosePrinter: Object must be a printer handle, not %s", handletype);
		if (((PyHANDLE *)obhprinter)->Close()){
			Py_INCREF(Py_None);
			return Py_None;
			}
		return NULL;
		}

	HANDLE hprinter;
	if (!PyWinObject_AsPrinterHANDLE(obhprinter, &hprinter))
		return NULL;
	
	if (!ClosePrinter(hprinter))
		return PyWin_SetAPIError("ClosePrinter");
	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *PyWinObject_FromPRINTER_INFO(LPBYTE printer_info, DWORD level)
{
	switch (level){
		case 1:
			PRINTER_INFO_1 *pi1;
			pi1=(PRINTER_INFO_1 *)printer_info;
			return Py_BuildValue("{s:k,s:N,s:N,s:N}",
				"Flags",pi1->Flags,
				"pDescription",PyWinObject_FromTCHAR(pi1->pDescription),
				"pName",PyWinObject_FromTCHAR(pi1->pName),
				"pComment",PyWinObject_FromTCHAR(pi1->pComment));
		case 2:
			PRINTER_INFO_2 *pi2;
			pi2=(PRINTER_INFO_2 *)printer_info;
			return Py_BuildValue("{s:N,s:N,s:N,s:N,s:N,s:N,s:N,s:N,s:N,s:N,s:N,s:N,s:N,s:k,s:k,s:k,s:k,s:k,s:k,s:k,s:k}",
				"pServerName",PyWinObject_FromTCHAR(pi2->pServerName),
				"pPrinterName",PyWinObject_FromTCHAR(pi2->pPrinterName),
				"pShareName",PyWinObject_FromTCHAR(pi2->pShareName),
				"pPortName",PyWinObject_FromTCHAR(pi2->pPortName),
				"pDriverName",PyWinObject_FromTCHAR(pi2->pDriverName),
				"pComment",PyWinObject_FromTCHAR(pi2->pComment),
				"pLocation",PyWinObject_FromTCHAR(pi2->pLocation), 
				"pDevMode",PyWinObject_FromDEVMODE(pi2->pDevMode),
				"pSepFile", PyWinObject_FromTCHAR(pi2->pSepFile),
				"pPrintProcessor",PyWinObject_FromTCHAR(pi2->pPrintProcessor),
				"pDatatype",PyWinObject_FromTCHAR(pi2->pDatatype),
				"pParameters",PyWinObject_FromTCHAR(pi2->pParameters),
				"pSecurityDescriptor",PyWinObject_FromSECURITY_DESCRIPTOR(pi2->pSecurityDescriptor),
				"Attributes",pi2->Attributes, "Priority",pi2->Priority,
				"DefaultPriority",pi2->DefaultPriority,
				"StartTime",pi2->StartTime, "UntilTime",pi2->UntilTime,
				"Status",pi2->Status, "cJobs",pi2->cJobs, "AveragePPM",pi2->AveragePPM);
		case 3:
			PRINTER_INFO_3 *pi3;
			pi3=(PRINTER_INFO_3 *)printer_info;
			return Py_BuildValue("{s:N}","pSecurityDescriptor",PyWinObject_FromSECURITY_DESCRIPTOR(pi3->pSecurityDescriptor));
		case 4:
			PRINTER_INFO_4 *pi4;
			pi4=(PRINTER_INFO_4 *)printer_info;
			return Py_BuildValue("{s:N,s:N,s:k}",
				"pPrinterName",PyWinObject_FromTCHAR(pi4->pPrinterName),
				"pServerName",PyWinObject_FromTCHAR(pi4->pServerName), 
				"Attributes",pi4->Attributes);
		case 5:
			PRINTER_INFO_5 *pi5;
			pi5=(PRINTER_INFO_5 *)printer_info;
			return Py_BuildValue("{s:N,s:N,s:k,s:k,s:k}",
				"pPrinterName",PyWinObject_FromTCHAR(pi5->pPrinterName),
				"pPortName",PyWinObject_FromTCHAR(pi5->pPortName),
				"Attributes",pi5->Attributes,
				"DeviceNotSelectedTimeout",pi5->DeviceNotSelectedTimeout,
				"TransmissionRetryTimeout",pi5->TransmissionRetryTimeout);
		case 7:
			PRINTER_INFO_7 *pi7;
			pi7=(PRINTER_INFO_7 *)printer_info;
			return Py_BuildValue("{s:N,s:k}",
				"ObjectGUID",PyWinObject_FromTCHAR(pi7->pszObjectGUID),
				"Action",pi7->dwAction);
		case 8:   // global printer defaults
			PRINTER_INFO_8 *pi8;
			pi8=(PRINTER_INFO_8 *)printer_info;
			return Py_BuildValue("{s:N}","pDevMode", PyWinObject_FromDEVMODE(pi8->pDevMode));
		case 9:  // per user printer defaults
			PRINTER_INFO_9 *pi9;
			pi9=(PRINTER_INFO_9 *)printer_info;
			return Py_BuildValue("{s:N}","pDevMode", PyWinObject_FromDEVMODE(pi9->pDevMode));
		default:
			return PyErr_Format(PyExc_NotImplementedError,"Level %d is not supported",level);
		}
}

// @pymethod dict|win32print|GetPrinter|Retrieves information about a printer
// @rdesc Returns a dictionary containing PRINTER_INFO_* data for level, or
//  returns a tuple of PRINTER_INFO_2 data if no level is passed in.
static PyObject *PyGetPrinter(PyObject *self, PyObject *args)
{
	HANDLE hprinter;
	DWORD needed, level;
	BOOL backward_compat;
	LPBYTE buf=NULL;
	PyObject *rc=NULL;
	PRINTER_INFO_2 *pi2;
	// @comm Original implementation used level 2 only and returned a tuple
	// Pass single arg as indicator to use old behaviour for backward compatibility
	if (PyArg_ParseTuple(args, "O&:GetPrinter", 
		PyWinObject_AsPrinterHANDLE, &hprinter)){ // @pyparm <o PyPrinterHANDLE>|hPrinter||handle to printer object as returned by <om win32print.OpenPrinter>
		backward_compat=TRUE;
		level=2;
		}
	else{
		PyErr_Clear();
		if (!PyArg_ParseTuple(args, "O&k:GetPrinter", 
			PyWinObject_AsPrinterHANDLE, &hprinter,
			&level)) // @pyparm int|Level|2|Level of data returned (1,2,3,4,5,7,8,9)
			return NULL;
		backward_compat=FALSE;
		}
	// first allocate memory.
	GetPrinter(hprinter, level, NULL, 0, &needed );
	if (GetLastError()!=ERROR_INSUFFICIENT_BUFFER)
		return PyWin_SetAPIError("GetPrinter");
	buf=(LPBYTE)malloc(needed);
	if (buf==NULL)
		return PyErr_Format(PyExc_MemoryError,"GetPrinter: Unable to allocate buffer of %d bytes", needed);
	if (!GetPrinter(hprinter, level, buf, needed, &needed )) {
		free(buf);
		return PyWin_SetAPIError("GetPrinter");
	}
	if (backward_compat){
		pi2=(PRINTER_INFO_2 *)buf;
		rc = Py_BuildValue("NNNNNNNONNNNOkkkkkkkk",
			PyWinObject_FromTCHAR(pi2->pServerName),
			PyWinObject_FromTCHAR(pi2->pPrinterName),
			PyWinObject_FromTCHAR(pi2->pShareName),
			PyWinObject_FromTCHAR(pi2->pPortName),
			PyWinObject_FromTCHAR(pi2->pDriverName),
			PyWinObject_FromTCHAR(pi2->pComment),
			PyWinObject_FromTCHAR(pi2->pLocation),
			Py_None,
			PyWinObject_FromTCHAR(pi2->pSepFile),
			PyWinObject_FromTCHAR(pi2->pPrintProcessor),
			PyWinObject_FromTCHAR(pi2->pDatatype),
			PyWinObject_FromTCHAR(pi2->pParameters),
			Py_None,
			pi2->Attributes, pi2->Priority, pi2->DefaultPriority, pi2->StartTime, pi2->UntilTime,
			pi2->Status, pi2->cJobs, pi2->AveragePPM);
		}
	else
		rc = PyWinObject_FromPRINTER_INFO(buf, level);
	free(buf);
	return rc;
}

void PyWinObject_FreePRINTER_INFO(DWORD level, LPBYTE pbuf)
{	
	if ((level==0) || (pbuf==NULL))
		return;
	switch(level){
		case 2:{
			PRINTER_INFO_2 *pi2 = (PRINTER_INFO_2 *)pbuf;
			PyWinObject_FreeTCHAR(pi2->pServerName);
			PyWinObject_FreeTCHAR(pi2->pPrinterName);
			PyWinObject_FreeTCHAR(pi2->pShareName);
			PyWinObject_FreeTCHAR(pi2->pPortName);
			PyWinObject_FreeTCHAR(pi2->pDriverName);
			PyWinObject_FreeTCHAR(pi2->pComment);
			PyWinObject_FreeTCHAR(pi2->pLocation);
			PyWinObject_FreeTCHAR(pi2->pSepFile);
			PyWinObject_FreeTCHAR(pi2->pPrintProcessor);
			PyWinObject_FreeTCHAR(pi2->pDatatype);
			PyWinObject_FreeTCHAR(pi2->pParameters);
			break;
			}
		case 4:{
			PRINTER_INFO_4 *pi4 = (PRINTER_INFO_4 *)pbuf;
			PyWinObject_FreeTCHAR(pi4->pPrinterName);
			PyWinObject_FreeTCHAR(pi4->pServerName);
			break;
			}
		case 5:{
			PRINTER_INFO_5 *pi5 = (PRINTER_INFO_5 *)pbuf;
			PyWinObject_FreeTCHAR(pi5->pPrinterName);
			PyWinObject_FreeTCHAR(pi5->pPortName);
			break;
			}
		case 7:{
			PRINTER_INFO_7 *pi7 = (PRINTER_INFO_7 *)pbuf;
			PyWinObject_FreeTCHAR(pi7->pszObjectGUID);
			break;
			}
		default:
			break;
		}
	free(pbuf);
}

BOOL PyWinObject_AsPRINTER_INFO(DWORD level, PyObject *obinfo, LPBYTE *pbuf)
{	
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
		case 2:{
			static char *pi2_keys[]={"pServerName","pPrinterName","pShareName","pPortName",
				"pDriverName","pComment","pLocation","pDevMode","pSepFile","pPrintProcessor",
				"pDatatype","pParameters","pSecurityDescriptor","Attributes","Priority",
				"DefaultPriority","StartTime","UntilTime","Status","cJobs","AveragePPM", NULL};
			static char *pi2_format="OOOOOOOOOOOOOkkkkkkkk:PRINTER_INFO_2";
			PyObject *obServerName=Py_None, *obPrinterName=Py_None, *obShareName=Py_None,
				*obPortName=Py_None, *obDriverName=Py_None, *obComment=Py_None,
				*obLocation=Py_None, *obDevMode=Py_None,
				*obSepFile=Py_None, *obPrintProcessor=Py_None,
				*obDatatype=Py_None, *obParameters=Py_None, *obSecurityDescriptor=Py_None;
			PRINTER_INFO_2 *pi2;
			bufsize=sizeof(PRINTER_INFO_2);
			if (NULL == (*pbuf= (LPBYTE)malloc(bufsize))){
				PyErr_Format(PyExc_MemoryError, "Malloc failed for %d bytes", bufsize);
				break;
				}
			ZeroMemory(*pbuf,bufsize);
			pi2=(PRINTER_INFO_2 *)*pbuf;

			ret=PyArg_ParseTupleAndKeywords(dummy_tuple, obinfo, pi2_format, pi2_keys,
					&obServerName, &obPrinterName, &obShareName, &obPortName,
					&obDriverName, &obComment, &obLocation,
					&obDevMode,
					&obSepFile, &obPrintProcessor, &obDatatype, &obParameters,
					&obSecurityDescriptor,
					&pi2->Attributes, &pi2->Priority, &pi2->DefaultPriority, &pi2->StartTime,
					&pi2->UntilTime, &pi2->Status, &pi2->cJobs, &pi2->AveragePPM)
				&&PyWinObject_AsTCHAR(obServerName, &pi2->pServerName, TRUE)
				&&PyWinObject_AsTCHAR(obPrinterName, &pi2->pPrinterName, TRUE)
				&&PyWinObject_AsTCHAR(obShareName, &pi2->pShareName, TRUE)
				&&PyWinObject_AsTCHAR(obPortName, &pi2->pPortName, TRUE)
				&&PyWinObject_AsTCHAR(obDriverName, &pi2->pDriverName, TRUE)
				&&PyWinObject_AsTCHAR(obComment, &pi2->pComment, TRUE)
				&&PyWinObject_AsTCHAR(obLocation, &pi2->pLocation, TRUE)
				&&PyWinObject_AsDEVMODE(obDevMode, &pi2->pDevMode,FALSE)
				&&PyWinObject_AsTCHAR(obSepFile, &pi2->pSepFile, TRUE)
				&&PyWinObject_AsTCHAR(obPrintProcessor, &pi2->pPrintProcessor, TRUE)
				&&PyWinObject_AsTCHAR(obDatatype, &pi2->pDatatype, TRUE)
				&&PyWinObject_AsTCHAR(obParameters, &pi2->pParameters, TRUE)
				&&PyWinObject_AsSECURITY_DESCRIPTOR(obSecurityDescriptor, &pi2->pSecurityDescriptor, TRUE);
			break;
			}
		case 3:{
			static char *pi3_keys[]={"pSecurityDescriptor", NULL};
			static char *pi3_format="O:PRINTER_INFO_3";
			PyObject *obSecurityDescriptor;
			PRINTER_INFO_3 *pi3;
			bufsize=sizeof(PRINTER_INFO_3);
			if (NULL == (*pbuf=(LPBYTE)malloc(bufsize))){
				PyErr_Format(PyExc_MemoryError, "Malloc failed for %d bytes", bufsize);
				break;
				}
			ZeroMemory(*pbuf,bufsize);
			pi3=(PRINTER_INFO_3 *)*pbuf;
			ret=PyArg_ParseTupleAndKeywords(dummy_tuple, obinfo, pi3_format, pi3_keys, &obSecurityDescriptor)
				&&PyWinObject_AsSECURITY_DESCRIPTOR(obSecurityDescriptor, &pi3->pSecurityDescriptor, FALSE);
			break;
			}
		case 4:{
			static char *pi4_keys[]={"pPrinterName","pServerName","Attributes", NULL};
			static char *pi4_format="OOk:PRINTER_INFO_4";
			PyObject *obPrinterName=Py_None, *obServerName=Py_None;
			PRINTER_INFO_4 *pi4;
			bufsize=sizeof(PRINTER_INFO_4);
			if (NULL == (*pbuf=(LPBYTE)malloc(bufsize))){
				PyErr_Format(PyExc_MemoryError, "Malloc failed for %d bytes", bufsize);
				break;
				}
			ZeroMemory(*pbuf,bufsize);
			pi4=(PRINTER_INFO_4 *)*pbuf;
			ret=PyArg_ParseTupleAndKeywords(dummy_tuple, obinfo, pi4_format, pi4_keys,
					&obPrinterName, &obServerName, &pi4->Attributes)
				&&PyWinObject_AsTCHAR(obPrinterName, &pi4->pPrinterName, TRUE)
				&&PyWinObject_AsTCHAR(obServerName, &pi4->pServerName, TRUE);
			break;
			}
		case 5:{
			static char *pi5_keys[]={"pPrinterName","pPortName","Attributes",
				"DeviceNotSelectedTimeout","TransmissionRetryTimeout", NULL};
			static char *pi5_format="OOkkk:PRINTER_INFO_5";
			PyObject *obPrinterName=Py_None, *obPortName=Py_None;

			PRINTER_INFO_5 *pi5;
			bufsize=sizeof(PRINTER_INFO_5);
			if (NULL == (*pbuf=(LPBYTE)malloc(bufsize))){
				PyErr_Format(PyExc_MemoryError, "Malloc failed for %d bytes", bufsize);
				break;
				}
			ZeroMemory(*pbuf,bufsize);
			pi5=(PRINTER_INFO_5 *)*pbuf;
			ret=PyArg_ParseTupleAndKeywords(dummy_tuple, obinfo, pi5_format, pi5_keys,
					&obPrinterName, &obPortName, &pi5->Attributes,
					&pi5->DeviceNotSelectedTimeout, &pi5->TransmissionRetryTimeout)
				&&PyWinObject_AsTCHAR(obPrinterName, &pi5->pPrinterName, TRUE)
				&&PyWinObject_AsTCHAR(obPortName, &pi5->pPortName, TRUE);
			break;
			}
		case 7:{
			static char *pi7_keys[]={"ObjectGUID","Action", NULL};
			static char *pi7_format="Ok:PRINTER_INFO_7";
			PyObject *obObjectGUID=Py_None;
			PRINTER_INFO_7 *pi7;
			bufsize=sizeof(PRINTER_INFO_7);
			if (NULL == (*pbuf=(LPBYTE)malloc(bufsize))){
				PyErr_Format(PyExc_MemoryError, "Malloc failed for %d bytes", bufsize);
				break;
				}
			ZeroMemory(*pbuf,bufsize);
			pi7=(PRINTER_INFO_7 *)*pbuf;
			ret=PyArg_ParseTupleAndKeywords(dummy_tuple, obinfo, pi7_format, pi7_keys,
					&obObjectGUID, &pi7->dwAction)
				&&PyWinObject_AsTCHAR(obObjectGUID, &pi7->pszObjectGUID, TRUE);
			break;
			}
		case 8:
		case 9:{   //identical structs, 8 is for global defaults and 9 is for user defaults
			static char *pi8_keys[]={"pDevMode", NULL};
			static char *pi8_format="O:PRINTER_INFO_8";
			PyObject *obDevMode;
			PRINTER_INFO_8 *pi8;
			bufsize=sizeof(PRINTER_INFO_8);
			if (NULL == (*pbuf=(LPBYTE)malloc(bufsize))){
				PyErr_Format(PyExc_MemoryError, "Malloc failed for %d bytes", bufsize);
				break;
				}
			ZeroMemory(*pbuf,bufsize);
			pi8=(PRINTER_INFO_8 *)*pbuf;
			ret=PyArg_ParseTupleAndKeywords(dummy_tuple, obinfo, pi8_format, pi8_keys, &obDevMode)
				&&PyWinObject_AsDEVMODE(obDevMode,&pi8->pDevMode,FALSE);
			break;
			}
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
	// @pyparm <o PyPrinterHANDLE>|hPrinter||Printer handle as returned by <om win32print.OpenPrinter>
	// @pyparm int|Level||Level of data contained in pPrinter
	// @pyparm dict|pPrinter||PRINTER_INFO_* dict as returned by <om win32print.GetPrinter>, can be None if level is 0
	// @pyparm int|Command||Command to send to printer - one of the PRINTER_CONTROL_* constants, or 0
	// @comm If Level is 0 and Command is PRINTER_CONTROL_SET_STATUS, pPrinter should be an integer,
	// and is interpreted as the new printer status to set (one of the PRINTER_STATUS_* constants). 
	if (!PyArg_ParseTuple(args, "O&kOk:SetPrinter", 
		PyWinObject_AsPrinterHANDLE, &hprinter, &level, &obinfo, &command))
		return NULL;
	if (!PyWinObject_AsPRINTER_INFO(level, obinfo, &buf))
		return NULL;
	if (!SetPrinter(hprinter, level, buf, command))
		PyWin_SetAPIError("SetPrinter");
	else{
		Py_INCREF(Py_None);
		ret=Py_None;
		}
	PyWinObject_FreePRINTER_INFO(level, buf);
	return ret;
}

// @pymethod None|win32print|AddPrinterConnection|Connects to remote printer
static PyObject *PyAddPrinterConnection(PyObject *self, PyObject *args)
{
	TCHAR *printer;
	PyObject *obprinter;
	if (!PyArg_ParseTuple(args, "O:AddPrinterConnection", 
	          &obprinter)) // @pyparm string|printer||printer to connect to (eg: \\server\printer).
		return NULL;
	if (!PyWinObject_AsTCHAR(obprinter, &printer, FALSE))
		return NULL;
	BOOL bsuccess=AddPrinterConnection(printer);
	PyWinObject_FreeTCHAR(printer);
	if (!bsuccess)
		return PyWin_SetAPIError("AddPrinterConnection");
	Py_INCREF(Py_None);
	return Py_None;
}


// @pymethod None|win32print|DeletePrinterConnection|Removes connection to remote printer
static PyObject *PyDeletePrinterConnection(PyObject *self, PyObject *args)
{
	TCHAR *printer;
	PyObject *obprinter;
	if (!PyArg_ParseTuple(args, "O:DeletePrinterConnection", 
	          &obprinter)) // @pyparm string|printer||printer to disconnect from (eg: \\server\printer).
		return NULL;
	if (!PyWinObject_AsTCHAR(obprinter, &printer, FALSE))
		return NULL;
	BOOL bsuccess=DeletePrinterConnection(printer);
	PyWinObject_FreeTCHAR(printer);
	if (!bsuccess)
		return PyWin_SetAPIError("DeletePrinterConnection");
	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod string|win32print|GetDefaultPrinter|Returns the default printer.
static PyObject *PyGetDefaultPrinter(PyObject *self, PyObject *args)
{
	TCHAR *printer, *s;
	int printer_size= 100;

	/* Windows < 2000 does not have a GetDefaultPrinter so the default printer
	   must be retrieved from registry */

	if (NULL == (printer= (TCHAR *)malloc(printer_size * sizeof(TCHAR))))
	{
		PyErr_SetString(PyExc_MemoryError, "Malloc failed.");
		return NULL;
	}
	if (0 == GetProfileString(TEXT("Windows"), TEXT("Device"), TEXT(""), printer, printer_size))
	{
		PyErr_SetString(PyExc_RuntimeError, "The default printer was not found.");
		return NULL;
	}
	if (NULL == (s= _tcschr(printer, TEXT(','))))
	{
		PyErr_SetString(PyExc_RuntimeError, "The returned printer is malformed.");
		return NULL;
	}
	*s= 0;
	PyObject *ret= PyWinObject_FromTCHAR(printer);
	free(printer);
	return ret;
}

// @pymethod <o PyUnicode>|win32print|GetDefaultPrinterW|Returns the default printer.
// @comm Unlike <om win32print.GetDefaultPrinter>, this method calls the GetDefaultPrinter API function.
static PyObject *PyGetDefaultPrinterW(PyObject *self, PyObject *args)
{
	CHECK_PFN(GetDefaultPrinter);
	WCHAR *printer=NULL;
	DWORD err, printer_size=100;
	PyObject *ret=NULL;

	printer= (WCHAR *)malloc(printer_size*sizeof(WCHAR));
	if (printer==NULL)
		return PyErr_Format(PyExc_MemoryError, "Unable to allocate %d bytes", printer_size*sizeof(WCHAR));
	if (!(*pfnGetDefaultPrinter)(printer, &printer_size)){
		err=GetLastError();
		if (err!=ERROR_INSUFFICIENT_BUFFER){
			PyWin_SetAPIError("GetDefaultPrinter");
			goto done;
			}
		free(printer);
		printer=(WCHAR *)malloc(printer_size*sizeof(WCHAR));
		if (printer==NULL)
			return PyErr_Format(PyExc_MemoryError, "Unable to allocate %d bytes", printer_size*sizeof(WCHAR));
		if (!(*pfnGetDefaultPrinter)(printer, &printer_size)){
			PyWin_SetAPIError("GetDefaultPrinter");
			goto done;
			}
		}
	ret=PyWinObject_FromWCHAR(printer);

	done:
	if (printer)
		free(printer);
	return ret;
}

// @pymethod None|win32print|SetDefaultPrinter|Sets the default printer.
// @comm This function uses the pre-win2k method of WriteProfileString rather than the SetDefaultPrinter API function 
static PyObject *PySetDefaultPrinter(PyObject *self, PyObject *args)
{
	TCHAR *printer=NULL, *info=NULL, *dprinter=NULL;
	int info_size= 100;
	PyObject *obprinter;
	/* Windows < 2000 does not have a SetDefaultPrinter so the default printer
	   must be set in the registry */
	if (!PyArg_ParseTuple(args, "O:SetDefaultPrinter", 
	        &obprinter)) // @pyparm string|printer||printer to set as default
		return NULL;
	if (!PyWinObject_AsTCHAR(obprinter, &printer, FALSE))
		return NULL;

	if (NULL == (info= (TCHAR *)malloc(info_size *sizeof(TCHAR))))
		PyErr_NoMemory();
	else if (0 == GetProfileString(TEXT("Devices"), printer, TEXT(""), info, info_size))
		PyErr_SetString(PyExc_RuntimeError, "The printer was not found.");
	else if (NULL == (dprinter= (TCHAR *)malloc((_tcslen(printer) + _tcslen(info) + 3) * sizeof(TCHAR))))
		PyErr_NoMemory();
	else{
		_tcscpy(dprinter, printer);
		_tcscat(dprinter, TEXT(","));
		_tcscat(dprinter, info);
		WriteProfileString(TEXT("Windows"), TEXT("device"), dprinter);
		SendNotifyMessage(HWND_BROADCAST,WM_SETTINGCHANGE,0,0);
		}
	if (dprinter)
		free(dprinter);
	if	(info)
		free(info);
	PyWinObject_FreeTCHAR(printer);
	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod None|win32print|SetDefaultPrinterW|Sets the default printer
// @comm Unlike <om win32print.SetDefaultPrinter>, this method calls the SetDefaultPrinter API function.
static PyObject *PySetDefaultPrinterW(PyObject *self, PyObject *args)
{
	CHECK_PFN(SetDefaultPrinter);
	WCHAR *printer=NULL;
	PyObject *obprinter, *ret=NULL;
	// @pyparm <o PyUnicode>|Printer||Name of printer, can be None to use first available printer
	if (!PyArg_ParseTuple(args, "O:SetDefaultPrinter", &obprinter))
		return NULL;
	if (!PyWinObject_AsWCHAR(obprinter, &printer, TRUE))
		return NULL;
	if (!(*pfnSetDefaultPrinter)(printer))
		PyWin_SetAPIError("SetDefaultPrinter");
	else{
		Py_INCREF(Py_None);
		ret = Py_None;
		}
	PyWinObject_FreeWCHAR(printer);
	return ret;
}

// @pymethod tuple|win32print|EnumPrinters|Enumerates printers, print servers, domains and print providers.
// @comm Use Flags=PRINTER_ENUM_NAME, Name=None, Level=1 to enumerate print providers.<nl>
// Use Flags=PRINTER_ENUM_NAME, Name=\\servername, Level=2 or 5 to list printers on another server.<nl>
// See MSDN docs for EnumPrinters for other specific combinations
static PyObject *PyEnumPrinters(PyObject *self, PyObject *args)
{
	DWORD flags;
	DWORD level= 1;
	BYTE *buf=NULL;
	DWORD bufsize;
	DWORD bufneeded;
	DWORD printersreturned;
	TCHAR *name= NULL;
	PyObject *obname=Py_None;
	DWORD i;
	PyObject *ret=NULL, *obprinter_info;
	static size_t printer_info_offset[]={
		sizeof(PRINTER_INFO_1),sizeof(PRINTER_INFO_2),sizeof(PRINTER_INFO_3),
		sizeof(PRINTER_INFO_4),sizeof(PRINTER_INFO_5),sizeof(PRINTER_INFO_6),
		sizeof(PRINTER_INFO_7),sizeof(PRINTER_INFO_8),sizeof(PRINTER_INFO_9)
		};
	if (!PyArg_ParseTuple(args, "k|Ok:EnumPrinters", 
					&flags,   // @pyparm int|flags||types of printer objects to enumerate (combination of PRINTER_ENUM_* constants).
					&obname,    // @pyparm string|name|None|name of printer object.
					&level))  // @pyparm int|level|1|type of printer info structure (Levels 1,2,4,5 supported)
		return NULL;
	if (level<1 || level>9)
		return PyErr_Format(PyExc_ValueError,"Level %d is not supported", level);
	if (!PyWinObject_AsTCHAR(obname, &name, TRUE))
		return NULL;	// last exit without cleanup

	// if call with NULL buffer succeeds, there's nothing to enumerate
	if (EnumPrinters(flags, name, level, NULL, 0, &bufneeded, &printersreturned)){
		ret = PyTuple_New(0);
		goto done;
		}
	if (GetLastError()!=ERROR_INSUFFICIENT_BUFFER){
		PyWin_SetAPIError("EnumPrinters");
		goto done;
		}
	bufsize= bufneeded;
	if (NULL == (buf= (BYTE *)malloc(bufsize))){
		PyErr_Format(PyExc_MemoryError,"EnumPrinters: unable to allocate %d bytes", bufsize);
		goto done;
		}

	// @rdesc Level 1 returns a tuple of tuples for backward compatibility.
	// Each individual element is a tuple of (flags, description, name, comment)<nl>
	// All other levels return a tuple of dictionaries representing PRINTER_INFO_* structures
	if (!EnumPrinters(flags, name, level, buf, bufsize, &bufneeded, &printersreturned))
		PyWin_SetAPIError("EnumPrinters");
	else{
		ret=PyTuple_New(printersreturned);
		if (ret!=NULL)
			for (i= 0; i < printersreturned; i++){
				if (level==1){
					PRINTER_INFO_1 *info;
					info= (PRINTER_INFO_1 *)(buf + i * sizeof(PRINTER_INFO_1));
					obprinter_info=Py_BuildValue("kNNN",
						info->Flags,
						PyWinObject_FromTCHAR(info->pDescription),
						PyWinObject_FromTCHAR(info->pName),
						PyWinObject_FromTCHAR(info->pComment));
					}
				else
					obprinter_info=PyWinObject_FromPRINTER_INFO(buf + i * printer_info_offset[level-1], level);
				if (obprinter_info==NULL){
					Py_DECREF(ret);
					ret=NULL;
					break;
					}
				PyTuple_SET_ITEM(ret, i, obprinter_info);
				}
		}
done:
	PyWinObject_FreeTCHAR(name);
	if (buf)
		free(buf);
	return ret;
}


// @pymethod int|win32print|StartDocPrinter|Notifies the print spooler that a document is to be spooled for printing. To be used before using WritePrinter. Returns the Jobid of the started job.
static PyObject *PyStartDocPrinter(PyObject *self, PyObject *args)
{
	HANDLE hprinter;
	DWORD level= 1;
	TCHAR *pDocName=NULL, *pOutputFile=NULL, *pDatatype=NULL;
	PyObject *obDocName, *obOutputFile, *obDatatype, *ret=NULL;
	DOC_INFO_1 info;
	DWORD JobID;

	if (!PyArg_ParseTuple(args, "O&k(OOO):StartDocPrinter",
	            PyWinObject_AsPrinterHANDLE, &hprinter, // @pyparm <o PyPrinterHANDLE>|hprinter||handle to printer (from <om win32print.OpenPrinter>)
	            &level,     // @pyparm int|level|1|type of docinfo structure (only docinfo level 1 supported)
	            &obDocName, &obOutputFile, &obDatatype // @pyparm data|tuple||A tuple corresponding to the level parameter.
	        ))
		return NULL;
	if (level != 1)
	{
		PyErr_SetString(PyExc_ValueError, "This information level is not supported");
		return NULL;
	}
	// @comm For level 1, the tuple is:
	// @tupleitem 0|string|docName|Specifies the name of the document.
	// @tupleitem 1|string|outputFile|Specifies the name of an output file. To print to a printer, set this to None.
	// @tupleitem 2|string|dataType|Identifies the type of data used to record the document, such 
	// as "raw" or "emf", used to record the print job. This member can be None. If it is not None,
	// the StartDoc function passes it to the printer driver. Note that the printer driver might 
	// ignore the requested data type. 
	if (PyWinObject_AsTCHAR(obDocName, &pDocName, FALSE)
		&&PyWinObject_AsTCHAR(obOutputFile, &pOutputFile, TRUE)
		&&PyWinObject_AsTCHAR(obDatatype, &pDatatype, TRUE)){
		info.pDocName= pDocName;
		info.pOutputFile= pOutputFile;
		info.pDatatype= pDatatype;
		Py_BEGIN_ALLOW_THREADS
		JobID= StartDocPrinter(hprinter, level, (LPBYTE)&info);
		Py_END_ALLOW_THREADS
		if (0 == JobID)
			PyWin_SetAPIError("StartDocPrinter");
		else
			ret = PyLong_FromUnsignedLong(JobID);
		}
	PyWinObject_FreeTCHAR(pDocName);
	PyWinObject_FreeTCHAR(pOutputFile);
	PyWinObject_FreeTCHAR(pDatatype);
	return ret;
}


// @pymethod None|win32print|EndDocPrinter|The EndDocPrinter function ends a print job for the specified printer. To be used after using WritePrinter.
static PyObject *PyEndDocPrinter(PyObject *self, PyObject *args)
{
	HANDLE hprinter;

	if (!PyArg_ParseTuple(args, "O&:EndDocPrinter",
		PyWinObject_AsPrinterHANDLE, &hprinter))  // @pyparm <o PyPrinterHANDLE>|hPrinter||handle to printer (from <om win32print.OpenPrinter>)
		return NULL;

	if (!EndDocPrinter(hprinter))
		return PyWin_SetAPIError("EndDocPrinter");

	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod |win32print|AbortPrinter|Deletes spool file for a printer
static PyObject *PyAbortPrinter(PyObject *self, PyObject *args)
{
	 // @pyparm <o PyPrinterHANDLE>|hPrinter||Handle to printer as returned by <om win32print.OpenPrinter>
	HANDLE hprinter;
	if (!PyArg_ParseTuple(args, "O&:AbortPrinter", PyWinObject_AsPrinterHANDLE, &hprinter))
		return NULL;
	if (!AbortPrinter(hprinter))
		return PyWin_SetAPIError("AbortPrinter");
	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod |win32print|StartPagePrinter|Notifies the print spooler that a page is to be printed on specified printer
static PyObject *PyStartPagePrinter(PyObject *self, PyObject *args)
{
	// @pyparm <o PyPrinterHANDLE>|hprinter||Printer handle as returned by <om win32print.OpenPrinter>
	HANDLE hprinter;
	if (!PyArg_ParseTuple(args, "O&:StartPagePrinter", PyWinObject_AsPrinterHANDLE, &hprinter))
		return NULL;
	if (!StartPagePrinter(hprinter))
		return PyWin_SetAPIError("StartPagePrinter");
	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod |win32print|EndPagePrinter|Ends a page in a print job
static PyObject *PyEndPagePrinter(PyObject *self, PyObject *args)
{
	// @pyparm <o PyPrinterHANDLE>|hprinter||Printer handle as returned by <om win32print.OpenPrinter>
	HANDLE hprinter;
	if (!PyArg_ParseTuple(args, "O&:EndPagePrinter", PyWinObject_AsPrinterHANDLE, &hprinter))
		return NULL;
	if (!EndPagePrinter(hprinter))
		return PyWin_SetAPIError("EndPagePrinter");
	Py_INCREF(Py_None);
	return Py_None;
}

void PyWinObject_FreeDOCINFO(DOCINFO *di)
{
	PyWinObject_FreeTCHAR((TCHAR *)di->lpszDocName);
	PyWinObject_FreeTCHAR((TCHAR *)di->lpszOutput);
	PyWinObject_FreeTCHAR((TCHAR *)di->lpszDatatype);
}

// @object DOCINFO|A tuple of information representing a DOCINFO struct
// @prop string/<o PyUnicode>|DocName|Name of document
// @prop string/<o PyUnicode>|Output|Name of output file when printing to file. Use None for normal printing.
// @prop string/<o PyUnicode>|DataType|Type of data to be sent to printer, eg RAW, EMF, TEXT. Use None for printer default.
// @prop int|Type|Flag specifying mode of operation.  Can be DI_APPBANDING, DI_ROPS_READ_DESTINATION, or 0
BOOL PyWinObject_AsDOCINFO(PyObject *obdocinfo, DOCINFO *di)
{
	PyObject *obDocName, *obOutput, *obDataType;
	ZeroMemory(di, sizeof(*di));
	if (!PyTuple_Check(obdocinfo)){
		PyErr_SetString(PyExc_TypeError,"DOCINFO must be a tuple");
		return FALSE;
		}
	di->cbSize=sizeof(DOCINFO);
	return PyArg_ParseTuple(obdocinfo, "OOOk", &obDocName, &obOutput, &obDataType, &di->fwType)
		&&PyWinObject_AsTCHAR(obDocName,	(TCHAR **)&di->lpszDocName, TRUE)
		&&PyWinObject_AsTCHAR(obOutput,		(TCHAR **)&di->lpszOutput, TRUE)
		&&PyWinObject_AsTCHAR(obDataType,	(TCHAR **)&di->lpszDatatype, TRUE);
}

// @pymethod int|win32print|StartDoc|Starts spooling a print job on a printer device context
static PyObject *PyStartDoc(PyObject *self, PyObject *args)
{
	// @pyparm <o PyHANDLE>|hdc||Printer device context handle as returned by <om win32gui.CreateDC>
	// @pyparm tuple|docinfo||<o DOCINFO> tuple specifying print job parameters
	// @rdesc On success, returns the job id of the print job
	HDC hdc;
	DOCINFO docinfo={0};
	int jobid;
	PyObject *obdocinfo;
	if (!PyArg_ParseTuple(args, "O&O:StartDoc", PyWinObject_AsPrinterHANDLE, &hdc, &obdocinfo))
		return NULL;
	if (!PyWinObject_AsDOCINFO(obdocinfo, &docinfo))
		return NULL;
	jobid=StartDoc(hdc, &docinfo);
	PyWinObject_FreeDOCINFO(&docinfo);
	if (jobid > 0)
		return PyLong_FromUnsignedLong(jobid);
	return PyWin_SetAPIError("StartDoc");
}

// @pymethod |win32print|EndDoc|Stops spooling a print job on a printer device context
static PyObject *PyEndDoc(PyObject *self, PyObject *args)
{
	// @pyparm <o PyHANDLE>|hdc||Printer device context handle as returned by <om win32gui.CreateDC>
	HDC hdc;
	int err;
	if (!PyArg_ParseTuple(args, "O&:EndDoc", PyWinObject_AsPrinterHANDLE, &hdc))
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
	// @pyparm <o PyHANDLE>|hdc||Printer device context handle as returned by <om win32gui.CreateDC>
	HDC hdc;
	int err;
	if (!PyArg_ParseTuple(args, "O&:AbortDoc", PyWinObject_AsPrinterHANDLE, &hdc))
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
	// @pyparm <o PyHANDLE>|hdc||Printer device context handle as returned by <om win32gui.CreateDC>
	HDC hdc;
	int err;
	if (!PyArg_ParseTuple(args, "O&:StartPage", PyWinObject_AsPrinterHANDLE, &hdc))
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
	// @pyparm <o PyHANDLE>|hdc||Printer device context handle as returned by <om win32gui.CreateDC>
	HDC hdc;
	int err;
	if (!PyArg_ParseTuple(args, "O&:EndPage", PyWinObject_AsPrinterHANDLE, &hdc))
		return NULL;
	err=EndPage(hdc);
	if (err > 0){
		Py_INCREF(Py_None);
		return Py_None;
		}
	return PyWin_SetAPIError("EndPage");
}

// @pymethod int|win32print|WritePrinter|Copies the specified bytes to the specified printer.
// Suitable for copying raw Postscript or HPGL files to a printer.
// StartDocPrinter and EndDocPrinter should be called before and after.
// @rdesc Returns number of bytes written to printer.
static PyObject *PyWritePrinter(PyObject *self, PyObject *args)
{
	HANDLE hprinter;
	LPVOID buf;
	DWORD buf_size;
	DWORD bufwritten_size;
	PyObject *obbuf;
	if (!PyArg_ParseTuple(args, "O&O:WritePrinter",
		PyWinObject_AsPrinterHANDLE, &hprinter,  // @pyparm <o PyPrinterHANDLE>|hprinter||Handle to printer as returned by <om win32print.OpenPrinter>.
		&obbuf))       // @pyparm string|buf||String or buffer containing data to send to printer. Embedded NULL bytes are allowed.  
		return NULL;
	if (!PyWinObject_AsReadBuffer(obbuf, &buf, &buf_size, FALSE))
		return NULL;
	if (!WritePrinter(hprinter, buf, buf_size, &bufwritten_size))
		return PyWin_SetAPIError("WritePrinter");
	return PyLong_FromUnsignedLong(bufwritten_size);
}


// convert a job structure to python. only works for level 1
PyObject *JobtoPy(DWORD level, LPBYTE buf)
{
	JOB_INFO_1 *job1;
	JOB_INFO_2 *job2;
	JOB_INFO_3 *job3;
	PyObject *ret;
	switch (level){
		case 1:{
			job1= (JOB_INFO_1 *)buf;
			ret= Py_BuildValue("{s:k, s:N, s:N, s:N, s:N, s:N, s:N, s:k, s:k, s:k, s:k, s:k, s:N}",
					"JobId", job1->JobId,
					"pPrinterName", PyWinObject_FromTCHAR(job1->pPrinterName),
					"pMachineName", PyWinObject_FromTCHAR(job1->pMachineName),
					"pUserName", PyWinObject_FromTCHAR(job1->pUserName),
					"pDocument", PyWinObject_FromTCHAR(job1->pDocument),
					"pDatatype", PyWinObject_FromTCHAR(job1->pDatatype),
					"pStatus", PyWinObject_FromTCHAR(job1->pStatus),
					"Status", job1->Status,
					"Priority", job1->Priority,
					"Position", job1->Position,
					"TotalPages", job1->TotalPages,
					"PagesPrinted", job1->PagesPrinted,
					"Submitted", PyWinObject_FromSYSTEMTIME(job1->Submitted));
			return ret;
			}
		case 2:{
			job2=(JOB_INFO_2 *)buf;
			ret= Py_BuildValue("{s:k, s:N, s:N, s:N, s:N, s:N, s:N, s:N, s:N, s:N, s:N, s:N, s:N, s:k, s:k, s:k, s:k, s:k, s:k, s:k, s:N, s:k, s:k}",
					"JobId", job2->JobId,
					"pPrinterName", PyWinObject_FromTCHAR(job2->pPrinterName),
					"pMachineName", PyWinObject_FromTCHAR(job2->pMachineName),
					"pUserName", PyWinObject_FromTCHAR(job2->pUserName),
					"pDocument", PyWinObject_FromTCHAR(job2->pDocument),
					"pNotifyName", PyWinObject_FromTCHAR(job2->pNotifyName),
					"pDatatype", PyWinObject_FromTCHAR(job2->pDatatype),
					"pPrintProcessor", PyWinObject_FromTCHAR(job2->pPrintProcessor),
					"pParameters", PyWinObject_FromTCHAR(job2->pParameters),
					"pDriverName", PyWinObject_FromTCHAR(job2->pDriverName),
					"pDevMode", PyWinObject_FromDEVMODE(job2->pDevMode),
					"pStatus", PyWinObject_FromTCHAR(job2->pStatus),
					"pSecurityDescriptor", PyWinObject_FromSECURITY_DESCRIPTOR(job2->pSecurityDescriptor),
					"Status", job2->Status,
					"Priority", job2->Priority,
					"Position", job2->Position,
					"StartTime", job2->StartTime,
					"UntilTime", job2->UntilTime,
					"TotalPages", job2->TotalPages,
					"Size", job2->Size,
					"Submitted", PyWinObject_FromSYSTEMTIME(job2->Submitted),
					"Time", job2->Time,
					"PagesPrinted", job2->PagesPrinted);
			return ret;
			}
	   	case 3:{
			job3=(JOB_INFO_3 *)buf;
			ret=Py_BuildValue("{s:k, s:k, s:k}",
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
	if (!PyArg_ParseTuple(args, "O&kk|k:EnumJobs",
		PyWinObject_AsPrinterHANDLE, &hprinter,   // @pyparm <o PyPrinterHANDLE>|hPrinter||Handle of printer.
		&firstjob,   // @pyparm int|FirstJob||location of first job in print queue to enumerate.
		&nojobs,     // @pyparm int|NoJobs||Number of jobs to enumerate.
		&level       // @pyparm int|Level|1|Level of information to return (JOB_INFO_1, JOB_INFO_2, JOB_INFO_3 supported).
	          ))
		return NULL;
	if ((level < 1)||(level > 3))
		return PyErr_Format(PyExc_ValueError, "Information level %d is not supported", level);
	if (EnumJobs(hprinter, firstjob, nojobs, level, NULL, 0, &bufneeded_size, &jobsreturned))
		return PyTuple_New(0);
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

	if (!PyArg_ParseTuple(args, "O&k|k:GetJob",
		PyWinObject_AsPrinterHANDLE, &hprinter,	// @pyparm <o PyPrinterHANDLE>|hPrinter||Handle to a printer as returned by <om win32print.OpenPrinter>.
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
	static char *job1_format="kzzzzzzkkkkk|O:JOB_INFO_1";

	static char *job2_keys[]={"JobId","pPrinterName","pMachineName","pUserName","pDocument","pNotifyName",
		"pDatatype","pPrintProcessor","pParameters","pDriverName","pDevMode","pStatus","pSecurityDescriptor",
		"Status","Priority","Position","StartTime","UntilTime","TotalPages","Size",
		"Submitted","Time","PagesPrinted", NULL};
	static char *job2_format="kzzzzzzzzzOzOkkkkkkkOkk:JOB_INFO_2";

	static char *job3_keys[]={"JobId","NextJobId","Reserved", NULL};
	static char *job3_format="kk|k:JOB_INFO_3";

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

	if (!PyArg_ParseTuple(args, "O&kkOk:SetJob",
	    PyWinObject_AsPrinterHANDLE, &hprinter,	// @pyparm <o PyPrinterHANDLE>|hPrinter||Handle of printer.
	    &jobid,   // @pyparm int|JobID||Job Identifier.
	    &level,   // @pyparm int|Level||Level of information in JobInfo dict (0, 1, 2, and 3 are supported).
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
// @rdesc If DM_IN_PROMPT is specified, returned value will be IDOK or IDCANCEL
static PyObject *PyDocumentProperties(PyObject *self, PyObject *args)
{
	long rc;
	HANDLE hprinter;
	HWND hwnd;
	TCHAR *devicename=NULL;
	PDEVMODE dmoutput, dminput;
	PyObject *obdmoutput, *obdminput, *obhwnd, *obdevicename, *ret=NULL;
	DWORD mode;
	// @pyparm <o PyHANDLE>|HWnd||Parent window handle to use if DM_IN_PROMPT is specified to display printer dialog
	// @pyparm <o PyPrinterHANDLE>|hPrinter||Printer handle as returned by <om win32print.OpenPrinter>
	// @pyparm string|DeviceName||Name of printer
	// @pyparm <o PyDEVMODE>|DevModeOutput||PyDEVMODE object that receives modified info, can be None if DM_OUT_BUFFER not specified
	// @pyparm <o PyDEVMODE>|DevModeInput||PyDEVMODE that specifies initial configuration, can be None if DM_IN_BUFFER not specified
	// @pyparm int|Mode||A combination of DM_IN_BUFFER, DM_OUT_BUFFER, and DM_IN_PROMPT - pass 0 to retrieve driver data size
	if (!PyArg_ParseTuple(args,"OO&OOOk:DocumentProperties", &obhwnd, 
		PyWinObject_AsPrinterHANDLE, &hprinter, 
		&obdevicename, &obdmoutput, &obdminput, &mode))
		return NULL;
	if (PyWinObject_AsTCHAR(obdevicename, &devicename, FALSE)
		&&PyWinObject_AsHANDLE(obhwnd, (HANDLE *)&hwnd)
		&&PyWinObject_AsDEVMODE(obdmoutput, &dmoutput, TRUE)
		&&PyWinObject_AsDEVMODE(obdminput, &dminput, TRUE)){
		rc=DocumentProperties(hwnd, hprinter, devicename, dmoutput, dminput, mode);
		if (rc < 0)
			PyWin_SetAPIError("DocumentProperties");
		else{
			if (obdmoutput!=Py_None)
				((PyDEVMODE *)obdmoutput)->modify_in_place();
			ret = PyInt_FromLong(rc);
			}
		}
	PyWinObject_FreeTCHAR(devicename);
	return ret;
}

// @pymethod (<o PyUnicode>,...)|win32print|EnumPrintProcessors|List printer processors for specified server and environment
static PyObject *PyEnumPrintProcessors(PyObject *self, PyObject *args)
{
	PRINTPROCESSOR_INFO_1W *info=NULL; // currently only level that exists
	LPBYTE buf=NULL;
	WCHAR *servername=NULL, *environment=NULL;
	PyObject *observername=Py_None, *obenvironment=Py_None;
	DWORD level=1, bufsize=0, bytes_needed, return_cnt;
	PyObject *ret=NULL, *tuple_item;
	// @pyparm string/<o PyUnicode>|Server|None|Name of print server, use None for local machine
	// @pyparm string/<o PyUnicode>|Environment|None|Environment - eg 'Windows NT x86' - use None for current client environment
	if (!PyArg_ParseTuple(args,"|OO:EnumPrintProcessors", &observername, &obenvironment))
		return NULL;
	if (!PyWinObject_AsWCHAR(observername, &servername, TRUE))
		goto done;
	if (!PyWinObject_AsWCHAR(obenvironment, &environment, TRUE))
		goto done;
	if (EnumPrintProcessorsW(servername, environment, level, buf, bufsize, &bytes_needed, &return_cnt)){
		ret=PyTuple_New(0);
		goto done;
		}
	if (bytes_needed==0){
		PyWin_SetAPIError("EnumPrintProcessors");
		goto done;
		}
	buf=(LPBYTE)malloc(bytes_needed);
	if (buf==NULL){
		PyErr_Format(PyExc_MemoryError,"EnumPrintProcessors: unable to allocate buffer of size %d", bytes_needed);
		goto done;
		}
	bufsize=bytes_needed;
	if (!EnumPrintProcessorsW(servername, environment, level, buf, bufsize, &bytes_needed, &return_cnt))
		PyWin_SetAPIError("EnumPrintProcessors");
	else{
		ret=PyTuple_New(return_cnt);
		if (ret!=NULL){
			info=(PRINTPROCESSOR_INFO_1W *)buf;
			for (DWORD buf_ind=0; buf_ind<return_cnt; buf_ind++){
				tuple_item=PyWinObject_FromWCHAR(info->pName);
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
done:
	if (buf!=NULL)
		free(buf);
	if (servername!=NULL)
		PyWinObject_FreeWCHAR(servername);
	if (environment!=NULL)
		PyWinObject_FreeWCHAR(environment);
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
	if (!PyArg_ParseTuple(args,"|OOk:EnumPrinterDrivers", &observername, &obenvironment, &level))
		return NULL;
	if (!PyWinObject_AsWCHAR(observername, &servername, TRUE))
		goto done;
	if (!PyWinObject_AsWCHAR(obenvironment, &environment, TRUE))
		goto done;

	if (EnumPrinterDriversW(servername, environment, level, buf, bufsize, &bytes_needed, &return_cnt)){
		ret=PyTuple_New(0);
		goto done;
		}
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
		PyWin_SetAPIError("EnumPrinterDrivers");
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
				tuple_item=Py_BuildValue("{s:l,s:u,s:u,s:u,s:u,s:u,s:u,s:N,s:u,s:u}",
					"Version",di3->cVersion,
					"Name",di3->pName,
					"Environment",di3->pEnvironment,
					"DriverPath",di3->pDriverPath,
					"DataFile",di3->pDataFile,
					"ConfigFile",di3->pConfigFile,
					"HelpFile", di3->pHelpFile,
					"DependentFiles",PyWinObject_FromMultipleString(di3->pDependentFiles),
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
				tuple_item=Py_BuildValue("{s:l,s:u,s:u,s:u,s:u,s:u,s:u,s:N,s:u,s:u,s:u}",
					"Version",di4->cVersion,
					"Name",di4->pName,
					"Environment",di4->pEnvironment,
					"DriverPath",di4->pDriverPath,
					"DataFile",di4->pDataFile,
					"ConfigFile",di4->pConfigFile,
					"HelpFile", di4->pHelpFile,
					"DependentFiles",PyWinObject_FromMultipleString(di4->pDependentFiles),
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
				tuple_item=Py_BuildValue("{s:l,s:u,s:u,s:u,s:u,s:u,s:u,s:N,s:u,s:u,s:u,s:N,s:L,s:u,s:u,s:u}",
					"Version",di6->cVersion,
					"Name",di6->pName,
					"Environment",di6->pEnvironment,
					"DriverPath",di6->pDriverPath,
					"DataFile",di6->pDataFile,
					"ConfigFile",di6->pConfigFile,
					"HelpFile", di6->pHelpFile,
					"DependentFiles",PyWinObject_FromMultipleString(di6->pDependentFiles),
					"MonitorName",di6->pMonitorName,
					"DefaultDataType",di6->pDefaultDataType,
					"PreviousNames",di6->pszzPreviousNames,
					"DriverDate", PyWinObject_FromFILETIME(di6->ftDriverDate),
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
	return Py_BuildValue("{s:k,s:u,s:{s:l,s:l},s:{s:l,s:l,s:l,s:l}}",
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
	// @pyparm <o PyPrinterHANDLE>|hprinter||Printer handle as returned by <om win32print.OpenPrinter>
	// @rdesc Returns a sequence of dictionaries representing FORM_INFO_1 structures
	PyObject *ret=NULL, *tuple_item;
	HANDLE hprinter;
	DWORD level=1, bufsize=0, bytes_needed=0, return_cnt, buf_ind;
	FORM_INFO_1W *fi1;
	LPBYTE buf=NULL;
	CHECK_PFN(EnumForms);

	if (!PyArg_ParseTuple(args,"O&:EnumForms", PyWinObject_AsPrinterHANDLE, &hprinter))
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
	return PyArg_ParseTupleAndKeywords(dummy_tuple, obform, "kuO&O&:FORM_INFO_1", form_keys, &fi1->Flags, &fi1->pName, 
		PyWinObject_AsSIZEL, &fi1->Size, PyWinObject_AsRECTL, &fi1->ImageableArea);
}

// @pymethod |win32print|AddForm|Adds a form for a printer
static PyObject *PyAddForm(PyObject *self, PyObject *args)
{
	// @pyparm <o PyPrinterHANDLE>|hprinter||Printer handle as returned by <om win32print.OpenPrinter>
	// @pyparm dict|Form||<o FORM_INFO_1> dictionary
	// @rdesc Returns None on success, throws an exception otherwise
	FORM_INFO_1W fi1;
	HANDLE hprinter;
	CHECK_PFN(AddForm);

	if (!PyArg_ParseTuple(args, "O&O&:AddForm", 
		PyWinObject_AsPrinterHANDLE, &hprinter, 
		PyWinObject_AsFORM_INFO_1, &fi1))
		return NULL;
	if (!(*pfnAddForm)(hprinter, 1, (LPBYTE)&fi1))
		return PyWin_SetAPIError("AddForm");
	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod |win32print|DeleteForm|Deletes a form defined for a printer
static PyObject *PyDeleteForm(PyObject *self, PyObject *args)
{
	// @pyparm <o PyPrinterHANDLE>|hprinter||Printer handle as returned by <om win32print.OpenPrinter>
	// @pyparm <o PyUnicode>|FormName||Name of form to be deleted
	// @rdesc Returns None on success, throws an exception otherwise
	HANDLE hprinter;
	WCHAR *formname;
	CHECK_PFN(DeleteForm);

	if (!PyArg_ParseTuple(args, "O&u:DeleteForm", PyWinObject_AsPrinterHANDLE, &hprinter, &formname))
		return NULL;
	if (!(*pfnDeleteForm)(hprinter, formname))
		return PyWin_SetAPIError("DeleteForm");
	Py_INCREF(Py_None);
	return Py_None;
}
// @pymethod |win32print|GetForm|Retrieves information about a form defined for a printer
static PyObject *PyGetForm(PyObject *self, PyObject *args)
{
	// @pyparm <o PyPrinterHANDLE>|hprinter||Printer handle as returned by <om win32print.OpenPrinter>
	// @pyparm <o PyUnicode>|FormName||Name of form for which to retrieve info
	// @rdesc Returns a <o FORM_INFO_1> dict
	HANDLE hprinter;
	WCHAR *formname;
	DWORD level=1, bufsize=0, bytes_needed=0;
	FORM_INFO_1W *fi1=NULL;
	LPBYTE buf=NULL;
	PyObject *ret=NULL;
	CHECK_PFN(GetForm);

	if (!PyArg_ParseTuple(args,"O&u:GetForm", PyWinObject_AsPrinterHANDLE, &hprinter, &formname))
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
	// @pyparm <o PyPrinterHANDLE>|hprinter||Printer handle as returned by <om win32print.OpenPrinter>
	// @pyparm <o PyUnicode>|FormName||Name of form
	// @pyparm dict|Form||<o FORM_INFO_1> dictionary
	// @rdesc Returns None on success
	FORM_INFO_1W fi1;
	HANDLE hprinter;
	WCHAR *formname;
	CHECK_PFN(SetForm);

	if (!PyArg_ParseTuple(args, "O&uO&:SetForm",
		PyWinObject_AsPrinterHANDLE, &hprinter, 
		&formname, 
		PyWinObject_AsFORM_INFO_1, &fi1))
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
	// @pyparm <o PyPrinterHANDLE>|hprinter||Printer handle as returned by <om win32print.OpenPrinter>
	HANDLE hprinter;
	DWORD level=1, bufsize, bytes_needed;
	LPBYTE buf=NULL;
	PyObject *ret=NULL;
	BOOL bsuccess;
	CHECK_PFN(AddJob);

	if (!PyArg_ParseTuple(args,"O&:AddJob", PyWinObject_AsPrinterHANDLE, &hprinter))
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
		ret=Py_BuildValue("uk",((ADDJOB_INFO_1 *)buf)->Path,((ADDJOB_INFO_1 *)buf)->JobId);
	if (buf!=NULL)
		free(buf);
	return ret;
}

// @pymethod |win32print|ScheduleJob|Schedules a spooled job to be printed
static PyObject *PyScheduleJob(PyObject *self, PyObject *args)
{
	// @pyparm <o PyPrinterHANDLE>|hprinter||Printer handle as returned by <om win32print.OpenPrinter>
	// @pyparm int|JobId||Job Id as returned by <om win32print.AddJob>
	HANDLE hprinter;
	DWORD jobid;
	CHECK_PFN(ScheduleJob);

	if (!PyArg_ParseTuple(args,"O&k:ScheduleJob", PyWinObject_AsPrinterHANDLE, &hprinter, &jobid))
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
	TCHAR *device=NULL, *port=NULL;
	PyObject *obdevice, *obport;
	WORD capability;
	LPTSTR buf=NULL;
	PDEVMODE pdevmode;
	PyObject *obdevmode=Py_None, *ret=NULL, *tuple_item;
	DWORD result, bufsize, bufindex;
	static DWORD papernamesize=64; // same for DC_PAPERNAMES, DC_MEDIATYPENAMES, DC_MEDIAREADY, DC_FILEDEPENDENCIES
	static DWORD binnamesize=24; // DC_BINNAMES
	static DWORD personalitysize=32; // DC_PERSONALITY
	DWORD retsize;

	if (!PyArg_ParseTuple(args,"OOh|O:DeviceCapabilities", &obdevice, &obport, &capability, &obdevmode))
		return NULL;
	if (!PyWinObject_AsTCHAR(obdevice, &device, FALSE))
		goto done;
	if (!PyWinObject_AsTCHAR(obport, &port, FALSE))
		goto done;
	if (!PyWinObject_AsDEVMODE(obdevmode, &pdevmode, TRUE))
		goto done;
	result=DeviceCapabilities(device,port,capability,buf,pdevmode);
	if (result==-1){
		PyWin_SetAPIError("DeviceCapabilities");
		goto done;
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
		case DC_ORIENTATION:
		case DC_PRINTRATE:
		case DC_PRINTRATEPPM:
		case DC_PRINTRATEUNIT:
		case DC_PRINTERMEM:
		case DC_SIZE:
		case DC_STAPLE:
		case DC_TRUETYPE:
		case DC_VERSION:
			ret=Py_BuildValue("k",result);
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
		// @flag DC_NUP|Sequence of ints containing supported logical page per physical page settings
		case DC_NUP:
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
				tuple_item=PyLong_FromUnsignedLong(*pdword++);
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
			TCHAR *retname;
			if (capability==DC_BINNAMES)
				retsize=binnamesize;
			else if (capability==DC_PERSONALITY)
				retsize=personalitysize;
			else
				retsize=papernamesize;
			bufsize=result*retsize*sizeof(TCHAR);
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
			retname=(TCHAR *)buf;
			for (bufindex=0;bufindex<result;bufindex++){
				if (*(retname+retsize-1)==0)
					tuple_item=PyWinObject_FromTCHAR(retname);
				else  // won't be null-terminated if string occupies entire space
					tuple_item=PyWinObject_FromTCHAR(retname,retsize);
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
done:
	if (buf!=NULL)
		free(buf);
	PyWinObject_FreeTCHAR(device);
	PyWinObject_FreeTCHAR(port);
	return ret;
}

// @pymethod int|win32print|GetDeviceCaps|Retrieves device-specific parameters and settings
// @comm Can also be used for Display DCs in addition to printer DCs
// @pyseeapi GetDeviceCaps
static PyObject *PyGetDeviceCaps(PyObject *self, PyObject *args)
{
	PyObject *obdc;
	DWORD index;
	int ret;
	HDC hdc;
	if (!PyArg_ParseTuple(args, "Ok",
		&obdc,		// @pyparm <o PyHANDLE>|hdc||Handle to a printer or display device context
		&index))	// @pyparm int|Index||The capability to return.  See MSDN for valid values.
		return NULL;
	if (!PyWinObject_AsHANDLE(obdc, (HANDLE *)&hdc))
		return NULL;
	ret=GetDeviceCaps(hdc, index);
	return Py_BuildValue("i", ret);
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

	if (!PyArg_ParseTuple(args,"Ok:EnumMonitors", &observer_name, &level))
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

	if (!PyArg_ParseTuple(args,"Ok:EnumPorts", &observer_name, &level))
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

// @pymethod <o PyPrinterHANDLE>|win32print|AddPrinter|Installs a printer on a server
// @rdesc Returns a handle to the new printer
static PyObject *PyAddPrinter(PyObject *self, PyObject *args)
{
	HANDLE hprinter;
	LPBYTE buf=NULL;
	DWORD level;
	PyObject *obinfo;
	TCHAR *server_name=NULL;
	PyObject *observer_name, *ret=NULL;
	// @pyparm string|Name||Name of server on which to install printer, None indicates local machine
	// @pyparm int|Level||Level of data contained in pPrinter, only level 2 currently supported
	// @pyparm dict|pPrinter||PRINTER_INFO_2 dict as returned by <om win32print.GetPrinter>
	// @comm pPrinterName, pPortName, pDriverName, and pPrintProcessor are required
	if (!PyArg_ParseTuple(args, "OkO:AddPrinter", &observer_name, &level, &obinfo))
		return NULL;
	if (level!=2){
		PyErr_SetString(PyExc_ValueError,"AddPrinter only accepts level 2");
		return NULL;
		}
	if (PyWinObject_AsPRINTER_INFO(level, obinfo, &buf)
		&&PyWinObject_AsTCHAR(observer_name, &server_name, TRUE)){
		hprinter=AddPrinter(server_name, level, buf);
		if (hprinter==NULL)
			PyWin_SetAPIError("AddPrinter");
		else
			ret = PyWinObject_FromPrinterHANDLE(hprinter);
		}
	PyWinObject_FreePRINTER_INFO(level, buf);
	PyWinObject_FreeTCHAR(server_name);
	return ret;
}

// @pymethod |win32print|DeletePrinter|Deletes an existing printer
// @comm Printer handle must be opened for PRINTER_ACCESS_ADMINISTER
// If there are any pending print jobs for the printer, actual deletion does not happen until they are done
static PyObject *PyDeletePrinter(PyObject *self, PyObject *args)
{
	// @pyparm <o PyPrinterHANDLE>|hPrinter||Handle to printer as returned by <om win32print.OpenPrinter> or <om win32print.AddPrinter>
	HANDLE hprinter;
	if (!PyArg_ParseTuple(args, "O&:DeletePrinter", PyWinObject_AsPrinterHANDLE, &hprinter))
		return NULL;
	if (!DeletePrinter(hprinter)){
		PyWin_SetAPIError("DeletePrinter");
		return NULL;
		}
	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod |win32print|DeletePrinterDriver|Removes the specified printer driver from a server
static PyObject *PyDeletePrinterDriver(PyObject *self, PyObject *args)
{
	PyObject *ret=NULL;
	PyObject *observername, *obenvironment, *obdrivername;
	WCHAR *servername=NULL, *environment=NULL, *drivername=NULL;
	// @pyparm string/<o PyUnicode>|Server||Name of print server, use None for local machine
	// @pyparm string/<o PyUnicode>|Environment||Environment - eg 'Windows NT x86' - use None for current client environment
	// @pyparm string/<o PyUnicode>|DriverName||Name of driver to remove
	// @comm Does not delete associated driver files - use <om win32print.DeletePrinterDriverEx> if this is required
	if (PyArg_ParseTuple(args,"OOO:DeletePrinterDriver", &observername, &obenvironment, &obdrivername)
		&&PyWinObject_AsWCHAR(observername, &servername, TRUE)
		&&PyWinObject_AsWCHAR(obenvironment, &environment, TRUE)
		&&PyWinObject_AsWCHAR(obdrivername, &drivername, FALSE))
		if (DeletePrinterDriverW(servername, environment, drivername)){
			Py_INCREF(Py_None);
			ret=Py_None;
			}
		else
			PyWin_SetAPIError("DeletePrinterDriver");

	if (servername!=NULL)
		PyWinObject_FreeWCHAR(servername);
	if (environment!=NULL)
		PyWinObject_FreeWCHAR(environment);
	if (drivername!=NULL)
		PyWinObject_FreeWCHAR(drivername);
	return ret;
}

// @pymethod |win32print|DeletePrinterDriverEx|Deletes a printer driver and its associated files
static PyObject *PyDeletePrinterDriverEx(PyObject *self, PyObject *args)
{
	PyObject *ret=NULL;
	PyObject *observername, *obenvironment, *obdrivername;
	WCHAR *servername=NULL, *environment=NULL, *drivername=NULL;
	DWORD deleteflag, versionflag;
	CHECK_PFN(DeletePrinterDriverEx);
	// @pyparm string/<o PyUnicode>|Server||Name of print server, use None for local machine
	// @pyparm string/<o PyUnicode>|Environment||Environment - eg 'Windows NT x86' - use None for current client environment
	// @pyparm string/<o PyUnicode>|DriverName||Name of driver to remove
	// @pyparm int|DeleteFlag||Combination of DPD_DELETE_SPECIFIC_VERSION, DPD_DELETE_UNUSED_FILES, and DPD_DELETE_ALL_FILES
	// @pyparm int|VersionFlag||Can be 0,1,2, or 3.  Only used if DPD_DELETE_SPECIFIC_VERSION is specified in DeleteFlag
	if (PyArg_ParseTuple(args,"OOOll:DeletePrinterDriverEx", &observername, &obenvironment, &obdrivername,
		&deleteflag, &versionflag)
		&&PyWinObject_AsWCHAR(observername, &servername, TRUE)
		&&PyWinObject_AsWCHAR(obenvironment, &environment, TRUE)
		&&PyWinObject_AsWCHAR(obdrivername, &drivername, FALSE))
		if ((*pfnDeletePrinterDriverEx)(servername, environment, drivername, deleteflag, versionflag)){
			Py_INCREF(Py_None);
			ret=Py_None;
			}
		else
			PyWin_SetAPIError("DeletePrinterDriverEx");

	if (servername!=NULL)
		PyWinObject_FreeWCHAR(servername);
	if (environment!=NULL)
		PyWinObject_FreeWCHAR(environment);
	if (drivername!=NULL)
		PyWinObject_FreeWCHAR(drivername);
	return ret;
}

// @pymethod int|win32print|FlushPrinter|Clears printer from error state if WritePrinter fails
// @rdesc Returns the number of bytes actually written to the printer
static PyObject *PyFlushPrinter(PyObject *self, PyObject *args)
{
	CHECK_PFN(FlushPrinter);
	HANDLE hprinter;
	PyObject *obbuf;
	void *buf;
	Py_ssize_t bufsize;
	DWORD bytes_written=0, sleep_ms;
	if (!PyArg_ParseTuple(args, "O&Ok",
		PyWinObject_AsPrinterHANDLE, &hprinter,	// @pyparm <o PyPrinterHANDLE>|Printer||Handle to a printer
		&obbuf,									// @pyparm str|Buf||Data to be sent to printer
		&sleep_ms))								// @pyparm int|Sleep||Number of milliseconds to suspend printer
		return NULL;
	if (PyString_AsStringAndSize(obbuf, (char **)&buf, &bufsize)==-1)
		return NULL;
	if (!(*pfnFlushPrinter)(hprinter, buf, 
	                        PyWin_SAFE_DOWNCAST(bufsize, Py_ssize_t, DWORD),
	                        &bytes_written, sleep_ms))
		return PyWin_SetAPIError("FlushPrinter");
	return PyLong_FromUnsignedLong(bytes_written);
}


/* List of functions exported by this module */
// @module win32print|A module encapsulating the Windows printing API.
static struct PyMethodDef win32print_functions[] = {
	{"OpenPrinter",				PyOpenPrinter, 1}, // @pymeth OpenPrinter|Retrieves a handle to a printer.
	{"GetPrinter",				PyGetPrinter       ,1}, // @pymeth GetPrinter|Retrieves information about a printer
	{"SetPrinter",				PySetPrinter, 1}, // @pymeth SetPrinter|Changes printer configuration and status
	{"ClosePrinter",			PyClosePrinter,     1}, // @pymeth ClosePrinter|Closes a handle to a printer.
	{"AddPrinterConnection",	PyAddPrinterConnection, 1}, // @pymeth AddPrinterConnection|Connects to a network printer.
	{"DeletePrinterConnection",	PyDeletePrinterConnection, 1}, // @pymeth DeletePrinterConnection|Disconnects from a network printer.
	{"EnumPrinters",			PyEnumPrinters, 1}, // @pymeth EnumPrinters|Enumerates printers, print servers, domains and print providers.
	{"GetDefaultPrinter",		PyGetDefaultPrinter, METH_NOARGS}, // @pymeth GetDefaultPrinter|Returns the default printer.
	{"GetDefaultPrinterW",		PyGetDefaultPrinterW, METH_NOARGS}, // @pymeth GetDefaultPrinterW|Returns the default printer.
	{"SetDefaultPrinter",		PySetDefaultPrinter, 1}, // @pymeth SetDefaultPrinter|Sets the default printer.
	{"SetDefaultPrinterW",		PySetDefaultPrinterW, 1}, // @pymeth SetDefaultPrinterW|Sets the default printer.
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
	{"GetDeviceCaps", PyGetDeviceCaps, METH_VARARGS}, //@pymeth GetDeviceCaps|Retrieves device-specific parameters and settings
	{"EnumMonitors", PyEnumMonitors, 1}, //@pymeth EnumMonitors|Lists installed printer port monitors
	{"EnumPorts", PyEnumPorts, 1}, //@pymeth EnumPorts|Lists printer ports on a server
	{"GetPrintProcessorDirectory", PyGetPrintProcessorDirectory, 1}, //@pymeth GetPrintProcessorDirectory|Returns the directory where print processor files reside
	{"GetPrinterDriverDirectory", PyGetPrinterDriverDirectory, 1}, //@pymeth GetPrinterDriverDirectory|Returns the directory where printer drivers are installed
	{"AddPrinter", PyAddPrinter, 1}, //@pymeth AddPrinter|Adds a new printer on a server
	{"DeletePrinter", PyDeletePrinter, 1}, //@pymeth DeletePrinter|Deletes an existing printer
	{"DeletePrinterDriver", PyDeletePrinterDriver,1}, //@pymeth DeletePrinterDriver|Deletes the specified driver from a server
	{"DeletePrinterDriverEx", PyDeletePrinterDriverEx,1}, //@pymeth DeletePrinterDriverEx|Deletes a printer driver and associated files
	{"FlushPrinter", PyFlushPrinter,1}, //@pymeth FlushPrinter|Clears printer from error state if WritePrinter fails
	{ NULL }
};


static void AddConstant(PyObject *dict, char *name, long val)
{
  PyObject *nv = PyInt_FromLong(val);
  PyDict_SetItemString(dict, name, nv );
  Py_XDECREF(nv);
}


PYWIN_MODULE_INIT_FUNC(win32print)
{
  PYWIN_MODULE_INIT_PREPARE(win32print, win32print_functions,
                            "A module encapsulating the Windows printing API.")

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

  // Status member of JOB_INFO_1 and JOB_INFO_2
  AddConstant(dict, "JOB_STATUS_DELETING", JOB_STATUS_DELETING);
  AddConstant(dict, "JOB_STATUS_ERROR", JOB_STATUS_ERROR);
  AddConstant(dict, "JOB_STATUS_OFFLINE", JOB_STATUS_OFFLINE);
  AddConstant(dict, "JOB_STATUS_PAPEROUT", JOB_STATUS_PAPEROUT);
  AddConstant(dict, "JOB_STATUS_PAUSED", JOB_STATUS_PAUSED);
  AddConstant(dict, "JOB_STATUS_PRINTED", JOB_STATUS_PRINTED);
  AddConstant(dict, "JOB_STATUS_PRINTING", JOB_STATUS_PRINTING);
  AddConstant(dict, "JOB_STATUS_SPOOLING", JOB_STATUS_SPOOLING);
  AddConstant(dict, "JOB_STATUS_DELETED", JOB_STATUS_DELETED);
  AddConstant(dict, "JOB_STATUS_BLOCKED_DEVQ", JOB_STATUS_BLOCKED_DEVQ);
  AddConstant(dict, "JOB_STATUS_USER_INTERVENTION", JOB_STATUS_USER_INTERVENTION);
  AddConstant(dict, "JOB_STATUS_RESTART", JOB_STATUS_RESTART);
  AddConstant(dict, "JOB_STATUS_COMPLETE", JOB_STATUS_COMPLETE);

  AddConstant(dict, "MIN_PRIORITY", MIN_PRIORITY);
  AddConstant(dict, "MAX_PRIORITY", MAX_PRIORITY);
  AddConstant(dict, "DEF_PRIORITY", DEF_PRIORITY);
  AddConstant(dict, "JOB_INFO_1", 1);

  // Job control codes used with SetJob
  AddConstant(dict, "JOB_CONTROL_CANCEL", JOB_CONTROL_CANCEL);
  AddConstant(dict, "JOB_CONTROL_PAUSE", JOB_CONTROL_PAUSE);
  AddConstant(dict, "JOB_CONTROL_RESTART", JOB_CONTROL_RESTART);
  AddConstant(dict, "JOB_CONTROL_RESUME", JOB_CONTROL_RESUME);
  AddConstant(dict, "JOB_CONTROL_DELETE", JOB_CONTROL_DELETE);
  AddConstant(dict, "JOB_CONTROL_SENT_TO_PRINTER", JOB_CONTROL_SENT_TO_PRINTER);
  AddConstant(dict, "JOB_CONTROL_LAST_PAGE_EJECTED", JOB_CONTROL_LAST_PAGE_EJECTED);

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

  // DeletePrinterDriverEx DeleteFlag
  AddConstant(dict, "DPD_DELETE_SPECIFIC_VERSION",DPD_DELETE_SPECIFIC_VERSION);
  AddConstant(dict, "DPD_DELETE_UNUSED_FILES",DPD_DELETE_UNUSED_FILES);
  AddConstant(dict, "DPD_DELETE_ALL_FILES",DPD_DELETE_ALL_FILES);

  // Port status and severity used in PORT_INFO_3
  AddConstant(dict, "PORT_STATUS_OFFLINE",PORT_STATUS_OFFLINE);
  AddConstant(dict, "PORT_STATUS_PAPER_JAM",PORT_STATUS_PAPER_JAM);
  AddConstant(dict, "PORT_STATUS_PAPER_OUT",PORT_STATUS_PAPER_OUT);
  AddConstant(dict, "PORT_STATUS_OUTPUT_BIN_FULL",PORT_STATUS_OUTPUT_BIN_FULL);
  AddConstant(dict, "PORT_STATUS_PAPER_PROBLEM",PORT_STATUS_PAPER_PROBLEM);
  AddConstant(dict, "PORT_STATUS_NO_TONER",PORT_STATUS_NO_TONER);
  AddConstant(dict, "PORT_STATUS_DOOR_OPEN",PORT_STATUS_DOOR_OPEN);
  AddConstant(dict, "PORT_STATUS_USER_INTERVENTION",PORT_STATUS_USER_INTERVENTION);
  AddConstant(dict, "PORT_STATUS_OUT_OF_MEMORY",PORT_STATUS_OUT_OF_MEMORY);
  AddConstant(dict, "PORT_STATUS_TONER_LOW",PORT_STATUS_TONER_LOW);
  AddConstant(dict, "PORT_STATUS_WARMING_UP",PORT_STATUS_WARMING_UP);
  AddConstant(dict, "PORT_STATUS_POWER_SAVE",PORT_STATUS_POWER_SAVE);

  AddConstant(dict, "PORT_STATUS_TYPE_ERROR",PORT_STATUS_TYPE_ERROR);
  AddConstant(dict, "PORT_STATUS_TYPE_WARNING",PORT_STATUS_TYPE_WARNING);
  AddConstant(dict, "PORT_STATUS_TYPE_INFO",PORT_STATUS_TYPE_INFO);

  HMODULE hmodule=LoadLibrary(TEXT("winspool.drv"));
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
	pfnDeletePrinterDriverEx=(DeletePrinterDriverExfunc)GetProcAddress(hmodule,"DeletePrinterDriverExW");
	pfnFlushPrinter=(FlushPrinterfunc)GetProcAddress(hmodule, "FlushPrinter");
	pfnGetDefaultPrinter=(GetDefaultPrinterfunc)GetProcAddress(hmodule, "GetDefaultPrinterW");
	pfnSetDefaultPrinter=(SetDefaultPrinterfunc)GetProcAddress(hmodule, "SetDefaultPrinterW");
  }
  dummy_tuple=PyTuple_New(0);

  PYWIN_MODULE_INIT_RETURN_SUCCESS;
}
