/*

	win32 module utilities

	Created January 1996, Mark Hammond (MHammond@skippinet.com.au)

Note that this source file contains embedded documentation.
This documentation consists of marked up text inside the
C comments, and is prefixed with an '@' symbol.  The source
files are processed by a tool called "autoduck" which
generates Windows .hlp files.
@doc

*/
#include "stdafx.h"

#include "win32win.h"
#include "win32doc.h"
#include "win32control.h"
#include "win32menu.h"
#include "win32dlg.h"
#include "win32dc.h"
#include "win32gdi.h"
#include "win32bitmap.h"
#include "win32font.h"
#include "win32dll.h"
#include "win32splitter.h"
#include "win32toolbar.h"
#include "win32prop.h"
#include "win32template.h"
#include "win32ctrlList.h"
#include "win32ctrlTree.h"
#include "win32RichEdit.h"
#ifdef HIER_LIST
#include "win32hl.h"
#endif

class PyCRectType : public ui_type {
public:
	PyCRectType( const char *name, ui_type *pBaseType, int typeSize, struct PyMethodDef* methodList, ui_base_class * (* thector)() );
};
// @object PyCRect|A Python interface the the MFC CRect class.
class PyCRect : public ui_base_class
{
public:
	static PyCRectType type;
	PyCRect(RECT *pRect, bool bTakeCopy) {
		m_owned = bTakeCopy;
		if (m_owned)
			m_pRect = new CRect(*pRect);
		else
			m_pRect = pRect;
		ob_type = &type;
		_Py_NewReference(this);
	}
	PyCRect(const RECT &rect) {
		m_owned = true;
		m_pRect = new CRect(rect);
		ob_type = &type;
		_Py_NewReference(this);
	}
	~PyCRect() {if (m_owned) delete m_pRect;}
	virtual PyObject *getattr(char *name);
	virtual int setattr(char *name, PyObject *v);
	static PyObject *getitem(PyObject *self, int index);
	static int getlength(PyObject *self);
	CString repr();
protected:
	bool m_owned;
	RECT *m_pRect;
};

PyObject *PyWinObject_FromRECT(RECT *p, bool bTakeCopy)
{
	return new PyCRect(p, bTakeCopy);
}
PyObject *PyWinObject_FromRECT(const RECT &r)
{
	return new PyCRect(r);
}
// Sequence stuff to provide compatibility with tuples.
static PySequenceMethods PyCRect_Sequence = 
{
	PyCRect::getlength, // sq_length;
	NULL, // sq_concat;
	NULL, // sq_repeat;
	PyCRect::getitem, // sq_item;
	NULL, // sq_slice;
	NULL, // sq_ass_item;
	NULL, // sq_ass_slice;
};

PyObject *PyCRect::getattr(char *name)
{
	if (strcmp(name, "left")==0)
		return PyInt_FromLong(m_pRect->left);
	if (strcmp(name, "right")==0)
		return PyInt_FromLong(m_pRect->right);
	if (strcmp(name, "top")==0)
		return PyInt_FromLong(m_pRect->top);
	if (strcmp(name, "bottom")==0)
		return PyInt_FromLong(m_pRect->bottom);
	return ui_base_class::getattr(name);
}

CString PyCRect::repr()
{
	CString csRet;
	csRet.Format("%s (%d, %d, %d, %d)", ui_base_class::repr(), m_pRect->left, m_pRect->top, m_pRect->right, m_pRect->bottom );
	return csRet;
}

/* static */ int PyCRect::getlength(PyObject *self)
{
	// NEVER CHANGE THIS - you will break all the old
	// code written when these object were tuples!
	return 4;
}

/* static */ PyObject *PyCRect::getitem(PyObject *self, int index)
{
	PyCRect *p = (PyCRect *)self;
	switch (index) {
		case 0: // @tupleitem 0|int|left|
			return PyInt_FromLong(p->m_pRect->left);
		case 1: // @tupleitem 1|int|top|
			return PyInt_FromLong(p->m_pRect->top);
		case 2: // @tupleitem 2|int|right|
			return PyInt_FromLong(p->m_pRect->right);
		case 3: // @tupleitem 3|int|bottom|
			return PyInt_FromLong(p->m_pRect->bottom);
	}
	PyErr_SetString(PyExc_IndexError, "index out of range");
	return NULL;
}

int PyCRect::setattr(char *name, PyObject *v)
{
	if (!PyInt_Check(v))
		RETURN_TYPE_ERR("PyCRect objects only have integer properties");
	if (strcmp(name, "left")==0) {
		m_pRect->left = PyInt_AsLong(v);
		return 0;
	}
	if (strcmp(name, "right")==0) {
		m_pRect->right = PyInt_AsLong(v);
		return 0;
	}
	if (strcmp(name, "top")==0) {
		m_pRect->top = PyInt_AsLong(v);
		return 0;
	}
	if (strcmp(name, "bottom")==0) {
		m_pRect->bottom = PyInt_AsLong(v);
		return 0;
	}
	return ui_base_class::setattr(name, v);
}

static struct PyMethodDef 
PyCRect_methods[] =
{
	{ NULL,			NULL }
};

PyCRectType::PyCRectType( const char *name, ui_type *pBaseType, int typeSize, struct PyMethodDef* methodList, ui_base_class * (* thector)() ) :
	  ui_type(name, pBaseType, typeSize, methodList, thector)
{
		  tp_as_sequence = &PyCRect_Sequence;
}

PyCRectType PyCRect::type("PyCRect",
							 &ui_base_class::type,
							 sizeof(PyCRect),
							 PyCRect_methods,
							 NULL);

