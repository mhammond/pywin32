# Microsoft Developer Studio Project File - Name="win32com" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (WCE x86em) Dynamic-Link Library" 0x7f02
# TARGTYPE "Win32 (WCE SH3) Dynamic-Link Library" 0x8102
# TARGTYPE "Win32 (WCE MIPS) Dynamic-Link Library" 0x8202
# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=win32com - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "win32com.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "win32com.mak" CFG="win32com - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "win32com - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "win32com - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "win32com - Win32 (WCE MIPS) Release" (based on "Win32 (WCE MIPS) Dynamic-Link Library")
!MESSAGE "win32com - Win32 (WCE SH3) Release" (based on "Win32 (WCE SH3) Dynamic-Link Library")
!MESSAGE "win32com - Win32 (WCE x86em) Debug" (based on "Win32 (WCE x86em) Dynamic-Link Library")
!MESSAGE "win32com - Win32 (WCE MIPS) Debug" (based on "Win32 (WCE MIPS) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Python/com/win32com", SAAAAAAA"
# PROP Scc_LocalPath "./win32com"

!IF  "$(CFG)" == "win32com - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Build\Release"
# PROP Intermediate_Dir "Build\Temp\win32com\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
F90=df.exe
CPP=cl.exe
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /Zi /O2 /I "win32com\src\include" /I "..\win32\src" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "BUILD_PYTHONCOM" /D "STRICT" /D "_STRICT" /Yu"stdafx.h" /FD /c
MTL=midl.exe
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o /win32 "NUL"
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o /win32 "NUL"
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 ole32.lib oleaut32.lib uuid.lib /nologo /base:"0x1e2a0000" /subsystem:windows /dll /pdb:"Build\System\pythoncom16.pdb" /debug /machine:I386 /def:".\win32com\src\PythonCOM.def" /out:"Build\System\pythoncom16.dll" /implib:"Build\pythoncom.lib" /libpath:"..\win32\build"
# SUBTRACT LINK32 /pdb:none
# Begin Custom Build - copy to system32
ProjDir=.
TargetPath=.\Build\System\pythoncom16.dll
TargetName=pythoncom16
InputPath=.\Build\System\pythoncom16.dll
SOURCE="$(InputPath)"

"$(ProjDir)\$(TargetName).flg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(TargetPath) %SYSTEMROOT%\System32\. && echo Done >                                      $(ProjDir)\$(TargetName).flg

# End Custom Build

!ELSEIF  "$(CFG)" == "win32com - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Build\Debug"
# PROP Intermediate_Dir "Build\Temp\win32com\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
F90=df.exe
CPP=cl.exe
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MDd /W3 /GX /ZI /Od /I "win32com\src\include" /I "..\win32\src" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "BUILD_PYTHONCOM" /D "STRICT" /D "_STRICT" /Yu"stdafx.h" /FD /c
MTL=midl.exe
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o /win32 "NUL"
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o /win32 "NUL"
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 ole32.lib oleaut32.lib uuid.lib /nologo /base:"0x1e2a0000" /subsystem:windows /dll /pdb:"Build\System\pythoncom16_d.pdb" /debug /machine:I386 /def:".\win32com\src\PythonCOM.def" /out:"Build\System\pythoncom16_d.dll" /implib:"Build\pythoncom_d.lib" /pdbtype:sept /libpath:"..\win32\build"
# SUBTRACT LINK32 /pdb:none
# Begin Custom Build - copy to system32
ProjDir=.
TargetPath=.\Build\System\pythoncom16_d.dll
TargetName=pythoncom16_d
InputPath=.\Build\System\pythoncom16_d.dll
SOURCE="$(InputPath)"

"$(ProjDir)\$(TargetName).flg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(TargetPath) %SYSTEMROOT%\System32\. && echo Done >                                      $(ProjDir)\$(TargetName).flg

# End Custom Build

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "win32co0"
# PROP BASE Intermediate_Dir "win32co0"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Build\ce\mips"
# PROP Intermediate_Dir "Build\Temp\win32event\ce\mips\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
LINK32=link.exe
# ADD BASE LINK32 ole32.lib oleaut32.lib uuid.lib commctrl.lib coredll.lib /nologo /dll /pdb:"Build\System\pythoncom15.pdb" /debug /machine:MIPS /def:".\win32com\src\PythonCOM.def" /out:"Build\System\pythoncom15.dll" /implib:"Build\pythoncom.lib"
# SUBTRACT BASE LINK32 /pdb:none /nodefaultlib
# ADD LINK32 ole32.lib oleaut32.lib uuid.lib commctrl.lib coredll.lib winsock.lib /nologo /dll /pdb:none /machine:MIPS /nodefaultlib:"$(CENoDefaultLib)" /def:".\win32com\src\PythonCOM.def" /out:"Build\ce\mips\pythoncom15.dll" /implib:"Build\ce\mips\pythoncom.lib" /subsystem:$(CESubsystem)
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
MTL=midl.exe
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /o /win32 "NUL"
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /o /win32 "NUL"
RSC=rc.exe
# ADD BASE RSC /l 0x409 /r /d "MIPS" /d "_MIPS_" /d UNDER_CE=$(CEVersion) /d _WIN32_WCE=$(CEVersion) /d "$(CEConfigName)" /d "UNICODE" /d "NDEBUG"
# ADD RSC /l 0x409 /r /d "MIPS" /d "_MIPS_" /d UNDER_CE=$(CEVersion) /d _WIN32_WCE=$(CEVersion) /d "$(CEConfigName)" /d "UNICODE" /d "NDEBUG"
CPP=clmips.exe
# ADD BASE CPP /nologo /ML /W3 /Zi /O2 /I "win32com\src\include" /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "NDEBUG" /D "MIPS" /D "_MIPS_" /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "_WINDLL" /D "_MBCS" /D "BUILD_PYTHONCOM" /D "STRICT" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /M$(CECrtMT) /W3 /O1 /I "win32com\src\include" /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "NDEBUG" /D "MIPS" /D "_MIPS_" /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "_WINDLL" /D "BUILD_PYTHONCOM" /D "STRICT" /Yu"stdafx.h" /FD /c
PFILE=pfile.exe
# ADD BASE PFILE COPY
# ADD PFILE COPY

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE SH3) Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "win32co1"
# PROP BASE Intermediate_Dir "win32co1"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Build\ce\sh"
# PROP Intermediate_Dir "Build\Temp\win32event\ce\sh\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
LINK32=link.exe
# ADD BASE LINK32 ole32.lib oleaut32.lib uuid.lib commctrl.lib coredll.lib /nologo /dll /pdb:"Build\System\pythoncom15.pdb" /debug /machine:SH3 /def:".\win32com\src\PythonCOM.def" /out:"Build\System\pythoncom15.dll" /implib:"Build\pythoncom.lib"
# SUBTRACT BASE LINK32 /pdb:none /nodefaultlib
# ADD LINK32 ole32.lib oleaut32.lib uuid.lib commctrl.lib coredll.lib /nologo /dll /pdb:none /machine:SH3 /nodefaultlib:"$(CENoDefaultLib)" /def:".\win32com\src\PythonCOM.def" /out:"Build\ce\sh\pythoncom15.dll" /implib:"Build\ce\sh\pythoncom.lib" /subsystem:$(CESubsystem)
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
MTL=midl.exe
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /o /win32 "NUL"
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /o /win32 "NUL"
RSC=rc.exe
# ADD BASE RSC /l 0x409 /r /d "SHx" /d "SH3" /d "_SH3_" /d UNDER_CE=$(CEVersion) /d _WIN32_WCE=$(CEVersion) /d "$(CEConfigName)" /d "UNICODE" /d "NDEBUG"
# ADD RSC /l 0x409 /r /d "SHx" /d "SH3" /d "_SH3_" /d UNDER_CE=$(CEVersion) /d _WIN32_WCE=$(CEVersion) /d "$(CEConfigName)" /d "UNICODE" /d "NDEBUG"
CPP=shcl.exe
# ADD BASE CPP /nologo /ML /W3 /Zi /O2 /I "win32com\src\include" /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "NDEBUG" /D "SHx" /D "SH3" /D "_SH3_" /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "_WINDLL" /D "_MBCS" /D "BUILD_PYTHONCOM" /D "STRICT" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /M$(CECrtMT) /W3 /O1 /I "win32com\src\include" /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "NDEBUG" /D "SHx" /D "SH3" /D "_SH3_" /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "_WINDLL" /D "_MBCS" /D "BUILD_PYTHONCOM" /D "STRICT" /Yu"stdafx.h" /FD /c
PFILE=pfile.exe
# ADD BASE PFILE COPY
# ADD PFILE COPY

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE x86em) Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "win32co0"
# PROP BASE Intermediate_Dir "win32co0"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Build\ce\x86em"
# PROP Intermediate_Dir "Build\Temp\win32com\ce\x86em\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
LINK32=link.exe
# ADD BASE LINK32 ole32.lib oleaut32.lib uuid.lib commctrl.lib coredll.lib /nologo /base:"0x1e2a0000" /subsystem:windows /dll /pdb:"Build\System\pythoncom15_d.pdb" /debug /machine:IX86 /def:".\win32com\src\PythonCOM.def" /out:"Build\System\pythoncom15_d.dll" /implib:"Build\pythoncom_d.lib" /pdbtype:sept
# SUBTRACT BASE LINK32 /pdb:none /incremental:no
# ADD LINK32 coredll.lib corelibc.lib ole32.lib oleaut32.lib uuid.lib /nologo /base:"0x1e2a0000" /subsystem:windows /dll /debug /machine:IX86 /nodefaultlib:"$(CENoDefaultLib)" /def:".\win32com\src\PythonCOM.def" /out:"Build\ce\x86em\pythoncom15_d.dll" /implib:"Build\pythoncom_d.lib" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
MTL=midl.exe
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /o /win32 "NUL"
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /o /win32 "NUL"
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d UNDER_CE=$(CEVersion) /d "UNICODE" /d "_UNICODE" /d "WIN32" /d "STRICT" /d _WIN32_WCE=$(CEVersion) /d "$(CEConfigName)" /d "_WIN32_WCE_EMULATION" /d "INTERNATIONAL" /d "USA" /d "INTLMSG_CODEPAGE" /d "_DEBUG" /d "x86" /d "i486" /d "_x86_"
# ADD RSC /l 0x409 /d UNDER_CE=$(CEVersion) /d "UNICODE" /d "_UNICODE" /d "WIN32" /d "STRICT" /d _WIN32_WCE=$(CEVersion) /d "$(CEConfigName)" /d "_WIN32_WCE_EMULATION" /d "INTERNATIONAL" /d "USA" /d "INTLMSG_CODEPAGE" /d "_DEBUG" /d "x86" /d "i486" /d "_x86_"
CPP=cl.exe
# ADD BASE CPP /nologo /MLd /W3 /Gm /ZI /Od /I "win32com\src\include" /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "_UNICODE" /D "WIN32" /D "STRICT" /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "_WIN32_WCE_EMULATION" /D "INTERNATIONAL" /D "USA" /D "INTLMSG_CODEPAGE" /D "_DEBUG" /D "x86" /D "i486" /D "_x86_" /D "_WINDLL" /D "_MBCS" /D "BUILD_PYTHONCOM" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MTd /W3 /Gm /Zi /Od /I "win32com\src\include" /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "_UNICODE" /D "WIN32" /D "STRICT" /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "_WIN32_WCE_EMULATION" /D "INTERNATIONAL" /D "USA" /D "INTLMSG_CODEPAGE" /D "_DEBUG" /D "x86" /D "i486" /D "_x86_" /D "_WINDLL" /D "_MBCS" /D "BUILD_PYTHONCOM" /YX"stdafx.h" /FD /c
EMPFILE=empfile.exe
# ADD BASE EMPFILE COPY
# ADD EMPFILE COPY

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "win32co0"
# PROP BASE Intermediate_Dir "win32co0"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Build\ce\mips"
# PROP Intermediate_Dir "Build\Temp\win32com\ce\mips\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
LINK32=link.exe
# ADD BASE LINK32 corelibc.lib ole32.lib oleaut32.lib uuid.lib commctrl.lib coredll.lib /nologo /dll /debug /machine:MIPS /def:".\win32com\src\PythonCOM.def" /out:"Build\ce\x86em\pythoncom15_d.dll" /implib:"Build\pythoncom_d.lib" /pdbtype:sept
# SUBTRACT BASE LINK32 /pdb:none /nodefaultlib
# ADD LINK32 corelibc.lib ole32.lib oleaut32.lib uuid.lib commctrl.lib coredll.lib /nologo /dll /debug /machine:MIPS /nodefaultlib:"$(CENoDefaultLib)" /def:".\win32com\src\PythonCOM.def" /out:"Build\ce\mips\pythoncom15_d.dll" /implib:"Build\ce\mips\pythoncom_d.lib" /pdbtype:sept /subsystem:$(CESubsystem)
# SUBTRACT LINK32 /pdb:none
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
MTL=midl.exe
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /o /win32 "NUL"
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /o /win32 "NUL"
RSC=rc.exe
# ADD BASE RSC /l 0x409 /r /d "MIPS" /d "_MIPS_" /d UNDER_CE=$(CEVersion) /d _WIN32_WCE=$(CEVersion) /d "$(CEConfigName)" /d "UNICODE" /d "DEBUG"
# ADD RSC /l 0x409 /r /d "MIPS" /d "_MIPS_" /d UNDER_CE=$(CEVersion) /d _WIN32_WCE=$(CEVersion) /d "$(CEConfigName)" /d "UNICODE" /d "DEBUG"
CPP=clmips.exe
# ADD BASE CPP /nologo /MLd /W3 /Zi /Od /I "win32com\src\include" /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "DEBUG" /D "MIPS" /D "_MIPS_" /D UNDER_CE=$(CEVersion) /D "UNICODE" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /M$(CECrtMT) /W3 /Zi /Od /I "win32com\src\include" /D "BUILD_PYTHONCOM" /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "DEBUG" /D "MIPS" /D "_MIPS_" /D UNDER_CE=$(CEVersion) /D "UNICODE" /Yu"stdafx.h" /FD /c
PFILE=pfile.exe
# ADD BASE PFILE COPY
# ADD PFILE COPY

