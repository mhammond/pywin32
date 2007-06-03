//  dibapi.cpp
//
// stolen from diblook MFC sample, and modified to kill off the Alloc/Lock style
// code.
//
//  Source file for Device-Independent Bitmap (DIB) API.  Provides
//  the following functions:
//
//  PaintDIB()          - Painting routine for a DIB
//  CreateDIBPalette()  - Creates a palette from a DIB
//  FindDIBBits()       - Returns a pointer to the DIB bits
//  DIBWidth()          - Gets the width of the DIB
//  DIBHeight()         - Gets the height of the DIB
//  PaletteSize()       - Gets the size required to store the DIB's palette
//  DIBNumColors()      - Calculates the number of colors
//                        in the DIB's color table
//  CopyHandle()        - Makes a copy of the given global memory block
//
// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) 1992 Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and Microsoft
// QuickHelp and/or WinHelp documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#include "stdafx.h"
#include "dibapi.h"

/*************************************************************************
 *
 * PaintDIB()
 *
 * Parameters:
 *
 * HDC hDC          - DC to do output to
 *
 * LPRECT lpDCRect  - rectangle on DC to do output to
 *
 * LPSTR lpDIBHdr       - pointer to packed-DIB memory block
 *
 * LPRECT lpDIBRect - rectangle of DIB to output into lpDCRect
 *
 * CPalette* pPal   - pointer to CPalette containing DIB's palette
 *
 * Return Value:
 *
 * BOOL             - TRUE if DIB was drawn, FALSE otherwise
 *
 * Description:
 *   Painting routine for a DIB.  Calls StretchDIBits() or
 *   SetDIBitsToDevice() to paint the DIB.  The DIB is
 *   output to the specified DC, at the coordinates given
 *   in lpDCRect.  The area of the DIB to be output is
 *   given by lpDIBRect.
 *
 ************************************************************************/

BOOL WINAPI PaintDIB(HDC     hDC,
					 LPRECT  lpDCRect,
					 LPSTR   lpDIBHdr,
					 LPRECT  lpDIBRect,
					 CPalette* pPal,
					 DWORD dwRop)
{
	LPSTR    lpDIBBits;           // Pointer to DIB bits
	BOOL     bSuccess=FALSE;      // Success/fail flag
	HPALETTE hPal=NULL;           // Our DIB's palette
	HPALETTE hOldPal=NULL;        // Previous palette

	/* Check for valid DIB  */
	if (lpDIBHdr == NULL)
		return FALSE;

	lpDIBBits = ::FindDIBBits(lpDIBHdr);

	// Get the DIB's palette, then select it into DC
	if (pPal != NULL)
	{
		hPal = (HPALETTE) pPal->m_hObject;

		// Select as background since we have
		// already realized in forground if needed
		hOldPal = ::SelectPalette(hDC, hPal, TRUE);
	    RealizePalette (hDC);
	}

	/* Make sure to use the stretching mode best for color pictures */
	::SetStretchBltMode(hDC, COLORONCOLOR);

	/* Determine whether to call StretchDIBits() or SetDIBitsToDevice() */
	if ((RECTWIDTH(lpDCRect)  == RECTWIDTH(lpDIBRect)) &&
	   (RECTHEIGHT(lpDCRect) == RECTHEIGHT(lpDIBRect)))
		bSuccess = ::SetDIBitsToDevice(hDC,                    // hDC
								   lpDCRect->left,             // DestX
								   lpDCRect->top,              // DestY
								   RECTWIDTH(lpDCRect),        // nDestWidth
								   RECTHEIGHT(lpDCRect),       // nDestHeight
								   lpDIBRect->left,            // SrcX
								   (int)DIBHeight(lpDIBHdr) -
									  lpDIBRect->top -
									  RECTHEIGHT(lpDIBRect),   // SrcY
								   0,                          // nStartScan
								   (WORD)DIBHeight(lpDIBHdr),  // nNumScans
								   lpDIBBits,                  // lpBits
								   (LPBITMAPINFO)lpDIBHdr,     // lpBitsInfo
								   DIB_RGB_COLORS);            // wUsage
   else
	  bSuccess = ::StretchDIBits(hDC,                          // hDC
								 lpDCRect->left,               // DestX
								 lpDCRect->top,                // DestY
								 RECTWIDTH(lpDCRect),          // nDestWidth
								 RECTHEIGHT(lpDCRect),         // nDestHeight
								 lpDIBRect->left,              // SrcX
								 lpDIBRect->top,               // SrcY
								 RECTWIDTH(lpDIBRect),         // wSrcWidth
								 RECTHEIGHT(lpDIBRect),        // wSrcHeight
								 lpDIBBits,                    // lpBits
								 (LPBITMAPINFO)lpDIBHdr,       // lpBitsInfo
								 DIB_RGB_COLORS,               // wUsage
								 dwRop);                       // dwROP

	/* Reselect old palette */
	if (hOldPal != NULL)
	{
		::SelectPalette(hDC, hOldPal, TRUE);
	}

   return bSuccess;
}

