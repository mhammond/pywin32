// -*- Mode: C++; tab-width: 4 -*-
//
//  Python brush wrapper.
//
//  Created Dec 1995, by Sam Rushing (rushing@nightmare.com)
//
// Note that this source file contains embedded documentation.
// This documentation consists of marked up text inside the
// C comments, and is prefixed with an '@' symbol.  The source
// files are processed by a tool called "autoduck" which
// generates Windows .hlp files.
// @doc

#include "stdafx.h"
#include "win32gdi.h"
#include "win32brush.h"

// this returns a pointer that should not be stored.
CBrush *PyCBrush::GetBrush(PyObject *self) { return (CBrush *)GetGoodCppObject(self, &type); }

// @pymethod <o PyCBrush>|win32ui|GetHalftoneBrush|Creates a new halftone brush object.
PyObject *ui_get_halftone_brush(PyObject *self, PyObject *args)
{
    PyCBrush *pb = (PyCBrush *)ui_assoc_object::make(PyCBrush::type, CDC::GetHalftoneBrush(), true);
    pb->bManualDelete = FALSE;  // this is a temp object
    return pb;
}

// @pymethod <o PyCBrush>|win32ui|CreateBrush|Creates a new brush object.
PyObject *PyCBrush::create(PyObject *self, PyObject *args)
{
    int n_brush_style;
    int n_hatch;
    long cr_color;
    LOGBRUSH lp;
    // Quick exit to make a empty brush
    if (PyArg_ParseTuple(args, "")) {
        // @comm If called with no arguments, an uninitialized brush is created.
        PyCBrush *ret = (PyCBrush *)ui_assoc_object::make(PyCBrush::type, new CBrush);
        ret->bManualDelete = TRUE;
        return ret;
    }
    PyErr_Clear();
    if (!PyArg_ParseTuple(args, "iil",
                          &n_brush_style,  // @pyparmalt1 int|style||The brush style.
                          &cr_color,       // @pyparmalt1 int|color||The brush color.
                          &n_hatch)) {     // @pyparmalt1 long|hatch||The brush hatching.
        return NULL;
    }
    lp.lbStyle = n_brush_style;
    lp.lbColor = cr_color;
    lp.lbHatch = n_hatch;

    CBrush *pBrush = new CBrush;
    if (!pBrush->CreateBrushIndirect(&lp)) {
        RETURN_ERR("CreateBrushIndirect call failed");
    }
    PyCBrush *ret = (PyCBrush *)ui_assoc_object::make(PyCBrush::type, pBrush);
    ret->bManualDelete = TRUE;
    return ret;
}

// @pymethod |PyCBrush|CreateSolidBrush|Initializes a brush with a specified solid color.
static PyObject *PyCBrush_CreateSolidBrush(PyObject *self, PyObject *args)
{
    int color;
    if (!PyArg_ParseTuple(args, "i:CreateSolidBrush", &color))
        return NULL;
    CBrush *pBrush = PyCBrush::GetBrush(self);
    if (pBrush == NULL)
        return NULL;
    if (!pBrush->CreateSolidBrush(color))
        RETURN_ERR("CreateSolidBrush failed");
    RETURN_NONE;
}

// @pymethod int|PyCBrush|GetSafeHandle|Retrieves the HBRUSH for the brush as an integer
static PyObject *PyCBrush_GetSafeHandle(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS2(args, "GetSafeHandle");
    CBrush *pBrush = PyCBrush::GetBrush(self);
    if (pBrush == NULL)
        return NULL;
    return PyWinLong_FromHANDLE(pBrush->GetSafeHandle());
}

// @object PyCBrush|An object encapsulating an MFC PyCBrush class.
static struct PyMethodDef PyCBrush_methods[] = {
    {"CreateSolidBrush", PyCBrush_CreateSolidBrush,
     1},  // @pymeth CreateSolidBrush|Initializes a brush with a specified solid color.
    {"GetSafeHandle", PyCBrush_GetSafeHandle,
     1},  // @pymeth GetSafeHandle|Retrieves the HBRUSH for the brush as an integer
    {NULL, NULL}};

ui_type_CObject PyCBrush::type("PyCBrush", &PyCGdiObject::type, RUNTIME_CLASS(CBrush), sizeof(PyCBrush),
                               PYOBJ_OFFSET(PyCBrush), PyCBrush_methods, GET_PY_CTOR(PyCBrush));
