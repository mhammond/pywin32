/*
 *	EDKMDB.H
 *
 *	Microsoft Exchange Information Store
 *	Copyright (C) 1986-2002, Microsoft Corporation
 *
 *	Contains declarations of additional properties and interfaces
 *	offered by Microsoft Exchange Information Store
 */

#ifndef EDKMDB_INCLUDED
#define EDKMDB_INCLUDED

/*
 *	WARNING: Many of the property id values contained within this
 *	file are subject to change.	 For best results please use the
 *	literals declared here instead of the numerical values.
 */

#define pidStoreNonTransMin				0x0E40
#define pidExchangeXmitReservedMin		0x3FE0
#define pidExchangeNonXmitReservedMin	0x65E0
#define pidProfileMin					0x6600
#define pidStoreMin						0x6618
#define pidFolderMin					0x6638
#define pidMessageReadOnlyMin			0x6640
#define pidMessageWriteableMin			0x6658
#define pidAttachReadOnlyMin			0x666C
#define pidSpecialMin					0x6670
#define pidAdminMin						0x6690
#define pidSecureProfileMin				PROP_ID_SECURE_MIN
#define pidRenMsgFldMin					0x1080
#define pidLocalStoreInternalMin		0x6500		// Using a portion of the user-defined non-tranmittable prop for local store
#define pidLocalStoreInternalMax		0x65C0


/*------------------------------------------------------------------------
 *
 *	PROFILE properties
 *
 *	These are used in profiles which contain the Exchange Messaging
 *	Service.  These profiles contain a "global section" used to store
 *	common data, plus individual sections for the transport provider,
 *	one store provider for the user, one store provider for the public
 *	store, and one store provider for each additional mailbox the user
 *	has delegate access to.
 *
 *-----------------------------------------------------------------------*/

/* GUID of the global section */

#define pbGlobalProfileSectionGuid	"\x13\xDB\xB0\xC8\xAA\x05\x10\x1A\x9B\xB0\x00\xAA\x00\x2F\xC4\x5A"


/* Properties in the global section */

#define PR_PROFILE_VERSION				PROP_TAG( PT_LONG, pidProfileMin+0x00)
#define PR_PROFILE_CONFIG_FLAGS			PROP_TAG( PT_LONG, pidProfileMin+0x01)
#define PR_PROFILE_HOME_SERVER			PROP_TAG( PT_STRING8, pidProfileMin+0x02)
#define PR_PROFILE_HOME_SERVER_DN		PROP_TAG( PT_STRING8, pidProfileMin+0x12)
#define PR_PROFILE_HOME_SERVER_ADDRS	PROP_TAG( PT_MV_STRING8, pidProfileMin+0x13)
#define PR_PROFILE_USER					PROP_TAG( PT_STRING8, pidProfileMin+0x03)
#define PR_PROFILE_CONNECT_FLAGS		PROP_TAG( PT_LONG, pidProfileMin+0x04)
#define PR_PROFILE_TRANSPORT_FLAGS		PROP_TAG( PT_LONG, pidProfileMin+0x05)
#define PR_PROFILE_UI_STATE				PROP_TAG( PT_LONG, pidProfileMin+0x06)
#define PR_PROFILE_UNRESOLVED_NAME		PROP_TAG( PT_STRING8, pidProfileMin+0x07)
#define PR_PROFILE_UNRESOLVED_SERVER	PROP_TAG( PT_STRING8, pidProfileMin+0x08)
#define PR_PROFILE_BINDING_ORDER		PROP_TAG( PT_STRING8, pidProfileMin+0x09)
#define PR_PROFILE_MAX_RESTRICT			PROP_TAG( PT_LONG, pidProfileMin+0x0D)
#define PR_PROFILE_AB_FILES_PATH		PROP_TAG( PT_STRING8, pidProfileMin+0xE)
#define PR_PROFILE_OFFLINE_STORE_PATH	PROP_TAG( PT_STRING8, pidProfileMin+0x10)
#define PR_PROFILE_OFFLINE_INFO			PROP_TAG( PT_BINARY, pidProfileMin+0x11)
#define PR_PROFILE_ADDR_INFO			PROP_TAG( PT_BINARY, pidSpecialMin+0x17)
#define PR_PROFILE_OPTIONS_DATA			PROP_TAG( PT_BINARY, pidSpecialMin+0x19)
#define PR_PROFILE_SECURE_MAILBOX		PROP_TAG( PT_BINARY, pidSecureProfileMin + 0)
#define PR_DISABLE_WINSOCK				PROP_TAG( PT_LONG, pidProfileMin+0x18)
#define PR_PROFILE_AUTH_PACKAGE			PROP_TAG( PT_LONG, pidProfileMin+0x19)	// dup tag of PR_USER_ENTRYID
#define PR_PROFILE_RECONNECT_INTERVAL	PROP_TAG( PT_LONG, pidProfileMin+0x1a)	// dup tag of PR_USER_NAME
#define PR_PROFILE_SERVER_VERSION		PROP_TAG( PT_LONG, pidProfileMin+0x1b)

/* SE 233155 - MarkH: EMSABP DCR /*
/* Properties in the abp section - I got these values from AlecDun (Outlook team) */
#define PR_PROFILE_ABP_ALLOW_RECONNECT		PROP_TAG( PT_LONG, pidProfileMin+0x39)
#define PR_PROFILE_ABP_MTHREAD_TIMEOUT_SECS	PROP_TAG( PT_LONG, pidProfileMin+0x3A)

/* Properties passed through the Service Entry to the OST */
#define PR_OST_ENCRYPTION				PROP_TAG(PT_LONG, 0x6702)

/* Values for PR_OST_ENCRYPTION */
#define OSTF_NO_ENCRYPTION				((DWORD)0x80000000)
#define OSTF_COMPRESSABLE_ENCRYPTION	((DWORD)0x40000000)
#define OSTF_BEST_ENCRYPTION			((DWORD)0x20000000)

/* Properties in each profile section */

#define PR_PROFILE_OPEN_FLAGS			PROP_TAG( PT_LONG, pidProfileMin+0x09)
#define PR_PROFILE_TYPE					PROP_TAG( PT_LONG, pidProfileMin+0x0A)
#define PR_PROFILE_MAILBOX				PROP_TAG( PT_STRING8, pidProfileMin+0x0B)
#define PR_PROFILE_SERVER				PROP_TAG( PT_STRING8, pidProfileMin+0x0C)
#define PR_PROFILE_SERVER_DN			PROP_TAG( PT_STRING8, pidProfileMin+0x14)

/* Properties in the Public Folders section */

#define PR_PROFILE_FAVFLD_DISPLAY_NAME	PROP_TAG(PT_STRING8, pidProfileMin+0x0F)
#define PR_PROFILE_FAVFLD_COMMENT		PROP_TAG(PT_STRING8, pidProfileMin+0x15)
#define PR_PROFILE_ALLPUB_DISPLAY_NAME	PROP_TAG(PT_STRING8, pidProfileMin+0x16)
#define PR_PROFILE_ALLPUB_COMMENT		PROP_TAG(PT_STRING8, pidProfileMin+0x17)

/* Properties for Multiple Offline Address Book support (MOAB) */

#define PR_PROFILE_MOAB					PROP_TAG( PT_STRING8, pidSpecialMin + 0x0B )
#define PR_PROFILE_MOAB_GUID			PROP_TAG( PT_STRING8, pidSpecialMin + 0x0C )
#define PR_PROFILE_MOAB_SEQ				PROP_TAG( PT_LONG, pidSpecialMin + 0x0D )

// Property for setting a list of prop_ids to be excluded
// from the GetProps(NULL) call.
#define PR_GET_PROPS_EXCLUDE_PROP_ID_LIST	PROP_TAG( PT_BINARY, pidSpecialMin + 0x0E )

// Current value for PR_PROFILE_VERSION
#define PROFILE_VERSION						((ULONG)0x501)

// Bit values for PR_PROFILE_CONFIG_FLAGS

#define CONFIG_SERVICE						((ULONG)0x00000001)
#define CONFIG_SHOW_STARTUP_UI				((ULONG)0x00000002)
#define CONFIG_SHOW_CONNECT_UI				((ULONG)0x00000004)
#define CONFIG_PROMPT_FOR_CREDENTIALS		((ULONG)0x00000008)
#define CONFIG_NO_AUTO_DETECT				((ULONG)0x00000010)
#define CONFIG_OST_CACHE_ONLY				((ULONG)0x00000020)

// Bit values for PR_PROFILE_CONNECT_FLAGS

#define CONNECT_USE_ADMIN_PRIVILEGE			((ULONG)1)
#define CONNECT_NO_RPC_ENCRYPTION			((ULONG)2)
#define CONNECT_USE_SEPARATE_CONNECTION		((ULONG)4)
#define CONNECT_NO_UNDER_COVER_CONNECTION	((ULONG)8)
#define CONNECT_ANONYMOUS_ACCESS			((ULONG)16)
#define CONNECT_NO_NOTIFICATIONS			((ULONG)32)
#define CONNECT_NO_TABLE_NOTIFICATIONS		((ULONG)32) /*	BUGBUG: TEMPORARY */
#define CONNECT_NO_ADDRESS_RESOLUTION		((ULONG)64)
#define CONNECT_RESTORE_DATABASE				((ULONG)128)


// Bit values for PR_PROFILE_TRANSPORT_FLAGS

#define TRANSPORT_DOWNLOAD					((ULONG)1)
#define TRANSPORT_UPLOAD					((ULONG)2)

// Bit values for PR_PROFILE_OPEN_FLAGS

#define OPENSTORE_USE_ADMIN_PRIVILEGE		((ULONG)0x00000001)
#define OPENSTORE_PUBLIC					((ULONG)0x00000002)
#define OPENSTORE_HOME_LOGON				((ULONG)0x00000004)
#define OPENSTORE_TAKE_OWNERSHIP			((ULONG)0x00000008)
#define OPENSTORE_OVERRIDE_HOME_MDB			((ULONG)0x00000010)
#define OPENSTORE_TRANSPORT					((ULONG)0x00000020)
#define OPENSTORE_REMOTE_TRANSPORT			((ULONG)0x00000040)
#define OPENSTORE_INTERNET_ANONYMOUS		((ULONG)0x00000080)
#define OPENSTORE_ALTERNATE_SERVER			((ULONG)0x00000100) /* reserved for internal use */
#define OPENSTORE_IGNORE_HOME_MDB			((ULONG)0x00000200) /* reserved for internal use */
#define OPENSTORE_NO_MAIL					((ULONG)0x00000400) /* reserved for internal use */
#define OPENSTORE_OVERRIDE_LAST_MODIFIER	((ULONG)0x00000800)
#define OPENSTORE_CALLBACK_LOGON			((ULONG)0x00001000) /* reserved for internal use */
#define OPENSTORE_LOCAL						((ULONG)0x00002000)
#define OPENSTORE_FAIL_IF_NO_MAILBOX		((ULONG)0x00004000) /* reserved for internal use */
#define OPENSTORE_CACHE_EXCHANGE			((ULONG)0x00008000)
#define OPENSTORE_CLI_WITH_NAMEDPROP_FIX	((ULONG)0x00010000) /* reserved for internal use */
#define OPENSTORE_ENABLE_LAZY_LOGGING		((ULONG)0x00020000) /* reserved for internal use */
#define OPENSTORE_CLI_WITH_REPLID_GUID_MAPPING_FIX	((ULONG)0x00040000) /* reserved for internal use */
#define OPENSTORE_NO_LOCALIZATION			((ULONG)0x00080000) /* reserved for internal use */
#define OPENSTORE_RESTORE_DATABASE			((ULONG)0x00100000)
#define OPENSTORE_XFOREST_MOVE				((ULONG)0x00200000) /* reserved for internal use */


// Values for PR_PROFILE_TYPE

#define PROFILE_PRIMARY_USER				((ULONG)1)
#define PROFILE_DELEGATE					((ULONG)2)
#define PROFILE_PUBLIC_STORE				((ULONG)3)
#define PROFILE_SUBSCRIPTION				((ULONG)4)


/*------------------------------------------------------------------------
 *
 *	MDB object properties
 *
 *-----------------------------------------------------------------------*/

/* PR_MDB_PROVIDER GUID in stores table */

#define pbExchangeProviderPrimaryUserGuid	"\x54\x94\xA1\xC0\x29\x7F\x10\x1B\xA5\x87\x08\x00\x2B\x2A\x25\x17"
#define pbExchangeProviderDelegateGuid		"\x9e\xb4\x77\x00\x74\xe4\x11\xce\x8c\x5e\x00\xaa\x00\x42\x54\xe2"
#define pbExchangeProviderPublicGuid		"\x78\xb2\xfa\x70\xaf\xf7\x11\xcd\x9b\xc8\x00\xaa\x00\x2f\xc4\x5a"
#define pbExchangeProviderXportGuid			"\xa9\x06\x40\xe0\xd6\x93\x11\xcd\xaf\x95\x00\xaa\x00\x4a\x35\xc3"
#define pbExchangeProviderLocalStoreGuid	"\x2D\xE5\x6B\xA1\x64\x6E\x11\xd2\x8D\x4E\x00\xC0\x4F\xAE\x23\x71"
#define pbExchangeProviderPersistStoreGuid	"\x98\xA2\x3D\x67\x62\xCF\x4d\x34\x82\x79\xDB\xFA\x6A\x50\x8B\x31"

// All properties in this section are readonly

// Identity of store
	// All stores
#define PR_USER_ENTRYID					PROP_TAG( PT_BINARY, pidStoreMin+0x01)
#define PR_USER_NAME					PROP_TAG( PT_STRING8, pidStoreMin+0x02)

	// All mailbox stores
#define PR_MAILBOX_OWNER_ENTRYID		PROP_TAG( PT_BINARY, pidStoreMin+0x03)
#define PR_MAILBOX_OWNER_NAME			PROP_TAG( PT_STRING8, pidStoreMin+0x04)
#define PR_OOF_STATE					PROP_TAG( PT_BOOLEAN, pidStoreMin+0x05)

// Bug#255023 Provide quota information to MAPI clients to avoid large emails from ever reaching the server
#define PR_MAX_SUBMIT_MESSAGE_SIZE		PROP_TAG( PT_LONG, 0x666D)
#define PR_PROHIBIT_SEND_QUOTA			PROP_TAG( PT_LONG, 0x666E)

	// Public stores -- name of hierarchy server
#define PR_HIERARCHY_SERVER				PROP_TAG( PT_TSTRING, pidStoreMin+0x1B)

// Entryids of special folders
	// All mailbox stores
#define PR_SCHEDULE_FOLDER_ENTRYID		PROP_TAG( PT_BINARY, pidStoreMin+0x06)

	// All mailbox and gateway stores
#define PR_IPM_DAF_ENTRYID				PROP_TAG( PT_BINARY, pidStoreMin+0x07)

	// Public store
#define PR_NON_IPM_SUBTREE_ENTRYID				PROP_TAG( PT_BINARY, pidStoreMin+0x08)
#define PR_EFORMS_REGISTRY_ENTRYID				PROP_TAG( PT_BINARY, pidStoreMin+0x09)
#define PR_SPLUS_FREE_BUSY_ENTRYID				PROP_TAG( PT_BINARY, pidStoreMin+0x0A)
#define PR_OFFLINE_ADDRBOOK_ENTRYID				PROP_TAG( PT_BINARY, pidStoreMin+0x0B)
#define PR_NNTP_CONTROL_FOLDER_ENTRYID			PROP_TAG( PT_BINARY, pidSpecialMin+0x1B)
#define PR_EFORMS_FOR_LOCALE_ENTRYID			PROP_TAG( PT_BINARY, pidStoreMin+0x0C)
#define PR_FREE_BUSY_FOR_LOCAL_SITE_ENTRYID		PROP_TAG( PT_BINARY, pidStoreMin+0x0D)
#define PR_ADDRBOOK_FOR_LOCAL_SITE_ENTRYID		PROP_TAG( PT_BINARY, pidStoreMin+0x0E)
#define PR_NEWSGROUP_ROOT_FOLDER_ENTRYID		PROP_TAG( PT_BINARY, pidSpecialMin+0x1C)
#define PR_OFFLINE_MESSAGE_ENTRYID				PROP_TAG( PT_BINARY, pidStoreMin+0x0F)
#define PR_IPM_FAVORITES_ENTRYID				PROP_TAG( PT_BINARY, pidStoreMin+0x18)
#define PR_IPM_PUBLIC_FOLDERS_ENTRYID			PROP_TAG( PT_BINARY, pidStoreMin+0x19)
#define PR_FAVORITES_DEFAULT_NAME				PROP_TAG( PT_STRING8, pidStoreMin+0x1D)
#define PR_SYS_CONFIG_FOLDER_ENTRYID			PROP_TAG( PT_BINARY, pidStoreMin+0x1E)
#define PR_NNTP_ARTICLE_FOLDER_ENTRYID			PROP_TAG( PT_BINARY, pidSpecialMin+0x1A)
#define PR_EVENTS_ROOT_FOLDER_ENTRYID			PROP_TAG( PT_BINARY, pidSpecialMin+0xA)

	// Gateway stores
#define PR_GW_MTSIN_ENTRYID				PROP_TAG( PT_BINARY, pidStoreMin+0x10)
#define PR_GW_MTSOUT_ENTRYID			PROP_TAG( PT_BINARY, pidStoreMin+0x11)
#define PR_TRANSFER_ENABLED				PROP_TAG( PT_BOOLEAN, pidStoreMin+0x12)

// This property is preinitialized to 256 bytes of zeros
// GetProp on this property is guaranteed to RPC.  May be used
// to determine line speed of connection to server.
#define PR_TEST_LINE_SPEED				PROP_TAG( PT_BINARY, pidStoreMin+0x13)

// Used with OpenProperty to get interface, also on folders
#define PR_HIERARCHY_SYNCHRONIZER		PROP_TAG( PT_OBJECT, pidStoreMin+0x14)
#define PR_CONTENTS_SYNCHRONIZER		PROP_TAG( PT_OBJECT, pidStoreMin+0x15)
#define PR_COLLECTOR					PROP_TAG( PT_OBJECT, pidStoreMin+0x16)

// Used with OpenProperty to get interface for folders, messages, attachmentson
#define PR_FAST_TRANSFER				PROP_TAG( PT_OBJECT, pidStoreMin+0x17)