// The CREATESTRUCT just has pointers (no buffers) for the name
// and classname.  Therefore, I dont treat them as strings, just 
// pointers (via long casts)
// @object CREATESTRUCT|A representation of a Windows CREATESTRUCT structure.
PyObject *PyObjectFromCreateStruct(LPCREATESTRUCT lpcs)
{
	return Py_BuildValue("(iiii(iiii)illi)",
	    lpcs->lpCreateParams, // @pyparm int|createParams||
   		lpcs->hInstance, // @pyparm int|hInstance||
   		lpcs->hMenu, // @pyparm int|hMenu||
   		lpcs->hwndParent, // @pyparm int|hwndParent||
   		lpcs->cy, // @pyparm (int, int, int, int)|cy, cx, y, x||
   		lpcs->cx, 
   		lpcs->y,
   		lpcs->x,
   		lpcs->style,// @pyparm int|style||
   		(long)lpcs->lpszName,// @pyparm int|lpszName||A string cast to a long.
   		(long)lpcs->lpszClass,// @pyparm int|lpszClass||A string cast to a long!?
   		lpcs->dwExStyle);// @pyparm int|dwExStyle||

	// @comm Note that the strings are passed as longs, which are there address
	// in memory.  This is due to the internal mechanics of passing this structure around.
}

BOOL CreateStructFromPyObject(LPCREATESTRUCT lpcs, PyObject *ob, const char *fnName, BOOL bFromTuple)
{
	char argBuf[80];
	if (fnName==NULL) fnName = "CREATESTRUCT value";
	if (bFromTuple)
		sprintf(argBuf, "(iiii(iiii)illi):%s", fnName);
	else
		sprintf(argBuf, "iiii(iiii)illi:%s", fnName);
	long name, className;
	BOOL ret =  PyArg_ParseTuple(ob, argBuf,
	    &lpcs->lpCreateParams,
   		&lpcs->hInstance, 
   		&lpcs->hMenu,
   		&lpcs->hwndParent,
   		&lpcs->cy,
   		&lpcs->cx,
   		&lpcs->y,
   		&lpcs->x,
   		&lpcs->style,
   		&name,
   		&className,
   		&lpcs->dwExStyle);
	// CCreateStruct
	lpcs->lpszName = (LPCTSTR)name;
	lpcs->lpszClass = (LPCTSTR)className;
	return ret;
}
/////////////////////////////////////////////////////////////////////
//
// Font conversion utilities
//
//
static const char *szFontName = "name";
static const char *szFontWeight = "weight";
static const char *szFontWidth = "width";
static const char *szFontHeight = "height";
static const char *szFontItalic = "italic";
static const char *szFontUnderline = "underline";
static const char *szFontPitch = "pitch and family";
static const char *szFontCharSet = "charset";

PyObject *LogFontToDict(const LOGFONT &lf)
{
	PyObject *ret = PyDict_New();
	PyMapping_SetItemString( ret, (char *)szFontName, PyString_FromString((char *)lf.lfFaceName) );
	PyMapping_SetItemString( ret, (char *)szFontHeight, PyInt_FromLong(-lf.lfHeight));
	PyMapping_SetItemString( ret, (char *)szFontWidth, PyInt_FromLong(lf.lfWidth));
	PyMapping_SetItemString( ret, (char *)szFontWeight, PyInt_FromLong(lf.lfWeight));
	PyMapping_SetItemString( ret, (char *)szFontPitch, PyInt_FromLong(lf.lfPitchAndFamily));
	PyMapping_SetItemString( ret, (char *)szFontCharSet, PyInt_FromLong(lf.lfCharSet));
	PyMapping_SetItemString( ret, (char *)szFontUnderline, lf.lfUnderline?PyInt_FromLong(1):Py_None);
	PyMapping_SetItemString( ret, (char *)szFontItalic, lf.lfItalic?PyInt_FromLong(1):Py_None);
	return ret;
}

BOOL DictToLogFont(PyObject *font_props, LOGFONT *pLF)
{
  ZeroMemory (pLF, sizeof(LOGFONT));

  // font default values
  pLF->lfCharSet = DEFAULT_CHARSET; // dont use ANSI_CHARSET to support Japanese charset.
  pLF->lfQuality = PROOF_QUALITY;  // don't scale raster fonts
  PyObject *v;


  v = PyDict_GetItemString (font_props, (char *)szFontName);
  if (v != NULL)
	if (PyString_Check(v))
	  strncpy (pLF->lfFaceName, PyString_AsString(v), LF_FACESIZE - 1);
	else
	  RETURN_ERR ("Expected string value for font name property");

  v = PyDict_GetItemString (font_props, (char *)szFontHeight);
  if (v != NULL)
	if (PyInt_Check (v))
	  pLF->lfHeight = -PyInt_AsLong(v);
	else
	  RETURN_ERR ("Expected integer value for font height property");

  v = PyDict_GetItemString (font_props, (char *)szFontWidth);
  if (v != NULL)
	if (PyInt_Check (v))
	  pLF->lfWidth = PyInt_AsLong(v);
	else
	  RETURN_ERR ("Expected integer value for font width property");

  v = PyDict_GetItemString (font_props, (char *)szFontPitch);
  if (v != NULL)
	if (PyInt_Check (v))
	  pLF->lfPitchAndFamily = (BYTE)PyInt_AsLong(v);
	else
	  RETURN_ERR ("Expected integer value for font 'pitch and family' property");

  v = PyDict_GetItemString (font_props, (char *)szFontCharSet);
  if (v != NULL)
	if (PyInt_Check (v))
	  pLF->lfCharSet = (BYTE)PyInt_AsLong(v);
	else
	  RETURN_ERR ("Expected integer value for font 'charset' property");

  v = PyDict_GetItemString (font_props, (char *)szFontWeight);
  if (v != NULL)
	if (PyInt_Check (v))
	  pLF->lfWeight = PyInt_AsLong(v);
	else
	  RETURN_ERR ("Expected integer value for font weight property");

  v = PyDict_GetItemString (font_props, (char *)szFontItalic);
  if (v != NULL && v != Py_None)
	pLF->lfItalic = TRUE;

  v = PyDict_GetItemString (font_props, (char *)szFontUnderline);
  if (v != NULL && v != Py_None)
	pLF->lfUnderline = TRUE;
  return TRUE;
}

