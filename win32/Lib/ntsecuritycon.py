# Hacked from winnt.h

DELETE = (0x00010000)
READ_CONTROL = (0x00020000)
WRITE_DAC = (0x00040000)
WRITE_OWNER = (0x00080000)
SYNCHRONIZE = (0x00100000)
STANDARD_RIGHTS_REQUIRED = (0x000F0000)
STANDARD_RIGHTS_READ = (READ_CONTROL)
STANDARD_RIGHTS_WRITE = (READ_CONTROL)
STANDARD_RIGHTS_EXECUTE = (READ_CONTROL)
STANDARD_RIGHTS_ALL = (0x001F0000)
SPECIFIC_RIGHTS_ALL = (0x0000FFFF)
ACCESS_SYSTEM_SECURITY = (0x01000000)
MAXIMUM_ALLOWED = (0x02000000)
GENERIC_READ = (0x80000000)
GENERIC_WRITE = (0x40000000)
GENERIC_EXECUTE = (0x20000000)
GENERIC_ALL = (0x10000000)


SECURITY_NULL_SID_AUTHORITY = (0,0,0,0,0,0)
SECURITY_WORLD_SID_AUTHORITY = (0,0,0,0,0,1)
SECURITY_LOCAL_SID_AUTHORITY = (0,0,0,0,0,2)
SECURITY_CREATOR_SID_AUTHORITY = (0,0,0,0,0,3)
SECURITY_NON_UNIQUE_AUTHORITY = (0,0,0,0,0,4)

SECURITY_NULL_RID                 = 0x00000000
SECURITY_WORLD_RID                = 0x00000000
SECURITY_LOCAL_RID                = 0X00000000

SECURITY_CREATOR_OWNER_RID        = 0x00000000
SECURITY_CREATOR_GROUP_RID        = 0x00000001

SECURITY_CREATOR_OWNER_SERVER_RID = 0x00000002
SECURITY_CREATOR_GROUP_SERVER_RID = 0x00000003


# NT well-known SIDs
SECURITY_NT_AUTHORITY = (0,0,0,0,0,5)

SECURITY_DIALUP_RID             = 0x00000001
SECURITY_NETWORK_RID            = 0x00000002
SECURITY_BATCH_RID              = 0x00000003
SECURITY_INTERACTIVE_RID        = 0x00000004
SECURITY_SERVICE_RID            = 0x00000006
SECURITY_ANONYMOUS_LOGON_RID    = 0x00000007
SECURITY_PROXY_RID              = 0x00000008
SECURITY_SERVER_LOGON_RID       = 0x00000009

SECURITY_LOGON_IDS_RID          = 0x00000005
SECURITY_LOGON_IDS_RID_COUNT    = 3

SECURITY_LOCAL_SYSTEM_RID       = 0x00000012

SECURITY_NT_NON_UNIQUE          = 0x00000015

SECURITY_BUILTIN_DOMAIN_RID     = 0x00000020

# well-known domain relative sub-authority values (RIDs)...
DOMAIN_USER_RID_ADMIN          = 0x000001F4
DOMAIN_USER_RID_GUEST          = 0x000001F5



# well-known groups ...

DOMAIN_GROUP_RID_ADMINS        = 0x00000200
DOMAIN_GROUP_RID_USERS         = 0x00000201
DOMAIN_GROUP_RID_GUESTS        = 0x00000202




# well-known aliases ...

DOMAIN_ALIAS_RID_ADMINS        = 0x00000220
DOMAIN_ALIAS_RID_USERS         = 0x00000221
DOMAIN_ALIAS_RID_GUESTS        = 0x00000222
DOMAIN_ALIAS_RID_POWER_USERS   = 0x00000223

DOMAIN_ALIAS_RID_ACCOUNT_OPS   = 0x00000224
DOMAIN_ALIAS_RID_SYSTEM_OPS    = 0x00000225
DOMAIN_ALIAS_RID_PRINT_OPS     = 0x00000226
DOMAIN_ALIAS_RID_BACKUP_OPS    = 0x00000227

DOMAIN_ALIAS_RID_REPLICATOR    = 0x00000228

SYSTEM_LUID                    = (0x3E7, 0x0)

# Group attributes

SE_GROUP_MANDATORY              = 0x00000001
SE_GROUP_ENABLED_BY_DEFAULT     = 0x00000002
SE_GROUP_ENABLED                = 0x00000004
SE_GROUP_OWNER                  = 0x00000008
SE_GROUP_LOGON_ID               = 0xC0000000


# User attributes
# (None yet defined.)

ACCESS_ALLOWED_ACE_TYPE          = 0x0
ACCESS_DENIED_ACE_TYPE           = 0x1
SYSTEM_AUDIT_ACE_TYPE            = 0x2
SYSTEM_ALARM_ACE_TYPE            = 0x3


#  The following are the inherit flags that go into the AceFlags field
#  of an Ace header.

OBJECT_INHERIT_ACE               = 0x1
CONTAINER_INHERIT_ACE            = 0x2
NO_PROPAGATE_INHERIT_ACE         = 0x4
INHERIT_ONLY_ACE                 = 0x8
VALID_INHERIT_FLAGS              = 0xF


SUCCESSFUL_ACCESS_ACE_FLAG       = 0x40
FAILED_ACCESS_ACE_FLAG           = 0x80

