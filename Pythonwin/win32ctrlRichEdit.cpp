// Rich Edit Control
#include "stdafx.h"

#include "win32win.h"
#include "win32control.h"
#include "win32doc.h"
#include "win32dc.h"
#include "win32RichEdit.h"

// @doc

extern CRichEditView *GetRichEditViewPtr(PyObject *self);

CRichEditCtrl *GetRichEditCtrl(PyObject *self)
{
    if (ui_base_class::is_uiobject(self, &PyCRichEditView::type)) {
        CRichEditView *pView = GetRichEditViewPtr(self);
        if (pView)
            return &(pView->GetRichEditCtrl());
        else
            return NULL;
    }
    if (ui_base_class::is_uiobject(self, &PyCRichEditCtrl::type))
        return (CRichEditCtrl *)PyCWnd::GetPythonGenericWnd(self, &PyCRichEditCtrl::type);
    RETURN_ERR("Python object can not be used as an edit control.");
}

PyCRichEditCtrl::PyCRichEditCtrl() {}
PyCRichEditCtrl::~PyCRichEditCtrl() {}

/////////////////////////////////////////////////////////////////////
//
// Rich Edit Control object
//
//////////////////////////////////////////////////////////////////////

// @pymethod <o PyCRichEditCtrl>|win32ui|CreateRichEditCtrl|Creates a rich edit control.
PyObject *PyCRichEditCtrl_create(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS(args);
    GUI_BGN_SAVE;
    CRichEditCtrl *pCtrl = new CRichEditCtrl();
    GUI_END_SAVE;
    return ui_assoc_object::make(PyCRichEditCtrl::type, pCtrl);
    // @comm This method only creates the RichEdit object. To create the window, (ie, the control itself), call <om
    // PyCRichEdit.CreateWindow>
}

// @pymethod|PyCRichEditCtrl|CreateWindow|Creates a rich edit control window.
static PyObject *PyCRichEditCtrl_create_window(PyObject *self, PyObject *args)
{
    RECT rect;
    int style, id;
    PyObject *obParentWnd;
    if (!PyArg_ParseTuple(args, "i(iiii)Oi:CreateWindow",
                          &style,  // @pyparm int|style||The control style
                          // @pyparm int,int,int,int|rect||The position of the control
                          &rect.left, &rect.top, &rect.right, &rect.bottom,
                          // @pyparm <o PyCWnd>|parent||The parent window.  Must not be None
                          &obParentWnd,
                          &id))  // @pyparm int|id||The control ID
        return NULL;
    CRichEditCtrl *pEdit = GetRichEditCtrl(self);
    if (!pEdit)
        return NULL;
    if (pEdit->m_hWnd)
        RETURN_ERR("The window already exists");
    CWnd *pParent = GetWndPtr(obParentWnd);
    if (pParent == NULL)
        return NULL;
    BOOL ok;
    GUI_BGN_SAVE;
    ok = pEdit->Create(style, rect, pParent, id);
    GUI_END_SAVE;
    if (!ok)
        RETURN_ERR("Create failed");
    RETURN_NONE;
}

DWORD CALLBACK PyCRichEditCallbackIn(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb)
{
    // documentation for ths function seems to be wrong WRT return values.
    // I need to return 0 on all success, rather than bytes in buffer.
    CEnterLeavePython _celp;
    int retval = -1;
    PyObject *method = (PyObject *)dwCookie;
    PyObject *args = Py_BuildValue("(i)", cb);
    PyObject *result = gui_call_object(method, args);
    Py_DECREF(args);
    if (result == NULL) {
        //		gui_print_error(); // let it filter up.
    }
    else if (result == Py_None) {
        retval = 0;
        *pcb = 0;
    }
    else {
        char *s = PyString_AsString(result);
        if (s == NULL) {
            //			gui_print_error();
        }
        else {
            strcpy((char *)pbBuff, s);
            *pcb = PyWin_SAFE_DOWNCAST(strlen(s), size_t, DWORD);
            retval = 0;
        }
    }
    Py_XDECREF(result);
    return retval;
}

DWORD CALLBACK PyCRichEditCallbackOut(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb)
{
    CEnterLeavePython _celp;
    int retval = 0;  // default abort stream
    PyObject *method = (PyObject *)dwCookie;
    PyObject *args = Py_BuildValue("(s#)", pbBuff, (Py_ssize_t)cb);
    PyObject *result = gui_call_object(method, args);
    Py_DECREF(args);
    if (result == NULL) {
        //		gui_print_error();
    }
    else {
        retval = PyInt_AsLong(result);
        if (PyErr_Occurred()) {
            gui_print_error();
            //			retval = 0;
        }
    }
    return retval;
}

// @pymethod |PyCRichEditCtrl|Copy|Copys the current selection to the clipboard.
static PyObject *PyCRichEditCtrl_copy(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS(args);
    CRichEditCtrl *pEdit = GetRichEditCtrl(self);
    if (!pEdit)
        return NULL;
    GUI_BGN_SAVE;
    pEdit->Copy();  // @pyseemfc CRichEditCtrl|Copy
    GUI_END_SAVE;
    RETURN_NONE;
}

// @pymethod int|PyCRichEditCtrl|Clear|Clears all text in an edit control.
static PyObject *PyCRichEditCtrl_clear(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS(args);
    CRichEditCtrl *pEdit = GetRichEditCtrl(self);
    if (!pEdit)
        return NULL;
    GUI_BGN_SAVE;
    pEdit->Clear();  // @pyseemfc CRichEditCtrl|Clear
    GUI_END_SAVE;
    RETURN_NONE;
}

