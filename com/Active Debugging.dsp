# Microsoft Developer Studio Project File - Name="Active Debugging" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=Active Debugging - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Active Debugging.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Active Debugging.mak" CFG="Active Debugging - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Active Debugging - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Active Debugging - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/com/win32comext/axdebug", TKAAAAAA"
# PROP Scc_LocalPath "win32comext/axdebug/src"
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Active Debugging - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\Release"
# PROP BASE Intermediate_Dir ".\Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Build"
# PROP Intermediate_Dir "Build\Temp\AXDebug\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
F90=df.exe
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MD /W3 /GX /Zi /O2 /I "..\win32\src" /I "..\com\win32com\src\include" /D "WIN32" /D "_WINDOWS" /D "NDEBUG" /D "STRICT" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0xc09 /d "NDEBUG"
# ADD RSC /l 0xc09 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 Build\axscript.lib msdbg.lib winspool.lib comdlg32.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /base:"0x1e3b0000" /subsystem:windows /dll /debug /machine:I386 /nodefaultlib:"libcmt" /out:"Build\axdebug.pyd" /libpath:"build" /libpath:"..\win32\build"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "Active Debugging - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\Debug"
# PROP BASE Intermediate_Dir ".\Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Build"
# PROP Intermediate_Dir "Build\Temp\AXSDebug\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
F90=df.exe
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MDd /W3 /GX /ZI /Od /I ".\win32\src" /I "..\win32\src" /I "..\com\win32com\src\include" /D "WIN32" /D "_WINDOWS" /D "_DEBUG" /D "STRICT" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0xc09 /d "_DEBUG"
# ADD RSC /l 0xc09 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386
# ADD LINK32 Build\axscript_d.lib msdbg.lib winspool.lib comdlg32.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /base:"0x1e3b0000" /subsystem:windows /dll /debug /machine:I386 /nodefaultlib:"libcmt" /nodefaultlib:"msvcrt.lib" /out:"Build\axdebug_d.pyd" /libpath:"build" /libpath:"..\win32\build"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "Active Debugging - Win32 Release"
# Name "Active Debugging - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90"
# Begin Source File

SOURCE=.\win32comext\AXDebug\src\AXDebug.cpp
# End Source File
# Begin Source File

SOURCE=.\win32comext\AXDebug\src\PyIActiveScriptDebug.cpp
# End Source File
# Begin Source File

SOURCE=.\win32comext\axdebug\src\PyIActiveScriptErrorDebug.cpp
# End Source File
# Begin Source File

SOURCE=.\win32comext\AXDebug\src\PyIActiveScriptSiteDebug.cpp
# End Source File
# Begin Source File

SOURCE=.\win32comext\AXDebug\src\PyIApplicationDebugger.cpp
# End Source File
# Begin Source File

SOURCE=.\win32comext\AXDebug\src\PyIDebugApplication.cpp
# End Source File
# Begin Source File

SOURCE=.\win32comext\AXDebug\src\PyIDebugApplicationNode.cpp
# End Source File
# Begin Source File

SOURCE=.\win32comext\AXDebug\src\PyIDebugApplicationNodeEvents.cpp
# End Source File
# Begin Source File

SOURCE=.\win32comext\AXDebug\src\PyIDebugApplicationThread.cpp
# End Source File
# Begin Source File

SOURCE=.\win32comext\AXDebug\src\PyIDebugCodeContext.cpp
# End Source File
# Begin Source File

SOURCE=.\win32comext\AXDebug\src\PyIDebugDocument.cpp
# End Source File
# Begin Source File

SOURCE=.\win32comext\AXDebug\src\PyIDebugDocumentContext.cpp
# End Source File
# Begin Source File

SOURCE=.\win32comext\AXDebug\src\PyIDebugDocumentHelper.cpp
# End Source File
# Begin Source File

SOURCE=.\win32comext\AXDebug\src\PyIDebugDocumentHost.cpp
# End Source File
# Begin Source File

SOURCE=.\win32comext\AXDebug\src\PyIDebugDocumentInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\win32comext\AXDebug\src\PyIDebugDocumentProvider.cpp
# End Source File
# Begin Source File

