!IF EXIST ($(WINDIR)\SYSTEM32\MSCOREE.DLL)
SYSTEM=$(WINDIR)\SYSTEM32
!ELSE
SYSTEM=$(WINDIR)\SYSTEM
!ENDIF

_ALINK=al.exe
_BC=vbc.exe
_CL=cl.exe

_PYC=..\..\cpy.py
_PYCFLAGS=/debug
_PYCFLAGS_DLL=/dll /out:$*.dll
_PYCFLAGS_EXE=/out:$*.exe

COMREG=regasm.exe
TLBEXP=tlbexp.exe
SN=sn.exe

all:	PowerGenerator.dll

TableGen.dll CalendarGen.dll MultiGen.dll:
	@echo *** Before running this sample, you must perform the following:
	@echo 1) Locate and build the standard NGWS "CrossLang" sample.
	@echo 2) Copy *.dll and *.mod from that directory into this directory.
	@echo 3) Re-run this make process.
	fail

PowerGenerator.dll: PowerGenerator.py PowerGenerator.key TableGen.dll
	$(_PYC) $(_PYCFLAGS) $(_PYCFLAGS_DLL) /no-python-runtime /ref:../../Python.dll /ref:TableGen.dll /assembly-keyfile:PowerGenerator.key $*.py
	$(COMREG) PowerGenerator.dll
	$(TLBEXP) PowerGenerator.dll
	al -i:PowerGenerator.dll

# For the key file, we avoid remaking
PowerGenerator.key:
	if not exist PowerGenerator.key $(SN) -k PowerGenerator.key

MakeTable_py.exe: MakeTable.py PowerGenerator.dll TableGen.dll CalendarGen.dll MultiGen.dll
	$(_PYC) $(_PYCFLAGS) $(_PYCFLAGS_EXE) /no-python-runtime /ref:../../Python.dll /ref:CalendarGen.dll /ref:MultiGen.dll /ref:PowerGenerator.dll /out:MakeTable_py.exe MakeTable.py

clean:
    if exist PowerGenerator.dll $(COMREG) /unregister PowerGenerator.dll
    if exist PowerGenerator.dll InstallUtil /u PowerGenerator.dll
    -erase *.pdb *.exe *.dll *.reg *.tlb *.mod *.InstallLog *.InstallState
