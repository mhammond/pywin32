/*****************************************************************

The template classes for Pythonwin support.

Typical usage of these classes are:

// Define my class which extends the MFC view support.
class CMyView : public CView {
    int SomeMethod();
    ... etc
};

// Now define a class which has all the hooks to make it
// a class fully extendable by Python.
typedef CPythonViewFramework< CMyView > CPythonMyView;

// Now in the C++ sources, we always make sure we create
// the CPythonMyView...

    CMyView *pMyView = new CPythonMyView(args...);

*****************************************************************/
// @doc
#pragma once  // Only ever want this once
#ifndef __WIN32UIEXT_H__
#define __WIN32UIEXT_H__

template <class T>
class CPythonWndFramework : public T {
   public:
    // EEEK - It seem necessary to have the _union_ of all possible base class ctors.
    // The ctors seem to only be referenced when used, so they don't worry classes that don't use them??
    // What a pain - anyone know how to avoid????

    // CWnd
    CPythonWndFramework() : T() { ; }
    // CPropertySheet.
    CPythonWndFramework(UINT nIDCaption, CWnd *pParentWnd, UINT iSelectPage) : T(nIDCaption, pParentWnd, iSelectPage)
    {
        ;
    }
    CPythonWndFramework(LPCTSTR pszCaption, CWnd *pParentWnd, UINT iSelectPage) : T(pszCaption, pParentWnd, iSelectPage)
    {
        ;
    }
    // CPropertyPage
    CPythonWndFramework(UINT id, UINT caption) : T(id, caption) { ; }
    CPythonWndFramework(LPCTSTR id, UINT caption) : T(id, caption) { ; }
    // CFormView
    CPythonWndFramework(UINT id) : T(id) { ; }
    CPythonWndFramework(LPCTSTR id) : T(id) { ; }
    // CCtrlView
    CPythonWndFramework(LPCTSTR lpszClass, DWORD dwStyle) : T(lpszClass, dwStyle) { ; }
    // CPrintDialog.
    CPythonWndFramework(BOOL bPrintSetupOnly, DWORD dwFlags, CWnd *pParentWnd) : T(bPrintSetupOnly, dwFlags, pParentWnd)
    {
        ;
    }
    // End of ctor hacks!!!!

    ~CPythonWndFramework()
    {
        Python_delete_assoc(this);  // frame dieing - make sure Python knows about it.
    }
    virtual BOOL OnCmdMsg(UINT nID, int nCode, void *pExtra, AFX_CMDHANDLERINFO *pHandlerInfo)
    {
        // yield to Python first
        if (Python_OnCmdMsg(this, nID, nCode, pExtra, pHandlerInfo))
            return TRUE;
        else {
            if (!IsWindow(this->m_hWnd))
                return TRUE;
            return T::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
        }
    }
    virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT *pResult)
    {
        // yield to Python first
        if (Python_OnNotify(this, wParam, lParam, pResult))
            return TRUE;
        else {
            if (!IsWindow(this->m_hWnd))
                return TRUE;
            return T::OnNotify(wParam, lParam, pResult);
        }
    }
#ifdef PYWIN_WITH_WINDOWPROC
    virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
    {
        LRESULT res;
        CVirtualHelper helper("WindowProc", this);
        if (!helper.HaveHandler() || !helper.call(message, wParam, lParam) || !helper.retval(res)) {
            helper.release_full();
            return T::WindowProc(message, wParam, lParam);
        }
        return res;
    }
