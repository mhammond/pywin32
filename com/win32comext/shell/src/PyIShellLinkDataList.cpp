// @doc - This file contains autoduck documentation
#include "shell_pch.h"
#include "PyIShellLinkDataList.h"

PyIShellLinkDataList::PyIShellLinkDataList(IUnknown *pdisp) : PyIUnknown(pdisp) { ob_type = &type; }

PyIShellLinkDataList::~PyIShellLinkDataList() {}

IShellLinkDataList *PyIShellLinkDataList::GetI(PyObject *self) { return (IShellLinkDataList *)PyIUnknown::GetI(self); }

BOOL PyObject_AsColorTable(PyObject *obColorTable, COLORREF *ColorTable)
{
    BOOL ret;
    PyObject *tpColorTable = PySequence_Tuple(obColorTable);
    if (tpColorTable == NULL)
        ret = FALSE;
    else if (PyTuple_GET_SIZE(tpColorTable) != 16) {
        PyErr_SetString(PyExc_TypeError, "ColorTable object must be a sequence of 16 ints");
        ret = FALSE;
    }
    else
        ret = PyArg_ParseTuple(tpColorTable, "kkkkkkkkkkkkkkkk", &ColorTable[0], &ColorTable[1], &ColorTable[2],
                               &ColorTable[3], &ColorTable[4], &ColorTable[5], &ColorTable[6], &ColorTable[7],
                               &ColorTable[8], &ColorTable[9], &ColorTable[10], &ColorTable[11], &ColorTable[12],
                               &ColorTable[13], &ColorTable[14], &ColorTable[15]);
    Py_DECREF(tpColorTable);
    return ret;
}

BOOL PyObject_AsCOORD(PyObject *obcoord, COORD *coord)
{
    PyObject *tpcoord = PySequence_Tuple(obcoord);
    if (tpcoord == NULL)
        return FALSE;
    BOOL bsuccess = PyArg_ParseTuple(tpcoord, "HH", &coord->X, &coord->Y);
    Py_DECREF(tpcoord);
    return bsuccess;
}

