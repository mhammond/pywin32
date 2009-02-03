// win32netmisc.cpp
//
// misc structures
//
// @doc
#include "PyWinTypes.h"
#include "lm.h"
#include "lmuseflg.h"
#include "win32net.h"
#include "stddef.h"

#define SI0_ENTRY(name, t, r) { #name, t, offsetof(SHARE_INFO_0, shi0_##name), r }
// @object PySHARE_INFO_0|A dictionary holding the infomation in a Win32 SHARE_INFO_0 structure.
static struct PyNET_STRUCT_ITEM si0[] = {
	SI0_ENTRY(netname, NSI_WSTR, 0), // @prop string/<o PyUnicode>|netname|
	{NULL}
};

#define SI1_ENTRY(name, t, r) { #name, t, offsetof(SHARE_INFO_1, shi1_##name), r }
// @object PySHARE_INFO_1|A dictionary holding the infomation in a Win32 SHARE_INFO_1 structure.
static struct PyNET_STRUCT_ITEM si1[] = {
	SI1_ENTRY(netname, NSI_WSTR, 0), // @prop string/<o PyUnicode>|netname|
	SI1_ENTRY(type, NSI_DWORD, 0), // @prop int|type|
	SI1_ENTRY(remark, NSI_WSTR, 0), // @prop string/<o PyUnicode>|remark|
	{NULL}
};

#define SI2_ENTRY(name, t, r) { #name, t, offsetof(SHARE_INFO_2, shi2_##name), r }
// @object PySHARE_INFO_2|A dictionary holding the infomation in a Win32 SHARE_INFO_2 structure.
static struct PyNET_STRUCT_ITEM si2[] = {
	SI2_ENTRY(netname, NSI_WSTR, 0), // @prop string/<o PyUnicode>|netname|
	SI2_ENTRY(type, NSI_DWORD, 0), // @prop int|type|
	SI2_ENTRY(remark, NSI_WSTR, 0), // @prop string/<o PyUnicode>|remark|
	SI2_ENTRY(permissions, NSI_DWORD, 0), // @prop int|permissions|
	SI2_ENTRY(max_uses, NSI_DWORD, 0), // @prop int|max_uses|
	SI2_ENTRY(current_uses, NSI_DWORD, 0), // @prop int|current_uses|
	SI2_ENTRY(path, NSI_WSTR, 0), // @prop string/<o PyUnicode>|path|
	SI2_ENTRY(passwd, NSI_WSTR, 0), // @prop string/<o PyUnicode>|passwd|
	{NULL}
};

#define SI501_ENTRY(name, t, r) { #name, t, offsetof(SHARE_INFO_501, shi501_##name), r }
// @object PySHARE_INFO_501|A dictionary holding the infomation in a Win32 SHARE_INFO_501 structure.
static struct PyNET_STRUCT_ITEM si501[] = {
	SI501_ENTRY(netname, NSI_WSTR, 0), // @prop string/<o PyUnicode>|netname|
	SI501_ENTRY(type, NSI_DWORD, 0), // @prop int|type|
	SI501_ENTRY(remark, NSI_WSTR, 0), // @prop string/<o PyUnicode>|remark|
	SI501_ENTRY(flags, NSI_DWORD, 0), // @prop int|flags|
	{NULL}
};

#define SI502_ENTRY(name, t, r) { #name, t, offsetof(SHARE_INFO_502, shi502_##name), r }
// @object PySHARE_INFO_502|A dictionary holding the infomation in a Win32 SHARE_INFO_502 structure.
static struct PyNET_STRUCT_ITEM si502[] = {
	SI502_ENTRY(netname, NSI_WSTR, 0), // @prop string/<o PyUnicode>|netname|
	SI502_ENTRY(type, NSI_DWORD, 0), // @prop int|type|
	SI502_ENTRY(remark, NSI_WSTR, 0), // @prop string/<o PyUnicode>|remark|
	SI502_ENTRY(permissions, NSI_DWORD, 0), // @prop int|permissions|
	SI502_ENTRY(max_uses, NSI_DWORD, 0), // @prop int|max_uses|
	SI502_ENTRY(current_uses, NSI_DWORD, 0), // @prop int|current_uses|
	SI502_ENTRY(path, NSI_WSTR, 0), // @prop string/<o PyUnicode>|path|
	SI502_ENTRY(passwd, NSI_WSTR, 0), // @prop string/<o PyUnicode>|passwd|
	SI502_ENTRY(reserved, NSI_DWORD, 0), // @prop int|reserved|
	SI502_ENTRY(security_descriptor, NSI_SECURITY_DESCRIPTOR, 0), // @prop <o PySECURITY_DESCRIPTOR>|security_descriptor|
	{NULL}
};

// @object PySHARE_INFO_*|The following SHARE_INFO levels are supported.
static struct PyNET_STRUCT share_infos[] = { // @flagh Level|Data
	{ 0, si0, sizeof(SHARE_INFO_0) },        // @flag 0|<o PySHARE_INFO_0>
	{ 1, si1, sizeof(SHARE_INFO_1) },		// @flag 1|<o PySHARE_INFO_1>
	{ 2, si2, sizeof(SHARE_INFO_2) },		// @flag 2|<o PySHARE_INFO_2>
	{ 501, si501, sizeof(SHARE_INFO_501) },		// @flag 501|<o PySHARE_INFO_501>
	{ 502, si502, sizeof(SHARE_INFO_502) },		// @flag 502|<o PySHARE_INFO_502>
	{ 0, NULL, 0}
};

#define WKI100_ENTRY(name, t, r) { #name, t, offsetof(WKSTA_INFO_100, wki100_##name), r }
// @object PyWKSTA_INFO_100|A dictionary holding the infomation in a Win32 WKSTA_INFO_100 structure.
static struct PyNET_STRUCT_ITEM wki100[] = {
	WKI100_ENTRY(platform_id, NSI_DWORD, 0), // @prop int|platform_id|Indicates platform level to use to retrieve platform specific information
	WKI100_ENTRY(computername, NSI_WSTR, 0), // @prop string/<o PyUnicode>|computername|Name of the local computer
	WKI100_ENTRY(langroup, NSI_WSTR, 0), // @prop string/<o PyUnicode>|langroup|Name of the domain to which computer belongs
	WKI100_ENTRY(ver_major, NSI_DWORD, 0), // @prop int|ver_major|Major version number of operating system running on the computer
	WKI100_ENTRY(ver_minor, NSI_DWORD, 0), // @prop int|ver_minor|Minor version number of operating system running on the computer
	{NULL}
};

#define WKI101_ENTRY(name, t, r) { #name, t, offsetof(WKSTA_INFO_101, wki101_##name), r }
// @object PyWKSTA_INFO_101|A dictionary holding the infomation in a Win32 WKSTA_INFO_101 structure.
static struct PyNET_STRUCT_ITEM wki101[] = {
	WKI101_ENTRY(platform_id, NSI_DWORD, 0), // @prop int|platform_id|Indicates platform level to use to retrieve platform specific information
	WKI101_ENTRY(computername, NSI_WSTR, 0), // @prop string/<o PyUnicode>|computername|Name of the local computer
	WKI101_ENTRY(langroup, NSI_WSTR, 0), // @prop string/<o PyUnicode>|langroup|Name of the domain to which computer belongs
	WKI101_ENTRY(ver_major, NSI_DWORD, 0), // @prop int|ver_major|Major version number of operating system running on the computer
	WKI101_ENTRY(ver_minor, NSI_DWORD, 0), // @prop int|ver_minor|Minor version number of operating system running on the computer
	WKI101_ENTRY(lanroot, NSI_WSTR, 0), // @prop string/<o PyUnicode>|lanroot|Path to the LANMAN directory
	{NULL}
};
 
#define WKI102_ENTRY(name, t, r) { #name, t, offsetof(WKSTA_INFO_102, wki102_##name), r }
// @object PyWKSTA_INFO_102|A dictionary holding the infomation in a Win32 WKSTA_INFO_102 structure.
static struct PyNET_STRUCT_ITEM wki102[] = {
	WKI102_ENTRY(platform_id, NSI_DWORD, 0), // @prop int|platform_id|Indicate platform level to use to retrieve platform specific information
	WKI102_ENTRY(computername, NSI_WSTR, 0), // @prop string/<o PyUnicode>|computername|Name of the local computer
	WKI102_ENTRY(langroup, NSI_WSTR, 0), // @prop string/<o PyUnicode>|langroup|Name of the domain to which computer belongs
	WKI102_ENTRY(ver_major, NSI_DWORD, 0), // @prop int|ver_major|Major version number of operating system running on the computer
	WKI102_ENTRY(ver_minor, NSI_DWORD, 0), // @prop int|ver_minor|Minor version number of operating system running on the computer
	WKI102_ENTRY(lanroot, NSI_WSTR, 0), // @prop string/<o PyUnicode>|lanroot|Path to the LANMAN directory
	WKI102_ENTRY(logged_on_users, NSI_DWORD, 0), // @prop int|logged_on_users|Number of users who are logged on to the local computer
	{NULL}
};

#define WKI302_ENTRY(name, t, r) { #name, t, offsetof(WKSTA_INFO_302, wki302_##name), r }
// @object PyWKSTA_INFO_302|A dictionary holding the infomation in a Win32 WKSTA_INFO_302 structure.
static struct PyNET_STRUCT_ITEM wki302[] = {
	WKI302_ENTRY(char_wait, NSI_DWORD, 0), // @prop int|char_wait|number of seconds the computer will wait for a remote resource to become available
	WKI302_ENTRY(collection_time, NSI_DWORD, 0), // @prop int|collection_time|number of milliseconds the computer will collect data before sending the data to a character device resource. The workstation waits the specified time or collects the number of characters specified by wki302_maximum_collection_count, whichever comes first.
	WKI302_ENTRY(maximum_collection_count , NSI_DWORD, 0), // @prop int|maximum_collection_count|Specifies the number of bytes of information the computer will collect before sending the data to a character device resource. The workstation collects the specified number of bytes or waits the time specified by wki302_collection_time, whichever comes first.
	WKI302_ENTRY(keep_conn, NSI_DWORD, 0), // @prop int|keep_conn|Specifies the
										   // number of seconds the server will
										   // maintain an inactive connection
										   // to a resource.
	WKI302_ENTRY(keep_search, NSI_DWORD, 0), // @prop int|keep_search|Defines
											 // the number of seconds an
											 // inactive search will continue.
	WKI302_ENTRY(max_cmds, NSI_DWORD, 0), // @prop int|max_cmds|Specifies the number of simultaneous network device driver commands that can be sent to the network.
	WKI302_ENTRY(num_work_buf, NSI_DWORD, 0), // @prop int|num_work_buf|Specifies the number of internal buffers the computer has.
	WKI302_ENTRY(siz_work_buf, NSI_DWORD, 0), // @prop int|siz_work_buf|Specifies the size, in bytes, of each internal buffer.
	WKI302_ENTRY(max_wrk_cache, NSI_DWORD, 0), // @prop int|max_wrk_cache|Specifies the maximum size, in bytes, of an internal cache buffer.
    WKI302_ENTRY(sess_timeout, NSI_DWORD, 0), // @prop int|max_wrk_cache|Indicates the number of seconds the server waits before disconnecting an inactive session.
    WKI302_ENTRY(siz_error, NSI_DWORD, 0),    // @prop int|siz_error|Specifies the size, in bytes, of an internal error buffer.
    WKI302_ENTRY(num_alerts, NSI_DWORD, 0),	  // @prop int|num_alerts|Specifies the maximum number of clients that can receive alert messages. (This member is not supported under MS-DOS.) The Alerter service registers at least three clients when it begins to run.
    WKI302_ENTRY(num_services, NSI_DWORD, 0), // @prop int|num_services|Specifies the number of services that can be installed on the computer at any time.
    WKI302_ENTRY(errlog_sz, NSI_DWORD, 0), 	  // @prop int|errlog_sz|Specifies the maximum size, in kilobytes, of the client's error log file.
    WKI302_ENTRY(print_buf_time, NSI_DWORD, 0), // @prop int|print_buf_time|Specifies the number of seconds the server waits before closing inactive compatibility-mode print jobs.
    WKI302_ENTRY(num_char_buf, NSI_DWORD, 0),  // @prop int|num_char_buf|Specifies the number of character pipe buffers and device buffers the client can have.
    WKI302_ENTRY(siz_char_buf, NSI_DWORD, 0),   // @prop int|siz_char_buf|Specifies the maximum size, in bytes, of a character pipe buffer and device buffer.
    WKI302_ENTRY(wrk_heuristics, NSI_WSTR, 0), // @prop string/<o PyUnicode>|wrk_heuristics|Pointer to a Unicode string of flags used to control a client's operation.
    WKI302_ENTRY(mailslots, NSI_DWORD, 0), // @prop int|mailslots|Specifies the maximum number of mailslots allowed.
    WKI302_ENTRY(num_dgram_buf, NSI_DWORD, 0), // @prop int|num_dgram_buf|Specifies the number of buffers to allocate for receiving datagrams.
	{NULL}
};

