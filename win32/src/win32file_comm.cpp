// The COMM port enhancements were added by Mark Hammond, and are
// (c) 2000, ActiveState Tools Corp.
//
// @doc

#include "windows.h"
#include "Python.h"
#include "structmember.h"
#include "PyWinTypes.h"
#include "PyWinObjects.h"

// Small enough we can use a tuple!
// @object COMMTIMEOUTS|A tuple representing a COMMTIMEOUTS structure.
// @tupleitem 0|int|ReadIntervalTimeout| 
// @tupleitem 1|int|ReadTotalTimeoutMultiplier| 
// @tupleitem 2|int|ReadTotalTimeoutConstant| 
// @tupleitem 3|int|WriteTotalTimeoutMultiplier| 
// @tupleitem 4|int|WriteTotalTimeoutConstant| 
PyObject *PyWinObject_FromCOMMTIMEOUTS( COMMTIMEOUTS *p)
{
	return Py_BuildValue("iiiii",
		p->ReadIntervalTimeout,
		p->ReadTotalTimeoutMultiplier,
		p->ReadTotalTimeoutConstant,
		p->WriteTotalTimeoutMultiplier,
		p->WriteTotalTimeoutConstant);
}

BOOL PyWinObject_AsCOMMTIMEOUTS( PyObject *ob, COMMTIMEOUTS *p)
{
	return PyArg_ParseTuple(ob, "iiiii",
		&p->ReadIntervalTimeout,
		&p->ReadTotalTimeoutMultiplier,
		&p->ReadTotalTimeoutConstant,
		&p->WriteTotalTimeoutMultiplier,
		&p->WriteTotalTimeoutConstant);
}

static const char *szNeedIntAttr = "Attribute '%s' must be an integer";

class PyDCB : public PyObject
{
public:
	DCB *GetDCB() {return &m_DCB;}

	PyDCB(void);
	PyDCB(const DCB &);
	~PyDCB();

	/* Python support */
	int compare(PyObject *ob);

	static void deallocFunc(PyObject *ob);
	static int compareFunc(PyObject *ob1, PyObject *ob2);

	static PyObject *getattr(PyObject *self, char *name);
	static int setattr(PyObject *self, char *name, PyObject *v);
	static struct memberlist memberlist[];
	static PyTypeObject type;

protected:
	DCB m_DCB;
};

#define PyDCB_Check(x) ((x)->ob_type==&PyDCB::type)

// @pymethod <o PyDCB>|win32file|DCB|Creates a new DCB object
PyObject *PyWinMethod_NewDCB(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ":DCB"))
		return NULL;
	return new PyDCB();
}

// @object PyDCB|A Python object, representing an DCB structure
// @comm Typically you query a device for its DCB using 
// <om win32file.GetCommState>, change any setting necessary, then
// call <om win32file.SetCommState> with the new structure.
BOOL PyWinObject_AsDCB(PyObject *ob, DCB **ppDCB, BOOL bNoneOK /*= TRUE*/)
{
	if (bNoneOK && ob==Py_None) {
		*ppDCB = NULL;
	} else if (!PyDCB_Check(ob)) {
		PyErr_SetString(PyExc_TypeError, "The object is not a PyDCB object");
		return FALSE;
	} else {
		*ppDCB = ((PyDCB *)ob)->GetDCB();
	}
	return TRUE;
}

PyObject *PyWinObject_FromDCB(const DCB *pDCB)
{
	if (pDCB==NULL) {
		Py_INCREF(Py_None);
		return Py_None;
	}
	PyObject *ret = new PyDCB(*pDCB);
	if(ret==NULL)
		PyErr_SetString(PyExc_MemoryError, "Allocating pDCB");
	return ret;
}

PyTypeObject PyDCB::type =
{
	PyObject_HEAD_INIT(&PyType_Type)
	0,
	"PyDCB",
	sizeof(PyDCB),
	0,
	PyDCB::deallocFunc,		/* tp_dealloc */
	0,		/* tp_print */
	PyDCB::getattr,				/* tp_getattr */
	PyDCB::setattr,				/* tp_setattr */
	0,	/* tp_compare */
	0,						/* tp_repr */
	0,						/* tp_as_number */
	0,	/* tp_as_sequence */
	0,						/* tp_as_mapping */
	0,
	0,						/* tp_call */
	0,		/* tp_str */
};

#define OFF(e) offsetof(PyDCB, e)

#define T_DWORD T_UINT
#define T_WORD T_USHORT

