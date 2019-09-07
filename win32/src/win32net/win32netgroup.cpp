// win32netgroup.cpp
//
// Groups related code for the win32net module.
//
// @doc
#include "PyWinTypes.h"
#include "lm.h"
#include "win32net.h"
#include "stddef.h"

#define GI0_ENTRY(name, t, r)                             \
    {                                                     \
#name, t, offsetof(GROUP_INFO_0, grpi0_##name), r \
    }
#define GI1_ENTRY(name, t, r)                             \
    {                                                     \
#name, t, offsetof(GROUP_INFO_1, grpi1_##name), r \
    }
#define GI2_ENTRY(name, t, r)                             \
    {                                                     \
#name, t, offsetof(GROUP_INFO_2, grpi2_##name), r \
    }
#define GI1002_ENTRY(name, t, r)                                \
    {                                                           \
#name, t, offsetof(GROUP_INFO_1002, grpi1002_##name), r \
    }
#define GI1005_ENTRY(name, t, r)                                \
    {                                                           \
#name, t, offsetof(GROUP_INFO_1005, grpi1005_##name), r \
    }

// @object PyGROUP_INFO_0|A dictionary holding the information in a Win32 GROUP_INFO_0 structure.
static struct PyNET_STRUCT_ITEM gi0[] = {
    GI0_ENTRY(name, NSI_WSTR, 0),  // @prop string/<o PyUnicode>|name|Name of the group
    {NULL}};

// @object PyGROUP_INFO_1|A dictionary holding the information in a Win32 GROUP_INFO_1 structure.
static struct PyNET_STRUCT_ITEM gi1[] = {
    GI1_ENTRY(name, NSI_WSTR, 0),     // @prop string/<o PyUnicode>|name|Name of the group
    GI1_ENTRY(comment, NSI_WSTR, 0),  // @prop string/<o PyUnicode>|comment|The group's comment.
    {NULL}};

// @object PyGROUP_INFO_2|A dictionary holding the information in a Win32 GROUP_INFO_2 structure.
static struct PyNET_STRUCT_ITEM gi2[] = {
    GI2_ENTRY(name, NSI_WSTR, 0),         // @prop string/<o PyUnicode>|name|Name of the group
    GI2_ENTRY(comment, NSI_WSTR, 0),      // @prop string/<o PyUnicode>|comment|The group's comment.
    GI2_ENTRY(group_id, NSI_DWORD, 0),    // @prop int|group_id|
    GI2_ENTRY(attributes, NSI_DWORD, 0),  // @prop int|attributes|
    {NULL}};

// @object PyGROUP_INFO_1002|A dictionary holding the information in a Win32 GROUP_INFO_1002 structure.
static struct PyNET_STRUCT_ITEM gi1002[] = {GI1002_ENTRY(comment, NSI_WSTR, 0),  // @prop string/<o PyUnicode>|comment|
                                            {NULL}};

// @object PyGROUP_INFO_1005|A dictionary holding the information in a Win32 GROUP_INFO_1005 structure.
static struct PyNET_STRUCT_ITEM gi1005[] = {GI1005_ENTRY(attributes, NSI_DWORD, 0),  // @prop int|attributes|
                                            {NULL}};

// @object PyGROUP_INFO_*|The following GROUP_INFO levels are supported.
static struct PyNET_STRUCT group_infos[] = {  // @flagh Level|Data
    {0, gi0, sizeof(GROUP_INFO_0)},           // @flag 0|<o PyGROUP_INFO_0>
    {1, gi1, sizeof(GROUP_INFO_1)},           // @flag 1|<o PyGROUP_INFO_1>
    {2, gi2, sizeof(GROUP_INFO_2)},           // @flag 2|<o PyGROUP_INFO_2>
    {1002, gi1002, sizeof(GROUP_INFO_1002)},  // @flag 1002|<o PyGROUP_INFO_1002>
    {1005, gi1005, sizeof(GROUP_INFO_1005)},  // @flag 1005|<o PyGROUP_INFO_1005>
    {0, NULL, 0}};

