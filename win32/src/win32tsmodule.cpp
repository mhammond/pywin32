// @doc
#include "PyWinTypes.h"
#include "PyWinObjects.h"
#include "structmember.h"

#include "WtsApi32.h"
#include "malloc.h"

#define CHECK_PFN(fname)    \
    if (pfn##fname == NULL) \
        return PyErr_Format(PyExc_NotImplementedError, "%s is not available on this platform", #fname);
typedef DWORD(WINAPI *WTSGetActiveConsoleSessionIdfunc)(VOID);
static WTSGetActiveConsoleSessionIdfunc pfnWTSGetActiveConsoleSessionId = NULL;
typedef BOOL(WINAPI *WTSQueryUserTokenfunc)(ULONG, PHANDLE);
static WTSQueryUserTokenfunc pfnWTSQueryUserToken = NULL;
typedef BOOL(WINAPI *WTSRegisterSessionNotificationfunc)(HWND, DWORD);
static WTSRegisterSessionNotificationfunc pfnWTSRegisterSessionNotification = NULL;
typedef BOOL(WINAPI *WTSUnRegisterSessionNotificationfunc)(HWND);
static WTSUnRegisterSessionNotificationfunc pfnWTSUnRegisterSessionNotification = NULL;

typedef BOOL(WINAPI *ProcessIdToSessionIdfunc)(DWORD, DWORD *);
static ProcessIdToSessionIdfunc pfnProcessIdToSessionId = NULL;

typedef BOOL(WINAPI *WTSVirtualChannelQueryfunc)(HANDLE, WTS_VIRTUAL_CLASS, PVOID *, DWORD *);
static WTSVirtualChannelQueryfunc pfnWTSVirtualChannelQuery = NULL;

// @object PyTS_HANDLE|Handle to a Terminal Server
class PyTS_HANDLE : public PyHANDLE {
   public:
    PyTS_HANDLE(HANDLE hInit) : PyHANDLE(hInit) {}
    virtual BOOL Close(void)
    {
        if (m_handle) {
            // No return value
            WTSCloseServer(m_handle);
            m_handle = 0;
        }
        return TRUE;
    }
    virtual const char *GetTypeName() { return "PyTS_HANDLE"; }
};

// @pymethod <o PyHANDLE>|win32ts|WTSOpenServer|Opens a handle to a terminal server
static PyObject *PyWTSOpenServer(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"ServerName", NULL};
    HANDLE h;
    WCHAR *ServerName = NULL;
    PyObject *obServerName;

    if (!PyArg_ParseTupleAndKeywords(
            args, kwargs, "O:WTSOpenServer", keywords,
            &obServerName))  // @pyparm <o PyUnicode>|ServerName||Name ot terminal server to be opened
        return NULL;
    if (!PyWinObject_AsWCHAR(obServerName, &ServerName, FALSE))
        return NULL;
    h = WTSOpenServer(ServerName);
    PyWinObject_FreeWCHAR(ServerName);
    if (h == NULL)
        return PyWin_SetAPIError("WTSOpenServer");
    return new PyTS_HANDLE(h);
}

// @pymethod |win32ts|WTSCloseServer|Closes a terminal server handle
static PyObject *PyWTSCloseServer(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"Server", NULL};
    PyObject *obh;
    HANDLE h;
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O:WTSCloseServer", keywords,
                                     &obh))  // @pyparm <o PyHANDLE>|Server||Terminal Server handle
        return NULL;
    if (!PyWinObject_AsHANDLE(obh, &h))
        return NULL;
    WTSCloseServer(h);
    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod object|win32ts|WTSQueryUserConfig|Returns user configuration
// @rdesc The type of the returned value is dependent on the config class requested
static PyObject *PyWTSQueryUserConfig(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"ServerName", "UserName", "WTSConfigClass", NULL};
    WCHAR *ServerName = NULL, *UserName = NULL;
    PyObject *obServerName, *obUserName, *ret = NULL;
    WTS_CONFIG_CLASS WTSConfigClass;
    LPWSTR buf = NULL;
    DWORD bufsize = 0;

    if (!PyArg_ParseTupleAndKeywords(
            args, kwargs, "OOk:WTSQueryUserConfig", keywords,
            &obServerName,     // @pyparm <o PyUnicode>|ServerName||Name ot terminal server
            &obUserName,       // @pyparm <o PyUnicode>|UserName||Name of user
            &WTSConfigClass))  // @pyparm int|ConfigClass||Type of information to be returned, win32ts.WTSUserConfig*
        return NULL;
    if (PyWinObject_AsWCHAR(obServerName, &ServerName, TRUE) && PyWinObject_AsWCHAR(obUserName, &UserName, FALSE)) {
        if (!WTSQueryUserConfig(ServerName, UserName, WTSConfigClass, &buf, &bufsize))
            PyWin_SetAPIError("WTSQueryUserConfig");
        else {
            switch (WTSConfigClass) {
                // @flagh ConfigClass|Returned value
                case WTSUserConfigInitialProgram:    // @flag WTSUserConfigInitialProgram|Unicode string, program to be
                                                     // run when user logs on
                case WTSUserConfigWorkingDirectory:  // @flag WTSUserConfigWorkingDirectory|Unicode string, working dir
                                                     // for initial program
                case WTSUserConfigModemCallbackPhoneNumber:   // @flag WTSUserConfigModemCallbackPhoneNumber|Unicode
                                                              // string
                case WTSUserConfigTerminalServerProfilePath:  // @flag WTSUserConfigTerminalServerProfilePath|Unicode
                                                              // string
                case WTSUserConfigTerminalServerHomeDir:      // @flag WTSUserConfigTerminalServerHomeDir|Unicode string
                case WTSUserConfigTerminalServerHomeDirDrive:  // @flag WTSUserConfigTerminalServerHomeDirDrive|Unicode
                                                               // string
                    ret = PyWinObject_FromWCHAR(buf);
                    break;
                case WTSUserConfigfInheritInitialProgram:      // @flag WTSUserConfigfInheritInitialProgram|Int
                case WTSUserConfigfAllowLogonTerminalServer:   // @flag WTSUserConfigfAllowLogonTerminalServer|Int, 1 if
                                                               // user can log on thru Terminal Service
                case WTSUserConfigTimeoutSettingsConnections:  // @flag WTSUserConfigTimeoutSettingsConnections |Int,
                                                               // max connection time (ms)
                case WTSUserConfigTimeoutSettingsDisconnections:  // @flag
                                                                  // WTSUserConfigTimeoutSettingsDisconnections|Int
                case WTSUserConfigTimeoutSettingsIdle:    // @flag WTSUserConfigTimeoutSettingsIdle|Int, max idle time
                                                          // (ms)
                case WTSUserConfigfDeviceClientDrives:    // @flag WTSUserConfigfDeviceClientDrives|Int
                case WTSUserConfigfDeviceClientPrinters:  // @flag WTSUserConfigfDeviceClientPrinters|Int
                case WTSUserConfigfDeviceClientDefaultPrinter:  // @flag WTSUserConfigfDeviceClientDefaultPrinter|Int
                case WTSUserConfigBrokenTimeoutSettings:        // @flag WTSUserConfigBrokenTimeoutSettings|Int
                case WTSUserConfigReconnectSettings:            // @flag WTSUserConfigReconnectSettings|Int
                case WTSUserConfigModemCallbackSettings:        // @flag WTSUserConfigModemCallbackSettings|Int
                case WTSUserConfigShadowingSettings:  // @flag WTSUserConfigShadowingSettings|Int, indicates if user's
                                                      // session my be monitored
                case WTSUserConfigfTerminalServerRemoteHomeDir:  // @flag WTSUserConfigfTerminalServerRemoteHomeDir|Int,
                    ret = PyLong_FromUnsignedLong(*(DWORD *)buf);
                    break;
                default:
                    PyErr_SetString(PyExc_NotImplementedError, "Config class not supported yet");
            }
        }
    }

    PyWinObject_FreeWCHAR(ServerName);
    PyWinObject_FreeWCHAR(UserName);
    if (buf)
        WTSFreeMemory(buf);
    return ret;
}

