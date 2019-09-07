/* win32ImageList : implementation file

    Image List object.

    Created Feb 1997, Mark Hammond (MHammond@skippinet.com.au)

Note that this source file contains embedded documentation.
This documentation consists of marked up text inside the
C comments, and is prefixed with an '@' symbol.  The source
files are processed by a tool called "autoduck" which
generates Windows .hlp files.
@doc

*/
#include "stdafx.h"
#include "win32win.h"
#include "win32ImageList.h"

/////////////////////////////////////////////////////////////////////////
//
// ImageList
PythonImageList::PythonImageList() {}
PythonImageList::~PythonImageList() { Python_delete_assoc(this); }
#ifdef _DEBUG
void PythonImageList::Dump(CDumpContext &dc) const
{
    CImageList::Dump(dc);
    // DumpAssocPyObject(dc, (void *)this);
}
#endif

PyCImageList::PyCImageList() {}
PyCImageList::~PyCImageList() {}

CImageList *PyCImageList::GetImageList(PyObject *self) { return (CImageList *)GetGoodCppObject(self, &type); }

// @pymethod int|win32ui|CreateImageList|Creates an image list.
PyObject *PyCImageList_Create(PyObject *self, PyObject *args)
{
    int cx, cy, nInitial, nGrow;
    COLORREF crMask;
    BOOL bMask;
    CImageList *pList = new PythonImageList();

    if (PyArg_ParseTuple(args, "iiiii",
                         &cx,        // @pyparm int|cx||Dimension of each image, in pixels.
                         &cy,        // @pyparm int|cy||Dimension of each image, in pixels.
                         &bMask,     // @pyparm int|mask||TRUE if the image contains a mask; otherwise FALSE.
                         &nInitial,  // @pyparm int|initial||Number of images that the image list initially contains.
                         &nGrow)) {  // @pyparm int|grow||Number of images by which the image list can grow when the
                                     // system needs to resize the list to make room for new images. This parameter
                                     // represents the number of new images the resized image list can contain.
        if (pList->Create(cx, cy, bMask, nInitial, nGrow))
            return ui_assoc_object::make(PyCImageList::type, pList)->GetGoodRet();
        GUI_BGN_SAVE;
        delete pList;
        GUI_END_SAVE;
        RETURN_ERR("PyCImage::Create failed");
    }

    PyErr_Clear();
    BOOL bRet;
    PyObject *obID;
    TCHAR *bitmapID = NULL;
    if (PyArg_ParseTuple(
            args, "Oiii",
            &obID,   // @pyparmalt1 <o PyResourceId>|bitmapId||Resource name or ID of the bitmap to be associated with
                     // the image list.
            &cx,     // @pyparmalt1 int|cx||Dimension of each image, in pixels.
            &nGrow,  // @pyparmalt1 int|grow||Number of images by which the image list can grow when the system needs to
                     // resize the list to make room for new images. This parameter represents the number of new images
                     // the resized image list can contain.
            &crMask)  // @pyparmalt1 int|crMask||Color used to generate a mask. Each pixel of this color in the
                      // specified bitmap is changed to black, and the corresponding bit in the mask is set to one.
        && PyWinObject_AsResourceId(obID, &bitmapID, FALSE)) {
        if (IS_INTRESOURCE(bitmapID))
            bRet = pList->Create(MAKEINTRESOURCE(bitmapID), cx, nGrow, crMask);
        else
            bRet = pList->Create(bitmapID, cx, nGrow, crMask);
        PyWinObject_FreeResourceId(bitmapID);
        if (bRet)
            return ui_assoc_object::make(PyCImageList::type, pList)->GetGoodRet();
        else
            PyErr_SetString(ui_module_error, "PyCImage::Create failed");
    }
    GUI_BGN_SAVE;
    delete pList;
    GUI_END_SAVE;
    return NULL;
}

