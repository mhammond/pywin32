// @doc

#include "PyWinTypes.h"
#include "PyWinObjects.h"

#include "malloc.h"
#include "Userenv.h"

#define CHECK_PFN(fname)    \
    if (pfn##fname == NULL) \
        return PyErr_Format(PyExc_NotImplementedError, "%s is not available on this platform", #fname);
typedef BOOL(WINAPI *DeleteProfilefunc)(WCHAR *, WCHAR *, WCHAR *);
static DeleteProfilefunc pfnDeleteProfile = NULL;
typedef BOOL(WINAPI *GetAllUsersProfileDirectoryfunc)(WCHAR *, DWORD *);
static GetAllUsersProfileDirectoryfunc pfnGetAllUsersProfileDirectory = NULL;
typedef BOOL(WINAPI *GetDefaultUserProfileDirectoryfunc)(WCHAR *, DWORD *);
static GetDefaultUserProfileDirectoryfunc pfnGetDefaultUserProfileDirectory = NULL;
typedef BOOL(WINAPI *GetProfilesDirectoryfunc)(WCHAR *, DWORD *);
static GetProfilesDirectoryfunc pfnGetProfilesDirectory = NULL;
typedef BOOL(WINAPI *GetProfileTypefunc)(DWORD *);
static GetProfileTypefunc pfnGetProfileType = NULL;
typedef BOOL(WINAPI *GetUserProfileDirectoryfunc)(HANDLE, WCHAR *, DWORD *);
static GetUserProfileDirectoryfunc pfnGetUserProfileDirectory = NULL;
typedef BOOL(WINAPI *LoadUserProfilefunc)(HANDLE, LPPROFILEINFOW);
static LoadUserProfilefunc pfnLoadUserProfile = NULL;
typedef BOOL(WINAPI *UnloadUserProfilefunc)(HANDLE, HANDLE);
static UnloadUserProfilefunc pfnUnloadUserProfile = NULL;

typedef BOOL(WINAPI *ExpandEnvironmentStringsForUserfunc)(HANDLE, LPWSTR, LPWSTR, DWORD);
static ExpandEnvironmentStringsForUserfunc pfnExpandEnvironmentStringsForUser = NULL;

/* Takes an environment block and returns a dict suitable for passing to CreateProcess
    or CreateProcessAsUser.  Length is not known, so you have to depend on the block being correctly formatted.
    There are also several other pieces of code handling these types of strings:
        win32process(building an environment block from a dict), win32service (service dependencies),
        win32print(list of driver files), win32api(REG_MULTI_SZ values)
    Should probably consolidate into one place
*/
PyObject *PyWinObject_FromEnvironmentBlock(WCHAR *multistring)
{
    PyObject *key, *val, *ret = NULL;
    WCHAR *eq;
    size_t keylen, vallen, totallen;
    if (multistring == NULL) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    ret = PyDict_New();
    if (ret == NULL)
        return NULL;
    totallen = wcslen(multistring);
    while (totallen) {
        /* Official docs say that the name of an environment variable cannot include an equal sign.
            Actually, there are names started with an equal sign, e.g. per-drive working dirs are stored in the form
            "=C:=C:\\somedir", "=D:=D:\\someotherdir". These are retrievable by win32api.GetEnvironmentVariable('=C:'),
            but don't appear in os.environ. Environment variable's value may contain an equal sign.
            So we use the first equal sign from which the string is not started as a separator
        */
        eq = wcschr(multistring + 1, '=');
        if (eq == NULL) {
            // Use blank string for value if no equal sign present. ???? Maybe throw an error instead ????
            vallen = 0;
            val = PyWinObject_FromWCHAR(L"");
        }
        else {
            vallen = wcslen(++eq);
            val = PyUnicode_FromWideChar(eq, vallen);
        }
        keylen = totallen - (vallen + 1);
        key = PyUnicode_FromWideChar(multistring, keylen);
        if ((key == NULL) || (val == NULL) || (PyDict_SetItem(ret, key, val) == -1)) {
            Py_XDECREF(key);
            Py_XDECREF(val);
            Py_DECREF(ret);
            return NULL;
        }
        Py_DECREF(key);
        Py_DECREF(val);
        multistring += (totallen + 1);
        totallen = wcslen(multistring);
    }
    return ret;
}

void PyWinObject_FreePROFILEINFO(LPPROFILEINFO pi)
{
    PyWinObject_FreeWCHAR(pi->lpUserName);
    PyWinObject_FreeWCHAR(pi->lpProfilePath);
    PyWinObject_FreeWCHAR(pi->lpDefaultPath);
    PyWinObject_FreeWCHAR(pi->lpServerName);
    PyWinObject_FreeWCHAR(pi->lpPolicyPath);
    ZeroMemory(pi, sizeof(PROFILEINFO));
}

// @object PyPROFILEINFO|Dictionary containing data to fill a PROFILEINFO struct, to be passed to <om
// win32profile.LoadUserProfile>. UserName is only required member.
// @pyseeapi PROFILEINFO
// @prop <o PyUnicode>|UserName|Name of user for which to load profile
// @prop int|Flags|Combination of PI_* flags
// @prop <o PyUnicode>|ProfilePath|Path to roaming profile, can be None.  Use <om win32net.NetUserGetInfo> to retrieve
// user's profile path
// @prop <o PyUnicode>|DefaultPath|Path to Default user profile, can be None
// @prop <o PyUnicode>|ServerName|Domain controller, can be None
// @prop <o PyUnicode>|PolicyPath|Location of policy file, can be None
// @prop <o PyHKEY>|Profile|Handle to root of user's registry key. This member is output.
BOOL PyWinObject_AsPROFILEINFO(PyObject *ob, LPPROFILEINFO pi)
{
    BOOL ret;
    static char *elements[] = {"UserName",   "Flags",      "ProfilePath", "DefaultPath",
                               "ServerName", "PolicyPath", "Profile",     NULL};
    PyObject *obUserName = Py_None, *obProfilePath = Py_None, *obDefaultPath = Py_None, *obServerName = Py_None,
             *obPolicyPath = Py_None, *obhProfile = Py_None;
    PyObject *dummy_args = NULL;
    ZeroMemory(pi, sizeof(PROFILEINFOW));
    pi->dwSize = sizeof(PROFILEINFOW);
    if (!PyDict_Check(ob)) {
        PyErr_SetString(PyExc_TypeError, "PROFILEINFO must be a dictionary");
        return FALSE;
    }
    dummy_args = PyTuple_New(0);
    if (dummy_args == NULL)
        return FALSE;
    ret = PyArg_ParseTupleAndKeywords(dummy_args, ob, "O|kOOOOO:LoadUserProfile", elements, &obUserName, &pi->dwFlags,
                                      &obProfilePath, &obDefaultPath, &obServerName, &obPolicyPath, &obhProfile) &&
          PyWinObject_AsWCHAR(obUserName, &pi->lpUserName, FALSE) &&
          PyWinObject_AsWCHAR(obProfilePath, &pi->lpProfilePath, TRUE) &&
          PyWinObject_AsWCHAR(obDefaultPath, &pi->lpDefaultPath, TRUE) &&
          PyWinObject_AsWCHAR(obServerName, &pi->lpServerName, TRUE) &&
          PyWinObject_AsWCHAR(obPolicyPath, &pi->lpPolicyPath, TRUE) && PyWinObject_AsHANDLE(obhProfile, &pi->hProfile);

    Py_DECREF(dummy_args);
    if (!ret)
        PyWinObject_FreePROFILEINFO(pi);
    return ret;
}

// @pymethod <o PyHKEY>|win32profile|LoadUserProfile|Loads user settings into registry
// @comm SE_BACKUP_NAME and SE_RESTORE_NAME privs are required, but do not have to be enabled
// @rdesc Returns a handle to user's registry key.
PyObject *PyLoadUserProfile(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"Token", "ProfileInfo", NULL};
    CHECK_PFN(LoadUserProfile)
    PyObject *obhToken, *obPROFILEINFO, *ret = NULL;
    HANDLE hToken;
    DWORD dwFlags = 0;
    BOOL success = TRUE;
    PROFILEINFOW profileinfo = {NULL};

    if (!PyArg_ParseTupleAndKeywords(
            args, kwargs, "OO:LoadUserProfile", keywords,
            &obhToken,  // @pyparm <o PyHANDLE>|hToken||Logon token as returned by <om win32security.LogonUser>, <om
                        // win32security.OpenThreadToken>, etc
            &obPROFILEINFO))  // @pyparm <o PyPROFILEINFO>|ProfileInfo||Dictionary representing a PROFILEINFO structure
        return NULL;
    if (!PyWinObject_AsHANDLE(obhToken, &hToken))
        return NULL;
    if (!PyWinObject_AsPROFILEINFO(obPROFILEINFO, &profileinfo))
        return NULL;
    if (!(*pfnLoadUserProfile)(hToken, &profileinfo))
        PyWin_SetAPIError("LoadUserProfile");
    else
        ret = new PyHKEY(profileinfo.hProfile);
    PyWinObject_FreePROFILEINFO(&profileinfo);
    return ret;
}

// @pymethod |win32profile|UnloadUserProfile|Unloads user profile loaded by <om win32profile.LoadUserProfile>
PyObject *PyUnloadUserProfile(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"Token", "Profile", NULL};
    CHECK_PFN(UnloadUserProfile);
    PyObject *obhToken = NULL, *obhProfile = NULL;
    HANDLE hToken, hProfile;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "OO:UnloadUserProfile", keywords,
                                     &obhToken,  // @pyparm <o PyHANDLE>|Token||Logon token as returned by <om
                                                 // win32security.LogonUser>, <om win32security.OpenProcessToken>, etc
                                     &obhProfile))  // @pyparm <o PyHKEY>|Profile||Registry handle as returned by <om
                                                    // win32profile.LoadUserProfile>
        return NULL;

    if (!PyWinObject_AsHANDLE(obhToken, &hToken))
        return NULL;
    if (!PyWinObject_AsHANDLE(obhProfile, &hProfile))
        return NULL;
    if (!(*pfnUnloadUserProfile)(hToken, hProfile)) {
        PyWin_SetAPIError("UnloadUserProfile");
        return NULL;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod <o PyUnicode>|win32profile|GetProfilesDirectory|Retrieves directory where user profiles are stored
static PyObject *PyGetProfilesDirectory(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {NULL};
    CHECK_PFN(GetProfilesDirectory);
    WCHAR *profile_path = NULL;
    DWORD bufsize = 0, err = 0;
    PyObject *ret = NULL;
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, ":GetProfilesDirectory", keywords))
        return (NULL);
    (*pfnGetProfilesDirectory)(profile_path, &bufsize);
    if (bufsize == 0)
        return PyWin_SetAPIError("GetProfilesDirectory");
    profile_path = (WCHAR *)malloc(bufsize * sizeof(WCHAR));
    if (profile_path == NULL) {
        PyErr_Format(PyExc_MemoryError, "Unable to allocate %d characters", bufsize);
        return NULL;
    }
    if (!(*pfnGetProfilesDirectory)(profile_path, &bufsize))
        PyWin_SetAPIError("GetProfilesDirectory");
    else
        ret = PyWinObject_FromWCHAR(profile_path);
    free(profile_path);
    return ret;
}

