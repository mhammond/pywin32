# Mt.exe - Manifest Tool

## Overview

The Mt.exe file is a tool that generates signed files and catalogs. It is available in the Microsoft Windows Software Development Kit (SDK). Mt.exe requires that the file referenced in the manifest be present in the same directory as the manifest.

Mt.exe generates hashes using the CryptoAPI implementation of the Secure Hash Algorithm (SHA-1). For more information about hash algorithms, see Hash and Signature Algorithms. Hashes are inserted as a hexadecimal string into the file tags in the manifest. The tool currently only generates SHA-1 hashes, although files in manifests may use other hashing schemes.

Mt.exe uses Makecat.exe to generate catalog files (.cat) from catalog definition files (.cdf). This tool fills out a standard template CDF with the name and location of your manifest. You can use this with Makecat.exe to generate the assembly catalog.

The version of Mt.exe provided in recent versions of the Windows SDK can also be used to generate manifests for managed assemblies and unmanaged side-by-side assemblies.

## Syntax

```
mt.exe [-manifest:<component1.manifest><component2.manifest>] [-identity:<identity string>] 
[-rgs:<file1.rgs>] [-tlb:<file2.tlb>] [-dll:<file3.dll>] [-replacements:<XML filename>]
[-managedassemblyname:<managed assembly>] [-nodependency] [-category] [-out:<output manifest name>]
[-inputresource:<file4>;[#]<resource_id>] [-outputresource:<file5>;[#]<resource_id>] 
[-updateresource:<file6>;[#]<resource_id>] [-hashupdate[:<path to files>]] [-makecdfs] [-validate_manifest]
[-validate_file_hashes:<path to files>] [-canonicalize] [-check_for_duplicates] [-nologo] [-verbose]
```

## Command Line Options

Mt.exe uses the following case-insensitive command line options.

### -manifest

Specifies the name of the manifest file. To modify a single manifest, specify one manifest file name. For example, component.manifest.

To merge multiple manifests, specify the names of the source manifests here. Specify the name of the updated manifest with either the -out, -outputresource, or -updateresource options. For example, the following command line requests an operation that merges two manifests, man1.manifest and man2.manifest, into a new manifest, man3.manifest.

```
mt.exe -manifest man1.manifest man2.manifest -out:man3.manifest
```

No colon (:) is required with the -manifest option.

### -identity

Provides the attributes values of the assemblyIdentity element of the manifest. The argument of the -identity option is a string value containing the attribute values in fields separated by commas. Provide the value of the name attribute in the first field, without including a "name=" substring. All the remaining fields specify the attributes and their values using the form: <attribute name>=<attribute_value>.

For example, to update the assemblyIdentity element of the manifest with the following information:

```xml
<assemblyIdentity type="win32" name="Microsoft.Windows.SampleAssembly" version="6.0.0.0" processorArchitecture="x86" publicKeyToken="a5aaf5ba15723d5"/>
```

include the following -identity option on the command line:

```
-identity:"Microsoft.Windows.SampleAssembly, processorArchitecture=x86, version=6.0.0.0, type=win32, publicKeyToken=a5aaf5ba15723d5"
```

### -rgs

Specifies the name of the registration script (.rgs) file. The -dll option is required to use the -rgs option.

### -tlb

Specifies the name of the type library (.tlb) file. The -dll option is required to use the -tlb option.

### -dll

Specifies the name of the dynamic-link library (DLL) file. The -dll option is required by mt.exe if the -rgs or -tlb options are used. Specify the name of the DLL you intend to eventually build from the .rgs or .tlb files.

For example, the following command requests an operation that generates a manifest from .rgs and .tlb files.

```
mt.exe -rgs:testreg1.rgs -tlb:testlib1.tlb -dll:test.dll -replacements:rep.manifest -identity:"Microsoft.Windows.SampleAssembly, processorArchitecture=x86, version=6.0.0.0, type=win32, publicKeyToken=a5aaf5ba15723d5" -out:rgstlb.manifest
```

### -replacements

Specifies the file that contains values for the replaceable string in the .rgs file.

### -managedassemblyname

Generates a manifest from the specified managed assembly. Use with the -nodependency option to generate a manifest without dependency elements. Use with the -category option to generate a manifest with category tags. For example, if managed.dll is a managed assembly, the following command line generates the out.manifest from managed.dll.

```
mt.exe -managedassemblyname:managed.dll -out:out.manifest
```

### -nodependency

Specifies an operation that generates a manifest without dependency elements. The -nodependency option requires the -managedassemblyname option. For example, if managed.dll is a managed assembly, the following command line generates the out.manifest from managed.dll without dependency information.

