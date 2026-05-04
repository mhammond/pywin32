/*
 *	M A P I G U I D . H
 *
 *	Master definitions of all GUID's for MAPI.
 *
 *	When included without INITGUID defined, this header file
 *	defines symbols that reference IIDs elsewhere.
 *
 *	When included with INITGUID defined and a "USES_IID_I..."
 *	statement for each IID used by the subsystem, it generates the
 *	bytes for those actual IIDs into the associated object file.
 *
 *	This range of 256 GUIDs reserved by OLE for MAPI use October 5, 1992.
 *
 *  Copyright (c) 2009 Microsoft Corporation. All Rights Reserved.
 */

/*
 *	List of GUIDS allocated by MAPI
 *	
 *	0x00020300	IID_IMAPISession
 *	0x00020301	IID_IMAPITable
 *	0x00020302	IID_IMAPIAdviseSink
 *	0x00020303	IID_IMAPIProp
 *	0x00020304	IID_IProfSect
 *	0x00020305	IID_IMAPIStatus
 *	0x00020306	IID_IMsgStore
 *	0x00020307	IID_IMessage
 *	0x00020308	IID_IAttachment
 *	0x00020309	IID_IAddrBook
 *	0x0002030A	IID_IMailUser
 *	0x0002030B	IID_IMAPIContainer
 *	0x0002030C	IID_IMAPIFolder
 *	0x0002030D	IID_IABContainer
 *	0x0002030E	IID_IDistList
 *	0x0002030F	IID_IMAPISup
 *	0x00020310	IID_IMSProvider
 *	0x00020311	IID_IABProvider
 *	0x00020312	IID_IXPProvider
 *	0x00020313	IID_IMSLogon
 *	0x00020314	IID_IABLogon
 *	0x00020315	IID_IXPLogon
 *	0x00020316	IID_IMAPITableData
 *	0x00020317	IID_IMAPISpoolerInit
 *	0x00020318	IID_IMAPISpoolerSession
 *	0x00020319	IID_ITNEF
 *	0x0002031A	IID_IMAPIPropData
 *	0x0002031B	IID_IMAPIControl
 *	0x0002031C	IID_IProfAdmin
 *	0x0002031D	IID_IMsgServiceAdmin
 *	0x0002031E	IID_IMAPISpoolerService
 *	0x0002031F	IID_IMAPIProgress
 *	0x00020320	IID_ISpoolerHook
 *	0x00020321	IID_IMAPIViewContext
 *	0x00020322	IID_IMAPIFormMgr
 *	0x00020323	IID_IEnumMAPIFormProp
 *	0x00020324	IID_IMAPIFormInfo
 *	0x00020325	IID_IProviderAdmin
 *	0x00020327	IID_IMAPIForm
 *	0x00020328	PS_MAPI
 *	0x00020329	PS_PUBLIC_STRINGS
 *	0x0002032A	IID_IPersistMessage
 *	0x0002032B	IID_IMAPIViewAdviseSink
 *	0x0002032C	IID_IStreamDocfile
 *	0x0002032D	IID_IMAPIFormProp
 *	0x0002032E	IID_IMAPIFormContainer
 *	0x0002032F	IID_IMAPIFormAdviseSink
 *	0x00020330	IID_IStreamTnef
 *	0x00020350	IID_IMAPIFormFactory
 *	0x00020370	IID_IMAPIMessageSite
 *	0x00020380	PS_ROUTING_EMAIL_ADDRESSES
 *	0x00020381	PS_ROUTING_ADDRTYPE
 *	0x00020382	PS_ROUTING_DISPLAY_NAME
 *	0x00020383	PS_ROUTING_ENTRYID
 *	0x00020384	PS_ROUTING_SEARCH_KEY
 *	0x00020385	MUID_PROFILE_INSTANCE
 *	0x00020397	IID_IMAPIClientShutdown
 *	0x00020398	IID_IMAPIProviderShutdown
 *	
 *	The remaining GUIDs from 0x00020300 to 0x000203FF are reserved by
 *	MAPI for future use.  The current maximum used by MAPI is 0x00020398
 *
 */

#ifndef MAPIGUID_H
#ifdef  INITGUID
#define MAPIGUID_H
#if _MSC_VER > 1000
#pragma once
#endif
#endif