#define LGI0_ENTRY(name, t, r)                                  \
    {                                                           \
#name, t, offsetof(LOCALGROUP_INFO_0, lgrpi0_##name), r \
    }
#define LGI1_ENTRY(name, t, r)                                  \
    {                                                           \
#name, t, offsetof(LOCALGROUP_INFO_1, lgrpi1_##name), r \
    }
#define LGI1002_ENTRY(name, t, r)                                     \
    {                                                                 \
#name, t, offsetof(LOCALGROUP_INFO_1002, lgrpi1002_##name), r \
    }

// @object PyLOCALGROUP_INFO_0|A dictionary holding the information in a Win32 LOCALGROUP_INFO_0 structure.
static struct PyNET_STRUCT_ITEM LGI0[] = {
    LGI0_ENTRY(name, NSI_WSTR, 0),  // @prop string/<o PyUnicode>|name|Name of the group
    {NULL}};

// @object PyLOCALGROUP_INFO_1|A dictionary holding the information in a Win32 LOCALGROUP_INFO_1 structure.
static struct PyNET_STRUCT_ITEM LGI1[] = {
    LGI1_ENTRY(name, NSI_WSTR, 0),     // @prop string/<o PyUnicode>|name|Name of the group
    LGI1_ENTRY(comment, NSI_WSTR, 0),  // @prop string/<o PyUnicode>|comment|The group's comment.
    {NULL}};

// @object PyLOCALGROUP_INFO_1002|A dictionary holding the information in a Win32 LOCALGROUP_INFO_1002 structure.
static struct PyNET_STRUCT_ITEM LGI1002[] = {
    LGI1002_ENTRY(comment, NSI_WSTR, 0),  // @prop string/<o PyUnicode>|comment|
    {NULL}};

// @object PyLOCALGROUP_INFO_*|The following LOCALGROUP_INFO levels are supported.
static struct PyNET_STRUCT localgroup_infos[] = {   // @flagh Level|Data
    {0, LGI0, sizeof(LOCALGROUP_INFO_0)},           // @flag 0|<o PyLOCALGROUP_INFO_0>
    {1, LGI1, sizeof(LOCALGROUP_INFO_1)},           // @flag 1|<o PyLOCALGROUP_INFO_1>
    {1002, LGI1002, sizeof(LOCALGROUP_INFO_1002)},  // @flag 1002|<o PyLOCALGROUP_INFO_1002>
    {0, NULL, 0}};

#define LGMI0_ENTRY(name, t, r)                                         \
    {                                                                   \
#name, t, offsetof(LOCALGROUP_MEMBERS_INFO_0, lgrmi0_##name), r \
    }
// @object PyLOCALGROUP_MEMBERS_INFO_0|A dictionary holding the information in a Win32 LOCALGROUP_MEMBERS_INFO_0
// structure.
static struct PyNET_STRUCT_ITEM lgmi0[] = {LGMI0_ENTRY(sid, NSI_SID, 0),  // @prop <o PySID>|sid|
                                           {NULL}};

#define LGMI1_ENTRY(name, t, r)                                         \
    {                                                                   \
#name, t, offsetof(LOCALGROUP_MEMBERS_INFO_1, lgrmi1_##name), r \
    }
// @object PyLOCALGROUP_MEMBERS_INFO_1|A dictionary holding the information in a Win32 LOCALGROUP_MEMBERS_INFO_1
// structure.
static struct PyNET_STRUCT_ITEM lgmi1[] = {LGMI1_ENTRY(sid, NSI_SID, 0),         // @prop <o PySID>|sid|
                                           LGMI1_ENTRY(sidusage, NSI_DWORD, 0),  // @prop int|sidusage|
                                           LGMI1_ENTRY(name, NSI_WSTR, 0),       // @prop string/<o PyUnicode>|name|
                                           {NULL}};

