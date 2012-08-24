/***********************************************************

win32net.cpp -- module for interface into Network API

NOTE: The Network API for NT uses UNICODE.  Therefore, you
can not simply pass python strings to the API functioms - some
conversion is required.

	Note: The NET functions have their own set of error codes in  2100-2200
	range.  The system error functions do not always apply.
	i.e. GetLastError may be useless.

  REV HISTORY:

  Original version: Mark Hammond's Build 109 distribution.

  October, 98:	rewrote PyNetUserChangePassword - changed error handling - needs testing
				rewrote PyNetUserGetGroups - fixed enumeration - tested
				rewrote PyNetUserGetLocalGroups - fixed enumeration - tested
				added PyNetShareEnum1 - exported as NetShareEnum - check assumptions - tested
				PyNetMessageBufferSend - didn't touch

  November, 98	Cleaned up return Lists (removed Integer count as redundant)
				
******************************************************************/
// @doc

#ifndef UNICODE
#error This project requires a Unicode build.
#endif

#include "PyWinTypes.h"
#include "lm.h"
#include "lmuseflg.h"
#include "win32net.h"

#include "assert.h"

#if WINVER >= 0x0500
NetGetJoinInformationfunc pfnNetGetJoinInformation=NULL;
#endif

/*****************************************************************************/
/* error helpers */

PyObject *ReturnNetError(char *fnName, long err /*=0*/)
{
	return PyWin_SetAPIError(fnName, err);
};

BOOL FindNET_STRUCT(DWORD level, PyNET_STRUCT *pBase, PyNET_STRUCT **ppRet)
{
	for (;pBase->entries;pBase++) {
		if (level==pBase->level) {
			*ppRet = pBase;
			return TRUE;
		}
	}
	PyErr_SetString(PyExc_ValueError, "This information level is not supported");
	return FALSE;
}

void PyObject_FreeNET_STRUCT(PyNET_STRUCT *pI, BYTE *pBuf)
{
	if (pBuf==NULL) return;
	// Free all the strings.
	PyNET_STRUCT_ITEM *pItem;
	for( pItem=pI->entries;pItem->attrname != NULL;pItem++) {
		switch (pItem->type) {
			case NSI_WSTR:
				if (*((WCHAR **)(pBuf+pItem->off)))
					PyWinObject_FreeWCHAR(*((WCHAR **)(pBuf+pItem->off)));
				break;
			case NSI_HOURS:
				if (*((char **)(pBuf+pItem->off)))
					free(*((char **)(pBuf+pItem->off)));
				break;
			case NSI_SID:
				if (*((SID **)(pBuf+pItem->off)))
					free(*((SID **)(pBuf+pItem->off)));
				break;
			case NSI_SECURITY_DESCRIPTOR:
				if (*((SECURITY_DESCRIPTOR **)(pBuf+pItem->off)))
					free(*((SECURITY_DESCRIPTOR **)(pBuf+pItem->off)));
				break;
			default:
				break;
		}
	}
	free(pBuf);
}
BOOL PyObject_AsNET_STRUCT( PyObject *ob, PyNET_STRUCT *pI, BYTE **ppRet )
{
	BOOL ok = FALSE;
	if (!PyMapping_Check(ob)) {
		PyErr_SetString(PyExc_TypeError, "The object must be a mapping");
		return FALSE;
	}
	// allocate the structure, and wipe it to zero.
	BYTE *buf = (BYTE *)malloc(pI->structsize);
	memset(buf, 0, pI->structsize);
	PyNET_STRUCT_ITEM *pItem;
	for( pItem=pI->entries;pItem->attrname != NULL;pItem++) {
		PyObject *subob = PyMapping_GetItemString(ob, pItem->attrname);

		if (subob==NULL) {
			PyErr_Clear();
			// See if it is OK.
			if (pItem->reqd) {
				PyErr_Format(PyExc_ValueError, "The mapping does not have the required attribute '%s'", pItem->attrname);
				goto done;
			}
		} else {
			switch (pItem->type) {
				case NSI_WSTR:
					WCHAR *wsz;
					if (!PyWinObject_AsWCHAR(subob, &wsz, !pItem->reqd)) {
						Py_DECREF(subob);
						goto done;
					}
					*((WCHAR **)(buf+pItem->off)) = wsz;
					break;
				case NSI_DWORD:
					*((DWORD *)(buf+pItem->off)) = PyInt_AsUnsignedLongMask(subob);
					if (*((DWORD *)(buf+pItem->off)) == -1 && PyErr_Occurred()){
						PyErr_Clear();
						PyErr_Format(PyExc_TypeError, "The mapping attribute '%s' must be an unsigned 32 bit int", pItem->attrname);
						Py_DECREF(subob);
						goto done;
					}
					break;
				case NSI_LONG:
					*((LONG *)(buf+pItem->off)) = PyInt_AsLong(subob);
					if (*((LONG *)(buf+pItem->off)) == -1 && PyErr_Occurred()){
						PyErr_Clear();
						PyErr_Format(PyExc_TypeError, "The mapping attribute '%s' must be an integer", pItem->attrname);
						Py_DECREF(subob);
						goto done;
					}
					break;
				case NSI_BOOL:
					*((BOOL *)(buf+pItem->off)) = PyObject_IsTrue(subob);
					if (*((BOOL *)(buf+pItem->off)) == -1 && PyErr_Occurred()){
						PyErr_Clear();
						PyErr_Format(PyExc_TypeError, "The mapping attribute '%s' must be boolean", pItem->attrname);
						Py_DECREF(subob);
						goto done;
					}
					
					break;
				case NSI_HOURS:
					if (subob != Py_None) {
						if (!PyString_Check(subob) || PyString_Size(subob)!=21) {
							PyErr_Format(PyExc_TypeError, "The mapping attribute '%s' must be a string of exactly length 21", pItem->attrname);
							Py_DECREF(subob);
							goto done;
						}
						*((char **)(buf+pItem->off)) = (char *)malloc(21);
						memcpy(*((char **)(buf+pItem->off)), PyString_AsString(subob), 21);
					}
					break;
				case NSI_SID: {
					PSID pSIDsrc;
					if (!PyWinObject_AsSID(subob, &pSIDsrc ,TRUE)) {
						Py_DECREF(subob);
						goto done;
					}
					PSID *ppSIDdest = ((PSID *)(buf+pItem->off));
					size_t len = GetLengthSid(pSIDsrc);
					*ppSIDdest = (SID *)malloc(len);
					memcpy(*ppSIDdest, pSIDsrc, len);
					}
					break;
				case NSI_SECURITY_DESCRIPTOR: {
					PSECURITY_DESCRIPTOR pSDsrc;
					if (!PyWinObject_AsSECURITY_DESCRIPTOR(subob, &pSDsrc ,TRUE)) {
						Py_DECREF(subob);
						goto done;
					}
					PSECURITY_DESCRIPTOR *ppSDdest = ((PSECURITY_DESCRIPTOR *)(buf+pItem->off));
					if (pSDsrc==NULL)
						*ppSDdest=NULL;
					else{
						size_t len = GetSecurityDescriptorLength(pSDsrc);
						*ppSDdest = (PSECURITY_DESCRIPTOR)malloc(len);
						memcpy(*ppSDdest, pSDsrc, len);
						}
					}
					break;
				
				default:
					PyErr_SetString(PyExc_RuntimeError, "invalid internal data type");
					Py_DECREF(subob);
					goto done;
			}
			Py_DECREF(subob);
		}
	}
	ok = TRUE;
done:
	if (!ok ) {
		PyObject_FreeNET_STRUCT(pI, buf);
		return FALSE;
	}
	*ppRet = buf;
	return TRUE;
}

