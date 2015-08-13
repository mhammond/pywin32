//
// PyUnicode.cpp -- Unicode string type for Python
//
// @doc

#include "PyWinTypes.h"
#include "PyWinObjects.h"
#include "malloc.h"
#include "tchar.h"
#include "locale.h"

// @object PyUnicode|A Python object, representing a Unicode string.
// @comm pywin32 uses the builtin Python Unicode object
// <nl>In general, any pywin32/COM function documented as taking a
// PyUnicode parameter will also accept a Python string object, which will
// be automatically encoded using the MBCS encoding before being passed to the function.
// Note that the reverse is generally *not* true - a function documented as accepting
// a string must be passed a string.

BOOL PyWinObject_AsPfnAllocatedWCHAR(PyObject *stringObject, void *(*pfnAllocator)(ULONG), WCHAR **ppResult, BOOL bNoneOK /*= FALSE*/,DWORD *pResultLen /*= NULL*/)
{
	BOOL rc = TRUE;
	if (PyString_Check(stringObject)) {
		int cch=PyString_Size(stringObject);
		const char *buf = PyString_AsString(stringObject);
		if (buf==NULL) return FALSE;

		/* We assume that we dont need more 'wide characters' for the result
		   then the number of bytes in the input. Often we
		   will need less, as the input may contain multi-byte chars, but we
		   should never need more 
		*/
		*ppResult = (LPWSTR)(*pfnAllocator)((cch+1)*sizeof(WCHAR));
		if (*ppResult)
			/* convert and get the final character size */
			cch = MultiByteToWideChar(CP_ACP, 0, buf, cch+1, *ppResult, cch+1);
		if (*ppResult && pResultLen) *pResultLen = cch;
	} else if (PyUnicode_Check(stringObject)) {
		// copy the value, including embedded NULLs
		WCHAR *v = (WCHAR *)PyUnicode_AS_UNICODE(stringObject);
		UINT cch = PyUnicode_GET_SIZE(stringObject);
		*ppResult = (WCHAR *)pfnAllocator((cch+1) * sizeof(WCHAR));
		if (*ppResult)
			memcpy(*ppResult, v, (cch+1) * sizeof(WCHAR));
		if (*ppResult && pResultLen) *pResultLen = cch;

	} else if (stringObject == Py_None) {
		if (bNoneOK) {
			*ppResult = NULL;
		} else {
			PyErr_SetString(PyExc_TypeError, "None is not a valid string in this context");
			rc = FALSE;
		}
	} else {
		const char *tp_name = stringObject && stringObject->ob_type ? stringObject->ob_type->tp_name : "<NULL!!>";
		PyErr_Format(PyExc_TypeError, "Objects of type '%s' can not be converted to Unicode.", tp_name);
		rc = FALSE;
	}
	if (rc && !ppResult) {
		PyErr_SetString(PyExc_MemoryError, "Allocating WCHAR");
		return FALSE;
	}
	return rc;
}

// Get around calling conversion issues.
void *AllocViaCoTaskMemAlloc(ULONG cb)
{
	return CoTaskMemAlloc(cb);
}

BOOL PyWinObject_AsTaskAllocatedWCHAR(PyObject *stringObject, WCHAR **ppResult, BOOL bNoneOK /*= FALSE*/,DWORD *pResultLen /*= NULL*/)
{
	return PyWinObject_AsPfnAllocatedWCHAR(stringObject, AllocViaCoTaskMemAlloc, ppResult, bNoneOK, pResultLen);
}

void PyWinObject_FreeTaskAllocatedWCHAR(WCHAR * str)
{
	CoTaskMemFree(str);
}

/* Implement our Windows Unicode API using the Python widestring object */

