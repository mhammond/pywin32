/******************************************************************************
  $Revision$

  A simple interface to win32 clipboard API

  Author: Roger Burnham, rburnham@cri-inc.com

  Note that this source file contains embedded documentation.
  This documentation consists of marked up text inside the
  C comments, and is prefixed with an '@' symbol.  The source
  files are processed by a tool called "autoduck" which
  generates Windows .hlp files.

  @doc

******************************************************************************/


#include "windows.h"
#include "Python.h"
#include "pywintypes.h"


#define CHECK_NO_ARGS2(args, fnName) do {\
  if(!PyArg_ParseTuple(args,":"#fnName)) return NULL;\
} while (0)

#define RETURN_TYPE_ERR(err) do {\
PyErr_SetString(PyExc_TypeError,err);return NULL;} while (0)

#define RETURN_NONE do {Py_INCREF(Py_None);return Py_None;} while (0)

PyObject *ReturnAPIError(char *fnName, long err = 0)
{
  return PyWin_SetAPIError(fnName, err);
}

//*****************************************************************************
//
// @pymethod int|win32clipboard|ChangeClipboardChain|The ChangeClipboardChain
// function removes a specified window from the chain of clipboard viewers.

static PyObject *
py_change_clipboard_chain(PyObject* self, PyObject* args)
{

  // @pyparm int|hWndRemove||Integer handle to the window to be removed from
  // the chain. The handle must have been passed to the SetClipboardViewer
  // function.

  // @pyparm int|hWndNewNext||Integer handle to the window that follows the
  // hWndRemove window in the clipboard viewer chain. (This is the handle
  // returned by SetClipboardViewer, unless the sequence was changed in
  // response to a WM_CHANGECBCHAIN message.) 

  HWND hWndRemove;
  HWND hWndNewNext;

  if (!PyArg_ParseTuple(args, "ii:ChangeClipboardChain",
                        &hWndRemove, &hWndNewNext)) {
    return NULL;
  }

  BOOL rc;
  Py_BEGIN_ALLOW_THREADS;
  rc = ChangeClipboardChain(hWndRemove, hWndNewNext);
  Py_END_ALLOW_THREADS;

  return (Py_BuildValue("i", (int)rc));

  // @comm The window identified by hWndNewNext replaces the hWndRemove window 
  // in the chain. The SetClipboardViewer function sends a WM_CHANGECBCHAIN
  // message to the first window in the clipboard viewer chain.

  // @pyseeapi ChangeClipboardChain

  // @rdesc The return value indicates the result of passing the
  // WM_CHANGECBCHAIN message to the windows in the clipboard viewer chain.
  // Because a window in the chain typically returns FALSE when it processes
  // WM_CHANGECBCHAIN, the return value from ChangeClipboardChain is typically
  // FALSE. If there is only one window in the chain, the return value is
  // typically TRUE.

}


//*****************************************************************************
//
// @pymethod None|win32clipboard|CloseClipboard|The CloseClipboard function closes
// the clipboard.<nl>

static PyObject *
py_close_clipboard(PyObject* self, PyObject* args)
{

  CHECK_NO_ARGS2(args, "CloseClipboard");

  BOOL rc;
  Py_BEGIN_ALLOW_THREADS;
  rc = CloseClipboard();
  Py_END_ALLOW_THREADS;

  if (!rc) {
    return ReturnAPIError("CloseClipboard");
  }

  RETURN_NONE;

  // @comm When the window has finished examining or changing the clipboard,
  // close the clipboard by calling CloseClipboard. This enables other windows
  // to access the clipboard.<nl>
  // Do not place an object on the clipboard after calling CloseClipboard.

  // @pyseeapi CloseClipboard

  // @rdesc If the function succeeds, the return value is None.<nl>
  // If the function fails, win32api.error is raised with the GetLastError
  // info.

}


//*****************************************************************************
//
// @pymethod int|win32clipboard|CountClipboardFormats|The CountClipboardFormats
// function retrieves the number of different data formats currently on the
// clipboard.

static PyObject *
py_count_clipboard_formats(PyObject* self, PyObject* args)
{

  CHECK_NO_ARGS2(args, "CountClipboardFormats");

  int rc;
  Py_BEGIN_ALLOW_THREADS;
  rc = CountClipboardFormats();
  Py_END_ALLOW_THREADS;

  if (!rc) {
    return ReturnAPIError("CountClipboardFormats");
  }

  return (Py_BuildValue("i", rc));

  // @pyseeapi CountClipboardFormats

  // @rdesc If the function succeeds, the return value is the number of
  // different data formats currently on the clipboard.
  // If the function fails, win32api.error is raised with the GetLastError
  // info.

}


