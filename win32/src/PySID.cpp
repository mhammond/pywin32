//
// @doc

#include "PyWinTypes.h"
#include "PyWinObjects.h"
#include "PySecurityObjects.h"

#ifndef NO_PYWINTYPES_SECURITY

// @pymethod <o PySID>|pywintypes|SID|Creates a new SID object
PyObject *PyWinMethod_NewSID(PyObject *self, PyObject *args)
{
	void *buf = NULL;
	int bufSize = 32; // xxxxxx64 - should be Py_ssize_t - but passed as 'i'
	// @pyparm int|bufSize|32|Size for the SID buffer
	if (!PyArg_ParseTuple(args, "|i:SID", &bufSize)) {
		PyErr_Clear();
		// @pyparmalt1 string|buffer||A raw data buffer, assumed to hold the SID data.
		if (!PyArg_ParseTuple(args, "s#:SID", &buf, &bufSize)) {
			/* Special case for one step setup of the SID */
			PyErr_Clear();
			// @pyparmalt2 <o SID_IDENTIFIER_AUTHORITY>|idAuthority||The identifier authority.
			// @pyparmalt2 [int, ...]|subAuthorities||A list of sub authorities.
			SID_IDENTIFIER_AUTHORITY sid_ia;
			PyObject *obSubs, *obSubsTuple;
			unsigned long sub0, sub1, sub2, sub3, sub4, sub5, sub6, sub7;

			if (!PyArg_ParseTuple(args, "(bbbbbb)O:SID", 
				&sid_ia.Value[0], &sid_ia.Value[1],&sid_ia.Value[2],
				&sid_ia.Value[3],&sid_ia.Value[4],&sid_ia.Value[5],
				&obSubs))
				return NULL;
			if (!PySequence_Check(obSubs)) {
				PyErr_SetString(PyExc_TypeError, "sub authorities must be a sequence of integers.");
				return NULL;
			}
			Py_ssize_t numSubs = PySequence_Length(obSubs);
			if (numSubs>8) {
				PyErr_SetString(PyExc_TypeError, "sub authorities sequence size must be <= 8");
				return NULL;
			}
			obSubsTuple=PySequence_Tuple(obSubs);
			if (!obSubsTuple)
				return NULL;
			BOOL bSuccess=PyArg_ParseTuple(obSubsTuple, "|llllllll:SID",
				&sub0, &sub1, &sub2, &sub3, &sub4, &sub5, &sub6, &sub7);
			Py_DECREF(obSubsTuple);
			if (!bSuccess){
				PyErr_SetString(PyExc_TypeError, "sub authorities must be a sequence of integers.");
				return NULL;
				}
			PSID pNew;
			if (!AllocateAndInitializeSid(&sid_ia, (BYTE)numSubs, sub0, sub1, sub2, sub3, sub4, sub5, sub6, sub7, &pNew))
				return PyWin_SetAPIError("AllocateAndInitializeSid");
			return new PySID(pNew);
		}
	}
	return new PySID(bufSize, buf);
}

BOOL PyWinObject_AsSID(PyObject *ob, PSID *ppSID, BOOL bNoneOK /*= TRUE*/)
{
	if (bNoneOK && ob==Py_None) {
		*ppSID = NULL;
	} else if (!PySID_Check(ob)) {
		PyErr_SetString(PyExc_TypeError, "The object is not a PySID object");
		return FALSE;
	} else {
		*ppSID = ((PySID *)ob)->GetSID();
	}
	return TRUE;
}

PyObject *PyWinObject_FromSID(PSID pSID)
{
	if (pSID==NULL) {
		Py_INCREF(Py_None);
		return Py_None;
	}
	return new PySID(pSID);
}

// @pymethod |PySID|Initialize|Initialize the SID.
PyObject *PySID::Initialize(PyObject *self, PyObject *args)
{
	PySID *This = (PySID *)self;
	byte cnt;
	// @pyparm <o SID_IDENTIFIER_AUTHORITY>|idAuthority||The identifier authority.
	// @pyparm int|numSubauthorities||The number of sub authorities to allocate.
	SID_IDENTIFIER_AUTHORITY sid_ia;
	if (!PyArg_ParseTuple(args, "(bbbbbb)b:Initialize", 
		&sid_ia.Value[0], &sid_ia.Value[1],&sid_ia.Value[2],
		&sid_ia.Value[3],&sid_ia.Value[4],&sid_ia.Value[5],
		&cnt))
		return NULL;
	if (!InitializeSid(This->GetSID(), &sid_ia, cnt))
		return PyWin_SetAPIError("InitializeSid");
	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod |PySID|IsValid|Determines if the SID is valid.
PyObject *PySID::IsValid(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ":IsValid"))
		return NULL;
	PySID *This = (PySID *)self;
	return PyBool_FromLong( IsValidSid(This->GetSID()) );
}

