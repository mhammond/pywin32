# MAKEFILE
# Builds documentation for Pythonwin using the AUTODUCK tool
#

!include "common_top.mak"

TARGET  = PythonWin
GENDIR  = ..\build\Temp\Help
TITLE   = $(TARGET) Help
DOCHDR  = $(TARGET) Reference

SOURCE_DIR = ../pythonwin
SOURCE  = $(SOURCE_DIR)\contents.d $(SOURCE_DIR)\*.cpp $(SOURCE_DIR)\*.h 

# Help and Doc targets
all: help htmlhlp

help : $(GENDIR) ..\$(TARGET).hlp

htmlhlp : $(GENDIR) "..\$(TARGET).chm"

doc : $(TARGET).doc

clean: cleanad

!include "common.mak"
