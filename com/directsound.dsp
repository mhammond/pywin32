# Microsoft Developer Studio Project File - Name="directsound" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=directsound - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "directsound.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "directsound.mak" CFG="directsound - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "directsound - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "directsound - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "directsound - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Build"
# PROP Intermediate_Dir "Build\Temp\directsound\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
F90=df.exe
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "DIRECTSOUND_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /Zi /O2 /I "..\com\win32com\src\include" /I "..\win32\src" /D "WIN32" /D "_WINDOWS" /D "NDEBUG" /D "STRICT" /YX"directsound_pch.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 odbccp32.lib kernel32.lib user32.lib gdi32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib dsound.lib dxguid.lib /nologo /dll /pdb:"Build/directsound_d.pdb" /debug /machine:I386 /out:"Build\directsound.pyd" /libpath:"..\win32\build"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "directsound - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Build"
# PROP Intermediate_Dir "Build\Temp\directsound\debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
F90=df.exe
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "DIRECTSOUND_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\com\win32com\src\include" /I "..\win32\src" /D "WIN32" /D "_WINDOWS" /D "_DEBUG" /D "STRICT" /FR /YX"directsound_pch.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x407 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib dsound.lib dxguid.lib /nologo /dll /debug /machine:I386 /out:"Build\directsound_d.pyd" /pdbtype:sept /libpath:"..\win32\build"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "directsound - Win32 Release"
# Name "directsound - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\win32comext\directsound\src\directsound.cpp
# End Source File
# Begin Source File

SOURCE=.\win32comext\directsound\src\PyDSBCAPS.cpp
# End Source File
# Begin Source File

SOURCE=.\win32comext\directsound\src\PyDSBUFFERDESC.cpp
# End Source File
# Begin Source File

SOURCE=.\win32comext\directsound\src\PyDSCAPS.cpp
# End Source File
# Begin Source File

SOURCE=.\win32comext\directsound\src\PyDSCBCAPS.cpp
# End Source File
# Begin Source File

SOURCE=.\win32comext\directsound\src\PyDSCBUFFERDESC.cpp
# End Source File
# Begin Source File

SOURCE=.\win32comext\directsound\src\PyDSCCAPS.cpp
# End Source File
# Begin Source File

SOURCE=.\win32comext\directsound\src\PyIDirectSound.cpp
# End Source File
# Begin Source File

SOURCE=.\win32comext\directsound\src\PyIDirectSoundBuffer.cpp
# End Source File
# Begin Source File

SOURCE=.\win32comext\directsound\src\PyIDirectSoundCapture.cpp
# End Source File
# Begin Source File

SOURCE=.\win32comext\directsound\src\PyIDirectSoundCaptureBuffer.cpp
# End Source File
# Begin Source File

SOURCE=.\win32comext\directsound\src\PyIDirectSoundNotify.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\win32comext\directsound\src\directsound_pch.h
# End Source File
# Begin Source File

SOURCE=.\win32comext\directsound\src\PyIDirectSound.h
# End Source File
# Begin Source File

SOURCE=.\win32comext\directsound\src\PyIDirectSoundBuffer.h
# End Source File
# Begin Source File

SOURCE=.\win32comext\directsound\src\PyIDirectSoundCapture.h
# End Source File
# Begin Source File

SOURCE=.\win32comext\directsound\src\PyIDirectSoundCaptureBuffer.h
# End Source File
# Begin Source File

SOURCE=.\win32comext\directsound\src\PyIDirectSoundNotify.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
