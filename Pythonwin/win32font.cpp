/*
  python font class

  Created September 1994, by Dave Brennan (brennan@hal.com)

Note that this source file contains embedded documentation.
This documentation consists of marked up text inside the
C comments, and is prefixed with an '@' symbol.  The source
files are processed by a tool called "autoduck" which
generates Windows .hlp files.
@doc

*/

#include "stdafx.h"
#include "win32gdi.h"
#include "win32font.h"
#include "win32dc.h"

// @pymethod <o PyCFont>|win32ui|CreateFont|Creates a <o PyCFont> object.
PyObject *
PyCFont::create (PyObject *self, PyObject *args)
{
  // @comm The code for the PyCFont was contributed by Dave Brennan
  // (Last known address is brennan@hal.com, but I hear he is now at Microsoft)
  // args contains a dict of font properties 
  PyObject *font_props; 
  PyObject *pydc = NULL; // @pyparm dict|properties||A dictionary containing the font
                  // properties.  Valid dictionary keys are:<nl> 
                  // height<nl>
                  // width<nl>
                  // escapement<nl>
                  // orientation<nl>
                  // weight<nl>
                  // italic<nl>
                  // underline<nl>
                  // strike out<nl>
                  // charset<nl>
                  // out precision<nl>
                  // clip precision<nl>
                  // quality<nl>
                  // pitch and family<nl>
                  // name
  if (!PyArg_ParseTuple (args, "O|O",
                 &font_props, &pydc) ||
      !PyDict_Check (font_props))
    {
      PyErr_Clear();
      RETURN_ERR ("Expected dictionary of font properties.");
    }
  // populate LOGFONT struct with values from dictionary
  LOGFONT lf;
  if (!DictToLogFont(font_props, &lf))
   return NULL;

  CDC *pDC = pydc ? ui_dc_object::GetDC (pydc) : NULL;

  CFont *pFont = new CFont;	// will except rather than fail!
  if (!pDC) {
    if (!pFont->CreateFontIndirect (&lf)) {
	  delete pFont;
      RETURN_ERR ("CreateFontIndirect call failed");
    }
  } else {
    if (!pFont->CreatePointFontIndirect (&lf, pDC)) {
	  delete pFont;
      RETURN_ERR ("CreatePointFontIndirect call failed");
    }
  }
  PyCFont *ret = (PyCFont *)ui_assoc_object::make (PyCFont::type, pFont, TRUE);
  if (ret)
    ret->bManualDelete = TRUE;
  return ret;
}

// @pymethod int|PyCFont|GetSafeHandle|Retrieves the HFONT for the font as an integer
static PyObject *PyCFont_GetSafeHandle(PyObject *self, PyObject *args)
{
  CHECK_NO_ARGS2(args, "GetSafeHandle");
  CFont *pFont = PyCFont::GetFont(self);
  if (pFont==NULL) return NULL;
  return PyWinLong_FromHANDLE(pFont->GetSafeHandle());
  return PyWinLong_FromHANDLE(pFont->GetSafeHandle());
}

// @object PyCFont|A windows font object.  Encapsulates an MFC <c CFont> class.
// Derived from a <o PyCGDIObject>. 
static struct PyMethodDef ui_font_methods[] =
{
 {"GetSafeHandle", PyCFont_GetSafeHandle, 1}, // @pymeth GetSafeHandle|Retrieves the HFONT for the font as an integer
 {NULL,			NULL}		// sentinel
};

ui_type_CObject PyCFont::type("PyCFont", 
         &PyCGdiObject::type, 
         RUNTIME_CLASS(CFont), 
         sizeof(PyCFont), 
         PYOBJ_OFFSET(PyCFont), 
         ui_font_methods, 
         GET_PY_CTOR(PyCFont));

