#pragma once

// system headers
#include <winsock2.h>
#include <windows.h>

// Include MAPI
#include <mapix.h>
#include <mapispi.h>

extern "C" {
	STDAPI_(HRESULT) HrOpenABEntryWithExchangeContext(
		LPMAPISESSION	pmsess,
		const MAPIUID *	pEmsmdbUID,
		LPADRBOOK		pAddrBook,
		ULONG			cbEntryID,
		LPENTRYID		lpEntryID,
		LPCIID			lpInterface,
		ULONG			ulFlags,
		ULONG FAR *		lpulObjType,
		LPUNKNOWN FAR *	lppUnk);

	STDAPI_(HRESULT) HrDoABDetailsWithExchangeContext(
		LPMAPISESSION	pmsess,
		const MAPIUID *	pEmsmdbUID,
		LPADRBOOK		pAddrBook,
		ULONG_PTR FAR *	lpulUIParam,
		LPFNDISMISS 	lpfnDismiss,
		LPVOID 			lpvDismissContext,
		ULONG 			cbEntryID,
		LPENTRYID 		lpEntryID,
		LPFNBUTTON 		lpfButtonCallback,
		LPVOID 			lpvButtonContext,
		LPTSTR 			lpszButtonText,
		ULONG 			ulFlags);

	STDAPI_(HRESULT) HrDoABDetailsWithProviderUID(
		const MAPIUID	*pEmsabpUID,
		LPADRBOOK		pAddrBook,
		ULONG_PTR FAR *	lpulUIParam,
		LPFNDISMISS 	lpfnDismiss,
		LPVOID 			lpvDismissContext,
		ULONG 			cbEntryID,
		LPENTRYID 		lpEntryID,
		LPFNBUTTON 		lpfButtonCallback,
		LPVOID 			lpvButtonContext,
		LPTSTR 			lpszButtonText,
		ULONG 			ulFlags);

	STDAPI_(HRESULT) HrOpenABEntryUsingDefaultContext(
		LPMAPISESSION	pmsess,
		LPADRBOOK		pAddrBook,
		ULONG			cbEntryID,
		LPENTRYID		lpEntryID,
		LPCIID			lpInterface,
		ULONG			ulFlags,
		ULONG FAR *		lpulObjType,
		LPUNKNOWN FAR *	lppUnk);

	STDAPI_(HRESULT) HrOpenABEntryWithProviderUID(
		const MAPIUID *	pEmsabpUID,
		LPADRBOOK		pAddrBook,
		ULONG			cbEntryID,
		LPENTRYID		lpEntryID,
		LPCIID			lpInterface,
		ULONG			ulFlags,
		ULONG FAR *		lpulObjType,
		LPUNKNOWN FAR *	lppUnk);

	STDAPI_(HRESULT) HrOpenABEntryWithProviderUIDSupport(
		const MAPIUID *	pEmsabpUID,
		LPMAPISUP		lpSup,
		ULONG			cbEntryID,
		LPENTRYID		lpEntryID,
		LPCIID			lpInterface,
		ULONG			ulFlags,
		ULONG FAR *		lpulObjType,
		LPUNKNOWN FAR *	lppUnk);

	STDAPI_(HRESULT) HrOpenABEntryWithResolvedRow(
		LPSRow			prwResolved,
		LPADRBOOK		pAddrBook,
		ULONG			cbEntryID,
		LPENTRYID		lpEntryID,
		LPCIID			lpInterface,
		ULONG			ulFlags,
		ULONG FAR *		lpulObjType,
		LPUNKNOWN FAR *	lppUnk);

	STDAPI_(HRESULT) HrCompareABEntryIDsWithExchangeContext(
		LPMAPISESSION	pmsess,
		const MAPIUID	*pEmsmdbUID,
		LPADRBOOK		pAddrBook,
		ULONG			cbEntryID1,
		LPENTRYID		lpEntryID1,
		ULONG			cbEntryID2,
		LPENTRYID		lpEntryID2,
		ULONG			ulFlags,
		ULONG *			lpulResult);

	STDAPI_(HRESULT) HrOpenABEntryWithSupport(
		LPMAPISUP		lpSup,
		ULONG			cbEntryID,
		LPENTRYID		lpEntryID,
		LPCIID			lpInterface,
		ULONG			ulFlags,
		ULONG FAR *		lpulObjType,
		LPUNKNOWN FAR *	lppUnk);

	STDAPI_(HRESULT) HrGetGALFromEmsmdbUID(
		LPMAPISESSION	pSess,
		LPADRBOOK		lpAdrBook,
		const MAPIUID *	pEmsmdbUID,
		ULONG *			lpcbeid,
		LPENTRYID *		lppeid);
}