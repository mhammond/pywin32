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

#include "PyWinTypes.h"
#include "PyWinObjects.h" // for the PyHANDLE impl.
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

/****************************************************************************
		A HANDLE object

****************************************************************************/
class PyNETENUMHANDLE : public PyHANDLE
{
public:
	PyNETENUMHANDLE(HANDLE hInit) : PyHANDLE(hInit) {}
	virtual BOOL Close(void);
	virtual const char *GetTypeName() {return "PyNetEnumHANDLE";}
};

PyObject *PyNETENUMObject_FromHANDLE(HANDLE h)
{
	return new PyNETENUMHANDLE(h);
}

BOOL PyNETENUMObject_CloseHANDLE(PyObject *obHandle)
{
	BOOL ok;
	if (PyHANDLE_Check(obHandle))
		// Python error already set.
		ok = ((PyNETENUMHANDLE *)obHandle)->Close();
	else {
		PyErr_SetString(PyExc_TypeError, "Not a handle object");
		ok = FALSE;
	}
	return ok;
}

// The non-static member functions
BOOL PyNETENUMHANDLE::Close(void)
{
	LONG rc = m_handle ? WNetCloseEnum(m_handle) : ERROR_SUCCESS;
	m_handle = 0;
	if (rc!= ERROR_SUCCESS)
		PyWin_SetAPIError("WNetCloseEnum", rc);
	return rc==ERROR_SUCCESS;
}


/****************************************************************************
		PYTHON METHODS

****************************************************************************/

// @pymethod |win32wnet|WNetAddConnection2|Creates a connection to a network resource. The function can redirect 
// a local device to the network resource.
// @comm This function also accepts backwards-compatible, positional-only
// arguments of (dwType, lpLocalName, lpRemoteName[, lpProviderName, Username, Password, flags])
// @comm Accepts keyword arguments.
// @pyseeapi WNetAddConnection2
static PyObject *PyWNetAddConnection2 (PyObject *self, PyObject *args, PyObject *kwargs)
{
	LPTSTR	Username = NULL;
	LPTSTR	Password = NULL;
	// values used for b/w compat args.
	DWORD	Type;
	PyObject *obLocalName, *obRemoteName, *obProviderName = Py_None;

	PyObject *obnr, *obPassword=Py_None, *obUsername=Py_None, *ret=NULL;
	DWORD	ErrorNo;		// holds the returned error number, if any
	DWORD	flags = 0;
	NETRESOURCE  *pNetResource;
	NETRESOURCE  tempNetResource;
	memset(&tempNetResource, 0, sizeof(tempNetResource));

	if (PyArg_ParseTuple(args,"iOO|OOOi",&Type,&obLocalName,&obRemoteName,&obProviderName,&obUsername,&obPassword, &flags)) {
		// the b/w compat args have been used - build the NETRESOURCE structure
		memset((void *)&tempNetResource, '\0', sizeof(NETRESOURCE));
		tempNetResource.dwType = Type;
		if (!PyWinObject_AsTCHAR(obLocalName, &tempNetResource.lpLocalName, TRUE)
		    || !PyWinObject_AsTCHAR(obRemoteName, &tempNetResource.lpRemoteName, FALSE)
		    || !PyWinObject_AsTCHAR(obProviderName, &tempNetResource.lpProvider, TRUE))
			goto done;
		pNetResource = &tempNetResource;
	} else {
		PyErr_Clear();
		static char *keywords[] = {"NetResource","Password","UserName","Flags", NULL};
		if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O|OOk", keywords,
			&obnr,			// @pyparm <o PyNETRESOURCE>|NetResource||Describes the network resource for the connection.
			&obPassword,	// @pyparm str|Password|None|The password to use.  Use None for default credentials.
			&obUsername,	// @pyparm str|UserName|None|The user name to connect as.  Use None for default credentials.
			&flags))		// @pyparm int|Flags|0|Combination win32netcon.CONNECT_* flags
			return NULL;
		if (!PyWinObject_AsNETRESOURCE(obnr, &pNetResource, FALSE))
			return NULL;
	}

	if (!PyWinObject_AsTCHAR(obPassword, &Password, TRUE)
	    || !PyWinObject_AsTCHAR(obUsername, &Username, TRUE))
		goto done;

	Py_BEGIN_ALLOW_THREADS
	#ifdef _WIN32_WCE_	// Windows CE only has the #3 version...use NULL for HWND to simulate #2
		ErrorNo = WNetAddConnection3(NULL, pNetResource, Password, Username, flags);
	#else
		ErrorNo = WNetAddConnection2(pNetResource, Password, Username, flags);
	#endif
	Py_END_ALLOW_THREADS
	if (ErrorNo != NO_ERROR)
		ReturnNetError("WNetAddConnection2", ErrorNo);
	else{
		Py_INCREF(Py_None);
		ret = Py_None;
	}