// @pymethod |PyCRichEditCtrl|Cut|Cuts the current selection to the clipboard.
static PyObject *PyCRichEditCtrl_cut(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS(args);
    CRichEditCtrl *pEdit = GetRichEditCtrl(self);
    if (!pEdit)
        return NULL;
    GUI_BGN_SAVE;
    pEdit->Cut();  // @pyseemfc CRichEditCtrl|Cut
    GUI_END_SAVE;
    RETURN_NONE;
}

// @pymethod int, (start, end)|PyCRichEditCtrl|FindText|Finds text in the control
static PyObject *PyCRichEditCtrl_find_text(PyObject *self, PyObject *args)
{
    CRichEditCtrl *pEdit = GetRichEditCtrl(self);
    if (!pEdit)
        return NULL;
    int flags;
    FINDTEXTEX ft = {{0, 0}, NULL, {0, 0}};
    // @pyparm int|charPos||The character position
    if (!PyArg_ParseTuple(args, "l(ll)s:FindText", &flags, &ft.chrg.cpMin, &ft.chrg.cpMax, &ft.lpstrText))
        return NULL;
    GUI_BGN_SAVE;
    long rc = pEdit->FindText(flags, &ft);
    GUI_END_SAVE;
    return Py_BuildValue("l(ll)", rc, ft.chrgText.cpMin, ft.chrgText.cpMax);
}

// @pymethod (tuple)|PyCRichEditCtrl|GetCharPos|Returns the location of the top-left corner of the character specified
// by charPos.
static PyObject *PyCRichEditCtrl_get_char_pos(PyObject *self, PyObject *args)
{
    CRichEditCtrl *pEdit = GetRichEditCtrl(self);
    if (!pEdit)
        return NULL;
    LONG lCharPos;
    // @pyparm int|charPos||The character position
    if (!PyArg_ParseTuple(args, "l:GetCharPos", &lCharPos))
        return NULL;
    GUI_BGN_SAVE;
    CPoint p = pEdit->GetCharPos(lCharPos);
    GUI_END_SAVE;
    return Py_BuildValue("ll", p.x, p.y);
    // @rdesc The return value is a <om win32ui.CHARFORMAT tuple>
}

// @pymethod (tuple)|PyCRichEditCtrl|GetDefaultCharFormat|Returns the current default character formatting attributes in
// this <o PyCRichEditCtrl> object.
static PyObject *PyCRichEditCtrl_get_default_char_format(PyObject *self, PyObject *args)
{
    CRichEditCtrl *pEdit = GetRichEditCtrl(self);
    if (!pEdit)
        return NULL;
    CHARFORMAT fmt;
    memset(&fmt, 0, sizeof(fmt));
    if (!PyArg_ParseTuple(args, ":GetDefaultCharFormat"))
        return NULL;
    fmt.cbSize = sizeof(CHARFORMAT);
    // Appears to be a documentation bug - dwMask always is
    // coming back as zero!  It appears you must set all dwMask
    // bits, and the other bits do get reset.
    fmt.dwMask = CFM_ALL;
    GUI_BGN_SAVE;
    /*fmt.dwMask = */
    pEdit->GetDefaultCharFormat(fmt);  // @pyseemfc CRichEditCtrl|GetDefaultCharFormat
    GUI_END_SAVE;
    return MakeCharFormatTuple(&fmt);
    // @rdesc The return value is a <om win32ui.CHARFORMAT tuple>
}

// @pymethod int|PyCRichEditCtrl|GetEventMask|Returns the current event mask.
static PyObject *PyCRichEditCtrl_get_event_mask(PyObject *self, PyObject *args)
{
    CRichEditCtrl *pEdit = GetRichEditCtrl(self);
    if (!pEdit)
        return NULL;
    if (!PyArg_ParseTuple(args, ":GetEventMask"))
        return NULL;
    GUI_BGN_SAVE;
    int rc = pEdit->GetEventMask();
    GUI_END_SAVE;
    return Py_BuildValue("i", rc);  // @pyseemfc CRichEditCtrl|GetEventMask
}

// @pymethod int|PyCRichEditCtrl|GetLineCount|Gets the number of lines in an edit control.
static PyObject *PyCRichEditCtrl_get_line_count(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS(args);
    CRichEditCtrl *pEdit = GetRichEditCtrl(self);
    if (!pEdit)
        return NULL;
    GUI_BGN_SAVE;
    int rc = pEdit->GetLineCount();
    GUI_END_SAVE;
    return Py_BuildValue("i", rc);  // @pyseemfc CRichEditCtrl|GetLineCount
    // @rdesc The number of lines in the buffer.  If the control is empty, the return value is 1.
}
// @pymethod int|PyCRichEditCtrl|GetFirstVisibleLine|Returns zero-based index of the topmost visible line.
static PyObject *PyCRichEditCtrl_get_first_visible(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS(args);
    CRichEditCtrl *pEdit = GetRichEditCtrl(self);
    if (!pEdit)
        return NULL;
    GUI_BGN_SAVE;
    int rc = pEdit->GetFirstVisibleLine();
    GUI_END_SAVE;
    return Py_BuildValue("i", rc);  // @pyseemfc CRichEditCtrl|GetFirstVisibleLine
    // @rdesc The zero-based index of the topmost visible line. For single-line edit controls, the return value is 0.
}

