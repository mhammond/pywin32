/******************************************************************
* Copyright (c) 1998-1999 Cisco Systems, Inc. All Rights Reserved
* Permission to use, copy, modify, and distribute this software and its
* documentation for any purpose and without fee is hereby granted,
* provided that the above copyright notice appear in all copies and that
* both that copyright notice and this permission notice appear in
* supporting documentation.
*
*
* CISCO SYSTEMS, INC. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
* SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
* FITNESS. IN NO EVENT SHALL CISCO SYSTEMS BE LIABLE FOR ANY LOST REVENUE, 
* PROFIT OR DATA, OR FOR SPECIAL, INDIRECT, CONSEQUENTIAL OR INCIDENTAL
* DAMAGES OR ANY OTHER DAMAGES WHATSOEVER, HOWEVER CAUSED AND REGARDLESS
* OF THE THEORY OF LIABILITY, ARISING OUT OF OR IN CONNECTION
* WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
******************************************************************/

// @doc

/********************************************************************
 Win32API WNet "Windows Networking" functions.

 WRT Mark Hammond, on who's examples much of this is based.

  This module seeks to implement the WNET win32 api functions for Windows NT.
  It will compile for both UNICODE and ASCII environments.  By extension of the
  UNICODE compatibility, it seeks to support Windows CE.  This functionality
  (Windows CE) has not yet been tested (or even compiled!).


  REVISION HISTORY:
  7/00  - Convert comments to autoduck strings.         mh
  12/99 - Pass errno instead of GetLastError(),
          WNetAddConnection2() allows a few NULLs       mh
  6/99ish - CE changes.                                 mh
  10/98	- Original version, ascii only.		Scott Cothrell, Cisco Systems Inc.
  12/98 - Unicode support added.			SC
  1/99	- Windows CE conditionals started. Not tested.	SC
  2/99	- First public release.  Work in Progress.	SC
*/

#if	defined(_WIN32_WCE_) // defined by Windows CE compiler environment

#	ifndef UNICODE
#	define UNICODE
#	endif

#	ifndef _UNICODE
#	define _UNICODE
#	endif

#endif

#include "windows.h"
#include "atlbase.h"
#include "Python.h"
#include "PyWinTypes.h"
#include "netres.h"			// NETRESOURCE Type
#include "pyncb.h"


/****************************************************************************
		HELPER FUNCTIONS

****************************************************************************/
/* error helper */
PyObject *ReturnError(char *msg, char *fnName = NULL, int errCode = 0)
{
	PyObject *v = Py_BuildValue("(izs)", errCode, fnName, msg);
	if (v != NULL) {
		PyErr_SetObject(PyWinExc_ApiError, v);
		Py_DECREF(v);
	}
	return NULL;
}
/* error helper - GetLastError() is provided, but this is for exceptions */
PyObject *ReturnNetError(char *fnName, long err = 0)
{
	return PyWin_SetAPIError(fnName, err);
}

// @pymethod |win32wnet|WNetAddConnection2|Creates a connection to a network resource. The function can redirect 
// a local device to the network resource.
static
PyObject *
PyWNetAddConnection2 (PyObject *self, PyObject *args)