// non-unicode version of PyWinObject_FromTCHAR; returned object depends on
// if we are running py3k or not.
PyObject *PyWinObject_FromTCHAR(const char *str, Py_ssize_t len /*=(Py_ssize_t)-1*/)
{
	if (str==NULL) {
		Py_INCREF(Py_None);
		return Py_None;
	}
	if (len==(Py_ssize_t)-1)
		len = strlen(str);
#if (PY_VERSION_HEX < 0x03000000)
	return PyString_FromStringAndSize(str, len);
#else
	// py3k - decode char * via mbcs encoding.
	return (PyObject *)PyUnicode_DecodeMBCS(str, len, NULL);
#endif
}

PyObject *PyWinCoreString_FromString(const char *str, Py_ssize_t len /*=(Py_ssize_t)-1*/)
{
	if (len==(Py_ssize_t)-1)
		len = strlen(str);
#if (PY_VERSION_HEX < 0x03000000)
	return PyString_FromStringAndSize(str, len);
#else
	return PyUnicode_DecodeMBCS(str, len, "ignore");
#endif
}

PyObject *PyWinCoreString_FromString(const WCHAR *str, Py_ssize_t len /*=(Py_ssize_t)-1*/)
{
	if (len==(Py_ssize_t)-1)
		len = wcslen(str);
#if (PY_VERSION_HEX < 0x03000000)
	return PyUnicode_EncodeMBCS(str, len, "ignore");
#else
	return PyUnicode_FromWideChar(str, len);
#endif
}

// Convert a Python object to a "char *" - allow embedded NULLs, None, etc.
BOOL PyWinObject_AsString(PyObject *stringObject, char **pResult, BOOL bNoneOK /*= FALSE*/, DWORD *pResultLen /* = NULL */)
{
	PyObject *tempObject = NULL;
	if (stringObject==Py_None) {
		if (!bNoneOK) {
			PyErr_SetString(PyExc_TypeError, "None is not a valid string in this context");
			return FALSE;
		}
		*pResult = NULL;
		if (pResultLen) *pResultLen = 0;
		return TRUE;
	}
	// Convert the string if a WIDE string.
	if (PyUnicode_Check(stringObject))
	{
		stringObject = tempObject = PyUnicode_EncodeMBCS(PyUnicode_AS_UNICODE(stringObject), PyUnicode_GET_SIZE(stringObject), NULL);
		if (!stringObject)
			return FALSE;
	}
	if (!PyString_Check(stringObject)) {
		PyErr_Format(PyExc_TypeError, "The object must be a string or unicode object (got '%s')",
					 stringObject->ob_type->tp_name);
		return FALSE;
	}
	char *temp = PyString_AsString(stringObject);
	int len = PyString_Size(stringObject);
	*pResult = (char *)PyMem_Malloc(len+1);
	if (*pResult) {
		memcpy(*pResult, temp, len+1);
		if (pResultLen) *pResultLen = len;
	}
	Py_XDECREF(tempObject);
	return (*pResult != NULL);
}

void PyWinObject_FreeString(char *str)
{
	PyMem_Free(str);
}
void PyWinObject_FreeString(WCHAR *str)
{
	PyMem_Free(str);
}

// Size info is available (eg, a fn returns a string and also fills in a size variable)
PyObject *PyWinObject_FromOLECHAR(const OLECHAR * str, int numChars)
{
	if (str==NULL) {
		Py_INCREF(Py_None);
		return Py_None;
	}
	return PyUnicode_FromWideChar((OLECHAR *)str, numChars);
}

// No size info avail.
PyObject *PyWinObject_FromOLECHAR(const OLECHAR * str)
{
	if (str==NULL) {
		Py_INCREF(Py_None);
		return Py_None;
	}
	return PyUnicode_FromWideChar( (OLECHAR *)str, wcslen(str) );
}

PyObject *PyWinObject_FromBstr(const BSTR bstr, BOOL takeOwnership /*=FALSE*/)
{
	if (bstr==NULL) {
		Py_INCREF(Py_None);
		return Py_None;
	}
	PyObject *ret = PyUnicode_FromWideChar(bstr, SysStringLen(bstr));
	if (takeOwnership) SysFreeString(bstr);
	return ret;
}