// @pymethod (tuple)|PyCRichEditCtrl|GetParaFormat|Returns the current paragraph formatting attributes.
static PyObject *PyCRichEditCtrl_get_para_format(PyObject *self, PyObject *args)
{
    CRichEditCtrl *pEdit = GetRichEditCtrl(self);
    if (!pEdit)
        return NULL;
    PARAFORMAT fmt;
    if (!PyArg_ParseTuple(args, ":GetParaFormat"))
        return NULL;
    fmt.cbSize = sizeof(PARAFORMAT);
    GUI_BGN_SAVE;
    pEdit->GetParaFormat(fmt);  // @pyseemfc CRichEditCtrl|GetParaFormat
    GUI_END_SAVE;
    return MakeParaFormatTuple(&fmt);
    // @rdesc The return value is a <om win32ui.PARAFORMAT tuple>
}

// @pymethod (start, end)|PyCRichEditCtrl|GetSel|Returns the start and end of the current selection.
static PyObject *PyCRichEditCtrl_get_sel(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS(args);
    CRichEditCtrl *pEdit = GetRichEditCtrl(self);
    if (!pEdit)
        return NULL;
    long start, end;
    GUI_BGN_SAVE;
    pEdit->GetSel(start, end);  // @pyseemfc CRichEditCtrl|GetSel
    GUI_END_SAVE;
    return Py_BuildValue("(ll)", start, end);
    // @rdesc The return tuple is (the first character in the current selection, first nonselected character past the
    // end of the current selection)
}

// @pymethod (tuple)|PyCRichEditCtrl|GetSelectionCharFormat|Returns the character formatting of the selection.
static PyObject *PyCRichEditCtrl_get_selection_char_format(PyObject *self, PyObject *args)
{
    CRichEditCtrl *pEdit = GetRichEditCtrl(self);
    if (!pEdit)
        return NULL;
    CHARFORMAT fmt;
    memset(&fmt, 0, sizeof(fmt));
    if (!PyArg_ParseTuple(args, ":GetSelectionCharFormat"))
        return NULL;
    fmt.cbSize = sizeof(CHARFORMAT);
    GUI_BGN_SAVE;
    fmt.dwMask = pEdit->GetSelectionCharFormat(fmt);  // @pyseemfc CRichEditCtrl|GetSelectionCharFormat
    GUI_END_SAVE;
    return MakeCharFormatTuple(&fmt);
}

// @pymethod |PyCRichEditCtrl|LimitText|Sets max length of text that user can enter
static PyObject *PyCRichEditCtrl_limit_text(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS(args);
    CRichEditCtrl *pEdit = GetRichEditCtrl(self);
    if (!pEdit)
        return NULL;
    int nChars = 0;
    // @pyparm int|nChars|0|Specifies the length (in bytes) of the text that the user can enter. If this parameter is 0,
    // the text length is set to UINT_MAX bytes. This is the default behavior.
    if (!PyArg_ParseTuple(args, "|i:LimitText", &nChars))
        return NULL;
    GUI_BGN_SAVE;
    pEdit->LimitText(nChars);  // @pyseemfc CRichEditCtrl|LimitText
    GUI_END_SAVE;
    RETURN_NONE;
}

// @pymethod int|PyCRichEditCtrl|LineIndex|Retrieves the character index of a line within a multiple-line edit control.
static PyObject *PyCRichEditCtrl_line_index(PyObject *self, PyObject *args)
{
    // @comm This method only works on multi-linr edit controls.
    CRichEditCtrl *pEdit = GetRichEditCtrl(self);
    if (!pEdit)
        return NULL;
    // @pyparm int|lineNo|-1|Contains the index value for the desired line in the text
    // of the edit control, or contains -1.  If -1, then it specifies the current line.
    int lineNo = -1;
    if (!PyArg_ParseTuple(args, "|i:LineIndex", &lineNo))
        return NULL;
    GUI_BGN_SAVE;
    int rc = pEdit->LineIndex(lineNo);
    GUI_END_SAVE;
    return Py_BuildValue("i", rc);  // @pyseemfc CRichEditCtrl|LineIndex
                                    // @rdesc The character index of the line specified in lineNo, or -1 if
                                    // the specified line number is greater then the number of lines in
                                    // the edit control.
}
// @pymethod int|PyCRichEditCtrl|LineScroll|Scroll the control vertically and horizontally
static PyObject *PyCRichEditCtrl_line_scroll(PyObject *self, PyObject *args)
{
    // @comm This method only works on multi-linr edit controls.
    CRichEditCtrl *pEdit = GetRichEditCtrl(self);
    if (!pEdit)
        return NULL;
    // @pyparm int|nLines||Specifies the number of lines to scroll vertically.
    // @pyparm int|nChars|0|Specifies the number of character positions to scroll horizontally. This value is ignored if
    // the edit control has either the ES_RIGHT or ES_CENTER style.
    int nLines, nChars = 0;
    if (!PyArg_ParseTuple(args, "i|i:LineScroll", &nLines, &nChars))
        return NULL;
    GUI_BGN_SAVE;
    pEdit->LineScroll(nLines, nChars);  // @pyseemfc CRichEditCtrl|LineScroll
    GUI_END_SAVE;
    RETURN_NONE;
}

// @pymethod int|PyCRichEditCtrl|LineFromChar|Returns the line number of the specified character.
static PyObject *PyCRichEditCtrl_line_from_char(PyObject *self, PyObject *args)
{
    CRichEditCtrl *pEdit = GetRichEditCtrl(self);
    if (!pEdit)
        return NULL;
    int charNo = -1;
    // @pyparm int|charNo|-1|Contains the zero-based index value for the desired character in the text of the edit
    // control, or -1.  If -1, then it specifies the current line.
    if (!PyArg_ParseTuple(args, "|i:LineFromChar", &charNo))
        return NULL;
    GUI_BGN_SAVE;
    int rc = pEdit->LineFromChar(charNo);
    GUI_END_SAVE;
    return Py_BuildValue("i", rc);  // @pyseemfc CRichEditCtrl|LineFromChar
    // @rdesc The zero-based line number of the line containing the character index specified by charNo.
    // If charNo is -1, the number of the line that contains the first character of the selection is returned.
    // If there is no selection, the current line number is returned.
}

