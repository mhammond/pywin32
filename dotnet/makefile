NMAKE=nmake

all: ..\ManagedPython.key pyruntime compiler

..\ManagedPython.key:
	@if not exist ..\ManagedPython.key echo Creating a new key for signing the assemblies
	if not exist ..\ManagedPython.key sn -k ..\ManagedPython.key

pyruntime::
	cd $*
	$(NMAKE) /nologo /$(MAKEFLAGS)
	cd $(MAKEDIR)

compiler::
	cd $*
	$(NMAKE) /nologo  /$(MAKEFLAGS)
	@echo You can now run the test suite by executing 'testall.bat' in the 'compiler' directory
	@echo You can also check out some of the samples in the 'compiler\samples' directory.