//*****************************************************************************
//
// @pymethod None|win32clipboard|EmptyClipboard|The EmptyClipboard function empties
// the clipboard and frees handles to data in the clipboard. The function then
// assigns ownership of the clipboard to the window that currently has the
// clipboard open.

static PyObject *
py_empty_clipboard(PyObject* self, PyObject* args)
{

  CHECK_NO_ARGS2(args, "EmptyClipboard");

  BOOL rc;
  Py_BEGIN_ALLOW_THREADS;
  rc = EmptyClipboard();
  Py_END_ALLOW_THREADS;

  if (!rc) {
    return ReturnAPIError("EmptyClipboard");
  }

  RETURN_NONE;

  // @comm Before calling EmptyClipboard, an application must open the
  // clipboard by using the OpenClipboard function. If the application
  // specifies a NULL window handle when opening the clipboard, EmptyClipboard
  // succeeds but sets the clipboard owner to NULL. 

  // @pyseeapi EmptyClipboard

  // @rdesc If the function succeeds, the return value is None.<nl>
  // If the function fails, win32api.error is raised with the GetLastError
  // info.

}


//*****************************************************************************
//
// @pymethod int|win32clipboard|EnumClipboardFormats|The EnumClipboardFormats
// function lets you enumerate the data formats that are currently available
// on the clipboard.

static PyObject *
py_enum_clipboard_formats(PyObject* self, PyObject* args)
{

  // @pyparm int|format|0|Specifies a clipboard format that is known to be
  // available.<nl>
  // To start an enumeration of clipboard formats, set format to zero. When
  // format is zero, the function retrieves the first available clipboard
  // format. For subsequent calls during an enumeration, set format to the
  // result of the previous EnumClipboardFormat call. 

  int format = 0;
  if (!PyArg_ParseTuple(args, "|i:EnumClipboardFormats",
                        &format)) {
    return NULL;
  }

  UINT rc;
  Py_BEGIN_ALLOW_THREADS;
  rc = EnumClipboardFormats(format);
  Py_END_ALLOW_THREADS;

  if (!rc) {
    DWORD errNum = GetLastError();
    if (errNum) {
      return ReturnAPIError("EnumClipboardFormats", errNum);
    }
  }

  return (Py_BuildValue("i", (int)rc));

  // @comm Clipboard data formats are stored in an ordered list. To perform an
  // enumeration of clipboard data formats, you make a series of calls to the
  // EnumClipboardFormats function. For each call, the format parameter
  // specifies an available clipboard format, and the function returns the next
  // available clipboard format.<nl>
  // You must open the clipboard before enumerating its formats. Use the
  // OpenClipboard function to open the clipboard. The EnumClipboardFormats
  // function fails if the clipboard is not open.<nl>
  // The EnumClipboardFormats function enumerates formats in the order that
  // they were placed on the clipboard. If you are copying information to the
  // clipboard, add clipboard objects in order from the most descriptive
  // clipboard format to the least descriptive clipboard format. If you are
  // pasting information from the clipboard, retrieve the first clipboard
  // format that you can handle. That will be the most descriptive clipboard
  // format that you can handle.<nl>
  // The system provides automatic type conversions for certain clipboard
  // formats. In the case of such a format, this function enumerates the
  // specified format, then enumerates the formats to which it can be
  // converted.  For more information, see Standard Clipboard Formats and 
  // Synthesized Clipboard Formats. 

  // @pyseeapi EnumClipboardFormats

  // @rdesc If the function succeeds, the return value is the clipboard
  // format that follows the specified format. In other words, the next
  // available clipboard format.<nl>
  // If there are no more clipboard formats to enumerate, the return value is
  // zero.<nl>
  // If the function fails, win32api.error is raised with the GetLastError
  // info.

}


//*****************************************************************************
//
// @pymethod string/unicode|win32clipboard|GetClipboardData|The GetClipboardData function
// retrieves data from the clipboard in a specified format. The clipboard
// must have been opened previously.