#define WKI402_ENTRY(name, t, r) { #name, t, offsetof(WKSTA_INFO_402, wki402_##name), r }
// @object PyWKSTA_INFO_402|A dictionary holding the infomation in a Win32 WKSTA_INFO_402 structure.
static struct PyNET_STRUCT_ITEM wki402[] = {
	WKI402_ENTRY(char_wait, NSI_DWORD, 0), // @prop int|char_wait|number of seconds the computer will wait for a remote resource to become available
	WKI402_ENTRY(collection_time, NSI_DWORD, 0), // @prop int|collection_time|number of milliseconds the computer will collect data before sending the data to a character device resource. The workstation waits the specified time or collects the number of characters specified by wki402_maximum_collection_count, whichever comes first.
	WKI402_ENTRY(maximum_collection_count , NSI_DWORD, 0), // @prop string/<o PyUnicode>|maximum_collection_count|Name of the domain to which computer belongs
	WKI402_ENTRY(keep_conn, NSI_DWORD, 0), // @prop int|keep_conn|Major version number of operating system running on the computer
	WKI402_ENTRY(keep_search, NSI_DWORD, 0), // @prop int|keep_search|Minor version number of operating system running on the computer
	WKI402_ENTRY(max_cmds, NSI_DWORD, 0), // @prop int|max_cmds|.. 
	WKI402_ENTRY(num_work_buf, NSI_DWORD, 0), // @prop int|num_work_buf|Number of users who are logged on to the local computer
	WKI402_ENTRY(siz_work_buf, NSI_DWORD, 0), // @prop int|siz_work_buf|Number of users who are logged on to the local computer
	WKI402_ENTRY(max_wrk_cache, NSI_DWORD, 0), // @prop int|max_wrk_cache|.. 
    WKI402_ENTRY(sess_timeout, NSI_DWORD, 0), // @prop int|sess_timeout|.. 
    WKI402_ENTRY(siz_error, NSI_DWORD, 0),    // @prop int|siz_error|.. 
    WKI402_ENTRY(num_alerts, NSI_DWORD, 0),	  // @prop int|num_alerts|.. 
    WKI402_ENTRY(num_services, NSI_DWORD, 0), // @prop int|num_services|.. 
    WKI402_ENTRY(errlog_sz, NSI_DWORD, 0), 	  // @prop int|errlog_sz|.. 
    WKI402_ENTRY(print_buf_time, NSI_DWORD, 0), // @prop int|print_buf_time|.. 
    WKI402_ENTRY(num_char_buf, NSI_DWORD, 0),  // @prop int|num_char_buf|.. 
    WKI402_ENTRY(siz_char_buf, NSI_DWORD, 0), // @prop int|siz_char_buf|Specifies the maximum size, in bytes, of a character pipe buffer and device buffer.
    WKI402_ENTRY(wrk_heuristics, NSI_WSTR, 0), // @prop string/<o PyUnicode>|siz_char_buf|..
    WKI402_ENTRY(mailslots, NSI_DWORD, 0), // @prop int|mailslots|.. 
    WKI402_ENTRY(num_dgram_buf, NSI_DWORD, 0), // @prop int|num_dgram_buf|.. 
    WKI402_ENTRY(max_threads, NSI_DWORD, 0), // @prop int|max_threads|Number of threads the computer can dedicate to the network
	{NULL}
};

#define WKI502_ENTRY(name, t, r) { #name, t, offsetof(WKSTA_INFO_502, wki502_##name), r }
// @object PyWKSTA_INFO_502|A dictionary holding the infomation in a Win32 WKSTA_INFO_502 structure.
static struct PyNET_STRUCT_ITEM wki502[] = {
	WKI502_ENTRY(char_wait, NSI_DWORD, 0), // @prop int|char_wait|number of seconds the computer will wait for a remote resource to become available
	WKI502_ENTRY(collection_time, NSI_DWORD, 0), // @prop int|collection_time|number of milliseconds the computer will collect data before sending the data to a character device resource. The workstation waits the specified time or collects the number of characters specified by wki502_maximum_collection_count, whichever comes first.
	WKI502_ENTRY(maximum_collection_count , NSI_DWORD, 0), // @prop int|maximum_collection_count|Specifies the number of bytes of information the computer will collect before sending the data to a character device resource. The workstation collects the specified number of bytes or waits the time specified by wki302_collection_time, whichever comes first.
	WKI502_ENTRY(keep_conn, NSI_DWORD, 0), // @prop int|keep_conn|Specifies the
										   // number of seconds the server will
										   // maintain an inactive connection
										   // to a resource.
	WKI502_ENTRY(max_cmds, NSI_DWORD, 0), // @prop int|max_cmds|Specifies the number of simultaneous network device driver commands that can be sent to the network.
    WKI502_ENTRY(sess_timeout, NSI_DWORD, 0), // @prop int|max_wrk_cache|Indicates the number of seconds the server waits before disconnecting an inactive session.
    WKI502_ENTRY(siz_char_buf, NSI_DWORD, 0), // @prop int|siz_char_buf|Specifies the maximum size, in bytes, of a character pipe buffer and device buffer.

	WKI502_ENTRY(lock_quota, NSI_DWORD, 0), //@prop int|lock_quota|TODO
    WKI502_ENTRY(lock_increment, NSI_DWORD, 0), //@prop int|lock_increment|TODO
    WKI502_ENTRY(lock_maximum, NSI_DWORD, 0), //@prop int|lock_maximum|TODO
    WKI502_ENTRY(pipe_increment, NSI_DWORD, 0), //@prop int|pipe_increment|TODO
    WKI502_ENTRY(pipe_maximum, NSI_DWORD, 0), //@prop int|pipe_maximum|TODO
    WKI502_ENTRY(cache_file_timeout, NSI_DWORD, 0), //@prop int|cache_file_timeout|TODO
    WKI502_ENTRY(dormant_file_limit, NSI_DWORD, 0), //@prop int|dormant_file_limit|TODO
    WKI502_ENTRY(read_ahead_throughput, NSI_DWORD, 0), //@prop int|read_ahead_throughput|TODO
	WKI502_ENTRY(num_mailslot_buffers, NSI_DWORD, 0), // @prop int|num_mailslot_buffers|TODO
    WKI502_ENTRY(num_srv_announce_buffers, NSI_DWORD, 0), // @prop int|num_srv_announce_buffers|TODO
    WKI502_ENTRY(max_illegal_datagram_events, NSI_DWORD, 0), // @prop int|max_illegal_datagram_events|TODO
    WKI502_ENTRY(illegal_datagram_event_reset_frequency, NSI_DWORD, 0), // @prop int|illegal_datagram_event_reset_frequency|TODO
    WKI502_ENTRY(log_election_packets, NSI_BOOL, 0), // @prop bool|log_election_packets|TODO
    WKI502_ENTRY(use_opportunistic_locking, NSI_BOOL, 0), // @prop bool|use_opportunistic_locking|TODO
    WKI502_ENTRY(use_unlock_behind, NSI_BOOL, 0), // @prop bool|use_unlock_behind|TODO
    WKI502_ENTRY(use_close_behind, NSI_BOOL, 0), // @prop bool|use_close_behind|TODO
    WKI502_ENTRY(buf_named_pipes, NSI_BOOL, 0), // @prop bool|buf_named_pipes|TODO
    WKI502_ENTRY(use_lock_read_unlock, NSI_BOOL, 0), // @prop bool|use_lock_read_unlock|TODO
    WKI502_ENTRY(utilize_nt_caching, NSI_BOOL, 0), // @prop bool|utilize_nt_caching|TODO
    WKI502_ENTRY(use_raw_read, NSI_BOOL, 0), // @prop bool|use_raw_read|TODO
    WKI502_ENTRY(use_raw_write, NSI_BOOL, 0), // @prop bool|use_raw_write|TODO
    WKI502_ENTRY(use_write_raw_data, NSI_BOOL, 0), // @prop bool|use_write_raw_data|TODO
    WKI502_ENTRY(use_encryption, NSI_BOOL, 0), // @prop bool|use_encryption|TODO
    WKI502_ENTRY(buf_files_deny_write, NSI_BOOL, 0), // @prop bool|buf_files_deny_write|TODO
    WKI502_ENTRY(buf_read_only_files, NSI_BOOL, 0), // @prop bool|buf_read_only_files|TODO
    WKI502_ENTRY(force_core_create_mode, NSI_BOOL, 0), // @prop bool|force_core_create_mode|TODO
    WKI502_ENTRY(use_512_byte_max_transfer, NSI_BOOL, 0), // @prop bool|use_512_byte_max_transfer|TODO
	{NULL}
};

//{302, wki302, sizeof(WKSTA_INFO_302) }, // flag 302,|o PyWKSTA_INFO_302
//{402, wki402, sizeof(WKSTA_INFO_402) }, // flag 402,|o PyWKSTA_INFO_402
// NOTE: XXX:
// 		The documentation of NetWkstaGetInfo seems to be out-of-date. Info
// 		levels 302 and 402 return ERROR_INVALID_LEVEL (124). Levels 100, 101,
// 		102 and 502 return correct information as documented.
//
// @object PyWKSTA_INFO_*|The following WKSTA_INFO levels are supported.
static struct PyNET_STRUCT wksta_infos[] = { // @flagh Level|Data
    {100, wki100, sizeof(WKSTA_INFO_100) },   // @flag 100,|<o PyWKSTA_INFO_100>
    {101, wki101, sizeof(WKSTA_INFO_101) },   // @flag 101,|<o PyWKSTA_INFO_101>
    {102, wki102, sizeof(WKSTA_INFO_102) },   // @flag 102,|<o PyWKSTA_INFO_102>
    {502, wki502, sizeof(WKSTA_INFO_502) }, // @flag 502,|<o PyWKSTA_INFO_502>
    {0, NULL, 0}
};

#define WKUI0_ENTRY(name, t, r) { #name, t, offsetof(WKSTA_USER_INFO_0, wkui0_##name), r }
// @object PyWKSTA_USER_INFO_0|A dictionary holding the infomation in a Win32 WKSTA_USER_INFO_0 structure.
static struct PyNET_STRUCT_ITEM wkui0[] = {
	WKUI0_ENTRY(username, NSI_WSTR, 0),  // @prop string/<o PyUnicode>|username|Name of user currently logged on to the workstation
	{NULL}
};

#define WKUI1_ENTRY(name, t, r) { #name, t, offsetof(WKSTA_USER_INFO_1, wkui1_##name), r }
// @object PyWKSTA_USER_INFO_1|A dictionary holding the infomation in a Win32 WKSTA_USER_INFO_1 structure.
static struct PyNET_STRUCT_ITEM wkui1[] = {
	WKUI1_ENTRY(username, NSI_WSTR, 0),  // @prop string/<o PyUnicode>|username|Name of user currently logged on to the workstation
    WKUI1_ENTRY(logon_domain, NSI_WSTR, 0), // @prop string/<o PyUnicode>|logon_domain|Returns the domain name of the user account of the user currently logged on to the workstation.
    WKUI1_ENTRY(oth_domains, NSI_WSTR, 0), // @prop string/<o PyUnicode>|oth_domains|Returns the list of other operating system domains browsed by the workstation. The domain names are separated by blanks.
    WKUI1_ENTRY(logon_server, NSI_WSTR, 0), // @prop string/<o PyUnicode>|logon_server|Returns the name of the computer that authenticated the server.
	{NULL}
};

