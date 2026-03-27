/* File : win32job.i */

%module win32job // An interface to the win32 Process and Thread API's,

%{
#include "PyWinTypes.h"

#define CHECK_PFN(fname)if (pfn##fname==NULL) return PyErr_Format(PyExc_NotImplementedError,"%s is not available on this platform", #fname);
typedef BOOL (WINAPI *IsProcessInJobfunc)(HANDLE,HANDLE,PBOOL);
static IsProcessInJobfunc pfnIsProcessInJob=NULL;
%}

%include "typemaps.i"
%include "pywin32.i"


%init %{
HMODULE hmodule = PyWin_GetOrLoadLibraryHandle("kernel32.dll");
if (hmodule != NULL) {
	pfnIsProcessInJob = (IsProcessInJobfunc)GetProcAddress(hmodule, "IsProcessInJob");
}

%}


// @pyswig |AssignProcessToJobObject|Associates a process with an existing job object.
BOOLAPI AssignProcessToJobObject(
  HANDLE hJob, // @pyparm <o PyHANDLE>|hJob||
  HANDLE hProcess // @pyparm <o PyHANDLE>|hProcess||
);

// @pyswig |CreateJobObject|Creates or opens a job object.
PyHANDLE CreateJobObject(
  SECURITY_ATTRIBUTES *lpJobAttributes, // @pyparm <o PySECURITY_ATTRIBUTES>|jobAttributes||
  WCHAR *lpName); // @pyparm unicode|name||

// @pyswig |OpenJobObject|Opens an existing job object.
PyHANDLE OpenJobObject(
  DWORD dwDesiredAccess, // @pyparm int|desiredAccess||
  BOOL bInheritHandles, // @pyparm bool|inheritHandles||
  WCHAR *lpName // @pyparm unicode|name||
);

// @pyswig |TerminateJobObject|Terminates all processes currently associated with the job.
BOOLAPI TerminateJobObject(
  HANDLE hJob, // @pyparm <o PyHANDLE>|hJob||
  unsigned int uExitCode // @pyparm int|exitCode||
);

// @pyswig |UserHandleGrantAccess|Grants or denies access to a handle to a User object to a job that has a user-interface restriction.
BOOLAPI UserHandleGrantAccess(
  HANDLE hUserHandle, // @pyparm <o PyHANDLE>|hUserHandle||
  HANDLE hJob, // @pyparm <o PyHANDLE>|hJob||
  BOOL bGrant // @pyparm bool|grant||
);


%{
// @pyswig boolean|IsProcessInJob|Determines if the process is running in the specified job.
// @comm Function is only available on WinXP and later
PyObject *PyIsProcessInJob(PyObject *self, PyObject *args)
{
	CHECK_PFN(IsProcessInJob);
	PyObject *obph, *objh;
	HANDLE ph, jh;
	BOOL res;
	if (!PyArg_ParseTuple(args, "OO",
		&obph,		// @pyparm <o PyHANDLE>|hProcess||Handle to a process
		&objh))		// @pyparm <o PyHANDLE>|hJob||Handle to a job, use None to check if process is part of any job
		return NULL;
	if (!PyWinObject_AsHANDLE(obph, &ph))
		return NULL;
	if (!PyWinObject_AsHANDLE(objh, &jh))
		return NULL;

	if (!(*pfnIsProcessInJob)(ph, jh, &res))
		return PyWin_SetAPIError("IsProcessInJob");
	return PyBool_FromLong(res);
}
%}
%native (IsProcessInJob) PyIsProcessInJob;

