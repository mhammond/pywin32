#include "stdafx.h"
#include "PythonCOM.h"
#include "PythonCOMServer.h"
#include "PyIStream.h"

STDMETHODIMP PyGStream::Read(
            /* [length_is][size_is][out] */ void __RPC_FAR *pv,
            /* [in] */ ULONG cb,
            /* [out] */ ULONG __RPC_FAR *pcbRead)
{
	if ( pv == NULL )
		return PyCom_SetCOMErrorFromSimple(E_POINTER, GetIID());

	PY_GATEWAY_METHOD;
	PyObject *result;
	HRESULT hr = InvokeViaPolicy("Read", &result, "l", cb);
	if ( FAILED(hr) )
		return hr;

	hr = E_FAIL;
	int len = PyObject_Length(result);
	if ( len != -1 )
	{
		const char *s = PyString_AsString(result);
		if ( s != NULL )
		{
			memcpy(pv, s, len);
			if ( pcbRead != NULL )
				*pcbRead = len;
			hr = S_OK;
		}
	}

	Py_DECREF(result);
	return PyCom_SetCOMErrorFromPyException(GetIID());
}

STDMETHODIMP PyGStream::Write(
            /* [size_is][in] */ const void __RPC_FAR *pv,
            /* [in] */ ULONG cb,
            /* [out] */ ULONG __RPC_FAR *pcbWritten)
{
	if ( pv == NULL )
		return PyCom_SetCOMErrorFromSimple(E_POINTER, GetIID());

	PY_GATEWAY_METHOD;
	PyObject *result;
	HRESULT hr = InvokeViaPolicy("Write", &result, "s#", pv, (int)cb);
	if ( FAILED(hr) )
		return hr;

	int cbWritten = PyInt_AsLong(result);
	Py_DECREF(result);
	if ( cbWritten == -1 )
		return PyCom_SetCOMErrorFromPyException(GetIID());
	if ( pcbWritten != NULL )
		*pcbWritten = cbWritten;

	return S_OK;
}

STDMETHODIMP PyGStream::Seek(
		/* [in] */ LARGE_INTEGER dlibMove,
		/* [in] */ DWORD dwOrigin,
		/* [out] */ ULARGE_INTEGER __RPC_FAR * plibNewPosition)
{
	PY_GATEWAY_METHOD;
	PyObject *obdlibMove = PyWinObject_FromLARGE_INTEGER(dlibMove);
	PyObject *result;
	HRESULT hr=InvokeViaPolicy("Seek", &result, "Oi", obdlibMove, dwOrigin);
	Py_XDECREF(obdlibMove);
	if (FAILED(hr)) return hr;
	// Process the Python results, and convert back to the real params
	PyObject *obplibNewPosition;
	if (!PyArg_Parse(result, "O" , &obplibNewPosition)) return PyCom_HandlePythonFailureToCOM(/*pexcepinfo*/);
	BOOL bPythonIsHappy = TRUE;
	if (!PyWinObject_AsULARGE_INTEGER(obplibNewPosition, plibNewPosition)) bPythonIsHappy = FALSE;
	if (!bPythonIsHappy) hr = PyCom_HandlePythonFailureToCOM(/*pexcepinfo*/);
	Py_DECREF(result);
	return hr;
}

STDMETHODIMP PyGStream::SetSize(
		/* [in] */ ULARGE_INTEGER libNewSize)
{
	PY_GATEWAY_METHOD;
	PyObject *oblibNewSize = PyWinObject_FromULARGE_INTEGER(libNewSize);
	HRESULT hr=InvokeViaPolicy("SetSize", NULL, "O", oblibNewSize);
	Py_XDECREF(oblibNewSize);
	return hr;
}

STDMETHODIMP PyGStream::CopyTo(
		/* [unique][in] */ IStream __RPC_FAR * pstm,
		/* [in] */ ULARGE_INTEGER cb,
		/* [out] */ ULARGE_INTEGER __RPC_FAR * pcbRead,
		/* [out] */ ULARGE_INTEGER __RPC_FAR * pcbWritten)
{
	PY_GATEWAY_METHOD;
	PyObject *obpstm = PyCom_PyObjectFromIUnknown(pstm, IID_IStream, TRUE);
	PyObject *obcb = PyWinObject_FromULARGE_INTEGER(cb);
	PyObject *result;
	HRESULT hr=InvokeViaPolicy("CopyTo", &result, "OO", obpstm, obcb);
	Py_XDECREF(obpstm);
	Py_XDECREF(obcb);
	if (FAILED(hr)) return hr;
	// Process the Python results, and convert back to the real params
	PyObject *obpcbRead;
	PyObject *obpcbWritten;
	if (!PyArg_ParseTuple(result, "OO" , &obpcbRead, &obpcbWritten)) return PyCom_HandlePythonFailureToCOM(/*pexcepinfo*/);
	BOOL bPythonIsHappy = TRUE;
	if (pcbRead && !PyWinObject_AsULARGE_INTEGER(obpcbRead, pcbRead)) bPythonIsHappy = FALSE;
	if (pcbWritten && !PyWinObject_AsULARGE_INTEGER(obpcbWritten, pcbWritten)) bPythonIsHappy = FALSE;
	if (!bPythonIsHappy) hr = PyCom_HandlePythonFailureToCOM(/*pexcepinfo*/);
	Py_DECREF(result);
	return hr;
}