static PyObject *
py_get_clipboard_data(PyObject* self, PyObject* args)
{

  PyObject *ret;

  // @pyparm int|format|CF_TEXT|Specifies a clipboard format. For a description of
  // the standard clipboard formats, see Standard Clipboard Formats.

  int format = CF_TEXT;
  if (!PyArg_ParseTuple(args, "|i:GetClipboardData:",
                        &format)) {
    return NULL;
  }

  HANDLE handle;
  Py_BEGIN_ALLOW_THREADS;
  handle = GetClipboardData((UINT)format);
  Py_END_ALLOW_THREADS;

  if (!handle) {
    return ReturnAPIError("GetClipboardData");
  }

  HGLOBAL cData;
  cData = GlobalLock(handle);
  if (!cData) {
    GlobalUnlock(handle);
    return ReturnAPIError("GetClipboardData:GlobalLock");
  }
  DWORD size = GlobalSize(cData);
  if (!size) {
    GlobalUnlock(handle);
    return ReturnAPIError("GetClipboardData:GlobalSize");
  }
  switch (format) {
    case CF_UNICODETEXT:
      ret = PyWinObject_FromWCHAR((wchar_t *)cData, (size / sizeof(wchar_t))-1);
      break;
    // For the text formats, strip the null!
    case CF_TEXT:
    case CF_OEMTEXT:
      ret = PyString_FromStringAndSize((char *)cData, size-1);
      break;
    default:
      ret = PyString_FromStringAndSize((char *)cData, size);
      break;
  }
  GlobalUnlock(handle);
  return ret;

  // @comm An application can enumerate the available formats in advance by
  // using the EnumClipboardFormats function.<nl>
  // The clipboard controls the handle that the GetClipboardData function
  // returns, not the application. The application should copy the data
  // immediately. The application cannot rely on being able to make long-term
  // use of the handle. The application must not free the handle nor leave it
  // locked.<nl>
  // The system performs implicit data format conversions between certain
  // clipboard formats when an application calls the GetClipboardData function.
  // For example, if the CF_OEMTEXT format is on the clipboard, a window can
  // retrieve data in the CF_TEXT format. The format on the clipboard is
  // converted to the requested format on demand. For more information, see
  // Synthesized Clipboard Formats. 

  // @pyseeapi GetClipboardData
  // @pyseeapi Standard Clipboard Formats

  // @rdesc If the function succeeds, the return value is the handle of a
  // clipboard object in the specified format.<nl>
  // If the function fails, win32api.error is raised with the GetLastError
  // info.

}


//*****************************************************************************
//
// @pymethod string|win32clipboard|GetClipboardFormatName|The GetClipboardFormatName
// function retrieves from the clipboard the name of the specified registered
// format.

static PyObject *
py_get_clipboard_formatName(PyObject* self, PyObject* args)
{

  // @pyparm int|format||Specifies the type of format to be retrieved. This
  // parameter must not specify any of the predefined clipboard formats. 

  int format;
  if (!PyArg_ParseTuple(args, "i:GetClipboardFormatName",
                        &format)) {
    return NULL;
  }

  char buf[256];
  int rc;
  Py_BEGIN_ALLOW_THREADS;
  rc = GetClipboardFormatName((UINT)format, buf, 255);
  Py_END_ALLOW_THREADS;

  if (!rc) {
    return ReturnAPIError("GetClipboardFormatName");
  }

  return Py_BuildValue("s", buf);

  // @pyseeapi GetClipboardFormatName

  // @rdesc If the function succeeds, the return value is the string containing
  // the format.<nl>
  // If the function fails, win32api.error is raised with the GetLastError
  // info.

}


//*****************************************************************************
//
// @pymethod int|win32clipboard|GetClipboardOwner|The GetClipboardOwner function
// retrieves the window handle of the current owner of the clipboard.

static PyObject *
py_get_clipboard_owner(PyObject* self, PyObject* args)
{

  CHECK_NO_ARGS2(args, "GetClipboardOwner");

  HWND rc;
  Py_BEGIN_ALLOW_THREADS;
  rc = GetClipboardOwner();
  Py_END_ALLOW_THREADS;

  if (!rc) {
    return ReturnAPIError("GetClipboardOwner");
  }

  return (Py_BuildValue("i", (int)rc));

  // @comm The clipboard can still contain data even if the clipboard is not
  // currently owned.<nl>
  // In general, the clipboard owner is the window that last placed data in
  // clipboard. The EmptyClipboard function assigns clipboard ownership. 

  // @pyseeapi GetClipboardOwner

  // @rdesc If the function succeeds, the return value is the handle of the
  // window that owns the clipboard. 
  // If the function fails, win32api.error is raised with the GetLastError
  // info.

}


