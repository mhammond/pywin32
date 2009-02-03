/***********************************************************

win32netuser.cpp -- module for interface into NetUser part
of the Network API.  This is part of the win32net module.

// SWT 2/8/01 - added accessors for USER_MODALS_INFO_* 

***********************************************************/
// @doc
#include "PyWinTypes.h"
#include "lm.h"
#include "win32net.h"
#include "stddef.h"

#define UI0_ENTRY(name, t, r) { #name, t, offsetof(USER_INFO_0, usri0_##name), r }
#define UI1_ENTRY(name, t, r) { #name, t, offsetof(USER_INFO_1, usri1_##name), r }
#define UI2_ENTRY(name, t, r) { #name, t, offsetof(USER_INFO_2, usri2_##name), r }
#define UI3_ENTRY(name, t, r) { #name, t, offsetof(USER_INFO_3, usri3_##name), r }
#define UI4_ENTRY(name, t, r) { #name, t, offsetof(USER_INFO_4, usri4_##name), r }
#define UI10_ENTRY(name, t, r) { #name, t, offsetof(USER_INFO_10, usri10_##name), r }
#define UI11_ENTRY(name, t, r) { #name, t, offsetof(USER_INFO_11, usri11_##name), r }
#define UI20_ENTRY(name, t, r) { #name, t, offsetof(USER_INFO_20, usri20_##name), r }
#define UI1003_ENTRY(name, t, r) { #name, t, offsetof(USER_INFO_1003, usri1003_##name), r }
#define UI1005_ENTRY(name, t, r) { #name, t, offsetof(USER_INFO_1005, usri1005_##name), r }
#define UI1006_ENTRY(name, t, r) { #name, t, offsetof(USER_INFO_1006, usri1006_##name), r }
#define UI1007_ENTRY(name, t, r) { #name, t, offsetof(USER_INFO_1007, usri1007_##name), r }
#define UI1008_ENTRY(name, t, r) { #name, t, offsetof(USER_INFO_1008, usri1008_##name), r }
#define UI1009_ENTRY(name, t, r) { #name, t, offsetof(USER_INFO_1009, usri1009_##name), r }
#define UI1010_ENTRY(name, t, r) { #name, t, offsetof(USER_INFO_1010, usri1010_##name), r }
#define UI1011_ENTRY(name, t, r) { #name, t, offsetof(USER_INFO_1011, usri1011_##name), r }

#define UMI0_ENTRY(name, t, r) { #name, t, offsetof(USER_MODALS_INFO_0, usrmod0_##name), r }
#define UMI1_ENTRY(name, t, r) { #name, t, offsetof(USER_MODALS_INFO_1, usrmod1_##name), r }
#define UMI2_ENTRY(name, t, r) { #name, t, offsetof(USER_MODALS_INFO_2, usrmod2_##name), r }
#define UMI3_ENTRY(name, t, r) { #name, t, offsetof(USER_MODALS_INFO_3, usrmod3_##name), r }

// @object PyUSER_INFO_0|A dictionary holding the information in a Win32 USER_INFO_0 structure.
static struct PyNET_STRUCT_ITEM ui0[] = {
	UI0_ENTRY(name, NSI_WSTR, 0), // @prop string/<o PyUnicode>|name|
	{NULL}
};

