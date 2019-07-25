/*

    device context data types

    Created July 1994, Mark Hammond (MHammond@cmutual.com.au)

    These are implemented using CDC's, and hDC's in the map

Note that this source file contains embedded documentation.
This documentation consists of marked up text inside the
C comments, and is prefixed with an '@' symbol.  The source
files are processed by a tool called "autoduck" which
generates Windows .hlp files.
@doc

*/
#define PY_SSIZE_T_CLEAN  // this module is Py_ssize_t clean!
#include "stdafx.h"

#include <winspool.h>

#include "win32dc.h"
#include "win32gdi.h"
#include "win32brush.h"
#include "win32font.h"
#include "win32pen.h"
#include "win32bitmap.h"
#include "win32rgn.h"

// LOGPALETTE support.
BOOL PyObject_AsLOGPALETTE(PyObject *obLogPal, LOGPALETTE **ppLogPal)
{
    BOOL ok = FALSE;
    if (!PySequence_Check(obLogPal)) {
        PyErr_SetString(PyExc_TypeError, "LOGPALETTE must be a sequence");
        return FALSE;
    }
    Py_ssize_t n = PySequence_Length(obLogPal);
    *ppLogPal = (LOGPALETTE *)malloc(sizeof(LOGPALETTE) + n * sizeof(PALETTEENTRY));
    LOGPALETTE *pPal = *ppLogPal;
    if (pPal == NULL) {
        PyErr_SetString(PyExc_MemoryError, "Allocating LOGPALETTE");
        return FALSE;
    }

    pPal->palVersion = 0x300;
    pPal->palNumEntries = PyWin_SAFE_DOWNCAST(n, Py_ssize_t, WORD);

    for (Py_ssize_t i = 0; i < n; i++) {
        PyObject *subOb = PySequence_GetItem(obLogPal, i);
        if (subOb == NULL)
            goto done;
        if (!PyArg_ParseTuple(subOb, "bbbb", &pPal->palPalEntry[i].peRed, &pPal->palPalEntry[i].peGreen,
                              &pPal->palPalEntry[i].peBlue, &pPal->palPalEntry[i].peFlags)) {
            Py_XDECREF(subOb);
            goto done;
        }
        Py_XDECREF(subOb);
    }
    ok = TRUE;
done:
    if (!ok) {
        free(pPal);
    }
    return ok;
}

void PyObject_FreeLOGPALETTE(LOGPALETTE *pLogPal)
{
    if (pLogPal)
        free(pLogPal);
}

PyObject *PyObject_FromLOGPALETTE(LOGPALETTE *pLP)
{
    PyObject *entries = PyTuple_New(pLP->palNumEntries);
    for (int i = 0; i < pLP->palNumEntries; i++) {
        PyTuple_SET_ITEM(entries, i,
                         Py_BuildValue("bbbb", pLP->palPalEntry[i].peRed, pLP->palPalEntry[i].peGreen,
                                       pLP->palPalEntry[i].peBlue, pLP->palPalEntry[i].peFlags));
    }
    PyObject *rc = Py_BuildValue("lO", pLP->palVersion, entries);
    Py_DECREF(entries);
    return rc;
}

// @pymethod int|win32ui|CreatePalette|Creates a HPALETTE
PyObject *win32uiCreatePalette(PyObject *self, PyObject *args)
{
    // @pyparm <o LOGPALETTE>|lp||The entries for the palette.
    PyObject *obLP;
    if (!PyArg_ParseTuple(args, "O", &obLP))
        return NULL;
    LOGPALETTE *pLP;
    if (!PyObject_AsLOGPALETTE(obLP, &pLP))
        return NULL;
    HPALETTE hp = CreatePalette(pLP);
    PyObject_FreeLOGPALETTE(pLP);
    if (hp == NULL)
        RETURN_API_ERR("CreatePalette");
    return PyWinLong_FromHANDLE(hp);
}

// this returns a pointer that should not be stored.
CDC *ui_dc_object::GetDC(PyObject *self) { return (CDC *)GetGoodCppObject(self, &type); }

void ui_dc_object::SetAssocInvalid()
{
    return;  // do nothing.  Dont call base as dont want my handle wiped.
}

ui_dc_object::~ui_dc_object()
{
    if (m_deleteDC) {
        CDC *pDC = GetDC(this);
        if (pDC)
            ::DeleteDC(pDC->m_hDC);
    }
}

// @pymethod |win32ui|CreateDC|Creates an uninitialised device context.
PyObject *ui_dc_object::create_dc(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS2(args, CreateDC);
    // create Python device context
    CDC *pDC = new CDC;
    ui_dc_object *dc = (ui_dc_object *)ui_assoc_object::make(ui_dc_object::type, pDC, true)->GetGoodRet();
    if (dc)
        dc->bManualDelete = true;
    return dc;
}

// @pymethod |win32ui|CreateDCFromHandle|Creates a DC object from an integer handle.
PyObject *ui_create_dc_from_handle(PyObject *self, PyObject *args)
{
    HDC hDC;
    if (!PyArg_ParseTuple(args, "O&:CreateDCFromHandle", PyWinObject_AsHANDLE, &hDC))
        return NULL;

    CDC *pDC = CDC::FromHandle(hDC);
    if (pDC == NULL)
        RETURN_ERR("Could not create DC.");

    // create Python device context
    ui_dc_object *dc = (ui_dc_object *)ui_assoc_object::make(ui_dc_object::type, pDC)->GetGoodRet();
    return dc;
}

// @pymethod |PyCDC|BitBlt|Copies a bitmap from the source device context to this device context.
static PyObject *ui_dc_bitblt(PyObject *self, PyObject *args)
{
    CDC *pDC = ui_dc_object::GetDC(self);
    if (!pDC)
        return NULL;
    int x, y, width, height, xsrc, ysrc;
    DWORD rop;
    PyObject *dc_ob;
    if (!PyArg_ParseTuple(
            args, "(ii)(ii)O(ii)i", &x, &y,  // @pyparm (x,y)-ints|destPos||The logical x,y coordinates of the
                                             // upper-left corner of the destination rectangle.
            &width, &height,  // @pyparm (width, height)-ints|size||Specifies the width and height (in logical units) of
                              // the destination rectangle and source bitmap.
            &dc_ob,  // @pyparm <o PyCDC>|dc||Specifies the PyCDC object from which the bitmap will be copied. It must
                     // be None if rop specifies a raster operation that does not include a source.
            &xsrc, &ysrc,  // @pyparm (xSrc, ySrc)-ints|srcPos||Specifies the logical x,y coordinates of the upper-left
                           // corner of the source bitmap.
            &rop))  // @pyparm int|rop||Specifies the raster operation to be performed. See the win32 api documentation
                    // for details.
        return NULL;
    if (!ui_base_class::is_uiobject(dc_ob, &ui_dc_object::type))
        RETURN_TYPE_ERR("The 'O' param must be a PyCDC object");
    CDC *pSrcDC = NULL;
    if (dc_ob != Py_None) {
        pSrcDC = ui_dc_object::GetDC(dc_ob);
        if (!pSrcDC)
            RETURN_ERR("The source DC is invalid");
    }
    GUI_BGN_SAVE;
    BOOL ok = pDC->BitBlt(x, y, width, height, pSrcDC, xsrc, ysrc, rop);
    GUI_END_SAVE;
    if (!ok)  // @pyseemfc CDC|BitBlt
        RETURN_ERR("BitBlt failed");
    RETURN_NONE;
}

// @pymethod |PyCDC|PatBlt|Creates a bit pattern on the device.
static PyObject *ui_dc_patblt(PyObject *self, PyObject *args)
{
    CDC *pDC = ui_dc_object::GetDC(self);
    if (!pDC)
        return NULL;
    int x, y, width, height;
    DWORD rop;
    if (!PyArg_ParseTuple(args, "(ii)(ii)i", &x, &y,  // @pyparm (x,y)-ints|destPos||The logical x,y coordinates of the
                                                      // upper-left corner of the destination rectangle.
                          &width, &height,  // @pyparm (width, height)-ints|size||Specifies the width and height (in
                                            // logical units) of the destination rectangle and source bitmap.
                          &rop))  // @pyparm int|rop||Specifies the raster operation to be performed. See the win32 api
                                  // documentation for details.
        return NULL;
    GUI_BGN_SAVE;
    BOOL ok = pDC->PatBlt(x, y, width, height, rop);
    GUI_END_SAVE;
    if (!ok)  // @pyseemfc CDC|BitBlt
        RETURN_ERR("PatBlt failed");
    RETURN_NONE;
}

// @pymethod |PyCDC|SetPixel|Sets a pixel in a device context
static PyObject *ui_dc_setpixel(PyObject *self, PyObject *args)
{
    CDC *pDC = ui_dc_object::GetDC(self);
    if (!pDC)
        return NULL;
    int x, y;
    long color, rcolor;

    if (!PyArg_ParseTuple(args, "iil",
                          &x,         // @pyparm int|x||Horizontal coordinate.
                          &y,         // @pyparm int|y||Vertical coordinate.
                          &color)) {  // @pyparm int|color||The brush color.
        return NULL;
    }
    GUI_BGN_SAVE;
    rcolor = pDC->SetPixel(x, y, color);
    GUI_END_SAVE;
    if (rcolor < 0)
        RETURN_ERR("SetPixel failed");
    return Py_BuildValue("l", rcolor);
}

// @pymethod int|PyCDC|GetSafeHdc|Returns the HDC of this DC object.
static PyObject *ui_dc_get_safe_hdc(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS(args);
    CDC *pDC = ui_dc_object::GetDC(self);
    if (!pDC)
        return NULL;
    // @pyseemfc CDC|GetSafeHdc
    HDC hdc = pDC->GetSafeHdc();
    return PyWinLong_FromHANDLE(hdc);
}

// @pymethod |PyCDC|GetPixel|Gets a pixel at a local in a device context
static PyObject *ui_dc_get_pixel(PyObject *self, PyObject *args)
{
    CDC *pDC = ui_dc_object::GetDC(self);
    if (!pDC)
        return NULL;
    int x, y;
    long rcolor;

    if (!PyArg_ParseTuple(args, "ii",
                          &x,     // @pyparm int|x||Horizontal coordinate.
                          &y)) {  // @pyparm int|y||Vertical coordinate.
        return NULL;
    }

    GUI_BGN_SAVE;
    rcolor = pDC->GetPixel(x, y);
    GUI_END_SAVE;
    if (rcolor < 0)
        RETURN_ERR("GetPixel failed");
    return Py_BuildValue("l", rcolor);
}

// @pymethod (x, y)|PyCDC|GetCurrentPosition|Retrieves the current position (in logical coordinates).
static PyObject *ui_dc_get_current_position(PyObject *self, PyObject *args)
{
    CDC *pDC = ui_dc_object::GetDC(self);
    if (!pDC)
        return NULL;
    CHECK_NO_ARGS(args);
    CPoint pt;

    pt = pDC->GetCurrentPosition();

    return Py_BuildValue("(ii)", pt.x, pt.y);
}

// @pymethod |PyCDC|Pie|Draws a pie slice in a device context
static PyObject *ui_dc_pie(PyObject *self, PyObject *args)
{
    CDC *pDC = ui_dc_object::GetDC(self);
    if (!pDC)
        return NULL;
    int x1, y1, x2, y2, x3, y3, x4, y4;

    if (!PyArg_ParseTuple(args, "iiiiiiii",
                          &x1,     // @pyparm int|x1||X coordinate of upper left corner
                          &y1,     // @pyparm int|y1||Y coordinate of upper left corner
                          &x2,     // @pyparm int|x2||X coordinate of lower right corner
                          &y2,     // @pyparm int|y2||Y coordinate of lower right corner
                          &x3,     // @pyparm int|x3||X coordinate of starting point of arc
                          &y3,     // @pyparm int|y3||Y coordinate of starting point of arc
                          &x4,     // @pyparm int|x4||X coordinate of ending point of arc
                          &y4)) {  // @pyparm int|y4||Y coordinate of ending point of arc
        return NULL;
    }
    GUI_BGN_SAVE;
    int rc = pDC->Pie(x1, y1, x2, y2, x3, y3, x4, y4);
    GUI_END_SAVE;
    return Py_BuildValue("i", rc);
}

// @pymethod |PyCDC|CreateCompatibleDC|Creates a memory device context that is compatible with this DC.
PyObject *ui_dc_object::create_compatible_dc(PyObject *self, PyObject *args)
{
    // @comm Note that unlike the MFC version, this function
    // calls the global CreateCompatibleDC function and returns
    // a new <o PyCDC> object.
    PyObject *obDCFrom = Py_None;
    CDC *pDC = ui_dc_object::GetDC(self);
    if (!pDC)
        return NULL;
    if (!PyArg_ParseTuple(args, "|O:CreateCompatibleDC", &obDCFrom))
        return NULL;  // @pyparm <o PyCDC>|dcFrom|None|The source DC, or None to make a screen compatible DC.
    CDC *dcFrom = NULL;
    if (obDCFrom != Py_None)
        dcFrom = GetDC(obDCFrom);
    HDC hDC = NULL;
    if (dcFrom) {
        hDC = dcFrom->GetSafeHdc();
    }
    GUI_BGN_SAVE;
    HDC hcDC = ::CreateCompatibleDC(hDC);
    GUI_END_SAVE;
    if (!hcDC)  // @pyseemfc CDC|CreateCompatibleDC
        RETURN_ERR("CreateCompatibleDC failed");
    // create Python device context
    CDC *pcDC = pDC->FromHandle(hcDC);
    ui_dc_object *dc = (ui_dc_object *)ui_assoc_object::make(ui_dc_object::type, pcDC)->GetGoodRet();
    return dc;
}

