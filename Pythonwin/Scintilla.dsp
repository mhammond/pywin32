# Microsoft Developer Studio Project File - Name="Scintilla" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) External Target" 0x0106

CFG=Scintilla - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Scintilla.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Scintilla.mak" CFG="Scintilla - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Scintilla - Win32 Release" (based on "Win32 (x86) External Target")
!MESSAGE "Scintilla - Win32 Debug" (based on "Win32 (x86) External Target")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""

!IF  "$(CFG)" == "Scintilla - Win32 Release"

# PROP BASE Use_MFC
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Build"
# PROP BASE Intermediate_Dir "Build\Temp\Scintilla\Release"
# PROP BASE Cmd_Line "NMAKE /f Scintilla.mak"
# PROP BASE Rebuild_Opt "/a"
# PROP BASE Target_File "Scintilla.exe"
# PROP BASE Bsc_Name "Scintilla.bsc"
# PROP BASE Target_Dir ""
# PROP Use_MFC
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Build"
# PROP Intermediate_Dir "Build\Temp\Scintilla\Release"
# PROP Cmd_Line "cd Scintilla && nmake /nologo /f makefile_pythonwin"
# PROP Rebuild_Opt "/a"
# PROP Target_File "Build\Scintilla.dll"
# PROP Bsc_Name ""
# PROP Target_Dir ""

!ELSEIF  "$(CFG)" == "Scintilla - Win32 Debug"

# PROP BASE Use_MFC
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Build"
# PROP BASE Intermediate_Dir "Build\Temp\Scintilla\Debug"
# PROP BASE Cmd_Line "NMAKE /f Scintilla.mak"
# PROP BASE Rebuild_Opt "/a"
# PROP BASE Target_File "Scintilla.exe"
# PROP BASE Bsc_Name "Scintilla.bsc"
# PROP BASE Target_Dir ""
# PROP Use_MFC
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Build"
# PROP Intermediate_Dir "Build\Temp\Scintilla\Debug"
# PROP Cmd_Line "cd Scintilla && nmake /f makefile_pythonwin DEBUG=1"
# PROP Rebuild_Opt "/a"
# PROP Target_File "Build\Scintilla_d.dll"
# PROP Bsc_Name ""
# PROP Target_Dir ""

!ENDIF 

# Begin Target

# Name "Scintilla - Win32 Release"
# Name "Scintilla - Win32 Debug"

!IF  "$(CFG)" == "Scintilla - Win32 Release"

!ELSEIF  "$(CFG)" == "Scintilla - Win32 Debug"

!ENDIF 

# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