!ENDIF 

# Begin Target

# Name "win32com - Win32 Release"
# Name "win32com - Win32 Debug"
# Name "win32com - Win32 (WCE MIPS) Release"
# Name "win32com - Win32 (WCE SH3) Release"
# Name "win32com - Win32 (WCE x86em) Debug"
# Name "win32com - Win32 (WCE MIPS) Debug"
# Begin Group "Source Files"

# PROP Default_Filter ""
# Begin Group "PythonCOM"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\win32com\src\dllmain.cpp

!IF  "$(CFG)" == "win32com - Win32 Release"

!ELSEIF  "$(CFG)" == "win32com - Win32 Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Release"

DEP_CPP_DLLMA=\
	".\win32com\src\include\PyFactory.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_DLLMA=\
	".\angeobject.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\ethodobject.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\rtapi.h"\
	".\tringobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\val.h"\
	".\xtapi.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ys\stat.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE SH3) Release"

DEP_CPP_DLLMA=\
	".\win32com\src\include\PyFactory.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_DLLMA=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
	".\eval.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\nicodeobject.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\object.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\tringobject.h"\
	".\ufferobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE x86em) Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Debug"

DEP_CPP_DLLMA=\
	".\win32com\src\include\PyFactory.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32com\src\ErrorUtils.cpp

!IF  "$(CFG)" == "win32com - Win32 Release"

!ELSEIF  "$(CFG)" == "win32com - Win32 Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Release"

DEP_CPP_ERROR=\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_ERROR=\
	".\angeobject.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\ethodobject.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\rtapi.h"\
	".\tringobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\val.h"\
	".\xtapi.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ys\stat.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE SH3) Release"

DEP_CPP_ERROR=\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_ERROR=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
	".\eval.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\nicodeobject.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\object.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\tringobject.h"\
	".\ufferobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE x86em) Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Debug"

DEP_CPP_ERROR=\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\stdafx.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32com\src\MiscTypes.cpp

!IF  "$(CFG)" == "win32com - Win32 Release"

!ELSEIF  "$(CFG)" == "win32com - Win32 Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Release"

DEP_CPP_MISCT=\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_MISCT=\
	".\angeobject.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\ethodobject.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\rtapi.h"\
	".\tringobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\val.h"\
	".\xtapi.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ys\stat.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE SH3) Release"

DEP_CPP_MISCT=\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_MISCT=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
	".\eval.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\nicodeobject.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\object.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\tringobject.h"\
	".\ufferobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE x86em) Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Debug"

DEP_CPP_MISCT=\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\stdafx.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32com\src\oleargs.cpp

!IF  "$(CFG)" == "win32com - Win32 Release"

!ELSEIF  "$(CFG)" == "win32com - Win32 Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Release"

DEP_CPP_OLEAR=\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_OLEAR=\
	".\angeobject.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\ethodobject.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\rtapi.h"\
	".\tringobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\val.h"\
	".\xtapi.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ys\stat.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE SH3) Release"

DEP_CPP_OLEAR=\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_OLEAR=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
	".\eval.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\nicodeobject.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\object.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\tringobject.h"\
	".\ufferobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE x86em) Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Debug"

DEP_CPP_OLEAR=\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\stdafx.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32com\src\PyComHelpers.cpp

!IF  "$(CFG)" == "win32com - Win32 Release"

!ELSEIF  "$(CFG)" == "win32com - Win32 Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Release"

DEP_CPP_PYCOM=\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYCOM=\
	".\angeobject.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\ethodobject.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\rtapi.h"\
	".\tringobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\val.h"\
	".\xtapi.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ys\stat.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinObjects.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE SH3) Release"

DEP_CPP_PYCOM=\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYCOM=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
	".\eval.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\nicodeobject.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\object.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\tringobject.h"\
	".\ufferobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinObjects.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE x86em) Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Debug"

DEP_CPP_PYCOM=\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYCOM=\
	".\win32com\src\PyWinObjects.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32com\src\PyFactory.cpp

!IF  "$(CFG)" == "win32com - Win32 Release"

!ELSEIF  "$(CFG)" == "win32com - Win32 Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Release"

DEP_CPP_PYFAC=\
	".\win32com\src\include\PyFactory.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYFAC=\
	".\angeobject.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\ethodobject.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\rtapi.h"\
	".\tringobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\val.h"\
	".\xtapi.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ys\stat.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE SH3) Release"

DEP_CPP_PYFAC=\
	".\win32com\src\include\PyFactory.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYFAC=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
	".\eval.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\nicodeobject.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\object.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\tringobject.h"\
	".\ufferobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE x86em) Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Debug"