// @pymethod |PyCDC|CreatePrinterDC|Creates a device context for a specific printer
PyObject *ui_dc_object::create_printer_dc(PyObject *self, PyObject *args)
{
    CDC *pDC = ui_dc_object::GetDC(self);
    if (!pDC)
        return NULL;
    TCHAR *printerName = NULL;
    PyObject *obprinterName = Py_None;
    if (!PyArg_ParseTuple(args, "|O:CreatePrinterDC", &obprinterName))
        return NULL;  // @pyparm string|printerName|None|The printer name, or None for the default printer
    if (!PyWinObject_AsTCHAR(obprinterName, &printerName, TRUE))
        return NULL;
    BOOL result;
    if (printerName == NULL) {
        // Attempt to open the default printer
        CPrintInfo info;
        if (!AfxGetApp()->GetPrinterDeviceDefaults(&(info.m_pPD->m_pd))) {
            RETURN_ERR("No default printer found");
            return NULL;
        }

        if (info.m_pPD->m_pd.hDC == NULL && !info.m_pPD->CreatePrinterDC()) {
            result = FALSE;
        }
        else {
            result = pDC->Attach(info.m_pPD->m_pd.hDC);
            info.m_pPD->m_pd.hDC = NULL;  // Prevent this DC from being deleted
        }
    }
    else {
        // Attempt to open a specific printer
        HANDLE hPrinter;
        if (!::OpenPrinter(printerName, &hPrinter, NULL)) {
            PyWinObject_FreeTCHAR(printerName);
            RETURN_ERR("Unable to open printer");
            return NULL;
        }
        PyWinObject_FreeTCHAR(printerName);

        DWORD len;
        unsigned char buf;
        ::GetPrinter(hPrinter, 2, &buf, 1, &len);
        unsigned char *buffer = new unsigned char[len];
        result = ::GetPrinter(hPrinter, 2, buffer, len, &len);
        ::ClosePrinter(hPrinter);
        if (!result) {
            RETURN_ERR("Unable to get printer info");
            delete[] buffer;
            return NULL;
        }

        PRINTER_INFO_2 *pinfo = (PRINTER_INFO_2 *)buffer;
        GUI_BGN_SAVE;
        result = pDC->CreateDC(pinfo->pDriverName, pinfo->pPrinterName, NULL, NULL);  // @pyseemfc CDC|CreateDC
        GUI_END_SAVE;
        delete[] buffer;
    }

    if (!result)
        RETURN_ERR("CreateDC failed");
    RETURN_NONE;
}

// @pymethod |PyCDC|DeleteDC|Deletes all resources associated with a device context.
static PyObject *ui_dc_delete_dc(PyObject *self, PyObject *args)
{
    CDC *pDC = ui_dc_object::GetDC(self);
    if (!pDC)
        return NULL;
    CHECK_NO_ARGS2(args, DeleteDC);
    GUI_BGN_SAVE;
    BOOL ok = pDC->DeleteDC();
    GUI_END_SAVE;
    if (!ok)
        RETURN_ERR("DeleteDC failed");
    RETURN_NONE;
    // @comm In general, do not call this function; the destructor will do it for you.
    // <nl>An application should not call DeleteDC if objects have been selected into the device context. Objects must
    // first be selected out of the device context before it it is deleted. <nl>An application must not delete a device
    // context whose handle was obtained by calling CWnd::GetDC. Instead, it must call CWnd::ReleaseDC to free the
    // device context. <nl>The DeleteDC function is generally used to delete device contexts created with CreateDC,
    // CreateIC, or CreateCompatibleDC.
}

// @pymethod |PyCDC|DrawIcon|Draws an icon on the DC.
static PyObject *ui_dc_draw_icon(PyObject *self, PyObject *args)
{
    CDC *pDC = ui_dc_object::GetDC(self);
    if (!pDC)
        return NULL;

    int x, y;
    HICON hIcon;
    if (!PyArg_ParseTuple(args, "(ii)O&:DrawIcon", &x, &y,  // @pyparm (x,y)|point||The point coordinate to draw to.
                          PyWinObject_AsHANDLE, &hIcon))  // @pyparm <o PyHANDLE>|hIcon||The handle of the icon to draw.
        return NULL;

    GUI_BGN_SAVE;
    BOOL ok = pDC->DrawIcon(x, y, hIcon);
    GUI_END_SAVE;
    if (!ok)  // @pyseemfc CDC|DrawIcon
        RETURN_ERR("DrawIcon failed");
    else
        RETURN_NONE;
}
// @pymethod |PyCDC|DrawFocusRect|Draws a rectangle in the style used to
// indicate the rectangle has focus
static PyObject *ui_dc_draw_focus_rect(PyObject *self, PyObject *args)
{
    CDC *pDC = ui_dc_object::GetDC(self);
    CRect rect;
    if (!pDC) {
        return NULL;
        // @pyparm (left, top, right, bottom)|rect||The coordinates of the
        // rectangle
    }
    else if (!PyArg_ParseTuple(args, "(iiii)", &rect.left, &rect.top, &rect.right, &rect.bottom)) {
        return NULL;
    }
    else {
        // it's a void function
        GUI_BGN_SAVE;
        pDC->DrawFocusRect(rect);  // @pyseemfc CDC|DrawFocusRect
        GUI_END_SAVE;
        RETURN_NONE;
    }
}

//@pymethod |PyCDC|ExtTextOut|Writes text to the DC.
static PyObject *ui_dc_ext_text_out(PyObject *self, PyObject *args)
{
    CDC *pDC = ui_dc_object::GetDC(self);
    if (!pDC)
        return NULL;
    TCHAR *text;
    DWORD strLen;
    int x, y;
    UINT options;
    PyObject *obtext, *rectObject, *widthObject = NULL;
    RECT rect, *rectPtr;
    int *widths = NULL;
    if (!PyArg_ParseTuple(args, "iiiOO|O:ExtTextOut",
                          &x,        // @pyparm x|int||The x coordinate to write the text to.
                          &y,        // @pyparm y|int||The y coordinate to write the text to.
                          &options,  // @pyparm nOptions|int||Specifies the rectangle type. This parameter can be one,
                                     // both, or neither of ETO_CLIPPED and ETO_OPAQUE
                          &rectObject,    // @pyparm (left, top, right, bottom)|rect||Specifies the text's bounding
                                          // rectangle.  (Can be None.)
                          &obtext,        // @pyparm text|string||The text to write.
                          &widthObject))  // @pyparm (width1, width2, ...)|tuple||Optional array of values that indicate
                                          // distance between origins of character cells.
    {
        return NULL;
    }

    // Parse out rectangle object
    if (rectObject != Py_None) {
        if (!PyArg_ParseTuple(rectObject, "iiii", &rect.left, &rect.top, &rect.right, &rect.bottom))
            return NULL;
        rectPtr = &rect;
    }
    else
        rectPtr = NULL;

    if (!PyWinObject_AsTCHAR(obtext, &text, FALSE, &strLen))
        return NULL;

    // Parse out widths
    if (widthObject) {
        BOOL error = !PyTuple_Check(widthObject);
        if (!error) {
            Py_ssize_t len = PyTuple_Size(widthObject);
            if (len == (strLen - 1)) {
                widths = new int[len + 1];
                for (Py_ssize_t i = 0; i < len; i++) {
                    PyObject *item = PyTuple_GetItem(widthObject, i);
                    if (!PyInt_Check(item))
                        error = TRUE;
                    else
                        widths[i] = PyInt_AsLong(item);
                }
            }
        }
        if (error) {
            delete[] widths;
            PyWinObject_FreeTCHAR(text);
            RETURN_TYPE_ERR(
                "The width param must be a tuple of integers with a length one less than that of the string");
        }
    }

    GUI_BGN_SAVE;
    BOOL ret = pDC->ExtTextOut(x, y, options, rectPtr, text, strLen, widths);
    // @pyseemfc CDC|ExtTextOut
    GUI_END_SAVE;
    PyWinObject_FreeTCHAR(text);
    delete[] widths;
    if (!ret) {
        RETURN_API_ERR("CDC::TextOut");
    }
    RETURN_NONE;
    // @rdesc Always none.  If the function fails, an exception is raised.
}

// @pymethod int|PyCDC|RectVisible|Determines whether any part of the given rectangle lies within the clipping region of
// the display context.
static PyObject *ui_dc_rect_visible(PyObject *self, PyObject *args)
{
    CDC *pDC = ui_dc_object::GetDC(self);
    if (!pDC)
        return NULL;
    CRect rect;
    // @pyparm (left, top, right, bottom)|rect||The coordinates of the reactangle to be checked.
    if (!PyArg_ParseTuple(args, "(iiii):RectVisible", &rect.left, &rect.top, &rect.right, &rect.bottom))
        return NULL;
    GUI_BGN_SAVE;
    int rc = pDC->RectVisible(&rect);
    GUI_END_SAVE;
    return Py_BuildValue("i", rc);  // @pyseemfc CDC|RectVisible
    // @rdesc Non zero if any part of the rectangle lies within the clipping region, else zero.
}

// @pymethod |PyCDC|Arc|Draws an eliptical arc.
static PyObject *ui_dc_arc(PyObject *self, PyObject *args)
{
    CDC *pDC = ui_dc_object::GetDC(self);
    if (!pDC)
        return NULL;
    CRect rect;
    POINT pts, pte;
    if (!PyArg_ParseTuple(args, "(iiii)(ii)(ii):Arc",
                          // @pyparm (left, top, right, bottom)|rect||Specifies the ellipse's bounding rectangle
                          &rect.left, &rect.top, &rect.right, &rect.bottom,
                          // @pyparm (x,y)|pointStart||Specifies the x- and y-coordinates
                          // of the point that defines the arc's starting point (in logical units).
                          // This point does not have to lie exactly on the arc.
                          &pts.x, &pts.y,
                          // @pyparm (x,y)|pointEnd||Specifies the x- and y-coordinates
                          // of the point that defines the arc's ending point (in logical units).
                          // This point does not have to lie exactly on the arc.
                          &pte.x, &pte.y))
        return NULL;
    GUI_BGN_SAVE;
    BOOL ret = pDC->Arc(&rect, pts, pte);  // @pyseemfc CDC|Arc
    GUI_END_SAVE;
    if (!ret)
        RETURN_API_ERR("CDC::Arc");
    RETURN_NONE;
    // @rdesc Always none.  If the function fails, an exception is raised.
    // @comm The arc drawn by using the function is a segment of the ellipse defined by the specified bounding
    // rectangle. The actual starting point of the arc is the point at which a ray drawn from the center of the bounding
    // rectangle through the specified starting point intersects the ellipse. The actual ending point of the arc is the
    // point at which a ray drawn from the center of the bounding rectangle through
    // the specified ending point intersects the ellipse. The arc is drawn in a
    // counterclockwise direction. Since an arc is not a closed figure, it is
    // not filled. Both the width and height of the rectangle must be greater
    // than 2 units and less than 32,767 units.
}

// @pymethod |PyCDC|Chord|Draws a chord.
static PyObject *ui_dc_chord(PyObject *self, PyObject *args)
{
    CDC *pDC = ui_dc_object::GetDC(self);
    if (!pDC)
        return NULL;
    CRect rect;
    POINT pts, pte;
    if (!PyArg_ParseTuple(args, "(iiii)(ii)(ii):Chord",
                          // @pyparm (left, top, right, bottom)|rect||Specifies the ellipse's bounding rectangle
                          &rect.left, &rect.top, &rect.right, &rect.bottom,
                          // @pyparm (x,y)|pointStart||Specifies the x- and y-coordinates
                          // of the point that defines the arc's starting point (in logical units).
                          // This point does not have to lie exactly on the arc.
                          &pts.x, &pts.y,
                          // @pyparm (x,y)|pointEnd||Specifies the x- and y-coordinates
                          // of the point that defines the arc's ending point (in logical units).
                          // This point does not have to lie exactly on the arc.
                          &pte.x, &pte.y))
        return NULL;
    GUI_BGN_SAVE;
    BOOL ret = pDC->Chord(&rect, pts, pte);  // @pyseemfc CDC|Chord
    GUI_END_SAVE;
    if (!ret)
        RETURN_API_ERR("CDC::Chord");
    RETURN_NONE;
    // @rdesc Always none.  If the function fails, an exception is raised.
    // @comm Draws a chord (a closed figure bounded by the intersection
    // of an ellipse and a line segment). The rect parameter specify the
    // upper-left and lower-right corners, respectively, of a rectangle
    // bounding the ellipse that is part of the chord.
    // The pointStart and pointEnd parameters specify
    // the endpoints of a line that intersects the ellipse.
    // The chord is drawn by using the selected pen and filled
    // by using the selected brush.
}

