// This file is not processed by Autoduck.  Tags for objects and functions are in win32security.i.

#include "PyWinTypes.h"
#include "structmember.h"
#include "PyWinObjects.h"
#include "PySecurityObjects.h"
#include "win32security_sspi.h"
#include "Lm.h" // for NetApiBufferFree, for some Ds functions.

static PyObject *PyObject_FromDS_NAME_RESULT(DS_NAME_RESULT *dsresult)
{
	PyObject *ret = PyList_New(dsresult->cItems);
	if (!ret) return NULL;
	for (DWORD i=0;i<dsresult->cItems;i++) {
		DS_NAME_RESULT_ITEM *pi = dsresult->rItems + i;
		PyList_SET_ITEM(ret, i,
						Py_BuildValue("iNN", pi->status,
									  PyWinObject_FromWCHAR(pi->pDomain),
									  PyWinObject_FromWCHAR(pi->pName)));
	}
	return ret;
}

// Directory service handle, yet another type of PyHANDLE
class PyDS_HANDLE: public PyHANDLE
{
public:
	PyDS_HANDLE(HANDLE hInit) : PyHANDLE(hInit) {}
	virtual BOOL Close(void) {
		DWORD err;
		if (!m_handle)
			return TRUE;	// already closed or Detached, nothing to do
		if (pfnDsUnBind==NULL){
			// should not happen if functions to create a Ds handle exist ...
			PyErr_SetString(PyExc_SystemError,"Error closing PyDS_HANDLE, DsUnBind is NULL");
			return FALSE;
			}
		err = (*pfnDsUnBind)(&m_handle);
		// ??? This function apparently never returns an error, no matter what you pass to it ???
		if (err==NO_ERROR){
			m_handle = 0;
			return TRUE;
			}
		PyWin_SetAPIError("PyDS_HANDLE::Close", err);
		return FALSE;
	}
	virtual const char *GetTypeName(){
		return "PyDS_HANDLE";
	}
};

// directory service functions for registering target Spns to be used with Kerberos
extern PyObject *PyDsBind(PyObject *self, PyObject *args)
{
	WCHAR *dc=NULL, *domain=NULL;
	PyObject *obdc=Py_None, *obdomain=Py_None;
	PyObject *ret=NULL;
	DWORD err;
	HANDLE dshandle;

	CHECK_PFN(DsBind);
	if (!PyArg_ParseTuple(args, "|OO:DsBind", obdc, obdomain))
		return NULL;
	if (PyWinObject_AsWCHAR(obdc, &dc, TRUE) &&
		PyWinObject_AsWCHAR(obdomain, &domain, TRUE)){
		Py_BEGIN_ALLOW_THREADS
		err=(*pfnDsBind)(dc, domain, &dshandle);
		Py_END_ALLOW_THREADS
		if (err==NO_ERROR)
			ret=new PyDS_HANDLE(dshandle);
		else
			PyWin_SetAPIError("DsBind",err);
		}
	PyWinObject_FreeWCHAR(dc);
	PyWinObject_FreeWCHAR(domain);
	return ret;
}

extern PyObject *PyDsUnBind(PyObject *self, PyObject *args)
{
	DWORD err;
	HANDLE dshandle;
	PyObject *obhandle;

	CHECK_PFN(DsUnBind);
	if (!PyArg_ParseTuple(args, "O:DsUnBind", &obhandle))
		return NULL;
	if (!PyWinObject_AsHANDLE(obhandle, &dshandle))
		return NULL;
	Py_BEGIN_ALLOW_THREADS
	err=(*pfnDsUnBind)(&dshandle);
	Py_END_ALLOW_THREADS
	if (err==NO_ERROR){
		Py_INCREF(Py_None);
		return Py_None;
		}
	PyWin_SetAPIError("DsUnBind",err);
	return NULL;
}