DEP_CPP_PYFAC=\
	".\win32com\src\include\PyFactory.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32com\src\PyGatewayBase.cpp

!IF  "$(CFG)" == "win32com - Win32 Release"

!ELSEIF  "$(CFG)" == "win32com - Win32 Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Release"

DEP_CPP_PYGAT=\
	".\win32com\src\include\PyFactory.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYGAT=\
	".\angeobject.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\ethodobject.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\rtapi.h"\
	".\tringobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\val.h"\
	".\xtapi.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ys\stat.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE SH3) Release"

DEP_CPP_PYGAT=\
	".\win32com\src\include\PyFactory.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYGAT=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
	".\eval.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\nicodeobject.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\object.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\tringobject.h"\
	".\ufferobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE x86em) Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Debug"

DEP_CPP_PYGAT=\
	".\win32com\src\include\PyFactory.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32com\src\PyIBase.cpp

!IF  "$(CFG)" == "win32com - Win32 Release"

!ELSEIF  "$(CFG)" == "win32com - Win32 Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Release"

DEP_CPP_PYIBA=\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYIBA=\
	".\angeobject.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\ethodobject.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\rtapi.h"\
	".\tringobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\val.h"\
	".\xtapi.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ys\stat.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE SH3) Release"

DEP_CPP_PYIBA=\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYIBA=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
	".\eval.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\nicodeobject.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\object.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\tringobject.h"\
	".\ufferobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE x86em) Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Debug"

DEP_CPP_PYIBA=\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\stdafx.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32com\src\PyIClassFactory.cpp

!IF  "$(CFG)" == "win32com - Win32 Release"

!ELSEIF  "$(CFG)" == "win32com - Win32 Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Release"

DEP_CPP_PYICL=\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYICL=\
	".\angeobject.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\ethodobject.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\rtapi.h"\
	".\tringobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\val.h"\
	".\xtapi.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ys\stat.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE SH3) Release"

DEP_CPP_PYICL=\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYICL=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
	".\eval.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\nicodeobject.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\object.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\tringobject.h"\
	".\ufferobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE x86em) Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Debug"

DEP_CPP_PYICL=\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32com\src\PyIDispatch.cpp

!IF  "$(CFG)" == "win32com - Win32 Release"

!ELSEIF  "$(CFG)" == "win32com - Win32 Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Release"

DEP_CPP_PYIDI=\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYIDI=\
	".\angeobject.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\ethodobject.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\rtapi.h"\
	".\tringobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\val.h"\
	".\xtapi.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ys\stat.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE SH3) Release"

DEP_CPP_PYIDI=\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYIDI=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
	".\eval.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\nicodeobject.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\object.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\tringobject.h"\
	".\ufferobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE x86em) Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Debug"

DEP_CPP_PYIDI=\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\stdafx.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32com\src\PyIUnknown.cpp

!IF  "$(CFG)" == "win32com - Win32 Release"

!ELSEIF  "$(CFG)" == "win32com - Win32 Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Release"

DEP_CPP_PYIUN=\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYIUN=\
	".\angeobject.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\ethodobject.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\rtapi.h"\
	".\tringobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\val.h"\
	".\xtapi.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ys\stat.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE SH3) Release"

DEP_CPP_PYIUN=\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYIUN=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
	".\eval.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\nicodeobject.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\object.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\tringobject.h"\
	".\ufferobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE x86em) Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Debug"

DEP_CPP_PYIUN=\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32com\src\PyRecord.cpp

!IF  "$(CFG)" == "win32com - Win32 Release"

!ELSEIF  "$(CFG)" == "win32com - Win32 Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Release"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE SH3) Release"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE x86em) Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32com\src\PyStorage.cpp

!IF  "$(CFG)" == "win32com - Win32 Release"

!ELSEIF  "$(CFG)" == "win32com - Win32 Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Release"

DEP_CPP_PYSTO=\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYSTO=\
	".\angeobject.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\ethodobject.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\rtapi.h"\
	".\tringobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\val.h"\
	".\xtapi.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ys\stat.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE SH3) Release"

DEP_CPP_PYSTO=\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYSTO=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
	".\eval.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\nicodeobject.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\object.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\tringobject.h"\
	".\ufferobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE x86em) Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Debug"

DEP_CPP_PYSTO=\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32com\src\PythonCOM.cpp

!IF  "$(CFG)" == "win32com - Win32 Release"

!ELSEIF  "$(CFG)" == "win32com - Win32 Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Release"

DEP_CPP_PYTHO=\
	".\win32com\src\include\PyFactory.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYTHO=\
	".\angeobject.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\ethodobject.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\rtapi.h"\
	".\tringobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\val.h"\
	".\xtapi.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ys\stat.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE SH3) Release"

DEP_CPP_PYTHO=\
	".\win32com\src\include\PyFactory.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYTHO=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
	".\eval.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\nicodeobject.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\object.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\tringobject.h"\
	".\ufferobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE x86em) Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Debug"

DEP_CPP_PYTHO=\
	".\win32com\src\include\PyFactory.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32com\src\Register.cpp

!IF  "$(CFG)" == "win32com - Win32 Release"

!ELSEIF  "$(CFG)" == "win32com - Win32 Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Release"

DEP_CPP_REGIS=\
	".\win32com\src\include\propbag.h"\
	".\win32com\src\include\PyGConnectionPoint.h"\
	".\win32com\src\include\PyGConnectionPointContainer.h"\
	".\win32com\src\include\PyGPersistStorage.h"\
	".\win32com\src\include\PyIBindCtx.h"\
	".\win32com\src\include\PyICatInformation.h"\
	".\win32com\src\include\PyICatRegister.h"\
	".\win32com\src\include\PyIEnumGUID.h"\
	".\win32com\src\include\PyIEnumSTATSTG.h"\
	".\win32com\src\include\PyIEnumVARIANT.h"\
	".\win32com\src\include\PyIExternalConnection.h"\
	".\win32com\src\include\PyILockBytes.h"\
	".\win32com\src\include\PyIMoniker.h"\
	".\win32com\src\include\PyIPersist.h"\
	".\win32com\src\include\PyIPersistFile.h"\
	".\win32com\src\include\PyIPersistStorage.h"\
	".\win32com\src\include\PyIPersistStream.h"\
	".\win32com\src\include\PyIPersistStreamInit.h"\
	".\win32com\src\include\PyIRunningObjectTable.h"\
	".\win32com\src\include\PyIServiceProvider.h"\
	".\win32com\src\include\PyIStorage.h"\
	".\win32com\src\include\PyIStream.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMRegister.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_REGIS=\
	".\angeobject.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\ethodobject.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\rtapi.h"\
	".\tringobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\val.h"\
	".\xtapi.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\yIEnumConnections.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ys\stat.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE SH3) Release"

DEP_CPP_REGIS=\
	".\win32com\src\include\propbag.h"\
	".\win32com\src\include\PyGConnectionPoint.h"\
	".\win32com\src\include\PyGConnectionPointContainer.h"\
	".\win32com\src\include\PyGPersistStorage.h"\
	".\win32com\src\include\PyIBindCtx.h"\
	".\win32com\src\include\PyICatInformation.h"\
	".\win32com\src\include\PyICatRegister.h"\
	".\win32com\src\include\pyicreatetypeinfo.h"\
	".\win32com\src\include\pyicreatetypelib.h"\
	".\win32com\src\include\PyIEnumGUID.h"\
	".\win32com\src\include\pyienumstatpropstg.h"\
	".\win32com\src\include\PyIEnumSTATSTG.h"\
	".\win32com\src\include\PyIEnumVARIANT.h"\
	".\win32com\src\include\PyIExternalConnection.h"\
	".\win32com\src\include\PyILockBytes.h"\
	".\win32com\src\include\PyIMoniker.h"\
	".\win32com\src\include\PyIPersist.h"\
	".\win32com\src\include\PyIPersistFile.h"\
	".\win32com\src\include\PyIPersistStorage.h"\
	".\win32com\src\include\PyIPersistStream.h"\
	".\win32com\src\include\PyIPersistStreamInit.h"\
	".\win32com\src\include\pyipropertysetstorage.h"\
	".\win32com\src\include\pyipropertystorage.h"\
	".\win32com\src\include\PyIRunningObjectTable.h"\
	".\win32com\src\include\PyIServiceProvider.h"\
	".\win32com\src\include\PyIStorage.h"\
	".\win32com\src\include\PyIStream.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMRegister.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_REGIS=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
	".\eval.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\nicodeobject.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\object.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\tringobject.h"\
	".\ufferobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\yIEnumConnections.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE x86em) Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Debug"

DEP_CPP_REGIS=\
	".\win32com\src\include\propbag.h"\
	".\win32com\src\include\PyGConnectionPoint.h"\
	".\win32com\src\include\PyGConnectionPointContainer.h"\
	".\win32com\src\include\PyGPersistStorage.h"\
	".\win32com\src\include\PyIBindCtx.h"\
	".\win32com\src\include\PyICatInformation.h"\
	".\win32com\src\include\PyICatRegister.h"\
	".\win32com\src\include\PyIEnumGUID.h"\
	".\win32com\src\include\PyIEnumSTATSTG.h"\
	".\win32com\src\include\PyIEnumVARIANT.h"\
	".\win32com\src\include\PyIExternalConnection.h"\
	".\win32com\src\include\PyILockBytes.h"\
	".\win32com\src\include\PyIMoniker.h"\
	".\win32com\src\include\PyIPersist.h"\
	".\win32com\src\include\PyIPersistFile.h"\
	".\win32com\src\include\PyIPersistStorage.h"\
	".\win32com\src\include\PyIPersistStream.h"\
	".\win32com\src\include\PyIPersistStreamInit.h"\
	".\win32com\src\include\PyIRunningObjectTable.h"\
	".\win32com\src\include\PyIServiceProvider.h"\
	".\win32com\src\include\PyIStorage.h"\
	".\win32com\src\include\PyIStream.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMRegister.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_REGIS=\
	".\yIEnumConnections.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32com\src\stdafx.cpp