#endif
    virtual BOOL PreTranslateMessage(MSG *pMsg)
    {
        // @pyvirtual tuple|PyCWnd|PreTranslateMessage|Allows a Window to override the PreTranslateMessage handling.
        // @pyparm tuple|msg||Built from a MSG structure using format "iiiii(ii)"
        CVirtualHelper helper("PreTranslateMessage", this);
        // @rdesc The result should be a tuple of the same format as the msg param,
        // in which case the MSG structure will be updated and TRUE returned
        // from the C++ function.  If None is returned, the default handler
        // is called.
        if (helper.HaveHandler() && helper.call(pMsg) && !helper.retnone() && helper.retval(pMsg))
            return TRUE;
        helper.release_full();
        return _BasePreTranslateMessage(pMsg);
    }
    BOOL _BasePreTranslateMessage(MSG *pMsg) { return T::PreTranslateMessage(pMsg); }
    virtual BOOL OnCommand(WPARAM wparam, LPARAM lparam)
    {
        // @pyvirtual tuple|PyCWnd|OnCommand|Allows a Window to override the OnCommand handling.
        // @pyparm int|wparam||
        // @pyparm int|lparam||
        // @comm The base class method must be called manually via <om PyCWnd.OnCommand>.
        // Failure to call the base implementation will prevent all Python command
        // handlers (hooked via <om PyCWnd.HookCommand>).
        BOOL ret;
        CVirtualHelper helper("OnCommand", this);
        if (helper.HaveHandler() && helper.call(wparam, lparam) && helper.retval(ret))
            return ret;
        helper.release_full();
        return _BaseOnCommand(wparam, lparam);
    }
    BOOL _BaseOnCommand(WPARAM wparam, LPARAM lparam) { return T::OnCommand(wparam, lparam); }
    virtual BOOL PreCreateWindow(CREATESTRUCT &cs)
    {
        // @pyvirtual BOOL|PyCWnd|PreCreateWindow|Called by the framework before the creation of the Windows window
        // attached to this View object.
        // @pyparm tuple|CREATESTRUCT||A tuple describing a CREATESTRUCT structure.
        // @xref <om PyCWnd.PreCreateWindow>
        CVirtualHelper helper("PreCreateWindow", this);
        if (helper.HaveHandler()) {
            if (!helper.call(&cs) || !helper.retval(cs))
                return FALSE;
            return TRUE;
        }
        else {
            helper.release_full();
            return T::PreCreateWindow(cs);
        }
    }
    afx_msg void OnClose()
    {
        // @pyvirtual void|PyCWnd|OnClose|Called for the WM_CLOSE message.
        // @comm The default calls DestroyWindow().  If you supply a handler, the default is not called.
        // @xref <om PyCWnd.OnClose>
        CVirtualHelper helper("OnClose", this);
        if (!helper.HaveHandler() || !helper.call()) {
            helper.release_full();
            _BaseOnClose();
        }
    }
    void _BaseOnClose() { T::OnClose(); }
    afx_msg void OnPaletteChanged(CWnd *pFocusWnd)
    {
        // @comm The MFC base class is always called before the Python method.
        T::OnPaletteChanged(pFocusWnd);
        // @pyvirtual |PyCWnd|OnPaletteChanged|Called to allow windows that use a color palette to realize their logical
        // palettes and update their client areas.
        CVirtualHelper helper("OnPaletteChanged", this);
        // @pyparm <o PyCWnd>|focusWnd||The window that caused the system palette to change.
        helper.call(pFocusWnd);
    }
    afx_msg void OnPaletteIsChanging(CWnd *pRealizeWnd)
    {
        T::OnPaletteIsChanging(pRealizeWnd);
        // @pyvirtual |PyCWnd|OnPaletteIsChanging|Informs other applications when an application is going to realize its
        // logical palette.
        CVirtualHelper helper("OnPaletteIsChanging", this);
        // @pyparm <o PyCWnd>|realizeWnd||Specifies the window that is about to realize its logical palette.
        // @comm The MFC base class is always called before the Python method.
        helper.call(pRealizeWnd);
    }
    afx_msg void OnWinIniChange(LPCTSTR lpszSection)
    {
        T::OnWinIniChange(lpszSection);
        // @pyvirtual |PyCWnd|OnWinIniChange|Called when the system configuration changes.
        // @pyparm string|section||The section which changed.
        // @comm The MFC base class is always called before the Python method.
        CVirtualHelper helper("OnWinIniChange", this);
        helper.call(lpszSection);
    }
    afx_msg HBRUSH OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor)
    {
        // @pyvirtual int|PyCWnd|OnCtlColor|Called for the WM_CTLCOLOR message.
        // @comm Setup dc to paint the control pWnd of type nCtlColor.
        // @comm The default calls OnCtlColor().  If you supply a handler, the default is not called.
        // @rdesc Handle of the brush to paint the control's background.
        // @xref <om PyCWnd.OnCtlColor>
        CVirtualHelper helper("OnCtlColor", this);
        HANDLE hRet;
        if (helper.HaveHandler() && helper.call(pDC, pWnd, nCtlColor) && helper.retval(hRet))
            return (HBRUSH)hRet;
        helper.release_full();
        return T::OnCtlColor(pDC, pWnd, nCtlColor);
    }
    afx_msg void OnSysColorChange()
    {
        T::OnSysColorChange();
        // @pyvirtual |PyCWnd|OnSysColorChange|Called for all top-level windows when a change is made in the system
        // color setting.
        // @comm The MFC base class is always called before the Python method.
        CVirtualHelper helper("OnSysColorChange", this);
        helper.call();
    }
    virtual BOOL OnEraseBkgnd(CDC *pDC)
    {
        // @pyvirtual int|PyCWnd|OnEraseBkgnd|Called for the WN_ERASEBACKGROUND message.
        // @rdesc Nonzero if it erases the background; otherwise 0.
        // @xref <om PyCWnd.OnEraseBkgnd>
        CVirtualHelper helper("OnEraseBkgnd", this);
        if (!helper.HaveHandler()) {
            helper.release_full();
            return T::OnEraseBkgnd(pDC);
        }
        BOOL bRet = FALSE;
        // @pyparm <o PyCDC>|dc||The device context.
        if (helper.call(pDC))
            helper.retval(bRet);
        return bRet;
    }
    afx_msg BOOL OnQueryNewPalette()
    {
        // @pyvirtual BOOL|PyCWnd|OnQueryNewPalette|Informs the window it is about to receive input focus, and shoudl
        // realize its logical palette.
        // @comm The base class method must be called manually via <om PyCScrollView.OnQueryNewPalette>
        // @xref <om PyCWnd.OnQueryNewPalette>
        BOOL ret;
        CVirtualHelper helper("OnQueryNewPalette", this);
        if (helper.call() && helper.retval(ret))
            return ret;
        return FALSE;
    }
    afx_msg void OnPaint()
    {
        // @pyvirtual int|PyCWnd|OnPaint|Default message handler.
        // @xref <om Wnd.OnPaint>
        CVirtualHelper helper("OnPaint", this);
        if (!helper.HaveHandler() || !helper.call()) {
            helper.release_full();
            T::OnPaint();
        }
    }
    afx_msg void OnTimer(UINT_PTR nIDEvent)
    {
        // @pyvirtual void|PyCWnd|OnTimer|Called for the WM_TIMER message.
        // @pyparm int|nIDEvent||Specifies the identifier of the timer.
        CVirtualHelper helper("OnTimer", this);
        if (!helper.HaveHandler() || !helper.call(nIDEvent)) {
            helper.release_full();
            T::OnTimer(nIDEvent);
        }
    }
    afx_msg HCURSOR OnQueryDragIcon()
    {
        // @pyvirtual int|PyCWnd|OnQueryDragIcon|Called for the WM_QUERYDRAGICON message.
        // @rdesc The result is an integer containing a HCURSOR for the icon.
        // @xref <om PyCWnd.OnQueryDragIcon>
        CVirtualHelper helper("OnQueryDragIcon", this);
        if (!helper.HaveHandler()) {
            helper.release_full();
            return T::OnQueryDragIcon();
        }
        HANDLE ret;
        if (!helper.call())
            return NULL;
        helper.retval(ret);
        return (HCURSOR)ret;
    }
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct)
    {
        // @pyvirtual int|PyCWnd|OnCreate|Called for the WM_CREATE message.
        // @rdesc The result is an integer indicating if the window should be created.
        // @comm The MFC implementation is always called before Python.
        if (T::OnCreate(lpCreateStruct))
            return -1;
        CVirtualHelper helper("OnCreate", this);
        int ret = 0;
        if (helper.call(lpCreateStruct))
            helper.retval(ret);
        return ret;
    }
    afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR *lpncsp)
    {
        // @pyvirtual |PyCWnd|OnNcCalcSize|Called for the WM_NCCALCSIZE message.
        CVirtualHelper helper("OnNcCalcSize", this);
        if (helper.HaveHandler()) {
            if (bCalcValidRects) {
                PyObject *rc1 = PyWinObject_FromRECT(&lpncsp->rgrc[0], false);
                PyObject *rc2 = PyWinObject_FromRECT(&lpncsp->rgrc[1], false);
                PyObject *rc3 = PyWinObject_FromRECT(&lpncsp->rgrc[2], false);
                WINDOWPOS *pwp = lpncsp->lppos;
                PyObject *obPos;
                if (pwp == NULL) {
                    obPos = Py_None;
                    Py_INCREF(obPos);
                }
                else
                    obPos = helper.build_args("iiiiiii", pwp->hwnd, pwp->hwndInsertAfter, pwp->x, pwp->y, pwp->cx,
                                              pwp->cy, pwp->flags);
                helper.call_args("i(NNNN)", bCalcValidRects, rc1, rc2, rc3, obPos);
            }
            else {
                PyObject *rc1 = PyWinObject_FromRECT((RECT *)lpncsp, false);
                helper.call_args("i(Nzzz)", bCalcValidRects, rc1, NULL, NULL, NULL);
            }
        }
        else {
            helper.release_full();
            T::OnNcCalcSize(bCalcValidRects, lpncsp);
        }
    }
    afx_msg
