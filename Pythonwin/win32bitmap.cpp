/* win32bitmap : implementation file

	Created October 1994, Mark Hammond (MHammond@skippinet.com.au)

Note that this source file contains embedded documentation.
This documentation consists of marked up text inside the
C comments, and is prefixed with an '@' symbol.  The source
files are processed by a tool called "autoduck" which
generates Windows .hlp files.
@doc

*/
#include "stdafx.h"

#include "win32gdi.h"
#include "win32bitmap.h"

#include "win32dll.h"
#include "win32dc.h"

#include "dibapi.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/*
 * Dib Header Marker - used in writing DIBs to files
 */
#define DIB_HEADER_MARKER   ((WORD) ('M' << 8) | 'B')


ui_bitmap::ui_bitmap()
{
	pPal = NULL;
	sizeBitmap = CSize(0,0);
}
ui_bitmap::~ui_bitmap()
{
	ClearSupportData();
}
void ui_bitmap::ClearSupportData()
{
	delete pPal;
	pPal = NULL;
	sizeBitmap = CSize(0,0);
}

// @pymethod <o PyCBitMap>|win32ui|CreateBitmap|Creates a bitmap object.
PyObject *ui_bitmap::create( PyObject *self, PyObject *args )
{
	return ui_assoc_object::make( ui_bitmap::type, new CBitmap);
}

// @pymethod <o PyCBitMap>|win32ui|CreateBitmapFromHandle|Creates a bitmap object from a HBITMAP.
PyObject *ui_bitmap::create_from_handle( PyObject *self, PyObject *args )
{
	PyObject *pObj, *obhandle;
	if (!PyArg_ParseTuple(args, "O:CreateBitmapFromHandle", &obhandle))
		return NULL;
	HBITMAP handle;
	if (!PyWinObject_AsHANDLE(obhandle, (HANDLE *)&handle))
		return NULL;
	CBitmap *pBitmap = new CBitmap;
//	if (!pBitmap->Attach((HGDIOBJ)handle)) {
	if (!pBitmap->Attach(handle)) {
		delete pBitmap;
		RETURN_ERR("Attach failed!");
	}
	pObj = ui_assoc_object::make( ui_bitmap::type, pBitmap);
	if (pObj) {
		BITMAP bm;
		ui_bitmap *pDIB = (ui_bitmap *) pObj;
		pBitmap->GetBitmap(&bm);
		pDIB->sizeBitmap = CSize (bm.bmWidth, bm.bmHeight);
	}
	return pObj;
}

// @pymethod |PyCBitmap|LoadBitmap|Loads a bitmap from a DLL object.
static PyObject *ui_bitmap_load_bitmap( PyObject *self, PyObject *args )
{
	int idRes;
	HINSTANCE hModLoad;
	PyObject *obDLL = NULL;
	ui_bitmap *pUIBitmap = (ui_bitmap *)self;
	if (!PyArg_ParseTuple(args,"i|O:LoadBitmap",
		&idRes, 	// @pyparm int|idRes||The resource ID of the bitmap
		&obDLL))	// @pyparm <o PyDLL>|obDLL|None|The DLL object to load from.
		return NULL;
	if (obDLL && obDLL!=Py_None) {
		// passed a DLL object.
		if (!ui_base_class::is_uiobject(obDLL, &dll_object::type))
			RETURN_TYPE_ERR("passed object must be a PyDLL");
		if (!(hModLoad = ((dll_object *)obDLL)->GetDll()))
			RETURN_ERR("Can not load from an uninitialised PyDLL object");
	} else
		hModLoad = AfxGetInstanceHandle();

  	CBitmap *pBitmap = ui_bitmap::GetBitmap( self );
	if (!pBitmap)
		return NULL;

	HMODULE save = AfxGetResourceHandle();
	AfxSetResourceHandle (hModLoad);
	BOOL bOK = pBitmap->LoadBitmap(idRes);
	AfxSetResourceHandle (save);
	if (!bOK)
		RETURN_ERR("LoadBitmap failed");
	// clear any palette and size info we have.
	pUIBitmap->ClearSupportData();
	pUIBitmap->pPal = NULL;
	BITMAP bm;
	if (pBitmap->GetObject(sizeof(bm), &bm)==0)
		RETURN_ERR("GetObject failed on bitmap");
	pUIBitmap->sizeBitmap = CSize( bm.bmWidth, bm.bmHeight );

	RETURN_NONE;
}