!IF  "$(CFG)" == "win32com - Win32 Release"

# ADD CPP /Yc"stdafx.h"

!ELSEIF  "$(CFG)" == "win32com - Win32 Debug"

# ADD CPP /Yc"stdafx.h"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Release"

DEP_CPP_STDAF=\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_STDAF=\
	".\angeobject.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\ethodobject.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\rtapi.h"\
	".\tringobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\val.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ys\stat.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	
# ADD BASE CPP /Yc"stdafx.h"
# ADD CPP /Yc"stdafx.h"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE SH3) Release"

DEP_CPP_STDAF=\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_STDAF=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
	".\eval.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\object.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\tringobject.h"\
	".\ufferobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE x86em) Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Debug"

DEP_CPP_STDAF=\
	".\win32com\src\include\stdafx.h"\
	
# ADD BASE CPP /Yc"stdafx.h"
# ADD CPP /Yc"stdafx.h"

!ENDIF 

# End Source File
# End Group
# Begin Group "Extensions"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\win32com\src\extensions\PyFUNCDESC.cpp

!IF  "$(CFG)" == "win32com - Win32 Release"

!ELSEIF  "$(CFG)" == "win32com - Win32 Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Release"

DEP_CPP_PYFUN=\
	".\win32com\src\include\PyComTypeObjects.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYFUN=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
	".\eval.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\nicodeobject.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\object.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\tringobject.h"\
	".\tructmember.h"\
	".\ufferobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE SH3) Release"

DEP_CPP_PYFUN=\
	".\win32com\src\include\PyComTypeObjects.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYFUN=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
	".\eval.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\nicodeobject.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\object.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\tringobject.h"\
	".\tructmember.h"\
	".\ufferobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE x86em) Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Debug"

DEP_CPP_PYFUN=\
	".\win32com\src\include\PyComTypeObjects.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYFUN=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
	".\eval.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\nicodeobject.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\object.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\tringobject.h"\
	".\tructmember.h"\
	".\ufferobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyGConnectionPoint.cpp

!IF  "$(CFG)" == "win32com - Win32 Release"

!ELSEIF  "$(CFG)" == "win32com - Win32 Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Release"

DEP_CPP_PYGCO=\
	".\win32com\src\include\PyGConnectionPoint.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYGCO=\
	".\angeobject.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\ethodobject.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\rtapi.h"\
	".\tringobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\val.h"\
	".\xtapi.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ys\stat.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE SH3) Release"

DEP_CPP_PYGCO=\
	".\win32com\src\include\PyGConnectionPoint.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYGCO=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
	".\eval.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\nicodeobject.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\object.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\tringobject.h"\
	".\ufferobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE x86em) Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Debug"

DEP_CPP_PYGCO=\
	".\win32com\src\include\PyGConnectionPoint.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyGConnectionPointContainer.cpp

!IF  "$(CFG)" == "win32com - Win32 Release"

!ELSEIF  "$(CFG)" == "win32com - Win32 Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Release"

DEP_CPP_PYGCON=\
	".\win32com\src\include\PyGConnectionPointContainer.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYGCON=\
	".\angeobject.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\ethodobject.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\rtapi.h"\
	".\tringobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\val.h"\
	".\xtapi.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ys\stat.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE SH3) Release"

DEP_CPP_PYGCON=\
	".\win32com\src\include\PyGConnectionPointContainer.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYGCON=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
	".\eval.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\nicodeobject.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\object.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\tringobject.h"\
	".\ufferobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE x86em) Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Debug"

DEP_CPP_PYGCON=\
	".\win32com\src\include\PyGConnectionPointContainer.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyGEnumVariant.cpp

!IF  "$(CFG)" == "win32com - Win32 Release"

!ELSEIF  "$(CFG)" == "win32com - Win32 Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Release"

DEP_CPP_PYGEN=\
	".\win32com\src\include\PyIEnumVARIANT.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYGEN=\
	".\angeobject.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\ethodobject.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\rtapi.h"\
	".\tringobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\val.h"\
	".\xtapi.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ys\stat.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE SH3) Release"

DEP_CPP_PYGEN=\
	".\win32com\src\include\PyIEnumVARIANT.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYGEN=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
	".\eval.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\nicodeobject.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\object.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\tringobject.h"\
	".\ufferobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE x86em) Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Debug"

DEP_CPP_PYGEN=\
	".\win32com\src\include\PyIEnumVARIANT.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyGErrorLog.cpp

!IF  "$(CFG)" == "win32com - Win32 Release"

!ELSEIF  "$(CFG)" == "win32com - Win32 Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Release"

DEP_CPP_PYGER=\
	".\win32com\src\include\propbag.h"\
	".\win32com\src\include\PyIPersist.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYGER=\
	".\angeobject.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\ethodobject.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\rtapi.h"\
	".\tringobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\val.h"\
	".\xtapi.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ys\stat.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE SH3) Release"

DEP_CPP_PYGER=\
	".\win32com\src\include\propbag.h"\
	".\win32com\src\include\PyIPersist.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYGER=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
	".\eval.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\nicodeobject.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\object.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\tringobject.h"\
	".\ufferobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE x86em) Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Debug"

DEP_CPP_PYGER=\
	".\win32com\src\include\propbag.h"\
	".\win32com\src\include\PyIPersist.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyGPersist.cpp

!IF  "$(CFG)" == "win32com - Win32 Release"

!ELSEIF  "$(CFG)" == "win32com - Win32 Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Release"

DEP_CPP_PYGPE=\
	".\win32com\src\include\propbag.h"\
	".\win32com\src\include\PyIPersist.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYGPE=\
	".\angeobject.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\ethodobject.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\rtapi.h"\
	".\tringobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\val.h"\
	".\xtapi.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ys\stat.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE SH3) Release"

DEP_CPP_PYGPE=\
	".\win32com\src\include\propbag.h"\
	".\win32com\src\include\PyIPersist.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYGPE=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
	".\eval.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\nicodeobject.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\object.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\tringobject.h"\
	".\ufferobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE x86em) Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Debug"

DEP_CPP_PYGPE=\
	".\win32com\src\include\propbag.h"\
	".\win32com\src\include\PyIPersist.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyGPersistPropertyBag.cpp

!IF  "$(CFG)" == "win32com - Win32 Release"

!ELSEIF  "$(CFG)" == "win32com - Win32 Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Release"

DEP_CPP_PYGPER=\
	".\win32com\src\include\propbag.h"\
	".\win32com\src\include\PyIPersist.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYGPER=\
	".\angeobject.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\ethodobject.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\rtapi.h"\
	".\tringobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\val.h"\
	".\xtapi.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ys\stat.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE SH3) Release"

DEP_CPP_PYGPER=\
	".\win32com\src\include\propbag.h"\
	".\win32com\src\include\PyIPersist.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYGPER=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
	".\eval.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\nicodeobject.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\object.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\tringobject.h"\
	".\ufferobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE x86em) Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Debug"

DEP_CPP_PYGPER=\
	".\win32com\src\include\propbag.h"\
	".\win32com\src\include\PyIPersist.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyGPersistStorage.cpp

!IF  "$(CFG)" == "win32com - Win32 Release"

!ELSEIF  "$(CFG)" == "win32com - Win32 Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Release"

DEP_CPP_PYGPERS=\
	".\win32com\src\include\PyGPersistStorage.h"\
	".\win32com\src\include\PyIPersist.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYGPERS=\
	".\angeobject.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\ethodobject.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\rtapi.h"\
	".\tringobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\val.h"\
	".\xtapi.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ys\stat.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE SH3) Release"

DEP_CPP_PYGPERS=\
	".\win32com\src\include\PyGPersistStorage.h"\
	".\win32com\src\include\PyIPersist.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYGPERS=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
	".\eval.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\nicodeobject.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\object.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\tringobject.h"\
	".\ufferobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE x86em) Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Debug"

DEP_CPP_PYGPERS=\
	".\win32com\src\include\PyGPersistStorage.h"\
	".\win32com\src\include\PyIPersist.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyGPersistStream.cpp

!IF  "$(CFG)" == "win32com - Win32 Release"

!ELSEIF  "$(CFG)" == "win32com - Win32 Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Release"

DEP_CPP_PYGPERSI=\
	".\win32com\src\include\PyIPersist.h"\
	".\win32com\src\include\PyIPersistStream.h"\
	".\win32com\src\include\PyIStream.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYGPERSI=\
	".\angeobject.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\ethodobject.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\rtapi.h"\
	".\tringobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\val.h"\
	".\xtapi.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ys\stat.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE SH3) Release"

DEP_CPP_PYGPERSI=\
	".\win32com\src\include\PyIPersist.h"\
	".\win32com\src\include\PyIPersistStream.h"\
	".\win32com\src\include\PyIStream.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYGPERSI=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
	".\eval.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\nicodeobject.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\object.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\tringobject.h"\
	".\ufferobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE x86em) Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Debug"

