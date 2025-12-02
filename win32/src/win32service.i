/* File : win32service.i */

%module win32service // An interface to the Windows NT Service API


%include "typemaps.i"
%include "pywin32.i"

%{
#undef PyHANDLE
#include "PyWinObjects.h"
#include "Dbt.h" // for device events

typedef BOOL (WINAPI *QueryServiceStatusExfunc)(SC_HANDLE,SC_STATUS_TYPE,LPBYTE,DWORD,LPDWORD);
QueryServiceStatusExfunc fpQueryServiceStatusEx=NULL;
typedef BOOL (WINAPI *ChangeServiceConfig2func)(SC_HANDLE,DWORD,LPVOID);
ChangeServiceConfig2func fpChangeServiceConfig2=NULL;
typedef BOOL (WINAPI *QueryServiceConfig2func)(SC_HANDLE,DWORD,LPBYTE,DWORD,LPDWORD);
QueryServiceConfig2func fpQueryServiceConfig2=NULL;
typedef BOOL (WINAPI *EnumServicesStatusExfunc)(SC_HANDLE,SC_ENUM_TYPE,DWORD,DWORD,
	LPBYTE,DWORD,LPDWORD,LPDWORD,LPDWORD,LPCTSTR);
EnumServicesStatusExfunc fpEnumServicesStatusEx=NULL;

// according to msdn, 256 is limit for service names and service display names
#define MAX_SERVICE_NAME_LEN 256

%}

%init %{

	if (PyType_Ready(&PyHWINSTAType) == -1 ||
		PyType_Ready(&PyHDESKType) == -1)
		return NULL;

	// All errors raised by this module are of this type.
	PyDict_SetItemString(d, "error", PyWinExc_ApiError);
	PyDict_SetItemString(d, "HWINSTAType", (PyObject *)&PyHWINSTAType);
	PyDict_SetItemString(d, "HDESKType", (PyObject *)&PyHDESKType);
	FARPROC fp;
	HMODULE hmod = PyWin_GetOrLoadLibraryHandle("advapi32.dll");
	if (hmod != NULL) {
		fp=GetProcAddress(hmod,"QueryServiceStatusEx");
		if (fp!=NULL)
			fpQueryServiceStatusEx=(QueryServiceStatusExfunc)fp;
		fp=GetProcAddress(hmod,"ChangeServiceConfig2W");
		if (fp!=NULL)
			fpChangeServiceConfig2=(ChangeServiceConfig2func)fp;
		fp=GetProcAddress(hmod,"QueryServiceConfig2W");
		if (fp!=NULL)
			fpQueryServiceConfig2=(QueryServiceConfig2func)fp;
		fp=GetProcAddress(hmod,"EnumServicesStatusExW");
		if (fp!=NULL)
			fpEnumServicesStatusEx=(EnumServicesStatusExfunc)fp;
	}
%}

%{
#include "structmember.h"

// @object PySC_HANDLE|Handle to a service or service control manager.
//	This is a variant of <o PyHANDLE> that releases its handle using CloseServiceHandle.
class PySC_HANDLE: public PyHANDLE
{
public:
	PySC_HANDLE(HANDLE hInit) : PyHANDLE(hInit) {}
	virtual BOOL Close(void){
		BOOL ret=TRUE;
		if (m_handle!=NULL){
			ret=CloseServiceHandle((SC_HANDLE)m_handle);
			m_handle = NULL;
			}
		if (!ret)
			PyWin_SetAPIError("CloseServiceHandle");
		return ret;
		}
	virtual const char *GetTypeName(){
		return "PySC_HANDLE";
		}
};

PyObject *PyWinObject_FromSC_HANDLE(SC_HANDLE sch)
{
	PyObject *ret=new PySC_HANDLE(sch);
	if (ret==NULL)
		PyErr_NoMemory();
	return ret;
}

// @object PyHWINSTA|Wrapper for a handle to a window station - returned by CreateWindowStation, OpenWindowStation, or GetProcessWindowStation
class PyHWINSTA : public PyHANDLE
{
public:
	PyHWINSTA(HWINSTA hwinsta);
	~PyHWINSTA(void);
	static void deallocFunc(PyObject *ob);
	static PyObject *EnumDesktops(PyObject *self, PyObject *args);
	static PyObject *SetProcessWindowStation(PyObject *self, PyObject *args);
	static PyObject *CloseWindowStation(PyObject *self, PyObject *args);
	static struct PyMemberDef members[];
	static struct PyMethodDef methods[];
	static PyObject *PyHWINSTA_new(PyTypeObject *tp, PyObject *args, PyObject *kwargs);
};

struct PyMethodDef PyHWINSTA::methods[] = {
	{"EnumDesktops",			PyHWINSTA::EnumDesktops, METH_VARARGS, "List desktop names within the window station"}, 	// @pymeth EnumDesktops|List desktop names within the window station
	{"SetProcessWindowStation",	PyHWINSTA::SetProcessWindowStation, METH_VARARGS, "Associates the calling process with the window station"}, // @pymeth SetProcessWindowStation|Associates the calling process with the window station
	{"CloseWindowStation",		PyHWINSTA::CloseWindowStation, METH_VARARGS, "Closes the window station handle"}, // @pymeth CloseWindowStation|Closes the window station handle
	{"Detach",					PyHANDLE::Detach, METH_VARARGS, "Releases reference to handle without closing it"}, //@pymeth Detach|Releases reference to handle without closing it
	{NULL}
};

struct PyMemberDef PyHWINSTA::members[] = {
	// ??? offsetof not working correctly ???
	// {"handle", T_LONG, offsetof(PyHWINSTA,m_handle), READONLY, "For use where an integer handle is required"},
	{NULL}
};

PyObject *PyHWINSTA::PyHWINSTA_new(PyTypeObject *tp, PyObject *args, PyObject *kwargs)
{
	static char *keywords[]={"handle",0};
	HWINSTA hwinsta;
	PyObject *obh;
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O", keywords, &obh))
		return NULL;
	if (!PyWinObject_AsHANDLE(obh, (HANDLE *)&hwinsta))
		return NULL;
	return new PyHWINSTA(hwinsta);
}

PyTypeObject PyHWINSTAType =
{
	PYWIN_OBJECT_HEAD
	"PyHWINSTA",
	sizeof(PyHWINSTA),
	0,
	PyHWINSTA::deallocFunc,		/* tp_dealloc */
	0,							/* tp_print */
	0,							/* tp_getattr */
	0,							/* tp_setattr */
	0,							/* tp_compare */
	0,							/* tp_repr */
	PyHANDLEType.tp_as_number,	/* tp_as_number */
	0,							/* tp_as_sequence */
	0,							/* tp_as_mapping */
	0,
	0,							/* tp_call */
	0,							/* tp_str */
	PyObject_GenericGetAttr,
	PyObject_GenericSetAttr,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	PyHWINSTA::methods,
	PyHWINSTA::members,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	PyHWINSTA::PyHWINSTA_new
};

#define PyHWINSTA_Check(ob)	((ob)->ob_type == &PyHWINSTAType)

PyHWINSTA::PyHWINSTA(HWINSTA hwinsta) : PyHANDLE((HANDLE)hwinsta)
{
	ob_type = &PyHWINSTAType;
}
PyHWINSTA::~PyHWINSTA(void)
{
	::CloseWindowStation((HWINSTA)m_handle);
}

void PyHWINSTA::deallocFunc(PyObject *ob)
{
	delete (PyHWINSTA *)ob;
}


// @object PyHDESK|Object representing a handle to a desktop, created by
// <om win32service.CreateDesktop>, <om win32service.GetThreadDesktop> and <om win32service.OpenDesktop>.

class PyHDESK : public PyHANDLE
{
public:
	PyHDESK(HDESK hdesk);
	~PyHDESK(void);
	static void deallocFunc(PyObject *ob);
	static PyObject *SetThreadDesktop(PyObject *self, PyObject *args);
	static PyObject *EnumDesktopWindows(PyObject *self, PyObject *args);
	static PyObject *SwitchDesktop(PyObject *self, PyObject *args);
	static PyObject *CloseDesktop(PyObject *self, PyObject *args);
	static struct PyMemberDef members[];
	static struct PyMethodDef methods[];
	static PyObject *PyHDESK_new(PyTypeObject *tp, PyObject *args, PyObject *kwargs);
};

struct PyMethodDef PyHDESK::methods[] = {
	{"SetThreadDesktop",	PyHDESK::SetThreadDesktop, METH_VARARGS, "Assigns desktop to calling thread"}, // @pymeth SetThreadDesktop|Assigns desktop to calling thread
	{"EnumDesktopWindows",	PyHDESK::EnumDesktopWindows, METH_VARARGS, "Lists all top-level windows on the desktop"}, 	// @pymeth EnumDesktopWindows|Lists all top-level windows on the desktop
	{"SwitchDesktop",		PyHDESK::SwitchDesktop, METH_VARARGS, "Activates the desktop"}, 	// @pymeth SwitchDesktop|Activates the desktop
	{"CloseDesktop",		PyHDESK::CloseDesktop, METH_VARARGS, "Closes the handle"}, //@pymeth CloseDesktop|Closes the desktop handle
	{"Detach",				PyHANDLE::Detach, METH_VARARGS, "Releases reference to handle without closing it"}, //@pymeth Detach|Releases reference to handle without closing it
	{NULL}
};

struct PyMemberDef PyHDESK::members[] = {
	// {"handle", T_LONG, offsetof(PyHDESK,m_handle), READONLY, "For use where an integer handle is required"},
	{NULL}
};

PyObject *PyHDESK::PyHDESK_new(PyTypeObject *tp, PyObject *args, PyObject *kwargs)
{
	static char *keywords[]={"handle",0};
	HDESK hdesk;
	PyObject *obh;
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O", keywords, &obh))
		return NULL;
	if (!PyWinObject_AsHANDLE(obh, (HANDLE *)&hdesk))
		return NULL;
	return new PyHDESK(hdesk);
}

PyTypeObject PyHDESKType =
{
	PYWIN_OBJECT_HEAD
	"PyHDESK",
	sizeof(PyHDESK),
	0,
	PyHDESK::deallocFunc,		/* tp_dealloc */
	0,							/* tp_print */
	0,							/* tp_getattr */
	0,							/* tp_setattr */
	0,							/* tp_compare */
	0,							/* tp_repr */
	PyHANDLEType.tp_as_number,	/* tp_as_number */
	0,							/* tp_as_sequence */
	0,							/* tp_as_mapping */
	0,							/* tp_hash */
	0,							/* tp_call */
	0,							/* tp_str */
	PyObject_GenericGetAttr,
	PyObject_GenericSetAttr,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	PyHDESK::methods,
	PyHDESK::members,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	PyHDESK::PyHDESK_new
};

#define PyHDESK_Check(ob)	((ob)->ob_type == &PyHDESKType)

PyHDESK::PyHDESK(HDESK hdesk) : PyHANDLE((HANDLE)hdesk)
{
	ob_type = &PyHDESKType;
}

PyHDESK::~PyHDESK(void)
{
	::CloseDesktop((HDESK)m_handle);
}

void PyHDESK::deallocFunc(PyObject *ob)
{
	delete (PyHDESK *)ob;
}

BOOL CALLBACK EnumWindowStationProc(LPWSTR winstaname, LPARAM ret)
{
	PyObject *obwinstaname=PyWinObject_FromWCHAR(winstaname);
	if (obwinstaname==NULL)
		return FALSE;
	PyList_Append((PyObject *)ret,obwinstaname);
	Py_DECREF(obwinstaname);
	return TRUE;
}

BOOL CALLBACK EnumDesktopsProc(LPWSTR desktopname, LPARAM ret)
{
	PyObject *obdesktopname=PyWinObject_FromWCHAR(desktopname);
	if (obdesktopname==NULL)
		return FALSE;
	PyList_Append((PyObject *)ret,obdesktopname);
	Py_DECREF(obdesktopname);
	return TRUE;
}

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM ret)
{
	PyObject *obhandle=PyWinObject_FromHANDLE(hwnd);
	if (obhandle==NULL)
		return FALSE;
	PyList_Append((PyObject *)ret,obhandle);
	Py_DECREF(obhandle);
	return TRUE;
}

