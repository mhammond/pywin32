;/*--
;Copyright (c) 1992  Microsoft Corporation
;
;Module Name:
;
;    pyperfmsgs.h
;       (derived from pyperfmsgs.mc by the message compiler  )
;
;Abstract:
;
;   Event message definitions used by routines
;
;Revision History:
;
;--*/
;//
;#ifndef _PYPERFMSG_H_
;#define _PYPERFMSG_H_
;//
MessageIdTypedef=DWORD
;//
;//     Perfutil messages
;//
MessageId=1900
Severity=Informational
Facility=Application
SymbolicName=UTIL_LOG_OPEN
Language=English
An extensible counter has opened the Event Log.
.
;//
MessageId=1999
Severity=Informational
Facility=Application
SymbolicName=UTIL_CLOSING_LOG
Language=English
An extensible counter has closed the Event Log.
.
;//
MessageId=2000
Severity=Error
Facility=Application
SymbolicName=PERFDATA_OPEN_FILE_MAPPING_ERROR
Language=English
Unable to open mapped file containing the performance data.
.
;//
MessageId=+1
Severity=Error
Facility=Application
SymbolicName=PERFDATA_UNABLE_MAP_VIEW_OF_FILE
Language=English
Unable to map to shared memory file containing the performance data.
.
;//
MessageId=+1
Severity=Error
Facility=Application
SymbolicName=PERFDATA_UNABLE_OPEN_DRIVER_KEY
Language=English
Unable open "Performance" key of application in registry. Status code is returned in data.
.
;//
MessageId=+1
Severity=Error
Facility=Application
SymbolicName=PERFDATA_UNABLE_READ_FIRST_COUNTER
Language=English
Unable to read the "First Counter" value under the {application}\Performance Key. Status codes returned in data.
.
;//
MessageId=+1
Severity=Error
Facility=Application
SymbolicName=PERFDATA_UNABLE_READ_FIRST_HELP
Language=English
Unable to read the "First Help" value under the {application}\Performance Key. Status codes returned in data.
.

MessageId=+1
Severity=Error
Facility=Application
SymbolicName=PERFDATA_STRUCTURE_MISMATCH
Language=English
The collection DLL and application have mismatched structure sizes.  The versions are probably not in synch.
.
;//
;#endif // _PYPERFMSG_H_