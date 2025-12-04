#ifndef __PYISTREAM_H__
#define __PYISTREAM_H__

#include "PythonCOM.h"
#include "PythonCOMServer.h"

/////////////////////////////////////////////////////////////////////////////
// class

class PYCOM_EXPORT PyIStream : public PyIUnknown {
   public:
    MAKE_PYCOM_CTOR(PyIStream);
    static PyComTypeObject type;
    static IStream *GetI(PyObject *self);

    static PyObject *Read(PyObject *self, PyObject *args);
    static PyObject *Write(PyObject *self, PyObject *args);
    static PyObject *Seek(PyObject *self, PyObject *args);
    static PyObject *SetSize(PyObject *self, PyObject *args);
    static PyObject *CopyTo(PyObject *self, PyObject *args);
    static PyObject *Commit(PyObject *self, PyObject *args);
    static PyObject *Revert(PyObject *self, PyObject *args);
    static PyObject *LockRegion(PyObject *self, PyObject *args);
    static PyObject *UnlockRegion(PyObject *self, PyObject *args);
    static PyObject *Stat(PyObject *self, PyObject *args);
    static PyObject *Clone(PyObject *self, PyObject *args);

   protected:
    PyIStream(IUnknown *);
    ~PyIStream();
};

class PyGStream : public PyGatewayBase, public IStream {
   protected:
    PyGStream(PyObject *instance) : PyGatewayBase(instance) { ; }
    PYGATEWAY_MAKE_SUPPORT(PyGStream, IStream, IID_IStream)

    // IStream
    STDMETHOD(Read)
    (
        /* [length_is][size_is][out] */ void __RPC_FAR *pv,
        /* [in] */ ULONG cb,
        /* [out] */ ULONG __RPC_FAR *pcbRead);

    STDMETHOD(Write)
    (
        /* [size_is][in] */ const void __RPC_FAR *pv,
        /* [in] */ ULONG cb,
        /* [out] */ ULONG __RPC_FAR *pcbWritten);

    STDMETHOD(Seek)
    (
        /* [in] */ LARGE_INTEGER dlibMove,
        /* [in] */ DWORD dwOrigin,
        /* [out] */ ULARGE_INTEGER __RPC_FAR *plibNewPosition);

    STDMETHOD(SetSize)
    (
        /* [in] */ ULARGE_INTEGER libNewSize);

    STDMETHOD(CopyTo)
    (
        /* [unique][in] */ IStream __RPC_FAR *pstm,
        /* [in] */ ULARGE_INTEGER cb,
        /* [out] */ ULARGE_INTEGER __RPC_FAR *pcbRead,
        /* [out] */ ULARGE_INTEGER __RPC_FAR *pcbWritten);

    STDMETHOD(Commit)
    (
        /* [in] */ DWORD grfCommitFlags);

    STDMETHOD(Revert)(void);

    STDMETHOD(LockRegion)
    (
        /* [in] */ ULARGE_INTEGER libOffset,
        /* [in] */ ULARGE_INTEGER cb,
        /* [in] */ DWORD dwLockType);

    STDMETHOD(UnlockRegion)
    (
        /* [in] */ ULARGE_INTEGER libOffset,
        /* [in] */ ULARGE_INTEGER cb,
        /* [in] */ DWORD dwLockType);

    STDMETHOD(Stat)
    (
        /* [out] */ STATSTG __RPC_FAR *pstatstg,
        /* [in] */ DWORD grfStatFlag);

    STDMETHOD(Clone)
    (
        /* [out] */ IStream __RPC_FAR *__RPC_FAR *ppstm);
};

#endif  // __PYISTREAM_H__