// @pymethod (string,...)|PyHWINSTA|EnumDesktops|Lists names of desktops in the window station
PyObject *PyHWINSTA::EnumDesktops(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ":EnumDesktops"))
		return NULL;
	PyObject *ret=PyList_New(0);
	if (ret==NULL)
		return NULL;
	if (!::EnumDesktopsW((HWINSTA)((PyHWINSTA *)self)->m_handle, EnumDesktopsProc, (LPARAM)ret)){
		Py_DECREF(ret);
		ret=NULL;
		if (!PyErr_Occurred())
			PyWin_SetAPIError("EnumDesktops",GetLastError());
		}
	return ret;
}

// @pymethod |PyHWINSTA|SetProcessWindowStation|Associates the calling process with the window station
PyObject *PyHWINSTA::SetProcessWindowStation(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ":SetProcessWindowStation"))
		return NULL;
	if (!::SetProcessWindowStation((HWINSTA)((PyHWINSTA *)self)->m_handle))
		return PyWin_SetAPIError("SetProcessWindowStation",GetLastError());
	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod |PyHWINSTA|CloseWindowStation|Closes the window station handle
// @comm This function cannot close the handle to current process's window station
PyObject *PyHWINSTA::CloseWindowStation(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ":CloseWindowStation"))
		return NULL;
	if (!::CloseWindowStation((HWINSTA)((PyHWINSTA *)self)->m_handle))
		return PyWin_SetAPIError("CloseWindowStation",GetLastError());
	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod |PyHDESK|SetThreadDesktop|Assigns this desktop to the calling thread
PyObject *PyHDESK::SetThreadDesktop(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ":SetThreadDesktop"))
		return NULL;
	if (!::SetThreadDesktop((HDESK)((PyHDESK *)self)->m_handle))
		return PyWin_SetAPIError("SetThreadDesktop",GetLastError());
	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod |PyHDESK|SwitchDesktop|Activates the desktop
PyObject *PyHDESK::SwitchDesktop(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ":SwitchDesktop"))
		return NULL;
	if (!::SwitchDesktop((HDESK)((PyHDESK *)self)->m_handle))
		return PyWin_SetAPIError("SwitchDesktop",GetLastError());
	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod |PyHDESK|CloseDesktop|Closes the desktop handle
PyObject *PyHDESK::CloseDesktop(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ":CloseDesktop"))
		return NULL;
	if (!::CloseDesktop((HDESK)((PyHDESK *)self)->m_handle))
		return PyWin_SetAPIError("CloseDesktop",GetLastError());
	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod (<o PyHANDLE>,...)|PyHDESK|EnumDesktopWindows|Returns a list of handles to all top-level windows on desktop
PyObject *PyHDESK::EnumDesktopWindows(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ":EnumDesktopWindows"))
		return NULL;
	PyObject *ret=PyList_New(0);
	if (ret==NULL)
		return NULL;
	if (!::EnumDesktopWindows((HDESK)((PyHDESK *)self)->m_handle, EnumWindowsProc, (LPARAM)ret)){
		Py_DECREF(ret);
		ret=NULL;
		if (!PyErr_Occurred())
			PyWin_SetAPIError("EnumDesktopWindows",GetLastError());
		}
	return ret;
}
%}

// @pyswig <o PyHDESK>|GetThreadDesktop|Retrieves a handle to the desktop for a thread
%native(GetThreadDesktop) PyGetThreadDesktop;
%{
PyObject *PyGetThreadDesktop(PyObject *self, PyObject *args)
{
	DWORD tid;
	HDESK hdesk;
	// @pyparm int|ThreadId||Id of thread
	if (!PyArg_ParseTuple(args, "l:GetThreadDesktop", &tid))
		return NULL;
	hdesk=GetThreadDesktop(tid);
	if (hdesk==NULL)
		return PyWin_SetAPIError("GetThreadDesktop",GetLastError());
	return new PyHDESK(hdesk);
}
%}

// @pyswig (string,,...)|EnumWindowStations|Lists names of window stations
// @comm Only window stations for which you have WINSTA_ENUMERATE access will be returned
%native(EnumWindowStations) PyEnumWindowStations;
%{
PyObject *PyEnumWindowStations(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ":EnumWindowStations"))
		return NULL;
	PyObject *ret=PyList_New(0);
	if (ret==NULL)
		return NULL;
	if (!EnumWindowStationsW(EnumWindowStationProc, (LPARAM)ret)){
		Py_DECREF(ret);
		ret=NULL;
		if (!PyErr_Occurred())
			PyWin_SetAPIError("EnumWindowStations",GetLastError());
		}
	return ret;
}
%}

// @pyswig |GetUserObjectInformation|Returns specified type of info about a window station or desktop
// @rdesc Return type is dependent on UOI_* constant passed in
%native(GetUserObjectInformation) PyGetUserObjectInformation;
%{
PyObject *PyGetUserObjectInformation(PyObject *self, PyObject *args)
{
	HANDLE handle;
	DWORD origbuflen=128, reqdbuflen=0, err;
#ifdef Py_DEBUG
	origbuflen=3;
#endif
	void *buf=NULL;
	PyObject *obhandle, *ret=NULL;
	BOOL bsuccess;
	int info_type;
	// @pyparm <o PyHANDLE>|Handle||Handle to window station or desktop
	// @pyparm int|type||Type of info to return, one of UOI_FLAGS,UOI_NAME, UOI_TYPE, or UOI_USER_SID
	if (!PyArg_ParseTuple(args, "Ol:GetUserObjectInformation", &obhandle, &info_type))
		return NULL;
	if (!PyWinObject_AsHANDLE(obhandle, &handle))
		return NULL;
	buf=malloc(origbuflen);
	if (buf==NULL)
		return PyErr_Format(PyExc_MemoryError, "GetUserObjectInformation unable to allocate %d bytes", origbuflen);
	bsuccess=GetUserObjectInformationW(handle, info_type, buf, origbuflen, &reqdbuflen);
	if (!bsuccess){
		err=GetLastError();
		if (err==ERROR_INSUFFICIENT_BUFFER){
			free(buf);
			buf=malloc(reqdbuflen);
			if (buf==NULL)
				return PyErr_Format(PyExc_MemoryError, "GetUserObjectInformation unable to allocate %d bytes", reqdbuflen);
			bsuccess=GetUserObjectInformationW(handle, info_type, buf, reqdbuflen, &reqdbuflen);
			if (!bsuccess)
				err=GetLastError();
			}
		}
	if (!bsuccess)
		PyWin_SetAPIError("GetUserObjectInformation",err);
	else
		switch(info_type){
			case UOI_NAME:
			case UOI_TYPE:{
				ret=PyWinObject_FromWCHAR((WCHAR *)buf);
				break;
				}
			case UOI_USER_SID:{
				if (reqdbuflen==0){
					Py_INCREF(Py_None);
					ret=Py_None;
					}
				else
					ret=PyWinObject_FromSID((PSID)buf);
				break;
				}
			case UOI_FLAGS:{
				ret=Py_BuildValue("{s:N,s:N,s:l}",
				 "Inherit",		PyBool_FromLong(((USEROBJECTFLAGS *)buf)->fInherit),
				 "Reserved",	PyBool_FromLong(((USEROBJECTFLAGS *)buf)->fReserved),
				 "Flags",		((USEROBJECTFLAGS *)buf)->dwFlags);
				 break;
				 }
			default:
				PyErr_SetString(PyExc_NotImplementedError,"Type of information is not supported yet");
				break;
			}
	if (buf)
		free (buf);
	return ret;
}
%}

// @pyswig |SetUserObjectInformation|Set specified type of info for a window station or desktop object
// @comm Currently only UOI_FLAGS supported
%native(SetUserObjectInformation) PySetUserObjectInformation;
%{
PyObject *PySetUserObjectInformation(PyObject *self, PyObject *args)
{
	HANDLE handle;
	USEROBJECTFLAGS uof;
	DWORD buflen=sizeof(USEROBJECTFLAGS);
	int info_type=UOI_FLAGS;
	PyObject *obhandle, *obinfo;
	static char *uof_members[]={"Inherit", "Reserved", "Flags", 0};
	static char *uof_format="Object must be a dictionary containing {'Inherit':bool, 'Reserved':bool, 'Flags':int}";
	// @pyparm <o PyHANDLE>|Handle||Handle to window station or desktop
	// @pyparm object|info||Information to set for handle, currently only a dictionary representing USEROBJECTFLAGS struct
	// @pyparm int|type|UOI_FLAGS|Type of info to set, currently only accepts UOI_FLAGS
	if (!PyArg_ParseTuple(args,"OO|l:SetUserObjectInformation", &obhandle, &obinfo, &info_type))
		return NULL;
	if (!PyWinObject_AsHANDLE(obhandle, &handle))
		return NULL;
	if (info_type!=UOI_FLAGS){
		PyErr_SetString(PyExc_TypeError,"Only UOI_FLAGS currently supported");
		return NULL;
		}
	if (!PyDict_Check(obinfo)){
		PyErr_SetString(PyExc_TypeError,uof_format);
		return NULL;
		}
	PyObject *dummy_tuple=PyTuple_New(0);
	if (!PyArg_ParseTupleAndKeywords(dummy_tuple, obinfo, "lll", uof_members, &uof.fInherit, &uof.fReserved, &uof.dwFlags)){
	 	PyErr_SetString(PyExc_TypeError,uof_format);
		return NULL;
		}
	if (!SetUserObjectInformationW(handle, info_type, (void *)&uof, buflen))
		return PyWin_SetAPIError("SetUserObjectInformation",GetLastError());
	Py_INCREF(Py_None);
	return Py_None;
}
%}


// @pyswig <o PyHWINSTA>|OpenWindowStation|Returns a handle to the specified window station
%native(OpenWindowStation) PyOpenWindowStation;
%{
PyObject *PyOpenWindowStation(PyObject *self, PyObject *args)
{
	WCHAR *winsta_name=NULL;
	BOOL Inherit;
	ACCESS_MASK DesiredAccess;
	PyObject *obwinsta_name, *ret=NULL;
	HWINSTA hwinsta;
	// @pyparm string|szWinSta||Name of window station
	// @pyparm Bool|Inherit||Allow handle to be inherited by subprocesses
	// @pyparm int|DesiredAccess||Bitmask of access types
	if (!PyArg_ParseTuple(args,"Oll:OpenWindowStation",&obwinsta_name, &Inherit, &DesiredAccess))
		return NULL;
	if (!PyWinObject_AsWCHAR(obwinsta_name,&winsta_name,FALSE))
		return NULL;
	hwinsta=OpenWindowStationW(winsta_name,Inherit,DesiredAccess);
	if (hwinsta==NULL)
		PyWin_SetAPIError("OpenWindowStation",GetLastError());
	else
		ret= new PyHWINSTA(hwinsta);
	PyWinObject_FreeWCHAR(winsta_name);
	return ret;
}
%}

// @pyswig <o PyHDESK>|OpenDesktop|Opens a handle to a desktop
%native(OpenDesktop) PyOpenDesktop;
%{
PyObject *PyOpenDesktop(PyObject *self, PyObject *args)
{
	WCHAR *desktop_name=NULL;
	BOOL Inherit;
	ACCESS_MASK DesiredAccess;
	DWORD Flags;
	PyObject *obdesktop_name, *ret=NULL;
	HDESK hdesk;
	// @pyparm string|szDesktop||Name of desktop to open
	// @pyparm int|Flags||DF_ALLOWOTHERACCOUNTHOOK or 0
	// @pyparm bool|Inherit||Allow handle to be inherited
	// @pyparm int|DesiredAccess||ACCESS_MASK specifying level of access for handle
	if (!PyArg_ParseTuple(args,"Olll:OpenWindowStation",&obdesktop_name, &Flags, &Inherit, &DesiredAccess))
		return NULL;
	if (!PyWinObject_AsWCHAR(obdesktop_name,&desktop_name,FALSE))
		return NULL;
	hdesk=OpenDesktopW(desktop_name, Flags, Inherit,DesiredAccess);
	if (hdesk==NULL)
		PyWin_SetAPIError("OpenDesktop",GetLastError());
	else
		ret= new PyHDESK(hdesk);
	PyWinObject_FreeWCHAR(desktop_name);
	return ret;
}
%}

