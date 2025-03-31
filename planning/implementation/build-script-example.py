# Example code to add to setup.py for embedding the manifest
# This is a simplified example - actual implementation may need adjustments

import os
import sys
import subprocess
from distutils.errors import DistutilsExecError

def build_extension(self, ext):
    # Call the original build_extension method
    original_build_extension(self, ext)
    
    # After pythonservice.exe is built, embed the manifest
    if ext.name == "servicemanager":
        try:
            # Path to the built pythonservice.exe
            exe_path = os.path.join(self.build_lib, "win32", "pythonservice.exe")
            
            # Path to the manifest file
            manifest_dir = os.path.join(os.path.dirname(__file__), "win32", "src", "PythonService")
            manifest_path = os.path.join(manifest_dir, "pythonservice.exe.manifest")
            
            # Ensure the manifest directory exists
            if not os.path.exists(manifest_dir):
                os.makedirs(manifest_dir)
            
            # Copy the manifest template if it doesn't exist
            if not os.path.exists(manifest_path):
                template_path = os.path.join(os.path.dirname(__file__), "planning", "implementation", "manifest-template.xml")
                if os.path.exists(template_path):
                    import shutil
                    shutil.copy(template_path, manifest_path)
                else:
                    # Create the manifest file directly
                    with open(manifest_path, "w") as f:
                        f.write("""<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
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
      <requestedPrivileges xmlns="urn:schemas-microsoft-com:asm.v3">
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
</assembly>""")
            
            # Check if mt.exe is available
            mt_exe = "mt.exe"
            try:
                # Try to find mt.exe in Windows SDK
                if sys.platform == "win32":
                    import winreg
                    try:
                        with winreg.OpenKey(winreg.HKEY_LOCAL_MACHINE, r"SOFTWARE\Microsoft\Windows Kits\Installed Roots") as key:
                            kit_root = winreg.QueryValueEx(key, "KitsRoot10")[0]
                            # Look for mt.exe in different bin directories
                            for arch in ["x64", "x86", "arm64"]:
                                for version in os.listdir(os.path.join(kit_root, "bin")):
                                    mt_path = os.path.join(kit_root, "bin", version, arch, "mt.exe")
                                    if os.path.exists(mt_path):
                                        mt_exe = mt_path
                                        break
                                if mt_exe != "mt.exe":
                                    break
                    except (WindowsError, OSError):
                        pass
                
                # Embed the manifest using mt.exe
                cmd = [mt_exe, "-manifest", manifest_path, "-outputresource:%s;1" % exe_path]
                print("Embedding manifest into pythonservice.exe...")
                subprocess.check_call(cmd)
                print("Successfully embedded manifest.")
            except (subprocess.SubprocessError, FileNotFoundError) as e:
                print(f"Warning: Failed to embed manifest: {e}")
                print("Long path support may not be available.")
                # Don't fail the build if mt.exe is not available
        except Exception as e:
            print(f"Error during manifest embedding: {e}")
            # Don't fail the build for manifest issues

# Store the original build_extension method
original_build_extension = YourExtensionBuilderClass.build_extension
# Replace with our version
YourExtensionBuilderClass.build_extension = build_extension