// @object PyUSER_INFO_1|A dictionary holding the information in a Win32 USER_INFO_1 structure.
static struct PyNET_STRUCT_ITEM ui1[] = {
	UI1_ENTRY(name, NSI_WSTR, 0), // @prop string/<o PyUnicode>|name|
	UI1_ENTRY(password, NSI_WSTR, 0), // @prop string/<o PyUnicode>|password|
	UI1_ENTRY(password_age, NSI_DWORD, 0), // @prop int|password_age|
	UI1_ENTRY(priv, NSI_DWORD, 0), // @prop int|priv|
	UI1_ENTRY(home_dir, NSI_WSTR, 0), // @prop string/<o PyUnicode>|home_dir|
	UI1_ENTRY(comment, NSI_WSTR, 0), // @prop string/<o PyUnicode>|comment|
	UI1_ENTRY(flags, NSI_DWORD, 0), // @prop int|flags|
	UI1_ENTRY(script_path, NSI_WSTR, 0), // @prop string/<o PyUnicode>|script_path|
	{NULL}
};
// @object PyUSER_INFO_2|A dictionary holding the information in a Win32 USER_INFO_2 structure.
static struct PyNET_STRUCT_ITEM ui2[] = {
	UI2_ENTRY(name, NSI_WSTR, 0), // @prop string/<o PyUnicode>|name|
	UI2_ENTRY(password, NSI_WSTR, 0), // @prop string/<o PyUnicode>|password|
	UI2_ENTRY(password_age, NSI_DWORD, 0), // @prop int|password_age|
	UI2_ENTRY(priv, NSI_DWORD, 0), // @prop int|priv|
	UI2_ENTRY(home_dir, NSI_WSTR, 0), // @prop string/<o PyUnicode>|home_dir|
	UI2_ENTRY(comment, NSI_WSTR, 0), // @prop string/<o PyUnicode>|comment|
	UI2_ENTRY(flags, NSI_DWORD, 0), // @prop int|flags|
	UI2_ENTRY(script_path, NSI_WSTR, 0), // @prop string/<o PyUnicode>|script_path|
	UI2_ENTRY(auth_flags, NSI_DWORD, 0), // @prop int|auth_flags|
	UI2_ENTRY(full_name, NSI_WSTR, 0), // @prop string/<o PyUnicode>|full_name|
	UI2_ENTRY(usr_comment,NSI_WSTR, 0), // @prop string/<o PyUnicode>|usr_comment|
	UI2_ENTRY(parms, NSI_WSTR, 0), // @prop string/<o PyUnicode>|parms|
	UI2_ENTRY(workstations, NSI_WSTR, 0), // @prop string/<o PyUnicode>|workstations|
	UI2_ENTRY(last_logon, NSI_DWORD, 0), // @prop int|last_logon|
	UI2_ENTRY(last_logoff, NSI_DWORD, 0), // @prop int|last_logoff|
	UI2_ENTRY(acct_expires, NSI_DWORD, 0), // @prop int|acct_expires|
	UI2_ENTRY(max_storage, NSI_DWORD, 0), // @prop int|max_storage|
	UI2_ENTRY(units_per_week, NSI_DWORD, 0), // @prop int|units_per_week|
	UI2_ENTRY(logon_hours, NSI_HOURS, 0), // @prop string|logon_hours|
	UI2_ENTRY(bad_pw_count, NSI_DWORD, 0), // @prop int|bad_pw_count|
	UI2_ENTRY(num_logons, NSI_DWORD, 0), // @prop int|num_logons|
	UI2_ENTRY(logon_server, NSI_WSTR, 0), // @prop string/<o PyUnicode>|logon_server|
	UI2_ENTRY(country_code, NSI_DWORD, 0), // @prop int|country_code|
	UI2_ENTRY(code_page, NSI_DWORD, 0), // @prop int|code_page|
	{NULL}
};

// @object PyUSER_INFO_3|A dictionary holding the information in a Win32 USER_INFO_3 structure.
static struct PyNET_STRUCT_ITEM ui3[] = {
	UI3_ENTRY(name, NSI_WSTR, 0), // @prop string/<o PyUnicode>|name|
	UI3_ENTRY(password, NSI_WSTR, 0), // @prop string/<o PyUnicode>|password|
	UI3_ENTRY(password_age, NSI_DWORD, 0), // @prop int|password_age|
	UI3_ENTRY(priv, NSI_DWORD, 0), // @prop int|priv|
	UI3_ENTRY(home_dir, NSI_WSTR, 0), // @prop string/<o PyUnicode>|home_dir|
	UI3_ENTRY(comment, NSI_WSTR, 0), // @prop string/<o PyUnicode>|comment|
	UI3_ENTRY(flags, NSI_DWORD, 0), // @prop int|flags|
	UI3_ENTRY(script_path, NSI_WSTR, 0), // @prop string/<o PyUnicode>|script_path|
	UI3_ENTRY(auth_flags, NSI_DWORD, 0), // @prop int|auth_flags|
	UI3_ENTRY(full_name, NSI_WSTR, 0), // @prop string/<o PyUnicode>|full_name|
	UI3_ENTRY(usr_comment,NSI_WSTR, 0), // @prop string/<o PyUnicode>|usr_comment|
	UI3_ENTRY(parms, NSI_WSTR, 0), // @prop string/<o PyUnicode>|parms|
	UI3_ENTRY(workstations, NSI_WSTR, 0), // @prop string/<o PyUnicode>|workstations|
	UI3_ENTRY(last_logon, NSI_DWORD, 0), // @prop int|last_logon|
	UI3_ENTRY(last_logoff, NSI_DWORD, 0), // @prop int|last_logoff|
	UI3_ENTRY(acct_expires, NSI_DWORD, 0), // @prop int|acct_expires|
	UI3_ENTRY(max_storage, NSI_DWORD, 0), // @prop int|max_storage|
	UI3_ENTRY(units_per_week, NSI_DWORD, 0), // @prop int|units_per_week|
	UI3_ENTRY(logon_hours, NSI_HOURS, 0), // @prop string|logon_hours|
	UI3_ENTRY(bad_pw_count, NSI_DWORD, 0), // @prop int|bad_pw_count|
	UI3_ENTRY(num_logons, NSI_DWORD, 0), // @prop int|num_logons|
	UI3_ENTRY(logon_server, NSI_WSTR, 0), // @prop string/<o PyUnicode>|logon_server|
	UI3_ENTRY(country_code, NSI_DWORD, 0), // @prop int|country_code|
	UI3_ENTRY(code_page, NSI_DWORD, 0), // @prop int|code_page|
	UI3_ENTRY(user_id, NSI_DWORD, 0), // @prop int|user_id|
	UI3_ENTRY(primary_group_id, NSI_DWORD, 0), // @prop int|primary_group_id|
	UI3_ENTRY(profile, NSI_WSTR, 0), // @prop string/<o PyUnicode>|profile|
	UI3_ENTRY(home_dir_drive, NSI_WSTR, 0), // @prop string/<o PyUnicode>|home_dir_drive|
	UI3_ENTRY(password_expired, NSI_DWORD, 0), // @prop int|password_expired|
	{NULL}
};

