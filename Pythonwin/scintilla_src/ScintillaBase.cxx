// Scintilla source code edit control
// ScintillaBase.cxx - an enhanced subclass of Editor with calltips, autocomplete and context menu
// Copyright 1998-1999 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "Platform.h"

#include "Scintilla.h"
#include "CellBuffer.h"
#include "CallTip.h"
#include "KeyMap.h"
#include "Indicator.h"
#include "LineMarker.h"
#include "Style.h"
#include "ViewStyle.h"
#include "AutoComplete.h"
#include "Document.h"
#include "Editor.h"
#include "ScintillaBase.h"

ScintillaBase::ScintillaBase() {}


ScintillaBase::~ScintillaBase() {}


void ScintillaBase::Finalise() {
	popup.Destroy();
}

void ScintillaBase::RefreshColourPalette(Palette &pal, bool want) {
	Editor::RefreshColourPalette(pal, want);
	ct.RefreshColourPalette(pal, want);
}

void ScintillaBase::AddChar(char ch) {
	bool acActiveBeforeCharAdded = ac.Active();
	Editor::AddChar(ch);
	if (acActiveBeforeCharAdded)
		AutoCompleteChanged(ch);
}

void ScintillaBase::Command(int cmdId) {

	switch (cmdId) {

	case idAutoComplete: 	// Nothing to do

		break;

	case idCallTip: 	// Nothing to do

		break;

	case idcmdUndo:
		Undo();
		break;

	case idcmdRedo:
		Redo();
		break;

	case idcmdCut:
		Cut();
		break;

	case idcmdCopy:
		Copy();
		break;

	case idcmdPaste:
		Paste();
		break;

	case idcmdDelete:
		Clear();
		break;

	case idcmdSelectAll:
		SelectAll();
		break;
	}
}

int ScintillaBase::KeyCommand(UINT iMessage) {
	// Most key commands cancel autocompletion mode
	if (ac.Active()) {
		switch (iMessage) {
			// Except for these
		case SCI_LINEDOWN:
			AutoCompleteMove(1);
			return 0;
		case SCI_LINEUP:
			AutoCompleteMove( -1);
			return 0;
		case SCI_PAGEDOWN:
			AutoCompleteMove(5);
			return 0;
		case SCI_PAGEUP:
			AutoCompleteMove( -5);
			return 0;
		case SCI_VCHOME:
			AutoCompleteMove( -5000);
			return 0;
		case SCI_LINEEND:
			AutoCompleteMove(5000);
			return 0;
		case SCI_DELETEBACK:
			DelCharBack();
			AutoCompleteChanged();
			EnsureCaretVisible();
			return 0;
		case SCI_TAB:
			AutoCompleteCompleted();
			return 0;

		default:
			ac.Cancel();
		}
	}

	if (ct.inCallTipMode) {
		if (
		    (iMessage != SCI_CHARLEFT) &&
		    (iMessage != SCI_CHARLEFTEXTEND) &&
		    (iMessage != SCI_CHARRIGHT) &&
		    (iMessage != SCI_CHARLEFTEXTEND) &&
		    (iMessage != SCI_EDITTOGGLEOVERTYPE) &&
		    (iMessage != SCI_DELETEBACK)
		) {
			ct.CallTipCancel();
		}
		if (iMessage == SCI_DELETEBACK) {
			if (currentPos <= ct.posStartCallTip) {
				ct.CallTipCancel();
			}
		}
	}
	return Editor::KeyCommand(iMessage);
}

void ScintillaBase::AutoCompleteStart(const char *list) {
	//Platform::DebugPrintf("AutoCOmplete %s\n", list);
	ct.CallTipCancel();

	ac.Start(wDraw, idAutoComplete, currentPos);

	PRectangle rcClient = GetClientRectangle();
	Point pt = LocationFromPosition(currentPos);

	//Platform::DebugPrintf("Auto complete %x\n", lbAutoComplete);
	int heightLB = 100;
	int widthLB = 100;
	if (pt.x >= rcClient.right - widthLB) {
		HorizontalScrollTo(xOffset + pt.x - rcClient.right + widthLB);
		Redraw();
		pt = LocationFromPosition(currentPos);
	}
	PRectangle rcac;
	rcac.left = pt.x - 5;
	if (pt.y >= rcClient.bottom - heightLB && // Wont fit below.
	    pt.y >= (rcClient.bottom + rcClient.top) / 2) { // and there is more room above.
		rcac.top = pt.y - heightLB;
		if (rcac.top < 0) {
			heightLB += rcac.top;
			rcac.top = 0;
		}
	} else {
		rcac.top = pt.y + vs.lineHeight;
	}
	rcac.right = rcac.left + widthLB;
	rcac.bottom = Platform::Minimum(rcac.top + heightLB, rcClient.bottom);
	ac.lb.SetPositionRelative(rcac, wMain);
	ac.lb.SetFont(vs.styles[0].font);

	int maxStrLen = ac.SetList(list);

	// Fiddle the position of the list so it is right next to the target and wide enough for all its strings
	PRectangle rcList = ac.lb.GetPosition();
	int heightAlloced = rcList.bottom - rcList.top;
	// Make an allowance for large strings in list
	rcList.left = pt.x - 5;
	rcList.right = rcList.left + Platform::Maximum(widthLB, maxStrLen * 8 + 16);
	if (pt.y >= rcClient.bottom - heightLB && // Wont fit below.
	    pt.y >= (rcClient.bottom + rcClient.top) / 2) { // and there is more room above.
		rcList.top = pt.y - heightAlloced;
	} else {
		rcList.top = pt.y + vs.lineHeight;
	}
	rcList.bottom = rcList.top + heightAlloced;
	ac.lb.SetPositionRelative(rcList, wMain);
	//lbAutoComplete.SetPosition(rcList);
	ac.Show();
}

