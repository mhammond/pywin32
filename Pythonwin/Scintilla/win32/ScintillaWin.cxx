// Scintilla source code edit control
// ScintillaWin.cxx - Windows specific subclass of ScintillaBase
// Copyright 1998-2000 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "Platform.h"

#include "Scintilla.h"
#ifdef SCI_LEXER
#include "SciLexer.h"
#include "PropSet.h"
#include "Accessor.h"
#include "KeyWords.h"
#endif
#include "ContractionState.h"
#include "SVector.h"
#include "CellBuffer.h"
#include "CallTip.h"
#include "KeyMap.h"
#include "Indicator.h"
#include "LineMarker.h"
#include "Style.h"
#include "AutoComplete.h"
#include "ViewStyle.h"
#include "Document.h"
#include "Editor.h"
#include "ScintillaBase.h"

//#include "CElapsed.h"

#ifndef SPI_GETWHEELSCROLLLINES
#define SPI_GETWHEELSCROLLLINES   104
#endif

#ifndef WM_IME_STARTCOMPOSITION
#include <imm.h>
#endif

#include <commctrl.h>
#ifndef __BORLANDC__
#include <zmouse.h>
#endif
#include <ole2.h>

#ifndef MK_ALT
#define MK_ALT 32
#endif

// TOTAL_CONTROL ifdef surrounds code that will only work when ScintillaWin
// is derived from ScintillaBase (all features) rather than directly from Editor (lightweight editor).
#define TOTAL_CONTROL

// GCC has trouble with the standard COM ABI so do it the old C way with explicit vtables.

class ScintillaWin; 	// Forward declaration for COM interface subobjects

class FormatEnumerator {
public:
	void **vtbl;
	int ref;
	int pos;
	FormatEnumerator(int pos_);
};

class DropSource {
public:
	void **vtbl;
	ScintillaWin *sci;
	DropSource();
};

class DataObject {
public:
	void **vtbl;
	ScintillaWin *sci;
	DataObject();
};

class DropTarget {
public:
	void **vtbl;
	ScintillaWin *sci;
	DropTarget();
};