// @object PyUSER_INFO_4|A dictionary holding the information in a Win32 USER_INFO_4 structure.
static struct PyNET_STRUCT_ITEM ui4[] = {
	UI4_ENTRY(name, NSI_WSTR, 0), // @prop string/<o PyUnicode>|name|
	UI4_ENTRY(password, NSI_WSTR, 0), // @prop string/<o PyUnicode>|password|
	UI4_ENTRY(password_age, NSI_DWORD, 0), // @prop int|password_age|
	UI4_ENTRY(priv, NSI_DWORD, 0), // @prop int|priv|
	UI4_ENTRY(home_dir, NSI_WSTR, 0), // @prop string/<o PyUnicode>|home_dir|
	UI4_ENTRY(comment, NSI_WSTR, 0), // @prop string/<o PyUnicode>|comment|
	UI4_ENTRY(flags, NSI_DWORD, 0), // @prop int|flags|
	UI4_ENTRY(script_path, NSI_WSTR, 0), // @prop string/<o PyUnicode>|script_path|
	UI4_ENTRY(auth_flags, NSI_DWORD, 0), // @prop int|auth_flags|
	UI4_ENTRY(full_name, NSI_WSTR, 0), // @prop string/<o PyUnicode>|full_name|
	UI4_ENTRY(usr_comment,NSI_WSTR, 0), // @prop string/<o PyUnicode>|usr_comment|
	UI4_ENTRY(parms, NSI_WSTR, 0), // @prop string/<o PyUnicode>|parms|
	UI4_ENTRY(workstations, NSI_WSTR, 0), // @prop string/<o PyUnicode>|workstations|
	UI4_ENTRY(last_logon, NSI_DWORD, 0), // @prop int|last_logon|
	UI4_ENTRY(last_logoff, NSI_DWORD, 0), // @prop int|last_logoff|
	UI4_ENTRY(acct_expires, NSI_DWORD, 0), // @prop int|acct_expires|
	UI4_ENTRY(max_storage, NSI_DWORD, 0), // @prop int|max_storage|
	UI4_ENTRY(units_per_week, NSI_DWORD, 0), // @prop int|units_per_week|
	UI4_ENTRY(logon_hours, NSI_HOURS, 0), // @prop string|logon_hours|
	UI4_ENTRY(bad_pw_count, NSI_DWORD, 0), // @prop int|bad_pw_count|
	UI4_ENTRY(num_logons, NSI_DWORD, 0), // @prop int|num_logons|
	UI4_ENTRY(logon_server, NSI_WSTR, 0), // @prop string/<o PyUnicode>|logon_server|
	UI4_ENTRY(country_code, NSI_DWORD, 0), // @prop int|country_code|
	UI4_ENTRY(code_page, NSI_DWORD, 0), // @prop int|code_page|
	UI4_ENTRY(user_sid, NSI_SID, 0), // @prop <o PySID>|user_sid|
	UI4_ENTRY(primary_group_id, NSI_DWORD, 0), // @prop int|primary_group_id|
	UI4_ENTRY(profile, NSI_WSTR, 0), // @prop string/<o PyUnicode>|profile|
	UI4_ENTRY(home_dir_drive, NSI_WSTR, 0), // @prop string/<o PyUnicode>|home_dir_drive|
	UI4_ENTRY(password_expired, NSI_DWORD, 0), // @prop int|password_expired|
	{NULL}
};

// @object PyUSER_INFO_10|A dictionary holding the information in a Win32 USER_INFO_10 structure.
static struct PyNET_STRUCT_ITEM ui10[] = {
	UI10_ENTRY(name, NSI_WSTR, 0), // @prop string/<o PyUnicode>|name|
	UI10_ENTRY(comment, NSI_WSTR, 0), // @prop string/<o PyUnicode>|comment|
	UI10_ENTRY(usr_comment, NSI_WSTR, 0), // @prop string/<o PyUnicode>|usr_comment|
	UI10_ENTRY(full_name, NSI_WSTR, 0), // @prop string/<o PyUnicode>|full_name|
	{NULL}
};

