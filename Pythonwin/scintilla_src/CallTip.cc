// Scintilla source code edit control
// CallTip.cc - code for displaying call tips
// Copyright 1998-1999 by Neil Hodgson <neilh@hare.net.au>
// The License.txt file describes the conditions under which this software may be distributed.

#include <windows.h>

#include <stdlib.h>
#include <string.h>

#include "Scintilla.h"
#include "CallTip.h"

class CallTip {
	static HINSTANCE hInstance;
	char *val;
	HFONT font;
	int startHighlight;
	int endHighlight;
	HWND hwnd;
	CallTip();
	void Paint();
	long WndProc(WORD iMessage,WPARAM wParam,LPARAM lParam);
	static LRESULT PASCAL CWndProc(
		HWND hWnd,UINT iMessage,WPARAM wParam, LPARAM lParam);
public:
	~CallTip();
	static void Register(HINSTANCE hInstance_);
};

HINSTANCE CallTip::hInstance = 0;

void CallTip::Register(HINSTANCE hInstance_) {
	hInstance = hInstance_;

	WNDCLASS wndclass;   // Structure used to register Windows class.

	wndclass.style         = CS_GLOBALCLASS | CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc   = (WNDPROC)CallTip::CWndProc;
	wndclass.cbClsExtra    = 0;
	// Reserve extra bytes for each instance of the window;
	// we will use these bytes to store a pointer to the C++
	// (Scintilla) object corresponding to the window.
	wndclass.cbWndExtra    = sizeof(CallTip*);
	wndclass.hInstance     = hInstance;
	wndclass.hIcon         = NULL;
	wndclass.hCursor       =  LoadCursor(NULL,IDC_IBEAM);
	wndclass.hbrBackground = NULL;
	wndclass.lpszMenuName  = NULL;
	wndclass.lpszClassName = callClassName;

	if (!RegisterClass(&wndclass))
		exit(FALSE);
}

CallTip::CallTip() {
	startHighlight = 0;
	endHighlight = 0;
	val = 0;
}

CallTip::~CallTip() {
	if (val)
		free(val);
	val = 0;
}

void CallTip::Paint() {
	RECT rcClient = {0,0,0,0};
	GetClientRect(hwnd,&rcClient);

	PAINTSTRUCT ps;
	BeginPaint(hwnd,&ps);

	HFONT fontOld = (HFONT)SelectObject(ps.hdc, font);

	SIZE sizeText = {100, 100};
	if (val)
		GetTextExtentPoint32(ps.hdc, val, strlen(val), &sizeText);

	if ((sizeText.cx + 10) > (rcClient.right - rcClient.left)) {
		SetWindowPos(hwnd, 0, 0, 0, sizeText.cx + 10, rcClient.bottom - rcClient.top, SWP_NOMOVE);
	} else {
		FillRect(ps.hdc, &rcClient, (HBRUSH)GetStockObject(WHITE_BRUSH));

		DrawEdge(ps.hdc, &rcClient, EDGE_RAISED, BF_RECT);

		if (val && strlen(val)) {
			InflateRect(&rcClient, -1, -1);
			int x = 5;
			SetTextColor(ps.hdc, RGB(0x80,0x80,0x80));
			GetTextExtentPoint32(ps.hdc, val, startHighlight, &sizeText);
			ExtTextOut(ps.hdc, x, 1, 0, &rcClient, val, startHighlight, NULL);
			x += sizeText.cx;
			SetTextColor(ps.hdc, RGB(0,0,0x80));
			GetTextExtentPoint32(ps.hdc, val + startHighlight, endHighlight - startHighlight, &sizeText);
			ExtTextOut(ps.hdc, x, 1, 0, &rcClient, val + startHighlight, endHighlight - startHighlight, NULL);
			x += sizeText.cx;
			SetTextColor(ps.hdc, RGB(0x80,0x80,0x80));
			ExtTextOut(ps.hdc, x, 1, 0, &rcClient, val + endHighlight, strlen(val) - endHighlight, NULL);
		}
	}

	SelectObject(ps.hdc, fontOld);
	EndPaint(hwnd,&ps);
}

long CallTip::WndProc(WORD iMessage,WPARAM wParam,LPARAM lParam) {
	//dprintf("S start wnd proc %d %d %d\n",iMessage, wParam, lParam);
	switch (iMessage) {

	case WM_CREATE:
		break;

	case WM_PAINT:
		Paint();
		break;

	case WM_SETFONT:
		font = (HFONT)wParam;
		break;

	case WM_SETTEXT:
		free(val);
		val = strdup((char *)lParam);
		startHighlight = 0;
		endHighlight = 0;
		InvalidateRect(hwnd,(LPRECT)NULL,FALSE);
		break;

	case SCI_CALLTIPSETHLT:
		startHighlight = wParam;
		endHighlight = lParam;
		InvalidateRect(hwnd,(LPRECT)NULL,FALSE);
		break;

	default:
		return DefWindowProc(hwnd,iMessage,wParam,lParam);
	}

	//dprintf("end wnd proc\n");
	return 0l;
}

LRESULT PASCAL CallTip::CWndProc(
    HWND hWnd,UINT iMessage,WPARAM wParam, LPARAM lParam) {
	//dprintf("C W:%x M:%d WP:%x L:%x\n", hWnd, iMessage, wParam, lParam);

	// Find C++ object associated with window.
	CallTip *ct = reinterpret_cast<CallTip *>(GetWindowLong(hWnd,0));
	// ct will be zero if WM_CREATE not seen yet
	if (ct == 0) {
		if (iMessage == WM_CREATE) {
			// Create C++ object associated with window
			ct = new CallTip();
			ct->hwnd = hWnd;
			SetWindowLong(hWnd, 0, reinterpret_cast<LONG>(ct));
			return ct->WndProc(iMessage, wParam, lParam);
		} else {
			return DefWindowProc(hWnd, iMessage, wParam, lParam);
		}
	} else {
		if (iMessage == WM_DESTROY) {
			delete ct;
			SetWindowLong(hWnd, 0, 0);
			return DefWindowProc(hWnd, iMessage, wParam, lParam);
		} else {
			return ct->WndProc(iMessage, wParam, lParam);
		}
	}
}

void CallTip_Register(HINSTANCE hInstance_) {
	CallTip::Register(hInstance_);
}