class ScintillaWin :
	public ScintillaBase {

	bool capturedMouse;

	UINT cfColumnSelect;
	
	DropSource ds;
	DataObject dob;
	DropTarget dt;

	static HINSTANCE hInstance;

	ScintillaWin(HWND hwnd);
	virtual ~ScintillaWin();

	virtual void Initialise();
	virtual void Finalise();

	static LRESULT PASCAL SWndProc(
		    HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
	static LRESULT PASCAL CTWndProc(
		    HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

	virtual void StartDrag();
	virtual LRESULT WndProc(UINT iMessage, WPARAM wParam, LPARAM lParam);
	virtual LRESULT DefWndProc(UINT iMessage, WPARAM wParam, LPARAM lParam);
	virtual void SetTicking(bool on);
	virtual void SetMouseCapture(bool on);
	virtual bool HaveMouseCapture();
	virtual void ScrollText(int linesToMove);
	virtual void SetVerticalScrollPos();
	virtual void SetHorizontalScrollPos();
	virtual bool ModifyScrollBars(int nMax, int nPage);
	virtual void NotifyChange();
	virtual void NotifyFocus(bool focus);
	virtual void NotifyParent(SCNotification scn);
	virtual void NotifyDoubleClick(Point pt, bool shift);
	virtual void Copy();
	virtual void Paste();
	virtual void CreateCallTipWindow(PRectangle rc);
	virtual void AddToPopUp(const char *label, int cmd = 0, bool enabled = true);
	virtual void ClaimSelection();

	// DBCS
	void ImeStartComposition();
	void ImeEndComposition();

	void GetIntelliMouseParameters();
	HGLOBAL GetSelText();
	void ScrollMessage(WPARAM wParam);
	void HorizontalScrollMessage(WPARAM wParam);
	void RealizeWindowPalette(bool inBackGround);
	void FullPaint();

public:
	// Implement IUnknown
	STDMETHODIMP QueryInterface(REFIID riid, PVOID *ppv);
	STDMETHODIMP_(ULONG)AddRef();
	STDMETHODIMP_(ULONG)Release();

	// Implement IDropTarget
	STDMETHODIMP DragEnter(LPDATAOBJECT pIDataSource, DWORD grfKeyState,
	                       POINTL pt, PDWORD pdwEffect);
	STDMETHODIMP DragOver(DWORD grfKeyState, POINTL pt, PDWORD pdwEffect);
	STDMETHODIMP DragLeave();
	STDMETHODIMP Drop(LPDATAOBJECT pIDataSource, DWORD grfKeyState,
	                  POINTL pt, PDWORD pdwEffect);

	// Implement important part of IDataObject
	STDMETHODIMP GetData(FORMATETC *pFEIn, STGMEDIUM *pSTM);

	static void Register(HINSTANCE hInstance_);
	friend class DropSource;
	friend class DataObject;
	friend class DropTarget;
	bool DragIsRectangularOK(UINT fmt) {
		return dragIsRectangle && (fmt == cfColumnSelect);
	}
};

HINSTANCE ScintillaWin::hInstance = 0;

ScintillaWin::ScintillaWin(HWND hwnd) {

	capturedMouse = false;

	// There does not seem to be a real standard for indicating that the clipboard contains a rectangular
	// selection, so copy Developer Studio.
	cfColumnSelect = ::RegisterClipboardFormat("MSDEVColumnSelect");
	
	wMain = hwnd;
	wDraw = hwnd;

	dob.sci = this;
	ds.sci = this;
	dt.sci = this;

	Initialise();
}

ScintillaWin::~ScintillaWin() {}

void ScintillaWin::Initialise() {
	// Initialize COM.  If the app has already done this it will have
	// no effect.  If the app hasnt, we really shouldnt ask them to call
	// it just so this internal feature works.
	OleInitialize(NULL);
}

void ScintillaWin::Finalise() {
	ScintillaBase::Finalise();
	SetTicking(false);
	RevokeDragDrop(wMain.GetID());
	OleUninitialize();
}

void ScintillaWin::StartDrag() {
	DWORD dwEffect = 0;
	dropWentOutside = true;
	IDataObject *pDataObject = reinterpret_cast<IDataObject *>(&dob);
	IDropSource *pDropSource = reinterpret_cast<IDropSource *>(&ds);
	//Platform::DebugPrintf("About to DoDragDrop %x %x\n", pDataObject, pDropSource);
	HRESULT hr = DoDragDrop(
	                 pDataObject,
	                 pDropSource,
	                 DROPEFFECT_COPY | DROPEFFECT_MOVE, &dwEffect);
	//Platform::DebugPrintf("DoDragDrop = %x\n", hr);
	if (SUCCEEDED(hr)) {
		if ((hr == DRAGDROP_S_DROP) && (dwEffect == DROPEFFECT_MOVE) && dropWentOutside) {
			// Remove dragged out text
			ClearSelection();
		}
	}
	inDragDrop = false;
	SetDragPosition(invalidPosition);
}

// Avoid warnings everywhere for old style casts by conecntrating them here
static WORD LoWord(DWORD l) {
	return LOWORD(l);
}

static WORD HiWord(DWORD l) {
	return HIWORD(l);
}

LRESULT ScintillaWin::WndProc(UINT iMessage, WPARAM wParam, LPARAM lParam) {
	switch (iMessage) {

	case WM_CREATE:
		ctrlID = wMain.GetDlgCtrlID();
		// Get Intellimouse scroll line parameters
		GetIntelliMouseParameters();
		RegisterDragDrop(wMain.GetID(), reinterpret_cast<IDropTarget *>(&dt));
		break;

	case WM_COMMAND:
#ifdef TOTAL_CONTROL
		if (LoWord(wParam) == idAutoComplete) {
			int cmd = HiWord(wParam);
			if (cmd == LBN_DBLCLK) {
				AutoCompleteCompleted();
			} else {
				if (cmd != LBN_SETFOCUS)
					SetFocus(wMain.GetID());
			}
		}
		Command(LoWord(wParam));
#endif
		break;

	case WM_PAINT: {
                        //CElapsed ce; ce.Begin();
			paintState = painting;
			PAINTSTRUCT ps;
			BeginPaint(wMain.GetID(), &ps);
			Surface surfaceWindow;
			surfaceWindow.Init(ps.hdc);
			rcPaint = PRectangle(ps.rcPaint.left, ps.rcPaint.top, ps.rcPaint.right, ps.rcPaint.bottom);
			PRectangle rcText = GetTextRectangle();
			paintingAllText = rcPaint.Contains(rcText);
			if (paintingAllText) {
				//Platform::DebugPrintf("Performing full text paint\n");
			} else {
				//Platform::DebugPrintf("Performing partial paint %d .. %d\n", rcPaint.top, rcPaint.bottom);
			}
			Paint(&surfaceWindow, rcPaint);
			surfaceWindow.Release();
			EndPaint(wMain.GetID(), &ps);
			if (paintState == paintAbandoned) {
				// Painting area was insufficient to cover new styling or brace highlight positions
				FullPaint();
			}
			paintState = notPainting;
                        //Platform::DebugPrintf("Paint took %g\n", ce.End());
		}
		break;

	case WM_VSCROLL:
		ScrollMessage(wParam);
		break;

	case WM_HSCROLL:
		HorizontalScrollMessage(wParam);
		break;

	case WM_SIZE: {
			//Platform::DebugPrintf("S start wnd proc %d %d %d\n",iMessage, wParam, lParam);
			PRectangle rsClient(0, 0, LoWord(lParam), HiWord(lParam));
			SetScrollBarsTo(rsClient);
			DropGraphics();
		}
		break;

	case WM_MOUSEWHEEL:
		// Don't handle datazoom.
		// (A good idea for datazoom would be to "fold" or "unfold" details.
		// i.e. if datazoomed out only class structures are visible, when datazooming in the control
		// structures appear, then eventually the individual statements...)
		if (wParam & MK_SHIFT) {
			return DefWindowProc(wMain.GetID(), iMessage, wParam, lParam);
		}

		// Either SCROLL or ZOOM. We handle the wheel steppings calculation
		cWheelDelta -= static_cast<short>(HiWord(wParam));
		if (abs(cWheelDelta) >= WHEEL_DELTA && ucWheelScrollLines > 0) {
			int cLineScroll;
			cLineScroll = ucWheelScrollLines;
			if (cLineScroll == 0) {
				cLineScroll++;
			}
			cLineScroll *= (cWheelDelta / WHEEL_DELTA);
			cWheelDelta = cWheelDelta % WHEEL_DELTA;

			if (wParam & MK_CONTROL) {
				// Zoom! We play with the font sizes in the styles.
				// Number of steps/line is ignored, we just care if sizing up or down
				if (cLineScroll < 0) {
					KeyCommand(SCI_ZOOMIN);
				} else {
					KeyCommand(SCI_ZOOMOUT);
				}
			} else {
				// Scroll
				ScrollTo(topLine + cLineScroll);
			}
		}
		return 0;

	case WM_TIMER:
		Tick();
		break;

	case WM_GETMINMAXINFO:
		return DefWindowProc(wMain.GetID(), iMessage, wParam, lParam);

	case WM_LBUTTONDOWN:
		//Platform::DebugPrintf("Buttdown %d %x %x %x %x %x\n",iMessage, wParam, lParam, 
		//	Platform::IsKeyDown(VK_SHIFT), 
		//	Platform::IsKeyDown(VK_CONTROL),
		//	Platform::IsKeyDown(VK_MENU));
		ButtonDown(Point::FromLong(lParam), GetTickCount(), 
			wParam & MK_SHIFT, wParam & MK_CONTROL, Platform::IsKeyDown(VK_MENU));
		SetFocus(wMain.GetID());
		break;

	case WM_MOUSEMOVE:
		ButtonMove(Point::FromLong(lParam));
		break;

	case WM_LBUTTONUP:
		ButtonUp(Point::FromLong(lParam), GetTickCount(), wParam & MK_CONTROL);
		break;

	case WM_SETCURSOR:
		if (LoWord(lParam) == HTCLIENT) {
			if (inDragDrop) {
				wDraw.SetCursor(Window::cursorUp);
			} else {
				// Display regular (drag) cursor over selection
				POINT pt;
				::GetCursorPos(&pt);
				::ScreenToClient(wMain.GetID(), &pt);
				if (PointInSelMargin(Point(pt.x, pt.y))) {
					wDraw.SetCursor(Window::cursorReverseArrow);
				} else if (PointInSelection(Point(pt.x, pt.y))) {
					wDraw.SetCursor(Window::cursorArrow);
				} else {
					wDraw.SetCursor(Window::cursorText);
				}
			}
			return TRUE;
		} else
			return DefWindowProc(wMain.GetID(), iMessage, wParam, lParam);

	case WM_CHAR:
		//Platform::DebugPrintf("S char proc %d %x %x\n",iMessage, wParam, lParam);
		if (!iscntrl(wParam&0xff))
			AddChar(static_cast<char>(wParam&0xff));
		return 1;

	case WM_KEYDOWN:
		//Platform::DebugPrintf("S keydown %d %x %x %x %x\n",iMessage, wParam, lParam, ::IsKeyDown(VK_SHIFT), ::IsKeyDown(VK_CONTROL));
		return KeyDown(wParam, Platform::IsKeyDown(VK_SHIFT),
		               Platform::IsKeyDown(VK_CONTROL), false);

	case WM_KEYUP:
		//Platform::DebugPrintf("S keyup %d %x %x\n",iMessage, wParam, lParam);
		break;

	case WM_SETTINGCHANGE:
		//Platform::DebugPrintf("Setting Changed\n");
		InvalidateStyleData();
		// Get Intellimouse scroll line parameters
		GetIntelliMouseParameters();
		break;

	case WM_GETDLGCODE:
		return DLGC_HASSETSEL | DLGC_WANTALLKEYS;

	case WM_KILLFOCUS:
		NotifyFocus(false);
		DropCaret();
		//RealizeWindowPalette(true);
		break;

	case WM_SETFOCUS:
		NotifyFocus(true);
		ShowCaretAtCurrentPosition();
		RealizeWindowPalette(false);
		break;

	case WM_SYSCOLORCHANGE:
		//Platform::DebugPrintf("Setting Changed\n");
		InvalidateStyleData();
		break;

	case WM_PALETTECHANGED:
		if (wParam != reinterpret_cast<unsigned int>(wMain.GetID())) {
			//Platform::DebugPrintf("** Palette Changed\n");
			RealizeWindowPalette(true);
		}
		break;

	case WM_QUERYNEWPALETTE:
		//Platform::DebugPrintf("** Query palette\n");
		RealizeWindowPalette(false);
		break;

	case WM_IME_STARTCOMPOSITION: 	// dbcs
		ImeStartComposition();
		return DefWindowProc(wMain.GetID(), iMessage, wParam, lParam);

	case WM_IME_ENDCOMPOSITION: 	// dbcs
		ImeEndComposition();
		return DefWindowProc(wMain.GetID(), iMessage, wParam, lParam);

	case WM_CONTEXTMENU:
#ifdef TOTAL_CONTROL
		ContextMenu(Point::FromLong(lParam));
#endif
		break;

	case EM_CANPASTE: {
			OpenClipboard(wMain.GetID());
			HGLOBAL hmemSelection = GetClipboardData(CF_TEXT);
			if (hmemSelection)
				GlobalUnlock(hmemSelection);
			CloseClipboard();
			return hmemSelection != 0;
		}

	case EM_SCROLL: {
			int topStart = topLine;
			ScrollMessage(wParam);
			return MAKELONG(topLine - topStart, TRUE);
		}

	default:
		return ScintillaBase::WndProc(iMessage, wParam, lParam);
	}
	return 0l;
}

LRESULT ScintillaWin::DefWndProc(UINT iMessage, WPARAM wParam, LPARAM lParam) {
	return ::DefWindowProc(wMain.GetID(), iMessage, wParam, lParam);
}

void ScintillaWin::SetTicking(bool on) {
	if (timer.ticking != on) {
		timer.ticking = on;
		if (timer.ticking) {
			timer.tickerID = ::SetTimer(wMain.GetID(), 1, timer.tickSize, NULL);
		} else {
			::KillTimer(wMain.GetID(), timer.tickerID);
			timer.tickerID = 0;
		}
	}
	timer.ticksToWait = caret.period;
}

void ScintillaWin::SetMouseCapture(bool on) {
	if (on) {
		::SetCapture(wMain.GetID());
	} else {
		::ReleaseCapture();
	}
	capturedMouse = on;
}

bool ScintillaWin::HaveMouseCapture() {
	// Cannot just see if GetCapture is this window as the scroll bar also sets capture for the window
	return capturedMouse && (::GetCapture() == wMain.GetID());
}

void ScintillaWin::ScrollText(int linesToMove) {
	//Platform::DebugPrintf("ScintillaWin::ScrollText %d\n", linesToMove);
	::ScrollWindow(wMain.GetID(), 0, 
		vs.lineHeight * (linesToMove), 0, 0);
	::UpdateWindow(wMain.GetID());
}

void ScintillaWin::SetVerticalScrollPos() {
	::SetScrollPos(wMain.GetID(), SB_VERT, topLine, TRUE);
}

void ScintillaWin::SetHorizontalScrollPos() {
	::SetScrollPos(wMain.GetID(), SB_HORZ, xOffset, TRUE);
}

bool ScintillaWin::ModifyScrollBars(int nMax, int nPage) {
	bool modified = false;
	SCROLLINFO sci = {
	    sizeof(sci)
	};
	sci.fMask = SIF_PAGE | SIF_RANGE;
	BOOL bz = ::GetScrollInfo(wMain.GetID(), SB_VERT, &sci);
	if ((sci.nMin != 0) || (sci.nMax != pdoc->LinesTotal()) ||
	        (sci.nPage != (pdoc->LinesTotal() - MaxScrollPos() + 1)) ||
	        (sci.nPos != 0)) {
		//Platform::DebugPrintf("Scroll info changed %d %d %d %d %d\n",
		//	sci.nMin, sci.nMax, sci.nPage, sci.nPos, sci.nTrackPos);
		sci.fMask = SIF_PAGE | SIF_RANGE;
		sci.nMin = 0;
		sci.nMax = nMax;
		sci.nPage = nPage;
		sci.nPos = 0;
		sci.nTrackPos = 1;
		::SetScrollInfo(wMain.GetID(), SB_VERT, &sci, TRUE);
		modified = true;
	}
	int horizStart = 0;
	int horizEnd = 2000;
	if (!::GetScrollRange(wMain.GetID(), SB_HORZ, &horizStart, &horizEnd) ||
	        horizStart != 0 || horizEnd != 2000) {
		::SetScrollRange(wMain.GetID(), SB_HORZ, 0, 2000, TRUE);
		//Platform::DebugPrintf("Horiz Scroll info changed\n");
		modified = true;
	}
	return modified;
}

void ScintillaWin::NotifyChange() {
	::SendMessage(GetParent(wMain.GetID()), WM_COMMAND,
	        MAKELONG(wMain.GetDlgCtrlID(), EN_CHANGE), 
		reinterpret_cast<LPARAM>(wMain.GetID()));
}

void ScintillaWin::NotifyFocus(bool focus) {
	::SendMessage(GetParent(wMain.GetID()), WM_COMMAND,
	        MAKELONG(wMain.GetDlgCtrlID(), focus ? EN_SETFOCUS : EN_KILLFOCUS), 
		reinterpret_cast<LPARAM>(wMain.GetID()));
}

void ScintillaWin::NotifyParent(SCNotification scn) {
	scn.nmhdr.hwndFrom = wMain.GetID();
	scn.nmhdr.idFrom = ctrlID;
	::SendMessage(GetParent(wMain.GetID()), WM_NOTIFY,
	              wMain.GetDlgCtrlID(), reinterpret_cast<LPARAM>(&scn));
}

void ScintillaWin::NotifyDoubleClick(Point pt, bool shift) {
	//Platform::DebugPrintf("ScintillaWin Double click 0\n");
	ScintillaBase::NotifyDoubleClick(pt, shift);
	// Send myself a WM_LBUTTONDBLCLK, so the container can handle it too.
	wMain.SendMessage(WM_LBUTTONDBLCLK,
	                  shift ? MK_SHIFT : 0,
	                  MAKELPARAM(pt.x, pt.y));
}

void ScintillaWin::Copy() {
	//Platform::DebugPrintf("Copy\n");
	if (currentPos != anchor) {
		HGLOBAL hmemSelection = GetSelText();
		::OpenClipboard(wMain.GetID());
		::EmptyClipboard();
		::SetClipboardData(CF_TEXT, hmemSelection);
		if (selType == selRectangle) {
			::SetClipboardData(cfColumnSelect, 0);
		}
		::CloseClipboard();
	}
}

void ScintillaWin::Paste() {
	pdoc->BeginUndoAction();
	int selStart = SelectionStart();
	ClearSelection();
	::OpenClipboard(wMain.GetID());
	bool isRectangular = ::IsClipboardFormatAvailable(cfColumnSelect);
	HGLOBAL hmemSelection = ::GetClipboardData(CF_TEXT);
	if (hmemSelection) {
		char *ptr = static_cast<char *>(
			::GlobalLock(hmemSelection));
		if (ptr) {
			unsigned int bytes = ::GlobalSize(hmemSelection);
			unsigned int len = bytes;
			for (unsigned int i = 0; i < bytes; i++) {
				if ((len == bytes) && (0 == ptr[i]))
					len = i;
			}
			if (isRectangular) {
				PasteRectangular(selStart, ptr, len);
			} else {
				pdoc->InsertString(currentPos, ptr, len);
				SetEmptySelection(currentPos + len);
			}
		}
		::GlobalUnlock(hmemSelection);
	}
	::CloseClipboard();
	pdoc->EndUndoAction();
	NotifyChange();
	Redraw();
}

void ScintillaWin::CreateCallTipWindow(PRectangle) {
#ifdef TOTAL_CONTROL
	ct.wCallTip = ::CreateWindow(callClassName, "ACallTip",
	                             WS_VISIBLE | WS_CHILD, 100, 100, 150, 20,
	                             wDraw.GetID(), reinterpret_cast<HMENU>(idCallTip), wDraw.GetInstance(), &ct);
	ct.wDraw = ct.wCallTip;
#endif
}

void ScintillaWin::AddToPopUp(const char *label, int cmd, bool enabled) {
#ifdef TOTAL_CONTROL
	if (!label[0])
		::AppendMenu(popup.GetID(), MF_SEPARATOR, 0, "");
	else if (enabled)
		::AppendMenu(popup.GetID(), MF_STRING, cmd, label);
	else
		::AppendMenu(popup.GetID(), MF_STRING | MF_DISABLED | MF_GRAYED, cmd, label);
#endif
}

void ScintillaWin::ClaimSelection() {
	// Windows does not have a primary selection
}

// Implement IUnknown

STDMETHODIMP_(ULONG)FormatEnumerator_AddRef(FormatEnumerator *fe);
STDMETHODIMP FormatEnumerator_QueryInterface(FormatEnumerator *fe, REFIID riid, PVOID *ppv) {
	//Platform::DebugPrintf("EFE QI");
	*ppv = NULL;
	if (riid == IID_IUnknown)
		*ppv = reinterpret_cast<IEnumFORMATETC *>(fe);
	if (riid == IID_IEnumFORMATETC)
		*ppv = reinterpret_cast<IEnumFORMATETC *>(fe);
	if (!*ppv)
		return E_NOINTERFACE;
	FormatEnumerator_AddRef(fe);
	return S_OK;
}
STDMETHODIMP_(ULONG)FormatEnumerator_AddRef(FormatEnumerator *fe) {
	return ++fe->ref;
}
STDMETHODIMP_(ULONG)FormatEnumerator_Release(FormatEnumerator *fe) {
	fe->ref--;
	if (fe->ref > 0)
		return fe->ref;
	delete fe;
	return 0;
}
// Implement IEnumFORMATETC
STDMETHODIMP FormatEnumerator_Next(FormatEnumerator *fe, ULONG celt, FORMATETC *rgelt, ULONG *pceltFetched) {
	//Platform::DebugPrintf("EFE Next %d %d", fe->pos, celt);
	if (rgelt == NULL) return E_POINTER;
	// We only support one format, so this is simple.
	unsigned int putPos = 0;
	while ((fe->pos < 1) && (putPos < celt)) {
		rgelt->cfFormat = CF_TEXT;
		rgelt->ptd = 0;
		rgelt->dwAspect = DVASPECT_CONTENT;
		rgelt->lindex = -1;
		rgelt->tymed = TYMED_HGLOBAL;
		fe->pos++;
		putPos++;
	}
	if (pceltFetched)
		*pceltFetched = putPos;
	return putPos ? S_OK : S_FALSE;
}
STDMETHODIMP FormatEnumerator_Skip(FormatEnumerator *fe, ULONG celt) {
	fe->pos += celt;
	return S_OK;
}
STDMETHODIMP FormatEnumerator_Reset(FormatEnumerator *fe) {
	fe->pos = 0;
	return S_OK;
}
STDMETHODIMP FormatEnumerator_Clone(FormatEnumerator *fe, IEnumFORMATETC **ppenum) {
	FormatEnumerator *pfe = new FormatEnumerator(fe->pos);
	return FormatEnumerator_QueryInterface(pfe, IID_IEnumFORMATETC,
	                                       reinterpret_cast<void **>(ppenum));
}

static void *vtFormatEnumerator[] = {
	FormatEnumerator_QueryInterface,
	FormatEnumerator_AddRef,
	FormatEnumerator_Release,
	FormatEnumerator_Next,
	FormatEnumerator_Skip,
	FormatEnumerator_Reset,
	FormatEnumerator_Clone
};

FormatEnumerator::FormatEnumerator(int pos_) {
	vtbl = vtFormatEnumerator;
	ref = 0;   // First QI adds first reference...
	pos = pos_;
}

// Implement IUnknown
STDMETHODIMP DropSource_QueryInterface(DropSource *ds, REFIID riid, PVOID *ppv) {
	return ds->sci->QueryInterface(riid, ppv);
}
STDMETHODIMP_(ULONG)DropSource_AddRef(DropSource *ds) {
	return ds->sci->AddRef();
}
STDMETHODIMP_(ULONG)DropSource_Release(DropSource *ds) {
	return ds->sci->Release();
}

// Implement IDropSource
STDMETHODIMP DropSource_QueryContinueDrag(DropSource *, BOOL fEsc, DWORD grfKeyState) {
	if (fEsc)
		return DRAGDROP_S_CANCEL;
	if (!(grfKeyState & MK_LBUTTON))
		return DRAGDROP_S_DROP;
	return S_OK;
}

STDMETHODIMP DropSource_GiveFeedback(DropSource *, DWORD) {
	return DRAGDROP_S_USEDEFAULTCURSORS;
}

static void *vtDropSource[] = {
	    DropSource_QueryInterface,
	    DropSource_AddRef,
	    DropSource_Release,
	    DropSource_QueryContinueDrag,
	    DropSource_GiveFeedback
	};

DropSource::DropSource() {
	vtbl = vtDropSource;
	sci = 0;
}

// Implement IUnkown
STDMETHODIMP DataObject_QueryInterface(DataObject *pd, REFIID riid, PVOID *ppv) {
	//Platform::DebugPrintf("DO QI %x\n", pd);
	return pd->sci->QueryInterface(riid, ppv);
}
STDMETHODIMP_(ULONG)DataObject_AddRef(DataObject *pd) {
	return pd->sci->AddRef();
}
STDMETHODIMP_(ULONG)DataObject_Release(DataObject *pd) {
	return pd->sci->Release();
}
// Implement IDataObject
STDMETHODIMP DataObject_GetData(DataObject *pd, FORMATETC *pFEIn, STGMEDIUM *pSTM) {
	return pd->sci->GetData(pFEIn, pSTM);
}

STDMETHODIMP DataObject_GetDataHere(DataObject *, FORMATETC *, STGMEDIUM *) {
	//Platform::DebugPrintf("DOB GetDataHere\n");
	return E_NOTIMPL;
}

STDMETHODIMP DataObject_QueryGetData(DataObject *pd, FORMATETC *pFE) {
	if (pd->sci->DragIsRectangularOK(pFE->cfFormat) && 
	    pFE->ptd == 0 &&
	    (pFE->dwAspect & DVASPECT_CONTENT) != 0 &&
	    pFE->lindex == -1 &&
	    (pFE->tymed & TYMED_HGLOBAL) != 0
	) {
		return S_OK;
	}
	
	if (
	    ((pFE->cfFormat != CF_TEXT) && (pFE->cfFormat != CF_HDROP)) ||
	    pFE->ptd != 0 ||
	    (pFE->dwAspect & DVASPECT_CONTENT) == 0 ||
	    pFE->lindex != -1 ||
	    (pFE->tymed & TYMED_HGLOBAL) == 0
	) {
		//Platform::DebugPrintf("DOB QueryGetData No %x\n",pFE->cfFormat);
		//return DATA_E_FORMATETC;
		return S_FALSE;
	}
	//Platform::DebugPrintf("DOB QueryGetData OK %x\n",pFE->cfFormat);
	return S_OK;
}

STDMETHODIMP DataObject_GetCanonicalFormatEtc(DataObject *, FORMATETC *, FORMATETC *pFEOut) {
	//Platform::DebugPrintf("DOB GetCanon\n");
	pFEOut->cfFormat = CF_TEXT;
	pFEOut->ptd = 0;
	pFEOut->dwAspect = DVASPECT_CONTENT;
	pFEOut->lindex = -1;
	pFEOut->tymed = TYMED_HGLOBAL;
	return S_OK;
}

STDMETHODIMP DataObject_SetData(DataObject *, FORMATETC *, STGMEDIUM *, BOOL) {
	//Platform::DebugPrintf("DOB SetData\n");
	return E_FAIL;
}

STDMETHODIMP DataObject_EnumFormatEtc(DataObject *, DWORD dwDirection, IEnumFORMATETC **ppEnum) {
	//Platform::DebugPrintf("DOB EnumFormatEtc %d\n", dwDirection);
	if (dwDirection != DATADIR_GET) {
		*ppEnum = 0;
		return E_FAIL;
	}
	FormatEnumerator *pfe = new FormatEnumerator(0);
	return FormatEnumerator_QueryInterface(pfe, IID_IEnumFORMATETC,
	                                       reinterpret_cast<void **>(ppEnum));
}

STDMETHODIMP DataObject_DAdvise(DataObject *, FORMATETC *, DWORD, IAdviseSink *, PDWORD) {
	//Platform::DebugPrintf("DOB DAdvise\n");
	return E_FAIL;
}

STDMETHODIMP DataObject_DUnadvise(DataObject *, DWORD) {
	//Platform::DebugPrintf("DOB DUnadvise\n");
	return E_FAIL;
}

STDMETHODIMP DataObject_EnumDAdvise(DataObject *, IEnumSTATDATA **) {
	//Platform::DebugPrintf("DOB EnumDAdvise\n");
	return E_FAIL;
}

static void *vtDataObject[] = {
	DataObject_QueryInterface,
	DataObject_AddRef,
	DataObject_Release,
	DataObject_GetData,
	DataObject_GetDataHere,
	DataObject_QueryGetData,
	DataObject_GetCanonicalFormatEtc,
	DataObject_SetData,
	DataObject_EnumFormatEtc,
	DataObject_DAdvise,
	DataObject_DUnadvise,
	DataObject_EnumDAdvise
};

DataObject::DataObject() {
	vtbl = vtDataObject;
	sci = 0;
}

// Implement IUnknown
STDMETHODIMP DropTarget_QueryInterface(DropTarget *dt, REFIID riid, PVOID *ppv) {
	//Platform::DebugPrintf("DT QI %x\n", dt);
	return dt->sci->QueryInterface(riid, ppv);
}
STDMETHODIMP_(ULONG)DropTarget_AddRef(DropTarget *dt) {
	return dt->sci->AddRef();
}
STDMETHODIMP_(ULONG)DropTarget_Release(DropTarget *dt) {
	return dt->sci->Release();
}

// Implement IDropTarget by forwarding to Scintilla
STDMETHODIMP DropTarget_DragEnter(DropTarget *dt, LPDATAOBJECT pIDataSource, DWORD grfKeyState,
                                  POINTL pt, PDWORD pdwEffect) {
	return dt->sci->DragEnter(pIDataSource, grfKeyState, pt, pdwEffect);
}
STDMETHODIMP DropTarget_DragOver(DropTarget *dt, DWORD grfKeyState, POINTL pt, PDWORD pdwEffect) {
	return dt->sci->DragOver(grfKeyState, pt, pdwEffect);
}
STDMETHODIMP DropTarget_DragLeave(DropTarget *dt) {
	return dt->sci->DragLeave();
}
STDMETHODIMP DropTarget_Drop(DropTarget *dt, LPDATAOBJECT pIDataSource, DWORD grfKeyState,
                             POINTL pt, PDWORD pdwEffect) {
	return dt->sci->Drop(pIDataSource, grfKeyState, pt, pdwEffect);
}

static void *vtDropTarget[] = {
	DropTarget_QueryInterface,
	DropTarget_AddRef,
	DropTarget_Release,
	DropTarget_DragEnter,
	DropTarget_DragOver,
	DropTarget_DragLeave,
	DropTarget_Drop
};

DropTarget::DropTarget() {
	vtbl = vtDropTarget;
	sci = 0;
}

// DBCS: support Input Method Editor (IME)
// Called when IME Window opened.
void ScintillaWin::ImeStartComposition() {
	if (caret.active) {
		// Move IME Window to current caret position
		HIMC hIMC = ::ImmGetContext(wMain.GetID());
		Point pos = LocationFromPosition(currentPos);
		COMPOSITIONFORM CompForm;
		CompForm.dwStyle = CFS_POINT;
		CompForm.ptCurrentPos.x = pos.x;
		CompForm.ptCurrentPos.y = pos.y;

		::ImmSetCompositionWindow(hIMC, &CompForm);

		// Set font of IME window to same as surrounded text.
		if (stylesValid) {
			// Since the style creation code has been made platform independent,
			// The logfont for the IME is recreated here.
			int styleHere = (pdoc->StyleAt(currentPos)) & 31;
			LOGFONT lf = {0};
			int sizeZoomed = vs.styles[styleHere].size + vs.zoomLevel;
			if (sizeZoomed <= 2)	// Hangs if sizeZoomed <= 1
				sizeZoomed = 2;
			Surface surface;
			int deviceHeight = (sizeZoomed * surface.LogPixelsY()) / 72;
			// The negative is to allow for leading
			lf.lfHeight = -(abs(deviceHeight));
			lf.lfWeight = vs.styles[styleHere].bold ? FW_BOLD : FW_NORMAL;
			lf.lfItalic = vs.styles[styleHere].italic ? 1 : 0;
			lf.lfCharSet = DEFAULT_CHARSET;
			strcpy(lf.lfFaceName, vs.styles[styleHere].fontName);

			::ImmSetCompositionFont(hIMC, &lf);
			::ImmReleaseContext(wMain.GetID(), hIMC);
		}
		// Caret is displayed in IME window. So, caret in Scintilla is useless.
		DropCaret();
	}
}

// Called when IME Window closed.
void ScintillaWin::ImeEndComposition() {
	ShowCaretAtCurrentPosition();
}

void ScintillaWin::GetIntelliMouseParameters() {
	// This retrieves the number of lines per scroll as configured inthe Mouse Properties sheet in Control Panel
	::SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &ucWheelScrollLines, 0);
}

HGLOBAL ScintillaWin::GetSelText() {
	int bytes = SelectionRangeLength();

	HGLOBAL hand = ::GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, 
		bytes + 1);
	if (hand) {
		char *ptr = static_cast<char *>(::GlobalLock(hand));
		char *selChars = CopySelectionRange();
		if (selChars) {
			memcpy(ptr, selChars, bytes);
			delete []selChars;
			//for (int i = 0; i < bytes; i++) {
			//	ptr[i] = pdoc->CharAt(startPos + i);
			//}
		}
		ptr[bytes] = '\0';
		::GlobalUnlock(hand);
	}
	return hand;
}

