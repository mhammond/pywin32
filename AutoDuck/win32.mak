# MAKEFILE
# Builds documentation for win32 extensions using the AUTODUCK tool
#

!include "common_top.mak"

TARGET  = Python Win32 Extensions
GENDIR  = ..\build\Temp\Help
TITLE   = $(TARGET) Help
DOCHDR  = $(TARGET) Reference

SOURCE_DIR = ../win32
SOURCE  = $(SOURCE_DIR)/src/*.cpp \
	  $(SOURCE_DIR)/src/*.h \
	  $(SOURCE_DIR)/src/perfmon/*.cpp \
	  $(SOURCE_DIR)/src/win32net/*.cpp \
	  $(SOURCE_DIR)/src/win32wnet/*.cpp \
	  $(SOURCE_DIR)/src/win32print/*.cpp \
          $(SOURCE_DIR)/help/*.d \
	  $(GENDIR)/win32evtlog.d $(GENDIR)/win32event.d $(GENDIR)/win32file.d \
	  $(GENDIR)/win32service.d $(GENDIR)/win32pipe.d $(GENDIR)/win32security.d \
	  $(GENDIR)/win32process.d $(GENDIR)/win32gui.d \
          $(GENDIR)/wincerapi.d

# Help and Doc targets

hwlp : $(GENDIR) "..\$(TARGET).hlp"

doc : "$(TARGET).doc"

clean: cleanad

$(GENDIR)/win32file.d: $(SOURCE_DIR)/src/win32file.i
	makedfromi.py -o$*.d $(SOURCE_DIR)/src/$(*B).i

$(GENDIR)/win32event.d: $(SOURCE_DIR)/src/win32event.i
	makedfromi.py -o$*.d $(SOURCE_DIR)/src/$(*B).i

$(GENDIR)/win32evtlog.d: $(SOURCE_DIR)/src/win32evtlog.i
	makedfromi.py -o$*.d $(SOURCE_DIR)/src/$(*B).i

$(GENDIR)/win32service.d: $(SOURCE_DIR)/src/win32service.i
	makedfromi.py -o$*.d $(SOURCE_DIR)/src/$(*B).i

$(GENDIR)/win32pipe.d: $(SOURCE_DIR)/src/win32pipe.i
	makedfromi.py -o$*.d $(SOURCE_DIR)/src/$(*B).i

$(GENDIR)/win32security.d: $(SOURCE_DIR)/src/$(*B).i
	makedfromi.py -o$*.d $(SOURCE_DIR)/src/$(*B).i

$(GENDIR)/win32process.d: $(SOURCE_DIR)/src/$(*B).i
	makedfromi.py -o$*.d $(SOURCE_DIR)/src/$(*B).i

$(GENDIR)/win32gui.d: $(SOURCE_DIR)/src/$(*B).i
	makedfromi.py -o$*.d $(SOURCE_DIR)/src/$(*B).i

$(GENDIR)/wincerapi.d: $(SOURCE_DIR)/src/$(*B).i
	makedfromi.py -o$*.d $(SOURCE_DIR)/src/$(*B).i

!include "common.mak"
