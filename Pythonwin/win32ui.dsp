# Microsoft Developer Studio Project File - Name="win32ui" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=win32ui - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "win32ui.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "win32ui.mak" CFG="win32ui - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "win32ui - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "win32ui - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Python/Pythonwin/win32ui", POAAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "win32ui - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\WinRel"
# PROP BASE Intermediate_Dir ".\WinRel"
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Build"
# PROP Intermediate_Dir "Build\Temp\win32ui\Release"
# PROP Ignore_Export_Lib 0
F90=df.exe
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /YX /c
# ADD CPP /nologo /MD /W3 /GX /Zi /O2 /I "..\win32\src" /D "NDEBUG" /D "BUILD_PYW" /D "_X86_" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_AFXEXT" /D "_MBCS" /YX"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 /nologo /base:"0x1e400000" /version:0.9 /subsystem:windows /dll /debug /machine:I386 /out:"Build\win32ui.pyd" /libpath:"..\win32\build"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "win32ui - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\WinDebug"
# PROP BASE Intermediate_Dir ".\WinDebug"
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Build"
# PROP Intermediate_Dir "Build\Temp\win32ui\Debug"
# PROP Ignore_Export_Lib 0
F90=df.exe
# ADD BASE CPP /nologo /MT /W3 /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /YX /c
# ADD CPP /nologo /MDd /W3 /GX /Zi /Od /I "..\win32\src" /D "DEBUG" /D "_DEBUG" /D "BUILD_PYW" /D "_X86_" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_AFXEXT" /D "_MBCS" /Yu"stdafx.h" /Fd"Build\Temp\win32ui\Debug" /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386
# ADD LINK32 /nologo /base:"0x1e400000" /version:0.9 /subsystem:windows /dll /debug /machine:I386 /out:"Build\win32ui_d.pyd" /libpath:"..\win32\build"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "win32ui - Win32 Release"
# Name "win32ui - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90"
# Begin Source File

SOURCE=.\dbgthread.cpp

!IF  "$(CFG)" == "win32ui - Win32 Release"

# ADD CPP /YX"stdafx.h"

!ELSEIF  "$(CFG)" == "win32ui - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\dibapi.cpp

!IF  "$(CFG)" == "win32ui - Win32 Release"

# ADD CPP /YX"stdafx.h"

!ELSEIF  "$(CFG)" == "win32ui - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\dllmain.cpp

!IF  "$(CFG)" == "win32ui - Win32 Release"

# ADD CPP /YX"stdafx.h"

!ELSEIF  "$(CFG)" == "win32ui - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\pythondoc.cpp

!IF  "$(CFG)" == "win32ui - Win32 Release"

# ADD CPP /YX"stdafx.h"

!ELSEIF  "$(CFG)" == "win32ui - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\pythonppage.cpp

!IF  "$(CFG)" == "win32ui - Win32 Release"

# ADD CPP /YX"stdafx.h"

!ELSEIF  "$(CFG)" == "win32ui - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\pythonpsheet.cpp

!IF  "$(CFG)" == "win32ui - Win32 Release"

# ADD CPP /YX"stdafx.h"

!ELSEIF  "$(CFG)" == "win32ui - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\pythonRichEditCntr.cpp

!IF  "$(CFG)" == "win32ui - Win32 Release"

# ADD CPP /YX"stdafx.h"

!ELSEIF  "$(CFG)" == "win32ui - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\pythonRichEditDoc.cpp

!IF  "$(CFG)" == "win32ui - Win32 Release"

# ADD CPP /YX"stdafx.h"

!ELSEIF  "$(CFG)" == "win32ui - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\pythonview.cpp

!IF  "$(CFG)" == "win32ui - Win32 Release"

# ADD CPP /YX"stdafx.h"

!ELSEIF  "$(CFG)" == "win32ui - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\stdafx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\win32app.cpp

!IF  "$(CFG)" == "win32ui - Win32 Release"

# ADD CPP /YX"stdafx.h"

!ELSEIF  "$(CFG)" == "win32ui - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32assoc.cpp

