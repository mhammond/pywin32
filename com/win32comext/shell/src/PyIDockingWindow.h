// This file declares the IDockingWindow Gateway for Python.
// ---------------------------------------------------------
//
// Gateway Declaration

class PyGDockingWindow: public PyGOleWindow, public IDockingWindow
{
protected:
	PyGDockingWindow(PyObject *instance) : PyGOleWindow(instance) { ; }
	PYGATEWAY_MAKE_SUPPORT2(PyGDockingWindow, IDockingWindow, IID_IDockingWindow, PyGOleWindow)

	// IOleWindow
	STDMETHOD(GetWindow)(
		HWND __RPC_FAR * phwnd);

	STDMETHOD(ContextSensitiveHelp)(
		BOOL fEnterMode);

	// IDockingWindow
	STDMETHOD(ShowDW)(
                BOOL fShow);
        STDMETHOD(CloseDW)(
                DWORD dwReserved);
        STDMETHOD(ResizeBorderDW)(
                    LPCRECT prcBorder,
                    IUnknown *punkToolbarSite,
                    BOOL fReserved);
};