SE_OWNER_DEFAULTED               = 0x0001
SE_GROUP_DEFAULTED               = 0x0002
SE_DACL_PRESENT                  = 0x0004
SE_DACL_DEFAULTED                = 0x0008
SE_SACL_PRESENT                  = 0x0010
SE_SACL_DEFAULTED                = 0x0020
SE_SELF_RELATIVE                 = 0x8000


SE_PRIVILEGE_ENABLED_BY_DEFAULT = 0x00000001
SE_PRIVILEGE_ENABLED            = 0x00000002
SE_PRIVILEGE_USED_FOR_ACCESS    = 0x80000000

PRIVILEGE_SET_ALL_NECESSARY    = 1

#               NT Defined Privileges

SE_CREATE_TOKEN_NAME              = "SeCreateTokenPrivilege"
SE_ASSIGNPRIMARYTOKEN_NAME        = "SeAssignPrimaryTokenPrivilege"
SE_LOCK_MEMORY_NAME               = "SeLockMemoryPrivilege"
SE_INCREASE_QUOTA_NAME            = "SeIncreaseQuotaPrivilege"
SE_UNSOLICITED_INPUT_NAME         = "SeUnsolicitedInputPrivilege"
SE_MACHINE_ACCOUNT_NAME           = "SeMachineAccountPrivilege"
SE_TCB_NAME                       = "SeTcbPrivilege"
SE_SECURITY_NAME                  = "SeSecurityPrivilege"
SE_TAKE_OWNERSHIP_NAME            = "SeTakeOwnershipPrivilege"
SE_LOAD_DRIVER_NAME               = "SeLoadDriverPrivilege"
SE_SYSTEM_PROFILE_NAME            = "SeSystemProfilePrivilege"
SE_SYSTEMTIME_NAME                = "SeSystemtimePrivilege"
SE_PROF_SINGLE_PROCESS_NAME       = "SeProfileSingleProcessPrivilege"
SE_INC_BASE_PRIORITY_NAME         = "SeIncreaseBasePriorityPrivilege"
SE_CREATE_PAGEFILE_NAME           = "SeCreatePagefilePrivilege"
SE_CREATE_PERMANENT_NAME          = "SeCreatePermanentPrivilege"
SE_BACKUP_NAME                    = "SeBackupPrivilege"
SE_RESTORE_NAME                   = "SeRestorePrivilege"
SE_SHUTDOWN_NAME                  = "SeShutdownPrivilege"
SE_DEBUG_NAME                     = "SeDebugPrivilege"
SE_AUDIT_NAME                     = "SeAuditPrivilege"
SE_SYSTEM_ENVIRONMENT_NAME        = "SeSystemEnvironmentPrivilege"
SE_CHANGE_NOTIFY_NAME             = "SeChangeNotifyPrivilege"
SE_REMOTE_SHUTDOWN_NAME           = "SeRemoteShutdownPrivilege"


# Enum SECURITY_IMPERSONATION_LEVEL:
SecurityAnonymous = 0
SecurityIdentification = 1
SecurityImpersonation = 2
SecurityDelegation = 3

SECURITY_MAX_IMPERSONATION_LEVEL = SecurityDelegation

DEFAULT_IMPERSONATION_LEVEL = SecurityImpersonation

TOKEN_ASSIGN_PRIMARY    = 0x0001
TOKEN_DUPLICATE         = 0x0002
TOKEN_IMPERSONATE       = 0x0004
TOKEN_QUERY             = 0x0008
TOKEN_QUERY_SOURCE      = 0x0010
TOKEN_ADJUST_PRIVILEGES = 0x0020
TOKEN_ADJUST_GROUPS     = 0x0040
TOKEN_ADJUST_DEFAULT    = 0x0080

TOKEN_ALL_ACCESS = (STANDARD_RIGHTS_REQUIRED  |\
                          TOKEN_ASSIGN_PRIMARY      |\
                          TOKEN_DUPLICATE           |\
                          TOKEN_IMPERSONATE         |\
                          TOKEN_QUERY               |\
                          TOKEN_QUERY_SOURCE        |\
                          TOKEN_ADJUST_PRIVILEGES   |\
                          TOKEN_ADJUST_GROUPS       |\
                          TOKEN_ADJUST_DEFAULT)


TOKEN_READ       = (STANDARD_RIGHTS_READ      |\
                          TOKEN_QUERY)


TOKEN_WRITE      = (STANDARD_RIGHTS_WRITE     |\
                          TOKEN_ADJUST_PRIVILEGES   |\
                          TOKEN_ADJUST_GROUPS       |\
                          TOKEN_ADJUST_DEFAULT)

TOKEN_EXECUTE    = (STANDARD_RIGHTS_EXECUTE)

SidTypeUser = 1
SidTypeGroup = 2
SidTypeDomain =3
SidTypeAlias = 4
SidTypeWellKnownGroup = 5
SidTypeDeletedAccount = 6
SidTypeInvalid = 7
SidTypeUnknown = 8

# Token types
TokenPrimary = 1
TokenImpersonation = 2

TokenUser = 1
TokenGroups = 2
TokenPrivileges = 3
TokenOwner = 4
TokenPrimaryGroup = 5
TokenDefaultDacl = 6
TokenSource = 7
TokenType = 8
TokenImpersonationLevel = 9
TokenStatistics = 10