// Used with OpenProperty to get interface for store object
#define PR_CHANGE_ADVISOR				PROP_TAG( PT_OBJECT, pidStoreMin+0x1C)

// used to set the ics notification suppression guid
#define PR_CHANGE_NOTIFICATION_GUID		PROP_TAG( PT_CLSID, pidStoreMin+0x1F)

// This property is available on mailbox and public stores.	 If it exists
// and its value is TRUE, the store is connected to the offline store provider.
#define PR_STORE_OFFLINE				PROP_TAG( PT_BOOLEAN, pidStoreMin+0x1A)

// In transit state for store object.  This state is
// set when mail is being moved and it pauses mail delivery
// to the mail box
#define PR_IN_TRANSIT					PROP_TAG( PT_BOOLEAN, pidStoreMin)

// Writable only with Admin rights, available on public stores and folders
#define PR_REPLICATION_STYLE			PROP_TAG( PT_LONG, pidAdminMin)
#define PR_REPLICATION_SCHEDULE			PROP_TAG( PT_BINARY, pidAdminMin+0x01)
#define PR_REPLICATION_MESSAGE_PRIORITY PROP_TAG( PT_LONG, pidAdminMin+0x02)

// Writable only with Admin rights, available on public stores
#define PR_OVERALL_MSG_AGE_LIMIT		PROP_TAG( PT_LONG, pidAdminMin+0x03 )
#define PR_REPLICATION_ALWAYS_INTERVAL	PROP_TAG( PT_LONG, pidAdminMin+0x04 )
#define PR_REPLICATION_MSG_SIZE			PROP_TAG( PT_LONG, pidAdminMin+0x05 )

// default replication style=always interval (minutes)
#define STYLE_ALWAYS_INTERVAL_DEFAULT	(ULONG) 15

// default replication message size limit (KB)
#define REPLICATION_MESSAGE_SIZE_LIMIT_DEFAULT	(ULONG) 300

// Values for PR_REPLICATION_STYLE
#define STYLE_NEVER				(ULONG) 0	// never replicate
#define STYLE_NORMAL			(ULONG) 1	// use 84 byte schedule TIB
#define STYLE_ALWAYS			(ULONG) 2	// replicate at fastest rate
#define STYLE_DEFAULT			(ULONG) -1	// default value

/*------------------------------------------------------------------------
 *
 *	INCREMENTAL CHANGE SYNCHRONIZATION
 *	folder and message properties
 *
 *-----------------------------------------------------------------------*/

#define PR_SOURCE_KEY					PROP_TAG( PT_BINARY, pidExchangeNonXmitReservedMin+0x0)
#define PR_PARENT_SOURCE_KEY			PROP_TAG( PT_BINARY, pidExchangeNonXmitReservedMin+0x1)
#define PR_CHANGE_KEY					PROP_TAG( PT_BINARY, pidExchangeNonXmitReservedMin+0x2)
#define PR_PREDECESSOR_CHANGE_LIST		PROP_TAG( PT_BINARY, pidExchangeNonXmitReservedMin+0x3)

// msg-folder only property
// actual FID for a msg-folder row
// ptagFID for message rows
// ptagInstanceID for subfolder rows
#define PR_SOURCE_FID					PROP_TAG(PT_I8, pidStoreNonTransMin+0x1F)

/*------------------------------------------------------------------------
 *
 *	FOLDER object properties
 *
 *-----------------------------------------------------------------------*/

// folders table property used by PKM to define the catalog guid for content
// indexing and searching; doubles as index enable/disable
#define PR_CATALOG					PROP_TAG(PT_BINARY, pidStoreNonTransMin+0x1B)

// Is CI searching enabled on this folder?
#define PR_CI_SEARCH_ENABLED		PROP_TAG(PT_BOOLEAN, pidStoreNonTransMin+0x1C)

// Is notification-based indexing enabled on this folder?
#define PR_CI_NOTIFICATION_ENABLED	PROP_TAG(PT_BOOLEAN, pidStoreNonTransMin+0x1D)

// Max number of cached view allowed
#define PR_MAX_CACHED_VIEWS			PROP_TAG(PT_LONG, pidStoreNonTransMin+0x28)

// Max number of indices allowed
// Review : this ptag is used for PR_MIME_HANDLER_CLASSIDS, but because the context
// is different I am reusing it here.
#define PR_MAX_INDICES				PROP_TAG(PT_LONG, pidStoreNonTransMin+0x1E)

// folders table property containing list of guid/restriction pairs
#define PR_IMPLIED_RESTRICTIONS		PROP_TAG( PT_MV_BINARY, pidSpecialMin+0x0F)

// Read only, available on all folders
#define PR_FOLDER_CHILD_COUNT		PROP_TAG( PT_LONG, pidFolderMin)
#define PR_RIGHTS					PROP_TAG( PT_LONG, pidFolderMin+0x01)
#define PR_ACL_TABLE				PROP_TAG( PT_OBJECT, pidExchangeXmitReservedMin)
#define PR_RULES_TABLE				PROP_TAG( PT_OBJECT, pidExchangeXmitReservedMin+0x1)
#define PR_HAS_RULES				PROP_TAG( PT_BOOLEAN, pidFolderMin+0x02)
#define PR_HAS_MODERATOR_RULES		PROP_TAG( PT_BOOLEAN, pidFolderMin+0x07 )

//Read only, available only for public folders
#define PR_ADDRESS_BOOK_ENTRYID		PROP_TAG( PT_BINARY, pidFolderMin+0x03)

//Writable, available on folders in all stores
#define PR_ACL_DATA					PROP_TAG( PT_BINARY, pidExchangeXmitReservedMin)
#define PR_RULES_DATA				PROP_TAG( PT_BINARY, pidExchangeXmitReservedMin+0x1)
#define PR_EXTENDED_ACL_DATA		PROP_TAG( PT_BINARY, pidExchangeXmitReservedMin+0x1E)
#define PR_FOLDER_DESIGN_FLAGS		PROP_TAG( PT_LONG, pidExchangeXmitReservedMin+0x2)
#define PR_DESIGN_IN_PROGRESS		PROP_TAG( PT_BOOLEAN, pidExchangeXmitReservedMin+0x4)
#define PR_SECURE_ORIGINATION		PROP_TAG( PT_BOOLEAN, pidExchangeXmitReservedMin+0x5)

//Writable, available only for public folders
#define PR_PUBLISH_IN_ADDRESS_BOOK		PROP_TAG( PT_BOOLEAN, pidExchangeXmitReservedMin+0x6)
#define PR_RESOLVE_METHOD				PROP_TAG( PT_LONG,	pidExchangeXmitReservedMin+0x7)
#define PR_ADDRESS_BOOK_DISPLAY_NAME	PROP_TAG( PT_TSTRING, pidExchangeXmitReservedMin+0x8)

//Writable, used to indicate locale id for eforms registry subfolders
#define PR_EFORMS_LOCALE_ID			PROP_TAG( PT_LONG, pidExchangeXmitReservedMin+0x9)

// Writable only with Admin rights, available only for public folders
#define PR_REPLICA_LIST				PROP_TAG( PT_BINARY, pidAdminMin+0x8)
#define PR_OVERALL_AGE_LIMIT		PROP_TAG( PT_LONG, pidAdminMin+0x9)

// Newsgroup related properties. Writable only with Admin rights.
#define PR_IS_NEWSGROUP_ANCHOR		PROP_TAG( PT_BOOLEAN, pidAdminMin+0x06)
#define PR_IS_NEWSGROUP				PROP_TAG( PT_BOOLEAN, pidAdminMin+0x07)
#define PR_NEWSGROUP_COMPONENT		PROP_TAG( PT_STRING8, pidAdminMin+0x15)
#define PR_INTERNET_NEWSGROUP_NAME	PROP_TAG( PT_STRING8, pidAdminMin+0x17)
#define PR_NEWSFEED_INFO			PROP_TAG( PT_BINARY,  pidAdminMin+0x16)

// Newsgroup related property.
#define PR_PREVENT_MSG_CREATE		PROP_TAG( PT_BOOLEAN, pidExchangeNonXmitReservedMin+0x14)

// IMAP internal date
#define PR_IMAP_INTERNAL_DATE		PROP_TAG( PT_SYSTIME, pidExchangeNonXmitReservedMin+0x15)

// Virtual properties to refer to Newsfeed DNs. Cannot get/set these on
// any object. Supported currently only in specifying restrictions.
#define PR_INBOUND_NEWSFEED_DN		PROP_TAG( PT_STRING8, pidSpecialMin+0x1D)
#define PR_OUTBOUND_NEWSFEED_DN		PROP_TAG( PT_STRING8, pidSpecialMin+0x1E)

// Used for controlling content conversion in NNTP
#define PR_INTERNET_CHARSET			PROP_TAG( PT_TSTRING, pidAdminMin+0xA)

//PR_RESOLVE_METHOD values
#define RESOLVE_METHOD_DEFAULT					((LONG)0) // default handling attach conflicts
#define RESOLVE_METHOD_LAST_WRITER_WINS			((LONG)1) // the last writer will win conflict
#define RESOLVE_METHOD_NO_CONFLICT_NOTIFICATION ((LONG)2) // no conflict notif

//Read only, available only for public folder favorites
#define PR_PUBLIC_FOLDER_ENTRYID	PROP_TAG( PT_BINARY, pidFolderMin+0x04)

//Read only. changes everytime a subfolder is created or deleted
#define PR_HIERARCHY_CHANGE_NUM		PROP_TAG( PT_LONG, pidFolderMin+0x06)

// For IFS/OLEDB to set and get user sid in LOGON
#define PR_USER_SID					PROP_TAG(PT_BINARY, PROP_ID(ptagSearchState)) // pidInternalNoAccessNonTransMin+0x23)
#define PR_CREATOR_TOKEN			PR_USER_SID


/*------------------------------------------------------------------------
 *
 *	MESSAGE object properties
 *
 *-----------------------------------------------------------------------*/

// Read only, automatically set on all messages in all stores
#define PR_HAS_NAMED_PROPERTIES			PROP_TAG(PT_BOOLEAN, pidMessageReadOnlyMin+0x0A)

// Read only but outside the provider specific range for replication thru GDK-GWs
#define PR_CREATOR_NAME					PROP_TAG(PT_TSTRING, pidExchangeXmitReservedMin+0x18)
#define PR_CREATOR_ENTRYID				PROP_TAG(PT_BINARY, pidExchangeXmitReservedMin+0x19)
#define PR_LAST_MODIFIER_NAME			PROP_TAG(PT_TSTRING, pidExchangeXmitReservedMin+0x1A)
#define PR_LAST_MODIFIER_ENTRYID		PROP_TAG(PT_BINARY, pidExchangeXmitReservedMin+0x1B)
#define PR_REPLY_RECIPIENT_SMTP_PROXIES PROP_TAG(PT_TSTRING, pidExchangeXmitReservedMin+0x1C)

// Read only, appears on messages which have DAM's pointing to them
#define PR_HAS_DAMS						PROP_TAG( PT_BOOLEAN, pidExchangeXmitReservedMin+0x0A)
#define PR_RULE_TRIGGER_HISTORY			PROP_TAG( PT_BINARY, pidExchangeXmitReservedMin+0x12)
#define PR_MOVE_TO_STORE_ENTRYID		PROP_TAG( PT_BINARY, pidExchangeXmitReservedMin+0x13)
#define PR_MOVE_TO_FOLDER_ENTRYID		PROP_TAG( PT_BINARY, pidExchangeXmitReservedMin+0x14)

// Read only, available only on messages in the public store
#define PR_REPLICA_SERVER				PROP_TAG(PT_TSTRING, pidMessageReadOnlyMin+0x04)
#define PR_REPLICA_VERSION				PROP_TAG(PT_I8, pidMessageReadOnlyMin+0x0B)

// SID versions of standard messaging properties
#define PR_CREATOR_SID								PROP_TAG(PT_BINARY, pidStoreNonTransMin+0x18)
#define PR_LAST_MODIFIER_SID						PROP_TAG(PT_BINARY, pidStoreNonTransMin+0x19)
#define PR_SENDER_SID								PROP_TAG(PT_BINARY, pidStoreNonTransMin+0x0d)
#define PR_SENT_REPRESENTING_SID					PROP_TAG(PT_BINARY, pidStoreNonTransMin+0x0e)
#define PR_ORIGINAL_SENDER_SID						PROP_TAG(PT_BINARY, pidStoreNonTransMin+0x0f)
#define PR_ORIGINAL_SENT_REPRESENTING_SID			PROP_TAG(PT_BINARY, pidStoreNonTransMin+0x10)
#define PR_READ_RECEIPT_SID							PROP_TAG(PT_BINARY, pidStoreNonTransMin+0x11)
#define PR_REPORT_SID								PROP_TAG(PT_BINARY, pidStoreNonTransMin+0x12)
#define PR_ORIGINATOR_SID							PROP_TAG(PT_BINARY, pidStoreNonTransMin+0x13)
#define PR_REPORT_DESTINATION_SID					PROP_TAG(PT_BINARY, pidStoreNonTransMin+0x14)
#define PR_ORIGINAL_AUTHOR_SID						PROP_TAG(PT_BINARY, pidStoreNonTransMin+0x15)
#define PR_RECEIVED_BY_SID							PROP_TAG(PT_BINARY, pidStoreNonTransMin+0x16)
#define PR_RCVD_REPRESENTING_SID					PROP_TAG(PT_BINARY, pidStoreNonTransMin+0x17)

#define PR_TRUST_SENDER_NO							0x00000000L
#define PR_TRUST_SENDER_YES							0x00000001L
#define PR_TRUST_SENDER								PROP_TAG(PT_LONG,	pidStoreNonTransMin+0x39)

// XML versions of SID properties
#define PR_CREATOR_SID_AS_XML						PROP_TAG(PT_TSTRING, pidStoreNonTransMin+0x2C)
#define PR_LAST_MODIFIER_SID_AS_XML					PROP_TAG(PT_TSTRING, pidStoreNonTransMin+0x2D)
#define PR_SENDER_SID_AS_XML						PROP_TAG(PT_TSTRING, pidStoreNonTransMin+0x2E)
#define PR_SENT_REPRESENTING_SID_AS_XML				PROP_TAG(PT_TSTRING, pidStoreNonTransMin+0x2F)
#define PR_ORIGINAL_SENDER_SID_AS_XML				PROP_TAG(PT_TSTRING, pidStoreNonTransMin+0x30)
#define PR_ORIGINAL_SENT_REPRESENTING_SID_AS_XML	PROP_TAG(PT_TSTRING, pidStoreNonTransMin+0x31)
#define PR_READ_RECEIPT_SID_AS_XML					PROP_TAG(PT_TSTRING, pidStoreNonTransMin+0x32)
#define PR_REPORT_SID_AS_XML						PROP_TAG(PT_TSTRING, pidStoreNonTransMin+0x33)
#define PR_ORIGINATOR_SID_AS_XML					PROP_TAG(PT_TSTRING, pidStoreNonTransMin+0x34)
#define PR_REPORT_DESTINATION_SID_AS_XML			PROP_TAG(PT_TSTRING, pidStoreNonTransMin+0x35)
#define PR_ORIGINAL_AUTHOR_SID_AS_XML				PROP_TAG(PT_TSTRING, pidStoreNonTransMin+0x36)
#define PR_RECEIVED_BY_SID_AS_XML					PROP_TAG(PT_TSTRING, pidStoreNonTransMin+0x37)
#define PR_RCVD_REPRESENTING_SID_AS_XML				PROP_TAG(PT_TSTRING, pidStoreNonTransMin+0x38)


// those two are pseudo-properties on folder. calling OFOLD::EcGetProps(PR_RESERVE_RANGE_OF_IDS) is
// equivalent to calling EcGetLocalRepIdsOp(), calling OFOLD::EcSetProps(PR_MERGE_MIDSET_DELETED)
// is equivalen to calling OFOLD::EcSetLocalRepMidsetDeleted()
#define PR_MERGE_MIDSET_DELETED						PROP_TAG(PT_BINARY, pidStoreNonTransMin+0x3a)  // 0x0E7A0102
#define PR_RESERVE_RANGE_OF_IDS						PROP_TAG(PT_BINARY, pidStoreNonTransMin+0x3b)  // 0x0E7B0102

// computed message property (read only)
// 44 byte binary property - used by PKM as globally unique message key
// 22 bytes of global ID for FID
// 22 bytes of global ID for VID
#define PR_FID_VID						PROP_TAG(PT_BINARY, pidMessageReadOnlyMin+0x0C)
#define PR_FID_MID						PR_FID_VID	 //NSK : temporary to allow transition

// message property - read only, xref ID in global ID format - used by PKM
#define PR_ORIGIN_ID					PROP_TAG( PT_BINARY, pidMessageReadOnlyMin+0x0D)

// computed message property used in search folders to determine quality of
// search hit match
// NOTE: ptag.h consumers, see also ptagMsgFolderTemplateRes3
#define PR_RANK							PROP_TAG( PT_LONG, pidAdminMin+0x82 )

// msg-folder property, read only
// value is PR_MSG_DELIVERY_TIME if it exists, else PR_CREATION_TIME
// used as the default sort time when subfolder rows are returned in views
#define PR_MSG_FOLD_TIME				PROP_TAG( PT_SYSTIME, pidMessageReadOnlyMin+0x14)
#define PR_ICS_CHANGE_KEY				PROP_TAG( PT_BINARY, pidMessageReadOnlyMin+0x15)

#define PR_DEFERRED_SEND_NUMBER			PROP_TAG( PT_LONG, pidExchangeXmitReservedMin+0xB)
#define PR_DEFERRED_SEND_UNITS			PROP_TAG( PT_LONG, pidExchangeXmitReservedMin+0xC)
#define PR_EXPIRY_NUMBER				PROP_TAG( PT_LONG, pidExchangeXmitReservedMin+0xD)
#define PR_EXPIRY_UNITS					PROP_TAG( PT_LONG, pidExchangeXmitReservedMin+0xE)

// Writeable, deferred send time
#define PR_DEFERRED_SEND_TIME			PROP_TAG( PT_SYSTIME, pidExchangeXmitReservedMin+0xF)

//Writeable, intended for both folders and messages in gateway mailbox
#define PR_GW_ADMIN_OPERATIONS			PROP_TAG( PT_LONG, pidMessageWriteableMin)

