// This file implements the IEmptyVolumeCache Gateway for Python.

#include "shell_pch.h"
#include "EmptyVC.h"
#include "PyIEmptyVolumeCache.h"

// @doc - This file contains autoduck documentation
// @object PyIEmptyVolumeCache|Used for cleaning up temporary file ("disk cleanup")
// @comm This is a "gateway" object only - you can only implement this
// interface - see the shell/demos/server/empty_volume_cache.py.  The methods
// described here are the methods you must implement - you can't call them.
// <nl>Please contribute to these docs!
// @pymeth PyIEmptyVolumeCache|Initialize
// @pymeth PyIEmptyVolumeCache|GetSpaceUsed
// @pymeth PyIEmptyVolumeCache|Purge
// @pymeth PyIEmptyVolumeCache|ShowProperties
// @pymeth PyIEmptyVolumeCache2|InitializeEx
//
// ---------------------------------------------------
//
// Gateway Implementation
// @pymethod |PyIEmptyVolumeCache|Initialize|
STDMETHODIMP PyGEmptyVolumeCache::Initialize(
    /* [in] */ HKEY hkRegKey,
    /* [in] */ LPCWSTR pcwszVolume,
    /* [out] */ LPWSTR *ppwszDisplayName,
    /* [out] */ LPWSTR *ppwszDescription,
    /* [out] */ DWORD *pdwFlags)
{
    PY_GATEWAY_METHOD;
    HRESULT hr;
    BOOL bPythonIsHappy = TRUE;
    ULONG dwFlags;
    PyObject *result;
    PyObject *obppwszDisplayName;
    PyObject *obppwszDescription;
    PyObject *obpcwszVolume = NULL;
    PyObject *obhkRegKey = PyWinObject_FromHKEY(hkRegKey);
    if (obhkRegKey == NULL)
        goto args_failed;
    if (!(obpcwszVolume = MakeOLECHARToObj(pcwszVolume)))
        goto args_failed;
    hr = InvokeViaPolicy("Initialize", &result, "NNk", obhkRegKey, obpcwszVolume, *pdwFlags);
    if (FAILED(hr))
        return hr;
    // Process the Python results, and convert back to the real params

    if (!PyTuple_Check(result)) {
        PyErr_Format(PyExc_TypeError, "Initialize must return a tuple of (unicode, unicode, long) - got '%s'",
                     result->ob_type->tp_name);
        bPythonIsHappy = FALSE;
    }
    if (bPythonIsHappy && !PyArg_ParseTuple(result, "OOl", &obppwszDisplayName, &obppwszDescription, &dwFlags))
        bPythonIsHappy = FALSE;
    if (bPythonIsHappy && !PyWinObject_AsWCHAR(obppwszDisplayName, ppwszDisplayName))
        bPythonIsHappy = FALSE;
    if (bPythonIsHappy && !PyWinObject_AsWCHAR(obppwszDescription, ppwszDescription))
        bPythonIsHappy = FALSE;
    if (!bPythonIsHappy)
        hr = PyCom_SetAndLogCOMErrorFromPyException("Initialize", IID_IEmptyVolumeCache);
    if (pdwFlags)
        *pdwFlags = dwFlags;

    Py_DECREF(result);
    return hr;
args_failed:
    // only hit on error convering input args, not normal exit.
    Py_XDECREF(obhkRegKey);
    Py_XDECREF(obpcwszVolume);
    return MAKE_PYCOM_GATEWAY_FAILURE_CODE("Initialize");
}

// @pymethod |PyIEmptyVolumeCache|GetSpaceUsed|
STDMETHODIMP PyGEmptyVolumeCache::GetSpaceUsed(
    /* [out] */ DWORDLONG *pdwlSpaceUsed,
    /* [in] */ IEmptyVolumeCacheCallBack *picb)
{
    PY_GATEWAY_METHOD;
    PyObject *obpicb;
    obpicb = PyCom_PyObjectFromIUnknown(picb, IID_IEmptyVolumeCacheCallBack, TRUE);
    if (!obpicb)
        return MAKE_PYCOM_GATEWAY_FAILURE_CODE("GetSpaceUsed");

    PyObject *result;
    HRESULT hr = InvokeViaPolicy("GetSpaceUsed", &result, "O", obpicb);
    Py_XDECREF(obpicb);
    if (FAILED(hr))
        return hr;
    // Process the Python results, and convert back to the real params

    if (!PyWinObject_AsUPY_LONG_LONG(result, pdwlSpaceUsed)) {
        PyErr_Format(PyExc_TypeError, "GetSpaceUsed must return a long - got '%s'", result->ob_type->tp_name);
        hr = PyCom_SetAndLogCOMErrorFromPyException("GetSpaceUsed", IID_IEmptyVolumeCache);
    }
    Py_DECREF(result);
    return hr;
}

