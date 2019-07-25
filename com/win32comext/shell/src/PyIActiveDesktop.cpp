// @doc - This file contains autoduck documentation
#include "shell_pch.h"
#include "PyIActiveDesktop.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////
// helper functions for converting various structs to/from dicts
///////////////////////////////////////////////////////////////////////////////////////////////////////
PyObject *PyWinObject_FromCOMPPOS(COMPPOS *comppos)
{
    return Py_BuildValue("{s:k,s:i,s:i,s:k,s:k,s:i,s:O&,s:O&,s:O&,s:i,s:i}", "Size", comppos->dwSize, "Left",
                         comppos->iLeft, "Top", comppos->iTop, "Width", comppos->dwWidth, "Height", comppos->dwHeight,
                         "zIndex", comppos->izIndex, "CanResize", PyBool_FromLong, comppos->fCanResize, "CanResizeX",
                         PyBool_FromLong, comppos->fCanResizeX, "CanResizeY", PyBool_FromLong, comppos->fCanResizeY,
                         "PreferredLeftPercent", comppos->iPreferredLeftPercent, "PreferredTopPercent",
                         comppos->iPreferredTopPercent);
}

BOOL PyWinObject_AsCOMPPOS(PyObject *ob, COMPPOS *comppos)
{
    static char *COMPPOS_keywords[] = {"Left",
                                       "Top",
                                       "Width",
                                       "Height",
                                       "zIndex",
                                       "CanResize",
                                       "CanResizeX",
                                       "CanResizeY",
                                       "PreferredLeftPercent",
                                       "PreferredTopPercent",
                                       "Size",
                                       NULL};
    ZeroMemory(comppos, sizeof(COMPPOS));
    if (!PyDict_Check(ob)) {
        PyErr_SetString(PyExc_TypeError, "COMPPOS must be a dictionary");
        return FALSE;
    }
    PyObject *dummy_args = PyTuple_New(0);
    if (dummy_args == NULL)
        return FALSE;
    // @object COMPPOS|A dictionary containing data to fill a COMPPOS struct
    BOOL bsuccess =
        PyArg_ParseTupleAndKeywords(dummy_args, ob, "iikkiiiiii|k:COMPPOS", COMPPOS_keywords,
                                    &comppos->iLeft,                  // @prop int|Left|
                                    &comppos->iTop,                   // @prop int|Top|
                                    &comppos->dwWidth,                // @prop int|Width|
                                    &comppos->dwHeight,               // @prop int|Height|
                                    &comppos->izIndex,                // @prop int|Index|
                                    &comppos->fCanResize,             // @prop int|CanResize|
                                    &comppos->fCanResizeX,            // @prop int|CanResizeX|
                                    &comppos->fCanResizeY,            // @prop int|CanResizeY|
                                    &comppos->iPreferredLeftPercent,  // @prop int|PreferredLeftPercent|
                                    &comppos->iPreferredTopPercent,   // @prop int|PreferredTopPercent|
                                    &comppos->dwSize);  // @prop int|Size|Size of structure, ignored on input
    comppos->dwSize = sizeof(COMPPOS);
    Py_DECREF(dummy_args);
    return bsuccess;
}

PyObject *PyWinObject_FromCOMPSTATEINFO(COMPSTATEINFO *compstateinfo)
{
    return Py_BuildValue("{s:k,s:i,s:i,s:k,s:k,s:k}", "Size", compstateinfo->dwSize, "Left", compstateinfo->iLeft,
                         "Top", compstateinfo->iTop, "Width", compstateinfo->dwWidth, "Height", compstateinfo->dwHeight,
                         "ItemState", compstateinfo->dwItemState);
}

BOOL PyWinObject_AsCOMPSTATEINFO(PyObject *ob, COMPSTATEINFO *compstateinfo)
{
    static char *COMPSTATEINFO_keywords[] = {"Left", "Top", "Width", "Height", "ItemState", "Size", NULL};
    ZeroMemory(compstateinfo, sizeof(COMPSTATEINFO));
    if (!PyDict_Check(ob)) {
        PyErr_SetString(PyExc_TypeError, "COMPSTATEINFO must be a dictionary");
        return FALSE;
    }
    PyObject *dummy_args = PyTuple_New(0);
    if (dummy_args == NULL)
        return FALSE;
    // @object COMPSTATEINFO|A dictionary containing data to fill a COMPSTATEINFO struct
    BOOL bsuccess = PyArg_ParseTupleAndKeywords(
        dummy_args, ob, "iikkk|k:COMPSTATEINFO", COMPSTATEINFO_keywords,
        &compstateinfo->iLeft,        // @prop int|Left|Specified as screen coordinates
        &compstateinfo->iTop,         // @prop int|Top|Specified as screen coordinates
        &compstateinfo->dwWidth,      // @prop int|Width|Measured in pixels
        &compstateinfo->dwHeight,     // @prop int|Height|Measured in pixels
        &compstateinfo->dwItemState,  // @prop int|dwItemState|One of IS_NORMAL, IS_FULLSCREEN  IS_SPLIT
        &compstateinfo->dwSize);      // @prop int|Size|Size of structure, ignored on input
    compstateinfo->dwSize = sizeof(COMPSTATEINFO);
    Py_DECREF(dummy_args);
    return bsuccess;
}