///////////////////////////////////////////////////////////
//
// Some utilities etc

PyWin_AutoFreeBstr::PyWin_AutoFreeBstr( BSTR bstr /*= NULL*/ )
 : m_bstr(bstr)
{
	return;
}

PyWin_AutoFreeBstr::~PyWin_AutoFreeBstr()
{
	SysFreeString(m_bstr);
}

void PyWin_AutoFreeBstr::SetBstr( BSTR bstr )
{
	SysFreeString(m_bstr);
	m_bstr = bstr;
}

// String conversions
// Convert a Python string object to a BSTR - allow embedded NULLs, etc.
static BOOL PyString_AsBstr(PyObject *stringObject, BSTR *pResult)
{
	int size=PyString_Size(stringObject);
	const char *buf = PyString_AsString(stringObject);
	if (buf==NULL) return FALSE;

	/* We assume that we dont need more 'wide characters' for the result
	   then the number of bytes in the input. Often we
	   will need less, as the input may contain multi-byte chars, but we
	   should never need more 
	*/

	LPWSTR wstr = (LPWSTR)malloc(size*sizeof(WCHAR));
	if (wstr==NULL) {
		PyErr_SetString(PyExc_MemoryError, "No memory for wide string buffer");
		return FALSE;
	}
	/* convert and get the final character size */
	size = MultiByteToWideChar(CP_ACP, 0, buf, size, wstr, size);
	*pResult = SysAllocStringLen(wstr, size);
	if (*pResult==NULL)
		PyErr_SetString(PyExc_MemoryError, "allocating BSTR");
	free(wstr);
	return *pResult != NULL;
}

// Convert a Python object to a BSTR - allow embedded NULLs, None, etc.
BOOL PyWinObject_AsBstr(PyObject *stringObject, BSTR *pResult, BOOL bNoneOK /*= FALSE*/,DWORD *pResultLen /*= NULL*/)
{
	BOOL rc = TRUE;
	if (PyString_Check(stringObject))
		rc = PyString_AsBstr(stringObject, pResult);
	else if (PyUnicode_Check(stringObject))
	{
		// copy the value, including embedded NULLs
		int nchars = PyUnicode_GET_SIZE(stringObject);
		*pResult = SysAllocStringLen(NULL, nchars);
		if (*pResult) {
#if (PY_VERSION_HEX < 0x03020000)
#define PUAWC_TYPE PyUnicodeObject *
#else
#define PUAWC_TYPE PyObject *
#endif
			if (PyUnicode_AsWideChar((PUAWC_TYPE)stringObject, *pResult, nchars)==-1) {
				rc = FALSE;
			} else {
				// The SysAllocStringLen docs indicate that nchars+1 bytes are allocated,
				// and that normally a \0 is appened by the function.  It also states 
				// the \0 is not necessary!  While it seems to work fine without it,
				// we do copy it, as the previous code, which used SysAllocStringLen
				// with a non-NULL arg is documented clearly as appending the \0.
				(*pResult)[nchars] = 0;
			}
		}
	}
	else if (stringObject == Py_None) {
		if (bNoneOK) {
			*pResult = NULL;
		} else {
			PyErr_SetString(PyExc_TypeError, "None is not a valid string in this context");
			rc = FALSE;
		}
	} else {
		const char *tp_name = stringObject && stringObject->ob_type ? stringObject->ob_type->tp_name : "<NULL!!>";
		PyErr_Format(PyExc_TypeError, "Objects of type '%s' can not be converted to Unicode.", tp_name);
		rc = FALSE;
	}
	if (rc && !pResult) {
		PyErr_SetString(PyExc_MemoryError, "Allocating BSTR");
		return FALSE;
	}
	if (rc && pResultLen) *pResultLen = SysStringLen(*pResult);
	return rc;
}

void PyWinObject_FreeBstr(BSTR str)
{
	SysFreeString(str);
}