extern PyObject *PyDsGetSpn(PyObject *self, PyObject *args)
{
	DWORD err, cSpns, bufsize, name_cnt;
	DS_SPN_NAME_TYPE ServiceType;
	WCHAR *ServiceClass=NULL, *ServiceName=NULL;
	PyObject *obServiceClass, *obServiceName;
	PyObject *ret=NULL, *tuple_item;
	LPWSTR *Spns=NULL, *InstanceNames=NULL;
	USHORT tuple_index, InstancePort=0, cInstanceNames=0, *InstancePorts=NULL;
	long port_nbr;
	PyObject *obInstanceNames=Py_None, *obInstancePorts=Py_None;
	PyObject *obInstancePorts_tuple=NULL;

	CHECK_PFN(DsGetSpn);
	CHECK_PFN(DsFreeSpnArray);

	if (!PyArg_ParseTuple(args,"lOO|HOO:DsGetSpn", &ServiceType, &obServiceClass, &obServiceName,
		&InstancePort, &obInstanceNames, &obInstancePorts))
		return NULL;
	if (!PyWinObject_AsWCHAR(obServiceClass, &ServiceClass, FALSE))
		goto done;
	if (!PyWinObject_AsWCHAR(obServiceName, &ServiceName, TRUE))
		goto done;
	if (obInstanceNames!=Py_None){
		if (!PyWinObject_AsWCHARArray(obInstanceNames, &InstanceNames, &name_cnt))
			goto done;
		if (name_cnt>USHRT_MAX){
			PyErr_Format(PyExc_ValueError, "Count of InstanceNames cannot exceed %d", USHRT_MAX);
			goto done;
			}
		cInstanceNames=(USHORT)name_cnt;
		}

	if (obInstancePorts!=Py_None){
		if ((obInstancePorts_tuple=PySequence_Tuple(obInstancePorts))==NULL)
			goto done;
		if (PyTuple_Size(obInstancePorts_tuple)!=cInstanceNames){
			PyErr_SetString(PyExc_ValueError,"DsGetSpn: InstancePorts must be same size sequence as InstanceNames");
			goto done;
			}
		bufsize=cInstanceNames * sizeof(USHORT);
		InstancePorts=(USHORT *)malloc(bufsize);
		if (InstancePorts==NULL){
			PyErr_Format(PyExc_MemoryError, "DsGetSpn: Unable to allocate %d bytes", bufsize);
			goto done;
			}
		for (tuple_index=0;tuple_index<cInstanceNames;tuple_index++){
			tuple_item=PyTuple_GET_ITEM(obInstancePorts_tuple,tuple_index);
			// convert a python int to a USHORT
			// ??? any API function to do this other than H format of PyArg_ParseTuple ???
			port_nbr=PyInt_AsLong(tuple_item);
			if ((port_nbr==(unsigned long)-1 && PyErr_Occurred()) || (port_nbr<0)){
				PyErr_Clear();
				PyErr_Format(PyExc_TypeError,"InstancePorts must be a sequence of ints in the range 0-%d",USHRT_MAX);
				goto done;
				}
			if (port_nbr > USHRT_MAX){
				PyErr_Format(PyExc_ValueError, "InstancePorts values cannot exceed %d", USHRT_MAX);
				goto done;
				}
			InstancePorts[tuple_index]=(USHORT)port_nbr;
			}
		}

	Py_BEGIN_ALLOW_THREADS
	err=(*pfnDsGetSpn)(ServiceType, ServiceClass, ServiceName, 
		InstancePort, cInstanceNames, (LPCWSTR *)InstanceNames,
		InstancePorts, &cSpns, &Spns);
	Py_END_ALLOW_THREADS
	if (err!=STATUS_SUCCESS)
		PyWin_SetAPIError("DsGetSpn",err);
	else{
		ret=PyTuple_New(cSpns);
		if (ret!=NULL){
			for (tuple_index=0;tuple_index<cSpns;tuple_index++){
				tuple_item=PyWinObject_FromWCHAR(Spns[tuple_index]);
				if (tuple_item==NULL){
					Py_DECREF(ret);
					ret=NULL;
					break;
					}
				PyTuple_SET_ITEM(ret, tuple_index, tuple_item);
				}
			}
		}
	done:
	if (Spns!=NULL)
		(*pfnDsFreeSpnArray)(cSpns, Spns);
	PyWinObject_FreeWCHARArray(InstanceNames, cInstanceNames);

	if (InstancePorts!=NULL)
		free(InstancePorts);
	if (obInstancePorts_tuple!=NULL)
		Py_DECREF(obInstancePorts_tuple);
	PyWinObject_FreeWCHAR(ServiceClass);
	PyWinObject_FreeWCHAR(ServiceName);
	return ret;
}