SOURCE=.\win32comext\AXDebug\src\PyIDebugDocumentText.cpp
# End Source File
# Begin Source File

SOURCE=.\win32comext\AXDebug\src\PyIDebugDocumentTextAuthor.cpp
# End Source File
# Begin Source File

SOURCE=.\win32comext\AXDebug\src\PyIDebugDocumentTextEvents.cpp
# End Source File
# Begin Source File

SOURCE=.\win32comext\AXDebug\src\PyIDebugDocumentTextExternalAuthor.cpp
# End Source File
# Begin Source File

SOURCE=.\win32comext\AXDebug\src\PyIDebugExpression.cpp
# End Source File
# Begin Source File

SOURCE=.\win32comext\AXDebug\src\PyIDebugExpressionCallBack.cpp
# End Source File
# Begin Source File

SOURCE=.\win32comext\AXDebug\src\PyIDebugExpressionContext.cpp
# End Source File
# Begin Source File

SOURCE=.\win32comext\AXDebug\src\PyIDebugProperties.cpp
# End Source File
# Begin Source File

SOURCE=.\win32comext\AXDebug\src\PyIDebugSessionProvider.cpp
# End Source File
# Begin Source File

SOURCE=.\win32comext\AXDebug\src\PyIDebugStackFrame.cpp
# End Source File
# Begin Source File

SOURCE=.\win32comext\AXDebug\src\PyIDebugStackFrameSniffer.cpp
# End Source File
# Begin Source File

SOURCE=.\win32comext\AXDebug\src\PyIDebugStackFrameSnifferEx.cpp
# End Source File
# Begin Source File

SOURCE=.\win32comext\AXDebug\src\PyIDebugSyncOperation.cpp
# End Source File
# Begin Source File

SOURCE=.\win32comext\AXDebug\src\PyIEnumDebugApplicationNodes.cpp
# End Source File
# Begin Source File

SOURCE=.\win32comext\AXDebug\src\PyIEnumDebugCodeContexts.cpp
# End Source File
# Begin Source File

SOURCE=.\win32comext\AXDebug\src\PyIEnumDebugExpressionContexts.cpp
# End Source File
# Begin Source File

SOURCE=.\win32comext\axdebug\src\PyIEnumDebugPropertyInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\win32comext\AXDebug\src\PyIEnumDebugStackFrames.cpp
# End Source File
# Begin Source File

SOURCE=.\win32comext\AXDebug\src\PyIEnumRemoteDebugApplications.cpp
# End Source File
# Begin Source File

SOURCE=.\win32comext\AXDebug\src\PyIEnumRemoteDebugApplicationThreads.cpp
# End Source File
# Begin Source File

SOURCE=.\win32comext\AXDebug\src\PyIMachineDebugManager.cpp
# End Source File
# Begin Source File

SOURCE=.\win32comext\AXDebug\src\PyIMachineDebugManagerEvents.cpp
# End Source File
# Begin Source File

SOURCE=.\win32comext\AXDebug\src\PyIProcessDebugManager.cpp
# End Source File
# Begin Source File

SOURCE=.\win32comext\axdebug\src\PyIProvideExpressionContexts.cpp
# End Source File
# Begin Source File

SOURCE=.\win32comext\AXDebug\src\PyIRemoteDebugApplication.cpp
# End Source File
# Begin Source File

SOURCE=.\win32comext\AXDebug\src\PyIRemoteDebugApplicationEvents.cpp
# End Source File
# Begin Source File

SOURCE=.\win32comext\AXDebug\src\PyIRemoteDebugApplicationThread.cpp
# End Source File
# Begin Source File

SOURCE=.\win32comext\AXDebug\src\stdafx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Source File

SOURCE=.\win32comext\AXDebug\src\PyIActiveScriptDebug.h
# End Source File
# Begin Source File

SOURCE=.\win32comext\AXDebug\src\PyIApplicationDebugger.h
# End Source File
# Begin Source File

SOURCE=.\win32comext\AXDebug\src\PyIDebugApplication.h
# End Source File
# Begin Source File

