/*

    win32 Notify Handler

    Created May 1995, Mark Hammond (MHammond@skippinet.com.au)

*/
#include "stdafx.h"
#include "win32win.h"

// Not very general purpose notify parser!
PyObject *PyNotifyMakeExtraTuple(NMHDR *ptr, char *fmt)
{
    char *pUse = (char *)(ptr + 1);
    int argNo = 0;
    int tupleLen = 0;
    for (char *szTemp = fmt; *szTemp; ++szTemp) {
        if (*szTemp == '-')
            ++szTemp;  // skip next one.
        else if (isdigit(*szTemp))
            ;  // ignore it.
        else
            ++tupleLen;  // count it
    }
    PyObject *ret = PyTuple_New(tupleLen);
    PyObject *ob;
    BOOL bIgnore;
    while (*fmt) {
#ifdef _DEBUG
        ob = NULL;
#endif
        bIgnore = *fmt == '-';
        if (bIgnore)
            ++fmt;
        switch (*fmt) {
            case 'i':
                ob = bIgnore ? NULL : PyInt_FromLong(*((int *)pUse));
                pUse += (sizeof(int));
                break;
            case 'P': {  // point
                LONG l1 = *((LONG *)pUse);
                pUse += (sizeof(long));
                LONG l2 = *((LONG *)pUse);
                pUse += (sizeof(long));
                ob = bIgnore ? NULL : Py_BuildValue("ll", l1, l2);
                break;
            }
            case 'z':  // string pointer
            case 's':  // string buffer - same for this parse
            {
                char *use = (*fmt == 'z') ? *(char **)pUse : pUse;
                ob = bIgnore ? NULL : PyString_FromString("");  // HACK HACK - FIX ME FIX ME
                if (*fmt == 's') {                              // followed by buffer size;
                    int val = 0;
                    while (fmt[1] && isdigit(fmt[1])) {
                        val = val * 10 + (fmt[1] - '0');
                        fmt++;
                    }
                    pUse += sizeof(char) * val;
                }
                else {
                    pUse += sizeof(char *);
                }
                break;
            }
            case 'Z':  // Unicode string pointer
            case 'S':  // Unicode buffer - same for this parse
            {
                char *use = (*fmt == 'Z') ? *(char **)pUse : pUse;
                ob = bIgnore ? NULL : PyString_FromString("");  // HACK HACK - FIX ME FIX ME
                if (*fmt == 'S') {                              // followed by buffer size;
                    int val = 0;
                    while (fmt[1] && isdigit(fmt[1])) {
                        val = val * 10 + (fmt[1] - '0');
                        fmt++;
                    }
                    pUse += sizeof(wchar_t) * val;
                }
                else {
                    pUse += sizeof(wchar_t *);
                }
                break;
            }

            case 'O':  // object with no reference count maintained
                ob = bIgnore ? NULL : (PyObject *)pUse;
                Py_INCREF(ob);
                pUse += (sizeof(PyObject *));
                break;
            case 'T': {  // TV_ITEM structure
                TV_ITEM *ptv = (TV_ITEM *)pUse;
                ob = bIgnore ? NULL : PyWinObject_FromTV_ITEM(ptv);
                pUse += (sizeof(TV_ITEM));
                break;
            }
            case 'L': {  // LV_ITEM structure
                LV_ITEM *plv = (LV_ITEM *)pUse;
                ob = bIgnore ? NULL : PyWinObject_FromLV_ITEM(plv);
                pUse += (sizeof(LV_ITEM));
                break;
            }
                /*
            case 'H': {// HD_ITEM structure
                HD_ITEM *phd = (HD_ITEM *)pUse;
                ob = bIgnore ? NULL : MakeHD_ITEMTuple(phd);
                pUse += (sizeof(HD_ITEM));
                break;
                }
                */
            case 'V': {  // Pointer-sized number, also used for HANDLE's
                ob = bIgnore ? NULL : PyWinLong_FromVoidPtr(*(void **)pUse);
                pUse += (sizeof(void *));
                break;
            }
            default:
                ASSERT(FALSE);
                Py_DECREF(ret);
                RETURN_ERR("Bad format char in internal WM_NOTIFY tuple conversion");
        }
        if (!bIgnore) {
            PyTuple_SET_ITEM(ret, argNo, ob);
            argNo++;
        }
        ASSERT(bIgnore == FALSE || ob == NULL);  // check bIgnore logic
        fmt++;
    }
    return ret;
}

