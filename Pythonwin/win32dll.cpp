/*

win32dll - A Python interface to a windows DLL.

Note that this source file contains embedded documentation.
This documentation consists of marked up text inside the
C comments, and is prefixed with an '@' symbol.  The source
files are processed by a tool called "autoduck" which
generates Windows .hlp files.
@doc

*/
#include "stdafx.h"
#include "win32dll.h"

dll_object::dll_object()
{
	bDidLoadLibrary = FALSE;
	pMFCExt = NULL;
	pCDLL = NULL;
}
dll_object::~dll_object()
{
	if (bDidLoadLibrary = TRUE) {
		::FreeLibrary (GetDll());
		TRACE("Python object freeing DLL reference\n");
	}
	if (pMFCExt) {
		AfxTermExtensionModule(*pMFCExt); // this deletes the DLL.
		delete pMFCExt;
		pMFCExt = NULL;
		pCDLL = NULL;
	}
}

// @pymethod <o PyDLL>|win32ui|LoadLibrary|Creates a DLL object, and loads a Windows DLL into the object.
PyObject *
dll_object::create (PyObject *self, PyObject *args)
{
	TCHAR *file;
	PyObject *obfile;
	int flags=0;
	// @pyparm string|fileName||The name of the DLL file to load.
	if (!PyArg_ParseTuple(args, "O|i", &obfile, &flags ))
		return NULL;
	if (!PyWinObject_AsTCHAR(obfile, &file, FALSE))
		return NULL;
	BOOL bDidLoadLib = FALSE;
	// must convert to full path, else GetModuleHandle may fail.
	TCHAR fullPath[MAX_PATH];
	if (!AfxFullPath(fullPath, file )){
		PyWinObject_FreeTCHAR(file);
		RETURN_ERR("The filename is invalid");
	}
	PyWinObject_FreeTCHAR(file);
	HINSTANCE dll = ::GetModuleHandle(fullPath);
	if (dll==NULL) {
		bDidLoadLib = TRUE;
		dll = ::LoadLibraryEx (fullPath, NULL, flags);
		if (dll == NULL) {
			if (GetLastError()==ERROR_MOD_NOT_FOUND) {
				PyErr_SetString(PyExc_IOError,"Module not found");
				return NULL;
			} else
				RETURN_API_ERR ("LoadLibraryEx");
		}
  	}
	dll_object *ret = (dll_object *)ui_assoc_object::make(dll_object::type, dll);
	if (bDidLoadLib)
		ret->bDidLoadLibrary = TRUE;
	return ret;
}
// @pymethod |PyDLL|AttachToMFC|Attaches the DLL object to the MFC list of DLL's.
// @comm After calling this method, MFC will search this DLL when looking for resources.
// A program can use this function once, instead of specifying the DLL
// in each call to load/find a resource.<nl>In addition, this is the only way that
// an application can provide status bar messages and tool tips for custom control
// ID's in an external DLL.
static PyObject *
dll_object_attach_to_mfc( PyObject *self, PyObject *args )
{
	CHECK_NO_ARGS(args);
	dll_object *dll = (dll_object *)self;
	if (dll->pMFCExt)
		RETURN_ERR("The DLL has already been attached to MFC");
	HINSTANCE hInst = dll->GetDll();
	if (hInst==NULL)
		RETURN_ERR("There is no DLL attached to the object");

	dll->pMFCExt = new AFX_EXTENSION_MODULE;	// this will except rather than return NULL
	dll->pMFCExt->bInitialized = 0;
	dll->pMFCExt->hModule = 0;
	if (!AfxInitExtensionModule( *dll->pMFCExt, hInst ))
		RETURN_ERR("AfxInitExtensionModule failed.");
	dll->pCDLL = new CDynLinkLibrary( *dll->pMFCExt );
	RETURN_NONE;
}

// @pymethod string|PyDLL|GetFileName|Returns the name of the module associated with the DLL.
// @comm Note that this is the name that Windows knows the DLL by, not necessarily
// the name that was specified!
static PyObject *
dll_object_get_file_name( PyObject *self, PyObject *args )
{
  CHECK_NO_ARGS(args);
  HINSTANCE hInst = ((dll_object *)self)->GetDll();
  if (hInst==NULL)
  	RETURN_ERR("There is no DLL attached to the object");
  CString csFileName;
  TCHAR *buf = csFileName.GetBuffer(MAX_PATH);
  ::GetModuleFileName(hInst, buf, MAX_PATH);
  csFileName.ReleaseBuffer();
  return PyWinObject_FromTCHAR(csFileName);
}

// @pymethod string|PyDLL|__repr__|Returns the HINSTANCE and filename of the DLL.
CString
dll_object::repr()
{
  HINSTANCE dll = GetDll();
  CString csRet;
  TCHAR *buf = csRet.GetBuffer(256);
  wsprintf (buf, _T(" HINSTANCE 0x%X, file = "), dll);
  csRet.ReleaseBuffer();

  CString csFileName;
  buf = csFileName.GetBuffer(MAX_PATH);
  if (dll) 
  	::GetModuleFileName(dll, buf, MAX_PATH); // @pyseeapi GetModuleFileName
  else
	_tcscpy(buf, _T("<None>"));
  csFileName.ReleaseBuffer();

  return ui_base_class::repr() + csRet + csFileName;
}

// @object PyDLL|A DLL object.  A general utility object, and not associated with an MFC object.
static struct PyMethodDef dll_methods[] =
{
  {"GetFileName",			dll_object_get_file_name,	1}, // @pymeth GetFileName|Returns the file name of the DLL associated with the object.
  {"AttachToMFC",           dll_object_attach_to_mfc,   1}, // @pymeth AttachToMFC|Attaches the DLL to the internal list of MFC DLL's.
  {NULL, 	NULL}
};

ui_type dll_object::type ("PyDLL", 
						  &ui_assoc_object::type, 
						  sizeof(dll_object), 
						  PYOBJ_OFFSET(dll_object), 
						  dll_methods, 
						  GET_PY_CTOR(dll_object));