{
	// @todo Eventually should update this to use a NETRESOURCE object (it was written before PyNETRESOURCE)
	USES_CONVERSION;

	DWORD	Type;  // @pyparm int|type||The resource type.  May be either RESOURCETYPE_DISK, RESOURCETYPE_PRINT, or RESOURCETYPE_ANY (from win32netcon)
	LPSTR	LocalName; // @pyparm string|localName||holds the name of a local device to map connection to; may be NULL
	LPSTR	RemoteName;	// @pyparm string|remoteName||holds the passed in remote machine\service name.
	LPSTR	ProviderName = NULL;	// @pyparm string|ProviderName|None|holds name of network provider to use (if any): NULL lets OS handle it
	LPSTR	Username = NULL; // @pyparm string|userName|None|The user name to connect as.
	LPSTR	Password = NULL; // @pyparm string|password|None|The password to use.
	
	DWORD	ErrorNo;		// holds the returned error number, if any
	DWORD	flags = 0; // @pyparm int|flags|0|Specifies a DWORD value that describes connection options. The following value is currently defined.
	// @flagh Value|Meaning
	// @flag CONNECT_UPDATE_PROFILE|The network resource connection should be remembered. 
	// <nl>If this bit flag is set, the operating system automatically attempts to restore the connection when the user logs on.
	// <nl>The operating system remembers only successful connections that redirect local devices. It does not remember connections that are unsuccessful or deviceless connections. (A deviceless connection occurs when the lpLocalName member is NULL or points to an empty string.)
	// <nl>If this bit flag is clear, the operating system does not automatically restore the connection at logon.
	NETRESOURCE  NetResource;

	if (!PyArg_ParseTuple(args,"izs|zzzi",&Type,&LocalName,&RemoteName,&ProviderName,&Username,&Password, &flags))
		return NULL;

// Build the NETRESOURCE structure
    Py_BEGIN_ALLOW_THREADS

	memset((void *)&NetResource, '\0', sizeof(NETRESOURCE));
	NetResource.dwType = Type;
	NetResource.lpLocalName = A2T(LocalName);
	NetResource.lpProvider = A2T(ProviderName);
	NetResource.lpRemoteName = A2T(RemoteName);

#ifdef _WIN32_WCE_	// Windows CE only has the #3 version...use NULL for HWND to simulate #2
	ErrorNo = WNetAddConnection3(NULL,&NetResource, A2T(Password), A2T(Username), 0);
#else
	ErrorNo = WNetAddConnection2(&NetResource, A2T(Password), A2T(Username), 0);
#endif
	Py_END_ALLOW_THREADS

	if (ErrorNo != NO_ERROR)
	{
		return ReturnNetError("WNetAddConnection2", ErrorNo);
	}

	Py_INCREF(Py_None);
	return Py_None;

};

// @pymethod |win32wnet|WNetCancelConnection2|Closes network connections made by WNetAddConnection2 or 3
static
PyObject *
PyWNetCancelConnection2 (PyObject *self, PyObject *args)
{
	USES_CONVERSION;

	LPSTR	lpName; // @pyparm string|name||Name of existing connection to be closed
	DWORD	dwFlags; // @pyparm int|flags||Currently determines if the persisent connection information will be updated as a result of this call.
	DWORD	bForce; // @pyparm int|force||indicates if the close operation should be forced. (i.e. ignore open files and connections)
	DWORD	ErrorNo;

	if(!PyArg_ParseTuple(args, "sii",&lpName, &dwFlags, &bForce))
		return NULL;

	Py_BEGIN_ALLOW_THREADS
		ErrorNo = WNetCancelConnection2(A2T(lpName), dwFlags, (BOOL)bForce);
	Py_END_ALLOW_THREADS

	if (ErrorNo != NO_ERROR)
	{
		return ReturnNetError("WNetCancelConnection2", ErrorNo);
	}
	Py_INCREF(Py_None);
	return Py_None;
};

// @pymethod <o PyHANDLE>|win32wnet|WNetOpenEnum|Opens an Enumeration Handle for Enumerating Resources with <om win32wnet.WNetEnumResource>
static
PyObject *
PyWNetOpenEnum(PyObject *self, PyObject *args)
{
	// @comm See the Microsoft SDK  for complete information on WNetOpenEnum.
	PyObject *	ob_nr;
	NETRESOURCE * p_nr;
	DWORD	dwScope, dwType, dwUsage; // not the same as the ones in NETRESOURCE
	DWORD	Errno;
	HANDLE	hEnum;
	// @pyparm int|scope||Specifies the scope of the enumeration.
	// @pyparm int|type||Specifies the resource types to enumerate.
	// @pyparm int|usage||Specifies the resource usage to be enumerated.
	// @pyparm <o NETRESOURCE>|resource||Python NETRESOURCE object.

	if (!PyArg_ParseTuple(args, "iiiO", &dwScope,&dwType,&dwUsage,&ob_nr))
		return NULL;
	if (ob_nr == Py_None)
		p_nr = NULL;
	else if
		(!PyWinObject_AsNETRESOURCE(ob_nr, &p_nr, FALSE))
			return(ReturnError("failed converting NetResource Object","WNetOpenEnum"));

	Py_BEGIN_ALLOW_THREADS
	Errno = WNetOpenEnum(dwScope, dwType, dwUsage, p_nr, &hEnum);
	Py_END_ALLOW_THREADS

	if (Errno != NO_ERROR)
		return(ReturnNetError("WNetOpenEnum", Errno));

	// @todo It appears there is a bug here - the handle returned will attempt to
	// be closed via CloseHandle, which is wrong.  We need a new private handle type!
	return (PyWinObject_FromHANDLE(hEnum));
	// @rdesc PyHANDLE representing the Win32 HANDLE for the open resource.
	// This handle should be closed via <om win32wnet.WNetCloseEnum>
};


