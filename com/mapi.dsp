# Microsoft Developer Studio Project File - Name="mapi" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=mapi - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "mapi.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "mapi.mak" CFG="mapi - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "mapi - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "mapi - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Python/com/win32comext/mapi", WHAAAAAA"
# PROP Scc_LocalPath "win32comext/mapi"
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "mapi - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Build"
# PROP Intermediate_Dir "Build\Temp\mapi\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
F90=df.exe
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /Zi /O2 /I "..\win32\src" /I "..\com\win32com\src\include" /D "__WIN32__" /D "WIN32" /D "_WINDOWS" /D "NDEBUG" /D "STRICT" /YX"PythonCOM.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o /win32 "NUL"
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o /win32 "NUL"
# ADD BASE RSC /l 0xc09 /d "NDEBUG"
# ADD RSC /l 0xc09 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 version.lib mapi32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /nodefaultlib:"libc" /out:"Build/mapi.pyd" /libpath:"..\win32\build"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "mapi - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Build"
# PROP Intermediate_Dir "Build\Temp\mapi\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
F90=df.exe
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MDd /W3 /GX /ZI /Od /I "..\win32\src" /I "..\com\win32com\src\include" /D "__WIN32__" /D "WIN32" /D "_WINDOWS" /D "_DEBUG" /D "STRICT" /YX"PythonCOM.h" /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o /win32 "NUL"
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o /win32 "NUL"
# ADD BASE RSC /l 0xc09 /d "_DEBUG"
# ADD RSC /l 0xc09 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 version.lib mapi32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /nodefaultlib:"libc" /out:"Build\mapi_d.pyd" /pdbtype:sept /libpath:"..\win32\build"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "mapi - Win32 Release"
# Name "mapi - Win32 Debug"
# Begin Group "Swigged"

# PROP Default_Filter "*.cpp"
# Begin Source File

SOURCE=.\win32comext\mapi\src\mapi.cpp
# End Source File
# Begin Source File

SOURCE=.\win32comext\mapi\src\PyIABContainer.cpp
# End Source File
# Begin Source File

SOURCE=.\win32comext\mapi\src\PyIAddrBook.cpp
# End Source File
# Begin Source File

SOURCE=.\win32comext\mapi\src\PyIAttach.cpp
# End Source File
# Begin Source File

SOURCE=.\win32comext\mapi\src\PyIDistList.cpp
# End Source File
# Begin Source File

SOURCE=.\win32comext\mapi\src\PyIMailUser.cpp
# End Source File
# Begin Source File

SOURCE=.\win32comext\mapi\src\PyIMAPIContainer.cpp
# End Source File
# Begin Source File

SOURCE=.\win32comext\mapi\src\PyIMAPIFolder.cpp
# End Source File
# Begin Source File

SOURCE=.\win32comext\mapi\src\PyIMAPIProp.cpp
# End Source File
# Begin Source File

SOURCE=.\win32comext\mapi\src\PyIMAPISession.cpp
# End Source File
# Begin Source File

SOURCE=.\win32comext\mapi\src\PyIMAPITable.cpp
# End Source File
# Begin Source File

SOURCE=.\win32comext\mapi\src\PyIMessage.cpp
# End Source File
# Begin Source File

SOURCE=.\win32comext\mapi\src\PyIMsgServiceAdmin.cpp
# End Source File
# Begin Source File

SOURCE=.\win32comext\mapi\src\PyIMsgStore.cpp
# End Source File
# Begin Source File

SOURCE=.\win32comext\mapi\src\PyIProfAdmin.cpp
# End Source File
# Begin Source File

SOURCE=.\win32comext\mapi\src\PyIProfSect.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\win32comext\mapi\src\mapi.i

!IF  "$(CFG)" == "mapi - Win32 Release"

USERDEP__MAPI_=".\win32comext\mapi\src\mapilib.i"	
# Begin Custom Build - Invoking SWIG...
InputDir=.\win32comext\mapi\src
InputPath=.\win32comext\mapi\src\mapi.i
InputName=mapi

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\swig.bat $(InputDir) -dnone -python -c++ -o $(InputName).cpp $(InputName).i