#define LGMI2_ENTRY(name, t, r)                                         \
    {                                                                   \
#name, t, offsetof(LOCALGROUP_MEMBERS_INFO_2, lgrmi2_##name), r \
    }
// @object PyLOCALGROUP_MEMBERS_INFO_2|A dictionary holding the information in a Win32 LOCALGROUP_MEMBERS_INFO_2
// structure.
static struct PyNET_STRUCT_ITEM lgmi2[] = {
    LGMI2_ENTRY(sid, NSI_SID, 0),             // @prop <o PySID>|sid|
    LGMI2_ENTRY(sidusage, NSI_DWORD, 0),      // @prop int|sidusage|
    LGMI2_ENTRY(domainandname, NSI_WSTR, 0),  // @prop string/<o PyUnicode>|domainandname|string containing the name of
                                              // the member prefixed by the domain name and the "\" separator character
    {NULL}};

#define LGMI3_ENTRY(name, t, r)                                         \
    {                                                                   \
#name, t, offsetof(LOCALGROUP_MEMBERS_INFO_3, lgrmi3_##name), r \
    }
// @object PyLOCALGROUP_MEMBERS_INFO_3|A dictionary holding the information in a Win32 LOCALGROUP_MEMBERS_INFO_3
// structure.
static struct PyNET_STRUCT_ITEM lgmi3[] = {
    LGMI3_ENTRY(domainandname, NSI_WSTR, 0),  // @prop string/<o PyUnicode>|domainandname|string containing the name of
                                              // the member prefixed by the domain name and the "\" separator character
    {NULL}};

// @object PyLOCALGROUP_MEMBERS_INFO_*|The following LOCALGROUP_MEMBER_INFO levels are supported.
static struct PyNET_STRUCT localgroup_members_infos[] = {  // @flagh Level|Data
    {0, lgmi0, sizeof(LOCALGROUP_MEMBERS_INFO_0)},         // @flag 0|<o PyLOCALGROUP_MEMBERS_INFO_0>
    {1, lgmi1, sizeof(LOCALGROUP_MEMBERS_INFO_1)},         // @flag 1|<o PyLOCALGROUP_MEMBERS_INFO_1>
    {2, lgmi2, sizeof(LOCALGROUP_MEMBERS_INFO_2)},         // @flag 2|<o PyLOCALGROUP_MEMBERS_INFO_2>
    {3, lgmi3, sizeof(LOCALGROUP_MEMBERS_INFO_3)},         // @flag 3|<o PyLOCALGROUP_MEMBERS_INFO_3>
    {NULL}};

#define GUI0_ENTRY(name, t, r)                                  \
    {                                                           \
#name, t, offsetof(GROUP_USERS_INFO_0, grui0_##name), r \
    }
#define GUI1_ENTRY(name, t, r)                                  \
    {                                                           \
#name, t, offsetof(GROUP_USERS_INFO_1, grui1_##name), r \
    }
// @object PyGROUP_USERS_INFO_0|A dictionary holding the information in a Win32 GROUP_USERS_INFO_0 structure.
static struct PyNET_STRUCT_ITEM gui0[] = {
    GUI0_ENTRY(name, NSI_WSTR, 0),  // @prop string/<o PyUnicode>|name|Name of the group or user
    {NULL}};
// @object PyGROUP_USERS_INFO_1|A dictionary holding the information in a Win32 GROUP_USERS_INFO_1 structure.
static struct PyNET_STRUCT_ITEM gui1[] = {
    GUI1_ENTRY(name, NSI_WSTR, 0),         // @prop string/<o PyUnicode>|name|Name of the group or user
    GUI1_ENTRY(attributes, NSI_DWORD, 0),  // @prop int|attributes|
    {NULL}};

