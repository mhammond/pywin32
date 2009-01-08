/* win32ctledit : implementation file

	Edit control object.  Note that these methods are shared by
	the edit view object

	Created July 1995, Mark Hammond (MHammond@skippinet.com.au)

Note that this source file contains embedded documentation.
This documentation consists of marked up text inside the
C comments, and is prefixed with an '@' symbol.  The source
files are processed by a tool called "autoduck" which
generates Windows .hlp files.
@doc

*/
#include "stdafx.h"

#include "win32win.h"
#include "win32control.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

extern CPythonEditView *GetEditViewPtr(PyObject *self);

PyCEdit::PyCEdit()
{
}
PyCEdit::~PyCEdit()
{
}

// @pymethod <o PyCEdit>|win32ui|CreateEdit|Creates an Edit object.  <om PyCEdit.CreateWindow> creates the actual control.
PyObject *
PyCEdit_create(PyObject *self, PyObject *args)
{
	CHECK_NO_ARGS(args);
	CEdit *pBut = new CEdit();
	return ui_assoc_object::make( PyCEdit::type, pBut );
}

CEdit *GetEditCtrl(PyObject *self)
{
	if (ui_base_class::is_uiobject(self, &PyCEditView::type)) {
		CPythonEditView *pView = GetEditViewPtr(self);
		if (pView)
			return &(pView->GetEditCtrl());
		else
			return NULL;
	}
	if (ui_base_class::is_uiobject(self, &PyCEdit::type))
		return (CEdit *)PyCWnd::GetPythonGenericWnd(self);
	RETURN_ERR("Python object can not be used as an edit control.");
}

// @pymethod |PyCEdit|CreateWindow|Creates the window for a new Edit object.
static PyObject *
PyCEdit_create_window(PyObject *self, PyObject *args)
{
	int style, id;
	PyObject *obParent;
	RECT rect;

	if (!PyArg_ParseTuple(args, "i(iiii)Oi:CreateWindow", 
			   &style, // @pyparm int|style||The style for the Edit.  Use any of the win32con.BS_* constants.
			   &rect.left,&rect.top,&rect.right,&rect.bottom,
			   // @pyparm (left, top, right, bottom)|rect||The size and position of the Edit.
			   &obParent, // @pyparm <o PyCWnd>|parent||The parent window of the Edit.  Usually a <o PyCDialog>.
			   &id )) // @pyparm int|id||The Edits control ID. 
		return NULL;

	if (!ui_base_class::is_uiobject(obParent, &PyCWnd::type))
		RETURN_TYPE_ERR("parent argument must be a window object");
	CWnd *pParent = GetWndPtr( obParent );
	if (pParent==NULL)
		return NULL;
	CEdit *pEdit = GetEditCtrl(self);
	if (!pEdit)
		return NULL;

	BOOL ok;
	GUI_BGN_SAVE;
	ok = pEdit->Create(style, rect, pParent, id );
	GUI_END_SAVE;
	if (!ok)
		RETURN_ERR("CEdit::Create");
	RETURN_NONE;
}

