# Microsoft Developer Studio Project File - Name="Active Scripting" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=Active Scripting - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Active Scripting.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Active Scripting.mak" CFG="Active Scripting - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Active Scripting - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Active Scripting - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Python/com/win32comext/axscript", RJAAAAAA"
# PROP Scc_LocalPath "win32comext/axscript/src"
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Active Scripting - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\win32comext\AXScript\src\Release"
# PROP BASE Intermediate_Dir ".\win32comext\AXScript\src\Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Build"
# PROP Intermediate_Dir "Build\Temp\AXScript\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
F90=df.exe
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MD /W3 /GX /Zi /O2 /I "..\com\win32com\src\include" /I "..\win32\src" /D "WIN32" /D "_WINDOWS" /D "WIDEOLE" /D "PY_BUILD_AXSCRIPT" /D "NDEBUG" /D "STRICT" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0xc09 /d "NDEBUG"
# ADD RSC /l 0xc09 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 winspool.lib comdlg32.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /base:"0x1e2b0000" /subsystem:windows /dll /debug /machine:I386 /out:"Build\axscript.pyd" /libpath:"..\win32\build"
# SUBTRACT LINK32 /pdb:none /nodefaultlib

!ELSEIF  "$(CFG)" == "Active Scripting - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\win32comext\AXScript\src\Debug"
# PROP BASE Intermediate_Dir ".\win32comext\AXScript\src\Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Build"
# PROP Intermediate_Dir "Build\Temp\AXScript\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
F90=df.exe
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MDd /W3 /GX /ZI /Od /I "..\com\win32com\src\include" /I "..\win32\src" /D "WIN32" /D "_WINDOWS" /D "WIDEOLE" /D "PY_BUILD_AXSCRIPT" /D "_DEBUG" /D "STRICT" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0xc09 /d "_DEBUG"
# ADD RSC /l 0xc09 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386
# ADD LINK32 winspool.lib comdlg32.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /base:"0x1e2b0000" /subsystem:windows /dll /debug /machine:I386 /nodefaultlib:"msvcrt.lib" /out:"Build\axscript_d.pyd" /libpath:"..\win32\build"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "Active Scripting - Win32 Release"
# Name "Active Scripting - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90"
# Begin Source File

SOURCE=.\win32comext\AXScript\src\AXScript.cpp
# End Source File
# Begin Source File

SOURCE=.\win32comext\AXScript\src\GUIDS.CPP
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\win32comext\AXScript\src\PyGActiveScript.cpp
# End Source File
# Begin Source File

SOURCE=.\win32comext\AXScript\src\PyGActiveScriptError.cpp
# End Source File
# Begin Source File

SOURCE=.\win32comext\AXScript\src\PyGActiveScriptParse.cpp
# End Source File
# Begin Source File

SOURCE=.\win32comext\AXScript\src\PyGActiveScriptSite.cpp
# End Source File
# Begin Source File

SOURCE=.\win32comext\AXScript\src\PyGObjectSafety.cpp
# End Source File
# Begin Source File

SOURCE=.\win32comext\AXScript\src\PyIActiveScript.cpp
# End Source File
# Begin Source File

SOURCE=.\win32comext\AXScript\src\PyIActiveScriptError.cpp
# End Source File
# Begin Source File

SOURCE=.\win32comext\AXScript\src\PyIActiveScriptParse.cpp
# End Source File
# Begin Source File

SOURCE=.\win32comext\AXScript\src\PyIActiveScriptParseProcedure.cpp
# End Source File
# Begin Source File

SOURCE=.\win32comext\AXScript\src\PyIActiveScriptSite.cpp
# End Source File
# Begin Source File

SOURCE=.\win32comext\AXScript\src\PyIMultiInfos.cpp
# End Source File
# Begin Source File

SOURCE=.\win32comext\AXScript\src\PyIObjectSafety.cpp
# End Source File
# Begin Source File

SOURCE=.\win32comext\AXScript\src\stdafx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Source File

SOURCE=.\win32comext\AXScript\src\AXScript.h
# End Source File
# Begin Source File

SOURCE=.\win32comext\AXScript\src\guids.h
# End Source File
# Begin Source File

SOURCE=.\win32comext\AXScript\src\PyGActiveScriptError.h
# End Source File
# Begin Source File

SOURCE=.\win32comext\AXScript\src\PyIActiveScriptError.h
# End Source File
# Begin Source File

SOURCE=.\win32comext\AXScript\src\PyIObjectSafety.h
# End Source File
# Begin Source File

SOURCE=.\win32comext\AXScript\src\PyIProvideMultipleClassInfo.h
# End Source File
# Begin Source File

SOURCE=.\win32comext\AXScript\src\stdafx.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
