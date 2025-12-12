#define CHECK_PFN(fname)    \
    if (pfn##fname == NULL) \
        return PyErr_Format(PyExc_NotImplementedError, "%s is not available on this platform", #fname);

PyObject *PyGetSystemCpuSetInformation(PyObject *self, PyObject *args);

typedef BOOL(WINAPI *GetSystemCpuSetInformationfunc)(PSYSTEM_CPU_SET_INFORMATION, ULONG, PULONG, HANDLE, ULONG);
extern GetSystemCpuSetInformationfunc pfnGetSystemCpuSetInformation;

extern __declspec(dllexport) PyTypeObject PySYSTEM_CPU_SET_INFORMATIONType;
extern PyObject *PyWinObject_FromSYSTEM_CPU_SET_INFORMATION(PSYSTEM_CPU_SET_INFORMATION pInfo);


// @object PySYSTEM_CPU_SET_INFORMATION|Wrapper for a SYSTEM_CPU_SET_INFORMATION struct.
// Provides CPU topology information for a single logical processor.
class PySYSTEM_CPU_SET_INFORMATION : public PyObject {
   public:
        PySYSTEM_CPU_SET_INFORMATION(void);
        PySYSTEM_CPU_SET_INFORMATION(PSYSTEM_CPU_SET_INFORMATION pInfo);

        static struct PyMemberDef members[];
        static void tp_dealloc(PyObject *ob);
        static PyObject *tp_str(PyObject *self);
        static PyObject *getattro(PyObject *self, PyObject *obname);
        static PyObject *tp_new(PyTypeObject *tp, PyObject *args, PyObject *kwargs);
        
    protected:
        DWORD Id;
        WORD Group;
        BYTE LogicalProcessorIndex;
        BYTE CoreIndex;
        BYTE LastLevelCacheIndex;
        BYTE NumaNodeIndex;
        BYTE EfficiencyClass;
        BYTE SchedulingClass;
        DWORD64 AllocationTag;
        DWORD AllFlags;
        ~PySYSTEM_CPU_SET_INFORMATION();
};
