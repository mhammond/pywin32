// Objects for use with the perfmon objects and Python.
//
// These objects keep copies of their key data for use by Python.
// They also contain a pointer to an underlying PERF_COUNTER_DEFINITION
// structure - but not immediately.
// The user sets up all the counter information, then adds them
// to a PERF_BLOCK structure, which allocates the underlying
// PERF_COUNTER_DEFINITION memory, inline with all others in the group.
//
#include "PyPerfMonControl.h"

// NOT a Python object, but a helper class which does basic admin of the
// shared memory.
class MappingManager {
   public:
    MappingManager();
    ~MappingManager();
    BOOL Init(const TCHAR *szServiceName, const TCHAR *mapName = NULL, const TCHAR *szEventSourceName = NULL);
    BOOL CheckStatus();
    void *AllocChunk(DWORD size);

   private:
    DWORD *m_pBytesUsed;  // Pointer to first few bytes in the mmapped file.
    HANDLE m_hMappedObject;
    void *m_pMapBlock;
    MappingManagerControlData *m_pControl;
};

class PyPerfMonManager : public PyObject {
   public:
    PyPerfMonManager();
    ~PyPerfMonManager();

    void Term();
    BOOL Init(MappingManager *pmm, PyObject *obPerfObjectTypes);

    /* Python support */
    static PyObject *Close(PyObject *self, PyObject *args);
    static void deallocFunc(PyObject *ob);

    static struct PyMemberDef members[];
    static struct PyMethodDef methods[];
    static PyTypeObject type;

   protected:
    MappingManager *m_pmm;
    PyObject *m_obPerfObTypes;
};

class PyPERF_COUNTER_DEFINITION : public PyObject {
   public:
    PyPERF_COUNTER_DEFINITION(DWORD counterNameTitleIndex);
    ~PyPERF_COUNTER_DEFINITION();

    PERF_COUNTER_DEFINITION *GetPCD() { return m_pPCD; }

    DWORD GetCounterDataSize() { return m_CounterSize; }
    DWORD GetDetailLevel() { return m_DetailLevel; }

    void AcceptBuffer(PyObject *obOwner, void *buffer);
    void SetupBuffer(void);
    void AcceptRawCounterBuffer(void *pBuffer, DWORD offset);

    /* Python support */
    static void deallocFunc(PyObject *ob);

    static PyObject *getattro(PyObject *self, PyObject *obname);
    static int setattro(PyObject *self, PyObject *obname, PyObject *v);

    static PyObject *Increment(PyObject *self, PyObject *args);
    static PyObject *Decrement(PyObject *self, PyObject *args);
    static PyObject *Set(PyObject *self, PyObject *args);
    static PyObject *Get(PyObject *self, PyObject *args);

    static struct PyMemberDef members[];
    static struct PyMethodDef methods[];
    static PyTypeObject type;

   protected:
    PERF_COUNTER_DEFINITION *m_pPCD;
    // Reference kept to owner of the underlying buffer in the shared mem.
    PyObject *m_obBufferOwner;
    // The counter itself (ie, the raw integer!) - or NULL if not yet setup.
    void *m_pCounterValue;
    DWORD m_DefaultScale;
    DWORD m_DetailLevel;
    DWORD m_CounterNameTitleIndex;
    DWORD m_CounterHelpTitleIndex;
    DWORD m_CounterType;
    DWORD m_CounterSize;
};

#define PyPERF_COUNTER_DEFINITION_Check(ob) ((ob)->ob_type == &PyPERF_COUNTER_DEFINITION::type)
BOOL PyWinObject_AsPyPERF_COUNTER_DEFINITION(PyObject *ob, PyPERF_COUNTER_DEFINITION **ppPERF_COUNTER_DEFINITION,
                                             BOOL bNoneOK /*= TRUE*/);

class PyPERF_OBJECT_TYPE : public PyObject {
   public:
    PyPERF_OBJECT_TYPE();
    ~PyPERF_OBJECT_TYPE();

    PERF_OBJECT_TYPE *GetPCD() { return m_pPOT; }

    BOOL InitPythonObjects(PyObject *obCounters);
    BOOL InitMemoryLayout(MappingManager *mm, PyPerfMonManager *obPMM);
    void Term();

    /* Python support */
    static void deallocFunc(PyObject *ob);
    static PyObject *Close(PyObject *self, PyObject *args);
    static struct PyMemberDef members[];
    static struct PyMethodDef methods[];
    static PyTypeObject type;

   protected:
    PERF_OBJECT_TYPE *m_pPOT;
    DWORD m_ObjectNameTitleIndex;
    DWORD m_ObjectHelpTitleIndex;
    DWORD m_DefaultCounter;
    PyObject *m_obCounters;
    PyObject *m_obPerfMonManager;
};

#define PyPERF_OBJECT_TYPE_Check(ob) ((ob)->ob_type == &PyPERF_OBJECT_TYPE::type)
BOOL PyWinObject_AsPyPERF_OBJECT_TYPE(PyObject *ob, PyPERF_OBJECT_TYPE **ppPyPERF_OBJECT_TYPE, BOOL bNoneOK /*= TRUE*/);
