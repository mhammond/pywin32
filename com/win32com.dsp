# Microsoft Developer Studio Project File - Name="win32com" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

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
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Python/com/win32com", SAAAAAAA"
# PROP Scc_LocalPath "./win32com"
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "win32com - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Build"
# PROP Intermediate_Dir "Build\Temp\win32com\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
F90=df.exe
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /Zi /O2 /I "win32com\src\include" /I "..\win32\src" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "BUILD_PYTHONCOM" /D "STRICT" /D "_STRICT" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o /win32 "NUL"
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o /win32 "NUL"
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 ole32.lib oleaut32.lib uuid.lib user32.lib /nologo /base:"0x1e340000" /subsystem:windows /dll /pdb:"Build\System\pythoncom24.pdb" /debug /machine:I386 /def:".\win32com\src\PythonCOM.def" /out:"Build\System\pythoncom24.dll" /implib:"Build\pythoncom.lib" /libpath:"..\win32\build"
# SUBTRACT LINK32 /pdb:none
# Begin Custom Build - copy to system32
ProjDir=.
TargetPath=.\Build\System\pythoncom24.dll
TargetName=pythoncom24
InputPath=.\Build\System\pythoncom24.dll
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
# PROP Output_Dir "Build"
# PROP Intermediate_Dir "Build\Temp\win32com\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
F90=df.exe
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MDd /W3 /GX /ZI /Od /I "win32com\src\include" /I "..\win32\src" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "BUILD_PYTHONCOM" /D "STRICT" /D "_STRICT" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o /win32 "NUL"
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o /win32 "NUL"
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 ole32.lib oleaut32.lib uuid.lib user32.lib /nologo /base:"0x1e340000" /subsystem:windows /dll /pdb:"Build\System\pythoncom24_d.pdb" /debug /machine:I386 /def:".\win32com\src\PythonCOM.def" /out:"Build\System\pythoncom24_d.dll" /implib:"Build\pythoncom_d.lib" /pdbtype:sept /libpath:"..\win32\build"
# SUBTRACT LINK32 /pdb:none
# Begin Custom Build - copy to system32
ProjDir=.
TargetPath=.\Build\System\pythoncom24_d.dll
TargetName=pythoncom24_d
InputPath=.\Build\System\pythoncom24_d.dll
SOURCE="$(InputPath)"

"$(ProjDir)\$(TargetName).flg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(TargetPath) %SYSTEMROOT%\System32\. && echo Done >                                      $(ProjDir)\$(TargetName).flg

# End Custom Build

!ENDIF 

# Begin Target

# Name "win32com - Win32 Release"
# Name "win32com - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter ""
# Begin Group "PythonCOM"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\win32com\src\dllmain.cpp
# End Source File
# Begin Source File

SOURCE=.\win32com\src\ErrorUtils.cpp
# End Source File
# Begin Source File

SOURCE=.\win32com\src\MiscTypes.cpp
# End Source File
# Begin Source File

SOURCE=.\win32com\src\oleargs.cpp
# End Source File
# Begin Source File

SOURCE=.\win32com\src\PyComHelpers.cpp
# End Source File
# Begin Source File

SOURCE=.\win32com\src\PyFactory.cpp
# End Source File
# Begin Source File

SOURCE=.\win32com\src\PyGatewayBase.cpp
# End Source File
# Begin Source File

SOURCE=.\win32com\src\PyIBase.cpp
# End Source File
# Begin Source File

SOURCE=.\win32com\src\PyIClassFactory.cpp
# End Source File
# Begin Source File

SOURCE=.\win32com\src\PyIDispatch.cpp
# End Source File
# Begin Source File

SOURCE=.\win32com\src\PyIUnknown.cpp
# End Source File
# Begin Source File

SOURCE=.\win32com\src\PyRecord.cpp
# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PySTGMEDIUM.cpp
# End Source File
# Begin Source File

SOURCE=.\win32com\src\PyStorage.cpp
# End Source File
# Begin Source File

SOURCE=.\win32com\src\PythonCOM.cpp
# End Source File
# Begin Source File

SOURCE=.\win32com\src\Register.cpp
# End Source File
# Begin Source File

SOURCE=.\win32com\src\stdafx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\win32com\src\univgw.cpp
# End Source File
# Begin Source File