// @object PyWKSTA_USER_INFO_*|The following WKSTA_USER_INFO levels are supported.
static struct PyNET_STRUCT wktau_infos[] = { // @flagh Level|Data
	{0, wkui0, sizeof(WKSTA_USER_INFO_0)},   // @flag 0,| <o PyWKSTA_USER_INFO_0>
	{1, wkui1, sizeof(WKSTA_USER_INFO_1)},	 // @flag 1,| <o PyWKSTA_USER_INFO_1>
	{0, NULL, 0}
};

#define WKTI0_ENTRY(name, t, r) { #name, t, offsetof(WKSTA_TRANSPORT_INFO_0, wkti0_##name), r }
// @object PyWKSTA_TRANSPORT_INFO_0|A dictionary holding the infomation in a Win32 WKSTA_TRANSPORT_INFO_0 structure.
static struct PyNET_STRUCT_ITEM wkti0[] = {
	WKTI0_ENTRY(quality_of_service, NSI_DWORD, 0),  // @prop int|quality_of_service|Supplies a value that specifies the search order of the transport protocol with respect to other transport protocols. The highest value is searched first.
    WKTI0_ENTRY(number_of_vcs, NSI_DWORD, 0), // @prop int|number_of_vcs|Specifies the number of clients communicating with the server using this transport protocol.
    WKTI0_ENTRY(transport_name, NSI_WSTR, 0), // @prop string/<o PyUnicode>|transport_name|Specifies the device name of the transport protocol.
    WKTI0_ENTRY(transport_address, NSI_WSTR, 0), // @prop string/<o PyUnicode>|transport_address|Specifies the address of the server on this transport protocol.
    WKTI0_ENTRY(wan_ish, NSI_BOOL, 0), // @prop bool|wan_ish|This member is ignored by the NetWkstaTransportAdd function. For the NetWkstaTransportEnum function, this member indicates that this transport protocol is a WAN transport protocol. This member is set TRUE for NetBIOS/TCIP; it is set FALSE for NetBEUI and NetBIOS/IPX.
	{NULL}
};

// @object PyWKSTA_TRANSPORT_INFO_*|The following WKSTA_TRANSPORT_INFO levels are supported.
static struct PyNET_STRUCT wkstransport_infos[] = { // @flagh Level|Data
	{0, wkti0, sizeof(WKSTA_TRANSPORT_INFO_0)},   // @flag 0,| <o PyWKSTA_TRANSPORT_INFO_0>
	{0, NULL, 0}
};


/**************************************************************************************************************
**   PyNetShareEnum1
**
**************************************************************************************************************/
// Old style before we got more flexible info levels.
static PyObject *PyNetShareEnum1(WCHAR *szServerName)
{
	DWORD dwLevel = 1;
	DWORD dwMaxLen = MAX_PREFERRED_LENGTH;
	NET_API_STATUS Errno;
	DWORD dwCount, dwMaxCount, dwResume = 0;
	SHARE_INFO_1 *lpBuffer;

	PyObject * pRetlist = PyList_New(0);	//create a return list of 0 size
	if (pRetlist==NULL) return NULL; // did we err?

	do
	{
		Py_BEGIN_ALLOW_THREADS
		Errno = NetShareEnum(szServerName,dwLevel,(LPBYTE *)&lpBuffer,dwMaxLen,&dwCount,&dwMaxCount,&dwResume);
		Py_END_ALLOW_THREADS

		if(Errno == NERR_Success)
		{

		SHARE_INFO_1 *p_nr = lpBuffer;

		if (dwCount > 0)	// we actually got something
		{
			dwMaxCount = dwMaxCount - dwCount;	// how many more we will try to get
			do
			{
				PyObject *t_ob = Py_BuildValue("(NiN)",
				                  PyWinObject_FromWCHAR(p_nr->shi1_netname),
				                  p_nr->shi1_type,
				                  PyWinObject_FromWCHAR(p_nr->shi1_remark));

				int listerr = PyList_Append(pRetlist,t_ob);	// append our PyNETRESOURCE obj...Append does an INCREF!

				Py_DECREF(t_ob);

				if (listerr)	// or bail
				{
					Py_DECREF(pRetlist);
					NetApiBufferFree((LPVOID)lpBuffer);
					return NULL;
				}

				p_nr++;	// next object (its a ++ because it is a typed pointer)
				dwCount--;
			} while (dwCount);  
		}; // if dwCount
		} // if Errno == NERR_Sucess
		else
		{
			Py_DECREF(pRetlist);
			return (ReturnNetError("NetShareEnum",Errno));
		}

	} while(dwMaxCount != 0);

	NetApiBufferFree((LPVOID)lpBuffer);
	return pRetlist;
}

// @pymethod ([dict, ...], total, resumeHandle)|win32net|NetShareEnum|Retrieves information about each shared resource on a server. 
// @rdesc The result is a list of items read (with each item being a dictionary of format
// <o PySHARE_INFO_*>, depending on the level parameter),
// the total available, and a new "resume handle".  The first time you call
// this function, you should pass zero for the resume handle.  If more data
// is available than what was returned, a new non-zero resume handle will be
// returned, which can be used to call the function again to fetch more data.
// This process may repeat, each time with a new resume handle, until zero is
// returned for the new handle, indicating all the data has been read.
PyObject *
PyNetShareEnum(PyObject *self, PyObject *args)
{
	PyObject *obServerName;
	// @pyparmalt1 string|serverName||The name of the server on which the call should execute, or None for the local computer.
	// @comm If the old style is used, the result is a list of [(shareName, type, remarks), ...]
	if (PyArg_ParseTuple(args, "O:NetShareEnum",&obServerName)) {
		WCHAR *wServerName;
		if (!PyWinObject_AsWCHAR(obServerName, &wServerName, TRUE))
			return NULL;
		PyObject *ret = PyNetShareEnum1(wServerName);
		PyWinObject_FreeWCHAR(wServerName);
		return ret;
	}
	PyErr_Clear();
	// Use new style
	// @pyparm string/<o PyUnicode>|server||The name of the server, or None.
	// @pyparm int|level||The level of data required.
	// @pyparm int|resumeHandle|0|A resume handle.  See the return description for more information.
	// @pyparm int|prefLen|MAX_PREFERRED_LENGTH|The preferred length of the data buffer.
	// @pyseeapi NetShareEnum
	// param 1 is not declared as const :-(
	PFNSIMPLEENUM pfn = (PFNSIMPLEENUM)&NetShareEnum;
	return PyDoSimpleEnum(self, args, pfn, "NetShareEnum", share_infos);
}

// @pymethod dict|win32net|NetShareGetInfo|Retrieves information about a particular share on a server.
PyObject *PyNetShareGetInfo(PyObject *self, PyObject *args) 
{
	PFNGETINFO pfn = (PFNGETINFO)&NetShareGetInfo;
	return PyDoGetInfo(self, args, pfn, "NetShareGetInfo", share_infos);
	// @pyparm string/<o PyUnicode>|server||The name of the server, or None.
	// @pyparm string/<o PyUnicode>|netname||The network name
	// @pyparm int|level||The information level contained in the data
	// @pyseeapi NetShareGetInfo
	// @rdesc The result will be a dictionary in one of the <o PySHARE_INFO_*>
	// formats, depending on the level parameter.
}

// @pymethod |win32net|NetShareSetInfo|Sets information about a particular share on a server.
PyObject *PyNetShareSetInfo(PyObject *self, PyObject *args)
{
	PFNSETINFO pfn = (PFNSETINFO)&NetShareSetInfo;
	return PyDoSetInfo(self, args, pfn, "NetShareSetInfo", share_infos);
	// @pyparm string/<o PyUnicode>|server||The name of the server, or None.
	// @pyparm string/<o PyUnicode>|netname||The network name
	// @pyparm int|level||The information level contained in the data
	// @pyparm mapping|data||A dictionary holding the share data.
	// @pyseeapi NetShareSetInfo
}

// @pymethod |win32net|NetShareAdd|Creates a new share.
PyObject *PyNetShareAdd(PyObject *self, PyObject *args) 
{
	// @pyparm string/<o PyUnicode>|server||The name of the server, or None.
	// @pyparm int|level||The information level contained in the data.  Must be level 2 or 502.
	// @pyparm mapping|data||A dictionary holding the share data, in the format of <o SHARE_INFO_*>
	PFNADD pfn = (PFNADD)&NetShareAdd;
	return PyDoAdd(self, args, pfn, "NetShareAdd", share_infos);
	// @pyseeapi NetShareAdd
}

// @pymethod |win32net|NetShareDel|Deletes a share
PyObject *PyNetShareDel(PyObject *self, PyObject *args) 
{
	// @pyparm string/<o PyUnicode>|server||The name of the server, or None.
	// @pyparm string/<o PyUnicode>|shareName||The share name
	// @pyparm int|reserved|0|Must be zero.
	WCHAR *szServer = NULL;
	WCHAR *szName = NULL;
	PyObject *obName, *obServer;
	PyObject *ret = NULL;
	DWORD reserved = 0;
	DWORD err = 0;
	if (!PyArg_ParseTuple(args, "OO|l", &obServer, &obName, &reserved))
		return NULL;
	if (!PyWinObject_AsWCHAR(obServer, &szServer, TRUE))
		goto done;
	if (!PyWinObject_AsWCHAR(obName, &szName, FALSE))
		goto done;

	Py_BEGIN_ALLOW_THREADS
	err = NetShareDel(szServer, szName, reserved);
	Py_END_ALLOW_THREADS
	if (err) {
		ReturnNetError("NetShareDel",err);	
		goto done;
	}
	ret = Py_None;
	Py_INCREF(Py_None);
done:
	PyWinObject_FreeWCHAR(szServer);
	PyWinObject_FreeWCHAR(szName);
	return ret;
	// @pyseeapi NetShareDel
}

// @pymethod (ret, type)|win32net|NetShareCheck|Checks if server is sharing a device
// @rdesc The result is (1, type-of-device) if device is shared, (0, None) if it is not shared.
PyObject *
PyNetShareCheck(PyObject *self, PyObject *args)
{
	// @pyparm string/<o PyUnicode>|server||The name of the server, or None.
	// @pyparm string/<o PyUnicode>|deviceName||The share name
	WCHAR *szServer = NULL;
	WCHAR *deviceName = NULL;
	PyObject *obName, *obServer;
	PyObject *ret = NULL;
	DWORD err = 0, type;
	if (!PyArg_ParseTuple(args, "OO", &obServer, &obName))
		return NULL;
	if (!PyWinObject_AsWCHAR(obServer, &szServer, TRUE))
		goto done;
	if (!PyWinObject_AsWCHAR(obName, &deviceName, FALSE))
		goto done;

	Py_BEGIN_ALLOW_THREADS
	err = NetShareCheck(szServer, deviceName, &type);
	Py_END_ALLOW_THREADS
	if (err) {
		if (err == NERR_DeviceNotShared) {
			ret = Py_BuildValue("(iO)", 0, Py_None);
		} else {
			ReturnNetError("NetShareCheck",err);
		}
		goto done;
	}
	ret = Py_BuildValue("(ii)", 1, type);
	//Py_INCREF(Py_None);
done:
	PyWinObject_FreeWCHAR(szServer);
	PyWinObject_FreeWCHAR(deviceName);
	return ret;
	// @pyseeapi NetShareCheck
}