void *PyObject_AsDATABLOCK(PyObject *obdb)
{
    static char *NT_CONSOLE_PROPS_keywords[] = {"Signature",
                                                "FillAttribute",
                                                "PopupFillAttribute",
                                                "ScreenBufferSize",
                                                "WindowSize",
                                                "WindowOrigin",
                                                "Font",
                                                "InputBufferSize",
                                                "FontSize",
                                                "FontFamily",
                                                "FontWeight",
                                                "FaceName",
                                                "CursorSize",
                                                "FullScreen",
                                                "QuickEdit",
                                                "InsertMode",
                                                "AutoPosition",
                                                "HistoryBufferSize",
                                                "NumberOfHistoryBuffers",
                                                "HistoryNoDup",
                                                "ColorTable",
                                                "Size",
                                                NULL};
    // Keep Size last so it can be optional.  It's ignored on input, but should be able to accept
    //  the output from CopyDataBlock
    static char *NT_FE_CONSOLE_PROPS_keywords[] = {"Signature", "CodePage", "Size", NULL};
    static char *EXP_SPECIAL_FOLDER_keywords[] = {"Signature", "idSpecialFolder", "Offset", "Size", NULL};
    static char *EXP_DARWIN_LINK_keywords[] = {"Signature", "DarwinID", "wDarwinID", "Size", NULL};
    static char *EXP_SZ_ICON_keywords[] = {"Signature", "Target", "wTarget", "Size", NULL};  // same for EXP_SZ_LINK
    DWORD sig, bufsize;
    PyObject *obsig;
    BOOL bsuccess = TRUE;
    PyObject *dummy_args = NULL;
    void *ret = NULL;
    if (!PyDict_Check(obdb)) {
        PyErr_SetString(PyExc_TypeError, "DataBlock must be a dictionary or mapping");
        return NULL;
    }
    obsig = PyDict_GetItemString(obdb, "Signature");
    if (obsig == NULL) {
        PyErr_SetString(PyExc_ValueError, "Data Block dict must contain a Signature member");
        return NULL;
    }
    sig = PyLong_AsUnsignedLong(obsig);
    if (sig == (DWORD)-1)
        return NULL;
    dummy_args = PyTuple_New(0);
    if (dummy_args == NULL)
        return NULL;
    switch (sig) {
        case NT_CONSOLE_PROPS_SIG: {
            PyObject *obFaceName, *obColorTable;
            WCHAR *FaceName = NULL;
            bufsize = sizeof(NT_CONSOLE_PROPS);
            NT_CONSOLE_PROPS *buf = (NT_CONSOLE_PROPS *)malloc(bufsize);
            if (buf == NULL) {
                PyErr_Format(PyExc_MemoryError, "Unable to allocate %s bytes", bufsize);
                break;
            }
            ZeroMemory(buf, bufsize);
            ret = (void *)buf;
            // @object NT_CONSOLE_PROPS|Dictionary containing information for a NT_CONSOLE_PROPS struct
            bsuccess =
                PyArg_ParseTupleAndKeywords(
                    dummy_args, obdb, "kHHO&O&O&kkO&kkOkllllkklO|k", NT_CONSOLE_PROPS_keywords,
                    &buf->dbh.dwSignature,  // @prop int|Signature|The type of data block, one of shellcon.*_SIG values
                    &buf->wFillAttribute,   // @prop int|FillAttribute|Character attributes for fill operations
                    &buf->wPopupFillAttribute,  // @prop int|PopupFillAttribute|Fill attributes for popups
                    PyObject_AsCOORD, &buf->dwScreenBufferSize,  // @prop (int,int)|ScreenBufferSize|Size of console
                                                                 // screen buffer, in character cells
                    PyObject_AsCOORD,
                    &buf->dwWindowSize,  // @prop (int,int)|WindowSize|Size of console window in character cells
                    PyObject_AsCOORD,
                    &buf->dwWindowOrigin,    // @prop (int,int)|WindowOrigin|Window position, in screen coordinates
                    &buf->nFont,             // @prop int|nFont|Number of font to be displayed.  See <om
                                             // win32console.GetNumberOfConsoleFonts>
                    &buf->nInputBufferSize,  // @prop int|InputBufferSize|Size of console's input buffer
                    PyObject_AsCOORD, &buf->dwFontSize,  // @prop (int,int)|FontSize|Size of font
                    &buf->uFontFamily,                   // @prop int|FontFamily|Font family
                    &buf->uFontWeight,                   // @prop int|FontWeight|Controls thickness of displayed font
                    &obFaceName,                         // @prop str|FaceName|Name of font face, 31 characters at most
                    &buf->uCursorSize,         // @prop int|CursorSize|Relative size of cursor, expressed as percent of
                                               // character size
                    &buf->bFullScreen,         // @prop bool|FullScreen|Causes console to run in full screen mode
                    &buf->bQuickEdit,          // @prop bool|QuickEdit|
                    &buf->bInsertMode,         // @prop bool|InsertMode|
                    &buf->bAutoPosition,       // @prop bool|AutoPosition|Lets system determine window placement
                    &buf->uHistoryBufferSize,  // @prop int|HistoryBufferSize|Size of command line history buffer
                    &buf->uNumberOfHistoryBuffers,  // @prop int|NumberOfHistoryBuffers|
                    &buf->bHistoryNoDup,            // @prop bool|HistoryNoDup|
                    &obColorTable,     // @prop tuple|ColorTable|Tuple of 16 ints containing console's color attributes
                    &buf->dbh.cbSize)  // @prop int|Size|Size of structure, ignored on input
                && PyWinObject_AsWCHAR(obFaceName, &FaceName, TRUE) &&
                PyObject_AsColorTable(obColorTable, buf->ColorTable);
            if (bsuccess)
                wcsncpy(buf->FaceName, FaceName, LF_FACESIZE);
            buf->dbh.cbSize = bufsize;
            PyWinObject_FreeWCHAR(FaceName);
            break;
        }
        case NT_FE_CONSOLE_PROPS_SIG: {
            bufsize = sizeof(NT_FE_CONSOLE_PROPS);
            NT_FE_CONSOLE_PROPS *buf = (NT_FE_CONSOLE_PROPS *)malloc(bufsize);
            if (buf == NULL) {
                PyErr_Format(PyExc_MemoryError, "Unable to allocate %s bytes", bufsize);
                break;
            }
            ZeroMemory(buf, bufsize);
            ret = (void *)buf;
            // @object NT_FE_CONSOLE_PROPS|Dictionary containing information for a NT_FE_CONSOLE_PROPS struct
            bsuccess = PyArg_ParseTupleAndKeywords(
                dummy_args, obdb, "kk|k", NT_FE_CONSOLE_PROPS_keywords,
                &buf->dbh.dwSignature,  // @prop int|Signature|The type of data block, one of shellcon.*_SIG values
                &buf->uCodePage,        // @prop int|CodePage|The codepage to be used for console text
                &buf->dbh.cbSize);      // @prop int|Size|Size of structure, ignored on input
            buf->dbh.cbSize = bufsize;
            break;
        }
        case EXP_SPECIAL_FOLDER_SIG: {
            // For some reason, this struct doesn't start with DATABLOCK_HEADER
            bufsize = sizeof(EXP_SPECIAL_FOLDER);
            EXP_SPECIAL_FOLDER *buf = (EXP_SPECIAL_FOLDER *)malloc(bufsize);
            if (buf == NULL) {
                PyErr_Format(PyExc_MemoryError, "Unable to allocate %s bytes", bufsize);
                break;
            }
            ZeroMemory(buf, bufsize);
            ret = (void *)buf;
            // @object EXP_SPECIAL_FOLDER|Dictionary containing information for a EXP_SPECIAL_FOLDER struct
            bsuccess = PyArg_ParseTupleAndKeywords(
                dummy_args, obdb, "kkk|k", EXP_SPECIAL_FOLDER_keywords,
                &buf->dwSignature,      // @prop int|Signature|The type of data block, one of shellcon.*_SIG values
                &buf->idSpecialFolder,  // @prop int|idSpecialFolder|The special folder id of the target
                                        // (shellcon.CSIDL_*)
                &buf->cbOffset,         // @prop int|Offset|Offset into the link's PIDL
                &buf->cbSize);          // @prop int|Size|Size of structure, ignored on input
            buf->cbSize = bufsize;
            break;
        }
        case EXP_DARWIN_ID_SIG: {
            bufsize = sizeof(EXP_DARWIN_LINK);
            EXP_DARWIN_LINK *buf = (EXP_DARWIN_LINK *)malloc(bufsize);
            if (buf == NULL) {
                PyErr_Format(PyExc_MemoryError, "Unable to allocate %s bytes", bufsize);
                break;
            }
            ZeroMemory(buf, bufsize);
            ret = (void *)buf;
            PyObject *obwDarwinID;
            CHAR *DarwinID;
            WCHAR *wDarwinID = NULL;
            // @object EXP_DARWIN_LINK|Dictionary containing information for a EXP_DARWIN_LINK struct
            bsuccess =
                PyArg_ParseTupleAndKeywords(
                    dummy_args, obdb, "ksO|k", EXP_DARWIN_LINK_keywords,
                    &buf->dbh.dwSignature,  // @prop int|Signature|The type of data block, one of shellcon.*_SIG values
                    &DarwinID,              // @prop str|DarwinID|The Windows Installer id for the link
                    &obwDarwinID,           // @prop <o PyUNICODE>|wDarwinID|The installer id as Unicode
                    &buf->dbh.cbSize)       // @prop int|Size|Size of structure, ignored on input
                && PyWinObject_AsWCHAR(obwDarwinID, &wDarwinID, FALSE);
            if (bsuccess) {
                strncpy(buf->szDarwinID, DarwinID, MAX_PATH);
                wcsncpy(buf->szwDarwinID, wDarwinID, MAX_PATH);
            }
            buf->dbh.cbSize = bufsize;
            PyWinObject_FreeWCHAR(wDarwinID);
            break;
        }
        case EXP_SZ_ICON_SIG:
        case EXP_SZ_LINK_SIG: {
            bufsize = sizeof(EXP_SZ_LINK);
            EXP_SZ_LINK *buf = (EXP_SZ_LINK *)malloc(bufsize);
            if (buf == NULL) {
                PyErr_Format(PyExc_MemoryError, "Unable to allocate %s bytes", bufsize);
                break;
            }
            ZeroMemory(buf, bufsize);
            ret = (void *)buf;
            PyObject *obwTarget;
            CHAR *Target;
            WCHAR *wTarget = NULL;
            // @object EXP_SZ_LINK|Dictionary containing information for an EXP_SZ_LINK or EXP_SZ_ICON struct
            bsuccess =
                PyArg_ParseTupleAndKeywords(
                    dummy_args, obdb, "ksO|k", EXP_SZ_ICON_keywords,
                    &buf->dwSignature,  // @prop int|Signature|The type of data block, one of shellcon.*_SIG values
                    &Target,            // @prop str|Target|The link's target or icon location
                    &obwTarget,         // @prop <o PyUNICODE>|wTarget|The target in Unicode form
                    &buf->cbSize)       // @prop int|Size|Size of structure, ignored on input
                && PyWinObject_AsWCHAR(obwTarget, &wTarget, FALSE);
            if (bsuccess) {
                strncpy(buf->szTarget, Target, MAX_PATH);
                wcsncpy(buf->swzTarget, wTarget, MAX_PATH);
            }
            buf->cbSize = bufsize;
            PyWinObject_FreeWCHAR(wTarget);
            break;
        }
        default:
            PyErr_Format(PyExc_NotImplementedError, "Signature %s is not supported", sig);
            bsuccess = FALSE;
    }
    if (!bsuccess)
        if (ret != NULL) {
            free(ret);
            ret = NULL;
        }
    Py_XDECREF(dummy_args);
    return ret;
}