#ifdef _WIN64  // add one more thing to things I don't understand..
        LRESULT
#else
        UINT
#endif
        OnNcHitTest(CPoint pt)
    {
        // @pyvirtual int|PyCWnd|OnNcHitTest|Called for the WM_NCHITTEST message.
        // @xref <om PyCWnd.OnNcHitTest>
        CVirtualHelper helper("OnNcHitTest", this);
        // @pyparm int, int|x,y||The point to test.
        if (helper.HaveHandler()) {
            if (helper.call_args("((ii))", pt.x, pt.y)) {
                int ret;
                if (helper.retval(ret))
                    return ret;
            }
        }
        helper.release_full();
        return T::OnNcHitTest(pt);
    }
    afx_msg BOOL OnSetCursor(CWnd *pWnd, UINT ht, UINT msg)
    {
        // @pyvirtual int|PyCWnd|OnSetCursor|Called for the WM_SETCURSOR message.
        // @xref <om PyCWnd.OnSetCursor>
        CVirtualHelper helper("OnSetCursor", this);
        BOOL ret;
        // @pyparm <o PyCWnd>|wnd||
        // @pyparm int|hitTest||
        // @pyparm int|msg||
        if (helper.HaveHandler() && helper.call(pWnd, ht, msg))
            helper.retval(ret);
        else {
            helper.release_full();
            ret = T::OnSetCursor(pWnd, ht, msg);
        }
        return ret;
    }
    afx_msg void OnMDIActivate(BOOL bActivate, CWnd *pAc, CWnd *pDe)
    {
        // @pyvirtual int|PyCWnd|OnMDIActivate|
        // @comm The MFC implementation is always called before this.
        // @pyparm int|bActivate||
        // @pyparm <o PyCWnd>|wndActivate||
        // @pyparm <o PyCWnd>|wndDeactivate||
        T::OnMDIActivate(bActivate, pAc, pDe);
        CVirtualHelper helper("OnMDIActivate", this);
        if (helper.HaveHandler()) {
            PyObject *oba, *obd;
            if (pAc) {
                oba = PyWinObject_FromCWnd(pAc);
            }
            else {
                oba = Py_None;
                Py_INCREF(Py_None);
            }
            if (pDe) {
                obd = PyWinObject_FromCWnd(pDe);
            }
            else {
                obd = Py_None;
                Py_INCREF(Py_None);
            }
            helper.call_args("(iNN)", bActivate, oba, obd);
        }
    }
    afx_msg int OnMouseActivate(CWnd *pDesktopWnd, UINT nHitTest, UINT message)
    {
        // @pyvirtual int|PyCWnd|OnMouseActivate|Called when the mouse is used to activate a window.
        // @xref <om PyCWnd.OnMouseActivate>
        CVirtualHelper helper("OnMouseActivate", this);
        BOOL ret;
        // @pyparm <o PyCWnd>|wndDesktop||
        // @pyparm int|hitTest||
        // @pyparm int|msg||
        if (helper.HaveHandler() && helper.call(pDesktopWnd, nHitTest, message))
            helper.retval(ret);
        else {
            helper.release_full();
            ret = _BaseOnMouseActivate(pDesktopWnd, nHitTest, message);
        }
        return ret;
    }
    int _BaseOnMouseActivate(CWnd *pDesktopWnd, UINT nHitTest, UINT message)
    {
        return T::OnMouseActivate(pDesktopWnd, nHitTest, message);
    }

    //	DECLARE_MESSAGE_MAP()
   private:
    static const AFX_MSGMAP_ENTRY _messageEntries[];

   protected:
    static AFX_DATA const AFX_MSGMAP messageMap;
    static const AFX_MSGMAP *PASCAL _GetBaseMessageMap() { return T::GetThisMessageMap(); }
    virtual const AFX_MSGMAP *GetMessageMap() const { return &messageMap; }
};