PyObject *PyObject_FromNET_STRUCT(PyNET_STRUCT *pI, BYTE *buf)
{
	PyObject *ret = PyDict_New();
	PyNET_STRUCT_ITEM *pItem;
	for( pItem=pI->entries;pItem->attrname != NULL;pItem++) {
		PyObject *newObj = NULL;
		switch (pItem->type) {
			case NSI_WSTR:
				newObj = PyWinObject_FromWCHAR(*((WCHAR **)(buf+pItem->off)));
				break;
			case NSI_DWORD:
				newObj = PyLong_FromUnsignedLong(*((DWORD *)(buf+pItem->off)));
				break;
			case NSI_LONG:
				newObj = PyInt_FromLong(*((LONG *)(buf+pItem->off)));
				break;
			case NSI_BOOL:
				newObj = *((BOOL *)(buf+pItem->off)) ? Py_True : Py_False;
				Py_INCREF(newObj);
				break;
			case NSI_HOURS: {
				char *data = *((char **)(buf+pItem->off));
				if (data) {
					newObj = PyString_FromStringAndSize(data,21);
				} else {
					newObj = Py_None;
					Py_INCREF(Py_None);
				}
				break;
				}
			case NSI_SID:
				newObj = PyWinObject_FromSID(*((SID **)(buf+pItem->off)));
				break;
			case NSI_SECURITY_DESCRIPTOR:
				newObj = PyWinObject_FromSECURITY_DESCRIPTOR(*((PSECURITY_DESCRIPTOR *)(buf+pItem->off)));
				break;
			default:
				PyErr_SetString(PyExc_RuntimeError, "invalid internal data");
				break;
		}
		if (newObj==NULL) {
			Py_DECREF(ret);
			return NULL;
		}
		PyMapping_SetItemString(ret, pItem->attrname, newObj);
		Py_DECREF(newObj);
	}
	return ret;
}

PyObject *PyDoSimpleEnum(PyObject *self, PyObject *args, PFNSIMPLEENUM pfn, char *fnname, PyNET_STRUCT *pInfos) 
{
	WCHAR *szServer = NULL;
	PyObject *obServer;
	PyObject *ret = NULL;
	PyNET_STRUCT *pInfo;
	DWORD err;
	DWORD dwPrefLen = MAX_PREFERRED_LENGTH;
	DWORD level;
	BOOL ok = FALSE;
	DWORD_PTR resumeHandle = 0;
	DWORD numRead, i;
	PyObject *list, *obResumeHandle = Py_None;
	BYTE *buf = NULL;
	DWORD totalEntries = 0;
	if (!PyArg_ParseTuple(args, "Oi|Oi", &obServer, &level, &obResumeHandle, &dwPrefLen))
		return NULL;
	if (!PyWinObject_AsWCHAR(obServer, &szServer, TRUE))
		goto done;
	if (obResumeHandle != Py_None && !PyWinLong_AsDWORD_PTR(obResumeHandle, &resumeHandle))
		goto done;

	if (!FindNET_STRUCT(level, pInfos, &pInfo))
		goto done;

	Py_BEGIN_ALLOW_THREADS
	/* Bad resume handles etc can cause access violations here - catch them. */
    PYWINTYPES_TRY {
		err = (*pfn)(szServer, level, &buf, dwPrefLen, &numRead, &totalEntries, &resumeHandle);
	}
    PYWINTYPES_EXCEPT {
		err = ERROR_INVALID_PARAMETER;
	}
	Py_END_ALLOW_THREADS
	if (err!=0 && err != ERROR_MORE_DATA) {
		ReturnNetError(fnname,err);
		goto done;
	}
	list = PyList_New(numRead);
	if (list==NULL) goto done;
	for (i=0;i<numRead;i++) {
		PyObject *sub = PyObject_FromNET_STRUCT(pInfo, buf+(i*pInfo->structsize));
		if (sub==NULL) goto done;
		PyList_SetItem(list, i, sub);
	}
	resumeHandle = err==0 ? 0 : resumeHandle;
	ret = Py_BuildValue("OlN", list, totalEntries, PyWinObject_FromDWORD_PTR(resumeHandle));
	Py_DECREF(list);
	ok = TRUE;
done:
	if (buf) NetApiBufferFree(buf);
	if (!ok) {
		Py_XDECREF(ret);
		ret = NULL;
	}
	PyWinObject_FreeWCHAR(szServer);
	return ret;
}

PyObject *PyDoNamedEnum(PyObject *self, PyObject *args, PFNNAMEDENUM pfn, char *fnname, PyNET_STRUCT *pInfos) 
{
	WCHAR *szServer = NULL, *szGroup = NULL;
	PyObject *obServer, *obGroup;
	PyObject *ret = NULL;
	PyNET_STRUCT *pInfo;
	DWORD err;
	DWORD dwPrefLen = 4096;
	DWORD level;
	BOOL ok = FALSE;
	DWORD_PTR resumeHandle = 0;
	PyObject *obResumeHandle = Py_None;
	DWORD numRead, i;
	PyObject *list;
	BYTE *buf = NULL;
	DWORD totalEntries = 0;
	if (!PyArg_ParseTuple(args, "OOi|Oi", &obServer, &obGroup, &level, &obResumeHandle, &dwPrefLen))
		return NULL;
	if (!PyWinObject_AsWCHAR(obServer, &szServer, TRUE))
		goto done;

	if (!PyWinObject_AsWCHAR(obGroup, &szGroup, FALSE))
		goto done;

	if (obResumeHandle != Py_None && !PyWinLong_AsDWORD_PTR(obResumeHandle, &resumeHandle))
		goto done;

	if (!FindNET_STRUCT(level, pInfos, &pInfo))
		goto done;

    Py_BEGIN_ALLOW_THREADS
	err = (*pfn)(szServer, szGroup, level, &buf, dwPrefLen, &numRead, &totalEntries, &resumeHandle);
    Py_END_ALLOW_THREADS
	if (err!=0 && err != ERROR_MORE_DATA) {
		ReturnNetError(fnname,err);
		goto done;
	}
	list = PyList_New(numRead);
	if (list==NULL) goto done;
	for (i=0;i<numRead;i++) {
		PyObject *sub = PyObject_FromNET_STRUCT(pInfo, buf+(i*pInfo->structsize));
		if (sub==NULL) goto done;
		PyList_SetItem(list, i, sub);
	}
	resumeHandle = err==0 ? 0 : resumeHandle;
	ret = Py_BuildValue("OlN", list, totalEntries, PyWinObject_FromDWORD_PTR(resumeHandle));
	Py_DECREF(list);
	ok = TRUE;
done:
	if (buf) NetApiBufferFree(buf);
	if (!ok) {
		Py_XDECREF(ret);
		ret = NULL;
	}
	PyWinObject_FreeWCHAR(szServer);
	PyWinObject_FreeWCHAR(szGroup);
	return ret;
}