//---------------------------------------------------------------------
//
// Function:   PaintDDB
//
// stolen from DIBLook sample.
//
// Purpose:    Painting routine for a DDB.  Calls BitBlt() or
//             StretchBlt() to paint the DDB.  The DDB is
//             output to the specified DC, at the coordinates given
//             in lpDCRect.  The area of the DDB to be output is
//             given by lpDDBRect.  The specified palette is used.
//
//             IMPORTANT assumption:  The palette has been realized
//             elsewhere...  We won't bother figuring out whether it
//             should be realized as a foreground or background palette
//             here.
//
// Parms:      hDC       == DC to do output to.
//             lpDCRect  == Rectangle on DC to do output to.
//             hDDB      == Handle to the device dependent bitmap (DDB).
//             lpDDBRect == Rect of DDB to output into lpDCRect.
//             hPal      == Palette to be used.
//
// History:   Date      Reason
//             6/01/91  Created
//
//---------------------------------------------------------------------

BOOL WINAPI PaintDDB (HDC hDC,
					  LPRECT lpDCRect,
					  HBITMAP hDDB,
					  LPRECT lpDDBRect,
					  CPalette *pPal,
					  DWORD dwRop)
{
   HDC      hMemDC;
   HBITMAP  hOldBitmap;
   HPALETTE hOldPal1 = NULL;
   HPALETTE hOldPal2 = NULL;

   HPALETTE hPal = (HPALETTE)pPal->GetSafeHandle();
   hMemDC = CreateCompatibleDC (hDC);

   if (!hMemDC)
      return FALSE;

   if (hPal)
      {
      hOldPal1   = SelectPalette (hMemDC, hPal, FALSE);
      hOldPal2   = SelectPalette (hDC, hPal, FALSE);
      // Assume the palette's already been realized (no need to
      //  call RealizePalette().  It should have been realized in
      //  our WM_QUERYNEWPALETTE or WM_PALETTECHANGED messages...
      }

   hOldBitmap = (HBITMAP)SelectObject (hMemDC, hDDB);

   SetStretchBltMode (hDC, COLORONCOLOR);

   if ((RECTWIDTH (lpDCRect)  == RECTWIDTH (lpDDBRect)) &&
       (RECTHEIGHT (lpDCRect) == RECTHEIGHT (lpDDBRect)))
      {
      BitBlt (hDC,
              lpDCRect->left,
              lpDCRect->top,
              lpDCRect->right - lpDCRect->left,
              lpDCRect->bottom - lpDCRect->top,
              hMemDC,
              lpDDBRect->left,
              lpDDBRect->top,
              dwRop);
      }
   else
      StretchBlt (hDC,
                  lpDCRect->left,
                  lpDCRect->top,
                  lpDCRect->right - lpDCRect->left,
                  lpDCRect->bottom - lpDCRect->top,
                  hMemDC,
                  lpDDBRect->left,
                  lpDDBRect->top,
                  lpDDBRect->right - lpDDBRect->left,
                  lpDDBRect->bottom - lpDDBRect->top,
                  dwRop);

   SelectObject (hMemDC, hOldBitmap);

   if (hOldPal1)
      SelectPalette (hMemDC, hOldPal1, FALSE);

   if (hOldPal2)
      SelectPalette (hDC, hOldPal2, FALSE);

   DeleteDC (hMemDC);
   return TRUE;
}