#define MY_RET_ERR(msg)                        \
    {                                          \
        PyErr_SetString(ui_module_error, msg); \
        return;                                \
    }
// Not very general purpose notify parser!
void PyNotifyParseExtraTuple(NMHDR *ptr, PyObject *args, char *fmt)
{
    char *pUse = (char *)(ptr + 1);
    BOOL bIgnoreFormat;
    BOOL bIgnoreValue;
    int argNum = 0;
    if (fmt == NULL) {
        PyErr_Format(PyExc_ValueError, "Notify code %d not expected to return data", ptr->code);
        return;
    }

    while (*fmt) {
        PyObject *ob = PyTuple_GetItem(args, argNum);
        if (ob == NULL)
            return;
        bIgnoreFormat = *fmt == '-';
        // The user can specify 'None' to say 'leave the value alone'
        bIgnoreValue = (ob == Py_None);
        BOOL bIgnore = bIgnoreFormat || bIgnoreValue;
        if (bIgnore)
            ++fmt;
        switch (*fmt) {
            case 'i':
                if (!bIgnore) {
                    if (!PyInt_Check(ob))
                        MY_RET_ERR("Expected integer object")
                    *((int *)pUse) = PyInt_AsLong(ob);
                }
                pUse += (sizeof(int));
                break;
            case 'P': {  // point
                ASSERT(FALSE);
                break;
            }
            case 'T': {  // TV_ITEM
                ASSERT(FALSE);
                break;
            }
            case 'V': {  // Pointer-sized number, also used for HANDLEs, LPARAMS, etc
                if (!bIgnore) {
                    if (!PyWinLong_AsVoidPtr(ob, (void **)pUse))
                        return;
                }
                pUse += (sizeof(void *));
                break;
            }
            case 'z':  // string pointer
                if (!bIgnore) {
                    ASSERT(FALSE);
                }
                pUse += (sizeof(char *));
                break;
            case 'Z':  // wide string pointer
                if (!bIgnore) {
                    ASSERT(FALSE);
                }
                pUse += (sizeof(wchar_t *));
                break;
            case 's':  // string buffer
            {
                int bufSize = 0;
                while (fmt[1] && isdigit(fmt[1])) {
                    bufSize = bufSize * 10 + (fmt[1] - '0');
                    fmt++;
                }
                ASSERT(bufSize);
                if (!bIgnore) {
                    if (!PyString_Check(ob))
                        MY_RET_ERR("Expected string object")
                    char *val = PyString_AsString(ob);
                    SSIZE_T slen = strlen(val);
                    SSIZE_T copylen = max(bufSize - 1, slen);
                    strncpy(pUse, val, copylen);
                    pUse[copylen] = '\0';
                }
                pUse += bufSize;
                break;
            }
            case 'S':  // Fixed size unicode buffer
            {
                DWORD bufSize = 0;
                while (fmt[1] && isdigit(fmt[1])) {
                    bufSize = bufSize * 10 + (fmt[1] - '0');
                    fmt++;
                }
                ASSERT(bufSize);
                if (!bIgnore) {
                    WCHAR *wchar_buf = NULL;
                    DWORD wchar_cnt;
                    if (!PyWinObject_AsWCHAR(ob, &wchar_buf, FALSE, &wchar_cnt))
                        return;
                    ZeroMemory(pUse, bufSize * sizeof(wchar_t));
                    wcsncpy((WCHAR *)pUse, wchar_buf, min(wchar_cnt, bufSize - 1));
                    PyWinObject_FreeWCHAR(wchar_buf);
                }
                pUse += bufSize * sizeof(wchar_t);
                break;
            }
            case 'O':  // object with no reference count maintained
                ASSERT(FALSE);
                break;
            default:
                ASSERT(FALSE);
                MY_RET_ERR("Bad format char in internal WM_NOTIFY tuple conversion");
        }
        fmt++;
        if (!bIgnoreFormat)
            argNum++;
    }
    return;
}