// @pymethod |PyCBitmap|CreateCompatibleBitmap|Creates a bitmap compatible with the specified device context.
static PyObject *ui_bitmap_create_compatible_bitmap( PyObject *self, PyObject *args )
{
	int width, height;
	PyObject *obDC;

	if (!PyArg_ParseTuple(args,"Oii:CreateCompatibleBitmap", 
		&obDC,      // @pyparm <o PyCDC>|dc||Specifies the device context.
		&width, 	// @pyparm int|width||The width (in bits) of the bitmap
		&height))	// @pyparm int|height||The height (in bits) of the bitmap.
		return NULL;
	CDC *pDC = ui_dc_object::GetDC(obDC);
	if (pDC==NULL)
		return NULL;
  	CBitmap *pBitmap = ui_bitmap::GetBitmap( self );
	if (!pBitmap)
		return NULL;
	if (!pBitmap->CreateCompatibleBitmap(pDC, width, height))
		RETURN_ERR("CreateCompatibleDC failed");
	RETURN_NONE;
}

//////////////////////////////////////////////////////////////////////
//
// Load BMP format file
//
//
// @pymethod |PyCBitmap|LoadBitmapFile|Loads a bitmap (.BMP) format
// from a file object.
PyObject *ui_bitmap_load_bitmap_file( PyObject *self, PyObject *args )
{
	ui_bitmap *pDIB = (ui_bitmap *)self;	// the python object
  	CBitmap *pBitmap = ui_bitmap::GetBitmap( self ); // the assoc window object.
	if (!pBitmap)
		return NULL;
	PyObject *fileObject;
	if (!PyArg_ParseTuple(args,"O", &fileObject)) // @pyparm file[.read]|fileObject||The file object to load the .BMP format file from.
		return NULL;
	
	PyObject *reader = PyObject_GetAttrString(fileObject, "read");
	if (reader == NULL)
		return NULL;
	PyObject *seeker = PyObject_GetAttrString(fileObject, "seek");
	if (reader == NULL)
		return NULL;

	args = Py_BuildValue("(i)", sizeof(BITMAPFILEHEADER));
	if (args == NULL) {
		DODECREF(reader);
		DODECREF(seeker);
		return NULL;
	}

	PyObject *result = gui_call_object(reader, args);
	DODECREF(args);
	if (result==NULL) {
		DODECREF(reader);
		DODECREF(seeker);
		return NULL;
	}
	if (!PyString_Check(result)) {
		DODECREF(result);
		DODECREF(seeker);
		DODECREF(reader);
		PyErr_SetString(PyExc_TypeError,
			   "object.readline() returned non-string");
		return NULL;
	}
	Py_ssize_t len = PyString_Size(result);
	if (len != sizeof(BITMAPFILEHEADER)) {
		DODECREF(seeker);
		DODECREF(reader);
		DODECREF(result);
		PyErr_SetString(PyExc_EOFError,
				   "EOF when reading DIB header");
		return NULL;
	}
	BITMAPFILEHEADER bmFileHeader;
	memcpy( &bmFileHeader, PyString_AsString(result), len);
	DODECREF(result);	// dont need this anymore
	if (bmFileHeader.bfType != DIB_HEADER_MARKER) {
		DODECREF(reader);
		PyErr_SetString(PyExc_TypeError,
				   "File is not a DIB format file");
		return NULL;
	}

	// read the bits themself.
/*	int bitsSize = bmFileHeader.bfSize - sizeof(BITMAPFILEHEADER);
	args = Py_BuildValue("(i)", bitsSize);
	if (args == NULL) {
		DODECREF(reader);
		return NULL;
	}
*/
/*	Attempt to load wierd bitmap format.
if (bmFileHeader.bfOffBits) {
		PyObject *args = Py_BuildValue("(i)", bmFileHeader.bfOffBits);
		result = gui_call_object(seeker, args);
		DODECREF(args);
		if (result==NULL) {
			DODECREF(reader);
			DODECREF(seeker);
			return NULL;
		}
		DODECREF(result);
	}
*/
	DODECREF(seeker);	// done with this.

	result = gui_call_object(reader, NULL);
	if (result==NULL) {
		DODECREF(reader);
		return NULL;
	}
	len = PyString_Size(result);
/*	if (len != bitsSize) {
		DODECREF(reader);
		DODECREF(result);
		err_setstr(EOFError,
				   "EOF when reading DIB bits");
		return NULL;
	}
*/
	char *pBits = new char[len];
	// XXX - need memory exception handler.
	memcpy( pBits, PyString_AsString(result), len);
	DODECREF(result);	// dont need this.
	DODECREF(reader); // or this.
    
	// kill old palette
	delete pDIB->pPal;
	// create the palette.
	pDIB->pPal = new CPalette;
	if (pDIB->pPal == NULL)
	{
		// we must be really low on memory
		delete pBits;
		RETURN_MEM_ERR("Allocating new palette");
	}
	if (::CreateDIBPalette(pBits, pDIB->pPal) == NULL)
	{
		// DIB may not have a palette
		delete pDIB->pPal;
		pDIB->pPal = NULL;
	}
	HBITMAP bitmap = DIBToBitmap( pBits, pDIB->pPal );
	if (!bitmap) {
		delete pBits;
		RETURN_API_ERR("CreateDIBitmap");
	}
	pBitmap->Attach(bitmap);
	pDIB->sizeBitmap = CSize( ::DIBWidth(pBits), ::DIBHeight(pBits));
	delete pBits;

	RETURN_NONE;
}

