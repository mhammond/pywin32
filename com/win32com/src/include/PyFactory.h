#ifndef _PYFACTORY_H_
#define _PYFACTORY_H_

/*
** The class factory for creating instances of PythonCOMObject.
*/
// Disable an OK warning...
#ifdef _MSC_VER
#pragma warning( disable : 4275 )
// warning C4275: non dll-interface struct 'IClassFactory' used as base for dll-interface class 'CPyFactory'
#endif // _MSC_VER

class PYCOM_EXPORT CPyFactory : public IClassFactory
{
public:
	CPyFactory(REFCLSID guidClassID);
	~CPyFactory();

	// IUnknown
	STDMETHOD(QueryInterface)(REFIID iid, void **ppv);
	STDMETHOD_(ULONG,AddRef)(void);
	STDMETHOD_(ULONG,Release)(void);

	// IClassFactory
	STDMETHOD(CreateInstance)(IUnknown *punkOuter, REFIID riid, void **ppv);
	STDMETHOD(LockServer)(BOOL);

protected:
	// CreateNewPythonInstance assumes that you have the Python thread lock already acquired.
	STDMETHODIMP CreateNewPythonInstance(REFCLSID rclsid, REFCLSID rReqiid, PyObject **ppNewInstance);

private:
	CLSID m_guidClassID;
	LONG m_cRef;
};

#ifdef _MSC_VER
#pragma warning(default : 4275 )
#endif // _MSC_VER

#endif /* _PYFACTORY_H_ */