PyObject *PyWinObject_FromCOMPONENT(COMPONENT *component)
{
    return Py_BuildValue("{s:k,s:k,s:i,s:O&,s:O&,s:O&,s:O&,s:u,s:u,s:u,s:k,s:O&,s:O&}", "Size", component->dwSize, "ID",
                         component->dwID, "ComponentType", component->iComponentType, "Checked", PyBool_FromLong,
                         component->fChecked, "Dirty", PyBool_FromLong, component->fDirty, "NoScroll", PyBool_FromLong,
                         component->fNoScroll, "Pos", PyWinObject_FromCOMPPOS, &component->cpPos, "FriendlyName",
                         component->wszFriendlyName, "Source", component->wszSource, "SubscribedURL",
                         component->wszSubscribedURL, "CurItemState", component->dwCurItemState, "Original",
                         PyWinObject_FromCOMPSTATEINFO, &component->csiOriginal, "Restored",
                         PyWinObject_FromCOMPSTATEINFO, &component->csiRestored);
}

BOOL PyWinObject_AsCOMPONENT(PyObject *ob, COMPONENT *component)
{
    static char *COMPONENT_keywords[] = {
        "ID",     "ComponentType", "Checked",      "Dirty",    "NoScroll", "Pos",  "FriendlyName",
        "Source", "SubscribedURL", "CurItemState", "Original", "Restored", "Size", NULL};
    ZeroMemory(component, sizeof(COMPONENT));
    if (!PyDict_Check(ob)) {
        PyErr_SetString(PyExc_TypeError, "COMPONENT must be a dictionary");
        return FALSE;
    }
    PyObject *dummy_args = PyTuple_New(0);
    if (dummy_args == NULL)
        return FALSE;
    // @object COMPONENT|A dictionary containing data to fill a COMPPOS struct
    PyObject *obFriendlyName, *obSource, *obSubscribedURL;
    WCHAR *FriendlyName = NULL, *Source = NULL, *SubscribedURL = NULL;
    bool bsuccess =
        PyArg_ParseTupleAndKeywords(
            dummy_args, ob, "kiiiiO&OOOkO&O&|k:COMPONENT", COMPONENT_keywords,
            &component->dwID,            // @prop int|ID|Id of component, ignored when adding a new component
            &component->iComponentType,  // @prop int|ComponentType|One of shellcon.COMP_TYPE_* values
            &component->fChecked,        // @prop bool|Checked|True indicates item is currently displayed
            &component->fDirty,          // @prop bool|fDirty|Indicates if unsaved changes exist
            &component->fNoScroll,       // @prop bool|NoScroll|True disables scrolling
            PyWinObject_AsCOMPPOS,
            &component->cpPos,  // @prop dict|Pos|<o COMPPOS> dictionary determining window size and placement
            &obFriendlyName,   // @prop <o PyUNICODE>|FriendlyName|String of at most MAX_PATH-1 characters, truncated if
                               // longer
            &obSource,         // @prop <o PyUNICODE>|Source|String of at most INTERNET_MAX_URL_LENGTH-1 characters
            &obSubscribedURL,  // @prop <o PyUNICODE>|SubscribedURL|String of at most INTERNET_MAX_URL_LENGTH-1
                               // characters
            &component->dwCurItemState,                            // @prop int|CurItemState|One of shellcon.IS_* flags
            PyWinObject_AsCOMPSTATEINFO, &component->csiOriginal,  // @prop dict|Original|<o COMPSTATEINFO> dictionary
            PyWinObject_AsCOMPSTATEINFO, &component->csiRestored,  // @prop dict|Restored|<o COMPSTATEINFO> dictionary
            &component->dwSize)                                    // @prop int|Size|Size of structure, ignored on input
        && PyWinObject_AsWCHAR(obFriendlyName, &FriendlyName, FALSE) && PyWinObject_AsWCHAR(obSource, &Source, FALSE) &&
        PyWinObject_AsWCHAR(obSubscribedURL, &SubscribedURL, FALSE);
    if (bsuccess) {
        wcsncpy(component->wszFriendlyName, FriendlyName, MAX_PATH - 1);
        wcsncpy(component->wszSource, Source, INTERNET_MAX_URL_LENGTH - 1);
        wcsncpy(component->wszSubscribedURL, SubscribedURL, INTERNET_MAX_URL_LENGTH - 1);
    }
    component->dwSize = sizeof(COMPONENT);
    PyWinObject_FreeWCHAR(FriendlyName);
    PyWinObject_FreeWCHAR(Source);
    PyWinObject_FreeWCHAR(SubscribedURL);
    Py_DECREF(dummy_args);
    return bsuccess;
}

PyObject *PyWinObject_FromCOMPONENTSOPT(COMPONENTSOPT *componentsopt)
{
    return Py_BuildValue("{s:k,s:O&,s:O&}", "Size", componentsopt->dwSize, "EnableComponents", PyBool_FromLong,
                         componentsopt->fEnableComponents, "ActiveDesktop", PyBool_FromLong,
                         componentsopt->fActiveDesktop);
}