//---------------------------------------------------------------------
//
// Function:   DIBToBitmap
//
// stolen from DIBLook sample.
//
// Purpose:    Given a handle to global memory with a DIB spec in it,
//             and a palette, returns a device dependent bitmap.  The
//             The DDB will be rendered with the specified palette.
//
// Parms:      hDIB == HANDLE to global memory containing a DIB spec
//                     (either BITMAPINFOHEADER or BITMAPCOREHEADER)
//             hPal == Palette to render the DDB with.  If it's NULL,
//                     use the default palette.
//
// History:   Date      Reason
//             6/01/91  Created
//
//---------------------------------------------------------------------

HBITMAP WINAPI DIBToBitmap (LPSTR lpDIBHdr, CPalette *pPal)
{
   LPSTR    lpDIBBits;
   HBITMAP  hBitmap;
   HDC      hDC;
   HPALETTE hOldPal = NULL;

   HPALETTE hPal = (HPALETTE)pPal->GetSafeHandle();
   lpDIBBits = FindDIBBits (lpDIBHdr);
   hDC       = GetDC (NULL);

   if (!hDC)
      return NULL;

   if (hPal)
      hOldPal = SelectPalette (hDC, hPal, FALSE);

   RealizePalette (hDC);

   hBitmap = CreateDIBitmap (hDC,
                             (LPBITMAPINFOHEADER) lpDIBHdr,
                             CBM_INIT,
                             lpDIBBits,
                             (LPBITMAPINFO) lpDIBHdr,
                             DIB_RGB_COLORS);

   if (!hBitmap)
      return NULL;

   if (hOldPal)
      SelectPalette (hDC, hOldPal, FALSE);

   ReleaseDC (NULL, hDC);

   return hBitmap;
}




/*************************************************************************
 *
 * CreateDIBPalette()
 *
 * Parameter:
 *
 * LPSTR lpbi       - pointer to packed-DIB memory block
 *
 * Return Value:
 *
 * HPALETTE         - specifies the palette
 *
 * Description:
 *
 * This function creates a palette from a DIB by allocating memory for the
 * logical palette, reading and storing the colors from the DIB's color table
 * into the logical palette, creating a palette from this logical palette,
 * and then returning the palette's handle. This allows the DIB to be
 * displayed using the best possible colors (important for DIBs with 256 or
 * more colors).
 *
 ************************************************************************/


BOOL WINAPI CreateDIBPalette(LPSTR lpbi, CPalette* pPal)
{
	LPLOGPALETTE lpPal;      // pointer to a logical palette
	int i;                   // loop index
	WORD wNumColors;         // number of colors in color table
	LPBITMAPINFO lpbmi;		 // pointer to BITMAPCOREINFO structure (win3)
	LPBITMAPCOREINFO lpbmc;  // pointer to BITMAPCOREINFO structure (old)
	BOOL bWinStyleDIB;       // flag which signifies whether this is a Win3.0 DIB
	BOOL bResult = FALSE;

	/* if DIB is invalid, return FALSE */
	if (lpbi == NULL)
	  return FALSE;
   lpbmi = (LPBITMAPINFO)lpbi;
   /* get pointer to BITMAPCOREINFO (old 1.x) */
   lpbmc = (LPBITMAPCOREINFO)lpbmi;

   /* get the number of colors in the DIB */
   wNumColors = ::DIBNumColors(lpbi);

   if (wNumColors != 0)
   {
		/* allocate memory block for logical palette */
		lpPal = (LPLOGPALETTE)new char[sizeof(LOGPALETTE)
									+ sizeof(PALETTEENTRY)
									* wNumColors];

		/* if not enough memory, clean up and return NULL */
		if (lpPal == 0)
			return FALSE;

		/* set version and number of palette entries */
		lpPal->palVersion = PALVERSION;
		lpPal->palNumEntries = (WORD)wNumColors;

		/* is this a Win 3.0 DIB? */
		bWinStyleDIB = IS_WIN30_DIB(lpbi);
		for (i = 0; i < (int)wNumColors; i++)
		{
			if (bWinStyleDIB)
			{
				lpPal->palPalEntry[i].peRed = lpbmi->bmiColors[i].rgbRed;
				lpPal->palPalEntry[i].peGreen = lpbmi->bmiColors[i].rgbGreen;
				lpPal->palPalEntry[i].peBlue = lpbmi->bmiColors[i].rgbBlue;
				lpPal->palPalEntry[i].peFlags = 0;
			}
			else
			{
				lpPal->palPalEntry[i].peRed = lpbmc->bmciColors[i].rgbtRed;
				lpPal->palPalEntry[i].peGreen = lpbmc->bmciColors[i].rgbtGreen;
				lpPal->palPalEntry[i].peBlue = lpbmc->bmciColors[i].rgbtBlue;
				lpPal->palPalEntry[i].peFlags = 0;
			}
		}

		/* create the palette and get handle to it */
		bResult = pPal->CreatePalette(lpPal);
		delete lpPal;
	}
	return bResult;
}

