#ifndef __PROPBAG_H__
#define __PROPBAG_H__

#include "PyIPersist.h"

/*-------------------------------------------------------------------------
**
** CLIENT SIDE INTERFACE OBJECTS
**
*/
class PyIPropertyBag : public PyIUnknown
{
public:
	MAKE_PYCOM_CTOR(PyIPropertyBag);
	static IPropertyBag *GetI(PyObject *self);
	static PyComTypeObject type;

	// The Python methods
	static PyObject *Read(PyObject *self, PyObject *args);
	static PyObject *Write(PyObject *self, PyObject *args);

protected:
	PyIPropertyBag(IUnknown *pdisp);
	~PyIPropertyBag();
};

class PyIPersistPropertyBag : public PyIUnknown
{
public:
	MAKE_PYCOM_CTOR(PyIPersistPropertyBag);
	static IPersistPropertyBag *GetI(PyObject *self);
	static PyComTypeObject type;

	// The Python methods
	static PyObject *InitNew(PyObject *self, PyObject *args);
	static PyObject *Load(PyObject *self, PyObject *args);
	static PyObject *Save(PyObject *self, PyObject *args);

protected:
	PyIPersistPropertyBag(IUnknown *pdisp);
	~PyIPersistPropertyBag();
};

class PyIErrorLog : public PyIUnknown
{
public:
	MAKE_PYCOM_CTOR(PyIErrorLog);
	static IErrorLog *GetI(PyObject *self);
	static PyComTypeObject type;

	// The Python methods
	static PyObject *AddError(PyObject *self, PyObject *args);

protected:
	PyIErrorLog(IUnknown *pdisp);
	~PyIErrorLog();
};

/*-------------------------------------------------------------------------
**
** SERVER SIDE GATEWAY OBJECTS
**
*/

class PyGPropertyBag : public PyGatewayBase, public IPropertyBag
{
protected:
	PyGPropertyBag(PyObject *instance) : PyGatewayBase(instance) { ; }
	PYGATEWAY_MAKE_SUPPORT(PyGPropertyBag, IPropertyBag, IID_IPropertyBag)

	// IPropertyBag
	STDMETHOD(Read)(
            /* [in] */ LPCOLESTR pszPropName,
            /* [out][in] */ VARIANT __RPC_FAR *pVar,
            /* [in] */ IErrorLog __RPC_FAR *pErrorLog);

	STDMETHOD(Write)(
            /* [in] */ LPCOLESTR pszPropName,
            /* [in] */ VARIANT __RPC_FAR *pVar);
};

class PyGPersistPropertyBag : public PyGPersist, public IPersistPropertyBag
{
protected:
	PyGPersistPropertyBag(PyObject *instance) : PyGPersist(instance) { ; }
	PYGATEWAY_MAKE_SUPPORT2(PyGPersistPropertyBag, IPersistPropertyBag, IID_IPersistPropertyBag, PyGPersist)

	// IPersist
	STDMETHOD(GetClassID)(CLSID FAR *pClassID) {return PyGPersist::GetClassID(pClassID);}

	// IPersistPropertyBag
	STDMETHOD(InitNew)(void);

	STDMETHOD(Load)(
            /* [in] */ IPropertyBag __RPC_FAR *pPropBag,
            /* [in] */ IErrorLog __RPC_FAR *pErrorLog);

	STDMETHOD(Save)(
            /* [in] */ IPropertyBag __RPC_FAR *pPropBag,
            /* [in] */ BOOL fClearDirty,
            /* [in] */ BOOL fSaveAllProperties);
};

class PyGErrorLog : public PyGatewayBase, public IErrorLog
{
protected:
	PyGErrorLog(PyObject *instance) : PyGatewayBase(instance) { ; }
	PYGATEWAY_MAKE_SUPPORT(PyGErrorLog, IErrorLog, IID_IErrorLog)

	// IErrorLog
	STDMETHOD(AddError)(
            /* [in] */ LPCOLESTR pszPropName,
            /* [in] */ EXCEPINFO __RPC_FAR *pExcepInfo);
};

#endif /* __PROPBAG_H__ */