// @pymethod |PyCDC|Ellipse|Draws an Ellipse.
static PyObject *ui_dc_ellipse(PyObject *self, PyObject *args)
{
    CDC *pDC = ui_dc_object::GetDC(self);
    if (!pDC)
        return NULL;
    CRect rect;
    // @pyparm (left, top, right, bottom)|rect||Specifies the ellipse's bounding rectangle
    if (!PyArg_ParseTuple(args, "(iiii):Ellipse", &rect.left, &rect.top, &rect.right, &rect.bottom))
        return NULL;
    GUI_BGN_SAVE;
    BOOL ret = pDC->Ellipse(rect);  // @pyseemfc CDC|Ellipse
    GUI_END_SAVE;
    if (!ret)
        RETURN_API_ERR("CDC::Ellipse");
    RETURN_NONE;
    // @rdesc Always none.  If the function fails, an exception is raised.
    // @comm The center of the ellipse is the center of the bounding rectangle
    // specified by rect. The ellipse is drawn with the current pen, and its
    // interior is filled with the current brush.
}

// @pymethod |PyCDC|Polygon|Draws an Polygon.
static PyObject *ui_dc_polygon(PyObject *self, PyObject *args)
{
    PyObject *point_list;
    CDC *pDC = ui_dc_object::GetDC(self);
    if (!pDC)
        return NULL;
    if (!PyArg_ParseTuple(args, "O:Polygon", &point_list)) {
        return NULL;
    }
    else if (!PyList_Check(point_list)) {
        return NULL;
    }
    else {
        // Convert the list of point tuples into an array of POINT structs
        Py_ssize_t num = PyList_Size(point_list);
        POINT *point_array = new POINT[num];
        for (Py_ssize_t i = 0; i < num; i++) {
            PyObject *point_tuple = PyList_GetItem(point_list, i);
            if (!PyTuple_Check(point_tuple) || PyTuple_Size(point_tuple) != 2) {
                PyErr_SetString(PyExc_ValueError, "point list must be a list of (x,y) tuples");
                delete[] point_array;
                return NULL;
            }
            else {
                long x, y;
                PyObject *px, *py;
                px = PyTuple_GetItem(point_tuple, 0);
                py = PyTuple_GetItem(point_tuple, 1);
                if ((!PyInt_Check(px)) || (!PyInt_Check(py))) {
                    PyErr_SetString(PyExc_ValueError, "point list must be a list of (x,y) tuples");
                    delete[] point_array;
                    return NULL;
                }
                else {
                    x = PyInt_AsLong(px);
                    y = PyInt_AsLong(py);
                    point_array[i].x = x;
                    point_array[i].y = y;
                }
            }
        }
        // we have an array of POINT structs, now we
        // can finally draw the polygon.
        GUI_BGN_SAVE;
        BOOL ret = pDC->Polygon(point_array, PyWin_SAFE_DOWNCAST(num, Py_ssize_t, int));
        GUI_END_SAVE;
        delete[] point_array;
        if (!ret) {
            RETURN_API_ERR("CDC::Polygon");
        }
        else {
            RETURN_NONE;
        }
    }
}

// @pymethod |PyCDC|PolyBezier|Draws one or more Bezier splines.
static PyObject *ui_dc_poly_bezier(PyObject *self, PyObject *args)
{
    PyObject *triple_list;
    int do_to = 0;
    CDC *pDC = ui_dc_object::GetDC(self);
    if (!pDC)
        return NULL;
    if (!PyArg_ParseTuple(args, "O|i:PolyBezier[To]", &triple_list, &do_to)) {
        return NULL;
    }
    else if (!PyList_Check(triple_list)) {
        return NULL;
    }
    else {
        int index = 0;
        Py_ssize_t num = PyList_Size(triple_list);

#define HURL                                                                                 \
    do {                                                                                     \
        PyErr_SetString(PyExc_ValueError, "arg must be a list of 3-tuples of (x,y) tuples"); \
        delete[] point_array;                                                                \
        return NULL;                                                                         \
    } while (0)

        POINT *point_array = new POINT[num * 3];
        for (Py_ssize_t i = 0; i < num; i++) {
            PyObject *triplet = PyList_GetItem(triple_list, i);
            if (!PyTuple_Check(triplet) || PyTuple_Size(triplet) != 3) {
                HURL;
            }
            else {
                for (int j = 0; j < 3; j++) {
                    PyObject *point = PyTuple_GetItem(triplet, j);
                    if (!PyTuple_Check(point) || PyTuple_Size(point) != 2) {
                        HURL;
                    }
                    else {
                        PyObject *px, *py;
                        px = PyTuple_GetItem(point, 0);
                        py = PyTuple_GetItem(point, 1);
                        if (!PyInt_Check(px) || !PyInt_Check(py)) {
                            HURL;
                        }
                        else {
                            point_array[index].x = PyInt_AsLong(px);
                            point_array[index].y = PyInt_AsLong(py);
                            index++;
                        }
                    }
                }
            }
        }
        // we have an array of POINT structs, now we
        // can finally draw the splines..
        BOOL result;
        if (do_to) {
            result = pDC->PolyBezierTo(point_array, index);
        }
        else {
            result = pDC->PolyBezier(point_array, index);
        }
        delete[] point_array;
        if (!result) {
            RETURN_API_ERR("CDC::PolyBezier[To]");
        }
        else {
            RETURN_NONE;
        }
    }
}

// @pymethod |PyCDC|FillRect|Fills a given rectangle with the specified brush
static PyObject *ui_dc_fillrect(PyObject *self, PyObject *args)
{
    CDC *pDC = ui_dc_object::GetDC(self);
    if (!pDC)
        return NULL;
    RECT rect;
    PyObject *obBrush;
    if (!PyArg_ParseTuple(args, "(iiii)O:FillRect", &rect.left, &rect.top, &rect.right, &rect.bottom,
                          // @pyparm (left, top, right, bottom|rect||Specifies the bounding rectangle, in logical units.
                          &obBrush))  // @pyparm <o PyCBrush>|brush||Specifies the brush to use.
        return NULL;
    if (!ui_base_class::is_uiobject(obBrush, &PyCBrush::type))
        RETURN_TYPE_ERR("The 'O' param must be a PyCBrush object");
    CBrush *pBrush = PyCBrush::GetBrush(obBrush);
    if (!pBrush)
        return NULL;
    GUI_BGN_SAVE;
    pDC->FillRect(&rect, pBrush);
    GUI_END_SAVE;
    // @pyseemfc CDC|FillRect
    RETURN_NONE;
}

// @pymethod |PyCDC|FillSolidRect|Fills the given rectangle with the specified solid color.
static PyObject *ui_dc_fillsolidrect(PyObject *self, PyObject *args)
{
    CDC *pDC = ui_dc_object::GetDC(self);
    if (!pDC)
        return NULL;
    RECT rect;
    int col;
    if (!PyArg_ParseTuple(args, "(iiii)i:FillSolidRect", &rect.left, &rect.top, &rect.right, &rect.bottom,
                          // @pyparm (left, top, right, bottom|rect||Specifies the bounding rectangle, in logical units.
                          &col))  // @pyparm int|color||Specifies the color to use.
        return NULL;
    GUI_BGN_SAVE;
    pDC->FillSolidRect(&rect, (COLORREF)col);
    GUI_END_SAVE;
    // @pyseemfc CDC|FillSolidRect
    RETURN_NONE;
}

// @pymethod |PyCDC|FrameRect|Draws a border around the rectangle specified by rect
static PyObject *ui_dc_framerect(PyObject *self, PyObject *args)
{
    CDC *pDC = ui_dc_object::GetDC(self);
    if (!pDC)
        return NULL;
    RECT rect;
    PyObject *obBrush;
    if (!PyArg_ParseTuple(args, "(iiii)O:FrameRect", &rect.left, &rect.top, &rect.right, &rect.bottom,
                          // @pyparm (left, top, right, bottom|rect||Specifies the bounding rectangle, in logical units.
                          &obBrush))  // @pyparm <o PyCBrush>|brush||Specifies the brush to use.
        return NULL;
    if (!ui_base_class::is_uiobject(obBrush, &PyCBrush::type))
        RETURN_TYPE_ERR("The 'O' param must be a PyCBrush object");
    CBrush *pBrush = PyCBrush::GetBrush(obBrush);
    if (!pBrush)
        RETURN_ERR("The PyCBrush parameter is invalid.");
    GUI_BGN_SAVE;
    pDC->FrameRect(&rect, pBrush);
    GUI_END_SAVE;
    // @pyseemfc CDC|FrameRect
    RETURN_NONE;
}

// @pymethod |PyCDC|Draw3dRect|Draws a three-dimensional rectangle.
static PyObject *ui_dc_draw3drect(PyObject *self, PyObject *args)
{
    CDC *pDC = ui_dc_object::GetDC(self);
    if (!pDC)
        return NULL;
    RECT rect;
    int ctl, cbr;
    if (!PyArg_ParseTuple(args, "(iiii)ii:Draw3dRect", &rect.left, &rect.top, &rect.right, &rect.bottom,
                          // @pyparm (left, top, right, bottom|rect||Specifies the bounding rectangle, in logical units.
                          &ctl,   // @pyparm int|colorTopLeft||Specifies the color of the top and left sides of the
                                  // three-dimensional rectangle.
                          &cbr))  // @pyparm int|colorBotRight||Specifies the color of the bottom and right sides of the
                                  // three-dimensional rectangle.
        return NULL;
    GUI_BGN_SAVE;
    pDC->Draw3dRect(&rect, ctl, cbr);
    GUI_END_SAVE;
    // @pyseemfc CDC|Draw3dRect
    RETURN_NONE;
}

// @pymethod int|PyCDC|GetNearestColor|Returns the closest color a device can map.
static PyObject *ui_dc_get_nearest_color(PyObject *self, PyObject *args)
{
    CDC *pDC = ui_dc_object::GetDC(self);
    if (!pDC)
        return NULL;
    int col;
    // @pyparm int|color||Specifies the color to be matched.
    if (!PyArg_ParseTuple(args, "i:GetNearestColor", &col))
        return NULL;
    GUI_BGN_SAVE;
    int rc = pDC->GetNearestColor(col);
    GUI_END_SAVE;
    return Py_BuildValue("i", rc);
}

// @pymethod (x,y)|PyCDC|GetTextExtentPoint|An alias for <om PyCDC.GetTextExtent>.
// GetTextExtentPoint is the preferred win32api name, but GetTextExtent is the MFC name.<nl>
// Calculates the width and height of a line of text using the current font to determine the dimensions.
// @pyparm string|text||The text to calculate for.
// @rdesc A tuple of integers with the size of the string, in logical units.

// @pymethod (x,y)|PyCDC|GetTextExtent|Calculates the width and height of a line of text using the current font to
// determine the dimensions.
static PyObject *ui_dc_get_text_extent(PyObject *self, PyObject *args)
{
    CDC *pDC = ui_dc_object::GetDC(self);
    if (!pDC)
        return NULL;
    TCHAR *text;
    PyObject *obtext;
    DWORD strLen;
    // @pyparm string|text||The text to calculate for.
    if (!PyArg_ParseTuple(args, "O:GetTextExtent", &obtext))
        return NULL;
    if (!PyWinObject_AsTCHAR(obtext, &text, FALSE, &strLen))
        return NULL;
    GUI_BGN_SAVE;
    CSize sz = pDC->GetTextExtent(text, strLen);
    // @pyseemfc CFC|GetTextExtent
    GUI_END_SAVE;
    PyWinObject_FreeTCHAR(text);
    return Py_BuildValue("(ii)", sz.cx, sz.cy);
    // @rdesc A tuple of integers with the size of the string, in logical units.
}

// @pymethod int|PyCDC|SetTextColor|Sets the text color to the specified color.
static PyObject *ui_dc_set_text_color(PyObject *self, PyObject *args)
{
    // @comm This text color is used when writing text to this device context and also when converting bitmaps between
    // color and monochrome device contexts. If the device cannot represent the specified color, the system sets the
    // text color to the nearest physical color. The background color for a character is specified by the SetBkColor and
    // SetBkMode member functions.
    CDC *pDC = ui_dc_object::GetDC(self);
    if (!pDC)
        return NULL;

    int new_color;
    // @pyparm int|color||A windows color specification.  See the win32api documentation for details.
    if (!PyArg_ParseTuple(args, "i", &new_color))
        return NULL;

    GUI_BGN_SAVE;
    int old_color = pDC->SetTextColor(new_color);  // @pyseemfc CDC|SetTextColor
    GUI_END_SAVE;
    return Py_BuildValue("i", old_color);
    // @rdesc The return value is the previous text color.
}