template <class T>
AFX_DATADEF const AFX_MSGMAP CPythonWndFramework<T>::messageMap = {&CPythonWndFramework<T>::_GetBaseMessageMap,
                                                                   &CPythonWndFramework<T>::_messageEntries[0]};

// ack - compile error on not x64, and only for ON_WM_NCHITTEST!
#if !defined(_WIN64)
#undef ON_WM_NCHITTEST
// from afxmsg_.h - the UINT was originally LRESULT
#define ON_WM_NCHITTEST() \
    {WM_NCHITTEST,        \
     0,                   \
     0,                   \
     0,                   \
     AfxSig_l_p,          \
     (AFX_PMSG)(AFX_PMSGW)(static_cast<UINT (AFX_MSG_CALL CWnd::*)(CPoint)>(&ThisClass ::OnNcHitTest))},

#endif

#define ThisClass CPythonWndFramework<T>
template <class T>
const AFX_MSGMAP_ENTRY CPythonWndFramework<T>::_messageEntries[] = {
    ON_WM_PALETTECHANGED() ON_WM_PALETTEISCHANGING() ON_WM_WININICHANGE() ON_WM_CTLCOLOR() ON_WM_SYSCOLORCHANGE()
        ON_WM_ERASEBKGND() ON_WM_QUERYNEWPALETTE() ON_WM_PAINT() ON_WM_TIMER() ON_WM_QUERYDRAGICON() ON_WM_CREATE()
            ON_WM_CLOSE() ON_WM_NCCALCSIZE() ON_WM_NCHITTEST() ON_WM_SETCURSOR() ON_WM_MDIACTIVATE()
                ON_WM_MOUSEACTIVATE(){0, 0, 0, 0, AfxSig_end, (AFX_PMSG)0}};
#undef ThisClass

template <class T>
class CPythonDlgFramework : public CPythonWndFramework<T> {
   public:
    // ctor hacks!
    CPythonDlgFramework() : CPythonWndFramework<T>() { ; }
    // CPropertyPage
    CPythonDlgFramework(UINT id, UINT caption) : CPythonWndFramework<T>(id, caption) { ; }
    CPythonDlgFramework(LPCTSTR id, UINT caption) : CPythonWndFramework<T>(id, caption) { ; }
    // CPrintDialog
    CPythonDlgFramework(BOOL bPrintSetupOnly, DWORD dwFlags, CWnd *pParentWnd)
        : CPythonWndFramework<T>(bPrintSetupOnly, dwFlags, pParentWnd)
    {
        ;
    }
    // End of ctor hacks!!!!

    virtual BOOL OnInitDialog()
    {
        // @pyvirtual int|PyCDialog|OnInitDialog|Override to augment dialog-box initialization.
        // @comm The base implementation is not called if a handler exists.  This can be
        // done via <om PyCDialog.OnInitDialog>.
        // @xref <om PyCDialog.OnInitDialog>
        BOOL result = FALSE;
        CVirtualHelper helper("OnInitDialog", this);
        if (!helper.HaveHandler()) {
            result = T::OnInitDialog();
        }
        else {
            if (helper.call())
                helper.retval(result);
        }
        return result;
        // @rdesc Specifies whether the application has set the input focus to
        // one of the controls in the dialog box. If OnInitDialog returns
        // nonzero, Windows sets the input focus to the first control
        // in the dialog box. The application can return 0/None only if
        // it has explicitly set the input focus to one of the controls in the
        // dialog box.
    }
    virtual void OnOK()
    {
        // @pyvirtual int|PyCDialog|OnOK|Called by the MFC architecture when the user selects the OK button.
        // @comm The procedure is expected to dismiss the window with the <om PyCDialog.EndDialog> method.
        // The base implementation (which dismisses the dialog) is not called if a handler exists.  This can be
        // done via <om PyCDialog.OnOK>.
        // @xref <om PyCDialog.OnOK>
        CVirtualHelper helper("OnOK", this);
        if (!helper.HaveHandler()) {
            helper.release_full();
            if (m_lpDialogTemplate == NULL && m_hDialogTemplate == NULL)
                // non modal dialog.
                DestroyWindow();
            else
                T::OnOK();
        }
        else {
            helper.call();
        }
    }
    virtual void OnCancel()
    {
        // @pyvirtual int|PyCDialog|OnCancel|Called by the MFC architecture when the user selects the Cancel button.
        // @comm The procedure is expected to dismiss the window with the <om PyCDialog.EndDialog> method.
        // The base implementation (which dismisses the dialog) is not called if a handler exists.  This can be
        // done via <om PyCDialog.OnCancel>.
        // @xref <om PyCDialog.OnCancel>
        CVirtualHelper helper("OnCancel", this);
        BOOL bCallDefault;
        if (!helper.HaveHandler()) {
            bCallDefault = TRUE;
        }
        else {
            bCallDefault = !helper.call();  // DO call default on exception, else dialog may never come down!
        }
        if (bCallDefault) {
            helper.release_full();
            DoOnCancel();
        }
    }
    void DoOnCancel()
    {
        if (m_lpDialogTemplate == NULL && m_hDialogTemplate == NULL)
            DestroyWindow();
        else
            T::OnCancel();
    }
    virtual void DoDataExchange(CDataExchange *pDX)
    {  // DDX/DDV support
        T::DoDataExchange(pDX);
        Python_do_exchange(this, pDX);
    }
};

template <class T>
class CPythonPropertySheetFramework : public CPythonWndFramework<T> {
   public:
    CPythonPropertySheetFramework(UINT nIDCaption, CWnd *pParentWnd = NULL, UINT iSelectPage = 0)
        : CPythonWndFramework<T>(nIDCaption, pParentWnd, iSelectPage)
    {
        ;
    }
    CPythonPropertySheetFramework(LPCTSTR pszCaption, CWnd *pParentWnd = NULL, UINT iSelectPage = 0)
        : CPythonWndFramework<T>(pszCaption, pParentWnd, iSelectPage)
    {
        ;
    }

