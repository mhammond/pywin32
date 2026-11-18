// dibapi.h

// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) 1992 Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and Microsoft
// QuickHelp and/or WinHelp documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#ifndef _INC_DIBAPI
#define _INC_DIBAPI

/* DIB constants */
#define PALVERSION 0x300

/* DIB Macros*/

#define IS_WIN30_DIB(lpbi) ((*(LPDWORD)(lpbi)) == sizeof(BITMAPINFOHEADER))
#define RECTWIDTH(lpRect) ((lpRect)->right - (lpRect)->left)
#define RECTHEIGHT(lpRect) ((lpRect)->bottom - (lpRect)->top)

// WIDTHBYTES performs DWORD-aligning of DIB scanlines.  The "bits"
// parameter is the bit count for the scanline (biWidth * biBitCount),
// and this macro returns the number of DWORD-aligned bytes needed
// to hold those bits.

#define WIDTHBYTES(bits) (((bits) + 31) / 32 * 4)

/* Function prototypes */
BOOL WINAPI PaintDIB(HDC, LPRECT, LPSTR, LPRECT, CPalette *, DWORD);
BOOL WINAPI PaintDDB(HDC, LPRECT, HBITMAP, LPRECT, CPalette *, DWORD);
HBITMAP WINAPI DIBToBitmap(LPSTR lpDIBHdr, CPalette *pPal);
BOOL WINAPI CreateDIBPalette(LPSTR lpbi, CPalette *cPal);
LPSTR WINAPI FindDIBBits(LPSTR lpbi);
DWORD WINAPI DIBWidth(LPSTR lpDIB);
DWORD WINAPI DIBHeight(LPSTR lpDIB);
WORD WINAPI PaletteSize(LPSTR lpbi);
WORD WINAPI DIBNumColors(LPSTR lpbi);
HANDLE WINAPI CopyHandle(HANDLE h);

#endif  //!_INC_DIBAPI