PyObject *
PyDoGroupSet(PyObject *self, PyObject *args, PFNGROUPSET pfn, char *fnname, PyNET_STRUCT *pInfos) 
{
	WCHAR *szServer = NULL;
	WCHAR *szGroup = NULL;
	PyObject *obServer, *obGroup, *obData;
	PyObject *ret = NULL;
	PyObject *members_tuple=NULL;
	DWORD level;
	DWORD err = 0;
	BYTE *buf = NULL;
	BYTE **ppTempObjects = NULL;
	DWORD i, numEntries;
	PyNET_STRUCT *pI;
	if (!PyArg_ParseTuple(args, "OOiO", &obServer, &obGroup, &level, &obData))
		return NULL;
	if (!PyWinObject_AsWCHAR(obServer, &szServer, TRUE))
		goto done;
	if (!PyWinObject_AsWCHAR(obGroup, &szGroup, FALSE))
		goto done;
	if (!FindNET_STRUCT(level, pInfos, &pI))
		goto done;


	members_tuple=PyWinSequence_Tuple(obData, &numEntries);
	if (members_tuple==NULL) {
		PyErr_SetString(PyExc_TypeError, "Data must be a sequence of dictionaries");
		goto done;
	}
	ppTempObjects = new BYTE *[numEntries];
	memset(ppTempObjects, 0, sizeof(BYTE *) * numEntries);
	for (i=0;i<numEntries;i++) {
		PyObject *sub = PyTuple_GET_ITEM(members_tuple, i);
		if (!PyObject_AsNET_STRUCT(sub, pI, ppTempObjects+i))
			goto done;
	}
	// OK - all objects are ok, and we are holding the buffers.
	// copy to our own buffer
	buf = new BYTE[numEntries*pI->structsize];
	if (buf==NULL)	{
		PyErr_SetString(PyExc_MemoryError, "Allocating buffer for members");
		goto done;
	}
	for (i=0;i<numEntries;i++)
		memcpy(buf+(i*pI->structsize), ppTempObjects[i], pI->structsize);
    Py_BEGIN_ALLOW_THREADS
	err = (*pfn)(szServer, szGroup, level, buf, numEntries);
    Py_END_ALLOW_THREADS
	if (err) {
		ReturnNetError(fnname,err);
		goto done;
	}
	ret = Py_None;
	Py_INCREF(Py_None);
done:
	PyWinObject_FreeWCHAR(szServer);
	PyWinObject_FreeWCHAR(szGroup);
	Py_XDECREF(members_tuple);
	if (ppTempObjects) {
		for (i=0;i<numEntries;i++) {
			PyObject_FreeNET_STRUCT(pI, ppTempObjects[i]);
		}
		delete [] ppTempObjects;
	}
	delete [] buf;
	return ret;
}

PyObject *PyDoGetInfo(PyObject *self, PyObject *args, PFNGETINFO pfn, char *fnname, PyNET_STRUCT *pInfos) 
{
	WCHAR *szServer = NULL;
	WCHAR *szName = NULL;
	PyObject *obName, *obServer;
	PyNET_STRUCT *pInfo;
	BYTE *buf = NULL;
	PyObject *ret = NULL;
	int typ;
	DWORD err;
	if (!PyArg_ParseTuple(args, "OOi", &obServer, &obName, &typ))
		return NULL;
	if (!PyWinObject_AsWCHAR(obServer, &szServer, TRUE))
		goto done;
	if (!PyWinObject_AsWCHAR(obName, &szName, FALSE))
		goto done;
	if (!FindNET_STRUCT(typ, pInfos, &pInfo))
		goto done;
    Py_BEGIN_ALLOW_THREADS
	err = (*pfn)(szServer, szName, typ, &buf);
    Py_END_ALLOW_THREADS
	if (err) {
		ReturnNetError(fnname,err);
		goto done;
	}
	ret= PyObject_FromNET_STRUCT(pInfo, buf);
done:
	if (buf) NetApiBufferFree(buf);
	PyWinObject_FreeWCHAR(szServer);
	PyWinObject_FreeWCHAR(szName);
	return ret;
}

PyObject *PyDoGetModalsInfo(PyObject *self, PyObject *args, PFNGETMODALSINFO pfn, char *fnname, PyNET_STRUCT *pInfos) 
{
	WCHAR *szServer = NULL;
	PyObject *obServer;
	PyNET_STRUCT *pInfo;
	BYTE *buf = NULL;
	PyObject *ret = NULL;
	int typ;
	DWORD err;
	if (!PyArg_ParseTuple(args, "Oi", &obServer, &typ))
		return NULL;
	if (!PyWinObject_AsWCHAR(obServer, &szServer, TRUE))
		goto done;
	if (!FindNET_STRUCT(typ, pInfos, &pInfo))
		goto done;
    Py_BEGIN_ALLOW_THREADS
	err = (*pfn)(szServer, typ, &buf);
    Py_END_ALLOW_THREADS
	if (err) {
		ReturnNetError(fnname,err);
		goto done;
	}
	ret= PyObject_FromNET_STRUCT(pInfo, buf);
done:
	if (buf) NetApiBufferFree(buf);
	PyWinObject_FreeWCHAR(szServer);
	return ret;
}


/*****************************************************************************/

// @pymethod |win32net|NetMessageBufferSend|sends a string to a registered message alias.
/******************************************************************************/

PyObject *
PyNetMessageBufferSend( PyObject *self, PyObject *args)
{
	DWORD rc;
	TCHAR *serverName = NULL;
	TCHAR *msgName = NULL;
	TCHAR *fromName = NULL;
	TCHAR *message = NULL;
	PyObject *obServerName, *obMsgName, *obFromName, *obMessage;
	PyObject *ret = NULL;
	DWORD msgLen;
	if (!PyArg_ParseTuple(args, "OOOO:NetMessageBufferSend", 
	          &obServerName,  // @pyparm string|domain||Specifies the name of the remote server on which the function is to execute. None or empty string the local computer.
	          &obMsgName, // @pyparm string|userName||Specifies the message name to which the message buffer should be sent.
	          &obFromName, // @pyparm string|fromName||The user the message is to come from, or None for the current user.
	          &obMessage)) // @pyparm string|message||The message text
		return NULL;

	if (!PyWinObject_AsTCHAR(obServerName, &serverName, TRUE))
		goto done;

	if (!PyWinObject_AsTCHAR(obMsgName, &msgName, FALSE))
		goto done;

	if (!PyWinObject_AsTCHAR(obFromName, &fromName, TRUE))
		goto done;

	if (!PyWinObject_AsTCHAR(obMessage, &message, FALSE, &msgLen))
		goto done;

    Py_BEGIN_ALLOW_THREADS
	// message is "BYTE *", but still expects Unicode?  Wonder why not LPTSTR like the other string args?
	rc=NetMessageBufferSend( serverName, msgName, fromName, (BYTE *)message, msgLen * sizeof(TCHAR));
    Py_END_ALLOW_THREADS
	if (rc) {
		ReturnNetError("NetMessageBufferSend",rc);	// @pyseeapi NetMessageBufferSend
		goto done;
	}

	Py_INCREF(Py_None);
	ret = Py_None;
done:
	PyWinObject_FreeTCHAR(serverName);
	PyWinObject_FreeTCHAR(msgName);
	PyWinObject_FreeTCHAR(fromName);
	PyWinObject_FreeTCHAR(message);
	return ret;
}