/*static*/ struct memberlist PyDCB::memberlist[] = {
// NOTE - bitfields missing.
  {"BaudRate", T_DWORD, OFF(m_DCB.BaudRate)},            // @prop integer|BaudRate|current baud rate 
  {"wReserved", T_WORD, OFF(m_DCB.wReserved)},          // @prop integer|wReserved|not currently used 
  {"XonLim", T_WORD, OFF(m_DCB.XonLim)},             // @prop integer|XonLim|transmit XON threshold 
  {"XoffLim", T_WORD, OFF(m_DCB.XoffLim)},            // @prop integer|XoffLim|transmit XOFF threshold 
  {"ByteSize", T_BYTE, OFF(m_DCB.ByteSize)},           // @prop integer|ByteSize|number of bits/byte, 4-8 
  {"Parity", T_BYTE, OFF(m_DCB.Parity)},             // @prop integer|Parity|0-4=no,odd,even,mark,space 
  {"StopBits", T_BYTE, OFF(m_DCB.StopBits)},           // @prop integer|StopBits|0,1,2 = 1, 1.5, 2 
  {"XonChar", T_CHAR, OFF(m_DCB.XonChar)},            // @prop integer|XonChar|Tx and Rx XON character 
  {"XoffChar", T_CHAR, OFF(m_DCB.XoffChar)},           // @prop integer|XoffChar|Tx and Rx XOFF character 
  {"ErrorChar", T_CHAR, OFF(m_DCB.ErrorChar)},          // @prop integer|ErrorChar|error replacement character 
  {"EofChar", T_CHAR, OFF(m_DCB.EofChar)},            // @prop integer|EofChar|end of input character 
  {"EvtChar", T_CHAR, OFF(m_DCB.EvtChar)},            // @prop integer|EvtChar|received event character 
  {"wReserved1", T_WORD, OFF(m_DCB.wReserved1)},         // @prop integer|wReserved1|reserved; do not use 
  {NULL}
};

// @prop integer|fBinary|binary mode, no EOF check 
// @prop integer|fParity|enable parity checking 
// @prop integer|fOutxCtsFlow|CTS output flow control 
// @prop integer|fOutxDsrFlow|DSR output flow control 
// @prop integer|fDtrControl|DTR flow control type 
// @prop integer|fDsrSensitivity|DSR sensitivity 
// @prop integer|fTXContinueOnXoff|XOFF continues Tx 
// @prop integer|fOutX|XON/XOFF out flow control 
// @prop integer|fInX|XON/XOFF in flow control 
// @prop integer|fErrorChar|enable error replacement 
// @prop integer|fNull|enable null stripping 
// @prop integer|fRtsControl|RTS flow control 
// @prop integer|fAbortOnError|abort on error 
// @prop integer|fDummy2|reserved 

PyDCB::PyDCB(void)
{
	ob_type = &type;
	_Py_NewReference(this);
	memset(&m_DCB, 0, sizeof(m_DCB));
	m_DCB.DCBlength = sizeof(m_DCB);
}

PyDCB::PyDCB(const DCB &other)
{
	ob_type = &type;
	_Py_NewReference(this);
	m_DCB = other;
}

PyDCB::~PyDCB(void)
{
}