// @pymethod int|PyCRichEditCtrl|GetLine|Returns the text in a specified line.
static PyObject *PyCRichEditCtrl_get_line(PyObject *self, PyObject *args)
{
    CRichEditCtrl *pEdit = GetRichEditCtrl(self);
    if (!pEdit)
        return NULL;
    GUI_BGN_SAVE;
    int lineNo = pEdit->LineFromChar(-1);
    GUI_END_SAVE;
    // @pyparm int|lineNo|current|Contains the zero-based index value for the desired line.
    // @comm This function is not an MFC wrapper.
    if (!PyArg_ParseTuple(args, "|i:GetLine", &lineNo))
        return NULL;
    int size = 1024;   // ahhhhh-this fails with 128, even when line len==4!
                       // god damn it - try and write a fairly efficient normal case,
                       // and handle worst case, and look what happens!
    CString csBuffer;  // use dynamic mem for buffer
    TCHAR *buf;
    int bytesCopied;
    // this TRACE _always_ returns the length of the first line - hence the
    // convaluted code below.
    //	TRACE("LineLength for line %d is %d\n", lineNo, pView->GetRichEditCtrl().LineLength(lineNo));
    // loop if buffer too small, increasing each time.
    while (size < 0x7FFF)  // reasonable line size max? - maxuint on 16 bit.
    {
        buf = csBuffer.GetBufferSetLength(size);
        if (buf == NULL)
            RETURN_ERR("Out of memory getting Edit control line value");

        GUI_BGN_SAVE;
        bytesCopied = pEdit->GetLine(lineNo, buf, size);
        GUI_END_SAVE;
        if (bytesCopied != size)  // ok - get out.
            break;
        // buffer too small
        size += size;  // try doubling!
        TRACE0("Doubling buffer for GetLine value\n");
    }
    if (bytesCopied == size)  // hit max.
        --bytesCopied;        // so NULL doesnt overshoot.
                              //	if (buf[bytesCopied-1]=='\r' || buf[bytesCopied-1]=='\n')	// kill newlines.
                              //		--bytesCopied;
    buf[bytesCopied] = '\0';
    return PyWinObject_FromTCHAR(buf);
}

// @pymethod |PyCRichEditCtrl|Paste|Pastes the contents of the clipboard into the control.
static PyObject *PyCRichEditCtrl_paste(PyObject *self, PyObject *args)
{
    CHECK_NO_ARGS(args);
    CRichEditCtrl *pEdit = GetRichEditCtrl(self);
    if (!pEdit)
        return NULL;
    GUI_BGN_SAVE;
    pEdit->Paste();  // @pyseemfc CRichEditCtrl|Paste
    GUI_END_SAVE;
    RETURN_NONE;
}

// @pymethod |PyCRichEditCtrl|ReplaceSel|Replaces the selection with the specified text.
static PyObject *PyCRichEditCtrl_replace_sel(PyObject *self, PyObject *args)
{
    CRichEditCtrl *pEdit = GetRichEditCtrl(self);
    TCHAR *msg;
    PyObject *obmsg;
    // @pyparm string|text||The text to replace the selection with.
    if (!pEdit || !PyArg_ParseTuple(args, "O:ReplaceSel", &obmsg) || !PyWinObject_AsTCHAR(obmsg, &msg, FALSE))
        return NULL;
    GUI_BGN_SAVE;
    pEdit->ReplaceSel(msg);  // @pyseemfc CRichEditCtrl|ReplaceSel
    GUI_END_SAVE;
    PyWinObject_FreeTCHAR(msg);
    RETURN_NONE;
}

// @pymethod int|PyCRichEditCtrl|SetBackgroundColor|Sets the background color for the control.
static PyObject *PyCRichEditCtrl_set_background_color(PyObject *self, PyObject *args)
{
    BOOL bSysColor;
    int cr = 0;
    CRichEditCtrl *pEdit = GetRichEditCtrl(self);
    if (!pEdit)
        return NULL;
    // @pyparm int|bSysColor||Indicates if the background color should be set to the system value. If this value is
    // TRUE, cr is ignored.
    // @pyparm int|cr|0|The requested background color. Used only if bSysColor is FALSE.
    if (!PyArg_ParseTuple(args, "i|i:SetBackgroundColor", &bSysColor, &cr))
        return NULL;
    GUI_BGN_SAVE;
    int rc = pEdit->SetBackgroundColor(bSysColor, (COLORREF)cr);
    GUI_END_SAVE;
    return Py_BuildValue("i", rc);  // @pyseemfc CRichEditCtrl|SetEventMask
                                    // @rdesc The return value is the previous background color.
}

// @pymethod int|PyCRichEditCtrl|SetEventMask|Sets the event motification mask.
static PyObject *PyCRichEditCtrl_set_event_mask(PyObject *self, PyObject *args)
{
    int event;
    CRichEditCtrl *pEdit = GetRichEditCtrl(self);
    if (!pEdit)
        return NULL;
    // @pyparm int|eventMask||The new event mask.  Must be one of the win32con.ENM_* flags.
    if (!PyArg_ParseTuple(args, "i:SetEventMask", &event))
        return NULL;
    GUI_BGN_SAVE;
    int rc = pEdit->SetEventMask(event);
    GUI_END_SAVE;
    return Py_BuildValue("i", rc);  // @pyseemfc CRichEditCtrl|SetEventMask
                                    // @rdesc The return value is the previous event mask.
}