// @pymethod <o PyUnicode>|win32profile|GetAllUsersProfileDirectory|Retrieve All Users profile path
static PyObject *PyGetAllUsersProfileDirectory(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {NULL};
    CHECK_PFN(GetAllUsersProfileDirectory);
    WCHAR *profile_path = NULL;
    DWORD bufsize = 0, err = 0;
    PyObject *ret = NULL;
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, ":GetAllUsersProfileDirectory", keywords))
        return (NULL);
    (*pfnGetAllUsersProfileDirectory)(profile_path, &bufsize);
    if (bufsize == 0)
        return PyWin_SetAPIError("GetAllUsersProfileDirectory");
    profile_path = (WCHAR *)malloc(bufsize * sizeof(WCHAR));
    if (profile_path == NULL) {
        PyErr_Format(PyExc_MemoryError, "Unable to allocate %d characters", bufsize);
        return NULL;
    }
    if (!(*pfnGetAllUsersProfileDirectory)(profile_path, &bufsize))
        PyWin_SetAPIError("GetAllUsersProfileDirectory");
    else
        ret = PyWinObject_FromWCHAR(profile_path);
    free(profile_path);
    return ret;
}

// @pymethod <o PyUnicode>|win32profile|GetDefaultUserProfileDirectory|Retrieve Default user profile
static PyObject *PyGetDefaultUserProfileDirectory(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {NULL};
    CHECK_PFN(GetDefaultUserProfileDirectory);
    WCHAR *profile_path = NULL;
    DWORD bufsize = 0, err = 0;
    PyObject *ret = NULL;
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, ":GetDefaultUserProfileDirectory", keywords))
        return (NULL);
    (*pfnGetDefaultUserProfileDirectory)(profile_path, &bufsize);
    if (bufsize == 0)
        return PyWin_SetAPIError("GetDefaultUserProfileDirectory");
    profile_path = (WCHAR *)malloc(bufsize * sizeof(WCHAR));
    if (profile_path == NULL) {
        PyErr_SetString(PyExc_MemoryError, "GetDefaultUserProfileDirectory unable to allocate unicode buffer");
        return NULL;
    }
    if (!(*pfnGetDefaultUserProfileDirectory)(profile_path, &bufsize))
        PyWin_SetAPIError("GetDefaultUserProfileDirectory");
    else
        ret = PyWinObject_FromWCHAR(profile_path);
    free(profile_path);
    return ret;
}