// @object PyUSER_INFO_11|A dictionary holding the information in a Win32 USER_INFO_11 structure.
static struct PyNET_STRUCT_ITEM ui11[] = {
	UI11_ENTRY(name, NSI_WSTR, 0), // @prop string/<o PyUnicode>|name|
	UI11_ENTRY(comment, NSI_WSTR, 0), // @prop string/<o PyUnicode>|comment|
	UI11_ENTRY(usr_comment, NSI_WSTR, 0), // @prop string/<o PyUnicode>|usr_comment|
	UI11_ENTRY(full_name, NSI_WSTR, 0), // @prop string/<o PyUnicode>|full_name|
	UI11_ENTRY(priv, NSI_DWORD, 0), // @prop int|priv|
	UI11_ENTRY(auth_flags, NSI_DWORD, 0), // @prop int|auth_flags|
	UI11_ENTRY(password_age, NSI_DWORD, 0), // @prop int|password_age|
	UI11_ENTRY(home_dir, NSI_WSTR, 0), // @prop string/<o PyUnicode>|home_dir|
	UI11_ENTRY(parms, NSI_WSTR, 0), // @prop string/<o PyUnicode>|parms|
	UI11_ENTRY(last_logon, NSI_DWORD, 0), // @prop int|last_logon|
	UI11_ENTRY(last_logoff, NSI_DWORD, 0), // @prop int|last_logoff|
	UI11_ENTRY(bad_pw_count, NSI_DWORD, 0), // @prop int|bad_pw_count|
	UI11_ENTRY(num_logons, NSI_DWORD, 0), // @prop int|num_logons|
	UI11_ENTRY(logon_server, NSI_WSTR, 0), // @prop string/<o PyUnicode>|logon_server|
	UI11_ENTRY(country_code, NSI_DWORD, 0), // @prop int|country_code|
	UI11_ENTRY(workstations, NSI_WSTR, 0), // @prop string/<o PyUnicode>|workstations|
	UI11_ENTRY(max_storage, NSI_DWORD, 0), // @prop int|max_storage|
	UI11_ENTRY(units_per_week, NSI_DWORD, 0), // @prop int|units_per_week|
	UI11_ENTRY(logon_hours, NSI_HOURS, 0), // @prop string|logon_hours|
	UI11_ENTRY(code_page, NSI_DWORD, 0), // @prop int|code_page|
	{NULL}
};


// @object PyUSER_INFO_20|A dictionary holding the information in a Win32 USER_INFO_20 structure.
static struct PyNET_STRUCT_ITEM ui20[] = {
	UI20_ENTRY(name, NSI_WSTR, 0), // @prop string/<o PyUnicode>|name|
	UI20_ENTRY(full_name, NSI_WSTR, 0), // @prop string/<o PyUnicode>|full_name|
	UI20_ENTRY(comment, NSI_WSTR, 0), // @prop string/<o PyUnicode>|comment|
	UI20_ENTRY(flags, NSI_DWORD, 0), // @prop int|flags|
	UI20_ENTRY(user_id, NSI_DWORD, 0), // @prop int|user_id|
	{NULL}
};

// @object PyUSER_INFO_1003|A dictionary holding the information in a Win32 USER_INFO_1003 structure.
static struct PyNET_STRUCT_ITEM ui1003[] = {
	UI1003_ENTRY(password, NSI_WSTR, 0), // @prop string/<o PyUnicode>|password|
	{NULL}
};

// @object PyUSER_INFO_1005|A dictionary holding the information in a Win32 USER_INFO_1005 structure.
static struct PyNET_STRUCT_ITEM ui1005[] = {
	UI1005_ENTRY(priv, NSI_DWORD, 0), // @prop int|priv|
	{NULL}
};

// @object PyUSER_INFO_1006|A dictionary holding the information in a Win32 USER_INFO_1006 structure.
static struct PyNET_STRUCT_ITEM ui1006[] = {
	UI1006_ENTRY(home_dir, NSI_WSTR, 0), // @prop string/<o PyUnicode>|home_dir|
	{NULL}
};

// @object PyUSER_INFO_1007|A dictionary holding the information in a Win32 USER_INFO_1007 structure.
static struct PyNET_STRUCT_ITEM ui1007[] = {
	UI1007_ENTRY(comment, NSI_WSTR, 0), // @prop string/<o PyUnicode>|comment|
	{NULL}
};

// @object PyUSER_INFO_1008|A dictionary holding the information in a Win32 USER_INFO_1008 structure.
static struct PyNET_STRUCT_ITEM ui1008[] = {
	UI1008_ENTRY(flags, NSI_DWORD, 0), // @prop int|flags|
	{NULL}
};

// @object PyUSER_INFO_1009|A dictionary holding the information in a Win32 USER_INFO_1009 structure.
static struct PyNET_STRUCT_ITEM ui1009[] = {
	UI1009_ENTRY(script_path, NSI_WSTR, 0), // @prop string/<o PyUnicode>|script_path|
	{NULL}
};

// @object PyUSER_INFO_1010|A dictionary holding the information in a Win32 USER_INFO_1010 structure.
static struct PyNET_STRUCT_ITEM ui1010[] = {
	UI1010_ENTRY(auth_flags, NSI_WSTR, 0), // @prop int|auth_flags|
	{NULL}
};

// @object PyUSER_INFO_1011|A dictionary holding the information in a Win32 USER_INFO_1011 structure.
static struct PyNET_STRUCT_ITEM ui1011[] = {
	UI1011_ENTRY(full_name, NSI_WSTR, 0), // @prop string/<o PyUnicode>|full_name|
	{NULL}
};

