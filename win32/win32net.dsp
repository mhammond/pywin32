# Microsoft Developer Studio Project File - Name="win32net" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=win32net - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "win32net.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "win32net.mak" CFG="win32net - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "win32net - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "win32net - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Python/win32", NHAAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "win32net - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Build"
# PROP BASE Intermediate_Dir "Build\Temp\win32net\Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Build"
# PROP Intermediate_Dir "Build\Temp\win32net\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
F90=df.exe
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /Zi /O2 /I "..\win32\src" /D "NDEBUG" /D "WINNT" /D "UNICODE" /D "_UNICODE" /D "_USRDLL" /D "_WINDLL" /D "_AFX_NOFORCE_LIBS" /D "WIN32" /D "_WINDOWS" /D "__WIN32__" /D "STRICT" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o /win32 "NUL"
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o /win32 "NUL"
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 netapi32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /base:"0x1e290000" /subsystem:windows /dll /debug /machine:I386 /out:"Build\win32net.pyd"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "win32net - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Build"
# PROP BASE Intermediate_Dir "Build\Temp\win32net\Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Build"
# PROP Intermediate_Dir "Build\Temp\win32net\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
F90=df.exe
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MDd /W3 /GX /ZI /Od /I "..\win32\src" /D "_DEBUG" /D "WINNT" /D "UNICODE" /D "_UNICODE" /D "_USRDLL" /D "_WINDLL" /D "_AFX_NOFORCE_LIBS" /D "WIN32" /D "_WINDOWS" /D "__WIN32__" /D "STRICT" /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o /win32 "NUL"
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o /win32 "NUL"
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 netapi32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /base:"0x1e290000" /subsystem:windows /dll /debug /machine:I386 /out:"Build\win32net_d.pyd" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "win32net - Win32 Release"
# Name "win32net - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\win32net\win32net.h
# End Source File
# Begin Source File

SOURCE=.\src\win32net\win32netgroup.cpp
# End Source File
# Begin Source File

SOURCE=.\src\win32net\win32netmisc.cpp
# End Source File
# Begin Source File

SOURCE=.\src\win32net\win32netmodule.cpp
# End Source File
# Begin Source File

SOURCE=.\src\win32net\win32netuse.cpp
# End Source File
# Begin Source File

SOURCE=.\src\win32net\win32netuser.cpp
# End Source File
# End Group
# End Target
# End Project