// @pymethod |win32ts|WTSSetUserConfig|Changes user configuration
static PyObject *PyWTSSetUserConfig(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"ServerName", "UserName", "WTSConfigClass", "Buffer", NULL};
    WCHAR *ServerName = NULL, *UserName = NULL;
    PyObject *obServerName, *obUserName, *obBuffer, *ret = NULL;
    WTS_CONFIG_CLASS WTSConfigClass;
    LPWSTR buf;
    WCHAR *wcharbuf = NULL;
    DWORD dwordbuf;
    DWORD bufsize;

    if (!PyArg_ParseTupleAndKeywords(
            args, kwargs, "OOkO:WTSSetUserConfig", keywords,
            &obServerName,    // @pyparm <o PyUnicode>|ServerName||Name ot terminal server
            &obUserName,      // @pyparm <o PyUnicode>|UserName||Name of user
            &WTSConfigClass,  // @pyparm int|ConfigClass||Type of information to be set, win32ts.WTSUserConfig*
            &obBuffer))
        return NULL;
    if (!PyWinObject_AsWCHAR(obServerName, &ServerName, TRUE))
        goto cleanup;
    if (!PyWinObject_AsWCHAR(obUserName, &UserName, FALSE))
        goto cleanup;

    switch (WTSConfigClass) {
        // @flagh ConfigClass|Type of data required
        case WTSUserConfigInitialProgram:    // @flag WTSUserConfigInitialProgram|Unicode string, program to be run when
                                             // user logs on
        case WTSUserConfigWorkingDirectory:  // @flag WTSUserConfigWorkingDirectory|Unicode string, working dir for
                                             // initial program
        case WTSUserConfigModemCallbackPhoneNumber:    // @flag WTSUserConfigModemCallbackPhoneNumber|Unicode string
        case WTSUserConfigTerminalServerProfilePath:   // @flag WTSUserConfigTerminalServerProfilePath|Unicode string
        case WTSUserConfigTerminalServerHomeDir:       // @flag WTSUserConfigTerminalServerHomeDir|Unicode string
        case WTSUserConfigTerminalServerHomeDirDrive:  // @flag WTSUserConfigTerminalServerHomeDirDrive|Unicode string
            if (!PyWinObject_AsWCHAR(obBuffer, &wcharbuf, FALSE, &bufsize))
                goto cleanup;
            buf = wcharbuf;
            bufsize++;  // apparently has to include null terminator
            break;
        case WTSUserConfigfInheritInitialProgram:     // @flag WTSUserConfigfInheritInitialProgram|Int
        case WTSUserConfigfAllowLogonTerminalServer:  // @flag WTSUserConfigfAllowLogonTerminalServer|Int, 1 if user can
                                                      // log on thru Terminal Service
        case WTSUserConfigTimeoutSettingsConnections:     // @flag WTSUserConfigTimeoutSettingsConnections |Int, max
                                                          // connection time (ms)
        case WTSUserConfigTimeoutSettingsDisconnections:  // @flag WTSUserConfigTimeoutSettingsDisconnections|Int
        case WTSUserConfigTimeoutSettingsIdle:    // @flag WTSUserConfigTimeoutSettingsIdle|Int, max idle time (ms)
        case WTSUserConfigfDeviceClientDrives:    // @flag WTSUserConfigfDeviceClientDrives|Int
        case WTSUserConfigfDeviceClientPrinters:  // @flag WTSUserConfigfDeviceClientPrinters|Int
        case WTSUserConfigfDeviceClientDefaultPrinter:  // @flag WTSUserConfigfDeviceClientDefaultPrinter|Int
        case WTSUserConfigBrokenTimeoutSettings:        // @flag WTSUserConfigBrokenTimeoutSettings|Int
        case WTSUserConfigReconnectSettings:            // @flag WTSUserConfigReconnectSettings|Int
        case WTSUserConfigModemCallbackSettings:        // @flag WTSUserConfigModemCallbackSettings|Int
        case WTSUserConfigShadowingSettings:  // @flag WTSUserConfigShadowingSettings|Int, indicates if user's session
                                              // my be monitored
        case WTSUserConfigfTerminalServerRemoteHomeDir:  // @flag WTSUserConfigfTerminalServerRemoteHomeDir|Int,
            dwordbuf = PyLong_AsUnsignedLong(obBuffer);
            if (dwordbuf == (DWORD)-1 && PyErr_Occurred())
                goto cleanup;
            buf = (LPWSTR)&dwordbuf;
            bufsize = sizeof(dwordbuf);
            break;
        default:
            PyErr_SetString(PyExc_NotImplementedError, "Config class not supported yet");
            goto cleanup;
    }
    if (!WTSSetUserConfig(ServerName, UserName, WTSConfigClass, buf, bufsize))
        PyWin_SetAPIError("WTSQueryUserConfig");
    else {
        Py_INCREF(Py_None);
        ret = Py_None;
    }

