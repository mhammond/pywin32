/* File : winxptheme.i */
// @doc

%module _winxptheme // A module which provides an interface to the Windows XP
                   // 'theme' API.
                   // <nl>Note that this module will fail to load on
                   // non-Windows XP versions.  Generally you should use the
                   // 'winxptheme' module which will load on all Windows
                   // versions, and provide implementations of
                   // IsThemeActive() or IsAppThemed() which return False when
                   // XP is not used, and provides all objects from this module
                   // when XP is used.  See winxptheme.py for more details.

%include "typemaps.i"
%include "pywintypes.i"

%{
#define _WIN32_IE 0x0501 // to enable balloon notifications in Shell_NotifyIcon
#define _WIN32_WINNT 0x0501
//#define ISOLATION_AWARE_ENABLED 1

#undef PyHANDLE
#include "pywinobjects.h"
#include "windows.h"
#include "Uxtheme.h"
#include "commctrl.h"


%}

// @object PyHTHEME|A <o PyHANDLE> object wrapping a HTHEME.
// <om _winxptheme.CloseThemeData> will be called when the object dies or
// <om PyHANDLE.Close> is called.

%{
#undef PyHANDLE

// Support for HTHEME objects.  Like a PyHANDLE, but calls CloseThemeData
class PyHTHEME: public PyHANDLE
{
public:
	PyHTHEME(HTHEME hInit) : PyHANDLE(hInit) {}
	virtual BOOL Close(void) {
		HRESULT err = m_handle ? CloseThemeData((HTHEME)m_handle) : S_OK;
		m_handle = 0;
		if (err!= S_OK)
			PyWin_SetAPIError("CloseThemeData", err);
		return err==S_OK;
	}
	virtual const char *GetTypeName() {
		return "PyHTHEME";
	}
};

%}

%typemap(python,except) HTHEME {
      Py_BEGIN_ALLOW_THREADS
      $function
      Py_END_ALLOW_THREADS
}


%typemap(python,out) HTHEME {
  if ($source==(HTHEME)0) {
    $target = Py_None;
    Py_INCREF(Py_None);
  } else
    $target = new PyHTHEME($source);
}

%typemap(python,ignore) HTHEME *(HTHEME temp)
{
  if (temp==(HTHEME)0) {
    $target = Py_None;
    Py_INCREF(Py_None);
  } else
    $target = new PyHTHEME(temp);
}
%typemap(python,in) HTHEME *(HTHEME temp)
{
    $target = &temp;
    if (!PyWinObject_AsHANDLE($source, $target))
        return NULL;
}

%apply HANDLE {HTHEME};
typedef HANDLE HTHEME;

typedef float HDC;
%typemap(python, in) HDC{
	if (!PyWinObject_AsHANDLE($source, (HANDLE *)&$target))
		return NULL;
}

%typemap(python,ignore) RECT *OUTPUT(RECT temp)
{
  $target = &temp;
}

%typemap(python,in) RECT *INPUT {
    RECT r;
	if (PyTuple_Check($source)) {
		if (PyArg_ParseTuple($source, "llll", &r.left, &r.top, &r.right, &r.bottom) == 0) {
			return PyErr_Format(PyExc_TypeError, "%s: This param must be a tuple of four integers", "$name");
		}
		$target = &r;
	} else {
		return PyErr_Format(PyExc_TypeError, "%s: This param must be a tuple of four integers", "$name");
	}
}

%typemap(python,in) RECT *INPUT_NULLOK {
    RECT r;
	if (PyTuple_Check($source)) {
		if (PyArg_ParseTuple($source, "llll", &r.left, &r.top, &r.right, &r.bottom) == 0) {
			return PyErr_Format(PyExc_TypeError, "%s: This param must be a tuple of four integers or None", "$name");
		}
		$target = &r;
	} else {
		if ($source == Py_None) {
            $target = NULL;
        } else {
            PyErr_SetString(PyExc_TypeError, "This param must be a tuple of four integers or None");
            return NULL;
		}
	}
}

// This uses a 'hresult' API

typedef long HRESULT;	// This will raise COM Exception.
%typedef long HRESULT_KEEP; // This will keep HRESULT, and return
typedef long FLAGS;


%typemap(python,out) HRESULT {
	$target = Py_None;
	Py_INCREF(Py_None);
}

%typemap(python,except) HRESULT {
      Py_BEGIN_ALLOW_THREADS
      $function
      Py_END_ALLOW_THREADS
      if (FAILED($source))  {
           $cleanup
           return PyWin_SetAPIError("$name", $source);
      }
}