//Writeable, used for DMS messages
#define PR_P1_CONTENT					PROP_TAG( PT_BINARY, 0x1100)
#define PR_P1_CONTENT_TYPE				PROP_TAG( PT_BINARY, 0x1101)

// Properties on deferred action messages
#define PR_CLIENT_ACTIONS				PROP_TAG(PT_BINARY, pidMessageReadOnlyMin+0x5)
#define PR_DAM_ORIGINAL_ENTRYID			PROP_TAG(PT_BINARY, pidMessageReadOnlyMin+0x6)
#define PR_DAM_BACK_PATCHED				PROP_TAG( PT_BOOLEAN, pidMessageReadOnlyMin+0x7)

// Properties on deferred action error messages
#define PR_RULE_ERROR					PROP_TAG(PT_LONG, pidMessageReadOnlyMin+0x8)
#define PR_RULE_ACTION_TYPE				PROP_TAG(PT_LONG, pidMessageReadOnlyMin+0x9)
#define PR_RULE_ACTION_NUMBER			PROP_TAG(PT_LONG, pidMessageReadOnlyMin+0x10)
#define PR_RULE_FOLDER_ENTRYID			PROP_TAG(PT_BINARY, pidMessageReadOnlyMin+0x11)

// Mime representation of a message.
// Defined as 3 different types for convenience. Will be stored as file handle
// internally.
#define PR_INTERNET_CONTENT				PROP_TAG(PT_BINARY, pidMessageWriteableMin+0x1)
#define PR_INTERNET_CONTENT_HANDLE		PROP_TAG(PT_FILE_HANDLE, pidMessageWriteableMin+0x1)
#define PR_INTERNET_CONTENT_EA			PROP_TAG(PT_FILE_EA, pidMessageWriteableMin+0x1)

// Dot-stuff state property on message
#define PR_DOTSTUFF_STATE				PROP_TAG(PT_LONG, pidUserNonTransmitMin+0x1)

// Raw byte count of mime stream, if mime exists.
#define PR_MIME_SIZE					PROP_TAG(PT_LONG, 0x6746)
#define PR_MIME_SIZE_EXTENDED			PROP_TAG(PT_I8, 0x6746)

// Raw byte count of ptagInternetContent, whether it is a mime message
// or freedoc using OURL
#define PR_FILE_SIZE					PROP_TAG(PT_LONG, 0x6747)
#define PR_FILE_SIZE_EXTENDED			PROP_TAG(PT_I8, 0x6747)

// Sender's editor format
#define PR_MSG_EDITOR_FORMAT			PROP_TAG( PT_LONG, 0x5909 )

#define EDITOR_FORMAT_DONTKNOW			((ULONG)0)
#define EDITOR_FORMAT_PLAINTEXT			((ULONG)1)
#define EDITOR_FORMAT_HTML				((ULONG)2)
#define EDITOR_FORMAT_RTF				((ULONG)3)

#ifdef	pidInternalWriteableNonTransMin
#if pidInternalWritableNonTranMin - 0x6740
#pragma error("pidInternalWritableNonTransMin definition has changed, must change definition of PR_MIME_SIZE")
#endif
#endif

// State of this inid as far as conversion is concerned.
// Reusing mailbox table property
#define PR_CONVERSION_STATE				PROP_TAG(PT_LONG, PROP_ID(ptagAdminNickName))

// Property to represent native html content - assumed to be in the internet
// codepage as determined by PR_INTERNET_CPID
//
#define PR_HTML						   PROP_TAG( PT_BINARY, PROP_ID( PR_BODY_HTML ) )

// computed property used for moderated folder rule
// its an EntryId whose value is:
// ptagSenderEntryId on delivery
// LOGON::PbUserEntryId() for all other cases (move/copy/post)
#define PR_ACTIVE_USER_ENTRYID			PROP_TAG(PT_BINARY, pidMessageReadOnlyMin+0x12)

// Property on conflict notification indicating entryid of conflicting object
#define PR_CONFLICT_ENTRYID				PROP_TAG(PT_BINARY, pidExchangeXmitReservedMin+0x10)

// Property on messages to indicate the language client used to create this message
#define PR_MESSAGE_LOCALE_ID			PROP_TAG(PT_LONG, pidExchangeXmitReservedMin+0x11)
#define PR_MESSAGE_CODEPAGE				PROP_TAG( PT_LONG, pidExchangeXmitReservedMin+0x1D)

// Properties on Quota warning messages to indicate Storage quota and Excess used
#define PR_STORAGE_QUOTA_LIMIT			PROP_TAG(PT_LONG, pidExchangeXmitReservedMin+0x15)
#define PR_EXCESS_STORAGE_USED			PROP_TAG(PT_LONG, pidExchangeXmitReservedMin+0x16)
#define PR_SVR_GENERATING_QUOTA_MSG		PROP_TAG(PT_TSTRING, pidExchangeXmitReservedMin+0x17)

// Property affixed by delegation rule and deleted on forwards
#define PR_DELEGATED_BY_RULE			PROP_TAG( PT_BOOLEAN, pidExchangeXmitReservedMin+0x3)

// Message status bit used to indicate message is in conflict
#define MSGSTATUS_IN_CONFLICT			((ULONG) 0x800)

// Message status bit used to indicate the IMAP4 $MDNSent flag
#define MSGSTATUS_MDNSENT			((ULONG) 0x4000)

// used to indicate how much X400 private extension data is present: none, just the
// message level, or both the message and recipient levels
// !!The high order byte of this ULONG is reserved.!!
#define ENV_BLANK						((ULONG)0x00000000)
#define ENV_RECIP_NUM					((ULONG)0x00000001)
#define ENV_MSG_EXT						((ULONG)0x00000002)
#define ENV_RECIP_EXT					((ULONG)0x00000004)



#define PR_X400_ENVELOPE_TYPE			PROP_TAG(PT_LONG, pidMessageReadOnlyMin+0x13)
#define X400_ENV_PLAIN					(ENV_BLANK) // no extension
#define X400_ENV_VALID_RECIP			(ENV_RECIP_NUM | ENV_MSG_EXT)					// just the message level extension
#define X400_ENV_FULL_EXT				(ENV_RECIP_NUM | ENV_MSG_EXT | ENV_RECIP_EXT)	// both message and recipient levels

//
// bitmask that indicates whether RN, NRN, DR, NDR, OOF, Auto-Reply should be suppressed
//
#define AUTO_RESPONSE_SUPPRESS_DR			((ULONG)0x00000001)
#define AUTO_RESPONSE_SUPPRESS_NDR			((ULONG)0x00000002)
#define AUTO_RESPONSE_SUPPRESS_RN			((ULONG)0x00000004)
#define AUTO_RESPONSE_SUPPRESS_NRN			((ULONG)0x00000008)
#define AUTO_RESPONSE_SUPPRESS_OOF			((ULONG)0x00000010)
#define AUTO_RESPONSE_SUPPRESS_AUTO_REPLY	((ULONG)0x00000020)

// raid 91101 - Flag indicates No RFC821 From field
#define AUTO_RESPONSE_SUPPRESS_NORFC821FROM ((ULONG)0x00000040)

#define PR_AUTO_RESPONSE_SUPPRESS		PROP_TAG(PT_LONG, pidExchangeXmitReservedMin - 0x01)
#define PR_INTERNET_CPID				PROP_TAG(PT_LONG, pidExchangeXmitReservedMin - 0x02)

#define PR_SYNCEVENT_FIRED				PROP_TAG(PT_BOOLEAN, pidMessageReadOnlyMin + 0x0F)

/*------------------------------------------------------------------------
 *
 *	ATTACHMENT object properties
 *
 *-----------------------------------------------------------------------*/

// Appears on attachments to a message marked to be in conflict.  Identifies
// those attachments which are conflicting versions of the top level message
#define PR_IN_CONFLICT					PROP_TAG(PT_BOOLEAN, pidAttachReadOnlyMin)


/*------------------------------------------------------------------------
 *
 *	DUMPSTER properties
 *
 *-----------------------------------------------------------------------*/

// Indicates when a message, folder, or mailbox has been deleted.
// (Read only, non transmittable property).
#define PR_DELETED_ON							PROP_TAG(PT_SYSTIME, pidSpecialMin+0x1F)

// Read-only folder properties which indicate the number of messages, and child folders
// that have been "soft" deleted in this folder (and the time the first message was deleted).
#define PR_DELETED_MSG_COUNT					PROP_TAG(PT_LONG, pidFolderMin+0x08)
#define PR_DELETED_ASSOC_MSG_COUNT				PROP_TAG(PT_LONG, pidFolderMin+0x0B)
#define PR_DELETED_FOLDER_COUNT					PROP_TAG(PT_LONG, pidFolderMin + 0x09)
#define PR_OLDEST_DELETED_ON					PROP_TAG(PT_SYSTIME, pidFolderMin + 0x0A)

// Total size of all soft deleted messages
#define PR_DELETED_MESSAGE_SIZE_EXTENDED		PROP_TAG(PT_I8, pidAdminMin+0xB)

// Total size of all normal soft deleted messages
#define PR_DELETED_NORMAL_MESSAGE_SIZE_EXTENDED PROP_TAG(PT_I8, pidAdminMin+0xC)

// Total size of all associated soft deleted messages
#define PR_DELETED_ASSOC_MESSAGE_SIZE_EXTENDED	PROP_TAG(PT_I8, pidAdminMin+0xD)

// This property controls the retention age limit (minutes) for the Private/Public MDB,
// Mailbox (private only), or Folder (public).
// Note - the Folder/Mailbox retention, if set, overrides the MDB retention.
#define PR_RETENTION_AGE_LIMIT					PROP_TAG(PT_LONG, pidAdminMin+0x34)

// This property controls if we maintain per user read/unread for a public
// folder. By default (if this property is missing or set to FALSE) we will
// maintain per user read/unread.
#define PR_DISABLE_PERUSER_READ					PROP_TAG(PT_BOOLEAN, pidAdminMin+0x35)

// This property is set by JET after a full backup has occurred.
// It is used to determine whether or not messages and folders can be "hard" deleted
// before a full backup has captured the last modification to the object.
#define PR_LAST_FULL_BACKUP						PROP_TAG(PT_SYSTIME, pidSpecialMin+0x15)

/*------------------------------------------------------------------------
 *	URL related properties
 *-----------------------------------------------------------------------*/
// This is read only property.
#define PR_URL_NAME						PROP_TAG(PT_TSTRING, pidAdminMin+0x77)	   //0x6707
#define PR_URL_NAME_A					PROP_TAG(PT_STRING8, pidAdminMin+0x77)
#define PR_URL_NAME_W					PROP_TAG(PT_UNICODE, pidAdminMin+0x77)

// This is a read-write property.
#define PR_URL_COMP_NAME				PROP_TAG(PT_TSTRING, pidRenMsgFldMin+0x73)
#define PR_URL_COMP_NAME_A				PROP_TAG(PT_STRING8, pidRenMsgFldMin+0x73)
#define PR_URL_COMP_NAME_W				PROP_TAG(PT_UNICODE, pidRenMsgFldMin+0x73)

// this is a read-only property
#define PR_PARENT_URL_NAME				PROP_TAG(PT_TSTRING, pidAdminMin+0x7D) // 0x670d
#define PR_PARENT_URL_NAME_A			PROP_TAG(PT_STRING8, pidAdminMin+0x7D)
#define PR_PARENT_URL_NAME_W			PROP_TAG(PT_UNICODE, pidAdminMin+0x7D)

// read-only property
#define PR_FLAT_URL_NAME				PROP_TAG(PT_TSTRING, pidAdminMin+0x7E) // 0x670e
#define PR_FLAT_URL_NAME_A				PROP_TAG(PT_STRING8, pidAdminMin+0x7E)
#define PR_FLAT_URL_NAME_W				PROP_TAG(PT_UNICODE, pidAdminMin+0x7E)

// read-only property
#define PR_SRC_URL_NAME					PROP_TAG(PT_TSTRING, pidAdminMin+0x7F) // 0x670f
#define PR_SRC_URL_NAME_A				PROP_TAG(PT_STRING8, pidAdminMin+0x7F)
#define PR_SRC_URL_NAME_W				PROP_TAG(PT_UNICODE, pidAdminMin+0x7F)


// Constant wstring to specify URL with fid</mid> encoded directly in the URL
// For example, URL L"/~FlatUrlSpace/1-401" will refer to folder with FID 1-401
// and URL L"/~FlatUrlSpace/1-401/2-8fb" will refer to message with MID 2-8fb
// in that folder.
// But remember that the FID/MID have to be long term, i.e GUID-Globcnt,
// the replid used above is simply to explain the idea simpler.
#define WSZ_URL_FLAT_FOLDER_SPACE		L"/-FlatUrlSpace-/"
#define cwchUrlFlatFolderSpace			16

// Property that defines whether a folder is secure or not
#define PR_SECURE_IN_SITE				PROP_TAG(PT_BOOLEAN, pidAdminMin+0xE)

// PR_LOCAL_COMMIT_TIME is maintained on folders and messages. It is the
// FileTime when the object was modified last on the given MDB. It is updated
// any time the object is modified (including replicated in change).This is
// strictly computed, non-transmittable and non-copyable.
#define PR_LOCAL_COMMIT_TIME			PROP_TAG(PT_SYSTIME, pidAdminMin+0x79)

// PR_LOCAL_COMMIT_TIME_MAX is maintained on folders only.
// It is >= PR_LOCAL_COMMIT_TIME of all messages in the folder. It is updated
// any time any message in the folder is modified. This is strictly computed,
// non-transmittable and non-copyable.
#define PR_LOCAL_COMMIT_TIME_MAX		PROP_TAG(PT_SYSTIME, pidAdminMin+0x7a)

// PR_DELETED_COUNT_TOTAL is maintained on folders only.
// It is the total number of messages deleted in this folder from the beginning
// of time (well, rather from the time this feature is checked-in, folders from
// old servers that are upgraded will start with 0). If the count overflows the
// 4 bytes, it will start again at 0. This is updated whenever a message in the
// folder is deleted. This is strictly computed, non-transmitabble and
// non-copyable.
#define PR_DELETED_COUNT_TOTAL			PROP_TAG(PT_LONG, pidAdminMin+0x7b)

// PR_AUTO_RESET is maintained on messages only. Its PT_MV_CLSID and is deleted
// (by the store) anytime a message is saved, if it has not been
// explicitly set on the message between the time it was opened and saved
// (by the user/app that opened and later saved the message).
// It is intended to be used by async callback agents.
#define PR_AUTO_RESET				PROP_TAG(PT_MV_CLSID, pidAdminMin+0x7c)

/*------------------------------------------------------------------------
 *
 *	TABLE object properties
 *
 *	Id Range: 0x662F-0x662F
 *
 *-----------------------------------------------------------------------*/

//This property can be used in a contents table to get PR_ENTRYID returned
//as a long term entryid instead of a short term entryid.
#define PR_LONGTERM_ENTRYID_FROM_TABLE	PROP_TAG(PT_BINARY, pidSpecialMin)

// This is read only property that is used for contents tables that include
// subfolder entries.
#define PR_SUBFOLDER					PROP_TAG(PT_BOOLEAN, pidAdminMin+0x78)


/*------------------------------------------------------------------------
 *
 *	Gateway "MTE" ENVELOPE properties
 *
 *	Id Range:  0x66E0-0x66FF
 *
 *-----------------------------------------------------------------------*/

#define PR_ORIGINATOR_NAME				PROP_TAG( PT_TSTRING, pidMessageWriteableMin+0x3)
#define PR_ORIGINATOR_ADDR				PROP_TAG( PT_TSTRING, pidMessageWriteableMin+0x4)
#define PR_ORIGINATOR_ADDRTYPE			PROP_TAG( PT_TSTRING, pidMessageWriteableMin+0x5)
#define PR_ORIGINATOR_ENTRYID			PROP_TAG( PT_BINARY, pidMessageWriteableMin+0x6)
#define PR_ARRIVAL_TIME					PROP_TAG( PT_SYSTIME, pidMessageWriteableMin+0x7)
#define PR_TRACE_INFO					PROP_TAG( PT_BINARY, pidMessageWriteableMin+0x8)
#define PR_INTERNAL_TRACE_INFO			PROP_TAG( PT_BINARY, pidMessageWriteableMin+0x12)
#define PR_SUBJECT_TRACE_INFO			PROP_TAG( PT_BINARY, pidMessageWriteableMin+0x9)
#define PR_RECIPIENT_NUMBER				PROP_TAG( PT_LONG, pidMessageWriteableMin+0xA)
#define PR_MTS_SUBJECT_ID				PROP_TAG(PT_BINARY, pidMessageWriteableMin+0xB)
#define PR_REPORT_DESTINATION_NAME		PROP_TAG(PT_TSTRING, pidMessageWriteableMin+0xC)
#define PR_REPORT_DESTINATION_ENTRYID	PROP_TAG(PT_BINARY, pidMessageWriteableMin+0xD)
#define PR_CONTENT_SEARCH_KEY			PROP_TAG(PT_BINARY, pidMessageWriteableMin+0xE)
#define PR_FOREIGN_ID					PROP_TAG(PT_BINARY, pidMessageWriteableMin+0xF)
#define PR_FOREIGN_REPORT_ID			PROP_TAG(PT_BINARY, pidMessageWriteableMin+0x10)
#define PR_FOREIGN_SUBJECT_ID			PROP_TAG(PT_BINARY, pidMessageWriteableMin+0x11)
#define PR_PROMOTE_PROP_ID_LIST			PROP_TAG(PT_BINARY, pidMessageWriteableMin+0x13)
#define PR_MTS_ID						PR_MESSAGE_SUBMISSION_ID
#define PR_MTS_REPORT_ID				PR_MESSAGE_SUBMISSION_ID

/*------------------------------------------------------------------------
 *
 *	Trace properties format
 *		PR_TRACE_INFO
 *		PR_INTERNAL_TRACE_INFO
 *
 *-----------------------------------------------------------------------*/

#define MAX_ADMD_NAME_SIZ		17
#define MAX_PRMD_NAME_SIZ		17
#define MAX_COUNTRY_NAME_SIZ	4
#define MAX_MTA_NAME_SIZ		33

#define ADMN_PAD				3
#define PRMD_PAD				3
#define COUNTRY_PAD				0
#define MTA_PAD					3
#define PRMD_PAD_FOR_ACTIONS	2
#define MTA_PAD_FOR_ACTIONS		2