cleanup:
    PyWinObject_FreeWCHAR(ServerName);
    PyWinObject_FreeWCHAR(UserName);
    PyWinObject_FreeWCHAR(wcharbuf);
    return ret;
}

// @pymethod (<o PyUnicode>,...)|win32ts|WTSEnumerateServers|Lists terminal servers in a domain
static PyObject *PyWTSEnumerateServers(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"DomainName", "Version", "Reserved", NULL};
    WCHAR *DomainName = NULL;
    DWORD Reserved = 0, Version = 1, cnt;
    PyObject *obDomainName = Py_None, *ret = NULL;
    PWTS_SERVER_INFO buf = NULL;
    if (!PyArg_ParseTupleAndKeywords(
            args, kwargs, "|Okk:WTSEnumerateServers", keywords,
            &obDomainName,  // @pyparm <o PyUnicode>|DomainName|None|Use None for current domain
            &Version,       // @pyparm int|Version|1|Version of request, currently 1 is only valid value
            &Reserved))     // @pyparm int|Reserved|0|Reserved, use 0 if passed in
        return NULL;
    if (!PyWinObject_AsWCHAR(obDomainName, &DomainName, TRUE))
        return NULL;

    if (!WTSEnumerateServers(DomainName, Reserved, Version, &buf, &cnt))
        PyWin_SetAPIError("WTSEnumerateServers");
    else {
        ret = PyTuple_New(cnt);
        if (ret)
            for (DWORD i = 0; i < cnt; i++) {
                PyObject *tuple_item = PyWinObject_FromWCHAR(buf[i].pServerName);
                if (!tuple_item) {
                    Py_DECREF(ret);
                    ret = NULL;
                    break;
                }
                PyTuple_SET_ITEM(ret, i, tuple_item);
            }
    }

    PyWinObject_FreeWCHAR(DomainName);
    if (buf)
        WTSFreeMemory(buf);
    return ret;
}

// @pymethod (dict,...)|win32ts|WTSEnumerateSessions|Lists sessions on a server
// @rdesc Returns a sequence of dictionaries representing WTS_SESSION_INFO structs, containing {SessionId:int,
// WinStationName:str, State:int}
static PyObject *PyWTSEnumerateSessions(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"Server", "Version", "Reserved", NULL};
    HANDLE h = WTS_CURRENT_SERVER_HANDLE;
    DWORD Reserved = 0, Version = 1, cnt;
    PyObject *obh = NULL, *ret = NULL;
    PWTS_SESSION_INFO buf = NULL;
    if (!PyArg_ParseTupleAndKeywords(
            args, kwargs, "|Okk:WTSEnumerateServers", keywords,
            &obh,        // @pyparm <o PyHANDLE>|Server|WTS_CURRENT_SERVER_HANDLE|Handle to a terminal server
            &Version,    // @pyparm int|Version|1|Version of request, currently 1 is only valid value
            &Reserved))  // @pyparm int|Reserved|0|Reserved, use 0 if passed in
        return NULL;
    if (obh)
        if (!PyWinObject_AsHANDLE(obh, &h))
            return NULL;

    if (!WTSEnumerateSessions(h, Reserved, Version, &buf, &cnt))
        PyWin_SetAPIError("WTSEnumerateSessions");
    else {
        ret = PyTuple_New(cnt);
        if (ret)
            for (DWORD i = 0; i < cnt; i++) {
                PyObject *tuple_item = Py_BuildValue("{s:k,s:u,s:k}", "SessionId", buf[i].SessionId, "WinStationName",
                                                     buf[i].pWinStationName, "State", buf[i].State);
                if (!tuple_item) {
                    Py_DECREF(ret);
                    ret = NULL;
                    break;
                }
                PyTuple_SET_ITEM(ret, i, tuple_item);
            }
    }
    if (buf)
        WTSFreeMemory(buf);
    return ret;
}

// @pymethod |win32ts|WTSLogoffSession|Logs off a user logged in through Terminal Services
static PyObject *PyWTSLogoffSession(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"Server", "SessionId", "Wait", NULL};
    HANDLE h;
    PyObject *obh;
    DWORD SessionId;
    BOOL Wait;

    if (!PyArg_ParseTupleAndKeywords(
            args, kwargs, "Okl:WTSLogoffSession", keywords,
            &obh,        // @pyparm <o PyHANDLE>|Server||Handle to a terminal server
            &SessionId,  // @pyparm int|SessionId||Terminal services session id as returned by <om
                         // win32ts.WTSEnumerateSessions>
            &Wait))      // @pyparm boolean|Wait||Indicates whether operation should be performed asynchronously
        return NULL;

    if (!PyWinObject_AsHANDLE(obh, &h))
        return NULL;
    if (!WTSLogoffSession(h, SessionId, Wait))
        return PyWin_SetAPIError("WTSLogoffSession");
    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod |win32ts|WTSDisconnectSession|Disconnects a session without logging it off
static PyObject *PyWTSDisconnectSession(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"Server", "SessionId", "Wait", NULL};
    HANDLE h;
    PyObject *obh;
    DWORD SessionId;
    BOOL Wait;

    if (!PyArg_ParseTupleAndKeywords(
            args, kwargs, "Okl:WTSDisconnectSession", keywords,
            &obh,        // @pyparm <o PyHANDLE>|Server||Handle to a terminal server
            &SessionId,  // @pyparm int|SessionId||Terminal services session id as returned by <om
                         // win32ts.WTSEnumerateSessions>
            &Wait))      // @pyparm boolean|Wait||Indicates whether operation should be performed asynchronously
        return NULL;

    if (!PyWinObject_AsHANDLE(obh, &h))
        return NULL;
    if (!WTSDisconnectSession(h, SessionId, Wait))
        return PyWin_SetAPIError("WTSDisconnectSession");
    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod |win32ts|WTSQuerySessionInformation|Returns information about a terminal services session