extern PyObject *PyDsWriteAccountSpn(PyObject *self, PyObject *args)
{
	DWORD err, spn_cnt;
	HANDLE dshandle;
	DS_SPN_WRITE_OP Operation;
	LPWSTR acct, *spns=NULL;
	PyObject *ret=NULL, *obhandle, *obacct, *obspns;
	CHECK_PFN(DsWriteAccountSpn);
	if (!PyArg_ParseTuple(args, "OlOO:DsWriteAccountSpn", &obhandle, &Operation, &obacct, &obspns))
		return NULL;
	if (!PyWinObject_AsHANDLE(obhandle, &dshandle))
		return NULL;
	if (!PyWinObject_AsWCHAR(obacct, &acct))
		goto done;
	if (!PyWinObject_AsWCHARArray(obspns, &spns, &spn_cnt))
		goto done;
	Py_BEGIN_ALLOW_THREADS
	err=(*pfnDsWriteAccountSpn)(dshandle, Operation, acct, spn_cnt, (LPCWSTR *)spns);
	Py_END_ALLOW_THREADS
	if (err!=STATUS_SUCCESS)
		PyWin_SetAPIError("DsWriteAccountSpn", err);
	else{
		Py_INCREF(Py_None);
		ret=Py_None;
		}
done:
	PyWinObject_FreeWCHAR(acct);
	PyWinObject_FreeWCHARArray(spns, spn_cnt);
	return ret;
}

PyObject *PyDsGetDcName(PyObject *self, PyObject *args, PyObject *kw)
{
	static char *kw_items[]= {
		"computerName","domainName","domainGUID","siteName", "flags", NULL,
	};

	CHECK_PFN(DsGetDcName);
	PyObject *obServer = Py_None, *obDomain = Py_None, *obSiteName = Py_None;
	PyObject *obGUID = Py_None;
	WCHAR *szServer = NULL, *szDomain = NULL, *szSiteName = NULL;
	GUID guidBuf, *pGUID = NULL;
	PyObject *ret = NULL;
	DOMAIN_CONTROLLER_INFO *pdci = NULL;
	DWORD flags = 0;
	DWORD err;
	if (!PyArg_ParseTupleAndKeywords(args, kw, "|OOOOi:DsGetDcName", kw_items,
									 &obServer,
									 &obDomain,
									 &obGUID,
									 &obSiteName,
									 &flags))
		return NULL;
	if (!PyWinObject_AsWCHAR(obServer, &szServer, TRUE))
		goto done;
	if (!PyWinObject_AsWCHAR(obDomain, &szDomain, TRUE))
		goto done;
	if (!PyWinObject_AsWCHAR(obSiteName, &szSiteName, TRUE))
		goto done;
	if (obGUID != Py_None) {
		if (!PyWinObject_AsIID(obGUID, &guidBuf))
			goto done;
		pGUID = &guidBuf;
	}
	Py_BEGIN_ALLOW_THREADS
	err = (*pfnDsGetDcName)(szServer, szDomain, pGUID, szSiteName, flags, &pdci);
	Py_END_ALLOW_THREADS
	if (err) {
		PyWin_SetAPIError("DsGetDcName", err);
		goto done;
	}
	ret = Py_BuildValue("{s:N,s:N,s:i,s:N,s:N,s:N,s:i,s:N,s:N}",
						"DomainControllerName", PyWinObject_FromTCHAR(pdci->DomainControllerName),
						"DomainControllerAddress", PyWinObject_FromTCHAR(pdci->DomainControllerAddress),
						"DomainControllerAddressType", pdci->DomainControllerAddressType,
						"DomainGuid", PyWinObject_FromIID(pdci->DomainGuid),
						"DomainName", PyWinObject_FromTCHAR(pdci->DomainName),
						"DnsForestName", PyWinObject_FromTCHAR(pdci->DnsForestName),
						"Flags", pdci->Flags,
						"DcSiteName", PyWinObject_FromTCHAR(pdci->DcSiteName),
						"ClientSiteName", PyWinObject_FromTCHAR(pdci->ClientSiteName));
	// @rdesc The result is a dictionary with keys having the same name as the
	// Win32 DOMAIN_CONTROLLER_INFO struct.
done:
	PyWinObject_FreeWCHAR(szServer);
	PyWinObject_FreeWCHAR(szDomain);
	PyWinObject_FreeWCHAR(szSiteName);
	NetApiBufferFree(pdci);
	return ret;
}