// @pymethod int|PyCDC|SetBkColor|Sets the current background color to the specified color.
static PyObject *ui_dc_set_bk_color(PyObject *self, PyObject *args)
{
    // @comm If the background mode is OPAQUE, the system uses the background color
    // to fill the gaps in styled lines, the gaps between hatched lines in brushes, and
    // the background in character cells.
    // The system also uses the background color when converting bitmaps between color and
    // monochrome device contexts.
    CDC *pDC = ui_dc_object::GetDC(self);
    if (!pDC)
        return NULL;
    int new_color;
    if (!PyArg_ParseTuple(args, "i", &new_color))  // @pyparm int|color||A windows color specification.  See the
                                                   // win32api documentation for details.
        return NULL;
    GUI_BGN_SAVE;
    int old_color = pDC->SetBkColor(new_color);  // @pyseemfc CDC|SetBkColor
    GUI_END_SAVE;
    return Py_BuildValue("i", old_color);
    // @rdesc The return value is the previous background color.
}

// @pymethod int|PyCDC|SetBkMode|Sets the current background mode to the specified mode.
static PyObject *ui_dc_set_bk_mode(PyObject *self, PyObject *args)
{
    // @comm Specifies the mode to be set.  This parameter can be either OPAQUE or TRANSPARENT
    CDC *pDC = ui_dc_object::GetDC(self);
    if (!pDC)
        return NULL;
    int new_mode;
    if (!PyArg_ParseTuple(args, "i",
                          &new_mode))  // @pyparm int|mode||A background mode.  May be either TRANSPARENT or OPAQUE.
        return NULL;
    GUI_BGN_SAVE;
    int old_mode = pDC->SetBkMode(new_mode);  // @pyseemfc CDC|SetBkMode
    GUI_END_SAVE;
    return Py_BuildValue("i", old_mode);
    // @rdesc The return value is the previous background mode.
}

// @pymethod (int, int)|PyCDC|SetBrushOrg|Specifies the origin that GDI will assign to the next brush that the
// application selects into the device context.
static PyObject *ui_dc_set_brush_org(PyObject *self, PyObject *args)
{
    CDC *pDC = ui_dc_object::GetDC(self);
    if (!pDC)
        return NULL;
    int x, y;
    // @pyparm (x,y)|point||The new origin in device units.
    if (!PyArg_ParseTuple(args, "(ii)", &x, &y))
        return NULL;
    GUI_BGN_SAVE;
    CPoint pt = pDC->SetBrushOrg(x, y);  // @pyseemfc CDC|SetBrushOrg
    GUI_END_SAVE;
    return Py_BuildValue("(ii)", pt.x, pt.y);
    // @rdesc The previous origin in device units.
}

// @pymethod (int,int)|PyCDC|GetBrushOrg|Retrieves the origin (in device units) of the brush currently selected for the
// device context.
static PyObject *ui_dc_get_brush_org(PyObject *self, PyObject *args)
{
    CDC *pDC = ui_dc_object::GetDC(self);
    if (!pDC)
        return NULL;
    int item;
    if (!PyArg_ParseTuple(args, ":GetBrushOrg", &item))
        return NULL;
    GUI_BGN_SAVE;
    CPoint pt = pDC->GetBrushOrg();  // @pyseemfc CDC|GetBrushOrg
    GUI_END_SAVE;
    return Py_BuildValue("ii", pt.x, pt.y);
}

// @pymethod int|PyCDC|GetDeviceCaps|Retrieves a capability of the device context.
static PyObject *ui_dc_get_device_caps(PyObject *self, PyObject *args)
{
    CDC *pDC = ui_dc_object::GetDC(self);
    if (!pDC)
        return NULL;
    int item;
    if (!PyArg_ParseTuple(
            args, "i",
            &item))  // @pyparm int|index||The information requested.  See the win32api documentation for details.
        return NULL;
    GUI_BGN_SAVE;
    int value = pDC->GetDeviceCaps(item);  // @pyseemfc CDC|GetDeviceCaps
    GUI_END_SAVE;
    return Py_BuildValue("i", value);
    // @rdesc The value of the requested capability
}

// @pymethod int|PyCDC|SetMapMode|Sets the mapping mode for the device context.
static PyObject *ui_dc_set_map_mode(PyObject *self, PyObject *args)
{
    CDC *pDC = ui_dc_object::GetDC(self);
    if (!pDC)
        return NULL;
    int new_mode;
    if (!PyArg_ParseTuple(args, "i", &new_mode))
        // @pyparm int|newMode||The new mode.  Can be one of
        // MM_ANISOTROPIC, MM_HIENGLISH, MM_HIMETRIC, MM_ISOTROPIC, MM_LOENGLISH, MM_LOMETRIC, MM_TEXT, MM_TWIPS
        return NULL;
    GUI_BGN_SAVE;
    int old_mode = pDC->SetMapMode(new_mode);  // @pyseemfc CDC|SetMapMode
    GUI_END_SAVE;
    if (old_mode == 0)
        RETURN_ERR("SetMapMode failed");
    else
        return Py_BuildValue("i", old_mode);
    // @rdesc The previous mapping mode.
}

// @pymethod int|PyCDC|GetMapMode|Gets the mapping mode for the device context.
static PyObject *ui_dc_get_map_mode(PyObject *self, PyObject *args)
{
    CDC *pDC = ui_dc_object::GetDC(self);
    if (!pDC)
        return NULL;
    if (!PyArg_ParseTuple(args, ""))
        return NULL;
    GUI_BGN_SAVE;
    int mode = pDC->GetMapMode();  // @pyseemfc CDC|GetMapMode
    GUI_END_SAVE;
    if (mode == 0)
        RETURN_ERR("GetMapMode failed");
    return PyInt_FromLong(mode);
}

// @pymethod x, y|PyCDC|SetWindowOrg|Sets the window origin of the device context
static PyObject *ui_dc_set_window_org(PyObject *self, PyObject *args)
{
    CDC *pDC = ui_dc_object::GetDC(self);
    if (!pDC) {
        return NULL;
    }
    else {
        int x, y;
        // @pyparm int, int|x,y||The new origin.
        if (!PyArg_ParseTuple(args, "(ii)", &x, &y)) {
            return NULL;
        }
        else {
            GUI_BGN_SAVE;
            CSize old_size = pDC->SetWindowOrg(x, y);
            GUI_END_SAVE;
            return Py_BuildValue("(ii)", old_size.cx, old_size.cy);
        }
    }
}

// @pymethod x, y|PyCDC|GetWindowOrg|Retrieves the x- and y-coordinates of the origin of the window associated with the
// device context.
static PyObject *ui_dc_get_window_org(PyObject *self, PyObject *args)
{
    CDC *pDC = ui_dc_object::GetDC(self);
    if (!pDC) {
        return NULL;
    }
    else {
        if (!PyArg_ParseTuple(args, "")) {
            return NULL;
        }
        else {
            GUI_BGN_SAVE;
            CSize org = pDC->GetWindowOrg();
            GUI_END_SAVE;
            return Py_BuildValue("(ii)", org.cx, org.cy);
        }
    }
}

// @pymethod x, y|PyCDC|SetViewportOrg|Sets the viewport origin of the device context
static PyObject *ui_dc_set_viewport_org(PyObject *self, PyObject *args)
{
    CDC *pDC = ui_dc_object::GetDC(self);
    if (!pDC) {
        return NULL;
    }
    else {
        int x, y;
        // @pyparm int, int|x,y||The new origin.
        if (!PyArg_ParseTuple(args, "(ii)", &x, &y)) {
            return NULL;
        }
        else {
            GUI_BGN_SAVE;
            CSize old_size = pDC->SetViewportOrg(x, y);
            GUI_END_SAVE;
            return Py_BuildValue("(ii)", old_size.cx, old_size.cy);
        }
    }
}

// @pymethod x, y|PyCDC|GetViewportOrg|Gets the viewport origin of the device context
static PyObject *ui_dc_get_viewport_org(PyObject *self, PyObject *args)
{
    CDC *pDC = ui_dc_object::GetDC(self);
    if (!pDC) {
        return NULL;
    }
    else {
        if (!PyArg_ParseTuple(args, "")) {
            return NULL;
        }
        else {
            GUI_BGN_SAVE;
            CSize org = pDC->GetViewportOrg();
            GUI_END_SAVE;
            return Py_BuildValue("(ii)", org.cx, org.cy);
        }
    }
}

// @pymethod x, y|PyCDC|GetViewportExt|Gets the viewport extent of the device context
static PyObject *ui_dc_get_viewport_ext(PyObject *self, PyObject *args)
{
    CDC *pDC = ui_dc_object::GetDC(self);
    if (!pDC) {
        return NULL;
    }
    else {
        if (!PyArg_ParseTuple(args, "")) {
            return NULL;
        }
        else {
            GUI_BGN_SAVE;
            CSize ext = pDC->GetViewportExt();
            GUI_END_SAVE;
            return Py_BuildValue("(ii)", ext.cx, ext.cy);
        }
    }
}

// @pymethod x, y|PyCDC|GetWindowExt|Gets the window extent of the device context
static PyObject *ui_dc_get_window_ext(PyObject *self, PyObject *args)
{
    CDC *pDC = ui_dc_object::GetDC(self);
    if (!pDC) {
        return NULL;
    }
    else {
        if (!PyArg_ParseTuple(args, "")) {
            return NULL;
        }
        else {
            GUI_BGN_SAVE;
            CSize ext = pDC->GetWindowExt();
            GUI_END_SAVE;
            return Py_BuildValue("(ii)", ext.cx, ext.cy);
        }
    }
}

// @pymethod int|PyCDC|SetGraphicsMode|Sets the graphics mode for the specified device context
static PyObject *ui_dc_set_graphics_mode(PyObject *self, PyObject *args)
{
    CDC *pDC = ui_dc_object::GetDC(self);
    if (!pDC) {
        return NULL;
    }
    else {
        int mode;
        // @pyparm int|mode||The new mode.
        if (!PyArg_ParseTuple(args, "i", &mode)) {
            return NULL;
        }
        else {
            return Py_BuildValue("i", SetGraphicsMode(pDC->GetSafeHdc(), mode));
        }
    }
}

// @pymethod int|PyCDC|SetWorldTransform|sets a two-dimensional linear transformation between world space and page space
// for the specified device context. This transformation can be used to scale, rotate, shear, or translate graphics
// output.
static PyObject *ui_dc_set_world_transform(PyObject *self, PyObject *args)
{
    CDC *pDC = ui_dc_object::GetDC(self);
    if (!pDC) {
        return NULL;
    }
    else {
        XFORM xf;
        if (!PyArg_ParseTuple(args, "ffffff", &xf.eM11, &xf.eM12, &xf.eM21, &xf.eM22, &xf.eDx, &xf.eDy)) {
            return NULL;
        }
        else {
            return Py_BuildValue("i", SetWorldTransform(pDC->GetSafeHdc(), &xf));
        }
    }
}

// @pymethod (x,y)|PyCDC|SetWindowExt|Sets the x,y extents of the window associated with the device context.
static PyObject *ui_dc_set_window_ext(PyObject *self, PyObject *args)
{
    CDC *pDC = ui_dc_object::GetDC(self);
    if (!pDC)
        return NULL;
    int x, y;
    // @pyparm (x,y)|size||The new size.
    if (!PyArg_ParseTuple(args, "(ii)", &x, &y))
        return NULL;
    GUI_BGN_SAVE;
    CSize old_size = pDC->SetWindowExt(x, y);  // @pyseemfc CDC|SetWindowExt
    GUI_END_SAVE;
    if (old_size.cx == 0 && old_size.cy == 0)
        RETURN_ERR("SetWindowExt failed");
    else
        return Py_BuildValue("(ii)", old_size.cx, old_size.cy);
    // @rdesc The previous extents of the window (in logical units).
}

// @pymethod (x,y)|PyCDC|SetViewportExt|Sets the x,y extents of the viewport of the device context.
static PyObject *ui_dc_set_viewport_ext(PyObject *self, PyObject *args)
{
    CDC *pDC = ui_dc_object::GetDC(self);
    if (!pDC)
        return NULL;
    int x, y;
    // @pyparm (x,y)|size||The new size.
    if (!PyArg_ParseTuple(args, "(ii)", &x, &y))
        return NULL;
    GUI_BGN_SAVE;
    CSize old_size = pDC->SetViewportExt(x, y);  // @pyseemfc CDC|SetViewportExt
    GUI_END_SAVE;
    if (old_size.cx == 0 && old_size.cy == 0)
        RETURN_ERR("SetViewportExt failed");
    else
        return Py_BuildValue("(ii)", old_size.cx, old_size.cy);
    // @rdesc The previous extents of the viewport (in logical units).
}