%{
PyObject *PyWinObject_FromJOBOBJECT_BASIC_LIMIT_INFORMATION(PJOBOBJECT_BASIC_LIMIT_INFORMATION jbli)
{
	return Py_BuildValue("{s:L,s:L,s:k,s:k,s:k,s:k,s:k,s:k,s:k}",
		"PerProcessUserTimeLimit",jbli->PerProcessUserTimeLimit,
		"PerJobUserTimeLimit",jbli->PerJobUserTimeLimit,
		"LimitFlags",jbli->LimitFlags,
		"MinimumWorkingSetSize",jbli->MinimumWorkingSetSize,
		"MaximumWorkingSetSize",jbli->MaximumWorkingSetSize,
		"ActiveProcessLimit",jbli->ActiveProcessLimit,
		"Affinity",jbli->Affinity,
		"PriorityClass",jbli->PriorityClass,
		"SchedulingClass",jbli->SchedulingClass);
}

PyObject *PyWinObject_FromJOBOBJECT_BASIC_ACCOUNTING_INFORMATION(PJOBOBJECT_BASIC_ACCOUNTING_INFORMATION jbai)
{
	return Py_BuildValue("{s:L,s:L,s:L,s:L,s:k,s:k,s:k,s:k}",
		"TotalUserTime",jbai->TotalUserTime,
		"TotalKernelTime",jbai->TotalKernelTime,
		"ThisPeriodTotalUserTime",jbai->ThisPeriodTotalUserTime,
		"ThisPeriodTotalKernelTime",jbai->ThisPeriodTotalKernelTime,
		"TotalPageFaultCount",jbai->TotalPageFaultCount,
		"TotalProcesses",jbai->TotalProcesses,
		"ActiveProcesses",jbai->ActiveProcesses,
		"TotalTerminatedProcesses",jbai->TotalTerminatedProcesses);
}

// @pyswig dict|QueryInformationJobObject|Retrieves limit and job state information from the job object.
// @rdesc The type of the returned information is dependent on the class requested
PyObject *PyQueryInformationJobObject(PyObject *self, PyObject *args)
{
	PyObject *objh;
	HANDLE jh;
	JOBOBJECTINFOCLASS infoclass;
	if (!PyArg_ParseTuple(args, "Ok",
		&objh,			// @pyparm <o PyHANDLE>|Job||Handle to a job, use None for job that calling process is part of
		&infoclass))	// @pyparm int|JobObjectInfoClass||The type of data required, one of JobObject* values
		return NULL;
	if (!PyWinObject_AsHANDLE(objh, &jh))
		return NULL;
	// @flagh JobObjectInfoClass|Type of information returned
	switch (infoclass){
		// @flag JobObjectBasicAccountingInformation|Returns a dict representing a JOBOBJECT_BASIC_ACCOUNTING_INFORMATION struct
		case JobObjectBasicAccountingInformation:{
			JOBOBJECT_BASIC_ACCOUNTING_INFORMATION info;
			if (!QueryInformationJobObject(jh, infoclass, &info, sizeof(info), NULL))
				return PyWin_SetAPIError("QueryInformationJobObject");
			return PyWinObject_FromJOBOBJECT_BASIC_ACCOUNTING_INFORMATION(&info);
			}
 		// @flag JobObjectBasicAndIoAccountingInformation|Returns a dict representing a JOBOBJECT_BASIC_AND_IO_ACCOUNTING_INFORMATION struct
		case JobObjectBasicAndIoAccountingInformation:{
			JOBOBJECT_BASIC_AND_IO_ACCOUNTING_INFORMATION info;
			if (!QueryInformationJobObject(jh, infoclass, &info, sizeof(info), NULL))
				return PyWin_SetAPIError("QueryInformationJobObject");
			return Py_BuildValue("{s:N,s:N}",
				"BasicInfo",PyWinObject_FromJOBOBJECT_BASIC_ACCOUNTING_INFORMATION(&info.BasicInfo),
				"IoInfo",	PyWinObject_FromIO_COUNTERS(&info.IoInfo));
			}
		// @flag JobObjectBasicLimitInformation|Returns a dict representing a JOBOBJECT_BASIC_LIMIT_INFORMATION struct
		case JobObjectBasicLimitInformation:{
			JOBOBJECT_BASIC_LIMIT_INFORMATION info;
			if (!QueryInformationJobObject(jh, infoclass, &info, sizeof(info), NULL))
				return PyWin_SetAPIError("QueryInformationJobObject");
			return PyWinObject_FromJOBOBJECT_BASIC_LIMIT_INFORMATION(&info);
			}
		// @flag JobObjectExtendedLimitInformation|Returns a dict representing a JOBOBJECT_EXTENDED_LIMIT_INFORMATION struct
		case JobObjectExtendedLimitInformation:{
			JOBOBJECT_EXTENDED_LIMIT_INFORMATION info;
			if (!QueryInformationJobObject(jh, infoclass, &info, sizeof(info), NULL))
				return PyWin_SetAPIError("QueryInformationJobObject");
			return Py_BuildValue("{s:N,s:N,s:N,s:N,s:N,s:N}",
				"BasicLimitInformation",PyWinObject_FromJOBOBJECT_BASIC_LIMIT_INFORMATION(&info.BasicLimitInformation),
				"IoInfo",				PyWinObject_FromIO_COUNTERS(&info.IoInfo),
				"ProcessMemoryLimit",	PyLong_FromUnsignedLongLong(info.ProcessMemoryLimit),
				"JobMemoryLimit",		PyLong_FromUnsignedLongLong(info.JobMemoryLimit),
				"PeakProcessMemoryUsed",PyLong_FromUnsignedLongLong(info.PeakProcessMemoryUsed),
				"PeakJobMemoryUsed",	PyLong_FromUnsignedLongLong(info.PeakJobMemoryUsed));
			}

		// @flag JobObjectEndOfJobTimeInformation|Returns a dict representing a JOBOBJECT_END_OF_JOB_TIME_INFORMATION struct
		case JobObjectEndOfJobTimeInformation:{
			JOBOBJECT_END_OF_JOB_TIME_INFORMATION info;
			if (!QueryInformationJobObject(jh, infoclass, &info, sizeof(info), NULL))
				return PyWin_SetAPIError("QueryInformationJobObject");
			return Py_BuildValue("{s:k}","EndOfJobTimeAction",info.EndOfJobTimeAction);
			}
		// @flag JobObjectBasicUIRestrictions|Returns a dict representing a JOBOBJECT_BASIC_UI_RESTRICTIONS struct
		case JobObjectBasicUIRestrictions:{
			JOBOBJECT_BASIC_UI_RESTRICTIONS info;
			if (!QueryInformationJobObject(jh, infoclass, &info, sizeof(info), NULL))
				return PyWin_SetAPIError("QueryInformationJobObject");
			return Py_BuildValue("{s:k}","UIRestrictionsClass",info.UIRestrictionsClass);
			}
		// @flag JobObjectBasicProcessIdList|Returns a sequence of pids of processes assigned to the job
		case JobObjectBasicProcessIdList:{
			JOBOBJECT_BASIC_PROCESS_ID_LIST *pinfo=NULL;
			DWORD pids_allocated=50, buflen, tuple_index;
			PyObject *ret=NULL;
			while (1){
				if (pinfo)
					free(pinfo);
				buflen=sizeof(DWORD)*2 + sizeof(ULONG_PTR)*pids_allocated;
				pinfo=(JOBOBJECT_BASIC_PROCESS_ID_LIST *)malloc(buflen);
				if (pinfo==NULL)
					return PyErr_Format(PyExc_MemoryError, "Failed to allocate %d bytes", buflen);
				ZeroMemory(pinfo, buflen);
				if (QueryInformationJobObject(jh, infoclass, pinfo, buflen, NULL))
					break;
				if (GetLastError()!=ERROR_MORE_DATA){
					PyWin_SetAPIError("QueryInformationJobObject");
					goto done;;
					}
				pids_allocated=pinfo->NumberOfAssignedProcesses+2;
				}
			ret=PyTuple_New(pinfo->NumberOfAssignedProcesses);
			if (ret==NULL)
				goto done;;
			for (tuple_index=0; tuple_index<pinfo->NumberOfProcessIdsInList; tuple_index++){
				PyObject *tuple_item=PyLong_FromUnsignedLongLong(pinfo->ProcessIdList[tuple_index]);
				if (tuple_item==NULL){
					Py_DECREF(ret);
					ret=NULL;
					break;
					}
				PyTuple_SET_ITEM(ret, tuple_index, tuple_item);
				}
			done:
				free(pinfo);
				return ret;
			}
		// @flag JobObjectJobSetInformation|Returns a dict representing a JOBOBJECT_JOBSET_INFORMATION struct (not documented on MSDN)
		case JobObjectJobSetInformation:{
			JOBOBJECT_JOBSET_INFORMATION info;
			if (!QueryInformationJobObject(jh, infoclass, &info, sizeof(info), NULL))
				return PyWin_SetAPIError("QueryInformationJobObject");
			return Py_BuildValue("{s:k}","MemberLevel",info.MemberLevel);
			}
		// @flag JobObjectSecurityLimitInformation|JOBOBJECT_SECURITY_LIMIT_INFORMATION Not implemented
		// @flag JobObjectAssociateCompletionPortInformation|JOBOBJECT_ASSOCIATE_COMPLETION_PORT Not implemented
		default:
			return PyErr_Format(PyExc_NotImplementedError, "Job information class %d is not supported yet");
	}
}
%}
%native (QueryInformationJobObject) PyQueryInformationJobObject;