# End Custom Build

!ELSEIF  "$(CFG)" == "mapi - Win32 Debug"

USERDEP__MAPI_=".\win32comext\mapi\src\mapilib.i"	
# Begin Custom Build - Invoking SWIG...
InputDir=.\win32comext\mapi\src
InputPath=.\win32comext\mapi\src\mapi.i
InputName=mapi

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\swig.bat $(InputDir) -dnone -python -c++ -o $(InputName).cpp $(InputName).i

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32comext\mapi\src\mapiguids.cpp
# SUBTRACT CPP /YX
# End Source File
# Begin Source File

SOURCE=.\win32comext\mapi\src\mapilib.i
# End Source File
# Begin Source File

SOURCE=.\win32comext\mapi\src\mapiutil.cpp
# SUBTRACT CPP /YX
# End Source File
# Begin Source File

SOURCE=.\win32comext\mapi\src\PyIABContainer.i

!IF  "$(CFG)" == "mapi - Win32 Release"

# Begin Custom Build
InputDir=.\win32comext\mapi\src
InputPath=.\win32comext\mapi\src\PyIABContainer.i
InputName=PyIABContainer

BuildCmds= \
	..\swig.bat $(InputDir) -dnone -pythoncom -com_interface_parent IMAPIContainer -c++ -o              $(InputName).cpp  $(InputName).i

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "mapi - Win32 Debug"

# Begin Custom Build
InputDir=.\win32comext\mapi\src
InputPath=.\win32comext\mapi\src\PyIABContainer.i
InputName=PyIABContainer

BuildCmds= \
	..\swig.bat $(InputDir) -dnone -pythoncom -com_interface_parent IMAPIContainer -c++ -o              $(InputName).cpp  $(InputName).i

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32comext\mapi\src\PyIAddrBook.i

!IF  "$(CFG)" == "mapi - Win32 Release"

# Begin Custom Build - Invoking SWIG...
InputDir=.\win32comext\mapi\src
InputPath=.\win32comext\mapi\src\PyIAddrBook.i
InputName=PyIAddrBook

BuildCmds= \
	..\swig.bat $(InputDir) -dnone -pythoncom -com_interface_parent IMAPIProp -c++ -o                    $(InputName).cpp $(InputName).i

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "mapi - Win32 Debug"

# Begin Custom Build - Invoking SWIG...
InputDir=.\win32comext\mapi\src
InputPath=.\win32comext\mapi\src\PyIAddrBook.i
InputName=PyIAddrBook

BuildCmds= \
	..\swig.bat $(InputDir) -dnone -pythoncom -com_interface_parent IMAPIProp -c++ -o                    $(InputName).cpp $(InputName).i

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32comext\mapi\src\PyIAttach.i

!IF  "$(CFG)" == "mapi - Win32 Release"

USERDEP__PYIAT=".\win32comext\mapi\src\mapilib.i"	
# Begin Custom Build - Invoking SWIG...
InputDir=.\win32comext\mapi\src
InputPath=.\win32comext\mapi\src\PyIAttach.i
InputName=PyIAttach

BuildCmds= \
	..\swig.bat $(InputDir) -dnone -pythoncom -com_interface_parent IMAPIProp -c++ -o                    $(InputName).cpp $(InputName).i

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "mapi - Win32 Debug"

USERDEP__PYIAT=".\win32comext\mapi\src\mapilib.i"	
# Begin Custom Build - Invoking SWIG...
InputDir=.\win32comext\mapi\src
InputPath=.\win32comext\mapi\src\PyIAttach.i
InputName=PyIAttach

BuildCmds= \
	..\swig.bat $(InputDir) -dnone -pythoncom -com_interface_parent IMAPIProp -c++ -o                    $(InputName).cpp $(InputName).i

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32comext\mapi\src\PyIDistList.i

!IF  "$(CFG)" == "mapi - Win32 Release"