!IF  "$(CFG)" == "win32ui - Win32 Release"

# ADD CPP /YX"stdafx.h"

!ELSEIF  "$(CFG)" == "win32ui - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32bitmap.cpp

!IF  "$(CFG)" == "win32ui - Win32 Release"

# ADD CPP /YX"stdafx.h"

!ELSEIF  "$(CFG)" == "win32ui - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32brush.cpp

!IF  "$(CFG)" == "win32ui - Win32 Release"

# ADD CPP /YX"stdafx.h"

!ELSEIF  "$(CFG)" == "win32ui - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32cmd.cpp

!IF  "$(CFG)" == "win32ui - Win32 Release"

# ADD CPP /YX"stdafx.h"

!ELSEIF  "$(CFG)" == "win32ui - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32cmdui.cpp

!IF  "$(CFG)" == "win32ui - Win32 Release"

# ADD CPP /YX"stdafx.h"

!ELSEIF  "$(CFG)" == "win32ui - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32context.cpp

!IF  "$(CFG)" == "win32ui - Win32 Release"

# ADD CPP /YX"stdafx.h"

!ELSEIF  "$(CFG)" == "win32ui - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32control.cpp

!IF  "$(CFG)" == "win32ui - Win32 Release"

# ADD CPP /YX"stdafx.h"

!ELSEIF  "$(CFG)" == "win32ui - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32ctledit.cpp

!IF  "$(CFG)" == "win32ui - Win32 Release"

# ADD CPP /YX"stdafx.h"

!ELSEIF  "$(CFG)" == "win32ui - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32ctrlList.cpp

!IF  "$(CFG)" == "win32ui - Win32 Release"

# ADD CPP /YX"stdafx.h"

!ELSEIF  "$(CFG)" == "win32ui - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32ctrlRichEdit.cpp

!IF  "$(CFG)" == "win32ui - Win32 Release"

# ADD CPP /YX"stdafx.h"

!ELSEIF  "$(CFG)" == "win32ui - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32ctrlTree.cpp

!IF  "$(CFG)" == "win32ui - Win32 Release"

# ADD CPP /YX"stdafx.h"

!ELSEIF  "$(CFG)" == "win32ui - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32dc.cpp

!IF  "$(CFG)" == "win32ui - Win32 Release"

# ADD CPP /YX"stdafx.h"

!ELSEIF  "$(CFG)" == "win32ui - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32dlg.cpp

!IF  "$(CFG)" == "win32ui - Win32 Release"

# ADD CPP /YX"stdafx.h"

!ELSEIF  "$(CFG)" == "win32ui - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32dlgbar.cpp

!IF  "$(CFG)" == "win32ui - Win32 Release"

# ADD CPP /YX"stdafx.h"

!ELSEIF  "$(CFG)" == "win32ui - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32dll.cpp

!IF  "$(CFG)" == "win32ui - Win32 Release"

# ADD CPP /YX"stdafx.h"

!ELSEIF  "$(CFG)" == "win32ui - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32doc.cpp

!IF  "$(CFG)" == "win32ui - Win32 Release"

# ADD CPP /YX"stdafx.h"

!ELSEIF  "$(CFG)" == "win32ui - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\win32\src\win32dynamicdialog.cpp

!IF  "$(CFG)" == "win32ui - Win32 Release"

# SUBTRACT CPP /YX

!ELSEIF  "$(CFG)" == "win32ui - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32font.cpp

!IF  "$(CFG)" == "win32ui - Win32 Release"

# ADD CPP /YX"stdafx.h"

!ELSEIF  "$(CFG)" == "win32ui - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32gdi.cpp

!IF  "$(CFG)" == "win32ui - Win32 Release"

# ADD CPP /YX"stdafx.h"

!ELSEIF  "$(CFG)" == "win32ui - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32ImageList.cpp

!IF  "$(CFG)" == "win32ui - Win32 Release"

# ADD CPP /YX"stdafx.h"

!ELSEIF  "$(CFG)" == "win32ui - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32menu.cpp

!IF  "$(CFG)" == "win32ui - Win32 Release"