/* Derive from IUnknown */
#if !defined(INITGUID) || defined(USES_IID_IMAPISession)
DEFINE_OLEGUID(IID_IMAPISession,	0x00020300, 0, 0);
#endif
#if !defined(INITGUID) || defined(USES_IID_IMAPITable)
DEFINE_OLEGUID(IID_IMAPITable,		0x00020301, 0, 0);
#endif
#if !defined(INITGUID) || defined(USES_IID_IMAPIAdviseSink)
DEFINE_OLEGUID(IID_IMAPIAdviseSink,	0x00020302, 0, 0);
#endif
#if !defined(INITGUID) || defined(USES_IID_IMAPIControl)
DEFINE_OLEGUID(IID_IMAPIControl,	0x0002031B, 0, 0);
#endif
#if !defined(INITGUID) || defined(USES_IID_IProfAdmin)
DEFINE_OLEGUID(IID_IProfAdmin,		0x0002031C, 0, 0);
#endif
#if !defined(INITGUID) || defined(USES_IID_IMsgServiceAdmin)
DEFINE_OLEGUID(IID_IMsgServiceAdmin,0x0002031D, 0, 0);
#endif
#if !defined(INITGUID) || defined(USES_IID_IProviderAdmin)
DEFINE_OLEGUID(IID_IProviderAdmin,	0x00020325, 0, 0);
#endif
#if !defined(INITGUID) || defined(USES_IID_IMAPIProgress)
DEFINE_OLEGUID(IID_IMAPIProgress,	0x0002031F, 0, 0);
#endif

/* MAPIProp or derive from MAPIProp */
#if !defined(INITGUID) || defined(USES_IID_IMAPIProp)
DEFINE_OLEGUID(IID_IMAPIProp,		0x00020303, 0, 0);
#endif
#if !defined(INITGUID) || defined(USES_IID_IProfSect)
DEFINE_OLEGUID(IID_IProfSect,		0x00020304, 0, 0);
#endif
#if !defined(INITGUID) || defined(USES_IID_IMAPIStatus)
DEFINE_OLEGUID(IID_IMAPIStatus,			0x00020305, 0, 0);
#endif
#if !defined(INITGUID) || defined(USES_IID_IMsgStore)
DEFINE_OLEGUID(IID_IMsgStore,		0x00020306, 0, 0);
#endif
#if !defined(INITGUID) || defined(USES_IID_IMessage)
DEFINE_OLEGUID(IID_IMessage,		0x00020307, 0, 0);
#endif
#if !defined(INITGUID) || defined(USES_IID_IAttachment)
DEFINE_OLEGUID(IID_IAttachment,		0x00020308, 0, 0);
#endif
#if !defined(INITGUID) || defined(USES_IID_IAddrBook)
DEFINE_OLEGUID(IID_IAddrBook,		0x00020309, 0, 0);
#endif
#if !defined(INITGUID) || defined(USES_IID_IMailUser)
DEFINE_OLEGUID(IID_IMailUser,		0x0002030A, 0, 0);
#endif

/* MAPIContainer or derive from MAPIContainer */
#if !defined(INITGUID) || defined(USES_IID_IMAPIContainer)
DEFINE_OLEGUID(IID_IMAPIContainer,	0x0002030B, 0, 0);
#endif
#if !defined(INITGUID) || defined(USES_IID_IMAPIFolder)
DEFINE_OLEGUID(IID_IMAPIFolder,		0x0002030C, 0, 0);
#endif
#if !defined(INITGUID) || defined(USES_IID_IABContainer)
DEFINE_OLEGUID(IID_IABContainer,	0x0002030D, 0, 0);
#endif
#if !defined(INITGUID) || defined(USES_IID_IDistList)
DEFINE_OLEGUID(IID_IDistList,		0x0002030E, 0, 0);
#endif

/* MAPI Support Object */
#if !defined(INITGUID) || defined(USES_IID_IMAPISup)
DEFINE_OLEGUID(IID_IMAPISup,		0x0002030F, 0, 0);
#endif

/* Provider INIT objects */
#if !defined(INITGUID) || defined(USES_IID_IMSProvider)
DEFINE_OLEGUID(IID_IMSProvider,		0x00020310, 0, 0);
#endif
#if !defined(INITGUID) || defined(USES_IID_IABProvider)
DEFINE_OLEGUID(IID_IABProvider,		0x00020311, 0, 0);
#endif
#if !defined(INITGUID) || defined(USES_IID_IXPProvider)
DEFINE_OLEGUID(IID_IXPProvider,		0x00020312, 0, 0);
#endif