static PyObject *PyWTSQuerySessionInformation(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"Server", "SessionId", "WTSInfoClass", NULL};
    HANDLE h;
    PyObject *obh, *ret = NULL;
    DWORD SessionId;
    WTS_INFO_CLASS WTSInfoClass;
    LPWSTR buf = NULL;
    DWORD bufsize;

    if (!PyArg_ParseTupleAndKeywords(
            args, kwargs, "Okk:WTSQuerySessionInformation", keywords,
            &obh,  // @pyparm <o PyHANDLE>|Server||Handle to a terminal server as returned by <om win32ts.WTSOpenServer>
            &SessionId,      // @pyparm int|SessionId||Terminal services session id as returned by <om
                             // win32ts.WTSEnumerateSessions>
            &WTSInfoClass))  // @pyparm int|WTSInfoClass||Type of information requested, from WTS_INFO_CLASS enum
        return NULL;

    if (!PyWinObject_AsHANDLE(obh, &h))
        return NULL;
    if (!WTSQuerySessionInformation(h, SessionId, WTSInfoClass, &buf, &bufsize)) {
        PyWin_SetAPIError("WTSQuerySessionInformation");
        goto cleanup;
    }
    // @flagh InfoClass|Returned value
    switch (WTSInfoClass) {
        case WTSApplicationName:   // @flag WTSApplicationName|Unicode string
        case WTSClientDirectory:   // @flag WTSClientDirectory|Unicode string
        case WTSClientName:        // @flag WTSClientName|Unicode string
        case WTSDomainName:        // @flag WTSDomainName|Unicode string
        case WTSInitialProgram:    // @flag WTSInitialProgram|Unicode string
        case WTSOEMId:             // @flag WTSOEMId|Unicode string
        case WTSUserName:          // @flag WTSUserName|Unicode string
        case WTSWinStationName:    // @flag WTSWinStationName|Unicode string
        case WTSWorkingDirectory:  // @flag WTSWorkingDirectory|Unicode string
            ret = PyWinObject_FromWCHAR(buf);
            break;
        // USHORTs
        case WTSClientProtocolType:  // @flag WTSClientProtocolType|Int, one of
                                     // WTS_PROTOCOL_TYPE_CONSOLE,WTS_PROTOCOL_TYPE_ICA,WTS_PROTOCOL_TYPE_RDP
        case WTSClientProductId:     // @flag WTSClientProductId|Int
            ret = PyLong_FromLong(*(USHORT *)buf);
            break;
        // ULONGs
        case WTSClientBuildNumber:  // @flag WTSClientBuildNumber|Int
        case WTSClientHardwareId:   // @flag WTSClientHardwareId|Int
        case WTSSessionId:          // @flag WTSSessionId|Int
            ret = PyLong_FromUnsignedLong(*(ULONG *)buf);
            break;
        case WTSConnectState:  // @flag WTSConnectState|Int, from WTS_CONNECTSTATE_CLASS
            ret = PyLong_FromLong(*(INT *)buf);
            break;
        case WTSIsRemoteSession:  // @flag WTSIsRemoteSession|Boolean
            ret = PyBool_FromLong(*(BYTE *)buf);
            break;
        case WTSClientDisplay: {  // @flag WTSClientDisplay|Dict containing client's display settings
            WTS_CLIENT_DISPLAY *wcd = (WTS_CLIENT_DISPLAY *)buf;
            ret = Py_BuildValue("{s:k, s:k, s:k}", "HorizontalResolution", wcd->HorizontalResolution,
                                "VerticalResolution", wcd->VerticalResolution, "ColorDepth", wcd->ColorDepth);
            break;
        }
        case WTSClientAddress: {  // @flag WTSClientAddress|Dict containing type and value of client's IP address
                                  // (None if console session)
            PyObject *obaddress;
            size_t address_cnt, address_ind;
            WTS_CLIENT_ADDRESS *wca = (WTS_CLIENT_ADDRESS *)buf;
            // ??? According to MSDN, buffer may be NULL for console session. (but I don't see it in practice) ???
            if (wca == NULL) {
                Py_INCREF(Py_None);
                ret = Py_None;
                break;
            }
            address_cnt = ARRAYSIZE(wca->Address);
            obaddress = PyTuple_New(address_cnt);
            if (obaddress != NULL)
                for (address_ind = 0; address_ind < address_cnt; address_ind++) {
                    PyObject *obaddress_element = PyLong_FromLong(wca->Address[address_ind]);
                    if (obaddress_element == NULL) {
                        Py_DECREF(obaddress);
                        obaddress = NULL;
                        break;
                    }
                    PyTuple_SET_ITEM(obaddress, address_ind, obaddress_element);
                }
            if (obaddress != NULL)
                ret = Py_BuildValue("{s:k, s:N}", "AddressFamily", wca->AddressFamily, "Address", obaddress);
            break;
        }
        default:
            PyErr_Format(PyExc_NotImplementedError, "InfoClass %d not yet supported", WTSInfoClass);
    }

cleanup:
    if (buf)
        WTSFreeMemory(buf);
    return ret;
}

// @pymethod (<o PyUnicode>,...)|win32ts|WTSEnumerateProcesses|Lists processes on a terminal server
static PyObject *PyWTSEnumerateProcesses(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"Server", "Version", "Reserved", NULL};
    HANDLE h = WTS_CURRENT_SERVER_HANDLE;
    DWORD Reserved = 0, Version = 1, cnt;
    PyObject *obh = NULL, *ret = NULL;
    PWTS_PROCESS_INFO buf = NULL;
    if (!PyArg_ParseTupleAndKeywords(
            args, kwargs, "|Okk:WTSEnumerateProcesses", keywords,
            &obh,        // @pyparm <o PyHANDLE>|Server|WTS_CURRENT_SERVER_HANDLE|Handle to a terminal server
            &Version,    // @pyparm int|Version|1|Version of request, currently 1 is only valid value
            &Reserved))  // @pyparm int|Reserved|0|Reserved, use 0 if passed in
        return NULL;
    if (obh)
        if (!PyWinObject_AsHANDLE(obh, &h))
            return NULL;

    if (!WTSEnumerateProcesses(h, Reserved, Version, &buf, &cnt))
        PyWin_SetAPIError("WTSEnumerateProcesses");
    else {
        ret = PyTuple_New(cnt);
        if (ret)
            for (DWORD i = 0; i < cnt; i++) {
                PyObject *tuple_item =
                    Py_BuildValue("kkNN", buf[i].SessionId, buf[i].ProcessId,
                                  PyWinObject_FromWCHAR(buf[i].pProcessName), PyWinObject_FromSID(buf[i].pUserSid));
                if (!tuple_item) {
                    Py_DECREF(ret);
                    ret = NULL;
                    break;
                }
                PyTuple_SET_ITEM(ret, i, tuple_item);
            }
    }
    if (buf)
        WTSFreeMemory(buf);
    return ret;
}