// @pymethod |PyCRichEditCtrl|SetDefaultCharFormat|Sets the current default character formatting attributes in this <o
// PyCRichEditCtrl> object.
static PyObject *PyCRichEditCtrl_set_default_char_format(PyObject *self, PyObject *args)
{
    CRichEditCtrl *pEdit = GetRichEditCtrl(self);
    if (!pEdit)
        return NULL;
    CHARFORMAT fmt;
    memset(&fmt, 0, sizeof(fmt));
    fmt.cbSize = sizeof(CHARFORMAT);
    PyObject *fmtTuple;
    // @pyparm tuple|charFormat||A charformat tuple.  See <om win32ui.CHARFORMAT tuple> for details.
    if (!PyArg_ParseTuple(args, "O:SetDefaultCharFormat", &fmtTuple))
        return NULL;
    if (!ParseCharFormatTuple(fmtTuple, &fmt))
        return NULL;
    GUI_BGN_SAVE;
    BOOL ok = pEdit->SetDefaultCharFormat(fmt);
    GUI_END_SAVE;
    if (!ok)  // @pyseemfc CRichEditCtrl|SetDefaultCharFornmat
        RETURN_ERR("SetDefaultCharFormat failed");
    RETURN_NONE;
}

// @pymethod |PyCRichEditCtrl|SetWordCharFormat|Sets the currently selected word's character formatting attributes.
static PyObject *PyCRichEditCtrl_set_word_char_format(PyObject *self, PyObject *args)
{
    CRichEditCtrl *pEdit = GetRichEditCtrl(self);
    if (!pEdit)
        return NULL;
    CHARFORMAT fmt;
    memset(&fmt, 0, sizeof(fmt));
    fmt.cbSize = sizeof(CHARFORMAT);
    PyObject *fmtTuple;
    // @pyparm tuple|charFormat||A charformat tuple.  See <om win32ui.CHARFORMAT tuple> for details.
    if (!PyArg_ParseTuple(args, "O:SetWordCharFormat", &fmtTuple))
        return NULL;
    if (!ParseCharFormatTuple(fmtTuple, &fmt))
        return NULL;
    GUI_BGN_SAVE;
    BOOL ok = pEdit->SetWordCharFormat(fmt);
    GUI_END_SAVE;
    if (!ok)  // @pyseemfc CRichEditCtrl|SetWordCharFormat
        RETURN_ERR("SetWordCharFormat failed");
    RETURN_NONE;
}

// @pymethod int|PyCRichEditCtrl|SetParaFormat|Sets the paragraph formatting
static PyObject *PyCRichEditCtrl_set_para_format(PyObject *self, PyObject *args)
{
    CRichEditCtrl *pEdit = GetRichEditCtrl(self);
    if (!pEdit)
        return NULL;
    PARAFORMAT fmt;
    memset(&fmt, 0, sizeof(fmt));
    // @pyparm tuple|paraFormat||A charformat tuple.  See <om win32ui.PARAFORMAT tuple> for details.
    fmt.cbSize = sizeof(PARAFORMAT);
    PyObject *fmtTuple;
    if (!PyArg_ParseTuple(args, "O:SetParaFormat", &fmtTuple))
        return NULL;
    if (!ParseParaFormatTuple(fmtTuple, &fmt))
        return NULL;
    GUI_BGN_SAVE;
    int rc = pEdit->SetParaFormat(fmt);
    GUI_END_SAVE;
    return Py_BuildValue("i", rc);  // @pyseemfc CRichEditCtrl|SetParaFormat
                                    // @rdesc This function seems to return occasionally return failure, but
                                    // the formatting is applied.  Therefore an exception is not raised on failure,
                                    // but the BOOL return code is passed back.
}

// @pymethod |PyCRichEditCtrl|SetSelectionCharFormat|Sets the current selections character formatting attributes.
static PyObject *PyCRichEditCtrl_set_selection_char_format(PyObject *self, PyObject *args)
{
    CRichEditCtrl *pEdit = GetRichEditCtrl(self);
    if (!pEdit)
        return NULL;
    CHARFORMAT fmt;
    memset(&fmt, 0, sizeof(fmt));
    fmt.cbSize = sizeof(CHARFORMAT);
    fmt.dwMask = 0;
    PyObject *fmtTuple;
    // @pyparm tuple|charFormat||A charformat tuple.  See <om win32ui.CHARFORMAT tuple> for details.
    if (!PyArg_ParseTuple(args, "O:SetSelectionCharFormat", &fmtTuple))
        return NULL;
    if (!ParseCharFormatTuple(fmtTuple, &fmt))
        return NULL;
    GUI_BGN_SAVE;
    BOOL ok = pEdit->SetSelectionCharFormat(fmt);
    GUI_END_SAVE;
    if (!ok)  // @pyseemfc CRichEditCtrl|SetSelectionCharFormat
        RETURN_ERR("SetSelectionCharFormat failed");
    RETURN_NONE;
}