// @pyswig <o PyHDESK>|CreateDesktop|Creates a new desktop in calling process's current window station
%native(CreateDesktop) PyCreateDesktop;
%{
PyObject *PyCreateDesktop(PyObject *self, PyObject *args)
{
	PyObject *obDesktop, *obSA=NULL, *ret=NULL;
	WCHAR *Desktop=NULL;
	DWORD Flags;
	ACCESS_MASK DesiredAccess;
	PSECURITY_ATTRIBUTES pSA;
	HDESK hdesk;
	// @pyparm string|Desktop||Name of desktop to create
	// @pyparm int|Flags||DF_ALLOWOTHERACCOUNTHOOK or 0
	// @pyparm int|DesiredAccess||An ACCESS_MASK determining level of access available thru returned handle
	// @pyparm <o PySECURITY_ATTRIBUTES>|SecurityAttributes||Specifies inheritance and controls access to desktop
	if (!PyArg_ParseTuple(args,"OllO:CreateDesktop", &obDesktop, &Flags, &DesiredAccess, &obSA))
		return NULL;
	if (!PyWinObject_AsSECURITY_ATTRIBUTES(obSA, &pSA, TRUE))
		return NULL;
	if (!PyWinObject_AsWCHAR(obDesktop, &Desktop, FALSE))
		return NULL;

	hdesk=CreateDesktopW(Desktop,NULL,NULL,Flags,DesiredAccess, pSA);
	if (hdesk==NULL)
		PyWin_SetAPIError("CreateDesktop",GetLastError());
	else
		ret= new PyHDESK(hdesk);

	PyWinObject_FreeWCHAR(Desktop);
	return ret;
}
%}

// @pyswig <o PyHDESK>|OpenInputDesktop|Returns a handle to desktop for logged-in user
%native(OpenInputDesktop) PyOpenInputDesktop;
%{
PyObject *PyOpenInputDesktop(PyObject *self, PyObject *args)
{
	DWORD Flags;
	BOOL Inherit;
	ACCESS_MASK DesiredAccess;
	HDESK hdesk;
	// @pyparm int|Flags||DF_ALLOWOTHERACCOUNTHOOK or 0
	// @pyparm bool|Inherit||Specifies if handle will be inheritable
	// @pyparm int|DesiredAccess||ACCESS_MASK specifying access available to returned handle
	if (!PyArg_ParseTuple(args,"lll:OpenInputDesktop",&Flags,&Inherit,&DesiredAccess))
		return NULL;
	hdesk=OpenInputDesktop(Flags,Inherit,DesiredAccess);
	if (hdesk==NULL)
		return PyWin_SetAPIError("OpenInputDesktop",GetLastError());
	return new PyHDESK(hdesk);
}
%}

// @pyswig <o PyHWINSTA>|GetProcessWindowStation|Returns a handle to calling process's current window station
%native(GetProcessWindowStation) PyGetProcessWindowStation;
%{
PyObject *PyGetProcessWindowStation(PyObject *self, PyObject *args)
{
	HWINSTA hwinsta;
	if (!PyArg_ParseTuple(args,":GetProcessWindowStation"))
		return NULL;
	hwinsta=::GetProcessWindowStation();
	if (hwinsta==NULL)
		return PyWin_SetAPIError("GetProcessWindowStation",GetLastError());
	return new PyHWINSTA(hwinsta);
}
%}

// @pyswig <o PyHWINSTA>|CreateWindowStation|Creates a new window station
// @comm If name is None or empty string, name is formatteded from logon id
%native(CreateWindowStation) PyCreateWindowStation;
%{
PyObject *PyCreateWindowStation(PyObject *self, PyObject *args)
{
	HWINSTA hwinsta;
	WCHAR *winsta_name;
	DWORD Flags;
	ACCESS_MASK DesiredAccess;
	PSECURITY_ATTRIBUTES pSA;
	PyObject *obwinsta_name, *obSA;
	// @pyparm string|WindowStation||Name of window station to create, or None
	// @pyparm int|Flags||CWF_CREATE_ONLY or 0
	// @pyparm int|DesiredAccess||Bitmask of access types available to returned handle
	// @pyparm <o PySECURITY_ATTRIBUTES>|SecurityAttributes||Specifies security for window station, and whether handle is inheritable
	if (!PyArg_ParseTuple(args,"OllO:CreateWindowStation", &obwinsta_name, &Flags, &DesiredAccess, &obSA))
		return NULL;
	if (!PyWinObject_AsSECURITY_ATTRIBUTES(obSA, &pSA, TRUE))
		return NULL;
	if (!PyWinObject_AsWCHAR(obwinsta_name, &winsta_name, TRUE))
		return NULL;
	hwinsta=CreateWindowStationW(winsta_name, Flags, DesiredAccess, pSA);
	PyWinObject_FreeWCHAR(winsta_name);
	if (hwinsta==NULL)
		return PyWin_SetAPIError("CreateWindowStation",GetLastError());
	return new PyHWINSTA(hwinsta);
}
%}

%{
PyObject *MyCreateService(
    SC_HANDLE hSCManager,	// handle to service control manager database
    TCHAR *lpServiceName,	// pointer to name of service to start
    TCHAR *lpDisplayName,	// pointer to display name
    DWORD dwDesiredAccess,	// type of access to service
    DWORD dwServiceType,	// type of service
    DWORD dwStartType,		// when to start service
    DWORD dwErrorControl,	// severity if service fails to start
    TCHAR * lpBinaryPathName,	// pointer to name of binary file
    TCHAR * lpLoadOrderGroup,	// pointer to name of load ordering group
    BOOL  bFetchTag,
    PyObject *obDeps,		// array of dependency names
    TCHAR *lpServiceStartName,	// pointer to account name of service
    TCHAR *lpPassword 	// pointer to password for service account
   )
{
	PyObject *rc = NULL;
	TCHAR *lpDeps = NULL;
	DWORD tagID;
	DWORD *pTagID = bFetchTag ? &tagID : NULL;
	SC_HANDLE sh = 0;
	if (!PyWinObject_AsMultipleString(obDeps, &lpDeps, TRUE))
		goto cleanup;

	Py_BEGIN_ALLOW_THREADS
	sh = CreateService(hSCManager,lpServiceName,lpDisplayName,dwDesiredAccess,
	                             dwServiceType, dwStartType, dwErrorControl, lpBinaryPathName,
	                             lpLoadOrderGroup, pTagID, lpDeps, lpServiceStartName, lpPassword);
	Py_END_ALLOW_THREADS
	if (sh==0) {
		PyWin_SetAPIError("CreateService");
		rc = NULL;
	} else {
		if (bFetchTag)
			rc = Py_BuildValue("Nl", PyWinObject_FromSC_HANDLE(sh), tagID);
		else
			rc = PyWinObject_FromSC_HANDLE(sh);
	}
cleanup:
	PyWinObject_FreeMultipleString(lpDeps);
	return rc;

}

PyObject *MyChangeServiceConfig(
    SC_HANDLE hSCManager,	// handle to service control manager database
    DWORD dwServiceType,	// type of service
    DWORD dwStartType,		// when to start service
    DWORD dwErrorControl,	// severity if service fails to start
    TCHAR * lpBinaryPathName,	// pointer to name of binary file
    TCHAR * lpLoadOrderGroup,	// pointer to name of load ordering group
    BOOL  bFetchTag,
    PyObject *obDeps,		// array of dependency names
    TCHAR *lpServiceStartName,	// pointer to account name of service
    TCHAR *lpPassword, 	// pointer to password for service account
    TCHAR *lpDisplayName	// pointer to display name
   )
{
	PyObject *rc = NULL;
	TCHAR *lpDeps = NULL;
	DWORD tagID;
	DWORD *pTagID = bFetchTag ? &tagID : NULL;
	SC_HANDLE sh = 0;
	if (!PyWinObject_AsMultipleString(obDeps, &lpDeps, TRUE))
		goto cleanup;

	if (!ChangeServiceConfig(hSCManager,
                         dwServiceType, dwStartType, dwErrorControl, lpBinaryPathName,
                         lpLoadOrderGroup, pTagID, lpDeps, lpServiceStartName, lpPassword,
						 lpDisplayName))
		rc = PyWin_SetAPIError("ChangeServiceConfig");
	else if (bFetchTag)
		rc = PyLong_FromLong(tagID);
	else {
		rc = Py_None;
		Py_INCREF(rc);
	}
cleanup:
	PyWinObject_FreeMultipleString(lpDeps);
	return rc;

}

PyObject *MyStartService( SC_HANDLE scHandle, PyObject *serviceArgs )
{
	LPWSTR *pArgs;
	DWORD numStrings = 0;
	if (!PyWinObject_AsWCHARArray(serviceArgs, &pArgs, &numStrings, TRUE))
		return NULL;

	PyObject *rc;
        BOOL ok = FALSE;
	Py_BEGIN_ALLOW_THREADS
	ok = StartService(scHandle, numStrings, (LPCWSTR *)pArgs);
	Py_END_ALLOW_THREADS
	if (ok) {
		rc = Py_None;
		Py_INCREF(Py_None);
	} else
		rc = PyWin_SetAPIError("StartService");
	PyWinObject_FreeWCHARArray(pArgs, numStrings);
	return rc;
}
%}

// These 3 function contributed by Curt Hagenlocher

// @pyswig (tuple,...)|EnumServicesStatus|Returns a tuple of status info for each service that meets specified criteria
// @rdesc Returns a sequence of tuples representing ENUM_SERVICE_STATUS structs: (ServiceName, DisplayName, <o SERVICE_STATUS>)
%native (EnumServicesStatus) MyEnumServicesStatus;

%{
static PyObject *MyEnumServicesStatus(PyObject *self, PyObject *args)
{
	// @pyparm <o PySC_HANDLE>|hSCManager||Handle to service control manager as returned by <om win32service.OpenSCManager>
	// @pyparm int|ServiceType|SERVICE_WIN32|Types of services to enumerate (SERVICE_DRIVER and/or SERVICE_WIN32)
	// @pyparm int|ServiceState|SERVICE_STATE_ALL|Limits to services in specified state
	SC_HANDLE hscm;
	PyObject *obhscm;
	DWORD serviceType = SERVICE_WIN32;
	DWORD serviceState = SERVICE_STATE_ALL;
	if (!PyArg_ParseTuple(args, "O|ll:EnumServicesStatus", &obhscm, &serviceType, &serviceState))
	{
		return NULL;
	}
	if (!PyWinObject_AsHANDLE(obhscm, (HANDLE *)&hscm))
		return NULL;
	long tmp;
	LPENUM_SERVICE_STATUS services = (LPENUM_SERVICE_STATUS)&tmp;
	DWORD bytesNeeded, servicesReturned, resumeHandle = 0;
	BOOL result = FALSE;
	char *buffer = NULL;

	Py_BEGIN_ALLOW_THREADS

	EnumServicesStatus(hscm, serviceType, serviceState, services, sizeof(tmp), &bytesNeeded,
		&servicesReturned, &resumeHandle);

	if (GetLastError() == ERROR_MORE_DATA)
	{
		buffer = new char[bytesNeeded + 1];
		services = (LPENUM_SERVICE_STATUS)buffer;
		result = EnumServicesStatus(hscm, serviceType, serviceState, services, bytesNeeded + 1,
			&bytesNeeded, &servicesReturned, &resumeHandle);
	}

	Py_END_ALLOW_THREADS

	if (!result)
	{
		delete[] buffer;
		return PyWin_SetAPIError("EnumServicesStatus");
	}

	PyObject *retval = PyTuple_New(servicesReturned);
	for (DWORD i = 0; i < servicesReturned; i++)
	{
		PyObject *obServiceName = PyWinObject_FromTCHAR(services[i].lpServiceName);
		PyObject *obDisplayName = PyWinObject_FromTCHAR(services[i].lpDisplayName);
		PyTuple_SetItem(retval, i, Py_BuildValue("OO(lllllll)",
			obServiceName,
			obDisplayName,
			services[i].ServiceStatus.dwServiceType,
			services[i].ServiceStatus.dwCurrentState,
			services[i].ServiceStatus.dwControlsAccepted,
			services[i].ServiceStatus.dwWin32ExitCode,
			services[i].ServiceStatus.dwServiceSpecificExitCode,
			services[i].ServiceStatus.dwCheckPoint,
			services[i].ServiceStatus.dwWaitHint));
		Py_XDECREF(obServiceName);
		Py_XDECREF(obDisplayName);
	}

	delete[] buffer;
	return retval;
}
%}