/////////////////////////////////////////////////////////////////////
//
// Edit Control object
//
//////////////////////////////////////////////////////////////////////
// @pymethod int|PyCEdit|FmtLines|Sets the formatting options for the control.
static PyObject *PyCEdit_fmt_lines(PyObject *self, PyObject *args)
{
	// @ comm Sets the inclusion of soft line-break characters on or off within a multiple-line edit control.
	// A soft line break consists of two carriage returns and a linefeed inserted at the end of a line that is 
	// broken because of word wrapping.
	// A hard line break consists of one carriage return and a linefeed. 
	// Lines that end with a hard line break are not affected by FmtLines. ~
	// This function is inly effective on multi-line edit controls.
	CEdit *pEdit = GetEditCtrl(self);
	BOOL format; // @pyparm int|bAddEOL||Specifies whether soft line-break characters are to be inserted.
	             // A value of TRUE inserts the characters; a value of FALSE removes them.
	if (!pEdit || !PyArg_ParseTuple(args, "i:FmtLines", &format))
		return NULL;
	GUI_BGN_SAVE;
	BOOL rc = pEdit->FmtLines( format ); // @pyseemfc CEdit|FmtLines
	GUI_END_SAVE;
	return Py_BuildValue("i", rc );
	// @rdesc Nonzero if any formatting occurs; otherwise 0.
}
// @pymethod (start, end)|PyCEdit|GetSel|Returns the start and end of the current selection.
static PyObject *PyCEdit_get_sel(PyObject *self, PyObject *args)
{
	CHECK_NO_ARGS(args);
	CEdit *pEdit = GetEditCtrl(self);
	if (!pEdit)
		return NULL;
	int start,end;
	GUI_BGN_SAVE;
	pEdit->GetSel(start,end); // @pyseemfc CEdit|GetSel
	GUI_END_SAVE;
	return Py_BuildValue("(ii)",start,end);
	// @rdesc The return tuple is (the first character in the current selection, first nonselected character past the end of the current selection)
}
// @pymethod |PyCEdit|SetSel|Sets the selection in the edit control.
static PyObject *PyCEdit_set_sel(PyObject *self, PyObject *args)
{
	CEdit *pEdit = GetEditCtrl(self);
	int start=0,end=0;
	BOOL bNoScroll = FALSE;
	if (!pEdit)
		return NULL;
	if (!PyArg_ParseTuple(args, "i|ii:SetSel", 
	                    &start, // @pyparm int|start||Specifies the starting position. 
	                            // If start is 0 and end is -1, all the text in the edit control is selected. 
	                            // If start is -1, any current selection is removed.
	                    &end,   // @pyparm int|end|start|Specifies the ending position. 
	                    &bNoScroll)) {  // @pyparm int|bNoScroll|0|Indicates whether the caret should be scrolled into view. If 0, the caret is scrolled into view. If 1, the caret is not scrolled into view.
		PyErr_Clear();
		bNoScroll = FALSE;
		if (!PyArg_ParseTuple(args, "(ii)|i:SetSel", 
							&start, // @pyparmalt2 (int, int)|start,end)||As for normal start, end args.
							&end,  
							&bNoScroll)) // @pyparmalt2 int|bNoScroll|0|Indicates whether the caret should be scrolled into view. If 0, the caret is scrolled into view. If 1, the caret is not scrolled into view.
			return NULL;
	}
	if (start!=end && end==0)
		end=start;
	GUI_BGN_SAVE;
	pEdit->SetSel(start,end,bNoScroll);	 // @pyseemfc CEdit|SetSel
	GUI_END_SAVE;
	RETURN_NONE;
}
// @pymethod |PyCEdit|Cut|Cuts the current selection to the clipboard.
static PyObject *PyCEdit_cut(PyObject *self, PyObject *args)
{
	CHECK_NO_ARGS(args);
	CEdit *pEdit = GetEditCtrl(self);
	if (!pEdit)
		return NULL;
	GUI_BGN_SAVE;
	pEdit->Cut(); // @pyseemfc CEdit|Cut
	GUI_END_SAVE;
	RETURN_NONE;
}
// @pymethod |PyCEdit|Copy|Copys the current selection to the clipboard.
static PyObject *PyCEdit_copy(PyObject *self, PyObject *args)
{
	CHECK_NO_ARGS(args);
	CEdit *pEdit = GetEditCtrl(self);
	if (!pEdit)
		return NULL;
	GUI_BGN_SAVE;
	pEdit->Copy(); // @pyseemfc CEdit|Copy
	GUI_END_SAVE;
	RETURN_NONE;
}
// @pymethod |PyCEdit|Paste|Pastes the contents of the clipboard into the control.
static PyObject *PyCEdit_paste(PyObject *self, PyObject *args)
{
	CHECK_NO_ARGS(args);
	CEdit *pEdit = GetEditCtrl(self);
	if (!pEdit)
		return NULL;
	GUI_BGN_SAVE;
	pEdit->Paste(); // @pyseemfc CEdit|Paste
	GUI_END_SAVE;
	RETURN_NONE;
}