    virtual BOOL OnInitDialog()
    {
        // @pyvirtual int|PyCPropertySheet|OnInitDialog|Override to augment dialog-box initialization.
        // @comm The base implementation is not called if a handler exists.  This can be
        // done via <om PyCPropertySheet.OnInitDialog>.
        // @xref <om PyCPropertySheet.OnInitDialog>
        BOOL result = FALSE;
        CVirtualHelper helper("OnInitDialog", this);
        if (!helper.HaveHandler()) {
            helper.release_full();
            result = T::OnInitDialog();
        }
        else {
            if (helper.call())
                helper.retval(result);
        }
        return result;
        // @rdesc Specifies whether the application has set the input focus to
        // one of the controls in the dialog box. If OnInitDialog returns
        // nonzero, Windows sets the input focus to the first control
        // in the dialog box. The application can return 0/None only if
        // it has explicitly set the input focus to one of the controls in the
        // dialog box.
    }
};

// Property Pages.
template <class T>
class CPythonPropertyPageFramework : public CPythonDlgFramework<T> {
   public:
    CPythonPropertyPageFramework(UINT id, UINT caption) : CPythonDlgFramework<T>(id, caption) { ; }
    CPythonPropertyPageFramework(LPCTSTR id, UINT caption) : CPythonDlgFramework<T>(id, caption) { ; }

    virtual BOOL OnKillActive()
    {
        // @pyvirtual int|PyCPropertyPage|OnKillActive|Called when the page loses focus.
        // @rdesc The method should return TRUE if the page can be de-activated.
        // @comm Note - If you provide a handler, you must call the underlying MFC method (<om
        // PyCPropertyPage.OnKillActive>) yourself
        // @xref <om PyCPropertyPage.OnKillActive>
        CVirtualHelper helper("OnKillActive", this);
        if (!helper.HaveHandler()) {
            helper.release_full();
            return T::OnKillActive();
        }
        BOOL bOK = TRUE;
        if (helper.call())
            helper.retval(bOK);
        return bOK;
    }

    virtual BOOL OnSetActive()
    {
        // @pyvirtual int|PyCPropertyPage|OnSetActive|Called when the page becomes active.
        // @rdesc The method should return TRUE if the page can be activated.
        // @comm Note - If you provide a handler, you must call the underlying MFC method (<om
        // PyCPropertyPage.OnSetActive>) yourself
        // @xref <om PyCPropertyPage.OnSetActive>
        CVirtualHelper helper("OnSetActive", this);
        if (!helper.HaveHandler()) {
            helper.release_full();
            return T::OnSetActive();
        }
        BOOL bOK = TRUE;
        if (helper.call())
            helper.retval(bOK);
        return bOK;
    }

    virtual BOOL OnApply()
    {
        // @pyvirtual int|PyCPropertyPage|OnApply|Called by the framework when the user chooses the OK or the Apply Now
        // button.
        // @rdesc Return Nonzero if the changes are accepted; otherwise 0.
        // @comm Note - If you provide a handler, you must call the underlying MFC method (<om PyCPropertyPage.OnApply>)
        // yourself
        // @xref <om PyCPropertyPage.OnApply>
        CVirtualHelper helper("OnApply", this);
        if (!helper.HaveHandler()) {
            helper.release_full();
            return T::OnApply();
        }
        BOOL bOK = TRUE;
        if (helper.call())
            helper.retval(bOK);
        return bOK;
    }

    virtual void OnReset()
    {
        // @pyvirtual void|PyCPropertyPage|OnReset|Called by the framework when the user chooses the Cancel button.
        // @comm Note - If you provide a handler, you must call the underlying MFC method (<om PyCPropertyPage.OnReset>)
        // yourself
        // @xref <om PyCPropertyPage.OnReset>
        CVirtualHelper helper("OnReset", this);
        if (!helper.HaveHandler()) {
            helper.release_full();
            T::OnReset();
        }
        else {
            helper.call();
        }
    }

    virtual BOOL OnQueryCancel()
    {
        // @pyvirtual int|PyCPropertyPage|OnQueryCancel|Called by the framework when the user clicks the Cancel button
        // and before the cancel action has taken place.
        // @rdesc Return FALSE to prevent the cancel operation or TRUE to allow it.
        // @comm Note - If you provide a handler, you must call the underlying MFC method (<om
        // PyCPropertyPage.OnQueryCancel>) yourself
        // @xref <om PyCPropertyPage.OnQueryCancel>
        CVirtualHelper helper("OnQueryCancel", this);
        if (!helper.HaveHandler()) {
            helper.release_full();
            return T::OnQueryCancel();
        }
        BOOL bOK = TRUE;
        if (helper.call())
            helper.retval(bOK);
        return bOK;
    }

    virtual LRESULT OnWizardBack()
    {
        // @pyvirtual int|PyCPropertyPage|OnWizardBack|Called by the framework when the user clicks on the Back button
        // in a wizard.
        // @rdesc Return 0 to automatically advance to the next page;  -1 to prevent the page from changing. To jump to
        // a page other than the next one, return the identifier of the dialog to be displayed.
        // @comm Note - If you provide a handler, you must call the underlying MFC method (<om
        // PyCPropertyPage.OnWizardBack>) yourself
        // @xref <om PyCPropertyPage.OnWizardBack>
        CVirtualHelper helper("OnWizardBack", this);
        if (!helper.HaveHandler()) {
            return T::OnWizardBack();
        }
        long result = TRUE;
        if (helper.call())
            helper.retval(result);
        return result;
    }

