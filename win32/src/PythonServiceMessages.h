 /*
-------------------------------------------------------------------------
 MESSAGE DEFINITION SECTION

 Following the header section is the body of the Message Compiler
 source file. The body consists of zero or more message definitions.
 Each message definition begins with one or more of the following
 statements:

 MessageId = [number|+number]
 Severity = severity_name
 Facility = facility_name
 SymbolicName = name

 The MessageId statement marks the beginning of the message
 definition. A MessageID statement is required for each message,
 although the value is optional. If no value is specified, the value
 used is the previous value for the facility plus one. If the value
 is specified as +number then the value used is the previous value
 for the facility, plus the number after the plus sign. Otherwise, if
 a numeric value is given, that value is used. Any MessageId value
 that does not fit in 16 bits is an error.

 The Severity and Facility statements are optional. These statements
 specify additional bits to OR into the final 32-bit message code. If
 not specified they default to the value last specified for a message
 definition. The initial values prior to processing the first message
 definition are:

 Severity=Success
 Facility=Application

 The value associated with Severity and Facility must match one of
 the names given in the FacilityNames and SeverityNames statements in
 the header section. The SymbolicName statement allows you to
 associate a C/C++ symbolic constant with the final 32-bit message
 code.
 */
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
// MessageId: PYS_E_GENERIC_ERROR
//
// MessageText:
//
//  %1
//
#define PYS_E_GENERIC_ERROR              ((DWORD)0xC00000FFL)

//
// MessageId: PYS_SERVICE_STARTING
//
// MessageText:
//
//  The %1 service is starting%2.
//
#define PYS_SERVICE_STARTING             ((DWORD)0x40001000L)

//
// MessageId: PYS_SERVICE_STARTED
//
// MessageText:
//
//  The %1 service has started%2.
//
#define PYS_SERVICE_STARTED              ((DWORD)0x40001002L)

//
// MessageId: PYS_SERVICE_STOPPING
//
// MessageText:
//
//  The %1 service is stopping%2.
//
#define PYS_SERVICE_STOPPING             ((DWORD)0x40001003L)

//
// MessageId: PYS_SERVICE_STOPPED
//
// MessageText:
//
//  The %1 service has stopped%2.
//
#define PYS_SERVICE_STOPPED              ((DWORD)0x40001004L)

//
// MessageId: PYS_E_GENERIC_WARNING
//
// MessageText:
//
//  %1
//
#define PYS_E_GENERIC_WARNING            ((DWORD)0x400000FFL)

//
// MessageId: E_PYS_NOT_CONTROL_HANDLER
//
// MessageText:
//
//  The Python class did not register a service control handler.
//  %n%1
//  %n%2: %3
//
#define E_PYS_NOT_CONTROL_HANDLER        ((DWORD)0xC0000001L)

//
// MessageId: E_PYS_NO_RUN_METHOD
//
// MessageText:
//
//  The instance does not have a SvcRun() method.
//  %n%1
//  %n%2: %3
//
#define E_PYS_NO_RUN_METHOD              ((DWORD)0xC0000002L)

//
// MessageId: E_PYS_START_FAILED
//
// MessageText:
//
//  The instance's SvcRun() method failed
//  %n%1
//  %n%2: %3
//
#define E_PYS_START_FAILED               ((DWORD)0xC0000003L)

//
// MessageId: E_PYS_NO_MODULE
//
// MessageText:
//
//  Python could not import the service's module
//  %n%1
//  %n%2: %3
//
#define E_PYS_NO_MODULE                  ((DWORD)0xC0000004L)

//
// MessageId: E_PYS_NO_CLASS
//
// MessageText:
//
//  Python could find the service class in the module
//  %n%1
//  %n%2: %3
//
#define E_PYS_NO_CLASS                   ((DWORD)0xC0000005L)

//
// MessageId: E_UNUSED
//
// MessageText:
//
//  E_UNUSED
//
#define E_UNUSED                         ((DWORD)0xC0000006L)

//
// MessageId: E_UNUSED2
//
// MessageText:
//
//  E_UNUSED2
//
#define E_UNUSED2                        ((DWORD)0xC0000007L)

//
// MessageId: PYS_E_NO_MEMORY_FOR_ARGS
//
// MessageText:
//
//  Python could not allocate memory for the argument tuple.
//  %n%1
//  %n%2: %3
//
#define PYS_E_NO_MEMORY_FOR_ARGS         ((DWORD)0xC0000008L)

