; /*
MessageIdTypedef=DWORD
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

MessageId=0xFF
Severity=Error
SymbolicName=PYS_E_GENERIC_ERROR
Language=English
%1
.

MessageId=0x1000
Severity=Informational
SymbolicName=PYS_SERVICE_STARTING
Language=English
The %1 service is starting%2.
.

MessageId=0x1002
Severity=Informational
SymbolicName=PYS_SERVICE_STARTED
Language=English
The %1 service has started%2.
.

MessageId=0x1003
Severity=Informational
SymbolicName=PYS_SERVICE_STOPPING
Language=English
The %1 service is stopping%2.
.

MessageId=0x1004
Severity=Informational
SymbolicName=PYS_SERVICE_STOPPED
Language=English
The %1 service has stopped%2.
.

MessageId=0xFF
Severity=Informational
SymbolicName=PYS_E_GENERIC_WARNING
Language=English
%1
.


MessageId=0x1
Severity=Error
SymbolicName=E_PYS_NOT_CONTROL_HANDLER
Language=English
The Python class did not register a service control handler.
%n%1
%n%2: %3
.

MessageId=0x2
Severity=Error
SymbolicName=E_PYS_NO_RUN_METHOD
Language=English
The instance does not have a SvcRun() method.
%n%1
%n%2: %3
.
MessageId=0x3
Severity=Error
SymbolicName=E_PYS_START_FAILED
Language=English
The instance's SvcRun() method failed
%n%1
%n%2: %3
.
MessageId=0x4
Severity=Error
SymbolicName=E_PYS_NO_MODULE
Language=English
Python could not import the service's module
%n%1
%n%2: %3
.
MessageId=0x5
Severity=Error
SymbolicName=E_PYS_NO_CLASS
Language=English
Python could find the service class in the module
%n%1
%n%2: %3
.
MessageId=0x6
Severity=Error
SymbolicName=E_PYS_NO_SERVICE
Language=English
An attempt was made to start the service '%1', but this service
is not hosted in this process.
.

MessageId=0x7
Severity=Error
SymbolicName=E_UNUSED2
Language=English
.

MessageId=0x8
Severity=Error
SymbolicName=PYS_E_NO_MEMORY_FOR_ARGS
Language=English
Python could not allocate memory for the argument tuple.
%n%1
%n%2: %3
.

MessageId=0x9
Severity=Error
SymbolicName=PYS_E_BAD_ARGS
Language=English
Python could not convert the service arguments
%n%1
%n%2: %3
.

MessageId=0xA
Severity=Error
SymbolicName=PYS_E_BAD_CLASS
Language=English
Python could not construct the class instance
%n%1
%n%2: %3
.

MessageId=0xB
Severity=Error
SymbolicName=PYS_E_SERVICE_CONTROL_FAILED
Language=English
The Python service control handler failed.
%n%1
%n%2: %3
.

MessageId=0xC
Severity=Error
SymbolicName=PYS_E_NO_MEMORY_FOR_SYS_PATH
Language=English
Python could not create a string with the modules path
%n%1
%n%2: %3
.

MessageId=0xD
Severity=Error
SymbolicName=PYS_E_NO_SYS_PATH
Language=English
Could not get the sys.path from Python.
%n%1
%n%2: %3
.

MessageId=0xE
Severity=Error
SymbolicName=PYS_E_NO_SERVICEMANAGER
Language=English
Could not locate the Python servicemanager.
%n%1
%n%2: %3
.


MessageId=0x80
Severity=Error
SymbolicName=PYS_E_CANT_LOCATE_MODULE_NAME
Language=English
Could not locate the module name in the Python class string (ie, no '.')
.

MessageId=0xF0
Severity=Error
SymbolicName=PYS_E_API_CANT_START_SERVICE
Language=English
StartServiceCtrlDispatcher could not start the service.
Error %1 - %2
.

MessageId=0xF1
Severity=Error
SymbolicName=PYS_E_API_CANT_SET_PENDING
Language=English
SetServiceStatus failed setting START_PENDING status
Error %1 - %2
.

MessageId=0xF2
Severity=Error
SymbolicName=PYS_E_API_CANT_SET_STOPPED
Language=English
SetServiceStatus failed setting STOPPED status
Error %1 - %2
.

MessageId=0xF3
Severity=Error
SymbolicName=PYS_E_API_CANT_CONVERT_ASCII
Language=English
WideCharToMultiByte could not convert service name to ASCII
Error %1 - %2
.

MessageId=0xF4
Severity=Error
SymbolicName=PYS_E_API_CANT_LOCATE_PYTHON_CLASS
Language=English
Could not find the service's PythonClass entry in the registry
Error %1 - %2
.

MessageId=0xF000
Severity=Error
SymbolicName=MSG_ER1
Language=English
%1
.

MessageId=
Severity=Error
SymbolicName=MSG_ER2
Language=English
%1
.

MessageId=
Severity=Error
SymbolicName=MSG_ER3
Language=English
%1
.

MessageId=
Severity=Error
SymbolicName=MSG_ER4
Language=English
%1
.

MessageId=
Severity=Error
SymbolicName=MSG_ER5
Language=English
%1
.

MessageId=
Severity=Error
SymbolicName=MSG_ER6
Language=English
%1
.

MessageId=
Severity=Error
SymbolicName=MSG_ER7
Language=English
%1
.

MessageId=
Severity=Error
SymbolicName=MSG_ER8
Language=English
%1
.

MessageId=
Severity=Error
SymbolicName=MSG_ER9
Language=English
%1
.

; // Informational messages

MessageId=0xF000
Severity=Informational
SymbolicName=MSG_IR1
Language=English
%1
.
MessageId=
Severity=Informational
SymbolicName=MSG_IR2
Language=English
%1
.
MessageId=
Severity=Informational
SymbolicName=MSG_IR3
Language=English
%1
.
MessageId=
Severity=Informational
SymbolicName=MSG_IR4
Language=English
%1
.
MessageId=
Severity=Informational
SymbolicName=MSG_IR5
Language=English
%1
.
MessageId=
Severity=Informational
SymbolicName=MSG_IR6
Language=English
%1
.
MessageId=
Severity=Informational
SymbolicName=MSG_IR7
Language=English
%1
.
MessageId=
Severity=Informational
SymbolicName=MSG_IR8
Language=English
%1
.
MessageId=
Severity=Informational
SymbolicName=MSG_IR9
Language=English
%1
.


MessageId=0xF000
Severity=Success
SymbolicName=MSG_SR1
Language=English
%1
.

MessageId=
Severity=Success
SymbolicName=MSG_SR2
Language=English
%1
.
MessageId=
Severity=Success
SymbolicName=MSG_SR3
Language=English
%1
.
MessageId=
Severity=Success
SymbolicName=MSG_SR4
Language=English
%1
.
MessageId=
Severity=Success
SymbolicName=MSG_SR5
Language=English
%1
.
MessageId=
Severity=Success
SymbolicName=MSG_SR6
Language=English
%1
.
MessageId=
Severity=Success
SymbolicName=MSG_SR7
Language=English
%1
.
MessageId=
Severity=Success
SymbolicName=MSG_SR8
Language=English
%1
.
MessageId=
Severity=Success
SymbolicName=MSG_SR9
Language=English
%1
.


MessageId=0xF000
Severity=Warning
SymbolicName=MSG_WR1
Language=English
%1
.

MessageId=
Severity=Warning
SymbolicName=MSG_WR2
Language=English
%1
.
MessageId=
Severity=Warning
SymbolicName=MSG_WR3
Language=English
%1
.
MessageId=
Severity=Warning
SymbolicName=MSG_WR4
Language=English
%1
.
MessageId=
Severity=Warning
SymbolicName=MSG_WR5
Language=English
%1
.
MessageId=
Severity=Warning
SymbolicName=MSG_WR6
Language=English
%1
.
MessageId=
Severity=Warning
SymbolicName=MSG_WR7
Language=English
%1
.
MessageId=
Severity=Warning
SymbolicName=MSG_WR8
Language=English
%1
.
MessageId=
Severity=Warning
SymbolicName=MSG_WR9
Language=English
%1
.

