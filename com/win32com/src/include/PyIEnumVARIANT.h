#ifndef __PYIENUMVARIANT_H__
#define __PYIENUMVARIANT_H__

#include "PythonCOM.h"
#include "PythonCOMServer.h"

class PYCOM_EXPORT PyIEnumVARIANT : public PyIUnknown
{
public:
	MAKE_PYCOM_CTOR(PyIEnumVARIANT);
	static IEnumVARIANT *GetI(PyObject *self);
	static PyComEnumTypeObject type;

	// The Python methods
	static PyObject *Next(PyObject *self, PyObject *args);
	static PyObject *Skip(PyObject *self, PyObject *args);
	static PyObject *Reset(PyObject *self, PyObject *args);
	static PyObject *Clone(PyObject *self, PyObject *args);

protected:
	PyIEnumVARIANT(IUnknown *pdisp);
	~PyIEnumVARIANT();
};

class PyGEnumVARIANT :
	public PyGatewayBase,
	public IEnumVARIANT
{
protected:
	PyGEnumVARIANT(PyObject *instance) : PyGatewayBase(instance) {;}
	PYGATEWAY_MAKE_SUPPORT(PyGEnumVARIANT, IEnumVARIANT, IID_IEnumVARIANT)

	// IEnumVARIANT
	STDMETHOD(Next)( 
            /* [in] */ ULONG celt,
            /* [length_is][size_is][out] */ VARIANT __RPC_FAR *rgVar,
            /* [out] */ ULONG __RPC_FAR *pCeltFetched);
	STDMETHOD(Skip)( 
            /* [in] */ ULONG celt);
	STDMETHOD(Reset)( void);
	STDMETHOD(Clone)( 
            /* [out] */ IEnumVARIANT __RPC_FAR *__RPC_FAR *ppEnum);
};

#endif /* __PYIENUMVARIANT_H__ */