// @object PyGROUP_USERS_INFO_*|The following GROUP_USERS_INFO levels are supported.
static struct PyNET_STRUCT group_users_infos[] = {  // @flagh Level|Data
    {0, gui0, sizeof(GROUP_USERS_INFO_0)},          // @flag 0|<o PyGROUP_USERS_INFO_0>
    {1, gui1, sizeof(GROUP_USERS_INFO_1)},          // @flag 1|<o PyGROUP_USERS_INFO_1>
    {0, NULL, 0}};

// @pymethod dict|win32net|NetGroupGetInfo|Retrieves information about a particular group on a server.
PyObject *PyNetGroupGetInfo(PyObject *self, PyObject *args)
{
    // @pyparm string/<o PyUnicode>|server||The name of the server, or None.
    // @pyparm string/<o PyUnicode>|groupname||The group name
    // @pyparm int|level||The information level contained in the data
    // @pyseeapi NetGroupGetInfo
    // @rdesc The result will be a dictionary in one of the <o PyGROUP_INFO_*>
    // formats, depending on the level parameter.
    return PyDoGetInfo(self, args, NetGroupGetInfo, "NetGroupGetInfo", group_infos);
}

// @pymethod |win32net|NetGroupSetInfo|Sets information about a particular group account on a server.
PyObject *PyNetGroupSetInfo(PyObject *self, PyObject *args)
{
    // @pyparm string/<o PyUnicode>|server||The name of the server, or None.
    // @pyparm string/<o PyUnicode>|groupname||The group name
    // @pyparm int|level||The information level contained in the data
    // @pyparm <o PyGROUP_INFO_*>|data||A dictionary holding the group data.
    // @pyseeapi NetGroupSetInfo
    return PyDoSetInfo(self, args, NetGroupSetInfo, "NetGroupSetInfo", group_infos);
}

// @pymethod |win32net|NetGroupAdd|Creates a new group.
PyObject *PyNetGroupAdd(PyObject *self, PyObject *args)
{
    // @pyparm string/<o PyUnicode>|server||The name of the server, or None.
    // @pyparm int|level||The information level contained in the data
    // @pyparm <o PyGROUP_INFO_*>|data||A dictionary holding the group data.
    return PyDoAdd(self, args, &NetGroupAdd, "NetGroupAdd", group_infos);
    // @pyseeapi NetGroupAdd
}

// @pymethod |win32net|NetGroupDel|Deletes a group.
PyObject *PyNetGroupDel(PyObject *self, PyObject *args)
{
    // @pyparm string/<o PyUnicode>|server||The name of the server, or None.
    // @pyparm string/<o PyUnicode>|groupname||The group name
    return PyDoDel(self, args, &NetGroupDel, "NetGroupDel");
    // @pyseeapi NetGroupDel
}

// @pymethod |win32net|NetGroupAddUser|Adds a user to the group
PyObject *PyNetGroupAddUser(PyObject *self, PyObject *args)
{
    WCHAR *szServer = NULL;
    WCHAR *szName = NULL;
    WCHAR *szGroup = NULL;
    PyObject *obName, *obServer, *obGroup;
    PyObject *ret = NULL;
    DWORD err = 0;
    // @pyparm string/<o PyUnicode>|server||The name of the server, or None.
    // @pyparm string/<o PyUnicode>|group||The group name
    // @pyparm string/<o PyUnicode>|username||The user to add to the group.
    if (!PyArg_ParseTuple(args, "OOO", &obServer, &obGroup, &obName))
        return NULL;
    if (!PyWinObject_AsWCHAR(obServer, &szServer, TRUE))
        goto done;
    if (!PyWinObject_AsWCHAR(obName, &szName, FALSE))
        goto done;
    if (!PyWinObject_AsWCHAR(obGroup, &szGroup, FALSE))
        goto done;

    err = NetGroupAddUser(szServer, szGroup, szName);
    if (err) {
        ReturnNetError("NetGroupAddUser", err);  // @pyseeapi NetGroupAddUser
        goto done;
    }
    ret = Py_None;
    Py_INCREF(Py_None);
done:
    PyWinObject_FreeWCHAR(szServer);
    PyWinObject_FreeWCHAR(szName);
    PyWinObject_FreeWCHAR(szGroup);
    return ret;
}