// @pymethod |PyIShellLinkDataList|AddDataBlock|Inserts a data block into the link
PyObject *PyIShellLinkDataList::AddDataBlock(PyObject *self, PyObject *args)
{
    IShellLinkDataList *pISL = GetI(self);
    if (pISL == NULL)
        return NULL;
    PyObject *obdb;
    void *buf;
    // @pyparm dict|DataBlock||Contents are dependent on type of data block being added
    // @Comm Input should be one of <o NT_CONSOLE_PROPS>, <o NT_FE_CONSOLE_PROPS>, <o EXP_SPECIAL_FOLDER>,
    // <o EXP_DARWIN_LINK>, or <o EXP_SZ_LINK>.  Expected form is indicated by the Signature member.
    if (!PyArg_ParseTuple(args, "O:AddDataBlock", &obdb))
        return NULL;
    buf = PyObject_AsDATABLOCK(obdb);
    if (buf == NULL)
        return NULL;

    HRESULT hr;
    PY_INTERFACE_PRECALL;
    hr = pISL->AddDataBlock(buf);
    PY_INTERFACE_POSTCALL;
    free(buf);
    if (FAILED(hr))
        return PyCom_BuildPyException(hr, pISL, IID_IShellLinkDataList);
    Py_INCREF(Py_None);
    return Py_None;
}