//
// MessageId: PYS_E_BAD_ARGS
//
// MessageText:
//
//  Python could not convert the service arguments
//  %n%1
//  %n%2: %3
//
#define PYS_E_BAD_ARGS                   ((DWORD)0xC0000009L)

//
// MessageId: PYS_E_BAD_CLASS
//
// MessageText:
//
//  Python could not construct the class instance
//  %n%1
//  %n%2: %3
//
#define PYS_E_BAD_CLASS                  ((DWORD)0xC000000AL)

//
// MessageId: PYS_E_SERVICE_CONTROL_FAILED
//
// MessageText:
//
//  The Python service control failed.
//  %n%1
//  %n%2: %3
//
#define PYS_E_SERVICE_CONTROL_FAILED     ((DWORD)0xC000000BL)

//
// MessageId: PYS_E_NO_MEMORY_FOR_SYS_PATH
//
// MessageText:
//
//  Python could not create a string with the modules path
//  %n%1
//  %n%2: %3
//
#define PYS_E_NO_MEMORY_FOR_SYS_PATH     ((DWORD)0xC000000CL)

//
// MessageId: PYS_E_NO_SYS_PATH
//
// MessageText:
//
//  Could not get the sys.path from Python.
//  %n%1
//  %n%2: %3
//
#define PYS_E_NO_SYS_PATH                ((DWORD)0xC000000DL)

//
// MessageId: PYS_E_CANT_LOCATE_MODULE_NAME
//
// MessageText:
//
//  Could not locate the module name in the Python class string (ie, no '.')
//
#define PYS_E_CANT_LOCATE_MODULE_NAME    ((DWORD)0xC0000080L)

//
// MessageId: PYS_E_API_CANT_START_SERVICE
//
// MessageText:
//
//  StartServiceCtrlDispatcher could not start the service.
//  Error %1 - %2
//
#define PYS_E_API_CANT_START_SERVICE     ((DWORD)0xC00000F0L)

//
// MessageId: PYS_E_API_CANT_SET_PENDING
//
// MessageText:
//
//  SetServiceStatus failed setting START_PENDING status
//  Error %1 - %2
//
#define PYS_E_API_CANT_SET_PENDING       ((DWORD)0xC00000F1L)

//
// MessageId: PYS_E_API_CANT_SET_STOPPED
//
// MessageText:
//
//  SetServiceStatus failed setting STOPPED status
//  Error %1 - %2
//
#define PYS_E_API_CANT_SET_STOPPED       ((DWORD)0xC00000F2L)

//
// MessageId: PYS_E_API_CANT_CONVERT_ASCII
//
// MessageText:
//
//  WideCharToMultiByte could not convert service name to ASCII
//  Error %1 - %2
//
#define PYS_E_API_CANT_CONVERT_ASCII     ((DWORD)0xC00000F3L)

//
// MessageId: PYS_E_API_CANT_LOCATE_PYTHON_CLASS
//
// MessageText:
//
//  Could not find the service's PythonClass entry in the registry
//  Error %1 - %2
//
#define PYS_E_API_CANT_LOCATE_PYTHON_CLASS ((DWORD)0xC00000F4L)

//
// MessageId: MSG_ER1
//
// MessageText:
//
//  %1
//
#define MSG_ER1                          ((DWORD)0xC000F000L)

//
// MessageId: MSG_ER2
//
// MessageText:
//
//  %1
//
#define MSG_ER2                          ((DWORD)0xC000F001L)

//
// MessageId: MSG_ER3
//
// MessageText:
//
//  %1
//
#define MSG_ER3                          ((DWORD)0xC000F002L)

//
// MessageId: MSG_ER4
//
// MessageText:
//
//  %1
//
#define MSG_ER4                          ((DWORD)0xC000F003L)

//
// MessageId: MSG_ER5
//
// MessageText:
//
//  %1
//
#define MSG_ER5                          ((DWORD)0xC000F004L)

//
// MessageId: MSG_ER6
//
// MessageText:
//
//  %1
//
#define MSG_ER6                          ((DWORD)0xC000F005L)

//
// MessageId: MSG_ER7
//
// MessageText:
//
//  %1
//
#define MSG_ER7                          ((DWORD)0xC000F006L)

//
// MessageId: MSG_ER8
//
// MessageText:
//
//  %1
//
#define MSG_ER8                          ((DWORD)0xC000F007L)

