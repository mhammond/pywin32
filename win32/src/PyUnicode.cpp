//
// PyUnicode.cpp -- Unicode string type for Python
//
// @doc

#include "PyWinTypes.h"
#include "PyWinObjects.h"
#include "malloc.h"
#include "tchar.h"

#ifndef MS_WINCE
#include "locale.h"
#endif


#ifndef MS_WINCE

BOOL PyWinObject_AsTaskAllocatedWCHAR(PyObject *stringObject, WCHAR **ppResult, BOOL bNoneOK /*= FALSE*/,DWORD *pResultLen /*= NULL*/)
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
		*ppResult = (LPWSTR)CoTaskMemAlloc(cch*sizeof(WCHAR));
		if (*ppResult)
			/* convert and get the final character size */
			cch = MultiByteToWideChar(CP_ACP, 0, buf, cch, *ppResult, cch);
		if (*ppResult && pResultLen) *pResultLen = cch;
	} else if (PyUnicode_Check(stringObject)) {
		// copy the value, including embedded NULLs
#if defined(PYWIN_USE_PYUNICODE)
		WCHAR *v = (WCHAR *)PyUnicode_AS_UNICODE(stringObject);
		UINT cch = PyUnicode_GET_SIZE(stringObject);
#else
		WCHAR *v = ((PyUnicode *)stringObject)->m_bstrValue;
		UINT cch = SysStringLen(v);
#endif
		*ppResult = (WCHAR *)CoTaskMemAlloc(cch * sizeof(WCHAR));
		if (*ppResult)
			memcpy(*ppResult, v, cch * sizeof(WCHAR));
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
		PyErr_SetString(PyExc_MemoryError, "Allocating WCHAR via CoTaskMemAlloc");
		return FALSE;
	}
	return rc;
}

void PyWinObject_FreeTaskAllocatedWCHAR(WCHAR * str)
{
	CoTaskMemFree(str);
}

#endif /* MS_WINCE */

#if defined(PYWIN_USE_PYUNICODE)

/* Implement our Windows Unicode API using the Python widestring object */
PyObject *PyUnicodeObject_FromString(const char *string)
{
	if (string==NULL) {
		Py_INCREF(Py_None);
		return Py_None;
	}
	return (PyObject *)PyUnicode_DecodeMBCS(string, strlen(string), NULL);
}

