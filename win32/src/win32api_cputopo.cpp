// @doc - This file contains autoduck documentation
// CPU topology related types and functions for win32api
#include "PyWinTypes.h"
#include "structmember.h"
#include "PyWinObjects.h"
#include "win32api_cputopo.h"

// from kernel32.dll, loaded in win32api's init function
// Not available before Windows 10 / Windows Server 2016
GetSystemCpuSetInformationfunc pfnGetSystemCpuSetInformation = NULL;

struct PyMemberDef PySYSTEM_CPU_SET_INFORMATION::members[] = {
    // @prop int|Id|Unique ID for this CPU set
    {"Id", T_UINT, offsetof(PySYSTEM_CPU_SET_INFORMATION, Id), READONLY, "Unique ID for this CPU set"},
    // @prop int|Group|Processor group (>0 only if more than 64 logical processors)
    {"Group", T_USHORT, offsetof(PySYSTEM_CPU_SET_INFORMATION, Group), READONLY,
     "Processor group (>0 only if more than 64 logical processors)"},
    // @prop int|LogicalProcessorIndex|Logical processor index within the group
    {"LogicalProcessorIndex", T_UBYTE, offsetof(PySYSTEM_CPU_SET_INFORMATION, LogicalProcessorIndex), READONLY,
     "Logical processor index within the group"},
    // @prop int|CoreIndex|Physical core index
    {"CoreIndex", T_UBYTE, offsetof(PySYSTEM_CPU_SET_INFORMATION, CoreIndex), READONLY, "Physical core index"},
    // @prop int|LastLevelCacheIndex|Last level cache index
    {"LastLevelCacheIndex", T_UBYTE, offsetof(PySYSTEM_CPU_SET_INFORMATION, LastLevelCacheIndex), READONLY,
     "Last level cache index"},
    // @prop int|NumaNodeIndex|NUMA node index
    {"NumaNodeIndex", T_UBYTE, offsetof(PySYSTEM_CPU_SET_INFORMATION, NumaNodeIndex), READONLY, "NUMA node index"},
    // @prop int|EfficiencyClass|Efficiency class (0=E-core, 1+=P-core on hybrid CPUs)
    {"EfficiencyClass", T_UBYTE, offsetof(PySYSTEM_CPU_SET_INFORMATION, EfficiencyClass), READONLY,
     "Efficiency class (0=E-core, 1+=P-core on heterogenous CPU architectures)"},
    // @prop int|SchedulingClass|Scheduling class for different performance cores
    {"SchedulingClass", T_UBYTE, offsetof(PySYSTEM_CPU_SET_INFORMATION, SchedulingClass), READONLY,
     "Scheduling class for different performance cores"},
    // @prop int|AllocationTag|Allocation tag for affinity purposes
    {"AllocationTag", T_ULONGLONG, offsetof(PySYSTEM_CPU_SET_INFORMATION, AllocationTag), READONLY,
     "Allocation tag for affinity purposes"},
    {NULL}};

PyTypeObject PySYSTEM_CPU_SET_INFORMATIONType = {
    PYWIN_OBJECT_HEAD "PySYSTEM_CPU_SET_INFORMATION",
    sizeof(PySYSTEM_CPU_SET_INFORMATION),
    0,
    PySYSTEM_CPU_SET_INFORMATION::tp_dealloc,
    0,                                       // tp_vectorcall_offset
    0,                                       // tp_getattr
    0,                                       // tp_setattr
    0,                                       // tp_as_async
    PySYSTEM_CPU_SET_INFORMATION::tp_str,    // tp_repr
    0,                                       // tp_as_number
    0,                                       // tp_as_sequence
    0,                                       // tp_as_mapping
    0,                                       // tp_hash
    0,                                       // tp_call
    PySYSTEM_CPU_SET_INFORMATION::tp_str,    // tp_str
    PySYSTEM_CPU_SET_INFORMATION::getattro,  // tp_getattro
    PyObject_GenericSetAttr,                 // tp_setattro
    0,                                       // tp_as_buffer
    Py_TPFLAGS_DEFAULT,                      // tp_flags
    "Wrapper for SYSTEM_CPU_SET_INFORMATION. Contains CPU topology info for a logical processor.",  // tp_doc
    0,                                                                                              // tp_traverse
    0,                                                                                              // tp_clear
    0,                                                                                              // tp_richcompare
    0,                                                                                              // tp_weaklistoffset
    0,                                                                                              // tp_iter
    0,                                                                                              // tp_iternext
    0,                                                                                              // tp_methods
    PySYSTEM_CPU_SET_INFORMATION::members,
    0,  // tp_getset
    0,  // tp_base
    0,  // tp_dict
    0,  // tp_descr_get
    0,  // tp_descr_set
    0,  // tp_dictoffset
    0,  // tp_init
    0,  // tp_alloc
    PySYSTEM_CPU_SET_INFORMATION::tp_new};