%{
BOOL PyWinObject_AsJOBOBJECT_END_OF_JOB_TIME_INFORMATION(PyObject *ob, PJOBOBJECT_END_OF_JOB_TIME_INFORMATION jeoj)
{
	static char *keywords[]={"EndOfJobTimeAction",NULL};
	static char *fmt="k:JOBOBJECT_END_OF_JOB_TIME_INFORMATION";
	if (!PyDict_Check(ob)){
		PyErr_SetString(PyExc_TypeError,"JOBOBJECT_END_OF_JOB_TIME_INFORMATION must be a dict");
		return FALSE;
		}
	PyObject *dummyargs=PyTuple_New(0);
	if (dummyargs==NULL)
		return FALSE;
	BOOL bsuccess=PyArg_ParseTupleAndKeywords(dummyargs, ob, fmt, keywords,
		&jeoj->EndOfJobTimeAction);
	Py_DECREF(dummyargs);
	return bsuccess;
}

BOOL PyWinObject_AsJOBOBJECT_BASIC_UI_RESTRICTIONS(PyObject *ob, PJOBOBJECT_BASIC_UI_RESTRICTIONS jbur)
{
	static char *keywords[]={"UIRestrictionsClass",NULL};
	static char *fmt="k:JOBOBJECT_BASIC_UI_RESTRICTIONS";
	if (!PyDict_Check(ob)){
		PyErr_SetString(PyExc_TypeError,"JOBOBJECT_BASIC_UI_RESTRICTIONS must be a dict");
		return FALSE;
		}
	PyObject *dummyargs=PyTuple_New(0);
	if (dummyargs==NULL)
		return FALSE;
	BOOL bsuccess=PyArg_ParseTupleAndKeywords(dummyargs, ob, fmt, keywords,
		&jbur->UIRestrictionsClass);
	Py_DECREF(dummyargs);
	return bsuccess;
}

BOOL PyWinObject_AsIO_COUNTERS(PyObject *ob, PIO_COUNTERS pioc)
{
	static char *keywords[]={"ReadOperationCount","WriteOperationCount","OtherOperationCount",
		"ReadTransferCount","WriteTransferCount","OtherTransferCount",NULL};
	static char *fmt="KKKKKK:IO_COUNTERS";
	if (!PyDict_Check(ob)){
		PyErr_SetString(PyExc_TypeError,"IO_COUNTERS must be a dict");
		return FALSE;
		}
	PyObject *dummyargs=PyTuple_New(0);
	if (dummyargs==NULL)
		return FALSE;
	BOOL bsuccess=PyArg_ParseTupleAndKeywords(dummyargs, ob, fmt, keywords,
		&pioc->ReadOperationCount,
		&pioc->WriteOperationCount,
		&pioc->OtherOperationCount,
		&pioc->ReadTransferCount,
		&pioc->WriteTransferCount,
		&pioc->OtherTransferCount);
	Py_DECREF(dummyargs);
	return bsuccess;
}

BOOL PyWinObject_AsJOBOBJECT_BASIC_LIMIT_INFORMATION(PyObject *ob, PJOBOBJECT_BASIC_LIMIT_INFORMATION jbli)
{
	static char *keywords[]={"PerProcessUserTimeLimit","PerJobUserTimeLimit","LimitFlags",
		"MinimumWorkingSetSize","MaximumWorkingSetSize","ActiveProcessLimit",
		"Affinity","PriorityClass","SchedulingClass",NULL};
	// contains a couple of datatypes that change size for 64-bit
#ifdef _WIN64
	static char *fmt="LLkKKkKkk:JOBOBJECT_BASIC_LIMIT_INFORMATION";
#else
	static char *fmt="LLkkkkkkk:JOBOBJECT_BASIC_LIMIT_INFORMATION";
#endif
	if (!PyDict_Check(ob)){
		PyErr_SetString(PyExc_TypeError,"JOBOBJECT_BASIC_LIMIT_INFORMATION must be a dict");
		return FALSE;
		}
	PyObject *dummyargs=PyTuple_New(0);
	if (dummyargs==NULL)
		return FALSE;
	BOOL bsuccess=PyArg_ParseTupleAndKeywords(dummyargs, ob, fmt, keywords,
		&jbli->PerProcessUserTimeLimit,
		&jbli->PerJobUserTimeLimit,
		&jbli->LimitFlags,
		&jbli->MinimumWorkingSetSize,
		&jbli->MaximumWorkingSetSize,
		&jbli->ActiveProcessLimit,
		&jbli->Affinity,
		&jbli->PriorityClass,
		&jbli->SchedulingClass);
	Py_DECREF(dummyargs);
	return bsuccess;
}

BOOL PyWinObject_AsJOBOBJECT_EXTENDED_LIMIT_INFORMATION(PyObject *ob, PJOBOBJECT_EXTENDED_LIMIT_INFORMATION jeli)
{
	static char *keywords[]={"BasicLimitInformation","IoInfo","ProcessMemoryLimit",
		"JobMemoryLimit","PeakProcessMemoryUsed","PeakJobMemoryUsed",NULL};
#ifdef _WIN64
	static char *fmt="O&O&KKKK:JOBOBJECT_EXTENDED_LIMIT_INFORMATION";
#else
	static char *fmt="O&O&kkkk:JOBOBJECT_EXTENDED_LIMIT_INFORMATION";
#endif
	if (!PyDict_Check(ob)){
		PyErr_SetString(PyExc_TypeError,"JOBOBJECT_EXTENDED_LIMIT_INFORMATION must be a dict");
		return FALSE;
		}
	PyObject *dummyargs=PyTuple_New(0);
	if (dummyargs==NULL)
		return FALSE;
	BOOL bsuccess=PyArg_ParseTupleAndKeywords(dummyargs, ob, fmt, keywords,
		PyWinObject_AsJOBOBJECT_BASIC_LIMIT_INFORMATION, &jeli->BasicLimitInformation,
		PyWinObject_AsIO_COUNTERS, &jeli->IoInfo,
		&jeli->ProcessMemoryLimit,
		&jeli->JobMemoryLimit,
		&jeli->PeakProcessMemoryUsed,
		&jeli->PeakJobMemoryUsed);
	Py_DECREF(dummyargs);
	return bsuccess;
}

// @pyswig |SetInformationJobObject|Sets quotas and limits for a job
PyObject *PySetInformationJobObject(PyObject *self, PyObject *args)
{
	PyObject *objh;
	PyObject *obinfo;
	HANDLE jh;
	JOBOBJECTINFOCLASS infoclass;
	if (!PyArg_ParseTuple(args, "OkO!",
		&objh,			// @pyparm <o PyHANDLE>|Job||Handle to a job
		&infoclass,		// @pyparm int|JobObjectInfoClass||The type of data required, one of JobObject* values
		&PyDict_Type, &obinfo))		// @pyparm dict|JobObjectInfo||Dictionary containing info to be set, as returned by <om win32job.QueryInformationJobObject>
		return NULL;
	if (!PyWinObject_AsHANDLE(objh, &jh))
		return NULL;
	// @flagh JobObjectInfoClass|Type of information to be set
	switch (infoclass){
		// @flag JobObjectBasicLimitInformation|A JOBOBJECT_BASIC_LIMIT_INFORMATION dict
		case JobObjectBasicLimitInformation:{
			JOBOBJECT_BASIC_LIMIT_INFORMATION info;
			if (!PyWinObject_AsJOBOBJECT_BASIC_LIMIT_INFORMATION(obinfo, &info))
				return NULL;
			if (!SetInformationJobObject(jh, infoclass, &info, sizeof(info)))
				return PyWin_SetAPIError("SetInformationJobObject");
			break;
			}
		// @flag JobObjectExtendedLimitInformation|dict representing a JOBOBJECT_EXTENDED_LIMIT_INFORMATION struct
		case JobObjectExtendedLimitInformation:{
			JOBOBJECT_EXTENDED_LIMIT_INFORMATION info;
			if (!PyWinObject_AsJOBOBJECT_EXTENDED_LIMIT_INFORMATION(obinfo, &info))
				return NULL;
			if (!SetInformationJobObject(jh, infoclass, &info, sizeof(info)))
				return PyWin_SetAPIError("SetInformationJobObject");
			break;
			}

		// @flag JobObjectEndOfJobTimeInformation|dict representing a JOBOBJECT_END_OF_JOB_TIME_INFORMATION struct
		case JobObjectEndOfJobTimeInformation:{
			JOBOBJECT_END_OF_JOB_TIME_INFORMATION info;
			if (!PyWinObject_AsJOBOBJECT_END_OF_JOB_TIME_INFORMATION(obinfo, &info))
				return NULL;
			if (!SetInformationJobObject(jh, infoclass, &info, sizeof(info)))
				return PyWin_SetAPIError("SetInformationJobObject");
			break;
			}
		// @flag JobObjectBasicUIRestrictions|dict representing a JOBOBJECT_BASIC_UI_RESTRICTIONS struct
		case JobObjectBasicUIRestrictions:{
			JOBOBJECT_BASIC_UI_RESTRICTIONS info;
			if (!PyWinObject_AsJOBOBJECT_BASIC_UI_RESTRICTIONS(obinfo, &info))
				return NULL;
			if (!SetInformationJobObject(jh, infoclass, &info, sizeof(info)))
				return PyWin_SetAPIError("SetInformationJobObject");
			break;
			}
		// @flag JobObjectJobSetInformation|Input is a JOBOBJECT_JOBSET_INFORMATION dict - Not implemented
		// @flag JobObjectSecurityLimitInformation|Input is a JOBOBJECT_SECURITY_LIMIT_INFORMATION dict - Not implemented
		// @flag JobObjectAssociateCompletionPortInformation|Input is a JOBOBJECT_ASSOCIATE_COMPLETION_PORT dict - Not implemented
		default:
			return PyErr_Format(PyExc_NotImplementedError, "Job information class %d is not supported yet");
	}
	Py_INCREF(Py_None);
	return Py_None;
}
%}
%native (SetInformationJobObject) PySetInformationJobObject;



