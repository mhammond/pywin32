# Microsoft Developer Studio Project File - Name="win32process" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (WCE x86em) Dynamic-Link Library" 0x7f02
# TARGTYPE "Win32 (WCE SH3) Dynamic-Link Library" 0x8102
# TARGTYPE "Win32 (WCE MIPS) Dynamic-Link Library" 0x8202
# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=win32process - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "win32process.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "win32process.mak" CFG="win32process - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "win32process - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "win32process - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "win32process - Win32 (WCE MIPS) Release" (based on "Win32 (WCE MIPS) Dynamic-Link Library")
!MESSAGE "win32process - Win32 (WCE MIPS) Debug" (based on "Win32 (WCE MIPS) Dynamic-Link Library")
!MESSAGE "win32process - Win32 (WCE SH3) Debug" (based on "Win32 (WCE SH3) Dynamic-Link Library")
!MESSAGE "win32process - Win32 (WCE SH3) Release" (based on "Win32 (WCE SH3) Dynamic-Link Library")
!MESSAGE "win32process - Win32 (WCE x86em) Debug" (based on "Win32 (WCE x86em) Dynamic-Link Library")
!MESSAGE "win32process - Win32 (WCE x86em) Release" (based on "Win32 (WCE x86em) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath "H/PC Ver. 2.00"

!IF  "$(CFG)" == "win32process - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Build"
# PROP BASE Intermediate_Dir "Build\Temp\win32process\Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Build"
# PROP Intermediate_Dir "Build\Temp\win32process\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
F90=df.exe
CPP=cl.exe
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /Zi /O2 /D "WIN32" /D "_WINDOWS" /D "__WIN32__" /D "NDEBUG" /D "STRICT" /YX /FD /c
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
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /base:"0x1e700000" /subsystem:windows /dll /debug /machine:I386 /out:"Build\win32process.pyd" /libpath:".\Build"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "win32process - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Build"
# PROP BASE Intermediate_Dir "Build\Temp\win32process\Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Build"
# PROP Intermediate_Dir "Build\Temp\win32process\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
F90=df.exe
CPP=cl.exe
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MDd /W3 /GX /ZI /Od /D "WIN32" /D "_WINDOWS" /D "__WIN32__" /D "_DEBUG" /D "STRICT" /YX /FD /c
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
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /base:"0x1e700000" /subsystem:windows /dll /debug /machine:I386 /out:"Build\win32process_d.pyd" /pdbtype:sept /libpath:".\Build"

!ELSEIF  "$(CFG)" == "win32process - Win32 (WCE MIPS) Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "win32pro"
# PROP BASE Intermediate_Dir "win32pro"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Build\ce\mips"
# PROP Intermediate_Dir "Build\Temp\win32process\ce\mips\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib commctrl.lib coredll.lib /nologo /dll /debug /machine:MIPS /out:"Build\win32process.pyd"
# SUBTRACT BASE LINK32 /pdb:none /nodefaultlib
# ADD LINK32 coredll.lib /nologo /dll /machine:MIPS /nodefaultlib:"$(CENoDefaultLib)" /out:"Build\ce\mips\win32process.pyd" /subsystem:$(CESubsystem)
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
# ADD BASE CPP /nologo /ML /W3 /Zi /O2 /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "NDEBUG" /D "MIPS" /D "_MIPS_" /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "__WIN32__" /D "STRICT" /YX /FD /c
# ADD CPP /nologo /M$(CECrtMT) /W3 /O1 /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "NDEBUG" /D "MIPS" /D "_MIPS_" /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "__WIN32__" /D "STRICT" /YX /FD /c
PFILE=pfile.exe
# ADD BASE PFILE COPY
# ADD PFILE COPY

!ELSEIF  "$(CFG)" == "win32process - Win32 (WCE MIPS) Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "win32pr0"
# PROP BASE Intermediate_Dir "win32pr0"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Build\ce\mips"
# PROP Intermediate_Dir "Build\Temp\win32process\ce\mips\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib commctrl.lib coredll.lib /nologo /dll /debug /machine:MIPS /out:"Build\win32process_d.pyd" /pdbtype:sept
# SUBTRACT BASE LINK32 /pdb:none /nodefaultlib
# ADD LINK32 coredll.lib /nologo /dll /debug /machine:MIPS /nodefaultlib:"$(CENoDefaultLib)" /out:"Build\ce\mips\win32process_d.pyd" /pdbtype:sept /subsystem:$(CESubsystem)
# SUBTRACT LINK32 /pdb:none
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
MTL=midl.exe
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /o /win32 "NUL"
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /o /win32 "NUL"
RSC=rc.exe
# ADD BASE RSC /l 0xc09 /r /d "MIPS" /d "_MIPS_" /d UNDER_CE=$(CEVersion) /d _WIN32_WCE=$(CEVersion) /d "$(CEConfigName)" /d "UNICODE" /d "DEBUG"
# ADD RSC /l 0xc09 /r /d "MIPS" /d "_MIPS_" /d UNDER_CE=$(CEVersion) /d _WIN32_WCE=$(CEVersion) /d "$(CEConfigName)" /d "UNICODE" /d "DEBUG"
CPP=clmips.exe
# ADD BASE CPP /nologo /MLd /W3 /ZI /Od /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "DEBUG" /D "MIPS" /D "_MIPS_" /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "__WIN32__" /D "STRICT" /YX /FD /c
# ADD CPP /nologo /M$(CECrtDebug) /W3 /Zi /Od /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D UNDER_CE=$(CEVersion) /D "__WIN32__" /D "STRICT" /D "_DEBUG" /D "DEBUG" /D "MIPS" /D "_MIPS_" /D "UNICODE" /YX /FD /c
PFILE=pfile.exe
# ADD BASE PFILE COPY
# ADD PFILE COPY

!ELSEIF  "$(CFG)" == "win32process - Win32 (WCE SH3) Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "win32pr1"
# PROP BASE Intermediate_Dir "win32pr1"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Build\ce\sh"
# PROP Intermediate_Dir "Build\Temp\win32process\ce\sh\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib commctrl.lib coredll.lib /nologo /dll /debug /machine:SH3 /out:"Build\win32process_d.pyd" /pdbtype:sept
# SUBTRACT BASE LINK32 /pdb:none /nodefaultlib
# ADD LINK32 coredll.lib /nologo /dll /debug /machine:SH3 /nodefaultlib:"$(CENoDefaultLib)" /out:"Build\ce\sh\win32process_d.pyd" /pdbtype:sept /subsystem:$(CESubsystem)
# SUBTRACT LINK32 /pdb:none
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
MTL=midl.exe
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /o /win32 "NUL"
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /o /win32 "NUL"
RSC=rc.exe
# ADD BASE RSC /l 0xc09 /r /d "SHx" /d "SH3" /d "_SH3_" /d UNDER_CE=$(CEVersion) /d _WIN32_WCE=$(CEVersion) /d "$(CEConfigName)" /d "UNICODE" /d "DEBUG"
# ADD RSC /l 0xc09 /r /d "SHx" /d "SH3" /d "_SH3_" /d UNDER_CE=$(CEVersion) /d _WIN32_WCE=$(CEVersion) /d "$(CEConfigName)" /d "UNICODE" /d "DEBUG"
CPP=shcl.exe
# ADD BASE CPP /nologo /MLd /W3 /ZI /Od /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "DEBUG" /D "SHx" /D "SH3" /D "_SH3_" /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "__WIN32__" /D "STRICT" /YX /FD /c
# ADD CPP /nologo /M$(CECrtDebug) /W3 /Zi /Od /D "__WIN32__" /D "STRICT" /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "DEBUG" /D "_DEBUG" /D "SHx" /D "SH3" /D "_SH3_" /D UNDER_CE=$(CEVersion) /D "UNICODE" /YX /FD /c
PFILE=pfile.exe
# ADD BASE PFILE COPY
# ADD PFILE COPY

!ELSEIF  "$(CFG)" == "win32process - Win32 (WCE SH3) Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "win32pr2"
# PROP BASE Intermediate_Dir "win32pr2"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Build\ce\sh"
# PROP Intermediate_Dir "Build\Temp\win32process\ce\sh\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib commctrl.lib coredll.lib /nologo /dll /debug /machine:SH3 /out:"Build\win32process.pyd"
# SUBTRACT BASE LINK32 /pdb:none /nodefaultlib
# ADD LINK32 coredll.lib /nologo /dll /machine:SH3 /nodefaultlib:"$(CENoDefaultLib)" /out:"Build\ce\sh\win32process.pyd" /subsystem:$(CESubsystem)
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
# ADD BASE CPP /nologo /ML /W3 /Zi /O2 /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "NDEBUG" /D "SHx" /D "SH3" /D "_SH3_" /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "__WIN32__" /D "STRICT" /YX /FD /c
# ADD CPP /nologo /M$(CECrtMT) /W3 /O1 /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "NDEBUG" /D "SHx" /D "SH3" /D "_SH3_" /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "__WIN32__" /D "STRICT" /YX /FD /c
PFILE=pfile.exe
# ADD BASE PFILE COPY
# ADD PFILE COPY

!ELSEIF  "$(CFG)" == "win32process - Win32 (WCE x86em) Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "win32pr3"
# PROP BASE Intermediate_Dir "win32pr3"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Build\ce\x86em"
# PROP Intermediate_Dir "Build\Temp\win32process\ce\x86em\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib commctrl.lib coredll.lib /nologo /base:"0x1e700000" /subsystem:windows /dll /debug /machine:IX86 /out:"Build\win32process_d.pyd" /pdbtype:sept
# SUBTRACT BASE LINK32 /incremental:no
# ADD LINK32 coredll.lib /nologo /base:"0x1e700000" /entry:"_DllMainCRTStartup@12" /subsystem:windows /dll /debug /machine:IX86 /nodefaultlib:"$(CENoDefaultLib)" /out:"Build\ce\x86em\win32process_d.pyd" /pdbtype:sept /subsystem:$(CESubsystem)
# SUBTRACT LINK32 /pdb:none
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
MTL=midl.exe
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /o /win32 "NUL"
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /o /win32 "NUL"
RSC=rc.exe
# ADD BASE RSC /l 0xc09 /d UNDER_CE=$(CEVersion) /d "UNICODE" /d "_UNICODE" /d "WIN32" /d "STRICT" /d _WIN32_WCE=$(CEVersion) /d "$(CEConfigName)" /d "_WIN32_WCE_EMULATION" /d "INTERNATIONAL" /d "USA" /d "INTLMSG_CODEPAGE" /d "_DEBUG" /d "x86" /d "i486" /d "_x86_"
# ADD RSC /l 0xc09 /d UNDER_CE=$(CEVersion) /d "UNICODE" /d "_UNICODE" /d "WIN32" /d "STRICT" /d _WIN32_WCE=$(CEVersion) /d "$(CEConfigName)" /d "_WIN32_WCE_EMULATION" /d "INTERNATIONAL" /d "USA" /d "INTLMSG_CODEPAGE" /d "_DEBUG" /d "x86" /d "i486" /d "_x86_"
CPP=cl.exe
# ADD BASE CPP /nologo /MLd /W3 /Gm /ZI /Od /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "_UNICODE" /D "WIN32" /D "STRICT" /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "_WIN32_WCE_EMULATION" /D "INTERNATIONAL" /D "USA" /D "INTLMSG_CODEPAGE" /D "_DEBUG" /D "x86" /D "i486" /D "_x86_" /D "__WIN32__" /YX /FD /c
# ADD CPP /nologo /MT /W3 /Gm /Zi /Od /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "_UNICODE" /D "WIN32" /D "STRICT" /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "_WIN32_WCE_EMULATION" /D "INTERNATIONAL" /D "USA" /D "INTLMSG_CODEPAGE" /D "_DEBUG" /D "x86" /D "i486" /D "_x86_" /D "__WIN32__" /YX /FD /c
EMPFILE=empfile.exe
# ADD BASE EMPFILE COPY
# ADD EMPFILE COPY

!ELSEIF  "$(CFG)" == "win32process - Win32 (WCE x86em) Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "win32pr4"
# PROP BASE Intermediate_Dir "win32pr4"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Build\ce\x86em"
# PROP Intermediate_Dir "Build\Temp\win32process\ce\x86em\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib commctrl.lib coredll.lib /nologo /base:"0x1e700000" /subsystem:windows /dll /debug /machine:IX86 /out:"Build\win32process.pyd"
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib commctrl.lib coredll.lib /nologo /base:"0x1e700000" /entry:"_DllMainCRTStartup@12" /subsystem:windows /dll /machine:IX86 /nodefaultlib:"$(CENoDefaultLib)" /out:"Build\ce\x86em\win32process.pyd" /subsystem:$(CESubsystem)
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
# ADD BASE CPP /nologo /ML /W3 /Gm /Zi /O2 /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "_UNICODE" /D "WIN32" /D "STRICT" /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "_WIN32_WCE_EMULATION" /D "INTERNATIONAL" /D "USA" /D "INTLMSG_CODEPAGE" /D "NDEBUG" /D "x86" /D "i486" /D "_x86_" /D "__WIN32__" /YX /FD /c
# ADD CPP /nologo /MT /W3 /O1 /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "_UNICODE" /D "WIN32" /D "STRICT" /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "_WIN32_WCE_EMULATION" /D "INTERNATIONAL" /D "USA" /D "INTLMSG_CODEPAGE" /D "NDEBUG" /D "x86" /D "i486" /D "_x86_" /D "__WIN32__" /YX /FD /c
EMPFILE=empfile.exe
# ADD BASE EMPFILE COPY
# ADD EMPFILE COPY

!ENDIF 

# Begin Target

# Name "win32process - Win32 Release"
# Name "win32process - Win32 Debug"
# Name "win32process - Win32 (WCE MIPS) Release"
# Name "win32process - Win32 (WCE MIPS) Debug"
# Name "win32process - Win32 (WCE SH3) Debug"
# Name "win32process - Win32 (WCE SH3) Release"
# Name "win32process - Win32 (WCE x86em) Debug"
# Name "win32process - Win32 (WCE x86em) Release"
# Begin Source File

SOURCE=.\src\win32process.i

!IF  "$(CFG)" == "win32process - Win32 Release"

# Begin Custom Build - Invoking SWIG...
InputDir=.\src
InputPath=.\src\win32process.i
InputName=win32process

"$(InputDir)\$(InputName)module_win32.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\swig.bat $(InputDir) -python -c++ -o $(InputName)module_win32.cpp $(InputName).i 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "win32process - Win32 Debug"

# Begin Custom Build - Invoking SWIG...
InputDir=.\src
InputPath=.\src\win32process.i
InputName=win32process

"$(InputDir)\$(InputName)module_win32.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\swig.bat $(InputDir) -python -c++ -o $(InputName)module_win32.cpp $(InputName).i 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "win32process - Win32 (WCE MIPS) Release"

# Begin Custom Build - Invoking SWIG...
InputDir=.\src	InputPath=.\src\win32process.i	InputName=win32process	

"$(InputDir)\$(InputName)module_wince.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\swig.bat $(InputDir) -python -c++ -DMS_WINCE -o $(InputName)module_wince.cpp        $(InputName).i 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "win32process - Win32 (WCE MIPS) Debug"

# Begin Custom Build - Invoking SWIG...
InputDir=.\src	InputPath=.\src\win32process.i	InputName=win32process	

"$(InputDir)\$(InputName)module_wince.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\swig.bat $(InputDir) -python -c++ -DMS_WINCE -o $(InputName)module_wince.cpp        $(InputName).i 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "win32process - Win32 (WCE SH3) Debug"

# Begin Custom Build - Invoking SWIG...
InputDir=.\src	InputPath=.\src\win32process.i	InputName=win32process	

"$(InputDir)\$(InputName)module_wince.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\swig.bat $(InputDir) -python -c++ -DMS_WINCE -o $(InputName)module_wince.cpp        $(InputName).i 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "win32process - Win32 (WCE SH3) Release"

# Begin Custom Build - Invoking SWIG...
InputDir=.\src	InputPath=.\src\win32process.i	InputName=win32process	

"$(InputDir)\$(InputName)module_wince.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\swig.bat $(InputDir) -python -c++ -DMS_WINCE -o $(InputName)module_wince.cpp        $(InputName).i 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "win32process - Win32 (WCE x86em) Debug"

# Begin Custom Build - Invoking SWIG...
InputDir=.\src	InputPath=.\src\win32process.i	InputName=win32process	

"$(InputDir)\$(InputName)module_wince.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\swig.bat $(InputDir) -python -c++ -DMS_WINCE -o $(InputName)module_wince.cpp        $(InputName).i 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "win32process - Win32 (WCE x86em) Release"

# Begin Custom Build - Invoking SWIG...
InputDir=.\src	InputPath=.\src\win32process.i	InputName=win32process	

"$(InputDir)\$(InputName)module_wince.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\swig.bat $(InputDir) -python -c++ -DMS_WINCE -o $(InputName)module_wince.cpp        $(InputName).i 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\win32processmodule.cpp

!IF  "$(CFG)" == "win32process - Win32 Release"

!ELSEIF  "$(CFG)" == "win32process - Win32 Debug"

!ELSEIF  "$(CFG)" == "win32process - Win32 (WCE MIPS) Release"

NODEP_CPP_WIN32=\
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
	".\src\win32processmodule_win32.cpp"\
	".\src\win32processmodule_wince.cpp"\
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
	

!ELSEIF  "$(CFG)" == "win32process - Win32 (WCE MIPS) Debug"

NODEP_CPP_WIN32=\
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
	".\src\win32processmodule_win32.cpp"\
	".\src\win32processmodule_wince.cpp"\
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
	

!ELSEIF  "$(CFG)" == "win32process - Win32 (WCE SH3) Debug"

NODEP_CPP_WIN32=\
	".\src\Python.h"\
	".\src\structmember.h"\
	".\src\unicodeobject.h"\
	".\src\win32processmodule_win32.cpp"\
	".\src\win32processmodule_wince.cpp"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32process - Win32 (WCE SH3) Release"

NODEP_CPP_WIN32=\
	".\src\Python.h"\
	".\src\structmember.h"\
	".\src\unicodeobject.h"\
	".\src\win32processmodule_win32.cpp"\
	".\src\win32processmodule_wince.cpp"\
	".\yWinTypes.h"\
	

!ELSEIF  "$(CFG)" == "win32process - Win32 (WCE x86em) Debug"

!ELSEIF  "$(CFG)" == "win32process - Win32 (WCE x86em) Release"

!ENDIF 

# End Source File
# End Target
# End Project