DEP_CPP_PYGPERSI=\
	".\win32com\src\include\PyIPersist.h"\
	".\win32com\src\include\PyIPersistStream.h"\
	".\win32com\src\include\PyIStream.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyGPersistStreamInit.cpp

!IF  "$(CFG)" == "win32com - Win32 Release"

!ELSEIF  "$(CFG)" == "win32com - Win32 Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Release"

DEP_CPP_PYGPERSIS=\
	".\win32com\src\include\PyIPersist.h"\
	".\win32com\src\include\PyIPersistStream.h"\
	".\win32com\src\include\PyIPersistStreamInit.h"\
	".\win32com\src\include\PyIStream.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYGPERSIS=\
	".\angeobject.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\ethodobject.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\rtapi.h"\
	".\tringobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\val.h"\
	".\xtapi.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ys\stat.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE SH3) Release"

DEP_CPP_PYGPERSIS=\
	".\win32com\src\include\PyIPersist.h"\
	".\win32com\src\include\PyIPersistStream.h"\
	".\win32com\src\include\PyIPersistStreamInit.h"\
	".\win32com\src\include\PyIStream.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYGPERSIS=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
	".\eval.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\nicodeobject.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\object.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\tringobject.h"\
	".\ufferobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE x86em) Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Debug"

DEP_CPP_PYGPERSIS=\
	".\win32com\src\include\PyIPersist.h"\
	".\win32com\src\include\PyIPersistStream.h"\
	".\win32com\src\include\PyIPersistStreamInit.h"\
	".\win32com\src\include\PyIStream.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyGPropertyBag.cpp

!IF  "$(CFG)" == "win32com - Win32 Release"

!ELSEIF  "$(CFG)" == "win32com - Win32 Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Release"

DEP_CPP_PYGPR=\
	".\win32com\src\include\propbag.h"\
	".\win32com\src\include\PyIPersist.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYGPR=\
	".\angeobject.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\ethodobject.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\rtapi.h"\
	".\tringobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\val.h"\
	".\xtapi.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ys\stat.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE SH3) Release"

DEP_CPP_PYGPR=\
	".\win32com\src\include\propbag.h"\
	".\win32com\src\include\PyIPersist.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYGPR=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
	".\eval.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\nicodeobject.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\object.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\tringobject.h"\
	".\ufferobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE x86em) Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Debug"

DEP_CPP_PYGPR=\
	".\win32com\src\include\propbag.h"\
	".\win32com\src\include\PyIPersist.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyGStream.cpp

!IF  "$(CFG)" == "win32com - Win32 Release"

!ELSEIF  "$(CFG)" == "win32com - Win32 Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Release"

DEP_CPP_PYGST=\
	".\win32com\src\include\PyIStream.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYGST=\
	".\angeobject.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\ethodobject.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\rtapi.h"\
	".\tringobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\val.h"\
	".\xtapi.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ys\stat.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE SH3) Release"

DEP_CPP_PYGST=\
	".\win32com\src\include\PyIStream.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYGST=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
	".\eval.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\nicodeobject.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\object.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\tringobject.h"\
	".\ufferobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE x86em) Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Debug"

DEP_CPP_PYGST=\
	".\win32com\src\include\PyIStream.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyIBindCtx.cpp

!IF  "$(CFG)" == "win32com - Win32 Release"

!ELSEIF  "$(CFG)" == "win32com - Win32 Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Release"

DEP_CPP_PYIBI=\
	".\win32com\src\include\PyIBindCtx.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYIBI=\
	".\angeobject.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\ethodobject.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\rtapi.h"\
	".\tringobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\val.h"\
	".\xtapi.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ys\stat.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE SH3) Release"

DEP_CPP_PYIBI=\
	".\win32com\src\include\PyIBindCtx.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYIBI=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
	".\eval.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\nicodeobject.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\object.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\tringobject.h"\
	".\ufferobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE x86em) Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Debug"

DEP_CPP_PYIBI=\
	".\win32com\src\include\PyIBindCtx.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\stdafx.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyICatInformation.cpp

!IF  "$(CFG)" == "win32com - Win32 Release"

!ELSEIF  "$(CFG)" == "win32com - Win32 Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Release"

DEP_CPP_PYICA=\
	".\win32com\src\include\PyICatInformation.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYICA=\
	".\angeobject.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\ethodobject.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\rtapi.h"\
	".\tringobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\val.h"\
	".\xtapi.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ys\stat.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE SH3) Release"

DEP_CPP_PYICA=\
	".\win32com\src\include\PyICatInformation.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYICA=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
	".\eval.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\nicodeobject.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\object.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\tringobject.h"\
	".\ufferobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE x86em) Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Debug"

DEP_CPP_PYICA=\
	".\win32com\src\include\PyICatInformation.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\stdafx.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyICatRegister.cpp

!IF  "$(CFG)" == "win32com - Win32 Release"

!ELSEIF  "$(CFG)" == "win32com - Win32 Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Release"

DEP_CPP_PYICAT=\
	".\win32com\src\include\PyICatRegister.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYICAT=\
	".\angeobject.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\ethodobject.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\rtapi.h"\
	".\tringobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\val.h"\
	".\xtapi.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ys\stat.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE SH3) Release"

DEP_CPP_PYICAT=\
	".\win32com\src\include\PyICatRegister.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYICAT=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
	".\eval.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\nicodeobject.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\object.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\tringobject.h"\
	".\ufferobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE x86em) Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Debug"

DEP_CPP_PYICAT=\
	".\win32com\src\include\PyICatRegister.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\stdafx.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyIConnectionPoint.cpp

!IF  "$(CFG)" == "win32com - Win32 Release"

!ELSEIF  "$(CFG)" == "win32com - Win32 Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Release"

DEP_CPP_PYICO=\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYICO=\
	".\angeobject.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\ethodobject.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\rtapi.h"\
	".\tringobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\val.h"\
	".\xtapi.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ys\stat.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE SH3) Release"

DEP_CPP_PYICO=\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYICO=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
	".\eval.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\nicodeobject.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\object.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\tringobject.h"\
	".\ufferobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE x86em) Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Debug"

DEP_CPP_PYICO=\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\stdafx.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyIConnectionPointContainer.cpp

!IF  "$(CFG)" == "win32com - Win32 Release"

!ELSEIF  "$(CFG)" == "win32com - Win32 Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Release"

DEP_CPP_PYICON=\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYICON=\
	".\angeobject.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\ethodobject.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\rtapi.h"\
	".\tringobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\val.h"\
	".\xtapi.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ys\stat.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE SH3) Release"

DEP_CPP_PYICON=\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYICON=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
	".\eval.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\nicodeobject.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\object.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\tringobject.h"\
	".\ufferobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE x86em) Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Debug"

DEP_CPP_PYICON=\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\stdafx.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyICreateTypeInfo.cpp

!IF  "$(CFG)" == "win32com - Win32 Release"

!ELSEIF  "$(CFG)" == "win32com - Win32 Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Release"

DEP_CPP_PYICR=\
	".\win32com\src\include\PyComTypeObjects.h"\
	".\win32com\src\include\pyicreatetypeinfo.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYICR=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
	".\eval.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\nicodeobject.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\object.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\tringobject.h"\
	".\ufferobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE SH3) Release"

DEP_CPP_PYICR=\
	".\win32com\src\include\PyComTypeObjects.h"\
	".\win32com\src\include\pyicreatetypeinfo.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYICR=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
	".\eval.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\nicodeobject.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\object.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\tringobject.h"\
	".\ufferobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE x86em) Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Debug"

DEP_CPP_PYICR=\
	".\win32com\src\include\PyComTypeObjects.h"\
	".\win32com\src\include\pyicreatetypeinfo.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYICR=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
	".\eval.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\nicodeobject.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\object.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\tringobject.h"\
	".\ufferobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyICreateTypeLib.cpp

!IF  "$(CFG)" == "win32com - Win32 Release"

!ELSEIF  "$(CFG)" == "win32com - Win32 Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Release"

DEP_CPP_PYICRE=\
	".\win32com\src\include\pyicreatetypelib.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYICRE=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
	".\eval.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\nicodeobject.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\object.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\tringobject.h"\
	".\ufferobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE SH3) Release"

DEP_CPP_PYICRE=\
	".\win32com\src\include\pyicreatetypelib.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYICRE=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
	".\eval.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\nicodeobject.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\object.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\tringobject.h"\
	".\ufferobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE x86em) Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Debug"

DEP_CPP_PYICRE=\
	".\win32com\src\include\pyicreatetypelib.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYICRE=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
	".\eval.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\nicodeobject.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\object.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\tringobject.h"\
	".\ufferobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyIEnumCATEGORYINFO.cpp

!IF  "$(CFG)" == "win32com - Win32 Release"

!ELSEIF  "$(CFG)" == "win32com - Win32 Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Release"

DEP_CPP_PYIEN=\
	".\win32com\src\include\PyIEnumGUID.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYIEN=\
	".\angeobject.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\ethodobject.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\rtapi.h"\
	".\tringobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\val.h"\
	".\xtapi.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ys\stat.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE SH3) Release"

DEP_CPP_PYIEN=\
	".\win32com\src\include\PyIEnumGUID.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYIEN=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
	".\eval.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\nicodeobject.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\object.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\tringobject.h"\
	".\ufferobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE x86em) Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Debug"