```
mt.exe -managedassemblyname:managed.dll -out:out.manifest -nodependency
```

### -category

Specifies an operation that generates a manifest with category tags. The -category option requires the -managedassemblyname option. For example, if managed.dll is a managed assembly, the following command line generates the out.manifest from managed.dll with category tags.

```
mt.exe -managedassemblyname:managed.dll -out:out.manifest -category
```

### -nologo

Specifies an operation that is run without displaying standard Microsoft copyright data. If mt.exe runs as part of a build process, this option can be used to prevent writing unwanted information into the log files.

### -out

Specifies the name of the updated manifest. If this is a single-manifest operation, and the -out option is omitted, the original manifest is modified.

### -inputresource

Specifies an operation performed on a manifest obtained from a resource of type RT_MANIFEST. If the -inputresource option is used without specifying the resource identifier, <resource_id>, the operation uses the value CREATEPROCESS_MANIFEST_RESOURCE.

For example, the following command requests an operation that merges a manifest from a DLL, dll_with_manifest.dll, and a manifest file, man2.manifest. The merged manifests are received by a manifest in the resource file of another DLL, dll_with_merged_manifests.

```
mt.exe -inputresource:dll_with_manifest.dll;#1 -manifest man2.manifest -outputresource:dll_with_merged_manifest.dll;#3
```

To extract the manifest from a DLL, specify the DLL file name. For example, the following command extracts the manifest from lib1.dll and man3.manifest receives the extracted manifest.

```
mt.exe -inputresource:lib.dll;#1 -out:man3.manifest
```

### -outputresource

Specifies an operation that generates a manifest to be received by a resource of type RT_MANIFEST. If the -outputresource option is used without specifying the resource identifier, <resource_id>, the operation uses the value CREATEPROCESS_MANIFEST_RESOURCE.

### -updateresource

Specifies an operation that is equivalent to using the -inputresource and -outputresource options with identical arguments. For example, the following command requests an operation that computes a hash of the files at the specified path and updates the manifest of a resource of a portable executable (PE).

```
mt.exe -updateresource:dll_with_manifest.dll;#1 -hashupdate:f:\files.
```

### -hashupdate

Computes the hash value of the files at the specified paths and updates the value of the hash attribute of the File element with this value.

For example, the following command requests an operation that merges two manifest files, man1.manifest and man2.manifest, and updates the value of the hash attribute of the File element in the manifest that receives the merged information, merged.manifest.

```
mt.exe -manifest man1.manifest man2.manifest -hashupdate:d:\filerepository -out:merged.manifest
```

If the paths to the files are not specified, the operation searches location of the manifest specified to receive the update. For example, the following command requests an operation that computes the updated hash value using files found by searching the location of updated.manifest.

```
mt.exe -manifest yourComponent.manifest -hashupdate -out:updated.manifest
```

### -validate_manifest

Specifies an operation that performs a syntax check of the conformance of the manifest with the manifest schema. For example, the following command requests a check to validate the conformance of man1.manifest with its schema.

```
mt.exe -manifest man1.manifest -validate_manifest
```

### -validate_file_hashes

Specifies an operation that validates the hash values of the File elements of the manifest. For example, the following command requests an operation that validates the hash values of all the File elements of the man1.manifest.

```
mt.exe -manifest man1.manifest -validate_file_hashes:"c;\files"
```

### -canonicalize

Specifies an operation to update the manifest to canonical form. For example, the following command updates man1.manifest to canonical form.

```
mt.exe -manifest man1.manifest
```

### -check_for_duplicates

Specifies an operation that checks the manifest for duplicate elements. For example, the following command checks man1.manifest for duplicate elements.

```
mt.exe -man1.manifest -check_for_duplicates
```

### -makecdfs

Generates .cdf files to make catalogs. For example, to the following command requests an operation that updates the hash value and generates a .cdf file.

```
mt.exe -manifest comp1.manifest -hashupdate -makecdfs -out:updated.manifest
```

### -verbose

Displays verbose debugging information.

### -?

When run with -?, or with no options and arguments, Mt.exe displays help text.

## Using Mt.exe with pywin32

For pywin32 development, Mt.exe can be particularly useful for:

1. Adding application manifests to enable features like long path support
2. Embedding manifests into DLLs and EXEs as resources
3. Validating manifests for correctness
4. Generating manifests for COM components

When building pywin32 extensions that need to declare specific Windows features or dependencies, using Mt.exe to create and embed appropriate manifests is an essential step in the build process.