#if(WINVER >= 0x0500)
//*****************************************************************************
//
// @pymethod int|win32clipboard|GetClipboardSequenceNumber|The
// GetClipboardSequenceNumber function returns the clipboard sequence number
// for the current window station.

static PyObject *
py_get_clipboard_sequence_number(PyObject* self, PyObject* args)
{

  CHECK_NO_ARGS2(args, "GetClipboardSequenceNumber");

  DWORD rc;
  Py_BEGIN_ALLOW_THREADS;
  rc = GetClipboardSequenceNumber();
  Py_END_ALLOW_THREADS;

  return (Py_BuildValue("i", (int)rc));

  // @comm [This is preliminary documentation and subject to change.]<nl>
  // The system keeps a 32-bit serial number for the clipboard for each window
  // station. This number is incremented whenever the contents of the
  // clipboard change or the clipboard is emptied. You can track this value to
  // determine whether the clipboard contents have changed and optimize
  // creating DataObjects. If clipboard rendering is delayed, the sequence
  // number is not incremented until the changes are rendered.

  // @pyseeapi GetClipboardSequenceNumber

  // @rdesc The return value is the clipboard sequence number. If you do not
  // have WINSTA_ACCESSCLIPBOARD access to the window station, the function
  // returns zero. 

}
#endif /* WINVER >= 0x0500 */


//*****************************************************************************
//
// @pymethod int|win32clipboard|GetClipboardViewer|The GetClipboardViewer function
// retrieves the handle of the first window in the clipboard viewer chain. 

static PyObject *
py_get_clipboard_viewer(PyObject* self, PyObject* args)
{

  CHECK_NO_ARGS2(args, "GetClipboardViewer");

  HWND rc;
  Py_BEGIN_ALLOW_THREADS;
  rc = GetClipboardViewer();
  Py_END_ALLOW_THREADS;

  if (!rc) {
    return ReturnAPIError("GetClipboardViewer");
  }

  return (Py_BuildValue("i", (int)rc));

  // @pyseeapi GetClipboardViewer

  // @rdesc If the function succeeds, the return value is the handle of the
  // first window in the clipboard viewer chain. 
  // If the function fails, win32api.error is raised with the GetLastError
  // info.

}


//*****************************************************************************
//
// @pymethod int|win32clipboard|GetOpenClipboardWindow|The GetOpenClipboardWindow
// function retrieves the handle of the window that currently has the
// clipboard open.

static PyObject *
py_get_open_clipboard_window(PyObject* self, PyObject* args)
{

  CHECK_NO_ARGS2(args, "GetOpenClipboardWindow");

  HWND rc;
  Py_BEGIN_ALLOW_THREADS;
  rc = GetOpenClipboardWindow();
  Py_END_ALLOW_THREADS;

  if (!rc) {
    return ReturnAPIError("GetOpenClipboardWindow");
  }

  return (Py_BuildValue("i", (int)rc));

  // @comm If an application or dynamic-link library (DLL) specifies a NULL
  // window handle when calling the OpenClipboard function, the clipboard is
  // opened but is not associated with a window. In such a case,
  // GetOpenClipboardWindow returns NULL. 

  // @pyseeapi GetOpenClipboardWindow

  // @rdesc If the function succeeds, the return value is the handle of the
  // window that has the clipboard open. 
  // If the function fails, win32api.error is raised with the GetLastError
  // info.

}


//*****************************************************************************
//
// @pymethod int|win32clipboard|GetPriorityClipboardFormat|The
// GetPriorityClipboardFormat function returns the first available clipboard
// format in the specified list. 

