# MAKEFILE
# Builds documentation for Pythonwin using the AUTODUCK tool
#

!include "common_top.mak"

TARGET  = PythonWin
GENDIR  = ..\build\Temp\Help
TITLE   = $(TARGET) Help
DOCHDR  = $(TARGET) Reference

SOURCE_DIR = ../pythonwin

# Name of the subdirectory to copy $(HTML_FILES) into
# for building of the .CHM file.
HTML_DIR = pythonwin
# Extraneous HTML files to include into the .CHM:
HTML_FILES = $(SOURCE_DIR)/readme.html $(SOURCE_DIR)/doc/* $(SOURCE_DIR)/doc/debugger/*

# Non-autoduck overview topics data file:
EXT_TOPICS = pythonwinOverviews.dat

SOURCE  = $(SOURCE_DIR)\contents.d $(SOURCE_DIR)\*.cpp $(SOURCE_DIR)\*.h 

# Help and Doc targets
all: help htmlhlp

help : $(GENDIR) ..\$(TARGET).hlp

htmlhlp : $(GENDIR) "..\$(TARGET).chm"

doc : $(TARGET).doc

clean: cleanad

!include "common.mak"
