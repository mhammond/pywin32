#ifndef __PYIPERSISTSTREAM_H__
#define __PYIPERSISTSTREAM_H__

#include "PythonCOM.h"
#include "PythonCOMServer.h"

#include "PyIPersist.h"

class PYCOM_EXPORT PyIPersistStream : public PyIPersist {
   public:
    MAKE_PYCOM_CTOR(PyIPersistStream);
    static PyComTypeObject type;
    static IPersistStream *GetI(PyObject *self);

    static PyObject *IsDirty(PyObject *self, PyObject *args);
    static PyObject *Load(PyObject *self, PyObject *args);
    static PyObject *Save(PyObject *self, PyObject *args);
    static PyObject *GetSizeMax(PyObject *self, PyObject *args);

   protected:
    PyIPersistStream(IUnknown *);
    ~PyIPersistStream();
};

class PyGPersistStream : public PyGPersist, public IPersistStream {
   protected:
    PyGPersistStream(PyObject *instance) : PyGPersist(instance) { ; }
    PYGATEWAY_MAKE_SUPPORT2(PyGPersistStream, IPersistStream, IID_IPersistStream, PyGPersist)

    // IPersist
    STDMETHOD(GetClassID)(CLSID FAR *pClassID) { return PyGPersist::GetClassID(pClassID); }

    // IPersistStream
    STDMETHOD(IsDirty)(void);

    STDMETHOD(Load)
    (
        /* [unique][in] */ IStream __RPC_FAR *pStm);

    STDMETHOD(Save)
    (
        /* [unique][in] */ IStream __RPC_FAR *pStm,
        /* [in] */ BOOL fClearDirty);

    STDMETHOD(GetSizeMax)
    (
        /* [out] */ ULARGE_INTEGER __RPC_FAR *pcbSize);
};

#endif  // __PYIPERSISTSTREAM_H__