// Convert a WCHAR string to "char *"
//  If len is known, pass it, else -1
// NOTE - string must be freed with PyWinObject_FreeString
BOOL PyWin_WCHAR_AsString(WCHAR *input, DWORD inLen, char **pResult)
{
	if (inLen==-1)
		inLen = wcslen(input);
	// convert from string len to include terminator.
	inLen++;
	char *buf = (char *)PyMem_Malloc(inLen);

	DWORD len = WideCharToMultiByte(CP_ACP, 0, input, inLen, buf, inLen, NULL, NULL);
	if (len==0) {
		PyMem_Free(buf);
		PyWin_SetAPIError("WideCharToMultiByte");
		return FALSE;
	}
	*pResult = buf;
	return TRUE;
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
		stringObject = tempObject = PyUnicode_EncodeMBCS(PyUnicode_AS_UNICODE(stringObject), PyUnicode_GET_SIZE(stringObject), NULL);

	if (!PyString_Check(stringObject)) {
		PyErr_SetString(PyExc_TypeError, "The object must be a string or unicode object");
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

// Convert a "char *" string to "WCHAR *"
//  If len is known, pass it, else -1
// NOTE - string must be freed with PyWinObject_FreeString
BOOL PyWin_String_AsWCHAR(char *input, DWORD inLen, WCHAR **pResult)
{
	if (inLen==(DWORD)-1)
		inLen = strlen(input);
	inLen += 1; // include NULL term in all ops
	/* use MultiByteToWideChar() to see how much we need. */
	/* NOTE: this will include the null-term in the length */
	DWORD cchWideChar = MultiByteToWideChar(CP_ACP, 0, input, inLen, NULL, 0);
	// alloc the buffer
	*pResult = (WCHAR *)PyMem_Malloc(cchWideChar * sizeof(WCHAR));
	if (*pResult==NULL) {
		PyErr_SetString(PyExc_MemoryError, "Not enough memory to allocate wide string buffer.");
		return FALSE;
	}
	/* do the conversion */
   	if (0==MultiByteToWideChar(CP_ACP, 0, input, inLen, *pResult, cchWideChar)) {
		PyMem_Free(*pResult);
		PyWin_SetAPIError("MultiByteToWideChar");
		return FALSE;
	}
	return TRUE;
}

void PyWinObject_FreeString(char *str)
{
	PyMem_Free(str);
}
void PyWinObject_FreeString(WCHAR *str)
{
	PyMem_Free(str);
}

// Convert a "char *" to a BSTR - free via ::SysFreeString()
BSTR PyWin_String_AsBstr(const char *value)
{
	if (value==NULL || *value=='\0')
		return SysAllocStringLen(L"", 0);
	/* use MultiByteToWideChar() as a "good" strlen() */
	/* NOTE: this will include the null-term in the length */
	int cchWideChar = MultiByteToWideChar(CP_ACP, 0, value, -1, NULL, 0);

	/* alloc a temporary conversion buffer, but dont use alloca, as super
	   large strings will blow our stack */
	LPWSTR wstr = (LPWSTR)malloc(cchWideChar * sizeof(WCHAR));
	if (wstr==NULL) {
		PyErr_SetString(PyExc_MemoryError, "Not enough memory to allocate wide string buffer.");
		return NULL;
	}

	/* convert the input into the temporary buffer */
   	MultiByteToWideChar(CP_ACP, 0, value, -1, wstr, cchWideChar);

	/* don't place the null-term into the BSTR */
	BSTR ret = SysAllocStringLen(wstr, cchWideChar - 1);
	if (ret==NULL)
		PyErr_SetString(PyExc_MemoryError, "allocating BSTR");
	free(wstr);
	return ret;
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

PyObject *PyString_FromUnicode( const OLECHAR *str )
{
	if (str==NULL) {
		Py_INCREF(Py_None);
		return Py_None;
	}
	PyObject *uo = PyWinObject_FromOLECHAR(str);
	if (uo==NULL) return NULL;
	PyObject *ret = PyUnicode_EncodeMBCS(PyUnicode_AS_UNICODE(uo), PyUnicode_GET_SIZE(uo), NULL);
	Py_DECREF(uo);
	return ret;
}

int PyUnicode_Size(PyObject *op)
{
	if (!PyUnicode_Check(op)) {
		PyErr_BadInternalCall();
		return -1;
	}
	return PyUnicode_GET_SIZE(op);
}

#ifndef NO_PYWINTYPES_BSTR
PyObject *PyWinObject_FromBstr(const BSTR bstr, BOOL takeOwnership /*=FALSE*/)
{
	PyObject *ret = PyUnicode_FromWideChar(bstr, SysStringLen(bstr));
	if (takeOwnership) SysFreeString(bstr);
	return ret;
}
#endif // NO_PYWINTYPES_BSTR

#else /* not PYWIN_USE_PYUNICODE */

PyObject *PyString_FromUnicode( const OLECHAR *str )
{
	char *buf;
	if (str==NULL) {
		Py_INCREF(Py_None);
		return Py_None;
	}
	if (!PyWin_WCHAR_AsString((OLECHAR *)str, -1, &buf))
		return NULL;
	PyObject *ret = PyString_FromString(buf);
	PyWinObject_FreeString(buf);
	return ret;
}

PyObject *PyUnicodeObject_FromString(const char *string)
{
	return new PyUnicode(string);
}

int PyUnicode_Size(PyObject *op)
{
	if (!PyUnicode_Check(op)) {
		PyErr_BadInternalCall();
		return -1;
	}
	return PyUnicode::lengthFunc((PyUnicode *)op);
}

// Moved to a #define as it clashes with the new standard Python function
/**
WCHAR *PyUnicode_AsUnicode(PyObject *op)
{
	return ((PyUnicode *)op)->m_bstrValue;
}
**/


// Convert a WCHAR string to "char *"
//  If len is known, pass it, else -1
// NOTE - string must be freed with PyWinObject_FreeString
BOOL PyWin_WCHAR_AsString(WCHAR *input, DWORD inLen, char **pResult)
{
	if (inLen==-1)
		inLen = wcslen(input);
	// convert from string len to include terminator.
	inLen++;
	char *buf = (char *)PyMem_Malloc(inLen);

	DWORD len = WideCharToMultiByte(CP_ACP, 0, input, inLen, buf, inLen, NULL, NULL);
	if (len==0) {
		PyMem_Free(buf);
		PyWin_SetAPIError("WideCharToMultiByte");
		return FALSE;
	}
	*pResult = buf;
	return TRUE;
}

BOOL PyWin_Bstr_AsString(BSTR input, char **pResult)
{
	DWORD wideSize = SysStringLen(input);
	return PyWin_WCHAR_AsString(input, wideSize, pResult);
}

// Convert a Python object to a "char *" - allow embedded NULLs, None, etc.
BOOL PyWinObject_AsString(PyObject *stringObject, char **pResult, BOOL bNoneOK /*= FALSE*/, DWORD *pResultLen /* = NULL */)
{
	int strLen;
	BOOL rc = TRUE;
	if (PyString_Check(stringObject)) {
		strLen = PyString_Size(stringObject);
		*pResult = (char *)PyMem_Malloc((strLen + 1) * sizeof(char));
		if (*pResult==NULL) {
			PyErr_SetString(PyExc_MemoryError, "copying string");
			return FALSE;
		}
		memcpy(*pResult, PyString_AsString(stringObject), strLen);
		(*pResult)[strLen] = '\0';
	} else if (PyUnicode_Check(stringObject)) {
			strLen = SysStringLen(((PyUnicode *)stringObject)->m_bstrValue);
			rc = PyWin_Bstr_AsString(((PyUnicode *)stringObject)->m_bstrValue, pResult);
	} else if (stringObject == Py_None) {
		strLen = 0;
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
		PyErr_SetString(PyExc_MemoryError, "Allocating string");
		return FALSE;
	}
	if (rc && pResultLen) *pResultLen = strLen;
	return rc;
}

void PyWinObject_FreeString(char *str)
{
	PyMem_Free(str);
}



// PyWinObject_FromBstr - convert a BSTR into a Python string.
//
// ONLY USE THIS FOR TRUE BSTR's - Use the fn below for OLECHAR *'s.
// NOTE - does not use standard macros, so NULLs get through!
PyObject *PyWinObject_FromBstr(const BSTR bstr, BOOL takeOwnership/*=FALSE*/)
{
	if (bstr==NULL) {
		Py_INCREF(Py_None);
		return Py_None;
	}
	return new PyUnicode(bstr, takeOwnership);
}

// Size info is available (eg, a fn returns a string and also fills in a size variable)
PyObject *PyWinObject_FromOLECHAR(const OLECHAR * str, int numChars)
{
	if (str==NULL) {
		Py_INCREF(Py_None);
		return Py_None;
	}
	return new PyUnicode(str, numChars);
}

// No size info avail.
PyObject *PyWinObject_FromOLECHAR(const OLECHAR * str)
{
	if (str==NULL) {
		Py_INCREF(Py_None);
		return Py_None;
	}
	return new PyUnicode(str);
}


// @object PyUnicode|A Python object, representing a Unicode string.
// @comm A PyUnicode object is used primarily when exchanging string
// information across a COM interface.


static PySequenceMethods PyUnicode_SequenceMethods = {
	(inquiry)PyUnicode::lengthFunc,			/*sq_length*/
	(binaryfunc)PyUnicode::concatFunc,		/*sq_concat*/
	(intargfunc)PyUnicode::repeatFunc,		/*sq_repeat*/
	(intargfunc)PyUnicode::itemFunc,		/*sq_item*/
	(intintargfunc)PyUnicode::sliceFunc,	/*sq_slice*/
	0,		/*sq_ass_item*/
	0,		/*sq_ass_slice*/
};

PYWINTYPES_EXPORT PyTypeObject PyUnicodeType =
{
	PyObject_HEAD_INIT(&PyType_Type)
	0,
	"PyUnicode",
	sizeof(PyUnicode),
	0,
	PyUnicode::deallocFunc,		/* tp_dealloc */
	// @pymeth __print__|Used when the object is printed.
	PyUnicode::printFunc,		/* tp_print */
	PyUnicode::getattrFunc,		/* tp_getattr */
	0,						/* tp_setattr */
	// @pymeth __cmp__|Used when Unicode objects are compared.
	PyUnicode::compareFunc,	/* tp_compare */
	// @pymeth __repr__|Used when repr(object) is used.
	PyUnicode::reprFunc,	/* tp_repr */
	0,						/* tp_as_number */
	&PyUnicode_SequenceMethods,	/* tp_as_sequence */
	0,						/* tp_as_mapping */
	// @pymeth __hash__|Used when the hash value of an object is required
	PyUnicode::hashFunc,		/* tp_hash */
	0,						/* tp_call */
	// @pymeth __str__|Used when an (8-bit) string representation is required
	PyUnicode::strFunc,		/* tp_str */
};

PyUnicode::PyUnicode(void)
{
	ob_type = &PyUnicodeType;
	_Py_NewReference(this);

	m_bstrValue = NULL;
}

PyUnicode::PyUnicode(const char *value)
{
	ob_type = &PyUnicodeType;
	_Py_NewReference(this);
	m_bstrValue = PyWin_String_AsBstr(value);
}

PyUnicode::PyUnicode(const char *value, unsigned int numBytes)
{
	ob_type = &PyUnicodeType;
	_Py_NewReference(this);

	m_bstrValue = SysAllocStringByteLen(value, numBytes);
}

PyUnicode::PyUnicode(const OLECHAR *value)
{
	ob_type = &PyUnicodeType;
	_Py_NewReference(this);

	m_bstrValue = SysAllocString(value);
}

PyUnicode::PyUnicode(const OLECHAR *value, int numChars)
{
	ob_type = &PyUnicodeType;
	_Py_NewReference(this);

	m_bstrValue = SysAllocStringLen(value, numChars);
}

PyUnicode::PyUnicode(const BSTR value, BOOL takeOwnership /* = FALSE */)
{
	ob_type = &PyUnicodeType;
	_Py_NewReference(this);

	if ( takeOwnership )
		m_bstrValue = value;
	else
		// copy the value, including embedded NULLs
		m_bstrValue = SysAllocStringLen(value, SysStringLen(value));
}

PyUnicode::PyUnicode(PyObject *value)
{
	ob_type = &PyUnicodeType;
	_Py_NewReference(this);

	m_bstrValue = NULL;
	(void)PyWinObject_AsBstr(value, &m_bstrValue);
}

PyUnicode::~PyUnicode(void)
{
	SysFreeString(m_bstrValue);
}

int PyUnicode::compare(PyObject *ob)
{
	int l1 = SysStringByteLen(m_bstrValue);
	OLECHAR *s = ((PyUnicode *)ob)->m_bstrValue;
	int l2 = SysStringByteLen(s);
	if ( l1 == 0 )
		if ( l2 > 0 )
			return -1;
		else
			return 0;
	if ( l2 == 0 && l1 > 0 )
		return 1;
	int len = l1;
	if ( l2 < l1 )
		len = l2;
	int cmp = memcmp(m_bstrValue, s, len);
	if ( cmp == 0 )
	{
		if ( l1 < l2 )
			return -1;
		return l1 > l2;
	}
	return cmp;
}

PyObject * PyUnicode::concat(PyObject *ob)
{
	if ( !PyUnicode_Check(ob) ) {
		PyErr_SetString(PyExc_TypeError, "illegal argument type for PyUnicode concatenation");
		return NULL;
	}

	BSTR s2 = ((PyUnicode *)ob)->m_bstrValue;
	int l1 = SysStringLen(m_bstrValue);
	int l2 = SysStringLen(s2);
	BSTR bres = SysAllocStringLen(NULL, l1 + l2);
	if ( m_bstrValue )
		memcpy(bres, m_bstrValue, l1 * sizeof(*bres));
	if ( s2 )
		memcpy(&bres[l1], s2, l2 * sizeof(*bres));
	bres[l1+l2] = L'\0';
	return new PyUnicode(bres, /* takeOwnership= */ TRUE);
}

PyObject * PyUnicode::repeat(int count)
{
	int l = SysStringLen(m_bstrValue);
	if ( l == 0 )
		return new PyUnicode();

	BSTR bres = SysAllocStringLen(NULL, l * count);
	OLECHAR *p = bres;
	for ( int i = count; i--; p += l )
		memcpy(p, m_bstrValue, l * sizeof(*p));
	bres[l*count] = L'\0';
	return new PyUnicode(bres, /* takeOwnership= */ TRUE);
}

PyObject * PyUnicode::item(int index)
{
	int l = SysStringLen(m_bstrValue);
	if ( index < 0 || index >= l )
	{
		PyErr_SetString(PyExc_IndexError, "unicode index out of range");
		return NULL;
	}
	// ### tricky to get the correct constructor
	return new PyUnicode((const OLECHAR *)&m_bstrValue[index], (int)1);
}

PyObject * PyUnicode::slice(int start, int end)
{
	int l = SysStringLen(m_bstrValue);
	if ( start < 0 )
		start = 0;
	if ( end < 0 )
		end = 0;
	if ( end > l )
		end = l;
	if ( start == 0 && end == l )
	{
		Py_INCREF(this);
		return this;
	}
	if ( end <= start )
		return new PyUnicode();

	BSTR bres = SysAllocStringLen(&m_bstrValue[start], end - start);
	return new PyUnicode(bres, /* takeOwnership= */ TRUE);
}

long PyUnicode::hash(void)
{
	/* snarfed almost exactly from stringobject.c */

	int orig_len = SysStringByteLen(m_bstrValue);
	register int len = orig_len;
	register unsigned char *p;
	register long x;

	p = (unsigned char *)m_bstrValue;
	x = *p << 7;
	while (--len >= 0)
		x = (1000003*x) ^ *p++;
	x ^= orig_len;
	if (x == -1)
		x = -2;
	return x;
}

PyObject * PyUnicode::asStr(void)
{
	if ( m_bstrValue == NULL )
		return PyString_FromString("");

	/*
	** NOTE: we always provide lengths to avoid computing null-term and
	** and to carry through any NULL values.
	*/

	/* how many chars (including nulls) are in the BSTR? */
	int cchWideChar = SysStringLen(m_bstrValue);

	/* get the output length */
	int cchMultiByte = WideCharToMultiByte(CP_ACP, 0, m_bstrValue, cchWideChar,
										   NULL, 0, NULL, NULL);

	/* Create the Python string, and use it as the conversion buffer. */
	PyObject *result = PyString_FromStringAndSize(NULL, cchMultiByte);
	if (result==NULL) return NULL;

	/* do the conversion */
   	WideCharToMultiByte(CP_ACP, 0, m_bstrValue, cchWideChar, PyString_AS_STRING((PyStringObject *)result),
						cchMultiByte, NULL, NULL);

	/* return the Python object */
	return result;
}

int PyUnicode::print(FILE *fp, int flags)
{
	LPSTR s;
	if ( m_bstrValue )
	{
		/* NOTE: BSTR values are *always* null-termed */
		int numBytes = WideCharToMultiByte(CP_ACP, 0, m_bstrValue, -1, NULL, 0, NULL, NULL);
		s = (LPSTR)alloca(numBytes+1);
		WideCharToMultiByte(CP_ACP, 0, m_bstrValue, -1, s, numBytes, NULL, NULL);
	}
	else
		s = NULL;

//	USES_CONVERSION;
//	char *s = W2A(m_bstrValue);
	TCHAR resBuf[80];

	if ( s == NULL )
		_tcscpy(resBuf, _T("<PyUnicode: NULL>"));
	else if ( strlen(s) > 40 )
	{
		s[40] = '\0';
		wsprintf(resBuf, _T("<PyUnicode: '%s'...>"), s);
	}
	else
		wsprintf(resBuf, _T("<PyUnicode: '%s'>"), s);

	//
    // ### ACK! Python uses a non-debug runtime. We can't use stream
	// ### functions when in DEBUG mode!!  (we link against a different
	// ### runtime library)  Hack it by getting Python to do the print!
	//
	// ### - Double Ack - Always use the hack!
// #ifdef _DEBUG
	PyObject *ob = PyString_FromTCHAR(resBuf);
	PyObject_Print(ob, fp, flags|Py_PRINT_RAW);
	Py_DECREF(ob);
/***#else
	fputs(resBuf, fp);
#endif
***/
	return 0;
}

PyObject *PyUnicode::repr()
{
	// Do NOT write an "L" - Python opted for "u" anyway,
	// and pre 2.0 builds work nicer if we just pretend we are a string in repr.
	PyObject *obStr = asStr();
	if (obStr==NULL)
		return NULL;
	PyObject *obRepr = PyObject_Repr(obStr);
	Py_DECREF(obStr);
	return obRepr;
/***
	// This is not quite correct, but good enough for now.
	// To save me lots of work, I convert the Unicode to a temporary
	// string object, then perform a repr on the string object, then
	// simply prefix with an 'L' to indicate the string is Unicode.
	PyObject *obStr = asStr();
	if (obStr==NULL)
		return NULL;
	PyObject *obRepr = PyObject_Repr(obStr);
	Py_DECREF(obStr);
	if (obRepr==NULL)
		return NULL;

	char *szVal = PyString_AsString(obRepr);
	int strSize = PyString_Size(obRepr);
	char *buffer = (char *)alloca(strSize+2); // trailing NULL and L
	buffer[0] = 'L';
	memcpy(buffer+1, szVal, strSize);
	buffer[strSize+1] = '\0';
	Py_DECREF(obRepr);
	return PyString_FromStringAndSize(buffer, strSize+1);
***/
}

PyObject * PyUnicode::upper(void)
{
	/* copy the value; don't worry about NULLs since _wcsupr doesn't */
	BSTR v = SysAllocString(m_bstrValue);

	/* upper-case the thing */
	if ( v )
	{
#ifndef MS_WINCE
		setlocale(LC_CTYPE, "");
#endif
		_wcsupr(v);
	}

	/* wrap it into a new object and return it */
	return new PyUnicode(v, /* takeOwnership= */ TRUE);
}

PyObject * PyUnicode::lower(void)
{
	/* copy the value; don't worry about NULLs since _wcsupr doesn't */
	BSTR v = SysAllocString(m_bstrValue);

	/* upper-case the thing */
	if ( v )
	{
#ifndef MS_WINCE
		setlocale(LC_CTYPE, "");
#endif
		_wcslwr(v);
	}

	/* wrap it into a new object and return it */
	return new PyUnicode(v, /* takeOwnership= */ TRUE);
}

static struct PyMethodDef PyUnicode_methods[] = {
	{ "upper",	PyUnicode::upperFunc,	METH_VARARGS },
	{ "lower",	PyUnicode::lowerFunc,	METH_VARARGS },
	{ NULL,		NULL }		/* sentinel */
};

PyObject * PyUnicode::getattr(char *name)
{
	if ( !strcmp(name, "raw") )
	{
		if ( m_bstrValue == NULL )
			return PyString_FromString("");

		int len = SysStringByteLen(m_bstrValue);
		return PyString_FromStringAndSize((char *)(void *)m_bstrValue, len);
	}

	return Py_FindMethod(PyUnicode_methods, this, name);
}

/*static*/ void PyUnicode::deallocFunc(PyObject *ob)
{
	delete (PyUnicode *)ob;
}

// @pymethod int|PyUnicode|__cmp__|Used when objects are compared.
int PyUnicode::compareFunc(PyObject *ob1, PyObject *ob2)
{
	return ((PyUnicode *)ob1)->compare(ob2);
}

// @pymethod int|PyUnicode|__hash__|Used when the hash value of a Unicode object is required
long PyUnicode::hashFunc(PyObject *ob)
{
	return ((PyUnicode *)ob)->hash();
}

// @pymethod |PyUnicode|__str__|Used when a (8-bit) string representation of the Unicode object is required.
 PyObject * PyUnicode::strFunc(PyObject *ob)
{
	return ((PyUnicode *)ob)->asStr();
}

// @pymethod |PyUnicode|__print__|Used when the Unicode object is printed.
int PyUnicode::printFunc(PyObject *ob, FILE *fp, int flags)
{
	return ((PyUnicode *)ob)->print(fp, flags);
}

// @pymethod |PyUnicode|__repr__|Used when repr(object) is used.
PyObject *PyUnicode::reprFunc(PyObject *ob)
{
	// @comm Note the format is L'string' and that the string portion
	// is currently not escaped, as Python does for normal strings.
	return ((PyUnicode *)ob)->repr();
}

// @pymethod |PyUnicode|__getattr__|Used to access attributes of the Unicode object.
PyObject * PyUnicode::getattrFunc(PyObject *ob, char *name)
{
	return ((PyUnicode *)ob)->getattr(name);
}

int PyUnicode::lengthFunc(PyObject *ob)
{
	return SysStringLen(((PyUnicode *)ob)->m_bstrValue);
}

PyObject * PyUnicode::concatFunc(PyObject *ob1, PyObject *ob2)
{
	return ((PyUnicode *)ob1)->concat(ob2);
}

PyObject * PyUnicode::repeatFunc(PyObject *ob, int count)
{
	return ((PyUnicode *)ob)->repeat(count);
}

PyObject * PyUnicode::itemFunc(PyObject *ob, int index)
{
	return ((PyUnicode *)ob)->item(index);
}

PyObject * PyUnicode::sliceFunc(PyObject *ob, int start, int end)
{
	return ((PyUnicode *)ob)->slice(start, end);
}

PyObject * PyUnicode::upperFunc(PyObject *ob, PyObject *args)
{
    if ( !PyArg_ParseTuple(args, ":upper"))
       return NULL;
	return ((PyUnicode *)ob)->upper();
}

PyObject * PyUnicode::lowerFunc(PyObject *ob, PyObject *args)
{
    if ( !PyArg_ParseTuple(args, ":lower"))
       return NULL;
	return ((PyUnicode *)ob)->lower();
}

#endif /* PYWIN_USE_PYUNICODE */

///////////////////////////////////////////////////////////
//
// Some utilities etc
#ifndef NO_PYWINTYPES_BSTR

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
#if defined(PYWIN_USE_PYUNICODE)
		wchar_t *v = (wchar_t *)PyUnicode_AS_UNICODE(stringObject);
		*pResult = SysAllocStringLen(v, PyUnicode_GET_SIZE(stringObject));
#else
		BSTR v = ((PyUnicode *)stringObject)->m_bstrValue;
		*pResult = SysAllocStringLen(v, SysStringLen(v));
#endif
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

#endif // NO_PYWINTYPES_BSTR

// String conversions
// Convert a Python object to a WCHAR - allow embedded NULLs, None, etc.
BOOL PyWinObject_AsWCHAR(PyObject *stringObject, WCHAR **pResult, BOOL bNoneOK /*= FALSE*/,DWORD *pResultLen /*= NULL*/)
{
	BOOL rc = TRUE;
	int resultLen = 0;
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
	else if (PyUnicode_Check(stringObject))
	{
		resultLen = PyUnicode_Size(stringObject);
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
