; /*
; Microsoft Developer Support
; Copyright (c) 1992 Microsoft Corporation
;
; This file contains the message definitions for the Win32
; messages.exe sample program.

;-------------------------------------------------------------------------
; HEADER SECTION
;
; The header section defines names and language identifiers for use
; by the message definitions later in this file. The MessageIdTypedef,
; SeverityNames, FacilityNames, and LanguageNames keywords are
; optional and not required.
;
;
MessageIdTypedef=DWORD
;
; The MessageIdTypedef keyword gives a typedef name that is used in a
; type cast for each message code in the generated include file. Each
; message code appears in the include file with the format: #define
; name ((type) 0xnnnnnnnn) The default value for type is empty, and no
; type cast is generated. It is the programmer's responsibility to
; specify a typedef statement in the application source code to define
; the type. The type used in the typedef must be large enough to
; accomodate the entire 32-bit message code.
;
;

; Use Default

;
; The SeverityNames keyword defines the set of names that are allowed
; as the value of the Severity keyword in the message definition. The
; set is delimited by left and right parentheses. Associated with each
; severity name is a number that, when shifted left by 30, gives the
; bit pattern to logical-OR with the Facility value and MessageId
; value to form the full 32-bit message code. The default value of
; this keyword is:
;
; SeverityNames=(
;   Success=0x0
;   Informational=0x1
;   Warning=0x2
;   Error=0x3
;   )
;
; Severity values occupy the high two bits of a 32-bit message code.
; Any severity value that does not fit in two bits is an error. The
; severity codes can be given symbolic names by following each value
; with :name
;
;

; XXX - Default facility names?

; The FacilityNames keyword defines the set of names that are allowed
; as the value of the Facility keyword in the message definition. The
; set is delimited by left and right parentheses. Associated with each
; facility name is a number that, when shift it left by 16 bits, gives
; the bit pattern to logical-OR with the Severity value and MessageId
; value to form the full 32-bit message code. The default value of
; this keyword is:
;
; FacilityNames=(
;   System=0x0FF
;   Application=0xFFF
;   )
;
; Facility codes occupy the low order 12 bits of the high order
; 16-bits of a 32-bit message code. Any facility code that does not
; fit in 12 bits is an error. This allows for 4,096 facility codes.
; The first 256 codes are reserved for use by the system software. The
; facility codes can be given symbolic names by following each value
; with :name
;
;
; The LanguageNames keyword defines the set of names that are allowed
; as the value of the Language keyword in the message definition. The
; set is delimited by left and right parentheses. Associated with each
; language name is a number and a file name that are used to name the
; generated resource file that contains the messages for that
; language. The number corresponds to the language identifier to use
; in the resource table. The number is separated from the file name
; with a colon. The initial value of LanguageNames is:
;
; LanguageNames=(English=1:MSG00001)
;
; Any new names in the source file which don't override the built-in
; names are added to the list of valid languages. This allows an
; application to support private languages with descriptive names.
;
;
;-------------------------------------------------------------------------
; MESSAGE DEFINITION SECTION
;
; Following the header section is the body of the Message Compiler
; source file. The body consists of zero or more message definitions.
; Each message definition begins with one or more of the following
; statements:
;
; MessageId = [number|+number]
; Severity = severity_name
; Facility = facility_name
; SymbolicName = name
;
; The MessageId statement marks the beginning of the message
; definition. A MessageID statement is required for each message,
; although the value is optional. If no value is specified, the value
; used is the previous value for the facility plus one. If the value
; is specified as +number then the value used is the previous value
; for the facility, plus the number after the plus sign. Otherwise, if
; a numeric value is given, that value is used. Any MessageId value
; that does not fit in 16 bits is an error.
;
; The Severity and Facility statements are optional. These statements
; specify additional bits to OR into the final 32-bit message code. If
; not specified they default to the value last specified for a message
; definition. The initial values prior to processing the first message
; definition are:
;
; Severity=Success
; Facility=Application
;
; The value associated with Severity and Facility must match one of
; the names given in the FacilityNames and SeverityNames statements in
; the header section. The SymbolicName statement allows you to
; associate a C/C++ symbolic constant with the final 32-bit message
; code.
; */

