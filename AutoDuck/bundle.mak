# Creates the wrapper .chm file for the Python Windows docs.

!include "common_top.mak"

TARGET  = PyWin32
GENDIR  = ..\build\Temp\Help
TITLE   = $(TARGET) Help
DOCHDR  = $(TARGET) Reference

GENSOURCE = "$(GENDIR)\PyWin32.html" \
	"$(GENDIR)\PyWin32.hhc" \
	"$(GENDIR)\PyWin32.hhk" \
	"$(GENDIR)\PyWin32.hhp"

CHMS = "..\PythonWin.chm" "..\win32com.chm" "..\PythonWin32Extensions.chm"

FILES = PyWin32.html \
	PyWin32.hhc \
	PyWin32.hhk \
	PyWin32.hhp

# Help and Doc targets
htmlhlp : "..\$(TARGET).chm"

"..\$(TARGET).chm": $(GENSOURCE) $(CHMS)
	-$(HHC) "$(GENDIR)\$(TARGET).hhp"
	if exist "..\$(TARGET).chm" del "..\$(TARGET).chm"
	move "$(GENDIR)\$(TARGET).chm" "..\$(TARGET).chm" 


"$(GENDIR)\PyWin32.html": PyWin32.html
	copy $? $@

"$(GENDIR)\PyWin32.hhk": PyWin32.hhk
	copy $? $@

"$(GENDIR)\PyWin32.hhc": PyWin32.hhc
	copy $? $@

"$(GENDIR)\PyWin32.hhp": PyWin32.hhp
	copy $? $@

$(GENDIR):
	@if not exist $(GENDIR)\. md $(GENDIR)