typedef struct {
	LONG	 lAction;				 // The routing action the tracing site
									 // took.(1984 actions only)
	FILETIME ftArrivalTime;			 // The time at which the communique
									 // entered the tracing site.
	FILETIME ftDeferredTime;		 // The time are which the tracing site
									 // released the message.
	char	 rgchADMDName[MAX_ADMD_NAME_SIZ+ADMN_PAD];				// ADMD
	char	 rgchCountryName[MAX_COUNTRY_NAME_SIZ+COUNTRY_PAD];		// Country
	char	 rgchPRMDId[MAX_PRMD_NAME_SIZ+PRMD_PAD];				// PRMD
	char	 rgchAttADMDName[MAX_ADMD_NAME_SIZ+ADMN_PAD];			// Attempted ADMD
	char	 rgchAttCountryName[MAX_COUNTRY_NAME_SIZ+COUNTRY_PAD];	// Attempted Country
	char	 rgchAttPRMDId[MAX_PRMD_NAME_SIZ+PRMD_PAD_FOR_ACTIONS]; // Attempted PRMD
	BYTE	 bAdditionalActions;									// 1998 additional actions
}	TRACEENTRY, FAR * LPTRACEENTRY;

typedef struct {
	ULONG		cEntries;				// Number of trace entries
	TRACEENTRY	rgtraceentry[MAPI_DIM]; // array of trace entries
} TRACEINFO, FAR * LPTRACEINFO;

typedef struct
{
	LONG		lAction;				// The 1984 routing action the tracing domain took.
	FILETIME	ftArrivalTime;			// The time at which the communique entered the tracing domain.
	FILETIME	ftDeferredTime;			// The time are which the tracing domain released the message.
	char		rgchADMDName[MAX_ADMD_NAME_SIZ+ADMN_PAD];				// ADMD
	char		rgchCountryName[MAX_COUNTRY_NAME_SIZ+COUNTRY_PAD];		// Country
	char		rgchPRMDId[MAX_PRMD_NAME_SIZ+PRMD_PAD];					// PRMD
	char		rgchAttADMDName[MAX_ADMD_NAME_SIZ+ADMN_PAD];			// Attempted ADMD
	char		rgchAttCountryName[MAX_COUNTRY_NAME_SIZ+COUNTRY_PAD];	// Attempted Country
	char		rgchAttPRMDId[MAX_PRMD_NAME_SIZ+PRMD_PAD];				// Attempted PRMD
	char		rgchMTAName[MAX_MTA_NAME_SIZ+MTA_PAD];					// MTA Name
	char		rgchAttMTAName[MAX_MTA_NAME_SIZ+MTA_PAD_FOR_ACTIONS];	// Attempted MTA Name
	BYTE		bAdditionalActions;										// 1988 additional actions
}INTTRACEENTRY, *PINTTRACEENTRY;


typedef struct
{
	ULONG			cEntries;					// Number of trace entries
	INTTRACEENTRY	rgIntTraceEntry[MAPI_DIM];	// array of internal trace entries
}INTTRACEINFO, *PINTTRACEINFO;


/*------------------------------------------------------------------------
 *
 *	"IExchangeModifyTable" Interface Declaration
 *
 *	Used for get/set rules and access control on folders.
 *
 *-----------------------------------------------------------------------*/


/* ulRowFlags */
#define ROWLIST_REPLACE		((ULONG)1)

#define ROW_ADD				((ULONG)1)
#define ROW_MODIFY			((ULONG)2)
#define ROW_REMOVE			((ULONG)4)
#define ROW_EMPTY			(ROW_ADD|ROW_REMOVE)

typedef struct _ROWENTRY
{
	ULONG			ulRowFlags;
	ULONG			cValues;
	LPSPropValue	rgPropVals;
} ROWENTRY, FAR * LPROWENTRY;

typedef struct _ROWLIST
{
	ULONG			cEntries;
	ROWENTRY		aEntries[MAPI_DIM];
} ROWLIST, FAR * LPROWLIST;

#define EXCHANGE_IEXCHANGEMODIFYTABLE_METHODS(IPURE)					\
	MAPIMETHOD(GetLastError)											\
		(THIS_	HRESULT						hResult,					\
				ULONG						ulFlags,					\
				LPMAPIERROR FAR *			lppMAPIError) IPURE;		\
	MAPIMETHOD(GetTable)												\
		(THIS_	ULONG						ulFlags,					\
				LPMAPITABLE FAR *			lppTable) IPURE;			\
	MAPIMETHOD(ModifyTable)												\
		(THIS_	ULONG						ulFlags,					\
				LPROWLIST					lpMods) IPURE;

#undef		 INTERFACE
#define		 INTERFACE	IExchangeModifyTable
DECLARE_MAPI_INTERFACE_(IExchangeModifyTable, IUnknown)
{
	MAPI_IUNKNOWN_METHODS(PURE)
	EXCHANGE_IEXCHANGEMODIFYTABLE_METHODS(PURE)
};
#undef	IMPL
#define IMPL

DECLARE_MAPI_INTERFACE_PTR(IExchangeModifyTable,	LPEXCHANGEMODIFYTABLE);

/*	Special flag bit for GetContentsTable, GetHierarchyTable and
	OpenEntry.
	Supported by > 5.x servers
	If set in GetContentsTable and GetHierarchyTable
	we will show only items that are soft deleted, i.e deleted
	by user but not yet purged from the system. If set in OpenEntry
	we will open this item even if it is soft deleted */
/* Flag bits must not collide by existing definitions in Mapi */
/****** MAPI_UNICODE			((ULONG) 0x80000000) above */
/****** MAPI_DEFERRED_ERRORS	((ULONG) 0x00000008) below */
/****** MAPI_ASSOCIATED			((ULONG) 0x00000040) below */
/****** CONVENIENT_DEPTH		((ULONG) 0x00000001)	   */
#define SHOW_SOFT_DELETES		((ULONG) 0x00000002)
#define SHOW_SUBFOLDERS			((ULONG) 0x00000004)

// reserved flag bit(s) - do not set
#define MAPI_RESERVED1			((ULONG) 0x00010000)

// Do not block this OpenMessage (MAPI's OpenEntry)
#define MDB_OPEN_MSG_NO_BLOCK	((ULONG) 0x00000020)

// Unlock a MID at SaveChanges
/****** KEEP_OPEN_READONLY		((ULONG) 0x00000001)  */
/****** KEEP_OPEN_READWRITE		((ULONG) 0x00000002)  */
/****** FORCE_SAVE				((ULONG) 0x00000004)  */
/****** MAPI_DEFERRED_ERRORS	((ULONG) 0x00000008)  */
#define MDB_SAVE_MSG_UNLOCK		((ULONG) 0x00000040)


/*	Special flag bit for DeleteFolder
	Supported by > 5.x servers
	If set the server will hard delete the folder (i.e it will not be
	retained for later recovery) */
/* Flag bits must not collide by existing definitions in Mapi	*/
/*	DeleteFolder */
/*****	#define DEL_MESSAGES			((ULONG) 0x00000001)	*/
/*****	#define FOLDER_DIALOG			((ULONG) 0x00000002)	*/
/*****	#define DEL_FOLDERS				((ULONG) 0x00000004)	*/
/* EmptyFolder */
/*****	#define DEL_ASSOCIATED			((ULONG) 0x00000008)	*/

#define DELETE_HARD_DELETE				((ULONG) 0x00000010)

/* Access Control Specifics */

//Properties
#define PR_MEMBER_ID					PROP_TAG(PT_I8, pidSpecialMin+0x01)
#define PR_MEMBER_NAME					PROP_TAG(PT_TSTRING, pidSpecialMin+0x02)
#define PR_MEMBER_ENTRYID				PR_ENTRYID
#define PR_MEMBER_RIGHTS				PROP_TAG(PT_LONG, pidSpecialMin+0x03)

//Security bits
typedef DWORD RIGHTS;
#define frightsReadAny			0x0000001L
#define frightsCreate			0x0000002L
#define frightsEditOwned		0x0000008L
#define frightsDeleteOwned		0x0000010L
#define frightsEditAny			0x0000020L
#define frightsDeleteAny		0x0000040L
#define frightsCreateSubfolder	0x0000080L
#define frightsOwner			0x0000100L
#define frightsContact			0x0000200L	// NOTE: not part of rightsAll
#define frightsVisible			0x0000400L
#define rightsNone				0x00000000
#define rightsReadOnly			frightsReadAny
#define rightsReadWrite			(frightsReadAny|frightsEditAny)
#define rightsAll				0x00005FBL


//
//	Mailbox specific access rights.
//

//
//	Note that the sdpermUser rights do NOT exist in any security descriptor, they
//	are maintained on the mailbox object in the store, and initialized from the
//	user object in the DS.
//
#define fsdpermUserDeleteMailbox DELETE
#define fsdpermUserMailboxOwner 0x00000001
#define fsdpermUserSendAs		0x00000002
#define fsdpermUserPrimaryUser	0x00000004


#define sdpermUserGenericRead	  (STANDARD_RIGHTS_READ)

// generic execute
#define sdpermUserGenericExecute  (STANDARD_RIGHTS_EXECUTE)
// generic write
#define sdpermUserGenericWrite	  (STANDARD_RIGHTS_WRITE | fsdpermUserDeleteMailbox)

// generic all
#define sdpermUserGenericAll	  (STANDARD_RIGHTS_ALL | fsdpermUserMailboxOwner | fsdpermUserSendAs | fsdpermUserPrimaryUser)

//
//	Message specific rights.
//
typedef DWORD SDRIGHTS;

#define fsdrightReadBody			0x00000001		//** ONLY ON MESSAGES, SAME AS FILE_READ_DATA
#define fsdrightListContents		0x00000001		//** ONLY ON FOLDERS, SAME AS FILE_LIST_DATA - IGNORED
#define fsdrightWriteBody			0x00000002		//** ONLY ON MESSAGES, SAME AS FILE_WRITE_DATA
#define fsdrightCreateItem			0x00000002		//** ONLY ON FOLDERs, SAME AS FILE_ADD_FILE

#define fsdrightAppendMsg			0x00000004		//** ONLY ON MESSAGES,	SAME AS FILE_WRITE_DATA. ENFORCED BY IFS.
#define fsdrightCreateContainer		0x00000004		//** ONLY ON FOLDERS, SAME AS FILE_ADD_FILE

#define fsdrightReadProperty		0x00000008		//** SAME AS FILE_READ_EA
#define fsdrightWriteProperty		0x00000010		//** SAME AS FILE_WRITE_EA

#define fsdrightExecute				0x00000020		// Same as FILE_EXECUTE/FILE_TRAVERSE.	ENFORCED BY IFS
#define fsdrightReserved1			0x00000040		// Same as FILE_DELETE_CHILD.. Currently unused
#define fsdrightReadAttributes		0x00000080		// Same as FILE_READ_ATTRIBUTES. Currently unused
#define fsdrightWriteAttributes		0x00000100		// Same as FILE_WRITE_ATTRIBUTES. Currently unused

#define fsdrightWriteOwnProperty	0x00000200		//** ONLY ON MESSAGES
#define fsdrightDeleteOwnItem		0x00000400		//** ONLY ON MESSAGES
#define fsdrightViewItem			0x00000800
#define fsdrightOwner				0x00004000		//** ONLY ON FOLDERS
#define fsdrightContact				0x00008000		//** ONLY ON FOLDERS

//
//	Standard NT rights.
//
#define fsdrightWriteSD				WRITE_DAC
#define fsdrightDelete				DELETE
#define fsdrightWriteOwner			WRITE_OWNER
#define fsdrightReadControl			READ_CONTROL
#define fsdrightSynchronize			SYNCHRONIZE

#define sdrightsNone			0x00000000
#define sdrightsBestAccess		MAXIMUM_ALLOWED
#define sdrightsReadOnly		GENERIC_READ
#define sdrightsReadWrite		GENERIC_READ | GENERIC_WRITE

#define sdrightsGenericRead			(fsdrightReadControl | fsdrightReadBody | fsdrightReadAttributes | fsdrightReadProperty | fsdrightViewItem |\
									 fsdrightSynchronize)
#define sdrightsGenericWrite		(fsdrightReadControl | fsdrightWriteBody | fsdrightWriteAttributes | fsdrightWriteProperty | \
									 fsdrightAppendMsg | fsdrightCreateItem | fsdrightDelete | fsdrightCreateContainer | \
									 fsdrightOwner | fsdrightSynchronize | fsdrightWriteSD | fsdrightWriteOwner)

#define sdrightsGenericExecute		(fsdrightReadControl | fsdrightReadAttributes | fsdrightExecute | fsdrightViewItem | fsdrightSynchronize)

#define sdrightsGenericAll			(fsdrightDelete | fsdrightReadProperty | fsdrightWriteProperty |\
									 fsdrightCreateItem | fsdrightCreateContainer | fsdrightReadControl | fsdrightWriteSD |\
									 fsdrightWriteOwner | fsdrightReadControl | \
									 fsdrightViewItem | fsdrightOwner | \
									 fsdrightWriteOwnProperty | fsdrightDeleteOwnItem  | fsdrightSynchronize | \
									 fsdrightExecute | fsdrightReserved1 | fsdrightReadAttributes | fsdrightWriteAttributes | \
									 fsdrightReadBody | fsdrightWriteBody | fsdrightSynchronize | fsdrightContact)

//
//	SDRights that together make up rightsOwner.
//
#define sdrightsFolderOwner (fsdrightWriteProperty | fsdrightOwner | fsdrightWriteSD | fsdrightDelete | \
							fsdrightWriteOwner | fsdrightWriteAttributes)

//
// Rights that are valid on folders.
//
#define sdrightsFolders		(fsdrightDelete | fsdrightReadProperty | fsdrightReadAttributes | \
							fsdrightWriteProperty | fsdrightWriteAttributes | fsdrightWriteOwner | \
							fsdrightReadControl | fsdrightWriteSD | fsdrightExecute | \
							fsdrightCreateContainer | fsdrightViewItem | fsdrightOwner | \
							fsdrightContact | fsdrightCreateItem | fsdrightSynchronize | fsdrightListContents | fsdrightReserved1)

//
// Rights that are valid on messages.
//
//
//	NB: fsdrightWriteOwnProperty/fsdrightDeleteOwnItem are NOT in this list.
//
#define sdrightsItems		(fsdrightDelete | fsdrightReadBody | fsdrightReadAttributes | fsdrightReadProperty | \
							fsdrightWriteProperty | fsdrightWriteBody | fsdrightWriteAttributes | fsdrightReadControl | \
							fsdrightWriteOwner | fsdrightWriteSD | fsdrightViewItem | fsdrightWriteOwnProperty | \
							fsdrightDeleteOwnItem  | fsdrightSynchronize  | fsdrightExecute | fsdrightAppendMsg)

//
//	These access rights are ignored in the determination of a canonical ACL.  Since the exchange store ignores
//	these rights, their presence or absense doesn't make an ACL canonical.
//

#define sdrightsIgnored		(fsdrightExecute | fsdrightAppendMsg | fsdrightContact | fsdrightReserved1)

//
//	Backwards Compatible rights definitions.
//
#define msgrightsGenericRead		(sdrightsGenericRead & sdrightsItems)
#define msgrightsGenericWrite		(sdrightsGenericWrite & sdrightsItems)
#define msgrightsGenericExecute		(sdrightsGenericExecute & sdrightsItems)
#define msgrightsGenericAll			(sdrightsGenericAll & sdrightsItems)

#define fldrightsGenericRead		(sdrightsGenericRead & sdrightsFolders)
#define fldrightsGenericWrite		(sdrightsGenericWrite & sdrightsFolders)
#define fldrightsGenericExecute		(sdrightsGenericExecute & sdrightsFolders)
#define fldrightsGenericAll			(sdrightsGenericAll & sdrightsFolders)

//
//	If set in the RM control field of an NTSD, allows
//	an administrator to explicitly set the SD on an object.
//
#define EXCHANGE_RM_SET_EXPLICIT_SD 0x01

//
//	Retrieve the property ID from the guid

#define GUID_PROP_ID(pguid) ((pguid)->Data1 & 0xffff)
#define GUID_SUB_PROP_ID(pguid) ((pguid)->Data1 >> 16 & 0xffff)

#define SET_GUID_PROP_ID(pguid, ptag) (pguid)->Data1 = PROP_ID(ptag)
#define SET_GUID_SUB_PROP_ID(pguid, ptag, subptag) (pguid)->Data1 = (PROP_ID(ptag) | PROP_ID(subptag) << 16)

#define PROPERTY_GUID(ptag) { PROP_ID(ptag),			\
							0x6585, 0x11d3, \
							{0xb6, 0x19, 0x00, 0xaa, 0x00, 0x4b, 0x9c, 0x30}} \

#define SUB_PROPERTY_GUID(ptag, subptag) { PROP_ID(subptag) << 16 | PROP_ID(ptag),	\
							0x6585, 0x11d3,											\
							{0xb6, 0x19, 0x00, 0xaa, 0x00, 0x4b, 0x9c, 0x30}}		\


//
//	Transfer version for PR_NT_SECURITY_DESCRIPTOR.
//
//	When retrieving the security descriptor for an object, the SD returned is
//	actually composed of the following structure:
//
//		2 BYTES					Padding data length (including version)
//		2 BYTES					Version
//		4 BYTES					Security Information (for SetPrivateObjectSecurity)
//		<0 or more>
//			2 BYTES					Property Tag
//			16 BYTES				Named Property GUID
//			1 BYTE					Named property "kind"
//			if (kind == MNID_ID)
//				4 BYTES				Named property ID
//			else
//				<null terminated property name in UNICODE!!!!!>
//		Actual Security Descriptor
//
//	To determine the security descriptor from PR_NT_SECURITY_DESCRIPTOR,
//	use the SECURITY_DESCRIPTOR_OF macro.
//
//	To determine the version of the security descriptor, use the SECURITY_DESCRIPTOR_VERSION macro.
//
//
//	Please note that OLEDB/DAV reserves the even numbers of the transfer version, so it must ALWAYS be an odd number.
//
#define SECURITY_DESCRIPTOR_TRANSFER_VERSION	0x0003

#define SECURITY_DESCRIPTOR_OF(pb)	(((BYTE *)(pb)) + *((WORD *)(pb)))
#define SECURITY_DESCRIPTOR_VERSION(pb) (*((WORD *)((pb) + sizeof(WORD))))
#define SECURITY_INFORMATION_OF(pb) (*((DWORD *)((pb) + sizeof(WORD) + sizeof(WORD))))
#define CbSecurityDescriptorHeader(pb)	(*((WORD *)(pb)))

