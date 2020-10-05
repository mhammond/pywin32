# Make file for Scintilla on Windows
# Copyright 1998-2010 by Neil Hodgson <neilh@scintilla.org>
# The License.txt file describes the conditions under which this software may be distributed.
# This makefile assumes the MinGW32 version of GCC 4.x is used and changes will
# be needed to use other compilers.

.SUFFIXES: .cxx

RANLIB ?= ranlib
WINDRES ?= windres
CXX = g++

DIR_O=$(SUB_DIR_O)
DIR_BIN=$(SUB_DIR_BIN)

COMPONENT = $(DIR_BIN)/scintilla.dll
LEXCOMPONENT = $(DIR_BIN)/scilexer.dll
LEXLIB = Lexers.a

vpath %.h ../src ../include
vpath %.cxx ../src

LDFLAGS=-shared -static-libstdc++ -static-libgcc -s
LIBS=-luser32 -lgdi32 -limm32 -lole32 -luuid
INCLUDEDIRS=-I../include -I../src

CXXBASEFLAGS=-std=c++11 -Wall -fpermissive -fno-strict-aliasing

ifeq ($(QUIET), 1)
CXXBASEFLAGS=$(CXXBASEFLAGS) -w
endif

ifeq ($(DEBUG), 1)
CXXFLAGS=-DDEBUG -g $(CXXBASEFLAGS) $(INCLUDEDIRS)
else
CXXFLAGS=-DNDEBUG -Os $(CXXBASEFLAGS) $(INCLUDEDIRS)
endif


.PHONY: all clean silent

all:	$(COMPONENT) | silent

