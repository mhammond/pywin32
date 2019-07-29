// This file implements the IDockingWindow Interface and Gateway for Python.

#include "shell_pch.h"
#include "PyIOleWindow.h"
#include "PyIDockingWindow.h"

// @doc - This file contains autoduck documentation
// ---------------------------------------------------
//
// Gateway Implementation

STDMETHODIMP PyGDockingWindow::GetWindow(HWND __RPC_FAR *phwnd) { return PyGOleWindow::GetWindow(phwnd); }
STDMETHODIMP PyGDockingWindow::ContextSensitiveHelp(BOOL fEnterMode)
{
    return PyGOleWindow::ContextSensitiveHelp(fEnterMode);
}

STDMETHODIMP PyGDockingWindow::ShowDW(BOOL fShow)
{
    PY_GATEWAY_METHOD;
    return InvokeViaPolicy("ShowDW", NULL, "i", fShow);
}

STDMETHODIMP PyGDockingWindow::CloseDW(DWORD dwReserved)
{
    PY_GATEWAY_METHOD;
    return InvokeViaPolicy("CloseDW", NULL, "i", dwReserved);
}

STDMETHODIMP PyGDockingWindow::ResizeBorderDW(
    /* [in] */ LPCRECT prcBorder,
    /* [in] */ IUnknown *punkToolbarSite,
    /* [in] */ BOOL fReserved)
{
    PY_GATEWAY_METHOD;
    PyObject *obSite = PyCom_PyObjectFromIUnknown(punkToolbarSite, IID_IUnknown, TRUE);
    if (!obSite)
        return MAKE_PYCOM_GATEWAY_FAILURE_CODE("ResizeBorderDW");
    return InvokeViaPolicy("ResizeWindowDW", NULL, "(iiii)Ni", prcBorder->left, prcBorder->top, prcBorder->right,
                           prcBorder->bottom, obSite, fReserved);
}