// @object PyUSER_INFO_*|The following USER_INFO levels are supported.
static struct PyNET_STRUCT user_infos[] = { // @flagh Level|Data
	{ 0, ui0, sizeof(USER_INFO_0) },        // @flag 0|<o PyUSER_INFO_0>
	{ 1, ui1, sizeof(USER_INFO_1) },		// @flag 1|<o PyUSER_INFO_1>
	{ 2, ui2, sizeof(USER_INFO_2) },		// @flag 2|<o PyUSER_INFO_2>
	{ 3, ui3, sizeof(USER_INFO_3) },		// @flag 3|<o PyUSER_INFO_3>
	{ 4, ui4, sizeof(USER_INFO_4) },		// @flag 4|<o PyUSER_INFO_4>
	{ 10, ui10, sizeof(USER_INFO_10) },		// @flag 10|<o PyUSER_INFO_10>
	{ 11, ui11, sizeof(USER_INFO_11) },		// @flag 11|<o PyUSER_INFO_11>
	{ 20, ui20, sizeof(USER_INFO_20) },		// @flag 20|<o PyUSER_INFO_20>
	{ 1003, ui1003, sizeof(USER_INFO_1003) },// @flag 1003|<o PyUSER_INFO_1003>
	{ 1005, ui1005, sizeof(USER_INFO_1005) },// @flag 1005|<o PyUSER_INFO_1005>
	{ 1006, ui1006, sizeof(USER_INFO_1006) },// @flag 1006|<o PyUSER_INFO_1006>
	{ 1007, ui1007, sizeof(USER_INFO_1007) },// @flag 1007|<o PyUSER_INFO_1007>
	{ 1008, ui1008, sizeof(USER_INFO_1008) },// @flag 1008|<o PyUSER_INFO_1008>
	{ 1009, ui1009, sizeof(USER_INFO_1009) },// @flag 1009|<o PyUSER_INFO_1009>
	{ 1010, ui1010, sizeof(USER_INFO_1010) },// @flag 1010|<o PyUSER_INFO_1010>
	{ 1011, ui1011, sizeof(USER_INFO_1011) },// @flag 1011|<o PyUSER_INFO_1011>
	{NULL}
};

// @object PyUSER_MODALS_INFO_0|A dictionary holding the information in a Win32 USER_MODALS_INFO_0 structure.
static struct PyNET_STRUCT_ITEM umi0[] = {
  UMI0_ENTRY(min_passwd_len, NSI_DWORD, 0), // @prop int|min_passwd_len|
  UMI0_ENTRY(max_passwd_age, NSI_DWORD, 0), // @prop int|max_passwd_age|
  UMI0_ENTRY(min_passwd_age, NSI_DWORD, 0), // @prop int|min_passwd_age|
  UMI0_ENTRY(force_logoff, NSI_DWORD, 0), // @prop int|force_logoff|
  UMI0_ENTRY(password_hist_len, NSI_DWORD, 0), // @prop int|password_hist_len|
  {NULL}
};

// @object PyUSER_MODALS_INFO_1|A dictionary holding the information in a Win32 USER_MODALS_INFO_1 structure.
static struct PyNET_STRUCT_ITEM umi1[] = {
  UMI1_ENTRY(role, NSI_DWORD, 0), // @prop int|role|
  UMI1_ENTRY(primary, NSI_WSTR, 0), // @prop string/<o PyUnicode>|primary|
  {NULL}
};

// @object PyUSER_MODALS_INFO_2|A dictionary holding the information in a Win32 USER_MODALS_INFO_2 structure.
static struct PyNET_STRUCT_ITEM umi2[] = {
  UMI2_ENTRY(domain_name, NSI_WSTR, 0), // @prop string/<o PyUnicode>|domain_name|
  UMI2_ENTRY(domain_id, NSI_SID, 0), // @prop <o PySID>|domain_id|
  {NULL}
};

// @object PyUSER_MODALS_INFO_3|A dictionary holding the information in a Win32 USER_MODALS_INFO_3 structure.
static struct PyNET_STRUCT_ITEM umi3[] = {
  UMI3_ENTRY(lockout_duration, NSI_DWORD, 0), // @prop int|lockout_duration|
  UMI3_ENTRY(lockout_observation_window, NSI_DWORD, 0), // @prop int|lockout_observation_window|
  UMI3_ENTRY(lockout_threshold, NSI_DWORD, 0), // @prop int|usrmod3_lockout_threshold|
  {NULL}
};

// @object PyUSER_MODALS_INFO_*|The following USER_MODALS_INFO levels are supported.
static struct PyNET_STRUCT user_modals_infos[] = { // @flagh Level|Data
	{ 0, umi0, sizeof(USER_MODALS_INFO_0) },        // @flag 0|<o PyUSER_MODALS_INFO_0>
	{ 1, umi1, sizeof(USER_MODALS_INFO_1) },		// @flag 1|<o PyUSER_MODALS_INFO_1>
	{ 2, umi2, sizeof(USER_MODALS_INFO_2) },		// @flag 2|<o PyUSER_MODALS_INFO_2>
	{ 3, umi3, sizeof(USER_MODALS_INFO_3) },		// @flag 3|<o PyUSER_MODALS_INFO_3>
	{NULL}
};

