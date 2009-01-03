
//
// @doc

#include "PyWinTypes.h"
#include "winperf.h"
#include "pyperfmon.h"
#include "tchar.h"


BOOL PyWinObject_AsPyPERF_OBJECT_TYPE(PyObject *ob, PyPERF_OBJECT_TYPE **ppPyPERF_OBJECT_TYPE, BOOL bNoneOK /*= TRUE*/)
{
	if (bNoneOK && ob==Py_None) {
		*ppPyPERF_OBJECT_TYPE = NULL;
	} else if (!PyPERF_OBJECT_TYPE_Check(ob)) {
		PyErr_SetString(PyExc_TypeError, "The object is not a PyPERF_OBJECT_TYPE object");
		return FALSE;
	} else {
		*ppPyPERF_OBJECT_TYPE = ((PyPERF_OBJECT_TYPE *)ob);
	}
	return TRUE;
}

// @pymethod <o PyPERF_OBJECT_TYPE>|perfmon|ObjectType|Creates a new PERF_OBJECT_TYPE object
PyObject *PerfmonMethod_NewPERF_OBJECT_TYPE(PyObject *self, PyObject *args)
{
	PyObject *obCounters;

	if (!PyArg_ParseTuple(args, "O:ObjectType", &obCounters))
		return NULL;

	PyPERF_OBJECT_TYPE *pPOT = new(PyPERF_OBJECT_TYPE);
	if (pPOT==NULL) {
		PyErr_SetString(PyExc_MemoryError, "Allocating MappingManager or PERF_OBJECT_TYPE");
		return NULL;
	}
	if (!pPOT->InitPythonObjects( obCounters )) {
		delete pPOT;
		return NULL;
	}
	return pPOT;
}

// @pymethod |PyPERF_OBJECT_TYPE|Close|Closes the object.
PyObject *PyPERF_OBJECT_TYPE::Close(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ":Close"))
		return NULL;
	PyPERF_OBJECT_TYPE *This = (PyPERF_OBJECT_TYPE *)self;
	This->Term();
	Py_INCREF(Py_None);
	return Py_None;
}

// @object PyPERF_OBJECT_TYPE|A Python object, representing a PERF_OBJECT_TYPE structure
struct PyMethodDef PyPERF_OBJECT_TYPE::methods[] = {
	{"Close",          PyPERF_OBJECT_TYPE::Close, 1}, // @pymeth Close|Closes all counters.
	{NULL}
};