PyObject *PyDsCrackNames(PyObject *self, PyObject *args)
{
	DWORD err;
	HANDLE dshandle;
	DS_NAME_FLAGS flags;
	DS_NAME_FORMAT formatOffered, formatDesired;
	PyObject *obNames;
	PyObject *ret=NULL, *obhandle;
	LPWSTR *names;
	DWORD cnames;
	PDS_NAME_RESULT dsresult = NULL;

	CHECK_PFN(DsCrackNames);
	CHECK_PFN(DsFreeNameResult);
	if (!PyArg_ParseTuple(args, "OlllO:DsCrackNames", &obhandle, &flags, &formatOffered,
						  &formatDesired, &obNames))
		return NULL;
	if (!PyWinObject_AsHANDLE(obhandle, &dshandle))
		return NULL;
	if (!PyWinObject_AsWCHARArray(obNames, &names, &cnames))
		goto done;
	Py_BEGIN_ALLOW_THREADS
	err=(*pfnDsCrackNames)(dshandle, flags, formatOffered, formatDesired, cnames, names, &dsresult);
	Py_END_ALLOW_THREADS
	if (err!=STATUS_SUCCESS || !dsresult)
		PyWin_SetAPIError("DsCrackNames", err);
	else
		ret = PyObject_FromDS_NAME_RESULT(dsresult);
done:
	PyWinObject_FreeWCHARArray(names, cnames);
	if (dsresult)
		(*pfnDsFreeNameResult)(dsresult);
	return ret;
}

PyObject *PyDsListInfoForServer(PyObject *self, PyObject *args)
{
	DWORD err;
	HANDLE dshandle;
	PyObject *obName;
	PyObject *ret=NULL, *obhandle;
	PDS_NAME_RESULT dsresult = NULL;
	WCHAR *name = NULL;

	CHECK_PFN(DsListInfoForServer);
	CHECK_PFN(DsFreeNameResult);
	// @pyparm int|hds||
	// @pyparm <o PyUnicode>|server||
	if (!PyArg_ParseTuple(args, "OO:DsListInfoForServer", &obhandle, &obName))
		return NULL;
	if (!PyWinObject_AsHANDLE(obhandle, &dshandle))
		return NULL;
	if (!PyWinObject_AsWCHAR(obName, &name))
		goto done;
	Py_BEGIN_ALLOW_THREADS
	err=(*pfnDsListInfoForServer)(dshandle, name, &dsresult);
	Py_END_ALLOW_THREADS
	if (err!=STATUS_SUCCESS || !dsresult)
		PyWin_SetAPIError("DsListInfoForServer", err);
	else
		ret = PyObject_FromDS_NAME_RESULT(dsresult);
done:
	PyWinObject_FreeWCHAR(name);
	if (dsresult)
		(*pfnDsFreeNameResult)(dsresult);
	return ret;
}

PyObject *PyDsListServersForDomainInSite(PyObject *self, PyObject *args)
{
	DWORD err;
	HANDLE dshandle;
	PyObject *obName, *obSite;
	PyObject *ret=NULL, *obhandle;
	PDS_NAME_RESULT dsresult = NULL;
	WCHAR *name = NULL, *site = NULL;

	CHECK_PFN(DsListServersForDomainInSite);
	CHECK_PFN(DsFreeNameResult);
	// @pyparm int|hds||
	// @pyparm <o PyUnicode>|sute||
	if (!PyArg_ParseTuple(args, "OOO:DsListServersForDomainInSite", &obhandle, &obName, &obSite))
		return NULL;
	if (!PyWinObject_AsHANDLE(obhandle, &dshandle))
		return NULL;
	if (!PyWinObject_AsWCHAR(obName, &name))
		goto done;
	if (!PyWinObject_AsWCHAR(obSite, &site))
		goto done;
	Py_BEGIN_ALLOW_THREADS
	err=(*pfnDsListServersForDomainInSite)(dshandle, name, site, &dsresult);
	Py_END_ALLOW_THREADS
	if (err!=STATUS_SUCCESS || !dsresult)
		PyWin_SetAPIError("DsListServersForDomainInSite", err);
	else
		ret = PyObject_FromDS_NAME_RESULT(dsresult);
done:
	PyWinObject_FreeWCHAR(name);
	PyWinObject_FreeWCHAR(site);
	if (dsresult)
		(*pfnDsFreeNameResult)(dsresult);
	return ret;
}

