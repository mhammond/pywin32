# Microsoft Developer Studio Project File - Name="PyWinTypes" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=PyWinTypes - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "PyWinTypes.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "PyWinTypes.mak" CFG="PyWinTypes - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "PyWinTypes - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "PyWinTypes - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Python/win32", CDAAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "PyWinTypes - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Build"
# PROP BASE Intermediate_Dir "Build\Temp\PyWinTypes\Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Build"
# PROP Intermediate_Dir "Build\Temp\PyWinTypes\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
F90=df.exe
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /Zi /O2 /D "WIN32" /D "_WINDOWS" /D "BUILD_PYWINTYPES" /D "NDEBUG" /D "STRICT" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o /win32 "NUL"
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o /win32 "NUL"
# ADD BASE RSC /l 0xc09 /d "NDEBUG"
# ADD RSC /l 0xc09 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /base:"0x1e600000" /subsystem:windows /dll /pdb:"Build\System\PyWinTypes24.pdb" /debug /machine:I386 /out:"Build\System\PyWinTypes24.dll" /implib:"Build\PyWinTypes.lib"
# SUBTRACT LINK32 /pdb:none
# Begin Custom Build - copy to system32
ProjDir=.
TargetPath=.\Build\System\PyWinTypes24.dll
TargetName=PyWinTypes24
InputPath=.\Build\System\PyWinTypes24.dll
SOURCE="$(InputPath)"

"$(ProjDir)\$(TargetName).flg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(TargetPath) %SYSTEMROOT%\System32\. && echo Done >                                      $(ProjDir)\$(TargetName).flg

# End Custom Build

!ELSEIF  "$(CFG)" == "PyWinTypes - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Build"
# PROP BASE Intermediate_Dir "Build\Temp\PyWinTypes\Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Build"
# PROP Intermediate_Dir "Build\Temp\PyWinTypes\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
F90=df.exe
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MDd /W3 /GX /ZI /Od /D "WIN32" /D "_WINDOWS" /D "BUILD_PYWINTYPES" /D "_DEBUG" /D "DEBUG" /D "STRICT" /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o /win32 "NUL"
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o /win32 "NUL"
# ADD BASE RSC /l 0xc09 /d "_DEBUG"
# ADD RSC /l 0xc09 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /base:"0x1e600000" /subsystem:windows /dll /pdb:"Build\System\PyWinTypes24_d.pdb" /debug /machine:I386 /out:"Build\System\PyWinTypes24_d.dll" /implib:"Build\PyWinTypes_d.lib" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none
# Begin Custom Build - copy to system32
ProjDir=.
TargetPath=.\Build\System\PyWinTypes24_d.dll
TargetName=PyWinTypes24_d
InputPath=.\Build\System\PyWinTypes24_d.dll
SOURCE="$(InputPath)"

"$(ProjDir)\$(TargetName).flg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(TargetPath) %SYSTEMROOT%\System32\. && echo Done >                                      $(ProjDir)\$(TargetName).flg

# End Custom Build

!ENDIF 

# Begin Target

# Name "PyWinTypes - Win32 Release"
# Name "PyWinTypes - Win32 Debug"
# Begin Source File

SOURCE=.\src\PyACL.cpp
# End Source File
# Begin Source File

SOURCE=.\src\PyHANDLE.cpp
# End Source File
# Begin Source File

SOURCE=.\src\PyIID.cpp
# End Source File
# Begin Source File

SOURCE=.\src\PyLARGE_INTEGER.cpp
# End Source File
# Begin Source File

SOURCE=.\src\PyOVERLAPPED.cpp
# End Source File
# Begin Source File

SOURCE=.\src\PySECURITY_ATTRIBUTES.cpp
# End Source File
# Begin Source File

SOURCE=.\src\PySECURITY_DESCRIPTOR.cpp
# End Source File
# Begin Source File

SOURCE=.\src\PySecurityObjects.h
# End Source File
# Begin Source File

SOURCE=.\src\PySID.cpp
# End Source File
# Begin Source File

SOURCE=.\src\PyTime.cpp
# End Source File
# Begin Source File

SOURCE=.\src\PyUnicode.cpp
# End Source File
# Begin Source File

SOURCE=.\src\PyWinObjects.h
# End Source File
# Begin Source File

SOURCE=.\src\PyWinTypes.h
# End Source File
# Begin Source File

SOURCE=.\src\PyWinTypesmodule.cpp
# End Source File
# End Target
# End Project