PyTypeObject PyPERF_OBJECT_TYPE::type =
{
	PYWIN_OBJECT_HEAD
	"PyPERF_OBJECT_TYPE",
	sizeof(PyPERF_OBJECT_TYPE),
	0,
	PyPERF_OBJECT_TYPE::deallocFunc,	/* tp_dealloc */
	0,						/* tp_print */
	0,						/* tp_getattr */
	0,						/* tp_setattr */
	0,						/* tp_compare */
	0,						/* tp_repr */
	0,						/* tp_as_number */
	0,						/* tp_as_sequence */
	0,						/* tp_as_mapping */
	0,						/* tp_hash */
	0,						/* tp_call */
	0,						/* tp_str */
	PyObject_GenericGetAttr,	/* tp_getattro */
	PyObject_GenericSetAttr,	/* tp_setattro */
	0,						/* tp_as_buffer */
	Py_TPFLAGS_DEFAULT,		/* tp_flags */
	0,						/* tp_doc */
	0,						/* tp_traverse */
	0,						/* tp_clear */
	0,						/* tp_richcompare */
	0,						/* tp_weaklistoffset */
	0,						/* tp_iter */
	0,						/* tp_iternext */
	PyPERF_OBJECT_TYPE::methods,		/* tp_methods */
	PyPERF_OBJECT_TYPE::members,		/* tp_members */
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

#define OFF(e) offsetof(PyPERF_OBJECT_TYPE, e)


/*static*/ struct PyMemberDef PyPERF_OBJECT_TYPE::members[] = {
	{"ObjectNameTitleIndex",  T_LONG,  OFF(m_ObjectNameTitleIndex)}, // @prop integer|ObjectNameTitleIndex|
	{"ObjectHelpTitleIndex",  T_LONG,  OFF(m_ObjectHelpTitleIndex)}, // @prop integer|ObjectHelpTitleIndex|
	{"DefaultCounterIndex",        T_LONG,  OFF(m_DefaultCounter)}, // @prop integer|DefaultCounterIndex|
	{NULL}
};

PyPERF_OBJECT_TYPE::PyPERF_OBJECT_TYPE(void)
{
	ob_type = &type;
	_Py_NewReference(this);
	m_pPOT = NULL;
	m_obCounters = NULL;
	m_obPerfMonManager = NULL;
	m_ObjectNameTitleIndex = 0;
	m_ObjectHelpTitleIndex = 0;
	m_DefaultCounter = 0;
}

PyPERF_OBJECT_TYPE::~PyPERF_OBJECT_TYPE()
{
	Term();
}

void PyPERF_OBJECT_TYPE::Term()
{
	m_pPOT = NULL;
	Py_XDECREF(m_obCounters);
	m_obCounters = NULL;
	Py_XDECREF(m_obPerfMonManager);
	m_obPerfMonManager = NULL;
}

// Get the counter objects that Im gunna use.
BOOL PyPERF_OBJECT_TYPE::InitPythonObjects( PyObject *obCounters )
{
	m_obCounters = obCounters;
	Py_XINCREF(obCounters);
	return TRUE;
}

// Init the memory layout of the win32 perfmon structures from the mapping manager.
// Doesnt keep a reference to the mapping manager, but assumes it will stay alive
// until Im term'd!
// Also _removes_ the reference to the counters' and _adds_ a reference to
// the PyPerMonManager object
BOOL PyPERF_OBJECT_TYPE::InitMemoryLayout( MappingManager *pmm, PyPerfMonManager *obPerfMonManager)
{
	BOOL ok = FALSE;
	PyObject *obCounter = NULL;
	void *pBuffer;
	PyPERF_COUNTER_DEFINITION *pCounter;
	DWORD thisCounterSize;
	DWORD totalCounterSize;
	DWORD counterOffset;
	ULONG minDetail;
	DWORD numCounters;
	DWORD counterNum;
	PERF_COUNTER_BLOCK *pPCB;
	PyObject *obCounters = m_obCounters;

	if (obCounters==NULL) {
		PyErr_SetString(PyExc_RuntimeError, "The object has not been initialised with any counters!");
		return FALSE;
	}

	m_obPerfMonManager = obPerfMonManager;
	Py_INCREF(m_obPerfMonManager);

	// First allocate the PERF_OBJECT_TYPE structure.
	m_pPOT = (PERF_OBJECT_TYPE *)pmm->AllocChunk(sizeof(PERF_OBJECT_TYPE));
	if (m_pPOT==NULL)
		goto done;

	numCounters = (DWORD)PySequence_Length(obCounters);
	minDetail = (ULONG) -1;
	for (counterNum = 0;counterNum<numCounters;counterNum++) {
		// Cleanup from last time round the loop (done: cleans last loop!)
		if (obCounter) {
			Py_DECREF(obCounter);
			obCounter = NULL;
		}
		obCounter = PySequence_GetItem(obCounters, counterNum);
		if (obCounter==NULL)
			goto done;

		if (!PyWinObject_AsPyPERF_COUNTER_DEFINITION(obCounter, &pCounter, FALSE))
			goto done;
		if ((ULONG)pCounter->GetDetailLevel() < minDetail) {
			minDetail = pCounter->GetDetailLevel();
		}
		// Allocate memory for the counter object.
		pBuffer = pmm->AllocChunk(sizeof(PERF_COUNTER_DEFINITION));
		if (pBuffer==NULL)
			goto done;
		pCounter->AcceptBuffer(this, pBuffer);
		pCounter->SetupBuffer();
	}
	// Now loop allocating the actual buffer for the raw counter data.
	counterOffset = sizeof(DWORD);
	totalCounterSize = sizeof(DWORD);
	// Allocate 2 bytes which forms the header of the PERF_COUNTER_BLOCK
	// structure.  Then each counter has its slot allocated.
	pPCB = (PERF_COUNTER_BLOCK *)pmm->AllocChunk(sizeof(DWORD));
	for (counterNum = 0;counterNum<numCounters;counterNum++) {
		// Cleanup from last time round the loop (done: cleans last loop!)
		if (obCounter) {
			Py_DECREF(obCounter);
			obCounter = NULL;
		}
		obCounter = PySequence_GetItem(obCounters, counterNum);
		if (obCounter==NULL)
			goto done;

		if (!PyWinObject_AsPyPERF_COUNTER_DEFINITION(obCounter, &pCounter, FALSE))
			goto done;
		// Allocate memory for the raw counter data object.
		thisCounterSize = pCounter->GetCounterDataSize();
		totalCounterSize += thisCounterSize;
		pBuffer = pmm->AllocChunk(thisCounterSize);
		if (pCounter==NULL)
			goto done;
		pCounter->AcceptRawCounterBuffer(pBuffer, counterOffset);
		counterOffset += thisCounterSize;
	}
	pPCB->ByteLength = totalCounterSize;
	// Now back-fill the PERF_OBJECT_TYPE buffer.
	m_pPOT->TotalByteLength = sizeof(PERF_OBJECT_TYPE) + 
							  (numCounters * sizeof(PERF_COUNTER_DEFINITION)) + 
							  totalCounterSize;
	m_pPOT->DefinitionLength = sizeof(PERF_OBJECT_TYPE) + 
							  (numCounters * sizeof(PERF_COUNTER_DEFINITION));
	m_pPOT->HeaderLength = sizeof(PERF_OBJECT_TYPE);
	m_pPOT->ObjectNameTitleIndex = m_ObjectNameTitleIndex;
	m_pPOT->ObjectNameTitle = NULL;
	m_pPOT->ObjectHelpTitleIndex = m_ObjectHelpTitleIndex;
	m_pPOT->ObjectHelpTitle = NULL;
	m_pPOT->DetailLevel = minDetail;
	m_pPOT->NumCounters = numCounters;
	m_pPOT->DefaultCounter = m_DefaultCounter;
	m_pPOT->NumInstances = -1;
	m_pPOT->CodePage = 0;
	m_pPOT->PerfTime.QuadPart = 0;
	m_pPOT->PerfFreq.QuadPart = 0;

	Py_XDECREF(m_obCounters);
	m_obCounters = NULL;
	ok = TRUE;

done:
	Py_XDECREF(obCounter);
	return ok;
}

/*static*/ void PyPERF_OBJECT_TYPE::deallocFunc(PyObject *ob)
{
	delete (PyPERF_OBJECT_TYPE *)ob;
}