static PyObject *
py_getPriority_clipboard_format(PyObject* self, PyObject* args)
{

  // @pyparm tuple|formats||Tuple of integers identifying clipboard formats,
  // in priority order. For a description of the standard clipboard formats,
  // see Standard Clipboard Formats. 

  PyObject *formats;
  if (!PyArg_ParseTuple (args,"O:GetPriorityClipboardFormat", 
                         &formats)) {
    return NULL;
  }

  if (!PyTuple_Check(formats)) {
    RETURN_TYPE_ERR(
       "GetPriorityClipboardFormat requires a tuple of integer formats");
  }

  int num_formats = PyTuple_Size(formats);
  UINT *format_list = new UINT[num_formats];
  PyObject *o;
  for (int i = 0; i < num_formats; i++) {
    o = PyTuple_GetItem(formats, i);
    if (!PyInt_Check(o)) {
      delete format_list;
      RETURN_TYPE_ERR ("GetPriorityClipboardFormat expected integer formats.");
    }
    format_list[i] = PyInt_AsLong(o);
  }

  int rc;
  Py_BEGIN_ALLOW_THREADS;
  rc = GetPriorityClipboardFormat(format_list, num_formats);
  Py_END_ALLOW_THREADS;

  delete format_list;

  return (Py_BuildValue("i", rc));

  // @pyseeapi GetPriorityClipboardFormat
  // @pyseeapi Standard Clipboard Formats

  // @rdesc If the function succeeds, the return value is the first clipboard
  // format in the list for which data is available. If the clipboard is
  // empty, the return value is NULL. If the clipboard contains data, but not
  // in any of the specified formats, the return value is -1.

}


//*****************************************************************************
//
// @pymethod int|win32clipboard|IsClipboardFormatAvailable|The
// IsClipboardFormatAvailable function determines whether the clipboard
// contains data in the specified format.

static PyObject *
py_is_clipboard_format_available(PyObject* self, PyObject* args)
{

  // @pyparm int|format||Specifies a clipboard format. For a description of
  // the standard clipboard formats, see Standard Clipboard Formats.

  int format;
  if (!PyArg_ParseTuple(args, "i:IsClipboardFormatAvailable",
                        &format)) {
    return NULL;
  }

  BOOL rc;
  Py_BEGIN_ALLOW_THREADS;
  rc = IsClipboardFormatAvailable((UINT)format); 
  Py_END_ALLOW_THREADS;

  return (Py_BuildValue("i", (int)rc));

  // @comm Typically, an application that recognizes only one clipboard format
  // would call this function when processing the WM_INITMENU or
  // WM_INITMENUPOPUP message. The application would then enable or disable
  // the Paste menu item, depending on the return value. Applications that
  // recognize more than one clipboard format should use the
  // GetPriorityClipboardFormat function for this purpose. 

  // @pyseeapi IsClipboardFormatAvailable
  // @pyseeapi Standard Clipboard Formats

  // @rdesc If the clipboard format is available, the return value is nonzero.

}


//*****************************************************************************
//
// @pymethod None|win32clipboard|OpenClipboard|The OpenClipboard function opens the
// clipboard for examination and prevents other applications from modifying
// the clipboard content.

static PyObject *
py_open_clipboard(PyObject* self, PyObject* args)
{

  // @pyparm int|hWnd||Integer handle to the window to be associated with the
  // open clipboard. If this parameter is 0, the open clipboard is associated
  // with the current task. 

  HWND pyHwnd = 0;
  if (!PyArg_ParseTuple(args, "|i:OpenClipboard",
                        &pyHwnd)) {
    return NULL;
  }

  BOOL rc;
  Py_BEGIN_ALLOW_THREADS;
  rc = OpenClipboard(pyHwnd);
  Py_END_ALLOW_THREADS;

  if (!rc) {
    return ReturnAPIError("OpenClipboard");
  }

  RETURN_NONE;  

  // @comm OpenClipboard fails if another window has the clipboard open.<nl>
  // An application should call the CloseClipboard function after every
  // successful call to OpenClipboard.<nl>
  // The window identified by the hWnd parameter does not become the
  // clipboard owner unless the EmptyClipboard function is called. 

  // @pyseeapi OpenClipboard

  // @rdesc If the function succeeds, the return value is None.<nl>
  // If the function fails, win32api.error is raised with the GetLastError
  // info.

}


//*****************************************************************************
//
// @pymethod None|win32clipboard|RegisterClipboardFormat|The
// RegisterClipboardFormat function registers a new clipboard format.
// This format can then be used as a valid clipboard format.

static PyObject *
py_register_clipboard_format(PyObject* self, PyObject* args)
{

  // @pyparm string|name||String that names the new format.

  char *name;
  if (!PyArg_ParseTuple(args, "s:RegisterClipboardFormat",
                        &name)) {
    return NULL;
  }

  UINT rc;
  Py_BEGIN_ALLOW_THREADS;
  rc = RegisterClipboardFormat(name);
  Py_END_ALLOW_THREADS;

  if (!rc) {
    return ReturnAPIError("RegisterClipboardFormat");
  }

  return (Py_BuildValue("i", (int)rc));

  // @comm If a registered format with the specified name already exists, a
  // new format is not registered and the return value identifies the existing
  // format. This enables more than one application to copy and paste data
  // using the same registered clipboard format. Note that the format name
  // comparison is case-insensitive.<nl>
  // Registered clipboard formats are identified by values in the range 0xC000
  // through 0xFFFF. 

  // @pyseeapi RegisterClipboardFormat

  // @rdesc If the function succeeds, the return value identifies the
  // registered clipboard format.
  // If the function fails, win32api.error is raised with the GetLastError
  // info.

}


