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
PyNetSessionEnum(PyObject *self, PyObject *args)
{
	PyObject *server_name_obj =NULL;
	LPTSTR server_name = NULL;
	PyObject *client_name_obj = NULL;
	LPTSTR client_name = NULL;
	PyObject *user_name_obj =NULL;
	LPTSTR user_name = NULL;

	PyObject *ret_list =NULL;
	PyObject *curr_sess_dict =NULL;
	long sess_time=0;
	long sess_idle_time=0;

	LPSESSION_INFO_0 pBuf0 = NULL;
	LPSESSION_INFO_0 pTmpBuf0;
	LPSESSION_INFO_1 pBuf1 = NULL;
	LPSESSION_INFO_1 pTmpBuf1;
	LPSESSION_INFO_2 pBuf2 = NULL;
	LPSESSION_INFO_2 pTmpBuf2;
	LPSESSION_INFO_10 pBuf10 = NULL;
	LPSESSION_INFO_10 pTmpBuf10;
	LPSESSION_INFO_502 pBuf502 = NULL;
	LPSESSION_INFO_502 pTmpBuf502;

	DWORD buff_len = 0xFFFFFFFF;
	DWORD  dwEntriesRead= 0;
	DWORD dwTotalEntries = 0;
	DWORD dwResumeHandle = 0;
	DWORD i;

	NET_API_STATUS nStatus;
	long rc;
    long info_lvl;

    if (!PyArg_ParseTuple(args, "iO|OO", &info_lvl, &server_name_obj, &client_name_obj, &user_name_obj))
		return NULL;
    if ((info_lvl != 0) && (info_lvl != 1) && (info_lvl !=2) && 
		(info_lvl != 10) && (info_lvl != 502)){
		PyErr_SetString(PyExc_ValueError,"Invalid level for NetSessionEnum");
		return NULL;
	}

	rc = PyWinObject_AsWCHAR(server_name_obj, &server_name, TRUE);
	if (PyTuple_Size(args)>2){
		rc = PyWinObject_AsWCHAR(client_name_obj, &client_name, TRUE);
		// wprintf(client_name);
		}

	if (PyTuple_Size(args)>3){
		rc = PyWinObject_AsWCHAR(user_name_obj, &user_name, TRUE);
		// wprintf(user_name);
		}


   ret_list = PyList_New(0);
   switch (info_lvl){
   		case 0: {
			do{
			    Py_BEGIN_ALLOW_THREADS
			    nStatus = NetSessionEnum(server_name, client_name, user_name, info_lvl,
			       (LPBYTE*)&pBuf0, buff_len, &dwEntriesRead, &dwTotalEntries, &dwResumeHandle);
			    Py_END_ALLOW_THREADS
			
			    if ((nStatus == NERR_Success) || (nStatus == ERROR_MORE_DATA)){
			       if ((pTmpBuf0 = pBuf0) != NULL){
						for (i = 0; (i < dwEntriesRead); i++){
							PyObject* curr_sess_dict  = Py_BuildValue("{s:N}",
								"client_name", PyWinObject_FromWCHAR(pTmpBuf0->sesi0_cname));
							PyList_Append (ret_list, curr_sess_dict);
							Py_DECREF(curr_sess_dict);
							pTmpBuf0++;
						}
			       }
				}	
				else{
					ReturnNetError("NetSessionEnum",nStatus);
					Py_XDECREF(ret_list);
					ret_list=NULL;
				}
				if (pBuf0 != NULL){
					 NetApiBufferFree(pBuf0);
					 pBuf0 = NULL;
				}
			}
			while (nStatus == ERROR_MORE_DATA);
			if (pBuf0 != NULL)
				NetApiBufferFree(pBuf0);
			break;
		}

		case 1:{	
			do{
			     Py_BEGIN_ALLOW_THREADS
			     nStatus = NetSessionEnum(server_name, client_name, user_name, info_lvl,
			        (LPBYTE*)&pBuf1, buff_len, &dwEntriesRead, &dwTotalEntries, &dwResumeHandle);
			     Py_END_ALLOW_THREADS
			
			     if ((nStatus == NERR_Success) || (nStatus == ERROR_MORE_DATA)){
			        if ((pTmpBuf1 = pBuf1) != NULL){
						for (i = 0; (i < dwEntriesRead); i++){
							PyObject* curr_sess_dict  = Py_BuildValue("{s:N,s:N,s:i,s:i,s:i,s:i}",
							"client_name", PyWinObject_FromWCHAR(pTmpBuf1->sesi1_cname),
							"user_name", PyWinObject_FromWCHAR(pTmpBuf1->sesi1_username),
							"num_opens", pTmpBuf1->sesi1_num_opens,
							"active_time", pTmpBuf1->sesi1_time,
							"idle_time", pTmpBuf1->sesi1_idle_time,
							"user_flags", pTmpBuf1->sesi1_user_flags);
							
							PyList_Append (ret_list, curr_sess_dict);
							Py_DECREF(curr_sess_dict);
							pTmpBuf1++;
			           }
			        }
				}
			
				else{
					ReturnNetError("NetSessionEnum",nStatus);
					Py_XDECREF(ret_list);
					ret_list=NULL;
				}
				if (pBuf1 != NULL){
					NetApiBufferFree(pBuf1);
					pBuf1 = NULL;
				}
			}
			while (nStatus == ERROR_MORE_DATA);
			if (pBuf1 != NULL)
				NetApiBufferFree(pBuf1);

			break;
		}

		case 2: {
			do{
			        Py_BEGIN_ALLOW_THREADS
				nStatus = NetSessionEnum(server_name, client_name, user_name, info_lvl,
				(LPBYTE*)&pBuf2, buff_len, &dwEntriesRead, &dwTotalEntries, &dwResumeHandle);
				Py_END_ALLOW_THREADS
				
				if ((nStatus == NERR_Success) || (nStatus == ERROR_MORE_DATA)){
					if ((pTmpBuf2 = pBuf2) != NULL){
						for (i = 0; (i < dwEntriesRead); i++){
							PyObject* curr_sess_dict  = Py_BuildValue("{s:N,s:N,s:i,s:i,s:i,s:i,s:N}",
								"client_name", PyWinObject_FromWCHAR(pTmpBuf2->sesi2_cname),
								"user_name", PyWinObject_FromWCHAR(pTmpBuf2->sesi2_username),
								"num_opens", pTmpBuf2->sesi2_num_opens,
								"active_time", pTmpBuf2->sesi2_time,
								"idle_time", pTmpBuf2->sesi2_idle_time,
								"user_flags", pTmpBuf2->sesi2_user_flags,
								"client_type", PyWinObject_FromWCHAR(pTmpBuf2->sesi2_cltype_name));
							PyList_Append (ret_list, curr_sess_dict);
							Py_DECREF(curr_sess_dict);
							pTmpBuf2++;
						}
					}
				}
				
				else{
					ReturnNetError("NetSessionEnum",nStatus);
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
   
		case 10: {
			do{
			Py_BEGIN_ALLOW_THREADS
      			nStatus = NetSessionEnum(server_name, client_name, user_name, info_lvl,
         			(LPBYTE*)&pBuf10, buff_len, &dwEntriesRead, &dwTotalEntries, &dwResumeHandle);
			Py_END_ALLOW_THREADS
				if ((nStatus == NERR_Success) || (nStatus == ERROR_MORE_DATA)){
				   if ((pTmpBuf10 = pBuf10) != NULL){
						for (i = 0; (i < dwEntriesRead); i++){
							PyObject* curr_sess_dict  = Py_BuildValue("{s:N,s:N,s:i,s:i}",
								"client_name", PyWinObject_FromWCHAR(pTmpBuf10->sesi10_cname),
								"user_name", PyWinObject_FromWCHAR(pTmpBuf10->sesi10_username),
								"active_time", pTmpBuf10->sesi10_time,
								"idle_time", pTmpBuf10->sesi10_idle_time);
								PyList_Append (ret_list, curr_sess_dict);
								Py_DECREF(curr_sess_dict);
								pTmpBuf10++;
						}
				   }
				}
				
				else{
					ReturnNetError("NetSessionEnum",nStatus);
					Py_XDECREF(ret_list);
					ret_list=NULL;
				}
				if (pBuf10 != NULL){
					NetApiBufferFree(pBuf10);
					pBuf10 = NULL;
				}
				
			}
			while (nStatus == ERROR_MORE_DATA);
			if (pBuf10 != NULL)
				NetApiBufferFree(pBuf10);
			break;
		}

		case 502: {
			do{
				Py_BEGIN_ALLOW_THREADS
				nStatus = NetSessionEnum(server_name, client_name, user_name, info_lvl,
				(LPBYTE*)&pBuf502, buff_len, &dwEntriesRead, &dwTotalEntries, &dwResumeHandle);
				Py_END_ALLOW_THREADS
				if ((nStatus == NERR_Success) || (nStatus == ERROR_MORE_DATA)){
					if ((pTmpBuf502 = pBuf502) != NULL){
						for (i = 0; (i < dwEntriesRead); i++){
							PyObject* curr_sess_dict  = Py_BuildValue("{s:N,s:N,s:i,s:i,s:i,s:i,s:N,s:N}",
								"client_name", PyWinObject_FromWCHAR(pTmpBuf502->sesi502_cname),
								"user_name", PyWinObject_FromWCHAR(pTmpBuf502->sesi502_username),
								"num_opens", pTmpBuf502->sesi502_num_opens,
								"active_time", pTmpBuf502->sesi502_time,
								"idle_time", pTmpBuf502->sesi502_idle_time,
								"user_flags", pTmpBuf502->sesi502_user_flags,
								"client_type", PyWinObject_FromWCHAR(pTmpBuf502->sesi502_cltype_name),
								"transport", PyWinObject_FromWCHAR(pTmpBuf502->sesi502_transport));
							PyList_Append (ret_list, curr_sess_dict);
							Py_DECREF(curr_sess_dict);
							pTmpBuf502++;
						}
					}
				}
				else{
					ReturnNetError("NetSessionEnum",nStatus);
					Py_XDECREF(ret_list);
					ret_list=NULL;
				}
				if (pBuf502 != NULL){
					NetApiBufferFree(pBuf502);
					pBuf502 = NULL;
				}
			}
			while (nStatus == ERROR_MORE_DATA);
			if (pBuf502 != NULL)
				NetApiBufferFree(pBuf502);

		}
	}

	PyWinObject_FreeWCHAR(server_name);
	if (client_name != NULL)
		PyWinObject_FreeWCHAR(client_name);
	if (user_name != NULL)
		PyWinObject_FreeWCHAR(user_name);
	return ret_list;
}

PyObject *
PyNetSessionDel(PyObject *self, PyObject *args)

{
	PyObject *server_name_obj =NULL;
	LPTSTR server_name = NULL;
	PyObject *client_name_obj = NULL;
	LPTSTR client_name = NULL;
	PyObject *user_name_obj =NULL;
	LPTSTR user_name = NULL;
	NET_API_STATUS nStatus;
	long rc;


    if (!PyArg_ParseTuple(args, "O|OO", &server_name_obj, &client_name_obj, &user_name_obj))
		return NULL;

	rc = PyWinObject_AsWCHAR(server_name_obj, &server_name, TRUE);
	if (PyTuple_Size(args)>1)
		rc = PyWinObject_AsWCHAR(client_name_obj, &client_name, TRUE);
	if (PyTuple_Size(args)>2)
		rc = PyWinObject_AsWCHAR(user_name_obj, &user_name, TRUE);

   Py_BEGIN_ALLOW_THREADS
   nStatus = NetSessionDel(server_name, client_name, user_name);
   Py_END_ALLOW_THREADS

    PyWinObject_FreeWCHAR(server_name);
	if (client_name != NULL)
		PyWinObject_FreeWCHAR(client_name);
	if (user_name != NULL)
		PyWinObject_FreeWCHAR(user_name);

   if (nStatus == NERR_Success){
	 Py_INCREF(Py_None);
     return Py_None;
	}
   else{
	 ReturnNetError("NetSessionDel",nStatus);
     return NULL;
   }
}

PyObject *
PyNetSessionGetInfo(PyObject *self, PyObject *args)
{
	PyObject *server_name_obj =NULL;
	LPTSTR server_name = NULL;
	PyObject *client_name_obj = NULL;
	LPTSTR client_name = NULL;
	PyObject *user_name_obj =NULL;
	LPTSTR user_name = NULL;
	PyObject *ret_dict =NULL;

	LPSESSION_INFO_0 pTmpBuf0;
	LPSESSION_INFO_1 pTmpBuf1;
	LPSESSION_INFO_2 pTmpBuf2;
	LPSESSION_INFO_10 pTmpBuf10;
	LPSESSION_INFO_502 pTmpBuf502;
	NET_API_STATUS nStatus;

	long rc;
	long info_lvl;

	if (!PyArg_ParseTuple(args, "iOOO", &info_lvl, &server_name_obj, &client_name_obj, &user_name_obj))
		return NULL;
	if ((info_lvl != 0) && (info_lvl != 1) && (info_lvl !=2) && (info_lvl != 10) && (info_lvl != 502)){
		PyErr_SetString(PyExc_ValueError,"Invalid level for NetSessionGetInfo");
		return NULL;
	}

	rc = PyWinObject_AsWCHAR(server_name_obj, &server_name, FALSE);
	rc = PyWinObject_AsWCHAR(client_name_obj, &client_name, FALSE);
	rc = PyWinObject_AsWCHAR(user_name_obj, &user_name, FALSE);

	switch (info_lvl){
		case 0: {
			Py_BEGIN_ALLOW_THREADS
			nStatus = NetSessionGetInfo(server_name, client_name, user_name, info_lvl, (LPBYTE*)&pTmpBuf0);
			Py_END_ALLOW_THREADS
			if (nStatus == NERR_Success)
				ret_dict = Py_BuildValue("{s:N}", "client_name",
							 PyWinObject_FromWCHAR(pTmpBuf0->sesi0_cname));
			else{
				ReturnNetError("NetSessionGetInfo",nStatus);
				ret_dict=NULL;
			}
			if (pTmpBuf0 != NULL){
				NetApiBufferFree(pTmpBuf0);
				pTmpBuf0 = NULL;
			}
			break;
		}

		case 1:{
			Py_BEGIN_ALLOW_THREADS
			nStatus = NetSessionGetInfo(server_name, client_name, user_name, info_lvl, (LPBYTE*)&pTmpBuf1);
			Py_END_ALLOW_THREADS
			if (nStatus == NERR_Success)
				ret_dict = Py_BuildValue("{s:N,s:N,s:i,s:i,s:i,s:i}",
					"client_name", PyWinObject_FromWCHAR(pTmpBuf1->sesi1_cname),
					"user_name", PyWinObject_FromWCHAR(pTmpBuf1->sesi1_username),
					"num_opens", pTmpBuf1->sesi1_num_opens,
					"active_time", pTmpBuf1->sesi1_time,
					"idle_time", pTmpBuf1->sesi1_idle_time,
					"user_flags", pTmpBuf1->sesi1_user_flags);
			else{
				ReturnNetError("NetSessionGetInfo",nStatus);
				ret_dict=NULL;
			}
			if (pTmpBuf1 != NULL){
				NetApiBufferFree(pTmpBuf1);
				pTmpBuf1 = NULL;
			}
			break;
		}

		case 2:{
			Py_BEGIN_ALLOW_THREADS
			nStatus = NetSessionGetInfo(server_name, client_name, user_name, info_lvl, (LPBYTE*)&pTmpBuf2);
			Py_END_ALLOW_THREADS
			if (nStatus == NERR_Success)
				ret_dict = Py_BuildValue("{s:N,s:N,s:i,s:i,s:i,s:i,s:N}",
					"client_name", PyWinObject_FromWCHAR(pTmpBuf2->sesi2_cname),
					"user_name", PyWinObject_FromWCHAR(pTmpBuf2->sesi2_username),
					"num_opens", pTmpBuf2->sesi2_num_opens,
					"active_time", pTmpBuf2->sesi2_time,
					"idle_time", pTmpBuf2->sesi2_idle_time,
					"user_flags", pTmpBuf2->sesi2_user_flags,
					"client_type", PyWinObject_FromWCHAR(pTmpBuf2->sesi2_cltype_name));
			else{
				ReturnNetError("NetSessionGetInfo",nStatus);
				ret_dict=NULL;
			}
			if (pTmpBuf2 != NULL){
				NetApiBufferFree(pTmpBuf2);
				pTmpBuf2 = NULL;
			}
			break;
		}

		case 10:{
			Py_BEGIN_ALLOW_THREADS
			nStatus = NetSessionGetInfo(server_name, client_name, user_name, info_lvl, (LPBYTE*)&pTmpBuf10);
			Py_END_ALLOW_THREADS
			if (nStatus == NERR_Success)
				ret_dict = Py_BuildValue("{s:N,s:N,s:i,s:i}",
					"client_name", PyWinObject_FromWCHAR(pTmpBuf10->sesi10_cname),
					"user_name", PyWinObject_FromWCHAR(pTmpBuf10->sesi10_username),
					"active_time", pTmpBuf10->sesi10_time,
					"idle_time", pTmpBuf10->sesi10_idle_time);
			else{
				ReturnNetError("NetSessionGetInfo",nStatus);
				ret_dict=NULL;
			}
			if (pTmpBuf10 != NULL){
				NetApiBufferFree(pTmpBuf10);
				pTmpBuf10 = NULL;
			}
			break;
		}


		case 502:{
			Py_BEGIN_ALLOW_THREADS
			nStatus = NetSessionGetInfo(server_name, client_name, user_name, info_lvl, (LPBYTE*)&pTmpBuf502);
			Py_END_ALLOW_THREADS
			if (nStatus == NERR_Success)
				ret_dict = Py_BuildValue("{s:N,s:N,s:i,s:i,s:i,s:i,s:N,s:N}",
					"client_name", PyWinObject_FromWCHAR(pTmpBuf502->sesi502_cname),
					"user_name", PyWinObject_FromWCHAR(pTmpBuf502->sesi502_username),
					"num_opens", pTmpBuf502->sesi502_num_opens,
					"active_time", pTmpBuf502->sesi502_time,
					"idle_time", pTmpBuf502->sesi502_idle_time,
					"user_flags", pTmpBuf502->sesi502_user_flags,
					"client_type", PyWinObject_FromWCHAR(pTmpBuf502->sesi502_cltype_name),
					"transport", PyWinObject_FromWCHAR(pTmpBuf502->sesi502_transport));
			else{
				ReturnNetError("NetSessionGetInfo",nStatus);
				ret_dict=NULL;
			}
			if (pTmpBuf502 != NULL){
				NetApiBufferFree(pTmpBuf502);
				pTmpBuf502 = NULL;
			}		
		}
	}

	PyWinObject_FreeWCHAR(server_name);
	PyWinObject_FreeWCHAR(client_name);
	PyWinObject_FreeWCHAR(user_name);
	return ret_dict;
}