// @pymethod |win32wnet|WNetCloseEnum|Closes a PyHANDLE that represents an Open Enumeration (from <om win32wnet.WNetOpenEnum>)
static
PyObject *
PyWNetCloseEnum(PyObject *self, PyObject *args)
{
	PyObject *	ob_nr;
	HANDLE	hEnum;
	DWORD Errno;
	// @pyparm <o PyHANDLE>|handle||The handle to close, as obtained from <om win32wnet.WNetOpenEnum>
	// @comm You should perform a WNetClose for each handle returned from <om win32wnet.WNetOpenEnum>.

	if (!PyArg_ParseTuple(args, "O!", &PyHANDLEType, &ob_nr))
		return NULL;
	
	if(!PyWinObject_AsHANDLE(ob_nr, &hEnum, FALSE))	// error code set by callee (check this)
		return NULL;

	Py_BEGIN_ALLOW_THREADS
	Errno = WNetCloseEnum(hEnum);
	Py_END_ALLOW_THREADS

	if(Errno != NO_ERROR)
		return(ReturnNetError("WNetCloseEnum", Errno));

	Py_INCREF(Py_None);
	return Py_None;
};

// @pymethod [<o NETRESOURCE>, ...]|win32wnet|WNetEnumResource|Enumerates a list of resources
static
PyObject *
PyWNetEnumResource(PyObject *self, PyObject *args)
{
	// @rdesc The list contains PyNETRESOURCE entries. The total number of PyNETRESOURCE entries will be \<= number
	// requested (excepting the default behavior of requesting 0, which returns up to 64)

	// @comm Successive calls to win32wnet.WNetEnumResource will enumerate starting where the previous call
	// stopped. That is, the enumeration is not reset on successive calls UNLESS the enumeration handle is
	// closed and reopened.  This lets you process an enumeration in small chunks (as small as 1 item at a time)
	// and still fully enumerate a network object!

	PyObject * Eob;		// incoming Handle object from OpenEnum
	LPVOID	lpBuffer;	// buffer in virtual memory
	HANDLE	hEnum;		// handle from the OpenEnum call
	DWORD	dwBuffsize;	// size of lpBuffer
	DWORD	dwRefsize;	// reference size for virtualfree
	DWORD	dwCount;	// number of entries to get
	DWORD	dwMaxCount = 64;
	DWORD	Errno = 0;
	// @pyparm <o PyHANDLE>|handle||A handle to an open Enumeration Object (from <om win32wnet.WNetOpenEnum>)
	// @pyparm int|maxExtries|64|The maximum number of entries to return.
	if (!PyArg_ParseTuple(args, "O!|i", &PyHANDLEType, &Eob, &dwMaxCount)) // enforce the PyHANDLEType, Count is optional
		return NULL;
	
	if (!PyWinObject_AsHANDLE(Eob, &hEnum, FALSE))	// shouldn't fail unless out of memory?
		return NULL;

	// nothing hard & fast here, just a rough sizing..have to figure out something better later

	if (dwMaxCount == 0)				// using 0 to mean a default
		dwMaxCount = dwCount = 64;		// lets default at 64 items
	else
		dwCount = dwMaxCount;		// yes virginia, 0xffffffff is a LOT of items

	PyObject * pRetlist = PyList_New(0);	//create a return list of 0 size
	if (pRetlist == Py_None)				// did we err?
		return(ReturnError("Unable to create return list","WNetEnumResource"));

	
	do	// start the enumeration
	{
	dwRefsize = dwBuffsize = 64*1024;	// set size of buffer to request at 64K

	lpBuffer = VirtualAlloc(NULL, dwBuffsize, MEM_COMMIT, PAGE_READWRITE); // allocate out of Virtual Memory

	if (lpBuffer == NULL)	// whoops, not that much!!??
	{
		dwRefsize = dwBuffsize = 4 * 1024;	//back off to 4K
		lpBuffer = VirtualAlloc(NULL, dwBuffsize, MEM_COMMIT, PAGE_READWRITE);
		if(lpBuffer == NULL)
		{
			Py_DECREF(pRetlist);
			PyErr_SetString(PyExc_MemoryError, "VirtualAlloc error in WNetEnumResource");
			return NULL;
		}
	}

	Py_BEGIN_ALLOW_THREADS
	Errno = WNetEnumResource(hEnum, &dwCount, lpBuffer, &dwBuffsize);	// do the enumeration
	Py_END_ALLOW_THREADS

	if (Errno == NO_ERROR)	// if no error, then build the list
	{

		NETRESOURCE *p_nr = (NETRESOURCE *)lpBuffer;	// Enum Resource returns a buffer of successive NETRESOURCE structs

		if (dwCount > 0)	// we actually got something
		{
			dwMaxCount = dwMaxCount - dwCount;	// how many more we will try to get
			do
			{
				PyObject *t_ob = PyWinObject_FromNETRESOURCE(p_nr);

				int listerr = PyList_Append(pRetlist,t_ob);	// append our PyNETRESOURCE obj...Append does an INCREF!
				Py_DECREF(t_ob);

				if (listerr)	// or bail
				{
					Py_DECREF(pRetlist);
					VirtualFree(lpBuffer, dwRefsize, MEM_DECOMMIT);
					return(ReturnError("Unable to create return list","WNetEnumResource"));
				}

				p_nr++;	// next NETRESOURCE object (its a ++ because it is a typed pointer)
				dwCount--;
			} while (dwCount);
		}; // if

		dwCount = dwMaxCount;	// reset to how many left
	}
		
	VirtualFree(lpBuffer, dwRefsize, MEM_DECOMMIT);	// free the working buffer

	}while ((Errno == NO_ERROR) && (dwMaxCount != 0));	// No more because EnumResource returned "ERROR_NO_MORE_DATA"
														// or we have enumerated all that was asked for.
	return pRetlist;
};

