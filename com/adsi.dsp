# Microsoft Developer Studio Project File - Name="adsi" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=adsi - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "adsi.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "adsi.mak" CFG="adsi - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "adsi - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "adsi - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "adsi - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Build"
# PROP Intermediate_Dir "Build\Temp\adsi\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
F90=df.exe
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "ADSI_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /Zi /O2 /I "..\win32\src" /I "..\com\win32com\src\include" /D "NDEBUG" /D "__WIN32__" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "ADSI_EXPORTS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0xc09 /d "NDEBUG"
# ADD RSC /l 0xc09 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 ACTIVEDS.LIB ADSIID.LIB kernel32.lib user32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /base:"0x1e7f0000" /dll /debug /machine:I386 /out:"Build\adsi.pyd" /libpath:"..\win32\build"

!ELSEIF  "$(CFG)" == "adsi - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Build"
# PROP Intermediate_Dir "Build\Temp\adsi\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
F90=df.exe
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "ADSI_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\win32\src" /I "..\com\win32com\src\include" /D "_DEBUG" /D "__WIN32__" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "ADSI_EXPORTS" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0xc09 /d "_DEBUG"
# ADD RSC /l 0xc09 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 ACTIVEDS.LIB ADSIID.LIB kernel32.lib user32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /base:"0x1e7f0000" /dll /debug /machine:I386 /out:"Build\adsi_d.pyd" /pdbtype:sept /libpath:"..\win32\build"

!ENDIF 

# Begin Target

# Name "adsi - Win32 Release"
# Name "adsi - Win32 Debug"
# Begin Group "Swigged"

# PROP Default_Filter "*.cpp"
# Begin Source File

SOURCE=.\win32comext\adsi\src\adsi.cpp
# End Source File
# Begin Source File

SOURCE=.\win32comext\adsi\src\PyIADsContainer.cpp
# End Source File
# Begin Source File

SOURCE=.\win32comext\adsi\src\PyIADsUser.cpp
# End Source File
# Begin Source File

SOURCE=.\win32comext\adsi\src\PyIDirectoryObject.cpp
# End Source File
# Begin Source File

SOURCE=.\win32comext\adsi\src\PyIDirectorySearch.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\win32comext\adsi\src\adsi.i

!IF  "$(CFG)" == "adsi - Win32 Release"

# Begin Custom Build - Invoking SWIG on $(InputPath)
InputDir=.\win32comext\adsi\src
InputPath=.\win32comext\adsi\src\adsi.i
InputName=adsi

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\swig.bat $(InputDir) -dnone -python -c++ -o $(InputName).cpp $(InputName).i

# End Custom Build

!ELSEIF  "$(CFG)" == "adsi - Win32 Debug"

# Begin Custom Build - Invoking SWIG on $(InputPath)
InputDir=.\win32comext\adsi\src
InputPath=.\win32comext\adsi\src\adsi.i
InputName=adsi

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\swig.bat $(InputDir) -dnone -python -c++ -o $(InputName).cpp $(InputName).i

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32comext\adsi\src\adsilib.i
# End Source File
# Begin Source File

SOURCE=.\win32comext\adsi\src\PyADSIUtil.cpp
# End Source File
# Begin Source File

SOURCE=.\win32comext\adsi\src\PyADSIUtil.h
# End Source File
# Begin Source File

SOURCE=.\win32comext\adsi\src\PyIADsContainer.i

!IF  "$(CFG)" == "adsi - Win32 Release"

# Begin Custom Build - Invoking SWIG on $(InputPath)
InputDir=.\win32comext\adsi\src
InputPath=.\win32comext\adsi\src\PyIADsContainer.i
InputName=PyIADsContainer

BuildCmds= \
	..\swig.bat $(InputDir) -dnone -pythoncom -com_interface_parent IDispatch -c++ -o $(InputName).cpp  $(InputName).i

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "adsi - Win32 Debug"

# Begin Custom Build - Invoking SWIG on $(InputPath)
InputDir=.\win32comext\adsi\src
InputPath=.\win32comext\adsi\src\PyIADsContainer.i
InputName=PyIADsContainer

BuildCmds= \
	..\swig.bat $(InputDir) -dnone -pythoncom -com_interface_parent IDispatch -c++ -o $(InputName).cpp  $(InputName).i

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32comext\adsi\src\PyIADsUser.i

!IF  "$(CFG)" == "adsi - Win32 Release"

# Begin Custom Build - Invoking SWIG on $(InputPath)
InputDir=.\win32comext\adsi\src
InputPath=.\win32comext\adsi\src\PyIADsUser.i
InputName=PyIADsUser

BuildCmds= \
	..\swig.bat $(InputDir) -dnone -pythoncom -com_interface_parent IDispatch -c++ -o $(InputName).cpp  $(InputName).i

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "adsi - Win32 Debug"

# Begin Custom Build - Invoking SWIG on $(InputPath)
InputDir=.\win32comext\adsi\src
InputPath=.\win32comext\adsi\src\PyIADsUser.i
InputName=PyIADsUser

BuildCmds= \
	..\swig.bat $(InputDir) -dnone -pythoncom -com_interface_parent IDispatch -c++ -o $(InputName).cpp  $(InputName).i

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32comext\adsi\src\PyIDirectoryObject.i

!IF  "$(CFG)" == "adsi - Win32 Release"

USERDEP__PYIDI="win32comext\adsi\src\adsilib.i"	
# Begin Custom Build - Invoking SWIG on $(InputPath)
InputDir=.\win32comext\adsi\src
InputPath=.\win32comext\adsi\src\PyIDirectoryObject.i
InputName=PyIDirectoryObject

BuildCmds= \
	..\swig.bat $(InputDir)  -dnone -pythoncom -c++ -o $(InputName).cpp  $(InputName).i

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "adsi - Win32 Debug"

USERDEP__PYIDI="win32comext\adsi\src\adsilib.i"	
# Begin Custom Build - Invoking SWIG on $(InputPath)
InputDir=.\win32comext\adsi\src
InputPath=.\win32comext\adsi\src\PyIDirectoryObject.i
InputName=PyIDirectoryObject

BuildCmds= \
	..\swig.bat $(InputDir)  -dnone -pythoncom -c++ -o $(InputName).cpp  $(InputName).i

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32comext\adsi\src\PyIDirectorySearch.i
# End Source File
# End Target
# End Project