// String conversions
// Convert a Python object to a WCHAR - allow embedded NULLs, None, etc.
BOOL PyWinObject_AsWCHAR(PyObject *stringObject, WCHAR **pResult, BOOL bNoneOK /*= FALSE*/,DWORD *pResultLen /*= NULL*/)
{
	BOOL rc = TRUE;
	int resultLen = 0;
#if (PY_VERSION_HEX < 0x03000000)
	// Do NOT accept 'bytes' object when a plain 'WCHAR' is needed on py3k.
	if (PyString_Check(stringObject)) {
		int size=PyString_Size(stringObject);
		const char *buf = PyString_AsString(stringObject);
		if (buf==NULL) return FALSE;

		/* We assume that we dont need more 'wide characters' for the result
		   then the number of bytes in the input. Often we
		   will need less, as the input may contain multi-byte chars, but we
		   should never need more 
		*/
		*pResult = (LPWSTR)PyMem_Malloc((size+1)*sizeof(WCHAR));
		if (*pResult==NULL) {
			PyErr_SetString(PyExc_MemoryError, "No memory for wide string buffer");
			return FALSE;
		}
		/* convert and get the final character size */
		resultLen = MultiByteToWideChar(CP_ACP, 0, buf, size, *pResult, size);
		/* terminate the string */
		(*pResult)[resultLen] = L'\0';
	}
	else
#endif // py3k
	if (PyUnicode_Check(stringObject))
	{
		resultLen = PyUnicode_GET_SIZE(stringObject);
		size_t cb = sizeof(WCHAR) * (resultLen+1);
		*pResult = (WCHAR *)PyMem_Malloc(cb);
		if (*pResult==NULL) {
			PyErr_SetString(PyExc_MemoryError, "Allocating WCHAR array");
			return FALSE;
		}
		// copy the value, including embedded NULLs
		memcpy(*pResult, PyUnicode_AsUnicode(stringObject), cb);
	}
	else if (stringObject == Py_None) {
		if (bNoneOK) {
			*pResult = NULL;
		} else {
			PyErr_SetString(PyExc_TypeError, "None is not a valid string in this context");
			rc = FALSE;
		}
	} else {
		const char *tp_name = stringObject && stringObject->ob_type ? stringObject->ob_type->tp_name : "<NULL!!>";
		PyErr_Format(PyExc_TypeError, "Objects of type '%s' can not be converted to Unicode.", tp_name);
		rc = FALSE;
	}
	if (rc && pResultLen) *pResultLen = resultLen;
	return rc;
}

void PyWinObject_FreeWCHAR(WCHAR *str)
{
	PyMem_Free(str);
}

// Converts a series of consecutive null terminated strings into a list
// Note that a read overflow can result if the input is not properly terminated with an extra NULL.
// Should probably also add a counted version, as win32api uses for REG_MULTI_SZ
PyObject *PyWinObject_FromMultipleString(WCHAR *multistring)
{
	PyObject *obelement, *ret=NULL;
	size_t elementlen;
	if (multistring==NULL){
		Py_INCREF(Py_None);
		return Py_None;
		}
	ret=PyList_New(0);
	if (ret==NULL)
		return NULL;
	elementlen=wcslen(multistring);
	while (elementlen){
		obelement=PyWinObject_FromWCHAR(multistring, elementlen);
		if ((obelement==NULL)||(PyList_Append(ret,obelement)==-1)){
			Py_XDECREF(obelement);
			Py_DECREF(ret);
			return NULL;
			}
		Py_DECREF(obelement);
		multistring+=elementlen+1;
		elementlen=wcslen(multistring);
		}
	return ret;
}

