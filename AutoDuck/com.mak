# MAKEFILE
# Builds documentation for Pythonwin using the AUTODUCK tool
#

!include "common_top.mak"

TARGET  = win32com
GENDIR  = ..\build\Temp\Help
TITLE   = $(TARGET) Help
DOCHDR  = $(TARGET) Reference

WIN32COM_DIR = ../com/win32com
WIN32COMEXT_DIR = ../com/win32comext
MAPI_DIR = $(WIN32COMEXT_DIR)/mapi
ADSI_DIR = $(WIN32COMEXT_DIR)/adsi
HELP_DIR = ../com/help

# Name of the subdirectory to copy $(HTML_FILES) into
# for building of the .CHM file.
HTML_DIR = win32com
# Extraneous HTML files to include into the .CHM:
HTML_FILES = $(WIN32COM_DIR)/*.htm* $(WIN32COM_DIR)/HTML/*.html $(WIN32COM_DIR)/HTML/image/*

# Non-autoduck overview topics data file:
EXT_TOPICS = comOverviews.dat

SOURCE  = $(WIN32COM_DIR)\src\*.cpp \
	$(HELP_DIR)\*.d \
	$(WIN32COM_DIR)\src\extensions\*.cpp \
	$(WIN32COMEXT_DIR)\axscript\src\*.cpp \
	$(WIN32COMEXT_DIR)\axdebug\src\*.cpp \
	$(WIN32COMEXT_DIR)\axcontrol\src\*.cpp \
	$(WIN32COMEXT_DIR)\shell\src\*.cpp \
	$(WIN32COMEXT_DIR)\internet\src\*.cpp \
	$(WIN32COM_DIR)\src\include\*.h \
	$(MAPI_DIR)\src\*.cpp \
	$(GENDIR)\mapi.d \
	$(GENDIR)\PyIABContainer.d \
	$(GENDIR)\PyIAddrBook.d \
	$(GENDIR)\PyIAttach.d \
	$(GENDIR)\PyIDistList.d \
	$(GENDIR)\PyIMailUser.d \
	$(GENDIR)\PyIMAPIContainer.d \
	$(GENDIR)\PyIMAPIFolder.d \
	$(GENDIR)\PyIMAPIProp.d \
	$(GENDIR)\PyIMAPISession.d \
	$(GENDIR)\PyIMAPITable.d \
	$(GENDIR)\PyIMessage.d \
	$(GENDIR)\PyIMsgServiceAdmin.d \
	$(GENDIR)\PyIMsgStore.d \
	$(GENDIR)\PyIProfAdmin.d \
	$(GENDIR)\PyIProfSect.d \
	$(GENDIR)\exchange.d \
	$(GENDIR)\exchdapi.d \
	$(ADSI_DIR)\src\*.cpp \
	$(GENDIR)\adsi.d \
	$(GENDIR)\PyIADsContainer.d \
	$(GENDIR)\PyIADsUser.d \
	$(GENDIR)\PyIDirectoryObject.d \



# Help and Doc targets
all: help htmlhlp

help : $(GENDIR) ..\$(TARGET).hlp

htmlhlp : $(GENDIR) "..\$(TARGET).chm"

doc : $(TARGET).doc

clean: cleanad

$(GENDIR)\mapi.d: $(MAPI_DIR)/src/$(*B).i
	makedfromi.py -o$*.d $(MAPI_DIR)/src/$(*B).i

$(GENDIR)\PyIABContainer.d: $(MAPI_DIR)/src/$(*B).i
	makedfromi.py -o$*.d -p PyIMAPIContainer $(MAPI_DIR)/src/$(*B).i

$(GENDIR)\PyIAddrBook.d: $(MAPI_DIR)/src/$(*B).i
	makedfromi.py -o$*.d -p PyIMAPIProp $(MAPI_DIR)/src/$(*B).i

$(GENDIR)\PyIAttach.d: $(MAPI_DIR)/src/$(*B).i
	makedfromi.py -o$*.d -p PyIMAPIProp $(MAPI_DIR)/src/$(*B).i

$(GENDIR)\PyIDistList.d: $(MAPI_DIR)/src/$(*B).i
	makedfromi.py -o$*.d -p PyIMAPIProp $(MAPI_DIR)/src/$(*B).i

$(GENDIR)\PyIMailUser.d: $(MAPI_DIR)/src/$(*B).i
	makedfromi.py -o$*.d -p PyIMAPIContainer $(MAPI_DIR)/src/$(*B).i

$(GENDIR)\PyIMAPIContainer.d: $(MAPI_DIR)/src/$(*B).i
	makedfromi.py -o$*.d -p PyIMAPIProp $(MAPI_DIR)/src/$(*B).i

$(GENDIR)\PyIMAPIFolder.d: $(MAPI_DIR)/src/$(*B).i
	makedfromi.py -o$*.d -p PyIMAPIProp $(MAPI_DIR)/src/$(*B).i

$(GENDIR)\PyIMAPIProp.d: $(MAPI_DIR)/src/$(*B).i
	makedfromi.py -o$*.d -p PyIUnknown $(MAPI_DIR)/src/$(*B).i

$(GENDIR)\PyIMAPISession.d: $(MAPI_DIR)/src/$(*B).i
	makedfromi.py -o$*.d -p PyIUnknown $(MAPI_DIR)/src/$(*B).i

$(GENDIR)\PyIMAPITable.d: $(MAPI_DIR)/src/$(*B).i
	makedfromi.py -o$*.d -p PyIUnknown $(MAPI_DIR)/src/$(*B).i

$(GENDIR)\PyIMessage.d: $(MAPI_DIR)/src/$(*B).i
	makedfromi.py -o$*.d -p PyIMAPIProp $(MAPI_DIR)/src/$(*B).i

$(GENDIR)\PyIMsgServiceAdmin.d: $(MAPI_DIR)/src/$(*B).i
	makedfromi.py -o$*.d -p PyIUnknown $(MAPI_DIR)/src/$(*B).i

$(GENDIR)\PyIMsgStore.d: $(MAPI_DIR)/src/$(*B).i
	makedfromi.py -o$*.d -p PyIMAPIProp $(MAPI_DIR)/src/$(*B).i

$(GENDIR)\PyIProfAdmin.d: $(MAPI_DIR)/src/$(*B).i
	makedfromi.py -o$*.d -p PyIUnknown $(MAPI_DIR)/src/$(*B).i

$(GENDIR)\PyIProfSect.d: $(MAPI_DIR)/src/$(*B).i
	makedfromi.py -o$*.d -p PyIMAPIProp $(MAPI_DIR)/src/$(*B).i

# Exchange stuff.
$(GENDIR)\exchange.d: $(MAPI_DIR)/src/$(*B).i
	makedfromi.py -o$*.d $(MAPI_DIR)/src/$(*B).i

# Exchange stuff.
$(GENDIR)\exchdapi.d: $(MAPI_DIR)/src/$(*B).i
	makedfromi.py -o$*.d $(MAPI_DIR)/src/$(*B).i

# ADSI
$(GENDIR)\adsi.d: $(ADSI_DIR)/src/$(*B).i
	makedfromi.py -o$*.d $(ADSI_DIR)/src/$(*B).i

$(GENDIR)\PyIADsContainer.d: $(ADSI_DIR)/src/$(*B).i
	makedfromi.py -o$*.d $(ADSI_DIR)/src/$(*B).i

$(GENDIR)\PyIADsUser.d: $(ADSI_DIR)/src/$(*B).i
	makedfromi.py -o$*.d $(ADSI_DIR)/src/$(*B).i

$(GENDIR)\PyIDirectoryObject.d: $(ADSI_DIR)/src/$(*B).i
	makedfromi.py -o$*.d $(ADSI_DIR)/src/$(*B).i


!include "common.mak"