#if 0
/**********************************************************************************************************
**	Implements the WNetGetResourceInformation api call.

New functionality 
NOT TESTED, DO NOT USE (YET)
**
**********************************************************************************************************/

//static
PyObject *
PyWNetGetResourceInformation(PyObject *self, PyObject *args)
{
	PyObject *NRT;	//object placeholder for incoming NETRESOURCE object
	NETRESOURCE *p_nr;
	DWORD	dwRefsize, dwBuffsize;
	LPVOID	lpBuffer;
	DWORD	Errno = NO_ERROR;
	LPTSTR	*szFilePath = NULL;


	if (!PyArg_ParseTuple(args, "O!", &PyNETRESOURCEType, &NRT))
		return NULL;

	if (!PyWinObject_AsNETRESOURCE(NRT, &p_nr, FALSE))
		return(ReturnError("failed converting NetResource Object","WNetGetResourceInformation"));

	dwRefsize = dwBuffsize = 128*1024;	//size of memory buffer..worse case net/file path is 64K?
	lpBuffer = VirtualAlloc(NULL, dwBuffsize, MEM_COMMIT, PAGE_READWRITE); // allocate out of Virtual Memory

	if (lpBuffer == NULL)	// whoops, not that much!!??
		{
			PyErr_SetString(PyExc_MemoryError, "VirtualAlloc error in WNetGetResourceInformation");
			return NULL;
		}

	Py_BEGIN_ALLOW_THREADS
	Errno = WNetGetResourceInformation(p_nr, lpBuffer, &dwBuffsize, szFilePath);
	Py_END_ALLOW_THREADS

	if (Errno == NO_ERROR)
	{
		PyObject *t_ob = PyWinObject_FromNETRESOURCE((NETRESOURCE *)lpBuffer);
		PyObject *ret = Py_BuildValue("(O,s)", t_ob, szFilePath);
		Py_DECREF(t_ob);
		return ret;
	}
	else
		return(ReturnNetError("WNetGetResourceInformation", Errno));


}
#endif