//
//	To check to see if the security descriptor version matches the currently compiled
//	version.
//
#define FCheckSecurityDescriptorVersion(pb) (SECURITY_DESCRIPTOR_VERSION(pb) == SECURITY_DESCRIPTOR_TRANSFER_VERSION)

//
//	Role scopes
//
typedef BYTE ROLESCOPE;
#define ROLESCOPE_OBJECT	0x00		// Roles will be read from the object (folder or item) itself
#define ROLESCOPE_FOLDER	0x01		// Roles will be read from the folder itself, or the containing folder if it is an item
#define ROLESCOPE_MAX		ROLESCOPE_FOLDER

//
//	Security authority used for role sids
//
#define SECURITY_EXCHANGE_AUTHORITY			  {0,0,0,0,0,8}

//
//	Application role properties
//
#define PR_XMT_SECURITY_ROLE_1			PROP_TAG(PT_BINARY,0x3d25)
#define PR_XMT_SECURITY_ROLE_1_AS_XML		PROP_TAG(PT_TSTRING,0x3d25)
#define PR_XMT_SECURITY_ROLE_2			PROP_TAG(PT_BINARY,0x3d26)
#define PR_XMT_SECURITY_ROLE_2_AS_XML		PROP_TAG(PT_TSTRING,0x3d26)
#define PR_XMT_SECURITY_ROLE_3			PROP_TAG(PT_BINARY,0x3d27)
#define PR_XMT_SECURITY_ROLE_3_AS_XML		PROP_TAG(PT_TSTRING,0x3d27)
#define PR_XMT_SECURITY_ROLE_4			PROP_TAG(PT_BINARY,0x3d28)
#define PR_XMT_SECURITY_ROLE_4_AS_XML		PROP_TAG(PT_TSTRING,0x3d28)
#define PR_XMT_SECURITY_ROLE_5			PROP_TAG(PT_BINARY,0x3d29)
#define PR_XMT_SECURITY_ROLE_5_AS_XML		PROP_TAG(PT_TSTRING,0x3d29)
#define PR_XMT_SECURITY_ROLE_6			PROP_TAG(PT_BINARY,0x3d2A)
#define PR_XMT_SECURITY_ROLE_6_AS_XML		PROP_TAG(PT_TSTRING,0x3d2A)
#define PR_XMT_SECURITY_ROLE_7			PROP_TAG(PT_BINARY,0x3d2B)
#define PR_XMT_SECURITY_ROLE_7_AS_XML		PROP_TAG(PT_TSTRING,0x3d2B)
#define PR_XMT_SECURITY_ROLE_8			PROP_TAG(PT_BINARY,0x3d2C)
#define PR_XMT_SECURITY_ROLE_8_AS_XML		PROP_TAG(PT_TSTRING,0x3d2C)
#define PR_NON_XMT_SECURITY_ROLE_1		PROP_TAG(PT_BINARY,0x0E7C)
#define PR_NON_XMT_SECURITY_ROLE_1_AS_XML	PROP_TAG(PT_TSTRING,0x0E7C)
#define PR_NON_XMT_SECURITY_ROLE_2		PROP_TAG(PT_BINARY,0x0E7D)
#define PR_NON_XMT_SECURITY_ROLE_2_AS_XML	PROP_TAG(PT_TSTRING,0x0E7D)
#define PR_NON_XMT_SECURITY_ROLE_3		PROP_TAG(PT_BINARY,0x0E7E)
#define PR_NON_XMT_SECURITY_ROLE_3_AS_XML	PROP_TAG(PT_TSTRING,0x0E7E)
#define PR_NON_XMT_SECURITY_ROLE_4		PROP_TAG(PT_BINARY,0x0E7F)
#define PR_NON_XMT_SECURITY_ROLE_4_AS_XML	PROP_TAG(PT_TSTRING,0x0E7F)
#define PR_NON_XMT_SECURITY_ROLE_5		PROP_TAG(PT_BINARY,0x0E80)
#define PR_NON_XMT_SECURITY_ROLE_5_AS_XML	PROP_TAG(PT_TSTRING,0x0E80)
#define PR_NON_XMT_SECURITY_ROLE_6		PROP_TAG(PT_BINARY,0x0E81)
#define PR_NON_XMT_SECURITY_ROLE_6_AS_XML	PROP_TAG(PT_TSTRING,0x0E81)
#define PR_NON_XMT_SECURITY_ROLE_7		PROP_TAG(PT_BINARY,0x0E82)
#define PR_NON_XMT_SECURITY_ROLE_7_AS_XML	PROP_TAG(PT_TSTRING,0x0E82)
#define PR_NON_XMT_SECURITY_ROLE_8		PROP_TAG(PT_BINARY,0x0E83)
#define PR_NON_XMT_SECURITY_ROLE_8_AS_XML	PROP_TAG(PT_TSTRING,0x0E83)


/* Rules specifics */

// Property types
#define PT_SRESTRICTION				((ULONG) 0x00FD)
#define PT_ACTIONS					((ULONG) 0x00FE)

/*-----------------------------------------------------------------------
 *	PT_FILE_HANDLE: real data is in file specified by handle.
 *					prop.Value.l has file handle
 *	PT_FILE_EA: real data is in file specified by extended attribute
 *					prop.Value.bin has binary EA data
 *	PT_VIRTUAL: real data is computed on the fly.
 *					prop.Value.bin has raw binary virtual property blob that has
 *					information to do conversion. This is internal to the store and
 *					is not supported for outside calls.
 *-----------------------------------------------------------------------*/

#define PT_FILE_HANDLE					((ULONG) 0x0103)
#define PT_FILE_EA						((ULONG) 0x0104)
#define PT_VIRTUAL						((ULONG) 0x0105)

#define		FVirtualProp(ptag)			(PROP_TYPE(ptag) == PT_VIRTUAL)
#define		FFileHandleProp(ptag)		(PROP_TYPE(ptag) == PT_FILE_HANDLE || PROP_TYPE(ptag) == PT_FILE_EA)

//Properties in rule table
#define PR_RULE_ID						PROP_TAG(PT_I8, pidSpecialMin+0x04)
#define PR_RULE_IDS						PROP_TAG(PT_BINARY, pidSpecialMin+0x05)
#define PR_RULE_SEQUENCE				PROP_TAG(PT_LONG, pidSpecialMin+0x06)
#define PR_RULE_STATE					PROP_TAG(PT_LONG, pidSpecialMin+0x07)
#define PR_RULE_USER_FLAGS				PROP_TAG(PT_LONG, pidSpecialMin+0x08)
#define PR_RULE_CONDITION				PROP_TAG(PT_SRESTRICTION, pidSpecialMin+0x09)
#define PR_RULE_ACTIONS					PROP_TAG(PT_ACTIONS, pidSpecialMin+0x10)
#define PR_RULE_PROVIDER				PROP_TAG(PT_STRING8, pidSpecialMin+0x11)
#define PR_RULE_NAME					PROP_TAG(PT_TSTRING, pidSpecialMin+0x12)
#define PR_RULE_LEVEL					PROP_TAG(PT_LONG, pidSpecialMin+0x13)
#define PR_RULE_PROVIDER_DATA			PROP_TAG(PT_BINARY, pidSpecialMin+0x14)

#define PR_EXTENDED_RULE_ACTIONS		PROP_TAG(PT_BINARY, pidStoreNonTransMin+0x59)
#define PR_EXTENDED_RULE_CONDITION		PROP_TAG(PT_BINARY, pidStoreNonTransMin+0x5a)
#define PR_EXTENDED_RULE_SIZE_LIMIT		PROP_TAG(PT_LONG, pidStoreNonTransMin+0x5b)

// moved to ptag.h (scottno) - still needed for 2.27 upgrader
// #define	PR_RULE_VERSION				PROP_TAG( PT_I2, pidSpecialMin+0x1D)

//PR_STATE property values
#define ST_DISABLED						0x0000
#define ST_ENABLED						0x0001
#define ST_ERROR						0x0002
#define ST_ONLY_WHEN_OOF				0x0004
#define ST_KEEP_OOF_HIST				0x0008
#define ST_EXIT_LEVEL					0x0010
#define ST_SKIP_IF_SCL_IS_SAFE			0x0020
#define ST_RULE_PARSE_ERROR				0x0040
#define ST_CLEAR_OOF_HIST			0x80000000

//Empty restriction
#define NULL_RESTRICTION	0xff

// special RELOP for Member of DL
#define RELOP_MEMBER_OF_DL	100

//Action types
typedef enum
{
	OP_MOVE = 1,
	OP_COPY,
	OP_REPLY,
	OP_OOF_REPLY,
	OP_DEFER_ACTION,
	OP_BOUNCE,
	OP_FORWARD,
	OP_DELEGATE,
	OP_TAG,
	OP_DELETE,
	OP_MARK_AS_READ,

} ACTTYPE;

// provider name for moderator rules
#define szProviderModeratorRule		"MSFT:MR"
#define wszProviderModeratorRule	L"MSFT:MR"

// action flavors

// for OP_REPLY
#define DO_NOT_SEND_TO_ORIGINATOR		1
#define STOCK_REPLY_TEMPLATE			2

// for OP_FORWARD
#define FWD_PRESERVE_SENDER				1
#define FWD_DO_NOT_MUNGE_MSG			2
#define FWD_AS_ATTACHMENT				4

//scBounceCode values
#define BOUNCE_MESSAGE_SIZE_TOO_LARGE	(SCODE) MAPI_DIAG_LENGTH_CONSTRAINT_VIOLATD
#define BOUNCE_FORMS_MISMATCH			(SCODE) MAPI_DIAG_RENDITION_UNSUPPORTED
#define BOUNCE_ACCESS_DENIED			(SCODE) MAPI_DIAG_MAIL_REFUSED

//Message class prefix for Reply and OOF Reply templates
#define szReplyTemplateMsgClassPrefix	"IPM.Note.Rules.ReplyTemplate."
#define szOofTemplateMsgClassPrefix		"IPM.Note.Rules.OofTemplate."

//Action structure
typedef struct _action
{
	ACTTYPE		acttype;

	// to indicate which flavor of the action.
	ULONG		ulActionFlavor;

	// Action restriction
	// currently unused and must be set to NULL
	LPSRestriction	lpRes;

	// currently unused and must be set to NULL.
	LPSPropTagArray lpPropTagArray;

	// User defined flags
	ULONG		ulFlags;

	// padding to align the union on 8 byte boundary
	ULONG		dwAlignPad;

	union
	{
		// used for OP_MOVE and OP_COPY actions
		struct
		{
			ULONG		cbStoreEntryId;
			LPENTRYID	lpStoreEntryId;
			ULONG		cbFldEntryId;
			LPENTRYID	lpFldEntryId;
		} actMoveCopy;

		// used for OP_REPLY and OP_OOF_REPLY actions
		struct
		{
			ULONG		cbEntryId;
			LPENTRYID	lpEntryId;
			GUID		guidReplyTemplate;
		} actReply;

		// used for OP_DEFER_ACTION action
		struct
		{
			ULONG		cbData;
			BYTE		*pbData;
		} actDeferAction;

		// Error code to set for OP_BOUNCE action
		SCODE			scBounceCode;

		// list of address for OP_FORWARD and OP_DELEGATE action
		LPADRLIST		lpadrlist;

		// prop value for OP_TAG action
		SPropValue		propTag;
	};
} ACTION, FAR * LPACTION;

// Rules version
#define EDK_RULES_VERSION		1

//Array of actions
typedef struct _actions
{
	ULONG		ulVersion;		// use the #define above
	UINT		cActions;
	LPACTION	lpAction;
} ACTIONS;

#ifdef __cplusplus
extern "C" {
#endif
HRESULT WINAPI
HrSerializeSRestriction(IMAPIProp * pprop, LPSRestriction prest, BYTE ** ppbRest, ULONG * pcbRest);

HRESULT WINAPI
HrDeserializeSRestriction(IMAPIProp * pprop, BYTE * pbRest, ULONG cbRest, LPSRestriction * pprest);

HRESULT WINAPI
HrSerializeActions(IMAPIProp * pprop, ACTIONS * pActions, BYTE ** ppbActions, ULONG * pcbActions);

HRESULT WINAPI
HrDeserializeActions(IMAPIProp * pprop, BYTE * pbActions, ULONG cbActions, ACTIONS ** ppActions);
#ifdef __cplusplus
} // extern "C"
#endif

// message class definitions for Deferred Action and Deffered Error messages
#define szDamMsgClass		"IPC.Microsoft Exchange 4.0.Deferred Action"
#define szDemMsgClass		"IPC.Microsoft Exchange 4.0.Deferred Error"

#define szExRuleMsgClass	"IPM.ExtendedRule.Message"
#define wszExRuleMsgClass	L"IPM.ExtendedRule.Message"

/*
 *	Rule error codes
 *	Values for PR_RULE_ERROR
 */
#define RULE_ERR_UNKNOWN		1			//general catchall error
#define RULE_ERR_LOAD			2			//unable to load folder rules
#define RULE_ERR_DELIVERY		3			//unable to deliver message temporarily
#define RULE_ERR_PARSING		4			//error while parsing
#define RULE_ERR_CREATE_DAE		5			//error creating DAE message
#define RULE_ERR_NO_FOLDER		6			//folder to move/copy doesn't exist
#define RULE_ERR_NO_RIGHTS		7			//no rights to move/copy into folder
#define RULE_ERR_CREATE_DAM		8			//error creating DAM
#define RULE_ERR_NO_SENDAS		9			//can not send as another user
#define RULE_ERR_NO_TEMPLATE	10			//reply template is missing
#define RULE_ERR_EXECUTION		11			//error in rule execution
#define RULE_ERR_QUOTA_EXCEEDED 12			//mailbox quota size exceeded
#define RULE_ERR_TOO_MANY_RECIPS	13			//number of recips exceded upper limit

#define RULE_ERR_FIRST		RULE_ERR_UNKNOWN
#define RULE_ERR_LAST		RULE_ERR_TOO_MANY_RECIPS

/*------------------------------------------------------------------------
 *
 *	"IExchangeRuleAction" Interface Declaration
 *
 *	Used for get actions from a Deferred Action Message.
 *
 *-----------------------------------------------------------------------*/

#define EXCHANGE_IEXCHANGERULEACTION_METHODS(IPURE)						\
	MAPIMETHOD(ActionCount)												\
		(THIS_	ULONG FAR *					lpcActions) IPURE;			\
	MAPIMETHOD(GetAction)												\
		(THIS_	ULONG						ulActionNumber,				\
				LARGE_INTEGER	*			lpruleid,					\
				LPACTION FAR *				lppAction) IPURE;

#undef		 INTERFACE
#define		 INTERFACE	IExchangeRuleAction
DECLARE_MAPI_INTERFACE_(IExchangeRuleAction, IUnknown)
{
	MAPI_IUNKNOWN_METHODS(PURE)
	EXCHANGE_IEXCHANGERULEACTION_METHODS(PURE)
};
#undef	IMPL
#define IMPL

DECLARE_MAPI_INTERFACE_PTR(IExchangeRuleAction, LPEXCHANGERULEACTION);

/*------------------------------------------------------------------------
 *
 *	"IExchangeManageStore" Interface Declaration
 *
 *	Used for store management functions.
 *
 *-----------------------------------------------------------------------*/

#define EXCHANGE_IEXCHANGEMANAGESTORE_METHODS(IPURE)					\
	MAPIMETHOD(CreateStoreEntryID)										\
		(THIS_	LPSTR						lpszMsgStoreDN,				\
				LPSTR						lpszMailboxDN,				\
				ULONG						ulFlags,					\
				ULONG FAR *					lpcbEntryID,				\
				LPENTRYID FAR *				lppEntryID) IPURE;			\
	MAPIMETHOD(EntryIDFromSourceKey)									\
		(THIS_	ULONG						cFolderKeySize,				\
				BYTE FAR *					lpFolderSourceKey,			\
				ULONG						cMessageKeySize,			\
				BYTE FAR *					lpMessageSourceKey,			\
				ULONG FAR *					lpcbEntryID,				\
				LPENTRYID FAR *				lppEntryID) IPURE;			\
	MAPIMETHOD(GetRights)												\
		(THIS_	ULONG						cbUserEntryID,				\
				LPENTRYID					lpUserEntryID,				\
				ULONG						cbEntryID,					\
				LPENTRYID					lpEntryID,					\
				ULONG FAR *					lpulRights) IPURE;			\
	MAPIMETHOD(GetMailboxTable)											\
		(THIS_	LPSTR						lpszServerName,				\
				LPMAPITABLE FAR *			lppTable,					\
				ULONG						ulFlags) IPURE;				\
	MAPIMETHOD(GetPublicFolderTable)									\
		(THIS_	LPSTR						lpszServerName,				\
				LPMAPITABLE FAR *			lppTable,					\
				ULONG						ulFlags) IPURE;

#undef		 INTERFACE
#define		 INTERFACE	IExchangeManageStore
DECLARE_MAPI_INTERFACE_(IExchangeManageStore, IUnknown)
{
	MAPI_IUNKNOWN_METHODS(PURE)
	EXCHANGE_IEXCHANGEMANAGESTORE_METHODS(PURE)
};
#undef	IMPL
#define IMPL

DECLARE_MAPI_INTERFACE_PTR(IExchangeManageStore, LPEXCHANGEMANAGESTORE);

/*------------------------------------------------------------------------
 *
 *	"IExchangeManageStore2" Interface Declaration
 *
 *	Used for store management functions.
 *
 *-----------------------------------------------------------------------*/

#define EXCHANGE_IEXCHANGEMANAGESTORE2_METHODS(IPURE)					\
	MAPIMETHOD(CreateNewsgroupNameEntryID)								\
		(THIS_	LPSTR						lpszNewsgroupName,			\
				ULONG FAR *					lpcbEntryID,				\
				LPENTRYID FAR *				lppEntryID) IPURE;

#undef		 INTERFACE
#define		 INTERFACE	IExchangeManageStore2
DECLARE_MAPI_INTERFACE_(IExchangeManageStore2, IUnknown)
{
	MAPI_IUNKNOWN_METHODS(PURE)
	EXCHANGE_IEXCHANGEMANAGESTORE_METHODS(PURE)
	EXCHANGE_IEXCHANGEMANAGESTORE2_METHODS(PURE)
};
#undef	IMPL
#define IMPL