// @pyswig (dict,...)|EnumServicesStatusEx|Lists the status of services that meet the specified criteria
// @rdesc Returns a sequence of dicts, whose contents depend on information level requested.
// Currently, only information level supported is SC_ENUM_PROCESS_INFO (returns ENUM_SERVICE_STATUS_PROCESS).
// @pyseeapi EnumServicesStatusEx
%native (EnumServicesStatusEx) MyEnumServicesStatusEx;
%{
static PyObject *MyEnumServicesStatusEx(PyObject *self, PyObject *args)
{
	// @pyparm <o PySC_HANDLE>|SCManager||Handle to service control manager as returned by <om win32service.OpenSCManager>
	// @pyparm int|ServiceType|SERVICE_WIN32|Types of services to enumerate (SERVICE_DRIVER and/or SERVICE_WIN32)
	// @pyparm int|ServiceState|SERVICE_STATE_ALL|Limits to services in specified state
	// @pyparm str|GroupName|None|Name of group - use None for all, or '' for services that don't belong to a group
	// @pyparm int|InfoLevel|SC_ENUM_PROCESS_INFO|Currently SC_ENUM_PROCESS_INFO is only level defined
	SC_HANDLE hscm;
	PyObject *obgrp = Py_None, *ret = NULL;
	DWORD service_type = SERVICE_WIN32, service_state = SERVICE_STATE_ALL;
	TmpWCHAR grp;
	DWORD lvl = SC_ENUM_PROCESS_INFO;
	BYTE *buf=NULL;
	DWORD buf_size=0, buf_needed, nbr_returned;
	DWORD resume_handle =0, err=0;
	BOOL bsuccess;

	if (fpEnumServicesStatusEx == NULL){
		PyErr_SetString(PyExc_NotImplementedError, "EnumServicesStatusEx does not exist on this platform");
		return NULL;
		}
	if (!PyArg_ParseTuple(args, "O&|kkOk:EnumServicesStatusEx",
		PyWinObject_AsHANDLE, &hscm,
		&service_type, &service_state,
		&obgrp, &lvl))
		return NULL;
	if (lvl != SC_ENUM_PROCESS_INFO){
		PyErr_SetString(PyExc_NotImplementedError, "Unsupported information level");
		return NULL;
		}
	if (!PyWinObject_AsWCHAR(obgrp, &grp, TRUE))
		return NULL;
	ret = PyList_New(0);
	if (ret==NULL)
		return NULL;
	while (true){
		Py_BEGIN_ALLOW_THREADS
		bsuccess = (*fpEnumServicesStatusEx)(hscm, (SC_ENUM_TYPE)lvl, service_type, service_state,
			buf, buf_size, &buf_needed, &nbr_returned, &resume_handle, grp);
		Py_END_ALLOW_THREADS
		if (!bsuccess){
			err=GetLastError();
			if (err != ERROR_MORE_DATA){
				PyWin_SetAPIError("EnumServicesStatusEx", err);
				Py_DECREF(ret);
				ret = NULL;
				break;
				}
			}
		// Function can return false with ERROR_MORE_DATA after retrieving some statuses
		for (DWORD i=0; i<nbr_returned; i++){
			ENUM_SERVICE_STATUS_PROCESS *essp = (ENUM_SERVICE_STATUS_PROCESS *)buf;
			TmpPyObject stat = Py_BuildValue("{s:N, s:N, s:k, s:k, s:k, s:k, s:k, s:k, s:k, s:k, s:k}",
				"ServiceName", PyWinObject_FromTCHAR(essp[i].lpServiceName),
				"DisplayName", PyWinObject_FromTCHAR(essp[i].lpDisplayName),
				"ServiceType", essp[i].ServiceStatusProcess.dwServiceType,
				"CurrentState", essp[i].ServiceStatusProcess.dwCurrentState,
				"ControlsAccepted", essp[i].ServiceStatusProcess.dwControlsAccepted,
				"Win32ExitCode", essp[i].ServiceStatusProcess.dwWin32ExitCode,
				"ServiceSpecificExitCode", essp[i].ServiceStatusProcess.dwServiceSpecificExitCode,
				"CheckPoint", essp[i].ServiceStatusProcess.dwCheckPoint,
				"WaitHint", essp[i].ServiceStatusProcess.dwWaitHint,
				"ProcessId", essp[i].ServiceStatusProcess.dwProcessId,
				"ServiceFlags", essp[i].ServiceStatusProcess.dwServiceFlags);
			if (stat == NULL || PyList_Append(ret, stat) == -1){
				Py_DECREF(ret);
				ret=NULL;
				break;
				}
			}
		if (bsuccess || ret == NULL)
			break;
		if (buf)
			free(buf);
		buf=(BYTE *)malloc(buf_needed);
		if (buf == NULL){
			PyErr_NoMemory();
			Py_DECREF(ret);
			ret = NULL;
			break;
			}
		buf_size = buf_needed;
		}

	if (buf)
		free(buf);
	return ret;
}
%}

// @pyswig (tuple,...)|EnumDependentServices|Lists services that depend on a service
// @rdesc Returns a sequence of tuples representing ENUM_SERVICE_STATUS structs: (ServiceName, DisplayName, <o SERVICE_STATUS>)
%native (EnumDependentServices) MyEnumDependentServices;
%{
static PyObject *MyEnumDependentServices(PyObject *self, PyObject *args)
{
	// @pyparm <o PySC_HANDLE>|hService||Handle to service for which to list dependent services (as returned by <om win32service.OpenService>)
	// @pyparm int|ServiceState|SERVICE_STATE_ALL|Limits to services in specified state - One of SERVICE_STATE_ALL, SERVICE_ACTIVE, SERVICE_INACTIVE
	SC_HANDLE hsc;
	PyObject *obhsc;
	DWORD serviceState = SERVICE_STATE_ALL;
	if (!PyArg_ParseTuple(args, "O|l:EnumDependentServices", &obhsc, &serviceState))
	{
		return NULL;
	}
	if (!PyWinObject_AsHANDLE(obhsc, (HANDLE *)&hsc))
		return NULL;

	long tmp;
	LPENUM_SERVICE_STATUS services = (LPENUM_SERVICE_STATUS)&tmp;
	DWORD bytesNeeded, servicesReturned, resumeHandle = 0;
	BOOL result = FALSE;
	char *buffer = NULL;

	Py_BEGIN_ALLOW_THREADS

	result = EnumDependentServices(hsc, serviceState, services, sizeof(tmp), &bytesNeeded,
		&servicesReturned);

	if (!result && GetLastError() == ERROR_MORE_DATA)
	{
		buffer = new char[bytesNeeded + 1];
		services = (LPENUM_SERVICE_STATUS)buffer;
		result = EnumDependentServices(hsc, serviceState, services, bytesNeeded + 1,
			&bytesNeeded, &servicesReturned);
	}

	Py_END_ALLOW_THREADS

	if (!result)
	{
		delete[] buffer;
		return PyWin_SetAPIError("EnumDependentServices");
	}

	PyObject *retval = PyTuple_New(servicesReturned);
	for (DWORD i = 0; i < servicesReturned; i++)
	{
		PyObject *obServiceName = PyWinObject_FromTCHAR(services[i].lpServiceName);
		PyObject *obDisplayName = PyWinObject_FromTCHAR(services[i].lpDisplayName);
		PyTuple_SetItem(retval, i, Py_BuildValue("OO(lllllll)",
			obServiceName,
			obDisplayName,
			services[i].ServiceStatus.dwServiceType,
			services[i].ServiceStatus.dwCurrentState,
			services[i].ServiceStatus.dwControlsAccepted,
			services[i].ServiceStatus.dwWin32ExitCode,
			services[i].ServiceStatus.dwServiceSpecificExitCode,
			services[i].ServiceStatus.dwCheckPoint,
			services[i].ServiceStatus.dwWaitHint));
		Py_XDECREF(obServiceName);
		Py_XDECREF(obDisplayName);
	}

	delete[] buffer;
	return retval;
}
%}

// @pyswig tuple|QueryServiceConfig|Retrieves configuration parameters for a service
%native (QueryServiceConfig) MyQueryServiceConfig;
%{
static PyObject *MyQueryServiceConfig(PyObject *self, PyObject *args)
{
	// @pyparm <o PySC_HANDLE>|hService||Service handle as returned by <om win32service.OpenService>
	SC_HANDLE hsc;
	PyObject *obhsc;
	if (!PyArg_ParseTuple(args, "O:QueryServiceConfig", &obhsc))
	{
		return NULL;
	}
	if (!PyWinObject_AsHANDLE(obhsc, (HANDLE *)&hsc))
		return NULL;

	long tmp;
	LPQUERY_SERVICE_CONFIG config = (LPQUERY_SERVICE_CONFIG)&tmp;
	DWORD bytesNeeded;
	BOOL result = FALSE;
	char *buffer = NULL;

	Py_BEGIN_ALLOW_THREADS

	result = QueryServiceConfig(hsc, config, sizeof(tmp), &bytesNeeded);

	if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
	{
		buffer = new char[bytesNeeded + 1];
		config = (LPQUERY_SERVICE_CONFIG)buffer;
		result = QueryServiceConfig(hsc, config, bytesNeeded, &bytesNeeded);
	}

	Py_END_ALLOW_THREADS

	if (!result)
	{
		delete[] buffer;
		return PyWin_SetAPIError("QueryServiceConfig");
	}

	// @rdesc Returns a tuple representing a QUERY_SERVICE_CONFIG struct:
	// @tupleitem 0|int|ServiceType|Combination of SERVICE_*_DRIVER or SERVICE_*_PROCESS constants
	// @tupleitem 1|int|StartType|One of SERVICE_*_START constants
	// @tupleitem 2|int|ErrorControl|One of SERVICE_ERROR_* constants
	// @tupleitem 3|string|BinaryPathName|Service's binary executable, can also contain command line args
	// @tupleitem 4|string|LoadOrderGroup|Loading group that service is a member of
	// @tupleitem 5|int|TagId|Order of service within its load order group
	// @tupleitem 6|[string,...]|Dependencies|Sequence of names of services on which this service depends
	// @tupleitem 7|string|ServiceStartName|Account name under which service will run
	// @tupleitem 8|string|DisplayName|Name of service
	PyObject *retval = Py_BuildValue("lllNNlNNN",
			config->dwServiceType,
			config->dwStartType,
			config->dwErrorControl,
			PyWinObject_FromTCHAR(config->lpBinaryPathName),
			PyWinObject_FromTCHAR(config->lpLoadOrderGroup),
			config->dwTagId,
			PyWinObject_FromMultipleString(config->lpDependencies),
			PyWinObject_FromTCHAR(config->lpServiceStartName),
			PyWinObject_FromTCHAR(config->lpDisplayName));
	delete[] buffer;
	return retval;
}
%}

typedef float SC_HANDLE, SERVICE_STATUS_HANDLE, SC_LOCK;	// This is just to keep Swig from treating them as pointers
%typemap(python,out) SC_HANDLE{
	$target = PyWinObject_FromSC_HANDLE($source);
}
%typemap(python,in) SC_HANDLE, SERVICE_STATUS_HANDLE{
	if (!PyWinObject_AsHANDLE($source, (HANDLE *)&$target))
		return NULL;
}
%typemap(python,out) SC_LOCK{
	$target = PyWinLong_FromVoidPtr($source);
}
%typemap(python,in) SC_LOCK{
	if (!PyWinLong_AsVoidPtr($source, &$target))
		return NULL;
}