# Begin Custom Build
InputDir=.\win32comext\mapi\src
InputPath=.\win32comext\mapi\src\PyIDistList.i
InputName=PyIDistList

BuildCmds= \
	..\swig.bat $(InputDir) -dnone -pythoncom -com_interface_parent IMAPIProp -c++ -o              $(InputName).cpp  $(InputName).i

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "mapi - Win32 Debug"

# Begin Custom Build
InputDir=.\win32comext\mapi\src
InputPath=.\win32comext\mapi\src\PyIDistList.i
InputName=PyIDistList

BuildCmds= \
	..\swig.bat $(InputDir) -dnone -pythoncom -com_interface_parent IMAPIProp -c++ -o              $(InputName).cpp  $(InputName).i

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32comext\mapi\src\PyIMailUser.i

!IF  "$(CFG)" == "mapi - Win32 Release"

# Begin Custom Build
InputDir=.\win32comext\mapi\src
InputPath=.\win32comext\mapi\src\PyIMailUser.i
InputName=PyIMailUser

BuildCmds= \
	..\swig.bat $(InputDir) -dnone -pythoncom -com_interface_parent IMAPIContainer -c++ -o              $(InputName).cpp  $(InputName).i

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "mapi - Win32 Debug"

# Begin Custom Build
InputDir=.\win32comext\mapi\src
InputPath=.\win32comext\mapi\src\PyIMailUser.i
InputName=PyIMailUser

BuildCmds= \
	..\swig.bat $(InputDir) -dnone -pythoncom -com_interface_parent IMAPIContainer -c++ -o              $(InputName).cpp  $(InputName).i

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32comext\mapi\src\PyIMAPIContainer.i

!IF  "$(CFG)" == "mapi - Win32 Release"

USERDEP__PYIMA=".\win32comext\mapi\src\mapilib.i"	
# Begin Custom Build - Invoking SWIG...
InputDir=.\win32comext\mapi\src
InputPath=.\win32comext\mapi\src\PyIMAPIContainer.i
InputName=PyIMAPIContainer

BuildCmds= \
	..\swig.bat $(InputDir) -dnone -pythoncom -com_interface_parent IMAPIProp -c++ -o                    $(InputName).cpp  $(InputName).i

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "mapi - Win32 Debug"

USERDEP__PYIMA=".\win32comext\mapi\src\mapilib.i"	
# Begin Custom Build - Invoking SWIG...
InputDir=.\win32comext\mapi\src
InputPath=.\win32comext\mapi\src\PyIMAPIContainer.i
InputName=PyIMAPIContainer

BuildCmds= \
	..\swig.bat $(InputDir) -dnone -pythoncom -com_interface_parent IMAPIProp -c++ -o                    $(InputName).cpp  $(InputName).i

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32comext\mapi\src\PyIMAPIFolder.i

!IF  "$(CFG)" == "mapi - Win32 Release"

USERDEP__PYIMAP=".\win32comext\mapi\src\mapilib.i"	
# Begin Custom Build - Invoking SWIG...
InputDir=.\win32comext\mapi\src
InputPath=.\win32comext\mapi\src\PyIMAPIFolder.i
InputName=PyIMAPIFolder

BuildCmds= \
	..\swig.bat $(InputDir) -dnone -pythoncom -com_interface_parent IMAPIContainer -c++ -o                    $(InputName).cpp $(InputName).i

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "mapi - Win32 Debug"

USERDEP__PYIMAP=".\win32comext\mapi\src\mapilib.i"	
# Begin Custom Build - Invoking SWIG...
InputDir=.\win32comext\mapi\src
InputPath=.\win32comext\mapi\src\PyIMAPIFolder.i
InputName=PyIMAPIFolder

BuildCmds= \
	..\swig.bat $(InputDir) -dnone -pythoncom -com_interface_parent IMAPIContainer -c++ -o                    $(InputName).cpp $(InputName).i

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32comext\mapi\src\PyIMAPIProp.i

!IF  "$(CFG)" == "mapi - Win32 Release"