void ScintillaWin::ScrollMessage(WPARAM wParam) {
	//DWORD dwStart = timeGetTime();
	//Platform::DebugPrintf("Scroll %x %d\n", wParam, lParam);

	SCROLLINFO sci;
	memset(&sci, 0, sizeof(sci));
	sci.cbSize = sizeof(sci);
	sci.fMask = SIF_ALL;

	BOOL b = ::GetScrollInfo(wMain.GetID(), SB_VERT, &sci);

	//Platform::DebugPrintf("ScrollInfo %d mask=%x min=%d max=%d page=%d pos=%d track=%d\n", b,sci.fMask,
	//sci.nMin, sci.nMax, sci.nPage, sci.nPos, sci.nTrackPos);

	int topLineNew = topLine;
	switch (LoWord(wParam)) {
	case SB_LINEUP:
		topLineNew -= 1;
		break;
	case SB_LINEDOWN:
		topLineNew += 1;
		break;
	case SB_PAGEUP:
		topLineNew -= LinesToScroll(); break;
	case SB_PAGEDOWN: topLineNew += LinesToScroll(); break;
	case SB_TOP: topLineNew = 0; break;
	case SB_BOTTOM: topLineNew = MaxScrollPos(); break;
	case SB_THUMBPOSITION: topLineNew = sci.nTrackPos; break;
	case SB_THUMBTRACK: topLineNew = sci.nTrackPos; break;
	}
	ScrollTo(topLineNew);
}