// @pymethod dict|win32net|NetUserModalsGet|Retrieves global user information on a server.
PyObject *PyNetUserModalsGet(PyObject *self, PyObject *args) 
{
	// @pyparm string/<o PyUnicode>|server||The name of the server, or None.
	// @pyparm int|level||The information level contained in the data
	// @rdesc The result will be a dictionary in one of the <o PyUSER_MODALS_INFO_*>
	// formats, depending on the level parameter.
	// @pyseeapi NetUserModalsGet
	return PyDoGetModalsInfo(self, args, NetUserModalsGet, "NetUserModalsGet", user_modals_infos);
}

// @pymethod |win32net|NetUserModalsSet|Sets global user parameters on a server.
PyObject *PyNetUserModalsSet(PyObject *self, PyObject *args) 
{
	// @pyparm string/<o PyUnicode>|server||The name of the server, or None.
	// @pyparm int|level||The information level contained in the data
	// @pyparm mapping|data||A dictionary holding the data in the format of <o PyUSER_MODALS_INFO_*>.
	// @pyseeapi NetUserModalsSet
	return PyDoSetModalsInfo(self, args, NetUserModalsSet, "NetUserModalsSet", user_modals_infos);
}

// @pymethod dict|win32net|NetUserGetInfo|Retrieves information about a particular user account on a server.
PyObject *PyNetUserGetInfo(PyObject *self, PyObject *args) 
{
	// @pyparm string/<o PyUnicode>|server||The name of the server, or None.
	// @pyparm string/<o PyUnicode>|username||The user name
	// @pyparm int|level||The information level contained in the data
	// @rdesc The result will be a dictionary in one of the <o PyUSER_INFO_*>
	// formats, depending on the level parameter.
	// @pyseeapi NetUserGetInfo
	return PyDoGetInfo(self, args, NetUserGetInfo, "NetUserGetInfo", user_infos);
}

// @pymethod |win32net|NetUserSetInfo|Sets information about a particular user account on a server.
PyObject *PyNetUserSetInfo(PyObject *self, PyObject *args) 
{
	// @pyparm string/<o PyUnicode>|server||The name of the server, or None.
	// @pyparm string/<o PyUnicode>|username||The user name
	// @pyparm int|level||The information level contained in the data
	// @pyparm mapping|data||A dictionary holding the user data in the format of <o PyUSER_INFO_*>
	// @pyseeapi NetUserSetInfo
	return PyDoSetInfo(self, args, NetUserSetInfo, "NetUserSetInfo", user_infos);
}

// @pymethod |win32net|NetUserAdd|Creates a new user.
PyObject *PyNetUserAdd(PyObject *self, PyObject *args) 
{
	// @pyparm string/<o PyUnicode>|server||The name of the server, or None.
	// @pyparm int|level||The information level contained in the data
	// @pyparm mapping|data||A dictionary holding the user data in the format of <o PyUSER_INFO_*>.
	// @pyseeapi NetUserAdd
	return PyDoAdd(self, args, &NetUserAdd, "NetUserAdd", user_infos);
}


// @pymethod |win32net|NetUserDel|Deletes a user.
PyObject *PyNetUserDel(PyObject *self, PyObject *args) 
{
	return PyDoDel(self, args, &NetUserDel, "NetUserDel");
	// @pyparm string/<o PyUnicode>|server||The name of the server, or None.
	// @pyparm string/<o PyUnicode>|username||The user name
	// @pyseeapi NetUserDel

}
// @pymethod ([dict, ...], total, resumeHandle)|win32net|NetUserEnum|Enumerates all users.
// @rdesc The result is a list of items read (with each item being a dictionary of format
// <o PyUSER_INFO_*>, depending on the level parameter),
// the total available, and a new "resume handle".  The first time you call
// this function, you should pass zero for the resume handle.  If more data
// is available than what was returned, a new non-zero resume handle will be
// returned, which can be used to call the function again to fetch more data.
// This process may repeat, each time with a new resume handle, until zero is
// returned for the new handle, indicating all the data has been read.
PyObject *PyNetUserEnum(PyObject *self, PyObject *args) 
{
	WCHAR *szServer = NULL;
	PyObject *obServer;
	PyObject *ret = NULL;
	PyNET_STRUCT *pInfo;
	DWORD err;
	DWORD dwPrefLen = MAX_PREFERRED_LENGTH;
	DWORD level;
	DWORD filter = FILTER_NORMAL_ACCOUNT;
	BOOL ok = FALSE;
	DWORD resumeHandle = 0;
	DWORD numRead, i;
	PyObject *list;
	BYTE *buf = NULL;
	DWORD totalEntries = 0;
	// @pyparm string/<o PyUnicode>|server||The name of the server, or None.
	// @pyparm int|level||The level of data required.
	// @pyparm int|filter|win32netcon.FILTER_NORMAL_ACCOUNT|The types of accounts to enumerate.
	// @pyparm int|resumeHandle|0|A resume handle.  See the return description for more information.
	// @pyparm int|prefLen|MAX_PREFERRED_LENGTH|The preferred length of the data buffer.
	if (!PyArg_ParseTuple(args, "Oi|iii", &obServer, &level, &filter, &resumeHandle, &dwPrefLen))
		return NULL;
	if (!PyWinObject_AsWCHAR(obServer, &szServer, TRUE))
		goto done;

	if (!FindNET_STRUCT(level, user_infos, &pInfo))
		goto done;

	err = NetUserEnum(szServer, level, filter, &buf, dwPrefLen, &numRead, &totalEntries, &resumeHandle);
	if (err!=0 && err != ERROR_MORE_DATA) {
		ReturnNetError("NetUserEnum",err);	// @pyseeapi NetUserEnum
		goto done;
	}
	list = PyList_New(numRead);
	if (list==NULL) goto done;
	for (i=0;i<numRead;i++) {
		PyObject *sub = PyObject_FromNET_STRUCT(pInfo, buf+(i*pInfo->structsize));
		if (sub==NULL) goto done;
		PyList_SET_ITEM(list, i, sub);
	}
	resumeHandle = err==0 ? 0 : resumeHandle;
	ret = Py_BuildValue("Oll", list, totalEntries, resumeHandle);
	Py_DECREF(list);
	ok = TRUE;
done:
	if (buf) NetApiBufferFree(buf);
	if (!ok) {
		Py_XDECREF(ret);
		ret = NULL;
	}
	PyWinObject_FreeWCHAR(szServer);
	return ret;
}