// @pymethod int|win32wnet|Netbios|Executes a Netbios call.
PyObject *
PyWinMethod_Netbios(PyObject *self, PyObject *args)
{
	PyObject *obncb;
	// @pyparm <o NCB>|ncb||The NCB object to use for the call.
	if (!PyArg_ParseTuple(args, "O!:Netbios", &PyNCBType, &obncb))
		return NULL;
	PyNCB *pyncb = (PyNCB *)obncb;
	UCHAR rc;
	Py_BEGIN_ALLOW_THREADS
	rc = Netbios(&pyncb->m_ncb);
	Py_END_ALLOW_THREADS
	return PyInt_FromLong((long)rc);
}

// @pymethod buffer|win32wnet|NCBBuffer|Creates an NCB buffer of the relevant size.
PyObject *
PyWinMethod_NCBBuffer(PyObject *self, PyObject *args)
{
	int size;
	// @pyparm int|size||The number of bytes to allocate.
	if (!PyArg_ParseTuple(args, "i:NCBBuffer", &size))
		return NULL;
	return PyBuffer_New(size);
}

// @module win32wnet|A module that exposes the Windows Networking API.
static PyMethodDef win32wnet_functions[] = {
	// @pymeth NETRESOURCE|Creates a new <o NETRESOURCE> object
	{"NETRESOURCE",				PyWinMethod_NewNETRESOURCE,	1,	"NETRESOURCE Structure Object. x=NETRESOURCE() to instantiate"},
	// @pymeth NCB|Creates a new <o NCB> object
	{"NCB",						PyWinMethod_NewNCB,			1,	"NCB Netbios command structure Object"},
	// @pymeth NCBBuffer|Creates a new buffer
	{"NCBBuffer",					PyWinMethod_NCBBuffer,			1,	"Creates a memory buffer"},
	// @pymeth Netbios|Executes a Netbios call.
	{"Netbios",					PyWinMethod_Netbios,			1,	"Calls the windows Netbios function"},
	// @pymeth WNetAddConnection2|Creates a connection to a network resource.
	{"WNetAddConnection2",		PyWNetAddConnection2,		1,	"type,localname,remotename,provider,username,password (does not use NETRESOURCE)"},
	// @pymeth WNetCancelConnection2|Closes network connections made by WNetAddConnection2 or 3
	{"WNetCancelConnection2",	PyWNetCancelConnection2,	1,	"localname,dwflags,bforce"},
	// @pymeth WNetOpenEnum|Opens an Enumeration Handle for Enumerating Resources with <om win32wnet.WNetEnumResource>
	{"WNetOpenEnum",			PyWNetOpenEnum,				1,	"dwScope,dwType,dwUsage,NETRESOURCE - returns PyHANDLE"},
	// @pymeth WNetCloseEnum|Closes a PyHANDLE that represents an Open Enumeration (from <om win32wnet.WNetOpenEnum>)
	{"WNetCloseEnum",			PyWNetCloseEnum,			1,	"PyHANDLE from WNetOpenEnum()"},
	// @pymeth WNetEnumResource|Enumerates a list of resources
	{"WNetEnumResource",		PyWNetEnumResource,			1,	"Enum"},
#if 0
	{"WNetGetResourceInformation", PyWNetGetResourceInformation, 1, "NT_5 Only? DO NOT USE YET"},
#endif
	{NULL,			NULL}
};

extern "C" __declspec(dllexport) 
void
initwin32wnet(void)

{
  PyObject *dict, *module;
  module = Py_InitModule("win32wnet", win32wnet_functions);
  dict = PyModule_GetDict(module);
  PyWinGlobals_Ensure();
  PyDict_SetItemString(dict, "error", PyWinExc_ApiError);
  PyDict_SetItemString(dict, "NETRESOURCEType", (PyObject *)&PyNETRESOURCEType);
  PyDict_SetItemString(dict, "NCBType", (PyObject *)&PyNCBType);

  Py_INCREF(PyWinExc_ApiError);
}