// @pymethod int|PyCDC|SetTextAlign|Sets the text-alignment flags.
static PyObject *ui_dc_set_text_align(PyObject *self, PyObject *args)
{
    CDC *pDC = ui_dc_object::GetDC(self);
    if (!pDC)
        return NULL;
    int new_flags;
    // @pyparm int|newFlags||The new alignment flags.  Can be a combination of (TA_CENTER, TA_LEFT, TA_RIGHT),
    // (TA_BASELINE, TA_BOTTOM, TA_TOP) and (TA_NOUPDATECP, TA_UPDATECP)<nl> The default is
    // TA_LEFT\|TA_TOP\|TA_NOUPDATECP
    if (!PyArg_ParseTuple(args, "i", &new_flags))
        return NULL;
    GUI_BGN_SAVE;
    int old_flags = pDC->SetTextAlign(new_flags);  // @pyseemfc CDC|SetTextAlign
    GUI_END_SAVE;
    return Py_BuildValue("i", old_flags);
    // @rdesc The old alignment flags.
}

// @pymethod object|PyCDC|SelectObject|Selects an object into the device context.<nl>
// Currently, only <o PyCFont>, <o PyCBitMap>, <o PyCBrush> and <o PyCPen> objects are supported.
static PyObject *ui_dc_select_object(PyObject *self, PyObject *args)
{
    CDC *pDC = ui_dc_object::GetDC(self);
    if (!pDC)
        return NULL;
    PyObject *v;
    // @pyparm object|ob||The object to select.
    if (!PyArg_ParseTuple(args, "O", &v))
        return NULL;

    if (ui_base_class::is_uiobject(v, &PyCFont::type)) {
        PyCFont *new_font = (PyCFont *)v;
        GUI_BGN_SAVE;
        CFont *cFont = pDC->SelectObject(new_font->GetFont());
        GUI_END_SAVE;
        if (cFont == NULL)
            RETURN_ERR("Select font object failed");
        else {
            return ui_assoc_object::make(PyCFont::type, cFont);
        }
    }
    else if (ui_base_class::is_uiobject(v, &ui_bitmap::type)) {
        ui_bitmap *new_bitmap = (ui_bitmap *)v;
        GUI_BGN_SAVE;
        CBitmap *pbm = pDC->SelectObject(new_bitmap->GetBitmap());
        GUI_END_SAVE;
        if (pbm == NULL)
            RETURN_ERR("Select bitmap object failed");
        else {
            return ui_assoc_object::make(ui_bitmap::type, pbm);
        }
    }
    else if (ui_base_class::is_uiobject(v, &PyCBrush::type)) {
        PyCBrush *new_brush = (PyCBrush *)v;
        GUI_BGN_SAVE;
        CBrush *pbm = pDC->SelectObject(PyCBrush::GetBrush(new_brush));
        GUI_END_SAVE;
        if (pbm == NULL)
            RETURN_ERR("Select brush object failed");
        else {
            return ui_assoc_object::make(PyCBrush::type, pbm);
        }
    }
    else if (ui_base_class::is_uiobject(v, &ui_pen_object::type)) {
        ui_pen_object *new_pen = (ui_pen_object *)v;
        GUI_BGN_SAVE;
        CPen *cPen = pDC->SelectObject(new_pen->GetPen());
        GUI_END_SAVE;
        if (cPen == NULL) {
            RETURN_ERR("Select pen object failed");
        }
        else {
            return ui_assoc_object::make(ui_pen_object::type, cPen);
        }
    }
    RETURN_ERR("Attempt to select unsupported object type.");
    // @pyseemfc CDC|SelectObject
    // @rdesc The previously selected object.  This will be the same type as the object parameter.
}

// @pymethod int|PyCDC|SelectPalette|Sets the logical palette.
static PyObject *ui_dc_select_palette(PyObject *self, PyObject *args)
{
    CDC *pDC = ui_dc_object::GetDC(self);
    if (!pDC)
        return NULL;
    HPALETTE hPal;
    BOOL bForceBG = FALSE;
    // @pyparm int|hPalette||The handle to the palette
    // @pyparm int|forceBackground||Specifies whether the logical palette is forced to be a background palette.
    if (!PyArg_ParseTuple(args, "i|i:SelectPalette", &hPal, &bForceBG))
        return NULL;
    GUI_BGN_SAVE;
    HPALETTE ret = ::SelectPalette(pDC->GetSafeHdc(), hPal, bForceBG);  // @pyseemfc CDC|SelectePalette
    GUI_END_SAVE;
    return PyWinLong_FromHANDLE(ret);
    // @rdesc The previous palette handle.
}

// @pymethod int|PyCDC|RealizePalette|Maps palette entries in the current logical palette to the system palette.
static PyObject *ui_dc_realize_palette(PyObject *self, PyObject *args)
{
    CDC *pDC = ui_dc_object::GetDC(self);
    if (!pDC)
        return NULL;
    CHECK_NO_ARGS2(args, RealizePalette);
    GUI_BGN_SAVE;
    UINT ret = pDC->RealizePalette();
    GUI_END_SAVE;
    return Py_BuildValue("i", ret);
    // @rdesc Indicates how many entries in the logical palette were mapped to different entries
    // in the system palette. This represents the number of entries that this function
    // remapped to accommodate changes in the system palette since the logical palette
    // was last realized.
}

// @pymethod dict|PyCDC|SetROP2|Sets the current drawing mode.
static PyObject *ui_dc_set_rop2(PyObject *self, PyObject *args)
{
    int mode, old_mode;
    CDC *pDC = ui_dc_object::GetDC(self);
    if (!pDC) {
        return NULL;
    }
    // @pyparm int|mode||The new drawing mode.
    if (!PyArg_ParseTuple(args, "i", &mode)) {
        return NULL;
    }
    GUI_BGN_SAVE;
    old_mode = pDC->SetROP2(mode);  // @pyseemfc CDC|SetROP2
    GUI_END_SAVE;
    return Py_BuildValue("i", old_mode);
}

// @pymethod |PyCDC|TextOut|Outputs text to the display context, using the currently selected font.
static PyObject *ui_dc_text_out(PyObject *self, PyObject *args)
{
    CDC *pDC = ui_dc_object::GetDC(self);
    if (!pDC)
        return NULL;
    TCHAR *text;
    PyObject *obtext;
    DWORD strLen;
    int x, y;
    if (!PyArg_ParseTuple(args, "iiO:TextOut",
                          &x,        // @pyparm x|int||The x coordinate to write the text to.
                          &y,        // @pyparm y|int||The y coordinate to write the text to.
                          &obtext))  // @pyparm text|string||The text to write.
        return NULL;
    if (!PyWinObject_AsTCHAR(obtext, &text, FALSE, &strLen))
        return NULL;
    GUI_BGN_SAVE;
    BOOL ret = pDC->TextOut(x, y, text, strLen);
    // @pyseemfc CDC|TextOut
    GUI_END_SAVE;
    PyWinObject_FreeTCHAR(text);
    if (!ret)
        RETURN_API_ERR("CDC::TextOut");
    RETURN_NONE;
    // @rdesc Always none.  If the function fails, an exception is raised.
}

/* struct to dict macro (alpha version)
   move to win32ui_int eventually */
#define DICTADD(D, ST, M, TYPE) PyDict_SetItemString(D, #M, Py_BuildValue(TYPE, ST.M))

// @pymethod dict|PyCDC|GetTextMetrics|Retrieves the metrics for the current font in this device context.
static PyObject *ui_dc_get_text_metrics(PyObject *self, PyObject *args)
{
    CDC *pDC = ui_dc_object::GetDC(self);
    if (!pDC)
        return NULL;

    CHECK_NO_ARGS(args);

    TEXTMETRIC tm;

    if (!pDC->GetTextMetrics(&tm))  // @pyseemfc CDC|GetTextMetrics
        RETURN_ERR("GetTextMetrics failed");

    PyObject *d = PyDict_New();

    // @rdesc A dictionary of integers, keyed by the following strings:<nl>
    DICTADD(d, tm, tmHeight, "i");            // tmHeight<nl>
    DICTADD(d, tm, tmAscent, "i");            // tmAscent<nl>
    DICTADD(d, tm, tmDescent, "i");           // tmDescent<nl>
    DICTADD(d, tm, tmInternalLeading, "i");   // tmInternalLeading<nl>
    DICTADD(d, tm, tmExternalLeading, "i");   // tmExternalLeading<nl>
    DICTADD(d, tm, tmAveCharWidth, "i");      // tmAveCharWidth<nl>
    DICTADD(d, tm, tmMaxCharWidth, "i");      // tmMaxCharWidth<nl>
    DICTADD(d, tm, tmWeight, "i");            // tmWeight<nl>
    DICTADD(d, tm, tmItalic, "i");            // tmItalic<nl>
    DICTADD(d, tm, tmUnderlined, "i");        // tmUnderlined<nl>
    DICTADD(d, tm, tmStruckOut, "i");         // tmStruckOut<nl>
    DICTADD(d, tm, tmFirstChar, "i");         // tmFirstChar<nl>
    DICTADD(d, tm, tmLastChar, "i");          // tmLastChar<nl>
    DICTADD(d, tm, tmDefaultChar, "i");       // tmDefaultChar<nl>
    DICTADD(d, tm, tmBreakChar, "i");         // tmBreakChar<nl>
    DICTADD(d, tm, tmPitchAndFamily, "i");    // tmPitchAndFamily<nl>
    DICTADD(d, tm, tmCharSet, "i");           // tmCharSet<nl>
    DICTADD(d, tm, tmOverhang, "i");          // tmOverhang<nl>
    DICTADD(d, tm, tmDigitizedAspectX, "i");  // tmDigitizedAspectX<nl>
    DICTADD(d, tm, tmDigitizedAspectY, "i");  // tmDigitizedAspectY<nl>

    return d;
}

// @pymethod string|PyCDC|GetTextFace|Returns typeface name of the current font.
static PyObject *ui_dc_get_text_face(PyObject *self, PyObject *args)
{
    CDC *pDC = ui_dc_object::GetDC(self);
    if (!pDC)
        return NULL;

    if (!PyArg_ParseTuple(args, ":GetTextFace"))
        return NULL;

    TCHAR buf[LF_FACESIZE];

    GUI_BGN_SAVE;
    int ret = pDC->GetTextFace(LF_FACESIZE, buf);  // @pyseemfc CDC|GetTextFace
    GUI_END_SAVE;
    if (ret == 0)
        buf[0] = '\0';

    return PyWinObject_FromTCHAR(buf);
}

// @pymethod int|PyCDC|SaveDC|Saves the current state of the device context.  Windows manages a stack of state
// information. The saved device context can later be restored by using <om CDC.RestoreDC>
static PyObject *ui_dc_save_dc(PyObject *self, PyObject *args)
{
    CDC *pDC = ui_dc_object::GetDC(self);
    if (!pDC)
        return NULL;

    if (!PyArg_ParseTuple(args, ""))
        return NULL;

    GUI_BGN_SAVE;
    int ret = pDC->SaveDC();  // @pyseemfc CDC|SaveDC
    GUI_END_SAVE;
    if (ret == 0)
        RETURN_ERR("SaveDC failed");
    else
        return Py_BuildValue("i", ret);
    // @rdesc An integer identifying the context, which can be used by <om PyCDC.RestoreDC>.
    // An exception is raised if this function fails.
}

// @pymethod |PyCDC|RestoreDC|Restores the state of the device context.
static PyObject *ui_dc_restore_dc(PyObject *self, PyObject *args)
{
    CDC *pDC = ui_dc_object::GetDC(self);
    if (!pDC)
        return NULL;

    int saved;
    // @pyparm int|saved||The id of a previously saved device context.  See <om PyCDC.SaveDC>
    if (!PyArg_ParseTuple(args, "i", &saved))
        return NULL;

    GUI_BGN_SAVE;
    BOOL ok = pDC->RestoreDC(saved);
    GUI_END_SAVE;
    if (!ok)  // @pyseemfc CDC|RestoreDC
        RETURN_ERR("RestoreDC failed");
    else
        RETURN_NONE;
}

// @pymethod (x,y)|PyCDC|MoveTo|Moves the current position to a specified point.
static PyObject *ui_dc_move_to(PyObject *self, PyObject *args)
{
    CDC *pDC = ui_dc_object::GetDC(self);
    if (!pDC)
        return NULL;

    // @pyparm (x,y)|point||The point coordinate to move to.
    // @pyparmalt1 int|x||The x coordinate to move to.
    // @pyparmalt1 int|y||The y coordinate to move to.
    int x, y;
    CPoint prev;
    if (!PyArg_ParseTuple(args, "ii", &x, &y)) {
        PyErr_Clear();
        if (!PyArg_ParseTuple(args, "(ii)", &x, &y))
            return NULL;
    }

    GUI_BGN_SAVE;
    prev = pDC->MoveTo(x, y);  // @pyseemfc CDC|MoveTo
    GUI_END_SAVE;

    return Py_BuildValue("(ii)", prev.x, prev.y);
    // @rdesc The previous position.
}