%typemap(python,except) SC_HANDLE {
      Py_BEGIN_ALLOW_THREADS
      $function
      Py_END_ALLOW_THREADS
      if ($source==0)  {
           $cleanup;
           return PyWin_SetAPIError("$name");
      }
}

// SERVICE_STATUS support
%typemap(python,ignore) SERVICE_STATUS *outServiceStatus (SERVICE_STATUS temp) {
	$target = &temp;
}

// @object SERVICE_STATUS|A Win32 service status object is represented by a tuple:
%typemap(python,argout) SERVICE_STATUS *outServiceStatus {
	Py_DECREF($target);
	$target = Py_BuildValue("lllllll",
		$source->dwServiceType, // @tupleitem 0|int|serviceType|The type of service.
		$source->dwCurrentState, // @tupleitem 1|int|serviceState|The current state of the service.
		$source->dwControlsAccepted, // @tupleitem 2|int|controlsAccepted|The controls the service accepts.
		$source->dwWin32ExitCode, // @tupleitem 3|int|win32ExitCode|The win32 error code for the service.
		$source->dwServiceSpecificExitCode, // @tupleitem 4|int|serviceSpecificErrorCode|The service specific error code.
		$source->dwCheckPoint, // @tupleitem 5|int|checkPoint|The checkpoint reported by the service.
		$source->dwWaitHint); // @tupleitem 6|int|waitHint|The wait hint reported by the service.
}

%typemap(python,in) SERVICE_STATUS *inServiceStatus (SERVICE_STATUS junk) {
	$target = &junk;
	if (!PyArg_ParseTuple($source, "lllllll",
		&$target->dwServiceType,
		&$target->dwCurrentState,
		&$target->dwControlsAccepted,
		&$target->dwWin32ExitCode,
		&$target->dwServiceSpecificExitCode,
		&$target->dwCheckPoint,
		&$target->dwWaitHint))
		return NULL;
}

// @pyswig |StartService|Starts the specified service
%name (StartService) PyObject *MyStartService (
     SC_HANDLE  hService, // @pyparm <o PySC_HANDLE>|hService||Handle to the service to be started
     PyObject *pyobject /* serviceArgs */); // @pyparm [string, ...]|args||Arguments to the service.

// @pyswig <o PySC_HANDLE>|OpenService|Returns a handle to the specified service.
SC_HANDLE OpenService(
	SC_HANDLE hSCManager, // @pyparm <o PySC_HANDLE>|scHandle||Handle to the Service Control Mananger
	TCHAR *name, // @pyparm string|name||The name of the service to open.
	unsigned long desiredAccess); // @pyparm int|desiredAccess||The access desired.

// @pyswig <o PySC_HANDLE>|OpenSCManager|Returns a handle to the service control manager
SC_HANDLE OpenSCManager(
	TCHAR *INPUT_NULLOK, // @pyparm string|machineName||The name of the computer, or None
	TCHAR *INPUT_NULLOK, // @pyparm string|dbName||The name of the service database, or None
	unsigned long desiredAccess); // @pyparm int|desiredAccess||The access desired. (combination of win32service.SC_MANAGER_* flags)

%{
// @pyswig |CloseServiceHandle|Closes a service or SCM handle
static PyObject *PyCloseServiceHandle(PyObject *self, PyObject *args)
{
	PyObject *obsch;
	SC_HANDLE sch;
	if (!PyArg_ParseTuple(args, "O:CloseServiceHandle",
		&obsch))	// @pyparm <o PySC_HANDLE>|scHandle||Handle to close
		return NULL;
	if (PyHANDLE_Check(obsch)){
		// Calling Close() in this manner could close any type of PyHANDLE.
		if (strcmp(((PyHANDLE *)obsch)->GetTypeName(),"PySC_HANDLE")!=0){
			PyErr_SetString(PyExc_TypeError,"PyHANDLE passed to CloseServiceHandle must be a PySC_HANDLE");
			return NULL;
			}
		if (!((PySC_HANDLE *)obsch)->Close())
			return NULL;
		Py_INCREF(Py_None);
		return Py_None;
		}
	if (!PyWinObject_AsHANDLE(obsch, (HANDLE *)&sch))
		return NULL;
	if (!CloseServiceHandle(sch))
		return PyWin_SetAPIError("CloseServiceHandle");
	Py_INCREF(Py_None);
	return Py_None;
}
%}
%native (CloseServiceHandle) PyCloseServiceHandle;

// @pyswig <o SERVICE_STATUS>|QueryServiceStatus|Queries a service status
BOOLAPI QueryServiceStatus(SC_HANDLE handle, SERVICE_STATUS *outServiceStatus);
// @pyparm <o PySC_HANDLE>|hService||Handle to service to be queried

// @pyswig <o SERVICE_STATUS>|QueryServiceStatusEx|Queries a service status
%native (QueryServiceStatusEx) MyQueryServiceStatusEx;
%{
PyObject *MyQueryServiceStatusEx(PyObject *self, PyObject *args)
{
	if (fpQueryServiceStatusEx==NULL){
		PyErr_SetString(PyExc_NotImplementedError,"QueryServiceStatusEx does not exist on this platform");
		return NULL;
		}
	SC_HANDLE hService;
	PyObject *obhService;
	SC_STATUS_TYPE InfoLevel=SC_STATUS_PROCESS_INFO;  // only existing info level
	SERVICE_STATUS_PROCESS info;
	DWORD bufsize=sizeof(SERVICE_STATUS_PROCESS);
	DWORD reqdbufsize;
	// @pyparm <o PySC_HANDLE>|hService||Handle to service to be queried
	if (!PyArg_ParseTuple(args,"O:QueryServiceStatusEx",&obhService))
		return NULL;
	if (!PyWinObject_AsHANDLE(obhService, (HANDLE *)&hService))
		return NULL;

	if (!(*fpQueryServiceStatusEx)(hService,InfoLevel,(BYTE *)&info,bufsize,&reqdbufsize))
		return PyWin_SetAPIError("QueryServiceStatusEx", GetLastError());
	return Py_BuildValue("{s:l,s:l,s:l,s:l,s:l,s:l,s:l,s:l,s:l}",
		"ServiceType", info.dwServiceType,
		"CurrentState", info.dwCurrentState,
		"ControlsAccepted", info.dwControlsAccepted,
		"Win32ExitCode", info.dwWin32ExitCode,
		"ServiceSpecificExitCode",info.dwServiceSpecificExitCode,
		"CheckPoint", info.dwCheckPoint,
		"WaitHint", info.dwWaitHint,
		"ProcessId", info.dwProcessId,
		"ServiceFlags", info.dwServiceFlags);
}
%}

// @pyswig |SetServiceObjectSecurity|Set the security descriptor for a service
%native (SetServiceObjectSecurity) MySetServiceObjectSecurity;
%{
PyObject *MySetServiceObjectSecurity(PyObject *self, PyObject *args)
{
	PyObject *obSD;
	PSECURITY_DESCRIPTOR pSD;
	SECURITY_INFORMATION info;
	SC_HANDLE hsvc;
	PyObject *obhsvc;
	// @pyparm <o PySC_HANDLE>|Handle||Service handle
	// @pyparm int|SecurityInformation||Type of infomation to set, combination of values from SECURITY_INFORMATION enum
	// @pyparm <o PySECURITY_DESCRIPTOR>|SecurityDescriptor||PySECURITY_DESCRIPTOR containing infomation to set
	if (!PyArg_ParseTuple(args,"OlO:SetServiceObjectSecurity",&obhsvc, &info, &obSD))
		return NULL;
	if (!PyWinObject_AsHANDLE(obhsvc, (HANDLE *)&hsvc))
		return NULL;

	if (!PyWinObject_AsSECURITY_DESCRIPTOR(obSD,&pSD,FALSE))
		return NULL;
	if (!SetServiceObjectSecurity(hsvc,info,pSD))
		return PyWin_SetAPIError("SetServiceObjectSecurity",GetLastError());
	Py_INCREF(Py_None);
	return Py_None;
}
%}

// @pyswig <o PySECURITY_DESCRIPTOR>|QueryServiceObjectSecurity|Retrieves information from the security descriptor for a service
%native (QueryServiceObjectSecurity) MyQueryServiceObjectSecurity;
%{
PyObject *MyQueryServiceObjectSecurity(PyObject *self, PyObject *args)
{
	PyObject *ret=NULL;
	PSECURITY_DESCRIPTOR pSD=NULL;
	SECURITY_INFORMATION info;
	DWORD err, origbufsize=SECURITY_DESCRIPTOR_MIN_LENGTH, reqdbufsize=0;
	SC_HANDLE hsvc;
	PyObject *obhsvc;
	// @pyparm <o PySC_HANDLE>|Handle||Service handle
	// @pyparm int|SecurityInformation||Type of infomation to retrieve, combination of values from SECURITY_INFORMATION enum
	if (!PyArg_ParseTuple(args,"Ol:QueryServiceObjectSecurity",&obhsvc, &info))
		return NULL;
	if (!PyWinObject_AsHANDLE(obhsvc, (HANDLE *)&hsvc))
		return NULL;

	pSD=(PSECURITY_DESCRIPTOR)malloc(origbufsize);
	if (pSD==NULL){
		PyErr_Format(PyExc_MemoryError, "QueryServiceObjectSecurity: unable to allocate %d bytes", origbufsize);
		return NULL;
		}
	if (!QueryServiceObjectSecurity(hsvc,info,pSD,origbufsize,&reqdbufsize)){
		err=GetLastError();
		if (err==ERROR_INSUFFICIENT_BUFFER){
			free(pSD);
			pSD=(PSECURITY_DESCRIPTOR)malloc(reqdbufsize);
			if (pSD==NULL)
				PyErr_Format(PyExc_MemoryError,"QueryServiceObjectSecurity: unable to reallocatate %d bytes",reqdbufsize);
			else
				if (!QueryServiceObjectSecurity(hsvc,info,pSD,reqdbufsize,&reqdbufsize))
					PyWin_SetAPIError("QueryServiceObjectSecurity",GetLastError());
				else
					ret=PyWinObject_FromSECURITY_DESCRIPTOR(pSD);
			}
		else
			PyWin_SetAPIError("QueryServiceObjectSecurity",err);
		}
	else
		ret=PyWinObject_FromSECURITY_DESCRIPTOR(pSD);
	if (pSD!=NULL)
		free(pSD);
	return ret;
}
%}

// @pyswig string|GetServiceKeyName|Translates a service display name into its registry key name
%native (GetServiceKeyName) MyGetServiceKeyName;
%{
PyObject *MyGetServiceKeyName(PyObject *self, PyObject *args)
{
	// @pyparm <o PySC_HANDLE>|hSCManager||Handle to service control manager as returned by <om win32service.OpenSCManager>
	// @pyparm string|DisplayName||Display name of a service
	SC_HANDLE h;
	PyObject *obh;
	WCHAR *displayname;
	WCHAR keyname[MAX_SERVICE_NAME_LEN];
	DWORD bufsize=MAX_SERVICE_NAME_LEN;
	PyObject *obdisplayname, *ret=NULL;
	if (!PyArg_ParseTuple(args,"OO:GetServiceKeyName", &obh, &obdisplayname))
		return NULL;
	if (!PyWinObject_AsHANDLE(obh, (HANDLE *)&h))
		return NULL;
	if (!PyWinObject_AsWCHAR(obdisplayname, &displayname, FALSE))
		return NULL;
	if (!GetServiceKeyNameW(h, displayname, keyname, &bufsize))
		PyWin_SetAPIError("GetServiceKeyName");
	else
		ret=PyWinObject_FromWCHAR(keyname, bufsize);
	PyWinObject_FreeWCHAR(displayname);
	return ret;
}
%}