// @pymethod |win32net|NetGroupDelUser|Deletes a user from the group
PyObject *PyNetGroupDelUser(PyObject *self, PyObject *args)
{
    WCHAR *szServer = NULL;
    WCHAR *szName = NULL;
    WCHAR *szGroup = NULL;
    PyObject *obName, *obServer, *obGroup;
    PyObject *ret = NULL;
    DWORD err = 0;
    // @pyparm string/<o PyUnicode>|server||The name of the server, or None.
    // @pyparm string/<o PyUnicode>|group||The group name
    // @pyparm string/<o PyUnicode>|username||The user to delete from the group.
    if (!PyArg_ParseTuple(args, "OOO", &obServer, &obGroup, &obName))
        return NULL;
    if (!PyWinObject_AsWCHAR(obServer, &szServer, TRUE))
        goto done;
    if (!PyWinObject_AsWCHAR(obName, &szName, FALSE))
        goto done;
    if (!PyWinObject_AsWCHAR(obGroup, &szGroup, FALSE))
        goto done;

    err = NetGroupDelUser(szServer, szGroup, szName);
    if (err) {
        ReturnNetError("NetGroupDelUser", err);  // @pyseeapi NetGroupDelUser
        goto done;
    }
    ret = Py_None;
    Py_INCREF(Py_None);
done:
    PyWinObject_FreeWCHAR(szServer);
    PyWinObject_FreeWCHAR(szName);
    PyWinObject_FreeWCHAR(szGroup);
    return ret;
}

// @pymethod ([dict, ...], total, resumeHandle)|win32net|NetGroupEnum|Enumerates all groups.
// @rdesc The result is a list of items read (with each item being a dictionary of format
// <o PyGROUP_INFO_*>, depending on the level parameter),
// the total available, and a new "resume handle".  The first time you call
// this function, you should pass zero for the resume handle.  If more data
// is available than what was returned, a new non-zero resume handle will be
// returned, which can be used to call the function again to fetch more data.
// This process may repeat, each time with a new resume handle, until zero is
// returned for the new handle, indicating all the data has been read.
PyObject *PyNetGroupEnum(PyObject *self, PyObject *args)
{
    // @pyparm string/<o PyUnicode>|server||The name of the server, or None.
    // @pyparm int|level||The level of data required.
    // @pyparm int|resumeHandle|0|A resume handle.  See the return description for more information.
    // @pyparm int|prefLen|MAX_PREFERRED_LENGTH|The preferred length of the data buffer.
    // @pyseeapi NetGroupEnum
    return PyDoSimpleEnum(self, args, &NetGroupEnum, "NetGroupEnum", group_infos);
}

// @pymethod ([dict, ...], total, resumeHandle)|win32net|NetGroupGetUsers|Enumerates the users in a group.
// @rdesc The result is a list of items read (with each item being a dictionary of format
// <o PyGROUP_USERS_INFO_*>, depending on the level parameter),
// the total available, and a new "resume handle".  The first time you call
// this function, you should pass zero for the resume handle.  If more data
// is available than what was returned, a new non-zero resume handle will be
// returned, which can be used to call the function again to fetch more data.
// This process may repeat, each time with a new resume handle, until zero is
// returned for the new handle, indicating all the data has been read.
PyObject *PyNetGroupGetUsers(PyObject *self, PyObject *args)
{
    // @pyparm string/<o PyUnicode>|server||The name of the server, or None.
    // @pyparm string/<o PyUnicode>|groupName||The name of the local group.
    // @pyparm int|level||The level of data required.
    // @pyparm int|resumeHandle|0|A resume handle.  See the return description for more information.
    // @pyparm int|prefLen|4096|The preferred length of the data buffer.
    // @pyseeapi NetGroupGetUsers
    return PyDoNamedEnum(self, args, &NetGroupGetUsers, "NetGroupGetUsers", group_users_infos);
}