// @pymethod |win32net|NetMessageNameAdd|Adds a message alias for specified machine
PyObject *PyNetMessageNameAdd(PyObject *self, PyObject *args)
{
	NET_API_STATUS err;
	WCHAR *server=NULL, *alias=NULL;
	PyObject *observer=NULL, *obalias=NULL, *ret=NULL;
	// @pyparm str/unicode|server||Name of server on which to execute - leading backslashes required on NT - local machine used if None
	// @pyparm str/unicode|msgname||Message alias to add, 15 characters max
	if (!PyArg_ParseTuple(args,"OO:NetMessageNameAdd",&observer,&obalias))
		goto done;
	if (!PyWinObject_AsWCHAR(observer,&server,TRUE))
		goto done;
	if (!PyWinObject_AsWCHAR(obalias,&alias,FALSE))
		goto done;

	err=NetMessageNameAdd(server, alias);
	if (err==NERR_Success){
		Py_INCREF(Py_None);
		ret=Py_None;
		}
	else
		ReturnNetError("NetMessageNameAdd",err);
done:
	if (server!=NULL)
		PyWinObject_FreeWCHAR(server);
	if (alias!=NULL)
		PyWinObject_FreeWCHAR(alias);
	return ret;
}

// @pymethod |win32net|NetMessageNameDel|Removes a message alias for specified machine
PyObject *PyNetMessageNameDel(PyObject *self, PyObject *args)
{
	NET_API_STATUS err;
	WCHAR *server=NULL, *alias=NULL;
	PyObject *observer=NULL, *obalias=NULL, *ret=NULL;
	// @pyparm str/unicode|server||Name of server on which to execute - leading backslashes required on NT - local machine used if None
	// @pyparm str/unicode|msgname||Message alias to delete for specified machine
	if (!PyArg_ParseTuple(args,"OO:NetMessageNameDel",&observer,&obalias))
		goto done;
	if (!PyWinObject_AsWCHAR(observer,&server,TRUE))
		goto done;
	if (!PyWinObject_AsWCHAR(obalias,&alias,FALSE))
		goto done;

	err=NetMessageNameDel(server, alias);
	if (err==NERR_Success){
		Py_INCREF(Py_None);
		ret=Py_None;
		}
	else
		ReturnNetError("NetMessageNameDel",err);
done:
	if (server!=NULL)
		PyWinObject_FreeWCHAR(server);
	if (alias!=NULL)
		PyWinObject_FreeWCHAR(alias);
	return ret;
}

// @pymethod |win32net|NetMessageNameEnum|Lists aliases for a computer
PyObject *PyNetMessageNameEnum(PyObject *self, PyObject *args)
{
	NET_API_STATUS err=ERROR_MORE_DATA;
	DWORD maxlen=MAX_PREFERRED_LENGTH, level=0;
#ifdef Py_DEBUG
	maxlen=128;
#endif
	DWORD entriesread=0, totalentries=0, resume_handle=0;
	DWORD msg_ind;
	WCHAR *server=NULL;
	BYTE *buf;
	MSG_INFO_0 *pmsg0;
	PyObject *observer=NULL, *ret=NULL, *msg_item=NULL;
	if (!PyArg_ParseTuple(args,"|O:NetMessageNameEnum",&observer))
		return NULL;
	// @pyparm str/unicode|Server||Name of server on which to execute - leading backslashes required on NT - local machine used if None
	if (observer!=NULL)
		if (!PyWinObject_AsWCHAR(observer,&server,TRUE))
			return NULL;

	ret=PyList_New(0);
	if (!ret)
		return NULL;
	while (TRUE){
		buf=NULL;
		err=NetMessageNameEnum(server,level,&buf,maxlen,&entriesread,&totalentries,&resume_handle);
		if ((err==NERR_Success)||(err==ERROR_MORE_DATA)){
			pmsg0=(MSG_INFO_0 *)buf;
			for (msg_ind=0;msg_ind<entriesread;msg_ind++){
				msg_item=PyWinObject_FromWCHAR(pmsg0->msgi0_name);
				if (!msg_item){
					Py_DECREF(ret);
					ret=NULL;
					break;
					}
				PyList_Append(ret,msg_item);
				Py_DECREF(msg_item);
				pmsg0++;
				}
			}
		else{
			ReturnNetError("NetMessageNameEnum",err);
			Py_DECREF(ret);
			ret=NULL;
			}
		if (buf)
			NetApiBufferFree(buf);
		// With certain buffer size/return size combinations, function can actually return
		// ERROR_MORE_DATA when done, while setting resume_handle to 0, resulting in an infinite
		// loop if you use only the return code
		if ((ret==NULL)||(resume_handle==0))
			break;
		}
	if (server!=NULL)
		PyWinObject_FreeWCHAR(server);
	return ret;
}

PyObject *PyDoSetInfo(PyObject *self, PyObject *args, PFNSETINFO pfn, char *fnname, PyNET_STRUCT *pInfos)
{
	WCHAR *szServer = NULL;
	WCHAR *szName = NULL;
	PyObject *obName, *obServer, *obData;
	PyNET_STRUCT *pInfo;
	BYTE *buf = NULL;
	PyObject *ret = NULL;
	int typ;
	DWORD err = 0;
	if (!PyArg_ParseTuple(args, "OOiO", &obServer, &obName, &typ, &obData))
		return NULL;
	if (!PyWinObject_AsWCHAR(obServer, &szServer, TRUE))
		goto done;
	if (!PyWinObject_AsWCHAR(obName, &szName, FALSE))
		goto done;

	if (!FindNET_STRUCT(typ, pInfos, &pInfo))
		goto done;

	if (!PyObject_AsNET_STRUCT(obData, pInfo, &buf))
		goto done;

    Py_BEGIN_ALLOW_THREADS
	err = (*pfn)(szServer, szName, typ, buf, NULL);
    Py_END_ALLOW_THREADS
	if (err) {
		ReturnNetError(fnname,err);	
		goto done;
	}
	ret= Py_None;
	Py_INCREF(ret);
done:
	if (buf) PyObject_FreeNET_STRUCT(pInfo, buf);
	PyWinObject_FreeWCHAR(szServer);
	PyWinObject_FreeWCHAR(szName);
	return ret;
}