    virtual LRESULT OnWizardNext()
    {
        // @pyvirtual int|PyCPropertyPage|OnWizardNext|Called by the framework when the user clicks on the Next button
        // in a wizard.
        // @rdesc Return 0 to automatically advance to the next page;  -1 to prevent the page from changing. To jump to
        // a page other than the next one, return the identifier of the dialog to be displayed.
        // @comm Note - If you provide a handler, you must call the underlying MFC method (<om
        // PyCPropertyPage.OnWizardNext>) yourself
        // @xref <om PyCPropertyPage.OnWizardNext>
        CVirtualHelper helper("OnWizardNext", this);
        if (!helper.HaveHandler()) {
            helper.release_full();
            return T::OnWizardNext();
        }
        long result = TRUE;
        if (helper.call())
            helper.retval(result);
        return result;
    }

    virtual BOOL OnWizardFinish()
    {
        // @pyvirtual int|PyCPropertyPage|OnWizardFinish|Called by the framework when the user clicks on the Finish
        // button in a wizard.
        // @rdesc Return nonzero if the property sheet is destroyed when the wizard finishes; otherwise zero.
        // @comm Note - If you provide a handler, you must call the underlying MFC method (<om
        // PyCPropertyPage.OnWizardFinish>) yourself
        // @xref <om PyCPropertyPage.OnWizardFinish>
        CVirtualHelper helper("OnWizardFinish", this);
        if (!helper.HaveHandler()) {
            helper.release_full();
            return T::OnWizardFinish();
        }
        BOOL bOK = TRUE;
        if (helper.call())
            helper.retval(bOK);
        return bOK;
    }
};

// Views.
template <class T>
class CPythonViewFramework : public CPythonWndFramework<T> {
   public:
    // ctor hacks
    CPythonViewFramework() : CPythonWndFramework<T>() { ; }
    // CFormView ctors.
    CPythonViewFramework(UINT id) : CPythonWndFramework<T>(id) { ; }
    CPythonViewFramework(LPCTSTR id) : CPythonWndFramework<T>(id) { ; }
    // CCtrlView ctor
    CPythonViewFramework(LPCTSTR lpszClass, DWORD dwStyle) : CPythonWndFramework<T>(lpszClass, dwStyle) { ; }
    virtual BOOL OnPreparePrinting(CPrintInfo *pInfo)
    {
        // @pyvirtual |PyCView|OnPreparePrinting|Called by the framework before a document is printed or previewed
        // @xref <om PyCView.OnPreparePrinting>
        CVirtualHelper helper("OnPreparePrinting", this);
        BOOL result;
        if (!helper.HaveHandler()) {
            helper.release_full();
            result = T::OnPreparePrinting(pInfo);
        }
        else {
            helper.call(pInfo);
            helper.retval(result);
        }
        return result;
        // @pyparm <o PyCPrintInfo>|pInfo||The print info object.
    }
    virtual void OnBeginPrinting(CDC *pDC, CPrintInfo *pInfo)
    {
        // @pyvirtual |PyCView|OnBeginPrinting|Called by the framework at the beginning of a print or print preview job,
        // after OnPreparePrinting has been called.
        // @xref <om PyCView.OnBeginPrinting>
        CVirtualHelper helper("OnBeginPrinting", this);
        if (!helper.HaveHandler()) {
            helper.release_full();
            T::OnBeginPrinting(pDC, pInfo);
            return;
        }
        helper.call(pDC, pInfo);
        // @pyparm <o PyCDC>|dc||The DC object.
        // @pyparm <o PyCPrintInfo>|pInfo||The print info object.
    }
    virtual void OnEndPrinting(CDC *pDC, CPrintInfo *pInfo)
    {
        // @pyvirtual |PyCView|OnEndPrinting|Called by the framework after a document has been printed or previewed.
        // @xref <om PyCView.OnEndPrinting>
        CVirtualHelper helper("OnEndPrinting", this);
        if (!helper.HaveHandler()) {
            helper.release_full();
            T::OnEndPrinting(pDC, pInfo);
            return;
        }
        helper.call(pDC, pInfo);
        // @pyparm <o PyCDC>|dc||The DC object.
        // @pyparm <o PyCPrintInfo>|pInfo||The print info object.
    }
    virtual void OnPrepareDC(CDC *pDC, CPrintInfo *pInfo)
    {
        // @pyvirtual |PyCView|OnPrepareDC|Called to prepare the device context for a view.
        // @xref <om PyCWnd.OnPrepareDC>
        CVirtualHelper helper("OnPrepareDC", this);
        if (!helper.HaveHandler()) {
            helper.release_full();
            T::OnPrepareDC(pDC, pInfo);
            return;
        }
        helper.call(pDC, pInfo);
        // @pyparm <o PyCDC>|dc||The DC object.
        // @pyparm <o PyCPrintInfo>|pInfo||The print info object.
    }
    virtual void OnInitialUpdate()
    {
        // @pyvirtual tuple|PyCView|OnInitialUpdate|Called before the first update for a view.
        // @comm The MFC base class is called only if no handler exists.
        // @xref <om PyCView.OnInitialUpdate>
        CVirtualHelper helper("OnInitialUpdate", this);
        if (helper.HaveHandler())
            helper.call();
        else {
            helper.release_full();
            T::OnInitialUpdate();
        }
    }
    virtual void OnActivateView(BOOL bActivate, CView *pActivateView, CView *pDeactiveView)
    {
        // @pyvirtual |PyCView|OnActivateView|Called by the framework when a view is activated or deactivated.
        // @xref <om PyCView.OnActivateView>
        CVirtualHelper helper("OnActivateView", this);
        if (!helper.HaveHandler()) {
            helper.release_full();
            T::OnActivateView(bActivate, pActivateView, pDeactiveView);
        }
        else {
            // @pyparm int|bActivate||Indicates whether the view is being activated or deactivated.
            // @pyparm <o PyCWnd>|activateView||The view object that is being activated.
            // @pyparm <o PyCWnd>|DeactivateView||The view object that is being deactivated.
            helper.call(bActivate, pActivateView, pDeactiveView);

            // @comm If a handler exists, the base MFC implementation is not called.
            // <nl>The activateView and deactiveView parameters are the same objects if the
            // application's main frame window is activated with no change in the
            // active view for example, if the focus is being transferred from
            // another application to this one, rather than from one view to
            // another within the application.
            // This allows a view to re-realize its palette, if needed.
        }
    }
    virtual void OnDraw(CDC *pDC)
    {
        // @pyvirtual |PyCView|OnDraw|Called when the view should be drawn.
        CVirtualHelper helper("OnDraw", this);
        // @pyparm <o PyCDC>|dc||The DC object.
        // @xref <om PyCView.OnDraw>
        helper.call(pDC);
    }
    virtual void OnPrint(CDC *pDC, CPrintInfo *pInfo)
    {
        // @pyvirtual |PyCView|OnPrint|Called when the view should be printed.
        // @xref <om PyCView.OnPrint>
        CVirtualHelper helper("OnPrint", this);
        // @pyparm <o PyCDC>|dc||The DC object.
        // @pyparm <o PyPrintInfo>|pInfo||The PrintIfo object.
        helper.call(pDC, pInfo);
    }
    virtual void OnUpdate(CView *pSender, LPARAM lHint, CObject *pHint)
    {
        // @pyvirtual |PyCView|OnUpdate|Called by the framework when a view needs updating.
        // @comm Typically you should not perform any drawing directly from OnUpdate.
        // Instead, determine the rectangle describing, in device coordinates, the
        // area that requires updating; pass this rectangle to <om PyCWnd.InvalidateRect>.
        // You can then paint the update next <om PyCView.OnDraw>
        // @xref <om PyCView.OnUpdate>
        CVirtualHelper helper("OnUpdate", this);
        if (!helper.HaveHandler()) {
            // @pyparm <o PyCView>|sender||
            // @pyparm int|lHint||
            // @pyparm object|hint||
            helper.release_full();
            T::OnUpdate(pSender, lHint, pHint);
        }
        else
            helper.call(pSender, (PyObject *)lHint);
    }
};

