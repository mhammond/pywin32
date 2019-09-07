/*
    MappingManager.cpp

    Manages the shared memory/mapping used by the performance monitor extensions.

*/
// @doc

#include "PyWinTypes.h"
#include "winperf.h"
#include "pyperfmon.h"
#include "tchar.h"

MappingManager::MappingManager()
{
    m_hMappedObject = NULL;
    m_pMapBlock = NULL;
    m_pControl = NULL;
}

MappingManager::~MappingManager()
{
    if (m_pControl)
        m_pControl->supplierStatus = SupplierStatusStopped;
    UnmapViewOfFile(m_pMapBlock);
    CloseHandle(m_hMappedObject);
}

BOOL MappingManager::CheckStatus()
{
    if (m_pMapBlock == NULL) {
        PyErr_SetString(PyExc_ValueError, "The file mapping has not been initialised correctly");
        return FALSE;
    }
    return TRUE;
}

BOOL MappingManager::Init(const TCHAR *szServiceName, const TCHAR *szMappingName /* = NULL */,
                          const TCHAR *szEventSourceName /* = NULL */)
{
    TCHAR szGlobalMapping[MAX_PATH + 10] = _T("");

    if (szMappingName == NULL)
        szMappingName = szServiceName;
    if (szEventSourceName == NULL)
        szEventSourceName = szServiceName;

    // Use a TerminalServices friendly "Global\\" prefix if supported.
    OSVERSIONINFO info;
    info.dwOSVersionInfoSize = sizeof(info);
    GetVersionEx(&info);
    if (info.dwMajorVersion > 4)
        // 2000 or later - "Global\\" prefix OK.
        _tcscpy(szGlobalMapping, _T("Global\\"));

    _tcscat(szGlobalMapping, szMappingName);

    m_hMappedObject = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, 4096, szGlobalMapping);
    if (m_hMappedObject == NULL) {
        PyWin_SetAPIError("CreateFileMapping");
        return FALSE;
    }
    // map the section and assign the counter block pointer
    // to this section of memory
    //
    m_pMapBlock = MapViewOfFile(m_hMappedObject, FILE_MAP_ALL_ACCESS, 0, 0, 0);
    if (m_pMapBlock == NULL) {
        // Failed to Map View of file
        PyWin_SetAPIError("MapViewOfFile");
        return FALSE;
    }

    m_pControl = (MappingManagerControlData *)m_pMapBlock;
    m_pControl->ControlSize = sizeof(MappingManagerControlData);
    m_pControl->TotalSize = sizeof(MappingManagerControlData);
    _tcsncpy(m_pControl->ServiceName, szServiceName, MMCD_SERVICE_SIZE);
    m_pControl->ServiceName[MMCD_SERVICE_SIZE] = _T('\0');

    _tcsncpy(m_pControl->EventSourceName, szEventSourceName, MMCD_EVENTSOURCE_SIZE);
    m_pControl->EventSourceName[MMCD_EVENTSOURCE_SIZE] = _T('\0');
    m_pControl->supplierStatus = SupplierStatusRunning;
    return TRUE;
}

void *MappingManager::AllocChunk(DWORD numBytes)
{
    if (!CheckStatus())
        return NULL;
    void *result = ((BYTE *)m_pMapBlock) + (m_pControl->TotalSize);
    m_pControl->TotalSize += numBytes;
    return result;
}

// @pymethod <o PyPerfMonManager>|perfmon|PerfMonManager|Creates a new PERF_OBJECT_TYPE object
PyObject *PerfmonMethod_NewPerfMonManager(PyObject *self, PyObject *args)
{
    PyObject *ret = NULL;
    PyObject *obServiceName;
    PyObject *obPerfObTypes;
    PyObject *obEventSourceName = Py_None;
    PyObject *obMappingName = Py_None;
    TCHAR *szMappingName = NULL;
    TCHAR *szEventSourceName = NULL;
    TCHAR *szServiceName = NULL;
    MappingManager *m_pmm = NULL;
    PyPerfMonManager *pPOT = NULL;

    if (!PyArg_ParseTuple(
            args, "OO|OO:PerfMonManager",
            &obServiceName,  // @pyparm <o PyUnicode>|serviceName||The name of the service for which data is being
                             // provided.
            &obPerfObTypes,  // @pyparm [<o PyPERF_OBJECT_TYPE>, ...]|seqPerfObTypes||A sequence of objects to use in
                             // the performance monitor.  At this stage, len(seqPerfObTypes) must == 1.
            &obMappingName,  // @pyparm <o PyUnicode>|mappingName|None|The name of the mapping to open.  This must be
                             // the same as the DLL name providing the information.  If None, the serviceName is used.
            &obEventSourceName))  // @pyparm <o PyUnicode>|eventSourceName|None|The name used by the DLL for error
                                  // messages in the registry.  If None, the serviceName is used.
        goto done;
    // @comm The application need not be a service, but it must have an entry in the
    // Services section of the registry.  This limits the performance monitor to being able to
    // provide only one 'counter type', but still many counters within that type.
    // See the documentation for the Performance Monitor API for more details.
    if (!PyWinObject_AsTCHAR(obServiceName, &szServiceName, FALSE))
        goto done;

    if (!PyWinObject_AsTCHAR(obEventSourceName, &szEventSourceName, TRUE))
        goto done;

    if (!PyWinObject_AsTCHAR(obMappingName, &szMappingName, TRUE))
        goto done;

    m_pmm = new MappingManager();
    if (m_pmm == NULL) {
        PyErr_SetString(PyExc_MemoryError, "Allocating memory for MappingManager");
        goto done;
    }
    if (!m_pmm->Init(szServiceName, szMappingName, szEventSourceName))
        // Init has set Python error
        goto done;

    pPOT = new (PyPerfMonManager);
    if (pPOT == NULL) {
        PyErr_SetString(PyExc_MemoryError, "Allocating MappingManager or PERF_OBJECT_TYPE");
    }
    if (!pPOT->Init(m_pmm, obPerfObTypes))
        goto done;
    // we worked!
    ret = pPOT;
done:
    if (szMappingName)
        PyWinObject_FreeTCHAR(szMappingName);
    if (szServiceName)
        PyWinObject_FreeTCHAR(szServiceName);
    if (szEventSourceName)
        PyWinObject_FreeTCHAR(szEventSourceName);
    if (ret == NULL) {  // we have an error
        if (m_pmm)
            delete m_pmm;
        if (pPOT)
            delete pPOT;
    }
    return ret;
}