// @pymethod |PyCEdit|ReplaceSel|Replaces the selection with the specified text.
static PyObject *PyCEdit_replace_sel(PyObject *self, PyObject *args)
{
	CEdit *pEdit = GetEditCtrl(self);
	TCHAR *msg;
	PyObject *obmsg;
	// @pyparm string|text||The text to replace the selection with.
	if (!pEdit
		|| !PyArg_ParseTuple(args, "O:ReplaceSel", &obmsg)
		|| !PyWinObject_AsTCHAR(obmsg, &msg, FALSE))
		return NULL;
	GUI_BGN_SAVE;
	pEdit->ReplaceSel(msg); // @pyseemfc CEdit|ReplaceSel
	GUI_END_SAVE;
	PyWinObject_FreeTCHAR(msg);
	RETURN_NONE;
}

// @pymethod |PyCEdit|SetReadOnly|Sets or clears the read-only status of the listbox.
static PyObject *
PyCEdit_set_readonly(PyObject *self, PyObject *args)
{
	CEdit *pEdit = GetEditCtrl(self);
	if (!pEdit)
		return NULL;
	BOOL bState = TRUE;
	// @pyparm int|bReadOnly|1|The read-only state to set.
	if (!PyArg_ParseTuple(args, "|i:SetReadOnly", &bState))
		return NULL;
	GUI_BGN_SAVE;
	pEdit->SetReadOnly(bState);	// @pyseemfc CEdit|SetReadOnly
	GUI_END_SAVE;
	RETURN_NONE;	
}
// @pymethod int|PyCEdit|GetLineCount|Gets the number of lines in an edit control.
static PyObject *
PyCEdit_get_line_count(PyObject *self, PyObject *args)
{
	CHECK_NO_ARGS(args);
	CEdit *pEdit = GetEditCtrl(self);
	if (!pEdit	)
		return NULL;
	GUI_BGN_SAVE;
	int rc = pEdit->GetLineCount();
	GUI_END_SAVE;
	return Py_BuildValue("i", rc); // @pyseemfc CEdit|GetLineCount
	// @rdesc The number of lines in the buffer.  If the control is empty, the return value is 1.
}
// @pymethod int|PyCEdit|GetFirstVisibleLine|Returns zero-based index of the topmost visible line.
static PyObject *
PyCEdit_get_first_visible(PyObject *self, PyObject *args)
{
	CHECK_NO_ARGS(args);
	CEdit *pEdit = GetEditCtrl(self);
	if (!pEdit)
		return NULL;
	GUI_BGN_SAVE;
	int rc = pEdit->GetFirstVisibleLine();
	GUI_END_SAVE;

	return Py_BuildValue("i",rc); // @pyseemfc CEdit|GetFirstVisibleLine
	// @rdesc The zero-based index of the topmost visible line. For single-line edit controls, the return value is 0.
}
// @pymethod |PyCEdit|LimitText|Sets max length of text that user can enter
static PyObject *PyCEdit_limit_text(PyObject *self, PyObject *args)
{
	CEdit *pEdit = GetEditCtrl(self);
	if (!pEdit)
		return NULL;
	int nChars = 0;
	// @pyparm int|nChars|0|Specifies the length (in bytes) of the text that the user can enter. If this parameter is 0, the text length is set to 
	// UINT_MAX bytes. This is the default behavior.
	if (!PyArg_ParseTuple(args, "|i:LimitText", &nChars))
		return NULL;
	GUI_BGN_SAVE;
	pEdit->LimitText(nChars); // @pyseemfc CEdit|LimitText
	GUI_END_SAVE;
	RETURN_NONE;
}