void ScintillaBase::AutoCompleteCancel() {
	ac.Cancel();
}

void ScintillaBase::AutoCompleteMove(int delta) {
	ac.Move(delta);
}

void ScintillaBase::AutoCompleteChanged(char ch) {
	if (currentPos <= ac.posStart) {
		ac.Cancel();
	} else if (ac.IsStopChar(ch)) {
		ac.Cancel();
	} else {
		char wordCurrent[1000];
		int i;
		for (i = ac.posStart; i < currentPos; i++)
			wordCurrent[i - ac.posStart] = doc.CharAt(i);
		wordCurrent[i - ac.posStart] = '\0';
		ac.Select(wordCurrent);
	}
}

void ScintillaBase::AutoCompleteCompleted() {
	int item = ac.lb.GetSelection();
	char selected[200];
	if (item != -1) {
		ac.lb.GetValue(item, selected, sizeof(selected));
	}
	ac.Cancel();
	if (currentPos != ac.posStart) {
		doc.DeleteChars(ac.posStart, currentPos - ac.posStart);
	}
	SetEmptySelection(ac.posStart);
	if (item != -1) {
		doc.InsertString(currentPos, selected);
		SetEmptySelection(currentPos + strlen(selected));
	}
}

void ScintillaBase::ContextMenu(Point pt) {
	popup.CreatePopUp();
	AddToPopUp("Undo", idcmdUndo, doc.CanUndo());
	AddToPopUp("Redo", idcmdRedo, doc.CanRedo());
	AddToPopUp("");
	AddToPopUp("Cut", idcmdCut, currentPos != anchor);
	AddToPopUp("Copy", idcmdCopy, currentPos != anchor);
	AddToPopUp("Paste", idcmdPaste, WndProc(EM_CANPASTE, 0, 0));
	AddToPopUp("Delete", idcmdDelete, currentPos != anchor);
	AddToPopUp("");
	AddToPopUp("Select All", idcmdSelectAll);
	popup.Show(pt, wMain);
}

void ScintillaBase::ButtonDown(Point pt, unsigned int curTime, bool shift, bool ctrl) {
	AutoCompleteCancel();
	ct.CallTipCancel();
	Editor::ButtonDown(pt, curTime, shift, ctrl);
}

LRESULT ScintillaBase::WndProc(UINT iMessage, WPARAM wParam, LPARAM lParam) {
	switch (iMessage) {
	case SCI_AUTOCSHOW:
		AutoCompleteStart(reinterpret_cast<const char *>(lParam));
		break;

	case SCI_AUTOCCANCEL:
		AutoCompleteCancel();
		break;

	case SCI_AUTOCACTIVE:
		return ac.Active();
		break;

	case SCI_AUTOCPOSSTART:
		return ac.posStart;

	case SCI_AUTOCCOMPLETE:
		AutoCompleteCompleted();
		break;

	case SCI_AUTOCSTOPS:
		ac.SetStopChars(reinterpret_cast<char *>(lParam));
		break;

	case SCI_CALLTIPSHOW: {
			AutoCompleteCancel();
			if (!ct.wCallTip.Created()) {
				PRectangle rc = ct.CallTipStart(currentPos, LocationFromPosition(wParam),
				                                reinterpret_cast<char *>(lParam),
				                                vs.styles[0].fontName, vs.styles[0].size);
				// If the call-tip window would be out of the client
				// space, adjust so it displays above the text.
				PRectangle rcClient = GetClientRectangle();
				if (rc.bottom > rcClient.bottom) {
					int offset = vs.lineHeight + rc.Height();
					rc.top -= offset;
					rc.bottom -= offset;
				}
				// Now display the window.
				CreateCallTipWindow(rc);
				ct.wCallTip.SetPositionRelative(rc, wDraw);
				ct.wCallTip.Show();
			}
		}
		break;

	case SCI_CALLTIPCANCEL:
		ct.CallTipCancel();
		break;

	case SCI_CALLTIPACTIVE:
		return ct.inCallTipMode;

	case SCI_CALLTIPPOSSTART:
		return ct.posStartCallTip;

	case SCI_CALLTIPSETHLT:
		ct.SetHighlight(wParam, lParam);
		break;

	default:
		return Editor::WndProc(iMessage, wParam, lParam);
	}
	return 0l;
}