done:
	PyWinObject_FreeTCHAR(Password);
	PyWinObject_FreeTCHAR(Username);
	PyWinObject_FreeTCHAR(tempNetResource.lpLocalName);
	PyWinObject_FreeTCHAR(tempNetResource.lpRemoteName);
	PyWinObject_FreeTCHAR(tempNetResource.lpProvider);
	return ret;
};

// @pymethod |win32wnet|WNetAddConnection3|Creates a connection to a network resource.
// @comm Accepts keyword arguments.
// @pyseeapi WNetAddConnection3
static PyObject *PyWNetAddConnection3 (PyObject *self, PyObject *args, PyObject *kwargs)
{
	LPTSTR	Username = NULL;
	LPTSTR	Password = NULL;
	DWORD	ErrorNo;		// holds the returned error number, if any
	DWORD	flags = 0;
	NETRESOURCE  *pNetResource;
	PyObject *obPassword=Py_None, *obUsername=Py_None, *ret=NULL;
	PyObject *obhwnd, *obnr;

	static char *keywords[] = {"HwndOwner", "NetResource","Password","UserName","Flags", NULL};
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "OO|OOk", keywords,
		&obhwnd,		// @pyparm int|hwnd||Handle to a parent window.
		&obnr,			// @pyparm <o PyNETRESOURCE>|NetResource||Describes the network resource for the connection.
		&obPassword,	// @pyparm str|Password|None|The password to use.  Use None for default credentials.
		&obUsername,	// @pyparm str|UserName|None|The user name to connect as.  Use None for default credentials.
		&flags))		// @pyparm int|Flags|0|Combination win32netcon.CONNECT_* flags
		return NULL;

	HWND hwnd;
	if (!PyWinObject_AsHANDLE(obhwnd, (HANDLE *)&hwnd))
		return NULL;
	if (!PyWinObject_AsNETRESOURCE(obnr, &pNetResource, FALSE))
		return NULL;

	if (!PyWinObject_AsTCHAR(obPassword, &Password, TRUE)
	    || !PyWinObject_AsTCHAR(obUsername, &Username, TRUE))
		goto done;

	Py_BEGIN_ALLOW_THREADS
	ErrorNo = WNetAddConnection3(hwnd, pNetResource, Password, Username, flags);
	Py_END_ALLOW_THREADS
	if (ErrorNo != NO_ERROR)
		ReturnNetError("WNetAddConnection3", ErrorNo);
	else{
		Py_INCREF(Py_None);
		ret = Py_None;
	}
done:
	PyWinObject_FreeTCHAR(Password);
	PyWinObject_FreeTCHAR(Username);
	return ret;
};