# ADD CPP /YX"stdafx.h"

!ELSEIF  "$(CFG)" == "win32ui - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32notify.cpp

!IF  "$(CFG)" == "win32ui - Win32 Release"

# ADD CPP /YX"stdafx.h"

!ELSEIF  "$(CFG)" == "win32ui - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32pen.cpp

!IF  "$(CFG)" == "win32ui - Win32 Release"

# ADD CPP /YX"stdafx.h"

!ELSEIF  "$(CFG)" == "win32ui - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32prinfo.cpp
# End Source File
# Begin Source File

SOURCE=.\win32prop.cpp

!IF  "$(CFG)" == "win32ui - Win32 Release"

# ADD CPP /YX"stdafx.h"

!ELSEIF  "$(CFG)" == "win32ui - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32rgn.cpp
# End Source File
# Begin Source File

SOURCE=.\win32RichEdit.cpp

!IF  "$(CFG)" == "win32ui - Win32 Release"

# ADD CPP /YX"stdafx.h"

!ELSEIF  "$(CFG)" == "win32ui - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32RichEditDocTemplate.cpp

!IF  "$(CFG)" == "win32ui - Win32 Release"

# ADD CPP /YX"stdafx.h"

!ELSEIF  "$(CFG)" == "win32ui - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32splitter.cpp

!IF  "$(CFG)" == "win32ui - Win32 Release"

# ADD CPP /YX"stdafx.h"

!ELSEIF  "$(CFG)" == "win32ui - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32template.cpp

!IF  "$(CFG)" == "win32ui - Win32 Release"

# ADD CPP /YX"stdafx.h"

!ELSEIF  "$(CFG)" == "win32ui - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32thread.cpp

!IF  "$(CFG)" == "win32ui - Win32 Release"

# ADD CPP /YX"stdafx.h"

!ELSEIF  "$(CFG)" == "win32ui - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32toolbar.cpp

!IF  "$(CFG)" == "win32ui - Win32 Release"

# ADD CPP /YX"stdafx.h"

!ELSEIF  "$(CFG)" == "win32ui - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32tooltip.cpp
# End Source File
# Begin Source File

SOURCE=.\win32ui.rc
# End Source File
# Begin Source File

SOURCE=.\win32uimodule.cpp

!IF  "$(CFG)" == "win32ui - Win32 Release"

# ADD CPP /YX"stdafx.h"

!ELSEIF  "$(CFG)" == "win32ui - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32util.cpp

!IF  "$(CFG)" == "win32ui - Win32 Release"

# ADD CPP /YX"stdafx.h"

!ELSEIF  "$(CFG)" == "win32ui - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32view.cpp

!IF  "$(CFG)" == "win32ui - Win32 Release"

# ADD CPP /YX"stdafx.h"

!ELSEIF  "$(CFG)" == "win32ui - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32virt.cpp

!IF  "$(CFG)" == "win32ui - Win32 Release"

# ADD CPP /YX"stdafx.h"

!ELSEIF  "$(CFG)" == "win32ui - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32win.cpp

!IF  "$(CFG)" == "win32ui - Win32 Release"

# ADD CPP /YX"stdafx.h"

!ELSEIF  "$(CFG)" == "win32ui - Win32 Debug"

!ENDIF 

# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Source File

SOURCE=.\dibapi.h
# End Source File
# Begin Source File

SOURCE=.\pythoncbar.h
# End Source File
# Begin Source File

SOURCE=.\pythondoc.h
# End Source File
# Begin Source File

SOURCE=.\pythonframe.h
# End Source File
# Begin Source File

SOURCE=.\pythonppage.h
# End Source File
# Begin Source File

SOURCE=.\pythonpsheet.h
# End Source File
# Begin Source File

SOURCE=.\pythonRichEdit.h
# End Source File
# Begin Source File

SOURCE=.\pythonRichEditCntr.h
# End Source File
# Begin Source File

SOURCE=.\pythonRichEditDoc.h
# End Source File
# Begin Source File

SOURCE=.\pythonview.h
# End Source File
# Begin Source File

