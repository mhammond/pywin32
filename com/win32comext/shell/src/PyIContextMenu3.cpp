// This file implements the IContextMenu Interface and Gateway for Python.
// Generated by makegw.py

#include "shell_pch.h"
#include "PyIContextMenu3.h"

// @doc - This file contains autoduck documentation
// ---------------------------------------------------
//
// Gateway Implementation
STDMETHODIMP PyGContextMenu3::QueryContextMenu(HMENU hmenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast,
                                               UINT uFlags)
{
    return PyGContextMenu2::QueryContextMenu(hmenu, indexMenu, idCmdFirst, idCmdLast, uFlags);
}

STDMETHODIMP PyGContextMenu3::InvokeCommand(CMINVOKECOMMANDINFO *lpici)
{
    return PyGContextMenu2::InvokeCommand(lpici);
}

STDMETHODIMP PyGContextMenu3::GetCommandString(UINT_PTR idCmd, UINT uType, UINT __RPC_FAR *pwReserved, LPSTR pszName,
                                               UINT cchMax)
{
    return PyGContextMenu2::GetCommandString(idCmd, uType, pwReserved, pszName, cchMax);
}

STDMETHODIMP PyGContextMenu3::HandleMenuMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return PyGContextMenu2::HandleMenuMsg(uMsg, wParam, lParam);
}

STDMETHODIMP PyGContextMenu3::HandleMenuMsg2(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *lpResult)
{
    PY_GATEWAY_METHOD;
    PyObject *ret;
    HRESULT hr = InvokeViaPolicy("HandleMenuMsg2", &ret, "INN", uMsg, PyWinObject_FromPARAM(wParam),
                                 PyWinObject_FromPARAM(lParam));
    if (FAILED(hr))
        return hr;
    if (lpResult) {
        if (ret == Py_None)
            *lpResult = FALSE;
        else {
            PyWinObject_AsSimplePARAM(ret, (WPARAM *)lpResult);
            hr = PyCom_SetAndLogCOMErrorFromPyException("HandleMenuMsg2", IID_IContextMenu3);
        }
    }
    Py_DECREF(ret);
    return hr;
}
