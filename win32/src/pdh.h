/*++ 
 
Copyright (C) 1996 Microsoft Corporation 
 
Module Name: 
 
    PDH.H 
 
Abstract: 
 
    Header file for the Performance Data Helper (PDH) DLL functions. 
 
--*/ 
#ifndef _PDH_H_ 
#define _PDH_H_ 
 
// system include files required for datatype and constant definitions  
#include <windows.h>    // necessary for data types used in this file 
#include <winperf.h>    // necessary for the Detail Level definitions 
 
#ifdef __cplusplus 
extern "C" { 
#endif 
 
typedef LONG            PDH_STATUS; 
 
#define PDH_FUNCTION    PDH_STATUS __stdcall  
 
// version info 
#define PDH_CVERSION_WIN40  0x0400 
#define PDH_VERSION         PDH_CVERSION_WIN40 

/*
// define severity masks 
#define IsSuccessSeverity(ErrorCode)    \ 
    ((((DWORD)(ErrorCode) & (0xC0000000L)) == 0x00000000L) ? TRUE : FALSE)

#define IsInformationalSeverity(ErrorCode)    \ 
    ((((DWORD)(ErrorCode) & (0xC0000000L)) == 0x40000000L) ? TRUE : FALSE) 

#define IsWarningSeverity(ErrorCode)    \ 
    ((((DWORD)(ErrorCode) & (0xC0000000L)) == 0x80000000L) ? TRUE : FALSE) 

#define IsErrorSeverity(ErrorCode)      \ 
    ((((DWORD)(ErrorCode) & (0xC0000000L)) == 0xC0000000L) ? TRUE : FALSE) 
*/
// data type definitions 
 
typedef HANDLE  HCOUNTER; 
typedef HANDLE  HQUERY; 
 
typedef struct _PDH_RAW_COUNTER { 
    DWORD       CStatus; 
    FILETIME    TimeStamp; 
    LONGLONG    FirstValue; 
    LONGLONG    SecondValue; 
    DWORD       MultiCount; 
} PDH_RAW_COUNTER, *PPDH_RAW_COUNTER; 
 
typedef struct _PDH_FMT_COUNTERVALUE { 
    DWORD    CStatus; 
    union { 
        LONG        longValue; 
        double      doubleValue; 
        LONGLONG    largeValue; 
    }; 
} PDH_FMT_COUNTERVALUE, *PPDH_FMT_COUNTERVALUE; 
 
typedef struct _PDH_STATISTICS { 
    DWORD                   dwFormat; 
    DWORD                   count; 
    PDH_FMT_COUNTERVALUE    min; 
    PDH_FMT_COUNTERVALUE    max; 
    PDH_FMT_COUNTERVALUE    mean; 
} PDH_STATISTICS, *PPDH_STATISTICS; 
 
typedef struct _PDH_COUNTER_PATH_ELEMENTS_A { 
    LPSTR   szMachineName; 
    LPSTR   szObjectName; 
    LPSTR   szInstanceName; 
    LPSTR   szParentInstance; 
    DWORD   dwInstanceIndex; 
    LPSTR   szCounterName; 
} PDH_COUNTER_PATH_ELEMENTS_A, *PPDH_COUNTER_PATH_ELEMENTS_A; 
 
typedef struct _PDH_COUNTER_PATH_ELEMENTS_W { 
    LPWSTR  szMachineName; 
    LPWSTR  szObjectName; 
    LPWSTR  szInstanceName; 
    LPWSTR  szParentInstance; 
    DWORD   dwInstanceIndex; 
    LPWSTR  szCounterName; 
} PDH_COUNTER_PATH_ELEMENTS_W, *PPDH_COUNTER_PATH_ELEMENTS_W; 
 
typedef struct _PDH_COUNTER_INFO_A { 
    DWORD   dwLength; 
    DWORD   dwType; 
    DWORD   CVersion; 
    DWORD   CStatus; 
    LONG    lScale; 
    LONG    lDefaultScale; 
    DWORD   dwUserData; 
    DWORD   dwQueryUserData; 
    LPSTR   szFullPath; 
    union   { 
        PDH_COUNTER_PATH_ELEMENTS_A CounterPath; 
        struct { 
            LPSTR   szMachineName; 
            LPSTR   szObjectName; 
            LPSTR   szInstanceName; 
            LPSTR   szParentInstance; 
            DWORD   dwInstanceIndex; 
            LPSTR   szCounterName; 
        }; 
    }; 
    LPSTR   szExplainText; 
    DWORD   DataBuffer[1]; 
} PDH_COUNTER_INFO_A, *PPDH_COUNTER_INFO_A; 
 
typedef struct _PDH_COUNTER_INFO_W { 
    DWORD   dwLength; 
    DWORD   dwType; 
    DWORD   CVersion; 
    DWORD   CStatus; 
    LONG    lScale; 
    LONG    lDefaultScale; 
    DWORD   dwUserData; 
    DWORD   dwQueryUserData; 
    LPWSTR  szFullPath; 
    union   { 
        PDH_COUNTER_PATH_ELEMENTS_W CounterPath; 
        struct { 
            LPWSTR   szMachineName; 
            LPWSTR   szObjectName; 
            LPWSTR   szInstanceName; 
            LPWSTR   szParentInstance; 
            DWORD    dwInstanceIndex; 
            LPWSTR   szCounterName; 
        }; 
    }; 
    LPWSTR  szExplainText; 
    DWORD   DataBuffer[1]; 
} PDH_COUNTER_INFO_W, *PPDH_COUNTER_INFO_W; 
 
// function definitions 
 
PDH_FUNCTION  
PdhGetDllVersion( 
    IN  LPDWORD lpdwVersion 
); 
 
// 
//  Query Functions 
// 
 
PDH_FUNCTION 
PdhOpenQuery ( 
    IN      LPVOID      pReserved, 
    IN      DWORD       dwUserData, 
    IN      HQUERY      *phQuery 
); 
 
PDH_FUNCTION 
PdhAddCounterW ( 
    IN      HQUERY      hQuery, 
    IN      LPCWSTR     szFullCounterPath, 
    IN      DWORD       dwUserData, 
    IN      HCOUNTER    *phCounter 
); 
 
PDH_FUNCTION 
PdhAddCounterA ( 
    IN      HQUERY      hQuery, 
    IN      LPCSTR      szFullCounterPath, 
    IN      DWORD       dwUserData, 
    IN      HCOUNTER    *phCounter 
); 
 
PDH_FUNCTION 
PdhRemoveCounter ( 
    IN      HCOUNTER    hCounter 
); 
 
PDH_FUNCTION 
PdhCollectQueryData ( 
    IN      HQUERY      hQuery 
); 
 
PDH_FUNCTION 
PdhCloseQuery ( 
    IN      HQUERY      hQuery 
); 
     
// 
//  Counter Functions 
// 
 
PDH_FUNCTION 
PdhGetFormattedCounterValue ( 
    IN      HCOUNTER                hCounter, 
    IN      DWORD                   dwFormat, 
    IN      LPDWORD                 lpdwType, 
    IN      PPDH_FMT_COUNTERVALUE   pValue 
); 
 
// dwFormat flag values 
//  
#define PDH_FMT_RAW     ((DWORD)0x00000010) 
#define PDH_FMT_ANSI    ((DWORD)0x00000020) 
#define PDH_FMT_UNICODE ((DWORD)0x00000040) 
#define PDH_FMT_LONG    ((DWORD)0x00000100) 
#define PDH_FMT_DOUBLE  ((DWORD)0x00000200) 
#define PDH_FMT_LARGE   ((DWORD)0x00000400) 
#define PDH_FMT_NOSCALE ((DWORD)0x00001000) 
#define PDH_FMT_1000    ((DWORD)0x00002000) 
#define PDH_FMT_NODATA  ((DWORD)0x00004000) 
 
PDH_FUNCTION 
PdhGetRawCounterValue ( 
    IN      HCOUNTER            hCounter, 
    IN      LPDWORD             lpdwType, 
    IN      PPDH_RAW_COUNTER    pValue 
); 
 
PDH_FUNCTION 
PdhCalculateCounterFromRawValue ( 
    IN      HCOUNTER                hCounter, 
    IN      DWORD                   dwFormat, 
    IN      PPDH_RAW_COUNTER        rawValue1, 
    IN      PPDH_RAW_COUNTER        rawValue2, 
    IN      PPDH_FMT_COUNTERVALUE   fmtValue 
); 
 
PDH_FUNCTION 
PdhComputeCounterStatistics ( 
    IN      HCOUNTER            hCounter, 
    IN      DWORD               dwFormat, 
    IN      DWORD               dwFirstEntry, 
    IN      DWORD               dwNumEntries, 
    IN      PPDH_RAW_COUNTER    lpRawValueArray, 
    IN      PPDH_STATISTICS     data 
); 
 
PDH_FUNCTION 
PdhGetCounterInfoW ( 
    IN      HCOUNTER            hCounter, 
    IN      BOOLEAN             bRetrieveExplainText, 
    IN      LPDWORD             pdwBufferSize, 
    IN      PPDH_COUNTER_INFO_W lpBuffer 
); 
 
PDH_FUNCTION 
PdhGetCounterInfoA ( 
    IN      HCOUNTER            hCounter, 
    IN      BOOLEAN             bRetrieveExplainText, 
    IN      LPDWORD             pdwBufferSize, 
    IN      PPDH_COUNTER_INFO_A lpBuffer 
); 
 
#define PDH_MAX_SCALE    (7L) 
#define PDH_MIN_SCALE   (-7L) 
 
PDH_FUNCTION 
PdhSetCounterScaleFactor ( 
    IN      HCOUNTER    hCounter, 
    IN      LONG        lFactor 
); 
// 
//   Browsing and enumeration functions 
// 
PDH_FUNCTION 
PdhConnectMachineW ( 
    IN      LPCWSTR  szMachineName 
); 
 
PDH_FUNCTION 
PdhConnectMachineA ( 
    IN      LPCSTR  szMachineName 
); 
 
PDH_FUNCTION 
PdhEnumMachinesW ( 
    IN      LPCWSTR szReserved, 
    IN      LPWSTR  mszMachineList, 
    IN      LPDWORD pcchBufferSize 
); 
 
PDH_FUNCTION 
PdhEnumMachinesA ( 
    IN      LPCSTR   szReserved, 
    IN      LPSTR    mszMachineList, 
    IN      LPDWORD  pcchBufferSize 
); 
 
PDH_FUNCTION 
PdhEnumObjectsW ( 
    IN      LPCWSTR szReserved, 
    IN      LPCWSTR szMachineName, 
    IN      LPWSTR  mszObjectList, 
    IN      LPDWORD pcchBufferSize, 
    IN      DWORD   dwDetailLevel, 
    IN      BOOL    bRefresh 
); 
 
PDH_FUNCTION 
PdhEnumObjectsA ( 
    IN      LPCSTR  szReserved, 
    IN      LPCSTR  szMachineName, 
    IN      LPSTR   mszObjectList, 
    IN      LPDWORD pcchBufferSize, 
    IN      DWORD   dwDetailLevel, 
    IN      BOOL    bRefresh 
); 
 
PDH_FUNCTION 
PdhEnumObjectItemsW ( 
    IN      LPCWSTR szReserved, 
    IN      LPCWSTR szMachineName, 
    IN      LPCWSTR szObjectName, 
    IN      LPWSTR  mszCounterList, 
    IN      LPDWORD pcchCounterListLength, 
    IN      LPWSTR  mszInstanceList, 
    IN      LPDWORD pcchInstanceListLength, 
    IN      DWORD   dwDetailLevel, 
    IN      DWORD   dwFlags 
); 
 
PDH_FUNCTION 
PdhEnumObjectItemsA ( 
    IN      LPCSTR  szReserved, 
    IN      LPCSTR  szMachineName, 
    IN      LPCSTR  szObjectName, 
    IN      LPSTR   mszCounterList, 
    IN      LPDWORD pcchCounterListLength, 
    IN      LPSTR   mszInstanceList, 
    IN      LPDWORD pcchInstanceListLength, 
    IN      DWORD   dwDetailLevel, 
    IN      DWORD   dwFlags 
); 
 
PDH_FUNCTION 
PdhMakeCounterPathW ( 
    IN      PDH_COUNTER_PATH_ELEMENTS_W *pCounterPathElements, 
    IN      LPWSTR                      szFullPathBuffer, 
    IN      LPDWORD                     pcchBufferSize, 
    IN      DWORD                       dwFlags 
); 
 
PDH_FUNCTION 
PdhMakeCounterPathA ( 
    IN      PDH_COUNTER_PATH_ELEMENTS_A *pCounterPathElements, 
    IN      LPSTR                       szFullPathBuffer, 
    IN      LPDWORD                     pcchBufferSize, 
    IN      DWORD                       dwFlags 
); 
 
PDH_FUNCTION 
PdhParseCounterPathW ( 
    IN      LPCWSTR                     szFullPathBuffer, 
    IN      PDH_COUNTER_PATH_ELEMENTS_W *pCounterPathElements, 
    IN      LPDWORD                     pdwBufferSize, 
    IN      DWORD                       dwFlags 
); 
 
PDH_FUNCTION 
PdhParseCounterPathA ( 
    IN      LPCSTR                      szFullPathBuffer, 
    IN      PDH_COUNTER_PATH_ELEMENTS_A *pCounterPathElements, 
    IN      LPDWORD                     pdwBufferSize, 
    IN      DWORD                       dwFlags 
); 
 
PDH_FUNCTION 
PdhParseInstanceNameW ( 
    IN      LPCWSTR szInstanceString, 
    IN      LPWSTR  szInstanceName, 
    IN      LPDWORD pcchInstanceNameLength, 
    IN      LPWSTR  szParentName, 
    IN      LPDWORD pcchParentNameLength, 
    IN      LPDWORD lpIndex 
); 
 
PDH_FUNCTION 
PdhParseInstanceNameA ( 
    IN      LPCSTR  szInstanceString, 
    IN      LPSTR   szInstanceName, 
    IN      LPDWORD pcchInstanceNameLength, 
    IN      LPSTR   szParentName, 
    IN      LPDWORD pcchParentNameLength, 
    IN      LPDWORD lpIndex 
); 
 
PDH_FUNCTION 
PdhValidatePathW ( 
    IN      LPCWSTR szFullPathBuffer 
); 
 
PDH_FUNCTION 
PdhValidatePathA ( 
    IN      LPCSTR  szFullPathBuffer 
); 
 
PDH_FUNCTION 
PdhGetDefaultPerfObjectW ( 
    IN      LPCWSTR szReserved, 
    IN      LPCWSTR szMachineName, 
    IN      LPWSTR  szDefaultObjectName, 
    IN      LPDWORD pcchBufferSize 
); 
 
PDH_FUNCTION 
PdhGetDefaultPerfObjectA ( 
    IN      LPCSTR  szReserved, 
    IN      LPCSTR  szMachineName, 
    IN      LPSTR   szDefaultObjectName, 
    IN      LPDWORD pcchBufferSize 
); 
 
PDH_FUNCTION 
PdhGetDefaultPerfCounterW ( 
    IN      LPCWSTR szReserved, 
    IN      LPCWSTR szMachineName, 
    IN      LPCWSTR szObjectName, 
    IN      LPWSTR  szDefaultCounterName, 
    IN      LPDWORD pcchBufferSize 
); 
 
PDH_FUNCTION 
PdhGetDefaultPerfCounterA ( 
    IN      LPCSTR  szReserved, 
    IN      LPCSTR  szMachineName, 
    IN      LPCSTR  szObjectName, 
    IN      LPSTR   szDefaultCounterName, 
    IN      LPDWORD pcchBufferSize 
); 
 
typedef PDH_STATUS (__stdcall *CounterPathCallBack)(DWORD); 
 
typedef struct _BrowseDlgConfig_W { 
    // Configuration flags 
    DWORD   bIncludeInstanceIndex:1, 
            bSingleCounterPerAdd:1, 
            bSingleCounterPerDialog:1, 
            bLocalCountersOnly:1, 
            bWildCardInstances:1, 
            bHideDetailBox:1, 
            bInitializePath:1, 
            bDisableMachineSelection:1, 
            bReserved:24; 
 
    HWND                hWndOwner; 
    LPWSTR              szReserved; 
    LPWSTR              szReturnPathBuffer; 
    DWORD               cchReturnPathLength; 
    CounterPathCallBack pCallBack; 
    DWORD               dwCallBackArg; 
    PDH_STATUS          CallBackStatus; 
    DWORD               dwDefaultDetailLevel; 
    LPWSTR              szDialogBoxCaption; 
} PDH_BROWSE_DLG_CONFIG_W, *PPDH_BROWSE_DLG_CONFIG_W; 
 
typedef struct _BrowseDlgConfig_A { 
    // Configuration flags 
    DWORD   bIncludeInstanceIndex:1, 
            bSingleCounterPerAdd:1, 
            bSingleCounterPerDialog:1, 
            bLocalCountersOnly:1, 
            bWildCardInstances:1, 
            bHideDetailBox:1, 
            bInitializePath:1, 
            bDisableMachineSelection:1, 
            bReserved:24; 
 
    HWND                hWndOwner; 
    LPSTR               szReserved; 
    LPSTR               szReturnPathBuffer; 
    DWORD               cchReturnPathLength; 
    CounterPathCallBack pCallBack; 
    DWORD               dwCallBackArg; 
    PDH_STATUS          CallBackStatus; 
    DWORD               dwDefaultDetailLevel; 
    LPSTR               szDialogBoxCaption; 
} PDH_BROWSE_DLG_CONFIG_A, *PPDH_BROWSE_DLG_CONFIG_A; 
 
PDH_FUNCTION 
PdhBrowseCountersW ( 
    IN      PPDH_BROWSE_DLG_CONFIG_W    pBrowseDlgData 
); 
 
PDH_FUNCTION 
PdhBrowseCountersA ( 
    IN      PPDH_BROWSE_DLG_CONFIG_A    pBrowseDlgData 
); 
 
PDH_FUNCTION 
PdhExpandCounterPathW ( 
    IN      LPCWSTR     szWildCardPath, 
    IN      LPWSTR      mszExpandedPathList, 
    IN      LPDWORD     pcchPathListLength 
); 
 
PDH_FUNCTION 
PdhExpandCounterPathA ( 
    IN      LPCSTR      szWildCardPath, 
    IN      LPSTR       mszExpandedPathList, 
    IN      LPDWORD     pcchPathListLength 
); 
 
// 
//   Unicode/ANSI compatibility section 
// 
#ifdef UNICODE 
#ifndef _UNICODE 
#define _UNICODE 
#endif 
#endif 
 
#ifdef _UNICODE 
#ifndef UNICODE 
#define UNICODE 
#endif 
#endif 
 
#ifdef UNICODE 
// start of UNICODE definitions 
#define PdhAddCounter            PdhAddCounterW 
#define PdhGetCounterInfo           PdhGetCounterInfoW 
#define PDH_COUNTER_INFO        PDH_COUNTER_INFO_W 
#define PPDH_COUNTER_INFO        PPDH_COUNTER_INFO_W 
#define PdhConnectMachine           PdhConnectMachineW 
#define PdhEnumMachines             PdhEnumMachinesW 
#define PdhEnumObjects              PdhEnumObjectsW 
#define PdhEnumObjectItems          PdhEnumObjectItemsW 
#define PdhMakeCounterPath          PdhMakeCounterPathW 
#define PDH_COUNTER_PATH_ELEMENTS   PDH_COUNTER_PATH_ELEMENTS_W 
#define PPDH_COUNTER_PATH_ELEMENTS  PPDH_COUNTER_PATH_ELEMENTS_W 
#define PdhParseCounterPath         PdhParseCounterPathW 
#define PdhParseInstanceName        PdhParseInstanceNameW 
#define PdhValidatePath             PdhValidatePathW 
#define PdhGetDefaultPerfObject     PdhGetDefaultPerfObjectW 
#define PdhGetDefaultPerfCounter    PdhGetDefaultPerfCounterW 
#define PdhBrowseCounters           PdhBrowseCountersW 
#define PDH_BROWSE_DLG_CONFIG       PDH_BROWSE_DLG_CONFIG_W 
#define PPDH_BROWSE_DLG_CONFIG      PPDH_BROWSE_DLG_CONFIG_W 
#define PdhExpandCounterPath        PdhExpandCounterPathW 
// end of UNICODE definitions 
#else  
// start of ANSI definitions 
#define PdhAddCounter            PdhAddCounterA 
#define PdhGetCounterInfo           PdhGetCounterInfoA 
#define PDH_COUNTER_INFO        PDH_COUNTER_INFO_A 
#define PPDH_COUNTER_INFO        PPDH_COUNTER_INFO_A 
#define PdhConnectMachine           PdhConnectMachineA 
#define PdhEnumMachines             PdhEnumMachinesA 
#define PdhEnumObjects              PdhEnumObjectsA 
#define PdhEnumObjectItems          PdhEnumObjectItemsA 
#define PdhMakeCounterPath          PdhMakeCounterPathA 
#define PDH_COUNTER_PATH_ELEMENTS   PDH_COUNTER_PATH_ELEMENTS_A 
#define PPDH_COUNTER_PATH_ELEMENTS  PPDH_COUNTER_PATH_ELEMENTS_A 
#define PdhParseCounterPath         PdhParseCounterPathA 
#define PdhParseInstanceName        PdhParseInstanceNameA 
#define PdhValidatePath             PdhValidatePathA 
#define PdhGetDefaultPerfObject     PdhGetDefaultPerfObjectA 
#define PdhGetDefaultPerfCounter    PdhGetDefaultPerfCounterA 
#define PdhBrowseCounters           PdhBrowseCountersA 
#define PDH_BROWSE_DLG_CONFIG       PDH_BROWSE_DLG_CONFIG_A 
#define PPDH_BROWSE_DLG_CONFIG      PPDH_BROWSE_DLG_CONFIG_A 
#define PdhExpandCounterPath        PdhExpandCounterPathA 
// end of ANSI definitions 
#endif  // UNICODE 
 
#ifdef __cplusplus 
} 
#endif 
 
#endif //_PDH_H_ 
