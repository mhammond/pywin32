
//
//  Include Files
//
#include <windows.h>
#include <string.h>
#include <tchar.h>
#include <winperf.h>
// error message definition
#include "PyPerfMsgs.h"
#include "perfutil.h"
#include "PyPerfMonControl.h"

//// The constant below defines how many (if any) messages will be reported
// to the event logger. As the number goes up in value more and more events
// will be reported. The purpose of this is to allow lots of messages during
// development and debugging (e.g. a message level of 3) to a minimum of
// messages (e.g. operational messages with a level of 1) or no messages if
// message logging inflicts too much of a performance penalty. Right now
// this is a compile time constant, but could later become a registry entry.
//
//    Levels:  LOG_NONE = No event log messages ever
//             LOG_USER = User event log messages (e.g. errors)
//             LOG_DEBUG = Minimum Debugging 
//             LOG_VERBOSE = Maximum Debugging 

//#define  LOG_NONE     0
#define  LOG_USER     1
#define  LOG_DEBUG    2
#define  LOG_VERBOSE  3
#define  MESSAGE_LEVEL_DEFAULT  LOG_USER
// define macros//
// Format for event log calls without corresponding insertion strings is:
//    REPORT_xxx (message_value, message_level)
//       where:   
//          xxx is the severity to be displayed in the event log
//          message_value is the numeric ID from above
//          message_level is the "filtering" level of error reporting
//             using the error levels above.
//
// if the message has a corresponding insertion string whose symbol conforms
// to the format CONSTANT = numeric value and CONSTANT_S = string constant for
// that message, then the 
// 
//    REPORT_xxx_STRING (message_value, message_level)//// macro may be used.
//
//
// REPORT_SUCCESS was intended to show Success in the error log, rather it
// shows "N/A" so for now it's the same as information, though it could 
// (should) be changed  in the future
//
#define REPORT_SUCCESS(i,l) (MESSAGE_LEVEL >= l ? ReportEvent (hEventLog, EVENTLOG_INFORMATION_TYPE, \
   0, i, (PSID)NULL, 0, 0, NULL, (PVOID)NULL) : FALSE)
#define REPORT_INFORMATION(i,l) (MESSAGE_LEVEL >= l ? ReportEvent (hEventLog, EVENTLOG_INFORMATION_TYPE, \
   0, i, (PSID)NULL, 0, 0, NULL, (PVOID)NULL) : FALSE)
#define REPORT_WARNING(i,l) (MESSAGE_LEVEL >= l ? ReportEvent (hEventLog, EVENTLOG_WARNING_TYPE, \
   0, i, (PSID)NULL, 0, 0, NULL, (PVOID)NULL) : FALSE)
#define REPORT_ERROR(i,l) (MESSAGE_LEVEL >= l ? ReportEvent (hEventLog, EVENTLOG_ERROR_TYPE, \
   0, i, (PSID)NULL, 0, 0, NULL, (PVOID)NULL) : FALSE)
#define REPORT_INFORMATION_DATA(i,l,d,s) (MESSAGE_LEVEL >= l ? ReportEvent (hEventLog, EVENTLOG_INFORMATION_TYPE, \
   0, i, (PSID)NULL, 0, s, NULL, (PVOID)(d)) : FALSE)
#define REPORT_WARNING_DATA(i,l,d,s) (MESSAGE_LEVEL >= l ? ReportEvent (hEventLog, EVENTLOG_WARNING_TYPE, \
   0, i, (PSID)NULL, 0, s, NULL, (PVOID)(d)) : FALSE)
#define REPORT_ERROR_DATA(i,l,d,s) (MESSAGE_LEVEL >= l ? ReportEvent (hEventLog, EVENTLOG_ERROR_TYPE, \
   0, i, (PSID)NULL, 0, s, NULL, (PVOID)(d)) : FALSE)

// External Variables
DWORD  dwLogUsers;  // counter of event log using routines
DWORD  MESSAGE_LEVEL = 0; // event logging detail level


//  References to constants which initialize the Object type definitions
//
DWORD   dwOpenCount = 0;        // count of "Open" threads
BOOL    bInitOK = FALSE;        // true = DLL initialized OK
//
// data structures
//
HANDLE hEventLog;             // Handle to the event log for errors.
HANDLE hSharedMemory;         // Handle of Vga Shared Memory
HINSTANCE hDllHandle;         // Handle to this DLL.