DECLARE_MAPI_INTERFACE_PTR(IExchangeManageStore2, LPEXCHANGEMANAGESTORE2);


/*------------------------------------------------------------------------
 *
 *	"IExchangeManageStore3" Interface Declaration
 *
 *	Used for store management functions.
 *
 *-----------------------------------------------------------------------*/

#define EXCHANGE_IEXCHANGEMANAGESTORE3_METHODS(IPURE)					\
	MAPIMETHOD(GetMailboxTableOffset)											\
		(THIS_	LPSTR						lpszServerName,			\
				LPMAPITABLE FAR *			lppTable,					\
				ULONG						ulFlags,					\
				UINT						uOffset) IPURE;

#undef		 INTERFACE
#define		 INTERFACE  IExchangeManageStore3
DECLARE_MAPI_INTERFACE_(IExchangeManageStore3, IUnknown)
{
	MAPI_IUNKNOWN_METHODS(PURE)
	EXCHANGE_IEXCHANGEMANAGESTORE_METHODS(PURE)
	EXCHANGE_IEXCHANGEMANAGESTORE2_METHODS(PURE)
	EXCHANGE_IEXCHANGEMANAGESTORE3_METHODS(PURE)
};
#undef	IMPL
#define IMPL

DECLARE_MAPI_INTERFACE_PTR(IExchangeManageStore3, LPEXCHANGEMANAGESTORE3);


/*------------------------------------------------------------------------
 *
 *	"IExchangeManageStore4" Interface Declaration
 *
 *	Used for store management functions.
 *
 *-----------------------------------------------------------------------*/

#define EXCHANGE_IEXCHANGEMANAGESTORE4_METHODS(IPURE)					\
	MAPIMETHOD(GetPublicFolderTableOffset)									\
		(THIS_	LPSTR						lpszServerName,				\
				LPMAPITABLE FAR *			lppTable,					\
				ULONG						ulFlags,					\
				UINT						uOffset) IPURE;

#undef		 INTERFACE
#define		 INTERFACE  IExchangeManageStore4
DECLARE_MAPI_INTERFACE_(IExchangeManageStore4, IUnknown)
{
	MAPI_IUNKNOWN_METHODS(PURE)
	EXCHANGE_IEXCHANGEMANAGESTORE_METHODS(PURE)
	EXCHANGE_IEXCHANGEMANAGESTORE2_METHODS(PURE)
	EXCHANGE_IEXCHANGEMANAGESTORE3_METHODS(PURE)
	EXCHANGE_IEXCHANGEMANAGESTORE4_METHODS(PURE)
};
#undef	IMPL
#define IMPL

DECLARE_MAPI_INTERFACE_PTR(IExchangeManageStore4, LPEXCHANGEMANAGESTORE4);


/*------------------------------------------------------------------------
 *
 *	"IExchangeNntpNewsfeed" Interface Declaration
 *
 *	Used for Nntp pull newsfeed.
 *
 *-----------------------------------------------------------------------*/

#define EXCHANGE_IEXCHANGENNTPNEWSFEED_METHODS(IPURE)					\
	MAPIMETHOD(Configure)												\
		(THIS_	LPSTR						lpszNewsfeedDN,				\
				ULONG						cValues,					\
				LPSPropValue				lpIMailPropArray) IPURE;	\
	MAPIMETHOD(CheckMsgIds)												\
		(THIS_	LPSTR						lpszMsgIds,					\
				ULONG FAR *					lpcfWanted,					\
				BYTE FAR **					lppfWanted) IPURE;			\
	MAPIMETHOD(OpenArticleStream)										\
		(THIS_	LPSTREAM FAR *				lppStream) IPURE;			\


#undef		 INTERFACE
#define		 INTERFACE	IExchangeNntpNewsfeed
DECLARE_MAPI_INTERFACE_(IExchangeNntpNewsfeed, IUnknown)
{
	MAPI_IUNKNOWN_METHODS(PURE)
	EXCHANGE_IEXCHANGENNTPNEWSFEED_METHODS(PURE)
};
#undef	IMPL
#define IMPL

DECLARE_MAPI_INTERFACE_PTR(IExchangeNntpNewsfeed, LPEXCHANGENNTPNEWSFEED);

// Properties for GetMailboxTable
#define PR_NT_USER_NAME							PROP_TAG(PT_TSTRING, pidAdminMin+0x10)
//
// PR_LOCALE_ID definition has been moved down and combined with other
// locale-specific properties.	It is still being returned through the
// mailbox table.
//
//#define PR_LOCALE_ID							  PROP_TAG( PT_LONG, pidAdminMin+0x11 )
#define PR_LAST_LOGON_TIME						PROP_TAG(PT_SYSTIME, pidAdminMin+0x12 )
#define PR_LAST_LOGOFF_TIME						PROP_TAG(PT_SYSTIME, pidAdminMin+0x13 )
#define PR_STORAGE_LIMIT_INFORMATION			PROP_TAG(PT_LONG, pidAdminMin+0x14 )
// property on disabling message read (unread) receipt
// reusing Folders table property (pidAdminMin+0x15)

#define PR_INTERNET_MDNS						PROP_TAG(PT_BOOLEAN, PROP_ID(PR_NEWSGROUP_COMPONENT))

// properties for mailbox quota info - reusing properties from folder table -
// folder pathname, owner, and contacts re-used.
#define PR_QUOTA_WARNING_THRESHOLD              PROP_TAG(PT_LONG, pidAdminMin+0x91)
#define PR_QUOTA_SEND_THRESHOLD                 PROP_TAG(PT_LONG, pidAdminMin+0x92)
#define PR_QUOTA_RECEIVE_THRESHOLD              PROP_TAG(PT_LONG, pidAdminMin+0x93)


// Properties for GetPublicFolderTable
#define PR_FOLDER_FLAGS							PROP_TAG(PT_LONG, pidAdminMin+0x18)
#define PR_LAST_ACCESS_TIME						PROP_TAG(PT_SYSTIME, pidAdminMin+0x19)
#define PR_RESTRICTION_COUNT					PROP_TAG(PT_LONG, pidAdminMin+0x1A)
#define PR_CATEG_COUNT							PROP_TAG(PT_LONG, pidAdminMin+0x1B)
#define PR_CACHED_COLUMN_COUNT					PROP_TAG(PT_LONG, pidAdminMin+0x1C)
#define PR_NORMAL_MSG_W_ATTACH_COUNT			PROP_TAG(PT_LONG, pidAdminMin+0x1D)
#define PR_ASSOC_MSG_W_ATTACH_COUNT				PROP_TAG(PT_LONG, pidAdminMin+0x1E)
#define PR_RECIPIENT_ON_NORMAL_MSG_COUNT		PROP_TAG(PT_LONG, pidAdminMin+0x1F)
#define PR_RECIPIENT_ON_ASSOC_MSG_COUNT			PROP_TAG(PT_LONG, pidAdminMin+0x20)
#define PR_ATTACH_ON_NORMAL_MSG_COUNT			PROP_TAG(PT_LONG, pidAdminMin+0x21)
#define PR_ATTACH_ON_ASSOC_MSG_COUNT			PROP_TAG(PT_LONG, pidAdminMin+0x22)
#define PR_NORMAL_MESSAGE_SIZE					PROP_TAG(PT_LONG, pidAdminMin+0x23)
#define PR_NORMAL_MESSAGE_SIZE_EXTENDED			PROP_TAG(PT_I8, pidAdminMin+0x23)
#define PR_ASSOC_MESSAGE_SIZE					PROP_TAG(PT_LONG, pidAdminMin+0x24)
#define PR_ASSOC_MESSAGE_SIZE_EXTENDED			PROP_TAG(PT_I8, pidAdminMin+0x24)
#define PR_FOLDER_PATHNAME						PROP_TAG(PT_TSTRING, pidAdminMin+0x25)
#define PR_OWNER_COUNT							PROP_TAG(PT_LONG, pidAdminMin+0x26)
#define PR_CONTACT_COUNT						PROP_TAG(PT_LONG, pidAdminMin+0x27)

/* the absolute size limitation of a public folder */
#define PR_PF_OVER_HARD_QUOTA_LIMIT				PROP_TAG(PT_LONG, pidAdminMin+0x91)
/* the size limit of a message in a public folder */
#define PR_PF_MSG_SIZE_LIMIT					PROP_TAG(PT_LONG, pidAdminMin+0x92)

// Do not inherit expiry settings from the MDB wide settings and instead use folder specific ones
// (if folder specific is not set, it will still not get from MDB and remain with no expiry at all)
#define PR_PF_DISALLOW_MDB_WIDE_EXPIRY			PROP_TAG(PT_BOOLEAN, pidAdminMin+0x93)

// Locale-specific properties
#define PR_LOCALE_ID							PROP_TAG(PT_LONG, pidAdminMin+0x11)
#define PR_CODE_PAGE_ID							PROP_TAG(PT_LONG, pidAdminMin+0x33)
#define PR_SORT_LOCALE_ID						PROP_TAG(PT_LONG, pidAdminMin+0x75)

// PT_I8 version of PR_MESSAGE_SIZE defined in mapitags.h
#define PR_MESSAGE_SIZE_EXTENDED				PROP_TAG(PT_I8, PROP_ID(PR_MESSAGE_SIZE))

/* Bits in PR_FOLDER_FLAGS */
#define MDB_FOLDER_IPM					0x1
#define MDB_FOLDER_SEARCH				0x2
#define MDB_FOLDER_NORMAL				0x4
#define MDB_FOLDER_RULES				0x8

/* Bits used in ulFlags in GetPublicFolderTable() */
#define MDB_NON_IPM						0x10
#define MDB_IPM							0x20

/* Bits in PR_STORAGE_LIMIT_INFORMATION */
#define MDB_LIMIT_BELOW					0x1
#define MDB_LIMIT_ISSUE_WARNING			0x2
#define MDB_LIMIT_PROHIBIT_SEND			0x4
#define MDB_LIMIT_NO_CHECK				0x8
#define MDB_LIMIT_DISABLED				0x10

/* A define for "no quota infomation" when retrieving the quota information */
#define MDB_QUOTA_NOQUOTA				0xFFFFFFFF

/*------------------------------------------------------------------------
 *
 *	"IExchangeFastTransfer" Interface Declaration
 *
 *	Used for fast transfer interface used to
 *	implement CopyTo, CopyProps, CopyFolder, and
 *	CopyMessages.
 *
 *-----------------------------------------------------------------------*/

// Transfer flags
// Use MAPI_MOVE for move option

// Transfer methods
#define TRANSFER_COPYTO			1
#define TRANSFER_COPYPROPS		2
#define TRANSFER_COPYMESSAGES	3
#define TRANSFER_COPYFOLDER		4


#define EXCHANGE_IEXCHANGEFASTTRANSFER_METHODS(IPURE)			\
	MAPIMETHOD(Config)											\
		(THIS_	ULONG				ulFlags,					\
				ULONG				ulTransferMethod) IPURE;	\
	MAPIMETHOD(TransferBuffer)									\
		(THIS_	ULONG				cb,							\
				LPBYTE				lpb,						\
				ULONG				*lpcbProcessed) IPURE;		\
	STDMETHOD_(BOOL, IsInterfaceOk)								\
		(THIS_	ULONG				ulTransferMethod,			\
				REFIID				refiid,						\
				LPSPropTagArray		lpptagList,					\
				ULONG				ulFlags) IPURE;

#undef		 INTERFACE
#define		 INTERFACE	IExchangeFastTransfer
DECLARE_MAPI_INTERFACE_(IExchangeFastTransfer, IUnknown)
{
	MAPI_IUNKNOWN_METHODS(PURE)
	EXCHANGE_IEXCHANGEFASTTRANSFER_METHODS(PURE)
};
#undef	IMPL
#define IMPL

DECLARE_MAPI_INTERFACE_PTR(IExchangeFastTransfer, LPEXCHANGEFASTTRANSFER);



/*------------------------------------------------------------------------
 *
 *	"IExchangeExportChanges" Interface Declaration
 *
 *	Used for Incremental Synchronization
 *
 *-----------------------------------------------------------------------*/

#define EXCHANGE_IEXCHANGEEXPORTCHANGES_METHODS(IPURE)		\
	MAPIMETHOD(GetLastError)								\
		(THIS_	HRESULT				hResult,				\
				ULONG				ulFlags,				\
				LPMAPIERROR FAR *	lppMAPIError) IPURE;	\
	MAPIMETHOD(Config)										\
		(THIS_	LPSTREAM			lpStream,				\
				ULONG				ulFlags,				\
				LPUNKNOWN			lpUnk,					\
				LPSRestriction		lpRestriction,			\
				LPSPropTagArray		lpIncludeProps,			\
				LPSPropTagArray		lpExcludeProps,			\
				ULONG				ulBufferSize) IPURE;	\
	MAPIMETHOD(Synchronize)									\
		(THIS_	ULONG FAR *			lpulSteps,				\
				ULONG FAR *			lpulProgress) IPURE;	\
	MAPIMETHOD(UpdateState)									\
		(THIS_	LPSTREAM			lpStream) IPURE;

#undef		 INTERFACE
#define		 INTERFACE	IExchangeExportChanges
DECLARE_MAPI_INTERFACE_(IExchangeExportChanges, IUnknown)
{
	MAPI_IUNKNOWN_METHODS(PURE)
	EXCHANGE_IEXCHANGEEXPORTCHANGES_METHODS(PURE)
};
#undef	IMPL
#define IMPL

DECLARE_MAPI_INTERFACE_PTR(IExchangeExportChanges, LPEXCHANGEEXPORTCHANGES);

/*------------------------------------------------------------------------
 *
 *	"IExchangeExportChanges2" Interface Declaration
 *
 *	Used for Incremental Synchronization
 *	Has the Config2 method for configuring for internet format conversion streams
 *
 *-----------------------------------------------------------------------*/

#define EXCHANGE_IEXCHANGEEXPORTCHANGES2_METHODS(IPURE)		\
	MAPIMETHOD(ConfigForConversionStream)						\
		(THIS_	LPSTREAM			lpStream,				\
				ULONG				ulFlags,				\
				LPUNKNOWN			lpUnk,					\
				LPSRestriction		lpRestriction,			\
				ULONG				cValuesConversion,			\
				LPSPropValue		lpPropArrayConversion,		\
				ULONG				ulBufferSize) IPURE;

#undef		 INTERFACE
#define		 INTERFACE	IExchangeExportChanges2
DECLARE_MAPI_INTERFACE_(IExchangeExportChanges2, IExchangeExportChanges)
{
	MAPI_IUNKNOWN_METHODS(PURE)
	EXCHANGE_IEXCHANGEEXPORTCHANGES_METHODS(PURE)
	EXCHANGE_IEXCHANGEEXPORTCHANGES2_METHODS(PURE)
};
#undef	IMPL
#define IMPL

DECLARE_MAPI_INTERFACE_PTR(IExchangeExportChanges2, LPEXCHANGEEXPORTCHANGES2);

/*------------------------------------------------------------------------
 *
 *	"IExchangeExportChanges3" Interface Declaration
 *
 *	Used for Incremental Synchronization
 *	Has the Config3 method for configuring for selective message download
 *
 *-----------------------------------------------------------------------*/

#define EXCHANGE_IEXCHANGEEXPORTCHANGES3_METHODS(IPURE)		\
	MAPIMETHOD(ConfigForSelectiveSync)						\
		(THIS_	LPSTREAM			lpStream,				\
				ULONG				ulFlags,				\
				LPUNKNOWN			lpUnk,					\
				LPENTRYLIST			lpMsgList,				\
				LPSRestriction		lpRestriction,			\
				LPSPropTagArray		lpIncludeProps,			\
				LPSPropTagArray		lpExcludeProps,			\
				ULONG				ulBufferSize) IPURE;

#undef		 INTERFACE
#define		 INTERFACE	IExchangeExportChanges3
DECLARE_MAPI_INTERFACE_(IExchangeExportChanges3, IExchangeExportChanges2)
{
	MAPI_IUNKNOWN_METHODS(PURE)
	EXCHANGE_IEXCHANGEEXPORTCHANGES_METHODS(PURE)
	EXCHANGE_IEXCHANGEEXPORTCHANGES2_METHODS(PURE)
	EXCHANGE_IEXCHANGEEXPORTCHANGES3_METHODS(PURE)
};
#undef	IMPL
#define IMPL

DECLARE_MAPI_INTERFACE_PTR(IExchangeExportChanges3, LPEXCHANGEEXPORTCHANGES3);

typedef struct _ReadState
{
	ULONG		cbSourceKey;
	BYTE	*	pbSourceKey;
	ULONG		ulFlags;
} READSTATE, *LPREADSTATE;

/*------------------------------------------------------------------------
 *
 *	"IExchangeImportContentsChanges" Interface Declaration
 *
 *	Used for Incremental Synchronization of folder contents (i.e. messages)
 *
 *-----------------------------------------------------------------------*/


#define EXCHANGE_IEXCHANGEIMPORTCONTENTSCHANGES_METHODS(IPURE)		\
	MAPIMETHOD(GetLastError)										\
		(THIS_	HRESULT				hResult,						\
				ULONG				ulFlags,						\
				LPMAPIERROR FAR *	lppMAPIError) IPURE;			\
	MAPIMETHOD(Config)												\
		(THIS_	LPSTREAM				lpStream,					\
				ULONG					ulFlags) IPURE;				\
	MAPIMETHOD(UpdateState)											\
		(THIS_	LPSTREAM				lpStream) IPURE;			\
	MAPIMETHOD(ImportMessageChange)									\
		(THIS_	ULONG					cpvalChanges,				\
				LPSPropValue			ppvalChanges,				\
				ULONG					ulFlags,					\
				LPMESSAGE				*lppmessage) IPURE;			\
	MAPIMETHOD(ImportMessageDeletion)								\
		(THIS_	ULONG					ulFlags,					\
				LPENTRYLIST				lpSrcEntryList) IPURE;		\
	MAPIMETHOD(ImportPerUserReadStateChange)						\
		(THIS_	ULONG					cElements,					\
				LPREADSTATE				lpReadState) IPURE;			\
	MAPIMETHOD(ImportMessageMove)									\
		(THIS_	ULONG					cbSourceKeySrcFolder,		\
				BYTE FAR *				pbSourceKeySrcFolder,		\
				ULONG					cbSourceKeySrcMessage,		\
				BYTE FAR *				pbSourceKeySrcMessage,		\
				ULONG					cbPCLMessage,				\
				BYTE FAR *				pbPCLMessage,				\
				ULONG					cbSourceKeyDestMessage,		\
				BYTE FAR *				pbSourceKeyDestMessage,		\
				ULONG					cbChangeNumDestMessage,		\
				BYTE FAR *				pbChangeNumDestMessage) IPURE;


