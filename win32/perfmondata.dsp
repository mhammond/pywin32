# Microsoft Developer Studio Project File - Name="perfmondata" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=perfmondata - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "perfmondata.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "perfmondata.mak" CFG="perfmondata - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "perfmondata - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "perfmondata - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Python/win32/PerfMon", PNCAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "perfmondata - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Build"
# PROP BASE Intermediate_Dir "Build\Temp\perfmondata\Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Build"
# PROP Intermediate_Dir "Build\Temp\perfmon\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
F90=df.exe
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /Zi /O2 /D "UNICODE" /D "_UNICODE" /D "WIN32" /D "_WINDOWS" /D "NDEBUG" /D "STRICT" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o /win32 "NUL"
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o /win32 "NUL"
# ADD BASE RSC /l 0xc09 /d "NDEBUG"
# ADD RSC /l 0xc09 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /def:".\src\perfmon\perfmondata.def"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "perfmondata - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Build"
# PROP BASE Intermediate_Dir "Build\Temp\perfmondata\Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Build"
# PROP Intermediate_Dir "Build\Temp\perfmondata\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
F90=df.exe
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MDd /W3 /GX /ZI /Od /D "UNICODE" /D "_UNICODE" /D "WIN32" /D "_WINDOWS" /D "_DEBUG" /D "STRICT" /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o /win32 "NUL"
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o /win32 "NUL"
# ADD BASE RSC /l 0xc09 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"Build\perfmondata_d.dll" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "perfmondata - Win32 Release"
# Name "perfmondata - Win32 Debug"
# Begin Source File

SOURCE=.\src\perfmon\perfmondata.cpp
# End Source File
# Begin Source File

SOURCE=.\src\perfmon\perfmondata.def

!IF  "$(CFG)" == "perfmondata - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "perfmondata - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\perfmon\perfutil.h
# End Source File
# Begin Source File

SOURCE=.\src\perfmon\PyPerfMonControl.h
# End Source File
# Begin Source File

SOURCE=.\src\perfmon\PyPerfMsgs.mc

!IF  "$(CFG)" == "perfmondata - Win32 Release"

# Begin Custom Build
InputDir=.\src\perfmon
IntDir=.\Build\Temp\perfmon\Release
InputPath=.\src\perfmon\PyPerfMsgs.mc
InputName=PyPerfMsgs

BuildCmds= \
	mc -r $(IntDir) -h src\PerfMon\. -s $(InputDir)\$(InputName) \
	rc /Fo src\Perfmon\PyPerfMsgs.res $(IntDir)\PyPerfMsgs.rc \
	

"src\PerfMon\PyPerfMsgs.res" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"src\PerfMon\PyPerfMsgs.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(IntDir)\msg00001.bin" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(IntDir)\PyPerfMsgs.rc" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "perfmondata - Win32 Debug"

# Begin Custom Build
InputDir=.\src\perfmon
IntDir=.\Build\Temp\perfmondata\Debug
InputPath=.\src\perfmon\PyPerfMsgs.mc
InputName=PyPerfMsgs

BuildCmds= \
	mc -r $(IntDir) -h src\PerfMon\. -s $(InputDir)\$(InputName) \
	rc /Fo src\Perfmon\PyPerfMsgs.res $(IntDir)\PyPerfMsgs.rc \
	

"src\PerfMon\PyPerfMsgs.res" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"src\PerfMon\PyPerfMsgs.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(IntDir)\msg00001.bin" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(IntDir)\PyPerfMsgs.rc" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# End Target
# End Project