PyObject *PyDoSetModalsInfo(PyObject *self, PyObject *args, PFNSETMODALSINFO pfn, char *fnname, PyNET_STRUCT *pInfos)
{
	WCHAR *szServer = NULL;	
	PyObject *obServer, *obData;
	PyNET_STRUCT *pInfo;
	BYTE *buf = NULL;
	PyObject *ret = NULL;
	int typ;
	DWORD err = 0;
	if (!PyArg_ParseTuple(args, "OiO", &obServer, &typ, &obData))
		return NULL;
	if (!PyWinObject_AsWCHAR(obServer, &szServer, TRUE))
		goto done;

  if (!FindNET_STRUCT(typ, pInfos, &pInfo))
		goto done;

	if (!PyObject_AsNET_STRUCT(obData, pInfo, &buf))
		goto done;

    Py_BEGIN_ALLOW_THREADS
	err = (*pfn)(szServer, typ, buf, NULL);
    Py_END_ALLOW_THREADS
	if (err) {
		ReturnNetError(fnname,err);	
		goto done;
	}
	ret= Py_None;
	Py_INCREF(ret);
done:
	if (buf) PyObject_FreeNET_STRUCT(pInfo, buf);
	PyWinObject_FreeWCHAR(szServer);	
	return ret;
}


PyObject *PyDoAdd(PyObject *self, PyObject *args, PFNADD pfn, char *fnname, PyNET_STRUCT *pInfos)
{
	WCHAR *szServer = NULL;
	PyObject *obServer, *obData;
	PyNET_STRUCT *pInfo;
	BYTE *buf = NULL;
	PyObject *ret = NULL;
	int typ;
	DWORD err = 0;
	if (!PyArg_ParseTuple(args, "OiO", &obServer, &typ, &obData))
		return NULL;
	if (!PyWinObject_AsWCHAR(obServer, &szServer, TRUE))
		goto done;
	if (!FindNET_STRUCT(typ, pInfos, &pInfo))
		goto done;

	if (!PyObject_AsNET_STRUCT(obData, pInfo, &buf))
		goto done;

    Py_BEGIN_ALLOW_THREADS
	err = (*pfn)(szServer, typ, buf, NULL);
    Py_END_ALLOW_THREADS
	if (err) {
		ReturnNetError(fnname,err);	
		goto done;
	}
	ret= Py_None;
	Py_INCREF(ret);
done:
	if (buf) PyObject_FreeNET_STRUCT(pInfo, buf);
	PyWinObject_FreeWCHAR(szServer);
	return ret;
}

PyObject *PyDoDel(PyObject *self, PyObject *args, PFNDEL pfn, char *fnname)
{
	WCHAR *szServer = NULL;
	WCHAR *szName = NULL;
	PyObject *obName, *obServer;
	PyObject *ret = NULL;
	DWORD err = 0;
	if (!PyArg_ParseTuple(args, "OO", &obServer, &obName))
		return NULL;
	if (!PyWinObject_AsWCHAR(obServer, &szServer, TRUE))
		goto done;
	if (!PyWinObject_AsWCHAR(obName, &szName, FALSE))
		goto done;

    Py_BEGIN_ALLOW_THREADS
	err = (*pfn)(szServer, szName);
    Py_END_ALLOW_THREADS
	if (err) {
		ReturnNetError(fnname,err);	
		goto done;
	}
	ret = Py_None;
	Py_INCREF(Py_None);
done:
	PyWinObject_FreeWCHAR(szServer);
	PyWinObject_FreeWCHAR(szName);
	return ret;
}

PyObject *
PyDoGroupDelMembers(PyObject *self, PyObject *args)
{
	WCHAR *szServer = NULL;
	WCHAR *szGroup = NULL;
	PyObject *obServer, *obGroup, *obData;
	PyObject *ret = NULL;
	PyObject *members_tuple=NULL;
	DWORD err = 0;
	BYTE *buf = NULL;
	DWORD i, numEntries;
    DWORD level = 3;
    LOCALGROUP_MEMBERS_INFO_3 *plgrminfo;

	if (!PyArg_ParseTuple(args, "OOO", &obServer, &obGroup, &obData))
		return NULL;
	if (!PyWinObject_AsWCHAR(obServer, &szServer, TRUE))
		goto done;
	if (!PyWinObject_AsWCHAR(obGroup, &szGroup, FALSE))
		goto done;

	members_tuple=PyWinSequence_Tuple(obData, &numEntries);
	if (members_tuple==NULL) {
		PyErr_SetString(PyExc_TypeError, "Data must be a sequence of dictionaries");
		goto done;
	}
	plgrminfo = new LOCALGROUP_MEMBERS_INFO_3[numEntries];
	if (plgrminfo==NULL){
		PyErr_NoMemory();
		goto done;
		}
	// XXX - todo - we should allow a list of LOCALGROUP_MEMBER_INFO items *or* strings
	memset(plgrminfo, 0, sizeof(LOCALGROUP_MEMBERS_INFO_3) * numEntries);
	for (i = 0; i < numEntries; i++) {
		PyObject *sub = PyTuple_GET_ITEM(members_tuple, i);
		if (!PyWinObject_AsWCHAR(sub, &plgrminfo[i].lgrmi3_domainandname))
			goto done;
	}

	Py_BEGIN_ALLOW_THREADS
	err = NetLocalGroupDelMembers(szServer, szGroup, 3, (BYTE *)plgrminfo, numEntries);
	Py_END_ALLOW_THREADS
	if (err) {
		ReturnNetError("NetLocalGroupDelMembers", err);
		goto done;
	}

	ret = Py_None;
	Py_INCREF(Py_None);

done:
	PyWinObject_FreeWCHAR(szServer);
	PyWinObject_FreeWCHAR(szGroup);
	Py_XDECREF(members_tuple);
	if (plgrminfo) {
		for (i=0;i<numEntries;i++) {
			if (plgrminfo[i].lgrmi3_domainandname) {
				PyWinObject_FreeWCHAR(plgrminfo[i].lgrmi3_domainandname);
			}
		}
		delete [] plgrminfo;
	}
	return ret;
}

/* Other misc functions */
// @pymethod <o PyUnicode>|win32net|NetGetDCName|Returns the name of the primary domain controller (PDC).
PyObject *PyNetGetDCName(PyObject *self, PyObject *args)
{
	PyObject *obServer = Py_None, *obDomain = Py_None;
	WCHAR *szServer = NULL, *szDomain = NULL, *result = NULL;
	PyObject *ret = NULL;
	NET_API_STATUS err;

	// @pyparm <o PyUnicode>|server|None|Specifies the name of the remote server on which the function is to execute. If this parameter is None, the local computer is used.
	// @pyparm <o PyUnicode>|domain|None|Specifies the name of the domain. If this parameter is None, the name of the domain controller for the primary domain is used.
	if (!PyArg_ParseTuple(args, "|OO", &obServer, &obDomain))
		return NULL;
	if (!PyWinObject_AsWCHAR(obServer, &szServer, TRUE))
		goto done;
	if (!PyWinObject_AsWCHAR(obDomain, &szDomain, TRUE))
		goto done;
    Py_BEGIN_ALLOW_THREADS
    err = NetGetDCName(szServer, szDomain, (LPBYTE *)&result);
    Py_END_ALLOW_THREADS
	if (err) {
		ReturnNetError("NetGetDCName", err);
		goto done;
	}
	ret = PyWinObject_FromWCHAR(result);
done:
	PyWinObject_FreeWCHAR(szServer);
	PyWinObject_FreeWCHAR(szDomain);
	NetApiBufferFree(result);
	return ret;
}