//*****************************************************************************
//
// @pymethod int|win32clipboard|SetClipboardData|The SetClipboardData function
// places data on the clipboard in a specified clipboard format. The window 
// must be the current clipboard owner, and the application must have called 
// the OpenClipboard function. (When responding to the WM_RENDERFORMAT and
// WM_RENDERALLFORMATS messages, the clipboard owner must not call
// OpenClipboard before calling SetClipboardData.)

static PyObject *
py_set_clipboard_data(PyObject* self, PyObject* args)
{

	// @pyparm int|format||Specifies a clipboard format. For a description of
	// the standard clipboard formats, see Standard Clipboard Formats.

	// @pyparm int|hMem||Integer handle to the data in the specified format.
	// This parameter can be 0, indicating that the window provides data in
	// the specified clipboard format (renders the format) upon request. If a
	// window delays rendering, it must process the WM_RENDERFORMAT and
	// WM_RENDERALLFORMATS messages.<nl>
	// After SetClipboardData is called, the system owns the object identified
	// by the hMem parameter. The application can read the data, but must not
	// free the handle or leave it locked. If the hMem parameter identifies a
	// memory object, the object must have been allocated using the GlobalAlloc
	// function with the GMEM_MOVEABLE and GMEM_DDESHARE flags. 
	int format;
	HANDLE handle;
	int ihandle;
	if (PyArg_ParseTuple(args, "ii:SetClipboardData",
                        &format, &ihandle)) {
		handle = (HANDLE)ihandle;
	} else {
		PyErr_Clear();
		// @pyparmalt1 int|format||Specifies a clipboard format. For a description of
		// the standard clipboard formats, see Standard Clipboard Formats.

		// @pyparmalt1 object|ob||An object that has a read-buffer interface.
		// A global memory object is allocated, and the objects buffer is copied
		// to the new memory.
		PyObject *obBuf;
		if (!PyArg_ParseTuple(args, "iO:SetClipboardData",
                      &format, &obBuf))
		      return NULL;
		PyBufferProcs *pb = obBuf->ob_type->tp_as_buffer;
		if (pb==NULL)
			RETURN_TYPE_ERR("The object must support the buffer interfaces");
		void *buf = NULL;
		int bufSize = (*pb->bf_getreadbuffer)(obBuf, 0, &buf);
		// size doesnt include nulls!
		if (PyString_Check(obBuf))
			bufSize += 1;
		else if (PyUnicode_Check(obBuf))
			bufSize += sizeof(wchar_t);
		// else assume buffer needs no terminator...
		handle = GlobalAlloc(GHND, (DWORD)bufSize);
		if (handle == NULL) {
			return ReturnAPIError("GlobalAlloc");
		}
		void *dest = GlobalLock(handle);
		memcpy(dest, buf, bufSize);
		GlobalUnlock(handle);
	}
	HANDLE data;
	Py_BEGIN_ALLOW_THREADS;
	data = SetClipboardData((UINT)format, handle);
	Py_END_ALLOW_THREADS;

	if (!data) {
		return ReturnAPIError("SetClipboardData");
	}
	return (Py_BuildValue("i", (int)data));

	// @comm The uFormat parameter can identify a registered clipboard format,
	// or it can be one of the standard clipboard formats. For more information,
	// see Registered Clipboard Formats and Standard Clipboard Formats.<nl>
	// The system performs implicit data format conversions between certain
	// clipboard formats when an application calls the GetClipboardData function.
	// For example, if the CF_OEMTEXT format is on the clipboard, a window can
	// retrieve data in the CF_TEXT format. The format on the clipboard is
	// converted to the requested format on demand. For more information, see
	// Synthesized Clipboard Formats. 

	// @pyseeapi SetClipboardData

	// @rdesc If the function succeeds, the return value is integer handle
	// of the data.<nl>
	// If the function fails, win32api.error is raised with the GetLastError
	// info.
}