// @pymethod |PyCImageList|DeleteImageList|Deletes an image list.
PyObject *PyCImageList_DeleteImageList(PyObject *self, PyObject *args)
{
    // @comm This frees all resources associated with an image list.
    // No further operations on the object will be allowed.
    CImageList *pList;
    if (!(pList = PyCImageList::GetImageList(self)))
        return NULL;
    CHECK_NO_ARGS2(args, "DeleteImageList");
    // Kill the C++ object.
    GUI_BGN_SAVE;
    Python_delete_assoc(pList);
    BOOL ok = pList->DeleteImageList();
    delete pList;
    GUI_END_SAVE;
    if (!ok)
        RETURN_ERR("DeleteImageList failed");
    RETURN_NONE;
}

// @pymethod int|PyCImageList|Add|Adds an image to the list.
PyObject *PyCImageList_Add(PyObject *self, PyObject *args)
{
    PyObject *obbmp1, *obbmp2;
    int mask;
    CImageList *pList = PyCImageList::GetImageList(self);
    int rc;
    if (!pList)
        return NULL;
    if (PyArg_ParseTuple(args, "(OO)", &obbmp1,
                         &obbmp2)) {  // @pyparm (int,int)|bitmap, bitmapMask||2 Bitmaps to use (primary and mask)
        HBITMAP bmp1, bmp2;
        if (!PyWinObject_AsHANDLE(obbmp1, (HANDLE *)&bmp1) || !PyWinObject_AsHANDLE(obbmp2, (HANDLE *)&bmp2))
            return NULL;
        if (!IsGdiHandleValid(bmp1) || !IsGdiHandleValid(bmp2))
            RETURN_ERR("One of the bitmap handles is invalid");
        GUI_BGN_SAVE;
        rc = pList->Add(CBitmap::FromHandle(bmp1), CBitmap::FromHandle(bmp2));
        GUI_END_SAVE;
    }
    else {
        PyErr_Clear();
        HBITMAP bmp1;
        if (PyArg_ParseTuple(args, "Oi",
                             &obbmp1,   // @pyparmalt1 int|bitmap||Bitmap to use
                             &mask)) {  // @pyparmalt1 int|color||Color to use for the mask.
            if (!PyWinObject_AsHANDLE(obbmp1, (HANDLE *)&bmp1))
                return NULL;
            if (!IsGdiHandleValid(bmp1))
                RETURN_ERR("The bitmap handle is invalid");
            GUI_BGN_SAVE;
            rc = pList->Add(CBitmap::FromHandle(bmp1), (COLORREF)mask);
            GUI_END_SAVE;
        }
        else {
            PyErr_Clear();
            PyObject *obIcon;
            if (PyArg_ParseTuple(args, "O",
                                 &obIcon)) {  // @pyparmalt2 int|hIcon||Handle of an icon to add.
                HICON hIcon;
                if (!PyWinObject_AsHANDLE(obIcon, (HANDLE *)&hIcon))
                    return NULL;
                GUI_BGN_SAVE;
                rc = pList->Add(hIcon);
                GUI_END_SAVE;
            }
            else {
                PyErr_Clear();
                RETURN_ERR("Add requires '(hbitmap, hbitmap)', 'hbitmap, color' or 'hicon'");
            }
        }
    }
    if (rc == -1)
        RETURN_ERR("Add failed");
    return Py_BuildValue("i", rc);
    // @rdesc Zero-based index of the first new image.
}
// @pymethod |PyCImageList|Destroy|Destroys the underlying CImageList
PyObject *PyCImageList_Destroy(PyObject *self, PyObject *args)
{
    CImageList *pList;
    if (!(pList = PyCImageList::GetImageList(self)))
        return NULL;
    CHECK_NO_ARGS2(args, "Destroy");
    GUI_BGN_SAVE;
    delete pList;
    GUI_END_SAVE;
    // @comm This method actually calls delete() on the CImageList - you
    // should ensure that no controls still require access to this list.
    RETURN_NONE;
}

// @pymethod int|PyCImageList|GetBkColor|Retrieves the background color of an Image List.
PyObject *PyCImageList_GetBkColor(PyObject *self, PyObject *args)
{
    CImageList *pList;
    if (!(pList = PyCImageList::GetImageList(self)))
        return NULL;
    CHECK_NO_ARGS2(args, "GetBkColor");
    GUI_BGN_SAVE;
    int rc = pList->GetBkColor();
    GUI_END_SAVE;
    return Py_BuildValue("i", rc);
}