// @pymethod <o PyUnicode>|win32net|NetGetAnyDCName|Returns the name of any domain controller trusted by the specified server.
PyObject *PyNetGetAnyDCName(PyObject *self, PyObject *args)
{
	PyObject *obServer = Py_None, *obDomain = Py_None;
	WCHAR *szServer = NULL, *szDomain = NULL, *result = NULL;
	PyObject *ret = NULL;
	NET_API_STATUS err;

	// @pyparm <o PyUnicode>|server|None|Specifies the name of the remote server on which the function is to execute. If this parameter is None, the local computer is used.
	// @pyparm <o PyUnicode>|domain|None|Specifies the name of the domain. If this parameter is None, the name of the domain controller for the primary domain is used.
	if (!PyArg_ParseTuple(args, "|OO", &obServer, &obDomain))
		return NULL;
	if (!PyWinObject_AsWCHAR(obServer, &szServer, TRUE))
		goto done;
	if (!PyWinObject_AsWCHAR(obDomain, &szDomain, TRUE))
		goto done;
    Py_BEGIN_ALLOW_THREADS
    err = NetGetAnyDCName(szServer, szDomain, (LPBYTE *)&result);
    Py_END_ALLOW_THREADS
	if (err) {
		ReturnNetError("NetGetAnyDCName", err);
		goto done;
	}
	ret = PyWinObject_FromWCHAR(result);
done:
	PyWinObject_FreeWCHAR(szServer);
	PyWinObject_FreeWCHAR(szDomain);
	NetApiBufferFree(result);
	return ret;
}

#if WINVER >= 0x0500

// @pymethod <o PyUnicode>, int|win32net|NetGetJoinInformation|Retrieves join status information for the specified computer.
static PyObject *PyNetGetJoinInformation(PyObject *self, PyObject *args)
{
	PyObject *obServer = Py_None;
	WCHAR *server = NULL;;
	WCHAR *result = NULL;
	PyObject *ret = NULL;
	NET_API_STATUS err;
	NETSETUP_JOIN_STATUS status;
	if (!PyArg_ParseTuple(args, "|O:NetGetJoinInformation", &obServer))
		return NULL;
	if (pfnNetGetJoinInformation==NULL){
		PyErr_SetString(PyExc_NotImplementedError,"NetGetJoinInformation does not exist on this platform");
		goto done;
	}
	if (!PyWinObject_AsWCHAR(obServer, &server, TRUE))
		goto done;
	Py_BEGIN_ALLOW_THREADS
	err = (*pfnNetGetJoinInformation)(server, &result, &status);
	Py_END_ALLOW_THREADS
	if (err) {
		ReturnNetError("NetGetJoinInformation", err);
		goto done;
	}
	ret = Py_BuildValue("Nl", PyWinObject_FromWCHAR(result), status);
done:
	PyWinObject_FreeWCHAR(server);
	NetApiBufferFree(result);
	return ret;	
}
#endif // WINVER

/*************************************************************************************************************
**



*************************************************************************************************************/
extern PyObject *PyNetUserAdd(PyObject *self, PyObject *args);
extern PyObject *PyNetUserSetInfo(PyObject *self, PyObject *args);
extern PyObject *PyNetUserGetInfo(PyObject *self, PyObject *args);
extern PyObject *PyNetUserDel(PyObject *self, PyObject *args);
extern PyObject *PyNetUserEnum(PyObject *self, PyObject *args);
extern PyObject *PyNetUserChangePassword(PyObject *self, PyObject *args);
extern PyObject *PyNetUserGetLocalGroups( PyObject *self, PyObject *args);
extern PyObject *PyNetUserGetGroups( PyObject *self, PyObject *args);

extern PyObject *PyNetUserModalsGet(PyObject *self, PyObject *args);
extern PyObject *PyNetUserModalsSet(PyObject *self, PyObject *args);

extern PyObject *PyNetGroupGetInfo(PyObject *self, PyObject *args);
extern PyObject *PyNetGroupSetInfo(PyObject *self, PyObject *args);
extern PyObject *PyNetGroupAdd(PyObject *self, PyObject *args);
extern PyObject *PyNetGroupAddUser(PyObject *self, PyObject *args);
extern PyObject *PyNetGroupDel(PyObject *self, PyObject *args);
extern PyObject *PyNetGroupDelUser(PyObject *self, PyObject *args);
extern PyObject *PyNetGroupEnum(PyObject *self, PyObject *args);
extern PyObject *PyNetGroupGetUsers(PyObject *self, PyObject *args);
extern PyObject * PyNetGroupSetUsers(PyObject *self, PyObject *args) ;

extern PyObject *PyNetLocalGroupGetInfo(PyObject *self, PyObject *args);
extern PyObject *PyNetLocalGroupSetInfo(PyObject *self, PyObject *args);
extern PyObject *PyNetLocalGroupAdd(PyObject *self, PyObject *args);
extern PyObject *PyNetLocalGroupAddMembers(PyObject *self, PyObject *args);
extern PyObject *PyNetLocalGroupDelMembers(PyObject *self, PyObject *args);
extern PyObject *PyNetLocalGroupDel(PyObject *self, PyObject *args);
extern PyObject *PyNetLocalGroupEnum(PyObject *self, PyObject *args);
extern PyObject *PyNetLocalGroupGetMembers(PyObject *self, PyObject *args) ;
extern PyObject *PyNetLocalGroupSetMembers(PyObject *self, PyObject *args) ;

extern PyObject *PyNetServerEnum(PyObject *self, PyObject *args);
extern PyObject *PyNetServerGetInfo(PyObject *self, PyObject *args);
extern PyObject *PyNetServerSetInfo(PyObject *self, PyObject *args);

extern PyObject *PyNetShareAdd(PyObject *self, PyObject *args);
extern PyObject *PyNetShareDel(PyObject *self, PyObject *args);
extern PyObject *PyNetShareEnum(PyObject *self, PyObject *args);
extern PyObject *PyNetShareGetInfo(PyObject *self, PyObject *args);
extern PyObject *PyNetShareSetInfo(PyObject *self, PyObject *args);
extern PyObject * PyNetShareCheck(PyObject *self, PyObject *args);

extern PyObject *PyNetWkstaUserEnum(PyObject *self, PyObject *args);
extern PyObject * PyNetWkstaGetInfo(PyObject *self, PyObject *args);
extern PyObject * PyNetWkstaSetInfo(PyObject *self, PyObject *args);
extern PyObject * PyNetWkstaTransportEnum(PyObject *self, PyObject *args);
extern PyObject * PyNetWkstaTransportAdd(PyObject *self, PyObject *args);
extern PyObject * PyNetWkstaTransportDel(PyObject *self, PyObject *args);
extern PyObject * PyNetServerDiskEnum(PyObject *self, PyObject *args);
extern PyObject * PyNetStatisticsGet(PyObject *self, PyObject *args);

