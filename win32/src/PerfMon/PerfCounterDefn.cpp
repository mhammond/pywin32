//
// @doc

#include "windows.h"
#include "Python.h"
#include "structmember.h"
#include "PyWinTypes.h"
#include "winperf.h"
#include "pyperfmon.h"

// @pymethod <o PyPERF_COUNTER_DEFINITION>|perfmon|CounterDefinition|Creates a new <o PyPERF_COUNTER_DEFINITION> object
PyObject *PerfmonMethod_NewPERF_COUNTER_DEFINITION(PyObject *self, PyObject *args)
{
	long counterNameTitleIndex;
	if (!PyArg_ParseTuple(args, "l:PERF_COUNTER_DEFINITION", &counterNameTitleIndex))
		return NULL;
	return new PyPERF_COUNTER_DEFINITION(counterNameTitleIndex);
}

BOOL PyWinObject_AsPyPERF_COUNTER_DEFINITION(PyObject *ob, PyPERF_COUNTER_DEFINITION **ppPERF_COUNTER_DEFINITION, BOOL bNoneOK /*= TRUE*/)
{
	if (bNoneOK && ob==Py_None) {
		*ppPERF_COUNTER_DEFINITION = NULL;
	} else if (!PyPERF_COUNTER_DEFINITION_Check(ob)) {
		PyErr_SetString(PyExc_TypeError, "The object is not a PyPERF_COUNTER_DEFINITION object");
		return FALSE;
	} else {
		*ppPERF_COUNTER_DEFINITION = ((PyPERF_COUNTER_DEFINITION *)ob);
	}
	return TRUE;
}