// some of these are in winnt.py also
// Access rights used with OpenJobObject
#define JOB_OBJECT_ASSIGN_PROCESS JOB_OBJECT_ASSIGN_PROCESS
#define JOB_OBJECT_SET_ATTRIBUTES JOB_OBJECT_SET_ATTRIBUTES
#define JOB_OBJECT_QUERY JOB_OBJECT_QUERY
#define JOB_OBJECT_TERMINATE JOB_OBJECT_TERMINATE
#define JOB_OBJECT_SET_SECURITY_ATTRIBUTES JOB_OBJECT_SET_SECURITY_ATTRIBUTES
#define JOB_OBJECT_ALL_ACCESS JOB_OBJECT_ALL_ACCESS

#define JOB_OBJECT_TERMINATE_AT_END_OF_JOB JOB_OBJECT_TERMINATE_AT_END_OF_JOB
#define JOB_OBJECT_POST_AT_END_OF_JOB JOB_OBJECT_POST_AT_END_OF_JOB

#define JOB_OBJECT_MSG_END_OF_JOB_TIME JOB_OBJECT_MSG_END_OF_JOB_TIME
#define JOB_OBJECT_MSG_END_OF_PROCESS_TIME JOB_OBJECT_MSG_END_OF_PROCESS_TIME
#define JOB_OBJECT_MSG_ACTIVE_PROCESS_LIMIT JOB_OBJECT_MSG_ACTIVE_PROCESS_LIMIT
#define JOB_OBJECT_MSG_ACTIVE_PROCESS_ZERO JOB_OBJECT_MSG_ACTIVE_PROCESS_ZERO
#define JOB_OBJECT_MSG_NEW_PROCESS JOB_OBJECT_MSG_NEW_PROCESS
#define JOB_OBJECT_MSG_EXIT_PROCESS JOB_OBJECT_MSG_EXIT_PROCESS
#define JOB_OBJECT_MSG_ABNORMAL_EXIT_PROCESS JOB_OBJECT_MSG_ABNORMAL_EXIT_PROCESS
#define JOB_OBJECT_MSG_PROCESS_MEMORY_LIMIT JOB_OBJECT_MSG_PROCESS_MEMORY_LIMIT
#define JOB_OBJECT_MSG_JOB_MEMORY_LIMIT JOB_OBJECT_MSG_JOB_MEMORY_LIMIT