SOURCE=.\win32comext\AXDebug\src\PyIDebugApplicationNode.h
# End Source File
# Begin Source File

SOURCE=.\win32comext\AXDebug\src\PyIDebugApplicationNodeEvents.h
# End Source File
# Begin Source File

SOURCE=.\win32comext\AXDebug\src\PyIDebugApplicationThread.h
# End Source File
# Begin Source File

SOURCE=.\win32comext\AXDebug\src\PyIDebugCodeContext.h
# End Source File
# Begin Source File

SOURCE=.\win32comext\AXDebug\src\PyIDebugDocument.h
# End Source File
# Begin Source File

SOURCE=.\win32comext\AXDebug\src\PyIDebugDocumentContext.h
# End Source File
# Begin Source File

SOURCE=.\win32comext\AXDebug\src\PyIDebugDocumentHelper.h
# End Source File
# Begin Source File

SOURCE=.\win32comext\AXDebug\src\PyIDebugDocumentHost.h
# End Source File
# Begin Source File

SOURCE=.\win32comext\AXDebug\src\PyIDebugDocumentInfo.h
# End Source File
# Begin Source File

SOURCE=.\win32comext\AXDebug\src\PyIDebugDocumentProvider.h
# End Source File
# Begin Source File

SOURCE=.\win32comext\AXDebug\src\PyIDebugDocumentText.h
# End Source File
# Begin Source File

SOURCE=.\win32comext\AXDebug\src\PyIDebugDocumentTextAuthor.h
# End Source File
# Begin Source File

SOURCE=.\win32comext\AXDebug\src\PyIDebugDocumentTextEvents.h
# End Source File
# Begin Source File

SOURCE=.\win32comext\AXDebug\src\PyIDebugDocumentTextExternalAuthor.h
# End Source File
# Begin Source File

SOURCE=.\win32comext\AXDebug\src\PyIDebugExpressionCallBack.h
# End Source File
# Begin Source File

SOURCE=.\win32comext\AXDebug\src\PyIDebugExpressionContext.h
# End Source File
# Begin Source File

SOURCE=.\win32comext\AXDebug\src\PyIDebugSessionProvider.h
# End Source File
# Begin Source File

SOURCE=.\win32comext\AXDebug\src\PyIDebugStackFrameSniffer.h
# End Source File
# Begin Source File

SOURCE=.\win32comext\AXDebug\src\PyIDebugStackFrameSnifferEx.h
# End Source File
# Begin Source File

SOURCE=.\win32comext\AXDebug\src\PyIDebugSyncOperation.h
# End Source File
# Begin Source File

SOURCE=.\win32comext\AXDebug\src\PyIEnumDebugApplicationNodes.h
# End Source File
# Begin Source File

SOURCE=.\win32comext\AXDebug\src\PyIEnumDebugCodeContexts.h
# End Source File
# Begin Source File

SOURCE=.\win32comext\axdebug\src\PyIEnumDebugPropertyInfo.h
# End Source File
# Begin Source File

SOURCE=.\win32comext\AXDebug\src\PyIEnumDebugStackFrames.h
# End Source File
# Begin Source File

SOURCE=.\win32comext\AXDebug\src\PyIEnumRemoteDebugApplications.h
# End Source File
# Begin Source File

SOURCE=.\win32comext\AXDebug\src\PyIEnumRemoteDebugApplicationThreads.h
# End Source File
# Begin Source File

SOURCE=.\win32comext\AXDebug\src\PyIMachineDebugManager.h
# End Source File
# Begin Source File

SOURCE=.\win32comext\AXDebug\src\PyIMachineDebugManagerEvents.h
# End Source File
# Begin Source File

SOURCE=.\win32comext\AXDebug\src\PyIProcessDebugManager.h
# End Source File
# Begin Source File

SOURCE=.\win32comext\AXDebug\src\PyIRemoteDebugApplication.h
# End Source File
# Begin Source File

SOURCE=.\win32comext\AXDebug\src\PyIRemoteDebugApplicationEvents.h
# End Source File
# Begin Source File

SOURCE=.\win32comext\AXDebug\src\PyIRemoteDebugApplicationThread.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