#define SV100_ENTRY(name, t, r) { #name, t, offsetof(SERVER_INFO_100, sv100_##name), r }
// @object PySERVER_INFO_100|A dictionary holding the information in a Win32 SERVER_INFO_100 structure.
static struct PyNET_STRUCT_ITEM sv100[] = {
	SV100_ENTRY(platform_id, NSI_DWORD, 0), // @prop int|platform_id|
	SV100_ENTRY(name, NSI_WSTR, 0), // @prop string/<o PyUnicode>|name|
	{NULL}
};
#define SV101_ENTRY(name, t, r) { #name, t, offsetof(SERVER_INFO_101, sv101_##name), r }
// @object PySERVER_INFO_101|A dictionary holding the information in a Win32 SERVER_INFO_101 structure.
static struct PyNET_STRUCT_ITEM sv101[] = {
	SV101_ENTRY(platform_id, NSI_DWORD, 0), // @prop int|platform_id|
	SV101_ENTRY(name, NSI_WSTR, 0), // @prop string/<o PyUnicode>|name|
	SV101_ENTRY(version_major, NSI_DWORD, 0), // @prop int|version_major|
	SV101_ENTRY(version_minor, NSI_DWORD, 0), // @prop int|version_minor|
	SV101_ENTRY(type, NSI_DWORD, 0), // @prop int|type|one of the SV_TYPE_* constants
	SV101_ENTRY(comment, NSI_WSTR, 0), // @prop string/<o PyUnicode>|comment|
	{NULL}
};

#define SV102_ENTRY(name, t, r) { #name, t, offsetof(SERVER_INFO_102, sv102_##name), r }
// @object PySERVER_INFO_102|A dictionary holding the information in a Win32 SERVER_INFO_102 structure.
static struct PyNET_STRUCT_ITEM sv102[] = {
	SV102_ENTRY(platform_id, NSI_DWORD, 0), // @prop int|platform_id|
	SV102_ENTRY(name, NSI_WSTR, 0), // @prop string/<o PyUnicode>|name|
	SV102_ENTRY(version_major, NSI_DWORD, 0), // @prop int|version_major|
	SV102_ENTRY(version_minor, NSI_DWORD, 0), // @prop int|version_minor|
	SV102_ENTRY(type, NSI_DWORD, 0), // @prop int|type|one of the SV_TYPE_* constants
	SV102_ENTRY(comment, NSI_WSTR, 0), // @prop string/<o PyUnicode>|comment|
	SV102_ENTRY(users, NSI_DWORD, 0), // @prop int|users|
	SV102_ENTRY(disc, NSI_LONG, 0), // @prop int|disc|
	SV102_ENTRY(hidden, NSI_BOOL, 0), // @prop bool|hidden|
	SV102_ENTRY(announce, NSI_DWORD, 0), // @prop int|announce|
	SV102_ENTRY(anndelta, NSI_DWORD, 0), // @prop int|anndelta|
	SV102_ENTRY(userpath, NSI_WSTR, 0), // @prop string/<o PyUnicode>|userpath|
	{NULL}
};

#define SV402_ENTRY(name, t, r) { #name, t, offsetof(SERVER_INFO_402, sv402_##name), r }
// @object PySERVER_INFO_402|A dictionary holding the information in a Win32 SERVER_INFO_402 structure.
static struct PyNET_STRUCT_ITEM sv402[] = {
	SV402_ENTRY(ulist_mtime, NSI_DWORD, 0), // @prop int|ulist_mtime|
	SV402_ENTRY(glist_mtime, NSI_DWORD, 0), // @prop int|glist_mtime|
	SV402_ENTRY(alist_mtime, NSI_DWORD, 0), // @prop int|alist_mtime|
	SV402_ENTRY(security, NSI_DWORD, 0), // @prop int|security|
	SV402_ENTRY(numadmin, NSI_DWORD, 0), // @prop int|numadmin|
	SV402_ENTRY(lanmask, NSI_DWORD, 0), // @prop int|lanmask|
	SV402_ENTRY(guestacct, NSI_WSTR, 0), // @prop string/<o PyUnicode>|guestacct|
	SV402_ENTRY(chdevs, NSI_DWORD, 0), // @prop int|chdevs|
	SV402_ENTRY(chdevq, NSI_DWORD, 0), // @prop int|chdevq|
	SV402_ENTRY(chdevjobs, NSI_DWORD, 0), // @prop int|chdevjobs|
	SV402_ENTRY(connections, NSI_DWORD, 0), // @prop int|connections|
	SV402_ENTRY(shares, NSI_DWORD, 0), // @prop int|shares|
	SV402_ENTRY(openfiles, NSI_DWORD, 0), // @prop int|openfiles|
	SV402_ENTRY(sessopens, NSI_DWORD, 0), // @prop int|sessopens|
	SV402_ENTRY(sessvcs, NSI_DWORD, 0), // @prop int|sessvcs|
	SV402_ENTRY(sessreqs, NSI_DWORD, 0), // @prop int|sessreqs|
	SV402_ENTRY(opensearch, NSI_DWORD, 0), // @prop int|opensearch|
	SV402_ENTRY(activelocks, NSI_DWORD, 0), // @prop int|activelocks|
	SV402_ENTRY(numreqbuf, NSI_DWORD, 0), // @prop int|numreqbuf|
	SV402_ENTRY(sizreqbuf, NSI_DWORD, 0), // @prop int|sizreqbuf|
	SV402_ENTRY(numbigbuf, NSI_DWORD, 0), // @prop int|numbigbuf|
	SV402_ENTRY(numfiletasks, NSI_DWORD, 0), // @prop int|numfiletasks|
	SV402_ENTRY(alertsched, NSI_DWORD, 0), // @prop int|alertsched|
	SV402_ENTRY(erroralert, NSI_DWORD, 0), // @prop int|erroralert|
	SV402_ENTRY(logonalert, NSI_DWORD, 0), // @prop int|logonalert|
	SV402_ENTRY(accessalert, NSI_DWORD, 0), // @prop int|accessalert|
	SV402_ENTRY(diskalert, NSI_DWORD, 0), // @prop int|diskalert|
	SV402_ENTRY(netioalert, NSI_DWORD, 0), // @prop int|netioalert|
	SV402_ENTRY(maxauditsz, NSI_DWORD, 0), // @prop int|maxauditsz|
	SV402_ENTRY(srvheuristics, NSI_WSTR, 0), // @prop string/<o PyUnicode>|srvheuristics|
	{NULL}
};


#define SV403_ENTRY(name, t, r) { #name, t, offsetof(SERVER_INFO_403, sv403_##name), r }
// @object PySERVER_INFO_403|A dictionary holding the information in a Win32 SERVER_INFO_403 structure.
static struct PyNET_STRUCT_ITEM sv403[] = {
	SV403_ENTRY(ulist_mtime, NSI_DWORD, 0), // @prop int|ulist_mtime|
	SV403_ENTRY(glist_mtime, NSI_DWORD, 0), // @prop int|glist_mtime|
	SV403_ENTRY(alist_mtime, NSI_DWORD, 0), // @prop int|alist_mtime|
	SV403_ENTRY(security, NSI_DWORD, 0), // @prop int|security|
	SV403_ENTRY(numadmin, NSI_DWORD, 0), // @prop int|numadmin|
	SV403_ENTRY(lanmask, NSI_DWORD, 0), // @prop int|lanmask|
	SV403_ENTRY(guestacct, NSI_WSTR, 0), // @prop string/<o PyUnicode>|guestacct|
	SV403_ENTRY(chdevs, NSI_DWORD, 0), // @prop int|chdevs|
	SV403_ENTRY(chdevq, NSI_DWORD, 0), // @prop int|chdevq|
	SV403_ENTRY(chdevjobs, NSI_DWORD, 0), // @prop int|chdevjobs|
	SV403_ENTRY(connections, NSI_DWORD, 0), // @prop int|connections|
	SV403_ENTRY(shares, NSI_DWORD, 0), // @prop int|shares|
	SV403_ENTRY(openfiles, NSI_DWORD, 0), // @prop int|openfiles|
	SV403_ENTRY(sessopens, NSI_DWORD, 0), // @prop int|sessopens|
	SV403_ENTRY(sessvcs, NSI_DWORD, 0), // @prop int|sessvcs|
	SV403_ENTRY(sessreqs, NSI_DWORD, 0), // @prop int|sessreqs|
	SV403_ENTRY(opensearch, NSI_DWORD, 0), // @prop int|opensearch|
	SV403_ENTRY(activelocks, NSI_DWORD, 0), // @prop int|activelocks|
	SV403_ENTRY(numreqbuf, NSI_DWORD, 0), // @prop int|numreqbuf|
	SV403_ENTRY(sizreqbuf, NSI_DWORD, 0), // @prop int|sizreqbuf|
	SV403_ENTRY(numbigbuf, NSI_DWORD, 0), // @prop int|numbigbuf|
	SV403_ENTRY(numfiletasks, NSI_DWORD, 0), // @prop int|numfiletasks|
	SV403_ENTRY(alertsched, NSI_DWORD, 0), // @prop int|alertsched|
	SV403_ENTRY(erroralert, NSI_DWORD, 0), // @prop int|erroralert|
	SV403_ENTRY(logonalert, NSI_DWORD, 0), // @prop int|logonalert|
	SV403_ENTRY(accessalert, NSI_DWORD, 0), // @prop int|accessalert|
	SV403_ENTRY(diskalert, NSI_DWORD, 0), // @prop int|diskalert|
	SV403_ENTRY(netioalert, NSI_DWORD, 0), // @prop int|netioalert|
	SV403_ENTRY(maxauditsz, NSI_DWORD, 0), // @prop int|maxauditsz|
	SV403_ENTRY(srvheuristics, NSI_WSTR, 0), // @prop string/<o PyUnicode>|srvheuristics|
	SV403_ENTRY(auditedevents, NSI_DWORD, 0), // @prop int|auditedevents|
	SV403_ENTRY(autoprofile, NSI_DWORD, 0), // @prop int|autoprofile|
	SV403_ENTRY(autopath, NSI_WSTR, 0), // @prop string/<o PyUnicode>|autopath|
	{NULL}
};

#define SV502_ENTRY(name, t) { #name, t, offsetof(SERVER_INFO_502, sv502_##name), 0 }
// @object PySERVER_INFO_502|A dictionary holding the information in a Win32 SERVER_INFO_502 structure.
static struct PyNET_STRUCT_ITEM sv502[] = {
    SV502_ENTRY(sessopens, NSI_DWORD), // @prop int|sessopens|
    SV502_ENTRY(sessvcs, NSI_DWORD), // @prop int|sessvcs|
    SV502_ENTRY(opensearch, NSI_DWORD), // @prop int|opensearch|
    SV502_ENTRY(sizreqbuf, NSI_DWORD), // @prop int|sizreqbuf|
    SV502_ENTRY(initworkitems, NSI_DWORD), // @prop int|initworkitems|
    SV502_ENTRY(maxworkitems, NSI_DWORD), // @prop int|maxworkitems|
    SV502_ENTRY(rawworkitems, NSI_DWORD), // @prop int|rawworkitems|
    SV502_ENTRY(irpstacksize, NSI_DWORD), // @prop int|irpstacksize|
    SV502_ENTRY(maxrawbuflen, NSI_DWORD), // @prop int|maxrawbuflen|
    SV502_ENTRY(sessusers, NSI_DWORD), // @prop int|sessusers|
    SV502_ENTRY(sessconns, NSI_DWORD), // @prop int|sessconns|
    SV502_ENTRY(maxpagedmemoryusage, NSI_DWORD), // @prop int|maxpagedmemoryusage|
    SV502_ENTRY(maxnonpagedmemoryusage, NSI_DWORD), // @prop int|maxnonpagedmemoryusage|
    SV502_ENTRY(enableforcedlogoff, NSI_BOOL), // @prop bool|enableforcedlogoff|
    SV502_ENTRY(timesource, NSI_BOOL), // @prop bool|timesource|
    SV502_ENTRY(acceptdownlevelapis, NSI_BOOL), // @prop bool|acceptdownlevelapis|
    SV502_ENTRY(lmannounce, NSI_BOOL), // @prop bool|lmannounce|
	{NULL}
};

