/* File : win32service.i */

%module win32service // An interface to the Windows NT Service API


%include "typemaps.i"
%include "pywin32.i"

%{
#undef PyHANDLE
#include "PyWinObjects.h"
static BOOL (WINAPI *fpQueryServiceStatusEx)(SC_HANDLE,SC_STATUS_TYPE,LPBYTE,DWORD,LPDWORD) = NULL;
%}

%init %{
	// All errors raised by this module are of this type.
	Py_INCREF(PyWinExc_ApiError);
	PyDict_SetItemString(d, "error", PyWinExc_ApiError);
	PyDict_SetItemString(d, "HWINSTAType", (PyObject *)&PyHWINSTAType);
	PyDict_SetItemString(d, "HDESKType", (PyObject *)&PyHDESKType);
	HMODULE hmod;
	FARPROC fp;
	hmod=GetModuleHandle(_T("Advapi32"));
	if (hmod==NULL)
		hmod=LoadLibrary(_T("Advapi32"));
	if (hmod!=NULL){
		fp=GetProcAddress(hmod,"QueryServiceStatusEx");
		if (fp!=NULL)
			fpQueryServiceStatusEx=(BOOL (WINAPI *)(SC_HANDLE,SC_STATUS_TYPE,LPBYTE,DWORD,LPDWORD))fp;
		}
%}

%{
#include "structmember.h"
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
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "l", keywords, &hwinsta))
		return NULL;
	return new PyHWINSTA(hwinsta);
}

PyTypeObject PyHWINSTAType =
{
	PyObject_HEAD_INIT(&PyType_Type)
	0,
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


// @object PyHDESK|Object representing a handle to a desktop, created by CreateDesktop, GetThreadDesktop, and OpenDesktop. 

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
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "l", keywords, &hdesk))
		return NULL;
	return new PyHDESK(hdesk);
}

PyTypeObject PyHDESKType =
{
	PyObject_HEAD_INIT(&PyType_Type)
	0,
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

// @pymethod (PyUNICODE,...)|PyHWINSTA|EnumDesktops|Lists names of desktops in the window station
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

// @pyswig (PyUNICODE,...)|EnumWindowStations|Lists names of window stations
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
// @comm Return type is dependent on UOI_* constant passed in
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
	// @pyparm str/PyUNICODE|szWinSta||Name of window station
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
	// @pyparm str/unicode|szDesktop||Name of desktop to open
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
	// @pyparm str/unicode|Desktop||Name of desktop to create
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
	// @pyparm str/unicode|WindowStation||Name of window station to create, or None
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
BOOL BuildDeps(PyObject *obDeps, TCHAR **ppDeps)
{
	TCHAR *lpDeps = NULL;
	BOOL rc = FALSE;
	if (obDeps!=Py_None) {
		if (!PySequence_Check(obDeps)) {
			PyErr_SetString(PyExc_ValueError, "Dependencies must be None or a list of strings");
			goto cleanup;
		}
 		int numStrings = PySequence_Length(obDeps);
		// Need to loop twice - once to get the buffer length
		int len = 0;
		for (int i=0;i<numStrings;i++) {
			PyObject *obString = PySequence_GetItem(obDeps, i);
			if (obString==NULL)
				goto cleanup;
			if (!PyString_Check(obString)) {
				Py_DECREF(obString);
				PyErr_SetString(PyExc_ValueError, "The list items for Dependencies must all be strings");
				goto cleanup;
			}
			len += PyString_Size(obString) + 1;
			Py_DECREF(obString);
		}
		// Allocate the buffer
		lpDeps = new TCHAR[len+2]; // Double '\0' terminated
		TCHAR *p = lpDeps;
		for (i=0;i<numStrings;i++) {
			// We know the sequence is valid.
			PyObject *obString = PySequence_GetItem(obDeps, i);
			BSTR pStr;
			if (!PyWinObject_AsTCHAR(obString, &pStr)) {
				Py_DECREF(obString);
				goto cleanup;
			}
			int len = _tcslen(pStr);
			_tcsncpy(p, pStr, len);
			p += len;
			*p++ = L'\0';
			PyWinObject_FreeTCHAR(pStr);
			Py_DECREF(obString);
		}
		*p = L'\0'; // Add second terminator.
	}
	*ppDeps = lpDeps;
	rc = TRUE;
cleanup:
	if (!rc) {
		delete [] lpDeps;
	}
	return rc;
}

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
	if (!BuildDeps(obDeps, &lpDeps))
		goto cleanup;

	sh = CreateService(hSCManager,lpServiceName,lpDisplayName,dwDesiredAccess,
	                             dwServiceType, dwStartType, dwErrorControl, lpBinaryPathName,
	                             lpLoadOrderGroup, pTagID, lpDeps, lpServiceStartName, lpPassword);
	if (sh==0) {
		PyWin_SetAPIError("CreateService");
		rc = NULL;
	} else {
		if (bFetchTag)
			rc = Py_BuildValue("ll", sh, tagID);
		else
			rc = PyInt_FromLong((long)sh);
	}
cleanup:
	delete [] lpDeps;
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
	if (!BuildDeps(obDeps, &lpDeps))
		goto cleanup;

	if (!ChangeServiceConfig(hSCManager,
                         dwServiceType, dwStartType, dwErrorControl, lpBinaryPathName,
                         lpLoadOrderGroup, pTagID, lpDeps, lpServiceStartName, lpPassword,
						 lpDisplayName))
		rc = PyWin_SetAPIError("ChangeServiceConfig");
	else if (bFetchTag)
		rc = PyInt_FromLong(tagID);
	else {
		rc = Py_None;
		Py_INCREF(rc);
	}
cleanup:
	delete [] lpDeps;
	return rc;
		
}