PySYSTEM_CPU_SET_INFORMATION::PySYSTEM_CPU_SET_INFORMATION(void)
{
    ob_type = &PySYSTEM_CPU_SET_INFORMATIONType;
    Id = 0;
    Group = 0;
    LogicalProcessorIndex = 0;
    CoreIndex = 0;
    LastLevelCacheIndex = 0;
    NumaNodeIndex = 0;
    EfficiencyClass = 0;
    SchedulingClass = 0;
    AllocationTag = 0;
    AllFlags = 0;
    _Py_NewReference(this);
}

PySYSTEM_CPU_SET_INFORMATION::PySYSTEM_CPU_SET_INFORMATION(PSYSTEM_CPU_SET_INFORMATION pInfo)
{
    ob_type = &PySYSTEM_CPU_SET_INFORMATIONType;
    Id = pInfo->CpuSet.Id;
    Group = pInfo->CpuSet.Group;
    LogicalProcessorIndex = pInfo->CpuSet.LogicalProcessorIndex;
    CoreIndex = pInfo->CpuSet.CoreIndex;
    LastLevelCacheIndex = pInfo->CpuSet.LastLevelCacheIndex;
    NumaNodeIndex = pInfo->CpuSet.NumaNodeIndex;
    EfficiencyClass = pInfo->CpuSet.EfficiencyClass;
    SchedulingClass = pInfo->CpuSet.SchedulingClass;
    AllocationTag = pInfo->CpuSet.AllocationTag;
    AllFlags = pInfo->CpuSet.AllFlags;
    _Py_NewReference(this);
}

PySYSTEM_CPU_SET_INFORMATION::~PySYSTEM_CPU_SET_INFORMATION() {}

void PySYSTEM_CPU_SET_INFORMATION::tp_dealloc(PyObject *ob) { delete (PySYSTEM_CPU_SET_INFORMATION *)ob; }

PyObject *PySYSTEM_CPU_SET_INFORMATION::tp_str(PyObject *self)
{
    PySYSTEM_CPU_SET_INFORMATION *pThis = (PySYSTEM_CPU_SET_INFORMATION *)self;
    char buf[200];
    int chars_printed =
        _snprintf(buf, 200, "PySYSTEM_CPU_SET_INFORMATION(Id=%lu, LP=%u, Core=%u, Eff=%u, Sched=%u)", pThis->Id,
                  pThis->LogicalProcessorIndex, pThis->CoreIndex, pThis->EfficiencyClass, pThis->SchedulingClass);
    if (chars_printed < 0) {
        PyErr_SetString(PyExc_SystemError, "String representation too long for buffer");
        return NULL;
    }
    return PyWinCoreString_FromString(buf, chars_printed);
}

PyObject *PySYSTEM_CPU_SET_INFORMATION::getattro(PyObject *self, PyObject *obname)
{
    PySYSTEM_CPU_SET_INFORMATION *psystem_cpu_set_info = (PySYSTEM_CPU_SET_INFORMATION *)self;
    char *name = PYWIN_ATTR_CONVERT(obname);
    if (name == NULL)
        return NULL;

    // @prop bool|Parked|True if this CPU is parked for power saving
    if (strcmp(name, "Parked") == 0)
        return PyBool_FromLong((psystem_cpu_set_info->AllFlags & 0x1) != 0);
    // @prop bool|Allocated|True if allocated to a specific process
    if (strcmp(name, "Allocated") == 0)
        return PyBool_FromLong((psystem_cpu_set_info->AllFlags & 0x2) != 0);
    // @prop bool|AllocatedToTargetProcess|True if allocated to the target process
    if (strcmp(name, "AllocatedToTargetProcess") == 0)
        return PyBool_FromLong((psystem_cpu_set_info->AllFlags & 0x4) != 0);
    // @prop bool|RealTime|True if reserved for real-time use
    if (strcmp(name, "RealTime") == 0)
        return PyBool_FromLong((psystem_cpu_set_info->AllFlags & 0x8) != 0);

    return PyObject_GenericGetAttr(self, obname);
}