// @pymethod int|PyCEdit|LineIndex|Retrieves the character index of a line within a multiple-line edit control.
static PyObject *
PyCEdit_line_index(PyObject *self, PyObject *args)
{
	// @comm This method only works on multi-linr edit controls.
	CEdit *pEdit = GetEditCtrl(self);
	if (!pEdit)
		return NULL;
	// @pyparm int|lineNo|-1|Contains the index value for the desired line in the text 
	// of the edit control, or contains -1.  If -1, then it specifies the current line.
	int lineNo = -1;
	if (!PyArg_ParseTuple(args, "|i:LineIndex", &lineNo))
		return NULL;
	GUI_BGN_SAVE;
	long rc = pEdit->LineIndex(lineNo);
	GUI_END_SAVE;
	return Py_BuildValue("i",rc); // @pyseemfc CEdit|LineIndex
	// @rdesc The character index of the line specified in lineNo, or -1 if 
	// the specified line number is greater then the number of lines in 
	// the edit control.
}
// @pymethod int|PyCEdit|LineScroll|Scroll the control vertically and horizontally
static PyObject *
PyCEdit_line_scroll(PyObject *self, PyObject *args)
{
	// @comm This method only works on multi-linr edit controls.
	CEdit *pEdit = GetEditCtrl(self);
	if (!pEdit)
		return NULL;
	// @pyparm int|nLines||Specifies the number of lines to scroll vertically.
	// @pyparm int|nChars|0|Specifies the number of character positions to scroll horizontally. This value is ignored if the edit control has either the
	// ES_RIGHT or ES_CENTER style.
	int nLines, nChars = 0;
	if (!PyArg_ParseTuple(args, "i|i:LineScroll", &nLines, &nChars))
		return NULL;
	GUI_BGN_SAVE;
	pEdit->LineScroll(nLines, nChars); // @pyseemfc CEdit|LineScroll
	GUI_END_SAVE;
	RETURN_NONE;
}

// @pymethod int|PyCEdit|LineFromChar|Returns the line number of the specified character.
static PyObject *
PyCEdit_line_from_char(PyObject *self, PyObject *args)
{
	CEdit *pEdit = GetEditCtrl(self);
	if (!pEdit)
		return NULL;
	int charNo=-1;
	// @pyparm int|charNo|-1|Contains the zero-based index value for the desired character in the text of the edit 
	// control, or -1.  If -1, then it specifies the current line.
	if (!PyArg_ParseTuple(args, "|i:LineFromChar", &charNo))
		return NULL;
	GUI_BGN_SAVE;
	int rc = pEdit->LineFromChar(charNo);
	GUI_END_SAVE;
	return Py_BuildValue("i", rc); // @pyseemfc CEdit|LineFromChar
	// @rdesc The zero-based line number of the line containing the character index specified by charNo. 
	// If charNo is -1, the number of the line that contains the first character of the selection is returned.
	// If there is no selection, the current line number is returned.
}

// @pymethod int|PyCEdit|GetLine|Returns the text in a specified line.
static PyObject *
PyCEdit_get_line(PyObject *self, PyObject *args)
{
	CEdit *pEdit = GetEditCtrl(self);
	if (!pEdit)
		return NULL;
	GUI_BGN_SAVE;
	int lineNo = pEdit->LineFromChar();
	GUI_END_SAVE;
	// @pyparm int|lineNo|current|Contains the zero-based index value for the desired line.
	// @comm This function is not an MFC wrapper.
	if (!PyArg_ParseTuple(args, "|i:GetLine", &lineNo))
		return NULL;
	int size = 1024;	// ahhhhh-this fails with 128, even when line len==4!
					// god damn it - try and write a fairly efficient normal case,
					// and handle worst case, and look what happens!
	CString csBuffer;			// use dynamic mem for buffer
	TCHAR *buf;
	int bytesCopied;
	// this TRACE _always_ returns the length of the first line - hence the
	// convaluted code below.
//	TRACE("LineLength for line %d is %d\n", lineNo, pView->GetEditCtrl().LineLength(lineNo));
	// loop if buffer too small, increasing each time.
	while (size<0x7FFF)			// reasonable line size max? - maxuint on 16 bit.
	{
		buf = csBuffer.GetBufferSetLength(size);
		if (buf==NULL)
			RETURN_ERR("Out of memory getting Edit control line value");

		GUI_BGN_SAVE;
		bytesCopied = pEdit->GetLine(lineNo, buf, size);
		GUI_END_SAVE;
		if (bytesCopied!=size)	// ok - get out.
			break;
		// buffer too small
		size += size;	// try doubling!
		TRACE0("Doubling buffer for GetLine value\n");
	}
	if (bytesCopied==size)	// hit max.
		--bytesCopied;	// so NULL doesnt overshoot.
	if (buf[bytesCopied-1]=='\r' || buf[bytesCopied-1]=='\n')	// kill newlines.
		--bytesCopied;
	buf[bytesCopied] = '\0';
	return PyWinObject_FromTCHAR(buf);
}