DEP_CPP_PYIEN=\
	".\win32com\src\include\PyIEnumGUID.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\stdafx.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyIEnumConnectionPoints.cpp

!IF  "$(CFG)" == "win32com - Win32 Release"

!ELSEIF  "$(CFG)" == "win32com - Win32 Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Release"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE SH3) Release"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE x86em) Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyIEnumConnections.cpp

!IF  "$(CFG)" == "win32com - Win32 Release"

!ELSEIF  "$(CFG)" == "win32com - Win32 Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Release"

DEP_CPP_PYIENU=\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYIENU=\
	".\angeobject.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\ethodobject.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\rtapi.h"\
	".\tringobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\val.h"\
	".\xtapi.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\yIEnumConnections.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ys\stat.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE SH3) Release"

DEP_CPP_PYIENU=\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYIENU=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
	".\eval.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\nicodeobject.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\object.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\tringobject.h"\
	".\ufferobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\yIEnumConnections.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE x86em) Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Debug"

DEP_CPP_PYIENU=\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYIENU=\
	".\yIEnumConnections.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyIEnumGUID.cpp

!IF  "$(CFG)" == "win32com - Win32 Release"

!ELSEIF  "$(CFG)" == "win32com - Win32 Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Release"

DEP_CPP_PYIENUM=\
	".\win32com\src\include\PyIEnumGUID.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYIENUM=\
	".\angeobject.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\ethodobject.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\rtapi.h"\
	".\tringobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\val.h"\
	".\xtapi.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ys\stat.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE SH3) Release"

DEP_CPP_PYIENUM=\
	".\win32com\src\include\PyIEnumGUID.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYIENUM=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
	".\eval.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\nicodeobject.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\object.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\tringobject.h"\
	".\ufferobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE x86em) Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Debug"

DEP_CPP_PYIENUM=\
	".\win32com\src\include\PyIEnumGUID.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\stdafx.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyIEnumSTATPROPSTG.cpp

!IF  "$(CFG)" == "win32com - Win32 Release"

!ELSEIF  "$(CFG)" == "win32com - Win32 Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Release"

DEP_CPP_PYIENUMS=\
	".\win32com\src\include\pyienumstatpropstg.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYIENUMS=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
	".\eval.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\nicodeobject.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\object.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\tringobject.h"\
	".\ufferobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE SH3) Release"

DEP_CPP_PYIENUMS=\
	".\win32com\src\include\pyienumstatpropstg.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYIENUMS=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
	".\eval.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\nicodeobject.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\object.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\tringobject.h"\
	".\ufferobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE x86em) Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Debug"

DEP_CPP_PYIENUMS=\
	".\win32com\src\include\pyienumstatpropstg.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYIENUMS=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
	".\eval.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\nicodeobject.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\object.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\tringobject.h"\
	".\ufferobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyIEnumSTATSTG.cpp

!IF  "$(CFG)" == "win32com - Win32 Release"

!ELSEIF  "$(CFG)" == "win32com - Win32 Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Release"

DEP_CPP_PYIENUMST=\
	".\win32com\src\include\PyIEnumSTATSTG.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYIENUMST=\
	".\angeobject.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\ethodobject.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\rtapi.h"\
	".\tringobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\val.h"\
	".\xtapi.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ys\stat.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE SH3) Release"

DEP_CPP_PYIENUMST=\
	".\win32com\src\include\PyIEnumSTATSTG.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYIENUMST=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
	".\eval.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\nicodeobject.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\object.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\tringobject.h"\
	".\ufferobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE x86em) Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Debug"

DEP_CPP_PYIENUMST=\
	".\win32com\src\include\PyIEnumSTATSTG.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyIEnumVARIANT.cpp

!IF  "$(CFG)" == "win32com - Win32 Release"

!ELSEIF  "$(CFG)" == "win32com - Win32 Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Release"

DEP_CPP_PYIENUMV=\
	".\win32com\src\include\PyIEnumVARIANT.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYIENUMV=\
	".\angeobject.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\ethodobject.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\rtapi.h"\
	".\tringobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\val.h"\
	".\xtapi.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ys\stat.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE SH3) Release"

DEP_CPP_PYIENUMV=\
	".\win32com\src\include\PyIEnumVARIANT.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYIENUMV=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
	".\eval.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\nicodeobject.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\object.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\tringobject.h"\
	".\ufferobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE x86em) Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Debug"

DEP_CPP_PYIENUMV=\
	".\win32com\src\include\PyIEnumVARIANT.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyIErrorLog.cpp

!IF  "$(CFG)" == "win32com - Win32 Release"

!ELSEIF  "$(CFG)" == "win32com - Win32 Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Release"

DEP_CPP_PYIER=\
	".\win32com\src\include\propbag.h"\
	".\win32com\src\include\PyIPersist.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYIER=\
	".\angeobject.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\ethodobject.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\rtapi.h"\
	".\tringobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\val.h"\
	".\xtapi.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ys\stat.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE SH3) Release"

DEP_CPP_PYIER=\
	".\win32com\src\include\propbag.h"\
	".\win32com\src\include\PyIPersist.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYIER=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
	".\eval.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\nicodeobject.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\object.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\tringobject.h"\
	".\ufferobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE x86em) Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Debug"

DEP_CPP_PYIER=\
	".\win32com\src\include\propbag.h"\
	".\win32com\src\include\PyIPersist.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyIExternalConnection.cpp

!IF  "$(CFG)" == "win32com - Win32 Release"

!ELSEIF  "$(CFG)" == "win32com - Win32 Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Release"

DEP_CPP_PYIEX=\
	".\win32com\src\include\PyIExternalConnection.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYIEX=\
	".\angeobject.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\ethodobject.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\rtapi.h"\
	".\tringobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\val.h"\
	".\xtapi.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ys\stat.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE SH3) Release"

DEP_CPP_PYIEX=\
	".\win32com\src\include\PyIExternalConnection.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYIEX=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
	".\eval.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\nicodeobject.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\object.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\tringobject.h"\
	".\ufferobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE x86em) Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Debug"

DEP_CPP_PYIEX=\
	".\win32com\src\include\PyIExternalConnection.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyILockBytes.cpp

!IF  "$(CFG)" == "win32com - Win32 Release"

!ELSEIF  "$(CFG)" == "win32com - Win32 Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Release"

DEP_CPP_PYILO=\
	".\win32com\src\include\PyILockBytes.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYILO=\
	".\angeobject.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\ethodobject.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\rtapi.h"\
	".\tringobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\val.h"\
	".\xtapi.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ys\stat.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE SH3) Release"

DEP_CPP_PYILO=\
	".\win32com\src\include\PyILockBytes.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYILO=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
	".\eval.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\nicodeobject.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\object.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\tringobject.h"\
	".\ufferobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE x86em) Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Debug"

DEP_CPP_PYILO=\
	".\win32com\src\include\PyILockBytes.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyIMoniker.cpp

!IF  "$(CFG)" == "win32com - Win32 Release"

!ELSEIF  "$(CFG)" == "win32com - Win32 Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Release"

DEP_CPP_PYIMO=\
	".\win32com\src\include\PyIBindCtx.h"\
	".\win32com\src\include\PyIMoniker.h"\
	".\win32com\src\include\PyIPersist.h"\
	".\win32com\src\include\PyIPersistStream.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYIMO=\
	".\angeobject.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\ethodobject.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\rtapi.h"\
	".\tringobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\val.h"\
	".\xtapi.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ys\stat.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE SH3) Release"

DEP_CPP_PYIMO=\
	".\win32com\src\include\PyIBindCtx.h"\
	".\win32com\src\include\PyIMoniker.h"\
	".\win32com\src\include\PyIPersist.h"\
	".\win32com\src\include\PyIPersistStream.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYIMO=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
	".\eval.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\nicodeobject.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\object.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\tringobject.h"\
	".\ufferobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE x86em) Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Debug"

DEP_CPP_PYIMO=\
	".\win32com\src\include\PyIBindCtx.h"\
	".\win32com\src\include\PyIMoniker.h"\
	".\win32com\src\include\PyIPersist.h"\
	".\win32com\src\include\PyIPersistStream.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyIPersist.cpp

!IF  "$(CFG)" == "win32com - Win32 Release"

!ELSEIF  "$(CFG)" == "win32com - Win32 Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Release"

DEP_CPP_PYIPE=\
	".\win32com\src\include\PyIPersist.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYIPE=\
	".\angeobject.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\ethodobject.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\rtapi.h"\
	".\tringobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\val.h"\
	".\xtapi.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ys\stat.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE SH3) Release"

DEP_CPP_PYIPE=\
	".\win32com\src\include\PyIPersist.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYIPE=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
	".\eval.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\nicodeobject.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\object.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\tringobject.h"\
	".\ufferobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE x86em) Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Debug"

DEP_CPP_PYIPE=\
	".\win32com\src\include\PyIPersist.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyIPersistFile.cpp

!IF  "$(CFG)" == "win32com - Win32 Release"

!ELSEIF  "$(CFG)" == "win32com - Win32 Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Release"

DEP_CPP_PYIPER=\
	".\win32com\src\include\PyIPersist.h"\
	".\win32com\src\include\PyIPersistFile.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYIPER=\
	".\angeobject.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\ethodobject.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\rtapi.h"\
	".\tringobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\val.h"\
	".\xtapi.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ys\stat.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE SH3) Release"

