// This file declares the IDeskBand Gateway for Python.
// ---------------------------------------------------
// ---------------------------------------------------
//
// Gateway Declaration

class PyGDeskBand: public PyGDockingWindow, public IDeskBand
{
protected:
	PyGDeskBand(PyObject *instance) : PyGDockingWindow(instance) { ; }
	PYGATEWAY_MAKE_SUPPORT2(PyGDeskBand, IDeskBand, IID_IDeskBand, PyGDockingWindow)

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

	// IDeskBand
	STDMETHOD(GetBandInfo)(
		DWORD dwBandID, 
		DWORD dwViewMode, 
		DESKBANDINFO* pdbi);
};