#define JOB_OBJECT_LIMIT_WORKINGSET JOB_OBJECT_LIMIT_WORKINGSET
#define JOB_OBJECT_LIMIT_PROCESS_TIME JOB_OBJECT_LIMIT_PROCESS_TIME
#define JOB_OBJECT_LIMIT_JOB_TIME JOB_OBJECT_LIMIT_JOB_TIME
#define JOB_OBJECT_LIMIT_ACTIVE_PROCESS JOB_OBJECT_LIMIT_ACTIVE_PROCESS
#define JOB_OBJECT_LIMIT_AFFINITY JOB_OBJECT_LIMIT_AFFINITY
#define JOB_OBJECT_LIMIT_PRIORITY_CLASS JOB_OBJECT_LIMIT_PRIORITY_CLASS
#define JOB_OBJECT_LIMIT_PRESERVE_JOB_TIME JOB_OBJECT_LIMIT_PRESERVE_JOB_TIME
#define JOB_OBJECT_LIMIT_SCHEDULING_CLASS JOB_OBJECT_LIMIT_SCHEDULING_CLASS

#define JOB_OBJECT_LIMIT_PROCESS_MEMORY JOB_OBJECT_LIMIT_PROCESS_MEMORY
#define JOB_OBJECT_LIMIT_JOB_MEMORY JOB_OBJECT_LIMIT_JOB_MEMORY
#define JOB_OBJECT_LIMIT_DIE_ON_UNHANDLED_EXCEPTION JOB_OBJECT_LIMIT_DIE_ON_UNHANDLED_EXCEPTION
#define JOB_OBJECT_LIMIT_BREAKAWAY_OK JOB_OBJECT_LIMIT_BREAKAWAY_OK
#define JOB_OBJECT_LIMIT_SILENT_BREAKAWAY_OK JOB_OBJECT_LIMIT_SILENT_BREAKAWAY_OK
#define JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE
#define JOB_OBJECT_LIMIT_VALID_FLAGS JOB_OBJECT_LIMIT_VALID_FLAGS