BOOL PyWinObject_AsCOMPONENTSOPT(PyObject *ob, COMPONENTSOPT *componentsopt)
{
    static char *COMPONENTSOPT_keywords[] = {"EnableComponents", "ActiveDesktop", "Size", NULL};
    ZeroMemory(componentsopt, sizeof(COMPONENTSOPT));
    if (!PyDict_Check(ob)) {
        PyErr_SetString(PyExc_TypeError, "COMPONENTSOPT must be a dictionary");
        return FALSE;
    }
    PyObject *dummy_args = PyTuple_New(0);
    if (dummy_args == NULL)
        return FALSE;
    // @object COMPONENTSOPT|A dictionary containing data to fill a COMPONENTSOPT struct
    BOOL bsuccess = PyArg_ParseTupleAndKeywords(
        dummy_args, ob, "kk|k:COMPONENTSOPT", COMPONENTSOPT_keywords,
        &componentsopt->fEnableComponents,  // @prop bool|EnableComponents|True if components are enabled
        &componentsopt->fActiveDesktop,     // @prop bool|ActiveDesktop|True if Active Desktop is enabled
        &componentsopt->dwSize);            // @prop int|Size|Size of structure, ignored on input
    componentsopt->dwSize = sizeof(COMPONENTSOPT);
    Py_DECREF(dummy_args);
    return bsuccess;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
// IActiveDesktop
///////////////////////////////////////////////////////////////////////////////////////////////////////
PyIActiveDesktop::PyIActiveDesktop(IUnknown *pdisp) : PyIUnknown(pdisp) { ob_type = &type; }

PyIActiveDesktop::~PyIActiveDesktop() {}

IActiveDesktop *PyIActiveDesktop::GetI(PyObject *self) { return (IActiveDesktop *)PyIUnknown::GetI(self); }

// @pymethod |PyIActiveDesktop|ApplyChanges|Applies changes to ActiveDesktop settings and persists them to the registry.
PyObject *PyIActiveDesktop::ApplyChanges(PyObject *self, PyObject *args)
{
    IActiveDesktop *pIAD = GetI(self);
    if (pIAD == NULL)
        return NULL;
    DWORD Flags;
    // @pyparm int|Flags||Combination of shellcon.AD_APPLY_* flags
    if (!PyArg_ParseTuple(args, "k:ApplyChanges", &Flags))
        return NULL;
    HRESULT hr;
    PY_INTERFACE_PRECALL;
    hr = pIAD->ApplyChanges(Flags);

    PY_INTERFACE_POSTCALL;

    if (FAILED(hr))
        return PyCom_BuildPyException(hr, pIAD, IID_IActiveDesktop);
    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod <o PyUNICODE>|PyIActiveDesktop|GetWallpaper|Returns the current wallpaper
PyObject *PyIActiveDesktop::GetWallpaper(PyObject *self, PyObject *args)
{
    IActiveDesktop *pIAD = GetI(self);
    if (pIAD == NULL)
        return NULL;
    DWORD reserved = 0;
    UINT bufsize = MAX_PATH;
    WCHAR *Wallpaper = NULL;
    // @pyparm int|cchWallpaper|MAX_PATH|Number of characters to allocate for buffer
    // @pyparm int|Reserved|0|Use 0 if passed in
    if (!PyArg_ParseTuple(args, "|kk:GetWallpaper", &bufsize, &reserved))
        return NULL;
    Wallpaper = (WCHAR *)malloc(bufsize * sizeof(WCHAR));
    if (Wallpaper == NULL)
        return PyErr_Format(PyExc_MemoryError, "Unable to allocate %d bytes", bufsize * sizeof(WCHAR));

    HRESULT hr;
    PY_INTERFACE_PRECALL;
    hr = pIAD->GetWallpaper(Wallpaper, bufsize, reserved);
    PY_INTERFACE_POSTCALL;
    PyObject *ret = NULL;
    if (FAILED(hr))
        PyCom_BuildPyException(hr, pIAD, IID_IActiveDesktop);
    else
        ret = PyWinObject_FromWCHAR(Wallpaper);
    free(Wallpaper);
    return ret;
    ;
}

// @pymethod |PyIActiveDesktop|SetWallpaper|Sets the desktop wallpaper
PyObject *PyIActiveDesktop::SetWallpaper(PyObject *self, PyObject *args)
{
    IActiveDesktop *pIAD = GetI(self);
    if (pIAD == NULL)
        return NULL;
    PyObject *obWallpaper;
    WCHAR *Wallpaper;
    DWORD reserved = 0;
    if (!PyArg_ParseTuple(args, "O|k:SetWallpaper",
                          &obWallpaper,  // @pyparm <o PyUNICODE>|Wallpaper||File to be used as new wallpaper
                          &reserved))    // @pyparm int|Reserved|0|Reserved, use 0 if passed in
        return NULL;
    if (!PyWinObject_AsWCHAR(obWallpaper, &Wallpaper, FALSE))
        return NULL;
    HRESULT hr;
    PY_INTERFACE_PRECALL;
    hr = pIAD->SetWallpaper(Wallpaper, reserved);
    PY_INTERFACE_POSTCALL;
    PyWinObject_FreeWCHAR(Wallpaper);
    if (FAILED(hr))
        return PyCom_BuildPyException(hr, pIAD, IID_IActiveDesktop);
    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod int|PyIActiveDesktop|GetWallpaperOptions|Returns wallpaper style
// @rdesc Returns one of the WPSTYLE_* values
PyObject *PyIActiveDesktop::GetWallpaperOptions(PyObject *self, PyObject *args)
{
    IActiveDesktop *pIAD = GetI(self);
    if (pIAD == NULL)
        return NULL;

    DWORD reserved = 0;
    WALLPAPEROPT wpopt;
    wpopt.dwSize = sizeof(WALLPAPEROPT);
    if (!PyArg_ParseTuple(args, "|k:GetWallpaperOptions",
                          &reserved))  // @pyparm int|Reserved|0|Use 0 if passed in
        return NULL;

    HRESULT hr;
    PY_INTERFACE_PRECALL;
    hr = pIAD->GetWallpaperOptions(&wpopt, reserved);
    PY_INTERFACE_POSTCALL;

    if (FAILED(hr))
        return PyCom_BuildPyException(hr, pIAD, IID_IActiveDesktop);
    return PyLong_FromUnsignedLong(wpopt.dwStyle);
}

// @pymethod |PyIActiveDesktop|SetWallpaperOptions|Sets wallpaper style
PyObject *PyIActiveDesktop::SetWallpaperOptions(PyObject *self, PyObject *args)
{
    IActiveDesktop *pIAD = GetI(self);
    if (pIAD == NULL)
        return NULL;
    DWORD reserved = 0;
    WALLPAPEROPT wpopt;
    wpopt.dwSize = sizeof(WALLPAPEROPT);

    if (!PyArg_ParseTuple(args, "k|k:SetWallpaperOptions",
                          &wpopt.dwStyle,  // @pyparm int|Style||The wallpaper style, one of the WPSTYLE_* constants
                          &reserved))      // @pyparm int|Reserved|0|Reserved, use 0 if passed in
        return NULL;
    HRESULT hr;
    PY_INTERFACE_PRECALL;
    hr = pIAD->SetWallpaperOptions(&wpopt, reserved);
    PY_INTERFACE_POSTCALL;

    if (FAILED(hr))
        return PyCom_BuildPyException(hr, pIAD, IID_IActiveDesktop);
    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod |PyIActiveDesktop|GetPattern|Returns the wallpaper pattern
// @rdesc Returns a unicode string containing decimal values representing the pattern
PyObject *PyIActiveDesktop::GetPattern(PyObject *self, PyObject *args)
{
    IActiveDesktop *pIAD = GetI(self);
    if (pIAD == NULL)
        return NULL;
    DWORD reserved = 0;
    UINT bufsize = 1024;
    WCHAR *Pattern = NULL;
    if (!PyArg_ParseTuple(args, "|kk:GetPattern",
                          &bufsize,    // @pyparm int|cchPattern|1024|Number of characters to allocate for buffer
                          &reserved))  // @pyparm int|Reserved|0|Use 0 if passed in
        return NULL;
    Pattern = (WCHAR *)malloc(bufsize * sizeof(WCHAR));
    if (Pattern == NULL)
        return PyErr_Format(PyExc_MemoryError, "Unable to allocate %d bytes", bufsize * sizeof(WCHAR));
    HRESULT hr;
    PY_INTERFACE_PRECALL;
    hr = pIAD->GetPattern(Pattern, bufsize, reserved);
    PY_INTERFACE_POSTCALL;
    PyObject *ret = NULL;
    if (FAILED(hr))
        PyCom_BuildPyException(hr, pIAD, IID_IActiveDesktop);
    else
        ret = PyWinObject_FromWCHAR(Pattern);
    free(Pattern);
    return ret;
}

// @pymethod |PyIActiveDesktop|SetPattern|Sets the wallpaper pattern
PyObject *PyIActiveDesktop::SetPattern(PyObject *self, PyObject *args)
{
    IActiveDesktop *pIAD = GetI(self);
    if (pIAD == NULL)
        return NULL;
    PyObject *obPattern;
    WCHAR *Pattern = NULL;
    DWORD reserved = 0;
    if (!PyArg_ParseTuple(
            args, "O|k:SetPattern",
            &obPattern,  // @pyparm <o PyUNICODE>|Pattern||String of decimal numbers representing a picture
            &reserved))  // @pyparm int|Reserved|0|Use 0 if passed in
        return NULL;
    if (!PyWinObject_AsWCHAR(obPattern, &Pattern, TRUE))
        return NULL;

    HRESULT hr;
    PY_INTERFACE_PRECALL;
    hr = pIAD->SetPattern(Pattern, reserved);
    PY_INTERFACE_POSTCALL;

    if (FAILED(hr))
        return PyCom_BuildPyException(hr, pIAD, IID_IActiveDesktop);
    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod dict|PyIActiveDesktop|GetDesktopItemOptions|Returns options for Active Desktop.
// @rdesc Returns a <o COMPONENTSOPT> dictionary
PyObject *PyIActiveDesktop::GetDesktopItemOptions(PyObject *self, PyObject *args)
{
    IActiveDesktop *pIAD = GetI(self);
    if (pIAD == NULL)
        return NULL;
    if (!PyArg_ParseTuple(args, ":GetDesktopItemOptions"))
        return NULL;

    COMPONENTSOPT copt;
    copt.dwSize = sizeof(copt);
    DWORD reserved = 0;
    HRESULT hr;
    PY_INTERFACE_PRECALL;
    hr = pIAD->GetDesktopItemOptions(&copt, reserved);
    PY_INTERFACE_POSTCALL;

    if (FAILED(hr))
        return PyCom_BuildPyException(hr, pIAD, IID_IActiveDesktop);
    return PyWinObject_FromCOMPONENTSOPT(&copt);
}

// @pymethod |PyIActiveDesktop|SetDesktopItemOptions|Sets Active Desktop options
PyObject *PyIActiveDesktop::SetDesktopItemOptions(PyObject *self, PyObject *args)
{
    IActiveDesktop *pIAD = GetI(self);
    if (pIAD == NULL)
        return NULL;
    PyObject *obcopt;
    COMPONENTSOPT copt;
    DWORD reserved = 0;
    if (!PyArg_ParseTuple(args, "O|k:SetDesktopItemOptions",
                          &obcopt,     // @pyparm dict|comp||<o COMPONENTSOPT> dictionary
                          &reserved))  // @pyparm int|Reserved|0|Use 0 if passed in
        return NULL;
    if (!PyWinObject_AsCOMPONENTSOPT(obcopt, &copt))
        return NULL;

    HRESULT hr;
    PY_INTERFACE_PRECALL;
    hr = pIAD->SetDesktopItemOptions(&copt, reserved);
    PY_INTERFACE_POSTCALL;

    if (FAILED(hr))
        return PyCom_BuildPyException(hr, pIAD, IID_IActiveDesktop);
    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod |PyIActiveDesktop|AddDesktopItem|Creates a new item to display on the desktop
PyObject *PyIActiveDesktop::AddDesktopItem(PyObject *self, PyObject *args)
{
    IActiveDesktop *pIAD = GetI(self);
    if (pIAD == NULL)
        return NULL;
    PyObject *obcomp;
    DWORD reserved = 0;
    COMPONENT comp;
    if (!PyArg_ParseTuple(args, "O|k:AddDesktopItem",
                          &obcomp,     // @pyparm dict|comp||<o COMPONENT> dictionary
                          &reserved))  // @pyparm int|Reserved|0|Use 0 if passed in
        return NULL;
    if (!PyWinObject_AsCOMPONENT(obcomp, &comp))
        return NULL;

    HRESULT hr;
    PY_INTERFACE_PRECALL;
    hr = pIAD->AddDesktopItem(&comp, reserved);
    PY_INTERFACE_POSTCALL;

    if (FAILED(hr))
        return PyCom_BuildPyException(hr, pIAD, IID_IActiveDesktop);
    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod |PyIActiveDesktop|AddDesktopItemWithUI|Adds a desktop item, allowing user interaction
PyObject *PyIActiveDesktop::AddDesktopItemWithUI(PyObject *self, PyObject *args)
{
    IActiveDesktop *pIAD = GetI(self);
    if (pIAD == NULL)
        return NULL;
    PyObject *obcomp, *obhwnd;
    COMPONENT comp;
    DWORD flags;
    HWND hwnd;
    if (!PyArg_ParseTuple(args, "OOk:AddDesktopItemWithUI",
                          &obhwnd,  // @pyparm <o PyHANDLE>|hwnd||Handle to parent window
                          &obcomp,  // @pyparm dict|comp||<o COMPONENT> dictionary
                          &flags))  // @pyparm int|Flags||One of shellcon.DTI_ADDUI_* flags
        return NULL;
    if (!PyWinObject_AsHANDLE(obhwnd, (HANDLE *)&hwnd))
        return NULL;
    if (!PyWinObject_AsCOMPONENT(obcomp, &comp))
        return NULL;

    HRESULT hr;
    PY_INTERFACE_PRECALL;
    hr = pIAD->AddDesktopItemWithUI(hwnd, &comp, flags);
    PY_INTERFACE_POSTCALL;

    if (FAILED(hr))
        return PyCom_BuildPyException(hr, pIAD, IID_IActiveDesktop);
    Py_INCREF(Py_None);
    return Py_None;
}
// @pymethod |PyIActiveDesktop|ModifyDesktopItem|Changes parameters for a desktop item
PyObject *PyIActiveDesktop::ModifyDesktopItem(PyObject *self, PyObject *args)
{
    IActiveDesktop *pIAD = GetI(self);
    if (pIAD == NULL)
        return NULL;
    PyObject *obcomp;
    COMPONENT comp;
    DWORD flags;
    if (!PyArg_ParseTuple(args, "Ok:ModifyDesktopItem",
                          &obcomp,  // @pyparm dict|comp||<o COMPONENT> dictionary
                          &flags))  // @pyparm int|Flags||Combination of shellcon.COMP_ELEM_* flags
        return NULL;
    if (!PyWinObject_AsCOMPONENT(obcomp, &comp))
        return NULL;

    HRESULT hr;
    PY_INTERFACE_PRECALL;
    hr = pIAD->ModifyDesktopItem(&comp, flags);
    PY_INTERFACE_POSTCALL;

    if (FAILED(hr))
        return PyCom_BuildPyException(hr, pIAD, IID_IActiveDesktop);
    Py_INCREF(Py_None);
    return Py_None;
}
// @pymethod |PyIActiveDesktop|RemoveDesktopItem|Removes an item from the Active Desktop
PyObject *PyIActiveDesktop::RemoveDesktopItem(PyObject *self, PyObject *args)
{
    IActiveDesktop *pIAD = GetI(self);
    if (pIAD == NULL)
        return NULL;
    PyObject *obcomp;
    DWORD reserved = 0;
    COMPONENT comp;
    if (!PyArg_ParseTuple(args, "O|k:RemoveDesktopItem",
                          &obcomp,  // @pyparm dict|comp||<o COMPONENT> dictionary specifying which component to remove
                          &reserved))  // @pyparm int|Reserved|0|Use 0 if passed in
        return NULL;
    if (!PyWinObject_AsCOMPONENT(obcomp, &comp))
        return NULL;

    HRESULT hr;
    PY_INTERFACE_PRECALL;
    hr = pIAD->RemoveDesktopItem(&comp, reserved);
    PY_INTERFACE_POSTCALL;

    if (FAILED(hr))
        return PyCom_BuildPyException(hr, pIAD, IID_IActiveDesktop);
    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod |PyIActiveDesktop|GetDesktopItemCount|Returns number of defined desktop items.
PyObject *PyIActiveDesktop::GetDesktopItemCount(PyObject *self, PyObject *args)
{
    IActiveDesktop *pIAD = GetI(self);
    if (pIAD == NULL)
        return NULL;
    if (!PyArg_ParseTuple(args, ":GetDesktopItemCount"))
        return NULL;
    DWORD reserved = 0;
    int Count;

    HRESULT hr;
    PY_INTERFACE_PRECALL;
    hr = pIAD->GetDesktopItemCount(&Count, reserved);
    PY_INTERFACE_POSTCALL;

    if (FAILED(hr))
        return PyCom_BuildPyException(hr, pIAD, IID_IActiveDesktop);
    return PyInt_FromLong(Count);
}

// @pymethod dict|PyIActiveDesktop|GetDesktopItem|Returns desktop item parameters by index
// @rdesc Returns a <o COMPONENT> dictionary describing the item
PyObject *PyIActiveDesktop::GetDesktopItem(PyObject *self, PyObject *args)
{
    IActiveDesktop *pIAD = GetI(self);
    if (pIAD == NULL)
        return NULL;
    COMPONENT comp;
    int index;
    DWORD reserved = 0;
    if (!PyArg_ParseTuple(args, "i|k:GetDesktopItem",
                          &index,      // @pyparm int|Component||The zero-based index of the component to get
                          &reserved))  // @pyparm int|Reserved|0|Use 0 if passed in
        return NULL;

    comp.dwSize = sizeof(comp);
    HRESULT hr;
    PY_INTERFACE_PRECALL;
    hr = pIAD->GetDesktopItem(index, &comp, reserved);
    PY_INTERFACE_POSTCALL;

    if (FAILED(hr))
        return PyCom_BuildPyException(hr, pIAD, IID_IActiveDesktop);
    return PyWinObject_FromCOMPONENT(&comp);
}

// @pymethod dict|PyIActiveDesktop|GetDesktopItemByID|Returns desktop item parameters by Id
// @rdesc Returns a <o COMPONENT> dictionary
PyObject *PyIActiveDesktop::GetDesktopItemByID(PyObject *self, PyObject *args)
{
    IActiveDesktop *pIAD = GetI(self);
    if (pIAD == NULL)
        return NULL;
    DWORD ID, reserved = 0;
    COMPONENT comp;
    if (!PyArg_ParseTuple(args, "k|k:GetDesktopItemByID",
                          &ID,         // @pyparm int|ID||The Id of the desktop item
                          &reserved))  // @pyparm int|reserved|0|Use 0 if passed in
        return NULL;
    comp.dwSize = sizeof(comp);
    HRESULT hr;
    PY_INTERFACE_PRECALL;
    hr = pIAD->GetDesktopItemByID(ID, &comp, reserved);
    PY_INTERFACE_POSTCALL;

    if (FAILED(hr))
        return PyCom_BuildPyException(hr, pIAD, IID_IActiveDesktop);
    return PyWinObject_FromCOMPONENT(&comp);
}

// @pymethod |PyIActiveDesktop|GenerateDesktopItemHtml|Creates an HTML page for the desktop item
PyObject *PyIActiveDesktop::GenerateDesktopItemHtml(PyObject *self, PyObject *args)
{
    IActiveDesktop *pIAD = GetI(self);
    if (pIAD == NULL)
        return NULL;
    PyObject *obcomp, *obFileName;
    COMPONENT comp;
    DWORD reserved = 0;
    WCHAR *FileName = NULL;
    if (!PyArg_ParseTuple(args, "OO|k:GenerateDesktopItemHtml",
                          &obFileName,  // @pyparm <o PyUNICODE>|FileName||Name of file to be created
                          &obcomp,      // @pyparm dict|comp||<o COMPONENT> dictionary specifying the desktop item
                          &reserved))   // @pyparm int|Reserved|0|Use 0 if passed in
        return NULL;
    if (!PyWinObject_AsWCHAR(obFileName, &FileName, FALSE))
        return NULL;

    HRESULT hr;
    PY_INTERFACE_PRECALL;
    hr = pIAD->GenerateDesktopItemHtml(FileName, &comp, reserved);
    PY_INTERFACE_POSTCALL;

    PyWinObject_FreeWCHAR(FileName);
    if (FAILED(hr))
        return PyCom_BuildPyException(hr, pIAD, IID_IActiveDesktop);
    Py_INCREF(Py_None);
    return Py_None;
}
// @pymethod |PyIActiveDesktop|AddUrl|Adds a web page to desktop, allowing user interaction
PyObject *PyIActiveDesktop::AddUrl(PyObject *self, PyObject *args)
{
    IActiveDesktop *pIAD = GetI(self);
    if (pIAD == NULL)
        return NULL;
    HWND hwnd;
    PyObject *obcomp, *obSource, *obhwnd;
    WCHAR *Source = NULL;
    COMPONENT comp;
    DWORD flags;
    if (!PyArg_ParseTuple(args, "OOOk:AddUrl",
                          &obhwnd,    // @pyparm <o PyHANDLE>|hwnd||Parent windows for any user interactive
                          &obSource,  // @pyparm <o PyUNICODE>|Source||Source URL
                          &obcomp,    // @pyparm dict|comp||<o COMPONENT> dictionary
                          &flags))    // @pyparm int|Flags||ADDURL_SILENT, or 0
        return NULL;
    if (!PyWinObject_AsHANDLE(obhwnd, (HANDLE *)&hwnd))
        return NULL;
    if (!PyWinObject_AsCOMPONENT(obcomp, &comp))
        return NULL;
    if (!PyWinObject_AsWCHAR(obSource, &Source, FALSE))
        return NULL;

    HRESULT hr;
    PY_INTERFACE_PRECALL;
    hr = pIAD->AddUrl(hwnd, Source, &comp, flags);
    PY_INTERFACE_POSTCALL;
    PyWinObject_FreeWCHAR(Source);
    if (FAILED(hr))
        return PyCom_BuildPyException(hr, pIAD, IID_IActiveDesktop);
    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod dict|PyIActiveDesktop|GetDesktopItemBySource|Returns desktop item parameters by URL
// @rdesc Returns a <o COMPONENT> dictionary
PyObject *PyIActiveDesktop::GetDesktopItemBySource(PyObject *self, PyObject *args)
{
    IActiveDesktop *pIAD = GetI(self);
    if (pIAD == NULL)
        return NULL;

    PyObject *obSource;
    COMPONENT comp;
    DWORD reserved = 0;
    WCHAR *Source = NULL;
    if (!PyArg_ParseTuple(args, "O|k:GetDesktopItemBySource",
                          &obSource,   // @pyparm <o PyUNICODE>|Source||The URL address of the item to retrieve
                          &reserved))  // @pyparm int|Reserved|0|Use 0 if passed in
        return NULL;
    if (!PyWinObject_AsWCHAR(obSource, &Source, FALSE))
        return NULL;
    comp.dwSize = sizeof(comp);
    HRESULT hr;
    PY_INTERFACE_PRECALL;
    hr = pIAD->GetDesktopItemBySource(Source, &comp, reserved);
    PY_INTERFACE_POSTCALL;

    PyWinObject_FreeWCHAR(Source);
    if (FAILED(hr))
        return PyCom_BuildPyException(hr, pIAD, IID_IActiveDesktop);
    return PyWinObject_FromCOMPONENT(&comp);
}

// @object PyIActiveDesktop|An interface to the ActiveDesktop
static struct PyMethodDef PyIActiveDesktop_methods[] = {
    // @pymeth ApplyChanges|Applies changes to ActiveDesktop settings and persists them to the registry.
    {"ApplyChanges", PyIActiveDesktop::ApplyChanges, METH_VARARGS,
     "Applies changes to ActiveDesktop settings and persists them to the registry"},
    // @pymeth GetWallpaper|Returns the current wallpaper
    {"GetWallpaper", PyIActiveDesktop::GetWallpaper, METH_VARARGS, "Returns the current wallpaper"},
    // @pymeth SetWallpaper|Sets the desktop wallpaper
    {"SetWallpaper", PyIActiveDesktop::SetWallpaper, METH_VARARGS, "Sets the desktop wallpaper"},
    // @pymeth GetWallpaperOptions|Returns wallpaper style
    {"GetWallpaperOptions", PyIActiveDesktop::GetWallpaperOptions, METH_VARARGS, "Returns wallpaper style"},
    // @pymeth SetWallpaperOptions|Sets wallpaper style
    {"SetWallpaperOptions", PyIActiveDesktop::SetWallpaperOptions, METH_VARARGS, "Sets wallpaper style"},
    // @pymeth GetPattern|Returns the wallpaper pattern
    {"GetPattern", PyIActiveDesktop::GetPattern, METH_VARARGS, "Returns the wallpaper pattern"},
    // @pymeth SetPattern|Sets the wallpaper pattern
    {"SetPattern", PyIActiveDesktop::SetPattern, METH_VARARGS, "Sets the wallpaper pattern"},
    // @pymeth GetDesktopItemOptions|Returns options for Active Desktop.
    {"GetDesktopItemOptions", PyIActiveDesktop::GetDesktopItemOptions, METH_VARARGS,
     "Returns options for Active Desktop."},
    // @pymeth SetDesktopItemOptions|Sets Active Desktop options
    {"SetDesktopItemOptions", PyIActiveDesktop::SetDesktopItemOptions, METH_VARARGS, "Sets Active Desktop options"},
    // @pymeth AddDesktopItem|Creates a new item to display on the desktop
    {"AddDesktopItem", PyIActiveDesktop::AddDesktopItem, METH_VARARGS, "Creates a new item to display on the desktop"},
    // @pymeth AddDesktopItemWithUI|Adds a desktop item, allowing user interaction
    {"AddDesktopItemWithUI", PyIActiveDesktop::AddDesktopItemWithUI, METH_VARARGS,
     "Adds a desktop item, allowing user interaction"},
    // @pymeth ModifyDesktopItem|Changes parameters for a desktop item
    {"ModifyDesktopItem", PyIActiveDesktop::ModifyDesktopItem, METH_VARARGS, "Changes parameters for a desktop item"},
    // @pymeth RemoveDesktopItem|Removes an item from the Active Desktop
    {"RemoveDesktopItem", PyIActiveDesktop::RemoveDesktopItem, METH_VARARGS, "Removes an item from the Active Desktop"},
    // @pymeth GetDesktopItemCount|Returns number of defined desktop items.
    {"GetDesktopItemCount", PyIActiveDesktop::GetDesktopItemCount, METH_VARARGS,
     "Returns number of defined desktop items."},
    // @pymeth GetDesktopItem|Returns desktop item parameters by index
    {"GetDesktopItem", PyIActiveDesktop::GetDesktopItem, METH_VARARGS, "Returns desktop item parameters by index"},
    // @pymeth GetDesktopItemByID|Returns desktop item parameters by Id
    {"GetDesktopItemByID", PyIActiveDesktop::GetDesktopItemByID, METH_VARARGS, "Returns desktop item parameters by Id"},
    // @pymeth GenerateDesktopItemHtml|Creates an HTML page for the desktop item
    {"GenerateDesktopItemHtml", PyIActiveDesktop::GenerateDesktopItemHtml, METH_VARARGS,
     "Creates an HTML page for the desktop item"},
    // @pymeth AddUrl|Adds a web page to desktop, allowing user interaction
    {"AddUrl", PyIActiveDesktop::AddUrl, METH_VARARGS, "Adds a web page to desktop, allowing user interaction"},
    // @pymeth GetDesktopItemBySource|Returns desktop item parameters by URL
    {"GetDesktopItemBySource", PyIActiveDesktop::GetDesktopItemBySource, METH_VARARGS,
     "Returns desktop item parameters by URL"},
    {NULL}};

PyComTypeObject PyIActiveDesktop::type("PyIActiveDesktop", &PyIUnknown::type, sizeof(PyIActiveDesktop),
                                       PyIActiveDesktop_methods, GET_PYCOM_CTOR(PyIActiveDesktop));

///////////////////////////////////////////////////////////////////////////////////////////////////////
// IActiveDesktopP
///////////////////////////////////////////////////////////////////////////////////////////////////////
PyIActiveDesktopP::PyIActiveDesktopP(IUnknown *pdisp) : PyIUnknown(pdisp) { ob_type = &type; }

PyIActiveDesktopP::~PyIActiveDesktopP() {}

IActiveDesktopP *PyIActiveDesktopP::GetI(PyObject *self) { return (IActiveDesktopP *)PyIUnknown::GetI(self); }

// @pymethod |PyIActiveDesktopP|SetSafeMode|Changes Active Desktop to safe mode
PyObject *PyIActiveDesktopP::SetSafeMode(PyObject *self, PyObject *args)
{
    IActiveDesktopP *pIAD = GetI(self);
    if (pIAD == NULL)
        return NULL;
    DWORD Flags;
    // @pyparm int|Flags||One of shellcon.SSM_* flags
    if (!PyArg_ParseTuple(args, "k:SetSafeMode", &Flags))
        return NULL;

    HRESULT hr;
    PY_INTERFACE_PRECALL;
    hr = pIAD->SetSafeMode(Flags);
    PY_INTERFACE_POSTCALL;

    if (FAILED(hr))
        return PyCom_BuildPyException(hr, pIAD, IID_IActiveDesktopP);
    Py_INCREF(Py_None);
    return Py_None;
}

// @object PyIActiveDesktopP|An interface to the ActiveDesktop
static struct PyMethodDef PyIActiveDesktopP_methods[] = {
    // @pymeth SetSafeMode|Changes Active Desktop to safe mode
    {"SetSafeMode", PyIActiveDesktopP::SetSafeMode, METH_VARARGS, "Changes Active Desktop to safe mode"},
    {NULL}};

PyComTypeObject PyIActiveDesktopP::type("PyIActiveDesktopP", &PyIUnknown::type, sizeof(PyIActiveDesktopP),
                                        PyIActiveDesktopP_methods, GET_PYCOM_CTOR(PyIActiveDesktopP));

///////////////////////////////////////////////////////////////////////////////////////////////////////
// IADesktopP2
///////////////////////////////////////////////////////////////////////////////////////////////////////
PyIADesktopP2::PyIADesktopP2(IUnknown *pdisp) : PyIUnknown(pdisp) { ob_type = &type; }

PyIADesktopP2::~PyIADesktopP2() {}

IADesktopP2 *PyIADesktopP2::GetI(PyObject *self) { return (IADesktopP2 *)PyIUnknown::GetI(self); }

// @pymethod |PyIADesktopP2|UpdateAllDesktopSubscriptions|Updates webpage subscriptions on the desktop
PyObject *PyIADesktopP2::UpdateAllDesktopSubscriptions(PyObject *self, PyObject *args)
{
    IADesktopP2 *pIAD = GetI(self);
    if (pIAD == NULL)
        return NULL;
    if (!PyArg_ParseTuple(args, ":UpdateAllDesktopSubscriptions"))
        return NULL;

    HRESULT hr;
    PY_INTERFACE_PRECALL;
    hr = pIAD->UpdateAllDesktopSubscriptions();
    PY_INTERFACE_POSTCALL;

    if (FAILED(hr))
        return PyCom_BuildPyException(hr, pIAD, IID_IADesktopP2);
    Py_INCREF(Py_None);
    return Py_None;
}

// @object PyIADesktopP2|An interface to the ActiveDesktop
static struct PyMethodDef PyIADesktopP2_methods[] = {
    // @pymeth UpdateAllDesktopSubscriptions|Updates webpage subscriptions on the desktop
    {"UpdateAllDesktopSubscriptions", PyIADesktopP2::UpdateAllDesktopSubscriptions, METH_VARARGS,
     "Updates webpage subscriptions on the desktop"},
    {NULL}};

PyComTypeObject PyIADesktopP2::type("PyIADesktopP2", &PyIUnknown::type, sizeof(PyIADesktopP2), PyIADesktopP2_methods,
                                    GET_PYCOM_CTOR(PyIADesktopP2));