template <class T>
class CPythonFrameFramework : public CPythonWndFramework<T> {
   public:
   protected:
    // Operations
    virtual BOOL OnCreateClient(LPCREATESTRUCT cs, CCreateContext *pContext)
    {
        // @pyvirtual |PyCMDIChildWnd|OnCreateClient|Called by the framework during the execution of OnCreate.

        CVirtualHelper helper("OnCreateClient", this);
        PythonCreateContext *pCC = (PythonCreateContext *)pContext;
        // @pyparm tuple|CREATESTRUCT||A tuple describing a CREATESTRUCT structure.
        // @pyparm object|object||A Python object initially passed to LoadFrame
        if (helper.HaveHandler() && !helper.call(cs, pCC ? pCC->GetPythonObject() : NULL))
            return FALSE;
        // @rdesc The return value from this method is ignored, but an exception will prevent window creation.
        helper.release_full();
        return T::OnCreateClient(cs, pContext);
    }
    virtual void GetMessageString(UINT nID, CString &rMessage) const
    {
        // @pyvirtual string|PyCMDIChildWnd|GetMessageString|Gets the message string to use for a control specific ID.
        CVirtualHelper helper("GetMessageString", (void *)this);
        // @pyparm int|id||The command ID to retrieve the string for.
        // @xref <om PyCMDIChildWnd.GetMessageString>
        if (helper.call((int &)nID)) {
            helper.retval(rMessage);
        }
        else {
            helper.release_full();
            T::GetMessageString(nID, rMessage);
        }
    }
    virtual void ActivateFrame(int nCmdShow)
    {
        // @pyvirtual |PyCMDIChildWnd|ActivateFrame|Called to activate the frame window.
        // @comm If a handler for this function exists, then the base MFC implementation will not be called.
        // If you wish to use the default functionality, <om PyCMDIFrameWnd.ActivateFrame> can be called.
        // <nl>If there is no handler, the base MFC implementation will be called.
        // @xref <om PyCMDIChildWnd.ActivateFrame>
        CVirtualHelper helper("ActivateFrame", this);
        // @pyparm int|cmdShow||The parameter to be passed to <om PyCWnd.ShowWindow>
        if (helper.HaveHandler()) {
            helper.call(nCmdShow);
        }
        else {
            helper.release_full();
            T::ActivateFrame(nCmdShow);
        }
    }
};

#define IDC_PRINT_TO_FILE 1040
#define IDC_PRINT_COLLATE 1041
#define IDC_PRINT_COPIES 1154
#define IDC_PRINT_RANGE_ALL 1056
#define IDC_PRINT_RANGE_SELECTION 1057
#define IDC_PRINT_RANGE_PAGES 1058
#define IDC_PRINT_COPIES_LABEL 1092
#define IDC_PRINT_FROM 1152
#define IDC_PRINT_TO 1153

template <class T>
class CPythonPrtDlgFramework : public CPythonDlgFramework<T> {
   public:
    // ctor hacks!
    CPythonPrtDlgFramework(BOOL bPrintSetupOnly, DWORD dwFlags, CWnd *pParentWnd)
        : CPythonDlgFramework<T>(bPrintSetupOnly, dwFlags, pParentWnd)
    {
        ;
    }
    // End of ctor hacks!!!!

    afx_msg LRESULT HandleInitDialog(WPARAM, LPARAM)
    {
        PreInitDialog();
        LRESULT result = Default();
        CVirtualHelper helper("HandleInitDialog", this);
        BOOL hresult;
        if (helper.HaveHandler()) {
            if (helper.call())
                helper.retval(hresult);
        }
        return result;
    }

