!include "common_top.mak"

TARGET  = PyWin32
GENDIR  = ..\build\temp.autoduck
# how to get back to the build dir from $GENDIR
MYDIR_FROM_GENDIR  = ..\..\AutoDuck
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
ISAPI_DIR = ../isapi
ISAPI_SOURCE_DIR = $(ISAPI_DIR)/src


# Extraneous HTML files to include into the .CHM:
HTML_FILES = $(WIN32_HELP_DIR)\*.html \
		$(WIN32COM_DIR)/*.htm* \
		$(WIN32COM_DIR)/HTML/*.html \
		$(WIN32COM_DIR)/HTML/image/* \
		$(WIN32COM_HELP_DIR)/*.htm* \
		$(WIN32COMEXT_DIR)/axscript/demos/client/ie/* \
		$(ISAPI_DIR)/doc/*.html \
		$(PYTHONWIN_DIR)/readme.html $(PYTHONWIN_DIR)/doc/* $(PYTHONWIN_DIR)/doc/debugger/* \
		../CHANGES.txt \


WIN32_SOURCE = $(WIN32_SOURCE_DIR)/*.cpp \
	  $(WIN32_SOURCE_DIR)/*.h \
	  $(WIN32_HELP_DIR)/*.d \
	  $(WIN32_SOURCE_DIR)/perfmon/*.cpp \
	  $(WIN32_SOURCE_DIR)/win32net/*.cpp \
	  $(WIN32_SOURCE_DIR)/win32wnet/*.cpp \
	  $(WIN32_SOURCE_DIR)/win32print/*.cpp \
	  $(WIN32_SOURCE_DIR)/win32crypt/*.cpp \
	  $(GENDIR)/win32evtlog.d $(GENDIR)/win32event.d $(GENDIR)/win32file.d \
	  $(GENDIR)/win32service.d $(GENDIR)/win32pipe.d $(GENDIR)/win32security.d \
	  $(GENDIR)/win32process.d $(GENDIR)/wincerapi.d $(GENDIR)/win32gui.d \
	  $(GENDIR)/win32inet.d $(GENDIR)/_winxptheme.d \
	  $(GENDIR)/win32job.d \
	  winxpgui.d

WIN32COM_SOURCE = \
	  $(WIN32COM_DIR)\src\*.cpp \
	  $(WIN32COM_HELP_DIR)\*.d \
	  $(WIN32COM_DIR)\src\extensions\*.cpp \
	  $(WIN32COMEXT_DIR)\axscript\src\*.cpp \
	  $(WIN32COMEXT_DIR)\axdebug\src\*.cpp \
	  $(WIN32COMEXT_DIR)\axcontrol\src\*.cpp \
	  $(WIN32COMEXT_DIR)\shell\src\*.cpp \
	  $(WIN32COMEXT_DIR)\shell\src\*.h \
	  $(WIN32COMEXT_DIR)\internet\src\*.cpp \
	  $(WIN32COMEXT_DIR)\taskscheduler\src\*.cpp \
	  $(WIN32COMEXT_DIR)\authorization\src\*.cpp \
	  $(WIN32COMEXT_DIR)\authorization\src\*.h \
	  $(WIN32COMEXT_DIR)\directsound\src\*.cpp \
	  $(WIN32COMEXT_DIR)\propsys\src\*.cpp \
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
	  $(GENDIR)\PyIMAPIStatus.d \
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
	  $(GENDIR)\PyIDirectorySearch.d \
	  $(GENDIR)\PyIDsObjectPicker.d \

PYTHONWIN_SOURCE = \
	  $(PYTHONWIN_DIR)\contents.d $(PYTHONWIN_DIR)\*.cpp $(PYTHONWIN_DIR)\*.h

ISAPI_SOURCE = \
    $(ISAPI_SOURCE_DIR)\*.cpp $(ISAPI_SOURCE_DIR)\*.h $(GENDIR)\isapi_modules.d

GENERATED_D = $(GENDIR)\sspi.d $(GENDIR)\win32timezone.d

SOURCE=$(WIN32_SOURCE) $(WIN32COM_SOURCE) $(PYTHONWIN_SOURCE) $(ISAPI_SOURCE) $(GENERATED_D)

DOCUMENT_FILE = pywin32-document.xml

# Help and Doc targets
all: htmlhlp

help : $(GENDIR) "..\$(TARGET).hlp"

htmlhlp: $(GENDIR) "..\$(TARGET).chm"

doc : "$(TARGET).doc"

clean: cleanad

pseudo:

$(GENDIR)\isapi_modules.d: py2d.py pseudo
    $(PYTHON) py2d.py isapi isapi.install isapi.simple isapi.threaded_extension isapi.isapicon > $(GENDIR)\isapi_modules.d

$(GENDIR)\sspi.d: py2d.py pseudo
    $(PYTHON) py2d.py sspi > $(GENDIR)\sspi.d

$(GENDIR)\win32timezone.d: py2d.py pseudo
    $(PYTHON) py2d.py win32timezone > $(GENDIR)\win32timezone.d

"$(GENDIR)\$(TARGET).hhc" : $(SOURCE) Dump2HHC.py $(DOCUMENT_FILE) 
    rem Run autoduck over each category so we can create a nested TOC.
    $(ADHTMLFMT) /r html "/O$(GENDIR)\temp.html" "/G$(GENDIR)\win32.dump" /t8 $(WIN32_SOURCE)
    $(ADHTMLFMT) /r html "/O$(GENDIR)\temp.html" "/G$(GENDIR)\pythonwin.dump" /t8 $(PYTHONWIN_SOURCE)
    $(ADHTMLFMT) /r html "/O$(GENDIR)\temp.html" "/G$(GENDIR)\com.dump" /t8 $(WIN32COM_SOURCE)
    $(ADHTMLFMT) /r html "/O$(GENDIR)\temp.html" "/G$(GENDIR)\isapi.dump" /t8 $(ISAPI_SOURCE)
    $(PYTHON) Dump2HHC.py "$(GENDIR)" "$(GENDIR)\$(TARGET).hhc" "$(TITLE)" "$(TARGET)"


##
## win32 generated
##
$(GENDIR)/win32inet.d: $(WIN32_SOURCE_DIR)/win32inet.i
	$(PYTHON) makedfromi.py -o$*.d $(WIN32_SOURCE_DIR)/$(*B).i
    
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

$(GENDIR)/_winxptheme.d: $(WIN32_SOURCE_DIR)/$(*B).i
	$(PYTHON) makedfromi.py -o$*.d $(WIN32_SOURCE_DIR)/$(*B).i

$(GENDIR)/win32crypt.d: $(WIN32_SOURCE_DIR)/$(*B).i
	$(PYTHON) makedfromi.py -o$*.d $(WIN32_SOURCE_DIR)/$(*B).i

$(GENDIR)/win32job.d: $(WIN32_SOURCE_DIR)/$(*B).i
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

$(GENDIR)\PyIMAPIStatus.d: $(MAPI_DIR)/src/$(*B).i
	$(PYTHON) makedfromi.py -o$*.d -p PyIMAPIProp $(MAPI_DIR)/src/$(*B).i

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
	$(PYTHON) makedfromi.py -o$*.d -p PyIUnknown $(ADSI_DIR)/src/$(*B).i

$(GENDIR)\PyIADsUser.d: $(ADSI_DIR)/src/$(*B).i
	$(PYTHON) makedfromi.py -o$*.d -p PyIDispatch $(ADSI_DIR)/src/$(*B).i

$(GENDIR)\PyIDirectoryObject.d: $(ADSI_DIR)/src/$(*B).i
	$(PYTHON) makedfromi.py -o$*.d -p PyIUnknown $(ADSI_DIR)/src/$(*B).i

$(GENDIR)\PyIDirectorySearch.d: $(ADSI_DIR)/src/$(*B).i
	$(PYTHON) makedfromi.py -o$*.d -p PyIUnknown $(ADSI_DIR)/src/$(*B).i

$(GENDIR)\PyIDsObjectPicker.d: $(ADSI_DIR)/src/$(*B).i
	$(PYTHON) makedfromi.py -o$*.d -p PyIUnknown $(ADSI_DIR)/src/$(*B).i

!include "common.mak"

