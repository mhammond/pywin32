// This file declares the IEmptyVolumeCache Gateway for Python.
// ---------------------------------------------------
//
// Gateway Declaration

class PyGEmptyVolumeCache : public PyGatewayBase, public IEmptyVolumeCache
{
protected:
	PyGEmptyVolumeCache(PyObject *instance) : PyGatewayBase(instance) { ; }
	PYGATEWAY_MAKE_SUPPORT2(PyGEmptyVolumeCache, IEmptyVolumeCache, IID_IEmptyVolumeCache, PyGatewayBase)

	// IEmptyVolumeCache
	STDMETHOD(Initialize)(
		HKEY hkRegKey,
		LPCWSTR pcwszVolume,
		LPWSTR * ppwszDisplayName,
		LPWSTR * ppwszDescription,
		DWORD * pdwFlags);

	STDMETHOD(GetSpaceUsed)(
		DWORDLONG * pdwlSpaceUsed,
		IEmptyVolumeCacheCallBack * picb);

	STDMETHOD(Purge)(
		DWORDLONG dwlSpaceToFree,
		IEmptyVolumeCacheCallBack * picb);

	STDMETHOD(ShowProperties)(
		HWND hwnd);

	STDMETHOD(Deactivate)(
		DWORD * pdwFlags);

};

class PyGEmptyVolumeCache2 : public PyGEmptyVolumeCache, public IEmptyVolumeCache2
{
protected:
	PyGEmptyVolumeCache2(PyObject *instance) : PyGEmptyVolumeCache(instance) { ; }
	PYGATEWAY_MAKE_SUPPORT2(PyGEmptyVolumeCache2, IEmptyVolumeCache2, IID_IEmptyVolumeCache2, PyGEmptyVolumeCache)
	// IEmptyVolumeCache

	STDMETHOD(Initialize)(
		HKEY hkRegKey,
		LPCWSTR pcwszVolume,
		LPWSTR * ppwszDisplayName,
		LPWSTR * ppwszDescription,
		DWORD * pdwFlags) {
            return PyGEmptyVolumeCache::Initialize(hkRegKey, pcwszVolume, ppwszDisplayName, ppwszDescription, pdwFlags);
        }
	STDMETHOD(GetSpaceUsed)(
		DWORDLONG * pdwlSpaceUsed,
		IEmptyVolumeCacheCallBack * picb) {
            return PyGEmptyVolumeCache::GetSpaceUsed(pdwlSpaceUsed, picb);
        }

	STDMETHOD(Purge)(
		DWORDLONG dwlSpaceToFree,
		IEmptyVolumeCacheCallBack * picb) {
            return PyGEmptyVolumeCache::Purge(dwlSpaceToFree, picb);
        }

	STDMETHOD(ShowProperties)(
		HWND hwnd) {
            return PyGEmptyVolumeCache::ShowProperties(hwnd);
        }

	STDMETHOD(Deactivate)(
		DWORD * pdwFlags) {
            return PyGEmptyVolumeCache::Deactivate(pdwFlags);
        }

	// IEmptyVolumeCache2
	STDMETHOD(InitializeEx)(
		HKEY hkRegKey,
		LPCWSTR pcwszVolume,
		LPCWSTR pcwszKeyName,
		LPWSTR * ppwszDisplayName,
		LPWSTR * ppwszDescription,
		LPWSTR * ppwszBtnText,
		DWORD * pdwFlags);
};