    virtual void OnOK()
    {
        // @pyvirtual int|PyCPrintDialog|OnOK|Called by the MFC architecture when the user selects the OK button.
        // @comm The procedure is expected to dismiss the window with the <om PyCPrintDialog.EndDialog> method.
        // The base implementation (which dismisses the dialog) is not called if a handler exists.  This can be
        // done via <om PyCPrintDialog.OnOK>.
        // @xref <om PyCDialogDialog.OnOK>

        CString csText;

        CWnd *ctl = GetDlgItem(IDC_PRINT_FROM);
        ctl->GetWindowText(csText);
        m_pd.nFromPage = _ttoi(csText);

        ctl = GetDlgItem(IDC_PRINT_TO);
        ctl->GetWindowText(csText);
        m_pd.nToPage = _ttoi(csText);

        ctl = GetDlgItem(IDC_PRINT_COPIES);
        ctl->GetWindowText(csText);
        if (m_pd.Flags & PD_USEDEVMODECOPIES) {
            LPDEVMODE devMode = (LPDEVMODE)::GlobalLock((HGLOBAL)m_pd.hDevMode);
            devMode->dmCopies = _ttoi(csText);
            ::GlobalUnlock((HGLOBAL)m_pd.hDevMode);
        }
        m_pd.nCopies = _ttoi(csText);

        CVirtualHelper helper("OnOK", this);
        if (helper.HaveHandler()) {
            helper.call();
        }
    }

    virtual void OnCancel()
    {
        // @pyvirtual int|PyCPrintDialog|OnCancel|Called by the MFC architecture when the user selects the Cancel
        // button.
        // @comm The procedure is expected to dismiss the window with the <om PyCPrintDialog.EndDialog> method.
        // The base implementation (which dismisses the dialog) is not called if a handler exists.  This can be
        // done via <om PyCPrintDialog.OnCancel>.
        // @xref <om PyCDialog.OnCancel>
        CVirtualHelper helper("OnCancel", this);
        if (helper.HaveHandler()) {
            helper.call();
        }
        helper.release_full();
        T::OnCancel();
    }

    afx_msg void HandlePrintToFile()
    {
        CWnd *ctl = GetDlgItem(IDC_PRINT_TO_FILE);
        int val = ((CButton *)ctl)->GetCheck();
        if (val) {  // was checked, now will not be...
            m_pd.Flags |= PD_PRINTTOFILE;
        }
        else {  // was not checked, now will be...
            m_pd.Flags &= ~PD_PRINTTOFILE;
        }
    }

    afx_msg void HandleCollate()
    {
        CWnd *ctl = GetDlgItem(IDC_PRINT_COLLATE);
        int val = ((CButton *)ctl)->GetCheck();
        int useDevMode = (m_pd.Flags & PD_USEDEVMODECOPIES || m_pd.Flags & PD_USEDEVMODECOPIESANDCOLLATE);
        LPDEVMODE devMode;
        if (useDevMode) {
            devMode = (LPDEVMODE)::GlobalLock((HGLOBAL)m_pd.hDevMode);
        }
        if (val) {  // was checked, now will not be...
            m_pd.Flags |= PD_COLLATE;
        }
        else {  // was not checked, now will be...
            m_pd.Flags &= ~PD_COLLATE;
        }
        if (useDevMode) {
            devMode->dmCollate = DMCOLLATE_TRUE;
            ::GlobalUnlock((HGLOBAL)m_pd.hDevMode);
        }
    }

    afx_msg void HandlePrintRange(UINT nID)
    {
        CheckRadioButton(IDC_PRINT_RANGE_ALL, IDC_PRINT_RANGE_PAGES, nID);
        m_pd.Flags &= ~(PD_ALLPAGES | PD_SELECTION | PD_PAGENUMS);
        if (nID == IDC_PRINT_RANGE_ALL) {
            m_pd.Flags |= PD_ALLPAGES;
        }
        else if (nID == IDC_PRINT_RANGE_SELECTION) {
            m_pd.Flags |= PD_SELECTION;
        }
        else if (nID == IDC_PRINT_RANGE_PAGES) {
            m_pd.Flags |= PD_PAGENUMS;
        }
    }

    //	DECLARE_MESSAGE_MAP()
   private:
    static const AFX_MSGMAP_ENTRY _messageEntries[];

   protected:
    static AFX_DATA const AFX_MSGMAP messageMap;

    static const AFX_MSGMAP *PASCAL _GetBaseMessageMap() { return T::GetThisMessageMap(); }
    virtual const AFX_MSGMAP *GetMessageMap() const { return &messageMap; }
};

template <class T>
AFX_DATADEF const AFX_MSGMAP CPythonPrtDlgFramework<T>::messageMap = {&CPythonPrtDlgFramework<T>::_GetBaseMessageMap,
                                                                      &CPythonPrtDlgFramework<T>::_messageEntries[0]};

template <class T>
const AFX_MSGMAP_ENTRY CPythonPrtDlgFramework<T>::_messageEntries[] = {
    ON_MESSAGE(WM_INITDIALOG, &CPythonPrtDlgFramework<T>::HandleInitDialog)
        ON_COMMAND(IDC_PRINT_TO_FILE, &CPythonPrtDlgFramework<T>::HandlePrintToFile)
            ON_COMMAND(IDC_PRINT_COLLATE, &CPythonPrtDlgFramework<T>::HandleCollate)
                ON_COMMAND_RANGE(IDC_PRINT_RANGE_ALL, IDC_PRINT_RANGE_PAGES,
                                 &CPythonPrtDlgFramework<T>::HandlePrintRange){0, 0, 0, 0, AfxSig_end, (AFX_PMSG)0}};

#undef IDC_PRINT_TO_FILE
#undef IDC_PRINT_COLLATE
#undef IDC_PRINT_COPIES
#undef IDC_PRINT_RANGE_ALL
#undef IDC_PRINT_RANGE_SELECTION
#undef IDC_PRINT_RANGE_PAGES
#undef IDC_PRINT_COPIES_LABEL
#undef IDC_PRINT_FROM
#undef IDC_PRINT_TO

#endif  // __WIN32UIEXT_H__