// @pymethod |win32wnet|WNetCancelConnection2|Closes network connections made by WNetAddConnection2 or 3
static
PyObject *
PyWNetCancelConnection2 (PyObject *self, PyObject *args)
{
	LPTSTR	lpName; // @pyparm string|name||Name of existing connection to be closed
	DWORD	dwFlags; // @pyparm int|flags||Currently determines if the persisent connection information will be updated as a result of this call.
	DWORD	bForce; // @pyparm int|force||indicates if the close operation should be forced. (i.e. ignore open files and connections)
	DWORD	ErrorNo;

	PyObject *obName;
	if(!PyArg_ParseTuple(args, "Okk", &obName, &dwFlags, &bForce))
		return NULL;
	if (!PyWinObject_AsTCHAR(obName, &lpName, FALSE))
		return NULL;
	Py_BEGIN_ALLOW_THREADS
		ErrorNo = WNetCancelConnection2(lpName, dwFlags, (BOOL)bForce);
	Py_END_ALLOW_THREADS
	PyWinObject_FreeTCHAR(lpName);
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
	// @pyparm <o PyNETRESOURCE>|resource||Python NETRESOURCE object.

	if (!PyArg_ParseTuple(args, "iiiO", &dwScope,&dwType,&dwUsage,&ob_nr))
		return NULL;
	if (!PyWinObject_AsNETRESOURCE(ob_nr, &p_nr, TRUE))
		return NULL;

	Py_BEGIN_ALLOW_THREADS
	Errno = WNetOpenEnum(dwScope, dwType, dwUsage, p_nr, &hEnum);
	Py_END_ALLOW_THREADS

	if (Errno != NO_ERROR)
		return(ReturnNetError("WNetOpenEnum", Errno));

	return (PyNETENUMObject_FromHANDLE(hEnum));
	// @rdesc PyHANDLE representing the Win32 HANDLE for the open resource.
	// This handle will be automatically be closed via <om win32wnet.WNetCloseEnum>, but
	// good style dictates it still be closed manually.
};


// @pymethod |win32wnet|WNetCloseEnum|Closes a PyHANDLE that represents an Open Enumeration (from <om win32wnet.WNetOpenEnum>)
static
PyObject *
PyWNetCloseEnum(PyObject *self, PyObject *args)
{
	PyObject *	ob_nr;
	// @pyparm <o PyHANDLE>|handle||The handle to close, as obtained from <om win32wnet.WNetOpenEnum>
	// @comm You should perform a WNetClose for each handle returned from <om win32wnet.WNetOpenEnum>.

	if (!PyArg_ParseTuple(args, "O", &ob_nr))
		return NULL;

	if (!PyNETENUMObject_CloseHANDLE(ob_nr))
		return NULL;
	
	Py_INCREF(Py_None);
	return Py_None;
};

// @pymethod [<o PyNETRESOURCE>, ...]|win32wnet|WNetEnumResource|Enumerates a list of resources
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
	
	if (!PyWinObject_AsHANDLE(Eob, &hEnum))	// shouldn't fail unless out of memory?
		return NULL;

	// nothing hard & fast here, just a rough sizing..have to figure out something better later

	if (dwMaxCount == 0)				// using 0 to mean a default
		dwMaxCount = dwCount = 64;		// lets default at 64 items
	else
		dwCount = dwMaxCount;		// yes virginia, 0xffffffff is a LOT of items

	PyObject * pRetlist = PyList_New(0);	//create a return list of 0 size
	if (pRetlist == NULL)               // did we err?
		return NULL;

	
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
					VirtualFree(lpBuffer, 0, MEM_RELEASE);
					return(ReturnError("Unable to create return list","WNetEnumResource"));
				}

				p_nr++;	// next NETRESOURCE object (its a ++ because it is a typed pointer)
				dwCount--;
			} while (dwCount);
		}; // if

		dwCount = dwMaxCount;	// reset to how many left
	}
		
	VirtualFree(lpBuffer, 0, MEM_RELEASE);	// free the working buffer

	}while ((Errno == NO_ERROR) && (dwMaxCount != 0));	// No more because EnumResource returned "ERROR_NO_MORE_DATA"
														// or we have enumerated all that was asked for.
	return pRetlist;
};