// @pymethod |PyCRichEditCtrl|SetSelAndCharFormat|Sets the selection and char format.
// @comm   Highly optimised for speed for color editors.
static PyObject *PyCRichEditCtrl_set_sel_and_char_format(PyObject *self, PyObject *args)
{
    CRichEditCtrl *pEdit = GetRichEditCtrl(self);
    if (!pEdit)
        return NULL;
    CHARFORMAT fmt;
    memset(&fmt, 0, sizeof(fmt));
    fmt.cbSize = sizeof(CHARFORMAT);
    fmt.dwMask = 0;
    // @pyparm tuple|charFormat||A charformat tuple.  See <om win32ui.CHARFORMAT tuple> for details.
    if (!PyTuple_Check(args) || PyTuple_Size(args) != 3)
        RETURN_TYPE_ERR("Expected exactly 3 arguments.");

    //	PyErr_Clear();
    // Note - no reference added.
    long start = PyInt_AsLong(PyTuple_GET_ITEM(args, 0));
    long end = PyInt_AsLong(PyTuple_GET_ITEM(args, 1));
    PyObject *fmtTuple = PyTuple_GET_ITEM(args, 2);

    if (PyErr_Occurred())
        return NULL;

    if (!ParseCharFormatTuple(fmtTuple, &fmt))
        return NULL;

    GUI_BGN_SAVE;
    pEdit->SetSel(start, end);
    BOOL ok = pEdit->SetSelectionCharFormat(fmt);
    GUI_END_SAVE;
    // @pyseemfc CRichEditCtrl|SetSelectionCharFormat
    // @pyseemfc CRichEditCtrl|SetSel
    if (!ok)
        RETURN_ERR("SetSelectionCharFormat failed");
    RETURN_NONE;
}

// @pymethod |PyCRichEditCtrl|SetOptions|Sets options for the control.
static PyObject *PyCRichEditCtrl_set_options(PyObject *self, PyObject *args)
{
    int op, flags;
    CRichEditCtrl *pEdit = GetRichEditCtrl(self);
    if (!pEdit)
        return NULL;
    // @pyparm int|op||Indicates the operation.  Must be one of the win32con.ECOOP_* flags.
    // @pyparm int|flags||Indicates the options.  Must be one a combination of win32con.ECO_* flags.
    if (!PyArg_ParseTuple(args, "ii:SetOptions", &op, &flags))
        return NULL;
    GUI_BGN_SAVE;
    pEdit->SetOptions(op, flags);  // @pyseemfc CRichEditCtrl|SetOptions
    GUI_END_SAVE;
    RETURN_NONE;
}

// @pymethod |PyCRichEditCtrl|SetReadOnly|Sets or clears the read-only status of the listbox.
static PyObject *PyCRichEditCtrl_set_readonly(PyObject *self, PyObject *args)
{
    CRichEditCtrl *pEdit = GetRichEditCtrl(self);
    if (!pEdit)
        return NULL;
    BOOL bState = TRUE;
    // @pyparm int|bReadOnly|1|The read-only state to set.
    if (!PyArg_ParseTuple(args, "|i:SetReadOnly", &bState))
        return NULL;
    GUI_BGN_SAVE;
    pEdit->SetReadOnly(bState);  // @pyseemfc CRichEditCtrl|SetReadOnly
    GUI_END_SAVE;
    RETURN_NONE;
}

// @pymethod |PyCRichEditCtrl|SetSel|Sets the selection in the edit control.
static PyObject *PyCRichEditCtrl_set_sel(PyObject *self, PyObject *args)
{
    CRichEditCtrl *pEdit = GetRichEditCtrl(self);
    int start = 0, end = 0;
    if (!pEdit)
        return NULL;
    if (!PyArg_ParseTuple(args, "i|i:SetSel",
                          &start,   // @pyparm int|start||Specifies the starting position.
                                    // If start is 0 and end is -1, all the text in the edit control is selected.
                                    // If start is -1, any current selection is removed.
                          &end)) {  // @pyparm int|end|start|Specifies the ending position.
        PyErr_Clear();
        if (!PyArg_ParseTuple(args, "(ii):SetSel",
                              &start,  // @pyparmalt2 (int, int)|start,end)||As for normal start, end args.
                              &end))
            return NULL;
    }
    if (start != end && end == 0)
        end = start;
    GUI_BGN_SAVE;
    pEdit->SetSel(start, end);  // @pyseemfc CRichEditCtrl|SetSel
    GUI_END_SAVE;
    RETURN_NONE;
}

// @pymethod (int,int)|PyCRichEditCtrl|StreamIn|Invokes a callback to stream data into the control.
static PyObject *PyCRichEditCtrl_stream_in(PyObject *self, PyObject *args)
{
    CRichEditCtrl *pEdit = GetRichEditCtrl(self);
    if (!pEdit)
        return NULL;
    int format;
    PyObject *method;
    // @pyparm int|format||The format.  One of the win32con.SF_* flags (SF_TEXT,SF_RTF)
    // @pyparm object|method||A callable object (eg, a method or function)
    // This method is called with a single integer param, which is the maximum number of
    // bytes to fetch.  The method should return a zero length string, or None to
    // finish the operation, and a string otherwise.
    if (!PyArg_ParseTuple(args, "iO:StreamIn", &format, &method))
        return NULL;
    if (!PyCallable_Check(method))
        RETURN_ERR("The method parameter is not callable");
    DOINCREF(method);
    EDITSTREAM es;
    es.dwCookie = (DWORD_PTR)method;
    es.dwError = 0;
    es.pfnCallback = PyCRichEditCallbackIn;
    PyErr_Clear();
    GUI_BGN_SAVE;
    long rc = pEdit->StreamIn(format, es);  // @pyseemfc CRichEditCtrl|StreamIn
    GUI_END_SAVE;
    DODECREF(method);
    return PyErr_Occurred() ? NULL : Py_BuildValue("li", rc, es.dwError);
    // @rdesc The return value is a tuple of (no bytes written, error code)
}