//*****************************************************************************
//
// @pymethod int|win32clipboard|SetClipboardText|Convienience function to
// call SetClipboardData with text.

static PyObject *
py_set_clipboard_text(PyObject* self, PyObject* args)
{

  // @pyparm string|text||The text to place on the clipboard.

  int format = CF_TEXT;
  char *text;
  int size;
  if (!PyArg_ParseTuple(args, "s#:SetClipboardText",
                        &text, &size)) {
    return NULL;
  }

  HGLOBAL    hMem;
  LPTSTR     pszDst;

  hMem = GlobalAlloc(GHND, (DWORD)(size+1));
  if (hMem == NULL) {
    return ReturnAPIError("GlobalAlloc");
  }
  pszDst = (char*)GlobalLock(hMem);
  lstrcpy(pszDst, text);
  pszDst[size] = 0;
  GlobalUnlock(hMem);

  HANDLE data;
  Py_BEGIN_ALLOW_THREADS;
  data = SetClipboardData((UINT)format, hMem);
  Py_END_ALLOW_THREADS;

  if (!data) {
    return ReturnAPIError("SetClipboardText");
  }

  return (Py_BuildValue("i", (int)data));

  // @pyseeapi SetClipboardData

  // @rdesc If the function succeeds, the return value is integer handle
  // of the data.<nl>
  // If the function fails, win32api.error is raised with the GetLastError
  // info.

}


//*****************************************************************************
//
// @pymethod int|win32clipboard|SetClipboardViewer|The SetClipboardViewer function
// adds the specified window to the chain of clipboard viewers. Clipboard
// viewer windows receive a WM_DRAWCLIPBOARD message whenever the content of
// the clipboard changes.

static PyObject *
py_set_clipboard_viewer(PyObject* self, PyObject* args)
{

  // @pyparm int|hWndNewViewer||Integer handle to the window to be added to
  // the clipboard chain. 

  HWND hWndNewViewer;
  if (!PyArg_ParseTuple(args, "i:SetClipboardViewer",
                        &hWndNewViewer)) {
    return NULL;
  }

  HWND rc;
  Py_BEGIN_ALLOW_THREADS;
  rc = SetClipboardViewer(hWndNewViewer);
  Py_END_ALLOW_THREADS;

  if (!rc) {
    return ReturnAPIError("SetClipboardViewer");
  }

  return (Py_BuildValue("i", (int)rc));

  // @comm The windows that are part of the clipboard viewer chain, called
  // clipboard viewer windows, must process the clipboard messages
  // WM_CHANGECBCHAIN and WM_DRAWCLIPBOARD. Each clipboard viewer window calls
  // the SendMessage function to pass these messages to the next window in the
  // clipboard viewer chain.<nl>
  // A clipboard viewer window must eventually remove itself from the clipboard
  // viewer chain by calling the ChangeClipboardChain function -- for example,
  // in response to theWM_DESTROY message. 

  // @pyseeapi SetClipboardViewer

  // @rdesc If the function succeeds, the return value identifies the next
  // window in the clipboard viewer chain.<nl>
  // If an error occurs or there are no other windows in the clipboard viewer
  // chain, win32api.error is raised with the GetLastError info.

}


// @module win32clipboard|A module which supports the Windows Clipboard API.

// List of functions exported by this module
static struct PyMethodDef clipboard_functions[] = {

  // @pymeth ChangeClipboardChain|Removes a specified window from the chain
  // of clipboard viewers. 
  {"ChangeClipboardChain", py_change_clipboard_chain, 1},

  // @pymeth CloseClipboard|Closes the clipboard. 
  {"CloseClipboard", py_close_clipboard, 1},

  // @pymeth CountClipboardFormats|Retrieves the number of different data
  //formats currently on the clipboard.
  {"CountClipboardFormats", py_count_clipboard_formats, 1},

  // @pymeth EmptyClipboard|Empties the clipboard and frees handles to data
  // in the clipboard. 
  {"EmptyClipboard", py_empty_clipboard, 1},

  // @pymeth EnumClipboardFormats|Lets you enumerate the data formats that
  // are currently available on the clipboard.
  {"EnumClipboardFormats", py_enum_clipboard_formats, 1},

  // @pymeth GetClipboardData|Retrieves data from the clipboard in a
  // specified format. 
  {"GetClipboardData", py_get_clipboard_data, 1},

  // @pymeth GetClipboardFormatName|Retrieves from the clipboard the name
  // of the specified registered format. 
  {"GetClipboardFormatName", py_get_clipboard_formatName, 1},