// @pymethod |PyCImageList|SetBkColor|Sets the background color for an Image List.
PyObject *PyCImageList_SetBkColor(PyObject *self, PyObject *args)
{
    CImageList *pList;
    if (!(pList = PyCImageList::GetImageList(self)))
        return NULL;
    int col;
    // @pyparm int|color||The new background color.
    if (!PyArg_ParseTuple(args, "i:SetBkColor", &col))
        return NULL;
    GUI_BGN_SAVE;
    BOOL ok = pList->SetBkColor(col);
    GUI_END_SAVE;
    if (!ok)
        RETURN_ERR("SetBkColor failed");
    RETURN_NONE;
}

// @pymethod int|PyCImageList|GetSafeHandle|Retrieves the HIMAGELIST for the object
PyObject *PyCImageList_GetSafeHandle(PyObject *self, PyObject *args)
{
    CImageList *pList;
    if (!(pList = PyCImageList::GetImageList(self)))
        return NULL;
    CHECK_NO_ARGS2(args, "GetSafeHandle");
    return Py_BuildValue("i", pList->GetSafeHandle());
}

// @pymethod int|PyCImageList|GetImageCount|Retrieves the number of images in an image list.
PyObject *PyCImageList_GetImageCount(PyObject *self, PyObject *args)
{
    CImageList *pList;
    if (!(pList = PyCImageList::GetImageList(self)))
        return NULL;
    CHECK_NO_ARGS2(args, "GetImageCount");
    GUI_BGN_SAVE;
    int rc = pList->GetImageCount();
    GUI_END_SAVE;
    return Py_BuildValue("i", rc);
}

// @pymethod iiii(iiii)|PyCImageList|GetImageInfo|Retrieves information about an image.
PyObject *PyCImageList_GetImageInfo(PyObject *self, PyObject *args)
{
    CImageList *pList;
    int nIndex;
    if (!(pList = PyCImageList::GetImageList(self)))
        return NULL;
    // @pyparm int|index||Index of image.
    if (!PyArg_ParseTuple(args, "i:GetImageInfo", &nIndex))
        return NULL;
    IMAGEINFO info;
    GUI_BGN_SAVE;
    BOOL ok = pList->GetImageInfo(nIndex, &info);
    GUI_END_SAVE;
    if (!ok)
        RETURN_ERR("GetImageInfo failed");
    return Py_BuildValue("iiii(iiii)", info.hbmImage, info.hbmMask, info.Unused1, info.Unused2, info.rcImage.left,
                         info.rcImage.top, info.rcImage.right, info.rcImage.bottom);
    // @rdesc The return info is a tuple describing an IMAGELIST structure.
}

// @object PyCImageList|A Python type encapsulating an MFC CImageList class.
static struct PyMethodDef PyCImageList_methods[] = {
    {"Add", PyCImageList_Add, 1},          // @pymeth Add|Adds an icon to the image list.
    {"Destroy", PyCImageList_Destroy, 1},  // @pymeth Destroy|Destroys the underlying MFC imagelist object.
    {"DeleteImageList", PyCImageList_DeleteImageList, 1},  // @pymeth DeleteImageList|Deletes an image list.
    {"GetBkColor", PyCImageList_GetBkColor, 1},  // @pymeth GetBkColor|Retrieves the background color of an Image List.
    {"GetSafeHandle", PyCImageList_GetSafeHandle, 1},  // @pymeth GetSafeHandle|Retrieves the HIMAGELIST for the object
    {"GetImageCount", PyCImageList_GetImageCount,
     1},  // @pymeth GetImageCount|Retrieves the number of images in an image list.
    {"GetImageInfo", PyCImageList_GetImageInfo, 1},  // @pymeth GetImageInfo|Retrieves information about an image.
    {"SetBkColor", PyCImageList_SetBkColor, 1},      // @pymeth SetBkColor|Sets the background color for an Image List.
    {NULL, NULL}};

ui_type_CObject PyCImageList::type("PyCImageList", &ui_assoc_CObject::type, RUNTIME_CLASS(CImageList),
                                   sizeof(PyCImageList), PYOBJ_OFFSET(PyCImageList), PyCImageList_methods,
                                   GET_PY_CTOR(PyCImageList));