// @pymethod <o PyUnicode>|win32profile|GetUserProfileDirectory|Returns profile directory for a logon token
PyObject *PyGetUserProfileDirectory(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"Token", NULL};
    CHECK_PFN(GetUserProfileDirectory);
    HANDLE hToken;
    WCHAR *profile_path = NULL;
    DWORD bufsize = 0;
    PyObject *obhToken = NULL, *ret = NULL;
    if (!PyArg_ParseTupleAndKeywords(
            args, kwargs, "O:GetUserProfileDirectory", keywords,
            &obhToken))  // @pyparm <o PyHANDLE>|Token||User token as returned by <om win32security.LogonUser>
        return NULL;
    if (!PyWinObject_AsHANDLE(obhToken, &hToken))
        return NULL;
    (*pfnGetUserProfileDirectory)(hToken, profile_path, &bufsize);
    if (bufsize == 0)
        return PyWin_SetAPIError("GetUserProfileDirectory");
    profile_path = (WCHAR *)malloc(bufsize * sizeof(WCHAR));
    if (profile_path == NULL) {
        PyErr_Format(PyExc_MemoryError, "Unable to allocate %d characters", bufsize);
        return NULL;
    }
    if (!(*pfnGetUserProfileDirectory)(hToken, profile_path, &bufsize))
        PyWin_SetAPIError("GetUserProfileDirectory");
    else
        ret = PyWinObject_FromWCHAR(profile_path);
    free(profile_path);
    return ret;
}