// @pymethod |PyPERF_COUNTER_DEFINITION|Increment|Increments the value of the performance counter
PyObject *PyPERF_COUNTER_DEFINITION::Increment(PyObject *self, PyObject *args)
{
	PyPERF_COUNTER_DEFINITION *This = (PyPERF_COUNTER_DEFINITION *)self;
	int incrBy = 1;
	if (!PyArg_ParseTuple(args, "|i:Increment", &incrBy))
		return NULL;

	DWORD *pVal = (DWORD *)(This->m_pCounterValue);
	if (pVal)
		*pVal += incrBy;
	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod |PyPERF_COUNTER_DEFINITION|Decrement|Decrements the value of the performance counter
PyObject *PyPERF_COUNTER_DEFINITION::Decrement(PyObject *self, PyObject *args)
{
	PyPERF_COUNTER_DEFINITION *This = (PyPERF_COUNTER_DEFINITION *)self;
	int incrBy = 1;
	if (!PyArg_ParseTuple(args, "|i:Decrement", &incrBy))
		return NULL;

	DWORD *pVal = (DWORD *)(This->m_pCounterValue);
	if (pVal)
		*pVal -= incrBy;
	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod |PyPERF_COUNTER_DEFINITION|Set|Sets the counter to a specific value
PyObject *PyPERF_COUNTER_DEFINITION::Set(PyObject *self, PyObject *args)
{
	PyPERF_COUNTER_DEFINITION *This = (PyPERF_COUNTER_DEFINITION *)self;
	int setTo;
	if (!PyArg_ParseTuple(args, "i:Set", &setTo))
		return NULL;

	DWORD *pVal = (DWORD *)(This->m_pCounterValue);
	if (pVal)
		*pVal = setTo;
	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod |PyPERF_COUNTER_DEFINITION|Get|Gets the current value of the counter
PyObject *PyPERF_COUNTER_DEFINITION::Get(PyObject *self, PyObject *args)
{
	PyPERF_COUNTER_DEFINITION *This = (PyPERF_COUNTER_DEFINITION *)self;
	if (!PyArg_ParseTuple(args, ":Get"))
		return NULL;
	if (!This->m_pCounterValue==NULL) {
		PyErr_SetString(PyExc_ValueError, "The counter does not exist in a counter block");
		return NULL;
	}

	DWORD *pVal = (DWORD *)(This->m_pCounterValue);
	return PyInt_FromLong(*pVal);
}


// @object PyPERF_COUNTER_DEFINITION|An object encapsulating a Windows NT Performance Monitor counter definition (PERF_COUNTER_DEFINITION).
// @comm Note that all the counter "set" functions will silently do nothing
// if the counter does not appear in a block.  This is so the application can avoid
// excessive tests for lack of performance monitor functionality.
// However, the method <om PyPERF_COUNTER_DEFINITION.Get> will raise a ValueError exception in this case.
static struct PyMethodDef PyPERF_COUNTER_DEFINITION_methods[] = {
	{"Increment",      PyPERF_COUNTER_DEFINITION::Increment, 1}, 	// @pymeth Increment|Increments the value of the performance counter
	{"Decrement",      PyPERF_COUNTER_DEFINITION::Decrement, 1}, 	// @pymeth Decrement|Decrements the value of the performance counter
	{"Set",            PyPERF_COUNTER_DEFINITION::Set, 1}, 	// @pymeth Set|Sets the counter to a specific value
	{"Get",            PyPERF_COUNTER_DEFINITION::Get, 1}, 	// @pymeth Get|Gets the current value of the counter
	{NULL}
};


PyTypeObject PyPERF_COUNTER_DEFINITION::type =
{
	PyObject_HEAD_INIT(&PyType_Type)
	0,
	"PyPERF_COUNTER_DEFINITION",
	sizeof(PyPERF_COUNTER_DEFINITION),
	0,
	PyPERF_COUNTER_DEFINITION::deallocFunc,		/* tp_dealloc */
	0,		/* tp_print */
	PyPERF_COUNTER_DEFINITION::getattr,				/* tp_getattr */
	PyPERF_COUNTER_DEFINITION::setattr,				/* tp_setattr */
	0,	/* tp_compare */
	0,						/* tp_repr */
	0,						/* tp_as_number */
	0,	/* tp_as_sequence */
	0,						/* tp_as_mapping */
	0,
	0,						/* tp_call */
	0,		/* tp_str */
};

#define OFF(e) offsetof(PyPERF_COUNTER_DEFINITION, e)


/*static*/ struct memberlist PyPERF_COUNTER_DEFINITION::memberlist[] = {
	{"DefaultScale",  T_LONG,  OFF(m_DefaultScale)}, // @prop integer|DefaultScale|The default scale of the counter.
	{"DetailLevel",  T_LONG,  OFF(m_DetailLevel)}, // @prop integer|DetailLevel|The detail level of the counter.
	{"CounterType",  T_LONG,  OFF(m_CounterType)}, // @prop integer|CounterType|The counter type.
	{"CounterNameTitleIndex",  T_LONG,  OFF(m_CounterNameTitleIndex)}, // @prop integer|CounterNameTitleIndex|
	{"CounterHelpTitleIndex",  T_LONG,  OFF(m_CounterHelpTitleIndex)}, // @prop integer|CounterHelpTitleIndex|
	{NULL}	/* Sentinel */
};

PyPERF_COUNTER_DEFINITION::PyPERF_COUNTER_DEFINITION(DWORD counterNameTitleIndex)
{
	ob_type = &type;
	_Py_NewReference(this);
	m_pPCD = NULL;
	m_DefaultScale = 0;
	m_DetailLevel = PERF_DETAIL_NOVICE;
	m_CounterNameTitleIndex = counterNameTitleIndex;
	m_CounterHelpTitleIndex = counterNameTitleIndex;
	m_CounterType = PERF_COUNTER_COUNTER;
	m_CounterSize = sizeof(DWORD);
	m_pCounterValue = NULL;
	m_obBufferOwner = NULL;
}
PyPERF_COUNTER_DEFINITION::~PyPERF_COUNTER_DEFINITION()
{
	Py_XDECREF(m_obBufferOwner);
}

void PyPERF_COUNTER_DEFINITION::SetupBuffer()
{
	if (m_pPCD==NULL) return;
	m_pPCD->ByteLength = sizeof(PERF_COUNTER_DEFINITION);
	m_pPCD->CounterNameTitleIndex = m_CounterNameTitleIndex;
	m_pPCD->CounterNameTitle = NULL;
	m_pPCD->CounterHelpTitleIndex = m_CounterHelpTitleIndex;
	m_pPCD->CounterHelpTitle = NULL;
	m_pPCD->DefaultScale = m_DefaultScale;
	m_pPCD->DetailLevel = m_DetailLevel;
	m_pPCD->CounterType = m_CounterType;
	m_pPCD->CounterSize = m_CounterSize;
	// CounterOffset is not known yet!
}

void PyPERF_COUNTER_DEFINITION::AcceptBuffer( PyObject *obBufferOwner, void *pBuffer )
	{
	Py_XDECREF(m_obBufferOwner);
	m_obBufferOwner = obBufferOwner;
	Py_INCREF(obBufferOwner);
	m_pPCD = (PERF_COUNTER_DEFINITION *)pBuffer;
}

void PyPERF_COUNTER_DEFINITION::AcceptRawCounterBuffer( void *pBuffer, DWORD offset )
{
	if (m_pPCD==NULL) return;
	m_pPCD->CounterOffset = offset;
	m_pCounterValue = pBuffer;
	// Initialise the buffer to zero.
	memset( pBuffer, 0, m_CounterSize );
}

PyObject *PyPERF_COUNTER_DEFINITION::getattr(PyObject *self, char *name)
{
	PyObject *res;

	res = Py_FindMethod(PyPERF_COUNTER_DEFINITION_methods, self, name);
	if (res != NULL)
		return res;
	PyErr_Clear();
	return PyMember_Get((char *)self, memberlist, name);
}

int PyPERF_COUNTER_DEFINITION::setattr(PyObject *self, char *name, PyObject *v)
{
	if (v == NULL) {
		PyErr_SetString(PyExc_AttributeError, "can't delete PERF_COUNTER_DEFINITION attributes");
		return -1;
	}
	return PyMember_Set((char *)self, memberlist, name, v);
}

/*static*/ void PyPERF_COUNTER_DEFINITION::deallocFunc(PyObject *ob)
{
	delete (PyPERF_COUNTER_DEFINITION *)ob;
}