// @pyswig string|GetServiceDisplayName|Translates an internal service name into its display name
%native (GetServiceDisplayName) MyGetServiceDisplayName;
%{
PyObject *MyGetServiceDisplayName(PyObject *self, PyObject *args)
{
	// @pyparm <o PySC_HANDLE>|hSCManager||Handle to service control manager as returned by <om win32service.OpenSCManager>
	// @pyparm string|ServiceName||Name of service
	SC_HANDLE h;
	PyObject *obh;
	WCHAR *keyname;
	WCHAR displayname[MAX_SERVICE_NAME_LEN];
	DWORD bufsize=MAX_SERVICE_NAME_LEN;
	PyObject *obkeyname, *ret=NULL;
	if (!PyArg_ParseTuple(args,"OO:GetServiceDisplayName", &obh, &obkeyname))
		return NULL;
	if (!PyWinObject_AsHANDLE(obh, (HANDLE *)&h))
		return NULL;
	if (!PyWinObject_AsWCHAR(obkeyname, &keyname, FALSE))
		return NULL;
	if (!GetServiceDisplayNameW(h, keyname, displayname, &bufsize))
		PyWin_SetAPIError("GetServiceDisplayName");
	else
		ret=PyWinObject_FromWCHAR(displayname);
	PyWinObject_FreeWCHAR(keyname);
	return ret;
}
%}

// @pyswig |SetServiceStatus|Sets a service status
BOOLAPI SetServiceStatus(
	SERVICE_STATUS_HANDLE hSCManager, // @pyparm int|scHandle||Handle to set
	SERVICE_STATUS *inServiceStatus); // @pyparm <o SERVICE_STATUS>|serviceStatus||The new status

// @pyswig <o SERVICE_STATUS>|ControlService|Sends a control message to a service.
// @rdesc The result is the new service status.
BOOLAPI ControlService(
    SC_HANDLE handle, // @pyparm <o PySC_HANDLE>|scHandle||Handle to control
    DWORD status, // @pyparm int|code||The service control code.
    SERVICE_STATUS *outServiceStatus);

// @pyswig |DeleteService|Deletes the specified service
BOOLAPI DeleteService(SC_HANDLE);
// @pyparm <o PySC_HANDLE>|scHandle||Handle to service to be deleted

// @pyswig <o PySC_HANDLE>/(<o PySC_HANDLE>, int)|CreateService|Creates a new service.
%name (CreateService) PyObject * MyCreateService(
    SC_HANDLE hSCManager,	// @pyparm <o PySC_HANDLE>|scHandle||handle to service control manager database
    TCHAR *name,			// @pyparm string|name||Name of service
    TCHAR *displayName,		// @pyparm string|displayName||Display name
    DWORD dwDesiredAccess,	// @pyparm int|desiredAccess||type of access to service
    DWORD dwServiceType,	// @pyparm int|serviceType||type of service
    DWORD dwStartType,		// @pyparm int|startType||When/how to start service
    DWORD dwErrorControl,	// @pyparm int|errorControl||severity if service fails to start
    TCHAR *binaryFile,	// @pyparm string|binaryFile||name of binary file
    TCHAR *INPUT_NULLOK,	// @pyparm string|loadOrderGroup||name of load ordering group , or None
    BOOL  bFetchTag,            // @pyparm int|bFetchTag||Should the tag be fetched and returned?  If TRUE, the result is a tuple of (handle, tag), otherwise just handle.
    PyObject *pyobject,		// @pyparm [string,...]|serviceDeps||sequence of dependency names
    TCHAR *INPUT_NULLOK,	// @pyparm string|acctName||account name of service, or None
    TCHAR *INPUT_NULLOK 	// @pyparm string|password||password for service account , or None
   );

// @pyswig int/None|ChangeServiceConfig|Changes the configuration of an existing service.
%name (ChangeServiceConfig) PyObject * MyChangeServiceConfig(
    SC_HANDLE hService,		// @pyparm <o PySC_HANDLE>|hService||handle to service to be modified
    DWORD dwServiceType,	// @pyparm int|serviceType||type of service, or SERVICE_NO_CHANGE
    DWORD dwStartType,		// @pyparm int|startType||When/how to start service, or SERVICE_NO_CHANGE
    DWORD dwErrorControl,	// @pyparm int|errorControl||severity if service fails to start, or SERVICE_NO_CHANGE
    TCHAR *INPUT_NULLOK,	// @pyparm string|binaryFile||name of binary file, or None
    TCHAR *INPUT_NULLOK,	// @pyparm string|loadOrderGroup||name of load ordering group , or None
    BOOL  bFetchTag,		// @pyparm int|bFetchTag||Should the tag be fetched and returned?  If TRUE, the result is the tag, else None.
    PyObject *pyobject,		// @pyparm [string,...]|serviceDeps||sequence of dependency names
    TCHAR *INPUT_NULLOK,	// @pyparm string|acctName||account name of service, or None
    TCHAR *INPUT_NULLOK,	// @pyparm string|password||password for service account , or None
    TCHAR *INPUT_NULLOK		// @pyparm string|displayName||Display name
   );

// @pyswig int|LockServiceDatabase|Locks the service database.
SC_LOCK LockServiceDatabase(
	SC_HANDLE handle // @pyparm <o PySC_HANDLE>|sc_handle||A handle to the SCM.
);

// @pyswig int|UnlockServiceDatabase|Unlocks the service database.
BOOLAPI UnlockServiceDatabase(
	SC_LOCK lock // @pyparm int|lock||A lock provided by <om win32service.LockServiceDatabase>
);

%{
// @pyswig (int, string, int)|QueryServiceLockStatus|Retrieves the lock status of the specified service control manager database.
static PyObject *PyQueryServiceLockStatus(PyObject *self, PyObject *args)
{
	SC_HANDLE handle;
	PyObject *obhandle;
	// @pyparm <o PySC_HANDLE>|hSCManager||Handle to the SCM.
	if (!PyArg_ParseTuple(args, "O:QueryServiceLockStatus", &obhandle))
		return NULL;
	if (!PyWinObject_AsHANDLE(obhandle, (HANDLE *)&handle))
		return NULL;
	DWORD bufSize;
	QueryServiceLockStatus(handle, NULL, 0, &bufSize);
	if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
		return PyWin_SetAPIError("QueryServiceLockStatus");
	QUERY_SERVICE_LOCK_STATUS *buf;
	buf = (QUERY_SERVICE_LOCK_STATUS *)malloc(bufSize);
	if (buf==NULL) {
		PyErr_SetString(PyExc_MemoryError, "No memory for status buffer");
		return NULL;
	}
	BOOL ok = QueryServiceLockStatus((SC_HANDLE)handle, buf, bufSize, &bufSize);
	PyObject *ret;
	if (ok) {
		// @rdesc The result is a tuple of (bIsLocked, userName, lockDuration)
		PyObject *str = PyWinObject_FromTCHAR(buf->lpLockOwner);
		ret = Py_BuildValue("lOl", buf->fIsLocked, str, buf->dwLockDuration);
		Py_XDECREF(str);
	} else
		ret = PyWin_SetAPIError("QueryServiceLockStatus");
	free(buf);
	return ret;
}
%}
%native (QueryServiceLockStatus) PyQueryServiceLockStatus;

// @object SC_ACTION|Tuple of 2 ints (Type,Delay) used to represent an SC_ACTION structure
// @prop int|Type|One of SC_ACTION_NONE, SC_ACTION_REBOOT, SC_ACTION_RESTART, SC_ACTION_RUN_COMMAND
// @prop int|Delay|Time delay before specified action is taken (in milliseconds)

// @object SERVICE_FAILURE_ACTIONS|A dictionary representing a SERVICE_FAILURE_ACTIONS structure
// @prop int|ResetPeriod|Indicates how many seconds to wait to reset the failure count, can be INFINITE
// @prop string|RebootMsg|Message displayed when reboot action is taken
// @prop string|Command|Command line to execute for SC_ACTION_RUN_COMMAND
// @prop tuple|Actions|A tuple of <o SC_ACTION> tuples

%{
BOOL PyWinObject_AsSC_ACTION(PyObject *obAction, SC_ACTION *Action)
{
	static char* err="SC_ACTION must be a tuple of 2 ints (Type, Delay)";
	if (!PyTuple_Check(obAction)){
		PyErr_SetString(PyExc_TypeError,err);
		return FALSE;
		}
	if (!PyArg_ParseTuple(obAction,"ll", &Action->Type, &Action->Delay)){
		PyErr_Clear();
		PyErr_SetString(PyExc_TypeError,err);
		return FALSE;
		}
	return TRUE;
}

BOOL PyWinObject_AsSC_ACTIONS(PyObject *obActions, SC_ACTION **ppActions, LPDWORD cActions)
{
	static char* err="SC_ACTIONS must be a sequence of 2-tuples ((int, int),...)";
	DWORD action_ind;
	BOOL ret=TRUE;
	SC_ACTION *pAction;
	if (obActions==Py_None){
		*cActions=0;
		*ppActions=NULL;
		return TRUE;
		}
	PyObject *actions_tuple=PyWinSequence_Tuple(obActions, cActions);
	if (actions_tuple==NULL)
		return FALSE;

	*ppActions=(SC_ACTION *)malloc(*cActions*sizeof(SC_ACTION));
	if (*ppActions==NULL){
		Py_DECREF(actions_tuple);
		PyErr_Format(PyExc_MemoryError,"Unable to allocate %d SC_ACTION structures", *cActions);
		return FALSE;
		}
	pAction=*ppActions;
	for (action_ind=0;action_ind<*cActions;action_ind++){
		ret=PyWinObject_AsSC_ACTION(PyTuple_GET_ITEM(actions_tuple, action_ind), pAction);
		if (!ret){
			free(*ppActions);
			*ppActions=NULL;
			*cActions=0;
			break;
			}
		pAction++;
		}
	Py_DECREF(actions_tuple);
	return ret;
}

void PyWinObject_FreeSERVICE_FAILURE_ACTIONS(LPSERVICE_FAILURE_ACTIONSW psfa)
{
	if (psfa->lpRebootMsg!=NULL)
		PyWinObject_FreeWCHAR(psfa->lpRebootMsg);
	if (psfa->lpCommand!=NULL)
		PyWinObject_FreeWCHAR(psfa->lpCommand);
	if (psfa->lpsaActions!=NULL)
		free(psfa->lpsaActions);
}

BOOL PyWinObject_AsSERVICE_FAILURE_ACTIONS(PyObject *obinfo, LPSERVICE_FAILURE_ACTIONSW psfa)
{
	static char *sfa_keys[]={"ResetPeriod","RebootMsg","Command","Actions",0};
	static char *err="SERVICE_FAILURE_ACTIONS must be a dictionary containing {'ResetPeriod':int,'RebootMsg':unicode,'Command':unicode,'Actions':sequence of 2 tuples(int,int)";
	PyObject *dummy_tuple, *obActions, *obRebootMsg, *obCommand;
	BOOL ret;
	ZeroMemory(psfa, sizeof(SERVICE_FAILURE_ACTIONSW));
	if (!PyDict_Check(obinfo)){
		PyErr_SetString(PyExc_TypeError,err);
		return FALSE;
		}
	dummy_tuple=PyTuple_New(0);
	if (dummy_tuple==NULL)
		return FALSE;
	ret=PyArg_ParseTupleAndKeywords(dummy_tuple, obinfo, "lOOO:SERVICE_FAILURE_ACTIONS", sfa_keys,
		&psfa->dwResetPeriod, &obRebootMsg, &obCommand, &obActions);
	Py_DECREF(dummy_tuple);
	if (!ret){
		PyErr_Clear();
		PyErr_SetString(PyExc_TypeError,err);
		return FALSE;
		}
	if (PyWinObject_AsWCHAR(obRebootMsg, &psfa->lpRebootMsg, TRUE)
		&&PyWinObject_AsWCHAR(obCommand,   &psfa->lpCommand,   TRUE)
		&&PyWinObject_AsSC_ACTIONS(obActions,&psfa->lpsaActions, &psfa->cActions))
		return TRUE;
	PyWinObject_FreeSERVICE_FAILURE_ACTIONS(psfa);
	return FALSE;
}

PyObject *PyWinObject_FromSERVICE_FAILURE_ACTIONS(LPSERVICE_FAILURE_ACTIONSW psfa)
{
	PyObject *obActions, *obAction;
	SC_ACTION *pAction;
	DWORD action_ind;
	obActions=PyTuple_New(psfa->cActions);
	if (obActions==NULL)
		return NULL;
	pAction=psfa->lpsaActions;
	for (action_ind=0;action_ind<psfa->cActions;action_ind++){
		obAction=Py_BuildValue("ll",pAction->Type,pAction->Delay);
		if (obAction==NULL){
			Py_DECREF(obActions);
			return NULL;
			}
		PyTuple_SET_ITEM(obActions, action_ind, obAction);
		pAction++;
		}
	return Py_BuildValue("{s:l,s:u,s:u,s:N}",
		"ResetPeriod", psfa->dwResetPeriod,
		"RebootMsg", psfa->lpRebootMsg,
		"Command", psfa->lpCommand,
		"Actions", obActions);
}

%}