USERDEP__PYIMAPI=".\win32comext\mapi\src\mapilib.i"	
# Begin Custom Build - Invoking SWIG...
InputDir=.\win32comext\mapi\src
InputPath=.\win32comext\mapi\src\PyIMAPIProp.i
InputName=PyIMAPIProp

BuildCmds= \
	..\swig.bat $(InputDir) -dnone -pythoncom -c++ -o $(InputName).cpp $(InputName).i

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "mapi - Win32 Debug"

USERDEP__PYIMAPI=".\win32comext\mapi\src\mapilib.i"	
# Begin Custom Build - Invoking SWIG...
InputDir=.\win32comext\mapi\src
InputPath=.\win32comext\mapi\src\PyIMAPIProp.i
InputName=PyIMAPIProp

BuildCmds= \
	..\swig.bat $(InputDir) -dnone -pythoncom -c++ -o $(InputName).cpp $(InputName).i

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32comext\mapi\src\PyIMAPISession.i

!IF  "$(CFG)" == "mapi - Win32 Release"

USERDEP__PYIMAPIS=".\win32comext\mapi\src\mapilib.i"	
# Begin Custom Build - Invoking SWIG...
InputDir=.\win32comext\mapi\src
InputPath=.\win32comext\mapi\src\PyIMAPISession.i
InputName=PyIMAPISession

BuildCmds= \
	..\swig.bat $(InputDir) -dnone -pythoncom -c++ -o $(InputName).cpp $(InputName).i

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "mapi - Win32 Debug"

USERDEP__PYIMAPIS=".\win32comext\mapi\src\mapilib.i"	
# Begin Custom Build - Invoking SWIG...
InputDir=.\win32comext\mapi\src
InputPath=.\win32comext\mapi\src\PyIMAPISession.i
InputName=PyIMAPISession

BuildCmds= \
	..\swig.bat $(InputDir) -dnone -pythoncom -c++ -o $(InputName).cpp $(InputName).i

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32comext\mapi\src\PyIMAPITable.i

!IF  "$(CFG)" == "mapi - Win32 Release"

USERDEP__PYIMAPIT=".\win32comext\mapi\src\mapilib.i"	
# Begin Custom Build - Invoking SWIG...
InputDir=.\win32comext\mapi\src
InputPath=.\win32comext\mapi\src\PyIMAPITable.i
InputName=PyIMAPITable

BuildCmds= \
	..\swig.bat $(InputDir) -dnone -pythoncom -c++ -o $(InputName).cpp $(InputName).i

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "mapi - Win32 Debug"

USERDEP__PYIMAPIT=".\win32comext\mapi\src\mapilib.i"	
# Begin Custom Build - Invoking SWIG...
InputDir=.\win32comext\mapi\src
InputPath=.\win32comext\mapi\src\PyIMAPITable.i
InputName=PyIMAPITable

BuildCmds= \
	..\swig.bat $(InputDir) -dnone -pythoncom -c++ -o $(InputName).cpp $(InputName).i

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32comext\mapi\src\PyIMessage.i

!IF  "$(CFG)" == "mapi - Win32 Release"

USERDEP__PYIME=".\win32comext\mapi\src\mapilib.i"	
# Begin Custom Build - Invoking SWIG...
InputDir=.\win32comext\mapi\src
InputPath=.\win32comext\mapi\src\PyIMessage.i
InputName=PyIMessage

BuildCmds= \
	..\swig.bat $(InputDir) -dnone -pythoncom -com_interface_parent IMAPIProp -c++ -o                    $(InputName).cpp $(InputName).i

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "mapi - Win32 Debug"

USERDEP__PYIME=".\win32comext\mapi\src\mapilib.i"	
# Begin Custom Build - Invoking SWIG...
InputDir=.\win32comext\mapi\src
InputPath=.\win32comext\mapi\src\PyIMessage.i
InputName=PyIMessage

BuildCmds= \
	..\swig.bat $(InputDir) -dnone -pythoncom -com_interface_parent IMAPIProp -c++ -o                    $(InputName).cpp $(InputName).i

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32comext\mapi\src\PyIMsgServiceAdmin.i

