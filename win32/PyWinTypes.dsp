# Microsoft Developer Studio Project File - Name="PyWinTypes" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (WCE SH3) Dynamic-Link Library" 0x8102
# TARGTYPE "Win32 (WCE x86em) Dynamic-Link Library" 0x7f02
# TARGTYPE "Win32 (WCE MIPS) Dynamic-Link Library" 0x8202
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
!MESSAGE "PyWinTypes - Win32 (WCE MIPS) Release" (based on "Win32 (WCE MIPS) Dynamic-Link Library")
!MESSAGE "PyWinTypes - Win32 (WCE x86em) Debug" (based on "Win32 (WCE x86em) Dynamic-Link Library")
!MESSAGE "PyWinTypes - Win32 (WCE x86em) Release" (based on "Win32 (WCE x86em) Dynamic-Link Library")
!MESSAGE "PyWinTypes - Win32 (WCE MIPS) Debug" (based on "Win32 (WCE MIPS) Dynamic-Link Library")
!MESSAGE "PyWinTypes - Win32 (WCE SH3) Debug" (based on "Win32 (WCE SH3) Dynamic-Link Library")
!MESSAGE "PyWinTypes - Win32 (WCE SH3) Release" (based on "Win32 (WCE SH3) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Python/win32", CDAAAAAA"
# PROP Scc_LocalPath "."

!IF  "$(CFG)" == "PyWinTypes - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Build"
# PROP BASE Intermediate_Dir "Build\Temp\PyWinTypes\Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Build\Release"
# PROP Intermediate_Dir "Build\Temp\PyWinTypes\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
F90=df.exe
CPP=cl.exe
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "_WINDOWS" /D "BUILD_PYWINTYPES" /D "NDEBUG" /D "STRICT" /YX /FD /c
MTL=midl.exe
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o /win32 "NUL"
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o /win32 "NUL"
RSC=rc.exe
# ADD BASE RSC /l 0xc09 /d "NDEBUG"
# ADD RSC /l 0xc09 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /base:"0x1e600000" /subsystem:windows /dll /pdb:"Build\System\PyWinTypes15.pdb" /machine:I386 /out:"Build\System\PyWinTypes15.dll" /implib:"Build\PyWinTypes.lib"
# SUBTRACT LINK32 /pdb:none /debug
# Begin Custom Build - copy to system32
ProjDir=.
TargetPath=.\Build\System\PyWinTypes15.dll
TargetName=PyWinTypes15
InputPath=.\Build\System\PyWinTypes15.dll
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
# PROP Output_Dir "Build\Debug"
# PROP Intermediate_Dir "Build\Temp\PyWinTypes\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
F90=df.exe
CPP=cl.exe
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MDd /W3 /GX /ZI /Od /D "WIN32" /D "_WINDOWS" /D "BUILD_PYWINTYPES" /D "_DEBUG" /D "DEBUG" /D "STRICT" /YX /FD /c
MTL=midl.exe
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o /win32 "NUL"
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o /win32 "NUL"
RSC=rc.exe
# ADD BASE RSC /l 0xc09 /d "_DEBUG"
# ADD RSC /l 0xc09 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /base:"0x1e600000" /subsystem:windows /dll /pdb:"Build\System\PyWinTypes15_d.pdb" /debug /machine:I386 /out:"Build\System\PyWinTypes15_d.dll" /implib:"Build\PyWinTypes_d.lib" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none
# Begin Custom Build - copy to system32
ProjDir=.
TargetPath=.\Build\System\PyWinTypes15_d.dll
TargetName=PyWinTypes15_d
InputPath=.\Build\System\PyWinTypes15_d.dll
SOURCE="$(InputPath)"

"$(ProjDir)\$(TargetName).flg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(TargetPath) %SYSTEMROOT%\System32\. && echo Done >                                      $(ProjDir)\$(TargetName).flg

# End Custom Build

!ELSEIF  "$(CFG)" == "PyWinTypes - Win32 (WCE MIPS) Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "PyWinTy0"
# PROP BASE Intermediate_Dir "PyWinTy0"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Build\CE\mips"
# PROP Intermediate_Dir "Build\Temp\pywintypes\ce\mips\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib commctrl.lib coredll.lib /nologo /dll /pdb:"Build\System\PyWinTypes15.pdb" /debug /machine:MIPS /out:"Build\System\PyWinTypes15.dll" /implib:"Build\PyWinTypes.lib"
# SUBTRACT BASE LINK32 /pdb:none /nodefaultlib
# ADD LINK32 coredll.lib ole32.lib oleaut32.lib /nologo /dll /machine:MIPS /nodefaultlib:"$(CENoDefaultLib)" /out:"Build\ce\mips\PyWinTypes15.dll" /implib:"Build\ce\mips\PyWinTypes.lib" /subsystem:windowsce,1.0
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
MTL=midl.exe
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /o /win32 "NUL"
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /o /win32 "NUL"
RSC=rc.exe
# ADD BASE RSC /l 0xc09 /r /d "MIPS" /d "_MIPS_" /d UNDER_CE=$(CEVersion) /d _WIN32_WCE=$(CEVersion) /d "$(CEConfigName)" /d "UNICODE" /d "NDEBUG"
# ADD RSC /l 0xc09 /r /d "MIPS" /d "_MIPS_" /d UNDER_CE=$(CEVersion) /d _WIN32_WCE=$(CEVersion) /d "$(CEConfigName)" /d "UNICODE" /d "NDEBUG"
CPP=clmips.exe
# ADD BASE CPP /nologo /ML /W3 /Zi /O2 /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "NDEBUG" /D "MIPS" /D "_MIPS_" /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "BUILD_PYWINTYPES" /D "STRICT" /YX /FD /c
# ADD CPP /nologo /M$(CECrtMT) /W3 /O1 /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "NDEBUG" /D "MIPS" /D "_MIPS_" /D UNDER_CE=200 /D "UNICODE" /D "BUILD_PYWINTYPES" /D "STRICT" /YX /FD /c
PFILE=pfile.exe
# ADD BASE PFILE COPY
# ADD PFILE COPY

!ELSEIF  "$(CFG)" == "PyWinTypes - Win32 (WCE x86em) Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "PyWinTy0"
# PROP BASE Intermediate_Dir "PyWinTy0"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Build\ce\x86em"
# PROP Intermediate_Dir "Build\Temp\pywintypes\ce\x86em\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib commctrl.lib coredll.lib /nologo /base:"0x1e600000" /subsystem:windows /dll /pdb:"Build\System\PyWinTypes15.pdb" /debug /machine:IX86 /out:"Build\System\PyWinTypes15.dll" /implib:"Build\PyWinTypes.lib"
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 coredll.lib /nologo /base:"0x1e600000" /entry:"_DllMainCRTStartup@12" /subsystem:windows /dll /debug /machine:IX86 /nodefaultlib:"$(CENoDefaultLib)" /out:"Build\ce\x86em\PyWinTypes15_d.dll" /implib:"Build\ce\x86em\PyWinTypes_d.lib"
# SUBTRACT LINK32 /pdb:none
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
MTL=midl.exe
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /o /win32 "NUL"
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /o /win32 "NUL"
RSC=rc.exe
# ADD BASE RSC /l 0xc09 /d UNDER_CE=$(CEVersion) /d "UNICODE" /d "_UNICODE" /d "WIN32" /d "STRICT" /d _WIN32_WCE=$(CEVersion) /d "$(CEConfigName)" /d "_WIN32_WCE_EMULATION" /d "INTERNATIONAL" /d "USA" /d "INTLMSG_CODEPAGE" /d "NDEBUG"
# ADD RSC /l 0xc09 /d UNDER_CE=$(CEVersion) /d "UNICODE" /d "_UNICODE" /d "WIN32" /d "STRICT" /d _WIN32_WCE=$(CEVersion) /d "$(CEConfigName)" /d "_WIN32_WCE_EMULATION" /d "INTERNATIONAL" /d "USA" /d "INTLMSG_CODEPAGE" /d "NDEBUG"
CPP=cl.exe
# ADD BASE CPP /nologo /ML /W3 /Gm /Zi /O2 /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "_UNICODE" /D "WIN32" /D "STRICT" /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "_WIN32_WCE_EMULATION" /D "INTERNATIONAL" /D "USA" /D "INTLMSG_CODEPAGE" /D "NDEBUG" /D "x86" /D "i486" /D "_x86_" /D "BUILD_PYWINTYPES" /YX /FD /c
# ADD CPP /nologo /MT /W3 /Gm /Zi /Od /D "DEBUG" /D "_DEBUG" /D "_WIN32_WCE_EMULATION" /D UNDER_CE=200 /D "UNICODE" /D "_UNICODE" /D "WIN32" /D "STRICT" /D _WIN32_WCE=200 /D "INTERNATIONAL" /D "USA" /D "INTLMSG_CODEPAGE" /D "x86" /D "i486" /D "_x86_" /D "BUILD_PYWINTYPES" /YX /FD /c
EMPFILE=empfile.exe
# ADD BASE EMPFILE COPY
# ADD EMPFILE COPY

!ELSEIF  "$(CFG)" == "PyWinTypes - Win32 (WCE x86em) Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "PyWinTyp"
# PROP BASE Intermediate_Dir "PyWinTyp"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Build\ce\x86em"
# PROP Intermediate_Dir "Build\Temp\pywintypes\ce\x86em\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
LINK32=link.exe
# ADD BASE LINK32 winsockm.lib commctrl.lib coredll.lib corelibc.lib ole32m.lib oleautm.lib uuid.lib /nologo /base:"0x1e600000" /subsystem:windows /dll /pdb:"Build\System\PyWinTypes15.pdb" /debug /machine:IX86 /nodefaultlib:"libcmt.lib" /out:"Build\ce\x86\PyWinTypes_d.dll"
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 coredll.lib corelibc.lib /nologo /base:"0x1e600000" /subsystem:windows /dll /machine:IX86 /nodefaultlib:"libcmt.lib" /nodefaultlib:"$(CENoDefaultLib)" /out:"Build\ce\x86em\PyWinTypes15.dll" /implib:"Build\ce\x86em\PyWinTypes.lib"
# SUBTRACT LINK32 /pdb:none
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
MTL=midl.exe
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /o /win32 "NUL"
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /o /win32 "NUL"
RSC=rc.exe
# ADD BASE RSC /l 0xc09 /d UNDER_CE=$(CEVersion) /d "UNICODE" /d "_UNICODE" /d "WIN32" /d "STRICT" /d _WIN32_WCE=$(CEVersion) /d "$(CEConfigName)" /d "_WIN32_WCE_EMULATION" /d "INTERNATIONAL" /d "USA" /d "INTLMSG_CODEPAGE" /d "NDEBUG"
# ADD RSC /l 0xc09 /d UNDER_CE=$(CEVersion) /d "UNICODE" /d "_UNICODE" /d "WIN32" /d "STRICT" /d _WIN32_WCE=$(CEVersion) /d "$(CEConfigName)" /d "_WIN32_WCE_EMULATION" /d "INTERNATIONAL" /d "USA" /d "INTLMSG_CODEPAGE" /d "NDEBUG"
CPP=cl.exe
# ADD BASE CPP /nologo /MT /W3 /Gm /Zi /Od /D "DEBUG" /D "_DEBUG" /D "_WIN32_WCE_EMULATION" /D UNDER_CE=200 /D "UNICODE" /D "_UNICODE" /D "WIN32" /D "STRICT" /D _WIN32_WCE=200 /D "INTERNATIONAL" /D "USA" /D "INTLMSG_CODEPAGE" /D "x86" /D "i486" /D "_x86_" /D "BUILD_PYWINTYPES" /YX /FD /c
# ADD CPP /nologo /MT /W3 /O1 /D "NDEBUG" /D "_WIN32_WCE_EMULATION" /D UNDER_CE=200 /D "UNICODE" /D "_UNICODE" /D "WIN32" /D "STRICT" /D _WIN32_WCE=200 /D "INTERNATIONAL" /D "USA" /D "INTLMSG_CODEPAGE" /D "x86" /D "i486" /D "_x86_" /D "BUILD_PYWINTYPES" /YX /FD /c
EMPFILE=empfile.exe
# ADD BASE EMPFILE COPY
# ADD EMPFILE COPY

!ELSEIF  "$(CFG)" == "PyWinTypes - Win32 (WCE MIPS) Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "PyWinTyp"
# PROP BASE Intermediate_Dir "PyWinTyp"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Build\ce\mips"
# PROP Intermediate_Dir "Build\temp\pywintypes\ce\mips\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
LINK32=link.exe
# ADD BASE LINK32 coredll.lib /nologo /dll /pdb:"Build\System\PyWinTypes15.pdb" /debug /machine:MIPS /out:"Build\ce\mips\PyWinTypes15.dll" /implib:"Build\ce\mips\PyWinTypes.lib" /subsystem:windowsce
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 coredll.lib ole32.lib oleaut32.lib /nologo /dll /debug /machine:MIPS /nodefaultlib:"$(CENoDefaultLib)" /out:"Build\ce\mips\PyWinTypes15_d.dll" /implib:"Build\ce\mips\PyWinTypes_d.lib" /subsystem:windowsce
# SUBTRACT LINK32 /pdb:none
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
MTL=midl.exe
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /o /win32 "NUL"
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /o /win32 "NUL"
RSC=rc.exe
# ADD BASE RSC /l 0xc09 /r /d "MIPS" /d "_MIPS_" /d UNDER_CE=$(CEVersion) /d _WIN32_WCE=$(CEVersion) /d "$(CEConfigName)" /d "UNICODE" /d "NDEBUG"
# ADD RSC /l 0xc09 /r /d "MIPS" /d "_MIPS_" /d UNDER_CE=$(CEVersion) /d _WIN32_WCE=$(CEVersion) /d "$(CEConfigName)" /d "UNICODE" /d "NDEBUG"
CPP=clmips.exe
# ADD BASE CPP /nologo /MT /W3 /Zi /O1 /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "NDEBUG" /D "MIPS" /D "_MIPS_" /D UNDER_CE=200 /D "UNICODE" /D "BUILD_PYWINTYPES" /D "STRICT" /YX /FD /c
# ADD CPP /nologo /M$(CECrtMT) /W3 /Zi /Od /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "DEBUG" /D "_DEBUG" /D "MIPS" /D "_MIPS_" /D UNDER_CE=200 /D "UNICODE" /D "BUILD_PYWINTYPES" /D "STRICT" /YX /FD /c
PFILE=pfile.exe
# ADD BASE PFILE COPY
# ADD PFILE COPY

!ELSEIF  "$(CFG)" == "PyWinTypes - Win32 (WCE SH3) Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "PyWinTyp"
# PROP BASE Intermediate_Dir "PyWinTyp"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Build\ce\sh"
# PROP Intermediate_Dir "Build\Temp\pywintypes\ce\sh\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
LINK32=link.exe
# ADD BASE LINK32 commctrl.lib coredll.lib /nologo /dll /debug /machine:SH3 /out:"Build\ce\mips\PyWinTypes15_d.dll" /implib:"Build\ce\mips\PyWinTypes_d.lib" /subsystem:windowsce
# SUBTRACT BASE LINK32 /pdb:none /nodefaultlib
# ADD LINK32 commctrl.lib coredll.lib /nologo /dll /debug /machine:SH3 /nodefaultlib:"$(CENoDefaultLib)" /out:"Build\ce\sh\PyWinTypes15_d.dll" /implib:"Build\ce\sh\PyWinTypes_d.lib" /subsystem:windowsce
# SUBTRACT LINK32 /pdb:none
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
MTL=midl.exe
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /o /win32 "NUL"
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /o /win32 "NUL"
RSC=rc.exe
# ADD BASE RSC /l 0xc09 /r /d "SHx" /d "SH3" /d "_SH3_" /d UNDER_CE=$(CEVersion) /d _WIN32_WCE=$(CEVersion) /d "$(CEConfigName)" /d "UNICODE" /d "NDEBUG"
# ADD RSC /l 0xc09 /r /d "SHx" /d "SH3" /d "_SH3_" /d UNDER_CE=$(CEVersion) /d _WIN32_WCE=$(CEVersion) /d "$(CEConfigName)" /d "UNICODE" /d "NDEBUG"
CPP=shcl.exe
# ADD BASE CPP /nologo /ML /W3 /Zi /Od /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "SHx" /D "SH3" /D "_SH3_" /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "DEBUG" /YX /FD /c
# ADD CPP /nologo /M$(CECrtMT) /W3 /Zi /Od /D "BUILD_PYWINTYPES" /D "STRICT" /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "DEBUG" /D "_DEBUG" /D "SHx" /D "SH3" /D "_SH3_" /D UNDER_CE=$(CEVersion) /D "UNICODE" /YX /FD /c
PFILE=pfile.exe
# ADD BASE PFILE COPY
# ADD PFILE COPY

!ELSEIF  "$(CFG)" == "PyWinTypes - Win32 (WCE SH3) Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "PyWinTy0"
# PROP BASE Intermediate_Dir "PyWinTy0"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Build\ce\sh"
# PROP Intermediate_Dir "Build\Temp\pywintypes\ce\sh\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
LINK32=link.exe
# ADD BASE LINK32 commctrl.lib coredll.lib /nologo /dll /machine:SH3 /out:"Build\ce\mips\PyWinTypes15.dll" /implib:"Build\ce\mips\PyWinTypes.lib" /subsystem:windowsce,1.0
# SUBTRACT BASE LINK32 /pdb:none /nodefaultlib
# ADD LINK32 commctrl.lib coredll.lib /nologo /dll /machine:SH3 /nodefaultlib:"$(CENoDefaultLib)" /out:"Build\ce\sh\PyWinTypes15.dll" /implib:"Build\ce\sh\PyWinTypes.lib" /subsystem:windowsce,1.0
# SUBTRACT LINK32 /pdb:none
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
MTL=midl.exe
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /o /win32 "NUL"
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /o /win32 "NUL"
RSC=rc.exe
# ADD BASE RSC /l 0xc09 /r /d "SHx" /d "SH3" /d "_SH3_" /d UNDER_CE=$(CEVersion) /d _WIN32_WCE=$(CEVersion) /d "$(CEConfigName)" /d "UNICODE" /d "NDEBUG"
# ADD RSC /l 0xc09 /r /d "SHx" /d "SH3" /d "_SH3_" /d UNDER_CE=$(CEVersion) /d _WIN32_WCE=$(CEVersion) /d "$(CEConfigName)" /d "UNICODE" /d "NDEBUG"
CPP=shcl.exe
# ADD BASE CPP /nologo /ML /W3 /O1 /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "NDEBUG" /D "SHx" /D "SH3" /D "_SH3_" /D "UNICODE" /D UNDER_CE=200 /YX /FD /c
# ADD CPP /nologo /M$(CECrtMT) /W3 /O1 /D "BUILD_PYWINTYPES" /D "STRICT" /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "NDEBUG" /D "SHx" /D "SH3" /D "_SH3_" /D "UNICODE" /D UNDER_CE=200 /YX /FD /c
PFILE=pfile.exe
# ADD BASE PFILE COPY
# ADD PFILE COPY

!ENDIF 

# Begin Target

# Name "PyWinTypes - Win32 Release"
# Name "PyWinTypes - Win32 Debug"
# Name "PyWinTypes - Win32 (WCE MIPS) Release"
# Name "PyWinTypes - Win32 (WCE x86em) Debug"
# Name "PyWinTypes - Win32 (WCE x86em) Release"
# Name "PyWinTypes - Win32 (WCE MIPS) Debug"
# Name "PyWinTypes - Win32 (WCE SH3) Debug"
# Name "PyWinTypes - Win32 (WCE SH3) Release"
# Begin Source File

SOURCE=.\src\PyACL.cpp

!IF  "$(CFG)" == "PyWinTypes - Win32 Release"

!ELSEIF  "$(CFG)" == "PyWinTypes - Win32 Debug"

!ELSEIF  "$(CFG)" == "PyWinTypes - Win32 (WCE MIPS) Release"

DEP_CPP_PYACL=\
	".\src\PySecurityObjects.h"\
	".\src\PyWinObjects.h"\
	
NODEP_CPP_PYACL=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
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
	".\val.h"\
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
	

!ELSEIF  "$(CFG)" == "PyWinTypes - Win32 (WCE x86em) Debug"

!ELSEIF  "$(CFG)" == "PyWinTypes - Win32 (WCE x86em) Release"

!ELSEIF  "$(CFG)" == "PyWinTypes - Win32 (WCE MIPS) Debug"

DEP_CPP_PYACL=\
	".\src\PySecurityObjects.h"\
	".\src\PyWinObjects.h"\
	
NODEP_CPP_PYACL=\
	".\src\Python.h"\
	".\src\unicodeobject.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "PyWinTypes - Win32 (WCE SH3) Debug"

DEP_CPP_PYACL=\
	".\src\PySecurityObjects.h"\
	".\src\PyWinObjects.h"\
	
NODEP_CPP_PYACL=\
	".\src\Python.h"\
	".\src\unicodeobject.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "PyWinTypes - Win32 (WCE SH3) Release"

DEP_CPP_PYACL=\
	".\src\PySecurityObjects.h"\
	".\src\PyWinObjects.h"\
	
NODEP_CPP_PYACL=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
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
	".\val.h"\
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

SOURCE=.\src\PyHANDLE.cpp

!IF  "$(CFG)" == "PyWinTypes - Win32 Release"

!ELSEIF  "$(CFG)" == "PyWinTypes - Win32 Debug"

!ELSEIF  "$(CFG)" == "PyWinTypes - Win32 (WCE MIPS) Release"

DEP_CPP_PYHAN=\
	".\src\PyWinObjects.h"\
	
NODEP_CPP_PYHAN=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
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
	".\val.h"\
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
	

!ELSEIF  "$(CFG)" == "PyWinTypes - Win32 (WCE x86em) Debug"

!ELSEIF  "$(CFG)" == "PyWinTypes - Win32 (WCE x86em) Release"

!ELSEIF  "$(CFG)" == "PyWinTypes - Win32 (WCE MIPS) Debug"

DEP_CPP_PYHAN=\
	".\src\PyWinObjects.h"\
	
NODEP_CPP_PYHAN=\
	".\src\Python.h"\
	".\src\structmember.h"\
	".\src\unicodeobject.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "PyWinTypes - Win32 (WCE SH3) Debug"

DEP_CPP_PYHAN=\
	".\src\PyWinObjects.h"\
	
NODEP_CPP_PYHAN=\
	".\src\Python.h"\
	".\src\structmember.h"\
	".\src\unicodeobject.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "PyWinTypes - Win32 (WCE SH3) Release"

DEP_CPP_PYHAN=\
	".\src\PyWinObjects.h"\
	
NODEP_CPP_PYHAN=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
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
	".\val.h"\
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

SOURCE=.\src\PyIID.cpp

!IF  "$(CFG)" == "PyWinTypes - Win32 Release"

!ELSEIF  "$(CFG)" == "PyWinTypes - Win32 Debug"

!ELSEIF  "$(CFG)" == "PyWinTypes - Win32 (WCE MIPS) Release"

DEP_CPP_PYIID=\
	".\src\PyWinObjects.h"\
	
NODEP_CPP_PYIID=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
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
	".\val.h"\
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
	

!ELSEIF  "$(CFG)" == "PyWinTypes - Win32 (WCE x86em) Debug"

!ELSEIF  "$(CFG)" == "PyWinTypes - Win32 (WCE x86em) Release"

!ELSEIF  "$(CFG)" == "PyWinTypes - Win32 (WCE MIPS) Debug"

DEP_CPP_PYIID=\
	".\src\PyWinObjects.h"\
	
NODEP_CPP_PYIID=\
	".\src\Python.h"\
	".\src\unicodeobject.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "PyWinTypes - Win32 (WCE SH3) Debug"

DEP_CPP_PYIID=\
	".\src\PyWinObjects.h"\
	
NODEP_CPP_PYIID=\
	".\src\Python.h"\
	".\src\unicodeobject.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "PyWinTypes - Win32 (WCE SH3) Release"

DEP_CPP_PYIID=\
	".\src\PyWinObjects.h"\
	
NODEP_CPP_PYIID=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
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
	".\val.h"\
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

SOURCE=.\src\PyLARGE_INTEGER.cpp

!IF  "$(CFG)" == "PyWinTypes - Win32 Release"

!ELSEIF  "$(CFG)" == "PyWinTypes - Win32 Debug"

!ELSEIF  "$(CFG)" == "PyWinTypes - Win32 (WCE MIPS) Release"

NODEP_CPP_PYLAR=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
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
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongintrepr.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\tringobject.h"\
	".\ufferobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\val.h"\
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
	

!ELSEIF  "$(CFG)" == "PyWinTypes - Win32 (WCE x86em) Debug"

!ELSEIF  "$(CFG)" == "PyWinTypes - Win32 (WCE x86em) Release"

!ELSEIF  "$(CFG)" == "PyWinTypes - Win32 (WCE MIPS) Debug"

NODEP_CPP_PYLAR=\
	".\src\longintrepr.h"\
	".\src\Python.h"\
	".\src\unicodeobject.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "PyWinTypes - Win32 (WCE SH3) Debug"

NODEP_CPP_PYLAR=\
	".\src\longintrepr.h"\
	".\src\Python.h"\
	".\src\unicodeobject.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "PyWinTypes - Win32 (WCE SH3) Release"

NODEP_CPP_PYLAR=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
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
	".\odsupport.h"\
	".\oduleobject.h"\
	".\omplexobject.h"\
	".\onfig.h"\
	".\ongintrepr.h"\
	".\ongobject.h"\
	".\raceback.h"\
	".\tringobject.h"\
	".\ufferobject.h"\
	".\uncobject.h"\
	".\upleobject.h"\
	".\val.h"\
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

SOURCE=.\src\PyOVERLAPPED.cpp

!IF  "$(CFG)" == "PyWinTypes - Win32 Release"

!ELSEIF  "$(CFG)" == "PyWinTypes - Win32 Debug"

!ELSEIF  "$(CFG)" == "PyWinTypes - Win32 (WCE MIPS) Release"

DEP_CPP_PYOVE=\
	".\src\PyWinObjects.h"\
	
NODEP_CPP_PYOVE=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
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
	".\val.h"\
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
	

!ELSEIF  "$(CFG)" == "PyWinTypes - Win32 (WCE x86em) Debug"

!ELSEIF  "$(CFG)" == "PyWinTypes - Win32 (WCE x86em) Release"

!ELSEIF  "$(CFG)" == "PyWinTypes - Win32 (WCE MIPS) Debug"

DEP_CPP_PYOVE=\
	".\src\PyWinObjects.h"\
	
NODEP_CPP_PYOVE=\
	".\src\Python.h"\
	".\src\structmember.h"\
	".\src\unicodeobject.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "PyWinTypes - Win32 (WCE SH3) Debug"

DEP_CPP_PYOVE=\
	".\src\PyWinObjects.h"\
	
NODEP_CPP_PYOVE=\
	".\src\Python.h"\
	".\src\structmember.h"\
	".\src\unicodeobject.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "PyWinTypes - Win32 (WCE SH3) Release"

DEP_CPP_PYOVE=\
	".\src\PyWinObjects.h"\
	
NODEP_CPP_PYOVE=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
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
	".\val.h"\
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

SOURCE=.\src\PySECURITY_ATTRIBUTES.cpp

!IF  "$(CFG)" == "PyWinTypes - Win32 Release"

!ELSEIF  "$(CFG)" == "PyWinTypes - Win32 Debug"

!ELSEIF  "$(CFG)" == "PyWinTypes - Win32 (WCE MIPS) Release"

DEP_CPP_PYSEC=\
	".\src\PySecurityObjects.h"\
	".\src\PyWinObjects.h"\
	
NODEP_CPP_PYSEC=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
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
	".\val.h"\
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
	

!ELSEIF  "$(CFG)" == "PyWinTypes - Win32 (WCE x86em) Debug"

!ELSEIF  "$(CFG)" == "PyWinTypes - Win32 (WCE x86em) Release"

!ELSEIF  "$(CFG)" == "PyWinTypes - Win32 (WCE MIPS) Debug"

DEP_CPP_PYSEC=\
	".\src\PySecurityObjects.h"\
	".\src\PyWinObjects.h"\
	
NODEP_CPP_PYSEC=\
	".\src\Python.h"\
	".\src\structmember.h"\
	".\src\unicodeobject.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "PyWinTypes - Win32 (WCE SH3) Debug"

DEP_CPP_PYSEC=\
	".\src\PySecurityObjects.h"\
	".\src\PyWinObjects.h"\
	
NODEP_CPP_PYSEC=\
	".\src\Python.h"\
	".\src\structmember.h"\
	".\src\unicodeobject.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "PyWinTypes - Win32 (WCE SH3) Release"

DEP_CPP_PYSEC=\
	".\src\PySecurityObjects.h"\
	".\src\PyWinObjects.h"\
	
NODEP_CPP_PYSEC=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
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
	".\val.h"\
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

SOURCE=.\src\PySECURITY_DESCRIPTOR.cpp

!IF  "$(CFG)" == "PyWinTypes - Win32 Release"

!ELSEIF  "$(CFG)" == "PyWinTypes - Win32 Debug"

!ELSEIF  "$(CFG)" == "PyWinTypes - Win32 (WCE MIPS) Release"

DEP_CPP_PYSECU=\
	".\src\PySecurityObjects.h"\
	".\src\PyWinObjects.h"\
	
NODEP_CPP_PYSECU=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
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
	".\val.h"\
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
	

!ELSEIF  "$(CFG)" == "PyWinTypes - Win32 (WCE x86em) Debug"

!ELSEIF  "$(CFG)" == "PyWinTypes - Win32 (WCE x86em) Release"

!ELSEIF  "$(CFG)" == "PyWinTypes - Win32 (WCE MIPS) Debug"

DEP_CPP_PYSECU=\
	".\src\PySecurityObjects.h"\
	".\src\PyWinObjects.h"\
	
NODEP_CPP_PYSECU=\
	".\src\Python.h"\
	".\src\structmember.h"\
	".\src\unicodeobject.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "PyWinTypes - Win32 (WCE SH3) Debug"

DEP_CPP_PYSECU=\
	".\src\PySecurityObjects.h"\
	".\src\PyWinObjects.h"\
	
NODEP_CPP_PYSECU=\
	".\src\Python.h"\
	".\src\structmember.h"\
	".\src\unicodeobject.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "PyWinTypes - Win32 (WCE SH3) Release"

DEP_CPP_PYSECU=\
	".\src\PySecurityObjects.h"\
	".\src\PyWinObjects.h"\
	
NODEP_CPP_PYSECU=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
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
	".\val.h"\
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

SOURCE=.\src\PySecurityObjects.h
# End Source File
# Begin Source File

SOURCE=.\src\PySID.cpp

!IF  "$(CFG)" == "PyWinTypes - Win32 Release"

!ELSEIF  "$(CFG)" == "PyWinTypes - Win32 Debug"

!ELSEIF  "$(CFG)" == "PyWinTypes - Win32 (WCE MIPS) Release"

DEP_CPP_PYSID=\
	".\src\PySecurityObjects.h"\
	".\src\PyWinObjects.h"\
	
NODEP_CPP_PYSID=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
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
	".\val.h"\
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
	

!ELSEIF  "$(CFG)" == "PyWinTypes - Win32 (WCE x86em) Debug"

!ELSEIF  "$(CFG)" == "PyWinTypes - Win32 (WCE x86em) Release"

!ELSEIF  "$(CFG)" == "PyWinTypes - Win32 (WCE MIPS) Debug"

DEP_CPP_PYSID=\
	".\src\PySecurityObjects.h"\
	".\src\PyWinObjects.h"\
	
NODEP_CPP_PYSID=\
	".\src\Python.h"\
	".\src\unicodeobject.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "PyWinTypes - Win32 (WCE SH3) Debug"

DEP_CPP_PYSID=\
	".\src\PySecurityObjects.h"\
	".\src\PyWinObjects.h"\
	
NODEP_CPP_PYSID=\
	".\src\Python.h"\
	".\src\unicodeobject.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "PyWinTypes - Win32 (WCE SH3) Release"

DEP_CPP_PYSID=\
	".\src\PySecurityObjects.h"\
	".\src\PyWinObjects.h"\
	
NODEP_CPP_PYSID=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
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
	".\val.h"\
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

SOURCE=.\src\PyTime.cpp

!IF  "$(CFG)" == "PyWinTypes - Win32 Release"

!ELSEIF  "$(CFG)" == "PyWinTypes - Win32 Debug"

!ELSEIF  "$(CFG)" == "PyWinTypes - Win32 (WCE MIPS) Release"

DEP_CPP_PYTIM=\
	".\src\PyWinObjects.h"\
	
NODEP_CPP_PYTIM=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
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
	".\val.h"\
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
	

!ELSEIF  "$(CFG)" == "PyWinTypes - Win32 (WCE x86em) Debug"

!ELSEIF  "$(CFG)" == "PyWinTypes - Win32 (WCE x86em) Release"

!ELSEIF  "$(CFG)" == "PyWinTypes - Win32 (WCE MIPS) Debug"

DEP_CPP_PYTIM=\
	".\src\PyWinObjects.h"\
	
NODEP_CPP_PYTIM=\
	".\src\Python.h"\
	".\src\unicodeobject.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "PyWinTypes - Win32 (WCE SH3) Debug"

DEP_CPP_PYTIM=\
	".\src\PyWinObjects.h"\
	
NODEP_CPP_PYTIM=\
	".\src\Python.h"\
	".\src\unicodeobject.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "PyWinTypes - Win32 (WCE SH3) Release"

DEP_CPP_PYTIM=\
	".\src\PyWinObjects.h"\
	
NODEP_CPP_PYTIM=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
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
	".\val.h"\
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

SOURCE=.\src\PyUnicode.cpp

!IF  "$(CFG)" == "PyWinTypes - Win32 Release"

!ELSEIF  "$(CFG)" == "PyWinTypes - Win32 Debug"

!ELSEIF  "$(CFG)" == "PyWinTypes - Win32 (WCE MIPS) Release"

DEP_CPP_PYUNI=\
	".\src\PyWinObjects.h"\
	
NODEP_CPP_PYUNI=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
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
	".\val.h"\
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
	

!ELSEIF  "$(CFG)" == "PyWinTypes - Win32 (WCE x86em) Debug"

!ELSEIF  "$(CFG)" == "PyWinTypes - Win32 (WCE x86em) Release"

!ELSEIF  "$(CFG)" == "PyWinTypes - Win32 (WCE MIPS) Debug"

DEP_CPP_PYUNI=\
	".\src\PyWinObjects.h"\
	
NODEP_CPP_PYUNI=\
	".\src\Python.h"\
	".\src\unicodeobject.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "PyWinTypes - Win32 (WCE SH3) Debug"

DEP_CPP_PYUNI=\
	".\src\PyWinObjects.h"\
	
NODEP_CPP_PYUNI=\
	".\src\Python.h"\
	".\src\unicodeobject.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "PyWinTypes - Win32 (WCE SH3) Release"

DEP_CPP_PYUNI=\
	".\src\PyWinObjects.h"\
	
NODEP_CPP_PYUNI=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
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
	".\val.h"\
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

SOURCE=.\src\PyWinObjects.h
# End Source File
# Begin Source File

SOURCE=.\src\PyWinTypes.h
# End Source File
# Begin Source File

SOURCE=.\src\PyWinTypesmodule.cpp

!IF  "$(CFG)" == "PyWinTypes - Win32 Release"

!ELSEIF  "$(CFG)" == "PyWinTypes - Win32 Debug"

!ELSEIF  "$(CFG)" == "PyWinTypes - Win32 (WCE MIPS) Release"

DEP_CPP_PYWIN=\
	".\src\PySecurityObjects.h"\
	".\src\PyWinObjects.h"\
	
NODEP_CPP_PYWIN=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
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
	".\val.h"\
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
	

!ELSEIF  "$(CFG)" == "PyWinTypes - Win32 (WCE x86em) Debug"

!ELSEIF  "$(CFG)" == "PyWinTypes - Win32 (WCE x86em) Release"

!ELSEIF  "$(CFG)" == "PyWinTypes - Win32 (WCE MIPS) Debug"

DEP_CPP_PYWIN=\
	".\src\PySecurityObjects.h"\
	".\src\PyWinObjects.h"\
	
NODEP_CPP_PYWIN=\
	".\src\Python.h"\
	".\src\unicodeobject.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "PyWinTypes - Win32 (WCE SH3) Debug"

DEP_CPP_PYWIN=\
	".\src\PySecurityObjects.h"\
	".\src\PyWinObjects.h"\
	
NODEP_CPP_PYWIN=\
	".\src\Python.h"\
	".\src\unicodeobject.h"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "PyWinTypes - Win32 (WCE SH3) Release"

DEP_CPP_PYWIN=\
	".\src\PySecurityObjects.h"\
	".\src\PyWinObjects.h"\
	
NODEP_CPP_PYWIN=\
	".\angeobject.h"\
	".\atchlevel.h"\
	".\bject.h"\
	".\bjimpl.h"\
	".\bstract.h"\
	".\e\crtapi.h"\
	".\ethodobject.h"\
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
	".\val.h"\
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
# End Target
# End Project
