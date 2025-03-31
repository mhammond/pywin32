# PythonService Implementation Guide

## Overview

The PythonService component is a key part of the pywin32 package that enables Python code to run as Windows services. It consists of two main components:

1. **servicemanager.pyd** - A Python extension module containing all the core functionality
2. **PythonService.exe** - A simple executable that loads servicemanager.pyd and calls its public functions

This document explains how the PythonService implementation works and how developers can extend it.

## Architecture

### Dual-Purpose Source File

The `PythonService.cpp` file is used to compile two separate targets:

- **servicemanager.pyd**: The Python extension module containing all service functionality
- **PythonService.exe**: A thin wrapper that loads servicemanager.pyd and calls its functions

The code uses conditional compilation with the `PYSERVICE_BUILD_DLL` macro to determine which sections are included in each target.

### Core Components

#### Public API Functions

These functions are exported from servicemanager.pyd and used by PythonService.exe:

```c
BOOL PythonService_Initialize(const TCHAR *evtsrc_name, const TCHAR *evtsrc_file);
void PythonService_Finalize();
BOOL PythonService_PrepareToHostSingle(PyObject *);
BOOL PythonService_PrepareToHostMultiple(const TCHAR *service_name, PyObject *klass);
BOOL PythonService_StartServiceCtrlDispatcher();
int PythonService_main(int argc, TCHAR **argv);
```

#### Service Control Manager Integration

The implementation includes:

1. **Service Main Function**: `service_main()` - Entry point called by the Service Control Manager (SCM)
2. **Service Control Handlers**: `service_ctrl()` and `service_ctrl_ex()` - Handle service control requests
3. **Dispatch Table**: Maps service names to their respective service_main functions

#### Python Integration

The code provides:

1. **Python Class Loading**: `LoadPythonServiceClass()` - Loads a Python class from a module
2. **Instance Creation**: `LoadPythonServiceInstance()` - Creates an instance of the service class
3. **Python Callback Handling**: Dispatches SCM events to Python handler methods

## Service Lifecycle

### Initialization

1. The service process starts with `PythonService_main()`
2. `PythonService_Initialize()` is called to set up event logging
3. `PythonService_PrepareToHostSingle()` or `PythonService_PrepareToHostMultiple()` is called to register services
4. `PythonService_StartServiceCtrlDispatcher()` is called to connect to the SCM

### Service Startup

1. SCM calls `service_main()` with service arguments
2. Python is initialized with `PyService_InitPython()`
3. The Python service class is loaded from the registry or provided class object
4. An instance of the service class is created
5. The service registers a control handler via `RegisterServiceCtrlHandler()`
6. The service's `SvcRun()` method is called to start the service

### Service Control

1. SCM sends control requests to `service_ctrl()` or `service_ctrl_ex()`
2. These are forwarded to the Python control handler via `dispatchServiceCtrl()`
3. The Python handler returns a status code that is passed back to the SCM

### Service Shutdown

1. When `SvcRun()` returns, the service is considered stopped
2. Final status is reported to the SCM
3. Resources are cleaned up

## Extending PythonService

### Creating a New Service

To create a new Windows service using PythonService:

1. Create a Python class that implements:
   - `__init__(self, args)` - Constructor that receives service arguments
   - `SvcRun(self)` - Main service function that runs until service stops
   - `SvcStop(self)` - Method to handle stop requests (called by control handler)

2. Register the service with Windows, specifying:
   - The path to PythonService.exe as the service binary
   - The Python class to load (in registry or as a command-line argument)

### Adding Custom Service Functionality

To add custom functionality to a service:

1. Implement additional methods in your service class to handle specific tasks
2. For handling service controls, register a control handler that processes:
   - Standard controls (stop, pause, continue)
   - Custom controls (user-defined values)
   - Device events, power events, and session changes (with extended handler)

### Multiple Services in One Process

PythonService supports hosting multiple services in a single process:

1. Use `PythonService_PrepareToHostMultiple()` for each service
2. Each service needs its own Python class
3. The service process flags must be set to `SERVICE_WIN32_SHARE_PROCESS`

## Implementation Details

### Service Table Structure

The service uses two parallel arrays:

1. `SERVICE_TABLE_ENTRY DispatchTable[]` - Standard Windows service table
2. `PY_SERVICE_TABLE_ENTRY PythonServiceTable[]` - Corresponding Python service information

```c
typedef struct {
    PyObject *klass;                        // The Python class we instantiate as the service
    SERVICE_STATUS_HANDLE sshStatusHandle;  // The handle for this service
    PyObject *obServiceCtrlHandler;         // The Python control handler for the service
    BOOL bUseEx;                            // Does this handler expect the extra args?
} PY_SERVICE_TABLE_ENTRY;
```

### Service Status Reporting

The service maintains several pre-defined status structures:

- `startingStatus` - Used when service is starting
- `stoppedStatus` - Used when service stops normally
- `stoppedErrorStatus` - Used when service stops with an error
- `errorStatus` - Used when service encounters an error

### Event Logging

The service includes comprehensive error reporting:

1. `ReportError()` - Logs messages to the Windows Event Log
2. `ReportAPIError()` - Formats and logs Windows API errors
3. `ReportPythonError()` - Formats and logs Python exceptions

### Debug Mode

The service can run in debug mode:

1. Started with `-debug servicename` command-line argument
2. Console output instead of event log messages
3. Ctrl+C simulates service stop requests

## Python Module Interface

The `servicemanager` Python module provides these key functions:

- `RegisterServiceCtrlHandler(name, callback)` - Registers a Python function as the service control handler
- `LogMsg(type, eventId, strings)` - Logs a message to the event log
- `LogInfoMsg(msg)`, `LogErrorMsg(msg)`, `LogWarningMsg(msg)` - Convenience logging functions
- `RunningAsService()` - Returns True if running as a service
- `Debugging()` - Returns True if in debug mode
- `StartServiceCtrlDispatcher()` - Connects to the SCM

## Example: Adding a New Service Feature

To add support for a new Windows service feature (e.g., a new control code):

1. Add handling in `dispatchServiceCtrl()` for the new control code
2. Expose any necessary constants in the Python module
3. Update the Python control handler to process the new code
4. Add any required status reporting

For example, to add support for a custom control code:

```c
// In dispatchServiceCtrl():
case MY_CUSTOM_CONTROL:
    // Process custom control data
    sub = Py_BuildValue("(i)", customData);
    args = Py_BuildValue("(llN)", dwCtrlCode, dwEventType, sub);
    // Call Python handler...
    break;

// In PYWIN_MODULE_INIT_FUNC:
ADD_CONSTANT(MY_CUSTOM_CONTROL);
```

## Debugging Tips

1. Use the `-debug` flag to run the service in console mode
2. Check the Windows Event Log for service errors
3. Set `bServiceDebug = TRUE` to enable console output
4. Use `LogInfoMsg()` to add trace messages during development

## Common Issues and Solutions

1. **Service fails to start**: Check Python path and module imports
2. **Control handler not called**: Ensure RegisterServiceCtrlHandler was called successfully
3. **Python exceptions**: Look for traceback in event log
4. **DLL loading issues**: Verify all dependencies are in the system path