//////////////////////////////////////////////////////////////////////
//
// Load PPM format file
//
//
// @pymethod |PyCBitmap|LoadPPMFile|Loads a bitmap in Portable Pix Map (PPM) format
// from a file object.
PyObject *ui_bitmap_load_ppm_file( PyObject *self, PyObject *args )
{
	ui_bitmap *pDIB = (ui_bitmap *)self;
  	CBitmap *pBitmap = ui_bitmap::GetBitmap( self ); // the assoc window object.
	if (!pBitmap)
		return NULL;
	PyObject *fileObject;
	int rows, cols;
	const int bitsPerPixel=24;
	if (!PyArg_ParseTuple(args,"O(ii)", 
			&fileObject, // @pyparm file[.read]|fileObject||The file object to load the PPM format file from.
			&cols,       // @pyparm int|cols||The number of columns in the bitmap.
			&rows))      // @pyparm int|rows||The number of rows in the bitmap.
		return NULL;
	
	PyObject *reader = PyObject_GetAttrString(fileObject, "read");
	if (reader == NULL)
		return NULL;

	PyObject *result = gui_call_object(reader, NULL);
	if (result==NULL) {
		DODECREF(reader);
		return NULL;
	}
	Py_ssize_t lenRead = PyString_Size(result);
	// work out size of bitmap
	int headerSize = sizeof(BITMAPINFOHEADER);
	// Windows requires bitmap bits aligned to a "long", which is 32 bits!
	int imageBytesPerScan = cols * bitsPerPixel/8;
	int blocksPerScan = imageBytesPerScan / 4;
	if (imageBytesPerScan % 4)	// if not on 32 bit boundary, inc size.
		++blocksPerScan;

	int memBytesPerScan = blocksPerScan*4;

	int memSize = rows * memBytesPerScan;
	int totalSize = headerSize + memSize;
	if (lenRead!=rows*imageBytesPerScan) {
		DODECREF(reader);
		DODECREF(result);
		RETURN_ERR("loading PBM - bytes read from file is not consistant with the bitmap size given");
	}
	char *pBits = new char[totalSize];
	// XXX - need mem exception
	// copy the data in.  Windows wants scan lines bottom up.
	// and also wants RGB values reversed.
	char *pImg = PyString_AsString(result);
	char *pMem = ((char *)pBits)+headerSize+memSize-memBytesPerScan;
	BITMAPINFOHEADER *pInfo = (BITMAPINFOHEADER *)pBits;
	for (int row=0;row<rows;row++,pMem-=memBytesPerScan,pImg+=imageBytesPerScan)
		for (int col=0;col<imageBytesPerScan;col+=3) {
			pMem[col] = pImg[col+2];
			pMem[col+1] = pImg[col+1];
			pMem[col+2] = pImg[col];
		}

	DODECREF(result);	// dont need this.
	DODECREF(reader); // or this.

	// delete old palette - none for this format
	delete pDIB->pPal;
	pDIB->pPal = NULL;

	// set up the BITMAPINFOHEADER structure.
	pInfo->biSize=sizeof(BITMAPINFOHEADER);
	pInfo->biWidth=cols;
	pInfo->biHeight=rows;
	pInfo->biPlanes=1;
	pInfo->biBitCount=bitsPerPixel;
	pInfo->biCompression=BI_RGB;
	pInfo->biSizeImage=0;	// doco says may be zero for BI_RGB.
	pInfo->biXPelsPerMeter=0;
	pInfo->biYPelsPerMeter=0;
	pInfo->biClrUsed=0;	//??
	pInfo->biClrImportant=0;

	HBITMAP bitmap = DIBToBitmap( pBits, NULL );
	pBitmap->Attach(bitmap);
	pDIB->sizeBitmap = CSize(cols, rows);
	delete pBits;

	RETURN_NONE;
}