// NetUse Functions
extern PyObject * PyNetUseAdd(PyObject *self, PyObject *args);
extern PyObject * PyNetUseDel(PyObject *self, PyObject *args);
extern PyObject * PyNetUseEnum(PyObject *self, PyObject *args);
extern PyObject * PyNetUseGetInfo(PyObject *self, PyObject *args);
extern PyObject * PyNetSessionEnum(PyObject *self, PyObject *args);
extern PyObject * PyNetSessionDel(PyObject *self, PyObject *args);
extern PyObject * PyNetSessionGetInfo(PyObject *self, PyObject *args);
extern PyObject * PyNetFileEnum(PyObject *self, PyObject *args);
extern PyObject * PyNetFileClose(PyObject *self, PyObject *args);
extern PyObject * PyNetFileGetInfo(PyObject *self, PyObject *args);
extern PyObject * PyNetValidateName(PyObject *self, PyObject *args);
extern PyObject * PyNetValidatePasswordPolicy(PyObject *self, PyObject *args);

extern PyObject * PyNetServerComputerNameAdd(PyObject *self, PyObject *args);
extern PyObject * PyNetServerComputerNameDel(PyObject *self, PyObject *args);

/* List of functions exported by this module */
// @module win32net|A module encapsulating the Windows Network API.
static struct PyMethodDef win32net_functions[] = {
#if WINVER >= 0x0500
	{"NetGetJoinInformation",   PyNetGetJoinInformation,    1}, // @pymeth NetGetJoinInformation|Retrieves join status information for the specified computer.
#endif
	{"NetGroupGetInfo",         PyNetGroupGetInfo,          1}, // @pymeth NetGroupGetInfo|Retrieves information about a particular group on a server.
	{"NetGroupGetUsers",        PyNetGroupGetUsers,         1}, // @pymeth NetGroupGetUsers|Enumerates the users in a group.
	{"NetGroupSetUsers",        PyNetGroupSetUsers,         1}, // @pymeth NetGroupSetUsers|Sets the users in a group on server.
	{"NetGroupSetInfo",         PyNetGroupSetInfo,          1}, // @pymeth NetGroupSetInfo|Sets information about a particular group account on a server.
	{"NetGroupAdd",             PyNetGroupAdd,              1}, // @pymeth NetGroupAdd|Creates a new group.
	{"NetGroupAddUser",         PyNetGroupAddUser,          1}, // @pymeth NetGroupAddUser|Adds a user to a group
	{"NetGroupDel",             PyNetGroupDel,              1}, // @pymeth NetGroupDel|Deletes a group.
	{"NetGroupDelUser",         PyNetGroupDelUser,          1}, // @pymeth NetGroupDelUser|Deletes a user from the group
	{"NetGroupEnum",            PyNetGroupEnum,             1}, // @pymeth NetGroupEnum|Enumerates the groups.

	{"NetLocalGroupAdd",        PyNetLocalGroupAdd,              1}, // @pymeth NetGroupAdd|Creates a new group.
	{"NetLocalGroupAddMembers", PyNetLocalGroupAddMembers,  1}, // @pymeth NetLocalGroupAddMembers|Adds users to a local group.
    {"NetLocalGroupDelMembers", PyNetLocalGroupDelMembers,  1}, // @pymeth NetLocalGroupDelMembers|Deletes users from a local group.
	{"NetLocalGroupDel",        PyNetLocalGroupDel,              1}, // @pymeth NetGroupDel|Deletes a group.
	{"NetLocalGroupEnum",       PyNetLocalGroupEnum,             1}, // @pymeth NetGroupEnum|Enumerates the groups.
	{"NetLocalGroupGetInfo",    PyNetLocalGroupGetInfo,          1}, // @pymeth NetGroupGetInfo|Retrieves information about a particular group on a server.
	{"NetLocalGroupGetMembers", PyNetLocalGroupGetMembers,  1}, // @pymeth NetLocalGroupGetMembers|Enumerates the members in a local group.
	{"NetLocalGroupSetInfo",    PyNetLocalGroupSetInfo,          1}, // @pymeth NetGroupSetInfo|Sets information about a particular group account on a server.
	{"NetLocalGroupSetMembers", PyNetLocalGroupSetMembers,  1}, // @pymeth NetLocalGroupSetMembers|Sets the members of a local group.  Any existing members not listed are removed.

	{"NetMessageBufferSend",	PyNetMessageBufferSend,		1}, // @pymeth NetMessageBufferSend|sends a string to a registered message alias.
	{"NetMessageNameAdd",		PyNetMessageNameAdd,		1}, // @pymeth NetMessageNameAdd|Add a message alias for a computer
	{"NetMessageNameDel",		PyNetMessageNameDel,		1}, // @pymeth NetMessageNameDel|Removes a message alias
	{"NetMessageNameEnum",		PyNetMessageNameEnum,		1}, // @pymeth NetMessageNameEnum|List message aliases for a computer

	{"NetServerEnum",           PyNetServerEnum,            1}, // @pymeth NetServerEnum|Retrieves information about all servers of a specific type
	{"NetServerGetInfo",        PyNetServerGetInfo,         1}, // @pymeth NetServerGetInfo|Retrieves information about a particular server.
	{"NetServerSetInfo",        PyNetServerSetInfo,         1}, // @pymeth NetServerSetInfo|Sets information about a particular server.

	{"NetShareAdd",             PyNetShareAdd,              1}, // @pymeth NetShareAdd|Creates a new share.
	{"NetShareDel",             PyNetShareDel,              1}, // @pymeth NetShareDel|Deletes a share
	{"NetShareCheck",           PyNetShareCheck,            1}, // @pymeth NetShareCheck|Checks if server is sharing a device
	{"NetShareEnum",			PyNetShareEnum,				1,	"Obsolete Function,Level 1 call"},	// @pymeth NetShareEnum|Retrieves information about each shared resource on a server. 
	{"NetShareGetInfo",         PyNetShareGetInfo,          1}, // @pymeth NetShareGetInfo|Retrieves information about a particular share on a server.
	{"NetShareSetInfo",         PyNetShareSetInfo,          1}, // @pymeth NetShareSetInfo|Sets information about a particular share on a server.

	{"NetUserAdd",              PyNetUserAdd,               1}, // @pymeth NetUserAdd|Creates a new user.
	{"NetUserChangePassword",	PyNetUserChangePassword,	1}, // @pymeth NetUserChangePassword|Changes a users password on the specified domain.
	{"NetUserEnum",             PyNetUserEnum,              1}, // @pymeth NetUserEnum|Enumerates all users.
	{"NetUserGetGroups",		PyNetUserGetGroups,			1,	"Updated - New Behavior"}, // @pymeth NetUserGetGroups|Returns a list of groups,attributes for all groups for the user.
	{"NetUserGetInfo",          PyNetUserGetInfo,           1}, // @pymeth NetUserGetInfo|Retrieves information about a particular user account on a server.
	{"NetUserGetLocalGroups",	PyNetUserGetLocalGroups,	1,	"Updated - New Behavior"}, // @pymeth NetUserGetLocalGroups|Retrieves a list of local groups to which a specified user belongs.
	{"NetUserSetInfo",          PyNetUserSetInfo,           1}, // @pymeth NetUserSetInfo|Sets information about a particular user account on a server.
	{"NetUserDel",              PyNetUserDel,               1}, // @pymeth NetUserDel|Deletes a user.