// @pymethod <o PyHANDLE>|win32ts|WTSQueryUserToken|Retrieves the access token for a session
// @comm This function is intended only for use by trusted processes that have SE_TCB_PRIVILEGE enabled
static PyObject *PyWTSQueryUserToken(PyObject *self, PyObject *args, PyObject *kwargs)
{
    CHECK_PFN(WTSQueryUserToken);
    static char *keywords[] = {"SessionId", NULL};
    HANDLE h;
    ULONG SessionId;
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "k:WTSQueryUserToken", keywords,
                                     &SessionId))  // @pyparm int|SessionId||Terminal services session id
        return NULL;
    if (!(*pfnWTSQueryUserToken)(SessionId, &h))
        return PyWin_SetAPIError("WTSQueryUserToken");
    return PyWinObject_FromHANDLE(h);
}

// @pymethod |win32ts|WTSShutdownSystem|Issues a shutdown request to a terminal server
static PyObject *PyWTSShutdownSystem(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"Server", "ShutdownFlag", NULL};
    PyObject *obh;
    HANDLE h;
    DWORD flags;
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "Ok:WTSShutdownSystem", keywords,
                                     &obh,     // @pyparm <o PyHANDLE>|Server||Handle to a terminal server
                                     &flags))  // @pyparm int|ShutdownFlag||One of the win32ts.WTS_WSD_* values
        return NULL;
    if (!PyWinObject_AsHANDLE(obh, &h))
        return NULL;
    if (!WTSShutdownSystem(h, flags))
        return PyWin_SetAPIError("WTSShutdownSystem");
    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod |win32ts|WTSTerminateProcess|Kills a process on a terminal server
static PyObject *PyWTSTerminateProcess(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"Server", "ProcessId", "ExitCode", NULL};
    PyObject *obh;
    HANDLE h;
    DWORD ProcessId, ExitCode;
    if (!PyArg_ParseTupleAndKeywords(
            args, kwargs, "Okk:WTSTerminateProcess", keywords,
            &obh,        // @pyparm <o PyHANDLE>|Server||Handle to a terminal server
            &ProcessId,  // @pyparm int|ProcessId||Id of a process as returned by <om win32ts.WTSEnumerateProcesses>
            &ExitCode))  // @pyparm int|ExitCode||Exit code for the process
        return NULL;
    if (!PyWinObject_AsHANDLE(obh, &h))
        return NULL;
    if (!WTSTerminateProcess(h, ProcessId, ExitCode))
        return PyWin_SetAPIError("WTSTerminateProcess");
    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod int|win32ts|ProcessIdToSessionId|Finds the session under which a process is running
static PyObject *PyProcessIdToSessionId(PyObject *self, PyObject *args, PyObject *kwargs)
{
    CHECK_PFN(ProcessIdToSessionId);
    static char *keywords[] = {"ProcessId", NULL};
    DWORD ProcessId, SessionId;
    if (!PyArg_ParseTupleAndKeywords(
            args, kwargs, "k:ProcessIdToSessionId", keywords,
            &ProcessId))  // @pyparm int|ProcessId||Id of a process as returned by <om win32ts.WTSEnumerateProcesses>
        return NULL;
    if (!(*pfnProcessIdToSessionId)(ProcessId, &SessionId))
        return PyWin_SetAPIError("ProcessIdToSessionId");
    return PyLong_FromUnsignedLong(SessionId);
}

// @pymethod int|win32ts|WTSGetActiveConsoleSessionId|Returns the id of the console session
// @comm Returns 0xffffffff if no active console session exists
static PyObject *PyWTSGetActiveConsoleSessionId(PyObject *self, PyObject *args, PyObject *kwargs)
{
    CHECK_PFN(WTSGetActiveConsoleSessionId);
    static char *keywords[] = {NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, ":WTSGetActiveConsoleSessionId", keywords))
        return NULL;
    DWORD SessionId = (*pfnWTSGetActiveConsoleSessionId)();
    return PyLong_FromUnsignedLong(SessionId);
}