// @pymethod string|win32wnet|WNetGetUser|Retrieves the current default user name, or the user name used to establish a network connection.
static
PyObject *
PyWNetGetUser(PyObject *self, PyObject *args)
{
	PyObject *ret = NULL;
	PyObject *obConnection = Py_None;
	DWORD length = 0;
	DWORD errcode;
	TCHAR *szConnection = NULL;
	TCHAR *buf = NULL;

	// @pyparm string|connection|None|A string that specifies either the name of a local device that has been redirected to a network resource, or the remote name of a network resource to which a connection has been made without redirecting a local device. 
	// If this parameter is None, the system returns the name of the current user for the process.
	if (!PyArg_ParseTuple(args, "|O", &obConnection))
		return NULL;
	if (!PyWinObject_AsTCHAR(obConnection, &szConnection, TRUE))
		goto done;
	// get the buffer size
	{
	Py_BEGIN_ALLOW_THREADS
	errcode=WNetGetUser(szConnection, NULL, &length);
	Py_END_ALLOW_THREADS
	}
	if (length==0) {
		ReturnNetError("WNetGetUser", errcode);
		goto done;
	}
	buf = (TCHAR *)malloc( sizeof( TCHAR) * length);
	if (buf == NULL){
		PyErr_Format(PyExc_MemoryError, "Unable to allocate %d bytes", sizeof(TCHAR)*length);
		goto done;
	}
	Py_BEGIN_ALLOW_THREADS
	errcode = WNetGetUser(szConnection, buf, &length);
	Py_END_ALLOW_THREADS
	if (0 != errcode) {
		ReturnNetError("WNetGetUser", errcode);
		goto done;
	}
	// length includes the NULL - drop it (safely!)
	ret = PyWinObject_FromTCHAR(buf, (length > 0) ? length-1 : 0);
done:
	PyWinObject_FreeTCHAR(szConnection);
	if (buf) free(buf);
	return ret;
}

// @pymethod string/tuple|win32wnet|WNetGetUniversalName|Takes a drive-based path for a network resource and returns an information structure that contains a more universal form of the name.
static
PyObject *
PyWNetGetUniversalName(PyObject *self, PyObject *args)
{
	int level = UNIVERSAL_NAME_INFO_LEVEL;
	TCHAR *szLocalPath = NULL;
	PyObject *obLocalPath;
	void *buf = NULL;
	DWORD length = 0;
	PyObject *ret = NULL;
	DWORD errcode;
	if (!PyArg_ParseTuple(args, "O|i:WNetGetUniversalName", &obLocalPath, &level))
		return NULL;
	if (!PyWinObject_AsTCHAR(obLocalPath, &szLocalPath, FALSE))
		return NULL;
	// @pyparm string|localPath||A string that is a drive-based path for a network resource. 
	// <nl>For example, if drive H has been mapped to a network drive share, and the network 
	// resource of interest is a file named SAMPLE.DOC in the directory \WIN32\EXAMPLES on 
	// that share, the drive-based path is H:\WIN32\EXAMPLES\SAMPLE.DOC. 
	// @pyparm int|infoLevel|UNIVERSAL_NAME_INFO_LEVEL|Specifies the type of structure that the function stores in the buffer pointed to by the lpBuffer parameter. 
	// This parameter can be one of the following values.
	// @flagh Value|Meaning 
	// @flag UNIVERSAL_NAME_INFO_LEVEL (=1)|The function returns a simple string with the UNC name.
	// @flag REMOTE_NAME_INFO_LEVEL (=2)|The function returns a tuple based in the Win32 REMOTE_NAME_INFO data structure.
	// @rdesc If the infoLevel parameter is REMOTE_NAME_INFO_LEVEL, the result is a tuple of 3 strings: (UNCName, connectionName, remainingPath)

	// First get the buffer size.
	{
	Py_BEGIN_ALLOW_THREADS
	char temp_buf[] = ""; // doesnt appear to like NULL!!
	errcode = WNetGetUniversalName( szLocalPath, level, &temp_buf, &length);
	Py_END_ALLOW_THREADS
	}
	if (errcode != ERROR_MORE_DATA || length == 0) {
		ReturnNetError("WNetGetUniversalName (for buffer size)", errcode);
		goto done;
	}
	buf = malloc(length);
	if (buf==NULL) goto done;
	errcode = WNetGetUniversalName( szLocalPath, level, buf, &length);
	if (errcode != 0) {
		ReturnNetError("WNetGetUniversalName", errcode);
		goto done;
	}
	switch (level) {
	case UNIVERSAL_NAME_INFO_LEVEL:
		ret = PyWinObject_FromTCHAR( ((UNIVERSAL_NAME_INFO *)buf)->lpUniversalName );
		break;
	case REMOTE_NAME_INFO_LEVEL: {
		REMOTE_NAME_INFO *r = (REMOTE_NAME_INFO *)buf;
		ret = PyTuple_New(3);
		if (ret==NULL) goto done;
		PyTuple_SET_ITEM(ret, 0, PyWinObject_FromTCHAR( r->lpUniversalName) );
		PyTuple_SET_ITEM(ret, 1, PyWinObject_FromTCHAR( r->lpConnectionName) );
		PyTuple_SET_ITEM(ret, 2, PyWinObject_FromTCHAR( r->lpRemainingPath) );
		break;
		}
	default:
		PyErr_SetString(PyExc_TypeError, "Unsupported infoLevel");
	}
done:
	PyWinObject_FreeTCHAR(szLocalPath);
	if (buf) free(buf);
	return ret;
}