// @pymethod |PyCDC|LineTo|Draws a line to a specified point, using the currently selected pen.
static PyObject *ui_dc_line_to(PyObject *self, PyObject *args)
{
    CDC *pDC = ui_dc_object::GetDC(self);
    if (!pDC)
        return NULL;

    // @pyparm (x,y)|point||The point coordinate to draw to.
    // @pyparmalt1 int|x||The x coordinate to draw to.
    // @pyparmalt1 int|y||The y coordinate to draw to.
    int x, y;
    CPoint prev;
    if (!PyArg_ParseTuple(args, "ii", &x, &y)) {
        PyErr_Clear();
        if (!PyArg_ParseTuple(args, "(ii)", &x, &y))
            return NULL;
    }

    GUI_BGN_SAVE;
    BOOL ok = pDC->LineTo(x, y);
    GUI_END_SAVE;
    if (!ok)  // @pyseemfc CDC|LineTo
        RETURN_ERR("LineTo failed");
    else
        RETURN_NONE;
}

// @pymethod (x,y)|PyCDC|DPtoLP|Converts device units into logical units.
static PyObject *ui_dc_dp_to_lp(PyObject *self, PyObject *args)
{
    CDC *pDC = ui_dc_object::GetDC(self);
    if (!pDC)
        return NULL;

    CPoint pt;
    // @todo Should really handle list of (x,y) points
    // @pyparm (x,y)|point||The point to convert
    // @pyparmalt1 int|x||The x coordinate to convert.
    // @pyparmalt1 int|y||The y coordinate to convert.
    if (!PyArg_ParseTuple(args, "ii", &pt.x, &pt.y)) {
        PyErr_Clear();
        if (!PyArg_ParseTuple(args, "(ii)", &pt.x, &pt.y))
            return NULL;
    }

    GUI_BGN_SAVE;
    pDC->DPtoLP(&pt, 1);  // @pyseemfc CDC|DPtoLP
    GUI_END_SAVE;
    return (Py_BuildValue("(ii)", pt.x, pt.y));
    // @rdesc The converted coordinates.
}

// @pymethod (x,y)|PyCDC|LPtoDP|Converts logical units into device units.
static PyObject *ui_dc_lp_to_dp(PyObject *self, PyObject *args)
{
    CDC *pDC = ui_dc_object::GetDC(self);
    if (!pDC)
        return NULL;

    CPoint pt;
    // should really handle array of (x,y) points
    // @pyparm (x,y)|point||The point coordinate to convert.
    // @pyparmalt1 int|x||The x coordinate to convert.
    // @pyparmalt1 int|y||The y coordinate to convert.
    if (!PyArg_ParseTuple(args, "ii", &pt.x, &pt.y)) {
        PyErr_Clear();
        if (!PyArg_ParseTuple(args, "(ii)", &pt.x, &pt.y))
            return NULL;
    }

    GUI_BGN_SAVE;
    pDC->LPtoDP(&pt, 1);  // @pyseemfc CDC|LPtoDP
    GUI_END_SAVE;
    return (Py_BuildValue("(ii)", pt.x, pt.y));
    // @rdesc The converted coordinates.
}

// @pymethod (left, top, right, bottom)|PyCDC|GetClipBox|Retrieves the dimensions of the smallest bounding rectangle
// around the current clipping boundary.
static PyObject *ui_dc_get_clip_box(PyObject *self, PyObject *args)
{
    CDC *pDC = ui_dc_object::GetDC(self);
    if (!pDC)
        return NULL;

    if (!PyArg_ParseTuple(args, ""))
        return NULL;

    CRect rect;
    GUI_BGN_SAVE;
    int ret = pDC->GetClipBox(&rect);  // @pyseemfc CDC|GetClipBox
    GUI_END_SAVE;
    if (ret == ERROR)
        RETURN_ERR("GetClipBox failed");
    else
        return Py_BuildValue("(iiii)", rect.left, rect.top, rect.right, rect.bottom);
    // @rdesc A tuple of integers specifying the rectangle.
}

// @pymethod int|PyCDC|GetHandleAttrib|Retrieves the handle of the attribute device context.
static PyObject *ui_dc_get_handle_attrib(PyObject *self, PyObject *args)
{
    CDC *pDC = ui_dc_object::GetDC(self);
    if (!pDC)
        return NULL;

    if (!PyArg_ParseTuple(args, ":GetHandleAttrib"))
        return NULL;

    return Py_BuildValue("i", pDC->m_hAttribDC);
}
// @pymethod int|PyCDC|GetHandleOutput|Retrieves the handle of the output device context.
static PyObject *ui_dc_get_handle_output(PyObject *self, PyObject *args)
{
    CDC *pDC = ui_dc_object::GetDC(self);
    if (!pDC)
        return NULL;

    if (!PyArg_ParseTuple(args, ":GetHandleOutput"))
        return NULL;

    return Py_BuildValue("i", pDC->m_hDC);
}

// Path methods:
// BeginPath
// EndPath
// StrokePath
// FillPath
// StrokeAndFillPath

// @pymethod |PyCDC|BeginPath|Opens a path bracket in the device context
static PyObject *ui_dc_begin_path(PyObject *self, PyObject *args)
{
    CDC *pDC = ui_dc_object::GetDC(self);
    if (!pDC) {
        return NULL;
    }
    else if (!PyArg_ParseTuple(args, "")) {
        return NULL;
    }
    else {
        GUI_BGN_SAVE;
        BOOL ok = pDC->BeginPath();
        GUI_END_SAVE;
        if (!ok) {
            RETURN_API_ERR("CDC::BeginPath");
        }
        else {
            RETURN_NONE;
        }
    }
}

// @pymethod |PyCDC|EndPath|Closes a path bracket and selects the path defined by the bracket into the specified device
// context
static PyObject *ui_dc_end_path(PyObject *self, PyObject *args)
{
    CDC *pDC = ui_dc_object::GetDC(self);
    if (!pDC) {
        return NULL;
    }
    else if (!PyArg_ParseTuple(args, "")) {
        return NULL;
    }
    else {
        GUI_BGN_SAVE;
        BOOL ok = pDC->EndPath();
        GUI_END_SAVE;
        if (!ok) {
            RETURN_API_ERR("CDC::EndPath");
        }
        else {
            RETURN_NONE;
        }
    }
}

// @pymethod |PyCDC|FillPath|Closes any open figures in the current path and fills the path's interior by using the
// current brush and polygon-filling mode. After its interior is filled, the path is discarded from the device context.
static PyObject *ui_dc_fill_path(PyObject *self, PyObject *args)
{
    CDC *pDC = ui_dc_object::GetDC(self);
    if (!pDC) {
        return NULL;
    }
    else if (!PyArg_ParseTuple(args, "")) {
        return NULL;
    }
    else {
        GUI_BGN_SAVE;
        BOOL ok = pDC->FillPath();
        GUI_END_SAVE;
        if (!ok) {
            RETURN_API_ERR("CDC::FillPath");
        }
        else {
            RETURN_NONE;
        }
    }
}

// @pymethod |PyCDC|StrokePath|Renders the specified path by using the current pen.
static PyObject *ui_dc_stroke_path(PyObject *self, PyObject *args)
{
    CDC *pDC = ui_dc_object::GetDC(self);
    if (!pDC) {
        return NULL;
    }
    else if (!PyArg_ParseTuple(args, "")) {
        return NULL;
    }
    else {
        GUI_BGN_SAVE;
        BOOL ok = pDC->StrokePath();
        GUI_END_SAVE;
        if (!ok) {
            RETURN_API_ERR("CDC::StrokePath");
        }
        else {
            RETURN_NONE;
        }
    }
}

// @pymethod |PyCDC|StrokeAndFillPath|Closes any open figures in a path, strokes the outline of the path by using the
// current pen, and fills its interior by using the current brush. The device context must contain a closed path.
static PyObject *ui_dc_stroke_and_fill_path(PyObject *self, PyObject *args)
{
    CDC *pDC = ui_dc_object::GetDC(self);
    if (!pDC) {
        return NULL;
    }
    else if (!PyArg_ParseTuple(args, "")) {
        return NULL;
    }
    else {
        GUI_BGN_SAVE;
        BOOL ok = pDC->StrokeAndFillPath();
        GUI_END_SAVE;
        if (!ok) {
            RETURN_API_ERR("CDC::StrokeAndFillPath");
        }
        else {
            RETURN_NONE;
        }
    }
}

// @pymethod int|PyCDC|IsPrinting|Returns 1 if the DC is currently printing, else 0
static PyObject *ui_dc_is_printing(PyObject *self, PyObject *args)
{
    CDC *pDC = ui_dc_object::GetDC(self);
    if (!pDC) {
        return NULL;
    }
    else {
        GUI_BGN_SAVE;
        int rc = pDC->IsPrinting();
        GUI_END_SAVE;
        return Py_BuildValue("i", rc);
    }
}

// @pymethod x, y|PyCDC|ScaleWindowExt|Modifies the window extents relative to the current values.
static PyObject *ui_dc_scale_window_ext(PyObject *self, PyObject *args)
{
    CDC *pDC = ui_dc_object::GetDC(self);
    if (!pDC) {
        return NULL;
    }
    else {
        int xn, xd, yn, yd;
        if (!PyArg_ParseTuple(args, "iiii", &xn, &xd, &yn, &yd)) {
            return NULL;
        }
        else {
            GUI_BGN_SAVE;
            CSize r = pDC->ScaleWindowExt(xn, xd, yn, yd);
            GUI_END_SAVE;
            return Py_BuildValue("(ii)", r.cx, r.cy);
        }
    }
}

// @pymethod x, y|PyCDC|ScaleViewportExt|Modifies the viewport extents relative to the current values.
static PyObject *ui_dc_scale_viewport_ext(PyObject *self, PyObject *args)
{
    CDC *pDC = ui_dc_object::GetDC(self);
    if (!pDC) {
        return NULL;
    }
    else {
        int xn, xd, yn, yd;
        if (!PyArg_ParseTuple(args, "iiii", &xn, &xd, &yn, &yd)) {
            return NULL;
        }
        else {
            GUI_BGN_SAVE;
            CSize r = pDC->ScaleViewportExt(xn, xd, yn, yd);
            GUI_END_SAVE;
            return Py_BuildValue("(ii)", r.cx, r.cy);
        }
    }
}

// Printing functions

// @pymethod |PyCDC|AbortDoc|Aborts a print job
static PyObject *ui_dc_abort_doc(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS2(args, AbortDoc);
    CDC *pDC = ui_dc_object::GetDC(self);
    if (!pDC)
        return NULL;
    GUI_BGN_SAVE;
    pDC->AbortDoc();
    GUI_END_SAVE;
    RETURN_NONE;
}

// @pymethod |PyCDC|EndDoc|Finishes spooling the document and starts printing it
static PyObject *ui_dc_end_doc(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS2(args, EndDoc);
    CDC *pDC = ui_dc_object::GetDC(self);
    if (!pDC)
        return NULL;
    GUI_BGN_SAVE;
    int err = pDC->EndDoc();
    GUI_END_SAVE;
    if (err < 0) {
        char msg[64];
        sprintf(msg, "EndDoc failed (error code %d)", err);
        PyErr_SetString(ui_module_error, msg);
        return NULL;
    }
    RETURN_NONE;
}

// @pymethod |PyCDC|EndPage|Finishes a page on a printer DC
static PyObject *ui_dc_end_page(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS2(args, EndPage);
    CDC *pDC = ui_dc_object::GetDC(self);
    if (!pDC)
        return NULL;
    GUI_BGN_SAVE;
    int err = pDC->EndPage();
    GUI_END_SAVE;
    if (err < 0) {
        char msg[64];
        sprintf(msg, "EndDoc failed (error code %d)", err);
        PyErr_SetString(ui_module_error, msg);
        return NULL;
    }
    RETURN_NONE;
}

// @pymethod |PyCDC|StartDoc|Starts spooling a document to a printer DC
static PyObject *ui_dc_start_doc(PyObject *self, PyObject *args)
{
    CDC *pDC = ui_dc_object::GetDC(self);
    if (!pDC)
        return NULL;

    // @pyparm string|docName||The document name
    // @pyparm string|outputFile||The output file name. Use this to spool to a file. Omit to send to the printer.
    TCHAR *docName = NULL, *outputFile = NULL;
    PyObject *obdocName, *oboutputFile = Py_None;
    if (!PyArg_ParseTuple(args, "O|O:StartDoc", &obdocName, &oboutputFile))
        return NULL;
    if (!PyWinObject_AsTCHAR(obdocName, &docName, FALSE))
        return NULL;
    if (!PyWinObject_AsTCHAR(oboutputFile, &outputFile, TRUE)) {
        PyWinObject_FreeTCHAR(docName);
        return NULL;
    }
    DOCINFO info;
    info.cbSize = sizeof(DOCINFO);
    memset(&info, 0, sizeof(DOCINFO));
    info.lpszDocName = docName;
    info.lpszOutput = outputFile;

    GUI_BGN_SAVE;
    int rc = pDC->StartDoc(&info);
    GUI_END_SAVE;
    PyWinObject_FreeTCHAR(docName);
    PyWinObject_FreeTCHAR(outputFile);
    if (rc < 0) {
        RETURN_ERR("StartDoc failed");
    }

    RETURN_NONE;
}