PyObject *PyObject_FromDATABLOCK(void *datablock)
{
    DWORD sig = ((DATABLOCK_HEADER *)datablock)->dwSignature;
    switch (sig) {
        case NT_CONSOLE_PROPS_SIG: {
            NT_CONSOLE_PROPS *buf = (NT_CONSOLE_PROPS *)datablock;
            return Py_BuildValue(
                "{s:k,s:k,s:H,s:H,s:(HH),s:(HH),s:(HH),s:k,s:k,s:(HH),s:k,s:k,s:N,s:k,s:O&,s:O&,s:O&,s:O&,s:k,s:k,s:O&,"
                "s:(kkkkkkkkkkkkkkkk)}",
                "Size", buf->dbh.cbSize, "Signature", buf->dbh.dwSignature, "FillAttribute", buf->wFillAttribute,
                "PopupFillAttribute", buf->wPopupFillAttribute, "ScreenBufferSize", buf->dwScreenBufferSize.X,
                buf->dwScreenBufferSize.Y, "WindowSize", buf->dwWindowSize.X, buf->dwWindowSize.Y, "WindowOrigin",
                buf->dwWindowOrigin.X, buf->dwWindowOrigin.Y, "Font", buf->nFont, "InputBufferSize",
                buf->nInputBufferSize, "FontSize", buf->dwFontSize.X, buf->dwFontSize.Y, "FontFamily", buf->uFontFamily,
                "FontWeight", buf->uFontWeight, "FaceName",
                (buf->FaceName[LF_FACESIZE - 1] == L'\0') ? PyWinObject_FromWCHAR(buf->FaceName)
                                                          : PyWinObject_FromWCHAR(buf->FaceName, LF_FACESIZE),
                "CursorSize", buf->uCursorSize, "FullScreen", PyBool_FromLong, buf->bFullScreen, "QuickEdit",
                PyBool_FromLong, buf->bQuickEdit, "InsertMode", PyBool_FromLong, buf->bInsertMode, "AutoPosition",
                PyBool_FromLong, buf->bAutoPosition, "HistoryBufferSize", buf->uHistoryBufferSize,
                "NumberOfHistoryBuffers", buf->uNumberOfHistoryBuffers, "HistoryNoDup", PyBool_FromLong,
                buf->bHistoryNoDup, "ColorTable", buf->ColorTable[0], buf->ColorTable[1], buf->ColorTable[2],
                buf->ColorTable[3], buf->ColorTable[4], buf->ColorTable[5], buf->ColorTable[6], buf->ColorTable[7],
                buf->ColorTable[8], buf->ColorTable[9], buf->ColorTable[10], buf->ColorTable[11], buf->ColorTable[12],
                buf->ColorTable[13], buf->ColorTable[14], buf->ColorTable[15]);
        }
        case NT_FE_CONSOLE_PROPS_SIG: {
            NT_FE_CONSOLE_PROPS *buf = (NT_FE_CONSOLE_PROPS *)datablock;
            return Py_BuildValue("{s:k,s:k,s:k}", "Size", buf->dbh.cbSize, "Signature", buf->dbh.dwSignature,
                                 "CodePage", buf->uCodePage);
        }
        case EXP_SPECIAL_FOLDER_SIG: {
            // For some reason, this struct doesn't start with DATABLOCK_HEADER
            EXP_SPECIAL_FOLDER *buf = (EXP_SPECIAL_FOLDER *)datablock;
            return Py_BuildValue("{s:k,s:k,s:k,s:k}", "Size", buf->cbSize, "Signature", buf->dwSignature,
                                 "idSpecialFolder", buf->idSpecialFolder, "Offset", buf->cbOffset);
        }
        case EXP_DARWIN_ID_SIG: {
            EXP_DARWIN_LINK *buf = (EXP_DARWIN_LINK *)datablock;
            return Py_BuildValue("{s:k,s:k,s:s,s:u}", "Size", buf->dbh.cbSize, "Signature", buf->dbh.dwSignature,
                                 "DarwinID", buf->szDarwinID, "wDarwinID", buf->szwDarwinID);
        }
        case EXP_SZ_ICON_SIG:
        case EXP_SZ_LINK_SIG: {
            EXP_SZ_LINK *buf = (EXP_SZ_LINK *)datablock;
            return Py_BuildValue("{s:k,s:k,s:s,s:u}", "Size", buf->cbSize, "Signature", buf->dwSignature, "Target",
                                 buf->szTarget, "wTarget", buf->swzTarget);
        }
        default:
            return PyErr_Format(PyExc_NotImplementedError, "Signature %s is not supported", sig);
    }
}

