# Microsoft Developer Studio Project File - Name="exchdapi" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=exchdapi - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "exchdapi.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "exchdapi.mak" CFG="exchdapi - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "exchdapi - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "exchdapi - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "exchdapi - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Build"
# PROP Intermediate_Dir "Build\Temp\mapi\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
F90=df.exe
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /Zi /O2 /I "..\win32\src" /I "..\com\win32com\src\include" /D "__WIN32__" /D "WIN32" /D "_WINDOWS" /D "NO_PY_UNICODE" /D "NDEBUG" /D "STRICT" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o /win32 "NUL"
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o /win32 "NUL"
# ADD BASE RSC /l 0xc09 /d "NDEBUG"
# ADD RSC /l 0xc09 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 DAPI.LIB ADDRLKUP.LIB exchinst.lib EDKCFG.LIB EDKUTILS.LIB EDKMAPI.LIB mapi32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /base:"0x1e780000" /subsystem:windows /dll /debug /machine:I386 /nodefaultlib:"libc" /out:"Build/exchdapi.pyd" /libpath:"..\win32\build"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "exchdapi - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "exchdapi"
# PROP BASE Intermediate_Dir "exchdapi"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Build"
# PROP Intermediate_Dir "Build\Temp\mapi\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
F90=df.exe
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MDd /W3 /GX /ZI /Od /I "..\win32\src" /I "..\com\win32com\src\include" /D "__WIN32__" /D "WIN32" /D "_WINDOWS" /D "NO_PY_UNICODE" /D "_DEBUG" /D "STRICT" /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o /win32 "NUL"
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o /win32 "NUL"
# ADD BASE RSC /l 0xc09 /d "_DEBUG"
# ADD RSC /l 0xc09 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 DAPI.LIB ADDRLKUP.LIB exchinst.lib EDKCFG.LIB EDKUTILS.LIB EDKMAPI.LIB mapi32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /base:"0x1e780000" /subsystem:windows /dll /debug /machine:I386 /nodefaultlib:"libc" /out:"Build\exchdapi_d.pyd" /pdbtype:sept /libpath:"..\win32\build"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "exchdapi - Win32 Release"
# Name "exchdapi - Win32 Debug"
# Begin Source File

SOURCE=.\win32comext\mapi\src\exchdapi.cpp
# End Source File
# Begin Source File

SOURCE=.\win32comext\mapi\src\exchdapi.i

!IF  "$(CFG)" == "exchdapi - Win32 Release"

USERDEP__EXCHD=".\win32comext\mapi\src\mapilib.i"	
# Begin Custom Build - Invoking SWIG...
InputDir=.\win32comext\mapi\src
InputPath=.\win32comext\mapi\src\exchdapi.i
InputName=exchdapi

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\swig.bat $(InputDir) -dnone -python -c++ -o $(InputName).cpp $(InputName).i

# End Custom Build

!ELSEIF  "$(CFG)" == "exchdapi - Win32 Debug"

USERDEP__EXCHD=".\win32comext\mapi\src\mapilib.i"	
# Begin Custom Build - Invoking SWIG...
InputDir=.\win32comext\mapi\src
InputPath=.\win32comext\mapi\src\exchdapi.i
InputName=exchdapi

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\swig.bat $(InputDir) -dnone -python -c++ -o $(InputName).cpp $(InputName).i

# End Custom Build

!ENDIF 

# End Source File
# End Target
# End Project