// @pymethod (int, int)|PyCRichEditCtrl|StreamOut|Invokes a callback to stream data into the control.
static PyObject *PyCRichEditCtrl_stream_out(PyObject *self, PyObject *args)
{
    CRichEditCtrl *pEdit = GetRichEditCtrl(self);
    if (!pEdit)
        return NULL;
    int format;
    PyObject *method;
    // @pyparm int|format||The format.  One of the win32con.SF_* flags (SF_TEXT,SF_RTF) and may also combine
    // SFF_SELECTION.
    // @pyparm object|method||A callable object (eg, a method or function)
    // This method is called with a string parameter.  It should return an integer, zero to abort, non zero otherwise.
    if (!PyArg_ParseTuple(args, "iO:StreamOut", &format, &method))
        return NULL;
    if (!PyCallable_Check(method))
        RETURN_ERR("The method parameter is not callable");
    EDITSTREAM es;
    DOINCREF(method);
    es.dwCookie = (DWORD_PTR)method;
    es.dwError = 0;
    es.pfnCallback = PyCRichEditCallbackOut;
    PyErr_Clear();
    GUI_BGN_SAVE;
    long rc = pEdit->StreamOut(format, es);  // @pyseemfc CRichEditCtrl|StreamOut
    GUI_END_SAVE;
    DODECREF(method);
    return PyErr_Occurred() ? NULL : Py_BuildValue("li", rc, es.dwError);
    // @rdesc The return value is a tuple of (no bytes written, error code)
}

// @pymethod int|PyCRichEditCtrl|GetTextLength|Returns the length of the text in the control.
static PyObject *PyCRichEditCtrl_get_text_length(PyObject *self, PyObject *args)
{
    CRichEditCtrl *pEdit = GetRichEditCtrl(self);
    if (!pEdit)
        return NULL;
    CHECK_NO_ARGS2(args, GetTextLength);
    GUI_BGN_SAVE;
    long rc = pEdit->GetTextLength();  // @pyseemfc CRichEditCtrl|GetTextLength
    GUI_END_SAVE;
    return PyInt_FromLong(rc);
}

// @pymethod int|PyCRichEditCtrl|GetModify|Nonzero if the text in this control has been modified; otherwise 0.
static PyObject *PyCRichEditCtrl_get_modify(PyObject *self, PyObject *args)
{
    CRichEditCtrl *pEdit = GetRichEditCtrl(self);
    if (!pEdit)
        return NULL;
    CHECK_NO_ARGS2(args, GetModify);
    GUI_BGN_SAVE;
    BOOL rc = pEdit->GetModify();  // @pyseemfc CRichEditCtrl|GetModify
    GUI_END_SAVE;
    return PyInt_FromLong(rc);
}

// @pymethod |PyCRichEditCtrl|SetModify|Sets the modified flag for this control
static PyObject *PyCRichEditCtrl_set_modify(PyObject *self, PyObject *args)
{
    CRichEditCtrl *pEdit = GetRichEditCtrl(self);
    if (!pEdit)
        return NULL;
    BOOL modified = TRUE;
    // @pyparm int|modified|1|Indicates the new value for the modified flag.
    if (!PyArg_ParseTuple(args, "i:SetModify", &modified))
        return NULL;
    GUI_BGN_SAVE;
    pEdit->SetModify(modified);  // @pyseemfc CRichEditCtrl|SetModify
    GUI_END_SAVE;
    RETURN_NONE;
}

// @pymethod |PyCRichEditCtrl|SetTargetDevice|Sets the target device for the control
static PyObject *PyCRichEditCtrl_set_target_device(PyObject *self, PyObject *args)
{
    CRichEditCtrl *pEdit = GetRichEditCtrl(self);
    if (!pEdit)
        return NULL;
    PyObject *obDC;
    int lineWidth;
    // @pyparm <o PyCDC>|dc||The new DC - may be None
    // @pyparm int|lineWidth||Line width to use for formatting.
    if (!PyArg_ParseTuple(args, "Oi:SetTargetDevice", &obDC, &lineWidth))
        return NULL;

    CDC *pDC = NULL;
    if (obDC != Py_None) {
        pDC = ui_dc_object::GetDC(obDC);
        if (pDC == NULL)
            return NULL;
    }
    GUI_BGN_SAVE;
    BOOL ok = pEdit->SetTargetDevice(*pDC, lineWidth);
    GUI_END_SAVE;
    if (!ok)  // @pyseemfc CRichEditCtrl|SetTargetDevice
        RETURN_ERR("SetTargetDevice failed");
    RETURN_NONE;
}

// @pymethod string|PyCRichEditCtrl|GetSelText|Returns the currently selected text
static PyObject *PyCRichEditCtrl_get_sel_text(PyObject *self, PyObject *args)
{
    CRichEditCtrl *pEdit = GetRichEditCtrl(self);
    if (!pEdit)
        return NULL;
    CHECK_NO_ARGS2(args, GetSelText);
    CString str;
    GUI_BGN_SAVE;
    str = pEdit->GetSelText();
    GUI_END_SAVE;
    return PyWinObject_FromTCHAR(str);
}