// @pymethod (<o PyNETRESOURCE>, str)|win32wnet|WNetGetResourceInformation|Finds the type and provider of a network resource
// @rdesc Returns a NETRESOURCE and a string containing the trailing part of the remote path
PyObject *
PyWNetGetResourceInformation(PyObject *self, PyObject *args)
{
	PyObject *NRT, *ret=NULL;	//object placeholder for incoming NETRESOURCE object
	NETRESOURCE *nrin, *nrout=NULL;
	// buffer holds a NETRESOURCE struct and all its string members
	DWORD bufsize=sizeof(NETRESOURCE)+256;
	DWORD err;
	LPTSTR	szFilePath = NULL;
#ifdef Py_DEBUG
	bufsize=sizeof(NETRESOURCE);	// make sure it loops thru again in debug mode
#endif

	if (!PyArg_ParseTuple(args, "O!", &PyNETRESOURCEType, 
		&NRT))	// @pyparm <o PyNETRESOURCE>|NetResource||Describes a network resource.  lpRemoteName is required, dwType and lpProvider can be supplied if known
		return NULL;

	if (!PyWinObject_AsNETRESOURCE(NRT, &nrin, FALSE))
		return NULL;

	while (1){
		// function will not take NULL to return the buffer size, always pass in a valid buffer
		if (nrout)
			free(nrout);
		nrout=(NETRESOURCE *)malloc(bufsize);
		if (nrout==NULL)
			return PyErr_Format(PyExc_MemoryError,"Unable to allocate %d bytes", bufsize);
		Py_BEGIN_ALLOW_THREADS
		err = WNetGetResourceInformation(nrin, nrout, &bufsize, &szFilePath);
		Py_END_ALLOW_THREADS
		if (err == NO_ERROR){
			ret=Py_BuildValue("NN",
				PyWinObject_FromNETRESOURCE(nrout),
				PyWinObject_FromTCHAR(szFilePath));
			break;
			}
		else if (err!=ERROR_MORE_DATA){
			ReturnNetError("WNetGetResourceInformation", err);
			break;
			}
		}
	if (nrout)
		free(nrout);
	return ret;
}

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

// @pymethod (int,str,str)|win32wnet|WNetGetLastError|Retrieves extended error information set by a network provider when one of the WNet* functions fails
// @rdesc Returns the error code, a text description of the error, and the name of the network provider
// @comm The error description or the network provider name may be truncated if they exceed 1024 and 256 characters, respectively
PyObject *PyWNetGetLastError(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ":WNetGetLastError"))
		return NULL;
	DWORD err, extendederr;
	TCHAR errstr[1024], provider[256];
	Py_BEGIN_ALLOW_THREADS
	err=WNetGetLastError(&extendederr, errstr, sizeof(errstr)/sizeof(TCHAR), provider, sizeof(provider)/sizeof(TCHAR));
	Py_END_ALLOW_THREADS
	if (err==NO_ERROR)
		return Py_BuildValue("kNN", extendederr, PyWinObject_FromTCHAR(errstr), PyWinObject_FromTCHAR(provider));
	return ReturnNetError("WNetGetLastError", err);
}

