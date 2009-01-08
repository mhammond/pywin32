// Contributed by:
// kk@epsilon.com.gr
//
//
// Note that this source file contains embedded documentation.
// This documentation consists of marked up text inside the
// C comments, and is prefixed with an '@' symbol.  The source
// files are processed by a tool called "autoduck" which
// generates Windows .hlp files.
// @doc

// Purpose: It exports to Python the MFC GDI class CRgn.

#include "stdafx.h"
#include "win32gdi.h"
#include "win32rgn.h"

// this returns a pointer that should not be stored.
// Helper function that returns a CRgn object given a Python object
// Return Values: a CRgn object
CRgn *PyCRgn::GetRgn(PyObject *self)
{
	return (CRgn *)GetGoodCppObject( self, &type);
}

// @pymethod <o PyCRgn>|win32ui|CreateRgn|Creates a new rgn object.
// Return Values: a PyCRgn object
PyObject *
PyCRgn::create(PyObject *self, PyObject *args)
{
	CHECK_NO_ARGS(args);
	CRgn *pRgn = new CRgn;
	return ui_assoc_object::make (PyCRgn::type, pRgn);
}

// @pymethod int|PyCRgn|CreateRectRgn|Initializes a region to a rectangle
// Return Values: success or failure flag (BOOL)
PyObject *
PyCRgn::create_rect_rgn(PyObject *self, PyObject *args)
	{
	CRgn *pRgn = PyCRgn::GetRgn(self);
	if (!pRgn) return NULL;

	int x1,y1,x2,y2;
	if (!PyArg_ParseTuple(args,"(iiii):CreateRectRgn",
		&x1,&y1, &x2,&y2))
		return NULL;

	BOOL ok=pRgn->CreateRectRgn(x1,y1,x2,y2);

	return Py_BuildValue("i",ok);
	}

// @pymethod int|PyCRgn|CreateEllipticRgn|Initializes a region to an ellipse
// Return Values: success or failure flag (BOOL)
PyObject *
PyCRgn::create_elliptic_rgn(PyObject *self, PyObject *args)
      {
      CRgn *pRgn = PyCRgn::GetRgn(self);
      if (!pRgn) return NULL;

      int x1,y1,x2,y2;
      if (!PyArg_ParseTuple(args,"(iiii):CreateEllipticRgn",
              &x1,&y1, &x2,&y2))
              return NULL;

      BOOL ok=pRgn->CreateEllipticRgn(x1,y1,x2,y2);

      return Py_BuildValue("i",ok);
      }

// @pymethod int|PyCRgn|CombineRgn|Creates a new GDI region by combining two existing regions. The regions are combined as specified by nCombineMode
// Return Values: success or failure flag (BOOL)
PyObject *
PyCRgn::combine_rgn(PyObject *self, PyObject *args)
	{
	CRgn *pRgn = PyCRgn::GetRgn(self);
	if (!pRgn) return NULL;

	PyObject *objRgn1 = Py_None;
	PyObject *objRgn2 = Py_None;
	int nCombineMode=RGN_AND;

	if (!PyArg_ParseTuple(args,"OOi:CombineRgn",
			&objRgn1,&objRgn2,&nCombineMode))
		return NULL;

	CRgn *pRgn1 = PyCRgn::GetRgn(objRgn1);
	if (!pRgn1) return NULL;
	CRgn *pRgn2 = PyCRgn::GetRgn(objRgn2);
	if (!pRgn2) return NULL;

	int result=pRgn->CombineRgn(pRgn1,pRgn2,nCombineMode);

	return Py_BuildValue("i",result);
	}

// @pymethod int|PyCRgn|CopyRgn|Copies the region defined by pRgnSrc into the CRgn object
// Return Values: success or failure flag (BOOL)
PyObject *
PyCRgn::copy_rgn(PyObject *self, PyObject *args)
	{
	CRgn *pRgn = PyCRgn::GetRgn(self);
	if (!pRgn) return NULL;

	PyObject *objRgnSrc = Py_None;
	if (!PyArg_ParseTuple(args,"O:CopyRgn",&objRgnSrc))
		return NULL;

	CRgn *pRgnSrc = PyCRgn::GetRgn(objRgnSrc);
	if (!pRgnSrc) return NULL;

	int result=pRgn->CopyRgn(pRgnSrc);

	return Py_BuildValue("i",result);
	}

// @pymethod int|PyCRgn|GetRgnBox|Retrieves the coordinates of the bounding rectangle of the CRgn object
// Return Values: the bounding rectangle as a tuple (l,t,r,b)
PyObject *
PyCRgn::get_rgn_box(PyObject *self, PyObject *args)
	{
	CRgn *pRgn = PyCRgn::GetRgn(self);
	if (!pRgn) return NULL;

	CHECK_NO_ARGS2(args,GetRgnBox);

	RECT rect = {0,0,0,0};
	int result=pRgn->GetRgnBox(&rect);

	return Py_BuildValue("i(iiii)", result, rect.left, rect.top, rect.right, rect.bottom);
	}

// @pymethod int|PyCRgn|DeleteObject|Deletes the attached Windows GDI Rgn object from memory by freeing all system storage associated with the Windows GDI object
// Return Values: None
PyObject *
PyCRgn::delete_object(PyObject *self, PyObject *args)
	{
	CRgn *pRgn = PyCRgn::GetRgn(self);
	if (!pRgn) return NULL;

	CHECK_NO_ARGS2(args,DeleteObject);
	
	BOOL ok=TRUE;
	if(pRgn->GetSafeHandle())
		ok=pRgn->DeleteObject();
	if(!ok)
		RETURN_ERR("DeleteObject failed");
	pRgn->m_hObject=0; // assert 

	RETURN_NONE;
	}

// @pymethod int|PyCRgn|GetSafeHandle|A HANDLE to the attached Windows GDI object; otherwise NULL if no object is attached
// Return Values: the handle of the CRgn object
PyObject *
PyCRgn::get_safe_handle(PyObject *self, PyObject *args)
	{
	CRgn *pRgn = PyCRgn::GetRgn(self);
	if (!pRgn) return NULL;
	CHECK_NO_ARGS2(args,GetSafeHandle);
	HGDIOBJ hgdiobj=pRgn->GetSafeHandle();
	return Py_BuildValue("l",hgdiobj);
	}


// @object PyCRgn|An object encapsulating an MFC PyCRgn class.
static struct PyMethodDef PyCRgn_methods[] = {
	{"CreateRectRgn",PyCRgn::create_rect_rgn,1},
	{"CombineRgn",PyCRgn::combine_rgn,1},
	{"CopyRgn",PyCRgn::copy_rgn,1},
	{"GetRgnBox",PyCRgn::get_rgn_box,1},
	{"DeleteObject",PyCRgn::delete_object,1},
	{"GetSafeHandle",PyCRgn::get_safe_handle,1},
	{NULL,NULL}
};

ui_type_CObject PyCRgn::type ("PyCRgn",
							 &PyCGdiObject::type,
							 RUNTIME_CLASS(CRgn),
							 sizeof(PyCRgn),
							 PYOBJ_OFFSET(PyCRgn),
							 PyCRgn_methods,
							 GET_PY_CTOR(PyCRgn));