// @pymethod |PyCDC|StartPage|Starts a new page on a printer DC
static PyObject *ui_dc_start_page(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS2(args, StartPage);
    CDC *pDC = ui_dc_object::GetDC(self);
    if (!pDC)
        return NULL;
    GUI_BGN_SAVE;
    int rc = pDC->StartPage();
    GUI_END_SAVE;
    if (rc <= 0)
        RETURN_ERR("StartPage failed");
    RETURN_NONE;
}

/////////////////////////////////////////////////////////////////////
//
// DC methods contributed by: Kleanthis Kleanthous (kk@epsilon.com.gr)

// @pymethod |PyCDC|IntersectClipRect|Creates a new clipping region by forming the intersection of the current region
// and the rectangle specified
// @rdesc region type as integer
static PyObject *ui_dc_intersect_clip_rect(PyObject *self, PyObject *args)
{
    CDC *pDC = ui_dc_object::GetDC(self);
    if (!pDC)
        return NULL;

    RECT rect;
    if (!PyArg_ParseTuple(
            args, "(iiii):IntersectClipRect", &rect.left, &rect.top, &rect.right, &rect.bottom
            // @pyparm (left, top, right, bottom)|rect||Specifies the bounding rectangle, in logical units.
            ))
        return NULL;
    GUI_BGN_SAVE;
    int type = pDC->IntersectClipRect(&rect);
    GUI_END_SAVE;
    // @pyseemfc CDC|IntersectClipRect
    return Py_BuildValue("i", type);
}

// @pymethod (int)|PyCDC|SetPolyFillMode|Sets the polygon-filling mode.
// @rdesc The previous PolyFillMode as integer
static PyObject *ui_dc_set_poly_fill_mode(PyObject *self, PyObject *args)
{
    CDC *pDC = ui_dc_object::GetDC(self);
    if (!pDC)
        return NULL;
    int nPolyFillMode;
    // @pyparm (x,y)|point||The new origin in device units.
    if (!PyArg_ParseTuple(args, "i", &nPolyFillMode))
        return NULL;
    GUI_BGN_SAVE;
    int pr = pDC->SetPolyFillMode(nPolyFillMode);  // @pyseemfc CDC|SetPolyFillMode
    GUI_END_SAVE;
    return Py_BuildValue("i", pr);
    // @rdesc The previous PolyFillMode.
}

// @pymethod |PyCDC|Polyline|Draws a Polyline.
static PyObject *ui_dc_polyline(PyObject *self, PyObject *args)
{
    PyObject *point_list;
    CDC *pDC = ui_dc_object::GetDC(self);
    if (!pDC)
        return NULL;
    // @pyparm [(x, y), ...]|points||A sequence of points
    if (!PyArg_ParseTuple(args, "O:Polyline", &point_list)) {
        return NULL;
    }
    else if (!PySequence_Check(point_list)) {
        RETURN_TYPE_ERR("Argument must be a list of points");
    }
    else {
        // Convert the list of point tuples into an array of POINT structs
        Py_ssize_t num = PySequence_Length(point_list);
        POINT *point_array = new POINT[num];
        for (Py_ssize_t i = 0; i < num; i++) {
            PyObject *point_tuple = PySequence_GetItem(point_list, i);
            if (!PyTuple_Check(point_tuple) || PyTuple_Size(point_tuple) != 2) {
                PyErr_SetString(PyExc_ValueError, "point list must be a list of (x,y) tuples");
                delete[] point_array;
                return NULL;
            }
            else {
                long x, y;
                PyObject *px, *py;
                px = PyTuple_GetItem(point_tuple, 0);
                py = PyTuple_GetItem(point_tuple, 1);
                if ((!PyInt_Check(px)) || (!PyInt_Check(py))) {
                    PyErr_SetString(PyExc_ValueError, "point list must be a list of (x,y) tuples");
                    delete[] point_array;
                    return NULL;
                }
                else {
                    x = PyInt_AsLong(px);
                    y = PyInt_AsLong(py);
                    point_array[i].x = x;
                    point_array[i].y = y;
                }
            }
        }
        // we have an array of POINT structs, now we
        // can finally draw the polyline.
        GUI_BGN_SAVE;
        BOOL ret = pDC->Polyline(point_array, PyWin_SAFE_DOWNCAST(num, Py_ssize_t, int));
        GUI_END_SAVE;
        delete[] point_array;
        if (!ret) {
            RETURN_API_ERR("CDC::Polyline");
        }
        else {
            RETURN_NONE;
        }
    }
}

// @pymethod x, y|PyCDC|OffsetWindowOrg|Modifies the coordinates of the window origin relative to the coordinates of the
// current window origin.
// @rdesc The previous origin as a tuple (x,y)
static PyObject *ui_dc_offset_window_org(PyObject *self, PyObject *args)
{
    CDC *pDC = ui_dc_object::GetDC(self);
    if (!pDC)
        return NULL;

    int x, y;
    // @pyparm int, int|x,y||The new origin offset.
    if (!PyArg_ParseTuple(args, "(ii)", &x, &y))
        return NULL;

    GUI_BGN_SAVE;
    CPoint old_org = pDC->OffsetWindowOrg(x, y);
    GUI_END_SAVE;
    return Py_BuildValue("(ii)", old_org.x, old_org.y);
}

// @pymethod x, y|PyCDC|OffsetViewportOrg|Modifies the coordinates of the viewport origin relative to the coordinates of
// the current viewport origin
// @rdesc The previous viewport origin as a tuple (x,y)
static PyObject *ui_dc_offset_viewport_org(PyObject *self, PyObject *args)
{
    CDC *pDC = ui_dc_object::GetDC(self);
    if (!pDC)
        return NULL;

    int x, y;
    // @pyparm int, int|x,y||The new origin offset.
    if (!PyArg_ParseTuple(args, "(ii)", &x, &y))
        return NULL;

    GUI_BGN_SAVE;
    CPoint old_org = pDC->OffsetViewportOrg(x, y);
    GUI_END_SAVE;
    return Py_BuildValue("(ii)", old_org.x, old_org.y);
}

// @pymethod obRgn|PyCDC|SelectClipRgn|Selects the given region as the current clipping region for the device context
// @rdesc The return value specifies the region's complexity (integer)
static PyObject *ui_dc_select_clip_rgn(PyObject *self, PyObject *args)
{
    CDC *pDC = ui_dc_object::GetDC(self);
    if (!pDC)
        return NULL;

    PyObject *objRgn = Py_None;
    if (!PyArg_ParseTuple(args, "O:SelectClipRgn", &objRgn))
        return NULL;

    CRgn *pRgn = PyCRgn::GetRgn(objRgn);
    if (!pRgn)
        return NULL;

    GUI_BGN_SAVE;
    int r = pDC->SelectClipRgn(pRgn);
    GUI_END_SAVE;

    return Py_BuildValue("i", r);
}

// @pymethod rc|PyCDC|Rectangle|Draws a rectangle using the current pen. The interior of the rectangle is filled using
// the current brush.
static PyObject *ui_dc_rectangle(PyObject *self, PyObject *args)
{
    CDC *pDC = ui_dc_object::GetDC(self);
    if (!pDC)
        return NULL;
    RECT rect;
    if (!PyArg_ParseTuple(args, "(iiii)", &rect.left, &rect.top, &rect.right, &rect.bottom))
        return NULL;

    BOOL b = pDC->Rectangle(rect.left, rect.top, rect.right, rect.bottom);

    if (!b)
        RETURN_API_ERR("CDC::Rectangle");

    RETURN_NONE;
}

// @pymethod s,rc,forat|PyCDC|DrawText|Formats text in the given rectangle
// @rdesc Height of text in pixels
static PyObject *ui_dc_draw_text(PyObject *self, PyObject *args)
{
    CDC *pDC = ui_dc_object::GetDC(self);
    if (!pDC)
        return NULL;

    char *psz;
    RECT rect;
    UINT nFormat = DT_SINGLELINE | DT_CENTER | DT_VCENTER;
    if (!PyArg_ParseTuple(args, "s(iiii)|i",
                          &psz,  // @pyparm string|s||The desired output string
                                 // @pyparm (int, int, int, int)|tuple||The bounding rectangle in the form:
                                 // (left, top, right, bottom) expressed in logical units (depending on
                                 // selected coordinate system - see <om PyCDC.SetMapMode>)
                          &rect.left, &rect.top, &rect.right, &rect.bottom,
                          // @pyparm int|format||Specifies one or more bit-or'd format values, such as
                          // DT_BOTTOM, DT_CENTERDT_RIGHT, DT_VCENTER. For a complete list, see
                          // the Microsoft Win32 API documentation.
                          &nFormat))
        return NULL;

    CString str(psz);
    int height = pDC->DrawText(str, &rect, nFormat);
    // @rdesc The return value is the height of the text, in logical units.
    // If DT_VCENTER or DT_BOTTOM is specified, the return value is the
    // offset from rect.top to the bottom of the drawn text.
    // If the function fails, the return value is zero (no Python exception is thrown)
    return Py_BuildValue("i", height);
    // @ex Example|import win32ui<nl>
    // import win32con<nl>
    // INCH = 1440   # twips - 1440 per inch allows fine res<nl>
    // def drawtext_test():<nl>
    //     dc = win32ui.CreateDC()<nl>
    //     dc.CreatePrinterDC()                # ties to default printer<nl>
    //     dc.StartDoc('My Python Document')<nl>
    //     dc.StartPage()<nl>
    // <nl>
    //     # note: upper left is 0,0 with x increasing to the right,<nl>
    //     #       and y decreasing (negative) moving down<nl>
    //     dc.SetMapMode(win32con.MM_TWIPS)<nl>
    // <nl>
    //     # Centers "TEST" about an inch down on page<nl>
    //     dc.DrawText('TEST', (0,INCH*-1,INCH*8,INCH*-2), win32con.DT_CENTER )<nl>
    //     dc.EndPage()<nl>
    //     dc.EndDoc()<nl>
    //     del dc<nl>
}

// @pymethod |PyCDC|StretchBlt|Copies a bitmap from the source device context to this device context.
static PyObject *ui_dc_stretch_blt(PyObject *self, PyObject *args)
{
    CDC *pDC = ui_dc_object::GetDC(self);
    if (!pDC)
        return NULL;
    int x, y, width, height, xsrc, ysrc, widthsrc, heightsrc;
    DWORD rop;
    PyObject *dc_ob;
    if (!PyArg_ParseTuple(
            args, "(ii)(ii)O(ii)(ii)i", &x, &y,  // @pyparm (x,y)-ints|destPos||The logical x,y coordinates of the
                                                 // upper-left corner of the destination rectangle.
            &width, &height,  // @pyparm (width, height)-ints|size||Specifies the width and height (in logical units) of
                              // the destination rectangle and source bitmap.
            &dc_ob,  // @pyparm <o PyCDC>|dc||Specifies the PyCDC object from which the bitmap will be copied. It must
                     // be None if rop specifies a raster operation that does not include a source.
            &xsrc, &ysrc,  // @pyparm (xSrc, ySrc)-ints|srcPos||Specifies the logical x,y coordinates of the upper-left
                           // corner of the source bitmap.
            &widthsrc, &heightsrc,  // @pyparm (widthsrc, heightsrc)-ints|size||Specifies the width and height (in
                                    // logical units) of the destination rectangle and source bitmap.
            &rop))  // @pyparm int|rop||Specifies the raster operation to be performed. See the win32 api documentation
                    // for details.
        return NULL;
    if (!ui_base_class::is_uiobject(dc_ob, &ui_dc_object::type))
        RETURN_TYPE_ERR("The 'O' param must be a PyCDC object");
    CDC *pSrcDC = NULL;
    if (dc_ob != Py_None) {
        pSrcDC = ui_dc_object::GetDC(dc_ob);
        if (!pSrcDC)
            RETURN_ERR("The source DC is invalid");
    }
    GUI_BGN_SAVE;
    int prevMode = pDC->SetStretchBltMode(COLORONCOLOR);
    BOOL ok = pDC->StretchBlt(x, y, width, height, pSrcDC, xsrc, ysrc, widthsrc, heightsrc, rop);
    pDC->SetStretchBltMode(prevMode);
    GUI_END_SAVE;
    if (!ok)  // @pyseemfc CDC|StretchBlt
        RETURN_ERR("StretchBlt failed");
    RETURN_NONE;
}
// End of kk contributed methods!
// @pymethod |PyCDC|DrawFrameControl|Draws a frame control of the specified type and style.
static PyObject *ui_dc_draw_frame_control(PyObject *self, PyObject *args)
{
    CDC *pDC = ui_dc_object::GetDC(self);
    if (!pDC)
        return NULL;

    RECT rect;
    int typ, state;
    if (!PyArg_ParseTuple(
            args, "(iiii)ii:DrawFrameControl", &rect.left, &rect.top, &rect.right, &rect.bottom,
            // @pyparm (left, top, right, bottom)|rect||Specifies the bounding rectangle, in logical units.
            &typ,   // @pyparm int|typ||
            &state  // @pyparm int|state||
            ))
        return NULL;
    GUI_BGN_SAVE;
    BOOL ok = pDC->DrawFrameControl(&rect, typ, state);
    GUI_END_SAVE;
    if (!ok)
        RETURN_ERR("DrawFrameControl failed");
    // @pyseemfc CDC|DrawFrameControl
    RETURN_NONE;
}

