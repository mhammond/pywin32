# Detailed Design: Adding Long Path Support to PythonService.exe

## 1. Overview

This document outlines the implementation details for adding long path support to the `pythonservice.exe` component of the pywin32 package. Currently, Python scripts running as Windows services through `pythonservice.exe` cannot access paths exceeding the traditional MAX_PATH limit (260 characters), even when long path support is enabled in the Windows registry. This implementation will address this limitation.

## 2. Problem Statement

When running Python scripts as Windows services using pywin32's `pythonservice.exe`, attempts to access long paths (>260 characters) result in a "[WinError 3] The system cannot find the path specified" error, even when:
- Long path support is enabled in the Windows registry
- The same Python code can access long paths when run directly (not as a service)

This occurs because `pythonservice.exe` is not currently built with the `longPathAware` manifest attribute that would allow it to opt into Windows' long path support.

## 3. Requirements

### 3.1 Functional Requirements
- `pythonservice.exe` must be able to access file paths longer than 260 characters when the Windows registry has long path support enabled
- No changes should be required to user code; this should be transparent to service implementations
- The solution must work on Windows 10 version 1607 and later

### 3.2 Non-Functional Requirements
- The implementation should not affect performance
- The solution should be maintainable and follow pywin32's existing build patterns
- The implementation should not break backward compatibility

## 4. Technical Solution

### 4.1 Application Manifest Approach

The solution involves adding an application manifest to `pythonservice.exe` that includes the `longPathAware` element. This will opt the executable into Windows' long path support when the registry setting is enabled.

#### 4.1.1 Manifest Content

Create a manifest file (`pythonservice.exe.manifest`) with the following content:

```xml
<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<assembly xmlns="urn:schemas-microsoft-com:asm.v1" manifestVersion="1.0" xmlns:asmv3="urn:schemas-microsoft-com:asm.v3">
  <assemblyIdentity
    type="win32"
    name="Python.PythonService"
    version="1.0.0.0"
    processorArchitecture="*"
  />
  <description>Python Service Host</description>
  <trustInfo xmlns="urn:schemas-microsoft-com:asm.v3">
    <security>
      <requestedPrivileges>
        <requestedExecutionLevel level="asInvoker" uiAccess="false"/>
      </requestedPrivileges>
    </security>
  </trustInfo>
  <compatibility xmlns="urn:schemas-microsoft-com:compatibility.v1">
    <application>
      <!-- Windows 10 and Windows 11 -->
      <supportedOS Id="{8e0f7a12-bfb3-4fe8-b9a5-48fd50a15a9a}"/>
      <!-- Windows 8.1 -->
      <supportedOS Id="{1f676c76-80e1-4239-95bb-83d0f6d0da78}"/>
      <!-- Windows 8 -->
      <supportedOS Id="{4a2f28e3-53b9-4441-ba9c-d69d4a4a6e38}"/>
      <!-- Windows 7 -->
      <supportedOS Id="{35138b9a-5d96-4fbd-8e2d-a2440225f93a}"/>
    </application>
  </compatibility>
  <asmv3:application>
    <asmv3:windowsSettings xmlns:ws2="http://schemas.microsoft.com/SMI/2016/WindowsSettings">
      <ws2:longPathAware>true</ws2:longPathAware>
    </asmv3:windowsSettings>
  </asmv3:application>
</assembly>
```

### 4.2 Build System Integration

#### 4.2.1 Embedding the Manifest

The manifest needs to be embedded into the `pythonservice.exe` binary during the build process. This can be done using the Microsoft Manifest Tool (mt.exe) which is part of the Windows SDK.

#### 4.2.2 Build Script Modifications

Modify the build process in `setup.py` to:
1. Generate the manifest file
2. Embed the manifest into `pythonservice.exe` after it's built

Add the following steps to the build process:

```python
# In setup.py, modify the build_extension method of the appropriate Extension class

def build_extension(self, ext):
    # Existing build code...
    
    # After pythonservice.exe is built, embed the manifest
    if ext.name == "servicemanager":
        # Path to the built pythonservice.exe
        exe_path = os.path.join(self.build_lib, "win32", "pythonservice.exe")
        
        # Path to the manifest file
        manifest_path = os.path.join(os.path.dirname(__file__), "win32", "src", "PythonService", "pythonservice.exe.manifest")
        
        # Embed the manifest using mt.exe
        mt_cmd = ["mt.exe", "-manifest", manifest_path, "-outputresource:%s;1" % exe_path]
        self.spawn(mt_cmd)
```

### 4.3 Alternative Approach: Direct Code Modification

If embedding a manifest proves challenging in the build process, an alternative approach is to modify the source code directly to opt into long path support:

```cpp
// In PythonService.cpp, add the following code near the top of the file

#ifndef PYSERVICE_BUILD_DLL
// This ensures the executable opts into long path support
extern "C" {
    // MSVC-specific pragma to add a manifest dependency
    #pragma comment(linker, "/manifestdependency:\"type='win32' name='Python.PythonService' version='1.0.0.0' processorArchitecture='*' publicKeyToken='0000000000000000' language='*'\"")
    
    // This enables long path support when available
    #if _WIN32_WINNT >= 0x0A00  // Windows 10 or later
    #pragma comment(linker, "/MANIFESTDEPENDENCY:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
    #endif
}
#endif
```

## 5. Testing Strategy

### 5.1 Test Cases

1. **Basic Functionality Test**:
   - Create a Windows service using `pythonservice.exe`
   - Verify it can access normal paths (<260 characters)

2. **Long Path Test with Registry Enabled**:
   - Enable long path support in the registry
   - Create a deeply nested directory structure exceeding 260 characters
   - Verify the service can access files in this structure

3. **Long Path Test with Registry Disabled**:
   - Disable long path support in the registry
   - Verify the service behaves as expected (fails with appropriate error)

4. **Edge Cases**:
   - Test with paths just under and just over the 260 character limit
   - Test with Unicode characters in paths
   - Test with network paths

### 5.2 Test Environment

- Windows 10 version 1607 or later
- Python 3.6+ (multiple versions)
- Registry setting for long paths both enabled and disabled

## 6. Implementation Plan

### 6.1 Phase 1: Development
1. Create the manifest file
2. Modify the build process to embed the manifest
3. Build a test version of pywin32 with the changes

### 6.2 Phase 2: Testing
1. Execute the test cases defined above
2. Verify backward compatibility
3. Test on different Windows versions

### 6.3 Phase 3: Documentation and Release
1. Update documentation to mention long path support
2. Add a note to CHANGES.txt
3. Submit pull request

## 7. Considerations and Limitations

### 7.1 Registry Requirement
- Users must still enable long path support in the Windows registry
- Consider adding documentation about this requirement

### 7.2 Windows Version Compatibility
- Long path support is only available on Windows 10 version 1607 and later
- Older Windows versions will not benefit from this change

### 7.3 File System Compatibility
- Some file systems may have their own path length limitations
- Network paths may have additional constraints

## 8. References

1. [Enabling Long Path Support in Windows 10](https://docs.microsoft.com/en-us/windows/win32/fileio/maximum-file-path-limitation)
2. [Application Manifest Documentation](https://docs.microsoft.com/en-us/windows/win32/sbscs/application-manifests)
3. [Mt.exe (Manifest Tool) Documentation](https://docs.microsoft.com/en-us/windows/win32/sbscs/mt-exe)
4. [PythonService Implementation Guide](https://github.com/mhammond/pywin32/blob/main/win32/src/PythonService/PythonService.cpp)
