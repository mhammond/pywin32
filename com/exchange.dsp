# Microsoft Developer Studio Project File - Name="exchange" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=exchange - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "exchange.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "exchange.mak" CFG="exchange - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "exchange - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "exchange - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Python/com/win32comext/mapi", WHAAAAAA"
# PROP Scc_LocalPath "win32comext/mapi"
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "exchange - Win32 Release"

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
# ADD LINK32 MBLOGON.lib ADDRLKUP.LIB mapi32.lib exchinst.lib EDKCFG.LIB EDKUTILS.LIB EDKMAPI.LIB ACLCLS.LIB advapi32.lib ole32.lib oleaut32.lib uuid.lib version.lib /nologo /subsystem:windows /dll /debug /machine:I386 /nodefaultlib:"libc" /out:"Build/exchange.pyd" /libpath:"..\win32\build"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "exchange - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "exchange"
# PROP BASE Intermediate_Dir "exchange"
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
# ADD LINK32 MBLOGON.lib ADDRLKUP.LIB mapi32.lib exchinst.lib EDKCFG.LIB EDKUTILS.LIB EDKMAPI.LIB ACLCLS.LIB advapi32.lib ole32.lib oleaut32.lib uuid.lib version.lib /nologo /subsystem:windows /dll /debug /machine:I386 /nodefaultlib:"libc" /out:"Build\exchange_d.pyd" /pdbtype:sept /libpath:"..\win32\build"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "exchange - Win32 Release"
# Name "exchange - Win32 Debug"
# Begin Source File

SOURCE=.\win32comext\mapi\src\exchange.cpp
# End Source File
# Begin Source File

SOURCE=.\win32comext\mapi\src\exchange.i

!IF  "$(CFG)" == "exchange - Win32 Release"

USERDEP__EXCHA=".\win32comext\mapi\src\mapilib.i"	
# Begin Custom Build - Invoking SWIG...
InputDir=.\win32comext\mapi\src
InputPath=.\win32comext\mapi\src\exchange.i
InputName=exchange

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	%SWIG_EXE% -dnone -python -c++ -o $(InputName).cpp $(InputName).i 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "exchange - Win32 Debug"

USERDEP__EXCHA=".\win32comext\mapi\src\mapilib.i"	
# Begin Custom Build - Invoking SWIG...
InputDir=.\win32comext\mapi\src
InputPath=.\win32comext\mapi\src\exchange.i
InputName=exchange

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cd $(InputDir) 
	%SWIG_EXE% -dnone -python -c++ -o $(InputName).cpp $(InputName).i 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32comext\mapi\src\exchangeguids.cpp
# End Source File
# End Target
# End Project