MappingManagerControlData *pControlData;
PPERF_COUNTER_BLOCK pCounterBlock;
//
//  Function Prototypes
//
//      these are used to insure that the data collection functions
//      accessed by Perflib will have the correct calling format.
//
PM_OPEN_PROC   OpenPerformanceData;
PM_COLLECT_PROC    CollectPerformanceData;
PM_CLOSE_PROC  ClosePerformanceData;

TCHAR szFullModulePath[MAX_PATH];
TCHAR szModuleName[MAX_PATH]; // will point into the buffer above.

// These dwModuleFirst* variables are loaded in the open and are used to
// adjust the counter offsets.  However, we must update the offsets each
// run.
DWORD dwModuleFirstCounter;
DWORD dwModuleFirstHelp;    


BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	switch (dwReason) {
		case DLL_PROCESS_ATTACH: {
			hDllHandle = hInstance;
			GetModuleFileName(hDllHandle, szFullModulePath, sizeof(szFullModulePath)/sizeof(TCHAR));
			TCHAR *szStart = _tcsrchr(szFullModulePath, _T('\\'));
			if (szStart==NULL) 
				szStart = szFullModulePath;
			else
				szStart = szStart + 1; // skip the slash

			TCHAR *szEnd = _tcsrchr(szFullModulePath, _T('.'));
			UINT numBytes = szEnd - szStart;
			_tcsncpy(szModuleName, szStart, numBytes);
			szModuleName[numBytes]=_T('\0');

			break;
		}
		default:
			break;
	}
	return TRUE;    // ok
}



