# Microsoft Developer Studio Project File - Name="win32evtlog" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=win32evtlog - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "win32evtlog.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "win32evtlog.mak" CFG="win32evtlog - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "win32evtlog - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "win32evtlog - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Python/win32", IGAAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "win32evtlog - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Build"
# PROP BASE Intermediate_Dir "Build\Temp\win32evtlog\Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Build"
# PROP Intermediate_Dir "Build\Temp\win32evtlog\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
F90=df.exe
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /Zi /O2 /D "WIN32" /D "_WINDOWS" /D "__WIN32__" /D "NDEBUG" /D "STRICT" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o /win32 "NUL"
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o /win32 "NUL"
# ADD BASE RSC /l 0xc09 /d "NDEBUG"
# ADD RSC /l 0xc09 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 Build\Temp\win32evtlog\Release\win32evtlog_messages.res kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /base:"0x1e820000" /subsystem:windows /dll /debug /machine:I386 /out:"Build\win32evtlog.pyd" /libpath:"./Build"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "win32evtlog - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Build"
# PROP BASE Intermediate_Dir "Build\Temp\win32evtlog\Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Build"
# PROP Intermediate_Dir "Build\Temp\win32evtlog\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
F90=df.exe
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MDd /W3 /GX /ZI /Od /D "WIN32" /D "_WINDOWS" /D "__WIN32__" /D "_DEBUG" /D "STRICT" /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o /win32 "NUL"
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o /win32 "NUL"
# ADD BASE RSC /l 0xc09 /d "_DEBUG"
# ADD RSC /l 0xc09 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 Build\Temp\win32evtlog\Debug\win32evtlog_messages.res kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /base:"0x1e820000" /subsystem:windows /dll /debug /machine:I386 /out:"Build\win32evtlog_d.pyd" /pdbtype:sept /libpath:"./Build"

!ENDIF 

# Begin Target

# Name "win32evtlog - Win32 Release"
# Name "win32evtlog - Win32 Debug"
# Begin Source File

SOURCE=.\src\win32evtlog.i

!IF  "$(CFG)" == "win32evtlog - Win32 Release"

# Begin Custom Build - Invoking SWIG...
InputDir=.\src
InputPath=.\src\win32evtlog.i
InputName=win32evtlog

"$(InputDir)\$(InputName)module.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\swig.bat $(InputDir) -c++ -o $(InputName)module.cpp $(InputName).i

# End Custom Build

!ELSEIF  "$(CFG)" == "win32evtlog - Win32 Debug"

# Begin Custom Build - Invoking SWIG...
InputDir=.\src
InputPath=.\src\win32evtlog.i
InputName=win32evtlog

"$(InputDir)\$(InputName)module.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\swig.bat $(InputDir) -c++ -o $(InputName)module.cpp $(InputName).i

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\win32evtlog_messages.mc

!IF  "$(CFG)" == "win32evtlog - Win32 Release"

# Begin Custom Build
InputDir=.\src
IntDir=.\Build\Temp\win32evtlog\Release
InputPath=.\src\win32evtlog_messages.mc
InputName=win32evtlog_messages

"$(IntDir)\$(InputName).rc" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mc -h $(InputDir) -r $(IntDir) $(InputPath) 
	rc $(IntDir)\$(InputName).res 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "win32evtlog - Win32 Debug"

# Begin Custom Build
InputDir=.\src
IntDir=.\Build\Temp\win32evtlog\Debug
InputPath=.\src\win32evtlog_messages.mc
InputName=win32evtlog_messages

"$(IntDir)\$(InputName).rc" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	mc -h $(InputDir) -r $(IntDir) $(InputPath) 
	rc $(IntDir)\$(InputName).res 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\win32evtlogmodule.cpp
# End Source File
# End Target
# End Project