/* Provider LOGON Objects */
#if !defined(INITGUID) || defined(USES_IID_IMSLogon)
DEFINE_OLEGUID(IID_IMSLogon,		0x00020313, 0, 0);
#endif
#if !defined(INITGUID) || defined(USES_IID_IABLogon)
DEFINE_OLEGUID(IID_IABLogon,		0x00020314, 0, 0);
#endif
#if !defined(INITGUID) || defined(USES_IID_IXPLogon)
DEFINE_OLEGUID(IID_IXPLogon,		0x00020315, 0, 0);
#endif

/* IMAPITable-in-memory Table Data Object */
#if !defined(INITGUID) || defined(USES_IID_IMAPITableData)
DEFINE_OLEGUID(IID_IMAPITableData,	0x00020316, 0, 0);
#endif

/* MAPI Spooler Init Object (internal) */
#if !defined(INITGUID) || defined(USES_IID_IMAPISpoolerInit)
DEFINE_OLEGUID(IID_IMAPISpoolerInit,	0x00020317, 0, 0);
#endif

/* MAPI Spooler Session Object (internal) */
#if !defined(INITGUID) || defined(USES_IID_IMAPISpoolerSession)
DEFINE_OLEGUID(IID_IMAPISpoolerSession,	0x00020318, 0, 0);
#endif

/* MAPI TNEF Object Interface */
#if !defined(INITGUID) || defined(USES_IID_ITNEF)
DEFINE_OLEGUID(IID_ITNEF,			0x00020319, 0, 0);
#endif

/* IMAPIProp-in-memory Property Data Object */
#if !defined(INITGUID) || defined(USES_IID_IMAPIPropData)
DEFINE_OLEGUID(IID_IMAPIPropData,	0x0002031A, 0, 0);
#endif

/* MAPI Spooler Hook Object */
#if !defined(INITGUID) || defined(USES_IID_ISpoolerHook)
DEFINE_OLEGUID(IID_ISpoolerHook,	0x00020320, 0, 0);
#endif

/* MAPI Spooler Service Object */
#if !defined(INITGUID) || defined(USES_IID_IMAPISpoolerService)
DEFINE_OLEGUID(IID_IMAPISpoolerService,	0x0002031E, 0, 0);
#endif

/* MAPI forms, form manager, etc. */
#if !defined(INITGUID) || defined(USES_IID_IMAPIViewContext)
DEFINE_OLEGUID(IID_IMAPIViewContext,	0x00020321, 0, 0);
#endif
#if !defined(INITGUID) || defined(USES_IID_IMAPIFormMgr)
DEFINE_OLEGUID(IID_IMAPIFormMgr,	0x00020322, 0, 0);
#endif
#if !defined(INITGUID) || defined(USES_IID_IEnumMAPIFormProp)
DEFINE_OLEGUID(IID_IEnumMAPIFormProp,	0x00020323, 0, 0);
#endif
#if !defined(INITGUID) || defined(USES_IID_IMAPIFormInfo)
DEFINE_OLEGUID(IID_IMAPIFormInfo,	0x00020324, 0, 0);
#endif
#if !defined(INITGUID) || defined(USES_IID_IMAPIForm)
DEFINE_OLEGUID(IID_IMAPIForm,	0x00020327, 0, 0);
#endif


/* Well known guids for name<->id mappings */

/*  The name of MAPI's property set  */
#if !defined(INITGUID) || defined(USES_PS_MAPI)
DEFINE_OLEGUID(PS_MAPI,	0x00020328, 0, 0);
#endif

/*  The name of the set of public strings  */
#if !defined(INITGUID) || defined(USES_PS_PUBLIC_STRINGS)
DEFINE_OLEGUID(PS_PUBLIC_STRINGS,	0x00020329, 0, 0);
#endif


/*
 * Additional well known guids for name<->id mappings:
 * https://learn.microsoft.com/en-us/office/client-developer/outlook/mapi/commonly-used-property-sets
 */

/*  Calendar related properties */
#if !defined(INITGUID) || defined(USES_PSETID_Appointment)
DEFINE_OLEGUID(PSETID_Appointment, 0x00062002, 0, 0);
#endif

#if !defined(INITGUID) || defined(USES_PSETID_Meeting)
DEFINE_GUID(PSETID_Meeting, 0x6ED8DA90, 0x450B, 0x101B, 0x98, 0xDA, 0, 0xAA, 0, 0x3F, 0x13, 0x05);
#endif

