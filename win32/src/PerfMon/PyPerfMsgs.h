/*--
Copyright (c) 1992  Microsoft Corporation

Module Name:

    pyperfmsgs.h
       (derived from pyperfmsgs.mc by the message compiler  )

Abstract:

   Event message definitions used by routines

Revision History:

--*/
//
#ifndef _PYPERFMSG_H_
#define _PYPERFMSG_H_
//
//
//     Perfutil messages
//
//
//  Values are 32 bit values layed out as follows:
//
//   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//  +---+-+-+-----------------------+-------------------------------+
//  |Sev|C|R|     Facility          |               Code            |
//  +---+-+-+-----------------------+-------------------------------+
//
//  where
//
//      Sev - is the severity code
//
//          00 - Success
//          01 - Informational
//          10 - Warning
//          11 - Error
//
//      C - is the Customer code flag
//
//      R - is a reserved bit
//
//      Facility - is the facility code
//
//      Code - is the facility's status code
//
//
// Define the facility codes
//


//
// Define the severity codes
//


//
// MessageId: UTIL_LOG_OPEN
//
// MessageText:
//
//  An extensible counter has opened the Event Log.
//
#define UTIL_LOG_OPEN                    ((DWORD)0x4000076CL)

//
//
// MessageId: UTIL_CLOSING_LOG
//
// MessageText:
//
//  An extensible counter has closed the Event Log.
//
#define UTIL_CLOSING_LOG                 ((DWORD)0x400007CFL)

//
//
// MessageId: PERFDATA_OPEN_FILE_MAPPING_ERROR
//
// MessageText:
//
//  Unable to open mapped file containing the performance data.
//
#define PERFDATA_OPEN_FILE_MAPPING_ERROR ((DWORD)0xC00007D0L)

//
//
// MessageId: PERFDATA_UNABLE_MAP_VIEW_OF_FILE
//
// MessageText:
//
//  Unable to map to shared memory file containing the performance data.
//
#define PERFDATA_UNABLE_MAP_VIEW_OF_FILE ((DWORD)0xC00007D1L)

//
//
// MessageId: PERFDATA_UNABLE_OPEN_DRIVER_KEY
//
// MessageText:
//
//  Unable open "Performance" key of application in registry. Status code is returned in data.
//
#define PERFDATA_UNABLE_OPEN_DRIVER_KEY  ((DWORD)0xC00007D2L)

//
//
// MessageId: PERFDATA_UNABLE_READ_FIRST_COUNTER
//
// MessageText:
//
//  Unable to read the "First Counter" value under the {application}\Performance Key. Status codes returned in data.
//
#define PERFDATA_UNABLE_READ_FIRST_COUNTER ((DWORD)0xC00007D3L)

//
//
// MessageId: PERFDATA_UNABLE_READ_FIRST_HELP
//
// MessageText:
//
//  Unable to read the "First Help" value under the {application}\Performance Key. Status codes returned in data.
//
#define PERFDATA_UNABLE_READ_FIRST_HELP  ((DWORD)0xC00007D4L)

//
// MessageId: PERFDATA_STRUCTURE_MISMATCH
//
// MessageText:
//
//  The collection DLL and application have mismatched structure sizes.  The versions are probably not in synch.
//
#define PERFDATA_STRUCTURE_MISMATCH      ((DWORD)0xC00007D5L)

//
#endif // _PYPERFMSG_H_