#define GET_BITFIELD_ENTRY(bitfield_name) \
	else if (strcmp(name, #bitfield_name)==0) { \
		return PyInt_FromLong(pydcb->m_DCB.##bitfield_name); \
	} \

PyObject *PyDCB::getattr(PyObject *self, char *name)
{
	PyDCB *pydcb = (PyDCB *)self;
	if (0) // boot up our macro magic (the macro starts with an 'else')
		;
	GET_BITFIELD_ENTRY(fBinary)
	GET_BITFIELD_ENTRY(fParity)
	GET_BITFIELD_ENTRY(fOutxCtsFlow)
	GET_BITFIELD_ENTRY(fOutxDsrFlow)
	GET_BITFIELD_ENTRY(fDtrControl)
	GET_BITFIELD_ENTRY(fDsrSensitivity)
	GET_BITFIELD_ENTRY(fTXContinueOnXoff)
	GET_BITFIELD_ENTRY(fOutX)
	GET_BITFIELD_ENTRY(fInX)
	GET_BITFIELD_ENTRY(fErrorChar)
	GET_BITFIELD_ENTRY(fNull)
	GET_BITFIELD_ENTRY(fRtsControl)
	GET_BITFIELD_ENTRY(fAbortOnError)
	GET_BITFIELD_ENTRY(fDummy2)
	return PyMember_Get((char *)self, memberlist, name);
}

#define SET_BITFIELD_ENTRY(bitfield_name) \
	else if (strcmp(name, #bitfield_name)==0) { \
		if (!PyInt_Check(v)) { \
			PyErr_Format(PyExc_TypeError, szNeedIntAttr, #bitfield_name); \
			return -1; \
		} \
		pydcb->m_DCB.##bitfield_name = PyInt_AsLong(v); \
		return 0; \
	} \

int PyDCB::setattr(PyObject *self, char *name, PyObject *v)
{
	PyDCB *pydcb = (PyDCB *)self;
	if (v == NULL) {
		PyErr_SetString(PyExc_AttributeError, "can't delete DCB attributes");
		return -1;
	}
	SET_BITFIELD_ENTRY(fBinary)
	SET_BITFIELD_ENTRY(fParity)
	SET_BITFIELD_ENTRY(fOutxCtsFlow)
	SET_BITFIELD_ENTRY(fOutxDsrFlow)
	SET_BITFIELD_ENTRY(fDtrControl)
	SET_BITFIELD_ENTRY(fDsrSensitivity)
	SET_BITFIELD_ENTRY(fTXContinueOnXoff)
	SET_BITFIELD_ENTRY(fOutX)
	SET_BITFIELD_ENTRY(fInX)
	SET_BITFIELD_ENTRY(fErrorChar)
	SET_BITFIELD_ENTRY(fNull)
	SET_BITFIELD_ENTRY(fRtsControl)
	SET_BITFIELD_ENTRY(fAbortOnError)
	SET_BITFIELD_ENTRY(fDummy2)
	return PyMember_Set((char *)self, memberlist, name, v);
}

/*static*/ void PyDCB::deallocFunc(PyObject *ob)
{
	delete (PyDCB *)ob;
}

////////////////////////////////////////////////////////////////
//
// COMSTAT object.
//
////////////////////////////////////////////////////////////////
class PyCOMSTAT : public PyObject
{
public:
	COMSTAT *GetCOMSTAT() {return &m_COMSTAT;}

	PyCOMSTAT(void);
	PyCOMSTAT(const COMSTAT &);
	~PyCOMSTAT();

	/* Python support */
	int compare(PyObject *ob);

	static void deallocFunc(PyObject *ob);
	static int compareFunc(PyObject *ob1, PyObject *ob2);

	static PyObject *getattr(PyObject *self, char *name);
	static int setattr(PyObject *self, char *name, PyObject *v);
	static struct memberlist memberlist[];
	static PyTypeObject type;

protected:
	COMSTAT m_COMSTAT;
};

#define PyCOMSTAT_Check(x) ((x)->ob_type==&PyCOMSTAT::type)

// @pymethod <o PyCOMSTAT>|win32file|COMSTAT|Creates a new COMSTAT object
PyObject *PyWinMethod_NewCOMSTAT(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ":COMSTAT"))
		return NULL;
	return new PyCOMSTAT();
}

// @object PyCOMSTAT|A Python object, representing an COMSTAT structure

BOOL PyWinObject_AsCOMSTAT(PyObject *ob, COMSTAT **ppCOMSTAT, BOOL bNoneOK /*= TRUE*/)
{
	if (bNoneOK && ob==Py_None) {
		*ppCOMSTAT = NULL;
	} else if (!PyCOMSTAT_Check(ob)) {
		PyErr_SetString(PyExc_TypeError, "The object is not a PyCOMSTAT object");
		return FALSE;
	} else {
		*ppCOMSTAT = ((PyCOMSTAT *)ob)->GetCOMSTAT();
	}
	return TRUE;
}

PyObject *PyWinObject_FromCOMSTAT(const COMSTAT *pCOMSTAT)
{
	if (pCOMSTAT==NULL) {
		Py_INCREF(Py_None);
		return Py_None;
	}
	PyObject *ret = new PyCOMSTAT(*pCOMSTAT);
	if(ret==NULL)
		PyErr_SetString(PyExc_MemoryError, "Allocating pCOMSTAT");
	return ret;
}

PyTypeObject PyCOMSTAT::type =
{
	PyObject_HEAD_INIT(&PyType_Type)
	0,
	"PyCOMSTAT",
	sizeof(PyCOMSTAT),
	0,
	PyCOMSTAT::deallocFunc,		/* tp_dealloc */
	0,		/* tp_print */
	PyCOMSTAT::getattr,				/* tp_getattr */
	PyCOMSTAT::setattr,				/* tp_setattr */
	0,	/* tp_compare */
	0,						/* tp_repr */
	0,						/* tp_as_number */
	0,	/* tp_as_sequence */
	0,						/* tp_as_mapping */
	0,
	0,						/* tp_call */
	0,		/* tp_str */
};

#undef OFF
#define OFF(e) offsetof(PyCOMSTAT, e)


/*static*/ struct memberlist PyCOMSTAT::memberlist[] = {
// NOTE - bitfields missing.
  {"cbInQue", T_DWORD, OFF(m_COMSTAT.cbInQue)},            // @prop integer|cbInQue|Specifies the number of bytes received by the serial provider but not yet read by a <om win32file.ReadFile> operation
  {"cbOutQue", T_WORD, OFF(m_COMSTAT.cbOutQue)},          // @prop integer|cbOutQue|Specifies the number of bytes of user data remaining to be transmitted for all write operations. This value will be zero for a nonoverlapped write. 
  {NULL}
};

// @prop integer|fCtsHold|Specifies whether transmission is waiting for the CTS (clear-to-send) signal to be sent. If this member is TRUE, transmission is waiting. 
// @prop integer|fDsrHold|Specifies whether transmission is waiting for the DSR (data-set-ready) signal to be sent. If this member is TRUE, transmission is waiting. 
// @prop integer|fRlsdHold|Specifies whether transmission is waiting for the RLSD (receive-line-signal-detect) signal to be sent. If this member is TRUE, transmission is waiting. 
// @prop integer|fXoffHold|Specifies whether transmission is waiting because the XOFF character was received. If this member is TRUE, transmission is waiting. 
// @prop integer|fXoffSent|Specifies whether transmission is waiting because the XOFF character was transmitted. If this member is TRUE, transmission is waiting. Transmission halts when the XOFF character is transmitted to a system that takes the next character as XON, regardless of the actual character. 
// @prop integer|fEof|Specifies whether the end-of-file (EOF) character has been received. If this member is TRUE, the EOF character has been received. 
// @prop integer|fTxim|If this member is TRUE, there is a character queued for transmission that has come to the communications device by way of the TransmitCommChar function. The communications device transmits such a character ahead of other characters in the device's output buffer. 
// @prop integer|fReserved|Reserved; do not use. 

PyCOMSTAT::PyCOMSTAT(void)
{
	ob_type = &type;
	_Py_NewReference(this);
	memset(&m_COMSTAT, 0, sizeof(m_COMSTAT));
}

PyCOMSTAT::PyCOMSTAT(const COMSTAT &other)
{
	ob_type = &type;
	_Py_NewReference(this);
	m_COMSTAT = other;
}

PyCOMSTAT::~PyCOMSTAT(void)
{
}

#undef GET_BITFIELD_ENTRY
#define GET_BITFIELD_ENTRY(bitfield_name) \
	else if (strcmp(name, #bitfield_name)==0) { \
		return PyInt_FromLong(pyCOMSTAT->m_COMSTAT.##bitfield_name); \
	} \

PyObject *PyCOMSTAT::getattr(PyObject *self, char *name)
{
	PyCOMSTAT *pyCOMSTAT = (PyCOMSTAT *)self;
	if (0) // boot up our macro magic (the macro starts with an 'else')
		;
	GET_BITFIELD_ENTRY(fCtsHold )
	GET_BITFIELD_ENTRY(fDsrHold)
	GET_BITFIELD_ENTRY(fRlsdHold)
	GET_BITFIELD_ENTRY(fXoffHold)
	GET_BITFIELD_ENTRY(fXoffSent)
	GET_BITFIELD_ENTRY(fEof)
	GET_BITFIELD_ENTRY(fTxim)
	GET_BITFIELD_ENTRY(fReserved)
	return PyMember_Get((char *)self, memberlist, name);
}

#undef SET_BITFIELD_ENTRY
#define SET_BITFIELD_ENTRY(bitfield_name) \
	else if (strcmp(name, #bitfield_name)==0) { \
		if (!PyInt_Check(v)) { \
			PyErr_Format(PyExc_TypeError, szNeedIntAttr, #bitfield_name); \
			return -1; \
		} \
		pyCOMSTAT->m_COMSTAT.##bitfield_name = PyInt_AsLong(v); \
		return 0; \
	} \

int PyCOMSTAT::setattr(PyObject *self, char *name, PyObject *v)
{
	PyCOMSTAT *pyCOMSTAT = (PyCOMSTAT *)self;
	if (v == NULL) {
		PyErr_SetString(PyExc_AttributeError, "can't delete COMSTAT attributes");
		return -1;
	}
	SET_BITFIELD_ENTRY(fCtsHold )
	SET_BITFIELD_ENTRY(fDsrHold)
	SET_BITFIELD_ENTRY(fRlsdHold)
	SET_BITFIELD_ENTRY(fXoffHold)
	SET_BITFIELD_ENTRY(fXoffSent)
	SET_BITFIELD_ENTRY(fEof)
	SET_BITFIELD_ENTRY(fTxim)
	SET_BITFIELD_ENTRY(fReserved)
	return PyMember_Set((char *)self, memberlist, name, v);
}

/*static*/ void PyCOMSTAT::deallocFunc(PyObject *ob)
{
	delete (PyCOMSTAT *)ob;
}

