!IFNDEF CORSDK
CORSDK=..\..
!ENDIF

!IF EXIST ($(WINDIR)\SYSTEM32\MSCOREE.DLL)
SYSTEM=$(WINDIR)\SYSTEM32
!ELSE
SYSTEM=$(WINDIR)\SYSTEM
!ENDIF

KEY_FILE=..\..\ManagedPython.key
PYRUNTIME_DLL=Python.dll

all: $(PYRUNTIME_DLL)

$(PYRUNTIME_DLL): PyRuntime.cs $(KEY_FILE)
	- gacutil /nologo /u Python
	csc /nologo /target:library /out:$(PYRUNTIME_DLL) /debug+ PyRuntime.cs
    gacutil /nologo /i $(PYRUNTIME_DLL)
    copy Python.dll ..\compiler

#al -i:$(PYRUNTIME_DLL)

clean:
	- gacutil /nologo /u Python
	- del $(PYRUNTIME_DLL)
	- del *.pdb