// @pymethod <o PyNETRESOURCE>|win32wnet|WNetGetResourceParent|Finds the parent resource of a network resource
PyObject *PyWNetGetResourceParent(PyObject *self, PyObject *args)
{
	NETRESOURCE *nr, *parentnr=NULL;
	DWORD err, bufsize=sizeof(NETRESOURCE)+256;
#ifdef Py_DEBUG
	bufsize=sizeof(NETRESOURCE);
#endif
	PyObject *obnr, *ret=NULL;
	if (!PyArg_ParseTuple(args, "O:WNetGetResourceParent", 
		&obnr))		// @pyparm <o PyNETRESOURCE>|NetResource||Describes a network resource.  lpRemoteName and lpProvider are required, dwType is recommended for efficiency
		return NULL;
	if (!PyWinObject_AsNETRESOURCE(obnr, &nr, FALSE))
		return NULL;
	// buffer includes NETRESOURCE plus character strings it contains
	// Will not accept NULL to retrieve buffer size
	while (1){
		if (parentnr)
			free(parentnr);
		parentnr=(NETRESOURCE *)malloc(bufsize);
		if (parentnr==NULL)
			return PyErr_Format(PyExc_MemoryError,"Unable to allocate %d bytes", bufsize);
		Py_BEGIN_ALLOW_THREADS
		err=WNetGetResourceParent(nr, parentnr, &bufsize);
		Py_END_ALLOW_THREADS
		if (err==NO_ERROR){
			ret=PyWinObject_FromNETRESOURCE(parentnr);
			break;
			}
		else if (err!=ERROR_MORE_DATA){
			ReturnNetError("WNetGetResourceParent", err);
			break;
			}
		}
	if (parentnr)
		free(parentnr);
	return ret;
}


// @pymethod string|win32wnet|WNetGetConnection|Retrieves the name of the network resource associated with a local device.
static
PyObject *
PyWNetGetConnection(PyObject *self, PyObject *args)
{
	PyObject *ret = NULL;
	PyObject *obConnection = Py_None;
	DWORD length = 0;
	DWORD errcode;
	TCHAR *szConnection = NULL;
	TCHAR *buf = NULL;

	// @pyparm string|connection|None|A string that is a drive-based path for a network resource. 
	// For example, if drive H has been mapped to a network drive share, and the network resource of interest is a file named Sample.doc in the directory \Win32\Examples on that share, the drive-based path is H:\Win32\Examples\Sample.doc.
	if (!PyArg_ParseTuple(args, "|O", &obConnection))
		return NULL;
	if (!PyWinObject_AsTCHAR(obConnection, &szConnection, TRUE))
		goto done;
	// get the buffer size
	{
		Py_BEGIN_ALLOW_THREADS
			errcode=WNetGetConnection(szConnection, NULL, &length);
		Py_END_ALLOW_THREADS
	}
	if (length==0) {
		ReturnNetError("WNetGetConnection", errcode);
		goto done;
	}
	buf = (TCHAR *)malloc( sizeof( TCHAR) * length);
	if (buf == NULL){
		PyErr_Format(PyExc_MemoryError, "Unable to allocate %d bytes", sizeof(TCHAR)*length);
		goto done;
	}
	Py_BEGIN_ALLOW_THREADS
		errcode = WNetGetConnection(szConnection, buf, &length);
	Py_END_ALLOW_THREADS
		if (0 != errcode) {
			ReturnNetError("WNetGetConnection", errcode);
			goto done;
		}
		// length includes the NULL - drop it (safely!)
		ret = PyWinObject_FromTCHAR(buf, (length > 0) ? length-1 : 0);
done:
		PyWinObject_FreeTCHAR(szConnection);
		if (buf) free(buf);
		return ret;
}