//@pymethod |win32profile|DeleteProfile|Remove profile for a user identified by string SID from specified machine.
PyObject *PyDeleteProfile(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"SidString", "ProfilePath", "ComputerName", NULL};
    CHECK_PFN(DeleteProfile);
    PyObject *obstrsid = Py_None, *obprofile_path = Py_None, *obmachine = Py_None, *ret = NULL;
    WCHAR *strsid = NULL, *profile_path = NULL, *machine = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O|OO:DeleteProfile", keywords,
                                     &obstrsid,  // @pyparm <o PyUnicode>|SidString||String representation of user's
                                                 // Sid.  See <om win32security.ConvertSidToStringSid>.
                                     &obprofile_path,  // @pyparm <o PyUnicode>|ProfilePath|None|Profile directory,
                                                       // value queried from registry if not specified
                                     &obmachine))      // @pyparm <o PyUnicode>|ComputerName|None|Name of computer from
                                                   // which to delete profile, local machine assumed if not specified
        return NULL;
    if (PyWinObject_AsWCHAR(obstrsid, &strsid, FALSE) && PyWinObject_AsWCHAR(obprofile_path, &profile_path, TRUE) &&
        PyWinObject_AsWCHAR(obmachine, &machine, TRUE)) {
        if ((*pfnDeleteProfile)(strsid, profile_path, machine)) {
            Py_INCREF(Py_None);
            ret = Py_None;
        }
        else
            PyWin_SetAPIError("DeleteProfile");
    }
    PyWinObject_FreeWCHAR(strsid);
    PyWinObject_FreeWCHAR(profile_path);
    PyWinObject_FreeWCHAR(machine);
    return ret;
}

// @pymethod int|win32profile|GetProfileType|Returns type of current user's profile
// @rdesc Returns a combination of PT_* flags
PyObject *PyGetProfileType(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {NULL};
    CHECK_PFN(GetProfileType);
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, ":GetProfileType", keywords))
        return NULL;

    DWORD ptype = 0;
    if (!(*pfnGetProfileType)(&ptype)) {
        PyWin_SetAPIError("GetProfileType");
        return NULL;
    }
    return PyLong_FromUnsignedLong(ptype);
}