/////////////////////////////////////////////////////////////////////
//
//  ListView conversion utilities
//
//
// LV_ITEM 
PyObject *MakeLV_ITEMTuple(LV_ITEM *item)
{
	PyObject *ret = PyTuple_New(7);
	if (ret==NULL) return NULL;
	PyTuple_SET_ITEM(ret, 0, PyInt_FromLong(item->iItem));
	PyTuple_SET_ITEM(ret, 1, PyInt_FromLong(item->iSubItem));
	if (item->mask & LVIF_STATE) {
		PyTuple_SET_ITEM(ret, 2, PyInt_FromLong(item->state));
		PyTuple_SET_ITEM(ret, 3, PyInt_FromLong(item->stateMask));
	} else {
		Py_INCREF(Py_None);
		Py_INCREF(Py_None);
		PyTuple_SET_ITEM(ret, 2, Py_None);
		PyTuple_SET_ITEM(ret, 3, Py_None);
	}
	if ((item->mask & LVIF_TEXT) && (item->pszText != NULL)) {
		PyTuple_SET_ITEM(ret, 4, PyString_FromString(item->pszText));
	} else {
		Py_INCREF(Py_None);
		PyTuple_SET_ITEM(ret, 4, Py_None);
	}
	if (item->mask & LVIF_IMAGE) {
		PyTuple_SET_ITEM(ret, 5, PyInt_FromLong(item->iImage));
	} else {
		Py_INCREF(Py_None);
		PyTuple_SET_ITEM(ret, 5, Py_None);
	}
	if (item->mask & LVIF_PARAM && item->lParam) {
		PyObject *ob = PyInt_FromLong(item->lParam);
		PyTuple_SET_ITEM(ret, 6, ob);
	} else {
		Py_INCREF(Py_None);
		PyTuple_SET_ITEM(ret, 6, Py_None);
	}
	return ret;
}

// @object LV_ITEM|Describes an LV_ITEM tuple, used by the <o PyCListCtrl> object.
// @tupleitem 0|int|item|The item number.
// @tupleitem 1|int|subItem|The sub-item number.
// @tupleitem 2|int|state|The items state.
// @tupleitem 3|int|stateMask|A mask indicating which of the state bits are valid..
// @tupleitem 4|string|text|The text for the item
// @tupleitem 5|int|iImage|The image offset for the item
// @tupleitem 6|int|userObject|Any integer to be associated with the item.
// @comm When passed to Python, will always be a tuple of size 7, and items may be None if not available.
// <nl>When passed from Python, the tuple must be at least 2 items long, and any item may be None.
// <nl>userob is any Python object at all, but no reference count is kept, so you must ensure the object remains referenced throught the lists life.
BOOL ParseLV_ITEMTuple( PyObject *args, LV_ITEM *pItem)
{
	PyObject *ob;
	pItem->mask = 0;
	int len = PyTuple_Size(args);
	if (len<2 || len > 7) {
		PyErr_SetString(PyExc_TypeError, "LV_ITEM tuple has invalid size");
		return FALSE;
	}
	PyErr_Clear(); // clear any errors, so I can detect my own.
	// 0 - iItem.
	if ((ob=PyTuple_GetItem(args, 0))==NULL)
		return FALSE;
	pItem->iItem = (UINT)PyInt_AsLong(ob);
	if (PyErr_Occurred()) return FALSE;
	// 1 - iSubItem
	if ((ob=PyTuple_GetItem(args, 1))==NULL)
		return FALSE;
	pItem->iSubItem = (UINT)PyInt_AsLong(ob);
	if (PyErr_Occurred()) return FALSE;
	// 1/2 - state/stateMask
	if (len<4) return TRUE;
	if ((ob=PyTuple_GetItem(args, 2))==NULL)
		return FALSE;
	if (ob != Py_None) {
		pItem->state = (UINT)PyInt_AsLong(ob);
		if (PyErr_Occurred() || (ob=PyTuple_GetItem(args, 3))==NULL)
			return FALSE;
		pItem->stateMask = (UINT)PyInt_AsLong(ob);
		if (PyErr_Occurred()) return FALSE;
		pItem->mask |= LVIF_STATE;
	}
	if (len<5) return TRUE;
	if ((ob=PyTuple_GetItem(args, 4))==NULL)
		return FALSE;
	if (ob != Py_None) {
		if (!PyString_Check(ob)) RETURN_TYPE_ERR("The text item must be a string or None");
		pItem->mask |= LVIF_TEXT;
		pItem->pszText = PyString_AsString(ob);
		pItem->cchTextMax = strlen(pItem->pszText)+1;
	}
	if (len<6) return TRUE;
	if ((ob=PyTuple_GetItem(args, 5))==NULL)
		return FALSE;
	if (ob != Py_None) {
		pItem->mask |= LVIF_IMAGE;
		pItem->iImage = PyInt_AsLong(ob);
		if (PyErr_Occurred())
			return FALSE;
	}
	if (len<7) return TRUE;
	if ((ob=PyTuple_GetItem(args, 6))==NULL)
		return FALSE;
	if (ob != Py_None) {
		pItem->mask |= LVIF_PARAM;
		pItem->lParam = PyInt_AsLong(ob);
	}
	return TRUE;
}