DEP_CPP_PYIPER=\
	".\win32com\src\include\PyIPersist.h"\
	".\win32com\src\include\PyIPersistFile.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYIPER=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
	".\eval.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\nicodeobject.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\object.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\tringobject.h"\
	".\ufferobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE x86em) Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Debug"

DEP_CPP_PYIPER=\
	".\win32com\src\include\PyIPersist.h"\
	".\win32com\src\include\PyIPersistFile.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyIPersistPropertyBag.cpp

!IF  "$(CFG)" == "win32com - Win32 Release"

!ELSEIF  "$(CFG)" == "win32com - Win32 Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Release"

DEP_CPP_PYIPERS=\
	".\win32com\src\include\propbag.h"\
	".\win32com\src\include\PyIPersist.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYIPERS=\
	".\angeobject.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\ethodobject.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\rtapi.h"\
	".\tringobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\val.h"\
	".\xtapi.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ys\stat.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE SH3) Release"

DEP_CPP_PYIPERS=\
	".\win32com\src\include\propbag.h"\
	".\win32com\src\include\PyIPersist.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYIPERS=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
	".\eval.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\nicodeobject.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\object.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\tringobject.h"\
	".\ufferobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE x86em) Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Debug"

DEP_CPP_PYIPERS=\
	".\win32com\src\include\propbag.h"\
	".\win32com\src\include\PyIPersist.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyIPersistStorage.cpp

!IF  "$(CFG)" == "win32com - Win32 Release"

!ELSEIF  "$(CFG)" == "win32com - Win32 Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Release"

DEP_CPP_PYIPERSI=\
	".\win32com\src\include\PyIPersist.h"\
	".\win32com\src\include\PyIPersistStorage.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYIPERSI=\
	".\angeobject.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\ethodobject.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\rtapi.h"\
	".\tringobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\val.h"\
	".\xtapi.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ys\stat.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE SH3) Release"

DEP_CPP_PYIPERSI=\
	".\win32com\src\include\PyIPersist.h"\
	".\win32com\src\include\PyIPersistStorage.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYIPERSI=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
	".\eval.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\nicodeobject.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\object.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\tringobject.h"\
	".\ufferobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE x86em) Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Debug"

DEP_CPP_PYIPERSI=\
	".\win32com\src\include\PyIPersist.h"\
	".\win32com\src\include\PyIPersistStorage.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyIPersistStream.cpp

!IF  "$(CFG)" == "win32com - Win32 Release"

!ELSEIF  "$(CFG)" == "win32com - Win32 Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Release"

DEP_CPP_PYIPERSIS=\
	".\win32com\src\include\PyIPersist.h"\
	".\win32com\src\include\PyIPersistStream.h"\
	".\win32com\src\include\PyIStream.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYIPERSIS=\
	".\angeobject.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\ethodobject.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\rtapi.h"\
	".\tringobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\val.h"\
	".\xtapi.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ys\stat.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE SH3) Release"

DEP_CPP_PYIPERSIS=\
	".\win32com\src\include\PyIPersist.h"\
	".\win32com\src\include\PyIPersistStream.h"\
	".\win32com\src\include\PyIStream.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYIPERSIS=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
	".\eval.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\nicodeobject.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\object.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\tringobject.h"\
	".\ufferobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE x86em) Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Debug"

DEP_CPP_PYIPERSIS=\
	".\win32com\src\include\PyIPersist.h"\
	".\win32com\src\include\PyIPersistStream.h"\
	".\win32com\src\include\PyIStream.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyIPersistStreamInit.cpp

!IF  "$(CFG)" == "win32com - Win32 Release"

!ELSEIF  "$(CFG)" == "win32com - Win32 Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Release"

DEP_CPP_PYIPERSIST=\
	".\win32com\src\include\PyIPersist.h"\
	".\win32com\src\include\PyIPersistStream.h"\
	".\win32com\src\include\PyIPersistStreamInit.h"\
	".\win32com\src\include\PyIStream.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYIPERSIST=\
	".\angeobject.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\ethodobject.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\rtapi.h"\
	".\tringobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\val.h"\
	".\xtapi.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ys\stat.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE SH3) Release"

DEP_CPP_PYIPERSIST=\
	".\win32com\src\include\PyIPersist.h"\
	".\win32com\src\include\PyIPersistStream.h"\
	".\win32com\src\include\PyIPersistStreamInit.h"\
	".\win32com\src\include\PyIStream.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYIPERSIST=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
	".\eval.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\nicodeobject.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\object.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\tringobject.h"\
	".\ufferobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE x86em) Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Debug"

DEP_CPP_PYIPERSIST=\
	".\win32com\src\include\PyIPersist.h"\
	".\win32com\src\include\PyIPersistStream.h"\
	".\win32com\src\include\PyIPersistStreamInit.h"\
	".\win32com\src\include\PyIStream.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyIPropertyBag.cpp

!IF  "$(CFG)" == "win32com - Win32 Release"

!ELSEIF  "$(CFG)" == "win32com - Win32 Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Release"

DEP_CPP_PYIPR=\
	".\win32com\src\include\propbag.h"\
	".\win32com\src\include\PyIPersist.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYIPR=\
	".\angeobject.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\ethodobject.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\rtapi.h"\
	".\tringobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\val.h"\
	".\xtapi.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ys\stat.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE SH3) Release"

DEP_CPP_PYIPR=\
	".\win32com\src\include\propbag.h"\
	".\win32com\src\include\PyIPersist.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYIPR=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
	".\eval.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\nicodeobject.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\object.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\tringobject.h"\
	".\ufferobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE x86em) Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Debug"

DEP_CPP_PYIPR=\
	".\win32com\src\include\propbag.h"\
	".\win32com\src\include\PyIPersist.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyIPropertySetStorage.cpp

!IF  "$(CFG)" == "win32com - Win32 Release"

!ELSEIF  "$(CFG)" == "win32com - Win32 Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Release"

DEP_CPP_PYIPRO=\
	".\win32com\src\include\pyipropertysetstorage.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYIPRO=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
	".\eval.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\nicodeobject.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\object.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\tringobject.h"\
	".\ufferobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE SH3) Release"

DEP_CPP_PYIPRO=\
	".\win32com\src\include\pyipropertysetstorage.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYIPRO=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
	".\eval.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\nicodeobject.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\object.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\tringobject.h"\
	".\ufferobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE x86em) Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Debug"

DEP_CPP_PYIPRO=\
	".\win32com\src\include\pyipropertysetstorage.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYIPRO=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
	".\eval.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\nicodeobject.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\object.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\tringobject.h"\
	".\ufferobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyIPropertyStorage.cpp

!IF  "$(CFG)" == "win32com - Win32 Release"

!ELSEIF  "$(CFG)" == "win32com - Win32 Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Release"

DEP_CPP_PYIPROP=\
	".\win32com\src\include\pyipropertystorage.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYIPROP=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
	".\eval.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\nicodeobject.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\object.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\tringobject.h"\
	".\ufferobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE SH3) Release"

DEP_CPP_PYIPROP=\
	".\win32com\src\include\pyipropertystorage.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYIPROP=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
	".\eval.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\nicodeobject.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\object.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\tringobject.h"\
	".\ufferobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE x86em) Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Debug"

DEP_CPP_PYIPROP=\
	".\win32com\src\include\pyipropertystorage.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYIPROP=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
	".\eval.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\nicodeobject.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\object.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\tringobject.h"\
	".\ufferobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyIProvideClassInfo.cpp

!IF  "$(CFG)" == "win32com - Win32 Release"

!ELSEIF  "$(CFG)" == "win32com - Win32 Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Release"

DEP_CPP_PYIPROV=\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYIPROV=\
	".\angeobject.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\ethodobject.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\rtapi.h"\
	".\tringobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\val.h"\
	".\xtapi.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ys\stat.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE SH3) Release"

DEP_CPP_PYIPROV=\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYIPROV=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
	".\eval.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\nicodeobject.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\object.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\tringobject.h"\
	".\ufferobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE x86em) Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Debug"

DEP_CPP_PYIPROV=\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\stdafx.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyIRunningObjectTable.cpp

!IF  "$(CFG)" == "win32com - Win32 Release"

!ELSEIF  "$(CFG)" == "win32com - Win32 Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Release"

DEP_CPP_PYIRU=\
	".\win32com\src\include\PyIMoniker.h"\
	".\win32com\src\include\PyIPersist.h"\
	".\win32com\src\include\PyIPersistStream.h"\
	".\win32com\src\include\PyIRunningObjectTable.h"\
	".\win32com\src\include\PyIStream.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYIRU=\
	".\angeobject.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\ethodobject.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\rtapi.h"\
	".\tringobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\val.h"\
	".\xtapi.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ys\stat.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE SH3) Release"

DEP_CPP_PYIRU=\
	".\win32com\src\include\PyIMoniker.h"\
	".\win32com\src\include\PyIPersist.h"\
	".\win32com\src\include\PyIPersistStream.h"\
	".\win32com\src\include\PyIRunningObjectTable.h"\
	".\win32com\src\include\PyIStream.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYIRU=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
	".\eval.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\nicodeobject.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\object.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\tringobject.h"\
	".\ufferobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE x86em) Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Debug"

DEP_CPP_PYIRU=\
	".\win32com\src\include\PyIMoniker.h"\
	".\win32com\src\include\PyIPersist.h"\
	".\win32com\src\include\PyIPersistStream.h"\
	".\win32com\src\include\PyIRunningObjectTable.h"\
	".\win32com\src\include\PyIStream.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyIServiceProvider.cpp