  // @pymeth GetClipboardOwner|Retrieves the window handle of the current
  // owner of the clipboard. 
  {"GetClipboardOwner", py_get_clipboard_owner, 1},

#if(WINVER >= 0x0500)
  // @pymeth GetClipboardSequenceNumber|Returns the clipboard sequence number
  // for the current window station. 
  {"GetClipboardSequenceNumber", py_get_clipboard_sequence_number, 1},
#endif /* WINVER >= 0x0500 */

  // @pymeth GetClipboardViewer|Retrieves the handle of the first window in
  // the clipboard viewer chain. 
  {"GetClipboardViewer", py_get_clipboard_viewer, 1},

  // @pymeth GetOpenClipboardWindow|Retrieves the handle of the window that
  // currently has the clipboard open. 
  {"GetOpenClipboardWindow", py_get_open_clipboard_window, 1},

  // @pymeth GetPriorityClipboardFormat|Returns the first available clipboard
  // format in the specified list. 
  {"GetPriorityClipboardFormat", py_getPriority_clipboard_format, 1},

  // @pymeth IsClipboardFormatAvailable|Determines whether the clipboard
  // contains data in the specified format.
  {"IsClipboardFormatAvailable", py_is_clipboard_format_available, 1},

  // @pymeth OpenClipboard|Opens the clipboard for examination.
  {"OpenClipboard", py_open_clipboard, 1},

  // @pymeth RegisterClipboardFormat|Registers a new clipboard format.
  {"RegisterClipboardFormat", py_register_clipboard_format, 1},

  // @pymeth SetClipboardData|Places data on the clipboard in a specified
  // clipboard format. 
  {"SetClipboardData", py_set_clipboard_data, 1},

  // @pymeth SetClipboardText|Places text on the clipboard . 
  {"SetClipboardText", py_set_clipboard_text, 1},

  // @pymeth SetClipboardViewer|Adds the specified window to the chain of
  // clipboard viewers
  {"SetClipboardViewer", py_set_clipboard_viewer, 1},

  {NULL, NULL}
};


static int AddConstant(PyObject *dict, char *key, long value)
{
	PyObject *okey = PyString_FromString(key);
	PyObject *oval = PyInt_FromLong(value);
	if (!okey || !oval) {
		Py_XDECREF(okey);
		Py_XDECREF(oval);
		return 1;
	}
	int rc = PyDict_SetItem(dict,okey, oval);
	Py_XDECREF(okey);
	Py_XDECREF(oval);
	return rc;
}

#define ADD_CONSTANT(tok) if (rc=AddConstant(dict,#tok, tok)) return rc

static int AddConstants(PyObject *dict)
{
	int rc;
	ADD_CONSTANT(CF_TEXT);
	ADD_CONSTANT(CF_BITMAP);
	ADD_CONSTANT(CF_METAFILEPICT);
	ADD_CONSTANT(CF_SYLK);
	ADD_CONSTANT(CF_DIF);
	ADD_CONSTANT(CF_TIFF);
	ADD_CONSTANT(CF_OEMTEXT);
	ADD_CONSTANT(CF_DIB);
	ADD_CONSTANT(CF_PALETTE);
	ADD_CONSTANT(CF_PENDATA);
	ADD_CONSTANT(CF_RIFF);
	ADD_CONSTANT(CF_WAVE);
	ADD_CONSTANT(CF_UNICODETEXT);
	ADD_CONSTANT(CF_ENHMETAFILE);
	ADD_CONSTANT(CF_HDROP);
	ADD_CONSTANT(CF_LOCALE);
	ADD_CONSTANT(CF_MAX);
	ADD_CONSTANT(CF_OWNERDISPLAY);
	ADD_CONSTANT(CF_DSPTEXT);
	ADD_CONSTANT(CF_DSPBITMAP);
	ADD_CONSTANT(CF_DSPMETAFILEPICT);
	ADD_CONSTANT(CF_DSPENHMETAFILE);
	return 0;
}


extern "C" __declspec(dllexport) void
initwin32clipboard(void)
{
  PyObject *dict, *module;
  module = Py_InitModule("win32clipboard", clipboard_functions);
  dict = PyModule_GetDict(module);
  PyWinGlobals_Ensure();
  AddConstants(dict);
  Py_INCREF(PyWinExc_ApiError);
  PyDict_SetItemString(dict, "error", PyWinExc_ApiError);
}
