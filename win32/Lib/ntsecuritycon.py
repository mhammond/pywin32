# Hacked from winnt.h

DELETE = (65536)
READ_CONTROL = (131072)
WRITE_DAC = (262144)
WRITE_OWNER = (524288)
SYNCHRONIZE = (1048576)
STANDARD_RIGHTS_REQUIRED = (983040)
STANDARD_RIGHTS_READ = (READ_CONTROL)
STANDARD_RIGHTS_WRITE = (READ_CONTROL)
STANDARD_RIGHTS_EXECUTE = (READ_CONTROL)
STANDARD_RIGHTS_ALL = (2031616)
SPECIFIC_RIGHTS_ALL = (65535)
ACCESS_SYSTEM_SECURITY = (16777216)
MAXIMUM_ALLOWED = (33554432)
GENERIC_READ = (-2147483648)
GENERIC_WRITE = (1073741824)
GENERIC_EXECUTE = (536870912)
GENERIC_ALL = (268435456)

# file security permissions
FILE_READ_DATA=            ( 1 )
FILE_LIST_DIRECTORY=       ( 1 )
FILE_WRITE_DATA=           ( 2 )
FILE_ADD_FILE=             ( 2 )
FILE_APPEND_DATA=          ( 4 )
FILE_ADD_SUBDIRECTORY=     ( 4 )
FILE_CREATE_PIPE_INSTANCE= ( 4 )
FILE_READ_EA=              ( 8 )
FILE_WRITE_EA=             ( 16 )
FILE_EXECUTE=              ( 32 )
FILE_TRAVERSE=             ( 32 )
FILE_DELETE_CHILD=         ( 64 )
FILE_READ_ATTRIBUTES=      ( 128 )
FILE_WRITE_ATTRIBUTES=     ( 256 )
FILE_ALL_ACCESS=           (STANDARD_RIGHTS_REQUIRED | SYNCHRONIZE | 1023)
FILE_GENERIC_READ=         (STANDARD_RIGHTS_READ | FILE_READ_DATA | FILE_READ_ATTRIBUTES | FILE_READ_EA | SYNCHRONIZE)
FILE_GENERIC_WRITE=        (STANDARD_RIGHTS_WRITE | FILE_WRITE_DATA | FILE_WRITE_ATTRIBUTES | FILE_WRITE_EA | FILE_APPEND_DATA | SYNCHRONIZE)
FILE_GENERIC_EXECUTE=      (STANDARD_RIGHTS_EXECUTE | FILE_READ_ATTRIBUTES | FILE_EXECUTE | SYNCHRONIZE)


SECURITY_NULL_SID_AUTHORITY = (0,0,0,0,0,0)
SECURITY_WORLD_SID_AUTHORITY = (0,0,0,0,0,1)
SECURITY_LOCAL_SID_AUTHORITY = (0,0,0,0,0,2)
SECURITY_CREATOR_SID_AUTHORITY = (0,0,0,0,0,3)
SECURITY_NON_UNIQUE_AUTHORITY = (0,0,0,0,0,4)

SECURITY_NULL_RID                 = 0
SECURITY_WORLD_RID                = 0
SECURITY_LOCAL_RID                = 0X00000000

SECURITY_CREATOR_OWNER_RID        = 0
SECURITY_CREATOR_GROUP_RID        = 1

SECURITY_CREATOR_OWNER_SERVER_RID = 2
SECURITY_CREATOR_GROUP_SERVER_RID = 3


# NT well-known SIDs
SECURITY_NT_AUTHORITY = (0,0,0,0,0,5)

SECURITY_DIALUP_RID             = 1
SECURITY_NETWORK_RID            = 2
SECURITY_BATCH_RID              = 3
SECURITY_INTERACTIVE_RID        = 4
SECURITY_SERVICE_RID            = 6
SECURITY_ANONYMOUS_LOGON_RID    = 7
SECURITY_PROXY_RID              = 8
SECURITY_SERVER_LOGON_RID       = 9

SECURITY_LOGON_IDS_RID          = 5
SECURITY_LOGON_IDS_RID_COUNT    = 3

SECURITY_LOCAL_SYSTEM_RID       = 18

SECURITY_NT_NON_UNIQUE          = 21

SECURITY_BUILTIN_DOMAIN_RID     = 32

# well-known domain relative sub-authority values (RIDs)...
DOMAIN_USER_RID_ADMIN          = 500
DOMAIN_USER_RID_GUEST          = 501



# well-known groups ...

DOMAIN_GROUP_RID_ADMINS        = 512
DOMAIN_GROUP_RID_USERS         = 513
DOMAIN_GROUP_RID_GUESTS        = 514




# well-known aliases ...

DOMAIN_ALIAS_RID_ADMINS        = 544
DOMAIN_ALIAS_RID_USERS         = 545
DOMAIN_ALIAS_RID_GUESTS        = 546
DOMAIN_ALIAS_RID_POWER_USERS   = 547

DOMAIN_ALIAS_RID_ACCOUNT_OPS   = 548
DOMAIN_ALIAS_RID_SYSTEM_OPS    = 549
DOMAIN_ALIAS_RID_PRINT_OPS     = 550
DOMAIN_ALIAS_RID_BACKUP_OPS    = 551

DOMAIN_ALIAS_RID_REPLICATOR    = 552

SYSTEM_LUID                    = (999, 0)

# Group attributes

SE_GROUP_MANDATORY              = 1
SE_GROUP_ENABLED_BY_DEFAULT     = 2
SE_GROUP_ENABLED                = 4
SE_GROUP_OWNER                  = 8
SE_GROUP_LOGON_ID               = -1073741824


# User attributes
# (None yet defined.)

ACCESS_ALLOWED_ACE_TYPE          = 0
ACCESS_DENIED_ACE_TYPE           = 1
SYSTEM_AUDIT_ACE_TYPE            = 2
SYSTEM_ALARM_ACE_TYPE            = 3


#  The following are the inherit flags that go into the AceFlags field
#  of an Ace header.

OBJECT_INHERIT_ACE               = 1
CONTAINER_INHERIT_ACE            = 2
NO_PROPAGATE_INHERIT_ACE         = 4
INHERIT_ONLY_ACE                 = 8
VALID_INHERIT_FLAGS              = 15


SUCCESSFUL_ACCESS_ACE_FLAG       = 64
FAILED_ACCESS_ACE_FLAG           = 128

SE_OWNER_DEFAULTED               = 1
SE_GROUP_DEFAULTED               = 2
SE_DACL_PRESENT                  = 4
SE_DACL_DEFAULTED                = 8
SE_SACL_PRESENT                  = 16
SE_SACL_DEFAULTED                = 32
SE_SELF_RELATIVE                 = 32768


SE_PRIVILEGE_ENABLED_BY_DEFAULT = 1
SE_PRIVILEGE_ENABLED            = 2
SE_PRIVILEGE_USED_FOR_ACCESS    = -2147483648

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

TOKEN_ASSIGN_PRIMARY    = 1
TOKEN_DUPLICATE         = 2
TOKEN_IMPERSONATE       = 4
TOKEN_QUERY             = 8
TOKEN_QUERY_SOURCE      = 16
TOKEN_ADJUST_PRIVILEGES = 32
TOKEN_ADJUST_GROUPS     = 64
TOKEN_ADJUST_DEFAULT    = 128

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