STDMETHODIMP PyGStream::Commit(
		/* [in] */ DWORD grfCommitFlags)
{
	PY_GATEWAY_METHOD;
	HRESULT hr=InvokeViaPolicy("Commit", NULL, "i", grfCommitFlags);
	return hr;
}

STDMETHODIMP PyGStream::Revert(
		void)
{
	PY_GATEWAY_METHOD;
	HRESULT hr=InvokeViaPolicy("Revert");
	return hr;
}

STDMETHODIMP PyGStream::LockRegion(
		/* [in] */ ULARGE_INTEGER libOffset,
		/* [in] */ ULARGE_INTEGER cb,
		/* [in] */ DWORD dwLockType)
{
	PY_GATEWAY_METHOD;
	PyObject *oblibOffset = PyWinObject_FromULARGE_INTEGER(libOffset);
	PyObject *obcb = PyWinObject_FromULARGE_INTEGER(cb);
	HRESULT hr=InvokeViaPolicy("LockRegion", NULL, "OOi", oblibOffset, obcb, dwLockType);
	Py_XDECREF(oblibOffset);
	Py_XDECREF(obcb);
	return hr;
}

STDMETHODIMP PyGStream::UnlockRegion(
		/* [in] */ ULARGE_INTEGER libOffset,
		/* [in] */ ULARGE_INTEGER cb,
		/* [in] */ DWORD dwLockType)
{
	PY_GATEWAY_METHOD;
	PyObject *oblibOffset = PyWinObject_FromULARGE_INTEGER(libOffset);
	PyObject *obcb = PyWinObject_FromULARGE_INTEGER(cb);
	HRESULT hr=InvokeViaPolicy("UnlockRegion", NULL, "OOi", oblibOffset, obcb, dwLockType);
	Py_XDECREF(oblibOffset);
	Py_XDECREF(obcb);
	return hr;
}

STDMETHODIMP PyGStream::Stat(
		/* [out] */ STATSTG __RPC_FAR * pstatstg,
		/* [in] */ DWORD grfStatFlag)
{
	PY_GATEWAY_METHOD;
	PyObject *result;
	HRESULT hr=InvokeViaPolicy("Stat", &result, "i", grfStatFlag);
	if (FAILED(hr)) return hr;
	// Process the Python results, and convert back to the real params
	PyObject *obpstatstg;
	if (!PyArg_Parse(result, "O" , &obpstatstg)) return PyCom_HandlePythonFailureToCOM(/*pexcepinfo*/);
	BOOL bPythonIsHappy = TRUE;
	if (!PyCom_PyObjectAsSTATSTG(obpstatstg, pstatstg, 0/*flags*/)) bPythonIsHappy = FALSE;
	if (!bPythonIsHappy) hr = PyCom_HandlePythonFailureToCOM(/*pexcepinfo*/);
	Py_DECREF(result);
	return hr;
}

STDMETHODIMP PyGStream::Clone(
		/* [out] */ IStream __RPC_FAR *__RPC_FAR * ppstm)
{
	PY_GATEWAY_METHOD;
	if (ppstm==NULL) return E_POINTER;
	PyObject *result;
	HRESULT hr=InvokeViaPolicy("Clone", &result);
	if (FAILED(hr)) return hr;
	// Process the Python results, and convert back to the real params
	PyObject *obppstm;
	if (!PyArg_Parse(result, "O" , &obppstm)) return PyCom_HandlePythonFailureToCOM(/*pexcepinfo*/);
	BOOL bPythonIsHappy = TRUE;
	if (!PyCom_InterfaceFromPyObject(obppstm, IID_IStream, (void **)ppstm, FALSE /* bNoneOK */))
		 bPythonIsHappy = FALSE;
	if (!bPythonIsHappy) hr = PyCom_HandlePythonFailureToCOM(/*pexcepinfo*/);
	Py_DECREF(result);
	return hr;
}