// @pymethod (cx,cy)|PyCBitmap|GetSize|Returns the size of the bitmap object.
static PyObject *ui_bitmap_get_size( PyObject *self, PyObject *args )
{
	ui_bitmap *pDIB = (ui_bitmap *)self;
	return Py_BuildValue("(ii)", pDIB->sizeBitmap.cx, pDIB->sizeBitmap.cy);
}

// @pymethod int|PyCBitmap|GetHandle|Returns the HBITMAP for a bitmap object
static PyObject *ui_bitmap_get_handle( PyObject *self, PyObject *args )
{
//	ui_bitmap *pDIB = (ui_bitmap *)self;
//	return Py_BuildValue("i", (HBITMAP)pDIB);
	CBitmap *pBitmap = ui_bitmap::GetBitmap( self );
	return PyWinLong_FromHANDLE((HBITMAP)*pBitmap);
}

// @pymethod |PyCBitmap|Paint|Paint a bitmap.
static PyObject *ui_bitmap_paint( PyObject *self, PyObject *args )
{
	ui_bitmap *pDIB = (ui_bitmap *)self;
  	CBitmap *pBitmap = ui_bitmap::GetBitmap( self ); // the assoc window object.
	if (!pBitmap)
		return NULL;

	CRect rDest	= CFrameWnd::rectDefault;
	CRect rSrc = CFrameWnd::rectDefault;
	DWORD dwROP = SRCCOPY;
	PyObject *dcobject;
	if (!PyArg_ParseTuple(args,"O|(iiii)(iiii)i:Paint", 
						  // @pyparm <o PyCDC>|dcObject||The DC object to paint the bitmap to.
						  &dcobject, 
						  // @pyparm (left,top,right,bottom)|rectDest|(0,0,0,0)|The destination rectangle to paint to.
						  &rDest.left, &rDest.top, &rDest.right, &rDest.bottom, 
						  // @pyparm (left,top,right,bottom)|rectSrc|(0,0,0,0)|The source rectangle to paint from.
						  &rSrc.left, &rSrc.top, &rSrc.right, &rSrc.bottom,
						  &dwROP))
		return NULL;
	if (rDest==CFrameWnd::rectDefault) {
		rDest.left=rDest.top = 0;
		rDest.right = pDIB->sizeBitmap.cx;
		rDest.bottom = pDIB->sizeBitmap.cy;
	}

	if (rSrc==CFrameWnd::rectDefault)
		rSrc = rDest;
	if (!ui_base_class::is_uiobject(dcobject, &ui_dc_object::type))
		RETURN_TYPE_ERR("O param must be a PyCDC object");
	CDC *pDC = ui_dc_object::GetDC(dcobject);
	if (pDC==NULL)
		return NULL;
// #define PAINT_DIB
#ifdef PAINT_DIB
	if (!::PaintDIB(pDC->m_hDC, &rDest, pDIB->pBits,
			&rSrc, pDIB->pPal, dwROP))
		RETURN_ERR("Painting of DIB failed");
#else
	HBITMAP bitmap = (HBITMAP)pBitmap->GetSafeHandle();
	if (bitmap==NULL)
		RETURN_ERR("There is no windows bitmap associated with the object");

	BOOL bRes = ::PaintDDB(pDC->m_hDC, &rDest, bitmap, &rSrc, pDIB->pPal, dwROP);
	if (!bRes)
		RETURN_ERR("Painting of DDB failed");
#endif	
	RETURN_NONE;
}