// @pymethod dict|win32profile|CreateEnvironmentBlock|Retrieves environment variables for a user
PyObject *PyCreateEnvironmentBlock(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"Token", "Inherit", NULL};
    HANDLE hToken;
    BOOL inherit;
    LPVOID env;

    PyObject *obhToken = NULL, *ret = NULL;
    if (!PyArg_ParseTupleAndKeywords(
            args, kwargs, "Ol:CreateEnvironmentBlock", keywords,
            &obhToken,  // @pyparm <o PyHANDLE>|Token||User token as returned by <om win32security.LogonUser>, use None
                        // to retrieve system variables only
            &inherit))  // @pyparm boolean|Inherit||Indicates if environment of current process should be inherited
        return NULL;
    if (!PyWinObject_AsHANDLE(obhToken, &hToken))
        return NULL;
    if (!CreateEnvironmentBlock(&env, hToken, inherit))
        PyWin_SetAPIError("CreateEnvironmentBlock");
    else {
        ret = PyWinObject_FromEnvironmentBlock((WCHAR *)env);
        DestroyEnvironmentBlock(env);
    }
    return ret;
}

// @pymethod dict|win32profile|GetEnvironmentStrings|Retrieves environment variables for current process
PyObject *PyGetEnvironmentStrings(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {NULL};
    WCHAR *env;
    PyObject *ret = NULL;
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, ":GetEnvironmentStrings", keywords))
        return NULL;
    env = GetEnvironmentStrings();
    if (env == NULL)
        PyWin_SetAPIError("GetEnvironmentStrings");
    else {
        ret = PyWinObject_FromEnvironmentBlock((WCHAR *)env);
        FreeEnvironmentStrings(env);
    }
    return ret;
}

//@pymethod <o PyUnicode>|win32profile|ExpandEnvironmentStringsForUser|Replaces environment variables in a string with
// per-user values
PyObject *PyExpandEnvironmentStringsForUser(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"Token", "Src", NULL};
    CHECK_PFN(ExpandEnvironmentStringsForUser);
    PyObject *obtoken, *obsrc, *ret = NULL;
    HANDLE htoken;
    WCHAR *src = NULL, *dst = NULL;
    DWORD bufsize;

    if (!PyArg_ParseTupleAndKeywords(
            args, kwargs, "OO:ExpandEnvironmentStringsForUser", keywords,
            &obtoken,  // @pyparm <o PyHANDLE>|Token||The logon token for a user.  Use None for system variables.
            &obsrc))   // @pyparm <o PyUnicode>|Src||String containing environment variables enclosed in % signs
        return NULL;
    if (!PyWinObject_AsHANDLE(obtoken, &htoken) || !PyWinObject_AsWCHAR(obsrc, &src, FALSE, &bufsize))
        return NULL;

    // Increase initial allocation to reduce reallocation
    // MSDN says the Size param is in TCHARS, but it acts as if it's in bytes
    bufsize *= 4;
    while (TRUE) {
        if (dst != NULL)
            free(dst);
        bufsize *= 2;
        dst = (WCHAR *)malloc(bufsize);
        if (dst == NULL) {
            PyErr_Format(PyExc_MemoryError, "Unable to allocate %d bytes", bufsize);
            break;
        }
        if ((*pfnExpandEnvironmentStringsForUser)(htoken, src, dst, bufsize)) {
            ret = PyWinObject_FromWCHAR(dst);
            break;
        }
        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
            PyWin_SetAPIError("ExpandEnvironmentStringsForUser");
            break;
        }
    }
    PyWinObject_FreeWCHAR(src);
    if (dst != NULL)
        free(dst);
    return ret;
}

// @module win32profile|Wraps functions for dealing with user profiles
static struct PyMethodDef win32profile_functions[] = {

