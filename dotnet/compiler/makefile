
PY_BUILTINS_DLL=Python.Builtins.Builtins.dll
NMAKE=nmake

all: corglue $(PY_BUILTINS_DLL)

corglue::
	cd $*
	$(NMAKE) /nologo  /$(MAKEFLAGS)
	cd $(MAKEDIR)

# To get this DLL in the fusion cache, we execute the compiler with enough magic
# switches that forces the assembly to hold only the assembly manifest, and
# _all_ modules as external .mod files.  We then discard our assembly, and re-create
# a new one using ALINK.  This new assembly then works correctly with the cache.
# (The Python generated one doesnt cause the .mod files to also be dragged in - 
# this is presumably a COM+ bug - CSharp appears to have the same problem,
# a the CrossLang sample does something similar (it just doesnt comment it :-(
$(PY_BUILTINS_DLL): builtins\builtins.py modules\time.py
	@if exist $* del $*.dll
	cpy.py /dll /v /w:9 /assembly-keyfile:..\..\ManagedPython.key /assembly-filename:$(PY_BUILTINS_DLL) /o:builtins.mod /module-name:builtins builtins\builtins.py $$PYTHON_HOME\lib\string.py modules\time.py
#	peverify $(PY_BUILTINS_DLL)
#	del delme.delme
#	al /out:$(PY_BUILTINS_DLL) /keyf:..\..\ManagedPython.key builtins.mod string.mod time.mod
	gacutil /i $(PY_BUILTINS_DLL)

clean:
    cd corglue
    - $(NMAKE) /nologo  clean
    cd ..
	- gacutil /u Python.Builtins.Builtins
	- del $(PY_BUILTINS_DLL)
	- del Python.dll
	- del *.exe
	- del *.mod
	- del *.pdb
	- del *.pyc /s
	- del suite\*.exe
	- del suite\*.pdb
	- del suite\*.deb
	- del samples\*.exe /s
	- del samples\*.dll /s
	- del samples\*.pdb /s
	- del samples\*.deb /s
	- del samples\*.tlb /s
	- del samples\*.mod /s
	- del scratch\*.exe /s
	- del scratch\*.dll /s
	- del scratch\*.pdb /s
	- del scratch\*.deb /s
	- del scratch\*.tlb /s