void ScintillaWin::HorizontalScrollMessage(WPARAM wParam) {
	int xPos = xOffset;
	switch (LoWord(wParam)) {
	case SB_LINEUP:
		xPos -= 20;
		break;
	case SB_LINEDOWN:
		xPos += 20;
		break;
	case SB_PAGEUP:
		xPos -= 200;
		break;
	case SB_PAGEDOWN:
		xPos += 200;
		break;
	case SB_TOP:
		xPos = 0;
		break;
	case SB_BOTTOM:
		xPos = 2000;
		break;
	case SB_THUMBPOSITION:
		xPos = HiWord(wParam);
		break;
	case SB_THUMBTRACK:
		xPos = HiWord(wParam);
		break;
	}
	HorizontalScrollTo(xPos);
}

void ScintillaWin::RealizeWindowPalette(bool inBackGround) {
	RefreshStyleData();
	Surface surfaceWindow;
	HDC hdc = ::GetDC(wMain.GetID());
	surfaceWindow.Init(hdc);
	int changes = surfaceWindow.SetPalette(&palette, inBackGround);
	if (changes > 0)
		Redraw();
	surfaceWindow.Release();
	::ReleaseDC(wMain.GetID(), hdc);
}

// Redraw all of text area. This paint will not be abandoned.
void ScintillaWin::FullPaint() {
	paintState = painting;
	rcPaint = GetTextRectangle();
	paintingAllText = true;
	HDC hdc = ::GetDC(wMain.GetID());
	Surface surfaceWindow;
	surfaceWindow.Init(hdc);
	Paint(&surfaceWindow, rcPaint);
	surfaceWindow.Release();
	::ReleaseDC(wMain.GetID(), hdc);
	paintState = notPainting;
}

