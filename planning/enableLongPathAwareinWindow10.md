# Enabling Long Path Support in Windows 10

## Overview

Starting in Windows 10, version 1607, MAX_PATH limitations have been removed from many common Win32 file and directory functions. However, your application must opt-in to the new behavior.

To enable the new long path behavior per application, two conditions must be met:
1. A registry value must be set
2. The application manifest must include the longPathAware element

## Registry Setting to Enable Long Paths

> **Important**: Understand that enabling this registry setting will only affect applications that have been modified to take advantage of the new feature. Developers must declare their apps to be long path aware, as outlined in the application manifest settings below. This isn't a change that will affect all applications.

The registry value `HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\FileSystem LongPathsEnabled` (Type: REG_DWORD) must exist and be set to 1. The registry value will be cached by the system (per process) after the first call to an affected Win32 file or directory function (see below for the list of functions). The registry value will not be reloaded during the lifetime of the process. In order for all apps on the system to recognize the value, a reboot might be required because some processes may have started before the key was set.

You can also copy this code to a .reg file which can set this for you, or use the PowerShell command from a terminal window with elevated privileges:

### Registry (.reg) file
```
Windows Registry Editor Version 5.00

[HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\FileSystem]
"LongPathsEnabled"=dword:00000001
```

> **Note**: This registry setting can also be controlled via Group Policy at Computer Configuration > Administrative Templates > System > Filesystem > Enable Win32 long paths. The policy can also be applied with Microsoft Intune using Policy Configuration Service Provider (CSP).

## Application Manifest Updates to Declare Long Path Capability

The application manifest must also include the longPathAware element:

```xml
<application xmlns="urn:schemas-microsoft-com:asm.v3">
    <windowsSettings xmlns:ws2="http://schemas.microsoft.com/SMI/2016/WindowsSettings">
        <ws2:longPathAware>true</ws2:longPathAware>
    </windowsSettings>
</application>
```

## Functions Without MAX_PATH Restrictions

### Directory Management Functions
These are the directory management functions that no longer have MAX_PATH restrictions if you opt-in to long path behavior:
- CreateDirectoryW
- CreateDirectoryExW
- GetCurrentDirectoryW
- RemoveDirectoryW
- SetCurrentDirectoryW

### File Management Functions
These are the file management functions that no longer have MAX_PATH restrictions if you opt-in to long path behavior:
- CopyFileW
- CopyFile2
- CopyFileExW
- CreateFileW
- CreateFile2
- CreateHardLinkW
- CreateSymbolicLinkW
- DeleteFileW
- FindFirstFileW
- FindFirstFileExW
- FindNextFileW
- GetFileAttributesW
- GetFileAttributesExW
- SetFileAttributesW
- GetFullPathNameW
- GetLongPathNameW
- MoveFileW
- MoveFileExW
- MoveFileWithProgressW
- ReplaceFileW
- SearchPathW
- FindFirstFileNameW
- FindNextFileNameW
- FindFirstStreamW
- FindNextStreamW
- GetCompressedFileSizeW
- GetFinalPathNameByHandleW

## Implementation in pywin32

For pywin32 to take advantage of long path support, the following steps would be necessary:

1. Update the application manifest to include the longPathAware element
2. Ensure that all file and directory operations use the Unicode versions of the Win32 API functions (the W suffix versions)
3. Modify any path length checks in the code to accommodate paths longer than MAX_PATH
4. Test thoroughly with long paths to ensure compatibility

### Benefits for pywin32 Users

Enabling long path support would allow pywin32 users to:
- Work with deeply nested directory structures
- Handle file paths longer than the traditional 260 character limit
- Improve compatibility with modern development workflows that often create deep directory structures (e.g., node_modules in JavaScript projects)
- Reduce errors related to path length limitations

### Potential Issues

- Backward compatibility concerns with code that assumes MAX_PATH limitations
- Possible issues with third-party libraries that don't support long paths
- Need for testing across different Windows versions
