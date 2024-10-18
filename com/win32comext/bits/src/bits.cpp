#include "bits_pch.h"
#include "PyIBackgroundCopyManager.h"
#include "PyIBackgroundCopyCallback.h"
#include "PyIBackgroundCopyJob.h"
#include "PyIBackgroundCopyJob2.h"
#include "PyIBackgroundCopyJob3.h"
#include "PyIBackgroundCopyFile.h"
#include "PyIBackgroundCopyFile2.h"
#include "PyIBackgroundCopyError.h"
#include "PyIEnumBackgroundCopyJobs.h"
#include "PyIEnumBackgroundCopyFiles.h"

#include "PythonCOMRegister.h"
// @doc

BOOL PyObject_AsBG_FILE_INFO_LIST(PyObject *ob, ULONG *pnum, BG_FILE_INFO **fi)
{
    PyErr_SetString(PyExc_NotImplementedError, "fix me");
    return FALSE;
}

void PyObject_FreeBG_FILE_INFO_LIST(ULONG pnum, BG_FILE_INFO *fi) {}

BOOL PyObject_AsBG_FILE_RANGE_LIST(PyObject *ob, DWORD *pnum, BG_FILE_RANGE **fr)
{
    PyErr_SetString(PyExc_NotImplementedError, "fix me");
    return FALSE;
}
void PyObject_FreeBG_FILE_RANGE_LIST(DWORD num, BG_FILE_RANGE *fr) {}

PyObject *PyObject_FromBG_FILE_PROGRESS(BG_FILE_PROGRESS *fp)
{
    // @object PyObject_FromBG_FILE_PROGRESS|A tuple of 3 elements (bytesTotal, bytesTransferred, completed), (int, int,
    // bool)
    return Py_BuildValue("NNO", PyLong_FromLongLong(fp->BytesTotal), PyLong_FromLongLong(fp->BytesTransferred),
                         fp->Completed ? Py_True : Py_False);
}

PyObject *PyObject_FromBG_JOB_PROGRESS(BG_JOB_PROGRESS *jp)
{
    // @object PyObject_FromBG_JOB_PROGRESS|A tuple of 4 elements (bytesTotal, bytesTransferred, filesTotal,
    // filesTransferred), all ints.
    return Py_BuildValue("NNkk", PyLong_FromLongLong(jp->BytesTotal), PyLong_FromLongLong(jp->BytesTransferred),
                         jp->FilesTotal, jp->FilesTransferred);
}
PyObject *PyObject_FromBG_JOB_REPLY_PROGRESS(BG_JOB_REPLY_PROGRESS *jrs)
{
    // @object BG_JOB_REPLY_PROGRESS|A tuple of 2 elements (bytesTotal, bytesTransferred), both ints.
    return Py_BuildValue("NN", PyLong_FromLongLong(jrs->BytesTotal), PyLong_FromLongLong(jrs->BytesTransferred));
}

PyObject *MakeTimeOrNone(const FILETIME &t)
{
    if (t.dwLowDateTime == 0 && t.dwHighDateTime == 0) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    return PyWinObject_FromFILETIME(t);
}

PyObject *PyObject_FromBG_JOB_TIMES(BG_JOB_TIMES *jt)
{
    // @object BG_JOB_TIMES|A tuple of 3 elements, where each element may be
    // None or a <o PyDateTime> object.  The elements are the CreationTime,
    // ModificationTime and TransferCompletionTime, respectively.
    return Py_BuildValue("NNN", MakeTimeOrNone(jt->CreationTime), MakeTimeOrNone(jt->ModificationTime),
                         MakeTimeOrNone(jt->TransferCompletionTime));
}

BOOL PyObject_AsBG_AUTH_CREDENTIALS(PyObject *ob, BG_AUTH_CREDENTIALS *pRet)
{
    PyErr_SetString(PyExc_NotImplementedError, "fix me");
    return FALSE;
}

static struct PyMethodDef bits_methods[] = {NULL};

static const PyCom_InterfaceSupportInfo register_data[] = {
    PYCOM_INTERFACE_CLSID_ONLY(BackgroundCopyManager),   PYCOM_INTERFACE_CLIENT_ONLY(BackgroundCopyManager),
    PYCOM_INTERFACE_SERVER_ONLY(BackgroundCopyCallback), PYCOM_INTERFACE_CLIENT_ONLY(BackgroundCopyError),
    PYCOM_INTERFACE_CLIENT_ONLY(BackgroundCopyJob),      PYCOM_INTERFACE_CLIENT_ONLY(BackgroundCopyJob2),
    PYCOM_INTERFACE_CLIENT_ONLY(BackgroundCopyJob3),     PYCOM_INTERFACE_CLIENT_ONLY(BackgroundCopyFile),
    PYCOM_INTERFACE_CLIENT_ONLY(BackgroundCopyFile2),    PYCOM_INTERFACE_CLIENT_ONLY(EnumBackgroundCopyJobs),
    PYCOM_INTERFACE_CLIENT_ONLY(EnumBackgroundCopyFiles)

};

