!include "common_top.mak"

TARGET  = PyWin32
GENDIR  = ..\build\Temp\Help
TITLE   = Python for Win32 Extensions Help
DOCHDR  = Python for Win32 Extensions Reference

WIN32_SOURCE_DIR = ../win32/src
WIN32_HELP_DIR   = ../win32/help

WIN32COM_DIR = ../com/win32com
WIN32COMEXT_DIR = ../com/win32comext
MAPI_DIR = $(WIN32COMEXT_DIR)/mapi
ADSI_DIR = $(WIN32COMEXT_DIR)/adsi
WIN32COM_HELP_DIR = ../com/help

PYTHONWIN_DIR = ../pythonwin

# Extraneous HTML files to include into the .CHM:
HTML_FILES = $(WIN32_HELP_DIR)\*.html \
		$(WIN32COM_DIR)/*.htm* \
		$(WIN32COM_DIR)/HTML/*.html \
		$(WIN32COM_DIR)/HTML/image/* \
		$(WIN32COM_HELP_DIR)/*.htm* \
		$(WIN32COMEXT_DIR)/axscript/demos/client/ie/* \
		$(PYTHONWIN_DIR)/readme.html $(PYTHONWIN_DIR)/doc/* $(PYTHONWIN_DIR)/doc/debugger/* \


WIN32_SOURCE = $(WIN32_SOURCE_DIR)/*.cpp \
	  $(WIN32_SOURCE_DIR)/*.h \
	  $(WIN32_HELP_DIR)/*.d \
	  $(WIN32_SOURCE_DIR)/perfmon/*.cpp \
	  $(WIN32_SOURCE_DIR)/win32net/*.cpp \
	  $(WIN32_SOURCE_DIR)/win32wnet/*.cpp \
	  $(WIN32_SOURCE_DIR)/win32print/*.cpp \
	  $(GENDIR)/win32evtlog.d $(GENDIR)/win32event.d $(GENDIR)/win32file.d \
	  $(GENDIR)/win32service.d $(GENDIR)/win32pipe.d $(GENDIR)/win32security.d \
	  $(GENDIR)/win32process.d $(GENDIR)/wincerapi.d $(GENDIR)/win32gui.d

WIN32COM_SOURCE = \
	  $(WIN32COM_DIR)\src\*.cpp \
	  $(WIN32COM_HELP_DIR)\*.d \
	  $(WIN32COM_DIR)\src\extensions\*.cpp \
	  $(WIN32COMEXT_DIR)\axscript\src\*.cpp \
	  $(WIN32COMEXT_DIR)\axdebug\src\*.cpp \
	  $(WIN32COMEXT_DIR)\axcontrol\src\*.cpp \
	  $(WIN32COMEXT_DIR)\shell\src\*.cpp \
	  $(WIN32COMEXT_DIR)\internet\src\*.cpp \
	  $(WIN32COMEXT_DIR)\taskscheduler\src\*.cpp \
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
	  $(GENDIR)\PyIADsContainer.d \
	  $(GENDIR)\PyIADsUser.d \
	  $(GENDIR)\PyIDirectoryObject.d \

PYTHONWIN_SOURCE = \
	  $(PYTHONWIN_DIR)\contents.d $(PYTHONWIN_DIR)\*.cpp $(PYTHONWIN_DIR)\*.h

SOURCE=$(WIN32_SOURCE) $(WIN32COM_SOURCE) $(PYTHONWIN_SOURCE)

DOCUMENT_FILE = pywin32-document.xml

# Help and Doc targets
all: help htmlhlp

help : $(GENDIR) "..\$(TARGET).hlp"

htmlhlp: $(GENDIR) "..\$(TARGET).chm"

doc : "$(TARGET).doc"

clean: cleanad

"$(GENDIR)\$(TARGET).hhc" : $(SOURCE) Dump2HHC.py $(DOCUMENT_FILE) 
    rem Run autoduck over each category so we can create a nested TOC.
    $(ADHTMLFMT) /r html "/O$(GENDIR)\temp.html" "/G$(GENDIR)\win32.dump" /t8 $(WIN32_SOURCE)
    $(ADHTMLFMT) /r html "/O$(GENDIR)\temp.html" "/G$(GENDIR)\pythonwin.dump" /t8 $(PYTHONWIN_SOURCE)
    $(ADHTMLFMT) /r html "/O$(GENDIR)\temp.html" "/G$(GENDIR)\com.dump" /t8 $(WIN32COM_SOURCE)
    $(PYTHON) Dump2HHC.py "$(GENDIR)" "$(GENDIR)\$(TARGET).hhc" "$(TITLE)" "$(TARGET)"
    @del $(GENDIR)\win32.dump
    @del $(GENDIR)\pythonwin.dump
    @del $(GENDIR)\com.dump


##
## win32 generated
##
$(GENDIR)/win32file.d: $(WIN32_SOURCE_DIR)/win32file.i
	$(PYTHON) makedfromi.py -o$*.d $(WIN32_SOURCE_DIR)/$(*B).i

$(GENDIR)/win32event.d: $(WIN32_SOURCE_DIR)/win32event.i
	$(PYTHON) makedfromi.py -o$*.d $(WIN32_SOURCE_DIR)/$(*B).i

$(GENDIR)/win32evtlog.d: $(WIN32_SOURCE_DIR)/win32evtlog.i
	$(PYTHON) makedfromi.py -o$*.d $(WIN32_SOURCE_DIR)/$(*B).i

$(GENDIR)/win32service.d: $(WIN32_SOURCE_DIR)/win32service.i
	$(PYTHON) makedfromi.py -o$*.d $(WIN32_SOURCE_DIR)/$(*B).i

$(GENDIR)/win32pipe.d: $(WIN32_SOURCE_DIR)/win32pipe.i
	$(PYTHON) makedfromi.py -o$*.d $(WIN32_SOURCE_DIR)/$(*B).i

$(GENDIR)/win32security.d: $(WIN32_SOURCE_DIR)/$(*B).i
	$(PYTHON) makedfromi.py -o$*.d $(WIN32_SOURCE_DIR)/$(*B).i

$(GENDIR)/win32process.d: $(WIN32_SOURCE_DIR)/$(*B).i
	$(PYTHON) makedfromi.py -o$*.d $(WIN32_SOURCE_DIR)/$(*B).i

$(GENDIR)/wincerapi.d: $(WIN32_SOURCE_DIR)/$(*B).i
	$(PYTHON) makedfromi.py -o$*.d $(WIN32_SOURCE_DIR)/$(*B).i

$(GENDIR)/win32gui.d: $(WIN32_SOURCE_DIR)/$(*B).i
	$(PYTHON) makedfromi.py -o$*.d $(WIN32_SOURCE_DIR)/$(*B).i

##
## win32com generated
##
$(GENDIR)\mapi.d: $(MAPI_DIR)/src/$(*B).i
	$(PYTHON) makedfromi.py -o$*.d $(MAPI_DIR)/src/$(*B).i

$(GENDIR)\PyIABContainer.d: $(MAPI_DIR)/src/$(*B).i
	$(PYTHON) makedfromi.py -o$*.d -p PyIMAPIContainer $(MAPI_DIR)/src/$(*B).i

$(GENDIR)\PyIAddrBook.d: $(MAPI_DIR)/src/$(*B).i
	$(PYTHON) makedfromi.py -o$*.d -p PyIMAPIProp $(MAPI_DIR)/src/$(*B).i

$(GENDIR)\PyIAttach.d: $(MAPI_DIR)/src/$(*B).i
	$(PYTHON) makedfromi.py -o$*.d -p PyIMAPIProp $(MAPI_DIR)/src/$(*B).i

$(GENDIR)\PyIDistList.d: $(MAPI_DIR)/src/$(*B).i
	$(PYTHON) makedfromi.py -o$*.d -p PyIMAPIProp $(MAPI_DIR)/src/$(*B).i

$(GENDIR)\PyIMailUser.d: $(MAPI_DIR)/src/$(*B).i
	$(PYTHON) makedfromi.py -o$*.d -p PyIMAPIContainer $(MAPI_DIR)/src/$(*B).i

$(GENDIR)\PyIMAPIContainer.d: $(MAPI_DIR)/src/$(*B).i
	$(PYTHON) makedfromi.py -o$*.d -p PyIMAPIProp $(MAPI_DIR)/src/$(*B).i

$(GENDIR)\PyIMAPIFolder.d: $(MAPI_DIR)/src/$(*B).i
	$(PYTHON) makedfromi.py -o$*.d -p PyIMAPIProp $(MAPI_DIR)/src/$(*B).i

$(GENDIR)\PyIMAPIProp.d: $(MAPI_DIR)/src/$(*B).i
	$(PYTHON) makedfromi.py -o$*.d -p PyIUnknown $(MAPI_DIR)/src/$(*B).i

$(GENDIR)\PyIMAPISession.d: $(MAPI_DIR)/src/$(*B).i
	$(PYTHON) makedfromi.py -o$*.d -p PyIUnknown $(MAPI_DIR)/src/$(*B).i

$(GENDIR)\PyIMAPITable.d: $(MAPI_DIR)/src/$(*B).i
	$(PYTHON) makedfromi.py -o$*.d -p PyIUnknown $(MAPI_DIR)/src/$(*B).i

$(GENDIR)\PyIMessage.d: $(MAPI_DIR)/src/$(*B).i
	$(PYTHON) makedfromi.py -o$*.d -p PyIMAPIProp $(MAPI_DIR)/src/$(*B).i

$(GENDIR)\PyIMsgServiceAdmin.d: $(MAPI_DIR)/src/$(*B).i
	$(PYTHON) makedfromi.py -o$*.d -p PyIUnknown $(MAPI_DIR)/src/$(*B).i

$(GENDIR)\PyIMsgStore.d: $(MAPI_DIR)/src/$(*B).i
	$(PYTHON) makedfromi.py -o$*.d -p PyIMAPIProp $(MAPI_DIR)/src/$(*B).i

$(GENDIR)\PyIProfAdmin.d: $(MAPI_DIR)/src/$(*B).i
	$(PYTHON) makedfromi.py -o$*.d -p PyIUnknown $(MAPI_DIR)/src/$(*B).i

$(GENDIR)\PyIProfSect.d: $(MAPI_DIR)/src/$(*B).i
	$(PYTHON) makedfromi.py -o$*.d -p PyIMAPIProp $(MAPI_DIR)/src/$(*B).i

# Exchange stuff.
$(GENDIR)\exchange.d: $(MAPI_DIR)/src/$(*B).i
	$(PYTHON) makedfromi.py -o$*.d $(MAPI_DIR)/src/$(*B).i

# Exchange stuff.
$(GENDIR)\exchdapi.d: $(MAPI_DIR)/src/$(*B).i
	$(PYTHON) makedfromi.py -o$*.d $(MAPI_DIR)/src/$(*B).i

# ADSI
$(GENDIR)\adsi.d: $(ADSI_DIR)/src/$(*B).i
	$(PYTHON) makedfromi.py -o$*.d $(ADSI_DIR)/src/$(*B).i

$(GENDIR)\PyIADsContainer.d: $(ADSI_DIR)/src/$(*B).i
	$(PYTHON) makedfromi.py -o$*.d $(ADSI_DIR)/src/$(*B).i

$(GENDIR)\PyIADsUser.d: $(ADSI_DIR)/src/$(*B).i
	$(PYTHON) makedfromi.py -o$*.d $(ADSI_DIR)/src/$(*B).i

$(GENDIR)\PyIDirectoryObject.d: $(ADSI_DIR)/src/$(*B).i
	$(PYTHON) makedfromi.py -o$*.d $(ADSI_DIR)/src/$(*B).i


!include "common.mak"