//
// LV_COLUMN
PyObject *MakeLV_COLUMNTuple(LV_COLUMN *item)
{
	PyObject *ret = PyTuple_New(4);
	if (ret==NULL) return NULL;
	if (item->mask & LVCF_FMT) {
		PyTuple_SET_ITEM(ret, 0, PyInt_FromLong(item->fmt));
	} else {
		Py_INCREF(Py_None);
		PyTuple_SET_ITEM(ret, 0, Py_None);
	}
	if (item->mask & LVCF_WIDTH) {
		PyTuple_SET_ITEM(ret, 1, PyInt_FromLong(item->cx));
	} else {
		Py_INCREF(Py_None);
		PyTuple_SET_ITEM(ret, 1, Py_None);
	}
	if ((item->mask & LVCF_TEXT) && (item->pszText != NULL)) {
		PyTuple_SET_ITEM(ret, 2, PyString_FromString(item->pszText));
	} else {
		Py_INCREF(Py_None);
		PyTuple_SET_ITEM(ret, 2, Py_None);
	}
	if (item->mask & LVCF_SUBITEM) {
		PyTuple_SET_ITEM(ret, 3, PyInt_FromLong(item->iSubItem));
	} else {
		Py_INCREF(Py_None);
		PyTuple_SET_ITEM(ret, 3, Py_None);
	}
	return ret;
}
// @object LV_COLUMN|A tuple that describes a Win32 LV_COLUMN tuple. Used by the <o PyCListCtrl> object.
// A tuple of 4 items, being fmt, cx, pszText, iSubItem
// @tupleitem 0|int|fmt|Alignment of the column header and the subitem text in the column.
// @tupleitem 1|int|cx|Width of the column.
// @tupleitem 2|string|text|Column header text.
// @tupleitem 3|int|subItem|Index of subitem associated with the column. 
// <nl>When passed to Python, will always be a tuple of size 4, and items may be None if not available.
// <nl>When passed from Python, the tuple may be any length up to 4, and any item may be None.
BOOL ParseLV_COLUMNTuple( PyObject *args, LV_COLUMN *pItem)
{
	PyObject *ob;
	pItem->mask = 0;
	int len = PyTuple_Size(args);
	if (len > 4) {
		PyErr_SetString(PyExc_TypeError, "LV_COLUMN tuple has invalid size");
		return FALSE;
	}
	PyErr_Clear(); // clear any errors, so I can detect my own.
	// 0 - fmt
	if (len<1) return TRUE;
	if ((ob=PyTuple_GetItem(args, 0))==NULL)
		return FALSE;
	if (ob != Py_None) {
		pItem->fmt = (int)PyInt_AsLong(ob);
		if (PyErr_Occurred()) return FALSE;
		pItem->mask |= LVCF_FMT;
	}
	// 1 - cx
	if (len<2) return TRUE;
	if ((ob=PyTuple_GetItem(args, 1))==NULL)
		return FALSE;
	if (ob != Py_None) {
		pItem->cx = (int)PyInt_AsLong(ob);
		if (PyErr_Occurred()) return FALSE;
		pItem->mask |= LVCF_WIDTH;
	}
	// 2 - text
	if (len<3) return TRUE;
	if ((ob=PyTuple_GetItem(args, 2))==NULL)
		return FALSE;
	if (ob != Py_None) {
		if (!PyString_Check(ob)) RETURN_TYPE_ERR("The text item must be a string or None");
		pItem->mask |= LVCF_TEXT;
		pItem->pszText = PyString_AsString(ob);
		pItem->cchTextMax = strlen(pItem->pszText)+1;
	}
	// 3 - subitem
	if (len<4) return TRUE;
	if ((ob=PyTuple_GetItem(args, 3))==NULL)
		return FALSE;
	if (ob != Py_None) {
		pItem->mask |= LVCF_SUBITEM;
		pItem->iSubItem = PyInt_AsLong(ob);
		if (PyErr_Occurred())
			return FALSE;
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////
//
//  TreeView conversion utilities
//
//
// TV_ITEM 
PyObject *MakeTV_ITEMTuple(TV_ITEM *item)
{
	PyObject *ret = PyTuple_New(8);
	if (ret==NULL) return NULL;
	if (item->mask & TVIF_HANDLE)
		PyTuple_SET_ITEM(ret, 0, PyInt_FromLong((long)item->hItem));
	else {
		Py_INCREF(Py_None);
		PyTuple_SET_ITEM(ret, 0, Py_None);
	}
	if (item->mask & TVIF_STATE) {
		PyTuple_SET_ITEM(ret, 1, PyInt_FromLong(item->state));
		PyTuple_SET_ITEM(ret, 2, PyInt_FromLong(item->stateMask));
	} else {
		Py_INCREF(Py_None);
		Py_INCREF(Py_None);
		PyTuple_SET_ITEM(ret, 1, Py_None);
		PyTuple_SET_ITEM(ret, 2, Py_None);
	}
	if ((item->mask & TVIF_TEXT) && (item->pszText != NULL)) {
		PyTuple_SET_ITEM(ret, 3, PyString_FromString(item->pszText));
	} else {
		Py_INCREF(Py_None);
		PyTuple_SET_ITEM(ret, 3, Py_None);
	}
	if (item->mask & TVIF_IMAGE) {
		PyTuple_SET_ITEM(ret, 4, PyInt_FromLong(item->iImage));
	} else {
		Py_INCREF(Py_None);
		PyTuple_SET_ITEM(ret, 4, Py_None);
	}
	if (item->mask & TVIF_SELECTEDIMAGE) {
		PyTuple_SET_ITEM(ret, 5, PyInt_FromLong(item->iSelectedImage));
	} else {
		Py_INCREF(Py_None);
		PyTuple_SET_ITEM(ret, 5, Py_None);
	}

	if (item->mask & TVIF_CHILDREN) {
		PyTuple_SET_ITEM(ret, 6, PyInt_FromLong(item->cChildren));
	} else {
		Py_INCREF(Py_None);
		PyTuple_SET_ITEM(ret, 6, Py_None);
	}
	if (item->mask & TVIF_PARAM && item->lParam) {
		// assume lParam is an object
		PyTuple_SET_ITEM(ret, 7, PyInt_FromLong(item->lParam));
	} else {
		Py_INCREF(Py_None);
		PyTuple_SET_ITEM(ret, 7, Py_None);
	}
	return ret;
}

// @object TV_ITEM|Describes a TV_ITEM tuple, used by the <o PyCListCtrl> object.
// A tuple of 8 items:
// <nl>When passed to Python, will always be a tuple of size 8, and items may be None if not available.
// <nl>When passed from Python, the tuple may be any length up to 8, and any item may be None.
BOOL ParseTV_ITEMTuple( PyObject *args, TV_ITEM *pItem)
{
	PyObject *ob;
	PyObject *ob2;
	pItem->mask = 0;
	int len = PyTuple_Size(args);
	if (len > 8) {
		PyErr_SetString(PyExc_TypeError, "TV_ITEM tuple has invalid size");
		return FALSE;
	}
	PyErr_Clear(); // clear any errors, so I can detect my own.
	// 0 - hItem
	if (len<1) return TRUE;
	if ((ob=PyTuple_GetItem(args, 0))==NULL)
		return FALSE;
	if (ob != Py_None) {
		// @tupleitem 0|int|hItem|Item handle
		pItem->hItem = (HTREEITEM)PyInt_AsLong(ob);
		if (PyErr_Occurred()) return FALSE;
		pItem->mask |= TVIF_HANDLE;
	}
	// 1,2 - state/stateMask
	if (len<2) return TRUE;
	if (len<3) {
		PyErr_SetString(PyExc_TypeError, "TV_ITEM - state and stateMask must be provided");
		return FALSE;
	}
	if ((ob=PyTuple_GetItem(args, 1))==NULL)
		return FALSE;
	if ((ob2=PyTuple_GetItem(args, 2))==NULL)
		return FALSE;
	if (ob==Py_None && ob2==Py_None)
		;
	else if (ob==Py_None) {
		PyErr_SetString(PyExc_TypeError, "TV_ITEM - state and stateMask must both be None, or both not Noned");
		return FALSE;
	} else {
		// @tupleitem 1|int|state|Item state
		// @tupleitem 2|int|stateMask|Item state mask
		pItem->state = (int)PyInt_AsLong(ob);
		pItem->stateMask = (int)PyInt_AsLong(ob2);
		if (PyErr_Occurred()) return FALSE;
		pItem->mask |= TVIF_STATE;
	}
	// 3 - text
	if (len<4) return TRUE;
	if ((ob=PyTuple_GetItem(args, 3))==NULL)
		return FALSE;
	if (ob != Py_None) {
		// @tupleitem 3|string|text|Item text
		if (!PyString_Check(ob)) RETURN_TYPE_ERR("The text item must be a string or None");
		pItem->mask |= TVIF_TEXT;
		pItem->pszText = PyString_AsString(ob);
		pItem->cchTextMax = strlen(pItem->pszText)+1;
	}
	// 4 - image
	if (len<5) return TRUE;
	if ((ob=PyTuple_GetItem(args, 4))==NULL)
		return FALSE;
	if (ob != Py_None) {
		pItem->mask |= TVIF_IMAGE;
		// @tupleitem 4|int|iImage|Offset of items image.
		pItem->iImage = (int)PyInt_AsLong(ob);
	}
	// 5 - imageSelected
	if (len<6) return TRUE;
	if ((ob=PyTuple_GetItem(args, 5))==NULL)
		return FALSE;
	if (ob != Py_None) {
		// @tupleitem 5|int|iSelectedImage|Offset of items selected image.
		pItem->mask |= TVIF_SELECTEDIMAGE;
		pItem->iSelectedImage = (int)PyInt_AsLong(ob);
	}
	// 6 - cChildren
	if (len<7) return TRUE;
	if ((ob=PyTuple_GetItem(args, 6))==NULL)
		return FALSE;
	if (ob != Py_None) {
		// @tupleitem 6|int|cChildren|Number of child items.
		pItem->mask |= TVIF_CHILDREN;
		pItem->cChildren = (int)PyInt_AsLong(ob);
	}
	// 7 - object
	if (len<8) return TRUE;
	if ((ob=PyTuple_GetItem(args, 7))==NULL)
		return FALSE;
	if (ob != Py_None) {
		// @tupleitem 7|int|lParam|User defined integer param.
		pItem->mask |= LVIF_PARAM;
		pItem->lParam = PyInt_AsLong(ob);
	}
	return !PyErr_Occurred();
}

/////////////////////////////////////////////////////////////////////
//
//  Header Control conversion utilities
//
//
// HD_ITEM 
//HDI_BITMAP, HDI_FORMAT, HDI_HEIGHT, HDI_LPARAM, HDI_TEXT, HDI_WIDTH
//fmt is HDF_CENTER, HDF_LEFT, HDF_RIGHT, HDF_BITMAP, HDF_OWNERDRAW, HDF_STRING
PyObject *MakeHD_ITEMTuple(HD_ITEM *item)
{
	PyObject *ret = PyTuple_New(5);
	if (ret==NULL) return NULL;
	if (item->mask & HDI_HEIGHT)
		PyTuple_SET_ITEM(ret, 0, PyInt_FromLong((long)0));
	else if (item->mask & HDI_WIDTH)
		PyTuple_SET_ITEM(ret, 0, PyInt_FromLong((long)1));
	if ((item->mask & HDI_HEIGHT) || (item->mask & HDI_WIDTH)) 
		PyTuple_SET_ITEM(ret, 1, PyInt_FromLong((long)item->cxy));
	else {
		Py_INCREF(Py_None);
		Py_INCREF(Py_None);
		PyTuple_SET_ITEM(ret, 0, Py_None);
		PyTuple_SET_ITEM(ret, 1, Py_None);
	}
	if ((item->mask & HDI_TEXT) && (item->pszText != NULL) ) {
		PyTuple_SET_ITEM(ret, 2, PyString_FromString(item->pszText));
	} else {
		Py_INCREF(Py_None);
		PyTuple_SET_ITEM(ret, 2, Py_None);
	}
	if (item->mask & HDI_BITMAP) {
		// Should this support a bitmap object?
		PyTuple_SET_ITEM(ret, 3, PyInt_FromLong((long)item->hbm));
	} else {
		Py_INCREF(Py_None);
		PyTuple_SET_ITEM(ret, 3, Py_None);
	}
	if (item->mask & HDI_FORMAT) {
		PyTuple_SET_ITEM(ret, 4, PyInt_FromLong(item->fmt));
	} else {
		Py_INCREF(Py_None);
		PyTuple_SET_ITEM(ret, 4, Py_None);
	}
	if (item->mask & TVIF_PARAM && item->lParam) {
		// assume lParam is an object
		PyTuple_SET_ITEM(ret, 5, PyInt_FromLong(item->lParam));
	} else {
		Py_INCREF(Py_None);
		PyTuple_SET_ITEM(ret, 5, Py_None);
	}
	return ret;
}

// *** When PyCHeaderCtrl is implemented, return the '@' to the next line _and_ the parm!
// pymethod |PyCHeaderCtrl|HD_ITEM tuple|Describes a HD_ITEM tuple, used by the <o PyCHeaderCtrl> object.
// A tuple of 6 items:
// <nl>When passed to Python, will always be a tuple of size 6, and items may be None if not available.
// <nl>When passed from Python, the tuple may be any length up to 6, and any item may be None.
BOOL ParseHD_ITEMTuple( PyObject *args, HD_ITEM *pItem)
{
	PyObject *ob;
	pItem->mask = 0;
	int len = PyTuple_Size(args);
	if (len > 6) {
		PyErr_SetString(PyExc_TypeError, "HD_ITEM tuple has invalid size");
		return FALSE;
	}
	PyErr_Clear(); // clear any errors, so I can detect my own.
	// 0 - mask
	if (len<1) return TRUE;
	if ((ob=PyTuple_GetItem(args, 0))==NULL)
		return FALSE;
	if (ob != Py_None) {
		// pyparm int|<none>||Specifies if cxy is width (0) or height (1)
		if (ob)
			pItem->mask |= HDI_HEIGHT;
		else
			pItem->mask |= HDI_WIDTH;
	}
	// 1 - is cxy width or height of item
	if (len<2) return TRUE;
	if ((ob=PyTuple_GetItem(args, 1))==NULL)
		return FALSE;
	if (ob != Py_None) {
		// @pyparm int|cxy||Width or height of item
		pItem->cxy = (int)PyInt_AsLong(ob);
		if (PyErr_Occurred()) return FALSE;
		//mask updated above
	}
	// 2 - cxy (measurement of width or height depending on previous arg)
	
	// 3 - pszText address of item string
	if (len<3) return TRUE;
	if ((ob=PyTuple_GetItem(args, 2))==NULL)
		return FALSE;
	if (ob !=Py_None) {
		// @pyparm string|pszText||Item text
		if (!PyString_Check(ob)) RETURN_TYPE_ERR("The text item must be a string or None");
		pItem->pszText = PyString_AsString(ob);
		if (PyErr_Occurred()) return FALSE;
		pItem->cchTextMax = strlen(pItem->pszText)+1;
		pItem->mask |= HDI_TEXT;
	}
	// 3 - hbm handle of item bitmap
	if (len<4) return TRUE;
	if ((ob=PyTuple_GetItem(args, 3))==NULL)
		return FALSE;
	if (ob != Py_None) {
		// @pyparm string|text||Item text
		pItem->mask |= HDI_BITMAP;
		pItem->hbm = (HBITMAP)PyInt_AsLong(ob);
		if (PyErr_Occurred()) return FALSE;
	}
	// 4 - fmt of item string
	if (len<5) return TRUE;
	if ((ob=PyTuple_GetItem(args, 4))==NULL)
		return FALSE;
	if (ob != Py_None) {
		pItem->mask |= HDI_FORMAT;
		// @pyparm int|fmt||code for centering etc of string
		pItem->fmt = (int)PyInt_AsLong(ob);
	}
	// 5 - object
	if (len<6) return TRUE;
	if ((ob=PyTuple_GetItem(args, 5))==NULL)
		return FALSE;
	if (ob != Py_None) {
		// @pyparm int|lParam||User defined integer param.
		pItem->mask |= LVIF_PARAM;
		pItem->lParam = PyInt_AsLong(ob);
	}
	return !PyErr_Occurred();
}


/////////////////////////////////////////////////////////////////////
//
// CHARFORMAT and PARAFORMAT conversion utilities
//
//
// @object CHARFORMAT|Describes a CHARFORMAT tuple
BOOL ParseCharFormatTuple( PyObject *args, CHARFORMAT *pFmt)
{
	if (!PyTuple_Check(args))
		RETURN_TYPE_ERR("CHARFORMAT must be a tuple");
	int len = PyTuple_Size(args);
	if (len>0) pFmt->dwMask = PyInt_AsLong(PyTuple_GET_ITEM(args, 0));
	if (len>1) pFmt->dwEffects = PyInt_AsLong(PyTuple_GET_ITEM(args, 1));
	if (len>2) pFmt->yHeight = PyInt_AsLong(PyTuple_GET_ITEM(args, 2));
	if (len>3) pFmt->yOffset = PyInt_AsLong(PyTuple_GET_ITEM(args, 3));
	if (len>4) pFmt->crTextColor = PyInt_AsLong(PyTuple_GET_ITEM(args, 4));
	if (len>5) pFmt->bCharSet = (BYTE)PyInt_AsLong(PyTuple_GET_ITEM(args, 5));
	if (len>6) pFmt->bPitchAndFamily = (BYTE)PyInt_AsLong(PyTuple_GET_ITEM(args, 6));
	if (len>7) {
		PyObject *obFont = PyTuple_GET_ITEM(args, 7);
		if (obFont != Py_None) {
			if (!PyString_Check(obFont))
				RETURN_TYPE_ERR("Font name must be None or a string");
			strncpy(pFmt->szFaceName, PyString_AsString(obFont), sizeof(pFmt->szFaceName));
		}
	}
   // @tupleitem 0|int|mask|The mask to use.  Bits in this mask indicate which of the following parameter are interpreted.  Must be a combination the win32con.CFM_* constants.
   // @tupleitem 1|int|effects|The effects to use.  Must be a combination the win32con.CFE_* constants.
   // @tupleitem 2|int|yHeight|The y height.
   // @tupleitem 3|int|yOffset|Character offset from the baseline. If this member is positive, the character is a superscript; if it is negative, the character is a subscript.
   // @tupleitem 4|int|colorText|The color to use.
   // @tupleitem 5|int|bCharSet|The charset.  See the LOGFONT structure for details.
   // @tupleitem 6|int|bPitchAndFamily|The charset.  See the LOGFONT structure for details.
   // @tupleitem 7|string|faceName|The font name.

	// @comm  Executing d=win32ui.CreateFontDialog(); d.DoModal(); print d.GetCharFormat()
	// will print a valid CHARFORMAT tuple.
	return !PyErr_Occurred();
}

PyObject *MakeCharFormatTuple(CHARFORMAT *pFmt)
{
	return Py_BuildValue("iillibbs",
		       pFmt->dwMask,
			   pFmt->dwEffects,
			   pFmt->yHeight,
			   pFmt->yOffset,
			   pFmt->crTextColor,
			   pFmt->bCharSet,
			   pFmt->bPitchAndFamily,
			   pFmt->szFaceName);
}

// @object PARAFORMAT|Describes a PARAFORMAT tuple
BOOL ParseParaFormatTuple( PyObject *args, PARAFORMAT *pFmt)
{
	PyObject *obTabStops = Py_None;
	pFmt->cTabCount = 0;
	BOOL rc = PyArg_ParseTuple(args, "|iiiiiiiO:PARAFORMAT tuple", 
		       &pFmt->dwMask, // @tupleitem 0|int|mask|The mask to use.  Bits in this mask indicate which of the following parameters are interpreted.  Must be a combination the win32con.PFM_* constants.
			   &pFmt->wNumbering, // @tupleitem 1|int|numbering|The numbering style to use.
			   &pFmt->wReserved, // @tupleitem 2|int|yHeight|Reserved
			   &pFmt->dxStartIndent, // @tupleitem 3|int|dxStartIndent|Indentation of the first line.
			   &pFmt->dxRightIndent,// @tupleitem 4|int|dxRightIndent|Indentation from the right.
			   &pFmt->dxOffset,// @tupleitem 5|int|dxOffset|The indent of second and subsequent lines.
			   &pFmt->wAlignment,// @tupleitem 6|int|wAlignment|The alignment of the paragraph.
			   &obTabStops); // @tupleitem 7|[int ,...]|tabStops|The tabstops to use.
	if (rc && obTabStops != Py_None) {
		if (!PySequence_Check(obTabStops))
			RETURN_ERR("tabStops object must be None or a sequence");
		int tabCount = PyObject_Length(obTabStops);
		tabCount = min(MAX_TAB_STOPS, tabCount);
		for (int i=0;rc && i<tabCount;i++) {
			pFmt->rgxTabs[i] = PyInt_AsLong( PySequence_GetItem(obTabStops, i) );
			rc = PyErr_Occurred()==FALSE;
			if (!rc) break;
			pFmt->cTabCount++;
		}
	}
	return rc;
}

PyObject *MakeParaFormatTuple(PARAFORMAT *pFmt)
{
	PyObject *obTabs;
	if (pFmt->cTabCount==0) {
		Py_INCREF(Py_None);
		obTabs = Py_None;
	} else {
		obTabs = PyTuple_New(pFmt->cTabCount);
		for (int i=0;i<pFmt->cTabCount;i++)
			PyTuple_SetItem( obTabs, i, PyInt_FromLong(pFmt->rgxTabs[i]));
	}
	PyObject *ret = Py_BuildValue("iiiiiiiO", 
		       pFmt->dwMask,
			   pFmt->wNumbering,
			   pFmt->wReserved,
			   pFmt->dxStartIndent,
			   pFmt->dxRightIndent,
			   pFmt->dxOffset,
			   pFmt->wAlignment,
			   obTabs);
	Py_DECREF(obTabs); // ref added by BuildValue
//	Py_DECREF(obTabs); // reference I created.
	return ret;
}

/////////////////////////////////////////////////////////////////////
//
// Other utilities
//
//
// Given a long that holds a pointer, return 
// a Python object.  Used by listboxes and tree
// controls etc that keep a pointer to a Python object,
// but due to difficulties managing the lifetimes,
// does not keep a Python reference.  This function
// effectvly is just a cast with a fairly solid check
// that the object is still a valid PyObject * (ie,
// has not been destroyed since we copied the pointer).
// DOES NOT add a reference to the returned object.
PyObject *PyWin_GetPythonObjectFromLong(long val)
{
	PyObject *ret = (PyObject *)val;
	if (ret==NULL)
		return Py_None;
	BOOL ok;
	__try {
		ok = ret->ob_refcnt != 0;
		ok = ok && ret->ob_type->tp_name[0] != 0;
	}
	__except (EXCEPTION_ACCESS_VIOLATION) {
		ok = FALSE;
	}
	if (!ok)
		RETURN_ERR("The object is invalid");
	return ret;
}

CString GetAPIErrorString(char *fnName)
{
	CString csBuf = fnName + CString(" failed - ");
	DWORD errorCode = GetLastError();
	if (errorCode) {
		CString message = GetAPIErrorString(errorCode);
		if (message.GetLength()>0)
			csBuf += message;
		else {
			char buf[80];
			sprintf(buf,"error code was %d - no error message is available",errorCode);
			csBuf += buf;
		}
	}
	else
		csBuf += "no error code is available";
	return csBuf;
}

CString GetAPIErrorString(DWORD errCode)
{
	CString csBuf;
	const int bufSize = 512;
	char *buf = csBuf.GetBuffer( bufSize );
	::FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, errCode, 0, buf, bufSize, NULL );
	csBuf.ReleaseBuffer (-1);
	return csBuf;
}

ui_type_CObject *UITypeFromName( const char *name )
{
	ASSERT(ui_type_CObject::typemap);
	POSITION pos = ui_type_CObject::typemap->GetStartPosition();
	while (pos) {
		CRuntimeClass *key;
		ui_type_CObject *value;
		ui_type_CObject::typemap->GetNextAssoc(pos, key, value);
		if (strcmp(value->tp_name, name)==0)
			return value;
	}
	return NULL;
}

ui_type_CObject &UITypeFromCObject( CObject *ob )
{
	ui_type_CObject *ret;
	CRuntimeClass *prtCWnd = RUNTIME_CLASS(CWnd);

	// loop down the base class list.
	ASSERT(ui_type_CObject::typemap);
	ASSERT(ob);
	if(ui_type_CObject::typemap==NULL || ob==NULL) return ui_assoc_CObject::type;

	CRuntimeClass *prt = ob->GetRuntimeClass();
	while (prt) {
		// If we get here, and we only have a CWnd, then we use other tricks!!
		if (prt==prtCWnd) 
			return UITypeFromHWnd(((CWnd *)ob)->GetSafeHwnd());
		if (ui_type_CObject::typemap->Lookup(prt, ret))
			return *ret;
		prt = (*prt->m_pfnGetBaseClass)();
	}
	TRACE("Warning - unknown class type in UITypeFromCObject");
	ret = &ui_assoc_CObject::type;	// will have no methods - really an error
	return *ret;
}

ui_type_CObject &UITypeFromHWnd( HWND hwnd )
{
	ui_type_CObject *ret;
	// generic window - see if class name can help us.
	char szClassName[64];
	::GetClassName( hwnd, szClassName, sizeof(szClassName));
	// getting really lazy here.
	if (strcmp(szClassName, "ListBox")==0)
		ret = &PyCListBox::type;
	else if (strcmp(szClassName, "ComboBox")==0)
		ret = &PyCComboBox::type;
	else if (strcmp(szClassName, "Button")==0)
		ret = &PyCButton::type;
	else if (strcmp(szClassName, "Edit")==0)
		ret = &PyCEdit::type;
	else if (strcmp(szClassName, "RICHEDIT")==0)
		ret = &PyCRichEditCtrl::type;
	else if (strcmp(szClassName, "SysListView32")==0)
		ret = &PyCListCtrl::type;
	else if (strcmp(szClassName, "SysTreeView32")==0)
		ret = &PyCTreeCtrl::type;
	else if (strcmp(szClassName, "msctls_progress32")==0)
		ret = &PyCProgressCtrl::type;
	else if (strcmp(szClassName, "msctls_trackbar32")==0)
		ret = &PyCSliderCtrl::type;
	else if (strcmp(szClassName, "msctls_updown32")==0)
		ret = &PyCSpinButtonCtrl::type;
	// now handle some special cases to avoid warning below!
	else if (strcmp(szClassName, "MDIClient")==0 ||
		     strcmp(szClassName, "ConsoleWindowClass")==0)
		ret = &PyCWnd::type;
	else {
//		TRACE("Generic window returned for class name '%s'\n", szClassName);
		ret = &PyCWnd::type;
	}
	return *ret;
}

// utility to get a nice printable string from any object.
// reference neutral.
CString GetReprText( PyObject *objectUse )
{
	// special case for integers first.
	if (PyInt_Check(objectUse))
		return (int)PyInt_AsLong(objectUse);
	PyObject *s;
	if (PyString_Check(objectUse))  // if it is a string, then no need to repr it
		s = objectUse;              // and repr on a string may mangle it (eg
	else {                          // turn '\t' into a literal "\t"
		s = PyObject_Str(objectUse);
		if (s==NULL) {
			PyErr_Clear();
			s = PyObject_Repr(objectUse);
		}
		if (s==NULL) {
			PyErr_Clear();
			return CString("No str() or repr()?!");
		}
	}
	const char *szRepr = PyString_AsString(s);
	int len=strlen(szRepr);
	if (len > 2 && strchr("\"'[(", *szRepr)) {
		if (szRepr[len-1]==*szRepr) {
			++szRepr;
			len-=2;	// drop first and last chars.
		}
	}
	CString csRes = CString( szRepr, len );
	return csRes;
}