//  @pyswig <o PyHTHEME>|OpenThemeData|Open the theme data for the specified HWND and 
//                        semi-colon separated list of class names. 
//                        <nl>OpenThemeData() will try each class name, one at 
//                        a time, and use the first matching theme info
//                        found.  If a match is found, a theme handle
//                        to the data is returned.  If no match is found,
//                        a "NULL" handle is returned. 
//                        <nl>When the window is destroyed or a WM_THEMECHANGED
//                        msg is received, <om _winxptheme.CloseThemeData> should be 
//                        called to close the theme handle.
//  @pyparm int|hwnd||Window handle of the control/window to be themed
//
//  @pyparm string|pszClassList||Class name (or list of names) to match to theme data
//                        section.  if the list contains more than one name, 
//                        the names are tested one at a time for a match.  
//                        If a match is found, OpenThemeData() returns a 
//                        theme handle associated with the matching class. 
//                        This param is a list (instead of just a single 
//                        class name) to provide the class an opportunity 
//                        to get the "best" match between the class and 
//                        the current theme.  For example, a button might
//                        pass L"OkButton, Button" if its ID=ID_OK.  If 
//                        the current theme has an entry for OkButton, 
//                        that will be used.  Otherwise, we fall back on 
//                        the normal Button entry.

HTHEME OpenThemeData(HWND hwnd, WCHAR *pszClassList);

//  @pyswig |CloseThemeData|Closes the theme data handle.  This should be done 
//                        when the window being themed is destroyed or
//                        whenever a WM_THEMECHANGED msg is received 
//                        (followed by an attempt to create a new Theme data 
//                        handle).
//
//  @pyparm <o PyHTHEME>|hTheme||Open theme data handle (returned from prior call
//                        to OpenThemeData() API).

%{
static PyObject *MyCloseThemeData(PyObject *self, PyObject *args)
{
    PyObject *obHandle;
    if (!PyArg_ParseTuple(args, "O:CloseThemeData", &obHandle))
        return NULL;
    if (!PyHANDLE_Check(obHandle))
        return PyErr_Format(PyExc_TypeError,
                            "CloseThemeData requires a PyHTHEME object - got %s",
                            obHandle->ob_type->tp_name);
    PyHANDLE *p = (PyHANDLE *)obHandle;
    if (!p->Close())
        return NULL;
    Py_INCREF(Py_None);
    return Py_None;
}
%}
%native (CloseThemeData) MyCloseThemeData;

//  @pyswig |DrawThemeBackground|Draws the theme-specified border and fill for 
//                        the "iPartId" and "iStateId".  This could be 
//                        based on a bitmap file, a border and fill, or 
//                        other image description.  
//
//  @pyparm <o PyHTHEME>|hTheme||theme data handle
//  @pyparm int|hdc||HDC to draw into
//  @pyparm int|iPartId||part number to draw
//  @pyparm int|iStateId||state number (of the part) to draw
//  @pyparm rect|pRect||defines the size/location of the part
//  @pyparm rect|pClipRect||optional clipping rect (don't draw outside it)

HRESULT DrawThemeBackground(HTHEME hTheme, HDC hdc, 
    int iPartId, int iStateId, RECT *INPUT, RECT *INPUT_NULLOK);

//  @pyswig |DrawThemeText|Draws the text using the theme-specified 
//  color and font for the "iPartId" and "iStateId".  
//  @pyparm <o PyHTHEME>|hTheme||theme data handle
//  @pyparm int|hdc||HDC to draw into
//  @pyparm int|iPartId||part number to draw
//  @pyparm int|iStateId||state number (of the part) to draw
//  @pyparm string|pszText||actual text to draw
//  @pyparm int|dwCharCount||number of chars to draw (-1 for all)
//  @pyparm int|dwTextFlags||same as DrawText() "uFormat" param
//  @pyparm int|dwTextFlags2||additional drawing options 
//  @pyparm rect|pRect||defines the size/location of the part

HRESULT DrawThemeText(HTHEME hTheme, HDC hdc, int iPartId, 
    int iStateId, WCHAR *pszText, int iCharCount, DWORD dwTextFlags, 
    DWORD dwTextFlags2, RECT *INPUT);

//  @pyswig rect|GetThemeBackgroundContentRect|Gets the size of the content for the theme-defined 
//  background.  This is usually the area inside the borders or Margins.  
//      @pyparm <o PyHTHEME>|hTheme||theme data handle
//      @pyparm int|hdc||(optional) device content to be used for drawing
//      @pyparm int|iPartId||part number to draw
//      @pyparm int|iStateId||state number (of the part) to draw
//      @pyparm rect|pBoundingRect||the outer RECT of the part being drawn
//      @rdesc The result is a rect with the content area
HRESULT GetThemeBackgroundContentRect(HTHEME hTheme, HDC hdc, 
    int iPartId, int iStateId,  RECT *INPUT, 
    RECT *OUTPUT);