!IF  "$(CFG)" == "win32com - Win32 Release"

!ELSEIF  "$(CFG)" == "win32com - Win32 Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Release"

DEP_CPP_PYISE=\
	".\win32com\src\include\PyIServiceProvider.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYISE=\
	".\angeobject.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\ethodobject.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\rtapi.h"\
	".\tringobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\val.h"\
	".\xtapi.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ys\stat.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE SH3) Release"

DEP_CPP_PYISE=\
	".\win32com\src\include\PyIServiceProvider.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYISE=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
	".\eval.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\nicodeobject.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\object.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\tringobject.h"\
	".\ufferobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE x86em) Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Debug"

DEP_CPP_PYISE=\
	".\win32com\src\include\PyIServiceProvider.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyIStorage.cpp

!IF  "$(CFG)" == "win32com - Win32 Release"

!ELSEIF  "$(CFG)" == "win32com - Win32 Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Release"

DEP_CPP_PYIST=\
	".\win32com\src\include\PyIStorage.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYIST=\
	".\angeobject.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\ethodobject.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\rtapi.h"\
	".\tringobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\val.h"\
	".\xtapi.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ys\stat.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinObjects.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE SH3) Release"

DEP_CPP_PYIST=\
	".\win32com\src\include\PyIStorage.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYIST=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
	".\eval.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\nicodeobject.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\object.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\tringobject.h"\
	".\ufferobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinObjects.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE x86em) Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Debug"

DEP_CPP_PYIST=\
	".\win32com\src\include\PyIStorage.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYIST=\
	".\win32com\src\extensions\PyWinObjects.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyIStream.cpp

!IF  "$(CFG)" == "win32com - Win32 Release"

!ELSEIF  "$(CFG)" == "win32com - Win32 Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Release"

DEP_CPP_PYISTR=\
	".\win32com\src\include\PyIStream.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYISTR=\
	".\angeobject.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\ethodobject.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\rtapi.h"\
	".\tringobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\val.h"\
	".\xtapi.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ys\stat.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE SH3) Release"

DEP_CPP_PYISTR=\
	".\win32com\src\include\PyIStream.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYISTR=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
	".\eval.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\nicodeobject.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\object.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\tringobject.h"\
	".\ufferobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE x86em) Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Debug"

DEP_CPP_PYISTR=\
	".\win32com\src\include\PyIStream.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\PythonCOMServer.h"\
	".\win32com\src\include\stdafx.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyIType.cpp

!IF  "$(CFG)" == "win32com - Win32 Release"

!ELSEIF  "$(CFG)" == "win32com - Win32 Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Release"

DEP_CPP_PYITY=\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYITY=\
	".\angeobject.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\ethodobject.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\rtapi.h"\
	".\tringobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\val.h"\
	".\xtapi.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ys\stat.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE SH3) Release"

DEP_CPP_PYITY=\
	".\win32com\src\include\PyComTypeObjects.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYITY=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
	".\eval.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\nicodeobject.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\object.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\tringobject.h"\
	".\ufferobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE x86em) Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Debug"

DEP_CPP_PYITY=\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\stdafx.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyITypeObjects.cpp

!IF  "$(CFG)" == "win32com - Win32 Release"

!ELSEIF  "$(CFG)" == "win32com - Win32 Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Release"

DEP_CPP_PYITYP=\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYITYP=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
	".\eval.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\nicodeobject.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\object.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\tringobject.h"\
	".\ufferobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE SH3) Release"

DEP_CPP_PYITYP=\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYITYP=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
	".\eval.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\nicodeobject.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\object.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\tringobject.h"\
	".\ufferobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE x86em) Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Debug"

DEP_CPP_PYITYP=\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYITYP=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
	".\eval.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\nicodeobject.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\object.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\tringobject.h"\
	".\ufferobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyTYPEATTR.cpp

!IF  "$(CFG)" == "win32com - Win32 Release"

!ELSEIF  "$(CFG)" == "win32com - Win32 Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Release"

DEP_CPP_PYTYP=\
	".\win32com\src\include\PyComTypeObjects.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYTYP=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
	".\eval.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\nicodeobject.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\object.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\tringobject.h"\
	".\tructmember.h"\
	".\ufferobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE SH3) Release"

DEP_CPP_PYTYP=\
	".\win32com\src\include\PyComTypeObjects.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYTYP=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
	".\eval.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\nicodeobject.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\object.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\tringobject.h"\
	".\tructmember.h"\
	".\ufferobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE x86em) Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Debug"

DEP_CPP_PYTYP=\
	".\win32com\src\include\PyComTypeObjects.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYTYP=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
	".\eval.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\nicodeobject.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\object.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\tringobject.h"\
	".\tructmember.h"\
	".\ufferobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyVARDESC.cpp

!IF  "$(CFG)" == "win32com - Win32 Release"

!ELSEIF  "$(CFG)" == "win32com - Win32 Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Release"

DEP_CPP_PYVAR=\
	".\win32com\src\include\PyComTypeObjects.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYVAR=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
	".\eval.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\nicodeobject.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\object.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\tringobject.h"\
	".\tructmember.h"\
	".\ufferobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE SH3) Release"

DEP_CPP_PYVAR=\
	".\win32com\src\include\PyComTypeObjects.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYVAR=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
	".\eval.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\nicodeobject.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\object.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\tringobject.h"\
	".\tructmember.h"\
	".\ufferobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE x86em) Debug"

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Debug"

DEP_CPP_PYVAR=\
	".\win32com\src\include\PyComTypeObjects.h"\
	".\win32com\src\include\PythonCOM.h"\
	".\win32com\src\include\stdafx.h"\
	
NODEP_CPP_PYVAR=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
	".\eval.h"\
	".\ictobject.h"\
	".\ileobject.h"\
	".\istobject.h"\
	".\lassobject.h"\
	".\liceobject.h"\
	".\loatobject.h"\
	".\mport.h"\
	".\nicodeobject.h"\
	".\ntobject.h"\
	".\ntrcheck.h"\
	".\object.h"\
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\tringobject.h"\
	".\tructmember.h"\
	".\ufferobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	".\yWinTypes.h"\
	

!ENDIF 

# End Source File
# End Group
# Begin Group "Headers"

# PROP Default_Filter ".h"
# Begin Source File

SOURCE=.\win32com\src\include\propbag.h
# End Source File
# Begin Source File

SOURCE=.\win32com\src\include\PyComTypeObjects.h
# End Source File
# Begin Source File

SOURCE=.\win32com\src\include\PyFactory.h
# End Source File
# Begin Source File

SOURCE=.\win32com\src\include\PyGConnectionPoint.h
# End Source File
# Begin Source File

SOURCE=.\win32com\src\include\PyGConnectionPointContainer.h
# End Source File
# Begin Source File

SOURCE=.\win32com\src\include\PyGPersistStorage.h
# End Source File
# Begin Source File

SOURCE=.\win32com\src\include\PyIBindCtx.h
# End Source File
# Begin Source File

SOURCE=.\win32com\src\include\PyICatInformation.h
# End Source File
# Begin Source File

SOURCE=.\win32com\src\include\PyICatRegister.h
# End Source File
# Begin Source File

SOURCE=.\win32com\src\include\PyIEnumConnectionPoints.h
# End Source File
# Begin Source File

SOURCE=.\win32com\src\include\PyIEnumConnections.h
# End Source File
# Begin Source File

SOURCE=.\win32com\src\include\PyIEnumGUID.h
# End Source File
# Begin Source File

SOURCE=.\win32com\src\include\PyIEnumSTATSTG.h
# End Source File
# Begin Source File

SOURCE=.\win32com\src\include\PyIEnumVARIANT.h
# End Source File
# Begin Source File

SOURCE=.\win32com\src\include\PyIExternalConnection.h
# End Source File
# Begin Source File

SOURCE=.\win32com\src\include\PyILockBytes.h
# End Source File
# Begin Source File

SOURCE=.\win32com\src\include\PyIMoniker.h
# End Source File
# Begin Source File

SOURCE=.\win32com\src\include\PyIPersist.h
# End Source File
# Begin Source File

SOURCE=.\win32com\src\include\PyIPersistFile.h
# End Source File
# Begin Source File

SOURCE=.\win32com\src\include\PyIPersistStorage.h
# End Source File
# Begin Source File

SOURCE=.\win32com\src\include\PyIPersistStream.h
# End Source File
# Begin Source File

SOURCE=.\win32com\src\include\PyIPersistStreamInit.h
# End Source File
# Begin Source File

SOURCE=.\win32com\src\include\PyIRunningObjectTable.h
# End Source File
# Begin Source File

SOURCE=.\win32com\src\include\PyIStorage.h
# End Source File
# Begin Source File

SOURCE=.\win32com\src\include\PyIStream.h
# End Source File
# Begin Source File

SOURCE=.\win32com\src\include\PythonCOM.h
# End Source File
# Begin Source File

SOURCE=.\win32com\src\include\PythonCOMRegister.h
# End Source File
# Begin Source File

SOURCE=.\win32com\src\include\PythonCOMServer.h
# End Source File
# Begin Source File

SOURCE=.\win32com\src\include\stdafx.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\win32com\src\PythonCOM.def

!IF  "$(CFG)" == "win32com - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "win32com - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE SH3) Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE x86em) Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "win32com - Win32 (WCE MIPS) Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# End Target
# End Project