#define SV503_ENTRY(name, t) { #name, t, offsetof(SERVER_INFO_503, sv503_##name), 0 }
// @object PySERVER_INFO_503|A dictionary holding the information in a Win32 SERVER_INFO_503 structure.
static struct PyNET_STRUCT_ITEM sv503[] = {
    SV503_ENTRY(sessopens, NSI_DWORD), // @prop int|sessopens|
    SV503_ENTRY(sessvcs, NSI_DWORD), // @prop int|sessvcs|
    SV503_ENTRY(opensearch, NSI_DWORD), // @prop int|opensearch|
    SV503_ENTRY(sizreqbuf, NSI_DWORD), // @prop int|sizreqbuf|
    SV503_ENTRY(initworkitems, NSI_DWORD), // @prop int|initworkitems|
    SV503_ENTRY(maxworkitems, NSI_DWORD), // @prop int|maxworkitems|
    SV503_ENTRY(rawworkitems, NSI_DWORD), // @prop int|rawworkitems|
    SV503_ENTRY(irpstacksize, NSI_DWORD), // @prop int|irpstacksize|
    SV503_ENTRY(maxrawbuflen, NSI_DWORD), // @prop int|maxrawbuflen|
    SV503_ENTRY(sessusers, NSI_DWORD), // @prop int|sessusers|
    SV503_ENTRY(sessconns, NSI_DWORD), // @prop int|sessconns|
    SV503_ENTRY(maxpagedmemoryusage, NSI_DWORD), // @prop int|maxpagedmemoryusage|
    SV503_ENTRY(maxnonpagedmemoryusage, NSI_DWORD), // @prop int|maxnonpagedmemoryusage|
    SV503_ENTRY(enableforcedlogoff, NSI_BOOL), // @prop bool|enableforcedlogoff|
    SV503_ENTRY(timesource, NSI_BOOL), // @prop bool|timesource|
    SV503_ENTRY(acceptdownlevelapis, NSI_BOOL), // @prop bool|acceptdownlevelapis|
    SV503_ENTRY(lmannounce, NSI_BOOL), // @prop bool|lmannounce|
    SV503_ENTRY(domain, NSI_WSTR), // @prop string/<o PyUnicode>|domain|
    SV503_ENTRY(maxkeepsearch, NSI_DWORD), // @prop int|maxkeepsearch|
    SV503_ENTRY(scavtimeout, NSI_DWORD), // @prop int|scavtimeout|
    SV503_ENTRY(minrcvqueue, NSI_DWORD), // @prop int|minrcvqueue|
    SV503_ENTRY(minfreeworkitems, NSI_DWORD), // @prop int|minfreeworkitems|
    SV503_ENTRY(xactmemsize, NSI_DWORD), // @prop int|xactmemsize|
    SV503_ENTRY(threadpriority, NSI_DWORD), // @prop int|threadpriority|
    SV503_ENTRY(maxmpxct, NSI_DWORD), // @prop int|maxmpxct|
    SV503_ENTRY(oplockbreakwait, NSI_DWORD), // @prop int|oplockbreakwait|
    SV503_ENTRY(oplockbreakresponsewait, NSI_DWORD), // @prop int|oplockbreakresponsewait|
    SV503_ENTRY(enableoplocks, NSI_BOOL), // @prop bool|enableoplocks|
    SV503_ENTRY(enablefcbopens, NSI_BOOL), // @prop bool|enablefcbopens|
    SV503_ENTRY(enableraw, NSI_BOOL), // @prop bool|enableraw|
    SV503_ENTRY(enablesharednetdrives, NSI_BOOL), // @prop bool|enablesharednetdrives|
    SV503_ENTRY(minfreeconnections, NSI_DWORD), // @prop int|minfreeconnections|
    SV503_ENTRY(maxfreeconnections, NSI_DWORD), // @prop int|maxfreeconnections|
	{NULL}
};

// @object PySERVER_INFO_*|The following SERVER_INFO levels are supported.
static struct PyNET_STRUCT server_infos[] = { // @flagh Level|Data
	{ 100, sv100, sizeof(SERVER_INFO_100) },        // @flag 100|<o PySERVER_INFO_100>
	{ 101, sv101, sizeof(SERVER_INFO_101) },        // @flag 101|<o PySERVER_INFO_101>
	{ 102, sv102, sizeof(SERVER_INFO_102) },        // @flag 102|<o PySERVER_INFO_102>
	{ 402, sv402, sizeof(SERVER_INFO_402) },        // @flag 402|<o PySERVER_INFO_402>
	{ 403, sv403, sizeof(SERVER_INFO_403) },        // @flag 403|<o PySERVER_INFO_403>
	{ 502, sv502, sizeof(SERVER_INFO_502) },        // @flag 502|<o PySERVER_INFO_502>
	{ 503, sv503, sizeof(SERVER_INFO_503) },        // @flag 503|<o PySERVER_INFO_503>
	{0, NULL, 0}
};

// @pymethod ([dict, ...], total, resumeHandle)|win32net|NetServerEnum|Retrieves information about each server of a particular type
// @rdesc The result is a list of items read (with each item being a dictionary of format
// <o PySERVER_INFO_*>, depending on the level parameter),
// the total available, and a new "resume handle".  The first time you call
// this function, you should pass zero for the resume handle.  If more data
// is available than what was returned, a new non-zero resume handle will be
// returned, which can be used to call the function again to fetch more data.
// This process may repeat, each time with a new resume handle, until zero is
// returned for the new handle, indicating all the data has been read.
PyObject *
PyNetServerEnum(PyObject *self, PyObject *args)
{
	WCHAR *szServer = NULL, *szDomain = NULL;
	PyObject *obServer, *obDomain = Py_None;
	int serverType = SV_TYPE_ALL;
	PyObject *ret = NULL;
	PyNET_STRUCT *pInfo;
	DWORD err;
	DWORD dwPrefLen = MAX_PREFERRED_LENGTH;
	DWORD level;
	BOOL ok = FALSE;
	DWORD resumeHandle = 0;
	DWORD numRead, i;
	PyObject *list;
	BYTE *buf = NULL;
	DWORD totalEntries = 0;
	// @pyparm string/<o PyUnicode>|server||The name of the server to execute on, or None.
	// @pyparm int|level||The level of data required.
	// @pyparm int|type|SV_TYPE_ALL|Type of server to return - one of the SV_TYPE_* constants.
	// @pyparm string/<o PyUnicode>|domain|None|The domain to enumerate, or None for the current domain.
	// @pyparm int|resumeHandle|0|A resume handle.  See the return description for more information.
	// @pyparm int|prefLen|MAX_PREFERRED_LENGTH|The preferred length of the data buffer.
	if (!PyArg_ParseTuple(args, "Oi|iOii", &obServer, &level, &serverType, &obDomain, &resumeHandle, &dwPrefLen))
		return NULL;
	if (!PyWinObject_AsWCHAR(obServer, &szServer, TRUE))
		goto done;
	if (!PyWinObject_AsWCHAR(obDomain, &szDomain, TRUE))
		goto done;

	if (!FindNET_STRUCT(level, server_infos, &pInfo))
		goto done;

	Py_BEGIN_ALLOW_THREADS
	err = NetServerEnum(szServer, level, &buf, dwPrefLen, &numRead, &totalEntries, serverType, szDomain, &resumeHandle);
	Py_END_ALLOW_THREADS
	if (err!=0 && err != ERROR_MORE_DATA) {
		ReturnNetError("NetServerEnum",err);
		goto done;
	}
	list = PyList_New(numRead);
	if (list==NULL) goto done;
	for (i=0;i<numRead;i++) {
		PyObject *sub = PyObject_FromNET_STRUCT(pInfo, buf+(i*pInfo->structsize));
		if (sub==NULL) goto done;
		PyList_SetItem(list, i, sub);
	}
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
	PyWinObject_FreeWCHAR(szDomain);
	return ret;
	// @pyseeapi NetServerEnum
}

// @pymethod dict|win32net|NetServerGetInfo|Retrieves information about a particular server.
PyObject *
PyNetServerGetInfo(PyObject *self, PyObject *args) 
{
	WCHAR *szServer = NULL;
	PyObject *obServer;
	PyNET_STRUCT *pInfo;
	BYTE *buf = NULL;
	PyObject *ret = NULL;
	int typ;
	DWORD err;
	// @pyparm string/<o PyUnicode>|server||The name of the server to execute on, or None.
	// @pyparm int|level||The information level contained in the data
	if (!PyArg_ParseTuple(args, "Oi", &obServer, &typ))
		return NULL;
	if (!PyWinObject_AsWCHAR(obServer, &szServer, TRUE))
		goto done;
	if (!FindNET_STRUCT(typ, server_infos, &pInfo))
		goto done;
    Py_BEGIN_ALLOW_THREADS
	err = NetServerGetInfo(szServer, typ, &buf);
    Py_END_ALLOW_THREADS
	if (err) {
		ReturnNetError("NetServerGetInfo",err);
		goto done;
	}
	ret= PyObject_FromNET_STRUCT(pInfo, buf);
done:
	if (buf) NetApiBufferFree(buf);
	PyWinObject_FreeWCHAR(szServer);
	return ret;
	// @pyseeapi NetServerGetInfo
	// @rdesc The result will be a dictionary in one of the <o PySERVER_INFO_*>
	// formats, depending on the level parameter.
}

// @pymethod |win32net|NetServerSetInfo|Sets information about a particular server.
PyObject *
PyNetServerSetInfo(PyObject *self, PyObject *args)
{
	WCHAR *szServer = NULL;
	PyObject *obServer, *obData;
	PyNET_STRUCT *pInfo;
	BYTE *buf = NULL;
	PyObject *ret = NULL;
	int typ;
	DWORD err = 0;
	// @pyparm string/<o PyUnicode>|server||The name of the server, or None.
	// @pyparm int|level||The information level contained in the data
	// @pyparm mapping|data||A dictionary holding the share data.
	if (!PyArg_ParseTuple(args, "OiO", &obServer, &typ, &obData))
		return NULL;
	if (!PyWinObject_AsWCHAR(obServer, &szServer, TRUE))
		goto done;

	if (!FindNET_STRUCT(typ, server_infos, &pInfo))
		goto done;

	if (!PyObject_AsNET_STRUCT(obData, pInfo, &buf))
		goto done;

	Py_BEGIN_ALLOW_THREADS
	err = NetServerSetInfo(szServer, typ, buf, NULL);
	Py_END_ALLOW_THREADS
	if (err) {
		ReturnNetError("NetServerSetInfo",err);	
		goto done;
	}
	ret= Py_None;
	Py_INCREF(ret);
done:
	if (buf) PyObject_FreeNET_STRUCT(pInfo, buf);
	PyWinObject_FreeWCHAR(szServer);
	return ret;
	// @pyseeapi NetServerSetInfo
}

