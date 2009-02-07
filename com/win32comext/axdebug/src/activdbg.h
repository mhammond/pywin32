

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 6.00.0366 */
/* Compiler settings for activdbg.idl:
    Oicf, W1, Zp8, env=Win32 (32b run)
    protocol : dce , ms_ext, c_ext
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
//@@MIDL_FILE_HEADING(  )

#pragma warning( disable: 4049 )  /* more than 64k source lines */


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 440
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __activdbg_h__
#define __activdbg_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IActiveScriptDebug32_FWD_DEFINED__
#define __IActiveScriptDebug32_FWD_DEFINED__
typedef interface IActiveScriptDebug32 IActiveScriptDebug32;
#endif 	/* __IActiveScriptDebug32_FWD_DEFINED__ */


#ifndef __IActiveScriptDebug64_FWD_DEFINED__
#define __IActiveScriptDebug64_FWD_DEFINED__
typedef interface IActiveScriptDebug64 IActiveScriptDebug64;
#endif 	/* __IActiveScriptDebug64_FWD_DEFINED__ */


#ifndef __IActiveScriptSiteDebug32_FWD_DEFINED__
#define __IActiveScriptSiteDebug32_FWD_DEFINED__
typedef interface IActiveScriptSiteDebug32 IActiveScriptSiteDebug32;
#endif 	/* __IActiveScriptSiteDebug32_FWD_DEFINED__ */


#ifndef __IActiveScriptSiteDebug64_FWD_DEFINED__
#define __IActiveScriptSiteDebug64_FWD_DEFINED__
typedef interface IActiveScriptSiteDebug64 IActiveScriptSiteDebug64;
#endif 	/* __IActiveScriptSiteDebug64_FWD_DEFINED__ */


#ifndef __IActiveScriptErrorDebug_FWD_DEFINED__
#define __IActiveScriptErrorDebug_FWD_DEFINED__
typedef interface IActiveScriptErrorDebug IActiveScriptErrorDebug;
#endif 	/* __IActiveScriptErrorDebug_FWD_DEFINED__ */


#ifndef __IDebugCodeContext_FWD_DEFINED__
#define __IDebugCodeContext_FWD_DEFINED__
typedef interface IDebugCodeContext IDebugCodeContext;
#endif 	/* __IDebugCodeContext_FWD_DEFINED__ */


#ifndef __IDebugExpression_FWD_DEFINED__
#define __IDebugExpression_FWD_DEFINED__
typedef interface IDebugExpression IDebugExpression;
#endif 	/* __IDebugExpression_FWD_DEFINED__ */


#ifndef __IDebugExpressionContext_FWD_DEFINED__
#define __IDebugExpressionContext_FWD_DEFINED__
typedef interface IDebugExpressionContext IDebugExpressionContext;
#endif 	/* __IDebugExpressionContext_FWD_DEFINED__ */


#ifndef __IDebugExpressionCallBack_FWD_DEFINED__
#define __IDebugExpressionCallBack_FWD_DEFINED__
typedef interface IDebugExpressionCallBack IDebugExpressionCallBack;
#endif 	/* __IDebugExpressionCallBack_FWD_DEFINED__ */


#ifndef __IDebugStackFrame_FWD_DEFINED__
#define __IDebugStackFrame_FWD_DEFINED__
typedef interface IDebugStackFrame IDebugStackFrame;
#endif 	/* __IDebugStackFrame_FWD_DEFINED__ */


#ifndef __IDebugStackFrameSniffer_FWD_DEFINED__
#define __IDebugStackFrameSniffer_FWD_DEFINED__
typedef interface IDebugStackFrameSniffer IDebugStackFrameSniffer;
#endif 	/* __IDebugStackFrameSniffer_FWD_DEFINED__ */


#ifndef __IDebugStackFrameSnifferEx32_FWD_DEFINED__
#define __IDebugStackFrameSnifferEx32_FWD_DEFINED__
typedef interface IDebugStackFrameSnifferEx32 IDebugStackFrameSnifferEx32;
#endif 	/* __IDebugStackFrameSnifferEx32_FWD_DEFINED__ */


#ifndef __IDebugStackFrameSnifferEx64_FWD_DEFINED__
#define __IDebugStackFrameSnifferEx64_FWD_DEFINED__
typedef interface IDebugStackFrameSnifferEx64 IDebugStackFrameSnifferEx64;
#endif 	/* __IDebugStackFrameSnifferEx64_FWD_DEFINED__ */


#ifndef __IDebugSyncOperation_FWD_DEFINED__
#define __IDebugSyncOperation_FWD_DEFINED__
typedef interface IDebugSyncOperation IDebugSyncOperation;
#endif 	/* __IDebugSyncOperation_FWD_DEFINED__ */


#ifndef __IDebugAsyncOperation_FWD_DEFINED__
#define __IDebugAsyncOperation_FWD_DEFINED__
typedef interface IDebugAsyncOperation IDebugAsyncOperation;
#endif 	/* __IDebugAsyncOperation_FWD_DEFINED__ */


#ifndef __IDebugAsyncOperationCallBack_FWD_DEFINED__
#define __IDebugAsyncOperationCallBack_FWD_DEFINED__
typedef interface IDebugAsyncOperationCallBack IDebugAsyncOperationCallBack;
#endif 	/* __IDebugAsyncOperationCallBack_FWD_DEFINED__ */


#ifndef __IEnumDebugCodeContexts_FWD_DEFINED__
#define __IEnumDebugCodeContexts_FWD_DEFINED__
typedef interface IEnumDebugCodeContexts IEnumDebugCodeContexts;
#endif 	/* __IEnumDebugCodeContexts_FWD_DEFINED__ */


#ifndef __IEnumDebugStackFrames_FWD_DEFINED__
#define __IEnumDebugStackFrames_FWD_DEFINED__
typedef interface IEnumDebugStackFrames IEnumDebugStackFrames;
#endif 	/* __IEnumDebugStackFrames_FWD_DEFINED__ */


#ifndef __IEnumDebugStackFrames64_FWD_DEFINED__
#define __IEnumDebugStackFrames64_FWD_DEFINED__
typedef interface IEnumDebugStackFrames64 IEnumDebugStackFrames64;
#endif 	/* __IEnumDebugStackFrames64_FWD_DEFINED__ */


#ifndef __IDebugDocumentInfo_FWD_DEFINED__
#define __IDebugDocumentInfo_FWD_DEFINED__
typedef interface IDebugDocumentInfo IDebugDocumentInfo;
#endif 	/* __IDebugDocumentInfo_FWD_DEFINED__ */


#ifndef __IDebugDocumentProvider_FWD_DEFINED__
#define __IDebugDocumentProvider_FWD_DEFINED__
typedef interface IDebugDocumentProvider IDebugDocumentProvider;
#endif 	/* __IDebugDocumentProvider_FWD_DEFINED__ */


#ifndef __IDebugDocument_FWD_DEFINED__
#define __IDebugDocument_FWD_DEFINED__
typedef interface IDebugDocument IDebugDocument;
#endif 	/* __IDebugDocument_FWD_DEFINED__ */


#ifndef __IDebugDocumentText_FWD_DEFINED__
#define __IDebugDocumentText_FWD_DEFINED__
typedef interface IDebugDocumentText IDebugDocumentText;
#endif 	/* __IDebugDocumentText_FWD_DEFINED__ */


#ifndef __IDebugDocumentTextEvents_FWD_DEFINED__
#define __IDebugDocumentTextEvents_FWD_DEFINED__
typedef interface IDebugDocumentTextEvents IDebugDocumentTextEvents;
#endif 	/* __IDebugDocumentTextEvents_FWD_DEFINED__ */


#ifndef __IDebugDocumentTextAuthor_FWD_DEFINED__
#define __IDebugDocumentTextAuthor_FWD_DEFINED__
typedef interface IDebugDocumentTextAuthor IDebugDocumentTextAuthor;
#endif 	/* __IDebugDocumentTextAuthor_FWD_DEFINED__ */


#ifndef __IDebugDocumentTextExternalAuthor_FWD_DEFINED__
#define __IDebugDocumentTextExternalAuthor_FWD_DEFINED__
typedef interface IDebugDocumentTextExternalAuthor IDebugDocumentTextExternalAuthor;
#endif 	/* __IDebugDocumentTextExternalAuthor_FWD_DEFINED__ */


#ifndef __IDebugDocumentHelper32_FWD_DEFINED__
#define __IDebugDocumentHelper32_FWD_DEFINED__
typedef interface IDebugDocumentHelper32 IDebugDocumentHelper32;
#endif 	/* __IDebugDocumentHelper32_FWD_DEFINED__ */


#ifndef __IDebugDocumentHelper64_FWD_DEFINED__
#define __IDebugDocumentHelper64_FWD_DEFINED__
typedef interface IDebugDocumentHelper64 IDebugDocumentHelper64;
#endif 	/* __IDebugDocumentHelper64_FWD_DEFINED__ */


#ifndef __IDebugDocumentHost_FWD_DEFINED__
#define __IDebugDocumentHost_FWD_DEFINED__
typedef interface IDebugDocumentHost IDebugDocumentHost;
#endif 	/* __IDebugDocumentHost_FWD_DEFINED__ */


#ifndef __IDebugDocumentContext_FWD_DEFINED__
#define __IDebugDocumentContext_FWD_DEFINED__
typedef interface IDebugDocumentContext IDebugDocumentContext;
#endif 	/* __IDebugDocumentContext_FWD_DEFINED__ */


#ifndef __IDebugSessionProvider_FWD_DEFINED__
#define __IDebugSessionProvider_FWD_DEFINED__
typedef interface IDebugSessionProvider IDebugSessionProvider;
#endif 	/* __IDebugSessionProvider_FWD_DEFINED__ */


#ifndef __IApplicationDebugger_FWD_DEFINED__
#define __IApplicationDebugger_FWD_DEFINED__
typedef interface IApplicationDebugger IApplicationDebugger;
#endif 	/* __IApplicationDebugger_FWD_DEFINED__ */


#ifndef __IApplicationDebuggerUI_FWD_DEFINED__
#define __IApplicationDebuggerUI_FWD_DEFINED__
typedef interface IApplicationDebuggerUI IApplicationDebuggerUI;
#endif 	/* __IApplicationDebuggerUI_FWD_DEFINED__ */


#ifndef __IMachineDebugManager_FWD_DEFINED__
#define __IMachineDebugManager_FWD_DEFINED__
typedef interface IMachineDebugManager IMachineDebugManager;
#endif 	/* __IMachineDebugManager_FWD_DEFINED__ */


#ifndef __IMachineDebugManagerCookie_FWD_DEFINED__
#define __IMachineDebugManagerCookie_FWD_DEFINED__
typedef interface IMachineDebugManagerCookie IMachineDebugManagerCookie;
#endif 	/* __IMachineDebugManagerCookie_FWD_DEFINED__ */


#ifndef __IMachineDebugManagerEvents_FWD_DEFINED__
#define __IMachineDebugManagerEvents_FWD_DEFINED__
typedef interface IMachineDebugManagerEvents IMachineDebugManagerEvents;
#endif 	/* __IMachineDebugManagerEvents_FWD_DEFINED__ */


#ifndef __IProcessDebugManager32_FWD_DEFINED__
#define __IProcessDebugManager32_FWD_DEFINED__
typedef interface IProcessDebugManager32 IProcessDebugManager32;
#endif 	/* __IProcessDebugManager32_FWD_DEFINED__ */


#ifndef __IProcessDebugManager64_FWD_DEFINED__
#define __IProcessDebugManager64_FWD_DEFINED__
typedef interface IProcessDebugManager64 IProcessDebugManager64;
#endif 	/* __IProcessDebugManager64_FWD_DEFINED__ */


#ifndef __IRemoteDebugApplication_FWD_DEFINED__
#define __IRemoteDebugApplication_FWD_DEFINED__
typedef interface IRemoteDebugApplication IRemoteDebugApplication;
#endif 	/* __IRemoteDebugApplication_FWD_DEFINED__ */


#ifndef __IDebugApplication32_FWD_DEFINED__
#define __IDebugApplication32_FWD_DEFINED__
typedef interface IDebugApplication32 IDebugApplication32;
#endif 	/* __IDebugApplication32_FWD_DEFINED__ */


#ifndef __IDebugApplication64_FWD_DEFINED__
#define __IDebugApplication64_FWD_DEFINED__
typedef interface IDebugApplication64 IDebugApplication64;
#endif 	/* __IDebugApplication64_FWD_DEFINED__ */


#ifndef __IRemoteDebugApplicationEvents_FWD_DEFINED__
#define __IRemoteDebugApplicationEvents_FWD_DEFINED__
typedef interface IRemoteDebugApplicationEvents IRemoteDebugApplicationEvents;
#endif 	/* __IRemoteDebugApplicationEvents_FWD_DEFINED__ */


#ifndef __IDebugApplicationNode_FWD_DEFINED__
#define __IDebugApplicationNode_FWD_DEFINED__
typedef interface IDebugApplicationNode IDebugApplicationNode;
#endif 	/* __IDebugApplicationNode_FWD_DEFINED__ */


#ifndef __IDebugApplicationNodeEvents_FWD_DEFINED__
#define __IDebugApplicationNodeEvents_FWD_DEFINED__
typedef interface IDebugApplicationNodeEvents IDebugApplicationNodeEvents;
#endif 	/* __IDebugApplicationNodeEvents_FWD_DEFINED__ */


#ifndef __IDebugThreadCall32_FWD_DEFINED__
#define __IDebugThreadCall32_FWD_DEFINED__
typedef interface IDebugThreadCall32 IDebugThreadCall32;
#endif 	/* __IDebugThreadCall32_FWD_DEFINED__ */


#ifndef __IDebugThreadCall64_FWD_DEFINED__
#define __IDebugThreadCall64_FWD_DEFINED__
typedef interface IDebugThreadCall64 IDebugThreadCall64;
#endif 	/* __IDebugThreadCall64_FWD_DEFINED__ */


#ifndef __IRemoteDebugApplicationThread_FWD_DEFINED__
#define __IRemoteDebugApplicationThread_FWD_DEFINED__
typedef interface IRemoteDebugApplicationThread IRemoteDebugApplicationThread;
#endif 	/* __IRemoteDebugApplicationThread_FWD_DEFINED__ */


#ifndef __IDebugApplicationThread_FWD_DEFINED__
#define __IDebugApplicationThread_FWD_DEFINED__
typedef interface IDebugApplicationThread IDebugApplicationThread;
#endif 	/* __IDebugApplicationThread_FWD_DEFINED__ */


#ifndef __IDebugApplicationThread64_FWD_DEFINED__
#define __IDebugApplicationThread64_FWD_DEFINED__
typedef interface IDebugApplicationThread64 IDebugApplicationThread64;
#endif 	/* __IDebugApplicationThread64_FWD_DEFINED__ */


#ifndef __IDebugCookie_FWD_DEFINED__
#define __IDebugCookie_FWD_DEFINED__
typedef interface IDebugCookie IDebugCookie;
#endif 	/* __IDebugCookie_FWD_DEFINED__ */


#ifndef __IEnumDebugApplicationNodes_FWD_DEFINED__
#define __IEnumDebugApplicationNodes_FWD_DEFINED__
typedef interface IEnumDebugApplicationNodes IEnumDebugApplicationNodes;
#endif 	/* __IEnumDebugApplicationNodes_FWD_DEFINED__ */


#ifndef __IEnumRemoteDebugApplications_FWD_DEFINED__
#define __IEnumRemoteDebugApplications_FWD_DEFINED__
typedef interface IEnumRemoteDebugApplications IEnumRemoteDebugApplications;
#endif 	/* __IEnumRemoteDebugApplications_FWD_DEFINED__ */


#ifndef __IEnumRemoteDebugApplicationThreads_FWD_DEFINED__
#define __IEnumRemoteDebugApplicationThreads_FWD_DEFINED__
typedef interface IEnumRemoteDebugApplicationThreads IEnumRemoteDebugApplicationThreads;
#endif 	/* __IEnumRemoteDebugApplicationThreads_FWD_DEFINED__ */


#ifndef __IDebugFormatter_FWD_DEFINED__
#define __IDebugFormatter_FWD_DEFINED__
typedef interface IDebugFormatter IDebugFormatter;
#endif 	/* __IDebugFormatter_FWD_DEFINED__ */


#ifndef __ISimpleConnectionPoint_FWD_DEFINED__
#define __ISimpleConnectionPoint_FWD_DEFINED__
typedef interface ISimpleConnectionPoint ISimpleConnectionPoint;
#endif 	/* __ISimpleConnectionPoint_FWD_DEFINED__ */


#ifndef __IDebugHelper_FWD_DEFINED__
#define __IDebugHelper_FWD_DEFINED__
typedef interface IDebugHelper IDebugHelper;
#endif 	/* __IDebugHelper_FWD_DEFINED__ */


#ifndef __IEnumDebugExpressionContexts_FWD_DEFINED__
#define __IEnumDebugExpressionContexts_FWD_DEFINED__
typedef interface IEnumDebugExpressionContexts IEnumDebugExpressionContexts;
#endif 	/* __IEnumDebugExpressionContexts_FWD_DEFINED__ */


#ifndef __IProvideExpressionContexts_FWD_DEFINED__
#define __IProvideExpressionContexts_FWD_DEFINED__
typedef interface IProvideExpressionContexts IProvideExpressionContexts;
#endif 	/* __IProvideExpressionContexts_FWD_DEFINED__ */


#ifndef __IActiveScriptDebug32_FWD_DEFINED__
#define __IActiveScriptDebug32_FWD_DEFINED__
typedef interface IActiveScriptDebug32 IActiveScriptDebug32;
#endif 	/* __IActiveScriptDebug32_FWD_DEFINED__ */


#ifndef __IActiveScriptDebug64_FWD_DEFINED__
#define __IActiveScriptDebug64_FWD_DEFINED__
typedef interface IActiveScriptDebug64 IActiveScriptDebug64;
#endif 	/* __IActiveScriptDebug64_FWD_DEFINED__ */


#ifndef __IActiveScriptErrorDebug_FWD_DEFINED__
#define __IActiveScriptErrorDebug_FWD_DEFINED__
typedef interface IActiveScriptErrorDebug IActiveScriptErrorDebug;
#endif 	/* __IActiveScriptErrorDebug_FWD_DEFINED__ */


#ifndef __IActiveScriptSiteDebug32_FWD_DEFINED__
#define __IActiveScriptSiteDebug32_FWD_DEFINED__
typedef interface IActiveScriptSiteDebug32 IActiveScriptSiteDebug32;
#endif 	/* __IActiveScriptSiteDebug32_FWD_DEFINED__ */


#ifndef __IActiveScriptSiteDebug64_FWD_DEFINED__
#define __IActiveScriptSiteDebug64_FWD_DEFINED__
typedef interface IActiveScriptSiteDebug64 IActiveScriptSiteDebug64;
#endif 	/* __IActiveScriptSiteDebug64_FWD_DEFINED__ */


#ifndef __IApplicationDebugger_FWD_DEFINED__
#define __IApplicationDebugger_FWD_DEFINED__
typedef interface IApplicationDebugger IApplicationDebugger;
#endif 	/* __IApplicationDebugger_FWD_DEFINED__ */


#ifndef __IApplicationDebuggerUI_FWD_DEFINED__
#define __IApplicationDebuggerUI_FWD_DEFINED__
typedef interface IApplicationDebuggerUI IApplicationDebuggerUI;
#endif 	/* __IApplicationDebuggerUI_FWD_DEFINED__ */


#ifndef __IRemoteDebugApplication_FWD_DEFINED__
#define __IRemoteDebugApplication_FWD_DEFINED__
typedef interface IRemoteDebugApplication IRemoteDebugApplication;
#endif 	/* __IRemoteDebugApplication_FWD_DEFINED__ */


#ifndef __IDebugApplication32_FWD_DEFINED__
#define __IDebugApplication32_FWD_DEFINED__
typedef interface IDebugApplication32 IDebugApplication32;
#endif 	/* __IDebugApplication32_FWD_DEFINED__ */


#ifndef __IDebugApplication64_FWD_DEFINED__
#define __IDebugApplication64_FWD_DEFINED__
typedef interface IDebugApplication64 IDebugApplication64;
#endif 	/* __IDebugApplication64_FWD_DEFINED__ */


#ifndef __IDebugDocumentInfo_FWD_DEFINED__
#define __IDebugDocumentInfo_FWD_DEFINED__
typedef interface IDebugDocumentInfo IDebugDocumentInfo;
#endif 	/* __IDebugDocumentInfo_FWD_DEFINED__ */


#ifndef __IDebugDocumentProvider_FWD_DEFINED__
#define __IDebugDocumentProvider_FWD_DEFINED__
typedef interface IDebugDocumentProvider IDebugDocumentProvider;
#endif 	/* __IDebugDocumentProvider_FWD_DEFINED__ */


#ifndef __IDebugApplicationNode_FWD_DEFINED__
#define __IDebugApplicationNode_FWD_DEFINED__
typedef interface IDebugApplicationNode IDebugApplicationNode;
#endif 	/* __IDebugApplicationNode_FWD_DEFINED__ */


#ifndef __IDebugApplicationNodeEvents_FWD_DEFINED__
#define __IDebugApplicationNodeEvents_FWD_DEFINED__
typedef interface IDebugApplicationNodeEvents IDebugApplicationNodeEvents;
#endif 	/* __IDebugApplicationNodeEvents_FWD_DEFINED__ */


#ifndef __IRemoteDebugApplicationThread_FWD_DEFINED__
#define __IRemoteDebugApplicationThread_FWD_DEFINED__
typedef interface IRemoteDebugApplicationThread IRemoteDebugApplicationThread;
#endif 	/* __IRemoteDebugApplicationThread_FWD_DEFINED__ */


#ifndef __IDebugApplicationThread_FWD_DEFINED__
#define __IDebugApplicationThread_FWD_DEFINED__
typedef interface IDebugApplicationThread IDebugApplicationThread;
#endif 	/* __IDebugApplicationThread_FWD_DEFINED__ */


#ifndef __IDebugAsyncOperation_FWD_DEFINED__
#define __IDebugAsyncOperation_FWD_DEFINED__
typedef interface IDebugAsyncOperation IDebugAsyncOperation;
#endif 	/* __IDebugAsyncOperation_FWD_DEFINED__ */


#ifndef __IDebugAsyncOperationCallBack_FWD_DEFINED__
#define __IDebugAsyncOperationCallBack_FWD_DEFINED__
typedef interface IDebugAsyncOperationCallBack IDebugAsyncOperationCallBack;
#endif 	/* __IDebugAsyncOperationCallBack_FWD_DEFINED__ */


#ifndef __IDebugCodeContext_FWD_DEFINED__
#define __IDebugCodeContext_FWD_DEFINED__
typedef interface IDebugCodeContext IDebugCodeContext;
#endif 	/* __IDebugCodeContext_FWD_DEFINED__ */


#ifndef __IDebugCookie_FWD_DEFINED__
#define __IDebugCookie_FWD_DEFINED__
typedef interface IDebugCookie IDebugCookie;
#endif 	/* __IDebugCookie_FWD_DEFINED__ */


#ifndef __IDebugDocument_FWD_DEFINED__
#define __IDebugDocument_FWD_DEFINED__
typedef interface IDebugDocument IDebugDocument;
#endif 	/* __IDebugDocument_FWD_DEFINED__ */


#ifndef __IDebugDocumentContext_FWD_DEFINED__
#define __IDebugDocumentContext_FWD_DEFINED__
typedef interface IDebugDocumentContext IDebugDocumentContext;
#endif 	/* __IDebugDocumentContext_FWD_DEFINED__ */


#ifndef __IDebugDocumentHelper32_FWD_DEFINED__
#define __IDebugDocumentHelper32_FWD_DEFINED__
typedef interface IDebugDocumentHelper32 IDebugDocumentHelper32;
#endif 	/* __IDebugDocumentHelper32_FWD_DEFINED__ */


#ifndef __IDebugDocumentHelper64_FWD_DEFINED__
#define __IDebugDocumentHelper64_FWD_DEFINED__
typedef interface IDebugDocumentHelper64 IDebugDocumentHelper64;
#endif 	/* __IDebugDocumentHelper64_FWD_DEFINED__ */


#ifndef __IDebugDocumentHost_FWD_DEFINED__
#define __IDebugDocumentHost_FWD_DEFINED__
typedef interface IDebugDocumentHost IDebugDocumentHost;
#endif 	/* __IDebugDocumentHost_FWD_DEFINED__ */


#ifndef __IDebugDocumentText_FWD_DEFINED__
#define __IDebugDocumentText_FWD_DEFINED__
typedef interface IDebugDocumentText IDebugDocumentText;
#endif 	/* __IDebugDocumentText_FWD_DEFINED__ */


#ifndef __IDebugDocumentTextAuthor_FWD_DEFINED__
#define __IDebugDocumentTextAuthor_FWD_DEFINED__
typedef interface IDebugDocumentTextAuthor IDebugDocumentTextAuthor;
#endif 	/* __IDebugDocumentTextAuthor_FWD_DEFINED__ */


#ifndef __IDebugDocumentTextEvents_FWD_DEFINED__
#define __IDebugDocumentTextEvents_FWD_DEFINED__
typedef interface IDebugDocumentTextEvents IDebugDocumentTextEvents;
#endif 	/* __IDebugDocumentTextEvents_FWD_DEFINED__ */


#ifndef __IDebugDocumentTextExternalAuthor_FWD_DEFINED__
#define __IDebugDocumentTextExternalAuthor_FWD_DEFINED__
typedef interface IDebugDocumentTextExternalAuthor IDebugDocumentTextExternalAuthor;
#endif 	/* __IDebugDocumentTextExternalAuthor_FWD_DEFINED__ */


#ifndef __IDebugExpression_FWD_DEFINED__
#define __IDebugExpression_FWD_DEFINED__
typedef interface IDebugExpression IDebugExpression;
#endif 	/* __IDebugExpression_FWD_DEFINED__ */


#ifndef __IDebugExpressionCallBack_FWD_DEFINED__
#define __IDebugExpressionCallBack_FWD_DEFINED__
typedef interface IDebugExpressionCallBack IDebugExpressionCallBack;
#endif 	/* __IDebugExpressionCallBack_FWD_DEFINED__ */


#ifndef __IDebugExpressionContext_FWD_DEFINED__
#define __IDebugExpressionContext_FWD_DEFINED__
typedef interface IDebugExpressionContext IDebugExpressionContext;
#endif 	/* __IDebugExpressionContext_FWD_DEFINED__ */


#ifndef __IDebugFormatter_FWD_DEFINED__
#define __IDebugFormatter_FWD_DEFINED__
typedef interface IDebugFormatter IDebugFormatter;
#endif 	/* __IDebugFormatter_FWD_DEFINED__ */


#ifndef __IDebugHelper_FWD_DEFINED__
#define __IDebugHelper_FWD_DEFINED__
typedef interface IDebugHelper IDebugHelper;
#endif 	/* __IDebugHelper_FWD_DEFINED__ */


#ifndef __IDebugSessionProvider_FWD_DEFINED__
#define __IDebugSessionProvider_FWD_DEFINED__
typedef interface IDebugSessionProvider IDebugSessionProvider;
#endif 	/* __IDebugSessionProvider_FWD_DEFINED__ */


#ifndef __IDebugStackFrame_FWD_DEFINED__
#define __IDebugStackFrame_FWD_DEFINED__
typedef interface IDebugStackFrame IDebugStackFrame;
#endif 	/* __IDebugStackFrame_FWD_DEFINED__ */


#ifndef __IDebugStackFrameSniffer_FWD_DEFINED__
#define __IDebugStackFrameSniffer_FWD_DEFINED__
typedef interface IDebugStackFrameSniffer IDebugStackFrameSniffer;
#endif 	/* __IDebugStackFrameSniffer_FWD_DEFINED__ */


#ifndef __IDebugStackFrameSnifferEx32_FWD_DEFINED__
#define __IDebugStackFrameSnifferEx32_FWD_DEFINED__
typedef interface IDebugStackFrameSnifferEx32 IDebugStackFrameSnifferEx32;
#endif 	/* __IDebugStackFrameSnifferEx32_FWD_DEFINED__ */


#ifndef __IDebugStackFrameSnifferEx64_FWD_DEFINED__
#define __IDebugStackFrameSnifferEx64_FWD_DEFINED__
typedef interface IDebugStackFrameSnifferEx64 IDebugStackFrameSnifferEx64;
#endif 	/* __IDebugStackFrameSnifferEx64_FWD_DEFINED__ */


#ifndef __IDebugSyncOperation_FWD_DEFINED__
#define __IDebugSyncOperation_FWD_DEFINED__
typedef interface IDebugSyncOperation IDebugSyncOperation;
#endif 	/* __IDebugSyncOperation_FWD_DEFINED__ */


#ifndef __IDebugThreadCall32_FWD_DEFINED__
#define __IDebugThreadCall32_FWD_DEFINED__
typedef interface IDebugThreadCall32 IDebugThreadCall32;
#endif 	/* __IDebugThreadCall32_FWD_DEFINED__ */


#ifndef __IDebugThreadCall64_FWD_DEFINED__
#define __IDebugThreadCall64_FWD_DEFINED__
typedef interface IDebugThreadCall64 IDebugThreadCall64;
#endif 	/* __IDebugThreadCall64_FWD_DEFINED__ */


#ifndef __IEnumDebugApplicationNodes_FWD_DEFINED__
#define __IEnumDebugApplicationNodes_FWD_DEFINED__
typedef interface IEnumDebugApplicationNodes IEnumDebugApplicationNodes;
#endif 	/* __IEnumDebugApplicationNodes_FWD_DEFINED__ */


#ifndef __IEnumDebugCodeContexts_FWD_DEFINED__
#define __IEnumDebugCodeContexts_FWD_DEFINED__
typedef interface IEnumDebugCodeContexts IEnumDebugCodeContexts;
#endif 	/* __IEnumDebugCodeContexts_FWD_DEFINED__ */


#ifndef __IEnumDebugExpressionContexts_FWD_DEFINED__
#define __IEnumDebugExpressionContexts_FWD_DEFINED__
typedef interface IEnumDebugExpressionContexts IEnumDebugExpressionContexts;
#endif 	/* __IEnumDebugExpressionContexts_FWD_DEFINED__ */


#ifndef __IEnumDebugStackFrames_FWD_DEFINED__
#define __IEnumDebugStackFrames_FWD_DEFINED__
typedef interface IEnumDebugStackFrames IEnumDebugStackFrames;
#endif 	/* __IEnumDebugStackFrames_FWD_DEFINED__ */


#ifndef __IEnumDebugStackFrames64_FWD_DEFINED__
#define __IEnumDebugStackFrames64_FWD_DEFINED__
typedef interface IEnumDebugStackFrames64 IEnumDebugStackFrames64;
#endif 	/* __IEnumDebugStackFrames64_FWD_DEFINED__ */


#ifndef __IEnumRemoteDebugApplications_FWD_DEFINED__
#define __IEnumRemoteDebugApplications_FWD_DEFINED__
typedef interface IEnumRemoteDebugApplications IEnumRemoteDebugApplications;
#endif 	/* __IEnumRemoteDebugApplications_FWD_DEFINED__ */


#ifndef __IEnumRemoteDebugApplicationThreads_FWD_DEFINED__
#define __IEnumRemoteDebugApplicationThreads_FWD_DEFINED__
typedef interface IEnumRemoteDebugApplicationThreads IEnumRemoteDebugApplicationThreads;
#endif 	/* __IEnumRemoteDebugApplicationThreads_FWD_DEFINED__ */


#ifndef __IProcessDebugManager32_FWD_DEFINED__
#define __IProcessDebugManager32_FWD_DEFINED__
typedef interface IProcessDebugManager32 IProcessDebugManager32;
#endif 	/* __IProcessDebugManager32_FWD_DEFINED__ */


#ifndef __IProcessDebugManager64_FWD_DEFINED__
#define __IProcessDebugManager64_FWD_DEFINED__
typedef interface IProcessDebugManager64 IProcessDebugManager64;
#endif 	/* __IProcessDebugManager64_FWD_DEFINED__ */


#ifndef __IProvideExpressionContexts_FWD_DEFINED__
#define __IProvideExpressionContexts_FWD_DEFINED__
typedef interface IProvideExpressionContexts IProvideExpressionContexts;
#endif 	/* __IProvideExpressionContexts_FWD_DEFINED__ */


#ifndef __IMachineDebugManager_FWD_DEFINED__
#define __IMachineDebugManager_FWD_DEFINED__
typedef interface IMachineDebugManager IMachineDebugManager;
#endif 	/* __IMachineDebugManager_FWD_DEFINED__ */


#ifndef __IMachineDebugManagerCookie_FWD_DEFINED__
#define __IMachineDebugManagerCookie_FWD_DEFINED__
typedef interface IMachineDebugManagerCookie IMachineDebugManagerCookie;
#endif 	/* __IMachineDebugManagerCookie_FWD_DEFINED__ */


#ifndef __IMachineDebugManagerEvents_FWD_DEFINED__
#define __IMachineDebugManagerEvents_FWD_DEFINED__
typedef interface IMachineDebugManagerEvents IMachineDebugManagerEvents;
#endif 	/* __IMachineDebugManagerEvents_FWD_DEFINED__ */


#ifndef __IRemoteDebugApplicationEvents_FWD_DEFINED__
#define __IRemoteDebugApplicationEvents_FWD_DEFINED__
typedef interface IRemoteDebugApplicationEvents IRemoteDebugApplicationEvents;
#endif 	/* __IRemoteDebugApplicationEvents_FWD_DEFINED__ */


#ifndef __ISimpleConnectionPoint_FWD_DEFINED__
#define __ISimpleConnectionPoint_FWD_DEFINED__
typedef interface ISimpleConnectionPoint ISimpleConnectionPoint;
#endif 	/* __ISimpleConnectionPoint_FWD_DEFINED__ */


#ifndef __ProcessDebugManager_FWD_DEFINED__
#define __ProcessDebugManager_FWD_DEFINED__

#ifdef __cplusplus
typedef class ProcessDebugManager ProcessDebugManager;
#else
typedef struct ProcessDebugManager ProcessDebugManager;
#endif /* __cplusplus */

#endif 	/* __ProcessDebugManager_FWD_DEFINED__ */


#ifndef __DebugHelper_FWD_DEFINED__
#define __DebugHelper_FWD_DEFINED__

#ifdef __cplusplus
typedef class DebugHelper DebugHelper;
#else
typedef struct DebugHelper DebugHelper;
#endif /* __cplusplus */

#endif 	/* __DebugHelper_FWD_DEFINED__ */


#ifndef __CDebugDocumentHelper_FWD_DEFINED__
#define __CDebugDocumentHelper_FWD_DEFINED__

#ifdef __cplusplus
typedef class CDebugDocumentHelper CDebugDocumentHelper;
#else
typedef struct CDebugDocumentHelper CDebugDocumentHelper;
#endif /* __cplusplus */

#endif 	/* __CDebugDocumentHelper_FWD_DEFINED__ */


#ifndef __MachineDebugManager_FWD_DEFINED__
#define __MachineDebugManager_FWD_DEFINED__

#ifdef __cplusplus
typedef class MachineDebugManager MachineDebugManager;
#else
typedef struct MachineDebugManager MachineDebugManager;
#endif /* __cplusplus */

#endif 	/* __MachineDebugManager_FWD_DEFINED__ */


#ifndef __DefaultDebugSessionProvider_FWD_DEFINED__
#define __DefaultDebugSessionProvider_FWD_DEFINED__

#ifdef __cplusplus
typedef class DefaultDebugSessionProvider DefaultDebugSessionProvider;
#else
typedef struct DefaultDebugSessionProvider DefaultDebugSessionProvider;
#endif /* __cplusplus */

#endif 	/* __DefaultDebugSessionProvider_FWD_DEFINED__ */


/* header files for imported files */
#include "ocidl.h"
#include "activscp.h"
#include "dbgprop.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

/* interface __MIDL_itf_activdbg_0000 */
/* [local] */ 

//=--------------------------------------------------------------------------=
// ActivDbg.h
//=--------------------------------------------------------------------------=
// (C) Copyright 2000 Microsoft Corporation.  All Rights Reserved.
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//=--------------------------------------------------------------------------=
//
#pragma comment(lib,"uuid.lib")
//
// Declarations for ActiveX Scripting authoring/Debugging.
//

#ifndef __ActivDbg_h
#define __ActivDbg_h























































typedef 
enum tagBREAKPOINT_STATE
    {	BREAKPOINT_DELETED	= 0,
	BREAKPOINT_DISABLED	= 1,
	BREAKPOINT_ENABLED	= 2
    } 	BREAKPOINT_STATE;

typedef DWORD APPBREAKFLAGS;

#define	APPBREAKFLAG_DEBUGGER_BLOCK	( 0x1 )

#define	APPBREAKFLAG_DEBUGGER_HALT	( 0x2 )

#define	APPBREAKFLAG_STEP	( 0x10000 )

#define	APPBREAKFLAG_NESTED	( 0x20000 )

#define	APPBREAKFLAG_STEPTYPE_SOURCE	( 0 )

#define	APPBREAKFLAG_STEPTYPE_BYTECODE	( 0x100000 )

#define	APPBREAKFLAG_STEPTYPE_MACHINE	( 0x200000 )

#define	APPBREAKFLAG_STEPTYPE_MASK	( 0xf00000 )

#define	APPBREAKFLAG_IN_BREAKPOINT	( 0x80000000 )

typedef 
enum tagBREAKREASON
    {	BREAKREASON_STEP	= 0,
	BREAKREASON_BREAKPOINT	= BREAKREASON_STEP + 1,
	BREAKREASON_DEBUGGER_BLOCK	= BREAKREASON_BREAKPOINT + 1,
	BREAKREASON_HOST_INITIATED	= BREAKREASON_DEBUGGER_BLOCK + 1,
	BREAKREASON_LANGUAGE_INITIATED	= BREAKREASON_HOST_INITIATED + 1,
	BREAKREASON_DEBUGGER_HALT	= BREAKREASON_LANGUAGE_INITIATED + 1,
	BREAKREASON_ERROR	= BREAKREASON_DEBUGGER_HALT + 1,
	BREAKREASON_JIT	= BREAKREASON_ERROR + 1
    } 	BREAKREASON;

typedef 
enum tagBREAKRESUME_ACTION
    {	BREAKRESUMEACTION_ABORT	= 0,
	BREAKRESUMEACTION_CONTINUE	= BREAKRESUMEACTION_ABORT + 1,
	BREAKRESUMEACTION_STEP_INTO	= BREAKRESUMEACTION_CONTINUE + 1,
	BREAKRESUMEACTION_STEP_OVER	= BREAKRESUMEACTION_STEP_INTO + 1,
	BREAKRESUMEACTION_STEP_OUT	= BREAKRESUMEACTION_STEP_OVER + 1,
	BREAKRESUMEACTION_IGNORE	= BREAKRESUMEACTION_STEP_OUT + 1
    } 	BREAKRESUMEACTION;

typedef 
enum tagERRORRESUMEACTION
    {	ERRORRESUMEACTION_ReexecuteErrorStatement	= 0,
	ERRORRESUMEACTION_AbortCallAndReturnErrorToCaller	= ERRORRESUMEACTION_ReexecuteErrorStatement + 1,
	ERRORRESUMEACTION_SkipErrorStatement	= ERRORRESUMEACTION_AbortCallAndReturnErrorToCaller + 1
    } 	ERRORRESUMEACTION;

typedef 
enum tagDOCUMENTNAMETYPE
    {	DOCUMENTNAMETYPE_APPNODE	= 0,
	DOCUMENTNAMETYPE_TITLE	= DOCUMENTNAMETYPE_APPNODE + 1,
	DOCUMENTNAMETYPE_FILE_TAIL	= DOCUMENTNAMETYPE_TITLE + 1,
	DOCUMENTNAMETYPE_URL	= DOCUMENTNAMETYPE_FILE_TAIL + 1
    } 	DOCUMENTNAMETYPE;

typedef WORD SOURCE_TEXT_ATTR;

#define	SOURCETEXT_ATTR_KEYWORD	( 0x1 )

#define	SOURCETEXT_ATTR_COMMENT	( 0x2 )

#define	SOURCETEXT_ATTR_NONSOURCE	( 0x4 )

#define	SOURCETEXT_ATTR_OPERATOR	( 0x8 )

#define	SOURCETEXT_ATTR_NUMBER	( 0x10 )

#define	SOURCETEXT_ATTR_STRING	( 0x20 )

#define	SOURCETEXT_ATTR_FUNCTION_START	( 0x40 )

typedef DWORD TEXT_DOC_ATTR;

#define	TEXT_DOC_ATTR_READONLY	( 0x1 )

#define	DEBUG_TEXT_ISEXPRESSION	( 0x1 )

#define	DEBUG_TEXT_RETURNVALUE	( 0x2 )

#define	DEBUG_TEXT_NOSIDEEFFECTS	( 0x4 )

#define	DEBUG_TEXT_ALLOWBREAKPOINTS	( 0x8 )

#define	DEBUG_TEXT_ALLOWERRORREPORT	( 0x10 )

#define	DEBUG_TEXT_EVALUATETOCODECONTEXT	( 0x20 )

#ifndef DISABLE_ACTIVDBG_INTERFACE_WRAPPERS
#ifdef _WIN64
#define IActiveScriptDebug IActiveScriptDebug64
#define IID_IActiveScriptDebug IID_IActiveScriptDebug64
#else
#define IActiveScriptDebug IActiveScriptDebug32
#define IID_IActiveScriptDebug IID_IActiveScriptDebug32
#endif
#endif


extern RPC_IF_HANDLE __MIDL_itf_activdbg_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_activdbg_0000_v0_0_s_ifspec;

#ifndef __IActiveScriptDebug32_INTERFACE_DEFINED__
#define __IActiveScriptDebug32_INTERFACE_DEFINED__

/* interface IActiveScriptDebug32 */
/* [unique][uuid][object] */ 


EXTERN_C const IID IID_IActiveScriptDebug32;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("51973C10-CB0C-11d0-B5C9-00A0244A0E7A")
    IActiveScriptDebug32 : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetScriptTextAttributes( 
            /* [size_is][in] */ LPCOLESTR pstrCode,
            /* [in] */ ULONG uNumCodeChars,
            /* [in] */ LPCOLESTR pstrDelimiter,
            /* [in] */ DWORD dwFlags,
            /* [size_is][out][in] */ SOURCE_TEXT_ATTR *pattr) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetScriptletTextAttributes( 
            /* [size_is][in] */ LPCOLESTR pstrCode,
            /* [in] */ ULONG uNumCodeChars,
            /* [in] */ LPCOLESTR pstrDelimiter,
            /* [in] */ DWORD dwFlags,
            /* [size_is][out][in] */ SOURCE_TEXT_ATTR *pattr) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumCodeContextsOfPosition( 
            /* [in] */ DWORD dwSourceContext,
            /* [in] */ ULONG uCharacterOffset,
            /* [in] */ ULONG uNumChars,
            /* [out] */ IEnumDebugCodeContexts **ppescc) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IActiveScriptDebug32Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IActiveScriptDebug32 * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IActiveScriptDebug32 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IActiveScriptDebug32 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetScriptTextAttributes )( 
            IActiveScriptDebug32 * This,
            /* [size_is][in] */ LPCOLESTR pstrCode,
            /* [in] */ ULONG uNumCodeChars,
            /* [in] */ LPCOLESTR pstrDelimiter,
            /* [in] */ DWORD dwFlags,
            /* [size_is][out][in] */ SOURCE_TEXT_ATTR *pattr);
        
        HRESULT ( STDMETHODCALLTYPE *GetScriptletTextAttributes )( 
            IActiveScriptDebug32 * This,
            /* [size_is][in] */ LPCOLESTR pstrCode,
            /* [in] */ ULONG uNumCodeChars,
            /* [in] */ LPCOLESTR pstrDelimiter,
            /* [in] */ DWORD dwFlags,
            /* [size_is][out][in] */ SOURCE_TEXT_ATTR *pattr);
        
        HRESULT ( STDMETHODCALLTYPE *EnumCodeContextsOfPosition )( 
            IActiveScriptDebug32 * This,
            /* [in] */ DWORD dwSourceContext,
            /* [in] */ ULONG uCharacterOffset,
            /* [in] */ ULONG uNumChars,
            /* [out] */ IEnumDebugCodeContexts **ppescc);
        
        END_INTERFACE
    } IActiveScriptDebug32Vtbl;

    interface IActiveScriptDebug32
    {
        CONST_VTBL struct IActiveScriptDebug32Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IActiveScriptDebug32_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IActiveScriptDebug32_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IActiveScriptDebug32_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IActiveScriptDebug32_GetScriptTextAttributes(This,pstrCode,uNumCodeChars,pstrDelimiter,dwFlags,pattr)	\
    (This)->lpVtbl -> GetScriptTextAttributes(This,pstrCode,uNumCodeChars,pstrDelimiter,dwFlags,pattr)

#define IActiveScriptDebug32_GetScriptletTextAttributes(This,pstrCode,uNumCodeChars,pstrDelimiter,dwFlags,pattr)	\
    (This)->lpVtbl -> GetScriptletTextAttributes(This,pstrCode,uNumCodeChars,pstrDelimiter,dwFlags,pattr)

#define IActiveScriptDebug32_EnumCodeContextsOfPosition(This,dwSourceContext,uCharacterOffset,uNumChars,ppescc)	\
    (This)->lpVtbl -> EnumCodeContextsOfPosition(This,dwSourceContext,uCharacterOffset,uNumChars,ppescc)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IActiveScriptDebug32_GetScriptTextAttributes_Proxy( 
    IActiveScriptDebug32 * This,
    /* [size_is][in] */ LPCOLESTR pstrCode,
    /* [in] */ ULONG uNumCodeChars,
    /* [in] */ LPCOLESTR pstrDelimiter,
    /* [in] */ DWORD dwFlags,
    /* [size_is][out][in] */ SOURCE_TEXT_ATTR *pattr);


void __RPC_STUB IActiveScriptDebug32_GetScriptTextAttributes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveScriptDebug32_GetScriptletTextAttributes_Proxy( 
    IActiveScriptDebug32 * This,
    /* [size_is][in] */ LPCOLESTR pstrCode,
    /* [in] */ ULONG uNumCodeChars,
    /* [in] */ LPCOLESTR pstrDelimiter,
    /* [in] */ DWORD dwFlags,
    /* [size_is][out][in] */ SOURCE_TEXT_ATTR *pattr);


void __RPC_STUB IActiveScriptDebug32_GetScriptletTextAttributes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveScriptDebug32_EnumCodeContextsOfPosition_Proxy( 
    IActiveScriptDebug32 * This,
    /* [in] */ DWORD dwSourceContext,
    /* [in] */ ULONG uCharacterOffset,
    /* [in] */ ULONG uNumChars,
    /* [out] */ IEnumDebugCodeContexts **ppescc);


void __RPC_STUB IActiveScriptDebug32_EnumCodeContextsOfPosition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IActiveScriptDebug32_INTERFACE_DEFINED__ */


#ifndef __IActiveScriptDebug64_INTERFACE_DEFINED__
#define __IActiveScriptDebug64_INTERFACE_DEFINED__

/* interface IActiveScriptDebug64 */
/* [unique][uuid][object] */ 


EXTERN_C const IID IID_IActiveScriptDebug64;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("bc437e23-f5b8-47f4-bb79-7d1ce5483b86")
    IActiveScriptDebug64 : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetScriptTextAttributes( 
            /* [size_is][in] */ LPCOLESTR pstrCode,
            /* [in] */ ULONG uNumCodeChars,
            /* [in] */ LPCOLESTR pstrDelimiter,
            /* [in] */ DWORD dwFlags,
            /* [size_is][out][in] */ SOURCE_TEXT_ATTR *pattr) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetScriptletTextAttributes( 
            /* [size_is][in] */ LPCOLESTR pstrCode,
            /* [in] */ ULONG uNumCodeChars,
            /* [in] */ LPCOLESTR pstrDelimiter,
            /* [in] */ DWORD dwFlags,
            /* [size_is][out][in] */ SOURCE_TEXT_ATTR *pattr) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumCodeContextsOfPosition( 
            /* [in] */ DWORDLONG dwSourceContext,
            /* [in] */ ULONG uCharacterOffset,
            /* [in] */ ULONG uNumChars,
            /* [out] */ IEnumDebugCodeContexts **ppescc) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IActiveScriptDebug64Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IActiveScriptDebug64 * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IActiveScriptDebug64 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IActiveScriptDebug64 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetScriptTextAttributes )( 
            IActiveScriptDebug64 * This,
            /* [size_is][in] */ LPCOLESTR pstrCode,
            /* [in] */ ULONG uNumCodeChars,
            /* [in] */ LPCOLESTR pstrDelimiter,
            /* [in] */ DWORD dwFlags,
            /* [size_is][out][in] */ SOURCE_TEXT_ATTR *pattr);
        
        HRESULT ( STDMETHODCALLTYPE *GetScriptletTextAttributes )( 
            IActiveScriptDebug64 * This,
            /* [size_is][in] */ LPCOLESTR pstrCode,
            /* [in] */ ULONG uNumCodeChars,
            /* [in] */ LPCOLESTR pstrDelimiter,
            /* [in] */ DWORD dwFlags,
            /* [size_is][out][in] */ SOURCE_TEXT_ATTR *pattr);
        
        HRESULT ( STDMETHODCALLTYPE *EnumCodeContextsOfPosition )( 
            IActiveScriptDebug64 * This,
            /* [in] */ DWORDLONG dwSourceContext,
            /* [in] */ ULONG uCharacterOffset,
            /* [in] */ ULONG uNumChars,
            /* [out] */ IEnumDebugCodeContexts **ppescc);
        
        END_INTERFACE
    } IActiveScriptDebug64Vtbl;

    interface IActiveScriptDebug64
    {
        CONST_VTBL struct IActiveScriptDebug64Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IActiveScriptDebug64_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IActiveScriptDebug64_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IActiveScriptDebug64_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IActiveScriptDebug64_GetScriptTextAttributes(This,pstrCode,uNumCodeChars,pstrDelimiter,dwFlags,pattr)	\
    (This)->lpVtbl -> GetScriptTextAttributes(This,pstrCode,uNumCodeChars,pstrDelimiter,dwFlags,pattr)

#define IActiveScriptDebug64_GetScriptletTextAttributes(This,pstrCode,uNumCodeChars,pstrDelimiter,dwFlags,pattr)	\
    (This)->lpVtbl -> GetScriptletTextAttributes(This,pstrCode,uNumCodeChars,pstrDelimiter,dwFlags,pattr)

#define IActiveScriptDebug64_EnumCodeContextsOfPosition(This,dwSourceContext,uCharacterOffset,uNumChars,ppescc)	\
    (This)->lpVtbl -> EnumCodeContextsOfPosition(This,dwSourceContext,uCharacterOffset,uNumChars,ppescc)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IActiveScriptDebug64_GetScriptTextAttributes_Proxy( 
    IActiveScriptDebug64 * This,
    /* [size_is][in] */ LPCOLESTR pstrCode,
    /* [in] */ ULONG uNumCodeChars,
    /* [in] */ LPCOLESTR pstrDelimiter,
    /* [in] */ DWORD dwFlags,
    /* [size_is][out][in] */ SOURCE_TEXT_ATTR *pattr);


void __RPC_STUB IActiveScriptDebug64_GetScriptTextAttributes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveScriptDebug64_GetScriptletTextAttributes_Proxy( 
    IActiveScriptDebug64 * This,
    /* [size_is][in] */ LPCOLESTR pstrCode,
    /* [in] */ ULONG uNumCodeChars,
    /* [in] */ LPCOLESTR pstrDelimiter,
    /* [in] */ DWORD dwFlags,
    /* [size_is][out][in] */ SOURCE_TEXT_ATTR *pattr);


void __RPC_STUB IActiveScriptDebug64_GetScriptletTextAttributes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveScriptDebug64_EnumCodeContextsOfPosition_Proxy( 
    IActiveScriptDebug64 * This,
    /* [in] */ DWORDLONG dwSourceContext,
    /* [in] */ ULONG uCharacterOffset,
    /* [in] */ ULONG uNumChars,
    /* [out] */ IEnumDebugCodeContexts **ppescc);


void __RPC_STUB IActiveScriptDebug64_EnumCodeContextsOfPosition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IActiveScriptDebug64_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_activdbg_0305 */
/* [local] */ 

#ifndef DISABLE_ACTIVDBG_INTERFACE_WRAPPERS
#ifdef _WIN64
#define IActiveScriptSiteDebug IActiveScriptSiteDebug64
#define IID_IActiveScriptSiteDebug IID_IActiveScriptSiteDebug64
#else
#define IActiveScriptSiteDebug IActiveScriptSiteDebug32
#define IID_IActiveScriptSiteDebug IID_IActiveScriptSiteDebug32
#endif
#endif


extern RPC_IF_HANDLE __MIDL_itf_activdbg_0305_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_activdbg_0305_v0_0_s_ifspec;

#ifndef __IActiveScriptSiteDebug32_INTERFACE_DEFINED__
#define __IActiveScriptSiteDebug32_INTERFACE_DEFINED__

/* interface IActiveScriptSiteDebug32 */
/* [local][unique][uuid][object] */ 


EXTERN_C const IID IID_IActiveScriptSiteDebug32;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("51973C11-CB0C-11d0-B5C9-00A0244A0E7A")
    IActiveScriptSiteDebug32 : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetDocumentContextFromPosition( 
            /* [in] */ DWORD dwSourceContext,
            /* [in] */ ULONG uCharacterOffset,
            /* [in] */ ULONG uNumChars,
            /* [out] */ IDebugDocumentContext **ppsc) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetApplication( 
            /* [out] */ IDebugApplication32 **ppda) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetRootApplicationNode( 
            /* [out] */ IDebugApplicationNode **ppdanRoot) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnScriptErrorDebug( 
            /* [in] */ IActiveScriptErrorDebug *pErrorDebug,
            /* [out] */ BOOL *pfEnterDebugger,
            /* [out] */ BOOL *pfCallOnScriptErrorWhenContinuing) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IActiveScriptSiteDebug32Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IActiveScriptSiteDebug32 * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IActiveScriptSiteDebug32 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IActiveScriptSiteDebug32 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetDocumentContextFromPosition )( 
            IActiveScriptSiteDebug32 * This,
            /* [in] */ DWORD dwSourceContext,
            /* [in] */ ULONG uCharacterOffset,
            /* [in] */ ULONG uNumChars,
            /* [out] */ IDebugDocumentContext **ppsc);
        
        HRESULT ( STDMETHODCALLTYPE *GetApplication )( 
            IActiveScriptSiteDebug32 * This,
            /* [out] */ IDebugApplication32 **ppda);
        
        HRESULT ( STDMETHODCALLTYPE *GetRootApplicationNode )( 
            IActiveScriptSiteDebug32 * This,
            /* [out] */ IDebugApplicationNode **ppdanRoot);
        
        HRESULT ( STDMETHODCALLTYPE *OnScriptErrorDebug )( 
            IActiveScriptSiteDebug32 * This,
            /* [in] */ IActiveScriptErrorDebug *pErrorDebug,
            /* [out] */ BOOL *pfEnterDebugger,
            /* [out] */ BOOL *pfCallOnScriptErrorWhenContinuing);
        
        END_INTERFACE
    } IActiveScriptSiteDebug32Vtbl;

    interface IActiveScriptSiteDebug32
    {
        CONST_VTBL struct IActiveScriptSiteDebug32Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IActiveScriptSiteDebug32_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IActiveScriptSiteDebug32_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IActiveScriptSiteDebug32_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IActiveScriptSiteDebug32_GetDocumentContextFromPosition(This,dwSourceContext,uCharacterOffset,uNumChars,ppsc)	\
    (This)->lpVtbl -> GetDocumentContextFromPosition(This,dwSourceContext,uCharacterOffset,uNumChars,ppsc)

#define IActiveScriptSiteDebug32_GetApplication(This,ppda)	\
    (This)->lpVtbl -> GetApplication(This,ppda)

#define IActiveScriptSiteDebug32_GetRootApplicationNode(This,ppdanRoot)	\
    (This)->lpVtbl -> GetRootApplicationNode(This,ppdanRoot)

#define IActiveScriptSiteDebug32_OnScriptErrorDebug(This,pErrorDebug,pfEnterDebugger,pfCallOnScriptErrorWhenContinuing)	\
    (This)->lpVtbl -> OnScriptErrorDebug(This,pErrorDebug,pfEnterDebugger,pfCallOnScriptErrorWhenContinuing)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IActiveScriptSiteDebug32_GetDocumentContextFromPosition_Proxy( 
    IActiveScriptSiteDebug32 * This,
    /* [in] */ DWORD dwSourceContext,
    /* [in] */ ULONG uCharacterOffset,
    /* [in] */ ULONG uNumChars,
    /* [out] */ IDebugDocumentContext **ppsc);


void __RPC_STUB IActiveScriptSiteDebug32_GetDocumentContextFromPosition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveScriptSiteDebug32_GetApplication_Proxy( 
    IActiveScriptSiteDebug32 * This,
    /* [out] */ IDebugApplication32 **ppda);


void __RPC_STUB IActiveScriptSiteDebug32_GetApplication_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveScriptSiteDebug32_GetRootApplicationNode_Proxy( 
    IActiveScriptSiteDebug32 * This,
    /* [out] */ IDebugApplicationNode **ppdanRoot);


void __RPC_STUB IActiveScriptSiteDebug32_GetRootApplicationNode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveScriptSiteDebug32_OnScriptErrorDebug_Proxy( 
    IActiveScriptSiteDebug32 * This,
    /* [in] */ IActiveScriptErrorDebug *pErrorDebug,
    /* [out] */ BOOL *pfEnterDebugger,
    /* [out] */ BOOL *pfCallOnScriptErrorWhenContinuing);


void __RPC_STUB IActiveScriptSiteDebug32_OnScriptErrorDebug_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IActiveScriptSiteDebug32_INTERFACE_DEFINED__ */


#ifndef __IActiveScriptSiteDebug64_INTERFACE_DEFINED__
#define __IActiveScriptSiteDebug64_INTERFACE_DEFINED__

/* interface IActiveScriptSiteDebug64 */
/* [local][unique][uuid][object] */ 


EXTERN_C const IID IID_IActiveScriptSiteDebug64;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("d6b96b0a-7463-402c-92ac-89984226942f")
    IActiveScriptSiteDebug64 : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetDocumentContextFromPosition( 
            /* [in] */ DWORDLONG dwSourceContext,
            /* [in] */ ULONG uCharacterOffset,
            /* [in] */ ULONG uNumChars,
            /* [out] */ IDebugDocumentContext **ppsc) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetApplication( 
            /* [out] */ IDebugApplication64 **ppda) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetRootApplicationNode( 
            /* [out] */ IDebugApplicationNode **ppdanRoot) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnScriptErrorDebug( 
            /* [in] */ IActiveScriptErrorDebug *pErrorDebug,
            /* [out] */ BOOL *pfEnterDebugger,
            /* [out] */ BOOL *pfCallOnScriptErrorWhenContinuing) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IActiveScriptSiteDebug64Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IActiveScriptSiteDebug64 * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IActiveScriptSiteDebug64 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IActiveScriptSiteDebug64 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetDocumentContextFromPosition )( 
            IActiveScriptSiteDebug64 * This,
            /* [in] */ DWORDLONG dwSourceContext,
            /* [in] */ ULONG uCharacterOffset,
            /* [in] */ ULONG uNumChars,
            /* [out] */ IDebugDocumentContext **ppsc);
        
        HRESULT ( STDMETHODCALLTYPE *GetApplication )( 
            IActiveScriptSiteDebug64 * This,
            /* [out] */ IDebugApplication64 **ppda);
        
        HRESULT ( STDMETHODCALLTYPE *GetRootApplicationNode )( 
            IActiveScriptSiteDebug64 * This,
            /* [out] */ IDebugApplicationNode **ppdanRoot);
        
        HRESULT ( STDMETHODCALLTYPE *OnScriptErrorDebug )( 
            IActiveScriptSiteDebug64 * This,
            /* [in] */ IActiveScriptErrorDebug *pErrorDebug,
            /* [out] */ BOOL *pfEnterDebugger,
            /* [out] */ BOOL *pfCallOnScriptErrorWhenContinuing);
        
        END_INTERFACE
    } IActiveScriptSiteDebug64Vtbl;

    interface IActiveScriptSiteDebug64
    {
        CONST_VTBL struct IActiveScriptSiteDebug64Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IActiveScriptSiteDebug64_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IActiveScriptSiteDebug64_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IActiveScriptSiteDebug64_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IActiveScriptSiteDebug64_GetDocumentContextFromPosition(This,dwSourceContext,uCharacterOffset,uNumChars,ppsc)	\
    (This)->lpVtbl -> GetDocumentContextFromPosition(This,dwSourceContext,uCharacterOffset,uNumChars,ppsc)

#define IActiveScriptSiteDebug64_GetApplication(This,ppda)	\
    (This)->lpVtbl -> GetApplication(This,ppda)

#define IActiveScriptSiteDebug64_GetRootApplicationNode(This,ppdanRoot)	\
    (This)->lpVtbl -> GetRootApplicationNode(This,ppdanRoot)

#define IActiveScriptSiteDebug64_OnScriptErrorDebug(This,pErrorDebug,pfEnterDebugger,pfCallOnScriptErrorWhenContinuing)	\
    (This)->lpVtbl -> OnScriptErrorDebug(This,pErrorDebug,pfEnterDebugger,pfCallOnScriptErrorWhenContinuing)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IActiveScriptSiteDebug64_GetDocumentContextFromPosition_Proxy( 
    IActiveScriptSiteDebug64 * This,
    /* [in] */ DWORDLONG dwSourceContext,
    /* [in] */ ULONG uCharacterOffset,
    /* [in] */ ULONG uNumChars,
    /* [out] */ IDebugDocumentContext **ppsc);


void __RPC_STUB IActiveScriptSiteDebug64_GetDocumentContextFromPosition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveScriptSiteDebug64_GetApplication_Proxy( 
    IActiveScriptSiteDebug64 * This,
    /* [out] */ IDebugApplication64 **ppda);


void __RPC_STUB IActiveScriptSiteDebug64_GetApplication_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveScriptSiteDebug64_GetRootApplicationNode_Proxy( 
    IActiveScriptSiteDebug64 * This,
    /* [out] */ IDebugApplicationNode **ppdanRoot);


void __RPC_STUB IActiveScriptSiteDebug64_GetRootApplicationNode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveScriptSiteDebug64_OnScriptErrorDebug_Proxy( 
    IActiveScriptSiteDebug64 * This,
    /* [in] */ IActiveScriptErrorDebug *pErrorDebug,
    /* [out] */ BOOL *pfEnterDebugger,
    /* [out] */ BOOL *pfCallOnScriptErrorWhenContinuing);


void __RPC_STUB IActiveScriptSiteDebug64_OnScriptErrorDebug_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IActiveScriptSiteDebug64_INTERFACE_DEFINED__ */


#ifndef __IActiveScriptErrorDebug_INTERFACE_DEFINED__
#define __IActiveScriptErrorDebug_INTERFACE_DEFINED__

/* interface IActiveScriptErrorDebug */
/* [unique][uuid][object] */ 


EXTERN_C const IID IID_IActiveScriptErrorDebug;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("51973C12-CB0C-11d0-B5C9-00A0244A0E7A")
    IActiveScriptErrorDebug : public IActiveScriptError
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetDocumentContext( 
            /* [out] */ IDebugDocumentContext **ppssc) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetStackFrame( 
            /* [out] */ IDebugStackFrame **ppdsf) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IActiveScriptErrorDebugVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IActiveScriptErrorDebug * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IActiveScriptErrorDebug * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IActiveScriptErrorDebug * This);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *GetExceptionInfo )( 
            IActiveScriptErrorDebug * This,
            /* [out] */ EXCEPINFO *pexcepinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetSourcePosition )( 
            IActiveScriptErrorDebug * This,
            /* [out] */ DWORD *pdwSourceContext,
            /* [out] */ ULONG *pulLineNumber,
            /* [out] */ LONG *plCharacterPosition);
        
        HRESULT ( STDMETHODCALLTYPE *GetSourceLineText )( 
            IActiveScriptErrorDebug * This,
            /* [out] */ BSTR *pbstrSourceLine);
        
        HRESULT ( STDMETHODCALLTYPE *GetDocumentContext )( 
            IActiveScriptErrorDebug * This,
            /* [out] */ IDebugDocumentContext **ppssc);
        
        HRESULT ( STDMETHODCALLTYPE *GetStackFrame )( 
            IActiveScriptErrorDebug * This,
            /* [out] */ IDebugStackFrame **ppdsf);
        
        END_INTERFACE
    } IActiveScriptErrorDebugVtbl;

    interface IActiveScriptErrorDebug
    {
        CONST_VTBL struct IActiveScriptErrorDebugVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IActiveScriptErrorDebug_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IActiveScriptErrorDebug_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IActiveScriptErrorDebug_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IActiveScriptErrorDebug_GetExceptionInfo(This,pexcepinfo)	\
    (This)->lpVtbl -> GetExceptionInfo(This,pexcepinfo)

#define IActiveScriptErrorDebug_GetSourcePosition(This,pdwSourceContext,pulLineNumber,plCharacterPosition)	\
    (This)->lpVtbl -> GetSourcePosition(This,pdwSourceContext,pulLineNumber,plCharacterPosition)

#define IActiveScriptErrorDebug_GetSourceLineText(This,pbstrSourceLine)	\
    (This)->lpVtbl -> GetSourceLineText(This,pbstrSourceLine)


#define IActiveScriptErrorDebug_GetDocumentContext(This,ppssc)	\
    (This)->lpVtbl -> GetDocumentContext(This,ppssc)

#define IActiveScriptErrorDebug_GetStackFrame(This,ppdsf)	\
    (This)->lpVtbl -> GetStackFrame(This,ppdsf)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IActiveScriptErrorDebug_GetDocumentContext_Proxy( 
    IActiveScriptErrorDebug * This,
    /* [out] */ IDebugDocumentContext **ppssc);


void __RPC_STUB IActiveScriptErrorDebug_GetDocumentContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveScriptErrorDebug_GetStackFrame_Proxy( 
    IActiveScriptErrorDebug * This,
    /* [out] */ IDebugStackFrame **ppdsf);


void __RPC_STUB IActiveScriptErrorDebug_GetStackFrame_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IActiveScriptErrorDebug_INTERFACE_DEFINED__ */


#ifndef __IDebugCodeContext_INTERFACE_DEFINED__
#define __IDebugCodeContext_INTERFACE_DEFINED__

/* interface IDebugCodeContext */
/* [unique][uuid][object] */ 


EXTERN_C const IID IID_IDebugCodeContext;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("51973C13-CB0C-11d0-B5C9-00A0244A0E7A")
    IDebugCodeContext : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetDocumentContext( 
            /* [out] */ IDebugDocumentContext **ppsc) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetBreakPoint( 
            /* [in] */ BREAKPOINT_STATE bps) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDebugCodeContextVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDebugCodeContext * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDebugCodeContext * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDebugCodeContext * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetDocumentContext )( 
            IDebugCodeContext * This,
            /* [out] */ IDebugDocumentContext **ppsc);
        
        HRESULT ( STDMETHODCALLTYPE *SetBreakPoint )( 
            IDebugCodeContext * This,
            /* [in] */ BREAKPOINT_STATE bps);
        
        END_INTERFACE
    } IDebugCodeContextVtbl;

    interface IDebugCodeContext
    {
        CONST_VTBL struct IDebugCodeContextVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDebugCodeContext_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDebugCodeContext_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDebugCodeContext_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDebugCodeContext_GetDocumentContext(This,ppsc)	\
    (This)->lpVtbl -> GetDocumentContext(This,ppsc)

#define IDebugCodeContext_SetBreakPoint(This,bps)	\
    (This)->lpVtbl -> SetBreakPoint(This,bps)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IDebugCodeContext_GetDocumentContext_Proxy( 
    IDebugCodeContext * This,
    /* [out] */ IDebugDocumentContext **ppsc);


void __RPC_STUB IDebugCodeContext_GetDocumentContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugCodeContext_SetBreakPoint_Proxy( 
    IDebugCodeContext * This,
    /* [in] */ BREAKPOINT_STATE bps);


void __RPC_STUB IDebugCodeContext_SetBreakPoint_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IDebugCodeContext_INTERFACE_DEFINED__ */


#ifndef __IDebugExpression_INTERFACE_DEFINED__
#define __IDebugExpression_INTERFACE_DEFINED__

/* interface IDebugExpression */
/* [unique][uuid][object] */ 


EXTERN_C const IID IID_IDebugExpression;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("51973C14-CB0C-11d0-B5C9-00A0244A0E7A")
    IDebugExpression : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Start( 
            /* [in] */ IDebugExpressionCallBack *pdecb) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Abort( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE QueryIsComplete( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetResultAsString( 
            /* [out] */ HRESULT *phrResult,
            /* [out] */ BSTR *pbstrResult) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetResultAsDebugProperty( 
            /* [out] */ HRESULT *phrResult,
            /* [out] */ IDebugProperty **ppdp) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDebugExpressionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDebugExpression * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDebugExpression * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDebugExpression * This);
        
        HRESULT ( STDMETHODCALLTYPE *Start )( 
            IDebugExpression * This,
            /* [in] */ IDebugExpressionCallBack *pdecb);
        
        HRESULT ( STDMETHODCALLTYPE *Abort )( 
            IDebugExpression * This);
        
        HRESULT ( STDMETHODCALLTYPE *QueryIsComplete )( 
            IDebugExpression * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetResultAsString )( 
            IDebugExpression * This,
            /* [out] */ HRESULT *phrResult,
            /* [out] */ BSTR *pbstrResult);
        
        HRESULT ( STDMETHODCALLTYPE *GetResultAsDebugProperty )( 
            IDebugExpression * This,
            /* [out] */ HRESULT *phrResult,
            /* [out] */ IDebugProperty **ppdp);
        
        END_INTERFACE
    } IDebugExpressionVtbl;

    interface IDebugExpression
    {
        CONST_VTBL struct IDebugExpressionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDebugExpression_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDebugExpression_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDebugExpression_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDebugExpression_Start(This,pdecb)	\
    (This)->lpVtbl -> Start(This,pdecb)

#define IDebugExpression_Abort(This)	\
    (This)->lpVtbl -> Abort(This)

#define IDebugExpression_QueryIsComplete(This)	\
    (This)->lpVtbl -> QueryIsComplete(This)

#define IDebugExpression_GetResultAsString(This,phrResult,pbstrResult)	\
    (This)->lpVtbl -> GetResultAsString(This,phrResult,pbstrResult)

#define IDebugExpression_GetResultAsDebugProperty(This,phrResult,ppdp)	\
    (This)->lpVtbl -> GetResultAsDebugProperty(This,phrResult,ppdp)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IDebugExpression_Start_Proxy( 
    IDebugExpression * This,
    /* [in] */ IDebugExpressionCallBack *pdecb);


void __RPC_STUB IDebugExpression_Start_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugExpression_Abort_Proxy( 
    IDebugExpression * This);


void __RPC_STUB IDebugExpression_Abort_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugExpression_QueryIsComplete_Proxy( 
    IDebugExpression * This);


void __RPC_STUB IDebugExpression_QueryIsComplete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugExpression_GetResultAsString_Proxy( 
    IDebugExpression * This,
    /* [out] */ HRESULT *phrResult,
    /* [out] */ BSTR *pbstrResult);


void __RPC_STUB IDebugExpression_GetResultAsString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugExpression_GetResultAsDebugProperty_Proxy( 
    IDebugExpression * This,
    /* [out] */ HRESULT *phrResult,
    /* [out] */ IDebugProperty **ppdp);


void __RPC_STUB IDebugExpression_GetResultAsDebugProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IDebugExpression_INTERFACE_DEFINED__ */


#ifndef __IDebugExpressionContext_INTERFACE_DEFINED__
#define __IDebugExpressionContext_INTERFACE_DEFINED__

/* interface IDebugExpressionContext */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IDebugExpressionContext;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("51973C15-CB0C-11d0-B5C9-00A0244A0E7A")
    IDebugExpressionContext : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ParseLanguageText( 
            /* [in] */ LPCOLESTR pstrCode,
            /* [in] */ UINT nRadix,
            /* [in] */ LPCOLESTR pstrDelimiter,
            /* [in] */ DWORD dwFlags,
            /* [out] */ IDebugExpression **ppe) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetLanguageInfo( 
            /* [out] */ BSTR *pbstrLanguageName,
            /* [out] */ GUID *pLanguageID) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDebugExpressionContextVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDebugExpressionContext * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDebugExpressionContext * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDebugExpressionContext * This);
        
        HRESULT ( STDMETHODCALLTYPE *ParseLanguageText )( 
            IDebugExpressionContext * This,
            /* [in] */ LPCOLESTR pstrCode,
            /* [in] */ UINT nRadix,
            /* [in] */ LPCOLESTR pstrDelimiter,
            /* [in] */ DWORD dwFlags,
            /* [out] */ IDebugExpression **ppe);
        
        HRESULT ( STDMETHODCALLTYPE *GetLanguageInfo )( 
            IDebugExpressionContext * This,
            /* [out] */ BSTR *pbstrLanguageName,
            /* [out] */ GUID *pLanguageID);
        
        END_INTERFACE
    } IDebugExpressionContextVtbl;

    interface IDebugExpressionContext
    {
        CONST_VTBL struct IDebugExpressionContextVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDebugExpressionContext_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDebugExpressionContext_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDebugExpressionContext_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDebugExpressionContext_ParseLanguageText(This,pstrCode,nRadix,pstrDelimiter,dwFlags,ppe)	\
    (This)->lpVtbl -> ParseLanguageText(This,pstrCode,nRadix,pstrDelimiter,dwFlags,ppe)

#define IDebugExpressionContext_GetLanguageInfo(This,pbstrLanguageName,pLanguageID)	\
    (This)->lpVtbl -> GetLanguageInfo(This,pbstrLanguageName,pLanguageID)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IDebugExpressionContext_ParseLanguageText_Proxy( 
    IDebugExpressionContext * This,
    /* [in] */ LPCOLESTR pstrCode,
    /* [in] */ UINT nRadix,
    /* [in] */ LPCOLESTR pstrDelimiter,
    /* [in] */ DWORD dwFlags,
    /* [out] */ IDebugExpression **ppe);


void __RPC_STUB IDebugExpressionContext_ParseLanguageText_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugExpressionContext_GetLanguageInfo_Proxy( 
    IDebugExpressionContext * This,
    /* [out] */ BSTR *pbstrLanguageName,
    /* [out] */ GUID *pLanguageID);


void __RPC_STUB IDebugExpressionContext_GetLanguageInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IDebugExpressionContext_INTERFACE_DEFINED__ */


#ifndef __IDebugExpressionCallBack_INTERFACE_DEFINED__
#define __IDebugExpressionCallBack_INTERFACE_DEFINED__

/* interface IDebugExpressionCallBack */
/* [unique][uuid][object] */ 


EXTERN_C const IID IID_IDebugExpressionCallBack;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("51973C16-CB0C-11d0-B5C9-00A0244A0E7A")
    IDebugExpressionCallBack : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE onComplete( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDebugExpressionCallBackVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDebugExpressionCallBack * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDebugExpressionCallBack * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDebugExpressionCallBack * This);
        
        HRESULT ( STDMETHODCALLTYPE *onComplete )( 
            IDebugExpressionCallBack * This);
        
        END_INTERFACE
    } IDebugExpressionCallBackVtbl;

    interface IDebugExpressionCallBack
    {
        CONST_VTBL struct IDebugExpressionCallBackVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDebugExpressionCallBack_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDebugExpressionCallBack_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDebugExpressionCallBack_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDebugExpressionCallBack_onComplete(This)	\
    (This)->lpVtbl -> onComplete(This)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IDebugExpressionCallBack_onComplete_Proxy( 
    IDebugExpressionCallBack * This);


void __RPC_STUB IDebugExpressionCallBack_onComplete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IDebugExpressionCallBack_INTERFACE_DEFINED__ */


#ifndef __IDebugStackFrame_INTERFACE_DEFINED__
#define __IDebugStackFrame_INTERFACE_DEFINED__

/* interface IDebugStackFrame */
/* [unique][uuid][object] */ 


EXTERN_C const IID IID_IDebugStackFrame;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("51973C17-CB0C-11d0-B5C9-00A0244A0E7A")
    IDebugStackFrame : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetCodeContext( 
            /* [out] */ IDebugCodeContext **ppcc) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDescriptionString( 
            /* [in] */ BOOL fLong,
            /* [out] */ BSTR *pbstrDescription) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetLanguageString( 
            /* [in] */ BOOL fLong,
            /* [out] */ BSTR *pbstrLanguage) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetThread( 
            /* [out] */ IDebugApplicationThread **ppat) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDebugProperty( 
            /* [out] */ IDebugProperty **ppDebugProp) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDebugStackFrameVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDebugStackFrame * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDebugStackFrame * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDebugStackFrame * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetCodeContext )( 
            IDebugStackFrame * This,
            /* [out] */ IDebugCodeContext **ppcc);
        
        HRESULT ( STDMETHODCALLTYPE *GetDescriptionString )( 
            IDebugStackFrame * This,
            /* [in] */ BOOL fLong,
            /* [out] */ BSTR *pbstrDescription);
        
        HRESULT ( STDMETHODCALLTYPE *GetLanguageString )( 
            IDebugStackFrame * This,
            /* [in] */ BOOL fLong,
            /* [out] */ BSTR *pbstrLanguage);
        
        HRESULT ( STDMETHODCALLTYPE *GetThread )( 
            IDebugStackFrame * This,
            /* [out] */ IDebugApplicationThread **ppat);
        
        HRESULT ( STDMETHODCALLTYPE *GetDebugProperty )( 
            IDebugStackFrame * This,
            /* [out] */ IDebugProperty **ppDebugProp);
        
        END_INTERFACE
    } IDebugStackFrameVtbl;

    interface IDebugStackFrame
    {
        CONST_VTBL struct IDebugStackFrameVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDebugStackFrame_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDebugStackFrame_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDebugStackFrame_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDebugStackFrame_GetCodeContext(This,ppcc)	\
    (This)->lpVtbl -> GetCodeContext(This,ppcc)

#define IDebugStackFrame_GetDescriptionString(This,fLong,pbstrDescription)	\
    (This)->lpVtbl -> GetDescriptionString(This,fLong,pbstrDescription)

#define IDebugStackFrame_GetLanguageString(This,fLong,pbstrLanguage)	\
    (This)->lpVtbl -> GetLanguageString(This,fLong,pbstrLanguage)

#define IDebugStackFrame_GetThread(This,ppat)	\
    (This)->lpVtbl -> GetThread(This,ppat)

#define IDebugStackFrame_GetDebugProperty(This,ppDebugProp)	\
    (This)->lpVtbl -> GetDebugProperty(This,ppDebugProp)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IDebugStackFrame_GetCodeContext_Proxy( 
    IDebugStackFrame * This,
    /* [out] */ IDebugCodeContext **ppcc);


void __RPC_STUB IDebugStackFrame_GetCodeContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugStackFrame_GetDescriptionString_Proxy( 
    IDebugStackFrame * This,
    /* [in] */ BOOL fLong,
    /* [out] */ BSTR *pbstrDescription);


void __RPC_STUB IDebugStackFrame_GetDescriptionString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugStackFrame_GetLanguageString_Proxy( 
    IDebugStackFrame * This,
    /* [in] */ BOOL fLong,
    /* [out] */ BSTR *pbstrLanguage);


void __RPC_STUB IDebugStackFrame_GetLanguageString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugStackFrame_GetThread_Proxy( 
    IDebugStackFrame * This,
    /* [out] */ IDebugApplicationThread **ppat);


void __RPC_STUB IDebugStackFrame_GetThread_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugStackFrame_GetDebugProperty_Proxy( 
    IDebugStackFrame * This,
    /* [out] */ IDebugProperty **ppDebugProp);


void __RPC_STUB IDebugStackFrame_GetDebugProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IDebugStackFrame_INTERFACE_DEFINED__ */


#ifndef __IDebugStackFrameSniffer_INTERFACE_DEFINED__
#define __IDebugStackFrameSniffer_INTERFACE_DEFINED__

/* interface IDebugStackFrameSniffer */
/* [unique][uuid][object] */ 


EXTERN_C const IID IID_IDebugStackFrameSniffer;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("51973C18-CB0C-11d0-B5C9-00A0244A0E7A")
    IDebugStackFrameSniffer : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE EnumStackFrames( 
            /* [out] */ IEnumDebugStackFrames **ppedsf) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDebugStackFrameSnifferVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDebugStackFrameSniffer * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDebugStackFrameSniffer * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDebugStackFrameSniffer * This);
        
        HRESULT ( STDMETHODCALLTYPE *EnumStackFrames )( 
            IDebugStackFrameSniffer * This,
            /* [out] */ IEnumDebugStackFrames **ppedsf);
        
        END_INTERFACE
    } IDebugStackFrameSnifferVtbl;

    interface IDebugStackFrameSniffer
    {
        CONST_VTBL struct IDebugStackFrameSnifferVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDebugStackFrameSniffer_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDebugStackFrameSniffer_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDebugStackFrameSniffer_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDebugStackFrameSniffer_EnumStackFrames(This,ppedsf)	\
    (This)->lpVtbl -> EnumStackFrames(This,ppedsf)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IDebugStackFrameSniffer_EnumStackFrames_Proxy( 
    IDebugStackFrameSniffer * This,
    /* [out] */ IEnumDebugStackFrames **ppedsf);


void __RPC_STUB IDebugStackFrameSniffer_EnumStackFrames_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IDebugStackFrameSniffer_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_activdbg_0314 */
/* [local] */ 

#ifndef DISABLE_ACTIVDBG_INTERFACE_WRAPPERS
#ifdef _WIN64
#define IDebugStackFrameSnifferEx IDebugStackFrameSnifferEx64
#define IID_IDebugStackFrameSnifferEx IID_IDebugStackFrameSnifferEx64
#define EnumStackFramesEx EnumStackFramesEx64
#else
#define IDebugStackFrameSnifferEx IDebugStackFrameSnifferEx32
#define IID_IDebugStackFrameSnifferEx IID_IDebugStackFrameSnifferEx32
#define EnumStackFramesEx EnumStackFramesEx32
#endif
#endif


extern RPC_IF_HANDLE __MIDL_itf_activdbg_0314_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_activdbg_0314_v0_0_s_ifspec;

#ifndef __IDebugStackFrameSnifferEx32_INTERFACE_DEFINED__
#define __IDebugStackFrameSnifferEx32_INTERFACE_DEFINED__

/* interface IDebugStackFrameSnifferEx32 */
/* [unique][uuid][object] */ 


EXTERN_C const IID IID_IDebugStackFrameSnifferEx32;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("51973C19-CB0C-11d0-B5C9-00A0244A0E7A")
    IDebugStackFrameSnifferEx32 : public IDebugStackFrameSniffer
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE EnumStackFramesEx32( 
            /* [in] */ DWORD dwSpMin,
            /* [out] */ IEnumDebugStackFrames **ppedsf) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDebugStackFrameSnifferEx32Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDebugStackFrameSnifferEx32 * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDebugStackFrameSnifferEx32 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDebugStackFrameSnifferEx32 * This);
        
        HRESULT ( STDMETHODCALLTYPE *EnumStackFrames )( 
            IDebugStackFrameSnifferEx32 * This,
            /* [out] */ IEnumDebugStackFrames **ppedsf);
        
        HRESULT ( STDMETHODCALLTYPE *EnumStackFramesEx32 )( 
            IDebugStackFrameSnifferEx32 * This,
            /* [in] */ DWORD dwSpMin,
            /* [out] */ IEnumDebugStackFrames **ppedsf);
        
        END_INTERFACE
    } IDebugStackFrameSnifferEx32Vtbl;

    interface IDebugStackFrameSnifferEx32
    {
        CONST_VTBL struct IDebugStackFrameSnifferEx32Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDebugStackFrameSnifferEx32_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDebugStackFrameSnifferEx32_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDebugStackFrameSnifferEx32_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDebugStackFrameSnifferEx32_EnumStackFrames(This,ppedsf)	\
    (This)->lpVtbl -> EnumStackFrames(This,ppedsf)


#define IDebugStackFrameSnifferEx32_EnumStackFramesEx32(This,dwSpMin,ppedsf)	\
    (This)->lpVtbl -> EnumStackFramesEx32(This,dwSpMin,ppedsf)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IDebugStackFrameSnifferEx32_EnumStackFramesEx32_Proxy( 
    IDebugStackFrameSnifferEx32 * This,
    /* [in] */ DWORD dwSpMin,
    /* [out] */ IEnumDebugStackFrames **ppedsf);


void __RPC_STUB IDebugStackFrameSnifferEx32_EnumStackFramesEx32_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IDebugStackFrameSnifferEx32_INTERFACE_DEFINED__ */


#ifndef __IDebugStackFrameSnifferEx64_INTERFACE_DEFINED__
#define __IDebugStackFrameSnifferEx64_INTERFACE_DEFINED__

/* interface IDebugStackFrameSnifferEx64 */
/* [unique][uuid][object] */ 


EXTERN_C const IID IID_IDebugStackFrameSnifferEx64;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("8cd12af4-49c1-4d52-8d8a-c146f47581aa")
    IDebugStackFrameSnifferEx64 : public IDebugStackFrameSniffer
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE EnumStackFramesEx64( 
            /* [in] */ DWORDLONG dwSpMin,
            /* [out] */ IEnumDebugStackFrames64 **ppedsf) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDebugStackFrameSnifferEx64Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDebugStackFrameSnifferEx64 * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDebugStackFrameSnifferEx64 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDebugStackFrameSnifferEx64 * This);
        
        HRESULT ( STDMETHODCALLTYPE *EnumStackFrames )( 
            IDebugStackFrameSnifferEx64 * This,
            /* [out] */ IEnumDebugStackFrames **ppedsf);
        
        HRESULT ( STDMETHODCALLTYPE *EnumStackFramesEx64 )( 
            IDebugStackFrameSnifferEx64 * This,
            /* [in] */ DWORDLONG dwSpMin,
            /* [out] */ IEnumDebugStackFrames64 **ppedsf);
        
        END_INTERFACE
    } IDebugStackFrameSnifferEx64Vtbl;

    interface IDebugStackFrameSnifferEx64
    {
        CONST_VTBL struct IDebugStackFrameSnifferEx64Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDebugStackFrameSnifferEx64_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDebugStackFrameSnifferEx64_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDebugStackFrameSnifferEx64_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDebugStackFrameSnifferEx64_EnumStackFrames(This,ppedsf)	\
    (This)->lpVtbl -> EnumStackFrames(This,ppedsf)


#define IDebugStackFrameSnifferEx64_EnumStackFramesEx64(This,dwSpMin,ppedsf)	\
    (This)->lpVtbl -> EnumStackFramesEx64(This,dwSpMin,ppedsf)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IDebugStackFrameSnifferEx64_EnumStackFramesEx64_Proxy( 
    IDebugStackFrameSnifferEx64 * This,
    /* [in] */ DWORDLONG dwSpMin,
    /* [out] */ IEnumDebugStackFrames64 **ppedsf);


void __RPC_STUB IDebugStackFrameSnifferEx64_EnumStackFramesEx64_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IDebugStackFrameSnifferEx64_INTERFACE_DEFINED__ */


#ifndef __IDebugSyncOperation_INTERFACE_DEFINED__
#define __IDebugSyncOperation_INTERFACE_DEFINED__

/* interface IDebugSyncOperation */
/* [local][unique][uuid][object] */ 


EXTERN_C const IID IID_IDebugSyncOperation;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("51973C1a-CB0C-11d0-B5C9-00A0244A0E7A")
    IDebugSyncOperation : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetTargetThread( 
            /* [out] */ IDebugApplicationThread **ppatTarget) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Execute( 
            /* [out] */ IUnknown **ppunkResult) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE InProgressAbort( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDebugSyncOperationVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDebugSyncOperation * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDebugSyncOperation * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDebugSyncOperation * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTargetThread )( 
            IDebugSyncOperation * This,
            /* [out] */ IDebugApplicationThread **ppatTarget);
        
        HRESULT ( STDMETHODCALLTYPE *Execute )( 
            IDebugSyncOperation * This,
            /* [out] */ IUnknown **ppunkResult);
        
        HRESULT ( STDMETHODCALLTYPE *InProgressAbort )( 
            IDebugSyncOperation * This);
        
        END_INTERFACE
    } IDebugSyncOperationVtbl;

    interface IDebugSyncOperation
    {
        CONST_VTBL struct IDebugSyncOperationVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDebugSyncOperation_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDebugSyncOperation_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDebugSyncOperation_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDebugSyncOperation_GetTargetThread(This,ppatTarget)	\
    (This)->lpVtbl -> GetTargetThread(This,ppatTarget)

#define IDebugSyncOperation_Execute(This,ppunkResult)	\
    (This)->lpVtbl -> Execute(This,ppunkResult)

#define IDebugSyncOperation_InProgressAbort(This)	\
    (This)->lpVtbl -> InProgressAbort(This)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IDebugSyncOperation_GetTargetThread_Proxy( 
    IDebugSyncOperation * This,
    /* [out] */ IDebugApplicationThread **ppatTarget);


void __RPC_STUB IDebugSyncOperation_GetTargetThread_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugSyncOperation_Execute_Proxy( 
    IDebugSyncOperation * This,
    /* [out] */ IUnknown **ppunkResult);


void __RPC_STUB IDebugSyncOperation_Execute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugSyncOperation_InProgressAbort_Proxy( 
    IDebugSyncOperation * This);


void __RPC_STUB IDebugSyncOperation_InProgressAbort_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IDebugSyncOperation_INTERFACE_DEFINED__ */


#ifndef __IDebugAsyncOperation_INTERFACE_DEFINED__
#define __IDebugAsyncOperation_INTERFACE_DEFINED__

/* interface IDebugAsyncOperation */
/* [local][unique][uuid][object] */ 


EXTERN_C const IID IID_IDebugAsyncOperation;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("51973C1b-CB0C-11d0-B5C9-00A0244A0E7A")
    IDebugAsyncOperation : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetSyncDebugOperation( 
            /* [out] */ IDebugSyncOperation **ppsdo) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Start( 
            IDebugAsyncOperationCallBack *padocb) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Abort( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE QueryIsComplete( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetResult( 
            /* [out] */ HRESULT *phrResult,
            /* [out] */ IUnknown **ppunkResult) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDebugAsyncOperationVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDebugAsyncOperation * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDebugAsyncOperation * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDebugAsyncOperation * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetSyncDebugOperation )( 
            IDebugAsyncOperation * This,
            /* [out] */ IDebugSyncOperation **ppsdo);
        
        HRESULT ( STDMETHODCALLTYPE *Start )( 
            IDebugAsyncOperation * This,
            IDebugAsyncOperationCallBack *padocb);
        
        HRESULT ( STDMETHODCALLTYPE *Abort )( 
            IDebugAsyncOperation * This);
        
        HRESULT ( STDMETHODCALLTYPE *QueryIsComplete )( 
            IDebugAsyncOperation * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetResult )( 
            IDebugAsyncOperation * This,
            /* [out] */ HRESULT *phrResult,
            /* [out] */ IUnknown **ppunkResult);
        
        END_INTERFACE
    } IDebugAsyncOperationVtbl;

    interface IDebugAsyncOperation
    {
        CONST_VTBL struct IDebugAsyncOperationVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDebugAsyncOperation_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDebugAsyncOperation_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDebugAsyncOperation_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDebugAsyncOperation_GetSyncDebugOperation(This,ppsdo)	\
    (This)->lpVtbl -> GetSyncDebugOperation(This,ppsdo)

#define IDebugAsyncOperation_Start(This,padocb)	\
    (This)->lpVtbl -> Start(This,padocb)

#define IDebugAsyncOperation_Abort(This)	\
    (This)->lpVtbl -> Abort(This)

#define IDebugAsyncOperation_QueryIsComplete(This)	\
    (This)->lpVtbl -> QueryIsComplete(This)

#define IDebugAsyncOperation_GetResult(This,phrResult,ppunkResult)	\
    (This)->lpVtbl -> GetResult(This,phrResult,ppunkResult)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IDebugAsyncOperation_GetSyncDebugOperation_Proxy( 
    IDebugAsyncOperation * This,
    /* [out] */ IDebugSyncOperation **ppsdo);


void __RPC_STUB IDebugAsyncOperation_GetSyncDebugOperation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugAsyncOperation_Start_Proxy( 
    IDebugAsyncOperation * This,
    IDebugAsyncOperationCallBack *padocb);


void __RPC_STUB IDebugAsyncOperation_Start_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugAsyncOperation_Abort_Proxy( 
    IDebugAsyncOperation * This);


void __RPC_STUB IDebugAsyncOperation_Abort_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugAsyncOperation_QueryIsComplete_Proxy( 
    IDebugAsyncOperation * This);


void __RPC_STUB IDebugAsyncOperation_QueryIsComplete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugAsyncOperation_GetResult_Proxy( 
    IDebugAsyncOperation * This,
    /* [out] */ HRESULT *phrResult,
    /* [out] */ IUnknown **ppunkResult);


void __RPC_STUB IDebugAsyncOperation_GetResult_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IDebugAsyncOperation_INTERFACE_DEFINED__ */


#ifndef __IDebugAsyncOperationCallBack_INTERFACE_DEFINED__
#define __IDebugAsyncOperationCallBack_INTERFACE_DEFINED__

/* interface IDebugAsyncOperationCallBack */
/* [local][unique][uuid][object] */ 


EXTERN_C const IID IID_IDebugAsyncOperationCallBack;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("51973C1c-CB0C-11d0-B5C9-00A0244A0E7A")
    IDebugAsyncOperationCallBack : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE onComplete( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDebugAsyncOperationCallBackVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDebugAsyncOperationCallBack * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDebugAsyncOperationCallBack * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDebugAsyncOperationCallBack * This);
        
        HRESULT ( STDMETHODCALLTYPE *onComplete )( 
            IDebugAsyncOperationCallBack * This);
        
        END_INTERFACE
    } IDebugAsyncOperationCallBackVtbl;

    interface IDebugAsyncOperationCallBack
    {
        CONST_VTBL struct IDebugAsyncOperationCallBackVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDebugAsyncOperationCallBack_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDebugAsyncOperationCallBack_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDebugAsyncOperationCallBack_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDebugAsyncOperationCallBack_onComplete(This)	\
    (This)->lpVtbl -> onComplete(This)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IDebugAsyncOperationCallBack_onComplete_Proxy( 
    IDebugAsyncOperationCallBack * This);


void __RPC_STUB IDebugAsyncOperationCallBack_onComplete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IDebugAsyncOperationCallBack_INTERFACE_DEFINED__ */


#ifndef __IEnumDebugCodeContexts_INTERFACE_DEFINED__
#define __IEnumDebugCodeContexts_INTERFACE_DEFINED__

/* interface IEnumDebugCodeContexts */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IEnumDebugCodeContexts;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("51973C1d-CB0C-11d0-B5C9-00A0244A0E7A")
    IEnumDebugCodeContexts : public IUnknown
    {
    public:
        virtual /* [local] */ HRESULT __stdcall Next( 
            /* [in] */ ULONG celt,
            /* [out] */ IDebugCodeContext **pscc,
            /* [out] */ ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
            /* [in] */ ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
            /* [out] */ IEnumDebugCodeContexts **ppescc) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IEnumDebugCodeContextsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumDebugCodeContexts * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumDebugCodeContexts * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumDebugCodeContexts * This);
        
        /* [local] */ HRESULT ( __stdcall *Next )( 
            IEnumDebugCodeContexts * This,
            /* [in] */ ULONG celt,
            /* [out] */ IDebugCodeContext **pscc,
            /* [out] */ ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumDebugCodeContexts * This,
            /* [in] */ ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumDebugCodeContexts * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumDebugCodeContexts * This,
            /* [out] */ IEnumDebugCodeContexts **ppescc);
        
        END_INTERFACE
    } IEnumDebugCodeContextsVtbl;

    interface IEnumDebugCodeContexts
    {
        CONST_VTBL struct IEnumDebugCodeContextsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumDebugCodeContexts_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumDebugCodeContexts_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumDebugCodeContexts_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumDebugCodeContexts_Next(This,celt,pscc,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,pscc,pceltFetched)

#define IEnumDebugCodeContexts_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumDebugCodeContexts_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumDebugCodeContexts_Clone(This,ppescc)	\
    (This)->lpVtbl -> Clone(This,ppescc)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [call_as] */ HRESULT __stdcall IEnumDebugCodeContexts_RemoteNext_Proxy( 
    IEnumDebugCodeContexts * This,
    /* [in] */ ULONG celt,
    /* [length_is][size_is][out] */ IDebugCodeContext **pscc,
    /* [out] */ ULONG *pceltFetched);


void __RPC_STUB IEnumDebugCodeContexts_RemoteNext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumDebugCodeContexts_Skip_Proxy( 
    IEnumDebugCodeContexts * This,
    /* [in] */ ULONG celt);


void __RPC_STUB IEnumDebugCodeContexts_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumDebugCodeContexts_Reset_Proxy( 
    IEnumDebugCodeContexts * This);


void __RPC_STUB IEnumDebugCodeContexts_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumDebugCodeContexts_Clone_Proxy( 
    IEnumDebugCodeContexts * This,
    /* [out] */ IEnumDebugCodeContexts **ppescc);


void __RPC_STUB IEnumDebugCodeContexts_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IEnumDebugCodeContexts_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_activdbg_0320 */
/* [local] */ 

typedef struct tagDebugStackFrameDescriptor
    {
    IDebugStackFrame *pdsf;
    DWORD dwMin;
    DWORD dwLim;
    BOOL fFinal;
    IUnknown *punkFinal;
    } 	DebugStackFrameDescriptor;

typedef struct tagDebugStackFrameDescriptor64
    {
    IDebugStackFrame *pdsf;
    DWORDLONG dwMin;
    DWORDLONG dwLim;
    BOOL fFinal;
    IUnknown *punkFinal;
    } 	DebugStackFrameDescriptor64;



extern RPC_IF_HANDLE __MIDL_itf_activdbg_0320_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_activdbg_0320_v0_0_s_ifspec;

#ifndef __IEnumDebugStackFrames_INTERFACE_DEFINED__
#define __IEnumDebugStackFrames_INTERFACE_DEFINED__

/* interface IEnumDebugStackFrames */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IEnumDebugStackFrames;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("51973C1e-CB0C-11d0-B5C9-00A0244A0E7A")
    IEnumDebugStackFrames : public IUnknown
    {
    public:
        virtual /* [local] */ HRESULT __stdcall Next( 
            /* [in] */ ULONG celt,
            /* [out] */ DebugStackFrameDescriptor *prgdsfd,
            /* [out] */ ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
            /* [in] */ ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
            /* [out] */ IEnumDebugStackFrames **ppedsf) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IEnumDebugStackFramesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumDebugStackFrames * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumDebugStackFrames * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumDebugStackFrames * This);
        
        /* [local] */ HRESULT ( __stdcall *Next )( 
            IEnumDebugStackFrames * This,
            /* [in] */ ULONG celt,
            /* [out] */ DebugStackFrameDescriptor *prgdsfd,
            /* [out] */ ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumDebugStackFrames * This,
            /* [in] */ ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumDebugStackFrames * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumDebugStackFrames * This,
            /* [out] */ IEnumDebugStackFrames **ppedsf);
        
        END_INTERFACE
    } IEnumDebugStackFramesVtbl;

    interface IEnumDebugStackFrames
    {
        CONST_VTBL struct IEnumDebugStackFramesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumDebugStackFrames_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumDebugStackFrames_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumDebugStackFrames_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumDebugStackFrames_Next(This,celt,prgdsfd,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,prgdsfd,pceltFetched)

#define IEnumDebugStackFrames_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumDebugStackFrames_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumDebugStackFrames_Clone(This,ppedsf)	\
    (This)->lpVtbl -> Clone(This,ppedsf)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [call_as] */ HRESULT __stdcall IEnumDebugStackFrames_RemoteNext_Proxy( 
    IEnumDebugStackFrames * This,
    /* [in] */ ULONG celt,
    /* [length_is][size_is][out] */ DebugStackFrameDescriptor *prgdsfd,
    /* [out] */ ULONG *pceltFetched);


void __RPC_STUB IEnumDebugStackFrames_RemoteNext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumDebugStackFrames_Skip_Proxy( 
    IEnumDebugStackFrames * This,
    /* [in] */ ULONG celt);


void __RPC_STUB IEnumDebugStackFrames_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumDebugStackFrames_Reset_Proxy( 
    IEnumDebugStackFrames * This);


void __RPC_STUB IEnumDebugStackFrames_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumDebugStackFrames_Clone_Proxy( 
    IEnumDebugStackFrames * This,
    /* [out] */ IEnumDebugStackFrames **ppedsf);


void __RPC_STUB IEnumDebugStackFrames_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IEnumDebugStackFrames_INTERFACE_DEFINED__ */


#ifndef __IEnumDebugStackFrames64_INTERFACE_DEFINED__
#define __IEnumDebugStackFrames64_INTERFACE_DEFINED__

/* interface IEnumDebugStackFrames64 */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IEnumDebugStackFrames64;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0dc38853-c1b0-4176-a984-b298361027af")
    IEnumDebugStackFrames64 : public IEnumDebugStackFrames
    {
    public:
        virtual /* [local] */ HRESULT __stdcall Next64( 
            /* [in] */ ULONG celt,
            /* [out] */ DebugStackFrameDescriptor64 *prgdsfd,
            /* [out] */ ULONG *pceltFetched) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IEnumDebugStackFrames64Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumDebugStackFrames64 * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumDebugStackFrames64 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumDebugStackFrames64 * This);
        
        /* [local] */ HRESULT ( __stdcall *Next )( 
            IEnumDebugStackFrames64 * This,
            /* [in] */ ULONG celt,
            /* [out] */ DebugStackFrameDescriptor *prgdsfd,
            /* [out] */ ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumDebugStackFrames64 * This,
            /* [in] */ ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumDebugStackFrames64 * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumDebugStackFrames64 * This,
            /* [out] */ IEnumDebugStackFrames **ppedsf);
        
        /* [local] */ HRESULT ( __stdcall *Next64 )( 
            IEnumDebugStackFrames64 * This,
            /* [in] */ ULONG celt,
            /* [out] */ DebugStackFrameDescriptor64 *prgdsfd,
            /* [out] */ ULONG *pceltFetched);
        
        END_INTERFACE
    } IEnumDebugStackFrames64Vtbl;

    interface IEnumDebugStackFrames64
    {
        CONST_VTBL struct IEnumDebugStackFrames64Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumDebugStackFrames64_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumDebugStackFrames64_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumDebugStackFrames64_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumDebugStackFrames64_Next(This,celt,prgdsfd,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,prgdsfd,pceltFetched)

#define IEnumDebugStackFrames64_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumDebugStackFrames64_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumDebugStackFrames64_Clone(This,ppedsf)	\
    (This)->lpVtbl -> Clone(This,ppedsf)


#define IEnumDebugStackFrames64_Next64(This,celt,prgdsfd,pceltFetched)	\
    (This)->lpVtbl -> Next64(This,celt,prgdsfd,pceltFetched)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [call_as] */ HRESULT __stdcall IEnumDebugStackFrames64_RemoteNext64_Proxy( 
    IEnumDebugStackFrames64 * This,
    /* [in] */ ULONG celt,
    /* [length_is][size_is][out] */ DebugStackFrameDescriptor64 *prgdsfd,
    /* [out] */ ULONG *pceltFetched);


void __RPC_STUB IEnumDebugStackFrames64_RemoteNext64_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IEnumDebugStackFrames64_INTERFACE_DEFINED__ */


#ifndef __IDebugDocumentInfo_INTERFACE_DEFINED__
#define __IDebugDocumentInfo_INTERFACE_DEFINED__

/* interface IDebugDocumentInfo */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IDebugDocumentInfo;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("51973C1f-CB0C-11d0-B5C9-00A0244A0E7A")
    IDebugDocumentInfo : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetName( 
            /* [in] */ DOCUMENTNAMETYPE dnt,
            /* [out] */ BSTR *pbstrName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDocumentClassId( 
            /* [out] */ CLSID *pclsidDocument) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDebugDocumentInfoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDebugDocumentInfo * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDebugDocumentInfo * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDebugDocumentInfo * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetName )( 
            IDebugDocumentInfo * This,
            /* [in] */ DOCUMENTNAMETYPE dnt,
            /* [out] */ BSTR *pbstrName);
        
        HRESULT ( STDMETHODCALLTYPE *GetDocumentClassId )( 
            IDebugDocumentInfo * This,
            /* [out] */ CLSID *pclsidDocument);
        
        END_INTERFACE
    } IDebugDocumentInfoVtbl;

    interface IDebugDocumentInfo
    {
        CONST_VTBL struct IDebugDocumentInfoVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDebugDocumentInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDebugDocumentInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDebugDocumentInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDebugDocumentInfo_GetName(This,dnt,pbstrName)	\
    (This)->lpVtbl -> GetName(This,dnt,pbstrName)

#define IDebugDocumentInfo_GetDocumentClassId(This,pclsidDocument)	\
    (This)->lpVtbl -> GetDocumentClassId(This,pclsidDocument)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IDebugDocumentInfo_GetName_Proxy( 
    IDebugDocumentInfo * This,
    /* [in] */ DOCUMENTNAMETYPE dnt,
    /* [out] */ BSTR *pbstrName);


void __RPC_STUB IDebugDocumentInfo_GetName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugDocumentInfo_GetDocumentClassId_Proxy( 
    IDebugDocumentInfo * This,
    /* [out] */ CLSID *pclsidDocument);


void __RPC_STUB IDebugDocumentInfo_GetDocumentClassId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IDebugDocumentInfo_INTERFACE_DEFINED__ */


#ifndef __IDebugDocumentProvider_INTERFACE_DEFINED__
#define __IDebugDocumentProvider_INTERFACE_DEFINED__

/* interface IDebugDocumentProvider */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IDebugDocumentProvider;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("51973C20-CB0C-11d0-B5C9-00A0244A0E7A")
    IDebugDocumentProvider : public IDebugDocumentInfo
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetDocument( 
            /* [out] */ IDebugDocument **ppssd) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDebugDocumentProviderVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDebugDocumentProvider * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDebugDocumentProvider * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDebugDocumentProvider * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetName )( 
            IDebugDocumentProvider * This,
            /* [in] */ DOCUMENTNAMETYPE dnt,
            /* [out] */ BSTR *pbstrName);
        
        HRESULT ( STDMETHODCALLTYPE *GetDocumentClassId )( 
            IDebugDocumentProvider * This,
            /* [out] */ CLSID *pclsidDocument);
        
        HRESULT ( STDMETHODCALLTYPE *GetDocument )( 
            IDebugDocumentProvider * This,
            /* [out] */ IDebugDocument **ppssd);
        
        END_INTERFACE
    } IDebugDocumentProviderVtbl;

    interface IDebugDocumentProvider
    {
        CONST_VTBL struct IDebugDocumentProviderVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDebugDocumentProvider_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDebugDocumentProvider_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDebugDocumentProvider_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDebugDocumentProvider_GetName(This,dnt,pbstrName)	\
    (This)->lpVtbl -> GetName(This,dnt,pbstrName)

#define IDebugDocumentProvider_GetDocumentClassId(This,pclsidDocument)	\
    (This)->lpVtbl -> GetDocumentClassId(This,pclsidDocument)


#define IDebugDocumentProvider_GetDocument(This,ppssd)	\
    (This)->lpVtbl -> GetDocument(This,ppssd)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IDebugDocumentProvider_GetDocument_Proxy( 
    IDebugDocumentProvider * This,
    /* [out] */ IDebugDocument **ppssd);


void __RPC_STUB IDebugDocumentProvider_GetDocument_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IDebugDocumentProvider_INTERFACE_DEFINED__ */


#ifndef __IDebugDocument_INTERFACE_DEFINED__
#define __IDebugDocument_INTERFACE_DEFINED__

/* interface IDebugDocument */
/* [unique][uuid][object] */ 


EXTERN_C const IID IID_IDebugDocument;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("51973C21-CB0C-11d0-B5C9-00A0244A0E7A")
    IDebugDocument : public IDebugDocumentInfo
    {
    public:
    };
    
#else 	/* C style interface */

    typedef struct IDebugDocumentVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDebugDocument * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDebugDocument * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDebugDocument * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetName )( 
            IDebugDocument * This,
            /* [in] */ DOCUMENTNAMETYPE dnt,
            /* [out] */ BSTR *pbstrName);
        
        HRESULT ( STDMETHODCALLTYPE *GetDocumentClassId )( 
            IDebugDocument * This,
            /* [out] */ CLSID *pclsidDocument);
        
        END_INTERFACE
    } IDebugDocumentVtbl;

    interface IDebugDocument
    {
        CONST_VTBL struct IDebugDocumentVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDebugDocument_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDebugDocument_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDebugDocument_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDebugDocument_GetName(This,dnt,pbstrName)	\
    (This)->lpVtbl -> GetName(This,dnt,pbstrName)

#define IDebugDocument_GetDocumentClassId(This,pclsidDocument)	\
    (This)->lpVtbl -> GetDocumentClassId(This,pclsidDocument)


#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IDebugDocument_INTERFACE_DEFINED__ */


#ifndef __IDebugDocumentText_INTERFACE_DEFINED__
#define __IDebugDocumentText_INTERFACE_DEFINED__

/* interface IDebugDocumentText */
/* [unique][uuid][object] */ 


EXTERN_C const IID IID_IDebugDocumentText;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("51973C22-CB0C-11d0-B5C9-00A0244A0E7A")
    IDebugDocumentText : public IDebugDocument
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetDocumentAttributes( 
            /* [out] */ TEXT_DOC_ATTR *ptextdocattr) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSize( 
            /* [out] */ ULONG *pcNumLines,
            /* [out] */ ULONG *pcNumChars) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetPositionOfLine( 
            /* [in] */ ULONG cLineNumber,
            /* [out] */ ULONG *pcCharacterPosition) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetLineOfPosition( 
            /* [in] */ ULONG cCharacterPosition,
            /* [out] */ ULONG *pcLineNumber,
            /* [out] */ ULONG *pcCharacterOffsetInLine) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetText( 
            /* [in] */ ULONG cCharacterPosition,
            /* [size_is][length_is][out][in] */ WCHAR *pcharText,
            /* [full][size_is][length_is][out][in] */ SOURCE_TEXT_ATTR *pstaTextAttr,
            /* [out][in] */ ULONG *pcNumChars,
            /* [in] */ ULONG cMaxChars) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetPositionOfContext( 
            /* [in] */ IDebugDocumentContext *psc,
            /* [out] */ ULONG *pcCharacterPosition,
            /* [out] */ ULONG *cNumChars) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetContextOfPosition( 
            /* [in] */ ULONG cCharacterPosition,
            /* [in] */ ULONG cNumChars,
            /* [out] */ IDebugDocumentContext **ppsc) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDebugDocumentTextVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDebugDocumentText * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDebugDocumentText * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDebugDocumentText * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetName )( 
            IDebugDocumentText * This,
            /* [in] */ DOCUMENTNAMETYPE dnt,
            /* [out] */ BSTR *pbstrName);
        
        HRESULT ( STDMETHODCALLTYPE *GetDocumentClassId )( 
            IDebugDocumentText * This,
            /* [out] */ CLSID *pclsidDocument);
        
        HRESULT ( STDMETHODCALLTYPE *GetDocumentAttributes )( 
            IDebugDocumentText * This,
            /* [out] */ TEXT_DOC_ATTR *ptextdocattr);
        
        HRESULT ( STDMETHODCALLTYPE *GetSize )( 
            IDebugDocumentText * This,
            /* [out] */ ULONG *pcNumLines,
            /* [out] */ ULONG *pcNumChars);
        
        HRESULT ( STDMETHODCALLTYPE *GetPositionOfLine )( 
            IDebugDocumentText * This,
            /* [in] */ ULONG cLineNumber,
            /* [out] */ ULONG *pcCharacterPosition);
        
        HRESULT ( STDMETHODCALLTYPE *GetLineOfPosition )( 
            IDebugDocumentText * This,
            /* [in] */ ULONG cCharacterPosition,
            /* [out] */ ULONG *pcLineNumber,
            /* [out] */ ULONG *pcCharacterOffsetInLine);
        
        HRESULT ( STDMETHODCALLTYPE *GetText )( 
            IDebugDocumentText * This,
            /* [in] */ ULONG cCharacterPosition,
            /* [size_is][length_is][out][in] */ WCHAR *pcharText,
            /* [full][size_is][length_is][out][in] */ SOURCE_TEXT_ATTR *pstaTextAttr,
            /* [out][in] */ ULONG *pcNumChars,
            /* [in] */ ULONG cMaxChars);
        
        HRESULT ( STDMETHODCALLTYPE *GetPositionOfContext )( 
            IDebugDocumentText * This,
            /* [in] */ IDebugDocumentContext *psc,
            /* [out] */ ULONG *pcCharacterPosition,
            /* [out] */ ULONG *cNumChars);
        
        HRESULT ( STDMETHODCALLTYPE *GetContextOfPosition )( 
            IDebugDocumentText * This,
            /* [in] */ ULONG cCharacterPosition,
            /* [in] */ ULONG cNumChars,
            /* [out] */ IDebugDocumentContext **ppsc);
        
        END_INTERFACE
    } IDebugDocumentTextVtbl;

    interface IDebugDocumentText
    {
        CONST_VTBL struct IDebugDocumentTextVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDebugDocumentText_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDebugDocumentText_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDebugDocumentText_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDebugDocumentText_GetName(This,dnt,pbstrName)	\
    (This)->lpVtbl -> GetName(This,dnt,pbstrName)

#define IDebugDocumentText_GetDocumentClassId(This,pclsidDocument)	\
    (This)->lpVtbl -> GetDocumentClassId(This,pclsidDocument)



#define IDebugDocumentText_GetDocumentAttributes(This,ptextdocattr)	\
    (This)->lpVtbl -> GetDocumentAttributes(This,ptextdocattr)

#define IDebugDocumentText_GetSize(This,pcNumLines,pcNumChars)	\
    (This)->lpVtbl -> GetSize(This,pcNumLines,pcNumChars)

#define IDebugDocumentText_GetPositionOfLine(This,cLineNumber,pcCharacterPosition)	\
    (This)->lpVtbl -> GetPositionOfLine(This,cLineNumber,pcCharacterPosition)

#define IDebugDocumentText_GetLineOfPosition(This,cCharacterPosition,pcLineNumber,pcCharacterOffsetInLine)	\
    (This)->lpVtbl -> GetLineOfPosition(This,cCharacterPosition,pcLineNumber,pcCharacterOffsetInLine)

#define IDebugDocumentText_GetText(This,cCharacterPosition,pcharText,pstaTextAttr,pcNumChars,cMaxChars)	\
    (This)->lpVtbl -> GetText(This,cCharacterPosition,pcharText,pstaTextAttr,pcNumChars,cMaxChars)

#define IDebugDocumentText_GetPositionOfContext(This,psc,pcCharacterPosition,cNumChars)	\
    (This)->lpVtbl -> GetPositionOfContext(This,psc,pcCharacterPosition,cNumChars)

#define IDebugDocumentText_GetContextOfPosition(This,cCharacterPosition,cNumChars,ppsc)	\
    (This)->lpVtbl -> GetContextOfPosition(This,cCharacterPosition,cNumChars,ppsc)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IDebugDocumentText_GetDocumentAttributes_Proxy( 
    IDebugDocumentText * This,
    /* [out] */ TEXT_DOC_ATTR *ptextdocattr);


void __RPC_STUB IDebugDocumentText_GetDocumentAttributes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugDocumentText_GetSize_Proxy( 
    IDebugDocumentText * This,
    /* [out] */ ULONG *pcNumLines,
    /* [out] */ ULONG *pcNumChars);


void __RPC_STUB IDebugDocumentText_GetSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugDocumentText_GetPositionOfLine_Proxy( 
    IDebugDocumentText * This,
    /* [in] */ ULONG cLineNumber,
    /* [out] */ ULONG *pcCharacterPosition);


void __RPC_STUB IDebugDocumentText_GetPositionOfLine_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugDocumentText_GetLineOfPosition_Proxy( 
    IDebugDocumentText * This,
    /* [in] */ ULONG cCharacterPosition,
    /* [out] */ ULONG *pcLineNumber,
    /* [out] */ ULONG *pcCharacterOffsetInLine);


void __RPC_STUB IDebugDocumentText_GetLineOfPosition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugDocumentText_GetText_Proxy( 
    IDebugDocumentText * This,
    /* [in] */ ULONG cCharacterPosition,
    /* [size_is][length_is][out][in] */ WCHAR *pcharText,
    /* [full][size_is][length_is][out][in] */ SOURCE_TEXT_ATTR *pstaTextAttr,
    /* [out][in] */ ULONG *pcNumChars,
    /* [in] */ ULONG cMaxChars);


void __RPC_STUB IDebugDocumentText_GetText_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugDocumentText_GetPositionOfContext_Proxy( 
    IDebugDocumentText * This,
    /* [in] */ IDebugDocumentContext *psc,
    /* [out] */ ULONG *pcCharacterPosition,
    /* [out] */ ULONG *cNumChars);


void __RPC_STUB IDebugDocumentText_GetPositionOfContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugDocumentText_GetContextOfPosition_Proxy( 
    IDebugDocumentText * This,
    /* [in] */ ULONG cCharacterPosition,
    /* [in] */ ULONG cNumChars,
    /* [out] */ IDebugDocumentContext **ppsc);


void __RPC_STUB IDebugDocumentText_GetContextOfPosition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IDebugDocumentText_INTERFACE_DEFINED__ */


#ifndef __IDebugDocumentTextEvents_INTERFACE_DEFINED__
#define __IDebugDocumentTextEvents_INTERFACE_DEFINED__

/* interface IDebugDocumentTextEvents */
/* [unique][uuid][object] */ 


EXTERN_C const IID IID_IDebugDocumentTextEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("51973C23-CB0C-11d0-B5C9-00A0244A0E7A")
    IDebugDocumentTextEvents : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE onDestroy( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE onInsertText( 
            /* [in] */ ULONG cCharacterPosition,
            /* [in] */ ULONG cNumToInsert) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE onRemoveText( 
            /* [in] */ ULONG cCharacterPosition,
            /* [in] */ ULONG cNumToRemove) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE onReplaceText( 
            /* [in] */ ULONG cCharacterPosition,
            /* [in] */ ULONG cNumToReplace) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE onUpdateTextAttributes( 
            /* [in] */ ULONG cCharacterPosition,
            /* [in] */ ULONG cNumToUpdate) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE onUpdateDocumentAttributes( 
            /* [in] */ TEXT_DOC_ATTR textdocattr) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDebugDocumentTextEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDebugDocumentTextEvents * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDebugDocumentTextEvents * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDebugDocumentTextEvents * This);
        
        HRESULT ( STDMETHODCALLTYPE *onDestroy )( 
            IDebugDocumentTextEvents * This);
        
        HRESULT ( STDMETHODCALLTYPE *onInsertText )( 
            IDebugDocumentTextEvents * This,
            /* [in] */ ULONG cCharacterPosition,
            /* [in] */ ULONG cNumToInsert);
        
        HRESULT ( STDMETHODCALLTYPE *onRemoveText )( 
            IDebugDocumentTextEvents * This,
            /* [in] */ ULONG cCharacterPosition,
            /* [in] */ ULONG cNumToRemove);
        
        HRESULT ( STDMETHODCALLTYPE *onReplaceText )( 
            IDebugDocumentTextEvents * This,
            /* [in] */ ULONG cCharacterPosition,
            /* [in] */ ULONG cNumToReplace);
        
        HRESULT ( STDMETHODCALLTYPE *onUpdateTextAttributes )( 
            IDebugDocumentTextEvents * This,
            /* [in] */ ULONG cCharacterPosition,
            /* [in] */ ULONG cNumToUpdate);
        
        HRESULT ( STDMETHODCALLTYPE *onUpdateDocumentAttributes )( 
            IDebugDocumentTextEvents * This,
            /* [in] */ TEXT_DOC_ATTR textdocattr);
        
        END_INTERFACE
    } IDebugDocumentTextEventsVtbl;

    interface IDebugDocumentTextEvents
    {
        CONST_VTBL struct IDebugDocumentTextEventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDebugDocumentTextEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDebugDocumentTextEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDebugDocumentTextEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDebugDocumentTextEvents_onDestroy(This)	\
    (This)->lpVtbl -> onDestroy(This)

#define IDebugDocumentTextEvents_onInsertText(This,cCharacterPosition,cNumToInsert)	\
    (This)->lpVtbl -> onInsertText(This,cCharacterPosition,cNumToInsert)

#define IDebugDocumentTextEvents_onRemoveText(This,cCharacterPosition,cNumToRemove)	\
    (This)->lpVtbl -> onRemoveText(This,cCharacterPosition,cNumToRemove)

#define IDebugDocumentTextEvents_onReplaceText(This,cCharacterPosition,cNumToReplace)	\
    (This)->lpVtbl -> onReplaceText(This,cCharacterPosition,cNumToReplace)

#define IDebugDocumentTextEvents_onUpdateTextAttributes(This,cCharacterPosition,cNumToUpdate)	\
    (This)->lpVtbl -> onUpdateTextAttributes(This,cCharacterPosition,cNumToUpdate)

#define IDebugDocumentTextEvents_onUpdateDocumentAttributes(This,textdocattr)	\
    (This)->lpVtbl -> onUpdateDocumentAttributes(This,textdocattr)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IDebugDocumentTextEvents_onDestroy_Proxy( 
    IDebugDocumentTextEvents * This);


void __RPC_STUB IDebugDocumentTextEvents_onDestroy_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugDocumentTextEvents_onInsertText_Proxy( 
    IDebugDocumentTextEvents * This,
    /* [in] */ ULONG cCharacterPosition,
    /* [in] */ ULONG cNumToInsert);


void __RPC_STUB IDebugDocumentTextEvents_onInsertText_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugDocumentTextEvents_onRemoveText_Proxy( 
    IDebugDocumentTextEvents * This,
    /* [in] */ ULONG cCharacterPosition,
    /* [in] */ ULONG cNumToRemove);


void __RPC_STUB IDebugDocumentTextEvents_onRemoveText_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugDocumentTextEvents_onReplaceText_Proxy( 
    IDebugDocumentTextEvents * This,
    /* [in] */ ULONG cCharacterPosition,
    /* [in] */ ULONG cNumToReplace);


void __RPC_STUB IDebugDocumentTextEvents_onReplaceText_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugDocumentTextEvents_onUpdateTextAttributes_Proxy( 
    IDebugDocumentTextEvents * This,
    /* [in] */ ULONG cCharacterPosition,
    /* [in] */ ULONG cNumToUpdate);


void __RPC_STUB IDebugDocumentTextEvents_onUpdateTextAttributes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugDocumentTextEvents_onUpdateDocumentAttributes_Proxy( 
    IDebugDocumentTextEvents * This,
    /* [in] */ TEXT_DOC_ATTR textdocattr);


void __RPC_STUB IDebugDocumentTextEvents_onUpdateDocumentAttributes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IDebugDocumentTextEvents_INTERFACE_DEFINED__ */


#ifndef __IDebugDocumentTextAuthor_INTERFACE_DEFINED__
#define __IDebugDocumentTextAuthor_INTERFACE_DEFINED__

/* interface IDebugDocumentTextAuthor */
/* [unique][uuid][object] */ 


EXTERN_C const IID IID_IDebugDocumentTextAuthor;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("51973C24-CB0C-11d0-B5C9-00A0244A0E7A")
    IDebugDocumentTextAuthor : public IDebugDocumentText
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE InsertText( 
            /* [in] */ ULONG cCharacterPosition,
            /* [in] */ ULONG cNumToInsert,
            /* [size_is][in] */ OLECHAR pcharText[  ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RemoveText( 
            /* [in] */ ULONG cCharacterPosition,
            /* [in] */ ULONG cNumToRemove) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ReplaceText( 
            /* [in] */ ULONG cCharacterPosition,
            /* [in] */ ULONG cNumToReplace,
            /* [size_is][in] */ OLECHAR pcharText[  ]) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDebugDocumentTextAuthorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDebugDocumentTextAuthor * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDebugDocumentTextAuthor * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDebugDocumentTextAuthor * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetName )( 
            IDebugDocumentTextAuthor * This,
            /* [in] */ DOCUMENTNAMETYPE dnt,
            /* [out] */ BSTR *pbstrName);
        
        HRESULT ( STDMETHODCALLTYPE *GetDocumentClassId )( 
            IDebugDocumentTextAuthor * This,
            /* [out] */ CLSID *pclsidDocument);
        
        HRESULT ( STDMETHODCALLTYPE *GetDocumentAttributes )( 
            IDebugDocumentTextAuthor * This,
            /* [out] */ TEXT_DOC_ATTR *ptextdocattr);
        
        HRESULT ( STDMETHODCALLTYPE *GetSize )( 
            IDebugDocumentTextAuthor * This,
            /* [out] */ ULONG *pcNumLines,
            /* [out] */ ULONG *pcNumChars);
        
        HRESULT ( STDMETHODCALLTYPE *GetPositionOfLine )( 
            IDebugDocumentTextAuthor * This,
            /* [in] */ ULONG cLineNumber,
            /* [out] */ ULONG *pcCharacterPosition);
        
        HRESULT ( STDMETHODCALLTYPE *GetLineOfPosition )( 
            IDebugDocumentTextAuthor * This,
            /* [in] */ ULONG cCharacterPosition,
            /* [out] */ ULONG *pcLineNumber,
            /* [out] */ ULONG *pcCharacterOffsetInLine);
        
        HRESULT ( STDMETHODCALLTYPE *GetText )( 
            IDebugDocumentTextAuthor * This,
            /* [in] */ ULONG cCharacterPosition,
            /* [size_is][length_is][out][in] */ WCHAR *pcharText,
            /* [full][size_is][length_is][out][in] */ SOURCE_TEXT_ATTR *pstaTextAttr,
            /* [out][in] */ ULONG *pcNumChars,
            /* [in] */ ULONG cMaxChars);
        
        HRESULT ( STDMETHODCALLTYPE *GetPositionOfContext )( 
            IDebugDocumentTextAuthor * This,
            /* [in] */ IDebugDocumentContext *psc,
            /* [out] */ ULONG *pcCharacterPosition,
            /* [out] */ ULONG *cNumChars);
        
        HRESULT ( STDMETHODCALLTYPE *GetContextOfPosition )( 
            IDebugDocumentTextAuthor * This,
            /* [in] */ ULONG cCharacterPosition,
            /* [in] */ ULONG cNumChars,
            /* [out] */ IDebugDocumentContext **ppsc);
        
        HRESULT ( STDMETHODCALLTYPE *InsertText )( 
            IDebugDocumentTextAuthor * This,
            /* [in] */ ULONG cCharacterPosition,
            /* [in] */ ULONG cNumToInsert,
            /* [size_is][in] */ OLECHAR pcharText[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *RemoveText )( 
            IDebugDocumentTextAuthor * This,
            /* [in] */ ULONG cCharacterPosition,
            /* [in] */ ULONG cNumToRemove);
        
        HRESULT ( STDMETHODCALLTYPE *ReplaceText )( 
            IDebugDocumentTextAuthor * This,
            /* [in] */ ULONG cCharacterPosition,
            /* [in] */ ULONG cNumToReplace,
            /* [size_is][in] */ OLECHAR pcharText[  ]);
        
        END_INTERFACE
    } IDebugDocumentTextAuthorVtbl;

    interface IDebugDocumentTextAuthor
    {
        CONST_VTBL struct IDebugDocumentTextAuthorVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDebugDocumentTextAuthor_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDebugDocumentTextAuthor_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDebugDocumentTextAuthor_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDebugDocumentTextAuthor_GetName(This,dnt,pbstrName)	\
    (This)->lpVtbl -> GetName(This,dnt,pbstrName)

#define IDebugDocumentTextAuthor_GetDocumentClassId(This,pclsidDocument)	\
    (This)->lpVtbl -> GetDocumentClassId(This,pclsidDocument)



#define IDebugDocumentTextAuthor_GetDocumentAttributes(This,ptextdocattr)	\
    (This)->lpVtbl -> GetDocumentAttributes(This,ptextdocattr)

#define IDebugDocumentTextAuthor_GetSize(This,pcNumLines,pcNumChars)	\
    (This)->lpVtbl -> GetSize(This,pcNumLines,pcNumChars)

#define IDebugDocumentTextAuthor_GetPositionOfLine(This,cLineNumber,pcCharacterPosition)	\
    (This)->lpVtbl -> GetPositionOfLine(This,cLineNumber,pcCharacterPosition)

#define IDebugDocumentTextAuthor_GetLineOfPosition(This,cCharacterPosition,pcLineNumber,pcCharacterOffsetInLine)	\
    (This)->lpVtbl -> GetLineOfPosition(This,cCharacterPosition,pcLineNumber,pcCharacterOffsetInLine)

#define IDebugDocumentTextAuthor_GetText(This,cCharacterPosition,pcharText,pstaTextAttr,pcNumChars,cMaxChars)	\
    (This)->lpVtbl -> GetText(This,cCharacterPosition,pcharText,pstaTextAttr,pcNumChars,cMaxChars)

#define IDebugDocumentTextAuthor_GetPositionOfContext(This,psc,pcCharacterPosition,cNumChars)	\
    (This)->lpVtbl -> GetPositionOfContext(This,psc,pcCharacterPosition,cNumChars)

#define IDebugDocumentTextAuthor_GetContextOfPosition(This,cCharacterPosition,cNumChars,ppsc)	\
    (This)->lpVtbl -> GetContextOfPosition(This,cCharacterPosition,cNumChars,ppsc)


#define IDebugDocumentTextAuthor_InsertText(This,cCharacterPosition,cNumToInsert,pcharText)	\
    (This)->lpVtbl -> InsertText(This,cCharacterPosition,cNumToInsert,pcharText)

#define IDebugDocumentTextAuthor_RemoveText(This,cCharacterPosition,cNumToRemove)	\
    (This)->lpVtbl -> RemoveText(This,cCharacterPosition,cNumToRemove)

#define IDebugDocumentTextAuthor_ReplaceText(This,cCharacterPosition,cNumToReplace,pcharText)	\
    (This)->lpVtbl -> ReplaceText(This,cCharacterPosition,cNumToReplace,pcharText)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IDebugDocumentTextAuthor_InsertText_Proxy( 
    IDebugDocumentTextAuthor * This,
    /* [in] */ ULONG cCharacterPosition,
    /* [in] */ ULONG cNumToInsert,
    /* [size_is][in] */ OLECHAR pcharText[  ]);


void __RPC_STUB IDebugDocumentTextAuthor_InsertText_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugDocumentTextAuthor_RemoveText_Proxy( 
    IDebugDocumentTextAuthor * This,
    /* [in] */ ULONG cCharacterPosition,
    /* [in] */ ULONG cNumToRemove);


void __RPC_STUB IDebugDocumentTextAuthor_RemoveText_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugDocumentTextAuthor_ReplaceText_Proxy( 
    IDebugDocumentTextAuthor * This,
    /* [in] */ ULONG cCharacterPosition,
    /* [in] */ ULONG cNumToReplace,
    /* [size_is][in] */ OLECHAR pcharText[  ]);


void __RPC_STUB IDebugDocumentTextAuthor_ReplaceText_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IDebugDocumentTextAuthor_INTERFACE_DEFINED__ */


#ifndef __IDebugDocumentTextExternalAuthor_INTERFACE_DEFINED__
#define __IDebugDocumentTextExternalAuthor_INTERFACE_DEFINED__

/* interface IDebugDocumentTextExternalAuthor */
/* [unique][uuid][object] */ 


EXTERN_C const IID IID_IDebugDocumentTextExternalAuthor;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("51973C25-CB0C-11d0-B5C9-00A0244A0E7A")
    IDebugDocumentTextExternalAuthor : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetPathName( 
            /* [out] */ BSTR *pbstrLongName,
            /* [out] */ BOOL *pfIsOriginalFile) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetFileName( 
            /* [out] */ BSTR *pbstrShortName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE NotifyChanged( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDebugDocumentTextExternalAuthorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDebugDocumentTextExternalAuthor * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDebugDocumentTextExternalAuthor * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDebugDocumentTextExternalAuthor * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetPathName )( 
            IDebugDocumentTextExternalAuthor * This,
            /* [out] */ BSTR *pbstrLongName,
            /* [out] */ BOOL *pfIsOriginalFile);
        
        HRESULT ( STDMETHODCALLTYPE *GetFileName )( 
            IDebugDocumentTextExternalAuthor * This,
            /* [out] */ BSTR *pbstrShortName);
        
        HRESULT ( STDMETHODCALLTYPE *NotifyChanged )( 
            IDebugDocumentTextExternalAuthor * This);
        
        END_INTERFACE
    } IDebugDocumentTextExternalAuthorVtbl;

    interface IDebugDocumentTextExternalAuthor
    {
        CONST_VTBL struct IDebugDocumentTextExternalAuthorVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDebugDocumentTextExternalAuthor_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDebugDocumentTextExternalAuthor_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDebugDocumentTextExternalAuthor_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDebugDocumentTextExternalAuthor_GetPathName(This,pbstrLongName,pfIsOriginalFile)	\
    (This)->lpVtbl -> GetPathName(This,pbstrLongName,pfIsOriginalFile)

#define IDebugDocumentTextExternalAuthor_GetFileName(This,pbstrShortName)	\
    (This)->lpVtbl -> GetFileName(This,pbstrShortName)

#define IDebugDocumentTextExternalAuthor_NotifyChanged(This)	\
    (This)->lpVtbl -> NotifyChanged(This)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IDebugDocumentTextExternalAuthor_GetPathName_Proxy( 
    IDebugDocumentTextExternalAuthor * This,
    /* [out] */ BSTR *pbstrLongName,
    /* [out] */ BOOL *pfIsOriginalFile);


void __RPC_STUB IDebugDocumentTextExternalAuthor_GetPathName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugDocumentTextExternalAuthor_GetFileName_Proxy( 
    IDebugDocumentTextExternalAuthor * This,
    /* [out] */ BSTR *pbstrShortName);


void __RPC_STUB IDebugDocumentTextExternalAuthor_GetFileName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugDocumentTextExternalAuthor_NotifyChanged_Proxy( 
    IDebugDocumentTextExternalAuthor * This);


void __RPC_STUB IDebugDocumentTextExternalAuthor_NotifyChanged_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IDebugDocumentTextExternalAuthor_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_activdbg_0329 */
/* [local] */ 

#ifndef DISABLE_ACTIVDBG_INTERFACE_WRAPPERS
#ifdef _WIN64
#define IDebugDocumentHelper IDebugDocumentHelper64
#define IID_IDebugDocumentHelper IID_IDebugDocumentHelper64
#else
#define IDebugDocumentHelper IDebugDocumentHelper32
#define IID_IDebugDocumentHelper IID_IDebugDocumentHelper32
#endif
#endif


extern RPC_IF_HANDLE __MIDL_itf_activdbg_0329_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_activdbg_0329_v0_0_s_ifspec;

#ifndef __IDebugDocumentHelper32_INTERFACE_DEFINED__
#define __IDebugDocumentHelper32_INTERFACE_DEFINED__

/* interface IDebugDocumentHelper32 */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IDebugDocumentHelper32;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("51973C26-CB0C-11d0-B5C9-00A0244A0E7A")
    IDebugDocumentHelper32 : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Init( 
            /* [in] */ IDebugApplication32 *pda,
            /* [string][in] */ LPCOLESTR pszShortName,
            /* [string][in] */ LPCOLESTR pszLongName,
            /* [in] */ TEXT_DOC_ATTR docAttr) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Attach( 
            /* [in] */ IDebugDocumentHelper32 *pddhParent) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Detach( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddUnicodeText( 
            /* [string][in] */ LPCOLESTR pszText) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddDBCSText( 
            /* [string][in] */ LPCSTR pszText) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetDebugDocumentHost( 
            /* [in] */ IDebugDocumentHost *pddh) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddDeferredText( 
            /* [in] */ ULONG cChars,
            /* [in] */ DWORD dwTextStartCookie) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DefineScriptBlock( 
            /* [in] */ ULONG ulCharOffset,
            /* [in] */ ULONG cChars,
            /* [in] */ IActiveScript *pas,
            /* [in] */ BOOL fScriptlet,
            /* [out] */ DWORD *pdwSourceContext) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetDefaultTextAttr( 
            SOURCE_TEXT_ATTR staTextAttr) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetTextAttributes( 
            /* [in] */ ULONG ulCharOffset,
            /* [in] */ ULONG cChars,
            /* [size_is][length_is][in] */ SOURCE_TEXT_ATTR *pstaTextAttr) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetLongName( 
            /* [string][in] */ LPCOLESTR pszLongName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetShortName( 
            /* [string][in] */ LPCOLESTR pszShortName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetDocumentAttr( 
            /* [in] */ TEXT_DOC_ATTR pszAttributes) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDebugApplicationNode( 
            /* [out] */ IDebugApplicationNode **ppdan) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetScriptBlockInfo( 
            /* [in] */ DWORD dwSourceContext,
            /* [out] */ IActiveScript **ppasd,
            /* [out] */ ULONG *piCharPos,
            /* [out] */ ULONG *pcChars) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateDebugDocumentContext( 
            /* [in] */ ULONG iCharPos,
            /* [in] */ ULONG cChars,
            /* [out] */ IDebugDocumentContext **ppddc) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE BringDocumentToTop( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE BringDocumentContextToTop( 
            IDebugDocumentContext *pddc) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDebugDocumentHelper32Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDebugDocumentHelper32 * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDebugDocumentHelper32 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDebugDocumentHelper32 * This);
        
        HRESULT ( STDMETHODCALLTYPE *Init )( 
            IDebugDocumentHelper32 * This,
            /* [in] */ IDebugApplication32 *pda,
            /* [string][in] */ LPCOLESTR pszShortName,
            /* [string][in] */ LPCOLESTR pszLongName,
            /* [in] */ TEXT_DOC_ATTR docAttr);
        
        HRESULT ( STDMETHODCALLTYPE *Attach )( 
            IDebugDocumentHelper32 * This,
            /* [in] */ IDebugDocumentHelper32 *pddhParent);
        
        HRESULT ( STDMETHODCALLTYPE *Detach )( 
            IDebugDocumentHelper32 * This);
        
        HRESULT ( STDMETHODCALLTYPE *AddUnicodeText )( 
            IDebugDocumentHelper32 * This,
            /* [string][in] */ LPCOLESTR pszText);
        
        HRESULT ( STDMETHODCALLTYPE *AddDBCSText )( 
            IDebugDocumentHelper32 * This,
            /* [string][in] */ LPCSTR pszText);
        
        HRESULT ( STDMETHODCALLTYPE *SetDebugDocumentHost )( 
            IDebugDocumentHelper32 * This,
            /* [in] */ IDebugDocumentHost *pddh);
        
        HRESULT ( STDMETHODCALLTYPE *AddDeferredText )( 
            IDebugDocumentHelper32 * This,
            /* [in] */ ULONG cChars,
            /* [in] */ DWORD dwTextStartCookie);
        
        HRESULT ( STDMETHODCALLTYPE *DefineScriptBlock )( 
            IDebugDocumentHelper32 * This,
            /* [in] */ ULONG ulCharOffset,
            /* [in] */ ULONG cChars,
            /* [in] */ IActiveScript *pas,
            /* [in] */ BOOL fScriptlet,
            /* [out] */ DWORD *pdwSourceContext);
        
        HRESULT ( STDMETHODCALLTYPE *SetDefaultTextAttr )( 
            IDebugDocumentHelper32 * This,
            SOURCE_TEXT_ATTR staTextAttr);
        
        HRESULT ( STDMETHODCALLTYPE *SetTextAttributes )( 
            IDebugDocumentHelper32 * This,
            /* [in] */ ULONG ulCharOffset,
            /* [in] */ ULONG cChars,
            /* [size_is][length_is][in] */ SOURCE_TEXT_ATTR *pstaTextAttr);
        
        HRESULT ( STDMETHODCALLTYPE *SetLongName )( 
            IDebugDocumentHelper32 * This,
            /* [string][in] */ LPCOLESTR pszLongName);
        
        HRESULT ( STDMETHODCALLTYPE *SetShortName )( 
            IDebugDocumentHelper32 * This,
            /* [string][in] */ LPCOLESTR pszShortName);
        
        HRESULT ( STDMETHODCALLTYPE *SetDocumentAttr )( 
            IDebugDocumentHelper32 * This,
            /* [in] */ TEXT_DOC_ATTR pszAttributes);
        
        HRESULT ( STDMETHODCALLTYPE *GetDebugApplicationNode )( 
            IDebugDocumentHelper32 * This,
            /* [out] */ IDebugApplicationNode **ppdan);
        
        HRESULT ( STDMETHODCALLTYPE *GetScriptBlockInfo )( 
            IDebugDocumentHelper32 * This,
            /* [in] */ DWORD dwSourceContext,
            /* [out] */ IActiveScript **ppasd,
            /* [out] */ ULONG *piCharPos,
            /* [out] */ ULONG *pcChars);
        
        HRESULT ( STDMETHODCALLTYPE *CreateDebugDocumentContext )( 
            IDebugDocumentHelper32 * This,
            /* [in] */ ULONG iCharPos,
            /* [in] */ ULONG cChars,
            /* [out] */ IDebugDocumentContext **ppddc);
        
        HRESULT ( STDMETHODCALLTYPE *BringDocumentToTop )( 
            IDebugDocumentHelper32 * This);
        
        HRESULT ( STDMETHODCALLTYPE *BringDocumentContextToTop )( 
            IDebugDocumentHelper32 * This,
            IDebugDocumentContext *pddc);
        
        END_INTERFACE
    } IDebugDocumentHelper32Vtbl;

    interface IDebugDocumentHelper32
    {
        CONST_VTBL struct IDebugDocumentHelper32Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDebugDocumentHelper32_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDebugDocumentHelper32_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDebugDocumentHelper32_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDebugDocumentHelper32_Init(This,pda,pszShortName,pszLongName,docAttr)	\
    (This)->lpVtbl -> Init(This,pda,pszShortName,pszLongName,docAttr)

#define IDebugDocumentHelper32_Attach(This,pddhParent)	\
    (This)->lpVtbl -> Attach(This,pddhParent)

#define IDebugDocumentHelper32_Detach(This)	\
    (This)->lpVtbl -> Detach(This)

#define IDebugDocumentHelper32_AddUnicodeText(This,pszText)	\
    (This)->lpVtbl -> AddUnicodeText(This,pszText)

#define IDebugDocumentHelper32_AddDBCSText(This,pszText)	\
    (This)->lpVtbl -> AddDBCSText(This,pszText)

#define IDebugDocumentHelper32_SetDebugDocumentHost(This,pddh)	\
    (This)->lpVtbl -> SetDebugDocumentHost(This,pddh)

#define IDebugDocumentHelper32_AddDeferredText(This,cChars,dwTextStartCookie)	\
    (This)->lpVtbl -> AddDeferredText(This,cChars,dwTextStartCookie)

#define IDebugDocumentHelper32_DefineScriptBlock(This,ulCharOffset,cChars,pas,fScriptlet,pdwSourceContext)	\
    (This)->lpVtbl -> DefineScriptBlock(This,ulCharOffset,cChars,pas,fScriptlet,pdwSourceContext)

#define IDebugDocumentHelper32_SetDefaultTextAttr(This,staTextAttr)	\
    (This)->lpVtbl -> SetDefaultTextAttr(This,staTextAttr)

#define IDebugDocumentHelper32_SetTextAttributes(This,ulCharOffset,cChars,pstaTextAttr)	\
    (This)->lpVtbl -> SetTextAttributes(This,ulCharOffset,cChars,pstaTextAttr)

#define IDebugDocumentHelper32_SetLongName(This,pszLongName)	\
    (This)->lpVtbl -> SetLongName(This,pszLongName)

#define IDebugDocumentHelper32_SetShortName(This,pszShortName)	\
    (This)->lpVtbl -> SetShortName(This,pszShortName)

#define IDebugDocumentHelper32_SetDocumentAttr(This,pszAttributes)	\
    (This)->lpVtbl -> SetDocumentAttr(This,pszAttributes)

#define IDebugDocumentHelper32_GetDebugApplicationNode(This,ppdan)	\
    (This)->lpVtbl -> GetDebugApplicationNode(This,ppdan)

#define IDebugDocumentHelper32_GetScriptBlockInfo(This,dwSourceContext,ppasd,piCharPos,pcChars)	\
    (This)->lpVtbl -> GetScriptBlockInfo(This,dwSourceContext,ppasd,piCharPos,pcChars)

#define IDebugDocumentHelper32_CreateDebugDocumentContext(This,iCharPos,cChars,ppddc)	\
    (This)->lpVtbl -> CreateDebugDocumentContext(This,iCharPos,cChars,ppddc)

#define IDebugDocumentHelper32_BringDocumentToTop(This)	\
    (This)->lpVtbl -> BringDocumentToTop(This)

#define IDebugDocumentHelper32_BringDocumentContextToTop(This,pddc)	\
    (This)->lpVtbl -> BringDocumentContextToTop(This,pddc)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IDebugDocumentHelper32_Init_Proxy( 
    IDebugDocumentHelper32 * This,
    /* [in] */ IDebugApplication32 *pda,
    /* [string][in] */ LPCOLESTR pszShortName,
    /* [string][in] */ LPCOLESTR pszLongName,
    /* [in] */ TEXT_DOC_ATTR docAttr);


void __RPC_STUB IDebugDocumentHelper32_Init_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugDocumentHelper32_Attach_Proxy( 
    IDebugDocumentHelper32 * This,
    /* [in] */ IDebugDocumentHelper32 *pddhParent);


void __RPC_STUB IDebugDocumentHelper32_Attach_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugDocumentHelper32_Detach_Proxy( 
    IDebugDocumentHelper32 * This);


void __RPC_STUB IDebugDocumentHelper32_Detach_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugDocumentHelper32_AddUnicodeText_Proxy( 
    IDebugDocumentHelper32 * This,
    /* [string][in] */ LPCOLESTR pszText);


void __RPC_STUB IDebugDocumentHelper32_AddUnicodeText_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugDocumentHelper32_AddDBCSText_Proxy( 
    IDebugDocumentHelper32 * This,
    /* [string][in] */ LPCSTR pszText);


void __RPC_STUB IDebugDocumentHelper32_AddDBCSText_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugDocumentHelper32_SetDebugDocumentHost_Proxy( 
    IDebugDocumentHelper32 * This,
    /* [in] */ IDebugDocumentHost *pddh);


void __RPC_STUB IDebugDocumentHelper32_SetDebugDocumentHost_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugDocumentHelper32_AddDeferredText_Proxy( 
    IDebugDocumentHelper32 * This,
    /* [in] */ ULONG cChars,
    /* [in] */ DWORD dwTextStartCookie);


void __RPC_STUB IDebugDocumentHelper32_AddDeferredText_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugDocumentHelper32_DefineScriptBlock_Proxy( 
    IDebugDocumentHelper32 * This,
    /* [in] */ ULONG ulCharOffset,
    /* [in] */ ULONG cChars,
    /* [in] */ IActiveScript *pas,
    /* [in] */ BOOL fScriptlet,
    /* [out] */ DWORD *pdwSourceContext);


void __RPC_STUB IDebugDocumentHelper32_DefineScriptBlock_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugDocumentHelper32_SetDefaultTextAttr_Proxy( 
    IDebugDocumentHelper32 * This,
    SOURCE_TEXT_ATTR staTextAttr);


void __RPC_STUB IDebugDocumentHelper32_SetDefaultTextAttr_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugDocumentHelper32_SetTextAttributes_Proxy( 
    IDebugDocumentHelper32 * This,
    /* [in] */ ULONG ulCharOffset,
    /* [in] */ ULONG cChars,
    /* [size_is][length_is][in] */ SOURCE_TEXT_ATTR *pstaTextAttr);


void __RPC_STUB IDebugDocumentHelper32_SetTextAttributes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugDocumentHelper32_SetLongName_Proxy( 
    IDebugDocumentHelper32 * This,
    /* [string][in] */ LPCOLESTR pszLongName);


void __RPC_STUB IDebugDocumentHelper32_SetLongName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugDocumentHelper32_SetShortName_Proxy( 
    IDebugDocumentHelper32 * This,
    /* [string][in] */ LPCOLESTR pszShortName);


void __RPC_STUB IDebugDocumentHelper32_SetShortName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugDocumentHelper32_SetDocumentAttr_Proxy( 
    IDebugDocumentHelper32 * This,
    /* [in] */ TEXT_DOC_ATTR pszAttributes);


void __RPC_STUB IDebugDocumentHelper32_SetDocumentAttr_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugDocumentHelper32_GetDebugApplicationNode_Proxy( 
    IDebugDocumentHelper32 * This,
    /* [out] */ IDebugApplicationNode **ppdan);


void __RPC_STUB IDebugDocumentHelper32_GetDebugApplicationNode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugDocumentHelper32_GetScriptBlockInfo_Proxy( 
    IDebugDocumentHelper32 * This,
    /* [in] */ DWORD dwSourceContext,
    /* [out] */ IActiveScript **ppasd,
    /* [out] */ ULONG *piCharPos,
    /* [out] */ ULONG *pcChars);


void __RPC_STUB IDebugDocumentHelper32_GetScriptBlockInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugDocumentHelper32_CreateDebugDocumentContext_Proxy( 
    IDebugDocumentHelper32 * This,
    /* [in] */ ULONG iCharPos,
    /* [in] */ ULONG cChars,
    /* [out] */ IDebugDocumentContext **ppddc);


void __RPC_STUB IDebugDocumentHelper32_CreateDebugDocumentContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugDocumentHelper32_BringDocumentToTop_Proxy( 
    IDebugDocumentHelper32 * This);


void __RPC_STUB IDebugDocumentHelper32_BringDocumentToTop_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugDocumentHelper32_BringDocumentContextToTop_Proxy( 
    IDebugDocumentHelper32 * This,
    IDebugDocumentContext *pddc);


void __RPC_STUB IDebugDocumentHelper32_BringDocumentContextToTop_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IDebugDocumentHelper32_INTERFACE_DEFINED__ */


#ifndef __IDebugDocumentHelper64_INTERFACE_DEFINED__
#define __IDebugDocumentHelper64_INTERFACE_DEFINED__

/* interface IDebugDocumentHelper64 */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IDebugDocumentHelper64;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("c4c7363c-20fd-47f9-bd82-4855e0150871")
    IDebugDocumentHelper64 : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Init( 
            /* [in] */ IDebugApplication64 *pda,
            /* [string][in] */ LPCOLESTR pszShortName,
            /* [string][in] */ LPCOLESTR pszLongName,
            /* [in] */ TEXT_DOC_ATTR docAttr) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Attach( 
            /* [in] */ IDebugDocumentHelper64 *pddhParent) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Detach( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddUnicodeText( 
            /* [string][in] */ LPCOLESTR pszText) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddDBCSText( 
            /* [string][in] */ LPCSTR pszText) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetDebugDocumentHost( 
            /* [in] */ IDebugDocumentHost *pddh) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddDeferredText( 
            /* [in] */ ULONG cChars,
            /* [in] */ DWORD dwTextStartCookie) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DefineScriptBlock( 
            /* [in] */ ULONG ulCharOffset,
            /* [in] */ ULONG cChars,
            /* [in] */ IActiveScript *pas,
            /* [in] */ BOOL fScriptlet,
            /* [out] */ DWORDLONG *pdwSourceContext) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetDefaultTextAttr( 
            SOURCE_TEXT_ATTR staTextAttr) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetTextAttributes( 
            /* [in] */ ULONG ulCharOffset,
            /* [in] */ ULONG cChars,
            /* [size_is][length_is][in] */ SOURCE_TEXT_ATTR *pstaTextAttr) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetLongName( 
            /* [string][in] */ LPCOLESTR pszLongName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetShortName( 
            /* [string][in] */ LPCOLESTR pszShortName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetDocumentAttr( 
            /* [in] */ TEXT_DOC_ATTR pszAttributes) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDebugApplicationNode( 
            /* [out] */ IDebugApplicationNode **ppdan) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetScriptBlockInfo( 
            /* [in] */ DWORDLONG dwSourceContext,
            /* [out] */ IActiveScript **ppasd,
            /* [out] */ ULONG *piCharPos,
            /* [out] */ ULONG *pcChars) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateDebugDocumentContext( 
            /* [in] */ ULONG iCharPos,
            /* [in] */ ULONG cChars,
            /* [out] */ IDebugDocumentContext **ppddc) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE BringDocumentToTop( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE BringDocumentContextToTop( 
            IDebugDocumentContext *pddc) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDebugDocumentHelper64Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDebugDocumentHelper64 * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDebugDocumentHelper64 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDebugDocumentHelper64 * This);
        
        HRESULT ( STDMETHODCALLTYPE *Init )( 
            IDebugDocumentHelper64 * This,
            /* [in] */ IDebugApplication64 *pda,
            /* [string][in] */ LPCOLESTR pszShortName,
            /* [string][in] */ LPCOLESTR pszLongName,
            /* [in] */ TEXT_DOC_ATTR docAttr);
        
        HRESULT ( STDMETHODCALLTYPE *Attach )( 
            IDebugDocumentHelper64 * This,
            /* [in] */ IDebugDocumentHelper64 *pddhParent);
        
        HRESULT ( STDMETHODCALLTYPE *Detach )( 
            IDebugDocumentHelper64 * This);
        
        HRESULT ( STDMETHODCALLTYPE *AddUnicodeText )( 
            IDebugDocumentHelper64 * This,
            /* [string][in] */ LPCOLESTR pszText);
        
        HRESULT ( STDMETHODCALLTYPE *AddDBCSText )( 
            IDebugDocumentHelper64 * This,
            /* [string][in] */ LPCSTR pszText);
        
        HRESULT ( STDMETHODCALLTYPE *SetDebugDocumentHost )( 
            IDebugDocumentHelper64 * This,
            /* [in] */ IDebugDocumentHost *pddh);
        
        HRESULT ( STDMETHODCALLTYPE *AddDeferredText )( 
            IDebugDocumentHelper64 * This,
            /* [in] */ ULONG cChars,
            /* [in] */ DWORD dwTextStartCookie);
        
        HRESULT ( STDMETHODCALLTYPE *DefineScriptBlock )( 
            IDebugDocumentHelper64 * This,
            /* [in] */ ULONG ulCharOffset,
            /* [in] */ ULONG cChars,
            /* [in] */ IActiveScript *pas,
            /* [in] */ BOOL fScriptlet,
            /* [out] */ DWORDLONG *pdwSourceContext);
        
        HRESULT ( STDMETHODCALLTYPE *SetDefaultTextAttr )( 
            IDebugDocumentHelper64 * This,
            SOURCE_TEXT_ATTR staTextAttr);
        
        HRESULT ( STDMETHODCALLTYPE *SetTextAttributes )( 
            IDebugDocumentHelper64 * This,
            /* [in] */ ULONG ulCharOffset,
            /* [in] */ ULONG cChars,
            /* [size_is][length_is][in] */ SOURCE_TEXT_ATTR *pstaTextAttr);
        
        HRESULT ( STDMETHODCALLTYPE *SetLongName )( 
            IDebugDocumentHelper64 * This,
            /* [string][in] */ LPCOLESTR pszLongName);
        
        HRESULT ( STDMETHODCALLTYPE *SetShortName )( 
            IDebugDocumentHelper64 * This,
            /* [string][in] */ LPCOLESTR pszShortName);
        
        HRESULT ( STDMETHODCALLTYPE *SetDocumentAttr )( 
            IDebugDocumentHelper64 * This,
            /* [in] */ TEXT_DOC_ATTR pszAttributes);
        
        HRESULT ( STDMETHODCALLTYPE *GetDebugApplicationNode )( 
            IDebugDocumentHelper64 * This,
            /* [out] */ IDebugApplicationNode **ppdan);
        
        HRESULT ( STDMETHODCALLTYPE *GetScriptBlockInfo )( 
            IDebugDocumentHelper64 * This,
            /* [in] */ DWORDLONG dwSourceContext,
            /* [out] */ IActiveScript **ppasd,
            /* [out] */ ULONG *piCharPos,
            /* [out] */ ULONG *pcChars);
        
        HRESULT ( STDMETHODCALLTYPE *CreateDebugDocumentContext )( 
            IDebugDocumentHelper64 * This,
            /* [in] */ ULONG iCharPos,
            /* [in] */ ULONG cChars,
            /* [out] */ IDebugDocumentContext **ppddc);
        
        HRESULT ( STDMETHODCALLTYPE *BringDocumentToTop )( 
            IDebugDocumentHelper64 * This);
        
        HRESULT ( STDMETHODCALLTYPE *BringDocumentContextToTop )( 
            IDebugDocumentHelper64 * This,
            IDebugDocumentContext *pddc);
        
        END_INTERFACE
    } IDebugDocumentHelper64Vtbl;

    interface IDebugDocumentHelper64
    {
        CONST_VTBL struct IDebugDocumentHelper64Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDebugDocumentHelper64_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDebugDocumentHelper64_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDebugDocumentHelper64_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDebugDocumentHelper64_Init(This,pda,pszShortName,pszLongName,docAttr)	\
    (This)->lpVtbl -> Init(This,pda,pszShortName,pszLongName,docAttr)

#define IDebugDocumentHelper64_Attach(This,pddhParent)	\
    (This)->lpVtbl -> Attach(This,pddhParent)

#define IDebugDocumentHelper64_Detach(This)	\
    (This)->lpVtbl -> Detach(This)

#define IDebugDocumentHelper64_AddUnicodeText(This,pszText)	\
    (This)->lpVtbl -> AddUnicodeText(This,pszText)

#define IDebugDocumentHelper64_AddDBCSText(This,pszText)	\
    (This)->lpVtbl -> AddDBCSText(This,pszText)

#define IDebugDocumentHelper64_SetDebugDocumentHost(This,pddh)	\
    (This)->lpVtbl -> SetDebugDocumentHost(This,pddh)

#define IDebugDocumentHelper64_AddDeferredText(This,cChars,dwTextStartCookie)	\
    (This)->lpVtbl -> AddDeferredText(This,cChars,dwTextStartCookie)

#define IDebugDocumentHelper64_DefineScriptBlock(This,ulCharOffset,cChars,pas,fScriptlet,pdwSourceContext)	\
    (This)->lpVtbl -> DefineScriptBlock(This,ulCharOffset,cChars,pas,fScriptlet,pdwSourceContext)

#define IDebugDocumentHelper64_SetDefaultTextAttr(This,staTextAttr)	\
    (This)->lpVtbl -> SetDefaultTextAttr(This,staTextAttr)

#define IDebugDocumentHelper64_SetTextAttributes(This,ulCharOffset,cChars,pstaTextAttr)	\
    (This)->lpVtbl -> SetTextAttributes(This,ulCharOffset,cChars,pstaTextAttr)

#define IDebugDocumentHelper64_SetLongName(This,pszLongName)	\
    (This)->lpVtbl -> SetLongName(This,pszLongName)

#define IDebugDocumentHelper64_SetShortName(This,pszShortName)	\
    (This)->lpVtbl -> SetShortName(This,pszShortName)

#define IDebugDocumentHelper64_SetDocumentAttr(This,pszAttributes)	\
    (This)->lpVtbl -> SetDocumentAttr(This,pszAttributes)

#define IDebugDocumentHelper64_GetDebugApplicationNode(This,ppdan)	\
    (This)->lpVtbl -> GetDebugApplicationNode(This,ppdan)

#define IDebugDocumentHelper64_GetScriptBlockInfo(This,dwSourceContext,ppasd,piCharPos,pcChars)	\
    (This)->lpVtbl -> GetScriptBlockInfo(This,dwSourceContext,ppasd,piCharPos,pcChars)

#define IDebugDocumentHelper64_CreateDebugDocumentContext(This,iCharPos,cChars,ppddc)	\
    (This)->lpVtbl -> CreateDebugDocumentContext(This,iCharPos,cChars,ppddc)

#define IDebugDocumentHelper64_BringDocumentToTop(This)	\
    (This)->lpVtbl -> BringDocumentToTop(This)

#define IDebugDocumentHelper64_BringDocumentContextToTop(This,pddc)	\
    (This)->lpVtbl -> BringDocumentContextToTop(This,pddc)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IDebugDocumentHelper64_Init_Proxy( 
    IDebugDocumentHelper64 * This,
    /* [in] */ IDebugApplication64 *pda,
    /* [string][in] */ LPCOLESTR pszShortName,
    /* [string][in] */ LPCOLESTR pszLongName,
    /* [in] */ TEXT_DOC_ATTR docAttr);


void __RPC_STUB IDebugDocumentHelper64_Init_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugDocumentHelper64_Attach_Proxy( 
    IDebugDocumentHelper64 * This,
    /* [in] */ IDebugDocumentHelper64 *pddhParent);


void __RPC_STUB IDebugDocumentHelper64_Attach_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugDocumentHelper64_Detach_Proxy( 
    IDebugDocumentHelper64 * This);


void __RPC_STUB IDebugDocumentHelper64_Detach_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugDocumentHelper64_AddUnicodeText_Proxy( 
    IDebugDocumentHelper64 * This,
    /* [string][in] */ LPCOLESTR pszText);


void __RPC_STUB IDebugDocumentHelper64_AddUnicodeText_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugDocumentHelper64_AddDBCSText_Proxy( 
    IDebugDocumentHelper64 * This,
    /* [string][in] */ LPCSTR pszText);


void __RPC_STUB IDebugDocumentHelper64_AddDBCSText_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugDocumentHelper64_SetDebugDocumentHost_Proxy( 
    IDebugDocumentHelper64 * This,
    /* [in] */ IDebugDocumentHost *pddh);


void __RPC_STUB IDebugDocumentHelper64_SetDebugDocumentHost_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugDocumentHelper64_AddDeferredText_Proxy( 
    IDebugDocumentHelper64 * This,
    /* [in] */ ULONG cChars,
    /* [in] */ DWORD dwTextStartCookie);


void __RPC_STUB IDebugDocumentHelper64_AddDeferredText_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugDocumentHelper64_DefineScriptBlock_Proxy( 
    IDebugDocumentHelper64 * This,
    /* [in] */ ULONG ulCharOffset,
    /* [in] */ ULONG cChars,
    /* [in] */ IActiveScript *pas,
    /* [in] */ BOOL fScriptlet,
    /* [out] */ DWORDLONG *pdwSourceContext);


void __RPC_STUB IDebugDocumentHelper64_DefineScriptBlock_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugDocumentHelper64_SetDefaultTextAttr_Proxy( 
    IDebugDocumentHelper64 * This,
    SOURCE_TEXT_ATTR staTextAttr);


void __RPC_STUB IDebugDocumentHelper64_SetDefaultTextAttr_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugDocumentHelper64_SetTextAttributes_Proxy( 
    IDebugDocumentHelper64 * This,
    /* [in] */ ULONG ulCharOffset,
    /* [in] */ ULONG cChars,
    /* [size_is][length_is][in] */ SOURCE_TEXT_ATTR *pstaTextAttr);


void __RPC_STUB IDebugDocumentHelper64_SetTextAttributes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugDocumentHelper64_SetLongName_Proxy( 
    IDebugDocumentHelper64 * This,
    /* [string][in] */ LPCOLESTR pszLongName);


void __RPC_STUB IDebugDocumentHelper64_SetLongName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugDocumentHelper64_SetShortName_Proxy( 
    IDebugDocumentHelper64 * This,
    /* [string][in] */ LPCOLESTR pszShortName);


void __RPC_STUB IDebugDocumentHelper64_SetShortName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugDocumentHelper64_SetDocumentAttr_Proxy( 
    IDebugDocumentHelper64 * This,
    /* [in] */ TEXT_DOC_ATTR pszAttributes);


void __RPC_STUB IDebugDocumentHelper64_SetDocumentAttr_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugDocumentHelper64_GetDebugApplicationNode_Proxy( 
    IDebugDocumentHelper64 * This,
    /* [out] */ IDebugApplicationNode **ppdan);


void __RPC_STUB IDebugDocumentHelper64_GetDebugApplicationNode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugDocumentHelper64_GetScriptBlockInfo_Proxy( 
    IDebugDocumentHelper64 * This,
    /* [in] */ DWORDLONG dwSourceContext,
    /* [out] */ IActiveScript **ppasd,
    /* [out] */ ULONG *piCharPos,
    /* [out] */ ULONG *pcChars);


void __RPC_STUB IDebugDocumentHelper64_GetScriptBlockInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugDocumentHelper64_CreateDebugDocumentContext_Proxy( 
    IDebugDocumentHelper64 * This,
    /* [in] */ ULONG iCharPos,
    /* [in] */ ULONG cChars,
    /* [out] */ IDebugDocumentContext **ppddc);


void __RPC_STUB IDebugDocumentHelper64_CreateDebugDocumentContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugDocumentHelper64_BringDocumentToTop_Proxy( 
    IDebugDocumentHelper64 * This);


void __RPC_STUB IDebugDocumentHelper64_BringDocumentToTop_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugDocumentHelper64_BringDocumentContextToTop_Proxy( 
    IDebugDocumentHelper64 * This,
    IDebugDocumentContext *pddc);


void __RPC_STUB IDebugDocumentHelper64_BringDocumentContextToTop_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IDebugDocumentHelper64_INTERFACE_DEFINED__ */


#ifndef __IDebugDocumentHost_INTERFACE_DEFINED__
#define __IDebugDocumentHost_INTERFACE_DEFINED__

/* interface IDebugDocumentHost */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IDebugDocumentHost;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("51973C27-CB0C-11d0-B5C9-00A0244A0E7A")
    IDebugDocumentHost : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetDeferredText( 
            /* [in] */ DWORD dwTextStartCookie,
            /* [size_is][length_is][out][in] */ WCHAR *pcharText,
            /* [size_is][length_is][out][in] */ SOURCE_TEXT_ATTR *pstaTextAttr,
            /* [out][in] */ ULONG *pcNumChars,
            /* [in] */ ULONG cMaxChars) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetScriptTextAttributes( 
            /* [size_is][in] */ LPCOLESTR pstrCode,
            /* [in] */ ULONG uNumCodeChars,
            /* [in] */ LPCOLESTR pstrDelimiter,
            /* [in] */ DWORD dwFlags,
            /* [size_is][out][in] */ SOURCE_TEXT_ATTR *pattr) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnCreateDocumentContext( 
            /* [out] */ IUnknown **ppunkOuter) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetPathName( 
            /* [out] */ BSTR *pbstrLongName,
            /* [out] */ BOOL *pfIsOriginalFile) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetFileName( 
            /* [out] */ BSTR *pbstrShortName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE NotifyChanged( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDebugDocumentHostVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDebugDocumentHost * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDebugDocumentHost * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDebugDocumentHost * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetDeferredText )( 
            IDebugDocumentHost * This,
            /* [in] */ DWORD dwTextStartCookie,
            /* [size_is][length_is][out][in] */ WCHAR *pcharText,
            /* [size_is][length_is][out][in] */ SOURCE_TEXT_ATTR *pstaTextAttr,
            /* [out][in] */ ULONG *pcNumChars,
            /* [in] */ ULONG cMaxChars);
        
        HRESULT ( STDMETHODCALLTYPE *GetScriptTextAttributes )( 
            IDebugDocumentHost * This,
            /* [size_is][in] */ LPCOLESTR pstrCode,
            /* [in] */ ULONG uNumCodeChars,
            /* [in] */ LPCOLESTR pstrDelimiter,
            /* [in] */ DWORD dwFlags,
            /* [size_is][out][in] */ SOURCE_TEXT_ATTR *pattr);
        
        HRESULT ( STDMETHODCALLTYPE *OnCreateDocumentContext )( 
            IDebugDocumentHost * This,
            /* [out] */ IUnknown **ppunkOuter);
        
        HRESULT ( STDMETHODCALLTYPE *GetPathName )( 
            IDebugDocumentHost * This,
            /* [out] */ BSTR *pbstrLongName,
            /* [out] */ BOOL *pfIsOriginalFile);
        
        HRESULT ( STDMETHODCALLTYPE *GetFileName )( 
            IDebugDocumentHost * This,
            /* [out] */ BSTR *pbstrShortName);
        
        HRESULT ( STDMETHODCALLTYPE *NotifyChanged )( 
            IDebugDocumentHost * This);
        
        END_INTERFACE
    } IDebugDocumentHostVtbl;

    interface IDebugDocumentHost
    {
        CONST_VTBL struct IDebugDocumentHostVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDebugDocumentHost_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDebugDocumentHost_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDebugDocumentHost_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDebugDocumentHost_GetDeferredText(This,dwTextStartCookie,pcharText,pstaTextAttr,pcNumChars,cMaxChars)	\
    (This)->lpVtbl -> GetDeferredText(This,dwTextStartCookie,pcharText,pstaTextAttr,pcNumChars,cMaxChars)

#define IDebugDocumentHost_GetScriptTextAttributes(This,pstrCode,uNumCodeChars,pstrDelimiter,dwFlags,pattr)	\
    (This)->lpVtbl -> GetScriptTextAttributes(This,pstrCode,uNumCodeChars,pstrDelimiter,dwFlags,pattr)

#define IDebugDocumentHost_OnCreateDocumentContext(This,ppunkOuter)	\
    (This)->lpVtbl -> OnCreateDocumentContext(This,ppunkOuter)

#define IDebugDocumentHost_GetPathName(This,pbstrLongName,pfIsOriginalFile)	\
    (This)->lpVtbl -> GetPathName(This,pbstrLongName,pfIsOriginalFile)

#define IDebugDocumentHost_GetFileName(This,pbstrShortName)	\
    (This)->lpVtbl -> GetFileName(This,pbstrShortName)

#define IDebugDocumentHost_NotifyChanged(This)	\
    (This)->lpVtbl -> NotifyChanged(This)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IDebugDocumentHost_GetDeferredText_Proxy( 
    IDebugDocumentHost * This,
    /* [in] */ DWORD dwTextStartCookie,
    /* [size_is][length_is][out][in] */ WCHAR *pcharText,
    /* [size_is][length_is][out][in] */ SOURCE_TEXT_ATTR *pstaTextAttr,
    /* [out][in] */ ULONG *pcNumChars,
    /* [in] */ ULONG cMaxChars);


void __RPC_STUB IDebugDocumentHost_GetDeferredText_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugDocumentHost_GetScriptTextAttributes_Proxy( 
    IDebugDocumentHost * This,
    /* [size_is][in] */ LPCOLESTR pstrCode,
    /* [in] */ ULONG uNumCodeChars,
    /* [in] */ LPCOLESTR pstrDelimiter,
    /* [in] */ DWORD dwFlags,
    /* [size_is][out][in] */ SOURCE_TEXT_ATTR *pattr);


void __RPC_STUB IDebugDocumentHost_GetScriptTextAttributes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugDocumentHost_OnCreateDocumentContext_Proxy( 
    IDebugDocumentHost * This,
    /* [out] */ IUnknown **ppunkOuter);


void __RPC_STUB IDebugDocumentHost_OnCreateDocumentContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugDocumentHost_GetPathName_Proxy( 
    IDebugDocumentHost * This,
    /* [out] */ BSTR *pbstrLongName,
    /* [out] */ BOOL *pfIsOriginalFile);


void __RPC_STUB IDebugDocumentHost_GetPathName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugDocumentHost_GetFileName_Proxy( 
    IDebugDocumentHost * This,
    /* [out] */ BSTR *pbstrShortName);


void __RPC_STUB IDebugDocumentHost_GetFileName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugDocumentHost_NotifyChanged_Proxy( 
    IDebugDocumentHost * This);


void __RPC_STUB IDebugDocumentHost_NotifyChanged_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IDebugDocumentHost_INTERFACE_DEFINED__ */


#ifndef __IDebugDocumentContext_INTERFACE_DEFINED__
#define __IDebugDocumentContext_INTERFACE_DEFINED__

/* interface IDebugDocumentContext */
/* [unique][uuid][object] */ 


EXTERN_C const IID IID_IDebugDocumentContext;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("51973C28-CB0C-11d0-B5C9-00A0244A0E7A")
    IDebugDocumentContext : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetDocument( 
            /* [out] */ IDebugDocument **ppsd) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumCodeContexts( 
            /* [out] */ IEnumDebugCodeContexts **ppescc) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDebugDocumentContextVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDebugDocumentContext * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDebugDocumentContext * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDebugDocumentContext * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetDocument )( 
            IDebugDocumentContext * This,
            /* [out] */ IDebugDocument **ppsd);
        
        HRESULT ( STDMETHODCALLTYPE *EnumCodeContexts )( 
            IDebugDocumentContext * This,
            /* [out] */ IEnumDebugCodeContexts **ppescc);
        
        END_INTERFACE
    } IDebugDocumentContextVtbl;

    interface IDebugDocumentContext
    {
        CONST_VTBL struct IDebugDocumentContextVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDebugDocumentContext_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDebugDocumentContext_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDebugDocumentContext_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDebugDocumentContext_GetDocument(This,ppsd)	\
    (This)->lpVtbl -> GetDocument(This,ppsd)

#define IDebugDocumentContext_EnumCodeContexts(This,ppescc)	\
    (This)->lpVtbl -> EnumCodeContexts(This,ppescc)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IDebugDocumentContext_GetDocument_Proxy( 
    IDebugDocumentContext * This,
    /* [out] */ IDebugDocument **ppsd);


void __RPC_STUB IDebugDocumentContext_GetDocument_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugDocumentContext_EnumCodeContexts_Proxy( 
    IDebugDocumentContext * This,
    /* [out] */ IEnumDebugCodeContexts **ppescc);


void __RPC_STUB IDebugDocumentContext_EnumCodeContexts_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IDebugDocumentContext_INTERFACE_DEFINED__ */


#ifndef __IDebugSessionProvider_INTERFACE_DEFINED__
#define __IDebugSessionProvider_INTERFACE_DEFINED__

/* interface IDebugSessionProvider */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IDebugSessionProvider;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("51973C29-CB0C-11d0-B5C9-00A0244A0E7A")
    IDebugSessionProvider : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE StartDebugSession( 
            /* [in] */ IRemoteDebugApplication *pda) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDebugSessionProviderVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDebugSessionProvider * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDebugSessionProvider * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDebugSessionProvider * This);
        
        HRESULT ( STDMETHODCALLTYPE *StartDebugSession )( 
            IDebugSessionProvider * This,
            /* [in] */ IRemoteDebugApplication *pda);
        
        END_INTERFACE
    } IDebugSessionProviderVtbl;

    interface IDebugSessionProvider
    {
        CONST_VTBL struct IDebugSessionProviderVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDebugSessionProvider_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDebugSessionProvider_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDebugSessionProvider_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDebugSessionProvider_StartDebugSession(This,pda)	\
    (This)->lpVtbl -> StartDebugSession(This,pda)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IDebugSessionProvider_StartDebugSession_Proxy( 
    IDebugSessionProvider * This,
    /* [in] */ IRemoteDebugApplication *pda);


void __RPC_STUB IDebugSessionProvider_StartDebugSession_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IDebugSessionProvider_INTERFACE_DEFINED__ */


#ifndef __IApplicationDebugger_INTERFACE_DEFINED__
#define __IApplicationDebugger_INTERFACE_DEFINED__

/* interface IApplicationDebugger */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IApplicationDebugger;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("51973C2a-CB0C-11d0-B5C9-00A0244A0E7A")
    IApplicationDebugger : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE QueryAlive( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateInstanceAtDebugger( 
            /* [in] */ REFCLSID rclsid,
            /* [in] */ IUnknown *pUnkOuter,
            /* [in] */ DWORD dwClsContext,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ IUnknown **ppvObject) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE onDebugOutput( 
            /* [in] */ LPCOLESTR pstr) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE onHandleBreakPoint( 
            /* [in] */ IRemoteDebugApplicationThread *prpt,
            /* [in] */ BREAKREASON br,
            /* [in] */ IActiveScriptErrorDebug *pError) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE onClose( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE onDebuggerEvent( 
            /* [in] */ REFIID riid,
            /* [in] */ IUnknown *punk) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IApplicationDebuggerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IApplicationDebugger * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IApplicationDebugger * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IApplicationDebugger * This);
        
        HRESULT ( STDMETHODCALLTYPE *QueryAlive )( 
            IApplicationDebugger * This);
        
        HRESULT ( STDMETHODCALLTYPE *CreateInstanceAtDebugger )( 
            IApplicationDebugger * This,
            /* [in] */ REFCLSID rclsid,
            /* [in] */ IUnknown *pUnkOuter,
            /* [in] */ DWORD dwClsContext,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ IUnknown **ppvObject);
        
        HRESULT ( STDMETHODCALLTYPE *onDebugOutput )( 
            IApplicationDebugger * This,
            /* [in] */ LPCOLESTR pstr);
        
        HRESULT ( STDMETHODCALLTYPE *onHandleBreakPoint )( 
            IApplicationDebugger * This,
            /* [in] */ IRemoteDebugApplicationThread *prpt,
            /* [in] */ BREAKREASON br,
            /* [in] */ IActiveScriptErrorDebug *pError);
        
        HRESULT ( STDMETHODCALLTYPE *onClose )( 
            IApplicationDebugger * This);
        
        HRESULT ( STDMETHODCALLTYPE *onDebuggerEvent )( 
            IApplicationDebugger * This,
            /* [in] */ REFIID riid,
            /* [in] */ IUnknown *punk);
        
        END_INTERFACE
    } IApplicationDebuggerVtbl;

    interface IApplicationDebugger
    {
        CONST_VTBL struct IApplicationDebuggerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IApplicationDebugger_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IApplicationDebugger_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IApplicationDebugger_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IApplicationDebugger_QueryAlive(This)	\
    (This)->lpVtbl -> QueryAlive(This)

#define IApplicationDebugger_CreateInstanceAtDebugger(This,rclsid,pUnkOuter,dwClsContext,riid,ppvObject)	\
    (This)->lpVtbl -> CreateInstanceAtDebugger(This,rclsid,pUnkOuter,dwClsContext,riid,ppvObject)

#define IApplicationDebugger_onDebugOutput(This,pstr)	\
    (This)->lpVtbl -> onDebugOutput(This,pstr)

#define IApplicationDebugger_onHandleBreakPoint(This,prpt,br,pError)	\
    (This)->lpVtbl -> onHandleBreakPoint(This,prpt,br,pError)

#define IApplicationDebugger_onClose(This)	\
    (This)->lpVtbl -> onClose(This)

#define IApplicationDebugger_onDebuggerEvent(This,riid,punk)	\
    (This)->lpVtbl -> onDebuggerEvent(This,riid,punk)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IApplicationDebugger_QueryAlive_Proxy( 
    IApplicationDebugger * This);


void __RPC_STUB IApplicationDebugger_QueryAlive_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IApplicationDebugger_CreateInstanceAtDebugger_Proxy( 
    IApplicationDebugger * This,
    /* [in] */ REFCLSID rclsid,
    /* [in] */ IUnknown *pUnkOuter,
    /* [in] */ DWORD dwClsContext,
    /* [in] */ REFIID riid,
    /* [iid_is][out] */ IUnknown **ppvObject);


void __RPC_STUB IApplicationDebugger_CreateInstanceAtDebugger_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IApplicationDebugger_onDebugOutput_Proxy( 
    IApplicationDebugger * This,
    /* [in] */ LPCOLESTR pstr);


void __RPC_STUB IApplicationDebugger_onDebugOutput_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IApplicationDebugger_onHandleBreakPoint_Proxy( 
    IApplicationDebugger * This,
    /* [in] */ IRemoteDebugApplicationThread *prpt,
    /* [in] */ BREAKREASON br,
    /* [in] */ IActiveScriptErrorDebug *pError);


void __RPC_STUB IApplicationDebugger_onHandleBreakPoint_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IApplicationDebugger_onClose_Proxy( 
    IApplicationDebugger * This);


void __RPC_STUB IApplicationDebugger_onClose_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IApplicationDebugger_onDebuggerEvent_Proxy( 
    IApplicationDebugger * This,
    /* [in] */ REFIID riid,
    /* [in] */ IUnknown *punk);


void __RPC_STUB IApplicationDebugger_onDebuggerEvent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IApplicationDebugger_INTERFACE_DEFINED__ */


#ifndef __IApplicationDebuggerUI_INTERFACE_DEFINED__
#define __IApplicationDebuggerUI_INTERFACE_DEFINED__

/* interface IApplicationDebuggerUI */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IApplicationDebuggerUI;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("51973C2b-CB0C-11d0-B5C9-00A0244A0E7A")
    IApplicationDebuggerUI : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE BringDocumentToTop( 
            /* [in] */ IDebugDocumentText *pddt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE BringDocumentContextToTop( 
            /* [in] */ IDebugDocumentContext *pddc) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IApplicationDebuggerUIVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IApplicationDebuggerUI * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IApplicationDebuggerUI * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IApplicationDebuggerUI * This);
        
        HRESULT ( STDMETHODCALLTYPE *BringDocumentToTop )( 
            IApplicationDebuggerUI * This,
            /* [in] */ IDebugDocumentText *pddt);
        
        HRESULT ( STDMETHODCALLTYPE *BringDocumentContextToTop )( 
            IApplicationDebuggerUI * This,
            /* [in] */ IDebugDocumentContext *pddc);
        
        END_INTERFACE
    } IApplicationDebuggerUIVtbl;

    interface IApplicationDebuggerUI
    {
        CONST_VTBL struct IApplicationDebuggerUIVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IApplicationDebuggerUI_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IApplicationDebuggerUI_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IApplicationDebuggerUI_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IApplicationDebuggerUI_BringDocumentToTop(This,pddt)	\
    (This)->lpVtbl -> BringDocumentToTop(This,pddt)

#define IApplicationDebuggerUI_BringDocumentContextToTop(This,pddc)	\
    (This)->lpVtbl -> BringDocumentContextToTop(This,pddc)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IApplicationDebuggerUI_BringDocumentToTop_Proxy( 
    IApplicationDebuggerUI * This,
    /* [in] */ IDebugDocumentText *pddt);


void __RPC_STUB IApplicationDebuggerUI_BringDocumentToTop_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IApplicationDebuggerUI_BringDocumentContextToTop_Proxy( 
    IApplicationDebuggerUI * This,
    /* [in] */ IDebugDocumentContext *pddc);


void __RPC_STUB IApplicationDebuggerUI_BringDocumentContextToTop_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IApplicationDebuggerUI_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_activdbg_0336 */
/* [local] */ 

EXTERN_C const CLSID CLSID_MachineDebugManager;


extern RPC_IF_HANDLE __MIDL_itf_activdbg_0336_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_activdbg_0336_v0_0_s_ifspec;

#ifndef __IMachineDebugManager_INTERFACE_DEFINED__
#define __IMachineDebugManager_INTERFACE_DEFINED__

/* interface IMachineDebugManager */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IMachineDebugManager;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("51973C2c-CB0C-11d0-B5C9-00A0244A0E7A")
    IMachineDebugManager : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE AddApplication( 
            /* [in] */ IRemoteDebugApplication *pda,
            /* [out] */ DWORD *pdwAppCookie) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RemoveApplication( 
            /* [in] */ DWORD dwAppCookie) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumApplications( 
            /* [out] */ IEnumRemoteDebugApplications **ppeda) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IMachineDebugManagerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMachineDebugManager * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMachineDebugManager * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMachineDebugManager * This);
        
        HRESULT ( STDMETHODCALLTYPE *AddApplication )( 
            IMachineDebugManager * This,
            /* [in] */ IRemoteDebugApplication *pda,
            /* [out] */ DWORD *pdwAppCookie);
        
        HRESULT ( STDMETHODCALLTYPE *RemoveApplication )( 
            IMachineDebugManager * This,
            /* [in] */ DWORD dwAppCookie);
        
        HRESULT ( STDMETHODCALLTYPE *EnumApplications )( 
            IMachineDebugManager * This,
            /* [out] */ IEnumRemoteDebugApplications **ppeda);
        
        END_INTERFACE
    } IMachineDebugManagerVtbl;

    interface IMachineDebugManager
    {
        CONST_VTBL struct IMachineDebugManagerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMachineDebugManager_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMachineDebugManager_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMachineDebugManager_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMachineDebugManager_AddApplication(This,pda,pdwAppCookie)	\
    (This)->lpVtbl -> AddApplication(This,pda,pdwAppCookie)

#define IMachineDebugManager_RemoveApplication(This,dwAppCookie)	\
    (This)->lpVtbl -> RemoveApplication(This,dwAppCookie)

#define IMachineDebugManager_EnumApplications(This,ppeda)	\
    (This)->lpVtbl -> EnumApplications(This,ppeda)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IMachineDebugManager_AddApplication_Proxy( 
    IMachineDebugManager * This,
    /* [in] */ IRemoteDebugApplication *pda,
    /* [out] */ DWORD *pdwAppCookie);


void __RPC_STUB IMachineDebugManager_AddApplication_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMachineDebugManager_RemoveApplication_Proxy( 
    IMachineDebugManager * This,
    /* [in] */ DWORD dwAppCookie);


void __RPC_STUB IMachineDebugManager_RemoveApplication_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMachineDebugManager_EnumApplications_Proxy( 
    IMachineDebugManager * This,
    /* [out] */ IEnumRemoteDebugApplications **ppeda);


void __RPC_STUB IMachineDebugManager_EnumApplications_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IMachineDebugManager_INTERFACE_DEFINED__ */


#ifndef __IMachineDebugManagerCookie_INTERFACE_DEFINED__
#define __IMachineDebugManagerCookie_INTERFACE_DEFINED__

/* interface IMachineDebugManagerCookie */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IMachineDebugManagerCookie;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("51973C2d-CB0C-11d0-B5C9-00A0244A0E7A")
    IMachineDebugManagerCookie : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE AddApplication( 
            /* [in] */ IRemoteDebugApplication *pda,
            /* [in] */ DWORD dwDebugAppCookie,
            /* [out] */ DWORD *pdwAppCookie) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RemoveApplication( 
            /* [in] */ DWORD dwDebugAppCookie,
            /* [in] */ DWORD dwAppCookie) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumApplications( 
            /* [out] */ IEnumRemoteDebugApplications **ppeda) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IMachineDebugManagerCookieVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMachineDebugManagerCookie * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMachineDebugManagerCookie * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMachineDebugManagerCookie * This);
        
        HRESULT ( STDMETHODCALLTYPE *AddApplication )( 
            IMachineDebugManagerCookie * This,
            /* [in] */ IRemoteDebugApplication *pda,
            /* [in] */ DWORD dwDebugAppCookie,
            /* [out] */ DWORD *pdwAppCookie);
        
        HRESULT ( STDMETHODCALLTYPE *RemoveApplication )( 
            IMachineDebugManagerCookie * This,
            /* [in] */ DWORD dwDebugAppCookie,
            /* [in] */ DWORD dwAppCookie);
        
        HRESULT ( STDMETHODCALLTYPE *EnumApplications )( 
            IMachineDebugManagerCookie * This,
            /* [out] */ IEnumRemoteDebugApplications **ppeda);
        
        END_INTERFACE
    } IMachineDebugManagerCookieVtbl;

    interface IMachineDebugManagerCookie
    {
        CONST_VTBL struct IMachineDebugManagerCookieVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMachineDebugManagerCookie_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMachineDebugManagerCookie_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMachineDebugManagerCookie_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMachineDebugManagerCookie_AddApplication(This,pda,dwDebugAppCookie,pdwAppCookie)	\
    (This)->lpVtbl -> AddApplication(This,pda,dwDebugAppCookie,pdwAppCookie)

#define IMachineDebugManagerCookie_RemoveApplication(This,dwDebugAppCookie,dwAppCookie)	\
    (This)->lpVtbl -> RemoveApplication(This,dwDebugAppCookie,dwAppCookie)

#define IMachineDebugManagerCookie_EnumApplications(This,ppeda)	\
    (This)->lpVtbl -> EnumApplications(This,ppeda)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IMachineDebugManagerCookie_AddApplication_Proxy( 
    IMachineDebugManagerCookie * This,
    /* [in] */ IRemoteDebugApplication *pda,
    /* [in] */ DWORD dwDebugAppCookie,
    /* [out] */ DWORD *pdwAppCookie);


void __RPC_STUB IMachineDebugManagerCookie_AddApplication_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMachineDebugManagerCookie_RemoveApplication_Proxy( 
    IMachineDebugManagerCookie * This,
    /* [in] */ DWORD dwDebugAppCookie,
    /* [in] */ DWORD dwAppCookie);


void __RPC_STUB IMachineDebugManagerCookie_RemoveApplication_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMachineDebugManagerCookie_EnumApplications_Proxy( 
    IMachineDebugManagerCookie * This,
    /* [out] */ IEnumRemoteDebugApplications **ppeda);


void __RPC_STUB IMachineDebugManagerCookie_EnumApplications_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IMachineDebugManagerCookie_INTERFACE_DEFINED__ */


#ifndef __IMachineDebugManagerEvents_INTERFACE_DEFINED__
#define __IMachineDebugManagerEvents_INTERFACE_DEFINED__

/* interface IMachineDebugManagerEvents */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IMachineDebugManagerEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("51973C2e-CB0C-11d0-B5C9-00A0244A0E7A")
    IMachineDebugManagerEvents : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE onAddApplication( 
            /* [in] */ IRemoteDebugApplication *pda,
            /* [in] */ DWORD dwAppCookie) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE onRemoveApplication( 
            /* [in] */ IRemoteDebugApplication *pda,
            /* [in] */ DWORD dwAppCookie) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IMachineDebugManagerEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMachineDebugManagerEvents * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMachineDebugManagerEvents * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMachineDebugManagerEvents * This);
        
        HRESULT ( STDMETHODCALLTYPE *onAddApplication )( 
            IMachineDebugManagerEvents * This,
            /* [in] */ IRemoteDebugApplication *pda,
            /* [in] */ DWORD dwAppCookie);
        
        HRESULT ( STDMETHODCALLTYPE *onRemoveApplication )( 
            IMachineDebugManagerEvents * This,
            /* [in] */ IRemoteDebugApplication *pda,
            /* [in] */ DWORD dwAppCookie);
        
        END_INTERFACE
    } IMachineDebugManagerEventsVtbl;

    interface IMachineDebugManagerEvents
    {
        CONST_VTBL struct IMachineDebugManagerEventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMachineDebugManagerEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMachineDebugManagerEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMachineDebugManagerEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMachineDebugManagerEvents_onAddApplication(This,pda,dwAppCookie)	\
    (This)->lpVtbl -> onAddApplication(This,pda,dwAppCookie)

#define IMachineDebugManagerEvents_onRemoveApplication(This,pda,dwAppCookie)	\
    (This)->lpVtbl -> onRemoveApplication(This,pda,dwAppCookie)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IMachineDebugManagerEvents_onAddApplication_Proxy( 
    IMachineDebugManagerEvents * This,
    /* [in] */ IRemoteDebugApplication *pda,
    /* [in] */ DWORD dwAppCookie);


void __RPC_STUB IMachineDebugManagerEvents_onAddApplication_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMachineDebugManagerEvents_onRemoveApplication_Proxy( 
    IMachineDebugManagerEvents * This,
    /* [in] */ IRemoteDebugApplication *pda,
    /* [in] */ DWORD dwAppCookie);


void __RPC_STUB IMachineDebugManagerEvents_onRemoveApplication_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IMachineDebugManagerEvents_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_activdbg_0339 */
/* [local] */ 

EXTERN_C const CLSID CLSID_ProcessDebugManager;
#ifndef DISABLE_ACTIVDBG_INTERFACE_WRAPPERS
#ifdef _WIN64
#define IProcessDebugManager IProcessDebugManager64
#define IID_IProcessDebugManager IID_IProcessDebugManager64
#else
#define IProcessDebugManager IProcessDebugManager32
#define IID_IProcessDebugManager IID_IProcessDebugManager32
#endif
#endif


extern RPC_IF_HANDLE __MIDL_itf_activdbg_0339_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_activdbg_0339_v0_0_s_ifspec;

#ifndef __IProcessDebugManager32_INTERFACE_DEFINED__
#define __IProcessDebugManager32_INTERFACE_DEFINED__

/* interface IProcessDebugManager32 */
/* [local][unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IProcessDebugManager32;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("51973C2f-CB0C-11d0-B5C9-00A0244A0E7A")
    IProcessDebugManager32 : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE CreateApplication( 
            /* [out] */ IDebugApplication32 **ppda) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDefaultApplication( 
            /* [out] */ IDebugApplication32 **ppda) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddApplication( 
            /* [in] */ IDebugApplication32 *pda,
            /* [out] */ DWORD *pdwAppCookie) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RemoveApplication( 
            /* [in] */ DWORD dwAppCookie) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateDebugDocumentHelper( 
            /* [in] */ IUnknown *punkOuter,
            /* [out] */ IDebugDocumentHelper32 **pddh) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IProcessDebugManager32Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IProcessDebugManager32 * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IProcessDebugManager32 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IProcessDebugManager32 * This);
        
        HRESULT ( STDMETHODCALLTYPE *CreateApplication )( 
            IProcessDebugManager32 * This,
            /* [out] */ IDebugApplication32 **ppda);
        
        HRESULT ( STDMETHODCALLTYPE *GetDefaultApplication )( 
            IProcessDebugManager32 * This,
            /* [out] */ IDebugApplication32 **ppda);
        
        HRESULT ( STDMETHODCALLTYPE *AddApplication )( 
            IProcessDebugManager32 * This,
            /* [in] */ IDebugApplication32 *pda,
            /* [out] */ DWORD *pdwAppCookie);
        
        HRESULT ( STDMETHODCALLTYPE *RemoveApplication )( 
            IProcessDebugManager32 * This,
            /* [in] */ DWORD dwAppCookie);
        
        HRESULT ( STDMETHODCALLTYPE *CreateDebugDocumentHelper )( 
            IProcessDebugManager32 * This,
            /* [in] */ IUnknown *punkOuter,
            /* [out] */ IDebugDocumentHelper32 **pddh);
        
        END_INTERFACE
    } IProcessDebugManager32Vtbl;

    interface IProcessDebugManager32
    {
        CONST_VTBL struct IProcessDebugManager32Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IProcessDebugManager32_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IProcessDebugManager32_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IProcessDebugManager32_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IProcessDebugManager32_CreateApplication(This,ppda)	\
    (This)->lpVtbl -> CreateApplication(This,ppda)

#define IProcessDebugManager32_GetDefaultApplication(This,ppda)	\
    (This)->lpVtbl -> GetDefaultApplication(This,ppda)

#define IProcessDebugManager32_AddApplication(This,pda,pdwAppCookie)	\
    (This)->lpVtbl -> AddApplication(This,pda,pdwAppCookie)

#define IProcessDebugManager32_RemoveApplication(This,dwAppCookie)	\
    (This)->lpVtbl -> RemoveApplication(This,dwAppCookie)

#define IProcessDebugManager32_CreateDebugDocumentHelper(This,punkOuter,pddh)	\
    (This)->lpVtbl -> CreateDebugDocumentHelper(This,punkOuter,pddh)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IProcessDebugManager32_CreateApplication_Proxy( 
    IProcessDebugManager32 * This,
    /* [out] */ IDebugApplication32 **ppda);


void __RPC_STUB IProcessDebugManager32_CreateApplication_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IProcessDebugManager32_GetDefaultApplication_Proxy( 
    IProcessDebugManager32 * This,
    /* [out] */ IDebugApplication32 **ppda);


void __RPC_STUB IProcessDebugManager32_GetDefaultApplication_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IProcessDebugManager32_AddApplication_Proxy( 
    IProcessDebugManager32 * This,
    /* [in] */ IDebugApplication32 *pda,
    /* [out] */ DWORD *pdwAppCookie);


void __RPC_STUB IProcessDebugManager32_AddApplication_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IProcessDebugManager32_RemoveApplication_Proxy( 
    IProcessDebugManager32 * This,
    /* [in] */ DWORD dwAppCookie);


void __RPC_STUB IProcessDebugManager32_RemoveApplication_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IProcessDebugManager32_CreateDebugDocumentHelper_Proxy( 
    IProcessDebugManager32 * This,
    /* [in] */ IUnknown *punkOuter,
    /* [out] */ IDebugDocumentHelper32 **pddh);


void __RPC_STUB IProcessDebugManager32_CreateDebugDocumentHelper_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IProcessDebugManager32_INTERFACE_DEFINED__ */


#ifndef __IProcessDebugManager64_INTERFACE_DEFINED__
#define __IProcessDebugManager64_INTERFACE_DEFINED__

/* interface IProcessDebugManager64 */
/* [local][unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IProcessDebugManager64;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("56b9fc1c-63a9-4cc1-ac21-087d69a17fab")
    IProcessDebugManager64 : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE CreateApplication( 
            /* [out] */ IDebugApplication64 **ppda) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDefaultApplication( 
            /* [out] */ IDebugApplication64 **ppda) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddApplication( 
            /* [in] */ IDebugApplication64 *pda,
            /* [out] */ DWORD *pdwAppCookie) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RemoveApplication( 
            /* [in] */ DWORD dwAppCookie) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateDebugDocumentHelper( 
            /* [in] */ IUnknown *punkOuter,
            /* [out] */ IDebugDocumentHelper64 **pddh) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IProcessDebugManager64Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IProcessDebugManager64 * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IProcessDebugManager64 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IProcessDebugManager64 * This);
        
        HRESULT ( STDMETHODCALLTYPE *CreateApplication )( 
            IProcessDebugManager64 * This,
            /* [out] */ IDebugApplication64 **ppda);
        
        HRESULT ( STDMETHODCALLTYPE *GetDefaultApplication )( 
            IProcessDebugManager64 * This,
            /* [out] */ IDebugApplication64 **ppda);
        
        HRESULT ( STDMETHODCALLTYPE *AddApplication )( 
            IProcessDebugManager64 * This,
            /* [in] */ IDebugApplication64 *pda,
            /* [out] */ DWORD *pdwAppCookie);
        
        HRESULT ( STDMETHODCALLTYPE *RemoveApplication )( 
            IProcessDebugManager64 * This,
            /* [in] */ DWORD dwAppCookie);
        
        HRESULT ( STDMETHODCALLTYPE *CreateDebugDocumentHelper )( 
            IProcessDebugManager64 * This,
            /* [in] */ IUnknown *punkOuter,
            /* [out] */ IDebugDocumentHelper64 **pddh);
        
        END_INTERFACE
    } IProcessDebugManager64Vtbl;

    interface IProcessDebugManager64
    {
        CONST_VTBL struct IProcessDebugManager64Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IProcessDebugManager64_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IProcessDebugManager64_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IProcessDebugManager64_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IProcessDebugManager64_CreateApplication(This,ppda)	\
    (This)->lpVtbl -> CreateApplication(This,ppda)

#define IProcessDebugManager64_GetDefaultApplication(This,ppda)	\
    (This)->lpVtbl -> GetDefaultApplication(This,ppda)

#define IProcessDebugManager64_AddApplication(This,pda,pdwAppCookie)	\
    (This)->lpVtbl -> AddApplication(This,pda,pdwAppCookie)

#define IProcessDebugManager64_RemoveApplication(This,dwAppCookie)	\
    (This)->lpVtbl -> RemoveApplication(This,dwAppCookie)

#define IProcessDebugManager64_CreateDebugDocumentHelper(This,punkOuter,pddh)	\
    (This)->lpVtbl -> CreateDebugDocumentHelper(This,punkOuter,pddh)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IProcessDebugManager64_CreateApplication_Proxy( 
    IProcessDebugManager64 * This,
    /* [out] */ IDebugApplication64 **ppda);


void __RPC_STUB IProcessDebugManager64_CreateApplication_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IProcessDebugManager64_GetDefaultApplication_Proxy( 
    IProcessDebugManager64 * This,
    /* [out] */ IDebugApplication64 **ppda);


void __RPC_STUB IProcessDebugManager64_GetDefaultApplication_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IProcessDebugManager64_AddApplication_Proxy( 
    IProcessDebugManager64 * This,
    /* [in] */ IDebugApplication64 *pda,
    /* [out] */ DWORD *pdwAppCookie);


void __RPC_STUB IProcessDebugManager64_AddApplication_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IProcessDebugManager64_RemoveApplication_Proxy( 
    IProcessDebugManager64 * This,
    /* [in] */ DWORD dwAppCookie);


void __RPC_STUB IProcessDebugManager64_RemoveApplication_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IProcessDebugManager64_CreateDebugDocumentHelper_Proxy( 
    IProcessDebugManager64 * This,
    /* [in] */ IUnknown *punkOuter,
    /* [out] */ IDebugDocumentHelper64 **pddh);


void __RPC_STUB IProcessDebugManager64_CreateDebugDocumentHelper_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IProcessDebugManager64_INTERFACE_DEFINED__ */


#ifndef __IRemoteDebugApplication_INTERFACE_DEFINED__
#define __IRemoteDebugApplication_INTERFACE_DEFINED__

/* interface IRemoteDebugApplication */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IRemoteDebugApplication;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("51973C30-CB0C-11d0-B5C9-00A0244A0E7A")
    IRemoteDebugApplication : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ResumeFromBreakPoint( 
            /* [in] */ IRemoteDebugApplicationThread *prptFocus,
            /* [in] */ BREAKRESUMEACTION bra,
            /* [in] */ ERRORRESUMEACTION era) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CauseBreak( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ConnectDebugger( 
            /* [in] */ IApplicationDebugger *pad) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DisconnectDebugger( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDebugger( 
            /* [out] */ IApplicationDebugger **pad) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateInstanceAtApplication( 
            /* [in] */ REFCLSID rclsid,
            /* [in] */ IUnknown *pUnkOuter,
            /* [in] */ DWORD dwClsContext,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ IUnknown **ppvObject) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE QueryAlive( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumThreads( 
            /* [out] */ IEnumRemoteDebugApplicationThreads **pperdat) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetName( 
            /* [out] */ BSTR *pbstrName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetRootNode( 
            /* [out] */ IDebugApplicationNode **ppdanRoot) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumGlobalExpressionContexts( 
            /* [out] */ IEnumDebugExpressionContexts **ppedec) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IRemoteDebugApplicationVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRemoteDebugApplication * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRemoteDebugApplication * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRemoteDebugApplication * This);
        
        HRESULT ( STDMETHODCALLTYPE *ResumeFromBreakPoint )( 
            IRemoteDebugApplication * This,
            /* [in] */ IRemoteDebugApplicationThread *prptFocus,
            /* [in] */ BREAKRESUMEACTION bra,
            /* [in] */ ERRORRESUMEACTION era);
        
        HRESULT ( STDMETHODCALLTYPE *CauseBreak )( 
            IRemoteDebugApplication * This);
        
        HRESULT ( STDMETHODCALLTYPE *ConnectDebugger )( 
            IRemoteDebugApplication * This,
            /* [in] */ IApplicationDebugger *pad);
        
        HRESULT ( STDMETHODCALLTYPE *DisconnectDebugger )( 
            IRemoteDebugApplication * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetDebugger )( 
            IRemoteDebugApplication * This,
            /* [out] */ IApplicationDebugger **pad);
        
        HRESULT ( STDMETHODCALLTYPE *CreateInstanceAtApplication )( 
            IRemoteDebugApplication * This,
            /* [in] */ REFCLSID rclsid,
            /* [in] */ IUnknown *pUnkOuter,
            /* [in] */ DWORD dwClsContext,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ IUnknown **ppvObject);
        
        HRESULT ( STDMETHODCALLTYPE *QueryAlive )( 
            IRemoteDebugApplication * This);
        
        HRESULT ( STDMETHODCALLTYPE *EnumThreads )( 
            IRemoteDebugApplication * This,
            /* [out] */ IEnumRemoteDebugApplicationThreads **pperdat);
        
        HRESULT ( STDMETHODCALLTYPE *GetName )( 
            IRemoteDebugApplication * This,
            /* [out] */ BSTR *pbstrName);
        
        HRESULT ( STDMETHODCALLTYPE *GetRootNode )( 
            IRemoteDebugApplication * This,
            /* [out] */ IDebugApplicationNode **ppdanRoot);
        
        HRESULT ( STDMETHODCALLTYPE *EnumGlobalExpressionContexts )( 
            IRemoteDebugApplication * This,
            /* [out] */ IEnumDebugExpressionContexts **ppedec);
        
        END_INTERFACE
    } IRemoteDebugApplicationVtbl;

    interface IRemoteDebugApplication
    {
        CONST_VTBL struct IRemoteDebugApplicationVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRemoteDebugApplication_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRemoteDebugApplication_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRemoteDebugApplication_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRemoteDebugApplication_ResumeFromBreakPoint(This,prptFocus,bra,era)	\
    (This)->lpVtbl -> ResumeFromBreakPoint(This,prptFocus,bra,era)

#define IRemoteDebugApplication_CauseBreak(This)	\
    (This)->lpVtbl -> CauseBreak(This)

#define IRemoteDebugApplication_ConnectDebugger(This,pad)	\
    (This)->lpVtbl -> ConnectDebugger(This,pad)

#define IRemoteDebugApplication_DisconnectDebugger(This)	\
    (This)->lpVtbl -> DisconnectDebugger(This)

#define IRemoteDebugApplication_GetDebugger(This,pad)	\
    (This)->lpVtbl -> GetDebugger(This,pad)

#define IRemoteDebugApplication_CreateInstanceAtApplication(This,rclsid,pUnkOuter,dwClsContext,riid,ppvObject)	\
    (This)->lpVtbl -> CreateInstanceAtApplication(This,rclsid,pUnkOuter,dwClsContext,riid,ppvObject)

#define IRemoteDebugApplication_QueryAlive(This)	\
    (This)->lpVtbl -> QueryAlive(This)

#define IRemoteDebugApplication_EnumThreads(This,pperdat)	\
    (This)->lpVtbl -> EnumThreads(This,pperdat)

#define IRemoteDebugApplication_GetName(This,pbstrName)	\
    (This)->lpVtbl -> GetName(This,pbstrName)

#define IRemoteDebugApplication_GetRootNode(This,ppdanRoot)	\
    (This)->lpVtbl -> GetRootNode(This,ppdanRoot)

#define IRemoteDebugApplication_EnumGlobalExpressionContexts(This,ppedec)	\
    (This)->lpVtbl -> EnumGlobalExpressionContexts(This,ppedec)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IRemoteDebugApplication_ResumeFromBreakPoint_Proxy( 
    IRemoteDebugApplication * This,
    /* [in] */ IRemoteDebugApplicationThread *prptFocus,
    /* [in] */ BREAKRESUMEACTION bra,
    /* [in] */ ERRORRESUMEACTION era);


void __RPC_STUB IRemoteDebugApplication_ResumeFromBreakPoint_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRemoteDebugApplication_CauseBreak_Proxy( 
    IRemoteDebugApplication * This);


void __RPC_STUB IRemoteDebugApplication_CauseBreak_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRemoteDebugApplication_ConnectDebugger_Proxy( 
    IRemoteDebugApplication * This,
    /* [in] */ IApplicationDebugger *pad);


void __RPC_STUB IRemoteDebugApplication_ConnectDebugger_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRemoteDebugApplication_DisconnectDebugger_Proxy( 
    IRemoteDebugApplication * This);


void __RPC_STUB IRemoteDebugApplication_DisconnectDebugger_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRemoteDebugApplication_GetDebugger_Proxy( 
    IRemoteDebugApplication * This,
    /* [out] */ IApplicationDebugger **pad);


void __RPC_STUB IRemoteDebugApplication_GetDebugger_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRemoteDebugApplication_CreateInstanceAtApplication_Proxy( 
    IRemoteDebugApplication * This,
    /* [in] */ REFCLSID rclsid,
    /* [in] */ IUnknown *pUnkOuter,
    /* [in] */ DWORD dwClsContext,
    /* [in] */ REFIID riid,
    /* [iid_is][out] */ IUnknown **ppvObject);


void __RPC_STUB IRemoteDebugApplication_CreateInstanceAtApplication_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRemoteDebugApplication_QueryAlive_Proxy( 
    IRemoteDebugApplication * This);


void __RPC_STUB IRemoteDebugApplication_QueryAlive_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRemoteDebugApplication_EnumThreads_Proxy( 
    IRemoteDebugApplication * This,
    /* [out] */ IEnumRemoteDebugApplicationThreads **pperdat);


void __RPC_STUB IRemoteDebugApplication_EnumThreads_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRemoteDebugApplication_GetName_Proxy( 
    IRemoteDebugApplication * This,
    /* [out] */ BSTR *pbstrName);


void __RPC_STUB IRemoteDebugApplication_GetName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRemoteDebugApplication_GetRootNode_Proxy( 
    IRemoteDebugApplication * This,
    /* [out] */ IDebugApplicationNode **ppdanRoot);


void __RPC_STUB IRemoteDebugApplication_GetRootNode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRemoteDebugApplication_EnumGlobalExpressionContexts_Proxy( 
    IRemoteDebugApplication * This,
    /* [out] */ IEnumDebugExpressionContexts **ppedec);


void __RPC_STUB IRemoteDebugApplication_EnumGlobalExpressionContexts_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IRemoteDebugApplication_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_activdbg_0342 */
/* [local] */ 

#ifndef DISABLE_ACTIVDBG_INTERFACE_WRAPPERS
#ifdef _WIN64
#define IDebugApplication IDebugApplication64
#define IID_IDebugApplication IID_IDebugApplication64
#else
#define IDebugApplication IDebugApplication32
#define IID_IDebugApplication IID_IDebugApplication32
#endif
#endif


extern RPC_IF_HANDLE __MIDL_itf_activdbg_0342_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_activdbg_0342_v0_0_s_ifspec;

#ifndef __IDebugApplication32_INTERFACE_DEFINED__
#define __IDebugApplication32_INTERFACE_DEFINED__

/* interface IDebugApplication32 */
/* [local][unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IDebugApplication32;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("51973C32-CB0C-11d0-B5C9-00A0244A0E7A")
    IDebugApplication32 : public IRemoteDebugApplication
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetName( 
            /* [in] */ LPCOLESTR pstrName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE StepOutComplete( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DebugOutput( 
            /* [in] */ LPCOLESTR pstr) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE StartDebugSession( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE HandleBreakPoint( 
            /* [in] */ BREAKREASON br,
            /* [out] */ BREAKRESUMEACTION *pbra) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Close( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetBreakFlags( 
            /* [out] */ APPBREAKFLAGS *pabf,
            /* [out] */ IRemoteDebugApplicationThread **pprdatSteppingThread) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCurrentThread( 
            /* [out] */ IDebugApplicationThread **pat) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateAsyncDebugOperation( 
            /* [in] */ IDebugSyncOperation *psdo,
            /* [out] */ IDebugAsyncOperation **ppado) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddStackFrameSniffer( 
            /* [in] */ IDebugStackFrameSniffer *pdsfs,
            /* [out] */ DWORD *pdwCookie) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RemoveStackFrameSniffer( 
            /* [in] */ DWORD dwCookie) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE QueryCurrentThreadIsDebuggerThread( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SynchronousCallInDebuggerThread( 
            /* [in] */ IDebugThreadCall32 *pptc,
            /* [in] */ DWORD dwParam1,
            /* [in] */ DWORD dwParam2,
            /* [in] */ DWORD dwParam3) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateApplicationNode( 
            /* [out] */ IDebugApplicationNode **ppdanNew) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE FireDebuggerEvent( 
            /* [in] */ REFGUID riid,
            /* [in] */ IUnknown *punk) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE HandleRuntimeError( 
            /* [in] */ IActiveScriptErrorDebug *pErrorDebug,
            /* [in] */ IActiveScriptSite *pScriptSite,
            /* [out] */ BREAKRESUMEACTION *pbra,
            /* [out] */ ERRORRESUMEACTION *perra,
            /* [out] */ BOOL *pfCallOnScriptError) = 0;
        
        virtual BOOL STDMETHODCALLTYPE FCanJitDebug( void) = 0;
        
        virtual BOOL STDMETHODCALLTYPE FIsAutoJitDebugEnabled( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddGlobalExpressionContextProvider( 
            /* [in] */ IProvideExpressionContexts *pdsfs,
            /* [out] */ DWORD *pdwCookie) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RemoveGlobalExpressionContextProvider( 
            /* [in] */ DWORD dwCookie) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDebugApplication32Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDebugApplication32 * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDebugApplication32 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDebugApplication32 * This);
        
        HRESULT ( STDMETHODCALLTYPE *ResumeFromBreakPoint )( 
            IDebugApplication32 * This,
            /* [in] */ IRemoteDebugApplicationThread *prptFocus,
            /* [in] */ BREAKRESUMEACTION bra,
            /* [in] */ ERRORRESUMEACTION era);
        
        HRESULT ( STDMETHODCALLTYPE *CauseBreak )( 
            IDebugApplication32 * This);
        
        HRESULT ( STDMETHODCALLTYPE *ConnectDebugger )( 
            IDebugApplication32 * This,
            /* [in] */ IApplicationDebugger *pad);
        
        HRESULT ( STDMETHODCALLTYPE *DisconnectDebugger )( 
            IDebugApplication32 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetDebugger )( 
            IDebugApplication32 * This,
            /* [out] */ IApplicationDebugger **pad);
        
        HRESULT ( STDMETHODCALLTYPE *CreateInstanceAtApplication )( 
            IDebugApplication32 * This,
            /* [in] */ REFCLSID rclsid,
            /* [in] */ IUnknown *pUnkOuter,
            /* [in] */ DWORD dwClsContext,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ IUnknown **ppvObject);
        
        HRESULT ( STDMETHODCALLTYPE *QueryAlive )( 
            IDebugApplication32 * This);
        
        HRESULT ( STDMETHODCALLTYPE *EnumThreads )( 
            IDebugApplication32 * This,
            /* [out] */ IEnumRemoteDebugApplicationThreads **pperdat);
        
        HRESULT ( STDMETHODCALLTYPE *GetName )( 
            IDebugApplication32 * This,
            /* [out] */ BSTR *pbstrName);
        
        HRESULT ( STDMETHODCALLTYPE *GetRootNode )( 
            IDebugApplication32 * This,
            /* [out] */ IDebugApplicationNode **ppdanRoot);
        
        HRESULT ( STDMETHODCALLTYPE *EnumGlobalExpressionContexts )( 
            IDebugApplication32 * This,
            /* [out] */ IEnumDebugExpressionContexts **ppedec);
        
        HRESULT ( STDMETHODCALLTYPE *SetName )( 
            IDebugApplication32 * This,
            /* [in] */ LPCOLESTR pstrName);
        
        HRESULT ( STDMETHODCALLTYPE *StepOutComplete )( 
            IDebugApplication32 * This);
        
        HRESULT ( STDMETHODCALLTYPE *DebugOutput )( 
            IDebugApplication32 * This,
            /* [in] */ LPCOLESTR pstr);
        
        HRESULT ( STDMETHODCALLTYPE *StartDebugSession )( 
            IDebugApplication32 * This);
        
        HRESULT ( STDMETHODCALLTYPE *HandleBreakPoint )( 
            IDebugApplication32 * This,
            /* [in] */ BREAKREASON br,
            /* [out] */ BREAKRESUMEACTION *pbra);
        
        HRESULT ( STDMETHODCALLTYPE *Close )( 
            IDebugApplication32 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetBreakFlags )( 
            IDebugApplication32 * This,
            /* [out] */ APPBREAKFLAGS *pabf,
            /* [out] */ IRemoteDebugApplicationThread **pprdatSteppingThread);
        
        HRESULT ( STDMETHODCALLTYPE *GetCurrentThread )( 
            IDebugApplication32 * This,
            /* [out] */ IDebugApplicationThread **pat);
        
        HRESULT ( STDMETHODCALLTYPE *CreateAsyncDebugOperation )( 
            IDebugApplication32 * This,
            /* [in] */ IDebugSyncOperation *psdo,
            /* [out] */ IDebugAsyncOperation **ppado);
        
        HRESULT ( STDMETHODCALLTYPE *AddStackFrameSniffer )( 
            IDebugApplication32 * This,
            /* [in] */ IDebugStackFrameSniffer *pdsfs,
            /* [out] */ DWORD *pdwCookie);
        
        HRESULT ( STDMETHODCALLTYPE *RemoveStackFrameSniffer )( 
            IDebugApplication32 * This,
            /* [in] */ DWORD dwCookie);
        
        HRESULT ( STDMETHODCALLTYPE *QueryCurrentThreadIsDebuggerThread )( 
            IDebugApplication32 * This);
        
        HRESULT ( STDMETHODCALLTYPE *SynchronousCallInDebuggerThread )( 
            IDebugApplication32 * This,
            /* [in] */ IDebugThreadCall32 *pptc,
            /* [in] */ DWORD dwParam1,
            /* [in] */ DWORD dwParam2,
            /* [in] */ DWORD dwParam3);
        
        HRESULT ( STDMETHODCALLTYPE *CreateApplicationNode )( 
            IDebugApplication32 * This,
            /* [out] */ IDebugApplicationNode **ppdanNew);
        
        HRESULT ( STDMETHODCALLTYPE *FireDebuggerEvent )( 
            IDebugApplication32 * This,
            /* [in] */ REFGUID riid,
            /* [in] */ IUnknown *punk);
        
        HRESULT ( STDMETHODCALLTYPE *HandleRuntimeError )( 
            IDebugApplication32 * This,
            /* [in] */ IActiveScriptErrorDebug *pErrorDebug,
            /* [in] */ IActiveScriptSite *pScriptSite,
            /* [out] */ BREAKRESUMEACTION *pbra,
            /* [out] */ ERRORRESUMEACTION *perra,
            /* [out] */ BOOL *pfCallOnScriptError);
        
        BOOL ( STDMETHODCALLTYPE *FCanJitDebug )( 
            IDebugApplication32 * This);
        
        BOOL ( STDMETHODCALLTYPE *FIsAutoJitDebugEnabled )( 
            IDebugApplication32 * This);
        
        HRESULT ( STDMETHODCALLTYPE *AddGlobalExpressionContextProvider )( 
            IDebugApplication32 * This,
            /* [in] */ IProvideExpressionContexts *pdsfs,
            /* [out] */ DWORD *pdwCookie);
        
        HRESULT ( STDMETHODCALLTYPE *RemoveGlobalExpressionContextProvider )( 
            IDebugApplication32 * This,
            /* [in] */ DWORD dwCookie);
        
        END_INTERFACE
    } IDebugApplication32Vtbl;

    interface IDebugApplication32
    {
        CONST_VTBL struct IDebugApplication32Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDebugApplication32_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDebugApplication32_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDebugApplication32_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDebugApplication32_ResumeFromBreakPoint(This,prptFocus,bra,era)	\
    (This)->lpVtbl -> ResumeFromBreakPoint(This,prptFocus,bra,era)

#define IDebugApplication32_CauseBreak(This)	\
    (This)->lpVtbl -> CauseBreak(This)

#define IDebugApplication32_ConnectDebugger(This,pad)	\
    (This)->lpVtbl -> ConnectDebugger(This,pad)

#define IDebugApplication32_DisconnectDebugger(This)	\
    (This)->lpVtbl -> DisconnectDebugger(This)

#define IDebugApplication32_GetDebugger(This,pad)	\
    (This)->lpVtbl -> GetDebugger(This,pad)

#define IDebugApplication32_CreateInstanceAtApplication(This,rclsid,pUnkOuter,dwClsContext,riid,ppvObject)	\
    (This)->lpVtbl -> CreateInstanceAtApplication(This,rclsid,pUnkOuter,dwClsContext,riid,ppvObject)

#define IDebugApplication32_QueryAlive(This)	\
    (This)->lpVtbl -> QueryAlive(This)

#define IDebugApplication32_EnumThreads(This,pperdat)	\
    (This)->lpVtbl -> EnumThreads(This,pperdat)

#define IDebugApplication32_GetName(This,pbstrName)	\
    (This)->lpVtbl -> GetName(This,pbstrName)

#define IDebugApplication32_GetRootNode(This,ppdanRoot)	\
    (This)->lpVtbl -> GetRootNode(This,ppdanRoot)

#define IDebugApplication32_EnumGlobalExpressionContexts(This,ppedec)	\
    (This)->lpVtbl -> EnumGlobalExpressionContexts(This,ppedec)


#define IDebugApplication32_SetName(This,pstrName)	\
    (This)->lpVtbl -> SetName(This,pstrName)

#define IDebugApplication32_StepOutComplete(This)	\
    (This)->lpVtbl -> StepOutComplete(This)

#define IDebugApplication32_DebugOutput(This,pstr)	\
    (This)->lpVtbl -> DebugOutput(This,pstr)

#define IDebugApplication32_StartDebugSession(This)	\
    (This)->lpVtbl -> StartDebugSession(This)

#define IDebugApplication32_HandleBreakPoint(This,br,pbra)	\
    (This)->lpVtbl -> HandleBreakPoint(This,br,pbra)

#define IDebugApplication32_Close(This)	\
    (This)->lpVtbl -> Close(This)

#define IDebugApplication32_GetBreakFlags(This,pabf,pprdatSteppingThread)	\
    (This)->lpVtbl -> GetBreakFlags(This,pabf,pprdatSteppingThread)

#define IDebugApplication32_GetCurrentThread(This,pat)	\
    (This)->lpVtbl -> GetCurrentThread(This,pat)

#define IDebugApplication32_CreateAsyncDebugOperation(This,psdo,ppado)	\
    (This)->lpVtbl -> CreateAsyncDebugOperation(This,psdo,ppado)

#define IDebugApplication32_AddStackFrameSniffer(This,pdsfs,pdwCookie)	\
    (This)->lpVtbl -> AddStackFrameSniffer(This,pdsfs,pdwCookie)

#define IDebugApplication32_RemoveStackFrameSniffer(This,dwCookie)	\
    (This)->lpVtbl -> RemoveStackFrameSniffer(This,dwCookie)

#define IDebugApplication32_QueryCurrentThreadIsDebuggerThread(This)	\
    (This)->lpVtbl -> QueryCurrentThreadIsDebuggerThread(This)

#define IDebugApplication32_SynchronousCallInDebuggerThread(This,pptc,dwParam1,dwParam2,dwParam3)	\
    (This)->lpVtbl -> SynchronousCallInDebuggerThread(This,pptc,dwParam1,dwParam2,dwParam3)

#define IDebugApplication32_CreateApplicationNode(This,ppdanNew)	\
    (This)->lpVtbl -> CreateApplicationNode(This,ppdanNew)

#define IDebugApplication32_FireDebuggerEvent(This,riid,punk)	\
    (This)->lpVtbl -> FireDebuggerEvent(This,riid,punk)

#define IDebugApplication32_HandleRuntimeError(This,pErrorDebug,pScriptSite,pbra,perra,pfCallOnScriptError)	\
    (This)->lpVtbl -> HandleRuntimeError(This,pErrorDebug,pScriptSite,pbra,perra,pfCallOnScriptError)

#define IDebugApplication32_FCanJitDebug(This)	\
    (This)->lpVtbl -> FCanJitDebug(This)

#define IDebugApplication32_FIsAutoJitDebugEnabled(This)	\
    (This)->lpVtbl -> FIsAutoJitDebugEnabled(This)

#define IDebugApplication32_AddGlobalExpressionContextProvider(This,pdsfs,pdwCookie)	\
    (This)->lpVtbl -> AddGlobalExpressionContextProvider(This,pdsfs,pdwCookie)

#define IDebugApplication32_RemoveGlobalExpressionContextProvider(This,dwCookie)	\
    (This)->lpVtbl -> RemoveGlobalExpressionContextProvider(This,dwCookie)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IDebugApplication32_SetName_Proxy( 
    IDebugApplication32 * This,
    /* [in] */ LPCOLESTR pstrName);


void __RPC_STUB IDebugApplication32_SetName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugApplication32_StepOutComplete_Proxy( 
    IDebugApplication32 * This);


void __RPC_STUB IDebugApplication32_StepOutComplete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugApplication32_DebugOutput_Proxy( 
    IDebugApplication32 * This,
    /* [in] */ LPCOLESTR pstr);


void __RPC_STUB IDebugApplication32_DebugOutput_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugApplication32_StartDebugSession_Proxy( 
    IDebugApplication32 * This);


void __RPC_STUB IDebugApplication32_StartDebugSession_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugApplication32_HandleBreakPoint_Proxy( 
    IDebugApplication32 * This,
    /* [in] */ BREAKREASON br,
    /* [out] */ BREAKRESUMEACTION *pbra);


void __RPC_STUB IDebugApplication32_HandleBreakPoint_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugApplication32_Close_Proxy( 
    IDebugApplication32 * This);


void __RPC_STUB IDebugApplication32_Close_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugApplication32_GetBreakFlags_Proxy( 
    IDebugApplication32 * This,
    /* [out] */ APPBREAKFLAGS *pabf,
    /* [out] */ IRemoteDebugApplicationThread **pprdatSteppingThread);


void __RPC_STUB IDebugApplication32_GetBreakFlags_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugApplication32_GetCurrentThread_Proxy( 
    IDebugApplication32 * This,
    /* [out] */ IDebugApplicationThread **pat);


void __RPC_STUB IDebugApplication32_GetCurrentThread_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugApplication32_CreateAsyncDebugOperation_Proxy( 
    IDebugApplication32 * This,
    /* [in] */ IDebugSyncOperation *psdo,
    /* [out] */ IDebugAsyncOperation **ppado);


void __RPC_STUB IDebugApplication32_CreateAsyncDebugOperation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugApplication32_AddStackFrameSniffer_Proxy( 
    IDebugApplication32 * This,
    /* [in] */ IDebugStackFrameSniffer *pdsfs,
    /* [out] */ DWORD *pdwCookie);


void __RPC_STUB IDebugApplication32_AddStackFrameSniffer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugApplication32_RemoveStackFrameSniffer_Proxy( 
    IDebugApplication32 * This,
    /* [in] */ DWORD dwCookie);


void __RPC_STUB IDebugApplication32_RemoveStackFrameSniffer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugApplication32_QueryCurrentThreadIsDebuggerThread_Proxy( 
    IDebugApplication32 * This);


void __RPC_STUB IDebugApplication32_QueryCurrentThreadIsDebuggerThread_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugApplication32_SynchronousCallInDebuggerThread_Proxy( 
    IDebugApplication32 * This,
    /* [in] */ IDebugThreadCall32 *pptc,
    /* [in] */ DWORD dwParam1,
    /* [in] */ DWORD dwParam2,
    /* [in] */ DWORD dwParam3);


void __RPC_STUB IDebugApplication32_SynchronousCallInDebuggerThread_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugApplication32_CreateApplicationNode_Proxy( 
    IDebugApplication32 * This,
    /* [out] */ IDebugApplicationNode **ppdanNew);


void __RPC_STUB IDebugApplication32_CreateApplicationNode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugApplication32_FireDebuggerEvent_Proxy( 
    IDebugApplication32 * This,
    /* [in] */ REFGUID riid,
    /* [in] */ IUnknown *punk);


void __RPC_STUB IDebugApplication32_FireDebuggerEvent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugApplication32_HandleRuntimeError_Proxy( 
    IDebugApplication32 * This,
    /* [in] */ IActiveScriptErrorDebug *pErrorDebug,
    /* [in] */ IActiveScriptSite *pScriptSite,
    /* [out] */ BREAKRESUMEACTION *pbra,
    /* [out] */ ERRORRESUMEACTION *perra,
    /* [out] */ BOOL *pfCallOnScriptError);


void __RPC_STUB IDebugApplication32_HandleRuntimeError_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


BOOL STDMETHODCALLTYPE IDebugApplication32_FCanJitDebug_Proxy( 
    IDebugApplication32 * This);


void __RPC_STUB IDebugApplication32_FCanJitDebug_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


BOOL STDMETHODCALLTYPE IDebugApplication32_FIsAutoJitDebugEnabled_Proxy( 
    IDebugApplication32 * This);


void __RPC_STUB IDebugApplication32_FIsAutoJitDebugEnabled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugApplication32_AddGlobalExpressionContextProvider_Proxy( 
    IDebugApplication32 * This,
    /* [in] */ IProvideExpressionContexts *pdsfs,
    /* [out] */ DWORD *pdwCookie);


void __RPC_STUB IDebugApplication32_AddGlobalExpressionContextProvider_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugApplication32_RemoveGlobalExpressionContextProvider_Proxy( 
    IDebugApplication32 * This,
    /* [in] */ DWORD dwCookie);


void __RPC_STUB IDebugApplication32_RemoveGlobalExpressionContextProvider_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IDebugApplication32_INTERFACE_DEFINED__ */


#ifndef __IDebugApplication64_INTERFACE_DEFINED__
#define __IDebugApplication64_INTERFACE_DEFINED__

/* interface IDebugApplication64 */
/* [local][unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IDebugApplication64;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("4dedc754-04c7-4f10-9e60-16a390fe6e62")
    IDebugApplication64 : public IRemoteDebugApplication
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetName( 
            /* [in] */ LPCOLESTR pstrName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE StepOutComplete( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DebugOutput( 
            /* [in] */ LPCOLESTR pstr) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE StartDebugSession( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE HandleBreakPoint( 
            /* [in] */ BREAKREASON br,
            /* [out] */ BREAKRESUMEACTION *pbra) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Close( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetBreakFlags( 
            /* [out] */ APPBREAKFLAGS *pabf,
            /* [out] */ IRemoteDebugApplicationThread **pprdatSteppingThread) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCurrentThread( 
            /* [out] */ IDebugApplicationThread **pat) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateAsyncDebugOperation( 
            /* [in] */ IDebugSyncOperation *psdo,
            /* [out] */ IDebugAsyncOperation **ppado) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddStackFrameSniffer( 
            /* [in] */ IDebugStackFrameSniffer *pdsfs,
            /* [out] */ DWORD *pdwCookie) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RemoveStackFrameSniffer( 
            /* [in] */ DWORD dwCookie) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE QueryCurrentThreadIsDebuggerThread( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SynchronousCallInDebuggerThread( 
            /* [in] */ IDebugThreadCall64 *pptc,
            /* [in] */ DWORDLONG dwParam1,
            /* [in] */ DWORDLONG dwParam2,
            /* [in] */ DWORDLONG dwParam3) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateApplicationNode( 
            /* [out] */ IDebugApplicationNode **ppdanNew) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE FireDebuggerEvent( 
            /* [in] */ REFGUID riid,
            /* [in] */ IUnknown *punk) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE HandleRuntimeError( 
            /* [in] */ IActiveScriptErrorDebug *pErrorDebug,
            /* [in] */ IActiveScriptSite *pScriptSite,
            /* [out] */ BREAKRESUMEACTION *pbra,
            /* [out] */ ERRORRESUMEACTION *perra,
            /* [out] */ BOOL *pfCallOnScriptError) = 0;
        
        virtual BOOL STDMETHODCALLTYPE FCanJitDebug( void) = 0;
        
        virtual BOOL STDMETHODCALLTYPE FIsAutoJitDebugEnabled( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddGlobalExpressionContextProvider( 
            /* [in] */ IProvideExpressionContexts *pdsfs,
            /* [out] */ DWORDLONG *pdwCookie) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RemoveGlobalExpressionContextProvider( 
            /* [in] */ DWORDLONG dwCookie) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDebugApplication64Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDebugApplication64 * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDebugApplication64 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDebugApplication64 * This);
        
        HRESULT ( STDMETHODCALLTYPE *ResumeFromBreakPoint )( 
            IDebugApplication64 * This,
            /* [in] */ IRemoteDebugApplicationThread *prptFocus,
            /* [in] */ BREAKRESUMEACTION bra,
            /* [in] */ ERRORRESUMEACTION era);
        
        HRESULT ( STDMETHODCALLTYPE *CauseBreak )( 
            IDebugApplication64 * This);
        
        HRESULT ( STDMETHODCALLTYPE *ConnectDebugger )( 
            IDebugApplication64 * This,
            /* [in] */ IApplicationDebugger *pad);
        
        HRESULT ( STDMETHODCALLTYPE *DisconnectDebugger )( 
            IDebugApplication64 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetDebugger )( 
            IDebugApplication64 * This,
            /* [out] */ IApplicationDebugger **pad);
        
        HRESULT ( STDMETHODCALLTYPE *CreateInstanceAtApplication )( 
            IDebugApplication64 * This,
            /* [in] */ REFCLSID rclsid,
            /* [in] */ IUnknown *pUnkOuter,
            /* [in] */ DWORD dwClsContext,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ IUnknown **ppvObject);
        
        HRESULT ( STDMETHODCALLTYPE *QueryAlive )( 
            IDebugApplication64 * This);
        
        HRESULT ( STDMETHODCALLTYPE *EnumThreads )( 
            IDebugApplication64 * This,
            /* [out] */ IEnumRemoteDebugApplicationThreads **pperdat);
        
        HRESULT ( STDMETHODCALLTYPE *GetName )( 
            IDebugApplication64 * This,
            /* [out] */ BSTR *pbstrName);
        
        HRESULT ( STDMETHODCALLTYPE *GetRootNode )( 
            IDebugApplication64 * This,
            /* [out] */ IDebugApplicationNode **ppdanRoot);
        
        HRESULT ( STDMETHODCALLTYPE *EnumGlobalExpressionContexts )( 
            IDebugApplication64 * This,
            /* [out] */ IEnumDebugExpressionContexts **ppedec);
        
        HRESULT ( STDMETHODCALLTYPE *SetName )( 
            IDebugApplication64 * This,
            /* [in] */ LPCOLESTR pstrName);
        
        HRESULT ( STDMETHODCALLTYPE *StepOutComplete )( 
            IDebugApplication64 * This);
        
        HRESULT ( STDMETHODCALLTYPE *DebugOutput )( 
            IDebugApplication64 * This,
            /* [in] */ LPCOLESTR pstr);
        
        HRESULT ( STDMETHODCALLTYPE *StartDebugSession )( 
            IDebugApplication64 * This);
        
        HRESULT ( STDMETHODCALLTYPE *HandleBreakPoint )( 
            IDebugApplication64 * This,
            /* [in] */ BREAKREASON br,
            /* [out] */ BREAKRESUMEACTION *pbra);
        
        HRESULT ( STDMETHODCALLTYPE *Close )( 
            IDebugApplication64 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetBreakFlags )( 
            IDebugApplication64 * This,
            /* [out] */ APPBREAKFLAGS *pabf,
            /* [out] */ IRemoteDebugApplicationThread **pprdatSteppingThread);
        
        HRESULT ( STDMETHODCALLTYPE *GetCurrentThread )( 
            IDebugApplication64 * This,
            /* [out] */ IDebugApplicationThread **pat);
        
        HRESULT ( STDMETHODCALLTYPE *CreateAsyncDebugOperation )( 
            IDebugApplication64 * This,
            /* [in] */ IDebugSyncOperation *psdo,
            /* [out] */ IDebugAsyncOperation **ppado);
        
        HRESULT ( STDMETHODCALLTYPE *AddStackFrameSniffer )( 
            IDebugApplication64 * This,
            /* [in] */ IDebugStackFrameSniffer *pdsfs,
            /* [out] */ DWORD *pdwCookie);
        
        HRESULT ( STDMETHODCALLTYPE *RemoveStackFrameSniffer )( 
            IDebugApplication64 * This,
            /* [in] */ DWORD dwCookie);
        
        HRESULT ( STDMETHODCALLTYPE *QueryCurrentThreadIsDebuggerThread )( 
            IDebugApplication64 * This);
        
        HRESULT ( STDMETHODCALLTYPE *SynchronousCallInDebuggerThread )( 
            IDebugApplication64 * This,
            /* [in] */ IDebugThreadCall64 *pptc,
            /* [in] */ DWORDLONG dwParam1,
            /* [in] */ DWORDLONG dwParam2,
            /* [in] */ DWORDLONG dwParam3);
        
        HRESULT ( STDMETHODCALLTYPE *CreateApplicationNode )( 
            IDebugApplication64 * This,
            /* [out] */ IDebugApplicationNode **ppdanNew);
        
        HRESULT ( STDMETHODCALLTYPE *FireDebuggerEvent )( 
            IDebugApplication64 * This,
            /* [in] */ REFGUID riid,
            /* [in] */ IUnknown *punk);
        
        HRESULT ( STDMETHODCALLTYPE *HandleRuntimeError )( 
            IDebugApplication64 * This,
            /* [in] */ IActiveScriptErrorDebug *pErrorDebug,
            /* [in] */ IActiveScriptSite *pScriptSite,
            /* [out] */ BREAKRESUMEACTION *pbra,
            /* [out] */ ERRORRESUMEACTION *perra,
            /* [out] */ BOOL *pfCallOnScriptError);
        
        BOOL ( STDMETHODCALLTYPE *FCanJitDebug )( 
            IDebugApplication64 * This);
        
        BOOL ( STDMETHODCALLTYPE *FIsAutoJitDebugEnabled )( 
            IDebugApplication64 * This);
        
        HRESULT ( STDMETHODCALLTYPE *AddGlobalExpressionContextProvider )( 
            IDebugApplication64 * This,
            /* [in] */ IProvideExpressionContexts *pdsfs,
            /* [out] */ DWORDLONG *pdwCookie);
        
        HRESULT ( STDMETHODCALLTYPE *RemoveGlobalExpressionContextProvider )( 
            IDebugApplication64 * This,
            /* [in] */ DWORDLONG dwCookie);
        
        END_INTERFACE
    } IDebugApplication64Vtbl;

    interface IDebugApplication64
    {
        CONST_VTBL struct IDebugApplication64Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDebugApplication64_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDebugApplication64_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDebugApplication64_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDebugApplication64_ResumeFromBreakPoint(This,prptFocus,bra,era)	\
    (This)->lpVtbl -> ResumeFromBreakPoint(This,prptFocus,bra,era)

#define IDebugApplication64_CauseBreak(This)	\
    (This)->lpVtbl -> CauseBreak(This)

#define IDebugApplication64_ConnectDebugger(This,pad)	\
    (This)->lpVtbl -> ConnectDebugger(This,pad)

#define IDebugApplication64_DisconnectDebugger(This)	\
    (This)->lpVtbl -> DisconnectDebugger(This)

#define IDebugApplication64_GetDebugger(This,pad)	\
    (This)->lpVtbl -> GetDebugger(This,pad)

#define IDebugApplication64_CreateInstanceAtApplication(This,rclsid,pUnkOuter,dwClsContext,riid,ppvObject)	\
    (This)->lpVtbl -> CreateInstanceAtApplication(This,rclsid,pUnkOuter,dwClsContext,riid,ppvObject)

#define IDebugApplication64_QueryAlive(This)	\
    (This)->lpVtbl -> QueryAlive(This)

#define IDebugApplication64_EnumThreads(This,pperdat)	\
    (This)->lpVtbl -> EnumThreads(This,pperdat)

#define IDebugApplication64_GetName(This,pbstrName)	\
    (This)->lpVtbl -> GetName(This,pbstrName)

#define IDebugApplication64_GetRootNode(This,ppdanRoot)	\
    (This)->lpVtbl -> GetRootNode(This,ppdanRoot)

#define IDebugApplication64_EnumGlobalExpressionContexts(This,ppedec)	\
    (This)->lpVtbl -> EnumGlobalExpressionContexts(This,ppedec)


#define IDebugApplication64_SetName(This,pstrName)	\
    (This)->lpVtbl -> SetName(This,pstrName)

#define IDebugApplication64_StepOutComplete(This)	\
    (This)->lpVtbl -> StepOutComplete(This)

#define IDebugApplication64_DebugOutput(This,pstr)	\
    (This)->lpVtbl -> DebugOutput(This,pstr)

#define IDebugApplication64_StartDebugSession(This)	\
    (This)->lpVtbl -> StartDebugSession(This)

#define IDebugApplication64_HandleBreakPoint(This,br,pbra)	\
    (This)->lpVtbl -> HandleBreakPoint(This,br,pbra)

#define IDebugApplication64_Close(This)	\
    (This)->lpVtbl -> Close(This)

#define IDebugApplication64_GetBreakFlags(This,pabf,pprdatSteppingThread)	\
    (This)->lpVtbl -> GetBreakFlags(This,pabf,pprdatSteppingThread)

#define IDebugApplication64_GetCurrentThread(This,pat)	\
    (This)->lpVtbl -> GetCurrentThread(This,pat)

#define IDebugApplication64_CreateAsyncDebugOperation(This,psdo,ppado)	\
    (This)->lpVtbl -> CreateAsyncDebugOperation(This,psdo,ppado)

#define IDebugApplication64_AddStackFrameSniffer(This,pdsfs,pdwCookie)	\
    (This)->lpVtbl -> AddStackFrameSniffer(This,pdsfs,pdwCookie)

#define IDebugApplication64_RemoveStackFrameSniffer(This,dwCookie)	\
    (This)->lpVtbl -> RemoveStackFrameSniffer(This,dwCookie)

#define IDebugApplication64_QueryCurrentThreadIsDebuggerThread(This)	\
    (This)->lpVtbl -> QueryCurrentThreadIsDebuggerThread(This)

#define IDebugApplication64_SynchronousCallInDebuggerThread(This,pptc,dwParam1,dwParam2,dwParam3)	\
    (This)->lpVtbl -> SynchronousCallInDebuggerThread(This,pptc,dwParam1,dwParam2,dwParam3)

#define IDebugApplication64_CreateApplicationNode(This,ppdanNew)	\
    (This)->lpVtbl -> CreateApplicationNode(This,ppdanNew)

#define IDebugApplication64_FireDebuggerEvent(This,riid,punk)	\
    (This)->lpVtbl -> FireDebuggerEvent(This,riid,punk)

#define IDebugApplication64_HandleRuntimeError(This,pErrorDebug,pScriptSite,pbra,perra,pfCallOnScriptError)	\
    (This)->lpVtbl -> HandleRuntimeError(This,pErrorDebug,pScriptSite,pbra,perra,pfCallOnScriptError)

#define IDebugApplication64_FCanJitDebug(This)	\
    (This)->lpVtbl -> FCanJitDebug(This)

#define IDebugApplication64_FIsAutoJitDebugEnabled(This)	\
    (This)->lpVtbl -> FIsAutoJitDebugEnabled(This)

#define IDebugApplication64_AddGlobalExpressionContextProvider(This,pdsfs,pdwCookie)	\
    (This)->lpVtbl -> AddGlobalExpressionContextProvider(This,pdsfs,pdwCookie)

#define IDebugApplication64_RemoveGlobalExpressionContextProvider(This,dwCookie)	\
    (This)->lpVtbl -> RemoveGlobalExpressionContextProvider(This,dwCookie)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IDebugApplication64_SetName_Proxy( 
    IDebugApplication64 * This,
    /* [in] */ LPCOLESTR pstrName);


void __RPC_STUB IDebugApplication64_SetName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugApplication64_StepOutComplete_Proxy( 
    IDebugApplication64 * This);


void __RPC_STUB IDebugApplication64_StepOutComplete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugApplication64_DebugOutput_Proxy( 
    IDebugApplication64 * This,
    /* [in] */ LPCOLESTR pstr);


void __RPC_STUB IDebugApplication64_DebugOutput_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugApplication64_StartDebugSession_Proxy( 
    IDebugApplication64 * This);


void __RPC_STUB IDebugApplication64_StartDebugSession_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugApplication64_HandleBreakPoint_Proxy( 
    IDebugApplication64 * This,
    /* [in] */ BREAKREASON br,
    /* [out] */ BREAKRESUMEACTION *pbra);


void __RPC_STUB IDebugApplication64_HandleBreakPoint_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugApplication64_Close_Proxy( 
    IDebugApplication64 * This);


void __RPC_STUB IDebugApplication64_Close_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugApplication64_GetBreakFlags_Proxy( 
    IDebugApplication64 * This,
    /* [out] */ APPBREAKFLAGS *pabf,
    /* [out] */ IRemoteDebugApplicationThread **pprdatSteppingThread);


void __RPC_STUB IDebugApplication64_GetBreakFlags_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugApplication64_GetCurrentThread_Proxy( 
    IDebugApplication64 * This,
    /* [out] */ IDebugApplicationThread **pat);


void __RPC_STUB IDebugApplication64_GetCurrentThread_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugApplication64_CreateAsyncDebugOperation_Proxy( 
    IDebugApplication64 * This,
    /* [in] */ IDebugSyncOperation *psdo,
    /* [out] */ IDebugAsyncOperation **ppado);


void __RPC_STUB IDebugApplication64_CreateAsyncDebugOperation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugApplication64_AddStackFrameSniffer_Proxy( 
    IDebugApplication64 * This,
    /* [in] */ IDebugStackFrameSniffer *pdsfs,
    /* [out] */ DWORD *pdwCookie);


void __RPC_STUB IDebugApplication64_AddStackFrameSniffer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugApplication64_RemoveStackFrameSniffer_Proxy( 
    IDebugApplication64 * This,
    /* [in] */ DWORD dwCookie);


void __RPC_STUB IDebugApplication64_RemoveStackFrameSniffer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugApplication64_QueryCurrentThreadIsDebuggerThread_Proxy( 
    IDebugApplication64 * This);


void __RPC_STUB IDebugApplication64_QueryCurrentThreadIsDebuggerThread_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugApplication64_SynchronousCallInDebuggerThread_Proxy( 
    IDebugApplication64 * This,
    /* [in] */ IDebugThreadCall64 *pptc,
    /* [in] */ DWORDLONG dwParam1,
    /* [in] */ DWORDLONG dwParam2,
    /* [in] */ DWORDLONG dwParam3);


void __RPC_STUB IDebugApplication64_SynchronousCallInDebuggerThread_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugApplication64_CreateApplicationNode_Proxy( 
    IDebugApplication64 * This,
    /* [out] */ IDebugApplicationNode **ppdanNew);


void __RPC_STUB IDebugApplication64_CreateApplicationNode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugApplication64_FireDebuggerEvent_Proxy( 
    IDebugApplication64 * This,
    /* [in] */ REFGUID riid,
    /* [in] */ IUnknown *punk);


void __RPC_STUB IDebugApplication64_FireDebuggerEvent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugApplication64_HandleRuntimeError_Proxy( 
    IDebugApplication64 * This,
    /* [in] */ IActiveScriptErrorDebug *pErrorDebug,
    /* [in] */ IActiveScriptSite *pScriptSite,
    /* [out] */ BREAKRESUMEACTION *pbra,
    /* [out] */ ERRORRESUMEACTION *perra,
    /* [out] */ BOOL *pfCallOnScriptError);


void __RPC_STUB IDebugApplication64_HandleRuntimeError_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


BOOL STDMETHODCALLTYPE IDebugApplication64_FCanJitDebug_Proxy( 
    IDebugApplication64 * This);


void __RPC_STUB IDebugApplication64_FCanJitDebug_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


BOOL STDMETHODCALLTYPE IDebugApplication64_FIsAutoJitDebugEnabled_Proxy( 
    IDebugApplication64 * This);


void __RPC_STUB IDebugApplication64_FIsAutoJitDebugEnabled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugApplication64_AddGlobalExpressionContextProvider_Proxy( 
    IDebugApplication64 * This,
    /* [in] */ IProvideExpressionContexts *pdsfs,
    /* [out] */ DWORDLONG *pdwCookie);


void __RPC_STUB IDebugApplication64_AddGlobalExpressionContextProvider_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugApplication64_RemoveGlobalExpressionContextProvider_Proxy( 
    IDebugApplication64 * This,
    /* [in] */ DWORDLONG dwCookie);


void __RPC_STUB IDebugApplication64_RemoveGlobalExpressionContextProvider_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IDebugApplication64_INTERFACE_DEFINED__ */


#ifndef __IRemoteDebugApplicationEvents_INTERFACE_DEFINED__
#define __IRemoteDebugApplicationEvents_INTERFACE_DEFINED__

/* interface IRemoteDebugApplicationEvents */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IRemoteDebugApplicationEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("51973C33-CB0C-11d0-B5C9-00A0244A0E7A")
    IRemoteDebugApplicationEvents : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE OnConnectDebugger( 
            /* [in] */ IApplicationDebugger *pad) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnDisconnectDebugger( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnSetName( 
            /* [in] */ LPCOLESTR pstrName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnDebugOutput( 
            /* [in] */ LPCOLESTR pstr) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnClose( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnEnterBreakPoint( 
            /* [in] */ IRemoteDebugApplicationThread *prdat) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnLeaveBreakPoint( 
            /* [in] */ IRemoteDebugApplicationThread *prdat) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnCreateThread( 
            /* [in] */ IRemoteDebugApplicationThread *prdat) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnDestroyThread( 
            /* [in] */ IRemoteDebugApplicationThread *prdat) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnBreakFlagChange( 
            /* [in] */ APPBREAKFLAGS abf,
            /* [in] */ IRemoteDebugApplicationThread *prdatSteppingThread) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IRemoteDebugApplicationEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRemoteDebugApplicationEvents * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRemoteDebugApplicationEvents * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRemoteDebugApplicationEvents * This);
        
        HRESULT ( STDMETHODCALLTYPE *OnConnectDebugger )( 
            IRemoteDebugApplicationEvents * This,
            /* [in] */ IApplicationDebugger *pad);
        
        HRESULT ( STDMETHODCALLTYPE *OnDisconnectDebugger )( 
            IRemoteDebugApplicationEvents * This);
        
        HRESULT ( STDMETHODCALLTYPE *OnSetName )( 
            IRemoteDebugApplicationEvents * This,
            /* [in] */ LPCOLESTR pstrName);
        
        HRESULT ( STDMETHODCALLTYPE *OnDebugOutput )( 
            IRemoteDebugApplicationEvents * This,
            /* [in] */ LPCOLESTR pstr);
        
        HRESULT ( STDMETHODCALLTYPE *OnClose )( 
            IRemoteDebugApplicationEvents * This);
        
        HRESULT ( STDMETHODCALLTYPE *OnEnterBreakPoint )( 
            IRemoteDebugApplicationEvents * This,
            /* [in] */ IRemoteDebugApplicationThread *prdat);
        
        HRESULT ( STDMETHODCALLTYPE *OnLeaveBreakPoint )( 
            IRemoteDebugApplicationEvents * This,
            /* [in] */ IRemoteDebugApplicationThread *prdat);
        
        HRESULT ( STDMETHODCALLTYPE *OnCreateThread )( 
            IRemoteDebugApplicationEvents * This,
            /* [in] */ IRemoteDebugApplicationThread *prdat);
        
        HRESULT ( STDMETHODCALLTYPE *OnDestroyThread )( 
            IRemoteDebugApplicationEvents * This,
            /* [in] */ IRemoteDebugApplicationThread *prdat);
        
        HRESULT ( STDMETHODCALLTYPE *OnBreakFlagChange )( 
            IRemoteDebugApplicationEvents * This,
            /* [in] */ APPBREAKFLAGS abf,
            /* [in] */ IRemoteDebugApplicationThread *prdatSteppingThread);
        
        END_INTERFACE
    } IRemoteDebugApplicationEventsVtbl;

    interface IRemoteDebugApplicationEvents
    {
        CONST_VTBL struct IRemoteDebugApplicationEventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRemoteDebugApplicationEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRemoteDebugApplicationEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRemoteDebugApplicationEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRemoteDebugApplicationEvents_OnConnectDebugger(This,pad)	\
    (This)->lpVtbl -> OnConnectDebugger(This,pad)

#define IRemoteDebugApplicationEvents_OnDisconnectDebugger(This)	\
    (This)->lpVtbl -> OnDisconnectDebugger(This)

#define IRemoteDebugApplicationEvents_OnSetName(This,pstrName)	\
    (This)->lpVtbl -> OnSetName(This,pstrName)

#define IRemoteDebugApplicationEvents_OnDebugOutput(This,pstr)	\
    (This)->lpVtbl -> OnDebugOutput(This,pstr)

#define IRemoteDebugApplicationEvents_OnClose(This)	\
    (This)->lpVtbl -> OnClose(This)

#define IRemoteDebugApplicationEvents_OnEnterBreakPoint(This,prdat)	\
    (This)->lpVtbl -> OnEnterBreakPoint(This,prdat)

#define IRemoteDebugApplicationEvents_OnLeaveBreakPoint(This,prdat)	\
    (This)->lpVtbl -> OnLeaveBreakPoint(This,prdat)

#define IRemoteDebugApplicationEvents_OnCreateThread(This,prdat)	\
    (This)->lpVtbl -> OnCreateThread(This,prdat)

#define IRemoteDebugApplicationEvents_OnDestroyThread(This,prdat)	\
    (This)->lpVtbl -> OnDestroyThread(This,prdat)

#define IRemoteDebugApplicationEvents_OnBreakFlagChange(This,abf,prdatSteppingThread)	\
    (This)->lpVtbl -> OnBreakFlagChange(This,abf,prdatSteppingThread)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IRemoteDebugApplicationEvents_OnConnectDebugger_Proxy( 
    IRemoteDebugApplicationEvents * This,
    /* [in] */ IApplicationDebugger *pad);


void __RPC_STUB IRemoteDebugApplicationEvents_OnConnectDebugger_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRemoteDebugApplicationEvents_OnDisconnectDebugger_Proxy( 
    IRemoteDebugApplicationEvents * This);


void __RPC_STUB IRemoteDebugApplicationEvents_OnDisconnectDebugger_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRemoteDebugApplicationEvents_OnSetName_Proxy( 
    IRemoteDebugApplicationEvents * This,
    /* [in] */ LPCOLESTR pstrName);


void __RPC_STUB IRemoteDebugApplicationEvents_OnSetName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRemoteDebugApplicationEvents_OnDebugOutput_Proxy( 
    IRemoteDebugApplicationEvents * This,
    /* [in] */ LPCOLESTR pstr);


void __RPC_STUB IRemoteDebugApplicationEvents_OnDebugOutput_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRemoteDebugApplicationEvents_OnClose_Proxy( 
    IRemoteDebugApplicationEvents * This);


void __RPC_STUB IRemoteDebugApplicationEvents_OnClose_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRemoteDebugApplicationEvents_OnEnterBreakPoint_Proxy( 
    IRemoteDebugApplicationEvents * This,
    /* [in] */ IRemoteDebugApplicationThread *prdat);


void __RPC_STUB IRemoteDebugApplicationEvents_OnEnterBreakPoint_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRemoteDebugApplicationEvents_OnLeaveBreakPoint_Proxy( 
    IRemoteDebugApplicationEvents * This,
    /* [in] */ IRemoteDebugApplicationThread *prdat);


void __RPC_STUB IRemoteDebugApplicationEvents_OnLeaveBreakPoint_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRemoteDebugApplicationEvents_OnCreateThread_Proxy( 
    IRemoteDebugApplicationEvents * This,
    /* [in] */ IRemoteDebugApplicationThread *prdat);


void __RPC_STUB IRemoteDebugApplicationEvents_OnCreateThread_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRemoteDebugApplicationEvents_OnDestroyThread_Proxy( 
    IRemoteDebugApplicationEvents * This,
    /* [in] */ IRemoteDebugApplicationThread *prdat);


void __RPC_STUB IRemoteDebugApplicationEvents_OnDestroyThread_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRemoteDebugApplicationEvents_OnBreakFlagChange_Proxy( 
    IRemoteDebugApplicationEvents * This,
    /* [in] */ APPBREAKFLAGS abf,
    /* [in] */ IRemoteDebugApplicationThread *prdatSteppingThread);


void __RPC_STUB IRemoteDebugApplicationEvents_OnBreakFlagChange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IRemoteDebugApplicationEvents_INTERFACE_DEFINED__ */


#ifndef __IDebugApplicationNode_INTERFACE_DEFINED__
#define __IDebugApplicationNode_INTERFACE_DEFINED__

/* interface IDebugApplicationNode */
/* [unique][uuid][object] */ 


EXTERN_C const IID IID_IDebugApplicationNode;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("51973C34-CB0C-11d0-B5C9-00A0244A0E7A")
    IDebugApplicationNode : public IDebugDocumentProvider
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE EnumChildren( 
            /* [out] */ IEnumDebugApplicationNodes **pperddp) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetParent( 
            /* [out] */ IDebugApplicationNode **pprddp) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetDocumentProvider( 
            /* [in] */ IDebugDocumentProvider *pddp) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Close( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Attach( 
            /* [in] */ IDebugApplicationNode *pdanParent) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Detach( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDebugApplicationNodeVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDebugApplicationNode * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDebugApplicationNode * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDebugApplicationNode * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetName )( 
            IDebugApplicationNode * This,
            /* [in] */ DOCUMENTNAMETYPE dnt,
            /* [out] */ BSTR *pbstrName);
        
        HRESULT ( STDMETHODCALLTYPE *GetDocumentClassId )( 
            IDebugApplicationNode * This,
            /* [out] */ CLSID *pclsidDocument);
        
        HRESULT ( STDMETHODCALLTYPE *GetDocument )( 
            IDebugApplicationNode * This,
            /* [out] */ IDebugDocument **ppssd);
        
        HRESULT ( STDMETHODCALLTYPE *EnumChildren )( 
            IDebugApplicationNode * This,
            /* [out] */ IEnumDebugApplicationNodes **pperddp);
        
        HRESULT ( STDMETHODCALLTYPE *GetParent )( 
            IDebugApplicationNode * This,
            /* [out] */ IDebugApplicationNode **pprddp);
        
        HRESULT ( STDMETHODCALLTYPE *SetDocumentProvider )( 
            IDebugApplicationNode * This,
            /* [in] */ IDebugDocumentProvider *pddp);
        
        HRESULT ( STDMETHODCALLTYPE *Close )( 
            IDebugApplicationNode * This);
        
        HRESULT ( STDMETHODCALLTYPE *Attach )( 
            IDebugApplicationNode * This,
            /* [in] */ IDebugApplicationNode *pdanParent);
        
        HRESULT ( STDMETHODCALLTYPE *Detach )( 
            IDebugApplicationNode * This);
        
        END_INTERFACE
    } IDebugApplicationNodeVtbl;

    interface IDebugApplicationNode
    {
        CONST_VTBL struct IDebugApplicationNodeVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDebugApplicationNode_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDebugApplicationNode_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDebugApplicationNode_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDebugApplicationNode_GetName(This,dnt,pbstrName)	\
    (This)->lpVtbl -> GetName(This,dnt,pbstrName)

#define IDebugApplicationNode_GetDocumentClassId(This,pclsidDocument)	\
    (This)->lpVtbl -> GetDocumentClassId(This,pclsidDocument)


#define IDebugApplicationNode_GetDocument(This,ppssd)	\
    (This)->lpVtbl -> GetDocument(This,ppssd)


#define IDebugApplicationNode_EnumChildren(This,pperddp)	\
    (This)->lpVtbl -> EnumChildren(This,pperddp)

#define IDebugApplicationNode_GetParent(This,pprddp)	\
    (This)->lpVtbl -> GetParent(This,pprddp)

#define IDebugApplicationNode_SetDocumentProvider(This,pddp)	\
    (This)->lpVtbl -> SetDocumentProvider(This,pddp)

#define IDebugApplicationNode_Close(This)	\
    (This)->lpVtbl -> Close(This)

#define IDebugApplicationNode_Attach(This,pdanParent)	\
    (This)->lpVtbl -> Attach(This,pdanParent)

#define IDebugApplicationNode_Detach(This)	\
    (This)->lpVtbl -> Detach(This)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IDebugApplicationNode_EnumChildren_Proxy( 
    IDebugApplicationNode * This,
    /* [out] */ IEnumDebugApplicationNodes **pperddp);


void __RPC_STUB IDebugApplicationNode_EnumChildren_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugApplicationNode_GetParent_Proxy( 
    IDebugApplicationNode * This,
    /* [out] */ IDebugApplicationNode **pprddp);


void __RPC_STUB IDebugApplicationNode_GetParent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugApplicationNode_SetDocumentProvider_Proxy( 
    IDebugApplicationNode * This,
    /* [in] */ IDebugDocumentProvider *pddp);


void __RPC_STUB IDebugApplicationNode_SetDocumentProvider_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugApplicationNode_Close_Proxy( 
    IDebugApplicationNode * This);


void __RPC_STUB IDebugApplicationNode_Close_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugApplicationNode_Attach_Proxy( 
    IDebugApplicationNode * This,
    /* [in] */ IDebugApplicationNode *pdanParent);


void __RPC_STUB IDebugApplicationNode_Attach_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugApplicationNode_Detach_Proxy( 
    IDebugApplicationNode * This);


void __RPC_STUB IDebugApplicationNode_Detach_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IDebugApplicationNode_INTERFACE_DEFINED__ */


#ifndef __IDebugApplicationNodeEvents_INTERFACE_DEFINED__
#define __IDebugApplicationNodeEvents_INTERFACE_DEFINED__

/* interface IDebugApplicationNodeEvents */
/* [unique][uuid][object] */ 


EXTERN_C const IID IID_IDebugApplicationNodeEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("51973C35-CB0C-11d0-B5C9-00A0244A0E7A")
    IDebugApplicationNodeEvents : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE onAddChild( 
            /* [in] */ IDebugApplicationNode *prddpChild) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE onRemoveChild( 
            /* [in] */ IDebugApplicationNode *prddpChild) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE onDetach( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE onAttach( 
            /* [in] */ IDebugApplicationNode *prddpParent) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDebugApplicationNodeEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDebugApplicationNodeEvents * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDebugApplicationNodeEvents * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDebugApplicationNodeEvents * This);
        
        HRESULT ( STDMETHODCALLTYPE *onAddChild )( 
            IDebugApplicationNodeEvents * This,
            /* [in] */ IDebugApplicationNode *prddpChild);
        
        HRESULT ( STDMETHODCALLTYPE *onRemoveChild )( 
            IDebugApplicationNodeEvents * This,
            /* [in] */ IDebugApplicationNode *prddpChild);
        
        HRESULT ( STDMETHODCALLTYPE *onDetach )( 
            IDebugApplicationNodeEvents * This);
        
        HRESULT ( STDMETHODCALLTYPE *onAttach )( 
            IDebugApplicationNodeEvents * This,
            /* [in] */ IDebugApplicationNode *prddpParent);
        
        END_INTERFACE
    } IDebugApplicationNodeEventsVtbl;

    interface IDebugApplicationNodeEvents
    {
        CONST_VTBL struct IDebugApplicationNodeEventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDebugApplicationNodeEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDebugApplicationNodeEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDebugApplicationNodeEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDebugApplicationNodeEvents_onAddChild(This,prddpChild)	\
    (This)->lpVtbl -> onAddChild(This,prddpChild)

#define IDebugApplicationNodeEvents_onRemoveChild(This,prddpChild)	\
    (This)->lpVtbl -> onRemoveChild(This,prddpChild)

#define IDebugApplicationNodeEvents_onDetach(This)	\
    (This)->lpVtbl -> onDetach(This)

#define IDebugApplicationNodeEvents_onAttach(This,prddpParent)	\
    (This)->lpVtbl -> onAttach(This,prddpParent)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IDebugApplicationNodeEvents_onAddChild_Proxy( 
    IDebugApplicationNodeEvents * This,
    /* [in] */ IDebugApplicationNode *prddpChild);


void __RPC_STUB IDebugApplicationNodeEvents_onAddChild_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugApplicationNodeEvents_onRemoveChild_Proxy( 
    IDebugApplicationNodeEvents * This,
    /* [in] */ IDebugApplicationNode *prddpChild);


void __RPC_STUB IDebugApplicationNodeEvents_onRemoveChild_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugApplicationNodeEvents_onDetach_Proxy( 
    IDebugApplicationNodeEvents * This);


void __RPC_STUB IDebugApplicationNodeEvents_onDetach_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugApplicationNodeEvents_onAttach_Proxy( 
    IDebugApplicationNodeEvents * This,
    /* [in] */ IDebugApplicationNode *prddpParent);


void __RPC_STUB IDebugApplicationNodeEvents_onAttach_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IDebugApplicationNodeEvents_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_activdbg_0347 */
/* [local] */ 

#ifndef DISABLE_ACTIVDBG_INTERFACE_WRAPPERS
#ifdef _WIN64
#define IDebugThreadCall IDebugThreadCall64
#define IID_IDebugThreadCall IID_IDebugThreadCall64
#else
#define IDebugThreadCall IDebugThreadCall32
#define IID_IDebugThreadCall IID_IDebugThreadCall32
#endif
#endif


extern RPC_IF_HANDLE __MIDL_itf_activdbg_0347_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_activdbg_0347_v0_0_s_ifspec;

#ifndef __IDebugThreadCall32_INTERFACE_DEFINED__
#define __IDebugThreadCall32_INTERFACE_DEFINED__

/* interface IDebugThreadCall32 */
/* [local][unique][uuid][object] */ 


EXTERN_C const IID IID_IDebugThreadCall32;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("51973C36-CB0C-11d0-B5C9-00A0244A0E7A")
    IDebugThreadCall32 : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ThreadCallHandler( 
            /* [in] */ DWORD dwParam1,
            /* [in] */ DWORD dwParam2,
            /* [in] */ DWORD dwParam3) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDebugThreadCall32Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDebugThreadCall32 * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDebugThreadCall32 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDebugThreadCall32 * This);
        
        HRESULT ( STDMETHODCALLTYPE *ThreadCallHandler )( 
            IDebugThreadCall32 * This,
            /* [in] */ DWORD dwParam1,
            /* [in] */ DWORD dwParam2,
            /* [in] */ DWORD dwParam3);
        
        END_INTERFACE
    } IDebugThreadCall32Vtbl;

    interface IDebugThreadCall32
    {
        CONST_VTBL struct IDebugThreadCall32Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDebugThreadCall32_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDebugThreadCall32_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDebugThreadCall32_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDebugThreadCall32_ThreadCallHandler(This,dwParam1,dwParam2,dwParam3)	\
    (This)->lpVtbl -> ThreadCallHandler(This,dwParam1,dwParam2,dwParam3)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IDebugThreadCall32_ThreadCallHandler_Proxy( 
    IDebugThreadCall32 * This,
    /* [in] */ DWORD dwParam1,
    /* [in] */ DWORD dwParam2,
    /* [in] */ DWORD dwParam3);


void __RPC_STUB IDebugThreadCall32_ThreadCallHandler_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IDebugThreadCall32_INTERFACE_DEFINED__ */


#ifndef __IDebugThreadCall64_INTERFACE_DEFINED__
#define __IDebugThreadCall64_INTERFACE_DEFINED__

/* interface IDebugThreadCall64 */
/* [local][unique][uuid][object] */ 


EXTERN_C const IID IID_IDebugThreadCall64;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("cb3fa335-e979-42fd-9fcf-a7546a0f3905")
    IDebugThreadCall64 : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ThreadCallHandler( 
            /* [in] */ DWORDLONG dwParam1,
            /* [in] */ DWORDLONG dwParam2,
            /* [in] */ DWORDLONG dwParam3) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDebugThreadCall64Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDebugThreadCall64 * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDebugThreadCall64 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDebugThreadCall64 * This);
        
        HRESULT ( STDMETHODCALLTYPE *ThreadCallHandler )( 
            IDebugThreadCall64 * This,
            /* [in] */ DWORDLONG dwParam1,
            /* [in] */ DWORDLONG dwParam2,
            /* [in] */ DWORDLONG dwParam3);
        
        END_INTERFACE
    } IDebugThreadCall64Vtbl;

    interface IDebugThreadCall64
    {
        CONST_VTBL struct IDebugThreadCall64Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDebugThreadCall64_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDebugThreadCall64_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDebugThreadCall64_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDebugThreadCall64_ThreadCallHandler(This,dwParam1,dwParam2,dwParam3)	\
    (This)->lpVtbl -> ThreadCallHandler(This,dwParam1,dwParam2,dwParam3)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IDebugThreadCall64_ThreadCallHandler_Proxy( 
    IDebugThreadCall64 * This,
    /* [in] */ DWORDLONG dwParam1,
    /* [in] */ DWORDLONG dwParam2,
    /* [in] */ DWORDLONG dwParam3);


void __RPC_STUB IDebugThreadCall64_ThreadCallHandler_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IDebugThreadCall64_INTERFACE_DEFINED__ */


#ifndef __IRemoteDebugApplicationThread_INTERFACE_DEFINED__
#define __IRemoteDebugApplicationThread_INTERFACE_DEFINED__

/* interface IRemoteDebugApplicationThread */
/* [unique][uuid][object] */ 

typedef DWORD THREAD_STATE;

#define	THREAD_STATE_RUNNING	( 0x1 )

#define	THREAD_STATE_SUSPENDED	( 0x2 )

#define	THREAD_BLOCKED	( 0x4 )

#define	THREAD_OUT_OF_CONTEXT	( 0x8 )


EXTERN_C const IID IID_IRemoteDebugApplicationThread;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("51973C37-CB0C-11d0-B5C9-00A0244A0E7A")
    IRemoteDebugApplicationThread : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetSystemThreadId( 
            /* [out] */ DWORD *dwThreadId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetApplication( 
            /* [out] */ IRemoteDebugApplication **pprda) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumStackFrames( 
            /* [out] */ IEnumDebugStackFrames **ppedsf) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDescription( 
            /* [out] */ BSTR *pbstrDescription,
            /* [out] */ BSTR *pbstrState) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetNextStatement( 
            /* [in] */ IDebugStackFrame *pStackFrame,
            /* [in] */ IDebugCodeContext *pCodeContext) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetState( 
            /* [out] */ DWORD *pState) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Suspend( 
            /* [out] */ DWORD *pdwCount) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Resume( 
            /* [out] */ DWORD *pdwCount) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSuspendCount( 
            /* [out] */ DWORD *pdwCount) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IRemoteDebugApplicationThreadVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRemoteDebugApplicationThread * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRemoteDebugApplicationThread * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRemoteDebugApplicationThread * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetSystemThreadId )( 
            IRemoteDebugApplicationThread * This,
            /* [out] */ DWORD *dwThreadId);
        
        HRESULT ( STDMETHODCALLTYPE *GetApplication )( 
            IRemoteDebugApplicationThread * This,
            /* [out] */ IRemoteDebugApplication **pprda);
        
        HRESULT ( STDMETHODCALLTYPE *EnumStackFrames )( 
            IRemoteDebugApplicationThread * This,
            /* [out] */ IEnumDebugStackFrames **ppedsf);
        
        HRESULT ( STDMETHODCALLTYPE *GetDescription )( 
            IRemoteDebugApplicationThread * This,
            /* [out] */ BSTR *pbstrDescription,
            /* [out] */ BSTR *pbstrState);
        
        HRESULT ( STDMETHODCALLTYPE *SetNextStatement )( 
            IRemoteDebugApplicationThread * This,
            /* [in] */ IDebugStackFrame *pStackFrame,
            /* [in] */ IDebugCodeContext *pCodeContext);
        
        HRESULT ( STDMETHODCALLTYPE *GetState )( 
            IRemoteDebugApplicationThread * This,
            /* [out] */ DWORD *pState);
        
        HRESULT ( STDMETHODCALLTYPE *Suspend )( 
            IRemoteDebugApplicationThread * This,
            /* [out] */ DWORD *pdwCount);
        
        HRESULT ( STDMETHODCALLTYPE *Resume )( 
            IRemoteDebugApplicationThread * This,
            /* [out] */ DWORD *pdwCount);
        
        HRESULT ( STDMETHODCALLTYPE *GetSuspendCount )( 
            IRemoteDebugApplicationThread * This,
            /* [out] */ DWORD *pdwCount);
        
        END_INTERFACE
    } IRemoteDebugApplicationThreadVtbl;

    interface IRemoteDebugApplicationThread
    {
        CONST_VTBL struct IRemoteDebugApplicationThreadVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRemoteDebugApplicationThread_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRemoteDebugApplicationThread_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRemoteDebugApplicationThread_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRemoteDebugApplicationThread_GetSystemThreadId(This,dwThreadId)	\
    (This)->lpVtbl -> GetSystemThreadId(This,dwThreadId)

#define IRemoteDebugApplicationThread_GetApplication(This,pprda)	\
    (This)->lpVtbl -> GetApplication(This,pprda)

#define IRemoteDebugApplicationThread_EnumStackFrames(This,ppedsf)	\
    (This)->lpVtbl -> EnumStackFrames(This,ppedsf)

#define IRemoteDebugApplicationThread_GetDescription(This,pbstrDescription,pbstrState)	\
    (This)->lpVtbl -> GetDescription(This,pbstrDescription,pbstrState)

#define IRemoteDebugApplicationThread_SetNextStatement(This,pStackFrame,pCodeContext)	\
    (This)->lpVtbl -> SetNextStatement(This,pStackFrame,pCodeContext)

#define IRemoteDebugApplicationThread_GetState(This,pState)	\
    (This)->lpVtbl -> GetState(This,pState)

#define IRemoteDebugApplicationThread_Suspend(This,pdwCount)	\
    (This)->lpVtbl -> Suspend(This,pdwCount)

#define IRemoteDebugApplicationThread_Resume(This,pdwCount)	\
    (This)->lpVtbl -> Resume(This,pdwCount)

#define IRemoteDebugApplicationThread_GetSuspendCount(This,pdwCount)	\
    (This)->lpVtbl -> GetSuspendCount(This,pdwCount)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IRemoteDebugApplicationThread_GetSystemThreadId_Proxy( 
    IRemoteDebugApplicationThread * This,
    /* [out] */ DWORD *dwThreadId);


void __RPC_STUB IRemoteDebugApplicationThread_GetSystemThreadId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRemoteDebugApplicationThread_GetApplication_Proxy( 
    IRemoteDebugApplicationThread * This,
    /* [out] */ IRemoteDebugApplication **pprda);


void __RPC_STUB IRemoteDebugApplicationThread_GetApplication_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRemoteDebugApplicationThread_EnumStackFrames_Proxy( 
    IRemoteDebugApplicationThread * This,
    /* [out] */ IEnumDebugStackFrames **ppedsf);


void __RPC_STUB IRemoteDebugApplicationThread_EnumStackFrames_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRemoteDebugApplicationThread_GetDescription_Proxy( 
    IRemoteDebugApplicationThread * This,
    /* [out] */ BSTR *pbstrDescription,
    /* [out] */ BSTR *pbstrState);


void __RPC_STUB IRemoteDebugApplicationThread_GetDescription_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRemoteDebugApplicationThread_SetNextStatement_Proxy( 
    IRemoteDebugApplicationThread * This,
    /* [in] */ IDebugStackFrame *pStackFrame,
    /* [in] */ IDebugCodeContext *pCodeContext);


void __RPC_STUB IRemoteDebugApplicationThread_SetNextStatement_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRemoteDebugApplicationThread_GetState_Proxy( 
    IRemoteDebugApplicationThread * This,
    /* [out] */ DWORD *pState);


void __RPC_STUB IRemoteDebugApplicationThread_GetState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRemoteDebugApplicationThread_Suspend_Proxy( 
    IRemoteDebugApplicationThread * This,
    /* [out] */ DWORD *pdwCount);


void __RPC_STUB IRemoteDebugApplicationThread_Suspend_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRemoteDebugApplicationThread_Resume_Proxy( 
    IRemoteDebugApplicationThread * This,
    /* [out] */ DWORD *pdwCount);


void __RPC_STUB IRemoteDebugApplicationThread_Resume_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRemoteDebugApplicationThread_GetSuspendCount_Proxy( 
    IRemoteDebugApplicationThread * This,
    /* [out] */ DWORD *pdwCount);


void __RPC_STUB IRemoteDebugApplicationThread_GetSuspendCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IRemoteDebugApplicationThread_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_activdbg_0350 */
/* [local] */ 

#ifndef DISABLE_ACTIVDBG_INTERFACE_WRAPPERS
#ifdef _WIN64
#define SynchronousCallIntoThread SynchronousCallIntoThread64
#else
#define SynchronousCallIntoThread SynchronousCallIntoThread32
#endif
#endif


extern RPC_IF_HANDLE __MIDL_itf_activdbg_0350_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_activdbg_0350_v0_0_s_ifspec;

#ifndef __IDebugApplicationThread_INTERFACE_DEFINED__
#define __IDebugApplicationThread_INTERFACE_DEFINED__

/* interface IDebugApplicationThread */
/* [local][unique][uuid][object] */ 


EXTERN_C const IID IID_IDebugApplicationThread;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("51973C38-CB0C-11d0-B5C9-00A0244A0E7A")
    IDebugApplicationThread : public IRemoteDebugApplicationThread
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SynchronousCallIntoThread32( 
            /* [in] */ IDebugThreadCall32 *pstcb,
            /* [in] */ DWORD dwParam1,
            /* [in] */ DWORD dwParam2,
            /* [in] */ DWORD dwParam3) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE QueryIsCurrentThread( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE QueryIsDebuggerThread( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetDescription( 
            /* [in] */ LPCOLESTR pstrDescription) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetStateString( 
            /* [in] */ LPCOLESTR pstrState) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDebugApplicationThreadVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDebugApplicationThread * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDebugApplicationThread * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDebugApplicationThread * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetSystemThreadId )( 
            IDebugApplicationThread * This,
            /* [out] */ DWORD *dwThreadId);
        
        HRESULT ( STDMETHODCALLTYPE *GetApplication )( 
            IDebugApplicationThread * This,
            /* [out] */ IRemoteDebugApplication **pprda);
        
        HRESULT ( STDMETHODCALLTYPE *EnumStackFrames )( 
            IDebugApplicationThread * This,
            /* [out] */ IEnumDebugStackFrames **ppedsf);
        
        HRESULT ( STDMETHODCALLTYPE *GetDescription )( 
            IDebugApplicationThread * This,
            /* [out] */ BSTR *pbstrDescription,
            /* [out] */ BSTR *pbstrState);
        
        HRESULT ( STDMETHODCALLTYPE *SetNextStatement )( 
            IDebugApplicationThread * This,
            /* [in] */ IDebugStackFrame *pStackFrame,
            /* [in] */ IDebugCodeContext *pCodeContext);
        
        HRESULT ( STDMETHODCALLTYPE *GetState )( 
            IDebugApplicationThread * This,
            /* [out] */ DWORD *pState);
        
        HRESULT ( STDMETHODCALLTYPE *Suspend )( 
            IDebugApplicationThread * This,
            /* [out] */ DWORD *pdwCount);
        
        HRESULT ( STDMETHODCALLTYPE *Resume )( 
            IDebugApplicationThread * This,
            /* [out] */ DWORD *pdwCount);
        
        HRESULT ( STDMETHODCALLTYPE *GetSuspendCount )( 
            IDebugApplicationThread * This,
            /* [out] */ DWORD *pdwCount);
        
        HRESULT ( STDMETHODCALLTYPE *SynchronousCallIntoThread32 )( 
            IDebugApplicationThread * This,
            /* [in] */ IDebugThreadCall32 *pstcb,
            /* [in] */ DWORD dwParam1,
            /* [in] */ DWORD dwParam2,
            /* [in] */ DWORD dwParam3);
        
        HRESULT ( STDMETHODCALLTYPE *QueryIsCurrentThread )( 
            IDebugApplicationThread * This);
        
        HRESULT ( STDMETHODCALLTYPE *QueryIsDebuggerThread )( 
            IDebugApplicationThread * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetDescription )( 
            IDebugApplicationThread * This,
            /* [in] */ LPCOLESTR pstrDescription);
        
        HRESULT ( STDMETHODCALLTYPE *SetStateString )( 
            IDebugApplicationThread * This,
            /* [in] */ LPCOLESTR pstrState);
        
        END_INTERFACE
    } IDebugApplicationThreadVtbl;

    interface IDebugApplicationThread
    {
        CONST_VTBL struct IDebugApplicationThreadVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDebugApplicationThread_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDebugApplicationThread_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDebugApplicationThread_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDebugApplicationThread_GetSystemThreadId(This,dwThreadId)	\
    (This)->lpVtbl -> GetSystemThreadId(This,dwThreadId)

#define IDebugApplicationThread_GetApplication(This,pprda)	\
    (This)->lpVtbl -> GetApplication(This,pprda)

#define IDebugApplicationThread_EnumStackFrames(This,ppedsf)	\
    (This)->lpVtbl -> EnumStackFrames(This,ppedsf)

#define IDebugApplicationThread_GetDescription(This,pbstrDescription,pbstrState)	\
    (This)->lpVtbl -> GetDescription(This,pbstrDescription,pbstrState)

#define IDebugApplicationThread_SetNextStatement(This,pStackFrame,pCodeContext)	\
    (This)->lpVtbl -> SetNextStatement(This,pStackFrame,pCodeContext)

#define IDebugApplicationThread_GetState(This,pState)	\
    (This)->lpVtbl -> GetState(This,pState)

#define IDebugApplicationThread_Suspend(This,pdwCount)	\
    (This)->lpVtbl -> Suspend(This,pdwCount)

#define IDebugApplicationThread_Resume(This,pdwCount)	\
    (This)->lpVtbl -> Resume(This,pdwCount)

#define IDebugApplicationThread_GetSuspendCount(This,pdwCount)	\
    (This)->lpVtbl -> GetSuspendCount(This,pdwCount)


#define IDebugApplicationThread_SynchronousCallIntoThread32(This,pstcb,dwParam1,dwParam2,dwParam3)	\
    (This)->lpVtbl -> SynchronousCallIntoThread32(This,pstcb,dwParam1,dwParam2,dwParam3)

#define IDebugApplicationThread_QueryIsCurrentThread(This)	\
    (This)->lpVtbl -> QueryIsCurrentThread(This)

#define IDebugApplicationThread_QueryIsDebuggerThread(This)	\
    (This)->lpVtbl -> QueryIsDebuggerThread(This)

#define IDebugApplicationThread_SetDescription(This,pstrDescription)	\
    (This)->lpVtbl -> SetDescription(This,pstrDescription)

#define IDebugApplicationThread_SetStateString(This,pstrState)	\
    (This)->lpVtbl -> SetStateString(This,pstrState)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IDebugApplicationThread_SynchronousCallIntoThread32_Proxy( 
    IDebugApplicationThread * This,
    /* [in] */ IDebugThreadCall32 *pstcb,
    /* [in] */ DWORD dwParam1,
    /* [in] */ DWORD dwParam2,
    /* [in] */ DWORD dwParam3);


void __RPC_STUB IDebugApplicationThread_SynchronousCallIntoThread32_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugApplicationThread_QueryIsCurrentThread_Proxy( 
    IDebugApplicationThread * This);


void __RPC_STUB IDebugApplicationThread_QueryIsCurrentThread_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugApplicationThread_QueryIsDebuggerThread_Proxy( 
    IDebugApplicationThread * This);


void __RPC_STUB IDebugApplicationThread_QueryIsDebuggerThread_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugApplicationThread_SetDescription_Proxy( 
    IDebugApplicationThread * This,
    /* [in] */ LPCOLESTR pstrDescription);


void __RPC_STUB IDebugApplicationThread_SetDescription_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugApplicationThread_SetStateString_Proxy( 
    IDebugApplicationThread * This,
    /* [in] */ LPCOLESTR pstrState);


void __RPC_STUB IDebugApplicationThread_SetStateString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IDebugApplicationThread_INTERFACE_DEFINED__ */


#ifndef __IDebugApplicationThread64_INTERFACE_DEFINED__
#define __IDebugApplicationThread64_INTERFACE_DEFINED__

/* interface IDebugApplicationThread64 */
/* [local][unique][uuid][object] */ 


EXTERN_C const IID IID_IDebugApplicationThread64;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("9dac5886-dbad-456d-9dee-5dec39ab3dda")
    IDebugApplicationThread64 : public IDebugApplicationThread
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SynchronousCallIntoThread64( 
            /* [in] */ IDebugThreadCall64 *pstcb,
            /* [in] */ DWORDLONG dwParam1,
            /* [in] */ DWORDLONG dwParam2,
            /* [in] */ DWORDLONG dwParam3) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDebugApplicationThread64Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDebugApplicationThread64 * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDebugApplicationThread64 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDebugApplicationThread64 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetSystemThreadId )( 
            IDebugApplicationThread64 * This,
            /* [out] */ DWORD *dwThreadId);
        
        HRESULT ( STDMETHODCALLTYPE *GetApplication )( 
            IDebugApplicationThread64 * This,
            /* [out] */ IRemoteDebugApplication **pprda);
        
        HRESULT ( STDMETHODCALLTYPE *EnumStackFrames )( 
            IDebugApplicationThread64 * This,
            /* [out] */ IEnumDebugStackFrames **ppedsf);
        
        HRESULT ( STDMETHODCALLTYPE *GetDescription )( 
            IDebugApplicationThread64 * This,
            /* [out] */ BSTR *pbstrDescription,
            /* [out] */ BSTR *pbstrState);
        
        HRESULT ( STDMETHODCALLTYPE *SetNextStatement )( 
            IDebugApplicationThread64 * This,
            /* [in] */ IDebugStackFrame *pStackFrame,
            /* [in] */ IDebugCodeContext *pCodeContext);
        
        HRESULT ( STDMETHODCALLTYPE *GetState )( 
            IDebugApplicationThread64 * This,
            /* [out] */ DWORD *pState);
        
        HRESULT ( STDMETHODCALLTYPE *Suspend )( 
            IDebugApplicationThread64 * This,
            /* [out] */ DWORD *pdwCount);
        
        HRESULT ( STDMETHODCALLTYPE *Resume )( 
            IDebugApplicationThread64 * This,
            /* [out] */ DWORD *pdwCount);
        
        HRESULT ( STDMETHODCALLTYPE *GetSuspendCount )( 
            IDebugApplicationThread64 * This,
            /* [out] */ DWORD *pdwCount);
        
        HRESULT ( STDMETHODCALLTYPE *SynchronousCallIntoThread32 )( 
            IDebugApplicationThread64 * This,
            /* [in] */ IDebugThreadCall32 *pstcb,
            /* [in] */ DWORD dwParam1,
            /* [in] */ DWORD dwParam2,
            /* [in] */ DWORD dwParam3);
        
        HRESULT ( STDMETHODCALLTYPE *QueryIsCurrentThread )( 
            IDebugApplicationThread64 * This);
        
        HRESULT ( STDMETHODCALLTYPE *QueryIsDebuggerThread )( 
            IDebugApplicationThread64 * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetDescription )( 
            IDebugApplicationThread64 * This,
            /* [in] */ LPCOLESTR pstrDescription);
        
        HRESULT ( STDMETHODCALLTYPE *SetStateString )( 
            IDebugApplicationThread64 * This,
            /* [in] */ LPCOLESTR pstrState);
        
        HRESULT ( STDMETHODCALLTYPE *SynchronousCallIntoThread64 )( 
            IDebugApplicationThread64 * This,
            /* [in] */ IDebugThreadCall64 *pstcb,
            /* [in] */ DWORDLONG dwParam1,
            /* [in] */ DWORDLONG dwParam2,
            /* [in] */ DWORDLONG dwParam3);
        
        END_INTERFACE
    } IDebugApplicationThread64Vtbl;

    interface IDebugApplicationThread64
    {
        CONST_VTBL struct IDebugApplicationThread64Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDebugApplicationThread64_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDebugApplicationThread64_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDebugApplicationThread64_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDebugApplicationThread64_GetSystemThreadId(This,dwThreadId)	\
    (This)->lpVtbl -> GetSystemThreadId(This,dwThreadId)

#define IDebugApplicationThread64_GetApplication(This,pprda)	\
    (This)->lpVtbl -> GetApplication(This,pprda)

#define IDebugApplicationThread64_EnumStackFrames(This,ppedsf)	\
    (This)->lpVtbl -> EnumStackFrames(This,ppedsf)

#define IDebugApplicationThread64_GetDescription(This,pbstrDescription,pbstrState)	\
    (This)->lpVtbl -> GetDescription(This,pbstrDescription,pbstrState)

#define IDebugApplicationThread64_SetNextStatement(This,pStackFrame,pCodeContext)	\
    (This)->lpVtbl -> SetNextStatement(This,pStackFrame,pCodeContext)

#define IDebugApplicationThread64_GetState(This,pState)	\
    (This)->lpVtbl -> GetState(This,pState)

#define IDebugApplicationThread64_Suspend(This,pdwCount)	\
    (This)->lpVtbl -> Suspend(This,pdwCount)

#define IDebugApplicationThread64_Resume(This,pdwCount)	\
    (This)->lpVtbl -> Resume(This,pdwCount)

#define IDebugApplicationThread64_GetSuspendCount(This,pdwCount)	\
    (This)->lpVtbl -> GetSuspendCount(This,pdwCount)


#define IDebugApplicationThread64_SynchronousCallIntoThread32(This,pstcb,dwParam1,dwParam2,dwParam3)	\
    (This)->lpVtbl -> SynchronousCallIntoThread32(This,pstcb,dwParam1,dwParam2,dwParam3)

#define IDebugApplicationThread64_QueryIsCurrentThread(This)	\
    (This)->lpVtbl -> QueryIsCurrentThread(This)

#define IDebugApplicationThread64_QueryIsDebuggerThread(This)	\
    (This)->lpVtbl -> QueryIsDebuggerThread(This)

#define IDebugApplicationThread64_SetDescription(This,pstrDescription)	\
    (This)->lpVtbl -> SetDescription(This,pstrDescription)

#define IDebugApplicationThread64_SetStateString(This,pstrState)	\
    (This)->lpVtbl -> SetStateString(This,pstrState)


#define IDebugApplicationThread64_SynchronousCallIntoThread64(This,pstcb,dwParam1,dwParam2,dwParam3)	\
    (This)->lpVtbl -> SynchronousCallIntoThread64(This,pstcb,dwParam1,dwParam2,dwParam3)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IDebugApplicationThread64_SynchronousCallIntoThread64_Proxy( 
    IDebugApplicationThread64 * This,
    /* [in] */ IDebugThreadCall64 *pstcb,
    /* [in] */ DWORDLONG dwParam1,
    /* [in] */ DWORDLONG dwParam2,
    /* [in] */ DWORDLONG dwParam3);


void __RPC_STUB IDebugApplicationThread64_SynchronousCallIntoThread64_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IDebugApplicationThread64_INTERFACE_DEFINED__ */


#ifndef __IDebugCookie_INTERFACE_DEFINED__
#define __IDebugCookie_INTERFACE_DEFINED__

/* interface IDebugCookie */
/* [unique][helpstring][uuid][local][object] */ 


EXTERN_C const IID IID_IDebugCookie;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("51973C39-CB0C-11d0-B5C9-00A0244A0E7A")
    IDebugCookie : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetDebugCookie( 
            /* [in] */ DWORD dwDebugAppCookie) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDebugCookieVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDebugCookie * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDebugCookie * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDebugCookie * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetDebugCookie )( 
            IDebugCookie * This,
            /* [in] */ DWORD dwDebugAppCookie);
        
        END_INTERFACE
    } IDebugCookieVtbl;

    interface IDebugCookie
    {
        CONST_VTBL struct IDebugCookieVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDebugCookie_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDebugCookie_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDebugCookie_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDebugCookie_SetDebugCookie(This,dwDebugAppCookie)	\
    (This)->lpVtbl -> SetDebugCookie(This,dwDebugAppCookie)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IDebugCookie_SetDebugCookie_Proxy( 
    IDebugCookie * This,
    /* [in] */ DWORD dwDebugAppCookie);


void __RPC_STUB IDebugCookie_SetDebugCookie_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IDebugCookie_INTERFACE_DEFINED__ */


#ifndef __IEnumDebugApplicationNodes_INTERFACE_DEFINED__
#define __IEnumDebugApplicationNodes_INTERFACE_DEFINED__

/* interface IEnumDebugApplicationNodes */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IEnumDebugApplicationNodes;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("51973C3a-CB0C-11d0-B5C9-00A0244A0E7A")
    IEnumDebugApplicationNodes : public IUnknown
    {
    public:
        virtual /* [local] */ HRESULT __stdcall Next( 
            /* [in] */ ULONG celt,
            /* [out] */ IDebugApplicationNode **pprddp,
            /* [out] */ ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
            /* [in] */ ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
            /* [out] */ IEnumDebugApplicationNodes **pperddp) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IEnumDebugApplicationNodesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumDebugApplicationNodes * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumDebugApplicationNodes * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumDebugApplicationNodes * This);
        
        /* [local] */ HRESULT ( __stdcall *Next )( 
            IEnumDebugApplicationNodes * This,
            /* [in] */ ULONG celt,
            /* [out] */ IDebugApplicationNode **pprddp,
            /* [out] */ ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumDebugApplicationNodes * This,
            /* [in] */ ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumDebugApplicationNodes * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumDebugApplicationNodes * This,
            /* [out] */ IEnumDebugApplicationNodes **pperddp);
        
        END_INTERFACE
    } IEnumDebugApplicationNodesVtbl;

    interface IEnumDebugApplicationNodes
    {
        CONST_VTBL struct IEnumDebugApplicationNodesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumDebugApplicationNodes_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumDebugApplicationNodes_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumDebugApplicationNodes_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumDebugApplicationNodes_Next(This,celt,pprddp,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,pprddp,pceltFetched)

#define IEnumDebugApplicationNodes_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumDebugApplicationNodes_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumDebugApplicationNodes_Clone(This,pperddp)	\
    (This)->lpVtbl -> Clone(This,pperddp)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [call_as] */ HRESULT __stdcall IEnumDebugApplicationNodes_RemoteNext_Proxy( 
    IEnumDebugApplicationNodes * This,
    /* [in] */ ULONG celt,
    /* [length_is][size_is][out] */ IDebugApplicationNode **pprddp,
    /* [out] */ ULONG *pceltFetched);


void __RPC_STUB IEnumDebugApplicationNodes_RemoteNext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumDebugApplicationNodes_Skip_Proxy( 
    IEnumDebugApplicationNodes * This,
    /* [in] */ ULONG celt);


void __RPC_STUB IEnumDebugApplicationNodes_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumDebugApplicationNodes_Reset_Proxy( 
    IEnumDebugApplicationNodes * This);


void __RPC_STUB IEnumDebugApplicationNodes_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumDebugApplicationNodes_Clone_Proxy( 
    IEnumDebugApplicationNodes * This,
    /* [out] */ IEnumDebugApplicationNodes **pperddp);


void __RPC_STUB IEnumDebugApplicationNodes_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IEnumDebugApplicationNodes_INTERFACE_DEFINED__ */


#ifndef __IEnumRemoteDebugApplications_INTERFACE_DEFINED__
#define __IEnumRemoteDebugApplications_INTERFACE_DEFINED__

/* interface IEnumRemoteDebugApplications */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IEnumRemoteDebugApplications;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("51973C3b-CB0C-11d0-B5C9-00A0244A0E7A")
    IEnumRemoteDebugApplications : public IUnknown
    {
    public:
        virtual /* [local] */ HRESULT __stdcall Next( 
            /* [in] */ ULONG celt,
            /* [out] */ IRemoteDebugApplication **ppda,
            /* [out] */ ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
            /* [in] */ ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
            /* [out] */ IEnumRemoteDebugApplications **ppessd) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IEnumRemoteDebugApplicationsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumRemoteDebugApplications * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumRemoteDebugApplications * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumRemoteDebugApplications * This);
        
        /* [local] */ HRESULT ( __stdcall *Next )( 
            IEnumRemoteDebugApplications * This,
            /* [in] */ ULONG celt,
            /* [out] */ IRemoteDebugApplication **ppda,
            /* [out] */ ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumRemoteDebugApplications * This,
            /* [in] */ ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumRemoteDebugApplications * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumRemoteDebugApplications * This,
            /* [out] */ IEnumRemoteDebugApplications **ppessd);
        
        END_INTERFACE
    } IEnumRemoteDebugApplicationsVtbl;

    interface IEnumRemoteDebugApplications
    {
        CONST_VTBL struct IEnumRemoteDebugApplicationsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumRemoteDebugApplications_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumRemoteDebugApplications_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumRemoteDebugApplications_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumRemoteDebugApplications_Next(This,celt,ppda,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,ppda,pceltFetched)

#define IEnumRemoteDebugApplications_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumRemoteDebugApplications_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumRemoteDebugApplications_Clone(This,ppessd)	\
    (This)->lpVtbl -> Clone(This,ppessd)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [call_as] */ HRESULT __stdcall IEnumRemoteDebugApplications_RemoteNext_Proxy( 
    IEnumRemoteDebugApplications * This,
    /* [in] */ ULONG celt,
    /* [length_is][size_is][out] */ IRemoteDebugApplication **ppda,
    /* [out] */ ULONG *pceltFetched);


void __RPC_STUB IEnumRemoteDebugApplications_RemoteNext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumRemoteDebugApplications_Skip_Proxy( 
    IEnumRemoteDebugApplications * This,
    /* [in] */ ULONG celt);


void __RPC_STUB IEnumRemoteDebugApplications_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumRemoteDebugApplications_Reset_Proxy( 
    IEnumRemoteDebugApplications * This);


void __RPC_STUB IEnumRemoteDebugApplications_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumRemoteDebugApplications_Clone_Proxy( 
    IEnumRemoteDebugApplications * This,
    /* [out] */ IEnumRemoteDebugApplications **ppessd);


void __RPC_STUB IEnumRemoteDebugApplications_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IEnumRemoteDebugApplications_INTERFACE_DEFINED__ */


#ifndef __IEnumRemoteDebugApplicationThreads_INTERFACE_DEFINED__
#define __IEnumRemoteDebugApplicationThreads_INTERFACE_DEFINED__

/* interface IEnumRemoteDebugApplicationThreads */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IEnumRemoteDebugApplicationThreads;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("51973C3c-CB0C-11d0-B5C9-00A0244A0E7A")
    IEnumRemoteDebugApplicationThreads : public IUnknown
    {
    public:
        virtual /* [local] */ HRESULT __stdcall Next( 
            /* [in] */ ULONG celt,
            /* [out] */ IRemoteDebugApplicationThread **pprdat,
            /* [out] */ ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
            /* [in] */ ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
            /* [out] */ IEnumRemoteDebugApplicationThreads **pperdat) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IEnumRemoteDebugApplicationThreadsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumRemoteDebugApplicationThreads * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumRemoteDebugApplicationThreads * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumRemoteDebugApplicationThreads * This);
        
        /* [local] */ HRESULT ( __stdcall *Next )( 
            IEnumRemoteDebugApplicationThreads * This,
            /* [in] */ ULONG celt,
            /* [out] */ IRemoteDebugApplicationThread **pprdat,
            /* [out] */ ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumRemoteDebugApplicationThreads * This,
            /* [in] */ ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumRemoteDebugApplicationThreads * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumRemoteDebugApplicationThreads * This,
            /* [out] */ IEnumRemoteDebugApplicationThreads **pperdat);
        
        END_INTERFACE
    } IEnumRemoteDebugApplicationThreadsVtbl;

    interface IEnumRemoteDebugApplicationThreads
    {
        CONST_VTBL struct IEnumRemoteDebugApplicationThreadsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumRemoteDebugApplicationThreads_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumRemoteDebugApplicationThreads_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumRemoteDebugApplicationThreads_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumRemoteDebugApplicationThreads_Next(This,celt,pprdat,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,pprdat,pceltFetched)

#define IEnumRemoteDebugApplicationThreads_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumRemoteDebugApplicationThreads_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumRemoteDebugApplicationThreads_Clone(This,pperdat)	\
    (This)->lpVtbl -> Clone(This,pperdat)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [call_as] */ HRESULT __stdcall IEnumRemoteDebugApplicationThreads_RemoteNext_Proxy( 
    IEnumRemoteDebugApplicationThreads * This,
    /* [in] */ ULONG celt,
    /* [length_is][size_is][out] */ IRemoteDebugApplicationThread **ppdat,
    /* [out] */ ULONG *pceltFetched);


void __RPC_STUB IEnumRemoteDebugApplicationThreads_RemoteNext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumRemoteDebugApplicationThreads_Skip_Proxy( 
    IEnumRemoteDebugApplicationThreads * This,
    /* [in] */ ULONG celt);


void __RPC_STUB IEnumRemoteDebugApplicationThreads_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumRemoteDebugApplicationThreads_Reset_Proxy( 
    IEnumRemoteDebugApplicationThreads * This);


void __RPC_STUB IEnumRemoteDebugApplicationThreads_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumRemoteDebugApplicationThreads_Clone_Proxy( 
    IEnumRemoteDebugApplicationThreads * This,
    /* [out] */ IEnumRemoteDebugApplicationThreads **pperdat);


void __RPC_STUB IEnumRemoteDebugApplicationThreads_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IEnumRemoteDebugApplicationThreads_INTERFACE_DEFINED__ */


#ifndef __IDebugFormatter_INTERFACE_DEFINED__
#define __IDebugFormatter_INTERFACE_DEFINED__

/* interface IDebugFormatter */
/* [local][unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IDebugFormatter;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("51973C05-CB0C-11d0-B5C9-00A0244A0E7A")
    IDebugFormatter : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetStringForVariant( 
            /* [in] */ VARIANT *pvar,
            /* [in] */ ULONG nRadix,
            /* [out] */ BSTR *pbstrValue) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetVariantForString( 
            /* [in] */ LPCOLESTR pwstrValue,
            /* [out] */ VARIANT *pvar) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetStringForVarType( 
            /* [in] */ VARTYPE vt,
            /* [in] */ TYPEDESC *ptdescArrayType,
            /* [out] */ BSTR *pbstr) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDebugFormatterVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDebugFormatter * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDebugFormatter * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDebugFormatter * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetStringForVariant )( 
            IDebugFormatter * This,
            /* [in] */ VARIANT *pvar,
            /* [in] */ ULONG nRadix,
            /* [out] */ BSTR *pbstrValue);
        
        HRESULT ( STDMETHODCALLTYPE *GetVariantForString )( 
            IDebugFormatter * This,
            /* [in] */ LPCOLESTR pwstrValue,
            /* [out] */ VARIANT *pvar);
        
        HRESULT ( STDMETHODCALLTYPE *GetStringForVarType )( 
            IDebugFormatter * This,
            /* [in] */ VARTYPE vt,
            /* [in] */ TYPEDESC *ptdescArrayType,
            /* [out] */ BSTR *pbstr);
        
        END_INTERFACE
    } IDebugFormatterVtbl;

    interface IDebugFormatter
    {
        CONST_VTBL struct IDebugFormatterVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDebugFormatter_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDebugFormatter_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDebugFormatter_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDebugFormatter_GetStringForVariant(This,pvar,nRadix,pbstrValue)	\
    (This)->lpVtbl -> GetStringForVariant(This,pvar,nRadix,pbstrValue)

#define IDebugFormatter_GetVariantForString(This,pwstrValue,pvar)	\
    (This)->lpVtbl -> GetVariantForString(This,pwstrValue,pvar)

#define IDebugFormatter_GetStringForVarType(This,vt,ptdescArrayType,pbstr)	\
    (This)->lpVtbl -> GetStringForVarType(This,vt,ptdescArrayType,pbstr)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IDebugFormatter_GetStringForVariant_Proxy( 
    IDebugFormatter * This,
    /* [in] */ VARIANT *pvar,
    /* [in] */ ULONG nRadix,
    /* [out] */ BSTR *pbstrValue);


void __RPC_STUB IDebugFormatter_GetStringForVariant_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugFormatter_GetVariantForString_Proxy( 
    IDebugFormatter * This,
    /* [in] */ LPCOLESTR pwstrValue,
    /* [out] */ VARIANT *pvar);


void __RPC_STUB IDebugFormatter_GetVariantForString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugFormatter_GetStringForVarType_Proxy( 
    IDebugFormatter * This,
    /* [in] */ VARTYPE vt,
    /* [in] */ TYPEDESC *ptdescArrayType,
    /* [out] */ BSTR *pbstr);


void __RPC_STUB IDebugFormatter_GetStringForVarType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IDebugFormatter_INTERFACE_DEFINED__ */


#ifndef __ISimpleConnectionPoint_INTERFACE_DEFINED__
#define __ISimpleConnectionPoint_INTERFACE_DEFINED__

/* interface ISimpleConnectionPoint */
/* [local][unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_ISimpleConnectionPoint;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("51973C3e-CB0C-11d0-B5C9-00A0244A0E7A")
    ISimpleConnectionPoint : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetEventCount( 
            /* [out] */ ULONG *pulCount) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DescribeEvents( 
            /* [in] */ ULONG iEvent,
            /* [in] */ ULONG cEvents,
            /* [length_is][size_is][out] */ DISPID *prgid,
            /* [length_is][size_is][out] */ BSTR *prgbstr,
            /* [out] */ ULONG *pcEventsFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Advise( 
            /* [in] */ IDispatch *pdisp,
            /* [out] */ DWORD *pdwCookie) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Unadvise( 
            /* [in] */ DWORD dwCookie) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ISimpleConnectionPointVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISimpleConnectionPoint * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISimpleConnectionPoint * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISimpleConnectionPoint * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetEventCount )( 
            ISimpleConnectionPoint * This,
            /* [out] */ ULONG *pulCount);
        
        HRESULT ( STDMETHODCALLTYPE *DescribeEvents )( 
            ISimpleConnectionPoint * This,
            /* [in] */ ULONG iEvent,
            /* [in] */ ULONG cEvents,
            /* [length_is][size_is][out] */ DISPID *prgid,
            /* [length_is][size_is][out] */ BSTR *prgbstr,
            /* [out] */ ULONG *pcEventsFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Advise )( 
            ISimpleConnectionPoint * This,
            /* [in] */ IDispatch *pdisp,
            /* [out] */ DWORD *pdwCookie);
        
        HRESULT ( STDMETHODCALLTYPE *Unadvise )( 
            ISimpleConnectionPoint * This,
            /* [in] */ DWORD dwCookie);
        
        END_INTERFACE
    } ISimpleConnectionPointVtbl;

    interface ISimpleConnectionPoint
    {
        CONST_VTBL struct ISimpleConnectionPointVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISimpleConnectionPoint_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISimpleConnectionPoint_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISimpleConnectionPoint_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISimpleConnectionPoint_GetEventCount(This,pulCount)	\
    (This)->lpVtbl -> GetEventCount(This,pulCount)

#define ISimpleConnectionPoint_DescribeEvents(This,iEvent,cEvents,prgid,prgbstr,pcEventsFetched)	\
    (This)->lpVtbl -> DescribeEvents(This,iEvent,cEvents,prgid,prgbstr,pcEventsFetched)

#define ISimpleConnectionPoint_Advise(This,pdisp,pdwCookie)	\
    (This)->lpVtbl -> Advise(This,pdisp,pdwCookie)

#define ISimpleConnectionPoint_Unadvise(This,dwCookie)	\
    (This)->lpVtbl -> Unadvise(This,dwCookie)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE ISimpleConnectionPoint_GetEventCount_Proxy( 
    ISimpleConnectionPoint * This,
    /* [out] */ ULONG *pulCount);


void __RPC_STUB ISimpleConnectionPoint_GetEventCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISimpleConnectionPoint_DescribeEvents_Proxy( 
    ISimpleConnectionPoint * This,
    /* [in] */ ULONG iEvent,
    /* [in] */ ULONG cEvents,
    /* [length_is][size_is][out] */ DISPID *prgid,
    /* [length_is][size_is][out] */ BSTR *prgbstr,
    /* [out] */ ULONG *pcEventsFetched);


void __RPC_STUB ISimpleConnectionPoint_DescribeEvents_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISimpleConnectionPoint_Advise_Proxy( 
    ISimpleConnectionPoint * This,
    /* [in] */ IDispatch *pdisp,
    /* [out] */ DWORD *pdwCookie);


void __RPC_STUB ISimpleConnectionPoint_Advise_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISimpleConnectionPoint_Unadvise_Proxy( 
    ISimpleConnectionPoint * This,
    /* [in] */ DWORD dwCookie);


void __RPC_STUB ISimpleConnectionPoint_Unadvise_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ISimpleConnectionPoint_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_activdbg_0358 */
/* [local] */ 

EXTERN_C const CLSID CLSID_DebugHelper;


extern RPC_IF_HANDLE __MIDL_itf_activdbg_0358_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_activdbg_0358_v0_0_s_ifspec;

#ifndef __IDebugHelper_INTERFACE_DEFINED__
#define __IDebugHelper_INTERFACE_DEFINED__

/* interface IDebugHelper */
/* [local][unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IDebugHelper;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("51973C3f-CB0C-11d0-B5C9-00A0244A0E7A")
    IDebugHelper : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE CreatePropertyBrowser( 
            /* [in] */ VARIANT *pvar,
            /* [in] */ LPCOLESTR bstrName,
            /* [in] */ IDebugApplicationThread *pdat,
            /* [out] */ IDebugProperty **ppdob) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreatePropertyBrowserEx( 
            /* [in] */ VARIANT *pvar,
            /* [in] */ LPCOLESTR bstrName,
            /* [in] */ IDebugApplicationThread *pdat,
            /* [in] */ IDebugFormatter *pdf,
            /* [out] */ IDebugProperty **ppdob) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateSimpleConnectionPoint( 
            /* [in] */ IDispatch *pdisp,
            /* [out] */ ISimpleConnectionPoint **ppscp) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDebugHelperVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDebugHelper * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDebugHelper * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDebugHelper * This);
        
        HRESULT ( STDMETHODCALLTYPE *CreatePropertyBrowser )( 
            IDebugHelper * This,
            /* [in] */ VARIANT *pvar,
            /* [in] */ LPCOLESTR bstrName,
            /* [in] */ IDebugApplicationThread *pdat,
            /* [out] */ IDebugProperty **ppdob);
        
        HRESULT ( STDMETHODCALLTYPE *CreatePropertyBrowserEx )( 
            IDebugHelper * This,
            /* [in] */ VARIANT *pvar,
            /* [in] */ LPCOLESTR bstrName,
            /* [in] */ IDebugApplicationThread *pdat,
            /* [in] */ IDebugFormatter *pdf,
            /* [out] */ IDebugProperty **ppdob);
        
        HRESULT ( STDMETHODCALLTYPE *CreateSimpleConnectionPoint )( 
            IDebugHelper * This,
            /* [in] */ IDispatch *pdisp,
            /* [out] */ ISimpleConnectionPoint **ppscp);
        
        END_INTERFACE
    } IDebugHelperVtbl;

    interface IDebugHelper
    {
        CONST_VTBL struct IDebugHelperVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDebugHelper_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDebugHelper_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDebugHelper_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDebugHelper_CreatePropertyBrowser(This,pvar,bstrName,pdat,ppdob)	\
    (This)->lpVtbl -> CreatePropertyBrowser(This,pvar,bstrName,pdat,ppdob)

#define IDebugHelper_CreatePropertyBrowserEx(This,pvar,bstrName,pdat,pdf,ppdob)	\
    (This)->lpVtbl -> CreatePropertyBrowserEx(This,pvar,bstrName,pdat,pdf,ppdob)

#define IDebugHelper_CreateSimpleConnectionPoint(This,pdisp,ppscp)	\
    (This)->lpVtbl -> CreateSimpleConnectionPoint(This,pdisp,ppscp)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IDebugHelper_CreatePropertyBrowser_Proxy( 
    IDebugHelper * This,
    /* [in] */ VARIANT *pvar,
    /* [in] */ LPCOLESTR bstrName,
    /* [in] */ IDebugApplicationThread *pdat,
    /* [out] */ IDebugProperty **ppdob);


void __RPC_STUB IDebugHelper_CreatePropertyBrowser_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugHelper_CreatePropertyBrowserEx_Proxy( 
    IDebugHelper * This,
    /* [in] */ VARIANT *pvar,
    /* [in] */ LPCOLESTR bstrName,
    /* [in] */ IDebugApplicationThread *pdat,
    /* [in] */ IDebugFormatter *pdf,
    /* [out] */ IDebugProperty **ppdob);


void __RPC_STUB IDebugHelper_CreatePropertyBrowserEx_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDebugHelper_CreateSimpleConnectionPoint_Proxy( 
    IDebugHelper * This,
    /* [in] */ IDispatch *pdisp,
    /* [out] */ ISimpleConnectionPoint **ppscp);


void __RPC_STUB IDebugHelper_CreateSimpleConnectionPoint_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IDebugHelper_INTERFACE_DEFINED__ */


#ifndef __IEnumDebugExpressionContexts_INTERFACE_DEFINED__
#define __IEnumDebugExpressionContexts_INTERFACE_DEFINED__

/* interface IEnumDebugExpressionContexts */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IEnumDebugExpressionContexts;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("51973C40-CB0C-11d0-B5C9-00A0244A0E7A")
    IEnumDebugExpressionContexts : public IUnknown
    {
    public:
        virtual /* [local] */ HRESULT __stdcall Next( 
            /* [in] */ ULONG celt,
            /* [out] */ IDebugExpressionContext **ppdec,
            /* [out] */ ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
            /* [in] */ ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
            /* [out] */ IEnumDebugExpressionContexts **ppedec) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IEnumDebugExpressionContextsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumDebugExpressionContexts * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumDebugExpressionContexts * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumDebugExpressionContexts * This);
        
        /* [local] */ HRESULT ( __stdcall *Next )( 
            IEnumDebugExpressionContexts * This,
            /* [in] */ ULONG celt,
            /* [out] */ IDebugExpressionContext **ppdec,
            /* [out] */ ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumDebugExpressionContexts * This,
            /* [in] */ ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumDebugExpressionContexts * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumDebugExpressionContexts * This,
            /* [out] */ IEnumDebugExpressionContexts **ppedec);
        
        END_INTERFACE
    } IEnumDebugExpressionContextsVtbl;

    interface IEnumDebugExpressionContexts
    {
        CONST_VTBL struct IEnumDebugExpressionContextsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumDebugExpressionContexts_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumDebugExpressionContexts_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumDebugExpressionContexts_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumDebugExpressionContexts_Next(This,celt,ppdec,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,ppdec,pceltFetched)

#define IEnumDebugExpressionContexts_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumDebugExpressionContexts_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumDebugExpressionContexts_Clone(This,ppedec)	\
    (This)->lpVtbl -> Clone(This,ppedec)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [call_as] */ HRESULT __stdcall IEnumDebugExpressionContexts_RemoteNext_Proxy( 
    IEnumDebugExpressionContexts * This,
    /* [in] */ ULONG celt,
    /* [length_is][size_is][out] */ IDebugExpressionContext **pprgdec,
    /* [out] */ ULONG *pceltFetched);


void __RPC_STUB IEnumDebugExpressionContexts_RemoteNext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumDebugExpressionContexts_Skip_Proxy( 
    IEnumDebugExpressionContexts * This,
    /* [in] */ ULONG celt);


void __RPC_STUB IEnumDebugExpressionContexts_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumDebugExpressionContexts_Reset_Proxy( 
    IEnumDebugExpressionContexts * This);


void __RPC_STUB IEnumDebugExpressionContexts_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumDebugExpressionContexts_Clone_Proxy( 
    IEnumDebugExpressionContexts * This,
    /* [out] */ IEnumDebugExpressionContexts **ppedec);


void __RPC_STUB IEnumDebugExpressionContexts_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IEnumDebugExpressionContexts_INTERFACE_DEFINED__ */


#ifndef __IProvideExpressionContexts_INTERFACE_DEFINED__
#define __IProvideExpressionContexts_INTERFACE_DEFINED__

/* interface IProvideExpressionContexts */
/* [unique][uuid][object] */ 


EXTERN_C const IID IID_IProvideExpressionContexts;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("51973C41-CB0C-11d0-B5C9-00A0244A0E7A")
    IProvideExpressionContexts : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE EnumExpressionContexts( 
            /* [out] */ IEnumDebugExpressionContexts **ppedec) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IProvideExpressionContextsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IProvideExpressionContexts * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IProvideExpressionContexts * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IProvideExpressionContexts * This);
        
        HRESULT ( STDMETHODCALLTYPE *EnumExpressionContexts )( 
            IProvideExpressionContexts * This,
            /* [out] */ IEnumDebugExpressionContexts **ppedec);
        
        END_INTERFACE
    } IProvideExpressionContextsVtbl;

    interface IProvideExpressionContexts
    {
        CONST_VTBL struct IProvideExpressionContextsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IProvideExpressionContexts_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IProvideExpressionContexts_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IProvideExpressionContexts_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IProvideExpressionContexts_EnumExpressionContexts(This,ppedec)	\
    (This)->lpVtbl -> EnumExpressionContexts(This,ppedec)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IProvideExpressionContexts_EnumExpressionContexts_Proxy( 
    IProvideExpressionContexts * This,
    /* [out] */ IEnumDebugExpressionContexts **ppedec);


void __RPC_STUB IProvideExpressionContexts_EnumExpressionContexts_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IProvideExpressionContexts_INTERFACE_DEFINED__ */



#ifndef __ProcessDebugManagerLib_LIBRARY_DEFINED__
#define __ProcessDebugManagerLib_LIBRARY_DEFINED__

/* library ProcessDebugManagerLib */
/* [helpstring][version][uuid] */ 


























































EXTERN_C const CLSID CLSID_CDebugDocumentHelper;

EXTERN_C const IID LIBID_ProcessDebugManagerLib;

EXTERN_C const CLSID CLSID_ProcessDebugManager;

#ifdef __cplusplus

class DECLSPEC_UUID("78a51822-51f4-11d0-8f20-00805f2cd064")
ProcessDebugManager;
#endif

EXTERN_C const CLSID CLSID_DebugHelper;

#ifdef __cplusplus

class DECLSPEC_UUID("0BFCC060-8C1D-11d0-ACCD-00AA0060275C")
DebugHelper;
#endif

EXTERN_C const CLSID CLSID_CDebugDocumentHelper;

#ifdef __cplusplus

class DECLSPEC_UUID("83B8BCA6-687C-11D0-A405-00AA0060275C")
CDebugDocumentHelper;
#endif

EXTERN_C const CLSID CLSID_MachineDebugManager;

#ifdef __cplusplus

class DECLSPEC_UUID("0C0A3666-30C9-11D0-8F20-00805F2CD064")
MachineDebugManager;
#endif

EXTERN_C const CLSID CLSID_DefaultDebugSessionProvider;

#ifdef __cplusplus

class DECLSPEC_UUID("834128a2-51f4-11d0-8f20-00805f2cd064")
DefaultDebugSessionProvider;
#endif
#endif /* __ProcessDebugManagerLib_LIBRARY_DEFINED__ */

/* interface __MIDL_itf_activdbg_0361 */
/* [local] */ 


#endif  // __ActivDbg_h



extern RPC_IF_HANDLE __MIDL_itf_activdbg_0361_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_activdbg_0361_v0_0_s_ifspec;

/* Additional Prototypes for ALL interfaces */

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long *, BSTR * ); 

/* [local] */ HRESULT __stdcall IEnumDebugCodeContexts_Next_Proxy( 
    IEnumDebugCodeContexts * This,
    /* [in] */ ULONG celt,
    /* [out] */ IDebugCodeContext **pscc,
    /* [out] */ ULONG *pceltFetched);


/* [call_as] */ HRESULT __stdcall IEnumDebugCodeContexts_Next_Stub( 
    IEnumDebugCodeContexts * This,
    /* [in] */ ULONG celt,
    /* [length_is][size_is][out] */ IDebugCodeContext **pscc,
    /* [out] */ ULONG *pceltFetched);

/* [local] */ HRESULT __stdcall IEnumDebugStackFrames_Next_Proxy( 
    IEnumDebugStackFrames * This,
    /* [in] */ ULONG celt,
    /* [out] */ DebugStackFrameDescriptor *prgdsfd,
    /* [out] */ ULONG *pceltFetched);


/* [call_as] */ HRESULT __stdcall IEnumDebugStackFrames_Next_Stub( 
    IEnumDebugStackFrames * This,
    /* [in] */ ULONG celt,
    /* [length_is][size_is][out] */ DebugStackFrameDescriptor *prgdsfd,
    /* [out] */ ULONG *pceltFetched);

/* [local] */ HRESULT __stdcall IEnumDebugStackFrames64_Next64_Proxy( 
    IEnumDebugStackFrames64 * This,
    /* [in] */ ULONG celt,
    /* [out] */ DebugStackFrameDescriptor64 *prgdsfd,
    /* [out] */ ULONG *pceltFetched);


/* [call_as] */ HRESULT __stdcall IEnumDebugStackFrames64_Next64_Stub( 
    IEnumDebugStackFrames64 * This,
    /* [in] */ ULONG celt,
    /* [length_is][size_is][out] */ DebugStackFrameDescriptor64 *prgdsfd,
    /* [out] */ ULONG *pceltFetched);

/* [local] */ HRESULT __stdcall IEnumDebugApplicationNodes_Next_Proxy( 
    IEnumDebugApplicationNodes * This,
    /* [in] */ ULONG celt,
    /* [out] */ IDebugApplicationNode **pprddp,
    /* [out] */ ULONG *pceltFetched);


/* [call_as] */ HRESULT __stdcall IEnumDebugApplicationNodes_Next_Stub( 
    IEnumDebugApplicationNodes * This,
    /* [in] */ ULONG celt,
    /* [length_is][size_is][out] */ IDebugApplicationNode **pprddp,
    /* [out] */ ULONG *pceltFetched);

/* [local] */ HRESULT __stdcall IEnumRemoteDebugApplications_Next_Proxy( 
    IEnumRemoteDebugApplications * This,
    /* [in] */ ULONG celt,
    /* [out] */ IRemoteDebugApplication **ppda,
    /* [out] */ ULONG *pceltFetched);


/* [call_as] */ HRESULT __stdcall IEnumRemoteDebugApplications_Next_Stub( 
    IEnumRemoteDebugApplications * This,
    /* [in] */ ULONG celt,
    /* [length_is][size_is][out] */ IRemoteDebugApplication **ppda,
    /* [out] */ ULONG *pceltFetched);

/* [local] */ HRESULT __stdcall IEnumRemoteDebugApplicationThreads_Next_Proxy( 
    IEnumRemoteDebugApplicationThreads * This,
    /* [in] */ ULONG celt,
    /* [out] */ IRemoteDebugApplicationThread **pprdat,
    /* [out] */ ULONG *pceltFetched);


/* [call_as] */ HRESULT __stdcall IEnumRemoteDebugApplicationThreads_Next_Stub( 
    IEnumRemoteDebugApplicationThreads * This,
    /* [in] */ ULONG celt,
    /* [length_is][size_is][out] */ IRemoteDebugApplicationThread **ppdat,
    /* [out] */ ULONG *pceltFetched);

/* [local] */ HRESULT __stdcall IEnumDebugExpressionContexts_Next_Proxy( 
    IEnumDebugExpressionContexts * This,
    /* [in] */ ULONG celt,
    /* [out] */ IDebugExpressionContext **ppdec,
    /* [out] */ ULONG *pceltFetched);


/* [call_as] */ HRESULT __stdcall IEnumDebugExpressionContexts_Next_Stub( 
    IEnumDebugExpressionContexts * This,
    /* [in] */ ULONG celt,
    /* [length_is][size_is][out] */ IDebugExpressionContext **pprgdec,
    /* [out] */ ULONG *pceltFetched);



/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