// Implement IUnknown
STDMETHODIMP ScintillaWin::QueryInterface(REFIID riid, PVOID *ppv) {
	*ppv = NULL;
	if (riid == IID_IUnknown)
		*ppv = reinterpret_cast<IDropTarget *>(&dt);
	if (riid == IID_IDropSource)
		*ppv = reinterpret_cast<IDropSource *>(&ds);
	if (riid == IID_IDropTarget)
		*ppv = reinterpret_cast<IDropTarget *>(&dt);
	if (riid == IID_IDataObject)
		*ppv = reinterpret_cast<IDataObject *>(&dob);
	if (!*ppv)
		return E_NOINTERFACE;
	return S_OK;
}

STDMETHODIMP_(ULONG) ScintillaWin::AddRef() {
	return 1;
}

STDMETHODIMP_(ULONG) ScintillaWin::Release() {
	return 1;
}

// Implement IDropTarget
STDMETHODIMP ScintillaWin::DragEnter(LPDATAOBJECT, DWORD grfKeyState,
                                     POINTL, PDWORD pdwEffect) {
	if (inDragDrop)	// Internal defaults to move
		*pdwEffect = DROPEFFECT_MOVE;
	else
		*pdwEffect = DROPEFFECT_COPY;
	if (grfKeyState & MK_ALT)
		*pdwEffect = DROPEFFECT_MOVE;
	if (grfKeyState & MK_CONTROL)
		*pdwEffect = DROPEFFECT_COPY;
	return S_OK;
}