DWORD APIENTRY OpenPerformanceData( LPWSTR lpDeviceNames )
/*++
    Routine Description:

    This routine will open and map the memory used by the VGA driver to
    pass performance data in. This routine also initializes the data
    structures used to pass data back to the registryArguments:
    Pointer to object ID of each device to be opened (VGA)
	
	Return Value:    None.
--*/
{
    LONG status;
    HKEY hKeyDriverPerf;
	DWORD size;
	DWORD type;
	TCHAR registryKeyName[MAX_PATH];
	TCHAR szFileMapping[MAX_PATH+10] = _T("");

    // Use a TerminalServices friendly "Global\\" prefix if supported.
    OSVERSIONINFO info;
    info.dwOSVersionInfoSize = sizeof(info);
    GetVersionEx(&info);
    if (info.dwMajorVersion > 4)
        // 2000 or later - "Global\\" prefix OK.
        _tcscpy(szFileMapping, _T("Global\\"));
    _tcscat(szFileMapping, szModuleName);

	//
    //  Since SCREG is multi-threaded and will call this routine in
    //  order to service remote performance queries, this library
    //  must keep track of how many times it has been opened (i.e.
    //  how many threads have accessed it). the registry routines will
    //  limit access to the initialization routine to only one thread 
    //  at a time so synchronization (i.e. reentrancy) should not be 
    //  a problem    
	//
	if (!dwOpenCount) {        // open Eventlog interface
		// The memmapped file name is derived from the DLL name.
		// Later we may offer to look up a string resource, but not now!
		// NOTE - We dont open the event log yet, as we may wish to open it with a custom name
        // open shared memory used by application to pass performance values
        hSharedMemory = OpenFileMapping(FILE_MAP_READ,
                                FALSE,
						        szFileMapping);

        pCounterBlock = NULL;   
		// initialize pointer to memory
        // log error if unsuccessful        
		if (hSharedMemory == NULL) {
	        hEventLog = MonOpenEventLog(szModuleName);
            REPORT_ERROR (PERFDATA_OPEN_FILE_MAPPING_ERROR, LOG_USER);
            // this is fatal, if we can't get data then there's no
            // point in continuing.
            status = GetLastError(); // return error
            goto OpenExitPoint;
		}
        // map pointer to memory
        pControlData = (MappingManagerControlData *)
							MapViewOfFile(hSharedMemory,
                                          FILE_MAP_READ,
							              0, 0, 0);
        if (pControlData == NULL) {
	        hEventLog = MonOpenEventLog(szModuleName);
            REPORT_ERROR (PERFDATA_UNABLE_MAP_VIEW_OF_FILE, LOG_USER);
            // this is fatal, if we can't get data then there's no
            // point in continuing.
            status = GetLastError(); 
			// return error 
	        goto OpenExitPoint;
		}
		if (pControlData->ControlSize != sizeof(*pControlData)) {
	        hEventLog = MonOpenEventLog(szModuleName);
			REPORT_ERROR (PERFDATA_STRUCTURE_MISMATCH, LOG_USER);
	        goto OpenExitPoint;
		}
		// We now have an event name we can trust.
        hEventLog = MonOpenEventLog(pControlData->EventSourceName);


        // get counter and help index base values from registry
        //      Open key to registry entry
        //      read First Counter and First Help values
        //      update static data structures by adding base to 
        //          offset value in structure.
		_tcscpy(registryKeyName, _T("SYSTEM\\CurrentControlSet\\Services\\"));
		_tcscat(registryKeyName, pControlData->ServiceName);
		_tcscat(registryKeyName, _T("\\Performance"));
		status = RegOpenKeyEx (
					HKEY_LOCAL_MACHINE,
					registryKeyName,
					0L, KEY_ALL_ACCESS, &hKeyDriverPerf);
	    if (status != ERROR_SUCCESS) {
            REPORT_ERROR_DATA (PERFDATA_UNABLE_OPEN_DRIVER_KEY, LOG_USER, &status, sizeof(status));
            // this is fatal, if we can't get the base values of the 
            // counter or help names, then the names won't be available
            // to the requesting application  so there's not much
            // point in continuing.
			goto OpenExitPoint;
		}
        size = sizeof (DWORD);
		status = RegQueryValueEx(hKeyDriverPerf,
		                        _T("First Counter"),
								0L,
								&type,
								(LPBYTE)&dwModuleFirstCounter,
								&size);

        if (status != ERROR_SUCCESS) {
            REPORT_ERROR_DATA (PERFDATA_UNABLE_READ_FIRST_COUNTER, LOG_USER, &status, sizeof(status));
            // this is fatal, if we can't get the base values of the 
            // counter or help names, then the names won't be available
            // to the requesting application  so there's not much
            // point in continuing.            
			goto OpenExitPoint;
		}
        size = sizeof (DWORD);
		status = RegQueryValueEx(hKeyDriverPerf,
		                         _T("First Help"),
								 0L,
								 &type,
								 (LPBYTE)&dwModuleFirstHelp,
								 &size);

        if (status != ERROR_SUCCESS) {
            REPORT_ERROR_DATA (PERFDATA_UNABLE_READ_FIRST_HELP, LOG_USER, &status, sizeof(status));
            // this is fatal, if we can't get the base values of the 
            // counter or help names, then the names won't be available
            // to the requesting application  so there's not much
            // point in continuing.
			goto OpenExitPoint;
		} 
        //        //  NOTE: the initialization program could also retrieve
        //      LastCounter and LastHelp if they wanted to do 
        //      bounds checking on the new number. e.g.        //
        //      counter->CounterNameTitleIndex += dwFirstCounter;
        //      if (counter->CounterNameTitleIndex > dwLastCounter) {
        //          LogErrorToEventLog (INDEX_OUT_OF_BOUNDS);        //      }

        RegCloseKey (hKeyDriverPerf); // close key to registry
        bInitOK = TRUE; // ok to use this function    
	}
	dwOpenCount++;  // increment OPEN counter
	status = ERROR_SUCCESS; // for successful exit
OpenExitPoint:
	return status;
}

DWORD APIENTRY CollectPerformanceData(
    IN      LPWSTR  lpValueName,
	IN OUT  LPVOID  *lppData,
    IN OUT  LPDWORD lpcbTotalBytes,
	IN OUT  LPDWORD lpNumObjectTypes)