// @pyswig |ChangeServiceConfig2|Modifies advanced service parameters
%native (ChangeServiceConfig2) PyChangeServiceConfig2;
%{
PyObject *PyChangeServiceConfig2(PyObject *self, PyObject *args)
{
	if (fpChangeServiceConfig2==NULL){
		PyErr_SetString(PyExc_NotImplementedError,"ChangeServiceConfig2 is not available on this operating system");
		return NULL;
		}
	SC_HANDLE hService;
	PyObject *obhService;
	DWORD level;
	BOOL bsuccess;

	PyObject *obinfo;
	// @pyparm <o PySC_HANDLE>|hService||Service handle as returned by <om win32service.OpenService>
	// @pyparm int|InfoLevel||One of win32service.SERVICE_CONFIG_* values
	// @pyparm object|info||Type depends on InfoLevel
	if (!PyArg_ParseTuple(args,"OlO:ChangeServiceConfig2", &obhService, &level, &obinfo))
		return NULL;
	if (!PyWinObject_AsHANDLE(obhService, (HANDLE *)&hService))
		return NULL;

	switch (level){
		// @flagh InfoLevel|Input value
		// @flag SERVICE_CONFIG_DESCRIPTION|Unicode string
		case SERVICE_CONFIG_DESCRIPTION:{
			SERVICE_DESCRIPTIONW buf;
			if (!PyWinObject_AsWCHAR(obinfo, &buf.lpDescription, TRUE))
				return NULL;
			bsuccess=(*fpChangeServiceConfig2)(hService, level, (LPVOID)&buf);
			PyWinObject_FreeWCHAR(buf.lpDescription);
			break;
			}
		// @flag SERVICE_CONFIG_FAILURE_ACTIONS|Dict representing a SERVICE_FAILURE_ACTIONS struct
		case SERVICE_CONFIG_FAILURE_ACTIONS:{
			SERVICE_FAILURE_ACTIONSW buf;
			if (!PyWinObject_AsSERVICE_FAILURE_ACTIONS(obinfo, &buf))
				return NULL;
			bsuccess=(*fpChangeServiceConfig2)(hService, level, (LPVOID)&buf);
			PyWinObject_FreeSERVICE_FAILURE_ACTIONS(&buf);
			break;
			}
		// @flag SERVICE_CONFIG_DELAYED_AUTO_START_INFO|Boolean
		case SERVICE_CONFIG_DELAYED_AUTO_START_INFO:{
			SERVICE_DELAYED_AUTO_START_INFO buf;
			buf.fDelayedAutostart=PyObject_IsTrue(obinfo);
			bsuccess=(*fpChangeServiceConfig2)(hService,level, (LPVOID)&buf);
			break;
			}
		// @flag SERVICE_CONFIG_FAILURE_ACTIONS_FLAG|Boolean
		case SERVICE_CONFIG_FAILURE_ACTIONS_FLAG:{
			SERVICE_FAILURE_ACTIONS_FLAG buf;
			buf.fFailureActionsOnNonCrashFailures=PyObject_IsTrue(obinfo);
			bsuccess=(*fpChangeServiceConfig2)(hService,level, (LPVOID)&buf);
			break;
			}
		// @flag SERVICE_CONFIG_PRESHUTDOWN_INFO|int (shutdown timeout in milliseconds)
		case SERVICE_CONFIG_PRESHUTDOWN_INFO:{
			SERVICE_PRESHUTDOWN_INFO buf;
			buf.dwPreshutdownTimeout = PyLong_AsUnsignedLong(obinfo);
			if (buf.dwPreshutdownTimeout==(DWORD)-1 && PyErr_Occurred())
				return NULL;
			bsuccess=(*fpChangeServiceConfig2)(hService, level, (LPVOID)&buf);
			break;
			}
		// @flag SERVICE_CONFIG_SERVICE_SID_INFO|int (SERVICE_SID_TYPE_*)
		case SERVICE_CONFIG_SERVICE_SID_INFO:{
			SERVICE_SID_INFO buf;
			buf.dwServiceSidType=PyLong_AsUnsignedLong(obinfo);
			if (buf.dwServiceSidType==(DWORD)-1 && PyErr_Occurred())
				return NULL;
			bsuccess=(*fpChangeServiceConfig2)(hService, level, (LPVOID)&buf);
			break;
			}
		// @flag SERVICE_CONFIG_REQUIRED_PRIVILEGES_INFO|Sequence of unicode strings
		case SERVICE_CONFIG_REQUIRED_PRIVILEGES_INFO:{
			SERVICE_REQUIRED_PRIVILEGES_INFO buf;
			if (!PyWinObject_AsMultipleString(obinfo, &buf.pmszRequiredPrivileges))
				return NULL;
			bsuccess=(*fpChangeServiceConfig2)(hService, level, (LPVOID)&buf);
			PyWinObject_FreeMultipleString(buf.pmszRequiredPrivileges);
			break;
			}
		default:
			return PyErr_Format(PyExc_ValueError,"Info type %d is not supported",level);
		}

	if (!bsuccess)
		return PyWin_SetAPIError("ChangeServiceConfig2");
	Py_INCREF(Py_None);
	return Py_None;
}
%}

// @pyswig object|QueryServiceConfig2|Retrieves advanced service configuration options
// @rdesc Type of returned object depends on InfoLevel
%native (QueryServiceConfig2) PyQueryServiceConfig2;
%{
PyObject *PyQueryServiceConfig2(PyObject *self, PyObject *args)
{
	if (fpQueryServiceConfig2==NULL){
		PyErr_SetString(PyExc_NotImplementedError,"QueryServiceConfig2 is not available on this operating system");
		return NULL;
		}
	SC_HANDLE hService;
	PyObject *obhService;
	DWORD level, bytes_needed=0, bufsize=0;
	LPBYTE buf=NULL;
	PyObject *ret=NULL;
	// @pyparm <o PySC_HANDLE>|hService||Service handle as returned by <om win32service.OpenService>
	// @pyparm int|InfoLevel||One of win32service.SERVICE_CONFIG_* values
	if (!PyArg_ParseTuple(args,"Ol:QueryServiceConfig2", &obhService, &level))
		return NULL;
	if (!PyWinObject_AsHANDLE(obhService, (HANDLE *)&hService))
		return NULL;
	(*fpQueryServiceConfig2)(hService, level, buf, bufsize, &bytes_needed);
	if (bytes_needed==0){
		PyWin_SetAPIError("QueryServiceConfig2");
		return NULL;
		}
	buf=(LPBYTE)malloc(bytes_needed);
	if (buf==NULL)
		return PyErr_Format(PyExc_MemoryError,"QueryServiceConfig2: Unable to allocate buffer of %d bytes",bytes_needed);
	bufsize=bytes_needed;

	if ((*fpQueryServiceConfig2)(hService, level, buf, bufsize, &bytes_needed))
		switch(level){
			// @flagh InfoLevel|Type of value returned
			// @flag SERVICE_CONFIG_DESCRIPTION|Unicode string
			case SERVICE_CONFIG_DESCRIPTION:
				ret=PyWinObject_FromWCHAR(((SERVICE_DESCRIPTIONW *)buf)->lpDescription);
				break;
			// @flag SERVICE_CONFIG_FAILURE_ACTIONS|Dict representing a SERVICE_FAILURE_ACTIONS struct
			case SERVICE_CONFIG_FAILURE_ACTIONS:
				ret=PyWinObject_FromSERVICE_FAILURE_ACTIONS((LPSERVICE_FAILURE_ACTIONSW)buf);
				break;
			// @flag SERVICE_CONFIG_DELAYED_AUTO_START_INFO|Boolean
			case SERVICE_CONFIG_DELAYED_AUTO_START_INFO:
				ret=PyBool_FromLong(((SERVICE_DELAYED_AUTO_START_INFO *)buf)->fDelayedAutostart);
				break;
			// @flag SERVICE_CONFIG_FAILURE_ACTIONS_FLAG|Boolean
			case SERVICE_CONFIG_FAILURE_ACTIONS_FLAG:
				ret=PyBool_FromLong(((SERVICE_FAILURE_ACTIONS_FLAG *)buf)->fFailureActionsOnNonCrashFailures);
				break;
			// @flag SERVICE_CONFIG_PRESHUTDOWN_INFO|int (shutdown timeout in milliseconds)
			case SERVICE_CONFIG_PRESHUTDOWN_INFO:
				ret=PyLong_FromUnsignedLong(((SERVICE_PRESHUTDOWN_INFO *)buf)->dwPreshutdownTimeout);
				break;
			// @flag SERVICE_CONFIG_SERVICE_SID_INFO|int (SERVICE_SID_TYPE_*)
			case SERVICE_CONFIG_SERVICE_SID_INFO:
				ret=PyLong_FromUnsignedLong(((SERVICE_SID_INFO *)buf)->dwServiceSidType);
				break;
			// @flag SERVICE_CONFIG_REQUIRED_PRIVILEGES_INFO|List of unicode strings
			case SERVICE_CONFIG_REQUIRED_PRIVILEGES_INFO:
				ret=PyWinObject_FromMultipleString(((SERVICE_REQUIRED_PRIVILEGES_INFO *)buf)->pmszRequiredPrivileges);
				break;
			default:
				PyErr_Format(PyExc_NotImplementedError,"QueryServiceConfig2: Level %d is not supported", level);
			}
	else
		PyWin_SetAPIError("QueryServiceConfig2");
	free(buf);
	return ret;
}
%}


#define SERVICE_WIN32 SERVICE_WIN32

#define SERVICE_DRIVER SERVICE_DRIVER

#define SERVICE_ACTIVE SERVICE_ACTIVE

#define SERVICE_INACTIVE SERVICE_INACTIVE

#define SERVICE_STATE_ALL SERVICE_STATE_ALL

#define SERVICE_CONTROL_STOP SERVICE_CONTROL_STOP
// Requests the service to stop. The hService handle must have SERVICE_STOP access.
#define SERVICE_CONTROL_PAUSE SERVICE_CONTROL_PAUSE
// Requests the service to pause. The hService handle must have SERVICE_PAUSE_CONTINUE access.
#define SERVICE_CONTROL_CONTINUE SERVICE_CONTROL_CONTINUE
// Requests the paused service to resume. The hService handle must have SERVICE_PAUSE_CONTINUE access.
#define SERVICE_CONTROL_INTERROGATE SERVICE_CONTROL_INTERROGATE
// Requests the service to update immediately its current status information to the service control manager. The hService handle must have SERVICE_INTERROGATE access.
#define SERVICE_CONTROL_SHUTDOWN SERVICE_CONTROL_SHUTDOWN
// The ControlService function fails if this control code is specified.
#define SERVICE_CONTROL_PARAMCHANGE SERVICE_CONTROL_PARAMCHANGE
#define SERVICE_CONTROL_NETBINDADD SERVICE_CONTROL_NETBINDADD
#define SERVICE_CONTROL_NETBINDREMOVE SERVICE_CONTROL_NETBINDREMOVE
#define SERVICE_CONTROL_NETBINDENABLE SERVICE_CONTROL_NETBINDENABLE
#define SERVICE_CONTROL_NETBINDDISABLE SERVICE_CONTROL_NETBINDDISABLE
#define SERVICE_CONTROL_DEVICEEVENT SERVICE_CONTROL_DEVICEEVENT
#define SERVICE_CONTROL_HARDWAREPROFILECHANGE SERVICE_CONTROL_HARDWAREPROFILECHANGE
#define SERVICE_CONTROL_POWEREVENT SERVICE_CONTROL_POWEREVENT
#define SERVICE_CONTROL_SESSIONCHANGE SERVICE_CONTROL_SESSIONCHANGE
#define SERVICE_CONTROL_PRESHUTDOWN SERVICE_CONTROL_PRESHUTDOWN


