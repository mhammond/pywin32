# Microsoft Developer Studio Project File - Name="Active Scripting Debugging" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=Active Scripting Debugging - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Active Scripting Debugging.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Active Scripting Debugging.mak"\
 CFG="Active Scripting Debugging - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Active Scripting Debugging - Win32 Release" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Active Scripting Debugging - Win32 Debug" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""$/com/win32comext\axsdebug", KKAAAAAA"
# PROP Scc_LocalPath "win32comext/axdebug/src"
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Active Scripting Debugging - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\AXScriptDebug\Release"
# PROP BASE Intermediate_Dir ".\AXScriptDebug\Release"
# PROP BASE Target_Dir ".\AXScriptDebug"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Build"
# PROP Intermediate_Dir "Build\Temp\AXSDebug\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ".\AXScriptDebug"
F90=df.exe
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "d:\dbgsdk\include" /I ".\win32com\src\include" /I ".\win32comext\axscript\src" /I "..\win32\src" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /Yu"stdafxaxs.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0xc09 /d "NDEBUG"
# ADD RSC /l 0xc09 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 Build\AXScript.lib msdbg.lib winspool.lib comdlg32.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /base:"0x1e3B0000" /subsystem:windows /dll /pdb:none /machine:I386 /out:"Build\axsdebug.pyd"
# SUBTRACT LINK32 /debug

!ELSEIF  "$(CFG)" == "Active Scripting Debugging - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\AXScriptDebug\Debug"
# PROP BASE Intermediate_Dir ".\AXScriptDebug\Debug"
# PROP BASE Target_Dir ".\AXScriptDebug"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Build"
# PROP Intermediate_Dir "Build\Temp\AXSDebug\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ".\AXScriptDebug"
F90=df.exe
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /I "d:\dbgsdk\include" /I ".\win32com\src\include" /I ".\win32comext\axscript\src" /I "..\win32\src" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /Yu"stdafxaxs.h" /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0xc09 /d "_DEBUG"
# ADD RSC /l 0xc09 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386
# ADD LINK32 Build\AXScript_d.lib msdbg.lib winspool.lib comdlg32.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /base:"0x1e3B0000" /subsystem:windows /dll /debug /machine:I386 /nodefaultlib:"msvcrt" /out:"Build\axsdebug_d.pyd"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "Active Scripting Debugging - Win32 Release"
# Name "Active Scripting Debugging - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90"
# Begin Source File

SOURCE=.\win32comext\AXDebug\src\AXSDebug.cpp
# End Source File
# Begin Source File

SOURCE=.\win32comext\axdebug\src\PyIActiveScriptErrorDebug.cpp
# End Source File
# Begin Source File

SOURCE=.\win32comext\AXDebug\src\stdafxaxs.cpp
# ADD CPP /Yc"stdafxaxs.h"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Source File

SOURCE=.\win32comext\AXDebug\src\stdafxaxs.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