#define JOB_OBJECT_BASIC_LIMIT_VALID_FLAGS JOB_OBJECT_BASIC_LIMIT_VALID_FLAGS
#define JOB_OBJECT_EXTENDED_LIMIT_VALID_FLAGS JOB_OBJECT_EXTENDED_LIMIT_VALID_FLAGS
// This apparently went away in the win10 sdk?
// #define JOB_OBJECT_RESERVED_LIMIT_VALID_FLAGS JOB_OBJECT_RESERVED_LIMIT_VALID_FLAGS

#define JOB_OBJECT_UILIMIT_NONE JOB_OBJECT_UILIMIT_NONE
#define JOB_OBJECT_UILIMIT_HANDLES JOB_OBJECT_UILIMIT_HANDLES
#define JOB_OBJECT_UILIMIT_READCLIPBOARD JOB_OBJECT_UILIMIT_READCLIPBOARD
#define JOB_OBJECT_UILIMIT_WRITECLIPBOARD JOB_OBJECT_UILIMIT_WRITECLIPBOARD
#define JOB_OBJECT_UILIMIT_SYSTEMPARAMETERS JOB_OBJECT_UILIMIT_SYSTEMPARAMETERS
#define JOB_OBJECT_UILIMIT_DISPLAYSETTINGS JOB_OBJECT_UILIMIT_DISPLAYSETTINGS
#define JOB_OBJECT_UILIMIT_GLOBALATOMS JOB_OBJECT_UILIMIT_GLOBALATOMS
#define JOB_OBJECT_UILIMIT_DESKTOP JOB_OBJECT_UILIMIT_DESKTOP
#define JOB_OBJECT_UILIMIT_EXITWINDOWS JOB_OBJECT_UILIMIT_EXITWINDOWS
#define JOB_OBJECT_UILIMIT_ALL JOB_OBJECT_UILIMIT_ALL
#define JOB_OBJECT_UI_VALID_FLAGS JOB_OBJECT_UI_VALID_FLAGS