/*  Common properties */
#if !defined(INITGUID) || defined(USES_PSETID_Common)
DEFINE_OLEGUID(PSETID_Common, 0x00062008, 0, 0);
#endif

/*  Contact related properties */
#if !defined(INITGUID) || defined(USES_PSETID_Address)
DEFINE_OLEGUID(PSETID_Address, 0x00062004, 0, 0);
#endif

/*  Email related properties */
#if !defined(INITGUID) || defined(USES_PS_INTERNET_HEADERS)
DEFINE_OLEGUID(PS_INTERNET_HEADERS, 0x00020386, 0, 0);
#endif

/*  General messaging related properties */
#if !defined(INITGUID) || defined(USES_PSETID_Report)
DEFINE_OLEGUID(PSETID_Report, 0x00062013, 0, 0);
#endif

/*  Journal related properties */
#if !defined(INITGUID) || defined(USES_PSETID_Log)
DEFINE_OLEGUID(PSETID_Log, 0x0006200A, 0, 0);
#endif

/*  Messaging related properties */
#if !defined(INITGUID) || defined(USES_PSETID_Messaging)
DEFINE_GUID(PSETID_Messaging, 0x41F28F13, 0x83F4, 0x4114, 0xA5, 0x84, 0xEE, 0xDB, 0x5A, 0x6B, 0x0B, 0xFF);
#endif

/*  Remote messaging related properties */
#if !defined(INITGUID) || defined(USES_PSETID_Remote)
DEFINE_OLEGUID(PSETID_Remote, 0x00062014, 0, 0);
#endif

/*  RSS feed related properties */
#if !defined(INITGUID) || defined(USES_PSETID_PostRss)
DEFINE_OLEGUID(PSETID_PostRss, 0x00062041, 0, 0);
#endif

/*  Sharing related properties */
#if !defined(INITGUID) || defined(USES_PSETID_Sharing)
DEFINE_OLEGUID(PSETID_Sharing, 0x00062040, 0, 0);
#endif

/*  Sticky note related properties */
#if !defined(INITGUID) || defined(USES_PSETID_Note)
DEFINE_OLEGUID(PSETID_Note, 0x0006200E, 0, 0);
#endif

/*  Sync related properties */
#if !defined(INITGUID) || defined(USES_SETID_AirSync)
DEFINE_GUID(SETID_AirSync, 0x71035549, 0x0739, 0x4DCB, 0x91, 0x63, 0, 0xF0, 0x58, 0x0D, 0xBB, 0xDF);
#endif

/*  Task related properties */
#if !defined(INITGUID) || defined(USES_PSETID_Task)
DEFINE_OLEGUID(PSETID_Task, 0x00062003, 0, 0);
#endif

/*  Unified messaging related properties */
#if !defined(INITGUID) || defined(USES_PSETID_UnifiedMessaging)
DEFINE_GUID(PSETID_UnifiedMessaging, 0x4442858E, 0xA9E3, 0x4E80, 0xB9, 0, 0x31, 0x7A, 0x21, 0x0C, 0xC1, 0x5B);
#endif

/*
 * Additional well known guids for name<->id mappings:
 * https://learn.microsoft.com/en-us/openspecs/exchange_server_protocols/ms-oxprops/cc9d955b-1492-47de-9dce-5bdea80a3323
 */

/*  Extracted entities related properties */
#if !defined(INITGUID) || defined(USES_PSETID_XmlExtractedEntities)
DEFINE_GUID(PSETID_XmlExtractedEntities, 0x23239608, 0x685D, 0x4732, 0x9C, 0x55, 0x4C, 0x95, 0xCB, 0x4E, 0x8E, 0x33);
#endif

/*  Attachment related properties */
#if !defined(INITGUID) || defined(USES_PSETID_Attachment)
DEFINE_GUID(PSETID_Attachment, 0x96357F7F, 0x59E1, 0x47D0, 0x99, 0xA7, 0x46, 0x51, 0x5C, 0x18, 0x3B, 0x54);
#endif




/* MAPI forms, form manager, (cont) */
#if !defined(INITGUID) || defined(USES_IID_IPersistMessage)
DEFINE_OLEGUID(IID_IPersistMessage,	0x0002032A, 0, 0);
#endif

/* IMAPIViewAdviseSink */
#if !defined(INITGUID) || defined(USES_IID_IMAPIViewAdviseSink)
DEFINE_OLEGUID(IID_IMAPIViewAdviseSink,	0x0002032B, 0, 0);
#endif