PyObject *MyStartService( SC_HANDLE scHandle, PyObject *serviceArgs )
{
	LPTSTR *pArgs;
	DWORD numStrings = 0;
	if (serviceArgs==Py_None)
		pArgs = NULL;
	else if (!PySequence_Check(serviceArgs)) {
		PyErr_SetString(PyExc_ValueError, "Service arguments must be list of strings.");
		return NULL;
	} else {
		numStrings = PySequence_Length(serviceArgs);
		pArgs = new LPTSTR [numStrings];
		if (pArgs==NULL) {
			PyErr_SetString(PyExc_MemoryError, "Allocating argument arrays");
			return NULL;
		}
		for (DWORD i=0;i<numStrings;i++) {
			PyObject *obString = PySequence_GetItem(serviceArgs, (int)i);
			if (obString==NULL) {
				delete [] pArgs;
				return NULL;
			}
			pArgs[i] = NULL;
			PyWinObject_AsTCHAR(obString, pArgs+i);
			Py_DECREF(obString);
		}
	}
	PyObject *rc;
	if (StartService(scHandle, numStrings, (LPCTSTR *)pArgs)) {
		rc = Py_None;
		Py_INCREF(Py_None);
	} else
		rc = PyWin_SetAPIError("StartService");
	for (DWORD i=0;i<numStrings;i++)
		PyWinObject_FreeTCHAR(pArgs[i]);
	delete [] pArgs;
	return rc;
}
%}

// These 3 function contributed by Curt Hagenlocher

%native (EnumServicesStatus) MyEnumServicesStatus;

%{
static PyObject *MyEnumServicesStatus(PyObject *self, PyObject *args)
{
	SC_HANDLE hscm;
	DWORD serviceType = SERVICE_WIN32;
	DWORD serviceState = SERVICE_STATE_ALL;
	if (!PyArg_ParseTuple(args, "l|ll:EnumServicesStatus", &hscm, &serviceType, &serviceState))
	{
		return NULL;
	}

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
		delete buffer;
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

	delete buffer;
	return retval;
}
%}

%native (EnumDependentServices) MyEnumDependentServices;
%{
static PyObject *MyEnumDependentServices(PyObject *self, PyObject *args)
{
	SC_HANDLE hsc;
	DWORD serviceState = SERVICE_STATE_ALL;
	if (!PyArg_ParseTuple(args, "l|l:EnumDependentServices", &hsc, &serviceState))
	{
		return NULL;
	}

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
		delete buffer;
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

	delete buffer;
	return retval;
}
%}

%native (QueryServiceConfig) MyQueryServiceConfig;