// @pymethod dict|PyIShellLinkDataList|CopyDataBlock|Retrieves the specified data block from the link
// @rdesc The returned dictionary will contain different information depending on the value passed in
PyObject *PyIShellLinkDataList::CopyDataBlock(PyObject *self, PyObject *args)
{
    IShellLinkDataList *pISL = GetI(self);
    if (pISL == NULL)
        return NULL;
    DWORD sig;
    // @pyparm int|Sig||The type of data block to retrieve, one of the shellcon.*_SIG constants
    if (!PyArg_ParseTuple(args, "k:CopyDataBlock", &sig))
        return NULL;

    VOID *buf;
    HRESULT hr;
    PY_INTERFACE_PRECALL;
    hr = pISL->CopyDataBlock(sig, &buf);
    PY_INTERFACE_POSTCALL;

    if (FAILED(hr))
        return PyCom_BuildPyException(hr, pISL, IID_IShellLinkDataList);
    PyObject *ret = PyObject_FromDATABLOCK(buf);
    LocalFree(buf);
    return ret;
}

// @pymethod int|PyIShellLinkDataList|GetFlags|Retrieves the link's flags
// @rdesc Returns combination of shellcon.SLDF_* flags
PyObject *PyIShellLinkDataList::GetFlags(PyObject *self, PyObject *args)
{
    IShellLinkDataList *pISL = GetI(self);
    if (pISL == NULL)
        return NULL;
    DWORD flags;
    HRESULT hr;
    if (!PyArg_ParseTuple(args, ":GetFlags"))
        return NULL;
    PY_INTERFACE_PRECALL;
    hr = pISL->GetFlags(&flags);
    PY_INTERFACE_POSTCALL;
    if (FAILED(hr))
        return PyCom_BuildPyException(hr, pISL, IID_IShellLinkDataList);
    return PyLong_FromUnsignedLong(flags);
}