// @pymethod int|PySID|GetSubAuthority|Returns specified subauthority from SID
PyObject *PySID::GetSubAuthority(PyObject *self, PyObject *args)
{
	DWORD subauthInd;
	PSID psid;
	if (!PyArg_ParseTuple(args, "i:GetSubAuthority", &subauthInd))
		return NULL;
	PySID *This = (PySID *)self;
	psid = This->GetSID();

	if (subauthInd<0 || subauthInd >= *::GetSidSubAuthorityCount(psid)) {
		PyErr_SetString(PyExc_ValueError, "The index is out of range");
		return NULL;
	}
	return PyInt_FromLong(*GetSidSubAuthority(psid, subauthInd));
}

// @pymethod int|PySID|GetLength|return length of SID (GetLengthSid).
PyObject *PySID::GetLength(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ":GetLength"))
		return NULL;
	PySID *This = (PySID *)self;
	return PyInt_FromLong( GetLengthSid(This->GetSID()) );
}

// @pymethod int|PySID|GetSubAuthorityCount|return nbr of subauthorities from SID
PyObject *PySID::GetSubAuthorityCount(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ":GetSubAuthorityCount"))
		return NULL;
	PySID *This = (PySID *)self;
	return PyInt_FromLong(*::GetSidSubAuthorityCount(This->GetSID()));
}