#define DICTADD(D,ST,M,TYPE) PyDict_SetItemString (D, #M, Py_BuildValue (TYPE, ST.M))

// @pymethod dict|PyCBitmap|GetInfo|Returns the BITMAP structure info
static PyObject *ui_bitmap_info( PyObject *self, PyObject *args )
{
	if (!PyArg_ParseTuple(args,":GetInfo"))
		return NULL;
  	CBitmap *pBitmap = ui_bitmap::GetBitmap( self );
	BITMAP bm;
	if (pBitmap->GetObject(sizeof(bm), &bm)==0)
		RETURN_ERR("GetObject failed on bitmap");

        PyObject *d = PyDict_New();

        // @rdesc A dictionary of integers, keyed by the following strings:<nl>
        DICTADD (d, bm, bmType, "i"); // bmType<nl>
        DICTADD (d, bm, bmWidth, "i"); // bmWidth<nl>
        DICTADD (d, bm, bmHeight, "i"); // bmHeight<nl>
        DICTADD (d, bm, bmWidthBytes, "i"); // bmWidthBytes<nl>
        DICTADD (d, bm, bmPlanes, "i"); // bmPlanes<nl>
        DICTADD (d, bm, bmBitsPixel, "i"); // bmBitsPixel<nl>
  return d;
}

// @pymethod tuple/string|PyCBitmap|GetBitmapBits|Returns the bitmap bits.
static PyObject *ui_get_bitmap_bits( PyObject *self, PyObject *args )
{
	// @pyparm int|asString|0|If False, the result is a tuple of
	// integers, if True, the result is a Python string
	int asString = 0;
	if (!PyArg_ParseTuple(args,"|i:GetBitmapBits", &asString))
		return NULL;
  	CBitmap *pBitmap = ui_bitmap::GetBitmap( self );
	BITMAP bm;
	if (pBitmap->GetObject(sizeof(bm), &bm)==0)
		RETURN_ERR("GetObject failed on bitmap");
	UINT cnt = bm.bmHeight*bm.bmWidthBytes*bm.bmPlanes;
	char *bits = (char *)malloc(cnt);
	if (!bits)
		return PyErr_NoMemory();
	HBITMAP handle = (HBITMAP)pBitmap->GetSafeHandle();
	DWORD bytes = GetBitmapBits(handle, cnt, (void *)bits);
	if (bytes != (DWORD)cnt) {
		free(bits);
		RETURN_ERR("GetBitmapBits failed on bitmap");
	}
	PyObject* rc;
	if (asString) {
		rc = PyString_FromStringAndSize(bits, cnt);
	} else {
		rc = PyTuple_New(cnt);
		for (UINT i = 0; i < cnt; i++) {
			PyTuple_SetItem(rc, i, PyInt_FromLong((long)bits[i]));
		}
	}
	free(bits);
	return rc;
}

