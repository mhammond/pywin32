// Implemented and contributed by Roger Upole.
#include "stdio.h"
#include "assert.h"
#include "windows.h" 
#include "lm.h"
#include "windows.h"
#include "Python.h"
#include "WinUser.h"
#include "PyWinTypes.h"
#include "win32net.h"

PyObject *
PyNetFileEnum(PyObject *self, PyObject *args)

{
	PyObject *server_name_obj =NULL;
	LPTSTR server_name = NULL;
	PyObject *base_path_obj = NULL;
	LPTSTR base_path = NULL;
	PyObject *user_name_obj =NULL;
	LPTSTR user_name = NULL;

	PyObject *ret_list =NULL;
	PyObject *curr_file_list =NULL;

	LPFILE_INFO_3 pBuf3 = NULL;
	LPFILE_INFO_3 pTmpBuf3;
	LPFILE_INFO_2 pBuf2 = NULL;
	LPFILE_INFO_2 pTmpBuf2;

	DWORD buff_len = 0xFFFFFFFF;
	DWORD  dwEntriesRead= 0;
	DWORD dwTotalEntries = 0;
	DWORD dwResumeHandle = 0;
	DWORD i;

	NET_API_STATUS nStatus;
	long rc;
    long info_lvl=NULL;

    if (!PyArg_ParseTuple(args, "iO|OO", &info_lvl, &server_name_obj, &base_path_obj, &user_name_obj))
		return NULL;
    if ((info_lvl != 2) && (info_lvl != 3)){ 
		PyErr_SetString(PyExc_ValueError,"Invalid level for NetFileEnum");
		return NULL;
		}

	rc = PyWinObject_AsWCHAR(server_name_obj, &server_name, TRUE);
	if (PyTuple_Size(args)>2){
		rc = PyWinObject_AsWCHAR(base_path_obj, &base_path, TRUE);
		};
	if (PyTuple_Size(args)>3){
		rc = PyWinObject_AsWCHAR(user_name_obj, &user_name, TRUE);
		};
	
	ret_list = PyList_New(0);

	switch (info_lvl){
		case 2: {
			do{
			    nStatus = NetFileEnum(server_name, base_path, user_name, info_lvl,
			       (LPBYTE*)&pBuf2, buff_len, &dwEntriesRead, &dwTotalEntries, &dwResumeHandle);
			
			    if ((nStatus == NERR_Success) || (nStatus == ERROR_MORE_DATA)){
			       if ((pTmpBuf2 = pBuf2) != NULL){
						for (i = 0; (i < dwEntriesRead); i++){
							PyObject* curr_sess_dict  = Py_BuildValue("{s:i}","id",pTmpBuf2->fi2_id);
							PyList_Append (ret_list, curr_sess_dict);
							Py_DECREF(curr_sess_dict);
							pTmpBuf2++;
						}
			       }
				}	
				else{
					ReturnNetError("NetFileEnum",nStatus);
					Py_XDECREF(ret_list);
					ret_list=NULL;
				}
				if (pBuf2 != NULL){
					 NetApiBufferFree(pBuf2);
					 pBuf2 = NULL;
				}
			}
			while (nStatus == ERROR_MORE_DATA);
			if (pBuf2 != NULL)
				NetApiBufferFree(pBuf2);
			break;
		}
		case 3: {
			do {
				nStatus = NetFileEnum(server_name, base_path, user_name, info_lvl,
                    (LPBYTE*)&pBuf3, buff_len, &dwEntriesRead,
                     &dwTotalEntries, &dwResumeHandle);

				if ((nStatus == NERR_Success) || (nStatus == ERROR_MORE_DATA)){
					if ((pTmpBuf3 = pBuf3) != NULL){
						for (i = 0; (i < dwEntriesRead); i++){
							PyObject* curr_sess_dict  = Py_BuildValue("{s:i,s:i,s:i,s:u,s:u}",
								"id", pTmpBuf3->fi3_id,
								"permissions", pTmpBuf3->fi3_permissions,
								"num_locks", pTmpBuf3->fi3_num_locks,
								"path_name", pTmpBuf3->fi3_pathname,
								"user_name", pTmpBuf3->fi3_username);
							PyList_Append (ret_list, curr_sess_dict);
							Py_DECREF(curr_sess_dict);
							pTmpBuf3++;
						}
					}
				}
				else{
					ReturnNetError("NetFileEnum",nStatus);
					Py_XDECREF(ret_list);
					ret_list=NULL;
				}
				if (pBuf3 != NULL){
					NetApiBufferFree(pBuf3);
					pBuf3 = NULL;
				}
			}
			while (nStatus == ERROR_MORE_DATA);
			if (pBuf3 != NULL)
				NetApiBufferFree(pBuf3);
				
		}
	}

	PyWinObject_FreeWCHAR(server_name);
	if (base_path != NULL)
		PyWinObject_FreeWCHAR(base_path);
	if (user_name != NULL)
		PyWinObject_FreeWCHAR(user_name);
	return ret_list;
}