/* Message Store OpenProperty */
#if !defined(INITGUID) || defined(USES_IID_IStreamDocfile)
DEFINE_OLEGUID(IID_IStreamDocfile, 0x0002032C, 0, 0);
#endif

/* IMAPIFormProp */
#if !defined(INITGUID) || defined(USES_IID_IMAPIFormProp)
DEFINE_OLEGUID(IID_IMAPIFormProp,	0x0002032D, 0, 0);
#endif

/* IMAPIFormContainer */
#if !defined(INITGUID) || defined(USES_IID_IMAPIFormContainer)
DEFINE_OLEGUID(IID_IMAPIFormContainer, 0x0002032E, 0, 0);
#endif

/* IMAPIFormAdviseSink */
#if !defined(INITGUID) || defined(USES_IID_IMAPIFormAdviseSink)
DEFINE_OLEGUID(IID_IMAPIFormAdviseSink, 0x0002032F, 0, 0);
#endif

/* TNEF OpenProperty */
#if !defined(INITGUID) || defined(USES_IID_IStreamTnef)
DEFINE_OLEGUID(IID_IStreamTnef, 0x00020330, 0, 0);
#endif

/* IMAPIFormFactory */
#if !defined(INITGUID) || defined(USES_IID_IMAPIFormFactory)
DEFINE_OLEGUID(IID_IMAPIFormFactory, 0x00020350, 0, 0);
#endif

/* IMAPIMessageSite */
#if !defined(INITGUID) || defined(USES_IID_IMAPIMessageSite)
DEFINE_OLEGUID(IID_IMAPIMessageSite, 0x00020370, 0, 0);
#endif



/* Well known guids routing property sets.
   Usefull when writing applications that route documents
   (i.e. Workflow) across gateways.  Gateways that speak MAPI
   should convert the properties found in the follow property
   sets appropriately. */

/*  PS_ROUTING_EMAIL_ADDRESSES:  Addresses that need converting at gateways, etc. */
#if !defined(INITGUID) || defined(USES_PS_ROUTING_EMAIL_ADDRESSES)
DEFINE_OLEGUID(PS_ROUTING_EMAIL_ADDRESSES,	0x00020380, 0, 0);
#endif

/*  PS_ROUTING_ADDRTYPE:  Address types that need converting at gateways, etc. */
#if !defined(INITGUID) || defined(USES_PS_ROUTING_ADDRTYPE)
DEFINE_OLEGUID(PS_ROUTING_ADDRTYPE,	0x00020381, 0, 0);
#endif

/*  PS_ROUTING_DISPLAY_NAME:  Display Name that corresponds to the other props */
#if !defined(INITGUID) || defined(USES_PS_ROUTING_DISPLAY_NAME)
DEFINE_OLEGUID(PS_ROUTING_DISPLAY_NAME,	0x00020382, 0, 0);
#endif

/*  PS_ROUTING_ENTRYID:  (optional) EntryIDs that need converting at gateways, etc. */
#if !defined(INITGUID) || defined(USES_PS_ROUTING_ENTRYID)
DEFINE_OLEGUID(PS_ROUTING_ENTRYID,	0x00020383, 0, 0);
#endif

/*  PS_ROUTING_SEARCH_KEY:  (optional) search keys that need converting at gateways, etc. */
#if !defined(INITGUID) || defined(USES_PS_ROUTING_SEARCH_KEY)
DEFINE_OLEGUID(PS_ROUTING_SEARCH_KEY,	0x00020384, 0, 0);
#endif

/*	MUID_PROFILE_INSTANCE
	Well known section in a profile which contains a property (PR_SEARCH_KEY) which is unique
	for any given profile.  Applications and providers can depend on this value as being
	different for each unique profile. */
#if !defined(INITGUID) || defined(USES_MUID_PROFILE_INSTANCE)
DEFINE_OLEGUID(MUID_PROFILE_INSTANCE, 0x00020385, 0, 0);
#endif


/* Interface GUIDs for Fast Shutdown support */

/* IMAPIClientShutdown */
#if !defined(INITGUID) || defined(USES_IID_IMAPIClientShutdown)
DEFINE_OLEGUID(IID_IMAPIClientShutdown, 0x00020397, 0, 0);
#endif

/* IMAPIProviderShutdown */
#if !defined(INITGUID) || defined(USES_IID_IMAPIProviderShutdown)
DEFINE_OLEGUID(IID_IMAPIProviderShutdown, 0x00020398, 0, 0);
#endif

#endif	/* MAPIGUID_H */