// @pymethod |win32ts|WTSRegisterSessionNotification|Registers a window to receive terminal service notifications
static PyObject *PyWTSRegisterSessionNotification(PyObject *self, PyObject *args, PyObject *kwargs)
{
    CHECK_PFN(WTSRegisterSessionNotification);
    static char *keywords[] = {"Wnd", "Flags", NULL};
    PyObject *obhwnd;
    HWND hwnd;
    DWORD flags;
    if (!PyArg_ParseTupleAndKeywords(
            args, kwargs, "Ok:WTSRegisterSessionNotification", keywords,
            &obhwnd,  // @pyparm <o PyHANDLE>|Wnd||Window handle to receive terminal service messages
            &flags))  // @pyparm int|Flags||NOTIFY_FOR_THIS_SESSION or NOTIFY_FOR_ALL_SESSIONS
        return NULL;
    if (!PyWinObject_AsHANDLE(obhwnd, (HANDLE *)&hwnd))
        return NULL;
    if (!(*pfnWTSRegisterSessionNotification)(hwnd, flags))
        PyWin_SetAPIError("WTSRegisterSessionNotification");
    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod |win32ts|WTSUnRegisterSessionNotification|Disables terminal service window messages
static PyObject *PyWTSUnRegisterSessionNotification(PyObject *self, PyObject *args, PyObject *kwargs)
{
    CHECK_PFN(WTSUnRegisterSessionNotification);
    static char *keywords[] = {"Wnd", NULL};
    PyObject *obhwnd;
    HWND hwnd;
    if (!PyArg_ParseTupleAndKeywords(
            args, kwargs, "O:WTSUnRegisterSessionNotification", keywords,
            &obhwnd))  // @pyparm <o PyHANDLE>|Wnd||Window previously registered to receive session notifications
        return NULL;
    if (!PyWinObject_AsHANDLE(obhwnd, (HANDLE *)&hwnd))
        return NULL;
    if (!(*pfnWTSUnRegisterSessionNotification)(hwnd))
        PyWin_SetAPIError("WTSUnRegisterSessionNotification");
    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod int|win32ts|WTSWaitSystemEvent|Waits for an event to occur
// @rdesc Returns a bitmask of WTS_EVENT_* flags indication which event(s) occurred
static PyObject *PyWTSWaitSystemEvent(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"Server", "EventMask", NULL};
    PyObject *obh = NULL;
    HANDLE h = WTS_CURRENT_SERVER_HANDLE;
    DWORD EventMask = WTS_EVENT_ALL, EventFlags;
    if (!PyArg_ParseTupleAndKeywords(
            args, kwargs, "|Ok:WTSWaitSystemEvent", keywords,
            &obh,         // @pyparm <o PyHANDLE>|Server|WTS_CURRENT_SERVER_HANDLE|Handle to a terminal server, or
                          // WTS_CURRENT_SERVER_HANDLE
            &EventMask))  // @pyparm int|EventMask|WTS_EVENT_ALL|Combination of WTS_EVENT_* values
        return NULL;
    if (obh)
        if (!PyWinObject_AsHANDLE(obh, &h))
            return NULL;
    if (!WTSWaitSystemEvent(h, EventMask, &EventFlags))
        return PyWin_SetAPIError("WTSWaitSystemEvent");
    return PyLong_FromUnsignedLong(EventFlags);
}

// @pymethod int|win32ts|WTSSendMessage|Sends a popup message to a terminal services session
// @rdesc Returns one of IDABORT,IDCANCEL,IDIGNORE,IDNO,IDOK,IDRETRY,IDYES,IDASYNC,IDTIMEOUT,
static PyObject *PyWTSSendMessage(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"Server", "SessionId", "Title", "Message", "Style", "Timeout", "Wait", NULL};
    PyObject *obh = NULL;
    HANDLE h;
    DWORD SessionId, TitleLen, MessageLen, Style, Timeout, Response;
    WCHAR *Title = NULL, *Message = NULL;
    PyObject *obTitle, *obMessage, *ret = NULL;
    BOOL Wait;
    if (!PyArg_ParseTupleAndKeywords(
            args, kwargs, "OkOOkkl:WTSSendMessage", keywords,
            &obh,        // @pyparm <o PyHANDLE>|Server|WTS_CURRENT_SERVER_HANDLE|Handle to a terminal server, or
                         // WTS_CURRENT_SERVER_HANDLE
            &SessionId,  // @pyparm int|SessionId||Terminal services session id
            &obTitle,    // @pyparm <o PyUnicode>|Title||Title of dialog
            &obMessage,  // @pyparm <o PyUnicode>|Message||Message to be displayed
            &Style,      // @pyparm int|Style||Usually MB_OK
            &Timeout,    // @pyparm int|Timeout||Seconds to wait before returning (only used if Wait is True)
            &Wait))      // @pyparm boolean|Wait||Specifies if function should wait for user input before returning
        return NULL;
    if (PyWinObject_AsHANDLE(obh, &h) && PyWinObject_AsWCHAR(obTitle, &Title, FALSE, &TitleLen) &&
        PyWinObject_AsWCHAR(obMessage, &Message, FALSE, &MessageLen)) {
        if (WTSSendMessage(h, SessionId, Title, TitleLen * sizeof(WCHAR), Message, MessageLen * sizeof(WCHAR), Style,
                           Timeout, &Response, Wait))
            ret = PyLong_FromUnsignedLong(Response);
        else
            PyWin_SetAPIError("WTSSendMessage");
    }
    PyWinObject_FreeWCHAR(Title);
    PyWinObject_FreeWCHAR(Message);
    return ret;
}

// @module win32ts|Interface to the Terminal Services Api
//	All functions in this module accept keyword arguments
static struct PyMethodDef win32ts_functions[] = {
    // @pymeth WTSOpenServer|Opens a handle to a terminal server
    {"WTSOpenServer", (PyCFunction)PyWTSOpenServer, METH_VARARGS | METH_KEYWORDS,
     "Opens a handle to a terminal server"},
    // @pymeth WTSCloseServer|Closes a terminal server handle
    {"WTSCloseServer", (PyCFunction)PyWTSCloseServer, METH_VARARGS | METH_KEYWORDS, "Closes a terminal server handle"},
    // @pymeth WTSQueryUserConfig|Returns user configuration
    {"WTSQueryUserConfig", (PyCFunction)PyWTSQueryUserConfig, METH_VARARGS | METH_KEYWORDS,
     "Returns user configuration"},
    // @pymeth WTSSetUserConfig|Changes user configuration
    {"WTSSetUserConfig", (PyCFunction)PyWTSSetUserConfig, METH_VARARGS | METH_KEYWORDS, "Changes user configuration"},
    // @pymeth WTSEnumerateServers|Lists terminal servers in a domain
    {"WTSEnumerateServers", (PyCFunction)PyWTSEnumerateServers, METH_VARARGS | METH_KEYWORDS,
     "Lists terminal servers in a domain"},
    // @pymeth WTSEnumerateSessions|Lists sessions on a server
    {"WTSEnumerateSessions", (PyCFunction)PyWTSEnumerateSessions, METH_VARARGS | METH_KEYWORDS,
     "Lists sessions on a server"},
    // @pymeth WTSLogoffSession|Logs off a user logged in through Terminal Services
    {"WTSLogoffSession", (PyCFunction)PyWTSLogoffSession, METH_VARARGS | METH_KEYWORDS,
     "Logs off a user logged in through Terminal Services"},
    // @pymeth WTSDisconnectSession|Disconnects a session without logging it off
    {"WTSDisconnectSession", (PyCFunction)PyWTSDisconnectSession, METH_VARARGS | METH_KEYWORDS,
     "Disconnects a session without logging it off"},
    // @pymeth WTSQuerySessionInformation|Retrieve information about a session
    {"WTSQuerySessionInformation", (PyCFunction)PyWTSQuerySessionInformation, METH_VARARGS | METH_KEYWORDS,
     "Retrieve information about a session"},
    // @pymeth WTSEnumerateProcesses|Lists processes on a terminal server
    {"WTSEnumerateProcesses", (PyCFunction)PyWTSEnumerateProcesses, METH_VARARGS | METH_KEYWORDS,
     "Lists processes on a terminal server"},
    // @pymeth WTSQueryUserToken|Retrieves the access token for a session
    {"WTSQueryUserToken", (PyCFunction)PyWTSQueryUserToken, METH_VARARGS | METH_KEYWORDS,
     "Retrieves the access token for a session"},
    // @pymeth WTSShutdownSystem|Issues a shutdown request to a terminal server
    {"WTSShutdownSystem", (PyCFunction)PyWTSShutdownSystem, METH_VARARGS | METH_KEYWORDS,
     "Issues a shutdown request to a terminal server"},
    // @pymeth WTSTerminateProcess|Kills a process on a terminal server
    {"WTSTerminateProcess", (PyCFunction)PyWTSTerminateProcess, METH_VARARGS | METH_KEYWORDS,
     "Kills a process on a terminal server"},
    // @pymeth ProcessIdToSessionId|Finds the session under which a process is running
    {"ProcessIdToSessionId", (PyCFunction)PyProcessIdToSessionId, METH_VARARGS | METH_KEYWORDS,
     "Finds the session under which a process is running"},
    // @pymeth WTSGetActiveConsoleSessionId|Returns the id of the console session
    {"WTSGetActiveConsoleSessionId", (PyCFunction)PyWTSGetActiveConsoleSessionId, METH_VARARGS | METH_KEYWORDS,
     "Returns the id of the console session"},
    // @pymeth WTSRegisterSessionNotification|Registers a window to receive terminal service notifications
    {"WTSRegisterSessionNotification", (PyCFunction)PyWTSRegisterSessionNotification, METH_VARARGS | METH_KEYWORDS,
     "Registers a window to receive terminal service notifications"},
    // @pymeth WTSUnRegisterSessionNotification|Disables terminal service window messages
    {"WTSUnRegisterSessionNotification", (PyCFunction)PyWTSUnRegisterSessionNotification, METH_VARARGS | METH_KEYWORDS,
     "Disables terminal service window messages"},
    // @pymeth WTSWaitSystemEvent|Waits for an event to occur
    {"WTSWaitSystemEvent", (PyCFunction)PyWTSWaitSystemEvent, METH_VARARGS | METH_KEYWORDS,
     "Waits for an event to occur"},
    // @pymeth WTSSendMessage|Sends a popup message to a terminal services session
    {"WTSSendMessage", (PyCFunction)PyWTSSendMessage, METH_VARARGS | METH_KEYWORDS,
     "Sends a popup message to a terminal services session"},
    {NULL, NULL}};