// @pymethod |PyIShellLinkDataList|RemoveDataBlock|Deletes one of the link's data blocks
PyObject *PyIShellLinkDataList::RemoveDataBlock(PyObject *self, PyObject *args)
{
    IShellLinkDataList *pISL = GetI(self);
    if (pISL == NULL)
        return NULL;
    DWORD sig;
    // @pyparm int|Sig||Identifies which block is to be removed, one of shellcon.*_SIG constants
    if (!PyArg_ParseTuple(args, "k:RemoveDataBlock", &sig))
        return NULL;
    HRESULT hr;
    PY_INTERFACE_PRECALL;
    hr = pISL->RemoveDataBlock(sig);
    PY_INTERFACE_POSTCALL;
    if (FAILED(hr))
        return PyCom_BuildPyException(hr, pISL, IID_IShellLinkDataList);
    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod |PyIShellLinkDataList|SetFlags|Sets the flags indicating which data blocks are present
PyObject *PyIShellLinkDataList::SetFlags(PyObject *self, PyObject *args)
{
    IShellLinkDataList *pISL = GetI(self);
    if (pISL == NULL)
        return NULL;
    DWORD flags;
    // @pyparm int|Flags||Combination of shellcon.SLDF_* flags
    if (!PyArg_ParseTuple(args, "k:SetFlags", &flags))
        return NULL;
    HRESULT hr;
    PY_INTERFACE_PRECALL;
    hr = pISL->SetFlags(flags);
    PY_INTERFACE_POSTCALL;
    if (FAILED(hr))
        return PyCom_BuildPyException(hr, pISL, IID_IShellLinkDataList);
    Py_INCREF(Py_None);
    return Py_None;
}

// @object PyIShellLinkDataList|Interface to a link's extra data blocks. Can be obtained from <o PyIShellLink>
//	by calling QueryInterface with IID_IShellLinkDataList
static struct PyMethodDef PyIShellLinkDataList_methods[] = {
    // @pymeth AddDataBlock|Inserts a data block into the link
    {"AddDataBlock", PyIShellLinkDataList::AddDataBlock, METH_VARARGS, "Inserts a data block into the link"},
    // @pymeth CopyDataBlock|Retrieves the specified data block from the link
    {"CopyDataBlock", PyIShellLinkDataList::CopyDataBlock, METH_VARARGS,
     "Retrieves the specified data block from the link"},
    // @pymeth GetFlags|Retrieves the link's flags
    {"GetFlags", PyIShellLinkDataList::GetFlags, METH_VARARGS, "Retrieves the link's flags"},
    // @pymeth RemoveDataBlock|Deletes one of the link's data blocks
    {"RemoveDataBlock", PyIShellLinkDataList::RemoveDataBlock, METH_VARARGS, "Deletes one of the link's data blocks"},
    // @pymeth SetFlags|Sets the flags indicating which data blocks are present
    {"SetFlags", PyIShellLinkDataList::SetFlags, METH_VARARGS,
     "Sets the flags indicating which data blocks are present"},
    {NULL}};

PyComTypeObject PyIShellLinkDataList::type("PyIShellLinkDataList", &PyIUnknown::type, sizeof(PyIShellLinkDataList),
                                           PyIShellLinkDataList_methods, GET_PYCOM_CTOR(PyIShellLinkDataList));