PYWIN_MODULE_INIT_FUNC(bits)
{
    PYWIN_MODULE_INIT_PREPARE(bits, bits_methods,
                              "A module, encapsulating the Background Intelligent Transfer Service (bits)");

    // Register all of our interfaces, gateways and IIDs.
    PyCom_RegisterExtensionSupport(dict, register_data, sizeof(register_data) / sizeof(PyCom_InterfaceSupportInfo));

    // auth scheme
    PyModule_AddIntConstant(module, "BG_AUTH_SCHEME_BASIC", BG_AUTH_SCHEME_BASIC);
    PyModule_AddIntConstant(module, "BG_AUTH_SCHEME_DIGEST", BG_AUTH_SCHEME_DIGEST);
    PyModule_AddIntConstant(module, "BG_AUTH_SCHEME_NTLM", BG_AUTH_SCHEME_NTLM);
    PyModule_AddIntConstant(module, "BG_AUTH_SCHEME_NEGOTIATE", BG_AUTH_SCHEME_NEGOTIATE);
    PyModule_AddIntConstant(module, "BG_AUTH_SCHEME_PASSPORT", BG_AUTH_SCHEME_PASSPORT);

    // auth target
    PyModule_AddIntConstant(module, "BG_AUTH_TARGET_SERVER", BG_AUTH_TARGET_SERVER);
    PyModule_AddIntConstant(module, "BG_AUTH_TARGET_PROXY", BG_AUTH_TARGET_PROXY);

    // 	// cert store location
    PyModule_AddIntConstant(module, "BG_CERT_STORE_LOCATION_CURRENT_USER", BG_CERT_STORE_LOCATION_CURRENT_USER);
    PyModule_AddIntConstant(module, "BG_CERT_STORE_LOCATION_LOCAL_MACHINE", BG_CERT_STORE_LOCATION_LOCAL_MACHINE);
    PyModule_AddIntConstant(module, "BG_CERT_STORE_LOCATION_CURRENT_SERVICE", BG_CERT_STORE_LOCATION_CURRENT_SERVICE);
    PyModule_AddIntConstant(module, "BG_CERT_STORE_LOCATION_SERVICES", BG_CERT_STORE_LOCATION_SERVICES);
    PyModule_AddIntConstant(module, "BG_CERT_STORE_LOCATION_USERS", BG_CERT_STORE_LOCATION_USERS);
    PyModule_AddIntConstant(module, "BG_CERT_STORE_LOCATION_CURRENT_USER_GROUP_POLICY",
                            BG_CERT_STORE_LOCATION_CURRENT_USER_GROUP_POLICY);
    PyModule_AddIntConstant(module, "BG_CERT_STORE_LOCATION_LOCAL_MACHINE_GROUP_POLICY",
                            BG_CERT_STORE_LOCATION_LOCAL_MACHINE_GROUP_POLICY);
    PyModule_AddIntConstant(module, "BG_CERT_STORE_LOCATION_LOCAL_MACHINE_ENTERPRISE",
                            BG_CERT_STORE_LOCATION_LOCAL_MACHINE_ENTERPRISE);

    // error context
    PyModule_AddIntConstant(module, "BG_ERROR_CONTEXT_NONE", BG_ERROR_CONTEXT_NONE);
    PyModule_AddIntConstant(module, "BG_ERROR_CONTEXT_UNKNOWN", BG_ERROR_CONTEXT_UNKNOWN);
    PyModule_AddIntConstant(module, "BG_ERROR_CONTEXT_GENERAL_QUEUE_MANAGER", BG_ERROR_CONTEXT_GENERAL_QUEUE_MANAGER);
    PyModule_AddIntConstant(module, "BG_ERROR_CONTEXT_QUEUE_MANAGER_NOTIFICATION",
                            BG_ERROR_CONTEXT_QUEUE_MANAGER_NOTIFICATION);
    PyModule_AddIntConstant(module, "BG_ERROR_CONTEXT_LOCAL_FILE", BG_ERROR_CONTEXT_LOCAL_FILE);
    PyModule_AddIntConstant(module, "BG_ERROR_CONTEXT_REMOTE_FILE", BG_ERROR_CONTEXT_REMOTE_FILE);
    PyModule_AddIntConstant(module, "BG_ERROR_CONTEXT_GENERAL_TRANSPORT", BG_ERROR_CONTEXT_GENERAL_TRANSPORT);
    PyModule_AddIntConstant(module, "BG_ERROR_CONTEXT_REMOTE_APPLICATION", BG_ERROR_CONTEXT_REMOTE_APPLICATION);

    // job priority
    PyModule_AddIntConstant(module, "BG_JOB_PRIORITY_FOREGROUND", BG_JOB_PRIORITY_FOREGROUND);
    PyModule_AddIntConstant(module, "BG_JOB_PRIORITY_HIGH", BG_JOB_PRIORITY_HIGH);
    PyModule_AddIntConstant(module, "BG_JOB_PRIORITY_NORMAL", BG_JOB_PRIORITY_NORMAL);
    PyModule_AddIntConstant(module, "BG_JOB_PRIORITY_LOW", BG_JOB_PRIORITY_LOW);

    // job proxy usage
    PyModule_AddIntConstant(module, "BG_JOB_PROXY_USAGE_PRECONFIG", BG_JOB_PROXY_USAGE_PRECONFIG);
    PyModule_AddIntConstant(module, "BG_JOB_PROXY_USAGE_NO_PROXY", BG_JOB_PROXY_USAGE_NO_PROXY);
    PyModule_AddIntConstant(module, "BG_JOB_PROXY_USAGE_OVERRIDE", BG_JOB_PROXY_USAGE_OVERRIDE);
    PyModule_AddIntConstant(module, "BG_JOB_PROXY_USAGE_AUTODETECT", BG_JOB_PROXY_USAGE_AUTODETECT);

    // job state
    PyModule_AddIntConstant(module, "BG_JOB_STATE_QUEUED", BG_JOB_STATE_QUEUED);
    PyModule_AddIntConstant(module, "BG_JOB_STATE_CONNECTING", BG_JOB_STATE_CONNECTING);
    PyModule_AddIntConstant(module, "BG_JOB_STATE_TRANSFERRING", BG_JOB_STATE_TRANSFERRING);
    PyModule_AddIntConstant(module, "BG_JOB_STATE_SUSPENDED", BG_JOB_STATE_SUSPENDED);
    PyModule_AddIntConstant(module, "BG_JOB_STATE_ERROR", BG_JOB_STATE_ERROR);
    PyModule_AddIntConstant(module, "BG_JOB_STATE_TRANSIENT_ERROR", BG_JOB_STATE_TRANSIENT_ERROR);
    PyModule_AddIntConstant(module, "BG_JOB_STATE_TRANSFERRED", BG_JOB_STATE_TRANSFERRED);
    PyModule_AddIntConstant(module, "BG_JOB_STATE_ACKNOWLEDGED", BG_JOB_STATE_ACKNOWLEDGED);
    PyModule_AddIntConstant(module, "BG_JOB_STATE_CANCELLED", BG_JOB_STATE_CANCELLED);

    // job type
    PyModule_AddIntConstant(module, "BG_JOB_TYPE_DOWNLOAD", BG_JOB_TYPE_DOWNLOAD);
    PyModule_AddIntConstant(module, "BG_JOB_TYPE_UPLOAD", BG_JOB_TYPE_UPLOAD);
    PyModule_AddIntConstant(module, "BG_JOB_TYPE_UPLOAD_REPLY", BG_JOB_TYPE_UPLOAD_REPLY);

    // notify flags
    PyModule_AddIntConstant(module, "BG_NOTIFY_JOB_TRANSFERRED", BG_NOTIFY_JOB_TRANSFERRED);
    PyModule_AddIntConstant(module, "BG_NOTIFY_JOB_ERROR", BG_NOTIFY_JOB_ERROR);
    PyModule_AddIntConstant(module, "BG_NOTIFY_DISABLE", BG_NOTIFY_DISABLE);
    PyModule_AddIntConstant(module, "BG_NOTIFY_JOB_MODIFICATION", BG_NOTIFY_JOB_MODIFICATION);
    // PyModule_AddIntConstant(module, "BG_NOTIFY_FILE_TRANSFERRED", BG_NOTIFY_FILE_TRANSFERRED);

    PyModule_AddIntConstant(module, "BG_JOB_ENUM_ALL_USERS", BG_JOB_ENUM_ALL_USERS);
    PYWIN_MODULE_INIT_RETURN_SUCCESS;
}