	{"NetUserModalsGet",          PyNetUserModalsGet,           1}, // @pymeth NetUserModalsGet|Retrieves global user information on a server.
	{"NetUserModalsSet",          PyNetUserModalsSet,           1}, // @pymeth NetUserModalsSet|Sets global user information on a server.

    {"NetWkstaUserEnum",        PyNetWkstaUserEnum,         1}, // @pymeth NetWkstaUserEnum|Retrieves information about all users currently logged on to the workstation.
    {"NetWkstaGetInfo",         PyNetWkstaGetInfo,          1}, // @pymeth NetWkstaGetInfo|returns information about the configuration elements for a workstation.
    {"NetWkstaSetInfo",         PyNetWkstaSetInfo,          1}, // @pymeth NetWkstaSetInfo|Sets information about the configuration elements for a workstation.
    {"NetWkstaTransportEnum",   PyNetWkstaTransportEnum,    1}, // @pymeth NetWkstaTransportEnum|Retrieves information about transport protocols that are currently managed by the redirector.
    {"NetWkstaTransportAdd",    PyNetWkstaTransportAdd,    1}, // @pymeth NetWkstaTransportAdd|binds the redirector to a transport.
    {"NetWkstaTransportDel",    PyNetWkstaTransportDel,    1}, // @pymeth NetWkstaTransportDel|unbinds transport protocol from the redirector.
    {"NetServerDiskEnum",       PyNetServerDiskEnum,       1}, // @pymeth NetServerDiskEnum|Retrieves the list of disk drives on a server.
    
    {"NetUseAdd",               PyNetUseAdd,               1}, // @pymeth NetUseAdd|Establishes connection between local or NULL device name and a shared resource through redirector.
    {"NetUseDel",               PyNetUseDel,               1}, // @pymeth NetUseDel|Ends connection to a shared resource.
    {"NetUseEnum",              PyNetUseEnum,               1}, // @pymeth NetUseEnum|Enumerates connection between local machine and shared resources on remote computers.
    {"NetUseGetInfo",           PyNetUseGetInfo,           1}, // @pymeth NetUseGetInfo|Get information about locally mapped shared resource on remote computer.

	{"NetGetAnyDCName",         PyNetGetAnyDCName,         1}, // @pymeth NetGetAnyDCName|Returns the name of any domain controller trusted by the specified server.
	{"NetGetDCName",            PyNetGetDCName,            1}, // @pymeth NetGetDCName|Returns the name of the primary domain controller (PDC).

	{"NetSessionEnum",          PyNetSessionEnum,          1}, // @pymeth NetSessionEnum|Returns network session for the server, limited to single client and/or user if specified.
	{"NetSessionDel",           PyNetSessionDel,           1}, // @pymeth NetSessionDel|Delete network session for specified server, client computer and user. Returns None on success.
	{"NetSessionGetInfo",       PyNetSessionGetInfo,       1}, // @pymeth NetSessionGetInfo|Get network session information.
	{"NetFileEnum",             PyNetFileEnum,             1}, // @pymeth NetFileEnum|Returns open file resources for server (single client and/or user may also be passed as criteria).
	{"NetFileClose",            PyNetFileClose,            1}, // @pymeth NetFileClose|Closes file for specified server and file id.
	{"NetFileGetInfo",          PyNetFileGetInfo,          1}, // @pymeth NetFileGetInfo|Get info about files open on the server.
	{"NetStatisticsGet",		PyNetStatisticsGet,		   1}, // @pymeth NetStatisticsGet|Return server or workstation stats
	{"NetServerComputerNameAdd",PyNetServerComputerNameAdd,1}, // @pymeth NetServerComputerNameAdd|Adds an extra network name for a server
	{"NetServerComputerNameDel",PyNetServerComputerNameDel,1}, // @pymeth NetServerComputerNameDel|Deletes an emulated computer name created by <om win32net.PyNetServerComputerNameAdd>
#if WINVER >= 0x0500
	{"NetValidateName",			PyNetValidateName,		   1}, // @pymeth NetValidateName|Verify that computer/domain name is valid for given context
	{"NetValidatePasswordPolicy", PyNetValidatePasswordPolicy, 1}, // @pymeth NetValidatePasswordPolicy|Allows an application to check password compliance against an application-provided account database.
#endif
	{NULL,			NULL}
};

static void AddConstant(PyObject *dict, char *name, long val)
{
  PyObject *nv = PyInt_FromLong(val);
  PyDict_SetItemString(dict, name, nv );
  Py_XDECREF(nv);
}

PYWIN_MODULE_INIT_FUNC(win32net)
{
  PYWIN_MODULE_INIT_PREPARE(win32net, win32net_functions,
                            "A module encapsulating the Windows Network API.");

  PyDict_SetItemString(dict, "error", PyWinExc_ApiError);
  PyDict_SetItemString(dict, "SERVICE_SERVER", PyUnicode_FromWideChar(SERVICE_SERVER,wcslen(SERVICE_SERVER)));
  PyDict_SetItemString(dict, "SERVICE_WORKSTATION", PyUnicode_FromWideChar(SERVICE_WORKSTATION,wcslen(SERVICE_WORKSTATION)));

  HMODULE hmod = LoadLibraryEx(TEXT("netmsg.dll"), NULL, LOAD_LIBRARY_AS_DATAFILE);
  PyWin_RegisterErrorMessageModule(NERR_BASE,
                                   MAX_NERR,
                                   hmod);
  AddConstant(dict, "USE_NOFORCE", USE_NOFORCE);
  AddConstant(dict, "USE_FORCE", USE_FORCE);
  AddConstant(dict, "USE_LOTS_OF_FORCE", USE_LOTS_OF_FORCE);

  HMODULE hmodule=GetModuleHandle(_T("netapi32"));
#if WINVER >= 0x0500
  if (hmodule==NULL)
	  hmodule=LoadLibrary(_T("netapi32"));
  if (hmodule!=NULL) {
	  pfnNetValidateName=(NetValidateNamefunc)GetProcAddress(hmodule,"NetValidateName");
	  pfnNetGetJoinInformation=(NetGetJoinInformationfunc)GetProcAddress(hmodule,"NetGetJoinInformation");
	  pfnNetValidatePasswordPolicy=(NetValidatePasswordPolicyfunc)GetProcAddress(hmodule, "NetValidatePasswordPolicy");
	  pfnNetValidatePasswordPolicyFree=(NetValidatePasswordPolicyFreefunc)GetProcAddress(hmodule, "NetValidatePasswordPolicyFree");
  }
#endif
  PYWIN_MODULE_INIT_RETURN_SUCCESS;
}
