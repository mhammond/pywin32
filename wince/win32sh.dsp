# Microsoft Developer Studio Project File - Name="win32sh" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (WCE SH) Dynamic-Link Library" 0x0902
# TARGTYPE "Win32 (WCE x86em) Dynamic-Link Library" 0x0b02
# TARGTYPE "Win32 (WCE MIPS) Dynamic-Link Library" 0x0a02

CFG=win32sh - Win32 (WCE MIPS) Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "win32sh.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "win32sh.mak" CFG="win32sh - Win32 (WCE MIPS) Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "win32sh - Win32 (WCE MIPS) Release" (based on\
 "Win32 (WCE MIPS) Dynamic-Link Library")
!MESSAGE "win32sh - Win32 (WCE MIPS) Debug" (based on\
 "Win32 (WCE MIPS) Dynamic-Link Library")
!MESSAGE "win32sh - Win32 (WCE SH) Release" (based on\
 "Win32 (WCE SH) Dynamic-Link Library")
!MESSAGE "win32sh - Win32 (WCE SH) Debug" (based on\
 "Win32 (WCE SH) Dynamic-Link Library")
!MESSAGE "win32sh - Win32 (WCE x86em) Debug" (based on\
 "Win32 (WCE x86em) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
# PROP WCE_Configuration "H/PC Ver. 2.00"

!IF  "$(CFG)" == "win32sh - Win32 (WCE MIPS) Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "WMIPSRel"
# PROP BASE Intermediate_Dir "WMIPSRel"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\WMIPSRel"
# PROP Intermediate_Dir "..\WMIPSRel\Temp"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
CPP=clmips.exe
# ADD BASE CPP /nologo /ML /W3 /O2 /D "NDEBUG" /D "MIPS" /D "_MIPS_" /D "UNDER_CE" /D "UNICODE" /D _WIN32_WCE=100 /YX /QMRWCE /c
# ADD CPP /nologo /MT /W3 /O1 /I "." /I "..\..\Include" /I "..\Python15\Include" /D "NDEBUG" /D "MIPS" /D "_MIPS_" /D "UNDER_CE" /D "UNICODE" /D _WIN32_WCE=100 /YX /QMRWCE /c
RSC=rc.exe
# ADD BASE RSC /l 0x409 /r /d "MIPS" /d "_MIPS_" /d "UNDER_CE" /d "UNICODE" /d "NDEBUG" /d _WIN32_WCE=100
# ADD RSC /l 0x409 /r /d "MIPS" /d "_MIPS_" /d "UNDER_CE" /d "UNICODE" /d "NDEBUG" /d _WIN32_WCE=100
MTL=midl.exe
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL /win32
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 commctrl.lib coredll.lib /nologo /dll /machine:MIPS /subsystem:windowsce,1.0 /fixed:no
# SUBTRACT BASE LINK32 /pdb:none /nodefaultlib
# ADD LINK32 commctrl.lib coredll.lib ..\WMIPSRel\Python15.lib /nologo /dll /machine:MIPS /out:"..\WMIPSRel/win32sh.pyd" /subsystem:windowsce,1.0 /fixed:no
# SUBTRACT LINK32 /pdb:none /nodefaultlib
PFILE=pfile.exe
# ADD BASE PFILE COPY
# ADD PFILE COPY

!ELSEIF  "$(CFG)" == "win32sh - Win32 (WCE MIPS) Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "WMIPSDbg"
# PROP BASE Intermediate_Dir "WMIPSDbg"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "WMIPSDbg"
# PROP Intermediate_Dir "WMIPSDbg"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
CPP=clmips.exe
# ADD BASE CPP /nologo /MLd /W3 /Zi /Od /D "DEBUG" /D "MIPS" /D "_MIPS_" /D "UNDER_CE" /D "UNICODE" /D _WIN32_WCE=100 /YX /QMRWCE /c
# ADD CPP /nologo /MLd /W3 /Zi /Od /I "." /I "..\..\Include" /I "..\Python15\Include" /D "DEBUG" /D "MIPS" /D "_MIPS_" /D "UNDER_CE" /D "UNICODE" /D _WIN32_WCE=100 /YX /QMRWCE /c
RSC=rc.exe
# ADD BASE RSC /l 0x409 /r /d "MIPS" /d "_MIPS_" /d "UNDER_CE" /d "UNICODE" /d "DEBUG" /d _WIN32_WCE=100
# ADD RSC /l 0x409 /r /d "MIPS" /d "_MIPS_" /d "UNDER_CE" /d "UNICODE" /d "DEBUG" /d _WIN32_WCE=100
MTL=midl.exe
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 commctrl.lib coredll.lib /nologo /dll /debug /machine:MIPS /subsystem:windowsce,1.0 /fixed:no
# SUBTRACT BASE LINK32 /pdb:none /nodefaultlib
# ADD LINK32 commctrl.lib coredll.lib ..\Python15\WMIPSRel\Python15.lib /nologo /dll /debug /machine:MIPS /out:"WMIPSDbg/win32sh.pyd" /subsystem:windowsce,1.0 /fixed:no
# SUBTRACT LINK32 /pdb:none /nodefaultlib
PFILE=pfile.exe
# ADD BASE PFILE COPY
# ADD PFILE COPY

!ELSEIF  "$(CFG)" == "win32sh - Win32 (WCE SH) Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "WCESHRel"
# PROP BASE Intermediate_Dir "WCESHRel"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\WCESHRel"
# PROP Intermediate_Dir "..\WCESHRel"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
CPP=shcl.exe
# ADD BASE CPP /nologo /ML /W3 /O2 /D "NDEBUG" /D "SHx" /D "SH3" /D "_SH3_" /D "UNDER_CE" /D "UNICODE" /D _WIN32_WCE=100 /YX /c
# ADD CPP /nologo /MT /W3 /O1 /I "." /I "..\..\Include" /I "..\Python15\Include" /D "NDEBUG" /D "SHx" /D "SH3" /D "_SH3_" /D "UNDER_CE" /D "UNICODE" /D _WIN32_WCE=100 /YX /c
RSC=rc.exe
# ADD BASE RSC /l 0x409 /r /d "SHx" /d "SH3" /d "_SH3_" /d "UNDER_CE" /d "UNICODE" /d "NDEBUG" /d _WIN32_WCE=100
# ADD RSC /l 0x409 /r /d "SHx" /d "SH3" /d "_SH3_" /d "UNDER_CE" /d "UNICODE" /d "NDEBUG" /d _WIN32_WCE=100
MTL=midl.exe
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL /win32
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 commctrl.lib coredll.lib /nologo /dll /machine:SH3 /subsystem:windowsce,1.0 /fixed:no
# SUBTRACT BASE LINK32 /pdb:none /nodefaultlib
# ADD LINK32 commctrl.lib coredll.lib ..\WCESHRel\Python15.lib /nologo /dll /machine:SH3 /out:"..\WCESHRel/win32sh.pyd" /subsystem:windowsce,1.0 /fixed:no
# SUBTRACT LINK32 /pdb:none /nodefaultlib
PFILE=pfile.exe
# ADD BASE PFILE COPY
# ADD PFILE COPY

!ELSEIF  "$(CFG)" == "win32sh - Win32 (WCE SH) Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "WCESHDbg"
# PROP BASE Intermediate_Dir "WCESHDbg"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "WCESHDbg"
# PROP Intermediate_Dir "WCESHDbg"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
CPP=shcl.exe
# ADD BASE CPP /nologo /MLd /W3 /Zi /Od /D "DEBUG" /D "SHx" /D "SH3" /D "_SH3_" /D "UNDER_CE" /D "UNICODE" /D _WIN32_WCE=100 /YX /c
# ADD CPP /nologo /MLd /W3 /Zi /Od /I "." /I "..\..\Include" /I "..\Python15\Include" /D "DEBUG" /D "SHx" /D "SH3" /D "_SH3_" /D "UNDER_CE" /D "UNICODE" /D _WIN32_WCE=100 /YX /c
RSC=rc.exe
# ADD BASE RSC /l 0x409 /r /d "SHx" /d "SH3" /d "_SH3_" /d "UNDER_CE" /d "UNICODE" /d "DEBUG" /d _WIN32_WCE=100
# ADD RSC /l 0x409 /r /d "SHx" /d "SH3" /d "_SH3_" /d "UNDER_CE" /d "UNICODE" /d "DEBUG" /d _WIN32_WCE=100
MTL=midl.exe
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 commctrl.lib coredll.lib /nologo /dll /debug /machine:SH3 /subsystem:windowsce,1.0 /fixed:no
# SUBTRACT BASE LINK32 /pdb:none /nodefaultlib
# ADD LINK32 commctrl.lib coredll.lib ..\Python15\WMIPSRel\Python15.lib /nologo /dll /debug /machine:SH3 /subsystem:windowsce,1.0 /fixed:no
# SUBTRACT LINK32 /pdb:none /nodefaultlib
PFILE=pfile.exe
# ADD BASE PFILE COPY
# ADD PFILE COPY

!ELSEIF  "$(CFG)" == "win32sh - Win32 (WCE x86em) Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "win32sh_"
# PROP BASE Intermediate_Dir "win32sh_"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\Wx86Dbg"
# PROP Intermediate_Dir "..\Wx86Dbg\Temp"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
CPP=cl.exe
# ADD BASE CPP /nologo /MLd /W3 /Gm /Zi /Od /I "." /I "..\..\Include" /I "..\Python15\Include" /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "_UNICODE" /D "WIN32" /D "STRICT" /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "_WIN32_WCE_EMULATION" /D "INTERNATIONAL" /D "USA" /D "INTLMSG_CODEPAGE" /D "_DEBUG" /D "x86" /D "i486" /D "_x86_" /YX /QMRWCE /c
# ADD CPP /nologo /MT /W3 /Gm /Zi /Od /I "." /I "..\..\Include" /I "..\Python15\Include" /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "_UNICODE" /D "WIN32" /D "STRICT" /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "_WIN32_WCE_EMULATION" /D "INTERNATIONAL" /D "USA" /D "INTLMSG_CODEPAGE" /D "_DEBUG" /D "x86" /D "i486" /D "_x86_" /YX /QMRWCE /c
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d UNDER_CE=$(CEVersion) /d "UNICODE" /d "_UNICODE" /d "WIN32" /d "STRICT" /d _WIN32_WCE=$(CEVersion) /d "$(CEConfigName)" /d "_WIN32_WCE_EMULATION" /d "INTERNATIONAL" /d "USA" /d "INTLMSG_CODEPAGE" /d "_DEBUG" /d "x86" /d "i486" /d "_x86_"
# ADD RSC /l 0x409 /d UNDER_CE=$(CEVersion) /d "UNICODE" /d "_UNICODE" /d "WIN32" /d "STRICT" /d _WIN32_WCE=$(CEVersion) /d "$(CEConfigName)" /d "_WIN32_WCE_EMULATION" /d "INTERNATIONAL" /d "USA" /d "INTLMSG_CODEPAGE" /d "_DEBUG" /d "x86" /d "i486" /d "_x86_"
MTL=midl.exe
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 ..\Python15\WMIPSRel\Python15.lib commctrl.lib coredll.lib /nologo /subsystem:windows /dll /debug /machine:IX86 /subsystem:windowsce,1.0 /fixed:no
# SUBTRACT BASE LINK32 /pdb:none /incremental:no /nodefaultlib
# ADD LINK32 ..\Wx86Dbg\Python15.lib commctrl.lib coredll.lib /nologo /entry:"_DllMainCRTStartup@12" /subsystem:windows /dll /debug /machine:IX86 /subsystem:windowsce,1.0 /fixed:no
# SUBTRACT LINK32 /pdb:none
EMPFILE=empfile.exe
# ADD BASE EMPFILE -COPY
# ADD EMPFILE -COPY

!ENDIF 

# Begin Target

# Name "win32sh - Win32 (WCE MIPS) Release"
# Name "win32sh - Win32 (WCE MIPS) Debug"
# Name "win32sh - Win32 (WCE SH) Release"
# Name "win32sh - Win32 (WCE SH) Debug"
# Name "win32sh - Win32 (WCE x86em) Debug"
# Begin Source File

SOURCE=.\src\win32sh.cpp

!IF  "$(CFG)" == "win32sh - Win32 (WCE MIPS) Release"

DEP_CPP_WIN32=\
	{$(INCLUDE)}"eval.h"\
	{$(INCLUDE)}"object.h"\
	
NODEP_CPP_WIN32=\
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
	

!ELSEIF  "$(CFG)" == "win32sh - Win32 (WCE MIPS) Debug"

DEP_CPP_WIN32=\
	{$(INCLUDE)}"eval.h"\
	{$(INCLUDE)}"object.h"\
	
NODEP_CPP_WIN32=\
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
	".\xtapi.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	

!ELSEIF  "$(CFG)" == "win32sh - Win32 (WCE SH) Release"

DEP_CPP_WIN32=\
	{$(INCLUDE)}"abstract.h"\
	{$(INCLUDE)}"ceval.h"\
	{$(INCLUDE)}"classobject.h"\
	{$(INCLUDE)}"cobject.h"\
	{$(INCLUDE)}"complexobject.h"\
	{$(INCLUDE)}"config.h"\
	{$(INCLUDE)}"crtapi.h"\
	{$(INCLUDE)}"dictobject.h"\
	{$(INCLUDE)}"extapi.h"\
	{$(INCLUDE)}"fileobject.h"\
	{$(INCLUDE)}"floatobject.h"\
	{$(INCLUDE)}"funcobject.h"\
	{$(INCLUDE)}"import.h"\
	{$(INCLUDE)}"intobject.h"\
	{$(INCLUDE)}"intrcheck.h"\
	{$(INCLUDE)}"listobject.h"\
	{$(INCLUDE)}"longobject.h"\
	{$(INCLUDE)}"methodobject.h"\
	{$(INCLUDE)}"modsupport.h"\
	{$(INCLUDE)}"moduleobject.h"\
	{$(INCLUDE)}"mymalloc.h"\
	{$(INCLUDE)}"myproto.h"\
	{$(INCLUDE)}"object.h"\
	{$(INCLUDE)}"objimpl.h"\
	{$(INCLUDE)}"pydebug.h"\
	{$(INCLUDE)}"pyerrors.h"\
	{$(INCLUDE)}"pyfpe.h"\
	{$(INCLUDE)}"pystate.h"\
	{$(INCLUDE)}"Python.h"\
	{$(INCLUDE)}"pythonrun.h"\
	{$(INCLUDE)}"rangeobject.h"\
	{$(INCLUDE)}"sliceobject.h"\
	{$(INCLUDE)}"stringobject.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sysmodule.h"\
	{$(INCLUDE)}"traceback.h"\
	{$(INCLUDE)}"tupleobject.h"\
	

!ELSEIF  "$(CFG)" == "win32sh - Win32 (WCE SH) Debug"

DEP_CPP_WIN32=\
	{$(INCLUDE)}"eval.h"\
	{$(INCLUDE)}"object.h"\
	
NODEP_CPP_WIN32=\
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
	".\xtapi.h"\
	".\ydebug.h"\
	".\yerrors.h"\
	".\yfpe.h"\
	".\ymalloc.h"\
	".\yproto.h"\
	".\ysmodule.h"\
	".\ystate.h"\
	".\ython.h"\
	".\ythonrun.h"\
	

!ELSEIF  "$(CFG)" == "win32sh - Win32 (WCE x86em) Debug"

!ENDIF 

# End Source File
# End Target
# End Project
