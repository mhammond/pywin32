/*
  python GDI class

	Access to a CGdiObject.

Note that this source file contains embedded documentation.
This documentation consists of marked up text inside the
C comments, and is prefixed with an '@' symbol.  The source
files are processed by a tool called "autoduck" which
generates Windows .hlp files.
@doc

*/

#include "stdafx.h"
#include "win32gdi.h"

bool PyCGdiObject::CheckCppObject(ui_type *ui_type_check) const
{
//	if (!ui_assoc_CObject::CheckCppObject(ui_type_check))
//		return false;
	CGdiObject *pGDI = (CGdiObject *)assoc;
	ASSERT_VALID(pGDI);
	if (!IsGdiHandleValid(pGDI->m_hObject))
    	RETURN_ERR("The associated object is invalid");
	return true;
}

// utility functions.
// according to MFC2 sources, these pointers are permanent.
CGdiObject *PyCGdiObject::GetGdiObject (PyObject *self, DWORD gtype)
{
	CGdiObject *pGdi = (CGdiObject *)GetGoodCppObject( self, &type);
	if (gtype && !IsWin32s() && pGdi->m_hObject && ::GetObjectType(pGdi->m_hObject) != gtype)
		RETURN_ERR("The associated GDI object is not of the required type");
	return pGdi;
}

PyCGdiObject::~PyCGdiObject()
{
}

// @object PyCGdiObject|A class which encapsulates an MFC CGdiObject.
static struct PyMethodDef ui_gdi_methods[] = {
	{NULL,			NULL}		// sentinel
};

ui_type_CObject PyCGdiObject::type("gdi object", 
								   &ui_assoc_CObject::type, 
								   RUNTIME_CLASS(CGdiObject), 
								   sizeof(PyCGdiObject), 
								   PYOBJ_OFFSET(PyCGdiObject), 
								   ui_gdi_methods, 
								   NULL);