!IF  "$(CFG)" == "mapi - Win32 Release"

# Begin Custom Build
InputDir=.\win32comext\mapi\src
InputPath=.\win32comext\mapi\src\PyIMsgServiceAdmin.i
InputName=PyIMsgServiceAdmin

BuildCmds= \
	..\swig.bat $(InputDir) -dnone -pythoncom -c++ -o  $(InputName).cpp $(InputName).i

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "mapi - Win32 Debug"

# Begin Custom Build
InputDir=.\win32comext\mapi\src
InputPath=.\win32comext\mapi\src\PyIMsgServiceAdmin.i
InputName=PyIMsgServiceAdmin

BuildCmds= \
	..\swig.bat $(InputDir) -dnone -pythoncom -c++ -o  $(InputName).cpp $(InputName).i

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32comext\mapi\src\PyIMsgStore.i

!IF  "$(CFG)" == "mapi - Win32 Release"

USERDEP__PYIMS=".\win32comext\mapi\src\mapilib.i"	
# Begin Custom Build - Invoking SWIG...
InputDir=.\win32comext\mapi\src
InputPath=.\win32comext\mapi\src\PyIMsgStore.i
InputName=PyIMsgStore

BuildCmds= \
	..\swig.bat $(InputDir) -dnone -pythoncom -com_interface_parent IMAPIProp -c++ -o                    $(InputName).cpp $(InputName).i

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "mapi - Win32 Debug"

USERDEP__PYIMS=".\win32comext\mapi\src\mapilib.i"	
# Begin Custom Build - Invoking SWIG...
InputDir=.\win32comext\mapi\src
InputPath=.\win32comext\mapi\src\PyIMsgStore.i
InputName=PyIMsgStore

BuildCmds= \
	..\swig.bat $(InputDir) -dnone -pythoncom -com_interface_parent IMAPIProp -c++ -o                    $(InputName).cpp $(InputName).i

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32comext\mapi\src\PyIProfAdmin.i

!IF  "$(CFG)" == "mapi - Win32 Release"

USERDEP__PYIPR=".\win32comext\mapi\src\mapilib.i"	
# Begin Custom Build - Invoking SWIG...
InputDir=.\win32comext\mapi\src
InputPath=.\win32comext\mapi\src\PyIProfAdmin.i
InputName=PyIProfAdmin

BuildCmds= \
	..\swig.bat $(InputDir) -dnone -pythoncom -c++ -o $(InputName).cpp $(InputName).i

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "mapi - Win32 Debug"

USERDEP__PYIPR=".\win32comext\mapi\src\mapilib.i"	
# Begin Custom Build - Invoking SWIG...
InputDir=.\win32comext\mapi\src
InputPath=.\win32comext\mapi\src\PyIProfAdmin.i
InputName=PyIProfAdmin

BuildCmds= \
	..\swig.bat $(InputDir) -dnone -pythoncom -c++ -o $(InputName).cpp $(InputName).i

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32comext\mapi\src\PyIProfSect.i

!IF  "$(CFG)" == "mapi - Win32 Release"

# Begin Custom Build
InputDir=.\win32comext\mapi\src
InputPath=.\win32comext\mapi\src\PyIProfSect.i
InputName=PyIProfSect

BuildCmds= \
	..\swig.bat $(InputDir) -dnone -pythoncom -com_interface_parent IMAPIProp -c++ -o              $(InputName).cpp  $(InputName).i

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "mapi - Win32 Debug"

# Begin Custom Build
InputDir=.\win32comext\mapi\src
InputPath=.\win32comext\mapi\src\PyIProfSect.i
InputName=PyIProfSect

BuildCmds= \
	..\swig.bat $(InputDir) -dnone -pythoncom -com_interface_parent IMAPIProp -c++ -o              $(InputName).cpp  $(InputName).i

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\win32comext\mapi\src\PyMAPIUtil.h
# End Source File
# End Target
# End Project