// @pymethod None|PyCBitmap|SaveBitmapFile|Saves a bitmap to a file.
static PyObject *ui_bitmap_save_bitmap_file( PyObject *self, PyObject *args )
{
  PyObject *dcobject;
  TCHAR *pszFile;
  PyObject *obFile;
  if (!PyArg_ParseTuple(args,"OO:SaveBitmapFile",
                        &dcobject, // @pyparm <o PyCDC>|dcObject||The DC object that has rendered the bitmap.
                        &obFile))    // @pyparm string|Filename||The file to save the bitmap to
    return NULL;
  CDC *pDC = ui_dc_object::GetDC(dcobject);
  if (pDC==NULL)
    return NULL;
  HDC hDC = pDC->m_hDC;

  CBitmap *pBitmap = ui_bitmap::GetBitmap(self);
  HBITMAP hBmp = (HBITMAP)pBitmap->GetSafeHandle();

  BITMAP bmp; 
  PBITMAPINFO pbmi; 
  WORD    cClrBits; 

  // Retrieve the bitmap's color format, width, and height. 
  if (!GetObject(hBmp, sizeof(BITMAP), (LPSTR)&bmp)) 
    RETURN_ERR("GetObject failed"); 

  // Convert the color format to a count of bits. 
  cClrBits = (WORD)(bmp.bmPlanes * bmp.bmBitsPixel); 
  if (cClrBits == 1) 
    cClrBits = 1; 
  else if (cClrBits <= 4) 
    cClrBits = 4; 
  else if (cClrBits <= 8) 
    cClrBits = 8; 
  else if (cClrBits <= 16) 
    cClrBits = 16; 
  else if (cClrBits <= 24) 
    cClrBits = 24; 
  else cClrBits = 32; 

  // Allocate memory for the BITMAPINFO structure. (This structure 
  // contains a BITMAPINFOHEADER structure and an array of RGBQUAD 
  // data structures.) 

  if (cClrBits != 24) 
    pbmi = (PBITMAPINFO) LocalAlloc(LPTR, 
                                    sizeof(BITMAPINFOHEADER) + 
                                    sizeof(RGBQUAD) * (1<< cClrBits)); 

  // There is no RGBQUAD array for the 24-bit-per-pixel format. 

  else 
    pbmi = (PBITMAPINFO) LocalAlloc(LPTR, 
                                    sizeof(BITMAPINFOHEADER)); 
  
  // Initialize the fields in the BITMAPINFO structure. 

  pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER); 
  pbmi->bmiHeader.biWidth = bmp.bmWidth; 
  pbmi->bmiHeader.biHeight = bmp.bmHeight; 
  pbmi->bmiHeader.biPlanes = bmp.bmPlanes; 
  pbmi->bmiHeader.biBitCount = bmp.bmBitsPixel; 
  if (cClrBits < 24) 
    pbmi->bmiHeader.biClrUsed = (1<<cClrBits); 

  // If the bitmap is not compressed, set the BI_RGB flag. 
  pbmi->bmiHeader.biCompression = BI_RGB; 

  // Compute the number of bytes in the array of color 
  // indices and store the result in biSizeImage. 
  pbmi->bmiHeader.biSizeImage = (pbmi->bmiHeader.biWidth + 7) /8 
    * pbmi->bmiHeader.biHeight * cClrBits; 

  // Set biClrImportant to 0, indicating that all of the 
  // device colors are important. 
  pbmi->bmiHeader.biClrImportant = 0; 
  
  HANDLE hf;                 // file handle 
  BITMAPFILEHEADER hdr;       // bitmap file-header 
  PBITMAPINFOHEADER pbih;     // bitmap info-header 
  LPBYTE lpBits;              // memory pointer 
  DWORD dwTotal;              // total count of bytes 
  DWORD cb;                   // incremental count of bytes 
  BYTE *hp;                   // byte pointer 
  DWORD dwTmp; 

  pbih = (PBITMAPINFOHEADER) pbmi; 
  lpBits = (LPBYTE) GlobalAlloc(GMEM_FIXED, pbih->biSizeImage);

  if (!lpBits) 
     RETURN_ERR("GlobalAlloc failed"); 

  // Retrieve the color table (RGBQUAD array) and the bits 
  // (array of palette indices) from the DIB. 
  if (!GetDIBits(hDC, hBmp, 0, (WORD) pbih->biHeight, lpBits, pbmi, 
                 DIB_RGB_COLORS)) 
    {
      RETURN_ERR("GetDIBits failed"); 
    }

  // Create the .BMP file. 
  if (!PyWinObject_AsTCHAR(obFile, &pszFile, FALSE))
	  return NULL;
  hf = CreateFile(pszFile, 
                  GENERIC_READ | GENERIC_WRITE, 
                  (DWORD) 0, 
                  NULL, 
                  CREATE_ALWAYS, 
                  FILE_ATTRIBUTE_NORMAL, 
                  (HANDLE) NULL);
  PyWinObject_FreeTCHAR(pszFile);
  if (hf == INVALID_HANDLE_VALUE) 
    RETURN_ERR("CreateFile"); 
  hdr.bfType = 0x4d42;        // 0x42 = "B" 0x4d = "M" 
  // Compute the size of the entire file. 
  hdr.bfSize = (DWORD) (sizeof(BITMAPFILEHEADER) + 
                        pbih->biSize + pbih->biClrUsed 
                        * sizeof(RGBQUAD) + pbih->biSizeImage); 
  hdr.bfReserved1 = 0; 
  hdr.bfReserved2 = 0; 

  // Compute the offset to the array of color indices. 
  hdr.bfOffBits = (DWORD) sizeof(BITMAPFILEHEADER) + 
    pbih->biSize + pbih->biClrUsed 
    * sizeof (RGBQUAD); 

  // Copy the BITMAPFILEHEADER into the .BMP file. 
  if (!WriteFile(hf, (LPVOID) &hdr, sizeof(BITMAPFILEHEADER), 
                 (LPDWORD) &dwTmp,  NULL)) 
    {
      RETURN_ERR("WriteFile failed"); 
    }

  // Copy the BITMAPINFOHEADER and RGBQUAD array into the file. 
  if (!WriteFile(hf, (LPVOID) pbih, sizeof(BITMAPINFOHEADER) 
                 + pbih->biClrUsed * sizeof (RGBQUAD), 
                 (LPDWORD) &dwTmp, ( NULL)))
    RETURN_ERR("WriteFile failed"); 

  // Copy the array of color indices into the .BMP file. 
  dwTotal = cb = pbih->biSizeImage; 
  hp = lpBits; 
  if (!WriteFile(hf, (LPSTR) hp, (int) cb, (LPDWORD) &dwTmp,NULL)) 
    RETURN_ERR("WriteFile failed"); 

  // Close the .BMP file. 
  if (!CloseHandle(hf)) 
    RETURN_ERR("CloseHandle failed"); 

  // Free memory. 
  GlobalFree((HGLOBAL)lpBits);
  Py_INCREF(Py_None);
  return Py_None;  
}