// @pymethod |win32net|NetUserChangePassword|Changes the password for a user.
PyObject *PyNetUserChangePassword(PyObject *self, PyObject *args) 
{
	// @comm A server or domain can be configured to require that a
	// user log on to change the password on a user account.
	// If that is the case, you need administrator or account operator access
	// to change the password for another user acount.
	// If logging on is not required, you can change the password for
	// any user account, so long as you know the current password.
	WCHAR *szServer = NULL;
	WCHAR *szName = NULL;
	WCHAR *szOld = NULL;
	WCHAR *szNew = NULL;
	PyObject *obName, *obServer, *obOld, *obNew;
	PyObject *ret = NULL;
	DWORD err = 0;
	// @pyparm string/<o PyUnicode>|server||The name of the server, or None.
	// @pyparm string/<o PyUnicode>|username||The user name, or None for the current username.
	// @pyparm string/<o PyUnicode>|oldPassword||The old password
	// @pyparm string/<o PyUnicode>|newPassword||The new password
	if (!PyArg_ParseTuple(args, "OOOO", &obServer, &obName, &obOld, &obNew))
		return NULL;
	if (!PyWinObject_AsWCHAR(obServer, &szServer, TRUE))
		goto done;
	if (!PyWinObject_AsWCHAR(obName, &szName, TRUE))
		goto done;
	if (!PyWinObject_AsWCHAR(obOld, &szOld, FALSE))
		goto done;
	if (!PyWinObject_AsWCHAR(obNew, &szNew, FALSE))
		goto done;

	err = NetUserChangePassword(szServer, szName, szOld, szNew);
	if (err) {
		ReturnNetError("NetUserChangePassword",err);	// @pyseeapi NetUserChangePassword
		goto done;
	}
	ret = Py_None;
	Py_INCREF(Py_None);
done:
	PyWinObject_FreeWCHAR(szServer);
	PyWinObject_FreeWCHAR(szName);
	PyWinObject_FreeWCHAR(szOld);
	PyWinObject_FreeWCHAR(szNew);
	return ret;
}