// @pymethod int|PyCEdit|Clear|Clears all text in an edit control.
static PyObject *
PyCEdit_clear(PyObject *self, PyObject *args)
{
	CHECK_NO_ARGS(args);
	CEdit *pEdit = GetEditCtrl(self);
	if (!pEdit	)
		return NULL;
	GUI_BGN_SAVE;
	pEdit->Clear(); // @pyseemfc CEdit|Clear
	GUI_END_SAVE;
	RETURN_NONE;
}

// @object PyCEdit|A windows edit control.  Encapsulates an MFC <c CEdit> class.  Derived from a <o PyCControl> object.
static struct PyMethodDef PyCEdit_methods[] = {
	{"CreateWindow",    PyCEdit_create_window,1}, // @pymeth CreateWindow|Creates the window for a new edit object.
	{"Clear",			PyCEdit_clear,				1}, // @pymeth Clear|Clears all text from an edit control.
	{"Copy",			PyCEdit_copy,				1}, // @pymeth Copy|Copy the selection to the clipboard.
	{"Cut",				PyCEdit_cut,				1}, // @pymeth Cut|Cut the selection, and place it in the clipboard.
	{"FmtLines",		PyCEdit_fmt_lines,			1}, // @pymeth FmtLines|Change the formatting options for the edit control
	{"GetFirstVisibleLine",PyCEdit_get_first_visible,1}, // @pymeth GetFirstVisibleLine|Returns zero-based index of the topmost visible line.
	{"GetSel",			PyCEdit_get_sel,			1}, // @pymeth GetSel|Returns the selection.
	{"GetLine",			PyCEdit_get_line,			1}, // @pymeth GetLine|Returns a specified line.
	{"GetLineCount",	PyCEdit_get_line_count,		1}, // @pymeth GetLineCount|Returns the number of lines in an edit control.
	{"LimitText",	    PyCEdit_limit_text,			1}, // @pymeth LimitText|Sets max length of text that user can enter
	{"LineFromChar",	PyCEdit_line_from_char,		1}, // @pymeth LineFromChar|Returns the line number of a given character.
	{"LineIndex",		PyCEdit_line_index,			1}, // @pymeth LineIndex|Returns the line index
	{"LineScroll",		PyCEdit_line_scroll,		1}, // @pymeth LineScroll|Scroll the control vertically and horizontally
	{"Paste",			PyCEdit_paste,				1}, // @pymeth Paste|Pastes the contents of the clipboard into the edit control.
	{"ReplaceSel",		PyCEdit_replace_sel,		1}, // @pymeth ReplaceSel|Replace the selection with the specified text.
	{"SetReadOnly",		PyCEdit_set_readonly,		1}, // @pymeth SetReadOnly|Set the read only status of an edit control.
	{"SetSel",			PyCEdit_set_sel,			1}, // @pymeth SetSel|Changes the selection in an edit control.
	{NULL,			NULL}		// sentinel
};

ui_type_CObject PyCEdit::type("PyCEdit",
							  &ui_control_object::type, 
							  RUNTIME_CLASS(CEdit), 
							  sizeof(PyCEdit), 
							  PYOBJ_OFFSET(PyCEdit), 
							  PyCEdit_methods, 
							  GET_PY_CTOR(PyCEdit));