#define JOB_OBJECT_SECURITY_NO_ADMIN JOB_OBJECT_SECURITY_NO_ADMIN
#define JOB_OBJECT_SECURITY_RESTRICTED_TOKEN JOB_OBJECT_SECURITY_RESTRICTED_TOKEN
#define JOB_OBJECT_SECURITY_ONLY_TOKEN JOB_OBJECT_SECURITY_ONLY_TOKEN
#define JOB_OBJECT_SECURITY_FILTER_TOKENS JOB_OBJECT_SECURITY_FILTER_TOKENS
#define JOB_OBJECT_SECURITY_VALID_FLAGS JOB_OBJECT_SECURITY_VALID_FLAGS

// used with QueryInformationJobObject, from JOBOBJECTINFOCLASS enum
#define JobObjectBasicAccountingInformation JobObjectBasicAccountingInformation
#define JobObjectBasicLimitInformation JobObjectBasicLimitInformation
#define JobObjectBasicProcessIdList JobObjectBasicProcessIdList
#define JobObjectBasicUIRestrictions JobObjectBasicUIRestrictions
#define JobObjectSecurityLimitInformation JobObjectSecurityLimitInformation
#define JobObjectEndOfJobTimeInformation JobObjectEndOfJobTimeInformation
#define JobObjectAssociateCompletionPortInformation JobObjectAssociateCompletionPortInformation
#define JobObjectBasicAndIoAccountingInformation JobObjectBasicAndIoAccountingInformation
#define JobObjectExtendedLimitInformation JobObjectExtendedLimitInformation
#define JobObjectJobSetInformation JobObjectJobSetInformation
#define MaxJobObjectInfoClass MaxJobObjectInfoClass
