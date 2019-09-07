#ifndef __PYIPERSIST_H__
#define __PYIPERSIST_H__

#include "PythonCOM.h"
#include "PythonCOMServer.h"

class PYCOM_EXPORT PyIPersist : public PyIUnknown {
   public:
    MAKE_PYCOM_CTOR(PyIPersist);
    static PyComTypeObject type;
    static IPersist *GetI(PyObject *self);

    static PyObject *GetClassID(PyObject *self, PyObject *args);

   protected:
    PyIPersist(IUnknown *);
    ~PyIPersist();
};

// Disable an OK warning...
#pragma warning(disable : 4275)
// warning C4275: non dll-interface struct 'IPersist' used as base for dll-interface class 'GPersist'

class PYCOM_EXPORT PyGPersist : public PyGatewayBase, public IPersist {
   protected:
    PyGPersist(PyObject *instance) : PyGatewayBase(instance) { ; }
    PYGATEWAY_MAKE_SUPPORT(PyGPersist, IPersist, IID_IPersist)

    // IPersist
    STDMETHOD(GetClassID)
    (
        /* [out] */ CLSID __RPC_FAR *pClassID);
};

#pragma warning(default : 4275)

#endif  // __PYIPERSIST_H__