// @pyswig rect|GetThemeBackgroundExtent|Calculates the size/location of the theme-
// specified background based on the "pContentRect".
// @pyparm <o PyHTHEME>|hTheme||theme data handle
// @pyparm int|hdc||(optional) device content to be used for drawing
// @pyparm int|iPartId||part number to draw
// @pyparm int|iStateId||state number (of the part) to draw
// @pyparm rect|pContentRect||RECT that defines the content area
// @rdesc Result is a rect with the overall size/location of part
HRESULT GetThemeBackgroundExtent(HTHEME hTheme, HDC hdc,
    int iPartId, int iStateId, RECT *INPUT, 
    RECT *OUTPUT);

//  @pyswig bool|IsThemeActive|Can be used to test if a system theme is active
//  for the current user session.  
//  <nl>use the API <om _winxptheme.IsAppThemed> to test if a theme is
//  active for the calling process.
BOOL IsThemeActive();

//  @pyswig bool|IsAppThemed|Returns True if a theme is active and available to
//  the current process
BOOL IsAppThemed();

//  @pyswig <o PyHTHEME>|GetWindowTheme|If window is themed, returns its most recent
//  HTHEME from OpenThemeData() - otherwise, returns NULL.
//  @pyparm int|hwnd||The window to get the HTHEME of
HTHEME GetWindowTheme(HWND hwnd);

//  @pyswig |EnableThemeDialogTexture|Enables/disables dialog background theme.
//    This method can be used to 
//    tailor dialog compatibility with child windows and controls that 
//    may or may not coordinate the rendering of their client area backgrounds 
//    with that of their parent dialog in a manner that supports seamless 
//    background texturing.
// @pyparm int|hdlg||The window handle of the target dialog
// @pyparm int|dwFlags||ETDT_ENABLE to enable the theme-defined dialog background texturing,
//                     <nl>ETDT_DISABLE to disable background texturing,
//                     <nl>ETDT_ENABLETAB to enable the theme-defined background 
//                          texturing using the Tab texture
#define ETDT_DISABLE        ETDT_DISABLE
#define ETDT_ENABLE         ETDT_ENABLE
#define ETDT_USETABTEXTURE  ETDT_USETABTEXTURE
#define ETDT_ENABLETAB      ETDT_ENABLETAB

HRESULT EnableThemeDialogTexture(HWND hwnd, DWORD dwFlags);

//  @pyswig bool|IsThemeDialogTextureEnabled|Reports whether the dialog supports background texturing.
//  @pyparm int|hdlg||The window handle of the target dialog
BOOL IsThemeDialogTextureEnabled(HWND hwnd);

//  @pyswig int|GetThemeAppProperties|Returns the app property flags that control theming
DWORD GetThemeAppProperties();

//  @pyswig |EnableTheming|Enables or disables themeing for the current user
//  in the current and future sessions.
//  @pyparm bool|fEnable||if False, disable theming & turn themes off.
//                        <nl>if True, enable themeing and, if user previously
//                        had a theme active, make it active now.
HRESULT EnableTheming(BOOL fEnable);

//  @pyswig |SetWindowTheme|Rredirects an existing Window to use a different 
//  section of the current theme information than its class normally asks for.
//  @pyparm int|hwnd||The handle of the window (cannot be 0)
//  @pyparm string/None|pszSubAppName||App (group) name to use in place of the calling
//  app's name.  If NULL, the actual calling app name will be used.
//  @pyparm string/None|pszSubIdList||A semicolon separated list of class Id names to 
//  use in place of actual list passed by the window's class.  if NULL, the id
//  list from the calling class is used.
HRESULT SetWindowTheme(HWND hwnd, WCHAR *INPUT_NULLOK, WCHAR *INPUT_NULLOK);

//  @pyswig (string, string, string)|GetCurrentThemeName|
// Get the name of the current theme in-use, the
// canonical color scheme name (not the display name) and the
// canonical size name (not the display name).
%{
static PyObject *MyGetCurrentThemeName(PyObject *self, PyObject *args)
{
    if (!PyArg_ParseTuple(args, ":GetCurrentThemeName"))
        return NULL;
    WCHAR nameBuf[256] = {'\0'};
    WCHAR colorBuf[256] = {'\0'};
    WCHAR sizeBuf[256] = {'\0'};

    HRESULT hr = GetCurrentThemeName(nameBuf, sizeof(nameBuf)/sizeof(TCHAR),
                                     colorBuf, sizeof(colorBuf)/sizeof(TCHAR),
                                     sizeBuf, sizeof(sizeBuf)/sizeof(TCHAR));
    if (FAILED(hr))
        return PyWin_SetAPIError("GetCurrentThemeName", hr);
    return Py_BuildValue("NNN",
                         PyWinObject_FromWCHAR(nameBuf),
                         PyWinObject_FromWCHAR(colorBuf),
                         PyWinObject_FromWCHAR(sizeBuf));
}
%}
%native (GetCurrentThemeName) MyGetCurrentThemeName;


%init %{
    PyDict_SetItemString(d, "error", PyWinExc_ApiError);
%}