    //@pymeth CreateEnvironmentBlock|Retrieves environment variables for a user
    {"CreateEnvironmentBlock", (PyCFunction)PyCreateEnvironmentBlock, METH_VARARGS | METH_KEYWORDS,
     "Retrieves environment variables for a user"},
    // @pymeth DeleteProfile|Removes a user's profile
    {"DeleteProfile", (PyCFunction)PyDeleteProfile, METH_VARARGS | METH_KEYWORDS, "Remove a user's profile"},
    // @pymeth ExpandEnvironmentStringsForUser|Replaces environment variables in a string with per-user values
    {"ExpandEnvironmentStringsForUser", (PyCFunction)PyExpandEnvironmentStringsForUser, METH_VARARGS | METH_KEYWORDS,
     "Replaces environment variables in a string with per-user values"},
    //@pymeth GetAllUsersProfileDirectory|Retrieve All Users profile directory
    {"GetAllUsersProfileDirectory", (PyCFunction)PyGetAllUsersProfileDirectory, METH_VARARGS | METH_KEYWORDS,
     "Retrieve All Users profile directory"},
    //@pymeth GetDefaultUserProfileDirectory|Retrieve profile path for Default user
    {"GetDefaultUserProfileDirectory", (PyCFunction)PyGetDefaultUserProfileDirectory, METH_VARARGS | METH_KEYWORDS,
     "Retrieve profile path for Default user"},
    //@pymeth GetEnvironmentStrings|Retrieves environment variables for current process
    {"GetEnvironmentStrings", (PyCFunction)PyGetEnvironmentStrings, METH_VARARGS | METH_KEYWORDS,
     "Retrieves environment variables for current process"},
    //@pymeth GetProfilesDirectory|Retrieves directory where user profiles are stored
    {"GetProfilesDirectory", (PyCFunction)PyGetProfilesDirectory, METH_VARARGS | METH_KEYWORDS,
     "Retrieves directory where user profiles are stored"},
    //@pymeth GetProfileType|Returns type of current user's profile
    {"GetProfileType", (PyCFunction)PyGetProfileType, METH_VARARGS | METH_KEYWORDS,
     "Returns type of current user's profile"},
    // @pymeth GetUserProfileDirectory|Returns profile directory for a logon token
    {"GetUserProfileDirectory", (PyCFunction)PyGetUserProfileDirectory, METH_VARARGS | METH_KEYWORDS,
     "Returns profile directory for a logon token"},
    //@pymeth LoadUserProfile|Load user settings for a login token
    {"LoadUserProfile", (PyCFunction)PyLoadUserProfile, METH_VARARGS | METH_KEYWORDS,
     "Load user settings for a login token"},
    //@pymeth UnloadUserProfile|Unload profile loaded by LoadUserProfile
    {"UnloadUserProfile", (PyCFunction)PyUnloadUserProfile, METH_VARARGS | METH_KEYWORDS,
     "Unload profile loaded by LoadUserProfile"},
    {NULL, NULL}};

PYWIN_MODULE_INIT_FUNC(win32profile)
{
    PYWIN_MODULE_INIT_PREPARE(win32profile, win32profile_functions, "Interface to the User Profile Api.");

    // PROFILEINFO flags
    PyModule_AddIntConstant(module, "PI_NOUI", PI_NOUI);
    PyModule_AddIntConstant(module, "PI_APPLYPOLICY", PI_APPLYPOLICY);

    // profile types
    PyModule_AddIntConstant(module, "PT_MANDATORY", PT_MANDATORY);
    PyModule_AddIntConstant(module, "PT_ROAMING", PT_ROAMING);
    PyModule_AddIntConstant(module, "PT_TEMPORARY", PT_TEMPORARY);

    HMODULE hmodule = PyWin_GetOrLoadLibraryHandle("userenv.dll");
    if (hmodule != NULL) {
        pfnDeleteProfile = (DeleteProfilefunc)GetProcAddress(hmodule, "DeleteProfileW");
        pfnExpandEnvironmentStringsForUser =
            (ExpandEnvironmentStringsForUserfunc)GetProcAddress(hmodule, "ExpandEnvironmentStringsForUserW");
        pfnGetAllUsersProfileDirectory =
            (GetAllUsersProfileDirectoryfunc)GetProcAddress(hmodule, "GetAllUsersProfileDirectoryW");
        pfnGetDefaultUserProfileDirectory =
            (GetDefaultUserProfileDirectoryfunc)GetProcAddress(hmodule, "GetDefaultUserProfileDirectoryW");
        pfnGetProfilesDirectory = (GetProfilesDirectoryfunc)GetProcAddress(hmodule, "GetProfilesDirectoryW");
        pfnGetProfileType = (GetProfileTypefunc)GetProcAddress(hmodule, "GetProfileType");
        pfnGetUserProfileDirectory = (GetUserProfileDirectoryfunc)GetProcAddress(hmodule, "GetUserProfileDirectoryW");
        pfnLoadUserProfile = (LoadUserProfilefunc)GetProcAddress(hmodule, "LoadUserProfileW");
        pfnUnloadUserProfile = (UnloadUserProfilefunc)GetProcAddress(hmodule, "UnloadUserProfile");
    }
    PYWIN_MODULE_INIT_RETURN_SUCCESS;
}