SOURCE=.\win32com\src\univgw_dataconv.cpp
# End Source File
# End Group
# Begin Group "Extensions"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\win32com\src\extensions\PyFUNCDESC.cpp
# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyGConnectionPoint.cpp
# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyGConnectionPointContainer.cpp
# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyGEnumVariant.cpp
# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyGErrorLog.cpp
# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyGPersist.cpp
# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyGPersistPropertyBag.cpp
# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyGPersistStorage.cpp
# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyGPersistStream.cpp
# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyGPersistStreamInit.cpp
# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyGPropertyBag.cpp
# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyGStream.cpp
# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyIBindCtx.cpp
# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyICatInformation.cpp
# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyICatRegister.cpp
# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyIConnectionPoint.cpp
# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyIConnectionPointContainer.cpp
# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyICreateTypeInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyICreateTypeLib.cpp
# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyICreateTypeLib2.cpp
# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyIDataObject.cpp
# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyIDropSource.cpp
# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyIDropTarget.cpp
# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyIEnumCATEGORYINFO.cpp
# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyIEnumConnectionPoints.cpp
# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyIEnumConnections.cpp
# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyIEnumFORMATETC.cpp
# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyIEnumGUID.cpp
# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyIEnumSTATPROPSETSTG.cpp
# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyIEnumSTATPROPSTG.cpp
# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyIEnumSTATSTG.cpp
# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyIEnumString.cpp
# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyIEnumVARIANT.cpp
# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyIErrorLog.cpp
# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyIExternalConnection.cpp
# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyIGlobalInterfaceTable.cpp
# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyILockBytes.cpp
# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyIMoniker.cpp
# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyIOleWindow.cpp
# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyIPersist.cpp
# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyIPersistFile.cpp
# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyIPersistPropertyBag.cpp
# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyIPersistStorage.cpp
# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyIPersistStream.cpp
# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyIPersistStreamInit.cpp
# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyIPropertyBag.cpp
# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyIPropertySetStorage.cpp
# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyIPropertyStorage.cpp
# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyIProvideClassInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyIRunningObjectTable.cpp
# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyIServiceProvider.cpp
# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyIStorage.cpp
# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyIStream.cpp
# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyIType.cpp
# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyITypeObjects.cpp
# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyTYPEATTR.cpp
# End Source File
# Begin Source File

SOURCE=.\win32com\src\extensions\PyVARDESC.cpp
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

SOURCE=.\win32com\src\include\PyIDataObject.h
# End Source File
# Begin Source File

SOURCE=.\win32com\src\include\PyIDropSource.h
# End Source File
# Begin Source File

SOURCE=.\win32com\src\include\PyIDropTarget.h
# End Source File
# Begin Source File

SOURCE=.\win32com\src\include\PyIEnumConnectionPoints.h
# End Source File
# Begin Source File

SOURCE=.\win32com\src\include\PyIEnumConnections.h
# End Source File
# Begin Source File

SOURCE=.\win32com\src\include\PyIEnumFORMATETC.h
# End Source File
# Begin Source File

SOURCE=.\win32com\src\include\PyIEnumGUID.h
# End Source File
# Begin Source File

SOURCE=.\win32com\src\include\PyIEnumSTATPROPSETSTG.h
# End Source File
# Begin Source File

SOURCE=.\win32com\src\include\PyIEnumSTATSTG.h
# End Source File
# Begin Source File

SOURCE=.\win32com\src\include\PyIEnumString.h
# End Source File
# Begin Source File

SOURCE=.\win32com\src\include\PyIEnumVARIANT.h
# End Source File
# Begin Source File

SOURCE=.\win32com\src\include\PyIExternalConnection.h
# End Source File
# Begin Source File

SOURCE=.\win32com\src\include\PyIGlobalInterfaceTable.h
# End Source File
# Begin Source File

SOURCE=.\win32com\src\include\PyILockBytes.h
# End Source File
# Begin Source File

SOURCE=.\win32com\src\include\PyIMoniker.h
# End Source File
# Begin Source File

SOURCE=.\win32com\src\include\PyIOleWindow.h
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
# Begin Source File

SOURCE=.\win32com\src\include\univgw_dataconv.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\win32com\src\PythonCOM.def
# PROP Exclude_From_Build 1
# End Source File
# End Group
# End Target
# End Project