// @module win32wnet|A module that exposes the Windows Networking API.
static PyMethodDef win32wnet_functions[] = {
	// @pymeth NETRESOURCE|The <o PyNETRESOURCE> type - can be used to create a new <o PyNETRESOURCE> object.
	// @pymeth NCB|The <o PyNCB> type - can be used to create a new <o PyNCB> object.
	// @pymeth NCBBuffer|Creates a new buffer
	{"NCBBuffer",					PyWinMethod_NCBBuffer,			1,	"Creates a memory buffer"},
	// @pymeth Netbios|Executes a Netbios call.
	{"Netbios",					PyWinMethod_Netbios,			1,	"Calls the windows Netbios function"},
	// @pymeth WNetAddConnection2|Creates a connection to a network resource.
	{"WNetAddConnection2",		(PyCFunction)PyWNetAddConnection2,	METH_KEYWORDS|METH_VARARGS,	"WNetAddConnection2(NetResource, Password, UserName, Flags)"},
	// @pymeth WNetAddConnection3|Creates a connection to a network resource.
	{"WNetAddConnection3",		(PyCFunction)PyWNetAddConnection3,	METH_KEYWORDS|METH_VARARGS,	"WNetAddConnection3(HwndParent, NetResource, Password, UserName, Flags)"},
	// @pymeth WNetCancelConnection2|Closes network connections made by WNetAddConnection2 or 3
	{"WNetCancelConnection2",	PyWNetCancelConnection2,	1,	"localname,dwflags,bforce"},
	// @pymeth WNetOpenEnum|Opens an Enumeration Handle for Enumerating Resources with <om win32wnet.WNetEnumResource>
	{"WNetOpenEnum",			PyWNetOpenEnum,				1,	"dwScope,dwType,dwUsage,NETRESOURCE - returns PyHANDLE"},
	// @pymeth WNetCloseEnum|Closes a PyHANDLE that represents an Open Enumeration (from <om win32wnet.WNetOpenEnum>)
	{"WNetCloseEnum",			PyWNetCloseEnum,			1,	"PyHANDLE from WNetOpenEnum()"},
	// @pymeth WNetEnumResource|Enumerates a list of resources
	{"WNetEnumResource",		PyWNetEnumResource,			1,	"Enum"},
	// @pymeth WNetGetUser|Retrieves the current default user name, or the user name used to establish a network connection.
	{"WNetGetUser",             PyWNetGetUser,              1,  "connectionName=None"},
	// @pymeth WNetGetUniversalName|Takes a drive-based path for a network resource and returns an information structure that contains a more universal form of the name.
	{"WNetGetUniversalName",    PyWNetGetUniversalName,     1,  "localPath, infoLevel=UNIVERSAL_NAME_INFO_LEVEL"},
	// @pymeth WNetGetResourceInformation|Finds the type and provider of a network resource
	{"WNetGetResourceInformation", PyWNetGetResourceInformation, 1, "Finds the type and provider of a network resource"},
	// @pymeth WNetGetLastError|Retrieves extended error information set by a network provider when one of the WNet* functions fails
	{"WNetGetLastError", PyWNetGetLastError, 1, "Retrieves extended error information set by a network provider when one of the WNet* functions fails"},
	// @pymeth WNetGetResourceParent|Finds the parent resource of a network resource
	{"WNetGetResourceParent", PyWNetGetResourceParent, 1, "Finds the parent resource of a network resource"},
	// @pymeth WNetGetConnection|Retrieves the name of the network resource associated with a local device.
	{"WNetGetConnection",    PyWNetGetConnection,     1,  "Retrieves the name of the network resource associated with a local device"},


	{NULL,			NULL}
};

PYWIN_MODULE_INIT_FUNC(win32wnet)
{
	PYWIN_MODULE_INIT_PREPARE(win32wnet, win32wnet_functions,
	                          "A module that exposes the Windows Networking API.");

	PyDict_SetItemString(dict, "error", PyWinExc_ApiError);

	if ((PyType_Ready(&PyNETRESOURCEType) == -1) ||
		(PyType_Ready(&PyNCBType) == -1))
		PYWIN_MODULE_INIT_RETURN_ERROR;

	// old "deprecated" names, before types could create instances.
	PyDict_SetItemString(dict, "NETRESOURCEType", (PyObject *)&PyNETRESOURCEType);
	PyDict_SetItemString(dict, "NCBType", (PyObject *)&PyNCBType);

	PyDict_SetItemString(dict, "NETRESOURCE", (PyObject *)&PyNETRESOURCEType);
	PyDict_SetItemString(dict, "NCB", (PyObject *)&PyNCBType);

	PYWIN_MODULE_INIT_RETURN_SUCCESS;
}