PyObject *PyWinObject_FromMultipleString(char *multistring)
{
	PyObject *obelement, *ret=NULL;
	size_t elementlen;
	if (multistring==NULL){
		Py_INCREF(Py_None);
		return Py_None;
		}
	ret=PyList_New(0);
	if (ret==NULL)
		return NULL;
	elementlen=strlen(multistring);
	while (elementlen){
		obelement=PyString_FromStringAndSize(multistring, elementlen);
		if ((obelement==NULL)||(PyList_Append(ret,obelement)==-1)){
			Py_XDECREF(obelement);
			Py_DECREF(ret);
			return NULL;
			}
		Py_DECREF(obelement);
		multistring+=elementlen+1;
		elementlen=strlen(multistring);
		}
	return ret;
}

// Converts a sequence of str/unicode objects into a series of consecutive null-terminated
//	wide character strings with extra terminating null
BOOL PyWinObject_AsMultipleString(PyObject *ob, char **pmultistring, BOOL bNoneOK, DWORD *chars_returned)
{
	DWORD numStrings, i;
	char **pchars;
	BOOL rc=FALSE;

	*pmultistring=NULL;
	if (chars_returned)
		*chars_returned=0;
	if (!PyWinObject_AsCharArray(ob, &pchars, &numStrings, bNoneOK))
		return FALSE;
	// Shortcut for None
	if (pchars==NULL)
		return TRUE;

	size_t len=numStrings+1;	// One null for each string plus extra terminating null
	// Need to loop twice - once to get the buffer length
	for (i=0;i<numStrings;i++)
		len += strlen(pchars[i]);

	// Allocate the buffer
	*pmultistring = (char *)malloc(len * sizeof(char));
	if (*pmultistring == NULL)
		PyErr_NoMemory();
	else{
		char *p = *pmultistring;
		for (i=0;i<numStrings;i++) {
			strcpy(p, pchars[i]);
			p += strlen(pchars[i]);
			*p++ = '\0';
			}
		*p = '\0'; // Add second terminator.
		rc = TRUE;
		if (chars_returned)
			*chars_returned=len;
		}
	PyWinObject_FreeCharArray(pchars, numStrings);
	return rc;
}

// Converts a sequence of str/unicode objects into a series of consecutive null-terminated
//	char strings with extra terminating null
BOOL PyWinObject_AsMultipleString(PyObject *ob, WCHAR **pmultistring, BOOL bNoneOK, DWORD *chars_returned)
{
	DWORD numStrings, i;
	WCHAR **wchars;
	BOOL rc=FALSE;

	*pmultistring=NULL;
	if (chars_returned)
		*chars_returned=0;
	if (!PyWinObject_AsWCHARArray(ob, &wchars, &numStrings, bNoneOK))
		return FALSE;
	// Shortcut for None
	if (wchars==NULL)
		return TRUE;

	size_t len=numStrings+1;	// One null for each string plus extra terminating null
	// Need to loop twice - once to get the buffer length
	for (i=0;i<numStrings;i++)
		len += wcslen(wchars[i]);

	// Allocate the buffer
	*pmultistring = (WCHAR *)malloc(len * sizeof(WCHAR));
	if (*pmultistring == NULL)
		PyErr_NoMemory();
	else{
		WCHAR *p = *pmultistring;
		for (i=0;i<numStrings;i++) {
			wcscpy(p, wchars[i]);
			p += wcslen(wchars[i]);
			*p++ = L'\0';
			}
		*p = L'\0'; // Add second terminator.
		rc = TRUE;
		if (chars_returned)
			*chars_returned=len;
		}
	PyWinObject_FreeWCHARArray(wchars, numStrings);
	return rc;
}

void PyWinObject_FreeMultipleString(WCHAR *pmultistring)
{
	if (pmultistring)
		free (pmultistring);
}

void PyWinObject_FreeMultipleString(char *pmultistring)
{
	if (pmultistring)
		free (pmultistring);
}

// Converts a aequence of string or unicode objects into an array of WCHAR
void PyWinObject_FreeWCHARArray(LPWSTR *wchars, DWORD str_cnt)
{
	if (wchars!=NULL){
		for (DWORD wchar_index=0; wchar_index<str_cnt; wchar_index++)
			PyWinObject_FreeWCHAR(wchars[wchar_index]);
		free(wchars);
		}
}