STDMETHODIMP ScintillaWin::DragOver(DWORD grfKeyState, POINTL pt, PDWORD pdwEffect) {
	// These are the Wordpad semantics.
	if (inDragDrop)	// Internal defaults to move
		*pdwEffect = DROPEFFECT_MOVE;
	else
		*pdwEffect = DROPEFFECT_COPY;
	if (grfKeyState & MK_ALT)
		*pdwEffect = DROPEFFECT_MOVE;
	if (grfKeyState & MK_CONTROL)
		*pdwEffect = DROPEFFECT_COPY;
	// Update the cursor.
	POINT rpt = {pt.x, pt.y};
	::ScreenToClient(wMain.GetID(), &rpt);
	SetDragPosition(PositionFromLocation(Point(rpt.x, rpt.y)));

	return S_OK;
}

STDMETHODIMP ScintillaWin::DragLeave() {
	SetDragPosition(invalidPosition);
	return S_OK;
}

STDMETHODIMP ScintillaWin::Drop(LPDATAOBJECT pIDataSource, DWORD grfKeyState,
                                POINTL pt, PDWORD pdwEffect) {
	if (inDragDrop)	// Internal defaults to move
		*pdwEffect = DROPEFFECT_MOVE;
	else
		*pdwEffect = DROPEFFECT_COPY;
	if (grfKeyState & MK_ALT)
		*pdwEffect = DROPEFFECT_MOVE;
	if (grfKeyState & MK_CONTROL)
		*pdwEffect = DROPEFFECT_COPY;

	if (pIDataSource == NULL)
		return E_POINTER;

	SetDragPosition(invalidPosition);

	FORMATETC fmte = {CF_TEXT,
	                  NULL,
	                  DVASPECT_CONTENT,
	                  -1,
	                  TYMED_HGLOBAL
	                 };
	STGMEDIUM medium;

	HRESULT hres = pIDataSource->GetData(&fmte, &medium);
	if (FAILED(hres)) {
		//Platform::DebugPrintf("Bad data format: 0x%x\n", hres);
		return hres;
	}
	if (medium.hGlobal == 0) {
		return E_OUTOFMEMORY;
	}
	char *data = static_cast<char *>(::GlobalLock(medium.hGlobal));

	FORMATETC fmtr = {cfColumnSelect,
	                  NULL,
	                  DVASPECT_CONTENT,
	                  -1,
	                  TYMED_HGLOBAL
	                 };
	HRESULT hrRectangular = pIDataSource->QueryGetData(&fmtr);
	
	POINT rpt = {pt.x, pt.y};
	::ScreenToClient(wMain.GetID(), &rpt);
	Point npt(rpt.x, rpt.y);
	int movePos = PositionFromLocation(Point(rpt.x, rpt.y));

	DropAt(movePos, data, *pdwEffect == DROPEFFECT_MOVE, hrRectangular == S_OK);

	::GlobalUnlock(medium.hGlobal);

	// Free data
	if (medium.pUnkForRelease != NULL)
		medium.pUnkForRelease->Release();

	return S_OK;
}