// @object PyCRichEditCtrl|A windows Rich Text edit control.  Encapsulates an MFC <c CRichEditCtrl> class.  Derived from
// a <o PyCControl> object.
static struct PyMethodDef PyCRichEditCtrl_methods[] = {
    {"Clear", PyCRichEditCtrl_clear, 1},                 // @pymeth Clear|Clears all text from an edit control.
    {"Copy", PyCRichEditCtrl_copy, 1},                   // @pymeth Copy|Copy the selection to the clipboard.
    {"CreateWindow", PyCRichEditCtrl_create_window, 1},  // @pymeth CreateWindow|Creates a rich edit control.
    {"Cut", PyCRichEditCtrl_cut, 1},             // @pymeth Cut|Cut the selection, and place it in the clipboard.
    {"FindText", PyCRichEditCtrl_find_text, 1},  // @pymeth FindText|Finds text in the control
    {"GetCharPos", PyCRichEditCtrl_get_char_pos,
     1},  // @pymeth GetCharPos|Returns te location of the top-left corner of the specified character.
    {"GetDefaultCharFormat", PyCRichEditCtrl_get_default_char_format,
     1},  // @pymeth GetDefaultCharFormat|Returns the current default character formatting attributes in this <o
          // PyCRichEditCtrl> object.
    {"GetEventMask", PyCRichEditCtrl_get_event_mask, 1},  // @pymeth GetEventMask|Returns the current event mask.
    {"GetSelectionCharFormat", PyCRichEditCtrl_get_selection_char_format,
     1},  // @pymeth GetSelectionCharFormat|Returns the character formatting attributes of the current selection in this
          // <o PyCRichEditCtrl> object.
    {"GetFirstVisibleLine", PyCRichEditCtrl_get_first_visible,
     1},  // @pymeth GetFirstVisibleLine|Returns zero-based index of the topmost visible line.
    {"GetParaFormat", PyCRichEditCtrl_get_para_format,
     1},                                     // @pymeth GetParaFormat|Returns the formatting of the current paragraph.
    {"GetSel", PyCRichEditCtrl_get_sel, 1},  // @pymeth GetSel|Returns the selection.
    {"GetSelText", PyCRichEditCtrl_get_sel_text, 1},  // @pymeth GetSelText|Returns the currently selected text
    {"GetTextLength", PyCRichEditCtrl_get_text_length,
     1},                                       // @pymeth GetTextLength|Returns the length of the text in the control.
    {"GetLine", PyCRichEditCtrl_get_line, 1},  // @pymeth GetLine|Returns a specified line.
    {"GetModify", PyCRichEditCtrl_get_modify, 1},  // @pymeth GetModify|Determines if the control has been modified.
    {"GetLineCount", PyCRichEditCtrl_get_line_count,
     1},  // @pymeth GetLineCount|Returns the number of lines in an edit control.
    {"LimitText", PyCRichEditCtrl_limit_text, 1},  // @pymeth LimitText|Sets max length of text that user can enter
    {"LineFromChar", PyCRichEditCtrl_line_from_char,
     1},                                           // @pymeth LineFromChar|Returns the line number of a given character.
    {"LineIndex", PyCRichEditCtrl_line_index, 1},  // @pymeth LineIndex|Returns the line index
    {"LineScroll", PyCRichEditCtrl_line_scroll,
     1},                                  // @pymeth LineScroll|Scroll the control vertically and horizontally
    {"Paste", PyCRichEditCtrl_paste, 1},  // @pymeth Paste|Pastes the contents of the clipboard into the edit control.
    {"ReplaceSel", PyCRichEditCtrl_replace_sel,
     1},  // @pymeth ReplaceSel|Replace the selection with the specified text.
    {"SetBackgroundColor", PyCRichEditCtrl_set_background_color,
     1},  // @pymeth SetBackgroundColor|Sets the background color for the control.
    {"SetDefaultCharFormat", PyCRichEditCtrl_set_default_char_format,
     1},  // @pymeth SetDefaultCharFormat|Sets the current default character formatting attributes in this
          // PyCRichEditCtrl object.
    {"SetEventMask", PyCRichEditCtrl_set_event_mask, 1},  // @pymeth SetEventMask|Sets the event motification mask.
    {"SetSelectionCharFormat", PyCRichEditCtrl_set_selection_char_format,
     1},  // @pymeth SetSelectionCharFormat|Sets the character formatting attributes for the selection in this
          // PyCRichEditCtrl object.
    {"SetModify", PyCRichEditCtrl_set_modify, 1},           // @pymeth SetModify|Sets the modified flag.
    {"SetOptions", PyCRichEditCtrl_set_options, 1},         // @pymeth SetOptions|Sets options for the control.
    {"SetParaFormat", PyCRichEditCtrl_set_para_format, 1},  // @pymeth SetParaFormat|Sets the paragraph formatting.
    {"SetReadOnly", PyCRichEditCtrl_set_readonly,
     1},                                     // @pymeth SetReadOnly|Set the read only status of an edit control.
    {"SetSel", PyCRichEditCtrl_set_sel, 1},  // @pymeth SetSel|Changes the selection in an edit control.
    {"SetSelAndCharFormat", PyCRichEditCtrl_set_sel_and_char_format,
     1},  // @pymeth SetSelAndCharFormat|Sets the selection and the char format.
    {"SetTargetDevice", PyCRichEditCtrl_set_target_device,
     1},                                         // @pymeth SetTargetDevice|Sets the target device for the control
    {"StreamIn", PyCRichEditCtrl_stream_in, 1},  // @pymeth StreamIn|Invokes a callback to stream data into the control.
    {"StreamOut", PyCRichEditCtrl_stream_out,
     1},  // @pymeth StreamOut|Invokes a callback to stream data out of the control.
    {NULL, NULL}};

PyCCtrlView_Type PyCRichEditCtrl::type("PyCRichEditCtrl", &ui_control_object::type, &PyCRichEditCtrl::type,
                                       RUNTIME_CLASS(CRichEditCtrl), sizeof(PyCRichEditCtrl),
                                       PYOBJ_OFFSET(PyCRichEditCtrl), PyCRichEditCtrl_methods,
                                       GET_PY_CTOR(PyCRichEditCtrl));