MessageId=0x1
Severity=Error
SymbolicName=MSG_ER1
Language=English
%1
.

MessageId=0x2
Severity=Error
SymbolicName=MSG_ER2
Language=English
%1
.
MessageId=0x3
Severity=Error
SymbolicName=MSG_ER3
Language=English
%1
.
MessageId=0x4
Severity=Error
SymbolicName=MSG_ER4
Language=English
%1
.
MessageId=0x5
Severity=Error
SymbolicName=MSG_ER5
Language=English
%1
.
MessageId=0x6
Severity=Error
SymbolicName=MSG_ER6
Language=English
%1
.
MessageId=0x7
Severity=Error
SymbolicName=MSG_ER7
Language=English
%1
.
MessageId=0x8
Severity=Error
SymbolicName=MSG_ER8
Language=English
%1
.
MessageId=0x9
Severity=Error
SymbolicName=MSG_ER9
Language=English
%1
.

MessageId=0x1
Severity=Informational
SymbolicName=MSG_IR1
Language=English
%1
.

MessageId=0x2
Severity=Informational
SymbolicName=MSG_IR2
Language=English
%1
.
MessageId=0x3
Severity=Informational
SymbolicName=MSG_IR3
Language=English
%1
.
MessageId=0x4
Severity=Informational
SymbolicName=MSG_IR4
Language=English
%1
.
MessageId=0x5
Severity=Informational
SymbolicName=MSG_IR5
Language=English
%1
.
MessageId=0x6
Severity=Informational
SymbolicName=MSG_IR6
Language=English
%1
.
MessageId=0x7
Severity=Informational
SymbolicName=MSG_IR7
Language=English
%1
.
MessageId=0x8
Severity=Informational
SymbolicName=MSG_IR8
Language=English
%1
.
MessageId=0x9
Severity=Informational
SymbolicName=MSG_IR9
Language=English
%1
.


MessageId=0x1
Severity=Success
SymbolicName=MSG_SR1
Language=English
%1
.

MessageId=0x2
Severity=Success
SymbolicName=MSG_SR2
Language=English
%1
.
MessageId=0x3
Severity=Success
SymbolicName=MSG_SR3
Language=English
%1
.
MessageId=0x4
Severity=Success
SymbolicName=MSG_SR4
Language=English
%1
.
MessageId=0x5
Severity=Success
SymbolicName=MSG_SR5
Language=English
%1
.
MessageId=0x6
Severity=Success
SymbolicName=MSG_SR6
Language=English
%1
.
MessageId=0x7
Severity=Success
SymbolicName=MSG_SR7
Language=English
%1
.
MessageId=0x8
Severity=Success
SymbolicName=MSG_SR8
Language=English
%1
.
MessageId=0x9
Severity=Success
SymbolicName=MSG_SR9
Language=English
%1
.


MessageId=0x1
Severity=Warning
SymbolicName=MSG_WR1
Language=English
%1
.

MessageId=0x2
Severity=Warning
SymbolicName=MSG_WR2
Language=English
%1
.
MessageId=0x3
Severity=Warning
SymbolicName=MSG_WR3
Language=English
%1
.
MessageId=0x4
Severity=Warning
SymbolicName=MSG_WR4
Language=English
%1
.
MessageId=0x5
Severity=Warning
SymbolicName=MSG_WR5
Language=English
%1
.
MessageId=0x6
Severity=Warning
SymbolicName=MSG_WR6
Language=English
%1
.
MessageId=0x7
Severity=Warning
SymbolicName=MSG_WR7
Language=English
%1
.
MessageId=0x8
Severity=Warning
SymbolicName=MSG_WR8
Language=English
%1
.
MessageId=0x9
Severity=Warning
SymbolicName=MSG_WR9
Language=English
%1
.

