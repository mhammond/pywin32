# GNUmakefile for Scintilla in Pythonwin
# GNU make equivalent of makefile_pythonwin, delegating to win32/makefile.
#
# Usage (standalone):
#   make [-f GNUmakefile] [DEBUG=1] [SUB_DIR_O=<path>] [SUB_DIR_BIN=<path>]
#
# SUB_DIR_O and SUB_DIR_BIN are paths *relative to win32/* (or absolute).
# When called from setup.py they are passed as absolute paths.

ifdef DEBUG
_DEFAULT_DIR_O = ../Build/Temp/Scintilla/Debug
else
_DEFAULT_DIR_O = ../Build/Temp/Scintilla/Release
endif

# DIR_BIN / DIR_O: relative to this file's directory (pythonwin/Scintilla/)
DIR_BIN ?= ../Build
DIR_O   ?= $(_DEFAULT_DIR_O)

PYTHON ?= python

# SUB_DIR_*: same paths but from win32/'s perspective (one level deeper).
# If setup.py passes absolute paths these ?= defaults are ignored.
SUB_DIR_BIN ?= ../$(DIR_BIN)
SUB_DIR_O   ?= ../$(DIR_O)

ifdef DEBUG
SCINTILLA_DLL = $(SUB_DIR_BIN)/Scintilla_d.dll
else
SCINTILLA_DLL = $(SUB_DIR_BIN)/Scintilla.dll
endif

.PHONY: all Scintilla clean

all: Scintilla

clean:
	$(MAKE) -C win32 -f makefile DIR_BIN=$(SUB_DIR_BIN) clean

Scintilla:
	mkdir -p $(SUB_DIR_BIN) $(SUB_DIR_O)
	$(MAKE) -C win32 -f makefile \
		DIR_BIN=$(SUB_DIR_BIN) \
		LEXCOMPONENT=$(SCINTILLA_DLL) \
		COMPONENT=dummy \
		$(SCINTILLA_DLL)

scintillacon.py: Include/Scintilla.h Include/SciLexer.h
	$(PYTHON) ../../Win32/scripts/h2py.py Include/scintilla.h
	$(PYTHON) ../../Win32/scripts/h2py.py Include/scilexer.h
	cat scintilla.py > ../pywin/scintilla/scintillacon.py
	cat scilexer.py >> ../pywin/scintilla/scintillacon.py
	rm scintilla.py scilexer.py