%{
static PyObject *MyQueryServiceConfig(PyObject *self, PyObject *args)
{
	SC_HANDLE hsc;
	if (!PyArg_ParseTuple(args, "l:QueryServiceConfig", &hsc))
	{
		return NULL;
	}

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
		delete buffer;
		return PyWin_SetAPIError("QueryServiceConfig");
	}

	PyObject *obBinaryPathName = PyWinObject_FromTCHAR(config->lpBinaryPathName);
	PyObject *obLoadOrderGroup = PyWinObject_FromTCHAR(config->lpLoadOrderGroup);
	PyObject *obDependencies = PyWinObject_FromTCHAR(config->lpDependencies);
	PyObject *obServiceStartName = PyWinObject_FromTCHAR(config->lpServiceStartName);
	PyObject *obDisplayName = PyWinObject_FromTCHAR(config->lpDisplayName);
	PyObject *retval = Py_BuildValue("lllOOlOOO",
			config->dwServiceType,
			config->dwStartType,
			config->dwErrorControl,
			obBinaryPathName,
			obLoadOrderGroup,
			config->dwTagId,
			obDependencies,
			obServiceStartName,
			obDisplayName);
	Py_XDECREF(obBinaryPathName);
	Py_XDECREF(obLoadOrderGroup);
	Py_XDECREF(obDependencies);
	Py_XDECREF(obServiceStartName);
	Py_XDECREF(obDisplayName);

	delete buffer;
	return retval;
}
%}

typedef long SC_HANDLE; // 32 bit?
typedef long SC_LOCK;
typedef long SERVICE_STATUS_HANDLE
//typedef unsigned int TCHAR;

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
     SC_HANDLE  scHandle, // @pyparm int|scHandle||Handle to the Service Control Mananger
     PyObject *pyobject /* serviceArgs */); // @pyparm [string, ...]|args||Arguments to the service.

// @pyswig int|OpenService|Returns a handle to the specified service.
SC_HANDLE OpenService(
	SC_HANDLE hSCManager, // @pyparm int|scHandle||Handle to the Service Control Mananger
	TCHAR *name, // @pyparm <o PyUnicode>|name||The name of the service to open.
	unsigned long desiredAccess); // @pyparm int|desiredAccess||The access desired.

// @pyswig int|OpenSCManager|Returns a handle to the service control manager
SC_HANDLE OpenSCManager(
	TCHAR *INPUT_NULLOK, // @pyparm <o PyUnicode>|machineName||The name of the computer, or None
	TCHAR *INPUT_NULLOK, // @pyparm <o PyUnicode>|dbName||The name of the service database, or None
	unsigned long desiredAccess); // @pyparm int|desiredAccess||The access desired.

// @pyswig |CloseServiceHandle|Closes a service handle
BOOLAPI CloseServiceHandle(SC_HANDLE handle); // @pyparm int|scHandle||Handle to close

// @pyswig <o SERVICE_STATUS>|QueryServiceStatus|Queries a service status
BOOLAPI QueryServiceStatus(SC_HANDLE handle, SERVICE_STATUS *outServiceStatus);
// @pyparm int|scHandle||Handle to query

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
	SC_STATUS_TYPE InfoLevel=SC_STATUS_PROCESS_INFO;  // only existing info level
	SERVICE_STATUS_PROCESS info;
	DWORD bufsize=sizeof(SERVICE_STATUS_PROCESS);
	DWORD reqdbufsize;
	// @pyparm int|scHandle||Handle to query
	if (!PyArg_ParseTuple(args,"l:QueryServiceStatusEx",&hService))
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
	// @pyparm int|Handle||Service handle
	// @pyparm int|SecurityInformation||Type of infomation to set, combination of values from SECURITY_INFORMATION enum
	// @pyparm <o PySECURITY_DESCRIPTOR>|SecurityDescriptor||PySECURITY_DESCRIPTOR containing infomation to set
	if (!PyArg_ParseTuple(args,"llO:SetServiceObjectSecurity",&hsvc, &info, &obSD))
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
	// @pyparm int|Handle||Service handle
	// @pyparm int|SecurityInformation||Type of infomation to retrieve, combination of values from SECURITY_INFORMATION enum
	if (!PyArg_ParseTuple(args,"ll:QueryServiceObjectSecurity",&hsvc, &info))
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

// @pyswig <o SERVICE_STATUS>|SetServiceStatus|Sets a service status
BOOLAPI SetServiceStatus(
	SERVICE_STATUS_HANDLE hSCManager, // @pyparm int|scHandle||Handle to set
	SERVICE_STATUS *inServiceStatus); // @pyparm object|serviceStatus||The new status