/////////////////////////////////////////////////////////////////////
// DC Methods
// @object PyCDC|A Device Context.  Encapsulates an MFC <c CDC> class.
static struct PyMethodDef ui_dc_methods[] = {
    {"AbortDoc", ui_dc_abort_doc, 1},    // @pymeth AbortDoc|Aborts a print job
    {"Arc", ui_dc_arc, 1},               // @pymeth Arc|Draws an arc.
    {"BeginPath", ui_dc_begin_path, 1},  // @pymeth BeginPath|Opens a path bracket in the device context
    {"BitBlt", ui_dc_bitblt, 1},         // @pymeth BitBlt|Copies a bitmap
    {"Chord", ui_dc_chord, 1},           // @pymeth Chord|Draws a chord.
    {"CreateCompatibleDC", ui_dc_object::create_compatible_dc,
     1},  // @pymeth CreateCompatibleDC|Creates a memory DC compatible with this DC.
    {"CreatePrinterDC", ui_dc_object::create_printer_dc,
     1},                                  // @pymeth CreatePrinterDC|Creates a device context for a specific printer
    {"DeleteDC", ui_dc_delete_dc, 1},     // @pymeth DeleteDC|Deletes all resources associated with a device context.
    {"DPtoLP", ui_dc_dp_to_lp, 1},        // @pymeth DPtoLP|Convert from device points to logical points.
    {"Draw3dRect", ui_dc_draw3drect, 1},  // @pymeth Draw3dRect|Draws a three-dimensional rectangle.
    {"DrawFocusRect", ui_dc_draw_focus_rect,
     1},  // @pymeth DrawFocusRect|Draws a rectangle in the style used to indicate the rectangle has focus
    {"DrawFrameControl", ui_dc_draw_frame_control,
     1},  // @pymeth DrawFrameControl|Draws a frame control of the specified type and style.
    {"DrawIcon", ui_dc_draw_icon, 1},       // @pymeth DrawIcon|Draws an icon on the DC.
    {"DrawText", ui_dc_draw_text, 1},       // @pymeth DrawText|Formats text in the given rectangle
    {"Ellipse", ui_dc_ellipse, 1},          // @pymeth Ellipse|Draws an Ellipse.
    {"EndDoc", ui_dc_end_doc, 1},           // @pymeth EndDoc|Finishes spooling the document and starts printing it
    {"EndPage", ui_dc_end_page, 1},         // @pymeth EndPage|Finishes a page on a printer DC
    {"EndPath", ui_dc_end_path, 1},         // @pymeth EndPath|Closes a path bracket and selects the path defined by the
                                            // bracket into the specified device context
    {"ExtTextOut", ui_dc_ext_text_out, 1},  //@pymeth ExtTextOut|Writes text to the DC.
    {"FillPath", ui_dc_fill_path, 1},  // @pymeth FillPath|Closes any open figures in the current path and fills the
                                       // path's interior by using the current brush and polygon-filling mode.
    {"FillRect", ui_dc_fillrect, 1},   // @pymeth FillRect|Fills a given rectangle with the specified brush
    {"FillSolidRect", ui_dc_fillsolidrect,
     1},  // @pymeth FillSolidRect|Fills the given rectangle with the specified solid color.
    {"FrameRect", ui_dc_framerect, 1},        // @pymeth FrameRect|Draws a border around the rectangle specified by rect
    {"GetBrushOrg", ui_dc_get_brush_org, 1},  // @pymeth GetBrushOrg|Retrieves the origin (in device units) of the brush
                                              // currently selected for the device context.
    {"GetClipBox", ui_dc_get_clip_box, 1},    // @pymeth GetClipBox|Retrives the current clipping region.
    {"GetCurrentPosition", ui_dc_get_current_position,
     1},  // @pymeth GetCurrentPosition|Retrieves the current position (in logical coordinates).
    {"GetDeviceCaps", ui_dc_get_device_caps, 1},  // @pymeth GetDeviceCaps|Retrieves current device capabilities.
    {"GetHandleAttrib", ui_dc_get_handle_attrib,
     1},  // @pymeth GetHandleAttrib|Retrieves the handle of the attribute device context.
    {"GetHandleOutput", ui_dc_get_handle_output,
     1},  // @pymeth GetHandleOutput|Retrieves the handle of the output device context.
    {"GetMapMode", ui_dc_get_map_mode, 1},  //@pymeth GetMapMode|Gets the mapping mode for the device context.
    {"GetNearestColor", ui_dc_get_nearest_color,
     1},                               // @pymeth GetNearestColor|Returns the closest color a device can map.
    {"GetPixel", ui_dc_get_pixel, 1},  // @pymeth GetPixel|Returns the value of a pixel at a location
    {"GetSafeHdc", ui_dc_get_safe_hdc,
     1},  // @pymeth GetSafeHdc|Returns the underlying windows handle for the DC object.
    {"GetTextExtent", ui_dc_get_text_extent, 1},  // @pymeth GetTextExtent|Calculates the size of the string.
    {"GetTextExtentPoint", ui_dc_get_text_extent,
     1},  // @pymeth GetTextExtentPoint|Alias for GetTextExtent - Calculates the size of the string.
    {"GetTextFace", ui_dc_get_text_face, 1},  // @pymeth GetTextFace|Retrieves the name of the current font.
    {"GetTextMetrics", ui_dc_get_text_metrics,
     1},  // @pymeth GetTextMetrics|Retrieves the metrics for the current font.
    {"GetViewportExt", ui_dc_get_viewport_ext,
     1},  // @pymeth GetViewportExt|Gets the viewport extent of the device context
    {"GetViewportOrg", ui_dc_get_viewport_org,
     1},                                        // @pymeth GetViewportOrg|Gets the viewport origin of the device context
    {"GetWindowExt", ui_dc_get_window_ext, 1},  // @pymeth GetWindowExt|Gets the window extent of the device context
    {"GetWindowOrg", ui_dc_get_window_org, 1},  // @pymeth GetWindowOrg|Retrieves the x- and y-coordinates of the origin
                                                // of the window associated with the device context.
    {"IntersectClipRect", ui_dc_intersect_clip_rect,
     1},  // @pymeth IntersectClipRect|Creates a new clipping region by forming the intersection of the current region
          // and the rectangle specified
    {"IsPrinting", ui_dc_is_printing, 1},  // @pymeth IsPrinting|Returns 1 if the DC is currently printing, else 0
    {"LineTo", ui_dc_line_to, 1},          // @pymeth LineTo|Draws a line to a specified point.
    {"LPtoDP", ui_dc_lp_to_dp, 1},         // @pymeth LPtoDP|Convert from logical points to device points
    {"MoveTo", ui_dc_move_to, 1},          // @pymeth MoveTo|Moves the current position to a specifed point.
    {"OffsetWindowOrg", ui_dc_offset_window_org,
     1},  // @pymeth OffsetWindowOrg|Modifies the coordinates of the window origin relative to the coordinates of the
          // current window origin.
    {"OffsetViewportOrg", ui_dc_offset_viewport_org,
     1},  // @pymeth OffsetViewportOrg|Modifies the coordinates of the viewport origin relative to the coordinates of
          // the current viewport origin
    {"PatBlt", ui_dc_patblt, 1},  // @pymeth PatBlt|Creates a bit pattern on the device.
    {"Pie", ui_dc_pie, 1},  // @pymeth Pie|Draws a pie shape with specific starting and ending points in a rectangle
    {"PolyBezier", ui_dc_poly_bezier, 1},  // @pymeth PolyBezier|Draws one or more Bezier splines.
    {"Polygon", ui_dc_polygon, 1},         // @pymeth Polygon|Draws an Polygon.
    {"Polyline", ui_dc_polyline, 1},       // @pymeth Polyline|Draws a Polyline.
    {"RealizePalette", ui_dc_realize_palette,
     1},  // @pymeth RealizePalette|Maps palette entries in the current logical palette to the system palette.
    {"Rectangle", ui_dc_rectangle, 1},  // @pymeth Rectangle|Draws a rectangle using the current pen. The interior of
                                        // the rectangle is filled using the current brush.
    {"RectVisible", ui_dc_rect_visible,
     1},  // @pymeth RectVisible|Determines if a rectangle is currently visisble in the viewport.
    {"RestoreDC", ui_dc_restore_dc, 1},  // @pymeth RestoreDC|Restores a saved DC.
    {"SaveDC", ui_dc_save_dc, 1},        // @pymeth SaveDC|Saves a DC.
    {"ScaleWindowExt", ui_dc_scale_window_ext,
     1},  // @pymeth ScaleWindowExt|Modifies the window extents relative to the current values.
    {"ScaleViewportExt", ui_dc_scale_viewport_ext,
     1},  // @pymeth ScaleViewportExt|Modifies the viewport extents relative to the current values.
    {"SelectClipRgn", ui_dc_select_clip_rgn,
     1},  // @pymeth SelectClipRgn|Selects the given region as the current clipping region for the device context
    {"SelectObject", ui_dc_select_object, 1},    // @pymeth SelectObject|Selects an object into the DC.
    {"SelectPalette", ui_dc_select_palette, 1},  // @pymeth SelectObject|Selects the logical palette.
    {"SetBkColor", ui_dc_set_bk_color, 1},       // @pymeth SetBkColor|Sets the background color.
    {"SetBkMode", ui_dc_set_bk_mode, 1},         // @pymeth SetBkMode|Sets the background mode.
    {"SetBrushOrg", ui_dc_set_brush_org, 1},     // @pymeth SetBrushOrg|Specifies the origin that GDI will assign to the
                                                 // next brush that the application selects into the device context.
    {"SetGraphicsMode", ui_dc_set_graphics_mode,
     1},  // @pymeth SetGraphicsMode|Sets the graphics mode for the specified device context
    {"SetMapMode", ui_dc_set_map_mode, 1},             // @pymeth SetMapMode|Sets the device mapping mode.
    {"SetPixel", ui_dc_setpixel, 1},                   // @pymeth SetPixel|Set a pixel to a color
    {"SetPolyFillMode", ui_dc_set_poly_fill_mode, 1},  // @pymeth SetPolyFillMode|Sets the polygon-filling mode.
    {"SetROP2", ui_dc_set_rop2, 1},                    // @pymeth SetROP2|Sets the current drawing mode.
    {"SetTextAlign", ui_dc_set_text_align, 1},         // @pymeth SetTextAlign|Sets the text alignment.
    {"SetTextColor", ui_dc_set_text_color, 1},         // @pymeth SetTextColor|Sets the text foreground color.
    {"SetWindowExt", ui_dc_set_window_ext, 1},         // @pymeth SetWindowExt|Sets the extents of the window.
    {"SetWindowOrg", ui_dc_set_window_org, 1},      // @pymeth SetWindowOrg|Sets the window origin of the device context
    {"SetViewportExt", ui_dc_set_viewport_ext, 1},  // @pymeth SetViewportExt|Sets the extents of the window's viewport.
    {"SetViewportOrg", ui_dc_set_viewport_org,
     1},  // @pymeth SetViewportOrg|Sets the viewport origin of the device context
    {"SetWorldTransform", ui_dc_set_world_transform,
     1},  // @pymeth SetWorldTransform|sets a two-dimensional linear transformation between world space and page space
          // for the specified device context.
    {"StartDoc", ui_dc_start_doc, 1},    // @pymeth StartDoc|Starts spooling a document to a printer DC
    {"StartPage", ui_dc_start_page, 1},  // @pymeth StartPage|Starts a new page on a printer DC
    {"StretchBlt", ui_dc_stretch_blt,
     1},  // @pymeth StretchBlt|Copies a bitmap from the source device context to this device context.
    {"StrokeAndFillPath", ui_dc_stroke_and_fill_path,
     1},  // @pymeth StrokeAndFillPath|Closes any open figures in a path, strokes the outline of the path by using the
          // current pen, and fills its interior by using the current brush. The device context must contain a closed
          // path.
    {"StrokePath", ui_dc_stroke_path, 1},  // @pymeth StrokePath|Renders the specified path by using the current pen.
    {"TextOut", ui_dc_text_out, 1},        // @pymeth TextOut|Writes text to the DC.
    {NULL, NULL}};

ui_type_CObject ui_dc_object::type("PyCDC", &ui_assoc_object::type, RUNTIME_CLASS(CDC), sizeof(ui_dc_object),
                                   PYOBJ_OFFSET(ui_dc_object), ui_dc_methods, GET_PY_CTOR(ui_dc_object));