// @pymethod |PySID|SetSubAuthority|Sets a SID SubAuthority
// @comm See the function SetSidSubAuthority
PyObject *PySID::SetSubAuthority(PyObject *self, PyObject *args)
{
	PySID *This = (PySID *)self;
	int num;
	long val;
	// @pyparm int|index||The index of the sub authority to set
	// @pyparm int|val||The value for the sub authority
	if (!PyArg_ParseTuple(args, "il", &num, &val))
		return NULL;
	if (num<0 || num>=*::GetSidSubAuthorityCount(This->GetSID())) {
		PyErr_SetString(PyExc_ValueError, "The index is out of range");
		return NULL;
	}
	*GetSidSubAuthority(This->GetSID(), num) = val;
	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod (int,int,int,int,int,int)|PySID|GetSidIdentifierAuthority|Returns a tuple of 6 SID_IDENTIFIER_AUTHORITY constants
PyObject *PySID::GetSidIdentifierAuthority (PyObject *self, PyObject *args)
{
	PySID *This = (PySID *)self;
	if (!IsValidSid(This->GetSID())){
		PyErr_SetString(PyExc_ValueError, "GetSidIdentifierAuthority: Invalid SID in object");
		return NULL;
		}

	SID_IDENTIFIER_AUTHORITY *psia;  //wtf is this thing ?  Give it back to the user, let *him* figure it out
	psia = ::GetSidIdentifierAuthority(This->GetSID());
    return Py_BuildValue("(BBBBBB)",psia->Value[0],psia->Value[1],psia->Value[2],psia->Value[3],psia->Value[4],psia->Value[5]);
}

// @object PySID|A Python object, representing a SID structure
struct PyMethodDef PySID::methods[] = {
	{"Initialize",     PySID::Initialize, 1}, 	// @pymeth Initialize|Initialize the SID.
	{"IsValid",        PySID::IsValid, 1}, 	// @pymeth IsValid|Determines if the SID is valid.
	{"SetSubAuthority",PySID::SetSubAuthority, 1}, 	// @pymeth SetSubAuthority|Sets a SID SubAuthority
	{"GetLength",      PySID::GetLength, 1}, // @pymeth GetLength|Return length of sid (GetLengthSid)
	{"GetSubAuthorityCount",   PySID::GetSubAuthorityCount, 1}, // @pymeth GetSubAuthorityCount|Return nbr of subauthorities from SID	
	{"GetSubAuthority",PySID::GetSubAuthority, 1}, // @pymeth GetSubAuthority|Return specified subauthory from SID	
	{"GetSidIdentifierAuthority",PySID::GetSidIdentifierAuthority, 1}, // @pymeth GetSidIdentifierAuthority|Return identifier for the authority who issued the SID (one of the SID_IDENTIFIER_AUTHORITY constants)
	{NULL}
};



#if (PY_VERSION_HEX < 0x03000000)
/*static*/ Py_ssize_t PySID::getreadbuf(PyObject *self, Py_ssize_t index, void **ptr)
{
	if ( index != 0 ) {
		PyErr_SetString(PyExc_SystemError,
				"accessing non-existent SID segment");
		return -1;
	}
	PySID *pysid = (PySID *)self;
	*ptr = pysid->m_psid;
	return GetLengthSid(pysid->m_psid);
}

/*static*/ Py_ssize_t PySID::getsegcount(PyObject *self, Py_ssize_t *lenp)
{
	if ( lenp )
		*lenp = GetLengthSid(((PySID *)self)->m_psid);
	return 1;
}

static PyBufferProcs PySID_as_buffer = {
	PySID::getreadbuf,
	0,
	PySID::getsegcount,
	0,
};

#else	// New buffer interface in Py3k

/*static*/ int PySID::getbufferinfo(PyObject *self, Py_buffer *view, int flags)
{
	PySID *pysid = (PySID *)self;
	return PyBuffer_FillInfo(view, self, pysid->m_psid, GetLengthSid(pysid->m_psid), 1, flags);
}

static PyBufferProcs PySID_as_buffer = {
	PySID::getbufferinfo,
	NULL,	// Does not have any allocated mem in Py_buffer struct 
};

#endif	// PY_VERSION_HEX < 0x03000000

PYWINTYPES_EXPORT PyTypeObject PySIDType =
{
	PYWIN_OBJECT_HEAD
	"PySID",
	sizeof(PySID),
	0,
	PySID::deallocFunc,		/* tp_dealloc */
	0,						/* tp_print */
	0,						/* tp_getattr */
	0,						/* tp_setattr */
	0,						/* tp_compare */
	0,						/* tp_repr */
	0,						/* tp_as_number */
	0,						/* tp_as_sequence */
	0,						/* tp_as_mapping */
	0,
	0,						/* tp_call */
	PySID::strFunc,			/* tp_str */
	PyObject_GenericGetAttr,	/*tp_getattro*/
	0,						/*tp_setattro*/
	// @comm Note the PySID object supports the buffer interface.  Thus buffer(sid) can be used to obtain the raw bytes.
	&PySID_as_buffer,		/*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,	/* tp_flags */
	0,						/* tp_doc */
	0,						/* tp_traverse */
	0,						/* tp_clear */
	PySID::richcompareFunc,				/* tp_richcompare */
	0,						/* tp_weaklistoffset */
	0,						/* tp_iter */
	0,						/* tp_iternext */
	PySID::methods,			/* tp_methods */
	0,						/* tp_members */
	0,						/* tp_getset */
	0,						/* tp_base */
	0,						/* tp_dict */
	0,						/* tp_descr_get */
	0,						/* tp_descr_set */
	0,						/* tp_dictoffset */
	0,						/* tp_init */
	0,						/* tp_alloc */
	0,						/* tp_new */
};


PySID::PySID(int bufSize, void *buf /* = NULL */)
{
	ob_type = &PySIDType;
	_Py_NewReference(this);
	m_psid = (PSID)malloc(bufSize);
	if (buf==NULL)
		memset(m_psid, 0, bufSize);
	else
		memcpy(m_psid, buf, bufSize);
}

PySID::PySID(PSID pOther)
{
	ob_type = &PySIDType;
	_Py_NewReference(this);
	/* always Take my own copy */
	DWORD size = GetLengthSid(pOther);
	m_psid = (PSID)malloc(size);
	CopySid(size, m_psid, pOther);
}

PySID::~PySID()
{
	if (m_psid)
		free(m_psid);
}

PyObject *PySID::richcompare(PyObject *other, int op)
{
	if (!PySID_Check(other)) {
		Py_INCREF(Py_NotImplemented);
		return Py_NotImplemented;
	}
	PSID p2;
	if (!PyWinObject_AsSID(other, &p2, FALSE))
		return NULL;
	BOOL e = EqualSid(GetSID(), p2);
	PyObject *ret;
	if (op==Py_EQ)
		ret = e ? Py_True : Py_False;
	else if (op==Py_NE)
		ret = !e ? Py_True : Py_False;
	else
		ret = Py_NotImplemented;
	Py_INCREF(ret);
	return ret;
}

PyObject *PySID::richcompareFunc(PyObject *ob1, PyObject *ob2, int op)
{
	return ((PySID *)ob1)->richcompare(ob2, op);
}

/*static*/ void PySID::deallocFunc(PyObject *ob)
{
	delete (PySID *)ob;
}

// NOTE:  This function taken from KB Q131320.
BOOL GetTextualSid( 

    PSID pSid,          // binary Sid
    LPTSTR TextualSid,  // buffer for Textual representaion of Sid
    LPDWORD dwBufferLen // required/provided TextualSid buffersize
    )
{ 
    PSID_IDENTIFIER_AUTHORITY psia;
    DWORD dwSubAuthorities;
    DWORD dwSidRev=SID_REVISION;
    DWORD dwCounter;
    DWORD dwSidSize;

    // 
    // test if Sid passed in is valid
    // 
    if(!IsValidSid(pSid)) return FALSE;

    // obtain SidIdentifierAuthority
    psia=GetSidIdentifierAuthority(pSid);

    // obtain sidsubauthority count
    dwSubAuthorities=*GetSidSubAuthorityCount(pSid);

    // 
    // compute buffer length
    // S-SID_REVISION- + identifierauthority- + subauthorities- + NULL
    // 
    dwSidSize=(15 + 12 + (12 * dwSubAuthorities) + 1) * sizeof(TCHAR);

    // 
    // check provided buffer length.
    // If not large enough, indicate proper size and setlasterror
    // 
    if (*dwBufferLen < dwSidSize)
    {
        *dwBufferLen = dwSidSize;
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return FALSE;
    }

    // 
    // prepare S-SID_REVISION-
    // 
    dwSidSize=wsprintf(TextualSid, TEXT("S-%lu-"), dwSidRev );

    // 
    // prepare SidIdentifierAuthority
    // 
    if ( (psia->Value[0] != 0) || (psia->Value[1] != 0) )
    {
        dwSidSize+=wsprintf(TextualSid + lstrlen(TextualSid),
                    TEXT("0x%02hx%02hx%02hx%02hx%02hx%02hx"),
                    (USHORT)psia->Value[0],
                    (USHORT)psia->Value[1],
                    (USHORT)psia->Value[2],
                    (USHORT)psia->Value[3],
                    (USHORT)psia->Value[4],
                    (USHORT)psia->Value[5]);
    }
    else
    {
        dwSidSize+=wsprintf(TextualSid + lstrlen(TextualSid),
                    TEXT("%lu"),
                    (ULONG)(psia->Value[5]      )   +
                    (ULONG)(psia->Value[4] <<  8)   +
                    (ULONG)(psia->Value[3] << 16)   +
                    (ULONG)(psia->Value[2] << 24)   );
    }

    // 
    // loop through SidSubAuthorities
    // 
    for (dwCounter=0 ; dwCounter < dwSubAuthorities ; dwCounter++)
    {
        dwSidSize+=wsprintf(TextualSid + dwSidSize, TEXT("-%lu"),
                    *GetSidSubAuthority(pSid, dwCounter) );
    }

    return TRUE;
} 

/* static */ PyObject *PySID::strFunc(PyObject *ob)
{
	PySID *pySid = (PySID *)ob;
	PSID psid = pySid->m_psid;
	DWORD bufSize = 0;
	GetTextualSid(psid, NULL, &bufSize); // max size, NOT actual size!
	if (GetLastError()!=ERROR_INSUFFICIENT_BUFFER) {
		return PyString_FromString("PySID: Invalid SID");
	}
	// Space for the "PySID:" prefix.
	TCHAR *prefix = _T("PySID:");
	TCHAR *buf = (TCHAR *)malloc((_tcslen(prefix)+bufSize) * sizeof(TCHAR));
	if (buf==NULL) return PyErr_NoMemory();
	_tcscpy(buf, prefix);
	GetTextualSid(psid, buf+_tcslen(prefix), &bufSize);
	PyObject *ret = PyWinObject_FromTCHAR(buf);
	free(buf);
	return ret;
}
#else /* NO_PYWINTYPES_SECURITY */

BOOL PyWinObject_AsSID(PyObject *ob, PSID *ppSID, BOOL bNoneOK /*= TRUE*/)
{
	if (bNoneOK && ob==Py_None) {
		*ppSID = NULL;
	} else {
		if (bNoneOK)
			PyErr_SetString(PyExc_TypeError,
			                "This build of pywintypes only supports None as "
			                "a SID");
		else
			PyErr_SetString(PyExc_TypeError,
			                "This function can not work in this build, as "
			                "only None may be used as a SID");
		return FALSE;
	}
	return TRUE;
}
PyObject *PyWinObject_FromSID(PSID psid)
{
	if (psid==NULL) {
		Py_INCREF(Py_None);
		return Py_None;
	}
	PyErr_SetString(PyExc_RuntimeError,
	                "A non-NULL SID was passed, but security "
	                "descriptors are disabled from this build");
	return NULL;
}

#endif /* NO_PYWINTYPES_SECURITY */
