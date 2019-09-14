#include "PythonCOM.h"
#include "PythonCOMRegister.h"
#include "mstask.h"
#include "PyITaskScheduler.h"
// # include "PyIScheduledWorkItem.h"
#include "PyITask.h"
#include "PyITaskTrigger.h"
#include "PyIProvideTaskPage.h"

static struct PyMethodDef taskscheduler_methods[] = {NULL};

static const PyCom_InterfaceSupportInfo register_data[] = {
    PYCOM_INTERFACE_CLSID_ONLY(CTaskScheduler),  PYCOM_INTERFACE_CLIENT_ONLY(TaskScheduler),
    PYCOM_INTERFACE_CLSID_ONLY(CTask),           PYCOM_INTERFACE_CLIENT_ONLY(Task),
    PYCOM_INTERFACE_CLIENT_ONLY(TaskTrigger),    PYCOM_INTERFACE_CLIENT_ONLY(ScheduledWorkItem),
    PYCOM_INTERFACE_CLIENT_ONLY(ProvideTaskPage)};

PYWIN_MODULE_INIT_FUNC(taskscheduler)
{
    PYWIN_MODULE_INIT_PREPARE(taskscheduler, taskscheduler_methods, "Supports the Scheduled Tasks COM interfaces");

    if (PyType_Ready(&PyTASK_TRIGGERType) == -1)
        PYWIN_MODULE_INIT_RETURN_ERROR;

    // Register all of our interfaces, gateways and IIDs.
    PyCom_RegisterExtensionSupport(dict, register_data, sizeof(register_data) / sizeof(PyCom_InterfaceSupportInfo));

    // trigger types
    PyModule_AddIntConstant(module, "TASK_TIME_TRIGGER_ONCE", TASK_TIME_TRIGGER_ONCE);
    PyModule_AddIntConstant(module, "TASK_TIME_TRIGGER_DAILY", TASK_TIME_TRIGGER_DAILY);
    PyModule_AddIntConstant(module, "TASK_TIME_TRIGGER_WEEKLY", TASK_TIME_TRIGGER_WEEKLY);
    PyModule_AddIntConstant(module, "TASK_TIME_TRIGGER_MONTHLYDATE", TASK_TIME_TRIGGER_MONTHLYDATE);
    PyModule_AddIntConstant(module, "TASK_TIME_TRIGGER_MONTHLYDOW", TASK_TIME_TRIGGER_MONTHLYDOW);
    PyModule_AddIntConstant(module, "TASK_EVENT_TRIGGER_ON_IDLE", TASK_EVENT_TRIGGER_ON_IDLE);
    PyModule_AddIntConstant(module, "TASK_EVENT_TRIGGER_AT_SYSTEMSTART", TASK_EVENT_TRIGGER_AT_SYSTEMSTART);
    PyModule_AddIntConstant(module, "TASK_EVENT_TRIGGER_AT_LOGON", TASK_EVENT_TRIGGER_AT_LOGON);

    // trigger flags
    PyModule_AddIntConstant(module, "TASK_TRIGGER_FLAG_HAS_END_DATE", TASK_TRIGGER_FLAG_HAS_END_DATE);
    PyModule_AddIntConstant(module, "TASK_TRIGGER_FLAG_KILL_AT_DURATION_END", TASK_TRIGGER_FLAG_KILL_AT_DURATION_END);
    PyModule_AddIntConstant(module, "TASK_TRIGGER_FLAG_DISABLED", TASK_TRIGGER_FLAG_DISABLED);

    // task statuses from msterr.h
    PyModule_AddIntConstant(module, "SCHED_S_TASK_READY", SCHED_S_TASK_READY);
    PyModule_AddIntConstant(module, "SCHED_S_TASK_NOT_SCHEDULED", SCHED_S_TASK_NOT_SCHEDULED);
    PyModule_AddIntConstant(module, "SCHED_S_TASK_RUNNING", SCHED_S_TASK_RUNNING);
    PyModule_AddIntConstant(module, "SCHED_S_TASK_DISABLED", SCHED_S_TASK_DISABLED);
    PyModule_AddIntConstant(module, "SCHED_S_TASK_HAS_NOT_RUN", SCHED_S_TASK_HAS_NOT_RUN);
    PyModule_AddIntConstant(module, "SCHED_S_TASK_NO_MORE_RUNS", SCHED_S_TASK_NO_MORE_RUNS);
    PyModule_AddIntConstant(module, "SCHED_S_TASK_TERMINATED", SCHED_S_TASK_TERMINATED);
    PyModule_AddIntConstant(module, "SCHED_S_TASK_NO_VALID_TRIGGERS", SCHED_S_TASK_NO_VALID_TRIGGERS);
    PyModule_AddIntConstant(module, "SCHED_S_EVENT_TRIGGER", SCHED_S_EVENT_TRIGGER);

    // error codes from msterr.h
    PyModule_AddIntConstant(module, "SCHED_E_TRIGGER_NOT_FOUND", SCHED_E_TRIGGER_NOT_FOUND);
    PyModule_AddIntConstant(module, "SCHED_E_TASK_NOT_READY", SCHED_E_TASK_NOT_READY);
    PyModule_AddIntConstant(module, "SCHED_E_TASK_NOT_RUNNING", SCHED_E_TASK_NOT_RUNNING);
    PyModule_AddIntConstant(module, "SCHED_E_SERVICE_NOT_INSTALLED", SCHED_E_SERVICE_NOT_INSTALLED);
    PyModule_AddIntConstant(module, "SCHED_E_CANNOT_OPEN_TASK", SCHED_E_CANNOT_OPEN_TASK);
    PyModule_AddIntConstant(module, "SCHED_E_INVALID_TASK", SCHED_E_INVALID_TASK);
    PyModule_AddIntConstant(module, "SCHED_E_ACCOUNT_INFORMATION_NOT_SET", SCHED_E_ACCOUNT_INFORMATION_NOT_SET);
    PyModule_AddIntConstant(module, "SCHED_E_ACCOUNT_NAME_NOT_FOUND", SCHED_E_ACCOUNT_NAME_NOT_FOUND);
    PyModule_AddIntConstant(module, "SCHED_E_ACCOUNT_DBASE_CORRUPT", SCHED_E_ACCOUNT_DBASE_CORRUPT);
    PyModule_AddIntConstant(module, "SCHED_E_ACCOUNT_DBASE_CORRUPT", SCHED_E_ACCOUNT_DBASE_CORRUPT);
    PyModule_AddIntConstant(module, "SCHED_E_UNKNOWN_OBJECT_VERSION", SCHED_E_UNKNOWN_OBJECT_VERSION);

    // priority codes
    PyModule_AddIntConstant(module, "REALTIME_PRIORITY_CLASS", REALTIME_PRIORITY_CLASS);
    PyModule_AddIntConstant(module, "HIGH_PRIORITY_CLASS", HIGH_PRIORITY_CLASS);
    PyModule_AddIntConstant(module, "NORMAL_PRIORITY_CLASS", NORMAL_PRIORITY_CLASS);
    PyModule_AddIntConstant(module, "IDLE_PRIORITY_CLASS", IDLE_PRIORITY_CLASS);

    // task flags
    PyModule_AddIntConstant(module, "TASK_FLAG_INTERACTIVE", TASK_FLAG_INTERACTIVE);
    PyModule_AddIntConstant(module, "TASK_FLAG_DELETE_WHEN_DONE", TASK_FLAG_DELETE_WHEN_DONE);
    PyModule_AddIntConstant(module, "TASK_FLAG_DISABLED", TASK_FLAG_DISABLED);
    PyModule_AddIntConstant(module, "TASK_FLAG_HIDDEN", TASK_FLAG_HIDDEN);
    PyModule_AddIntConstant(module, "TASK_FLAG_RUN_ONLY_IF_LOGGED_ON", TASK_FLAG_RUN_ONLY_IF_LOGGED_ON);
    PyModule_AddIntConstant(module, "TASK_FLAG_START_ONLY_IF_IDLE", TASK_FLAG_START_ONLY_IF_IDLE);
    PyModule_AddIntConstant(module, "TASK_FLAG_RUN_ONLY_IF_DOCKED", TASK_FLAG_RUN_ONLY_IF_DOCKED);
    PyModule_AddIntConstant(module, "TASK_FLAG_SYSTEM_REQUIRED", TASK_FLAG_SYSTEM_REQUIRED);
    PyModule_AddIntConstant(module, "TASK_FLAG_KILL_ON_IDLE_END", TASK_FLAG_KILL_ON_IDLE_END);
    PyModule_AddIntConstant(module, "TASK_FLAG_RESTART_ON_IDLE_RESUME", TASK_FLAG_RESTART_ON_IDLE_RESUME);
    PyModule_AddIntConstant(module, "TASK_FLAG_DONT_START_IF_ON_BATTERIES", TASK_FLAG_DONT_START_IF_ON_BATTERIES);
    PyModule_AddIntConstant(module, "TASK_FLAG_KILL_IF_GOING_ON_BATTERIES", TASK_FLAG_KILL_IF_GOING_ON_BATTERIES);
    PyModule_AddIntConstant(module, "TASK_FLAG_RUN_IF_CONNECTED_TO_INTERNET", TASK_FLAG_RUN_IF_CONNECTED_TO_INTERNET);

    // DOW constants
    PyModule_AddIntConstant(module, "TASK_SUNDAY", TASK_SUNDAY);
    PyModule_AddIntConstant(module, "TASK_MONDAY", TASK_MONDAY);
    PyModule_AddIntConstant(module, "TASK_TUESDAY", TASK_TUESDAY);
    PyModule_AddIntConstant(module, "TASK_WEDNESDAY", TASK_WEDNESDAY);
    PyModule_AddIntConstant(module, "TASK_THURSDAY", TASK_THURSDAY);
    PyModule_AddIntConstant(module, "TASK_FRIDAY", TASK_FRIDAY);
    PyModule_AddIntConstant(module, "TASK_SATURDAY", TASK_SATURDAY);

    // month contants
    PyModule_AddIntConstant(module, "TASK_JANUARY", TASK_JANUARY);
    PyModule_AddIntConstant(module, "TASK_FEBRUARY", TASK_FEBRUARY);
    PyModule_AddIntConstant(module, "TASK_MARCH", TASK_MARCH);
    PyModule_AddIntConstant(module, "TASK_APRIL", TASK_APRIL);
    PyModule_AddIntConstant(module, "TASK_MAY", TASK_MAY);
    PyModule_AddIntConstant(module, "TASK_JUNE", TASK_JUNE);
    PyModule_AddIntConstant(module, "TASK_JULY", TASK_JULY);
    PyModule_AddIntConstant(module, "TASK_AUGUST", TASK_AUGUST);
    PyModule_AddIntConstant(module, "TASK_SEPTEMBER", TASK_SEPTEMBER);
    PyModule_AddIntConstant(module, "TASK_OCTOBER", TASK_OCTOBER);
    PyModule_AddIntConstant(module, "TASK_NOVEMBER", TASK_NOVEMBER);
    PyModule_AddIntConstant(module, "TASK_DECEMBER", TASK_DECEMBER);

    // week nbr constants
    PyModule_AddIntConstant(module, "TASK_FIRST_WEEK", TASK_FIRST_WEEK);
    PyModule_AddIntConstant(module, "TASK_SECOND_WEEK", TASK_SECOND_WEEK);
    PyModule_AddIntConstant(module, "TASK_THIRD_WEEK", TASK_THIRD_WEEK);
    PyModule_AddIntConstant(module, "TASK_FOURTH_WEEK", TASK_FOURTH_WEEK);
    PyModule_AddIntConstant(module, "TASK_LAST_WEEK", TASK_LAST_WEEK);

    // property sheet identifiers
    PyModule_AddIntConstant(module, "TASKPAGE_TASK", TASKPAGE_TASK);
    PyModule_AddIntConstant(module, "TASKPAGE_SCHEDULE", TASKPAGE_SCHEDULE);
    PyModule_AddIntConstant(module, "TASKPAGE_SETTINGS", TASKPAGE_SETTINGS);

    PYWIN_MODULE_INIT_RETURN_SUCCESS;
}