BOOL PyWinObject_AsWCHARArray(PyObject *str_seq, LPWSTR **wchars, DWORD *str_cnt, BOOL bNoneOK)
{
	BOOL ret=FALSE;
	PyObject *str_tuple=NULL, *tuple_item;
	DWORD bufsize, tuple_index;
	*wchars=NULL;
	*str_cnt=0;

	if (bNoneOK && str_seq==Py_None)
		return TRUE;
	if ((str_tuple=PyWinSequence_Tuple(str_seq, str_cnt))==NULL)
		return FALSE;
	bufsize=*str_cnt * sizeof(LPWSTR);
	*wchars=(LPWSTR *)malloc(bufsize);
	if (*wchars==NULL){
		PyErr_Format(PyExc_MemoryError, "Unable to allocate %d bytes", bufsize);
		goto done;
		}
	ZeroMemory(*wchars, bufsize);
	for (tuple_index=0;tuple_index<*str_cnt;tuple_index++){
		tuple_item=PyTuple_GET_ITEM(str_tuple, tuple_index);
		if (!PyWinObject_AsWCHAR(tuple_item, &((*wchars)[tuple_index]), FALSE)){
			PyWinObject_FreeWCHARArray(*wchars, *str_cnt);
			*wchars=NULL;
			*str_cnt=0;
			goto done;
			}
		}
	ret=TRUE;
done:
	Py_DECREF(str_tuple);
	return ret;
}

// Converts a aequence of string or unicode objects into an array of char pointers
void PyWinObject_FreeCharArray(char **pchars, DWORD str_cnt)
{
	if (pchars!=NULL){
		for (DWORD pchar_index=0; pchar_index<str_cnt; pchar_index++)
			PyWinObject_FreeString(pchars[pchar_index]);
		free(pchars);
		}
}

BOOL PyWinObject_AsCharArray(PyObject *str_seq, char ***pchars, DWORD *str_cnt, BOOL bNoneOK)
{
	BOOL ret=FALSE;
	PyObject *str_tuple=NULL, *tuple_item;
	DWORD bufsize, tuple_index;
	*pchars=NULL;
	*str_cnt=0;

	if (bNoneOK && str_seq==Py_None)
		return TRUE;
	if ((str_tuple=PyWinSequence_Tuple(str_seq, str_cnt))==NULL)
		return FALSE;
	bufsize=*str_cnt * sizeof(char *);
	*pchars=(char **)malloc(bufsize);
	if (*pchars==NULL){
		PyErr_Format(PyExc_MemoryError, "Unable to allocate %d bytes", bufsize);
		goto done;
		}
	ZeroMemory(*pchars, bufsize);
	for (tuple_index=0;tuple_index<*str_cnt;tuple_index++){
		tuple_item=PyTuple_GET_ITEM(str_tuple, tuple_index);
		if (!PyWinObject_AsString(tuple_item, &((*pchars)[tuple_index]), FALSE)){
			PyWinObject_FreeCharArray(*pchars, *str_cnt);
			*pchars=NULL;
			*str_cnt=0;
			goto done;
			}
		}
	ret=TRUE;
done:
	Py_DECREF(str_tuple);
	return ret;
}

// Copy s null terminated string so that it can be deallocated with PyWinObject_FreeString
WCHAR *PyWin_CopyString(const WCHAR *input){
	size_t len=wcslen(input);
	WCHAR *output=(WCHAR *)PyMem_Malloc((len+1) * sizeof(WCHAR));
	if (output==NULL)
		return NULL;
	return wcsncpy(output, input, len+1);
}

char *PyWin_CopyString(const char *input){
	size_t len=strlen(input);
	char *output=(char *)PyMem_Malloc((len+1) * sizeof(char));
	if (output==NULL)
		return NULL;
	return strncpy(output, input, len+1);
}