// @pymethod ([dict, ...], total, resumeHandle)|win32net|NetWkstaUserEnum|Retrieves information about all users currently logged on to the workstation.
// @rdesc The result is a list of items read (with each item being a dictionary of format
// <o PyWKSTA_USER_INFO_*>, depending on the level parameter),
// the total available, and a new "resume handle".  The first time you call
// this function, you should pass zero for the resume handle.  If more data
// is available than what was returned, a new non-zero resume handle will be
// returned, which can be used to call the function again to fetch more data.
// This process may repeat, each time with a new resume handle, until zero is
// returned for the new handle, indicating all the data has been read.
PyObject *
PyNetWkstaUserEnum(PyObject *self, PyObject *args)
{
	WCHAR *szServer = NULL, *szDomain = NULL;
	PyObject *obServer, *obDomain = Py_None;
	PyObject *ret = NULL;
	PyNET_STRUCT *pInfo;
	DWORD err;
	DWORD dwPrefLen = MAX_PREFERRED_LENGTH;
	DWORD level;
	BOOL ok = FALSE;
	DWORD resumeHandle = 0;
	DWORD numRead, i;
	PyObject *list;
	BYTE *buf = NULL;
	DWORD totalEntries = 0;
	// @pyparm string/<o PyUnicode>|server||The name of the server to execute on, or None.
	// @pyparm int|level||The level of data required.
	// @pyparm int|resumeHandle|0|A resume handle.  See the return description for more information.
	// @pyparm int|prefLen|MAX_PREFERRED_LENGTH|The preferred length of the data buffer.
	if (!PyArg_ParseTuple(args, "Oi|ii", &obServer, &level, &resumeHandle, &dwPrefLen))
		return NULL;
	if (!PyWinObject_AsWCHAR(obServer, &szServer, TRUE))
		goto done;
	if (!PyWinObject_AsWCHAR(obDomain, &szDomain, TRUE))
		goto done;

	if (!FindNET_STRUCT(level, wktau_infos, &pInfo))
		goto done;

	Py_BEGIN_ALLOW_THREADS
	err = NetWkstaUserEnum(szServer, level, &buf, dwPrefLen, &numRead, &totalEntries, &resumeHandle);
	Py_END_ALLOW_THREADS
	if (err!=0 && err != ERROR_MORE_DATA) {
		ReturnNetError("NetWkstaUserEnum",err);
		goto done;
	}
	list = PyList_New(numRead);
	if (list==NULL) goto done;
	for (i=0;i<numRead;i++) {
		PyObject *sub = PyObject_FromNET_STRUCT(pInfo, buf+(i*pInfo->structsize));
		if (sub==NULL) goto done;
		PyList_SetItem(list, i, sub);
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
	// @pyseeapi NetWkstaUserEnum
}

// @pymethod dict|win32net|NetWkstaGetInfo|Retrieves information about the configuration elements for a workstation
PyObject *
PyNetWkstaGetInfo(PyObject *self, PyObject *args)
{
	WCHAR *szServer = NULL;
	PyObject *obServer;
	PyNET_STRUCT *pInfo;
	BYTE *buf = NULL;
	PyObject *ret = NULL;
	int typ;
	DWORD err;
	// @pyparm string/<o PyUnicode>|server||The name of the server to execute on, or None.
	// @pyparm int|level||The information level contained in the data. NOTE: levels 302 and 402 don't seem to work correctly. They return error 124. So currently these info levels are not available.
	if (!PyArg_ParseTuple(args, "Oi", &obServer, &typ))
		return NULL;
	if (!PyWinObject_AsWCHAR(obServer, &szServer, TRUE))
		goto done;
	if (!FindNET_STRUCT(typ, wksta_infos, &pInfo))
		goto done;
	Py_BEGIN_ALLOW_THREADS
	err = NetWkstaGetInfo(szServer, typ, &buf);
	Py_END_ALLOW_THREADS
	if (err) {
		ReturnNetError("NetWkstaGetInfo",err);
		goto done;
	}
	ret= PyObject_FromNET_STRUCT(pInfo, buf);
done:
	if (buf) NetApiBufferFree(buf);
	PyWinObject_FreeWCHAR(szServer);
	return ret;
	// @pyseeapi NetWkstaGetInfo
	// @rdesc The result will be a dictionary in one of the <o PyWKSTA_INFO_*>
	// formats, depending on the level parameter.
}

// @pymethod |win32net|NetWkstaSetInfo|Sets information about the configuration elements for a workstation
PyObject *
PyNetWkstaSetInfo(PyObject *self, PyObject *args)
{
	WCHAR *szServer = NULL;
	PyObject *obServer, *obData;
	PyNET_STRUCT *pInfo;
	BYTE *buf = NULL;
	PyObject *ret = NULL;
	int typ;
	DWORD err = 0;
	// @pyparm string/<o PyUnicode>|server||The name of the server, or None.
	// @pyparm int|level||The information level contained in the data
	// @pyparm mapping|data||A dictionary holding the share data.
	if (!PyArg_ParseTuple(args, "OiO", &obServer, &typ, &obData))
		return NULL;
	if (!PyWinObject_AsWCHAR(obServer, &szServer, TRUE))
		goto done;

	if (!FindNET_STRUCT(typ, wksta_infos, &pInfo))
		goto done;

	if (!PyObject_AsNET_STRUCT(obData, pInfo, &buf))
		goto done;

	Py_BEGIN_ALLOW_THREADS
	err = NetWkstaSetInfo(szServer, typ, buf, NULL);
	Py_END_ALLOW_THREADS
	if (err) {
		ReturnNetError("NetWkstaSetInfo",err);	
		goto done;
	}
	ret= Py_None;
	Py_INCREF(ret);
done:
	if (buf) PyObject_FreeNET_STRUCT(pInfo, buf);
	PyWinObject_FreeWCHAR(szServer);
	return ret;
	// @pyseeapi NetWkstaSetInfo
}

// @pymethod ([dict, ...], total, resumeHandle)|win32net|NetWkstaTransportEnum|Retrieves information about transport protocols that are currently managed by the redirector
// @rdesc The result is a list of items read (with each item being a dictionary of format
// <o PyWKSTA_TRANSPORT_INFO_*>, depending on the level parameter),
// the total available, and a new "resume handle".  The first time you call
// this function, you should pass zero for the resume handle.  If more data
// is available than what was returned, a new non-zero resume handle will be
// returned, which can be used to call the function again to fetch more data.
// This process may repeat, each time with a new resume handle, until zero is
// returned for the new handle, indicating all the data has been read.
PyObject *
PyNetWkstaTransportEnum(PyObject *self, PyObject *args)
{
	WCHAR *szServer = NULL, *szDomain = NULL;
	PyObject *obServer, *obDomain = Py_None;
	PyObject *ret = NULL;
	PyNET_STRUCT *pInfo;
	DWORD err;
	DWORD dwPrefLen = MAX_PREFERRED_LENGTH;
	DWORD level;
	BOOL ok = FALSE;
	DWORD resumeHandle = 0;
	DWORD numRead, i;
	PyObject *list;
	BYTE *buf = NULL;
	DWORD totalEntries = 0;
	// @pyparm string/<o PyUnicode>|server||The name of the server to execute on, or None.
	// @pyparm int|level||The level of data required.
	// @pyparm int|resumeHandle|0|A resume handle.  See the return description for more information.
	// @pyparm int|prefLen|MAX_PREFERRED_LENGTH|The preferred length of the data buffer.
	if (!PyArg_ParseTuple(args, "Oi|ii", &obServer, &level, &resumeHandle, &dwPrefLen))
		return NULL;
	if (!PyWinObject_AsWCHAR(obServer, &szServer, TRUE))
		goto done;
	if (!PyWinObject_AsWCHAR(obDomain, &szDomain, TRUE))
		goto done;

	if (!FindNET_STRUCT(level, wkstransport_infos, &pInfo))
		goto done;

	Py_BEGIN_ALLOW_THREADS
	err = NetWkstaTransportEnum(szServer, level, &buf, dwPrefLen, &numRead, &totalEntries, &resumeHandle);
	Py_END_ALLOW_THREADS
	if (err!=0 && err != ERROR_MORE_DATA) {
		ReturnNetError("NetWkstaTransportEnum",err);
		goto done;
	}
	list = PyList_New(numRead);
	if (list==NULL) goto done;
	for (i=0;i<numRead;i++) {
		PyObject *sub = PyObject_FromNET_STRUCT(pInfo, buf+(i*pInfo->structsize));
		if (sub==NULL) goto done;
		PyList_SetItem(list, i, sub);
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
	// @pyseeapi NetWkstaTransportEnum
}

// @pymethod |win32net|NetWkstaTransportAdd|binds the redirector to a transport
PyObject *
PyNetWkstaTransportAdd(PyObject *self, PyObject *args)
{
	WCHAR *szServer = NULL;
	PyObject *obServer, *obData;
	PyNET_STRUCT *pInfo;
	BYTE *buf = NULL;
	PyObject *ret = NULL;
	int typ;
	DWORD err = 0;
	// @pyparm string/<o PyUnicode>|server||The name of the server, or None.
	// @pyparm int|level||The information level contained in the data
	// @pyparm mapping|data||A dictionary holding the share data.
	if (!PyArg_ParseTuple(args, "OiO", &obServer, &typ, &obData))
		return NULL;
	if (!PyWinObject_AsWCHAR(obServer, &szServer, TRUE))
		goto done;

	if (!FindNET_STRUCT(typ, wkstransport_infos, &pInfo))
		goto done;

	if (!PyObject_AsNET_STRUCT(obData, pInfo, &buf))
		goto done;

	Py_BEGIN_ALLOW_THREADS
	err = NetWkstaTransportAdd(szServer, typ, buf, NULL);
	Py_END_ALLOW_THREADS
	if (err) {
		ReturnNetError("NetWkstaTransportAdd",err);	
		goto done;
	}
	ret= Py_None;
	Py_INCREF(ret);
done:
	if (buf) PyObject_FreeNET_STRUCT(pInfo, buf);
	PyWinObject_FreeWCHAR(szServer);
	return ret;
	// @pyseeapi NetWkstaTransportAdd
}

// @pymethod |win32net|NetWkstaTransportDel|unbinds the transport protocol from redirector
PyObject *
PyNetWkstaTransportDel(PyObject *self, PyObject *args)
{
	WCHAR *szServer = NULL, *szTransport = NULL;
	PyObject *obServer, *obTransport;
	BYTE *buf = NULL;
	PyObject *ret = NULL;
	DWORD err = 0;
	int ucond = 0;
	// @pyparm string/<o PyUnicode>|server||The name of the server, or None.
	// @pyparm string/<o PyUnicode>|TransportName||The name of the transport to delete.
	// @pyparm int|ucond|0|Level of force to use. Can be USE_FORCE or USE_NOFORCE or USE_LOTS_OF_FORCE
	if (!PyArg_ParseTuple(args, "OO|i", &obServer, &obTransport, &ucond))
		return NULL;
	if (!PyWinObject_AsWCHAR(obServer, &szServer, TRUE))
		goto done;
	if (!PyWinObject_AsWCHAR(obTransport, &szTransport, TRUE))
		goto done;

	Py_BEGIN_ALLOW_THREADS
	err = NetWkstaTransportDel(szServer, szTransport, ucond);
	Py_END_ALLOW_THREADS
	if (err) {
		ReturnNetError("NetWkstaTransportDel",err);	
		goto done;
	}
	ret= Py_None;
	Py_INCREF(ret);
done:
	PyWinObject_FreeWCHAR(szServer);
	PyWinObject_FreeWCHAR(szTransport);
	return ret;
	// @pyseeapi NetWkstaTransportDel
}

// @pymethod list|win32net|NetServerDiskEnum|Retrieves the list of disk drives on a server.
// @rdesc The result is a list of drives on the server
PyObject *
PyNetServerDiskEnum(PyObject *self, PyObject *args)
{
	WCHAR *szServer=NULL, *disk=NULL;
	PyObject *obServer=NULL, *obdisk=NULL;
	PyObject *ret = NULL;
	DWORD err;
	DWORD dwPrefLen = MAX_PREFERRED_LENGTH;
	DWORD level=0;
	DWORD resumeHandle = 0;
	DWORD numRead, i;
	BYTE *buf = NULL;
	DWORD totalEntries = 0;
	// @pyparm string/<o PyUnicode>|server||The name of the server to execute on, or None.
	// @pyparm int|level||The level of data required. Must be 0.
	if (!PyArg_ParseTuple(args, "O|i", &obServer, &level))
		return NULL;
	if (!PyWinObject_AsWCHAR(obServer, &szServer, TRUE))
		return NULL;

	Py_BEGIN_ALLOW_THREADS
	err = NetServerDiskEnum(szServer, level, &buf, dwPrefLen, &numRead, &totalEntries, &resumeHandle);
	Py_END_ALLOW_THREADS
	if (err!=0) {
		ReturnNetError("NetServerDiskEnum",err);
		goto done;
	}
	ret = PyTuple_New(totalEntries);
	if (ret==NULL)
		goto done;
	// The return buffer contains drive letters as 3-char segments terminated by NULLs
	for (i=0;i<totalEntries;i++){
		disk=(WCHAR *)(buf+(i*sizeof(WCHAR)*3));
		obdisk = PyWinObject_FromWCHAR(disk,2);
		if (obdisk==NULL){
			Py_DECREF(ret);
			ret = NULL;
			break;
			}
		PyTuple_SetItem(ret, i, obdisk);
		}
done:
	if (buf) NetApiBufferFree(buf);
	if (szServer)
		PyWinObject_FreeWCHAR(szServer);
	return ret;
	// @pyseeapi NetServerDiskEnum
}

// @pymethod dict|win32net|NetStatisticsGet|Retrieves network statistics for specified service on specified machine
// @rdesc The result is a dictionary representing a STAT_SERVER_0 or STAT_WORKSTATION_0 struct
PyObject *
PyNetStatisticsGet(PyObject *self, PyObject *args)
{
	STAT_WORKSTATION_0 *stat_workstation;
	STAT_SERVER_0 *stat_server;
	NET_API_STATUS err;
	PyObject *obServer=NULL, *obService=NULL;
	PyObject *ret=NULL;
	WCHAR *server=NULL,*service=NULL;
	DWORD level=0,options=0;
	LPBYTE buf=NULL;
	int cmp_res;
	// @pyparm string/<o PyUnicode>|server||Name of server/workstation to retrieve statistics for (None or blank uses local).
	// @pyparm string/<o PyUnicode>|service||SERVICE_SERVER or SERVICE_WORKSTATION
	// @pyparm int|level||Only 0 currently supported.
	// @pyparm int|options||Must be zero.
	if (!PyArg_ParseTuple(args, "OO|ii", &obServer, &obService, &level, &options))
		return NULL;
	if (!PyWinObject_AsWCHAR(obServer, &server, TRUE))
		goto done;
	if (!PyWinObject_AsWCHAR(obService, &service, FALSE))
		goto done;

	err = NetStatisticsGet(server,service,level,options,&buf);
	if (err != NERR_Success){
		ReturnNetError("NetStatisticsGet",err);
		goto done;
		}

	cmp_res = CompareString(LOCALE_USER_DEFAULT,0,service,-1,SERVICE_SERVER,-1);
	if (cmp_res==CSTR_EQUAL){
		stat_server=(STAT_SERVER_0 *)buf;
		ret=Py_BuildValue(
				"{s:l,s:l,s:l,s:l,s:l,s:l,s:l,s:l,s:l,s:l,s:l,s:l,s:l,s:l,s:l,s:l,s:l}",
				"sts0_start",stat_server->sts0_start,
				"sts0_fopens",stat_server->sts0_fopens,
				"sts0_devopens",stat_server->sts0_devopens,
				"sts0_jobsqueued",stat_server->sts0_jobsqueued,
				"sts0_sopens",stat_server->sts0_sopens,
				"sts0_stimedout",stat_server->sts0_stimedout,
				"sts0_serrorout",stat_server->sts0_serrorout,
				"sts0_pwerrors",stat_server->sts0_pwerrors,
				"sts0_permerrors",stat_server->sts0_permerrors,
				"sts0_syserrors",stat_server->sts0_syserrors,
				"sts0_bytessent_low",stat_server->sts0_bytessent_low,
				"sts0_bytessent_high",stat_server->sts0_bytessent_high,
				"sts0_bytesrcvd_low",stat_server->sts0_bytesrcvd_low,
				"sts0_bytesrcvd_high",stat_server->sts0_bytesrcvd_high,
				"sts0_avresponse",stat_server->sts0_avresponse,
				"sts0_reqbufneed",stat_server->sts0_reqbufneed,
				"sts0_bigbufneed",stat_server->sts0_bigbufneed);
		goto done;
		}
	cmp_res = CompareString(LOCALE_USER_DEFAULT,0,service,-1,SERVICE_WORKSTATION,-1);
	if (cmp_res==CSTR_EQUAL){
		stat_workstation=(STAT_WORKSTATION_0 *)buf;
		ret=Py_BuildValue(
				"{s:N,s:N,s:N,s:N,s:N,s:N,s:N,s:N,s:N,s:N,s:N,s:N,s:N,s:l,s:l,s:l,s:l,s:l,s:l,s:l,s:l,s:l,s:l,s:l,s:l,s:l,s:l,s:l,s:l,s:l,s:l,s:l,s:l,s:l,s:l,s:l,s:l,s:l,s:l,s:l}",
				"StatisticsStartTime",PyWinObject_FromLARGE_INTEGER(stat_workstation->StatisticsStartTime),
				"BytesReceived",PyWinObject_FromLARGE_INTEGER(stat_workstation->BytesReceived),
				"SmbsReceived",PyWinObject_FromLARGE_INTEGER(stat_workstation->SmbsReceived),
				"PagingReadBytesRequested",PyWinObject_FromLARGE_INTEGER(stat_workstation->PagingReadBytesRequested),
				"NonPagingReadBytesRequested",PyWinObject_FromLARGE_INTEGER(stat_workstation->NonPagingReadBytesRequested),
				"CacheReadBytesRequested",PyWinObject_FromLARGE_INTEGER(stat_workstation->NonPagingReadBytesRequested),
				"NetworkReadBytesRequested",PyWinObject_FromLARGE_INTEGER(stat_workstation->NetworkReadBytesRequested),
				"BytesTransmitted",PyWinObject_FromLARGE_INTEGER(stat_workstation->BytesTransmitted),
				"SmbsTransmitted",PyWinObject_FromLARGE_INTEGER(stat_workstation->SmbsTransmitted),
				"PagingWriteBytesRequested",PyWinObject_FromLARGE_INTEGER(stat_workstation->PagingWriteBytesRequested),
				"NonPagingWriteBytesRequested",PyWinObject_FromLARGE_INTEGER(stat_workstation->NonPagingWriteBytesRequested),
				"CacheWriteBytesRequested",PyWinObject_FromLARGE_INTEGER(stat_workstation->CacheWriteBytesRequested),
				"NetworkWriteBytesRequested",PyWinObject_FromLARGE_INTEGER(stat_workstation->NetworkWriteBytesRequested),
				"InitiallyFailedOperations",stat_workstation->InitiallyFailedOperations,
				"FailedCompletionOperations",stat_workstation->FailedCompletionOperations,
				"ReadOperations",stat_workstation->ReadOperations,
				"RandomReadOperations",stat_workstation->RandomReadOperations,
				"ReadSmbs",stat_workstation->ReadSmbs,
				"LargeReadSmbs",stat_workstation->LargeReadSmbs,
				"SmallReadSmbs",stat_workstation->SmallReadSmbs,
				"WriteOperations",stat_workstation->WriteOperations,
				"RandomWriteOperations",stat_workstation->RandomWriteOperations,
				"WriteSmbs",stat_workstation->WriteSmbs,
				"LargeWriteSmbs",stat_workstation->LargeWriteSmbs,
				"SmallWriteSmbs",stat_workstation->SmallWriteSmbs,
				"RawReadsDenied",stat_workstation->RawReadsDenied,
				"RawWritesDenied",stat_workstation->RawWritesDenied,
				"NetworkErrors",stat_workstation->NetworkErrors,
				"Sessions",stat_workstation->Sessions,
				"FailedSessions",stat_workstation->FailedSessions,
				"Reconnects",stat_workstation->Reconnects,
				"CoreConnects",stat_workstation->CoreConnects,
				"Lanman20Connects",stat_workstation->Lanman20Connects,
				"Lanman21Connects",stat_workstation->Lanman21Connects,
				"LanmanNtConnects",stat_workstation->LanmanNtConnects,
				"ServerDisconnects",stat_workstation->ServerDisconnects,
				"HungSessions",stat_workstation->HungSessions,
				"UseCount",stat_workstation->UseCount,
				"FailedUseCount",stat_workstation->FailedUseCount,
				"CurrentCommands",stat_workstation->CurrentCommands
				);
		goto done;
		}
	PyErr_SetString(PyExc_ValueError, "Invalid service name");

	done:
	if (server!=NULL)
		PyWinObject_FreeWCHAR(server);
	if (service!=NULL)
		PyWinObject_FreeWCHAR(service);
	if (buf != NULL)
		NetApiBufferFree(buf);
	return ret;
}



// @pymethod |win32net|NetServerComputerNameAdd|Adds an additional network name for a server
// @rdesc Returns none on success
PyObject * PyNetServerComputerNameAdd(PyObject *self, PyObject *args)
{
	// @pyparm string/<o PyUnicode>|ServerName||Name of server that will receive additional name
	// @pyparm string/<o PyUnicode>|EmulatedDomainName||Domain under which to add the new server name, can be None
	// @pyparm string/<o PyUnicode>|EmulatedServerName||New network name that server will respond to
	NET_API_STATUS err;
	WCHAR *servername=NULL, *domain=NULL, *newname=NULL;
	PyObject *observername, *obdomain, *obnewname;
	PyObject *ret=NULL;
	if (!PyArg_ParseTuple(args,"OOO", &observername, &obdomain, &obnewname))
		return NULL;
	if (PyWinObject_AsWCHAR(observername, &servername, FALSE)
		&&PyWinObject_AsWCHAR(obdomain, &domain, TRUE)
		&&PyWinObject_AsWCHAR(obnewname, &newname, FALSE)){
		err=NetServerComputerNameAdd(servername, domain, newname);
		if (err==NERR_Success){
			Py_INCREF(Py_None);
			ret=Py_None;
			}
		else
			ReturnNetError("NetServerComputerNameAdd", err);
		}
	PyWinObject_FreeWCHAR(servername);
	PyWinObject_FreeWCHAR(domain);
	PyWinObject_FreeWCHAR(newname);
	return ret;
}

// @pymethod |win32net|NetServerComputerNameDel|Removes a network name added by <om win32net.NetServerComputerNameAdd>
// @rdesc Returns none on success
PyObject * PyNetServerComputerNameDel(PyObject *self, PyObject *args)
{
	// @pyparm string/<o PyUnicode>|ServerName||Name of server on which to operate
	// @pyparm string/<o PyUnicode>|EmulatedServerName||Network name to be removed
	NET_API_STATUS err;
	WCHAR *servername=NULL, *newname=NULL;
	PyObject *observername, *obnewname;
	PyObject *ret=NULL;
	if (!PyArg_ParseTuple(args,"OO", &observername, &obnewname))
		return NULL;
	if (PyWinObject_AsWCHAR(observername, &servername, FALSE)
		&&PyWinObject_AsWCHAR(obnewname, &newname, FALSE)){
		err=NetServerComputerNameDel(servername, newname);
		if (err==NERR_Success){
			Py_INCREF(Py_None);
			ret=Py_None;
			}
		else
			ReturnNetError("NetServerComputerNameDel", err);
		}
	PyWinObject_FreeWCHAR(servername);
	PyWinObject_FreeWCHAR(newname);
	return ret;
}

#if WINVER >= 0x0500

extern "C" NetValidateNamefunc pfnNetValidateName=NULL;
// @pymethod |win32net|NetValidateName|Checks that domain/machine/workgroup name is valid for given context
// @rdesc Returns none if valid, exception if not
// @comm If Account and Password aren't passed, current logon credentials are used
// @comm Will raise NotImplementedError if not available on this platform.
PyObject *
PyNetValidateName(PyObject *self, PyObject *args)
{
	// @pyparm string/<o PyUnicode>|Server||Name of server on which to execute (None or blank uses local)
	// @pyparm string/<o PyUnicode>|Name||Machine, domain, or workgroup name to validate
	// @pyparm int|NameType||Type of name to validate - from NETSETUP_NAME_TYPE enum (win32net.NetSetup*)
	// @pyparm string/<o PyUnicode>|Account|None|Account name to use while validating, current security context is used if not specified
	// @pyparm string/<o PyUnicode>|Password|None|Password for Account
	PyObject *obServer, *obName, *obAccount=Py_None, *obPassword=Py_None, *ret=NULL;
	WCHAR *Server=NULL, *Name=NULL, *Account=NULL, *Password=NULL;
	NET_API_STATUS err;
	NETSETUP_NAME_TYPE NameType;
	if (pfnNetValidateName==NULL){
		PyErr_SetString(PyExc_NotImplementedError,"NetValidateName does not exist on this platform");
		return NULL;
		}
	if (!PyArg_ParseTuple(args, "OOl|OO",&obServer, &obName, &NameType, &obAccount, &obPassword))
		return NULL;
	if (PyWinObject_AsWCHAR(obServer, &Server, TRUE)
		&&PyWinObject_AsWCHAR(obName, &Name, FALSE)
		&&PyWinObject_AsWCHAR(obAccount, &Account, TRUE)
		&&PyWinObject_AsWCHAR(obPassword, &Password, TRUE)){
		err=(*pfnNetValidateName)(Server, Name, Account, Password, NameType);
		if (err==NERR_Success)
			ret=Py_None;
		else
			ReturnNetError("NetValidateName", err);
		}
	if (Server)
		PyWinObject_FreeWCHAR(Server);
	if (Name)
		PyWinObject_FreeWCHAR(Name);
	if (Account)
		PyWinObject_FreeWCHAR(Account);
	if (Password)
		PyWinObject_FreeWCHAR(Password);
	Py_XINCREF(ret);
	return ret;
}

extern "C" NetValidatePasswordPolicyfunc pfnNetValidatePasswordPolicy=NULL;
extern "C" NetValidatePasswordPolicyFreefunc pfnNetValidatePasswordPolicyFree=NULL;

static void PyObject_CleanupAUTH_INPUT(NET_VALIDATE_AUTHENTICATION_INPUT_ARG *p)
{
	// Don't accept NET_VALIDATE_PERSISTED_FIELDS for input yet.
	//PyObject_CleanupNET_VALIDATE_PERSISTED_FIELDS(&p->InputPersistedFields);
}

// @object PyNET_VALIDATE_AUTHENTICATION_INPUT_ARG|A dictionary or tuple passed as input to <om win32net.NetValidatePasswordPolicy>
static BOOL PyObject_AsAUTH_INPUT(PyObject *ob, NET_VALIDATE_AUTHENTICATION_INPUT_ARG *p)
{
	// NOTE: We assume the caller (a) has initialized to 0 and (b) will call
	// PyObject_CleanupAUTH_INPUT() even on failure.
	static char *keywords[]={"InputPersistedFields","PasswordMatched", NULL};
	PyObject *obipf = Py_None, *obPassword = Py_None, *obAcct = Py_None;
	PyObject *kw, *args;
	BOOL decref_args = FALSE;
	if (PyDict_Check(ob)) {
		kw = ob;
		args = PyTuple_New(0);
		BOOL decref_args = (args != 0);
	} else if PyTuple_Check(ob) {
		kw = NULL;
		args = ob;
	} else {
		PyErr_SetString(PyExc_TypeError, "Must be tuple or dict");
		return FALSE;
	}

	int rc = PyArg_ParseTupleAndKeywords(args, kw, "|Oi:NET_VALIDATE_AUTHENTICATION_INPUT_ARG", keywords,
		&obipf,	// @pyparm <o NET_VALIDATE_PERSISTED_FIELDS>|InputPersistedFields|None|
		&p->PasswordMatched);	// @pyparm int|PasswordMatched|0|

	if (decref_args)
		Py_DECREF(args);
	if (!rc)
		return FALSE;

	if (obipf != Py_None) {
		PyErr_SetString(PyExc_ValueError, "InputPersistedFiles not yet implemented for input");
		return FALSE;
	}
	return TRUE;
}

static void PyObject_CleanupCHANGE_INPUT(NET_VALIDATE_PASSWORD_CHANGE_INPUT_ARG *p)
{
	// Don't accept NET_VALIDATE_PERSISTED_FIELDS for input yet.
	//PyObject_CleanupNET_VALIDATE_PERSISTED_FIELDS(&p->InputPersistedFields);
	if (p->ClearPassword)
		PyWinObject_FreeWCHAR(p->ClearPassword);
	if (p->UserAccountName)
		PyWinObject_FreeWCHAR(p->UserAccountName);
	// hashed-password is pointing at a Python buffer, so don't touch!
}

// @object PyNET_VALIDATE_PASSWORD_CHANGE_INPUT_ARG|A dictionary or tuple passed as input to <om win32net.NetValidatePasswordPolicy>
static BOOL PyObject_AsCHANGE_INPUT(PyObject *ob, NET_VALIDATE_PASSWORD_CHANGE_INPUT_ARG *p)
{
	// NOTE: We assume the caller (a) has initialized to 0 and (b) will call
	// PyObject_CleanupCHANGE_INPUT() even on failure.
	static char *keywords[]={"InputPersistedFields","ClearPassword","UserAccountName","HashedPassword", "PasswordMatch", NULL};
	PyObject *obipf = Py_None, *obPassword = Py_None, *obAcct = Py_None;

	PyObject *kw, *args;
	BOOL decref_args = FALSE;
	if (PyDict_Check(ob)) {
		kw = ob;
		args = PyTuple_New(0);
		BOOL decref_args = (args != 0);
	} else if PyTuple_Check(ob) {
		kw = NULL;
		args = ob;
	} else {
		PyErr_SetString(PyExc_TypeError, "Must be tuple or dict");
		return FALSE;
	}

	int rc = PyArg_ParseTupleAndKeywords(args, kw, "|OOOz#i:NET_VALIDATE_PASSWORD_CHANGE_INPUT_ARG", keywords,
		&obipf,	// @pyparm <o NET_VALIDATE_PERSISTED_FIELDS>|InputPersistedFields|None|
		&obPassword,	// @pyparm <o PyUnicode>|ClearPassword|None|
		&obAcct,	// @pyparm <o PyUnicode>|UserAccountName|None|
		&p->HashedPassword.Hash, // @pyparm buffer|HashedPassword|None|A string or anything else holding bytes.
		&p->HashedPassword.Length,
		&p->PasswordMatch);	// @pyparm int|PasswordMatch|0|Note MSDN incorrectly documents this member as PasswordMatched

	if (decref_args)
		Py_DECREF(args);
	if (!rc)
		return FALSE;

	if (obipf != Py_None) {
		PyErr_SetString(PyExc_ValueError, "InputPersistedFiles not yet implemented for input");
		return FALSE;
	}

	if (!PyWinObject_AsWCHAR(obPassword, &p->ClearPassword, TRUE))
		return FALSE;

	if (!PyWinObject_AsWCHAR(obAcct, &p->UserAccountName, TRUE))
		return FALSE;

	return TRUE;
}

#define SAFE_INSERT_NEW_REF(dict, name, ob) {\
	PyObject *tmp=(ob); \
	if (!tmp) { Py_DECREF(dict); return NULL; } \
	PyDict_SetItemString(dict, name, tmp); \
	Py_DECREF(tmp); \
	}

// @object PyNET_VALIDATE_PERSISTED_FIELDS|A dictionary returned by <om win32net.NetValidatePasswordPolicy>
static PyObject *PyObject_FromNET_VALIDATE_PERSISTED_FIELDS(NET_VALIDATE_PERSISTED_FIELDS *f)
{
	// @comm Note that these fields will only appear if the PresentFields
	// structure element indicates the fields are valid.  Thus, the result
	// dictionary may contain none, all, or any combination of these.
	PyObject *ret = PyDict_New();
	if (!ret)
		return NULL;
	// @pyparm <o PyTime>|PasswordLastSet||
	if (f->PresentFields & NET_VALIDATE_PASSWORD_LAST_SET)
		SAFE_INSERT_NEW_REF(ret, "PasswordLastSet", PyWinObject_FromFILETIME(f->PasswordLastSet));
	// @pyparm <o PyTime>|BadPasswordTime||
	if (f->PresentFields & NET_VALIDATE_BAD_PASSWORD_TIME)
		SAFE_INSERT_NEW_REF(ret, "BadPasswordTime", PyWinObject_FromFILETIME(f->BadPasswordTime));
	// @pyparm <o PyTime>|LockoutTime||
	if (f->PresentFields & NET_VALIDATE_LOCKOUT_TIME)
		SAFE_INSERT_NEW_REF(ret, "LockoutTime", PyWinObject_FromFILETIME(f->LockoutTime));
	// @pyparm int|BadPasswordCount||
	if (f->PresentFields & NET_VALIDATE_BAD_PASSWORD_COUNT)
		SAFE_INSERT_NEW_REF(ret, "BadPasswordCount", PyLong_FromUnsignedLong(f->BadPasswordCount));
	// @pyparm int|PasswordHistoryLength||
	if (f->PresentFields & NET_VALIDATE_PASSWORD_HISTORY_LENGTH)
		SAFE_INSERT_NEW_REF(ret, "PasswordHistoryLength", PyLong_FromUnsignedLong(f->PasswordHistoryLength));
	// @pyparm None/string|PasswordHistory||
	if (f->PresentFields & NET_VALIDATE_PASSWORD_HISTORY) {
		if (f->PasswordHistory) {
			SAFE_INSERT_NEW_REF(ret, "PasswordHistory", PyString_FromStringAndSize((char *)f->PasswordHistory->Hash, f->PasswordHistory->Length));
		} else
			PyDict_SetItemString(ret, "PasswordHistory", Py_None);
	}
	return ret;
}

// @pymethod |win32net|NetValidatePasswordPolicy|Allows an application to check
// password compliance against an application-provided account database and
// verify that passwords meet the complexity, aging, minimum length, and
// history reuse requirements of a password policy.
// @rdesc Returns a tuple of (<o PyNET_VALIDATE_PERSISTED_FIELDS>, int) with
// the integer being the ValidationResult.
// @comm Will raise NotImplementedError if not available on this platform, or
// raise win32net.error if the function fails.
PyObject *
PyNetValidatePasswordPolicy(PyObject *self, PyObject *args)
{
	// @pyparm string/<o PyUnicode>|Server||Name of server on which to execute (None or blank uses local)
	// @pyparm None|Qualifier||Reserved, must be None
	// @pyparm int|ValidationType||The type of password validation to perform
	// @pyparm dict/tuple|arg||Depends on the ValidationType param - either
	// a <o PyNET_VALIDATE_AUTHENTICATION_INPUT_ARG>,  <o PyNET_VALIDATE_PASSWORD_CHANGE_INPUT_ARG>
	// or <o PyNET_VALIDATE_PASSWORD_RESET_INPUT_ARG> tuple or dict.
	PyObject *obServer, *obQualifier, *obArg;
	int valType = -1;
	WCHAR *Server=NULL;
	PyObject *ret = NULL;
	NET_API_STATUS err;
	NET_VALIDATE_OUTPUT_ARG *out_arg = NULL;

	// We only use one of these
	union ALL_INS {
		NET_VALIDATE_AUTHENTICATION_INPUT_ARG auth;
		NET_VALIDATE_PASSWORD_CHANGE_INPUT_ARG change;
		NET_VALIDATE_PASSWORD_RESET_INPUT_ARG reset;
	};
	ALL_INS in_arg;
	memset(&in_arg, 0, sizeof(in_arg));

	if (pfnNetValidateName==NULL || pfnNetValidatePasswordPolicyFree==NULL){
		PyErr_SetString(PyExc_NotImplementedError,"NetValidatePasswordPolicy does not exist on this platform");
		return NULL;
	}
	if (!PyArg_ParseTuple(args, "OOiO",&obServer, &obQualifier, &valType, &obArg))
		return NULL;
	if (obQualifier != Py_None)
		return PyErr_Format(PyExc_ValueError, "The Qualifier arg must be None");

	if (!PyWinObject_AsWCHAR(obServer, &Server, TRUE))
		goto done;

	switch (valType) {
		case NetValidateAuthentication:
			if (!PyObject_AsAUTH_INPUT(obArg, &in_arg.auth))
				goto done;
			break;
		case NetValidatePasswordChange:
			if (!PyObject_AsCHANGE_INPUT(obArg, &in_arg.change))
				goto done;
			break;
//		case NetValidatePasswordReset:
//			break;
		default:
			PyErr_Format(PyExc_ValueError, "Unknown validation type (%d)", valType);
			goto done;
	}
	Py_BEGIN_ALLOW_THREADS
	err = (*pfnNetValidatePasswordPolicy)(Server, NULL, (NET_VALIDATE_PASSWORD_TYPE)valType, &in_arg, (void **)&out_arg);
	Py_END_ALLOW_THREADS
	if (NERR_Success != err) {
		ReturnNetError("NetValidatePasswordPolicy", err);
		goto done;
	}
	ret = Py_BuildValue("Ni", PyObject_FromNET_VALIDATE_PERSISTED_FIELDS(&out_arg->ChangedPersistedFields), out_arg->ValidationStatus);
done:
	if (Server)
		PyWinObject_FreeWCHAR(Server);
	switch (valType) {
		case NetValidateAuthentication:
			PyObject_CleanupAUTH_INPUT(&in_arg.auth);
			break;
		case NetValidatePasswordChange:
			PyObject_CleanupCHANGE_INPUT(&in_arg.change);
			break;
//		case NetValidatePasswordReset:
//			break;
		default:
			break;
	}
	if (out_arg)
		(*pfnNetValidatePasswordPolicyFree)(( void **) &out_arg);
	return ret;
}

#endif // WINVER