PyObject *
PyNetFileClose(PyObject *self, PyObject *args)

{
	PyObject *server_name_obj =NULL;
	LPTSTR server_name = NULL;
	NET_API_STATUS nStatus;
	long file_id;
	long rc;

    if (!PyArg_ParseTuple(args, "Oi", &server_name_obj, &file_id))
		return NULL;
	rc = PyWinObject_AsWCHAR(server_name_obj, &server_name, TRUE);
	nStatus=NetFileClose(server_name, file_id);

    PyWinObject_FreeWCHAR(server_name);

	if (nStatus == NERR_Success){
		Py_INCREF(Py_None);
		return Py_None;
	}
	else{
		ReturnNetError("NetFileClose",nStatus);
		return NULL;
	}
}

PyObject *
PyNetFileGetInfo(PyObject *self, PyObject *args)

{
	PyObject *server_name_obj = NULL;
	PyObject *ret_dict = NULL;
	LPTSTR server_name = NULL;
	long info_lvl=NULL;
	DWORD file_id = NULL;
	long rc;
	LPFILE_INFO_3 pTmpBuf3 = NULL;
	LPFILE_INFO_2 pTmpBuf2= NULL;
	NET_API_STATUS nStatus;

	if (!PyArg_ParseTuple(args, "iOi", &info_lvl, &server_name_obj, &file_id))
		return NULL;
	if ((info_lvl != 2) && (info_lvl != 3)){ 
		PyErr_SetString(PyExc_ValueError,"Invalid level for NetFileGetInfo");
		return NULL;
		}

	rc = PyWinObject_AsWCHAR(server_name_obj, &server_name, FALSE);
	switch (info_lvl){
		case 2:{
			nStatus = NetFileGetInfo(server_name, file_id, info_lvl,
			       (LPBYTE*)&pTmpBuf2);
			
			if (nStatus == NERR_Success)
				ret_dict = Py_BuildValue("{s:i}","id",pTmpBuf2->fi2_id);	
			else{
				ReturnNetError("NetFileEnum",nStatus);
				ret_dict=NULL;
			}
			if (pTmpBuf2 != NULL)
				NetApiBufferFree(pTmpBuf2);
			break;
		}
		case 3: {
			nStatus = NetFileGetInfo(server_name, file_id, info_lvl,
			       (LPBYTE*)&pTmpBuf3);
			
			if (nStatus == NERR_Success)
				ret_dict = Py_BuildValue("{s:i,s:i,s:i,s:u,s:u}",
					"id", pTmpBuf3->fi3_id,
					"permissions", pTmpBuf3->fi3_permissions,
					"num_locks", pTmpBuf3->fi3_num_locks,
					"path_name", pTmpBuf3->fi3_pathname,
					"user_name", pTmpBuf3->fi3_username);
			else{
				ReturnNetError("NetFileEnum",nStatus);
				ret_dict=NULL;
			}
			if (pTmpBuf3 != NULL)
				NetApiBufferFree(pTmpBuf3);
		}
	}

	PyWinObject_FreeWCHAR(server_name);
	return ret_dict;
}