#define SC_MANAGER_ALL_ACCESS SC_MANAGER_ALL_ACCESS
// Includes STANDARD_RIGHTS_REQUIRED, in addition to all of the access types listed in this table.
#define SC_MANAGER_CONNECT SC_MANAGER_CONNECT
// Enables connecting to the service control manager.
#define SC_MANAGER_CREATE_SERVICE SC_MANAGER_CREATE_SERVICE
// Enables calling of the CreateService function to create a service object and add it to the database.
#define SC_MANAGER_ENUMERATE_SERVICE SC_MANAGER_ENUMERATE_SERVICE
// Enables calling of the EnumServicesStatus function to list the services that are in the database.
#define SC_MANAGER_LOCK SC_MANAGER_LOCK
// Enables calling of the LockServiceDatabase function to acquire a lock on the database.
#define SC_MANAGER_QUERY_LOCK_STATUS SC_MANAGER_QUERY_LOCK_STATUS
// Enables calling of the QueryServiceLockStatus function to retrieve the lock status information for the database.

#define SC_MANAGER_MODIFY_BOOT_CONFIG SC_MANAGER_MODIFY_BOOT_CONFIG

#define SC_GROUP_IDENTIFIER  SC_GROUP_IDENTIFIER

#define SERVICE_WIN32_OWN_PROCESS SERVICE_WIN32_OWN_PROCESS
// A service type flag that indicates a Win32 service that runs in its own process.
#define SERVICE_WIN32_SHARE_PROCESS SERVICE_WIN32_SHARE_PROCESS
// A service type flag that indicates a Win32 service that shares a process with other services.
#define SERVICE_KERNEL_DRIVER SERVICE_KERNEL_DRIVER
// A service type flag that indicates a Windows NT device driver.
#define SERVICE_FILE_SYSTEM_DRIVER SERVICE_FILE_SYSTEM_DRIVER
// A service type flag that indicates a Windows NT file system driver.
#define SERVICE_INTERACTIVE_PROCESS  SERVICE_INTERACTIVE_PROCESS
// A flag that indicates a Win32 service process that can interact with the desktop.

#define SERVICE_STOPPED	SERVICE_STOPPED
// The service is not running.
#define SERVICE_START_PENDING SERVICE_START_PENDING
// The service is starting.
#define SERVICE_STOP_PENDING SERVICE_STOP_PENDING
// The service is stopping.
#define SERVICE_RUNNING SERVICE_RUNNING
// The service is running.
#define SERVICE_CONTINUE_PENDING SERVICE_CONTINUE_PENDING
// The service continue is pending.
#define SERVICE_PAUSE_PENDING SERVICE_PAUSE_PENDING
// The service pause is pending.
#define SERVICE_PAUSED SERVICE_PAUSED
// The service is paused.

#define SERVICE_ACCEPT_STOP SERVICE_ACCEPT_STOP
// The service can be stopped. This enables the SERVICE_CONTROL_STOP value.
#define SERVICE_ACCEPT_PAUSE_CONTINUE SERVICE_ACCEPT_PAUSE_CONTINUE
// The service can be paused and continued. This enables the SERVICE_CONTROL_PAUSE and SERVICE_CONTROL_CONTINUE values.
#define SERVICE_ACCEPT_SHUTDOWN SERVICE_ACCEPT_SHUTDOWN
// The service is notified when system shutdown occurs. This enables the system to send a SERVICE_CONTROL_SHUTDOWN value to the service. The ControlService function cannot send this control
#define SERVICE_ACCEPT_PARAMCHANGE SERVICE_ACCEPT_PARAMCHANGE
#define SERVICE_ACCEPT_NETBINDCHANGE SERVICE_ACCEPT_NETBINDCHANGE
#define SERVICE_ACCEPT_HARDWAREPROFILECHANGE SERVICE_ACCEPT_HARDWAREPROFILECHANGE
#define SERVICE_ACCEPT_POWEREVENT SERVICE_ACCEPT_POWEREVENT
#define SERVICE_ACCEPT_SESSIONCHANGE SERVICE_ACCEPT_SESSIONCHANGE
#define SERVICE_ACCEPT_PRESHUTDOWN SERVICE_ACCEPT_PRESHUTDOWN

//#define SERVICE_ERROR_IGNORER_IGNORE SERVICE_ERROR_IGNORER_IGNORE
#define SERVICE_BOOT_START SERVICE_BOOT_START
// Specifies a device driver started by the operating system loader. This value is valid only if the service type is SERVICE_KERNEL_DRIVER or SERVICE_FILE_SYSTEM_DRIVER.
#define SERVICE_SYSTEM_START SERVICE_SYSTEM_START
// Specifies a device driver started by the IoInitSystem function. This value is valid only if the service type is SERVICE_KERNEL_DRIVER or SERVICE_FILE_SYSTEM_DRIVER.
#define SERVICE_AUTO_START SERVICE_AUTO_START
// Specifies a device driver or Win32 service started by the service control manager automatically during system startup.
#define SERVICE_DEMAND_START SERVICE_DEMAND_START
// Specifies a device driver or Win32 service started by the service control manager when a process calls the StartService function.
#define SERVICE_DISABLED SERVICE_DISABLED
// Specifies a device driver or Win32 service that can no longer be started.

#define SERVICE_ERROR_IGNORE SERVICE_ERROR_IGNORE
// The startup (boot) program logs the error but continues the startup operation.
#define SERVICE_ERROR_NORMAL SERVICE_ERROR_NORMAL
// The startup program logs the error and displays a message box pop-up but continues the startup operation.
#define SERVICE_ERROR_SEVERE SERVICE_ERROR_SEVERE
// The startup program logs the error. If the last-known good configuration is being started,
	// the startup operation continues. Otherwise, the system is restarted with the last-known-good configuration.
#define SERVICE_ERROR_CRITICAL SERVICE_ERROR_CRITICAL
// The startup program logs the error, if possible. If the last-known good configuration is being started,
	// the startup operation fails. Otherwise, the system is restarted with the last-known good configuration.

#define SERVICE_ALL_ACCESS SERVICE_ALL_ACCESS
// Includes STANDARD_RIGHTS_REQUIRED in addition to all of the access types listed in this table.

#define SERVICE_CHANGE_CONFIG SERVICE_CHANGE_CONFIG
// Enables calling of the ChangeServiceConfig function to change the service configuration.

#define SERVICE_ENUMERATE_DEPENDENTS SERVICE_ENUMERATE_DEPENDENTS
//Enables calling of the EnumDependentServices function to enumerate all the services dependent on the service.

#define SERVICE_INTERROGATE SERVICE_INTERROGATE
// Enables calling of the ControlService function to ask the service to report its status immediately.

#define SERVICE_PAUSE_CONTINUE SERVICE_PAUSE_CONTINUE
// Enables calling of the ControlService function to pause or continue the service.

#define SERVICE_QUERY_CONFIG SERVICE_QUERY_CONFIG
// Enables calling of the QueryServiceConfig function to query the service configuration.

#define SERVICE_QUERY_STATUS SERVICE_QUERY_STATUS
// Enables calling of the QueryServiceStatus function to ask the service control manager about the status of the service.

#define SERVICE_START SERVICE_START
// Enables calling of the StartService function to start the service.

#define SERVICE_STOP SERVICE_STOP
// Enables calling of the ControlService function to stop the service.

#define SERVICE_USER_DEFINED_CONTROL SERVICE_USER_DEFINED_CONTROL
// Enables calling of the ControlService function to specify a user-defined control code.

#define SERVICE_NO_CHANGE SERVICE_NO_CHANGE // Indicates the parameter should not be changed.

#define SERVICE_SPECIFIC_ERROR ERROR_SERVICE_SPECIFIC_ERROR  // A service specific error has occurred.

#define UOI_FLAGS UOI_FLAGS
#define UOI_NAME UOI_NAME
#define UOI_TYPE UOI_TYPE
#define UOI_USER_SID UOI_USER_SID
#define WSF_VISIBLE WSF_VISIBLE
#define DF_ALLOWOTHERACCOUNTHOOK DF_ALLOWOTHERACCOUNTHOOK
// #define CWF_CREATE_ONLY CWF_CREATE_ONLY

// Types of info used with QueryServiceConfig2
#define SERVICE_CONFIG_DESCRIPTION SERVICE_CONFIG_DESCRIPTION
#define SERVICE_CONFIG_FAILURE_ACTIONS SERVICE_CONFIG_FAILURE_ACTIONS
// These require Vista or above
#define SERVICE_CONFIG_DELAYED_AUTO_START_INFO SERVICE_CONFIG_DELAYED_AUTO_START_INFO
#define SERVICE_CONFIG_FAILURE_ACTIONS_FLAG SERVICE_CONFIG_FAILURE_ACTIONS_FLAG
#define SERVICE_CONFIG_PRESHUTDOWN_INFO SERVICE_CONFIG_PRESHUTDOWN_INFO
#define SERVICE_CONFIG_REQUIRED_PRIVILEGES_INFO SERVICE_CONFIG_REQUIRED_PRIVILEGES_INFO
#define SERVICE_CONFIG_SERVICE_SID_INFO SERVICE_CONFIG_SERVICE_SID_INFO

// Info level for EnumServicesStatusEx
#define SC_ENUM_PROCESS_INFO SC_ENUM_PROCESS_INFO

// Used with SERVICE_CONFIG_SERVICE_SID_INFO
#define SERVICE_SID_TYPE_NONE SERVICE_SID_TYPE_NONE
#define SERVICE_SID_TYPE_RESTRICTED SERVICE_SID_TYPE_RESTRICTED
#define SERVICE_SID_TYPE_UNRESTRICTED SERVICE_SID_TYPE_UNRESTRICTED

// Service failure actions
#define SC_ACTION_NONE SC_ACTION_NONE
#define SC_ACTION_REBOOT SC_ACTION_REBOOT
#define SC_ACTION_RESTART SC_ACTION_RESTART
#define SC_ACTION_RUN_COMMAND SC_ACTION_RUN_COMMAND

// These constants relate to events.
// These power related ones aren't strictly related to services, but thats OK
#define DBT_DEVICEARRIVAL DBT_DEVICEARRIVAL // system detected a new device
#define DBT_DEVICEQUERYREMOVE DBT_DEVICEQUERYREMOVE // wants to remove, may fail
#define DBT_DEVICEQUERYREMOVEFAILED DBT_DEVICEQUERYREMOVEFAILED // removal aborted
#define DBT_DEVICEREMOVEPENDING DBT_DEVICEREMOVEPENDING // about to remove, still avail.
#define DBT_DEVICEREMOVECOMPLETE DBT_DEVICEREMOVECOMPLETE // device is gone
#define DBT_DEVICETYPESPECIFIC DBT_DEVICETYPESPECIFIC // type specific event
#define DBT_CUSTOMEVENT DBT_CUSTOMEVENT // user-defined event

#define DBT_QUERYCHANGECONFIG DBT_QUERYCHANGECONFIG
#define DBT_CONFIGCHANGED DBT_CONFIGCHANGED
#define DBT_CONFIGCHANGECANCELED DBT_CONFIGCHANGECANCELED