// @pyswig <o SERVICE_STATUS>|ControlService|Sends a control message to a service.
// @rdesc The result is the new service status.
BOOLAPI ControlService(
    SC_HANDLE handle, // @pyparm int|scHandle||Handle to control
    DWORD status, // @pyparm int|code||The service control code.
    SERVICE_STATUS *outServiceStatus);

// @pyswig |DeleteService|Deletes the specified service
BOOLAPI DeleteService(SC_HANDLE);
// @pyparm int|scHandle||Handle to delete

// @pyswig int/(int, int)|CreateService|Creates a new service.
%name (CreateService) PyObject * MyCreateService(
    SC_HANDLE hSCManager,	// @pyparm int|scHandle||handle to service control manager database  
    TCHAR *name,	// @pyparm <o PyUnicode>|name||Name of service
    TCHAR *displayName,	// @pyparm <o PyUnicode>|displayName||Display name 
    DWORD dwDesiredAccess,	// @pyparm int|desiredAccess||type of access to service 
    DWORD dwServiceType,	// @pyparm int|serviceType||type of service 
    DWORD dwStartType,		// @pyparm int|startType||When/how to start service 
    DWORD dwErrorControl,	// @pyparm int|errorControl||severity if service fails to start
    TCHAR *binaryFile,	// @pyparm <o PyUnicode>|binaryFile||name of binary file 
    TCHAR *INPUT_NULLOK,	// @pyparm <o PyUnicode>|loadOrderGroup||name of load ordering group , or None
    BOOL  bFetchTag,            // @pyparm int|bFetchTag||Should the tag be fetched and returned?  If TRUE, the result is a tuple of (handle, tag), otherwise just handle.
    PyObject *pyobject,		// @pyparm [<o PyUnicode>,...]|serviceDeps||sequence of dependency names 
    TCHAR *INPUT_NULLOK,	// @pyparm <o PyUnicode>|acctName||account name of service, or None
    TCHAR *INPUT_NULLOK 	// @pyparm <o PyUnicode>|password||password for service account , or None
   );

// @pyswig int/None|ChangeServiceConfig|Changes the configuration of an existing service.
%name (ChangeServiceConfig) PyObject * MyChangeServiceConfig(
    SC_HANDLE hSCManager,	// @pyparm int|scHandle||handle to service control manager database  
    DWORD dwServiceType,	// @pyparm int|serviceType||type of service, or SERVICE_NO_CHANGE
    DWORD dwStartType,		// @pyparm int|startType||When/how to start service, or SERVICE_NO_CHANGE
    DWORD dwErrorControl,	// @pyparm int|errorControl||severity if service fails to start, or SERVICE_NO_CHANGE
    TCHAR *INPUT_NULLOK,	// @pyparm <o PyUnicode>|binaryFile||name of binary file, or None
    TCHAR *INPUT_NULLOK,	// @pyparm <o PyUnicode>|loadOrderGroup||name of load ordering group , or None
    BOOL  bFetchTag,            // @pyparm int|bFetchTag||Should the tag be fetched and returned?  If TRUE, the result is the tag, else None.
    PyObject *pyobject,		// @pyparm [<o PyUnicode>,...]|serviceDeps||sequence of dependency names 
    TCHAR *INPUT_NULLOK,	// @pyparm <o PyUnicode>|acctName||account name of service, or None
    TCHAR *INPUT_NULLOK, 	// @pyparm <o PyUnicode>|password||password for service account , or None
    TCHAR *INPUT_NULLOK	// @pyparm <o PyUnicode>|displayName||Display name 
   );

// @pyswig int|LockServiceDatabase|Locks the service database.
SC_LOCK LockServiceDatabase(
	SC_HANDLE handle // @pyparm int|sc_handle||A handle to the SCM.
);

// @pyswig int|UnlockServiceDatabase|Unlocks the service database.
BOOLAPI UnlockServiceDatabase(
	SC_LOCK lock // @pyparm int|lock||A lock provided by <om win32service.LockServiceDatabase>
);

%{
// @pyswig (int, <o PyUnicode>, int)|QueryServiceLockStatus|Retrieves the lock status of the specified service control manager database. 
static PyObject *PyQueryServiceLockStatus(PyObject *self, PyObject *args)
{
	long handle;
	// @pyparm int|handle||Handle to the SCM.
	if (!PyArg_ParseTuple(args, "l:QueryServiceLockStatus", &handle))
		return NULL;

	DWORD bufSize;
	QueryServiceLockStatus((SC_HANDLE)handle, NULL, 0, &bufSize);
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
