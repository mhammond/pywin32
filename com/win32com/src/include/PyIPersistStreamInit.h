#ifndef __PYIPERSISTSTREAMINIT_H__
#define __PYIPERSISTSTREAMINIT_H__

#include "PythonCOM.h"
#include "PythonCOMServer.h"

#include "PyIPersistStream.h"

class PYCOM_EXPORT PyIPersistStreamInit : public PyIPersistStream
{
public:
	MAKE_PYCOM_CTOR(PyIPersistStreamInit);
	static PyComTypeObject type;
	static IPersistStreamInit *GetI(PyObject *self);

	static PyObject *InitNew(PyObject *self, PyObject *args);

protected:
	PyIPersistStreamInit(IUnknown *);
	~PyIPersistStreamInit();
};

class PyGPersistStreamInit : public PyGPersistStream, public IPersistStreamInit
{
protected:
	PyGPersistStreamInit(PyObject *instance) : PyGPersistStream(instance) { ; }
	PYGATEWAY_MAKE_SUPPORT2(PyGPersistStreamInit, IPersistStreamInit, IID_IPersistStreamInit, PyGPersistStream)
	// IPersist
	STDMETHOD(GetClassID)(CLSID FAR *pClassID) {return PyGPersistStream::GetClassID(pClassID);}

	// IPersistStream
	STDMETHOD(IsDirty)(void) {return PyGPersistStream::IsDirty();}

	STDMETHOD(Load)(IStream __RPC_FAR *pStm) {return PyGPersistStream::Load(pStm);}

	STDMETHOD(Save)(
            /* [unique][in] */ IStream __RPC_FAR *pStm,
            /* [in] */ BOOL fClearDirty)
		{return PyGPersistStream::Save(pStm, fClearDirty);}

	STDMETHOD(GetSizeMax)(ULARGE_INTEGER __RPC_FAR *pcbSize) {return PyGPersistStream::GetSizeMax(pcbSize);}

	// IPersistStreamInit
	STDMETHOD(InitNew)(void);
};

#endif // __PYIPERSISTSTREAMINIT_H__
