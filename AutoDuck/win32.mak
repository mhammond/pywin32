# MAKEFILE
# Builds documentation for win32 extensions using the AUTODUCK tool
#

!include "common_top.mak"

TARGET  = PythonWin32Extensions
GENDIR  = ..\build\Temp\Help
TITLE   = $(TARGET) Help
DOCHDR  = $(TARGET) Reference

SOURCE_DIR = ../win32/src
HELP_DIR   = ../win32/help

# Name of the subdirectory to copy $(HTML_FILES) into
# for building of the .CHM file.
HTML_DIR = win32
# Extraneous HTML files to include into the .CHM:
HTML_FILES = $(HELP_DIR)\*.html

# Non-autoduck overview topics data file:
EXT_TOPICS = win32Overviews.dat

SOURCE  = $(SOURCE_DIR)/*.cpp \
	  $(SOURCE_DIR)/*.h \
	  $(HELP_DIR)/*.d \
	  $(SOURCE_DIR)/perfmon/*.cpp \
	  $(SOURCE_DIR)/win32net/*.cpp \
	  $(SOURCE_DIR)/win32wnet/*.cpp \
	  $(SOURCE_DIR)/win32print/*.cpp \
	  $(GENDIR)/win32evtlog.d $(GENDIR)/win32event.d $(GENDIR)/win32file.d \
	  $(GENDIR)/win32service.d $(GENDIR)/win32pipe.d $(GENDIR)/win32security.d \
	  $(GENDIR)/win32process.d $(GENDIR)/wincerapi.d $(GENDIR)/win32gui.d

# Help and Doc targets
all: help htmlhlp

help : $(GENDIR) "..\$(TARGET).hlp"

htmlhlp: $(GENDIR) "..\$(TARGET).chm"

doc : "$(TARGET).doc"

clean: cleanad

$(GENDIR)/win32file.d: $(SOURCE_DIR)/win32file.i
	makedfromi.py -o$*.d $(SOURCE_DIR)/$(*B).i

$(GENDIR)/win32event.d: $(SOURCE_DIR)/win32event.i
	makedfromi.py -o$*.d $(SOURCE_DIR)/$(*B).i

$(GENDIR)/win32evtlog.d: $(SOURCE_DIR)/win32evtlog.i
	makedfromi.py -o$*.d $(SOURCE_DIR)/$(*B).i

$(GENDIR)/win32service.d: $(SOURCE_DIR)/win32service.i
	makedfromi.py -o$*.d $(SOURCE_DIR)/$(*B).i

$(GENDIR)/win32pipe.d: $(SOURCE_DIR)/win32pipe.i
	makedfromi.py -o$*.d $(SOURCE_DIR)/$(*B).i

$(GENDIR)/win32security.d: $(SOURCE_DIR)/$(*B).i
	makedfromi.py -o$*.d $(SOURCE_DIR)/$(*B).i

$(GENDIR)/win32process.d: $(SOURCE_DIR)/$(*B).i
	makedfromi.py -o$*.d $(SOURCE_DIR)/$(*B).i

$(GENDIR)/wincerapi.d: $(SOURCE_DIR)/$(*B).i
	makedfromi.py -o$*.d $(SOURCE_DIR)/$(*B).i

$(GENDIR)/win32gui.d: $(SOURCE_DIR)/$(*B).i
	makedfromi.py -o$*.d $(SOURCE_DIR)/$(*B).i

!include "common.mak"