#undef		 INTERFACE
#define		 INTERFACE	IExchangeImportContentsChanges
DECLARE_MAPI_INTERFACE_(IExchangeImportContentsChanges, IUnknown)
{
	MAPI_IUNKNOWN_METHODS(PURE)
	EXCHANGE_IEXCHANGEIMPORTCONTENTSCHANGES_METHODS(PURE)
};
#undef	IMPL
#define IMPL

DECLARE_MAPI_INTERFACE_PTR(IExchangeImportContentsChanges,
						   LPEXCHANGEIMPORTCONTENTSCHANGES);

/*------------------------------------------------------------------------
 *
 *	"IExchangeImportContentsChanges2" Interface Declaration
 *
 *	Used for Incremental Synchronization of folder contents (i.e. messages)
 *	This interface allows you to import message changes as an internet
 *	format conversion stream
 *
 *-----------------------------------------------------------------------*/


#define EXCHANGE_IEXCHANGEIMPORTCONTENTSCHANGES2_METHODS(IPURE)		\
	MAPIMETHOD(ConfigForConversionStream)								\
		(THIS_	LPSTREAM				lpStream,					\
				ULONG					ulFlags,					\
				ULONG					cValuesConversion,				\
				LPSPropValue			lpPropArrayConversion) IPURE;	\
	MAPIMETHOD(ImportMessageChangeAsAStream)						\
		(THIS_	ULONG					cpvalChanges,				\
				LPSPropValue			ppvalChanges,				\
				ULONG					ulFlags,					\
				LPSTREAM				*lppstream) IPURE;			\


#undef		 INTERFACE
#define		 INTERFACE	IExchangeImportContentsChanges2
DECLARE_MAPI_INTERFACE_(IExchangeImportContentsChanges2, IExchangeImportContentsChanges)
{
	MAPI_IUNKNOWN_METHODS(PURE)
	EXCHANGE_IEXCHANGEIMPORTCONTENTSCHANGES_METHODS(PURE)
	EXCHANGE_IEXCHANGEIMPORTCONTENTSCHANGES2_METHODS(PURE)
};
#undef	IMPL
#define IMPL

DECLARE_MAPI_INTERFACE_PTR(IExchangeImportContentsChanges2,
						   LPEXCHANGEIMPORTCONTENTSCHANGES2);

/*------------------------------------------------------------------------
 *
 *	"IExchangeImportHierarchyChanges" Interface Declaration
 *
 *	Used for Incremental Synchronization of folder hierarchy
 *
 *-----------------------------------------------------------------------*/

#define EXCHANGE_IEXCHANGEIMPORTHIERARCHYCHANGES_METHODS(IPURE)		\
	MAPIMETHOD(GetLastError)										\
		(THIS_	HRESULT				hResult,						\
				ULONG				ulFlags,						\
				LPMAPIERROR FAR *	lppMAPIError) IPURE;			\
	MAPIMETHOD(Config)												\
		(THIS_	LPSTREAM				lpStream,					\
				ULONG					ulFlags) IPURE;				\
	MAPIMETHOD(UpdateState)											\
		(THIS_	LPSTREAM				lpStream) IPURE;			\
	MAPIMETHOD(ImportFolderChange)									\
		(THIS_	ULONG						cpvalChanges,			\
				LPSPropValue				ppvalChanges) IPURE;	\
	MAPIMETHOD(ImportFolderDeletion)								\
		(THIS_	ULONG						ulFlags,				\
				LPENTRYLIST					lpSrcEntryList) IPURE;

#undef		 INTERFACE
#define		 INTERFACE	IExchangeImportHierarchyChanges
DECLARE_MAPI_INTERFACE_(IExchangeImportHierarchyChanges, IUnknown)
{
	MAPI_IUNKNOWN_METHODS(PURE)
	EXCHANGE_IEXCHANGEIMPORTHIERARCHYCHANGES_METHODS(PURE)
};
#undef	IMPL
#define IMPL

DECLARE_MAPI_INTERFACE_PTR(IExchangeImportHierarchyChanges,
						   LPEXCHANGEIMPORTHIERARCHYCHANGES);

#define		ulHierChanged		(0x01)

#define EXCHANGE_IEXCHANGECHANGEADVISESINK_METHODS(IPURE)			\
	MAPIMETHOD_(ULONG, OnNotify)									\
		(THIS_	ULONG						ulFlags,				\
				LPENTRYLIST					lpEntryList) IPURE;		\

#undef		 INTERFACE
#define		 INTERFACE	IExchangeChangeAdviseSink
DECLARE_MAPI_INTERFACE_(IExchangeChangeAdviseSink, IUnknown)
{
	BEGIN_INTERFACE
	MAPI_IUNKNOWN_METHODS(PURE)
	EXCHANGE_IEXCHANGECHANGEADVISESINK_METHODS(PURE)
};
#undef	IMPL
#define IMPL

DECLARE_MAPI_INTERFACE_PTR(IExchangeChangeAdviseSink,
						   LPEXCHANGECHANGEADVISESINK);

#define EXCHANGE_IEXCHANGECHANGEADVISOR_METHODS(IPURE)				\
	MAPIMETHOD(GetLastError)										\
		(THIS_	HRESULT				hResult,						\
				ULONG				ulFlags,						\
				LPMAPIERROR FAR *	lppMAPIError) IPURE;			\
	MAPIMETHOD(Config)												\
		(THIS_	LPSTREAM					lpStream,				\
				LPGUID						lpGUID,					\
				LPEXCHANGECHANGEADVISESINK	lpAdviseSink,			\
				ULONG						ulFlags) IPURE;			\
	MAPIMETHOD(UpdateState)											\
		(THIS_	LPSTREAM			lpStream) IPURE;				\
	MAPIMETHOD(AddKeys)												\
		(THIS_	LPENTRYLIST			lpEntryList) IPURE;				\
	MAPIMETHOD(RemoveKeys)											\
		(THIS_	LPENTRYLIST			lpEntryList) IPURE;

#undef		 INTERFACE
#define		 INTERFACE	IExchangeChangeAdvisor
DECLARE_MAPI_INTERFACE_(IExchangeChangeAdvisor, IUnknown)
{
	MAPI_IUNKNOWN_METHODS(PURE)
	EXCHANGE_IEXCHANGECHANGEADVISOR_METHODS(PURE)
};
#undef	IMPL
#define IMPL

DECLARE_MAPI_INTERFACE_PTR(IExchangeChangeAdvisor,
						   LPEXCHANGECHANGEADVISOR);

/*--------------------------------------------------------------------
 *
 *	"IExchangeBadItemCallback" Interface Declaration
 *
 *	Used to report bad items during move mailbox
 *
 *--------------------------------------------------------------------*/

#define EXCHANGE_IEXCHANGEBADITEMCALLBACK_METHODS(IPURE)	\
	MAPIMETHOD(BadItem)										\
		(THIS_	HRESULT			hResult,					\
	 	    	ULONG			ulFlags,					\
				LPWSTR			lpwszFolderName,			\
				LPSBinary		lpsbFolderEid,				\
				ULONG			cValues,					\
	 	    	LPSPropValue	lpPropArray) IPURE;

#undef  INTERFACE
#define INTERFACE  IExchangeBadItemCallback
DECLARE_MAPI_INTERFACE_(IExchangeBadItemCallback, IUnknown)
{
	MAPI_IUNKNOWN_METHODS(PURE)
	EXCHANGE_IEXCHANGEBADITEMCALLBACK_METHODS(PURE)
};
#undef  IMPL
#define IMPL

DECLARE_MAPI_INTERFACE_PTR(IExchangeBadItemCallback,
						   LPEXCHANGEBADITEMCALLBACK);

/*--------------------------------------------------------------------
 *
 *	"IExchangeMoveUserProgress" Interface Declaration
 *
 *	Used to report per folder progress during move mailbox
 *
 *--------------------------------------------------------------------*/

#define EXCHANGE_IEXCHANGEMOVEUSERPROGRESS_METHODS(IPURE)	\
	MAPIMETHOD(NextFolder)									\
		(THIS_	ULONG			ulFlags,					\
				LPWSTR			lpwszFolderName) IPURE;		\
	MAPIMETHOD(Progress)									\
		(THIS_	ULONG			ulFlags,					\
				ULONG			ulCount,					\
				ULONG			ulTotal) IPURE;				\
	MAPIMETHOD(Restart)										\
		(THIS_	ULONG			ulFlags) IPURE;				\

#undef  INTERFACE
#define INTERFACE  IExchangeMoveUserProgress
DECLARE_MAPI_INTERFACE_(IExchangeMoveUserProgress, IUnknown)
{
	MAPI_IUNKNOWN_METHODS(PURE)
	EXCHANGE_IEXCHANGEMOVEUSERPROGRESS_METHODS(PURE)
};
#undef  IMPL
#define IMPL

DECLARE_MAPI_INTERFACE_PTR(IExchangeMoveUserProgress,
						   LPEXCHANGEMOVEUSERPROGRESS);

// Internal flag for IMsgStore::CopyTo which tells the move user processing
// that there are potential extended callback objects hanhing off of the
// IMAPIProgress object.
#define MAPI_EXTENDEDCALLBACKS	((ULONG) 0x00000400)


/*------------------------------------------------------------------------
 *
 *	Errors returned by Exchange Incremental Change Synchronization Interface
 *
 *-----------------------------------------------------------------------*/

#define MAKE_SYNC_E(err)	(MAKE_SCODE(SEVERITY_ERROR, FACILITY_ITF, err))
#define MAKE_SYNC_W(warn)	(MAKE_SCODE(SEVERITY_SUCCESS, FACILITY_ITF, warn))

#define SYNC_E_UNKNOWN_FLAGS			MAPI_E_UNKNOWN_FLAGS
#define SYNC_E_INVALID_PARAMETER		E_INVALIDARG
#define SYNC_E_ERROR					E_FAIL
#define SYNC_E_OBJECT_DELETED			MAKE_SYNC_E(0x800)
#define SYNC_E_IGNORE					MAKE_SYNC_E(0x801)
#define SYNC_E_CONFLICT					MAKE_SYNC_E(0x802)
#define SYNC_E_NO_PARENT				MAKE_SYNC_E(0x803)
#define SYNC_E_CYCLE					MAKE_SYNC_E(0x804)
#define SYNC_E_UNSYNCHRONIZED			MAKE_SYNC_E(0x805)

#define SYNC_W_PROGRESS					MAKE_SYNC_W(0x820)
#define SYNC_W_CLIENT_CHANGE_NEWER		MAKE_SYNC_W(0x821)

/*------------------------------------------------------------------------
 *
 *	Flags used by Exchange Incremental Change Synchronization Interface
 *
 *-----------------------------------------------------------------------*/

#define SYNC_UNICODE				0x01
#define SYNC_NO_DELETIONS			0x02
#define SYNC_NO_SOFT_DELETIONS		0x04
#define SYNC_READ_STATE				0x08
#define SYNC_ASSOCIATED				0x10
#define SYNC_NORMAL					0x20
#define SYNC_NO_CONFLICTS			0x40
#define SYNC_ONLY_SPECIFIED_PROPS	0x80
#define SYNC_NO_FOREIGN_KEYS		0x100
#define SYNC_LIMITED_IMESSAGE		0x200
#define SYNC_CATCHUP				0x400
#define SYNC_NEW_MESSAGE			0x800	// only applicable to ImportMessageChange()
#define SYNC_MSG_SELECTIVE			0x1000	// Used internally.	 Will reject if used by clients.
#define SYNC_BEST_BODY				0x2000
#define SYNC_IGNORE_SPECIFIED_ON_ASSOCIATED 0x4000
#define SYNC_PROGRESS_MODE			0x8000	// AirMapi progress mode
#define SYNC_FXRECOVERMODE			0x10000
#define SYNC_DEFER_CONFIG			0x20000
#define SYNC_FORCE_UNICODE			0x40000	// Forces server to return Unicode properties

/*------------------------------------------------------------------------
 *
 *	Flags used by ImportMessageDeletion and ImportFolderDeletion methods
 *
 *-----------------------------------------------------------------------*/

#define SYNC_SOFT_DELETE			0x01
#define SYNC_EXPIRY					0x02

/*------------------------------------------------------------------------
 *
 *	Flags used by ImportPerUserReadStateChange method
 *
 *-----------------------------------------------------------------------*/

#define SYNC_READ					0x01

/*------------------------------------------------------------------------
 *
 *	Extended Flags used by CopyMessages method
 *
 *-----------------------------------------------------------------------*/

#define MESSAGE_BEST_BODY			0x10
#define MESSAGE_SEND_ENTRYID		0x20

/*------------------------------------------------------------------------
 *
 *	Extended Flags used by GetHierarchyTable method
 *
 *-----------------------------------------------------------------------*/

#define SUPRESS_NOTIFICATIONS_ON_MY_ACTIONS 0x01000


/*------------------------------------------------------------------------
 *
 *	"IExchangeFavorites" Interface Declaration
 *
 *	Used for adding or removing favorite folders from the public store.
 *	This interface is obtained by calling QueryInterface on the folder
 *	whose EntryID is specified by PR_IPM_FAVORITES_ENTRYID on the public
 *	store.
 *
 *-----------------------------------------------------------------------*/

#define EXCHANGE_IEXCHANGEFAVORITES_METHODS(IPURE)						\
	MAPIMETHOD(GetLastError)											\
		(THIS_	HRESULT						hResult,					\
				ULONG						ulFlags,					\
				LPMAPIERROR FAR *			lppMAPIError) IPURE;		\
	MAPIMETHOD(AddFavorites)											\
		(THIS_	LPENTRYLIST					lpEntryList) IPURE;			\
	MAPIMETHOD(DelFavorites)											\
		(THIS_	LPENTRYLIST					lpEntryList) IPURE;			\

#undef		 INTERFACE
#define		 INTERFACE	IExchangeFavorites
DECLARE_MAPI_INTERFACE_(IExchangeFavorites, IUnknown)
{
	MAPI_IUNKNOWN_METHODS(PURE)
	EXCHANGE_IEXCHANGEFAVORITES_METHODS(PURE)
};

DECLARE_MAPI_INTERFACE_PTR(IExchangeFavorites,	LPEXCHANGEFAVORITES);


/*------------------------------------------------------------------------
 *
 *	Properties used by the Favorites Folders API
 *
 *-----------------------------------------------------------------------*/

#define PR_AUTO_ADD_NEW_SUBS			PROP_TAG(PT_BOOLEAN, pidExchangeNonXmitReservedMin+0x5)
#define PR_NEW_SUBS_GET_AUTO_ADD		PROP_TAG(PT_BOOLEAN, pidExchangeNonXmitReservedMin+0x6)
/*------------------------------------------------------------------------
 *
 *	Properties used by the Offline Folders API
 *
 *-----------------------------------------------------------------------*/

#define PR_OFFLINE_FLAGS				PROP_TAG(PT_LONG, pidFolderMin+0x5)
#define PR_SYNCHRONIZE_FLAGS			PROP_TAG(PT_LONG, pidExchangeNonXmitReservedMin+0x4)


/*------------------------------------------------------------------------
 *
 *	Flags used by the Offline Folders API
 *
 *-----------------------------------------------------------------------*/

#define OF_AVAILABLE_OFFLINE					((ULONG) 0x00000001)
#define OF_FORCE								((ULONG) 0x80000000)

#define SF_DISABLE_STARTUP_SYNC					((ULONG) 0x00000001)

/*------------------------------------------------------------------------
 *
 *	"IExchangeMessageConversion" Interface Declaration
 *
 *	Used to configure message conversion streams
 *
 *-----------------------------------------------------------------------*/

#define EXCHANGE_IEXCHANGEMESSAGECONVERSION_METHODS(IPURE)					\
	MAPIMETHOD(OpenStream)										\
		(THIS_	ULONG						cValues,			\
				LPSPropValue				lpPropArray,		\
				LPSTREAM FAR *				lppStream) IPURE;
#undef		 INTERFACE
#define		 INTERFACE	IExchangeMessageConversion
DECLARE_MAPI_INTERFACE_(IExchangeMessageConversion, IUnknown)
{
	MAPI_IUNKNOWN_METHODS(PURE)
	EXCHANGE_IEXCHANGEMESSAGECONVERSION_METHODS(PURE)
};
#undef	IMPL
#define IMPL

DECLARE_MAPI_INTERFACE_PTR(IExchangeMessageConversion, LPEXCHANGEMESSAGECONVERSION);

#define PR_MESSAGE_SITE_NAME				PROP_TAG(PT_TSTRING, pidExchangeNonXmitReservedMin+0x7)
#define PR_MESSAGE_SITE_NAME_A				PROP_TAG(PT_STRING8, pidExchangeNonXmitReservedMin+0x7)
#define PR_MESSAGE_SITE_NAME_W				PROP_TAG(PT_UNICODE, pidExchangeNonXmitReservedMin+0x7)

#define PR_MESSAGE_PROCESSED				PROP_TAG(PT_BOOLEAN, pidExchangeNonXmitReservedMin+0x8)

#define PR_MSG_BODY_ID						PROP_TAG(PT_LONG, pidExchangeXmitReservedMin-0x03)


#define PR_BILATERAL_INFO					PROP_TAG(PT_BINARY, pidExchangeXmitReservedMin-0x04)
#define PR_DL_REPORT_FLAGS					PROP_TAG(PT_LONG, pidExchangeXmitReservedMin-0x05)

#define PRIV_DL_HIDE_MEMBERS	0x00000001
#define PRIV_DL_REPORT_TO_ORIG	0x00000002
#define PRIV_DL_REPORT_TO_OWNER 0x00000004
#define PRIV_DL_ALLOW_OOF		0x00000008

/*---------------------------------------------------------------------------------
 *
 *	PR_PREVIEW is a folder contents property that is either PR_ABSTRACT
 *		or the first 255 characters of PR_BODY.
 *	PR_PREVIEW_UNREAD is a folder contents property that is either PR_PREVIEW
 *		if the message is not read, or NULL if it is read.
 *
 *---------------------------------------------------------------------------------*/