// @pymethod |PyIEmptyVolumeCache|Purge|
STDMETHODIMP PyGEmptyVolumeCache::Purge(
    /* [in] */ DWORDLONG dwlSpaceToFree,
    /* [in] */ IEmptyVolumeCacheCallBack *picb)
{
    PY_GATEWAY_METHOD;
    PyObject *obdwlSpaceToFree = PyLong_FromUnsignedLongLong(dwlSpaceToFree);
    if (obdwlSpaceToFree == NULL)
        return MAKE_PYCOM_GATEWAY_FAILURE_CODE("Purge");
    PyObject *obpicb;
    obpicb = PyCom_PyObjectFromIUnknown(picb, IID_IEmptyVolumeCacheCallBack, TRUE);
    if (!obpicb) {
        Py_DECREF(obdwlSpaceToFree);
        return MAKE_PYCOM_GATEWAY_FAILURE_CODE("Purge");
    }
    return InvokeViaPolicy("Purge", NULL, "NN", obdwlSpaceToFree, obpicb);
}

// @pymethod |PyIEmptyVolumeCache|ShowProperties|
STDMETHODIMP PyGEmptyVolumeCache::ShowProperties(
    /* [in] */ HWND hwnd)
{
    PY_GATEWAY_METHOD;
    return InvokeViaPolicy("ShowProperties", NULL, "N", PyWinLong_FromHANDLE(hwnd));
}

// @pymethod |PyIEmptyVolumeCache|Deactivate|
STDMETHODIMP PyGEmptyVolumeCache::Deactivate(
    /* [out] */ DWORD *pdwFlags)
{
    PY_GATEWAY_METHOD;
    PyObject *result;
    HRESULT hr = InvokeViaPolicy("Deactivate", &result);
    if (FAILED(hr))
        return hr;
    // Process the Python results, and convert back to the real params

    BOOL bPythonIsHappy = TRUE;

    if (!PyWinObject_AsUPY_LONG_LONG(result, pdwFlags)) {
        PyErr_Format(PyExc_TypeError, "Deactivate must return a long - got '%s'", result->ob_type->tp_name);
        hr = PyCom_SetAndLogCOMErrorFromPyException("Deactivate", IID_IEmptyVolumeCache);
    }
    Py_DECREF(result);
    return hr;
}

// IEmptyVolumeCache2
// @object PyIEmptyVolumeCache2|See also <o PyIEmptyVolumeCache>
// @pymeth PyIEmptyVolumeCache|Deactivate
// @pymethod |PyIEmptyVolumeCache2|InitializeEx|
STDMETHODIMP PyGEmptyVolumeCache2::InitializeEx(
    /* [in] */ HKEY hkRegKey,
    /* [in] */ LPCWSTR pcwszVolume,
    /* [in] */ LPCWSTR pcwszKeyName,
    /* [out] */ LPWSTR *ppwszDisplayName,
    /* [out] */ LPWSTR *ppwszDescription,
    /* [out] */ LPWSTR *ppwszBtnText,
    /* [out] */ DWORD *pdwFlags)
{
    PY_GATEWAY_METHOD;
    BOOL bPythonIsHappy = TRUE;
    ULONG dwFlags;
    HRESULT hr;
    PyObject *result;
    PyObject *obppwszDisplayName;
    PyObject *obppwszDescription;
    PyObject *obppwszBtnText;
    PyObject *obpcwszVolume = NULL;
    PyObject *obpcwszKeyName = NULL;
    PyObject *obhkRegKey = PyWinObject_FromHKEY(hkRegKey);
    if (!obhkRegKey)
        goto args_failed;
    if (!(obpcwszVolume = MakeOLECHARToObj(pcwszVolume)))
        goto args_failed;
    if (!(obpcwszKeyName = MakeOLECHARToObj(pcwszKeyName)))
        goto args_failed;
    hr = InvokeViaPolicy("InitializeEx", &result, "NNNk", obhkRegKey, obpcwszVolume, obpcwszKeyName, *pdwFlags);
    // NOTE: From here, do *not* exit via args_failed - the args have been cleaned up
    if (FAILED(hr))
        return hr;
    // Process the Python results, and convert back to the real params

    if (!PyTuple_Check(result)) {
        PyErr_Format(PyExc_TypeError, "Initialize must return a tuple of (unicode, unicode, unicode, long) - got '%s'",
                     result->ob_type->tp_name);
        bPythonIsHappy = FALSE;
    }
    if (bPythonIsHappy &&
        !PyArg_ParseTuple(result, "OOOl", &obppwszDisplayName, &obppwszDescription, &obppwszBtnText, &dwFlags))
        bPythonIsHappy = FALSE;
    if (bPythonIsHappy && !PyWinObject_AsWCHAR(obppwszDisplayName, ppwszDisplayName))
        bPythonIsHappy = FALSE;
    if (bPythonIsHappy && !PyWinObject_AsWCHAR(obppwszDescription, ppwszDescription))
        bPythonIsHappy = FALSE;
    if (bPythonIsHappy && !PyWinObject_AsWCHAR(obppwszBtnText, ppwszBtnText))
        bPythonIsHappy = FALSE;
    if (!bPythonIsHappy)
        hr = PyCom_SetAndLogCOMErrorFromPyException("InitializeEx", IID_IEmptyVolumeCache);
    if (pdwFlags)
        *pdwFlags = dwFlags;

    Py_DECREF(result);
    return hr;
args_failed:
    // only hit on error convering input args, not normal exit.
    Py_XDECREF(obhkRegKey);
    Py_XDECREF(obpcwszVolume);
    Py_XDECREF(obpcwszKeyName);
    return MAKE_PYCOM_GATEWAY_FAILURE_CODE("InitializeEx");
}