/*++
Routine Description:    This routine will return the data for the VGA counters.
Arguments:   IN       LPWSTR   lpValueName
         pointer to a wide character string passed by registry.
   IN OUT   LPVOID   *lppData
         IN: pointer to the address of the buffer to receive the completed 
            PerfDataBlock and subordinate structures. This routine will
            append its data to the buffer starting at the point referenced
            by *lppData.
         OUT: points to the first byte after the data structure added by this
            routine. This routine updated the value at lppdata after appending
            its data.   IN OUT   LPDWORD  lpcbTotalBytes
         IN: the address of the DWORD that tells the size in bytes of the 
            buffer referenced by the lppData argument
         OUT: the number of bytes added by this routine is written to the 
            DWORD pointed to by this argument   IN OUT   LPDWORD  NumObjectTypes
         IN: the address of the DWORD to receive the number of objects added 
            by this routine 
         OUT: the number of objects added by this routine is written to the 
            DWORD pointed to by this argumentReturn Value:
      ERROR_MORE_DATA if buffer passed is too small to hold data
         any error conditions encountered are reported to the event log if
         event logging is enabled.
      ERROR_SUCCESS  if success or any other error. Errors, however are
         also reported to the event log.
--*/
{
    //  Variables for reformatting the data    
	ULONG SpaceNeeded;
    // variables used for error logging
    DWORD		dwQueryType;    
	//
    // before doing anything else, see if Open went OK    
	//
	if (!bInitOK || pControlData->supplierStatus != SupplierStatusRunning) {
        // unable to continue because open failed,
		// or my supplier of data is not running
        *lpcbTotalBytes = (DWORD) 0;
		*lpNumObjectTypes = (DWORD) 0;
        return ERROR_SUCCESS; // yes, this is a successful exit
	}
	//
    // see if this is a foreign (i.e. non-NT) computer data request     
	//
    dwQueryType = GetQueryType (lpValueName);    
    if (dwQueryType == QUERY_FOREIGN) {
        // this routine does not service requests for data from
        // Non-NT computers
		*lpcbTotalBytes = (DWORD) 0;
        *lpNumObjectTypes = (DWORD) 0;
		return ERROR_SUCCESS;
	}
	PERF_OBJECT_TYPE *pPOT = (PERF_OBJECT_TYPE *)(pControlData + 1);
	PERF_OBJECT_TYPE *pPOTResult = (PERF_OBJECT_TYPE *)(*lppData);
    if (dwQueryType == QUERY_ITEMS) {
		if ( !(IsNumberInUnicodeList (pPOT->ObjectNameTitleIndex+dwModuleFirstCounter, 
			                          lpValueName))) {
			// request received for data object not provided by this routine
            *lpcbTotalBytes = (DWORD) 0;
            *lpNumObjectTypes = (DWORD) 0;
			return ERROR_SUCCESS;
		}
	}
    SpaceNeeded = pControlData->TotalSize - sizeof(*pControlData);
    if ( *lpcbTotalBytes < SpaceNeeded ) {
	    *lpcbTotalBytes = (DWORD) 0;
        *lpNumObjectTypes = (DWORD) 0;
		return ERROR_MORE_DATA;
	}
	//
    // Copy the Object Type and counter definitions
    //  to the caller's data buffer 
	//
	memmove(pPOTResult,
            pPOT,
			SpaceNeeded);

	// Update all the counter and help values with the new offset
	pPOTResult->ObjectNameTitleIndex += dwModuleFirstCounter;
	pPOTResult->ObjectHelpTitleIndex += dwModuleFirstHelp;

	PERF_COUNTER_DEFINITION *pPCD = (PERF_COUNTER_DEFINITION *)(pPOTResult+1);
	for (DWORD i=0;i<pPOT->NumCounters;i++) {
		pPCD[i].CounterNameTitleIndex += dwModuleFirstCounter;
		pPCD[i].CounterHelpTitleIndex += dwModuleFirstHelp;
	}
	// debug !!
	PERF_COUNTER_BLOCK *pPerfCounterBlock = (PERF_COUNTER_BLOCK *)(((LPBYTE)pPOTResult)+(pPOT->NumCounters*sizeof(PERF_COUNTER_DEFINITION))+sizeof(PERF_OBJECT_TYPE));
	*lppData = (LPBYTE)(*lppData)+SpaceNeeded;
	// update arguments fore return    
    *lpNumObjectTypes = 1;
    *lpcbTotalBytes = SpaceNeeded;

    return ERROR_SUCCESS;
}

DWORD APIENTRY ClosePerformanceData()
/*++
Routine Description:
    This routine closes the open handles to VGA device performance counters

Arguments:

    None.

Return Value:

    ERROR_SUCCESS

--*/
{
    if (!(--dwOpenCount)) { // when this is the last thread...
        CloseHandle(hSharedMemory);
		pCounterBlock = NULL;
        MonCloseEventLog();
    }
	return ERROR_SUCCESS;
}






