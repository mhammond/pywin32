/* File : win32job.i */

%module win32job // An interface to the win32 Process and Thread API's,
// available in Windows 2000 and later.

%{
#define UNICODE
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x500
#endif

#include "windows.h"
#include "PyWinTypes.h"
%}

%include "typemaps.i"
%include "pywin32.i"


// @pyswig |AssignProcessToJobObject|Associates a process with an existing job object.
BOOLAPI AssignProcessToJobObject(
  HANDLE hJob, // @pyparm <o PyHANDLE>|hJob||
  HANDLE hProcess // @pyparm <o PyHANDLE>|hProcess||
);

// @pyswig |CreateJobObject|Creates or opens a job object.
PyHANDLE CreateJobObject(
  SECURITY_ATTRIBUTES *lpJobAttributes, // @pyparm <o PySECURITY_ATTRIBUTES>|jobAttributes||
  WCHAR *lpName); // @pyparm unicode|name||

// *sigh* - only XP and later!!

// xxpyswig |IsProcessInJob|Determines if the process is running in the specified job.
/*
BOOLAPI IsProcessInJob(
  HANDLE ProcessHandle, // @pyparm <o PyHANDLE>|hProcess||
  HANDLE JobHandle, // @pyparm <o PyHANDLE>|hJob||
  BOOL *OUTPUT);
*/
// @pyswig |OpenJobObject|Opens an existing job object.
PyHANDLE OpenJobObject(
  DWORD dwDesiredAccess, // @pyparm int|desiredAccess||
  BOOL bInheritHandles, // @pyparm bool|inheritHandles||
  WCHAR *lpName // @pyparm unicode|name||
);

// xxpyswig |QueryInformationJobObject|Retrieves limit and job state information from the job object.

// xxpyswig |SetInformationJobObject|Set limits for a job object.

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