#define PR_ABSTRACT							PROP_TAG(PT_TSTRING, pidExchangeXmitReservedMin-0x06)
#define PR_ABSTRACT_A						PROP_TAG(PT_STRING8, pidExchangeXmitReservedMin-0x06)
#define PR_ABSTRACT_W						PROP_TAG(PT_UNICODE, pidExchangeXmitReservedMin-0x06)

#define PR_PREVIEW							PROP_TAG(PT_TSTRING, pidExchangeXmitReservedMin-0x07)
#define PR_PREVIEW_A						PROP_TAG(PT_STRING8, pidExchangeXmitReservedMin-0x07)
#define PR_PREVIEW_W						PROP_TAG(PT_UNICODE, pidExchangeXmitReservedMin-0x07)

#define PR_PREVIEW_UNREAD					PROP_TAG(PT_TSTRING, pidExchangeXmitReservedMin-0x08)
#define PR_PREVIEW_UNREAD_A					PROP_TAG(PT_STRING8, pidExchangeXmitReservedMin-0x08)
#define PR_PREVIEW_UNREAD_W					PROP_TAG(PT_UNICODE, pidExchangeXmitReservedMin-0x08)

//
//	Informs IMAIL that full fidelity should be discarded for this message.
//
#define PR_DISABLE_FULL_FIDELITY			PROP_TAG(PT_BOOLEAN, pidRenMsgFldMin+0x72)

// file attributes for messages / folders
// need to be in REN property range in order to replicate
#define PR_ATTR_HIDDEN						PROP_TAG(PT_BOOLEAN, pidRenMsgFldMin+0x74)
#define PR_ATTR_SYSTEM						PROP_TAG(PT_BOOLEAN, pidRenMsgFldMin+0x75)
#define PR_ATTR_READONLY					PROP_TAG(PT_BOOLEAN, pidRenMsgFldMin+0x76)

// Flag indicating whether msg has been read or not (read-only prop for now - not replicated).
#define PR_READ								PROP_TAG(PT_BOOLEAN, pidStoreNonTransMin+0x29)

//	Administrative security descriptor for a folder, if present.
//
#define PR_ADMIN_SECURITY_DESCRIPTOR			PROP_TAG(PT_BINARY, 0x3d21)
//
//	Win32 compatible representation of folder/message security descriptor
//
#define PR_WIN32_SECURITY_DESCRIPTOR			PROP_TAG(PT_BINARY, 0x3d22)
//
//	TRUE if PR_NT_SECURITY_DESCRIPTOR describes non Win32 ACL semantics.
//	If this is set, components that use PR_WIN32_SECURITY_DESCRIPTOR cannot
//	allow modification of PR_NT_SECURITY_DESCRIPTOR (or PR_DEFAULT_MESSAGE_SD).
//
#define PR_NON_WIN32_ACL						PROP_TAG(PT_BOOLEAN, 0x3d23)

//
//	TRUE if any items in the folder contain item level ACLs
//
#define PR_ITEM_LEVEL_ACL						PROP_TAG(PT_BOOLEAN, 0x3d24)

#define PR_DAV_TRANSFER_SECURITY_DESCRIPTOR		PROP_TAG(PT_BINARY, 0x0E84)
//
//	XML formatted versions of the NT SECURITY DESCRIPTOR properties
#define PR_NT_SECURITY_DESCRIPTOR_AS_XML			PROP_TAG(PT_TSTRING, pidStoreNonTransMin+0x2A)
#define	 PR_NT_SECURITY_DESCRIPTOR_AS_XML_A			PROP_TAG(PT_STRING8, pidStoreNonTransMin+0x2A)
#define	 PR_NT_SECURITY_DESCRIPTOR_AS_XML_W			PROP_TAG(PT_UNICODE, pidStoreNonTransMin+0x2A)
#define PR_ADMIN_SECURITY_DESCRIPTOR_AS_XML			PROP_TAG(PT_TSTRING, pidStoreNonTransMin+0x2B)
#define PR_ADMIN_SECURITY_DESCRIPTOR_AS_XML_A		PROP_TAG(PT_STRING8, pidStoreNonTransMin+0x2B)
#define PR_ADMIN_SECURITY_DESCRIPTOR_AS_XML_W	PROP_TAG(PT_UNICODE, pidStoreNonTransMin+0x2B)


/*------------------------------------------------------------------------------------
*
*	OWA Info Property
*
*------------------------------------------------------------------------------------*/
#define PR_OWA_URL								PROP_TAG (PT_STRING8, pidRenMsgFldMin+0x71)


//$ The value of this property ID will change in the future.  Do not rely on
//$ its current value.	Rely on the define only.
#define PR_STORE_SLOWLINK						PROP_TAG(PT_BOOLEAN, 0x7c0a)


/*
 * Registry locations of settings
 */
#if defined(WIN32) && !defined(MAC)
#define SZ_HPC_V2	"Software\\Microsoft\\Windows CE Services"
#define SZ_HPC_V2_MAJOR "MajorVersion"	// = 2
#define SZ_HPC_V2_MINOR "MinorVersion"	// = 0 or 1

#define SZ_HPC_V1	"Software\\Microsoft\\Pegasus"
#define SZ_HPC_V1_MAJOR "MajorVersion"	// = 1 Major and Minor numbers didn't appear
#define SZ_HPC_V1_MINOR "MinorVersion"	// = 1 until after v1.0 was released
#define SZ_HPC_V1_0		"InstalledDir"	// present for v1.0

#define SZ_OUTL_OST_OPTIONS "Software\\Microsoft\\Office\\8.0\\Outlook\\OST"
#define SZ_NO_OST "NoOST"
#define NO_OST_FLAG_ALLOWED		0	// OST's are allowed on the machine
#define NO_OST_FLAG_CACHE_ONLY	1	// OST can only be used as a cache
#define NO_OST_FLAG_NOT_ALLOWED 2	// OST's are not allowed on the machine
#define NO_OST_FLAG_NO_CACHE	3	// OST's are not allowed as a cache
#define NO_OST_DEFAULT			NO_OST_FLAG_ALLOWED
#endif

/*
 *	Special GUID property for suppressing sync events for folders. If
 *	this property is set on a folder (any GIUD value), sync events will
 *	be suppressed for that folder. The caller can then selectively enable
 *	sync events for that folder by specifying the corresponding GUID in
 *	the NEWLOGON object.
 */
#define PR_SYNCEVENT_SUPPRESS_GUID				PROP_TAG( PT_BINARY,	0x3880 )

/*
 *	The following are the well-known GUIDS for the different special folders.
 *	By default, sync events are suppressed for these folders. You can insert
 *	GUIDs into your NEWLOGON object to selectively enable sync events for
 *	each folder.
 */
// {B2DC5B57-AF2D-4915-BAE3-90E5BDFB0070}
//static const GUID guidOutboxSyncEvents =
//{
//	0xb2dc5b57, 0xaf2d, 0x4915,
//	{
//		0xba, 0xe3, 0x90, 0xe5, 0xbd, 0xfb, 0x0, 0x70
//	}
//};
//
// {2185EE91-28CD-4d9b-BFB4-BC49BB1DD8C0}
//static const GUID guidMTSInSyncEvents =
//{
//	0x2185ee91, 0x28cd, 0x4d9b,
//	{
//		0xbf, 0xb4, 0xbc, 0x49, 0xbb, 0x1d, 0xd8, 0xc0
//	}
//};
//
// {1BDBAFD3-1384-449b-A200-DE4745B07839}
//static const GUID guidMTSOutSyncEvents =
//{
//	0x1bdbafd3, 0x1384, 0x449b,
//	{
//		0xa2, 0x0, 0xde, 0x47, 0x45, 0xb0, 0x78, 0x39
//	}
//};
//
// {221ED74D-0B5C-4c0e-8807-23AFDD8AC2FF}
//static const GUID guidTransportTempFolderSyncEvents =
//{
//	0x221ed74d, 0xb5c, 0x4c0e,
//	{
//		0x88, 0x7, 0x23, 0xaf, 0xdd, 0x8a, 0xc2, 0xff
//	}
//};


/*
 *	Lock properties
 */
 //REVIEW:: some of these definitions appear both in MAPITAGS.H and EDKMDB.H
 //one set of definitions should be removed
#define PR_LOCK_BRANCH_ID						PROP_TAG( PT_I8,		0x3800 )
#define PR_LOCK_RESOURCE_FID					PROP_TAG( PT_I8,		0x3801 )
#define PR_LOCK_RESOURCE_DID					PROP_TAG( PT_I8,		0x3802 )
#define PR_LOCK_RESOURCE_VID					PROP_TAG( PT_I8,		0x3803 )
#define PR_LOCK_ENLISTMENT_CONTEXT				PROP_TAG( PT_BINARY,	0x3804 )
#define PR_LOCK_TYPE							PROP_TAG( PT_SHORT,		0x3805 )
#define PR_LOCK_SCOPE							PROP_TAG( PT_SHORT,		0x3806 )
#define PR_LOCK_TRANSIENT_ID					PROP_TAG( PT_BINARY,	0x3807 )
#define PR_LOCK_DEPTH							PROP_TAG( PT_LONG,		0x3808 )
#define PR_LOCK_TIMEOUT							PROP_TAG( PT_LONG,		0x3809 )
#define PR_LOCK_EXPIRY_TIME						PROP_TAG( PT_SYSTIME,	0x380a )
#define PR_LOCK_GLID							PROP_TAG( PT_BINARY,	0x380b )
#define PR_LOCK_NULL_URL_W						PROP_TAG( PT_UNICODE,	0x380c )

/*
 * Lock flags
 */
#define LOCK_NON_PERSISTENT							0x00000001
#define LOCK_BLOCKING_MID_LOCK						0x00000002
#define LOCK_NULL_RESOURCE							0x00000004
#define LOCK_READ_ACCESS_CHECK_ONLY					0x00000008
#define LOCK_WRITE_THROUGH_GOP						0x00010000
// This bit is reserved and must not be set!
#define LOCK_RESERVED								0x80000000

/*
 * Unlock flags
 */
#define UNLOCK_CHECKIN_ABORT						0x00000001
#define UNLOCK_CHECKIN_KEEP_LOCKED					0x00000002
#define UNLOCK_BLOCKING_MID_LOCK_ALL				0x00000004
#define UNLOCK_BLOCKING_MID_LOCK_LOGON_ONLY			0x00000008
#define UNLOCK_NULL_RESOURCE						0x00000010
#define UNLOCK_WRITE_THROUGH_GOP					0x00010000

/*
 * Versioning flags for folder
 */
#define wNonVersionedFolder				((WORD)0x0000)
#define wVersionedFolderSimple			((WORD)0x0001)
#define wVersionedFolderAuto			((WORD)0x0002)	//When you auto version it is simple versioned as well.

/*
 * Versioning operation codes for version rows in ptagVersionedOperation
 */
#define fVersionedDelete		((ULONG)0x01)
#define fVersionedUnDelete		((ULONG)0x02)
#define fVersionedPin			((ULONG)0x04)
#define fVersionedUnPin			((ULONG)0x08)
#define fVersionedMoveIn		((ULONG)0x10)
#define fVersionedMoveOut		((ULONG)0x20)

/*------------------------------------------------------------------------
 *
 *	LocalStore specific internal properties
 *
 *	These are properties that will be used internally by local store.
 *	Properties that are listed here are used in components other than the local store
 *-----------------------------------------------------------------------*/
#define pidLISMsgFolderPropMin		pidLocalStoreInternalMin+0xa0						//	0x65a0
#define pidLISMsgFolderPropMax		pidLocalStoreInternalMin+0xc0						//	0x65c0

#define pidLISErrorCodeMin			pidLISMsgFolderPropMin+0xa							//	0x65aa
#define pidLISErrorCodeMax			pidLISMsgFolderPropMin+0x10							//	0x65b0

#define pidLISInterfacePropMin		pidLocalStoreInternalMin+0xd0						//	0x65d0
#define pidLISInterfacePropMax		pidLocalStoreInternalMin+0xe0						//	0x65e0

#define ptagLISSubfolders			PROP_TAG( PT_BOOLEAN,	pidLocalStoreInternalMin+0x0)
#define ptagLISUnreadCount			PROP_TAG( PT_LONG,		pidLocalStoreInternalMin+0x1)

#define ptagLISErrorCode			PROP_TAG( PT_LONG,		pidLISErrorCodeMin+0x0)		//	PROP_TAG(PT_LONG,		0x65aa)
#define ptagLISErrorItemType		PROP_TAG( PT_LONG,		pidLISErrorCodeMin+0x1)		//	PROP_TAG(PT_LONG,		0x65ab)
#define ptagLISErrorOperation		PROP_TAG( PT_LONG,		pidLISErrorCodeMin+0x2)		//	PROP_TAG(PT_LONG,		0x65ac)
#define ptagLISErrorItemUrl			PROP_TAG( PT_UNICODE,	pidLISErrorCodeMin+0x3)		//	PROP_TAG(PT_UNICODE,	0x65ad)
#define ptagLISErrorSourceUrl		PROP_TAG( PT_UNICODE,	pidLISErrorCodeMin+0x4)		//	PROP_TAG(PT_UNICODE,	0x65ae)
#define ptagLISModifiedPropertyList PROP_TAG( PT_UNICODE,	pidLISErrorCodeMin+0x5)		//	PROP_TAG(PT_UNICODE,	0x65af)
#define ptagLISExtendedErrorinfo	PROP_TAG( PT_LONG,		pidLISErrorCodeMin+0x6)		//	PROP_TAG(PT_LONG,		0x65b0)

// Not in msgfolder prop range
#define ptagLISErrorLogUrl			PROP_TAG( PT_UNICODE,	pidLocalStoreInternalMin+0x70)		//	PROP_TAG(PT_UNICODE,	0x6570)

// Ptags used between EXOLEDB and LSCache on client machine to pass
// along the actual client SQL query from EXOLEDB to LSCache in the RES_COMMENT
// prop val array of an SRestriction.  These ptags are never actually sent accross the
// wire or stored as properties on objects in the cache.
//
// ptagSql =	The identifying property for the SQL restriction.
//				The value will be the original complete clause.
#define ptagSql						PROP_TAG(PT_UNICODE,	pidLISInterfacePropMin+0x0)
#define ptagSqlSelect				PROP_TAG(PT_UNICODE,	pidLISInterfacePropMin+0x1)
#define ptagSqlFrom					PROP_TAG(PT_UNICODE,	pidLISInterfacePropMin+0x2)
#define ptagSqlWhere				PROP_TAG(PT_UNICODE,	pidLISInterfacePropMin+0x3)
#define ptagSqlOrder				PROP_TAG(PT_UNICODE,	pidLISInterfacePropMin+0x4)
#define ptagSqlGroup				PROP_TAG(PT_UNICODE,	pidLISInterfacePropMin+0x5)

#define PR_RULE_SERVER_RULE_ID		PROP_TAG(PT_I8, pidLISMsgFolderPropMin+0x0)			// Corresponding server RUID for LIS

// this is a hackish property to be used by sync event code to say that changes
// need client refresh. The only valid value is TRUE. See #168797 for more info
#define PR_FORCE_CLIENT_REFRESH		PROP_TAG(PT_BOOLEAN, pidLISMsgFolderPropMin+0x1)

/*------------------------------------------------------------------------
 *
 *	Anti-Virus products integration support
 *
 *	All properties are read-only.
 *-----------------------------------------------------------------------*/

// Name and version of anti-virus product that performed the last scan of an item.
#define PR_ANTIVIRUS_VENDOR				PROP_TAG(PT_STRING8,	pidStoreNonTransMin+0x45)	// 0x0E85001E
#define PR_ANTIVIRUS_VERSION			PROP_TAG(PT_LONG,		pidStoreNonTransMin+0x46)	// 0x0E860003

// Results ot the last scan of an item.
#define PR_ANTIVIRUS_SCAN_STATUS		PROP_TAG(PT_LONG,		pidStoreNonTransMin+0x47)	// 0x0E870003

// List of virus identification strings of all viruses found by the last scan, if virus has been cleaned
// or detected, separated by commans. Empty string if no virus has been found.
#define PR_ANTIVIRUS_SCAN_INFO			PROP_TAG(PT_STRING8,	pidStoreNonTransMin+0x48)	// 0x0E88001F

/*
 * Possible values of PR_ANTIVIRUS_SCAN_STATUS
 */
// Anti-virus product has completed scanning of an item, and found no virus.
#define ANTIVIRUS_SCAN_NO_VIRUS			0

// Anti-virus product has detected a virus in an item, or assumed the item may contain a virus
// based on item's properties, like filename or content type.
#define ANTIVIRUS_SCAN_VIRUS_PRESENT	1

// Anti-virus product has detected a virus in an item, and applied changes to remove the virus.
// The item should be safe to use after modifications.
#define ANTIVIRUS_SCAN_VIRUS_CLEANED	2

// Anti-virus product has detected a virus in an item, and has requested that the message be
// deleted.	 This item shouldn't be allowed to be opened.
#define ANTIVIRUS_SCAN_VIRUS_DELETED	3

// Presents the same list as PR_DISPLAY_TO except uses the format "[addrtype1:addr1]; [addrtype2:addr2]"
#define PR_ADDR_TO					PROP_TAG(PT_TSTRING, pidStoreNonTransMin+0x57) // 0x0E97
#define PR_ADDR_TO_A				PROP_TAG(PT_STRING8, pidStoreNonTransMin+0x57)
#define PR_ADDR_TO_W				PROP_TAG(PT_UNICODE, pidStoreNonTransMin+0x57)

// Presents the same list as PR_DISPLAY_CC except uses the format "[addrtype1:addr1]; [addrtype2:addr2]"
#define PR_ADDR_CC					PROP_TAG(PT_TSTRING, pidStoreNonTransMin+0x58) // 0x0E98
#define PR_ADDR_CC_A				PROP_TAG(PT_STRING8, pidStoreNonTransMin+0x58)
#define PR_ADDR_CC_W				PROP_TAG(PT_UNICODE, pidStoreNonTransMin+0x58)


// This property IS NO LONGER USED. I've left it here to avoid possible build break.
#define ptagLISNewMail				PROP_TAG(PT_BOOLEAN, 0x65c5)

#endif	//EDKMDB_INCLUDED
