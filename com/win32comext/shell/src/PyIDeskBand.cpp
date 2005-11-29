// This file implements the IDeskBand Interface and Gateway for Python.

#include "shell_pch.h"
#include "PyIOleWindow.h"
#include "PyIDockingWindow.h"
#include "PyIDeskBand.h"

// @doc - This file contains autoduck documentation
// ---------------------------------------------------
//
// Gateway Implementation
// IOleWindow
STDMETHODIMP PyGDeskBand::GetWindow(HWND __RPC_FAR * phwnd) {return PyGDockingWindow::GetWindow(phwnd);}
STDMETHODIMP PyGDeskBand::ContextSensitiveHelp(BOOL fEnterMode) {return PyGDockingWindow::ContextSensitiveHelp(fEnterMode);}

// IDockingWindow
STDMETHODIMP PyGDeskBand::ShowDW(BOOL fShow) {
	return PyGDockingWindow::ShowDW(fShow);
}
STDMETHODIMP PyGDeskBand::CloseDW(DWORD dwReserved) {
	return PyGDockingWindow::CloseDW(dwReserved);
}
STDMETHODIMP PyGDeskBand::ResizeBorderDW(
                    LPCRECT prcBorder,
                    IUnknown *punkToolbarSite,
                    BOOL fReserved) {
	return PyGDockingWindow::ResizeBorderDW(prcBorder, punkToolbarSite,
						fReserved);
}

STDMETHODIMP PyGDeskBand::GetBandInfo(
	DWORD dwBandID, 
	DWORD dwViewMode, 
	DESKBANDINFO* pdbi)
{
	PY_GATEWAY_METHOD;
	PyObject *ret;
	HRESULT hr=InvokeViaPolicy("GetBandInfo", &ret, "ikk", dwBandID,
	                           dwViewMode, pdbi->dwMask);
	if (FAILED(hr)) return hr;
	// I'm slack here - all values must be returned from Python (eg, even
	// if the mask doesn't want a value, you must provide one (usually 0)
	PyObject *obtitle;
	if (!PyArg_ParseTuple(ret, "(ii)(ii)(ii)(ii)Oii",
	                      &pdbi->ptMinSize.x,
	                      &pdbi->ptMinSize.y,
	                      &pdbi->ptMaxSize.x,
	                      &pdbi->ptMaxSize.y,
	                      &pdbi->ptIntegral.x,
	                      &pdbi->ptIntegral.y,
	                      &pdbi->ptActual.x,
	                      &pdbi->ptActual.y,
	                      &obtitle,
	                      &pdbi->dwModeFlags,
	                      &pdbi->crBkgnd))
		hr = MAKE_PYCOM_GATEWAY_FAILURE_CODE("GetBandInfo");
	else {
		WCHAR *title = NULL;
		if (!PyWinObject_AsWCHAR(obtitle, &title)) {
			hr = MAKE_PYCOM_GATEWAY_FAILURE_CODE("GetBandInfo");
		} else {
			wcsncpy(pdbi->wszTitle, title,
			        sizeof(pdbi->wszTitle)/sizeof(pdbi->wszTitle[0]));
			PyWinObject_FreeWCHAR(title);
		}
	}
	Py_DECREF(ret);
	return hr;
}