PyObject *PyNetGroupSetUsers(PyObject *self, PyObject *args)
{
    // @pymethod |win32net|NetGroupSetUsers|Sets the members of a local group.  Any existing members not listed are
    // removed.
    // @pyparm string/<o PyUnicode>|server||The name of the server, or None.
    // @pyparm string/<o PyUnicode>|group||The group name
    // @pyparm int|level||The level of information in the data. Must be 0
    // @pyparm [<o PyGROUP_USERS_INFO_0>, ..]|members||The list of new members
    // to add.
    // @pyseeapi NetGroupSetUsers
    return PyDoGroupSet(self, args, &NetGroupSetUsers, "NetGroupSetUsers", group_users_infos);
}

/////////////////////////////////////////////////////////////
//
// LocalGroup
// @pymethod dict|win32net|NetLocalGroupGetInfo|Retrieves information about a particular group on a server.
PyObject *PyNetLocalGroupGetInfo(PyObject *self, PyObject *args)
{
    return PyDoGetInfo(self, args, NetLocalGroupGetInfo, "NetLocalGroupGetInfo", localgroup_infos);
    // @pyparm string/<o PyUnicode>|server||The name of the server, or None.
    // @pyparm string/<o PyUnicode>|groupname||The group name
    // @pyparm int|level||The information level contained in the data
    // @pyseeapi NetLocalGroupGetInfo
    // @rdesc The result will be a dictionary in one of the <o PyLOCALGROUP_INFO_*>
    // formats, depending on the level parameter.
}

// @pymethod |win32net|NetLocalGroupSetInfo|Sets information about a particular group account on a server.
PyObject *PyNetLocalGroupSetInfo(PyObject *self, PyObject *args)
{
    return PyDoSetInfo(self, args, NetLocalGroupSetInfo, "NetLocalGroupSetInfo", localgroup_infos);
    // @pyparm string/<o PyUnicode>|server||The name of the server, or None.
    // @pyparm string/<o PyUnicode>|groupname||The group name
    // @pyparm int|level||The information level contained in the data
    // @pyparm <o PyLOCALGROUP_INFO_*>|data||A dictionary holding the group data.
    // @pyseeapi NetLocalGroupSetInfo
}

// @pymethod |win32net|NetLocalGroupAdd|Creates a new group.
PyObject *PyNetLocalGroupAdd(PyObject *self, PyObject *args)
{
    // @pyparm string/<o PyUnicode>|server||The name of the server, or None.
    // @pyparm int|level||The information level contained in the data
    // @pyparm <o PyLOCALGROUP_INFO_*>|data||A dictionary holding the group data.
    return PyDoAdd(self, args, &NetLocalGroupAdd, "NetLocalGroupAdd", localgroup_infos);
    // @pyseeapi NetLocalGroupAdd
}

// @pymethod |win32net|NetLocalGroupDel|Deletes a group.
PyObject *PyNetLocalGroupDel(PyObject *self, PyObject *args)
{
    // @pyparm string/<o PyUnicode>|server||The name of the server, or None.
    // @pyparm string/<o PyUnicode>|groupname||The group name
    return PyDoDel(self, args, &NetLocalGroupDel, "NetLocalGroupDel");
    // @pyseeapi NetLocalGroupDel
}