///////////////////////////////////////////////////////
// General notify handler for Python.
BOOL Python_OnNotify(CWnd *pFrom, WPARAM, LPARAM lParam, LRESULT *pResult)
{
    NMHDR *pHdr = (NMHDR *)lParam;
    if (pHdr == NULL)
        return FALSE;  // bad data passed?
    UINT code = pHdr->code;
    CEnterLeavePython _celp;
    PyCCmdTarget *pPyWnd = (PyCCmdTarget *)ui_assoc_CObject::GetAssocObject(pFrom);

    if (pPyWnd == NULL)
        return FALSE;  // no object.
    if (!pPyWnd->is_uiobject(&PyCWnd::type))
        return FALSE;  // unexpected object type.
    PyObject *method;

    if (!pPyWnd->pNotifyHookList || !pPyWnd->pNotifyHookList->Lookup(code, (void *&)method)) {
        Py_DECREF(pPyWnd);
        return FALSE;  // no hook installed.
    }
    Py_DECREF(pPyWnd);

    // have method to call.  Build arguments.
    PyObject *ob1 = Py_BuildValue("Nii", PyWinLong_FromHANDLE(pHdr->hwndFrom), pHdr->idFrom, pHdr->code);
    if (ob1 == NULL) {
        gui_print_error();
        return FALSE;
    }
    char *fmt;
    /*
    NMTOOLBAR format needs to be adjusted for 64-bit, and already doesn't work
    for 32-bit since PyNotifyMakeExtraTuple doesn't have a case for 'b'
    typedef struct tagNMTOOLBAR {
        NMHDR hdr;
        int iItem;
        TBBUTTON tbButton;
        int cchText;
        LPTSTR pszText;
        RECT rcButton;} NMTOOLBAR

    typedef struct _TBBUTTON {
        int         iBitmap;
        int         idCommand;
        BYTE     fsState;
        BYTE     fsStyle;
        #ifdef _WIN64
            BYTE     bReserved[6]     // padding for alignment
        #elif defined(_WIN32)
            BYTE     bReserved[2]     // padding for alignment
        #endif
        DWORD_PTR   dwData;
        INT_PTR          iString;} TBBUTTON
    */

    PyObject *ob2 = Py_None;  // Use None so we can catch NULL for error.
    if (code >= UDN_LAST && code <= UDN_FIRST)
        fmt = "ii";  // NMUPDOWN
    else if (code == TBN_GETBUTTONINFOW)
        fmt = "iiibbiiiZ";  // NMTOOLBAR
    else if (code == TBN_QUERYDELETE || code == TBN_QUERYINSERT || (code >= TBN_ENDDRAG && code <= TBN_FIRST))
        fmt = "iiibbiiiz";
    else if (code == TBN_CUSTHELP || code == TBN_TOOLBARCHANGE || (code >= TBN_RESET && code <= TBN_BEGINADJUST))
        fmt = NULL;  // NMHDR only
    else if (code >= TCN_LAST && code <= TCN_SELCHANGE)
        fmt = "V";  // HWND
    else if (code == TCN_KEYDOWN)
        fmt = "ii";  // NMTCKEYDOWN - ??? First element is a WORD, may work due to alignment ???
    else if (code == TTN_NEEDTEXTA)
        fmt = "-zs80Vi";  // TOOLTIPTEXTA - ie, NMTTDISPINFOA
    else if (code == TTN_NEEDTEXTW)
        fmt = "-ZS80Vi";  // TOOLTIPTEXTW - ie, NMTTDISPINFOW
    else if (code == TTN_POP || code == TTN_SHOW)
        fmt = NULL;  // NMHDR only
    else if (code == TVN_ENDLABELEDITW || code == TVN_BEGINLABELEDITW || code == TVN_SETDISPINFOW ||
             code == TVN_GETDISPINFOW || code == TVN_ENDLABELEDITA || code == TVN_BEGINLABELEDITA ||
             code == TVN_SETDISPINFOA || code == TVN_GETDISPINFOA)
        fmt = "T";  // TV_DISPINFO
    else if (code == TVN_KEYDOWN)
        fmt = "ii";  // NMTVKEYDOWN ??? First element is a WORD ???
    else if (code >= TVN_LAST && code <= TVN_FIRST) {
        /* NM_TREEVIEW
            On 64-bit, struct alignment prevents the size-based unpacking in PyNotifyMakeExtraTuple
            from correctly converting this struct.
        */
        fmt = NULL;
        NMTREEVIEW *nmtv = (NMTREEVIEW *)pHdr;
        ob2 = Py_BuildValue("iNN(ll)", nmtv->action, PyWinObject_FromTV_ITEM(&nmtv->itemOld),
                            PyWinObject_FromTV_ITEM(&nmtv->itemNew), nmtv->ptDrag.x, nmtv->ptDrag.y);
    }
    else if (code == HDN_ITEMDBLCLICKW || code == HDN_ITEMDBLCLICKA)
        fmt = NULL;  // NMHDR only
    else if (code >= HDN_LAST && code <= HDN_FIRST)
        fmt = "iiH";  // HD_NOTIFY
    else if (code == LVN_KEYDOWN)
        fmt = "ii";  // NMLVKEYDOWN ??? First element is a WORD ???
    else if ((code >= LVN_LAST && code <= LVN_GETDISPINFOW) || code == LVN_ENDLABELEDIT || code == LVN_BEGINLABELEDIT)
        fmt = "L";  // NMLVDISPINFO
    else if (code >= LVN_BEGINRDRAG && code <= LVN_FIRST)
        fmt = "iiiiiPV";  // NMLISTVIEW - Last item is an LPARAM
    else
        fmt = NULL;

    if (ob2 == Py_None) {
        if (fmt == NULL)
            ob2 = PyWinLong_FromVoidPtr(pHdr + 1);
        else
            ob2 = PyNotifyMakeExtraTuple(pHdr, fmt);
    }
    if (ob2 == NULL) {
        gui_print_error();
        return FALSE;
    }

    // make the call with my params.
    PyObject *args = Py_BuildValue("NN", ob1, ob2);
    if (args == NULL) {
        gui_print_error();
        return FALSE;
    }

    LRESULT rc = 0;
    BOOL bPassOn = FALSE;
    PyObject *obOther;
    PyObject *result = Python_do_callback(method, args);
    if (result == NULL)
        PyErr_Warn(PyExc_Warning, "Exception in OnNotify() handler");
    else if (result == Py_None)  // allow for None "dont pass on", else result to windows
        bPassOn = TRUE;
    else if
        PyTuple_Check(result)
        {
            // Result should be a tuple of the LRESULT and a tuple to fill the appropriate
            //	struct for this particular message
            if (PyArg_ParseTuple(result, "O&O", PyWinLong_AsVoidPtr, &rc, &obOther))
                PyNotifyParseExtraTuple(pHdr, obOther, fmt);
            if (PyErr_Occurred()) {
                gui_print_error();
                PyErr_Format(ui_module_error, "Error parsing OnNotify() extra return info for code %d, fmt='%s'", code,
                             fmt);
                gui_print_error();
            }
        }
    // Otherwise result is just the LRESULT, which can be anything that fits in pointer size
    else if (!PyWinObject_AsPARAM(result, (LPARAM *)&rc)) {
        gui_print_error();
        PyErr_SetString(ui_module_error,
                        "OnNotify did not return an LRESULT, or a tuple of (LRESULT, notify info tuple)");
        gui_print_error();
        rc = 0;
    }
    Py_XDECREF(result);
    *pResult = rc;
    return !bPassOn;
}