// Implement important part of IDataObject
STDMETHODIMP ScintillaWin::GetData(FORMATETC *pFEIn, STGMEDIUM *pSTM) {
	if (
	    ((pFEIn->cfFormat != CF_TEXT) && (pFEIn->cfFormat != CF_HDROP)) ||
	    pFEIn->ptd != 0 ||
	    (pFEIn->dwAspect & DVASPECT_CONTENT) == 0 ||
	    pFEIn->lindex != -1 ||
	    (pFEIn->tymed & TYMED_HGLOBAL) == 0
	) {
		//Platform::DebugPrintf("DOB GetData No %d %x %x fmt=%x\n", lenDrag, pFEIn, pSTM, pFEIn->cfFormat);
		return DATA_E_FORMATETC;
	}
	pSTM->tymed = TYMED_HGLOBAL;
	if (pFEIn->cfFormat == CF_HDROP) {
		pSTM->hGlobal = 0;
		pSTM->pUnkForRelease = 0;
		return S_OK;
	}
	//Platform::DebugPrintf("DOB GetData OK %d %x %x\n", lenDrag, pFEIn, pSTM);

	HGLOBAL hand = ::GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, 
		lenDrag + 1);
	if (hand) {
		char *ptr = static_cast<char *>(::GlobalLock(hand));
		for (int i = 0; i < lenDrag; i++) {
			ptr[i] = dragChars[i];
		}
		ptr[lenDrag] = '\0';
		::GlobalUnlock(hand);
	}
	pSTM->hGlobal = hand;
	pSTM->pUnkForRelease = 0;
	return S_OK;
}