PyObject *PySYSTEM_CPU_SET_INFORMATION::tp_new(PyTypeObject *tp, PyObject *args, PyObject *kwargs)
{
    PySYSTEM_CPU_SET_INFORMATION *self = new PySYSTEM_CPU_SET_INFORMATION();
    if (self == NULL) {
        PyErr_SetString(PyExc_MemoryError, "Unable to create PySYSTEM_CPU_SET_INFORMATION");
        return NULL;
    }
    return self;
}

PyObject *PyWinObject_FromSYSTEM_CPU_SET_INFORMATION(PSYSTEM_CPU_SET_INFORMATION pInfo)
{
    if (pInfo == NULL || pInfo->Type != CpuSetInformation)
        return NULL;  // Skip non-CpuSetInformation entries

    PyObject *ret = new PySYSTEM_CPU_SET_INFORMATION(pInfo);
    if (ret == NULL) {
        PyErr_SetString(PyExc_MemoryError, "Unable to create PySYSTEM_CPU_SET_INFORMATION");
        return NULL;
    }
    return ret;
}

// @pymethod list|win32api|GetSystemCpuSetInformation|Returns CPU Set information for all logical processors.
// @comm This function retrieves CPU topology information including efficiency class (P-core vs E-core),
// scheduling class, NUMA node, cache topology, and processor state flags.
// @rdesc A list of <o PySYSTEM_CPU_SET_INFORMATION> objects, one for each logical processor.
PyObject *PyGetSystemCpuSetInformation(PyObject *self, PyObject *args)
{
    CHECK_PFN(GetSystemCpuSetInformation);
    if (!PyArg_ParseTuple(args, ":GetSystemCpuSetInformation"))
        return NULL;

    // @pyseeapi GetSystemCpuSetInformation
    ULONG length = 0;

    // first call to get required buffer size
    (*pfnGetSystemCpuSetInformation)(NULL, 0, &length, NULL, 0);
    if (length == 0)
        return PyErr_Format(PyExc_RuntimeError, "GetSystemCpuSetInformation returned zero length");

    PSYSTEM_CPU_SET_INFORMATION buffer = (PSYSTEM_CPU_SET_INFORMATION)malloc(length);
    if (buffer == NULL)
        return PyErr_NoMemory();

    // second call to get the actual data
    BOOL result;
    Py_BEGIN_ALLOW_THREADS;
    result = (*pfnGetSystemCpuSetInformation)(buffer, length, &length, NULL, 0);
    Py_END_ALLOW_THREADS;

    if (!result) {
        DWORD err = GetLastError();
        free(buffer);
        return PyWin_SetAPIError("GetSystemCpuSetInformation", err);
    }

    PyObject *ret = PyList_New(0);
    if (ret == NULL) {
        free(buffer);
        return NULL;
    }

    // walk through structure and parse out cpu set info entries
    ULONG bytesProcessed = 0;  // proxy for index into a CPU core's info in buffer
    PSYSTEM_CPU_SET_INFORMATION current = buffer;
    while (bytesProcessed < length) {
        // skip anything with an unexpected type
        if (current->Type == CpuSetInformation) {
            PyObject *item = PyWinObject_FromSYSTEM_CPU_SET_INFORMATION(current);
            if (item == NULL) {
                Py_DECREF(ret);
                free(buffer);
                return NULL;
            }

            int success = PyList_Append(ret, item);
            Py_DECREF(item);

            if (success == -1) {
                Py_DECREF(ret);
                free(buffer);
                return NULL;
            }
        }

        // step to next entry in buffer (i.e., next core)
        bytesProcessed += current->Size;
        current = (PSYSTEM_CPU_SET_INFORMATION)((BYTE *)current + current->Size);
    }

    free(buffer);
    return ret;
}