PYWIN_MODULE_INIT_FUNC(win32ts)
{
    PYWIN_MODULE_INIT_PREPARE(win32ts, win32ts_functions, "Interface to the Terminal Services Api.");

    // WTS_CONNECTSTATE_CLASS
    PyModule_AddIntConstant(module, "WTSActive", WTSActive);
    PyModule_AddIntConstant(module, "WTSConnected", WTSConnected);
    PyModule_AddIntConstant(module, "WTSConnectQuery", WTSConnectQuery);
    PyModule_AddIntConstant(module, "WTSShadow", WTSShadow);
    PyModule_AddIntConstant(module, "WTSDisconnected", WTSDisconnected);
    PyModule_AddIntConstant(module, "WTSIdle", WTSIdle);
    PyModule_AddIntConstant(module, "WTSListen", WTSListen);
    PyModule_AddIntConstant(module, "WTSReset", WTSReset);
    PyModule_AddIntConstant(module, "WTSDown", WTSDown);
    PyModule_AddIntConstant(module, "WTSInit", WTSInit);

    // WTS_INFO_CLASS
    PyModule_AddIntConstant(module, "WTSInitialProgram", WTSInitialProgram);
    PyModule_AddIntConstant(module, "WTSApplicationName", WTSApplicationName);
    PyModule_AddIntConstant(module, "WTSWorkingDirectory", WTSWorkingDirectory);
    PyModule_AddIntConstant(module, "WTSOEMId", WTSOEMId);
    PyModule_AddIntConstant(module, "WTSSessionId", WTSSessionId);
    PyModule_AddIntConstant(module, "WTSUserName", WTSUserName);
    PyModule_AddIntConstant(module, "WTSWinStationName", WTSWinStationName);
    PyModule_AddIntConstant(module, "WTSDomainName", WTSDomainName);
    PyModule_AddIntConstant(module, "WTSConnectState", WTSConnectState);
    PyModule_AddIntConstant(module, "WTSClientBuildNumber", WTSClientBuildNumber);
    PyModule_AddIntConstant(module, "WTSClientName", WTSClientName);
    PyModule_AddIntConstant(module, "WTSClientDirectory", WTSClientDirectory);
    PyModule_AddIntConstant(module, "WTSClientProductId", WTSClientProductId);
    PyModule_AddIntConstant(module, "WTSClientHardwareId", WTSClientHardwareId);
    PyModule_AddIntConstant(module, "WTSClientAddress", WTSClientAddress);
    PyModule_AddIntConstant(module, "WTSClientDisplay", WTSClientDisplay);
    PyModule_AddIntConstant(module, "WTSClientProtocolType", WTSClientProtocolType);
    PyModule_AddIntConstant(module, "WTSIsRemoteSession", WTSIsRemoteSession);

    // WTS_CONFIG_CLASS
    PyModule_AddIntConstant(module, "WTSUserConfigInitialProgram", WTSUserConfigInitialProgram);
    PyModule_AddIntConstant(module, "WTSUserConfigWorkingDirectory", WTSUserConfigWorkingDirectory);
    PyModule_AddIntConstant(module, "WTSUserConfigfInheritInitialProgram", WTSUserConfigfInheritInitialProgram);
    PyModule_AddIntConstant(module, "WTSUserConfigfAllowLogonTerminalServer", WTSUserConfigfAllowLogonTerminalServer);
    PyModule_AddIntConstant(module, "WTSUserConfigTimeoutSettingsConnections", WTSUserConfigTimeoutSettingsConnections);
    PyModule_AddIntConstant(module, "WTSUserConfigTimeoutSettingsDisconnections",
                            WTSUserConfigTimeoutSettingsDisconnections);
    PyModule_AddIntConstant(module, "WTSUserConfigTimeoutSettingsIdle", WTSUserConfigTimeoutSettingsIdle);
    PyModule_AddIntConstant(module, "WTSUserConfigfDeviceClientDrives", WTSUserConfigfDeviceClientDrives);
    PyModule_AddIntConstant(module, "WTSUserConfigfDeviceClientPrinters", WTSUserConfigfDeviceClientPrinters);
    PyModule_AddIntConstant(module, "WTSUserConfigfDeviceClientDefaultPrinter",
                            WTSUserConfigfDeviceClientDefaultPrinter);
    PyModule_AddIntConstant(module, "WTSUserConfigBrokenTimeoutSettings", WTSUserConfigBrokenTimeoutSettings);
    PyModule_AddIntConstant(module, "WTSUserConfigReconnectSettings", WTSUserConfigReconnectSettings);
    PyModule_AddIntConstant(module, "WTSUserConfigModemCallbackSettings", WTSUserConfigModemCallbackSettings);
    PyModule_AddIntConstant(module, "WTSUserConfigModemCallbackPhoneNumber", WTSUserConfigModemCallbackPhoneNumber);
    PyModule_AddIntConstant(module, "WTSUserConfigShadowingSettings", WTSUserConfigShadowingSettings);
    PyModule_AddIntConstant(module, "WTSUserConfigTerminalServerProfilePath", WTSUserConfigTerminalServerProfilePath);
    PyModule_AddIntConstant(module, "WTSUserConfigTerminalServerHomeDir", WTSUserConfigTerminalServerHomeDir);
    PyModule_AddIntConstant(module, "WTSUserConfigTerminalServerHomeDirDrive", WTSUserConfigTerminalServerHomeDirDrive);
    PyModule_AddIntConstant(module, "WTSUserConfigfTerminalServerRemoteHomeDir",
                            WTSUserConfigfTerminalServerRemoteHomeDir);

    PyModule_AddIntConstant(module, "WTS_EVENT_NONE", WTS_EVENT_NONE);
    PyModule_AddIntConstant(module, "WTS_EVENT_CREATE", WTS_EVENT_CREATE);
    PyModule_AddIntConstant(module, "WTS_EVENT_DELETE", WTS_EVENT_DELETE);
    PyModule_AddIntConstant(module, "WTS_EVENT_RENAME", WTS_EVENT_RENAME);
    PyModule_AddIntConstant(module, "WTS_EVENT_CONNECT", WTS_EVENT_CONNECT);
    PyModule_AddIntConstant(module, "WTS_EVENT_DISCONNECT", WTS_EVENT_DISCONNECT);
    PyModule_AddIntConstant(module, "WTS_EVENT_LOGON", WTS_EVENT_LOGON);
    PyModule_AddIntConstant(module, "WTS_EVENT_LOGOFF", WTS_EVENT_LOGOFF);
    PyModule_AddIntConstant(module, "WTS_EVENT_STATECHANGE", WTS_EVENT_STATECHANGE);
    PyModule_AddIntConstant(module, "WTS_EVENT_LICENSE", WTS_EVENT_LICENSE);
    PyModule_AddIntConstant(module, "WTS_EVENT_ALL", WTS_EVENT_ALL);
    PyModule_AddIntConstant(module, "WTS_EVENT_FLUSH", WTS_EVENT_FLUSH);

    // WTS_VIRTUAL_CLASS
    PyModule_AddIntConstant(module, "WTSVirtualClientData", WTSVirtualClientData);
    PyModule_AddIntConstant(module, "WTSVirtualFileHandle", WTSVirtualFileHandle);

    PyModule_AddIntConstant(module, "WTS_PROTOCOL_TYPE_CONSOLE", WTS_PROTOCOL_TYPE_CONSOLE);
    PyModule_AddIntConstant(module, "WTS_PROTOCOL_TYPE_ICA", WTS_PROTOCOL_TYPE_ICA);
    PyModule_AddIntConstant(module, "WTS_PROTOCOL_TYPE_RDP", WTS_PROTOCOL_TYPE_RDP);

    // Flags used with WTSShutdownSystem
    PyModule_AddIntConstant(module, "WTS_WSD_LOGOFF", WTS_WSD_LOGOFF);
    PyModule_AddIntConstant(module, "WTS_WSD_SHUTDOWN", WTS_WSD_SHUTDOWN);
    PyModule_AddIntConstant(module, "WTS_WSD_REBOOT", WTS_WSD_REBOOT);
    PyModule_AddIntConstant(module, "WTS_WSD_POWEROFF", WTS_WSD_POWEROFF);
    PyModule_AddIntConstant(module, "WTS_WSD_FASTREBOOT", WTS_WSD_FASTREBOOT);

    // pseudo handles for current server and session
    PyModule_AddIntConstant(module, "WTS_CURRENT_SERVER", 0);
    PyModule_AddIntConstant(module, "WTS_CURRENT_SERVER_HANDLE", 0);
    PyModule_AddIntConstant(module, "WTS_CURRENT_SESSION", WTS_CURRENT_SESSION);
    Py_INCREF(Py_None);  // WTS_CURRENT_SERVER_NAME is defined as NULL
    PyModule_AddObject(module, "WTS_CURRENT_SERVER_NAME", Py_None);

    // Session notification constants
    PyModule_AddIntConstant(module, "NOTIFY_FOR_THIS_SESSION", NOTIFY_FOR_THIS_SESSION);
    PyModule_AddIntConstant(module, "NOTIFY_FOR_ALL_SESSIONS", NOTIFY_FOR_ALL_SESSIONS);

    HMODULE h = PyWin_GetOrLoadLibraryHandle("wtsapi32.dll");
    if (h != NULL) {
        pfnWTSQueryUserToken = (WTSQueryUserTokenfunc)GetProcAddress(h, "WTSQueryUserToken");
        pfnWTSRegisterSessionNotification =
            (WTSRegisterSessionNotificationfunc)GetProcAddress(h, "WTSRegisterSessionNotification");
        pfnWTSUnRegisterSessionNotification =
            (WTSUnRegisterSessionNotificationfunc)GetProcAddress(h, "WTSUnRegisterSessionNotification");
    }

    h = PyWin_GetOrLoadLibraryHandle("kernel32.dll");
    if (h != NULL) {
        pfnProcessIdToSessionId = (ProcessIdToSessionIdfunc)GetProcAddress(h, "ProcessIdToSessionId");
        pfnWTSGetActiveConsoleSessionId =
            (WTSGetActiveConsoleSessionIdfunc)GetProcAddress(h, "WTSGetActiveConsoleSessionId");
    }

    PYWIN_MODULE_INIT_RETURN_SUCCESS;
}