SOURCE=.\pythonwin.h
# End Source File
# Begin Source File

SOURCE=.\stdafx.h
# End Source File
# Begin Source File

SOURCE=.\Win32app.h
# End Source File
# Begin Source File

SOURCE=.\win32assoc.h
# End Source File
# Begin Source File

SOURCE=.\win32bitmap.h
# End Source File
# Begin Source File

SOURCE=.\win32brush.h
# End Source File
# Begin Source File

SOURCE=.\win32cmd.h
# End Source File
# Begin Source File

SOURCE=.\win32cmdui.h
# End Source File
# Begin Source File

SOURCE=.\win32control.h
# End Source File
# Begin Source File

SOURCE=.\win32ctrlList.h
# End Source File
# Begin Source File

SOURCE=.\win32ctrlTree.h
# End Source File
# Begin Source File

SOURCE=.\WIN32DC.H
# End Source File
# Begin Source File

SOURCE=.\win32dlg.h
# End Source File
# Begin Source File

SOURCE=.\win32dlgbar.h
# End Source File
# Begin Source File

SOURCE=.\win32dlgdyn.h
# End Source File
# Begin Source File

SOURCE=.\win32dll.h
# End Source File
# Begin Source File

SOURCE=.\win32doc.h
# End Source File
# Begin Source File

SOURCE=.\win32font.h
# End Source File
# Begin Source File

SOURCE=.\win32gdi.h
# End Source File
# Begin Source File

SOURCE=.\win32hl.h
# End Source File
# Begin Source File

SOURCE=.\win32ImageList.h
# End Source File
# Begin Source File

SOURCE=.\win32menu.h
# End Source File
# Begin Source File

SOURCE=.\win32pen.h
# End Source File
# Begin Source File

SOURCE=.\win32prinfo.h
# End Source File
# Begin Source File

SOURCE=.\win32prop.h
# End Source File
# Begin Source File

SOURCE=.\win32rgn.h
# End Source File
# Begin Source File

SOURCE=.\win32RichEdit.h
# End Source File
# Begin Source File

SOURCE=.\win32RichEditDocTemplate.h
# End Source File
# Begin Source File

SOURCE=.\win32splitter.h
# End Source File
# Begin Source File

SOURCE=.\win32template.h
# End Source File
# Begin Source File

SOURCE=.\win32toolbar.h
# End Source File
# Begin Source File

SOURCE=.\win32ui.h
# End Source File
# Begin Source File

SOURCE=.\win32uiExt.h
# End Source File
# Begin Source File

SOURCE=.\Win32uiHostGlue.h
# End Source File
# Begin Source File

SOURCE=.\win32win.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\BROWSER.BMP
# End Source File
# Begin Source File

SOURCE=.\DEBUGGER.BMP
# End Source File
# Begin Source File

SOURCE=.\res\debugger.ico
# End Source File
# Begin Source File

SOURCE=.\res\debugger_stack.bmp
# End Source File
# Begin Source File

SOURCE=.\res\HIERFOLD.BMP
# End Source File
# Begin Source File

SOURCE=.\ICO00002.ICO
# End Source File
# Begin Source File

SOURCE=.\res\ICO00002.ICO
# End Source File
# Begin Source File

SOURCE=.\res\IDR_MAIN.ICO
# End Source File
# Begin Source File

SOURCE=.\res\IDR_PYTH.ICO
# End Source File
# Begin Source File

SOURCE=.\res\PADDOC.ICO
# End Source File
# Begin Source File

SOURCE=.\res\pyc.ico
# End Source File
# Begin Source File

SOURCE=.\res\pycon.ico
# End Source File
# Begin Source File

SOURCE=.\res\pythonwin.rc2
# End Source File
# Begin Source File

SOURCE=.\res\temp.BMP
# End Source File
# Begin Source File

SOURCE=.\res\toolbar.bmp
# End Source File
# Begin Source File

SOURCE=.\toolbar.bmp
# End Source File
# Begin Source File

SOURCE=.\res\toolbar_debugger.bmp
# End Source File
# End Group
# End Target
# End Project
