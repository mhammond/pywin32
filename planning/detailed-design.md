# Long Path Support for pythonservice.exe - Detailed Design Document

## Overview

This document provides a detailed design for implementing long path support in the pythonservice.exe component of pywin32. The implementation will enable Python services running through pythonservice.exe to access paths longer than the traditional MAX_PATH limit (260 characters) when the host Windows system has long path support enabled.

## Background

Starting in Windows 10 version 1607, Microsoft introduced the ability for applications to opt into long path support by:
1. Setting a registry value (`HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\FileSystem LongPathsEnabled` to 1)
2. Including a manifest in the application that declares it as "longPathAware"

Python itself is long path aware by default, but pythonservice.exe in pywin32 is not currently configured with the necessary manifest, causing services running through it to be unable to access long paths even when the registry setting is enabled.

## Requirements

1. Make pythonservice.exe long path aware by adding the appropriate application manifest
2. Implement the change at build time, not as a post-build modification
3. Ensure backward compatibility with existing services

## Design

### Application Manifest Implementation

The primary change will be to modify PythonService.cpp directly to include the longPathAware manifest element. This will be done using direct source code embedding with the `#pragma comment(linker)` directive.

The manifest must include the `longPathAware` element in the proper namespace:

```xml
<application xmlns="urn:schemas-microsoft-com:asm.v3">
    <windowsSettings xmlns:ws2="http://schemas.microsoft.com/SMI/2016/WindowsSettings">
        <ws2:longPathAware>true</ws2:longPathAware>
    </windowsSettings>
</application>
```

#### Implementation in PythonService.cpp

Add the following code to PythonService.cpp:

```cpp
// Only needed when building the EXE, not the DLL
#if !defined(PYSERVICE_BUILD_DLL) && defined(_MSC_VER)
#pragma comment(linker, "/MANIFESTUAC:\"level='asInvoker' uiAccess='false'\"")
#pragma comment(linker, "/MANIFEST:EMBED")
#pragma comment(linker, "/MANIFESTINPUT:\"<?xml version='1.0' encoding='UTF-8' standalone='yes'?><assembly xmlns='urn:schemas-microsoft-com:asm.v1' manifestVersion='1.0'><application xmlns='urn:schemas-microsoft-com:asm.v3'><windowsSettings xmlns:ws2='http://schemas.microsoft.com/SMI/2016/WindowsSettings'><ws2:longPathAware>true</ws2:longPathAware></windowsSettings></application></assembly>\"")
#endif
```

This code should be placed near the top of the file, after any existing includes but before any function definitions.

## Testing

### Test Plan

1. **Build Verification**:
   - Build pythonservice.exe with the changes
   - Verify the manifest is correctly embedded using the `mt.exe` tool:
     ```
     mt.exe -inputresource:pythonservice.exe;#1 -out:manifest.xml
     ```
   - Check that the manifest.xml file contains the longPathAware element

2. **Functionality Testing**:
   - Create a test service that attempts to access a path longer than 260 characters
   - Test on a system with the registry setting enabled
   - Test on a system with the registry setting disabled

3. **Test Script**:
   Use the provided test script to verify functionality:

```python
# service_test.py
import win32serviceutil
import win32service
import win32event
import servicemanager
import socket
import time
import os
import logging

logger = logging.getLogger()
service_dir = os.path.dirname(__file__)
logFile = "service-test-log.txt"
asbLogFile = os.path.join(service_dir, logFile)

logging.basicConfig(
    filename = asbLogFile,
    level = logging.DEBUG, 
    format = '%(asctime)s [%(levelname)-7.7s] %(message)s'
)

class TestingService(win32serviceutil.ServiceFramework):
    _future = None
    _svc_name_ = "TESTING_Service"
    _svc_display_name_ = "TESTING Service"
   
    def __init__(self,args):
        win32serviceutil.ServiceFramework.__init__(self,args)
        self.stop_event = win32event.CreateEvent(None,0,0,None)
        socket.setdefaulttimeout(60)
        self.stop_requested = False

    def SvcStop(self):
        self.ReportServiceStatus(win32service.SERVICE_STOP_PENDING)
        win32event.SetEvent(self.stop_event)
        logger.info('Stopping service ...')
        self.stop_requested = True

    def SvcDoRun(self):
        servicemanager.LogMsg(
            servicemanager.EVENTLOG_INFORMATION_TYPE,
            servicemanager.PYS_SERVICE_STARTED,
            (self._svc_name_,'')
        )
        self.main()

    def main(self):
        logging.info('Attempting to Access Long Path')
        service_dir = os.path.dirname(__file__)
        
        # Create a long path for testing
        longPathFile = "longpath\\files\\longpath_testoutput_4k_fibfibfibfibfibfibfibfibfibfibfibfibfibfibfibfibfibfibfibfibfibfibfibfibfibfibfibfibfibfibfibfibfibfibfibfibfibfibfibfibfibfibfibfibfibfibfibfifibfibbfibfiibfibfibfibfibfibfibfibfibfibfibfibfibfibfibfibfibfibfibbo.exr.f307214C.txt"
        absLongPathFile = os.path.join(service_dir, longPathFile)

        try:
            # Create directory structure if it doesn't exist
            os.makedirs(os.path.dirname(absLongPathFile), exist_ok=True)
            
            # Create test file if it doesn't exist
            if not os.path.exists(absLongPathFile):
                with open(absLongPathFile, "w") as f:
                    f.write("Test content for long path file")
                logging.info(f"Created test file at: {absLongPathFile}")
            
            # Try to read the file
            with open(absLongPathFile, "r") as f:
                content = f.read()
                logging.info(f"Successfully read file with content: {content}")
        except Exception as err:
            logging.exception(f"Error accessing long path: {err}")


if __name__ == '__main__':
    try:
        win32serviceutil.HandleCommandLine(TestingService)
    except Exception as err:
        logging.exception(err)
```

### Test Instructions

1. Install pywin32 to the global site-packages:
   ```
   pip install pywin32
   ```

2. Install the test service:
   ```
   python service_test.py install
   ```

3. Run the pywin32 post-install script:
   ```
   pywin32_postinstall.py -install
   ```

4. Start the service:
   ```
   net start TESTING_Service
   ```

5. Check the log file for results

6. Delete the service when done:
   ```
   sc delete TESTING_Service
   ```

## Documentation Updates

### CHANGES.txt Update

Add the following entry to CHANGES.txt for the next release:

```
Coming in build XXX:
- Added long path support to pythonservice.exe when running on Windows systems with long path support enabled
```

## Implementation Notes

1. **Compatibility**: This change should not affect existing services that don't use long paths.

2. **Registry Requirement**: Long path support will only be active when the Windows registry has the appropriate setting enabled (`LongPathsEnabled=1`).

3. **Build Process**: The change is implemented at build time through source code modification, not as a post-build step.

4. **Windows Version**: Long path support requires Windows 10 version 1607 or later.

## Files to Modify

1. **PythonService.cpp**: Add the manifest embedding code

## Implementation Timeline

This change should be included in the next regular release of pywin32.