#define INITIAL_SIZE     1024L
#define EXTEND_SIZE      1024L
//
// Global data definitions.
//
ULONG                   ulInfoBufferSize = 0;
WCHAR GLOBAL_STRING[] = L"Global";
WCHAR FOREIGN_STRING[] = L"Foreign";
WCHAR COSTLY_STRING[] = L"Costly";
WCHAR NULL_STRING[] = L"\0";
// pointer to null string 
// test for delimiter, end of line and non-digit characters
// used by IsNumberInUnicodeList routine
//
#define DIGIT       1
#define DELIMITER   2
#define INVALID     3
#define EvalThisChar(c,d) ( \
     (c == d) ? DELIMITER : \
	 (c == 0) ? DELIMITER : \
     (c < (WCHAR)'0') ? INVALID : \
	 (c > (WCHAR)'9') ? INVALID : \
     DIGIT)
	 
HANDLE MonOpenEventLog (const TCHAR *szSourceName)
/*++Routine Description:
    Reads the level of event logging from the registry and opens the
        channel to the event logger for subsequent event log entries
		
	Arguments:
      None
	  
	Return Value:   
	  Handle to the event log for reporting events.
      NULL if open not successful.
--*/
{
    HKEY hAppKey;
    TCHAR LogLevelKeyName[] = _T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Perflib");

    TCHAR LogLevelValueName[] = _T("EventLogLevel");
	LONG lStatus;
    DWORD dwLogLevel;
	DWORD dwValueType;
	DWORD dwValueSize;   
    // if global value of the logging level not initialized or is disabled, 
    //  check the registry to see if it should be updated.
    if (!MESSAGE_LEVEL) {
		lStatus = RegOpenKeyEx (HKEY_LOCAL_MACHINE,
                               LogLevelKeyName,
                               0,                         
                               KEY_READ,
                               &hAppKey);
		dwValueSize = sizeof (dwLogLevel);
		if (lStatus == ERROR_SUCCESS) {
			lStatus = RegQueryValueEx (hAppKey,
                               LogLevelValueName,
                               (LPDWORD)NULL,           
                               &dwValueType,
                               (LPBYTE)&dwLogLevel,
                               &dwValueSize);
            if (lStatus == ERROR_SUCCESS) {
				MESSAGE_LEVEL = dwLogLevel;
			} else {
				MESSAGE_LEVEL = MESSAGE_LEVEL_DEFAULT;
			}
            RegCloseKey (hAppKey);
		} else {
			MESSAGE_LEVEL = MESSAGE_LEVEL_DEFAULT;
		}
	}       
	if (hEventLog == NULL){
		hEventLog = RegisterEventSource (
            (LPTSTR)NULL,            // Use Local Machine
            szSourceName);               // event log app name to find in registry
		if (hEventLog != NULL) {
			REPORT_INFORMATION (UTIL_LOG_OPEN, LOG_DEBUG);
         }
    }
    if (hEventLog != NULL) {
		dwLogUsers++;           // increment count of perfctr log users    
	}
	return (hEventLog);
}

VOID MonCloseEventLog ()

/*++Routine Description:
      Closes the handle to the event logger if this is the last caller      
Arguments:      None

Return Value:      None
--*/
{
	if (hEventLog != NULL) {
		dwLogUsers--;         // decrement usage
		if (dwLogUsers <= 0) {    // and if we're the last, then close up log
			REPORT_INFORMATION (UTIL_CLOSING_LOG, LOG_DEBUG);
			DeregisterEventSource (hEventLog);
		}
	}
}

DWORD GetQueryType (
    IN LPWSTR lpValue)