// @pymethod |PyPerfMonManager|Close|Closes the performance monitor manager.
PyObject *PyPerfMonManager::Close(PyObject *self, PyObject *args)
{
    if (!PyArg_ParseTuple(args, ":Close"))
        return NULL;
    PyPerfMonManager *This = (PyPerfMonManager *)self;
    This->Term();
    Py_INCREF(Py_None);
    return Py_None;
}

// @object PyPerfMonManager|A Python object
struct PyMethodDef PyPerfMonManager::methods[] = {
    {"Close", PyPerfMonManager::Close, 1},  // @pymeth Close|Closes all counters.
    {NULL}};

PyTypeObject PyPerfMonManager::type = {
    PYWIN_OBJECT_HEAD "PyPerfMonManager",
    sizeof(PyPerfMonManager),
    0,
    PyPerfMonManager::deallocFunc, /* tp_dealloc */
    0,                             /* tp_print */
    0,                             /* tp_getattr */
    0,                             /* tp_setattr */
    0,                             /* tp_compare */
    0,                             /* tp_repr */
    0,                             /* tp_as_number */
    0,                             /* tp_as_sequence */
    0,                             /* tp_as_mapping */
    0,                             /* tp_hash */
    0,                             /* tp_call */
    0,                             /* tp_str */
    PyObject_GenericGetAttr,       /* tp_getattro */
    PyObject_GenericSetAttr,       /* tp_setattro */
    0,                             /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,            /* tp_flags */
    0,                             /* tp_doc */
    0,                             /* tp_traverse */
    0,                             /* tp_clear */
    0,                             /* tp_richcompare */
    0,                             /* tp_weaklistoffset */
    0,                             /* tp_iter */
    0,                             /* tp_iternext */
    PyPerfMonManager::methods,     /* tp_methods */
    PyPerfMonManager::members,     /* tp_members */
    0,                             /* tp_getset */
    0,                             /* tp_base */
    0,                             /* tp_dict */
    0,                             /* tp_descr_get */
    0,                             /* tp_descr_set */
    0,                             /* tp_dictoffset */
    0,                             /* tp_init */
    0,                             /* tp_alloc */
    0,                             /* tp_new */
};

#define OFF(e) offsetof(PyPerfMonManager, e)

/*static*/ struct PyMemberDef PyPerfMonManager::members[] = {
    {NULL} /* Sentinel */
};

PyPerfMonManager::PyPerfMonManager(void)
{
    ob_type = &type;
    _Py_NewReference(this);
    m_pmm = NULL;
    m_obPerfObTypes = NULL;
}

PyPerfMonManager::~PyPerfMonManager() { Term(); }

void PyPerfMonManager::Term()
{
    // Get each of our objects to terminate before we delete the memory...
    if (m_obPerfObTypes) {
        PyObject *obType = PySequence_GetItem(m_obPerfObTypes, 0);
        PyPERF_OBJECT_TYPE *pPerfOb;
        if (obType) {
            if (PyWinObject_AsPyPERF_OBJECT_TYPE(obType, &pPerfOb, FALSE))
                pPerfOb->Term();
        }
        Py_XDECREF(obType);
        Py_DECREF(m_obPerfObTypes);
        m_obPerfObTypes = NULL;
    }

    // Then cleanup our mapping
    if (m_pmm) {
        delete m_pmm;
        m_pmm = NULL;
    }
}

// Initialize the mapping with the Python object definitions.
BOOL PyPerfMonManager::Init(MappingManager *pmm, PyObject *obPerfObTypes)
{
    if (PySequence_Length(obPerfObTypes) != 1) {
        PyErr_SetString(PyExc_ValueError, "The sequence of PyPERF_OBJECT_TYPEs must have 1 item!");
        return NULL;
    }
    PyObject *obType = PySequence_GetItem(obPerfObTypes, 0);
    PyPERF_OBJECT_TYPE *pPerfOb;
    if (!obType)
        return FALSE;
    BOOL ok = TRUE;
    ok = ok && PyWinObject_AsPyPERF_OBJECT_TYPE(obType, &pPerfOb, FALSE);
    ok = ok && pPerfOb->InitMemoryLayout(pmm, this);
    Py_DECREF(obType);
    m_pmm = pmm;
    m_obPerfObTypes = obPerfObTypes;
    Py_INCREF(m_obPerfObTypes);
    return ok;
}

/*static*/ void PyPerfMonManager::deallocFunc(PyObject *ob) { delete (PyPerfMonManager *)ob; }