clean:
	rm -f $(DIR_O)/*.o $(DIR_O)/*.pdb $(COMPONENT) $(LEXCOMPONENT) \
	$(DIR_O)/*.res $(DIR_BIN)/*.map $(DIR_BIN)/*.exp $(DIR_BIN)/*.pdb $(DIR_BIN)/*.a

silent:
	@:

SOBJS = \
	$(DIR_O)/AutoComplete.o \
	$(DIR_O)/CallTip.o \
	$(DIR_O)/CellBuffer.o \
	$(DIR_O)/CharClassify.o \
	$(DIR_O)/ContractionState.o \
	$(DIR_O)/Decoration.o \
	$(DIR_O)/Document.o \
	$(DIR_O)/Editor.o \
	$(DIR_O)/Indicator.o \
	$(DIR_O)/KeyMap.o \
	$(DIR_O)/LineMarker.o \
	$(DIR_O)/PlatWin.o \
	$(DIR_O)/PositionCache.o \
	$(DIR_O)/PropSet.o \
	$(DIR_O)/RESearch.o \
	$(DIR_O)/RunStyles.o \
	$(DIR_O)/ScintillaBase.o \
	$(DIR_O)/ScintillaWin.o \
	$(DIR_O)/Style.o \
	$(DIR_O)/UniConversion.o \
	$(DIR_O)/ViewStyle.o \
	$(DIR_O)/XPM.o

LEXOBJS=\
	$(DIR_O)/LexAbaqus.o \
	$(DIR_O)/LexAda.o \
	$(DIR_O)/LexAPDL.o \
	$(DIR_O)/LexAsm.o \
	$(DIR_O)/LexAsn1.o \
	$(DIR_O)/LexASY.o \
	$(DIR_O)/LexAU3.o \
	$(DIR_O)/LexAVE.o \
	$(DIR_O)/LexBaan.o \
	$(DIR_O)/LexBash.o \
	$(DIR_O)/LexBasic.o \
	$(DIR_O)/LexBullant.o \
	$(DIR_O)/LexCaml.o \
	$(DIR_O)/LexCLW.o \
	$(DIR_O)/LexCmake.o \
	$(DIR_O)/LexConf.o \
	$(DIR_O)/LexCPP.o \
	$(DIR_O)/LexCrontab.o \
	$(DIR_O)/LexCsound.o \
	$(DIR_O)/LexCSS.o \
	$(DIR_O)/LexD.o \
	$(DIR_O)/LexEiffel.o \
	$(DIR_O)/LexErlang.o \
	$(DIR_O)/LexEScript.o \
	$(DIR_O)/LexFlagship.o \
	$(DIR_O)/LexForth.o \
	$(DIR_O)/LexFortran.o \
	$(DIR_O)/LexGAP.o \
	$(DIR_O)/LexGui4Cli.o \
	$(DIR_O)/LexHaskell.o \
	$(DIR_O)/LexHTML.o \
	$(DIR_O)/LexInno.o \
	$(DIR_O)/LexKix.o \
	$(DIR_O)/LexLisp.o \
	$(DIR_O)/LexLout.o \
	$(DIR_O)/LexLua.o \
	$(DIR_O)/LexMagik.o \
	$(DIR_O)/LexMatlab.o \
	$(DIR_O)/LexMetapost.o \
	$(DIR_O)/LexMMIXAL.o \
	$(DIR_O)/LexMPT.o \
	$(DIR_O)/LexMSSQL.o \
	$(DIR_O)/LexMySQL.o \
	$(DIR_O)/LexNsis.o \
	$(DIR_O)/LexOpal.o \
	$(DIR_O)/LexOthers.o \
	$(DIR_O)/LexPascal.o \
	$(DIR_O)/LexPB.o \
	$(DIR_O)/LexPerl.o \
	$(DIR_O)/LexPLM.o \
	$(DIR_O)/LexPOV.o \
	$(DIR_O)/LexPowerShell.o \
	$(DIR_O)/LexProgress.o \
	$(DIR_O)/LexPS.o \
	$(DIR_O)/LexPython.o \
	$(DIR_O)/LexR.o \
	$(DIR_O)/LexRebol.o \
	$(DIR_O)/LexRuby.o \
	$(DIR_O)/LexScriptol.o \
	$(DIR_O)/LexSmalltalk.o \
	$(DIR_O)/LexSpecman.o \
	$(DIR_O)/LexSpice.o \
	$(DIR_O)/LexSQL.o \
	$(DIR_O)/LexTADS3.o \
	$(DIR_O)/LexTCL.o \
	$(DIR_O)/LexTeX.o \
	$(DIR_O)/LexVB.o \
	$(DIR_O)/LexVerilog.o \
	$(DIR_O)/LexVHDL.o \
	$(DIR_O)/LexYAML.o

LOBJS=\
	$(DIR_O)/AutoComplete.o \
	$(DIR_O)/CallTip.o \
	$(DIR_O)/CellBuffer.o \
	$(DIR_O)/CharClassify.o \
	$(DIR_O)/ContractionState.o \
	$(DIR_O)/Decoration.o \
	$(DIR_O)/Document.o \
	$(DIR_O)/DocumentAccessor.o \
	$(DIR_O)/Editor.o \
	$(DIR_O)/ExternalLexer.o \
	$(DIR_O)/Indicator.o \
	$(DIR_O)/KeyMap.o \
	$(DIR_O)/KeyWords.o \
	$(DIR_O)/LineMarker.o \
	$(DIR_O)/PlatWin.o \
	$(DIR_O)/PositionCache.o \
	$(DIR_O)/PropSet.o \
	$(DIR_O)/RESearch.o \
	$(DIR_O)/RunStyles.o \
	$(DIR_O)/Style.o \
	$(DIR_O)/StyleContext.o \
	$(DIR_O)/UniConversion.o \
	$(DIR_O)/ViewStyle.o \
	$(DIR_O)/XPM.o \
	$(LEXOBJS)

$(DIR_O)/ScintRes.rc.o: ScintRes.rc
	$(WINDRES) ScintRes.rc $@

$(COMPONENT): $(SOBJS) $(DIR_O)/ScintRes.rc.o
	$(CXX) $(LDFLAGS) -o $@ $(SOBJS) $(DIR_O)/ScintRes.rc.o $(LIBS)

$(LEXCOMPONENT): $(LOBJS) $(DIR_O)/ScintRes.rc.o
	$(CXX) $(LDFLAGS) -o $@ $(LOBJS) $(DIR_O)/ScintRes.rc.o $(LIBS)

$(LEXLIB): $(LOBJS)
	$(AR) rc $@ $^
	$(RANLIB) $@

# Define how to build all the objects and what they depend on

$(DIR_O)/%.o: %.cxx
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Some source files are compiled into more than one object because of different conditional compilation
$(DIR_O)/ScintillaBaseL.o: ../src/ScintillaBase.cxx
	$(CXX) $(CXXFLAGS) -DSCI_LEXER -c ../src/ScintillaBase.cxx -o $@

$(DIR_O)/ScintillaWinL.o: ScintillaWin.cxx
	$(CXX) $(CXXFLAGS) -DSCI_LEXER -c ScintillaWin.cxx -o $@

$(DIR_O)/ScintillaWinS.o: ScintillaWin.cxx
	$(CXX) $(CXXFLAGS) -DSTATIC_BUILD -c ScintillaWin.cxx -o $@


$(DIR_O)/ScintillaBaseL.o: ../src/ScintillaBase.cxx ../include/Platform.h \
  ../include/Scintilla.h ../include/PropSet.h ../include/SString.h \
  ../src/ContractionState.h ../src/SVector.h ../src/SplitVector.h \
  ../src/Partitioning.h ../src/RunStyles.h ../src/CellBuffer.h \
  ../src/CallTip.h ../src/KeyMap.h ../src/Indicator.h ../src/XPM.h \
  ../src/LineMarker.h ../src/Style.h ../src/ViewStyle.h \
  ../src/AutoComplete.h ../src/CharClassify.h ../src/Decoration.h \
  ../src/Document.h ../src/Editor.h ../src/ScintillaBase.h
$(DIR_O)/ScintillaWinL.o: ScintillaWin.cxx ../include/Platform.h \
  ../include/Scintilla.h ../include/SString.h ../src/ContractionState.h \
  ../src/SVector.h ../src/SplitVector.h ../src/Partitioning.h \
  ../src/RunStyles.h ../src/CellBuffer.h ../src/CallTip.h ../src/KeyMap.h \
  ../src/Indicator.h ../src/XPM.h ../src/LineMarker.h ../src/Style.h \
  ../src/AutoComplete.h ../src/ViewStyle.h ../src/CharClassify.h \
  ../src/Decoration.h ../src/Document.h ../src/Editor.h \
  ../src/ScintillaBase.h ../src/UniConversion.h
$(DIR_O)/ScintillaWinS.o: ScintillaWin.cxx ../include/Platform.h \
  ../include/Scintilla.h ../include/SString.h ../src/ContractionState.h \
  ../src/SVector.h ../src/SplitVector.h ../src/Partitioning.h \
  ../src/RunStyles.h ../src/CellBuffer.h ../src/CallTip.h ../src/KeyMap.h \
  ../src/Indicator.h ../src/XPM.h ../src/LineMarker.h ../src/Style.h \
  ../src/AutoComplete.h ../src/ViewStyle.h ../src/CharClassify.h \
  ../src/Decoration.h ../src/Document.h ../src/Editor.h \
  ../src/ScintillaBase.h ../src/UniConversion.h