//
// MessageId: MSG_ER9
//
// MessageText:
//
//  %1
//
#define MSG_ER9                          ((DWORD)0xC000F008L)

 // Informational messages
//
// MessageId: MSG_IR1
//
// MessageText:
//
//  %1
//
#define MSG_IR1                          ((DWORD)0x4000F000L)

//
// MessageId: MSG_IR2
//
// MessageText:
//
//  %1
//
#define MSG_IR2                          ((DWORD)0x4000F001L)

//
// MessageId: MSG_IR3
//
// MessageText:
//
//  %1
//
#define MSG_IR3                          ((DWORD)0x4000F002L)

//
// MessageId: MSG_IR4
//
// MessageText:
//
//  %1
//
#define MSG_IR4                          ((DWORD)0x4000F003L)

//
// MessageId: MSG_IR5
//
// MessageText:
//
//  %1
//
#define MSG_IR5                          ((DWORD)0x4000F004L)

//
// MessageId: MSG_IR6
//
// MessageText:
//
//  %1
//
#define MSG_IR6                          ((DWORD)0x4000F005L)

//
// MessageId: MSG_IR7
//
// MessageText:
//
//  %1
//
#define MSG_IR7                          ((DWORD)0x4000F006L)

//
// MessageId: MSG_IR8
//
// MessageText:
//
//  %1
//
#define MSG_IR8                          ((DWORD)0x4000F007L)

//
// MessageId: MSG_IR9
//
// MessageText:
//
//  %1
//
#define MSG_IR9                          ((DWORD)0x4000F008L)

//
// MessageId: MSG_SR1
//
// MessageText:
//
//  %1
//
#define MSG_SR1                          ((DWORD)0x0000F000L)

//
// MessageId: MSG_SR2
//
// MessageText:
//
//  %1
//
#define MSG_SR2                          ((DWORD)0x0000F001L)

//
// MessageId: MSG_SR3
//
// MessageText:
//
//  %1
//
#define MSG_SR3                          ((DWORD)0x0000F002L)

//
// MessageId: MSG_SR4
//
// MessageText:
//
//  %1
//
#define MSG_SR4                          ((DWORD)0x0000F003L)

//
// MessageId: MSG_SR5
//
// MessageText:
//
//  %1
//
#define MSG_SR5                          ((DWORD)0x0000F004L)

//
// MessageId: MSG_SR6
//
// MessageText:
//
//  %1
//
#define MSG_SR6                          ((DWORD)0x0000F005L)

//
// MessageId: MSG_SR7
//
// MessageText:
//
//  %1
//
#define MSG_SR7                          ((DWORD)0x0000F006L)

//
// MessageId: MSG_SR8
//
// MessageText:
//
//  %1
//
#define MSG_SR8                          ((DWORD)0x0000F007L)

//
// MessageId: MSG_SR9
//
// MessageText:
//
//  %1
//
#define MSG_SR9                          ((DWORD)0x0000F008L)

//
// MessageId: MSG_WR1
//
// MessageText:
//
//  %1
//
#define MSG_WR1                          ((DWORD)0x8000F000L)

//
// MessageId: MSG_WR2
//
// MessageText:
//
//  %1
//
#define MSG_WR2                          ((DWORD)0x8000F001L)

//
// MessageId: MSG_WR3
//
// MessageText:
//
//  %1
//
#define MSG_WR3                          ((DWORD)0x8000F002L)

//
// MessageId: MSG_WR4
//
// MessageText:
//
//  %1
//
#define MSG_WR4                          ((DWORD)0x8000F003L)

//
// MessageId: MSG_WR5
//
// MessageText:
//
//  %1
//
#define MSG_WR5                          ((DWORD)0x8000F004L)

//
// MessageId: MSG_WR6
//
// MessageText:
//
//  %1
//
#define MSG_WR6                          ((DWORD)0x8000F005L)

//
// MessageId: MSG_WR7
//
// MessageText:
//
//  %1
//
#define MSG_WR7                          ((DWORD)0x8000F006L)

//
// MessageId: MSG_WR8
//
// MessageText:
//
//  %1
//
#define MSG_WR8                          ((DWORD)0x8000F007L)

//
// MessageId: MSG_WR9
//
// MessageText:
//
//  %1
//
#define MSG_WR9                          ((DWORD)0x8000F008L)