/******************************************************************************************************************
**	NetUserGetGroups
**
******************************************************************************************************************/
// @pymethod [(groupName, attribute), ...]|win32net|NetUserGetGroups|Returns a list of groups,attributes for all groups for the user.
// @todo This needs to be extended to support the new model, while
// not breaking existing code.  A default arg would be perfect.
PyObject *
PyNetUserGetGroups( PyObject *self, PyObject *args)
{
	DWORD dwBuffsize = MAX_PREFERRED_LENGTH;	
	PyWin_AutoFreeBstr	wzServerName;		// storage for incoming servername string pointer
	PyWin_AutoFreeBstr	wzUserName;			// incoming username
	PyObject *		obServerName;
	PyObject *              obUserName;

	
	if (!PyArg_ParseTuple(args, "OO:NetUserGetGroups",
			&obServerName, // @pyparm string|serverName||The name of the remote server on which the function is to execute. None or an empty string specifies the server program running on the local computer.
			&obUserName)) // @pyparm string|userName||The name of the user to search for in each group account.
		return NULL;

	if (!PyWinObject_AsAutoFreeBstr(obServerName, &wzServerName, TRUE))
		return NULL;
	if (!PyWinObject_AsAutoFreeBstr(obUserName, &wzUserName, FALSE))
		return NULL;

	DWORD dwMaxCount, dwCount;		// see the win32api call for how these are used.
	GROUP_USERS_INFO_1 *lpBuffer;
	NET_API_STATUS Errno;

	dwMaxCount = dwCount = 0;

	PyObject * pRetlist = PyList_New(0);	//create a return list of 0 size
	if (pRetlist==NULL) return NULL; // did we err?
	
	Py_BEGIN_ALLOW_THREADS
	Errno = NetUserGetGroups((BSTR)wzServerName, (BSTR)wzUserName, 1, (LPBYTE *)&lpBuffer, dwBuffsize, &dwCount, &dwMaxCount);
	Py_END_ALLOW_THREADS

	if (Errno == NERR_Success)	// if no error, then build the list
	{

		GROUP_USERS_INFO_1 *p_nr = lpBuffer;	// Enum Resource returns a buffer of successive structs

		if (dwCount > 0)	// we actually got something
		{
			do
			{
				PyObject *obName = PyWinObject_FromWCHAR(p_nr->grui1_name);
				PyObject *t_ob = Py_BuildValue("(Oi)",obName, p_nr->grui1_attributes);	
				Py_XDECREF(obName);

				int listerr = PyList_Append(pRetlist,t_ob);				// append our obj...Append does an INCREF!
				Py_DECREF(t_ob);
				if (listerr)	// or bail
				{
					Py_DECREF(pRetlist);	// free the Python List
					NetApiBufferFree((LPVOID)lpBuffer);
					return NULL;
				}

				p_nr++;	// next object (its a ++ because it is a typed pointer!)
				dwCount--;
			} while (dwCount);
		}; // if (dwCount > 0)

	}	
	else {	// ERROR Occurred
		Py_DECREF(pRetlist);
		return ReturnNetError("NetUserGetGroups", Errno);
	}
	// @rdesc Always makes the level 1 call and returns all data.
	// Data return format is a Python List.  Each "Item"
	// is a tuple of (groupname, attributes).  "(s,i)" respectively.  In NT 4 the attributes seem to be hardcoded to 7.
	// Earlier version of NT have not been tested.
	NetApiBufferFree((LPVOID)lpBuffer);
	return pRetlist;

}
/************************************************************************************************************
**	NetUserGetLocalGroups
**
*************************************************************************************************************/
// @pymethod [groupName, ...]|win32net|NetUserGetLocalGroups|Retrieves a list of local groups to which a specified user belongs.
// @todo This needs to be extended to support the new model, while
// not breaking existing code.  A default arg would be perfect.
PyObject *
PyNetUserGetLocalGroups( PyObject *self, PyObject *args)
{
	DWORD dwFlags = LG_INCLUDE_INDIRECT;
	DWORD dwBuffsize = 0xFFFFFFFF;	// request it all baby! 
	PyWin_AutoFreeBstr wzServerName;		// storage for incoming domain string pointer
	PyWin_AutoFreeBstr wzUserName;			// incoming username
	PyObject *obServerName;
	PyObject *obUserName;

	if (!PyArg_ParseTuple(args, "OO|i:NetUserGetLocalGroups",
			&obServerName, // @pyparm string|serverName||The name of the remote server on which the function is to execute. None or an empty string specifies the server program running on the local computer.
			&obUserName, // @pyparm string|userName||The name of the user to search for in each group account. This parameter can be of the form \<UserName\>, in which case the username is expected to be found on servername. The user name can also be of the form \<DomainName\>\\\<UserName\> in which case \<DomainName\> is associated with servername and \<UserName\> is expected to be to be found on that domain. 
			&dwFlags)) // @pyparm int|flags|LG_INCLUDE_INDIRECT|Flags for the call.
		return NULL;

	if (!PyWinObject_AsAutoFreeBstr(obServerName, &wzServerName, TRUE))
		return NULL;
	if (!PyWinObject_AsAutoFreeBstr(obUserName, &wzUserName, FALSE))
		return NULL;

	DWORD dwMaxCount, dwCount;		// see the win32api call for how these are used.
	LOCALGROUP_USERS_INFO_0 *lpBuffer;
	NET_API_STATUS Errno;

	dwMaxCount = dwCount = 0;

	PyObject * pRetlist = PyList_New(0);	//create a return list of 0 size
	if (pRetlist==NULL) return NULL; // did we err?
	
	Py_BEGIN_ALLOW_THREADS
	Errno = NetUserGetLocalGroups(wzServerName, wzUserName, 0, dwFlags, (LPBYTE *)&lpBuffer, dwBuffsize, &dwCount, &dwMaxCount);	// do the enumeration
    Py_END_ALLOW_THREADS

	if (Errno == NERR_Success)	// if no error, then build the list
	{

		LOCALGROUP_USERS_INFO_0 *p_nr = lpBuffer;	// Enum Resource returns a buffer of successive structs

		if (dwCount > 0)	// we actually got something
		{
			do
			{
				PyObject *t_ob = PyWinObject_FromWCHAR(p_nr->lgrui0_name);

				int listerr = PyList_Append(pRetlist,t_ob);				// append our obj...Append does an INCREF!

				Py_DECREF(t_ob);

				if (listerr)	// or bail
				{
					Py_DECREF(pRetlist);	// free the Python List
					NetApiBufferFree((LPVOID)lpBuffer);
					return NULL;
				}

				p_nr++;	// next object (its a ++ because it is a typed pointer!)
				dwCount--;
			} while (dwCount);
		}; // if (dwCount > 0)

	}	
	else	// ERROR Occurred
	{
		Py_DECREF(pRetlist);
		return ReturnNetError("NetUserGetLocalGroups", Errno);
	}

	NetApiBufferFree((LPVOID)lpBuffer);
	return pRetlist;
}