PyObject *PyDsListServersInSite(PyObject *self, PyObject *args)
{
	DWORD err;
	HANDLE dshandle;
	PyObject *obName;
	PyObject *ret=NULL, *obhandle;
	PDS_NAME_RESULT dsresult = NULL;
	WCHAR *name = NULL;

	CHECK_PFN(DsListServersInSite);
	CHECK_PFN(DsFreeNameResult);
	// @pyparm int|hds||
	// @pyparm <o PyUnicode>|sute||
	if (!PyArg_ParseTuple(args, "OO:DsListServersInSite", &obhandle, &obName))
		return NULL;
	if (!PyWinObject_AsHANDLE(obhandle, &dshandle))
		return NULL;
	if (!PyWinObject_AsWCHAR(obName, &name))
		goto done;
	Py_BEGIN_ALLOW_THREADS
	err=(*pfnDsListServersInSite)(dshandle, name, &dsresult);
	Py_END_ALLOW_THREADS
	if (err!=STATUS_SUCCESS || !dsresult)
		PyWin_SetAPIError("DsListServersInSite", err);
	else
		ret = PyObject_FromDS_NAME_RESULT(dsresult);
done:
	PyWinObject_FreeWCHAR(name);
	if (dsresult)
		(*pfnDsFreeNameResult)(dsresult);
	return ret;
}

PyObject *PyDsListSites(PyObject *self, PyObject *args)
{
	DWORD err;
	HANDLE dshandle;
	PyObject *ret=NULL, *obhandle;
	PDS_NAME_RESULT dsresult = NULL;
	
	CHECK_PFN(DsListSites);
	CHECK_PFN(DsFreeNameResult);
	// @pyparm int|hds||
	if (!PyArg_ParseTuple(args, "O:DsListSites", &obhandle))
		return NULL;
	if (!PyWinObject_AsHANDLE(obhandle, &dshandle))
		return NULL;
	Py_BEGIN_ALLOW_THREADS
	err=(*pfnDsListSites)(dshandle, &dsresult);
	Py_END_ALLOW_THREADS
	if (err!=STATUS_SUCCESS || !dsresult)
		PyWin_SetAPIError("DsListSites", err);
	else
		ret = PyObject_FromDS_NAME_RESULT(dsresult);
	if (dsresult)
		(*pfnDsFreeNameResult)(dsresult);
	return ret;
}

PyObject *PyDsListRoles(PyObject *self, PyObject *args)
{
	DWORD err;
	HANDLE dshandle;
	PyObject *ret=NULL, *obhandle;
	PDS_NAME_RESULT dsresult = NULL;
	
	CHECK_PFN(DsListRoles);
	CHECK_PFN(DsFreeNameResult);
	// @pyparm int|hds||
	if (!PyArg_ParseTuple(args, "O:DsListRoles", &obhandle))
		return NULL;
	if (!PyWinObject_AsHANDLE(obhandle, &dshandle))
		return NULL;
	Py_BEGIN_ALLOW_THREADS
	err=(*pfnDsListRoles)(dshandle, &dsresult);
	Py_END_ALLOW_THREADS
	if (err!=STATUS_SUCCESS || !dsresult)
		PyWin_SetAPIError("DsListRoles", err);
	else
		ret = PyObject_FromDS_NAME_RESULT(dsresult);
	if (dsresult)
		(*pfnDsFreeNameResult)(dsresult);
	return ret;
}

PyObject *PyDsListDomainsInSite(PyObject *self, PyObject *args)
{
	DWORD err;
	HANDLE dshandle;
	PyObject *obName;
	PyObject *ret=NULL, *obhandle;
	PDS_NAME_RESULT dsresult = NULL;
	WCHAR *name = NULL;
	
	CHECK_PFN(DsListDomainsInSite);
	CHECK_PFN(DsFreeNameResult);
	// @pyparm int|hds||
	// @pyparm <o PyUnicode>|site||
	if (!PyArg_ParseTuple(args, "OO:DsListDomainsInSite", &obhandle, &obName))
		return NULL;
	if (!PyWinObject_AsHANDLE(obhandle, &dshandle))
		return NULL;
	if (!PyWinObject_AsWCHAR(obName, &name))
		goto done;
	Py_BEGIN_ALLOW_THREADS
	err=(*pfnDsListDomainsInSite)(dshandle, name, &dsresult);
	Py_END_ALLOW_THREADS
	if (err!=STATUS_SUCCESS || !dsresult)
		PyWin_SetAPIError("DsListDomainsInSite", err);
	else
		ret = PyObject_FromDS_NAME_RESULT(dsresult);
done:
	PyWinObject_FreeWCHAR(name);
	if (dsresult)
		(*pfnDsFreeNameResult)(dsresult);
	return ret;
}