// @pymethod ([dict, ...], total, resumeHandle)|win32net|NetLocalGroupEnum|Enumerates all groups.
// @rdesc The result is a list of items read (with each item being a dictionary of format
// <o PyGROUP_INFO_*>, depending on the level parameter),
// the total available, and a new "resume handle".  The first time you call
// this function, you should pass zero for the resume handle.  If more data
// is available than what was returned, a new non-zero resume handle will be
// returned, which can be used to call the function again to fetch more data.
// This process may repeat, each time with a new resume handle, until zero is
// returned for the new handle, indicating all the data has been read.
PyObject *PyNetLocalGroupEnum(PyObject *self, PyObject *args)
{
    // @pyparm string/<o PyUnicode>|server||The name of the server, or None.
    // @pyparm int|level||The level of data required.
    // @pyparm int|resumeHandle|0|A resume handle.  See the return description for more information.
    // @pyparm int|prefLen|MAX_PREFERRED_LENGTH|The preferred length of the data buffer.
    // @pyseeapi NetLocalGroupEnum
    return PyDoSimpleEnum(self, args, &NetLocalGroupEnum, "NetLocalGroupEnum", localgroup_infos);
}

PyObject *PyNetLocalGroupAddMembers(PyObject *self, PyObject *args)
{
    // @pymethod |win32net|NetLocalGroupAddMembers|Adds users to a local group.
    // @pyparm string/<o PyUnicode>|server||The name of the server, or None.
    // @pyparm string/<o PyUnicode>|group||The group name
    // @pyparm int|level||The level of information in the data.
    // @pyparm [<o PyLOCALGROUP_MEMBERS_INFO_*>, ]|members||The new members to add.
    // @pyseeapi NetLocalGroupAddMembers
    return PyDoGroupSet(self, args, &NetLocalGroupAddMembers, "NetLocalGroupAddMembers", localgroup_members_infos);
}

PyObject *PyNetLocalGroupSetMembers(PyObject *self, PyObject *args)
{
    // @pymethod |win32net|NetLocalGroupSetMembers|Sets the members of a local group. Any existing members not listed
    // are removed.
    // @pyparm string/<o PyUnicode>|server||The name of the server, or None.
    // @pyparm string/<o PyUnicode>|group||The group name
    // @pyparm int|level||The level of information in the data.
    // @pyparm [<o PyLOCALGROUP_MEMBERS_INFO_*>, ..]|members||The list of new members to add.
    // @pyseeapi NetLocalGroupSetMembers
    return PyDoGroupSet(self, args, &NetLocalGroupSetMembers, "NetLocalGroupSetMembers", localgroup_members_infos);
}

// @pymethod ([dict, ...], total, resumeHandle)|win32net|NetLocalGroupGetMembers|Enumerates the members in a local
// group.
// @rdesc The result is a list of items read (with each item being a dictionary of format
// <o PyLOCALGROUP_MEMBERS_INFO_*>, depending on the level parameter),
// the total available, and a new "resume handle".  The first time you call
// this function, you should pass zero for the resume handle.  If more data
// is available than what was returned, a new non-zero resume handle will be
// returned, which can be used to call the function again to fetch more data.
// This process may repeat, each time with a new resume handle, until zero is
// returned for the new handle, indicating all the data has been read.
PyObject *PyNetLocalGroupGetMembers(PyObject *self, PyObject *args)
{
    // @pyparm string/<o PyUnicode>|server||The name of the server, or None.
    // @pyparm string/<o PyUnicode>|groupName||The name of the local group.
    // @pyparm int|level||The level of data required.
    // @pyparm int|resumeHandle|0|A resume handle.  See the return description for more information.
    // @pyparm int|prefLen|4096|The preferred length of the data buffer.
    // @pyseeapi NetLocalGroupGetMembers
    return PyDoNamedEnum(self, args, &NetLocalGroupGetMembers, "NetLocalGroupGetMembers", localgroup_members_infos);
}

PyObject *PyNetLocalGroupDelMembers(PyObject *self, PyObject *args)
{
    // @pymethod |win32net|NetLocalGroupDelMembers|Deletes users from a local group.
    // @pyparm string/<o PyUnicode>|server||The name of the server, or None.
    // @pyparm string/<o PyUnicode>|group||The group name
    // @pyparm [string, ...]|members||A list of strings with fully qualified user names to
    // delete from a local group.
    // @pyseeapi NetLocalGroupDelMembers
    return PyDoGroupDelMembers(self, args);
}
