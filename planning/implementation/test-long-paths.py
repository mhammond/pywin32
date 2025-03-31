"""
Test script for long path support in pythonservice.exe

This script can be used to test if long path support is working correctly.
It creates a deeply nested directory structure and attempts to access files within it.

Requirements:
- Windows 10 version 1607 or later
- Long path support enabled in registry (HKLM\SYSTEM\CurrentControlSet\Control\FileSystem\LongPathsEnabled = 1)
- pywin32 installed with the modified pythonservice.exe

Usage:
1. Run this script directly to test Python's native long path support
2. Install this script as a Windows service to test pythonservice.exe long path support
"""

import os
import sys
import time
import servicemanager
import win32service
import win32serviceutil
import win32event
import win32api

# Configuration
BASE_DIR = "C:\\LongPathTest"
NESTING_DEPTH = 50  # Creates a path longer than 260 characters
TEST_FILE_NAME = "test_file.txt"
TEST_FILE_CONTENT = "This is a test file for long path support."

def create_long_path():
    """Create a deeply nested directory structure with a test file"""
    path = BASE_DIR
    
    # Create base directory if it doesn't exist
    if not os.path.exists(path):
        os.makedirs(path)
    
    # Create nested directories
    for i in range(NESTING_DEPTH):
        path = os.path.join(path, f"level_{i:03d}")
        if not os.path.exists(path):
            os.makedirs(path)
    
    # Create a test file in the deepest directory
    test_file_path = os.path.join(path, TEST_FILE_NAME)
    with open(test_file_path, 'w') as f:
        f.write(TEST_FILE_CONTENT)
    
    return test_file_path

def test_long_path_access(test_file_path):
    """Test if we can access the file at the long path"""
    try:
        # Check if file exists
        if not os.path.exists(test_file_path):
            return False, f"File does not exist: {test_file_path}"
        
        # Read file content
        with open(test_file_path, 'r') as f:
            content = f.read()
        
        # Verify content
        if content == TEST_FILE_CONTENT:
            return True, f"Successfully read file at path ({len(test_file_path)} chars): {test_file_path}"
        else:
            return False, f"File content mismatch: {content}"
    
    except Exception as e:
        return False, f"Error accessing long path: {str(e)}"

class LongPathTestService(win32serviceutil.ServiceFramework):
    _svc_name_ = "LongPathTest"
    _svc_display_name_ = "Long Path Test Service"
    _svc_description_ = "Tests long path support in pythonservice.exe"
    
    def __init__(self, args):
        win32serviceutil.ServiceFramework.__init__(self, args)
        self.stop_event = win32event.CreateEvent(None, 0, 0, None)
        self.running = False
    
    def SvcStop(self):
        self.ReportServiceStatus(win32service.SERVICE_STOP_PENDING)
        win32event.SetEvent(self.stop_event)
        self.running = False
    
    def SvcDoRun(self):
        servicemanager.LogMsg(
            servicemanager.EVENTLOG_INFORMATION_TYPE,
            servicemanager.PYS_SERVICE_STARTED,
            (self._svc_name_, '')
        )
        self.running = True
        self.main()
    
    def main(self):
        try:
            # Log service start
            servicemanager.LogInfoMsg("Long Path Test Service starting")
            
            # Create long path and test file
            servicemanager.LogInfoMsg("Creating long path structure...")
            test_file_path = create_long_path()
            servicemanager.LogInfoMsg(f"Created test file at: {test_file_path}")
            servicemanager.LogInfoMsg(f"Path length: {len(test_file_path)} characters")
            
            # Test access to the long path
            servicemanager.LogInfoMsg("Testing access to long path...")
            success, message = test_long_path_access(test_file_path)
            
            if success:
                servicemanager.LogInfoMsg(f"SUCCESS: {message}")
            else:
                servicemanager.LogErrorMsg(f"FAILURE: {message}")
            
            # Keep service running until stopped
            while self.running:
                # Wait for service stop signal
                rc = win32event.WaitForSingleObject(self.stop_event, 5000)
                if rc == win32event.WAIT_OBJECT_0:
                    break
            
            servicemanager.LogInfoMsg("Long Path Test Service stopping")
        
        except Exception as e:
            servicemanager.LogErrorMsg(f"Error in service: {str(e)}")

def run_test_directly():
    """Run the test directly (not as a service)"""
    print("Running long path test directly (not as a service)")
    print("Creating long path structure...")
    
    test_file_path = create_long_path()
    print(f"Created test file at: {test_file_path}")
    print(f"Path length: {len(test_file_path)} characters")
    
    print("Testing access to long path...")
    success, message = test_long_path_access(test_file_path)
    
    if success:
        print(f"SUCCESS: {message}")
    else:
        print(f"FAILURE: {message}")

if __name__ == '__main__':
    if len(sys.argv) == 1:
        # Run test directly
        run_test_directly()
    else:
        # Run as a service
        win32serviceutil.HandleCommandLine(LongPathTestService)