/////////////////////////////////////////////////////////////////////
//
// ui_bitmap
//
// @object PyCBitmap|A bitmap class, derived from a <o PyCGdiObject>.
static struct PyMethodDef ui_bitmap_methods[] = {
	{"CreateCompatibleBitmap", ui_bitmap_create_compatible_bitmap, 1}, // @pymeth CreateCompatibleBitmap|Creates a bitmap compatible with the specified device context.
	{"GetSize",			ui_bitmap_get_size,        1}, // @pymeth GetSize|Gets the size of the bitmap object, in pixels.
	{"GetHandle",       ui_bitmap_get_handle,      1}, // @pymeth GetHandle|Returns the HBITMAP for a bitmap.
	{"LoadBitmap",		ui_bitmap_load_bitmap,     1}, // @pymeth LoadBitmap|Loads a bitmap from a DLL object.
	{"LoadBitmapFile",	ui_bitmap_load_bitmap_file,1}, // @pymeth LoadBitmapFile|Loads a bitmap from a file object.
	{"LoadPPMFile",		ui_bitmap_load_ppm_file,   1}, // @pymeth LoadPPMFile|Loads a bitmap from a file object containing a PPM format bitmap.
	{"Paint",			ui_bitmap_paint,           1}, // @pymeth Paint|Paints a bitmap to a windows DC.
	{"GetInfo",			ui_bitmap_info,           1}, // @pymeth GetInfo|Returns the BITMAP structure info.
	{"GetBitmapBits",			ui_get_bitmap_bits,           1}, // @pymeth GetBitmapBits|Returns the bitmap bits.
	{"SaveBitmapFile",	ui_bitmap_save_bitmap_file, 1}, // @pymeth SaveBitmapFile|Saves a bitmap to a file.
	{NULL,			NULL}		/* sentinel */
};

ui_type_CObject ui_bitmap::type("PyCBitmap", 
								&PyCGdiObject::type, 
								RUNTIME_CLASS(CBitmap),
								sizeof(ui_bitmap), 
								PYOBJ_OFFSET(ui_bitmap), 
								ui_bitmap_methods, 
								GET_PY_CTOR(ui_bitmap));
