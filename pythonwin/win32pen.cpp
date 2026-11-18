// -*- Mode: C++; tab-width: 4 -*-
//
//  Python pen wrapper.
//
//  Created Dec 1995, by Sam Rushing (rushing@nightmare.com)
//
//
// Note that this source file contains embedded documentation.
// This documentation consists of marked up text inside the
// C comments, and is prefixed with an '@' symbol.  The source
// files are processed by a tool called "autoduck" which
// generates Windows .hlp files.
// @doc

#include "stdafx.h"
#include "win32gdi.h"
#include "win32pen.h"

// @pymethod <o PyCPen>|win32ui|CreatePen|Creates a <o PyCPen> object.
/*static*/ PyObject *ui_pen_object::create(PyObject *self, PyObject *args)
{
    int n_pen_style;
    int n_width;
    long cr_color;
    LOGPEN lp;

    if (!PyArg_ParseTuple(args, "iil",
                          &n_pen_style,  // @pyparm int|style||The pen style.
                          &n_width,      // @pyparm int|width||The pen width.
                          &cr_color)) {  // @pyparm long|color||The pen color.
        return NULL;
    }
    lp.lopnStyle = n_pen_style;
    lp.lopnWidth.x = n_width;
    lp.lopnWidth.y = 0;
    lp.lopnColor = cr_color;

    CPen *pPen = new CPen;
    if (!pPen->CreatePenIndirect(&lp)) {
        delete pPen;  // clean up on error.
        RETURN_ERR("CreatePenIndirect call failed");
    }
    return ui_assoc_object::make(ui_pen_object::type, pPen);
}

static struct PyMethodDef ui_pen_methods[] = {
    {NULL, NULL}  // sentinel
};

ui_type_CObject ui_pen_object::type("PyCPen", &PyCGdiObject::type, RUNTIME_CLASS(CPen), sizeof(ui_pen_object),
                                    PYOBJ_OFFSET(ui_pen_object), ui_pen_methods, GET_PY_CTOR(ui_pen_object));