const char scintillaClassName[] = "Scintilla";

void ScintillaWin::Register(HINSTANCE hInstance_) {

	hInstance = hInstance_;

	InitCommonControls();

	WNDCLASS wndclass;

	// Register the Scintilla class

	wndclass.style = CS_GLOBALCLASS | CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = ::ScintillaWin::SWndProc;
	wndclass.cbClsExtra = 0;
	// Reserve extra bytes for each instance of the window;
	// we will use these bytes to store a pointer to the C++
	// (ScintillaWin) object corresponding to the window.
	wndclass.cbWndExtra = sizeof(ScintillaWin *);
	wndclass.hInstance = hInstance;
	wndclass.hIcon = NULL;
	//wndclass.hCursor = LoadCursor(NULL,IDC_IBEAM);
	wndclass.hCursor = NULL;
	wndclass.hbrBackground = NULL;
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = scintillaClassName;

	if (!RegisterClass(&wndclass)) {
		//Platform::DebugPrintf("Could not register class\n");
		// TODO: fail nicely
		return;
	}

	// Register the CallTip class

	wndclass.lpfnWndProc = ScintillaWin::CTWndProc;
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.lpszClassName = callClassName;

	if (!RegisterClass(&wndclass)) {
		//Platform::DebugPrintf("Could not register class\n");
		return;
	}
}

LRESULT PASCAL ScintillaWin::CTWndProc(
    HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam) {

	// Find C++ object associated with window.
	CallTip *ctp = reinterpret_cast<CallTip *>(GetWindowLong(hWnd, 0));
	// ctp will be zero if WM_CREATE not seen yet
	if (ctp == 0) {
		if (iMessage == WM_CREATE) {
			// Associate CallTip object with window
			CREATESTRUCT *pCreate = reinterpret_cast<CREATESTRUCT *>(lParam);
			SetWindowLong(hWnd, 0,
			              reinterpret_cast<LONG>(pCreate->lpCreateParams));
			return 0;
		} else {
			return DefWindowProc(hWnd, iMessage, wParam, lParam);
		}
	} else {
		if (iMessage == WM_DESTROY) {
			SetWindowLong(hWnd, 0, 0);
			return DefWindowProc(hWnd, iMessage, wParam, lParam);
		} else if (iMessage == WM_PAINT) {
			PAINTSTRUCT ps;
			::BeginPaint(hWnd, &ps);
			Surface surfaceWindow;
			surfaceWindow.Init(ps.hdc);
			ctp->PaintCT(&surfaceWindow);
			surfaceWindow.Release();
			::EndPaint(hWnd, &ps);
			return 0;
		} else {
			return DefWindowProc(hWnd, iMessage, wParam, lParam);
		}
	}
}

LRESULT PASCAL ScintillaWin::SWndProc(
    HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam) {
	//Platform::DebugPrintf("S W:%x M:%d WP:%x L:%x\n", hWnd, iMessage, wParam, lParam);

	// Find C++ object associated with window.
	ScintillaWin *sci = reinterpret_cast<ScintillaWin *>(GetWindowLong(hWnd, 0));
	// sci will be zero if WM_CREATE not seen yet
	if (sci == 0) {
		if (iMessage == WM_CREATE) {
			// Create C++ object associated with window
			sci = new ScintillaWin(hWnd);
			SetWindowLong(hWnd, 0, reinterpret_cast<LONG>(sci));
			return sci->WndProc(iMessage, wParam, lParam);
		} else {
			return DefWindowProc(hWnd, iMessage, wParam, lParam);
		}
	} else {
		if (iMessage == WM_DESTROY) {
			sci->Finalise();
			delete sci;
			SetWindowLong(hWnd, 0, 0);
			return DefWindowProc(hWnd, iMessage, wParam, lParam);
		} else {
			return sci->WndProc(iMessage, wParam, lParam);
		}
	}
}

// This function is externally visible so it can be called from container when building statically
void Scintilla_RegisterClasses(HINSTANCE hInstance) {
	ScintillaWin::Register(hInstance);
}

#ifndef STATIC_BUILD
extern "C" int APIENTRY DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID) {
	//Platform::DebugPrintf("Scintilla::DllMain %d %d\n", hInstance, dwReason);
	if (dwReason == DLL_PROCESS_ATTACH) {
		Scintilla_RegisterClasses(hInstance);
	}
	return TRUE;
}
#endif
