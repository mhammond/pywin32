//
// @doc

#include "PyWinTypes.h"
#include "PyWinObjects.h"
#include "PySecurityObjects.h"

#ifndef MS_WINCE /* This code is not available on Windows CE */

// @pymethod <o PySID>|pywintypes|SID|Creates a new SID object
PyObject *PyWinMethod_NewSID(PyObject *self, PyObject *args)
{
	void *buf = NULL;
	int bufSize = 32;
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
			int numSubs = PySequence_Length(obSubs);
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
			if (!AllocateAndInitializeSid(&sid_ia, numSubs, sub0, sub1, sub2, sub3, sub4, sub5, sub6, sub7, &pNew))
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
	return PyInt_FromLong( IsValidSid(This->GetSID()) );
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
    return Py_BuildValue("(iiiiii)",psia->Value[0],psia->Value[1],psia->Value[2],psia->Value[3],psia->Value[4],psia->Value[5]);
}

// @object PySID|A Python object, representing a SID structure
static struct PyMethodDef PySID_methods[] = {
	{"Initialize",     PySID::Initialize, 1}, 	// @pymeth Initialize|Initialize the SID.
	{"IsValid",        PySID::IsValid, 1}, 	// @pymeth IsValid|Determines if the SID is valid.
	{"SetSubAuthority",PySID::SetSubAuthority, 1}, 	// @pymeth SetSubAuthority|Sets a SID SubAuthority
	{"GetLength",      PySID::GetLength, 1}, // @pymeth GetLength|Return length of sid (GetLengthSid)
	{"GetSubAuthorityCount",   PySID::GetSubAuthorityCount, 1}, // @pymeth GetSubAuthorityCount|Return nbr of subauthorities from SID	
	{"GetSubAuthority",PySID::GetSubAuthority, 1}, // @pymeth GetSubAuthority|Return specified subauthory from SID	
	{"GetSidIdentifierAuthority",PySID::GetSidIdentifierAuthority, 1}, // @pymeth GetSidIdentifierAuthority|Return identifier for the authority who issued the SID (one of the SID_IDENTIFIER_AUTHORITY constants)
	{NULL}
};

static PyBufferProcs PySID_as_buffer = {
	(getreadbufferproc)PySID::getreadbuf,
	(getwritebufferproc)0,
	(getsegcountproc)PySID::getsegcount,
	(getcharbufferproc)0,
};


PYWINTYPES_EXPORT PyTypeObject PySIDType =
{
	PyObject_HEAD_INIT(&PyType_Type)
	0,
	"PySID",
	sizeof(PySID),
	0,
	PySID::deallocFunc,		/* tp_dealloc */
	0,		/* tp_print */
	PySID::getattr,				/* tp_getattr */
	0,				/* tp_setattr */
	// @pymeth __cmp__|Used when objects are compared.
	PySID::compareFunc,	/* tp_compare */
	0,						/* tp_repr */
	0,						/* tp_as_number */
	0,	/* tp_as_sequence */
	0,						/* tp_as_mapping */
	0,
	0,						/* tp_call */
	PySID::strFunc,		/* tp_str */
	0,		/*tp_getattro*/
	0,		/*tp_setattro*/
	// @comm Note the PySID object supports the buffer interface.  Thus buffer(sid) can be used to obtain the raw bytes.
	&PySID_as_buffer,	/*tp_as_buffer*/
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

PyObject *PySID::getattr(PyObject *self, char *name)
{
	return Py_FindMethod(PySID_methods, self, name);
}

int PySID::compare(PyObject *ob)
{
	PSID p1 = NULL, p2 = NULL;
	PyWinObject_AsSID(this, &p1);
	PyWinObject_AsSID(ob, &p2);
	return EqualSid(p1, p2)==FALSE;
}


// @pymethod int|PySID|__cmp__|Used when objects are compared.
// @comm This method calls the Win32 API function EqualSid
int PySID::compareFunc(PyObject *ob1, PyObject *ob2)
{
	return ((PySID *)ob1)->compare(ob2);
}

/*static*/ void PySID::deallocFunc(PyObject *ob)
{
	delete (PySID *)ob;
}

/*static*/ int PySID::getreadbuf(PyObject *self, int index, const void **ptr)
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

/*static*/ int PySID::getsegcount(PyObject *self, int *lenp)
{
	if ( lenp )
		*lenp = GetLengthSid(((PySID *)self)->m_psid);
	return 1;
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
	const char *prefix = "PySID:";
	char *buf = (char *)malloc(strlen(prefix)+bufSize);
	if (buf==NULL) return PyErr_NoMemory();
	strcpy(buf, prefix);
	GetTextualSid(psid, buf+strlen(prefix), &bufSize);
	return PyString_FromString(buf);
}


#endif /* MS_WINCE */