/*************************************************************************
 *
 * FindDIBBits()
 *
 * Parameter:
 *
 * LPSTR lpbi       - pointer to packed-DIB memory block
 *
 * Return Value:
 *
 * LPSTR            - pointer to the DIB bits
 *
 * Description:
 *
 * This function calculates the address of the DIB's bits and returns a
 * pointer to the DIB bits.
 *
 ************************************************************************/


LPSTR WINAPI FindDIBBits(LPSTR lpbi)
{
	return (lpbi + *(LPDWORD)lpbi + ::PaletteSize(lpbi));
}


/*************************************************************************
 *
 * DIBWidth()
 *
 * Parameter:
 *
 * LPSTR lpbi       - pointer to packed-DIB memory block
 *
 * Return Value:
 *
 * DWORD            - width of the DIB
 *
 * Description:
 *
 * This function gets the width of the DIB from the BITMAPINFOHEADER
 * width field if it is a Windows 3.0-style DIB or from the BITMAPCOREHEADER
 * width field if it is an other-style DIB.
 *
 ************************************************************************/


DWORD WINAPI DIBWidth(LPSTR lpDIB)
{
	LPBITMAPINFOHEADER lpbmi;  // pointer to a Win 3.0-style DIB
	LPBITMAPCOREHEADER lpbmc;  // pointer to an other-style DIB

	/* point to the header (whether Win 3.0 and old) */

	lpbmi = (LPBITMAPINFOHEADER)lpDIB;
	lpbmc = (LPBITMAPCOREHEADER)lpDIB;

	/* return the DIB width if it is a Win 3.0 DIB */
	if (IS_WIN30_DIB(lpDIB))
		return lpbmi->biWidth;
	else  /* it is an other-style DIB, so return its width */
		return (DWORD)lpbmc->bcWidth;
}


/*************************************************************************
 *
 * DIBHeight()
 *
 * Parameter:
 *
 * LPSTR lpbi       - pointer to packed-DIB memory block
 *
 * Return Value:
 *
 * DWORD            - height of the DIB
 *
 * Description:
 *
 * This function gets the height of the DIB from the BITMAPINFOHEADER
 * height field if it is a Windows 3.0-style DIB or from the BITMAPCOREHEADER
 * height field if it is an other-style DIB.
 *
 ************************************************************************/


DWORD WINAPI DIBHeight(LPSTR lpDIB)
{
	LPBITMAPINFOHEADER lpbmi;  // pointer to a Win 3.0-style DIB
	LPBITMAPCOREHEADER lpbmc;  // pointer to an other-style DIB

	/* point to the header (whether old or Win 3.0 */

	lpbmi = (LPBITMAPINFOHEADER)lpDIB;
	lpbmc = (LPBITMAPCOREHEADER)lpDIB;

	/* return the DIB height if it is a Win 3.0 DIB */
	if (IS_WIN30_DIB(lpDIB))
		return lpbmi->biHeight;
	else  /* it is an other-style DIB, so return its height */
		return (DWORD)lpbmc->bcHeight;
}


