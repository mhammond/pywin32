// This file declares the IMAPIAdviseSink Gateway for Python.
// ----------------------------------------------------------
//
// Gateway Declaration

class PyGMAPIAdviseSink : public PyGatewayBase, public IMAPIAdviseSink
{
protected:
	PyGMAPIAdviseSink(PyObject *instance) : PyGatewayBase(instance) { ; }
	PYGATEWAY_MAKE_SUPPORT2(PyGMAPIAdviseSink, IMAPIAdviseSink, IID_IMAPIAdviseSink, PyGatewayBase)

	// IMAPIAdviseSink
        MAPIMETHOD_(ULONG, OnNotify)(ULONG cNotif,  LPNOTIFICATION lpNotifications);
};