/*++GetQueryType
    returns the type of query described in the lpValue string so that
    the appropriate processing method may be used
	
	Arguments    
	IN lpValue
        string passed to PerfRegQuery Value for processing
		
Return Value
    QUERY_GLOBAL        if lpValue == 0 (null pointer)
           lpValue == pointer to Null string
           lpValue == pointer to "Global" string    QUERY_FOREIGN
        if lpValue == pointer to "Foreign" string    QUERY_COSTLY
        if lpValue == pointer to "Costly" string    otherwise:    QUERY_ITEMS
--*/
{
	WCHAR   *pwcArgChar, *pwcTypeChar;
	BOOL    bFound;
	if (lpValue == 0) {
		return QUERY_GLOBAL;
    } else if (*lpValue == 0) {
		return QUERY_GLOBAL;    
	}
    // check for "Global" request
	pwcArgChar = lpValue;
    pwcTypeChar = GLOBAL_STRING;
    bFound = TRUE;  // assume found until contradicted
    // check to the length of the shortest string    
    while ((*pwcArgChar != 0) && (*pwcTypeChar != 0)) {
        if (*pwcArgChar++ != *pwcTypeChar++) {
            bFound = FALSE; // no match
            break;          // bail out now
		}
	}
    if (bFound) return QUERY_GLOBAL;    // check for "Foreign" request    
    pwcArgChar = lpValue;
	pwcTypeChar = FOREIGN_STRING;
    bFound = TRUE;  // assume found until contradicted
    // check to the length of the shortest string    
    while ((*pwcArgChar != 0) && (*pwcTypeChar != 0)) {
		if (*pwcArgChar++ != *pwcTypeChar++) {
            bFound = FALSE; // no match
            break;          // bail out now        
		}
	}
    if (bFound) return QUERY_FOREIGN;    // check for "Costly" request    

    pwcArgChar = lpValue;
	pwcTypeChar = COSTLY_STRING;
    bFound = TRUE;  // assume found until contradicted
    // check to the length of the shortest string    
    while ((*pwcArgChar != 0) && (*pwcTypeChar != 0)) {
        if (*pwcArgChar++ != *pwcTypeChar++) {
            bFound = FALSE; // no match
            break;          // bail out now
		}
	}
    if (bFound) return QUERY_COSTLY;

    // if not Global and not Foreign and not Costly, 
    // then it must be an item list
	return QUERY_ITEMS;
}

BOOL IsNumberInUnicodeList (    
	IN DWORD   dwNumber,
	IN LPWSTR  lpwszUnicodeList)
/*++
IsNumberInUnicodeList
Arguments:            
	IN dwNumber
        DWORD number to find in list    IN lpwszUnicodeList
        Null terminated, Space delimited list of decimal numbers
		
Return Value:
    TRUE:            dwNumber was found in the list of unicode number strings
    FALSE:            dwNumber was not found in the list.
--*/
{
    DWORD   dwThisNumber;    
	WCHAR   *pwcThisChar;    
	BOOL    bValidNumber;
    BOOL    bNewItem;    
    WCHAR   wcDelimiter;    
// could be an argument to be more flexible
    if (lpwszUnicodeList == 0) return FALSE;    // null pointer, # not found
    pwcThisChar = lpwszUnicodeList;
	dwThisNumber = 0;
    wcDelimiter = (WCHAR)' ';
	bValidNumber = FALSE;
	bNewItem = TRUE;    
    while (TRUE) {
		switch (EvalThisChar (*pwcThisChar, wcDelimiter)) {
            case DIGIT:
                // if this is the first digit after a delimiter, then 
                // set flags to start computing the new number
                if (bNewItem) {                    
					bNewItem = FALSE;
                    bValidNumber = TRUE;
				}
                if (bValidNumber) {
					dwThisNumber *= 10;
					dwThisNumber += (*pwcThisChar - (WCHAR)'0');
                }                
				break;
			case DELIMITER:
                // a delimiter is either the delimiter character or the 
                // end of the string ('\0') if when the delimiter has been
                // reached a valid number was found, then compare it to the
                // number from the argument list. if this is the end of the
                // string and no match was found, then return.                //
                if (bValidNumber) {
                    if (dwThisNumber == dwNumber) return TRUE;
                    bValidNumber = FALSE;                
				}
                if (*pwcThisChar == 0) {                    
					return FALSE;
                } else {
					bNewItem = TRUE;
                    dwThisNumber = 0;
				}
				break;
            case INVALID:
                // if an invalid character was encountered, ignore all
                // characters up to the next delimiter and then start fresh.
                // the invalid number is not compared.
                bValidNumber = FALSE;
				break;
			default:
                break;
		}
		pwcThisChar++;    
	}
}   // IsNumberInUnicodeList
