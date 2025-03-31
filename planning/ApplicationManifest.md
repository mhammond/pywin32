# Application Manifests

## Overview

An application manifest (also known as a side-by-side application manifest, or a fusion manifest) is an XML file that describes and identifies the shared and private side-by-side assemblies that an application should bind to at run time. These should be the same assembly versions that were used to test the application. Application manifests might also describe metadata for files that are private to the application.

For a complete listing of the XML schema, see Manifest file schema.

## File Location

If possible, you should embed the application manifest as a resource in your application's .exe file or .dll. If you can't do that, then you can place the application manifest file in the same directory as the .exe or .dll.

For more info, see Installing side-by-side assemblies.

## File Name

By convention an application manifest should have the same name as your app's executable file, with the .manifest extension appended to it.

For example, an application manifest that refers to example.exe or example.dll should use the following file name syntax (if resource ID is 1, then you can omit the <resource ID> segment of the syntax).

```
example.exe.<resource ID>.manifest
example.dll.<resource ID>.manifest
```

## Elements and Attributes

Application manifests have the following elements and attributes. Names of elements and attributes are case-sensitive. The values of elements and attributes are case-insensitive, except for the value of the type attribute.

| Element | Attributes | Required |
|---------|------------|----------|
| assembly | | Yes |
| | manifestVersion | Yes |
| noInherit | | No |
| assemblyIdentity | | Yes |
| | type | Yes |
| | name | Yes |
| | language | No |
| | processorArchitecture | No |
| | version | Yes |
| | publicKeyToken | No |
| compatibility | | No |
| application | | No |
| supportedOS | | No |
| | Id | Yes |
| maxversiontested | | No |
| | Id | Yes |
| dependency | | No |
| dependentAssembly | | No |
| file | | No |
| | name | Yes |
| | hashalg | No |
| | hash | No |
| activatableClass | | No |
| | name | Yes |
| | threadingModel | Yes |
| activeCodePage | | No |
| autoElevate | | No |
| disableTheming | | No |
| disableWindowFiltering | | No |
| dpiAware | | No |
| dpiAwareness | | No |
| gdiScaling | | No |
| highResolutionScrollingAware | | No |
| longPathAware | | No |
| printerDriverIsolation | | No |
| ultraHighResolutionScrollingAware | | No |
| msix | | No |
| heapType | | No |
| supportedArchitectures | | No |
| trustInfo | | No |

### Key Elements

#### assembly

A container element. Its first subelement must be a noInherit or assemblyIdentity element. Required.

The assembly element must be in the namespace urn:schemas-microsoft-com:asm.v1. Child elements of the assembly must also be in this namespace, by inheritance or by tagging.

The assembly element has the following attributes:

| Attribute | Description |
|-----------|-------------|
| manifestVersion | The manifestVersion attribute must be set to 1.0. |

#### assemblyIdentity

As the first subelement of an assembly element, assemblyIdentity describes and uniquely identifies the application owning this application manifest. As the first subelement of a dependentAssembly element, assemblyIdentity describes a side-by-side assembly required by the application.

The assemblyIdentity element has the following attributes:

| Attribute | Description |
|-----------|-------------|
| type | Specifies the application or assembly type. The value must be win32 and all in lower case. Required. |
| name | Uniquely names the application or assembly. Use the format: Organization.Division.Name. Required. |
| language | Identifies the language of the application or assembly. Optional. |
| processorArchitecture | Specifies the processor. Valid values include x86, amd64, arm and arm64. Optional. |
| version | Specifies the application or assembly version. Use the four-part format: mmmmm.nnnnn.ooooo.ppppp. Required. |
| publicKeyToken | A 16-character hexadecimal string representing the last 8 bytes of the SHA-1 hash of the public key. Required for shared side-by-side assemblies. |

#### compatibility

Contains at least one application element. It has no attributes. Optional.

#### longPathAware

Enables long paths that exceed MAX_PATH in length. This element is supported in Windows 10, version 1607, and later.

```xml
<assembly xmlns="urn:schemas-microsoft-com:asm.v1" manifestVersion="1.0" xmlns:asmv3="urn:schemas-microsoft-com:asm.v3">
  <asmv3:application>
    <asmv3:windowsSettings xmlns:ws2="http://schemas.microsoft.com/SMI/2016/WindowsSettings">
      <ws2:longPathAware>true</ws2:longPathAware>
    </asmv3:windowsSettings>
  </asmv3:application>
</assembly>
```

#### trustInfo

All UAC-compliant apps should have a requested execution level added to the application manifest. Requested execution levels specify the privileges required for an app.

```xml
<trustInfo xmlns="urn:schemas-microsoft-com:asm.v2">
  <security>
    <requestedPrivileges xmlns="urn:schemas-microsoft-com:asm.v3">
      <requestedExecutionLevel level="asInvoker" uiAccess="false" />
    </requestedPrivileges>
  </security>
</trustInfo>
```

## Example

The following is an example of an application manifest for an application named MySampleApp.exe:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<assembly xmlns="urn:schemas-microsoft-com:asm.v1" manifestVersion="1.0">
   <assemblyIdentity type="win32" name="MyOrganization.MyDivision.MySampleApp" version="6.0.0.0" processorArchitecture="*" />
   <dependency>
      <dependentAssembly>
         <assemblyIdentity type="win32" name="Proseware.Research.SampleAssembly" version="6.0.0.0" processorArchitecture="*" publicKeyToken="0000000000000000" />
      </dependentAssembly>
   </dependency>
   <compatibility xmlns="urn:schemas-microsoft-com:compatibility.v1">
      <application>
         <!-- Windows 10 and Windows 11 -->
         <supportedOS Id="{8e0f7a12-bfb3-4fe8-b9a5-48fd50a15a9a}" />
         <!-- Windows 8.1 -->
         <supportedOS Id="{1f676c76-80e1-4239-95bb-83d0f6d0da78}" />
         <!-- Windows 8 -->
         <supportedOS Id="{4a2f28e3-53b9-4441-ba9c-d69d4a4a6e38}" />
         <!-- Windows 7 -->
         <supportedOS Id="{35138b9a-5d96-4fbd-8e2d-a2440225f93a}" />
         <!-- Windows Vista -->
         <supportedOS Id="{e2011457-1546-43c5-a5fe-008deee3d3f0}" />
      </application>
   </compatibility>
</assembly>
```

## Using Application Manifests with pywin32

For pywin32 applications, application manifests can be particularly useful for:

1. **Enabling long path support** - Using the longPathAware element to handle paths longer than MAX_PATH
2. **Declaring OS compatibility** - Using the compatibility element to specify which Windows versions are supported
3. **Setting UAC requirements** - Using the trustInfo element to specify required privileges
4. **Setting DPI awareness** - Using the dpiAware or dpiAwareness elements to handle high-DPI displays

To add an application manifest to a pywin32 application:

1. Create the manifest XML file
2. Either:
   - Place it alongside your .exe with the appropriate naming convention
   - Embed it as a resource using the Mt.exe tool (see the mt-exe.md document)

For embedding a manifest as a resource, you would typically use a command like:

```
mt.exe -manifest myapp.manifest -outputresource:myapp.exe;1
```