/*************************************************************************
 *
 * PaletteSize()
 *
 * Parameter:
 *
 * LPSTR lpbi       - pointer to packed-DIB memory block
 *
 * Return Value:
 *
 * WORD             - size of the color palette of the DIB
 *
 * Description:
 *
 * This function gets the size required to store the DIB's palette by
 * multiplying the number of colors by the size of an RGBQUAD (for a
 * Windows 3.0-style DIB) or by the size of an RGBTRIPLE (for an other-
 * style DIB).
 *
 ************************************************************************/


WORD WINAPI PaletteSize(LPSTR lpbi)
{
   /* calculate the size required by the palette */
   if (IS_WIN30_DIB (lpbi))
	  return (WORD)(::DIBNumColors(lpbi) * sizeof(RGBQUAD));
   else
	  return (WORD)(::DIBNumColors(lpbi) * sizeof(RGBTRIPLE));
}


/*************************************************************************
 *
 * DIBNumColors()
 *
 * Parameter:
 *
 * LPSTR lpbi       - pointer to packed-DIB memory block
 *
 * Return Value:
 *
 * WORD             - number of colors in the color table
 *
 * Description:
 *
 * This function calculates the number of colors in the DIB's color table
 * by finding the bits per pixel for the DIB (whether Win3.0 or other-style
 * DIB). If bits per pixel is 1: colors=2, if 4: colors=16, if 8: colors=256,
 * if 24, no colors in color table.
 *
 ************************************************************************/


WORD WINAPI DIBNumColors(LPSTR lpbi)
{
	WORD wBitCount;  // DIB bit count

	/*  If this is a Windows-style DIB, the number of colors in the
	 *  color table can be less than the number of bits per pixel
	 *  allows for (i.e. lpbi->biClrUsed can be set to some value).
	 *  If this is the case, return the appropriate value.
	 */

	if (IS_WIN30_DIB(lpbi))
	{
		DWORD dwClrUsed;

		dwClrUsed = ((LPBITMAPINFOHEADER)lpbi)->biClrUsed;
		if (dwClrUsed != 0)
			return (WORD)dwClrUsed;
	}

	/*  Calculate the number of colors in the color table based on
	 *  the number of bits per pixel for the DIB.
	 */
	if (IS_WIN30_DIB(lpbi))
		wBitCount = ((LPBITMAPINFOHEADER)lpbi)->biBitCount;
	else
		wBitCount = ((LPBITMAPCOREHEADER)lpbi)->bcBitCount;

	/* return number of colors based on bits per pixel */
	switch (wBitCount)
	{
		case 1:
			return 2;

		case 4:
			return 16;

		case 8:
			return 256;

		default:
			return 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//// Clipboard support

//---------------------------------------------------------------------
//
// Function:   CopyHandle (from SDK DibView sample clipbrd.c)
//
// Purpose:    Makes a copy of the given global memory block.  Returns
//             a handle to the new memory block (NULL on error).
//
//             Routine stolen verbatim out of ShowDIB.
//
// Parms:      h == Handle to global memory to duplicate.
//
// Returns:    Handle to new global memory block.
//
//---------------------------------------------------------------------

HANDLE WINAPI CopyHandle (HANDLE h)
{
	BYTE  *lpCopy;
	BYTE  *lp;
	HANDLE hCopy;
	SIZE_T dwLen;

	if (h == NULL)
		return NULL;

	dwLen = ::GlobalSize((HGLOBAL) h);

	if ((hCopy = (HANDLE) ::GlobalAlloc (GHND, dwLen)) != NULL)
	{
		lpCopy = (BYTE  *) ::GlobalLock((HGLOBAL) hCopy);
		lp     = (BYTE  *) ::GlobalLock((HGLOBAL) h);

		while (dwLen--)
			*lpCopy++ = *lp++;

		::GlobalUnlock((HGLOBAL) hCopy);
		::GlobalUnlock((HGLOBAL) h);
	}

	return hCopy;
}

