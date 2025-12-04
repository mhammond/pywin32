/////////////////////////////////////////////////////////////////////////////
// class PyIEnumGUID

#include "PythonCOM.h"
#include "PythonCOMServer.h"

class PyIEnumGUID : public PyIUnknown {
   public:
    MAKE_PYCOM_CTOR(PyIEnumGUID);
    static PyComEnumTypeObject type;
    static IEnumGUID *GetI(PyObject *self);

    static PyObject *Next(PyObject *self, PyObject *args);
    static PyObject *Skip(PyObject *self, PyObject *args);
    static PyObject *Reset(PyObject *self, PyObject *args);
    static PyObject *Clone(PyObject *self, PyObject *args);

   protected:
    PyIEnumGUID(IUnknown *);
    ~PyIEnumGUID();
};

// ---------------------------------------------------
//
// Gateway Declaration

class PyGEnumGUID : public PyGatewayBase, public IEnumGUID {
   protected:
    PyGEnumGUID(PyObject *instance) : PyGatewayBase(instance) { ; }
    PYGATEWAY_MAKE_SUPPORT(PyGEnumGUID, IEnumGUID, IID_IEnumGUID)

    // IEnumGUID
    STDMETHOD(Next)(ULONG celt, GUID __RPC_FAR *rgelt, ULONG __RPC_FAR *pceltFetched);

    STDMETHOD(Skip)(ULONG celt);

    STDMETHOD(Reset)(void);

    STDMETHOD(Clone)(IEnumGUID __RPC_FAR * __RPC_FAR *ppenum);
};

/////////////////////////////////////////////////////////////////////////////
// class PyIEnumCATEGORYINFO
class PyIEnumCATEGORYINFO : public PyIUnknown {
   public:
    MAKE_PYCOM_CTOR(PyIEnumCATEGORYINFO);
    static PyComEnumTypeObject type;
    static IEnumCATEGORYINFO *GetI(PyObject *self);

    static PyObject *Next(PyObject *self, PyObject *args);
    static PyObject *Skip(PyObject *self, PyObject *args);
    static PyObject *Reset(PyObject *self, PyObject *args);
    static PyObject *Clone(PyObject *self, PyObject *args);

   protected:
    PyIEnumCATEGORYINFO(IUnknown *);
    ~PyIEnumCATEGORYINFO();
};
