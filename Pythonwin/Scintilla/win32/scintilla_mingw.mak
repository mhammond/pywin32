# Make file for Scintilla on Windows
# Copyright 1998-2010 by Neil Hodgson <neilh@scintilla.org>
# The License.txt file describes the conditions under which this software may be distributed.
# This makefile assumes the MSYS2 + Mingw-w64 version of GCC 7.1+ are used and changes will
# be needed to use other compilers.

.SUFFIXES: .cxx

DIR_BIN=$(SUB_DIR_BIN)
DIR_O=$(SUB_DIR_O)

COMPONENT = $(DIR_BIN)/scintilla.dll
LIBSCI = $(DIR_BIN)/libscintilla.a

ifdef CLANG
CXX = clang++
# Clang doesn't like omitting braces in array initialization but they just add noise,
# __uuidof is a Microsoft extension but makes COM code neater,
# public visibility avoids warnings like 'locally defined symbol __std_terminate imported'
CLANG_OPTS = -Wno-missing-braces -Wno-language-extension-token -Xclang -flto-visibility-public-std
else
# MinGW GCC
CXX = g++
LDMINGW = -Wl,--enable-runtime-pseudo-reloc-v2 -Wl,--add-stdcall-alias -Wl,--export-all-symbols
LIBSMINGW = -static-libgcc -static-libstdc++ -static -pthread
STRIPOPTION = -s
endif

CXXSTD = c++17

ifeq ($(OS),Windows_NT)
DEL = $(if $(wildcard $(dir $(SHELL))rm.exe), $(dir $(SHELL))rm.exe -f, del /q)
else
DEL = rm -f
endif
RANLIB ?= ranlib
WINDRES ?= windres

vpath %.h ../src ../include ../lexlib
vpath %.cxx ../src ../lexlib ../lexers

LDFLAGS=-shared -mconsole $(LDMINGW)
LIBS=-lgdi32 -luser32 -limm32 -lole32 -luuid -loleaut32 -lmsimg32 $(LIBSMINGW)
# Add -MMD to get dependencies
INCLUDEDIRS=-I ../include -I ../src -I ../lexlib

CXXBASEFLAGS=-std=$(CXXSTD) -Wall -pedantic -fpermissive -fno-strict-aliasing -D_CRT_SECURE_NO_DEPRECATE=1 $(CLANG_OPTS) #-DMINGW_HAS_SECURE_API=1

ifdef NO_CXX11_REGEX
REFLAGS=-DNO_CXX11_REGEX
endif

ifeq ($(QUIET), 1)
CXXBASEFLAGS=$(CXXBASEFLAGS) -w
endif

ifeq ($(DEBUG), 1)
CXXFLAGS=-DDEBUG -g $(CXXBASEFLAGS)
else
CXXFLAGS=-DNDEBUG -Os $(CXXBASEFLAGS) $(INCLUDEDIRS)
STRIPFLAG=$(STRIPOPTION)
endif

.PHONY: all clean silent

all:	$(COMPONENT) | silent

clean:
	$(DEL) *.exe *.o *.a *.obj *.dll *.res *.map *.plist

$(DIR_O)/%.o: %.cxx
	$(CXX) $(CXXFLAGS) $(REFLAGS) -c $< -o $@

$(DIR_O)%.o: ../lexlib/%.cxx
	$(CXX) $(CXXFLAGS) $(REFLAGS) -c $< -o $@

analyze:
	$(CXX) --analyze $(CXXFLAGS) *.cxx ../src/*.cxx ../lexlib/*.cxx

depend deps.mak:
	python DepGen.py

silent:
	@:

LEXOBJS:=$(addsuffix .o,$(basename $(sort $(notdir $(wildcard ../lexers/Lex*.cxx)))))

# Required for base Scintilla
BASEOBJS = \
	$(DIR_O)/AutoComplete.o \
	$(DIR_O)/CallTip.o \
	$(DIR_O)/CaseConvert.o \
	$(DIR_O)/CaseFolder.o \
	$(DIR_O)/CellBuffer.o \
	$(DIR_O)/CharacterCategory.o \
	$(DIR_O)/CharacterSet.o \
	$(DIR_O)/CharClassify.o \
	$(DIR_O)/ContractionState.o \
	$(DIR_O)/DBCS.o \
	$(DIR_O)/Decoration.o \
	$(DIR_O)/Document.o \
	$(DIR_O)/EditModel.o \
	$(DIR_O)/Editor.o \
	$(DIR_O)/EditView.o \
	$(DIR_O)/KeyMap.o \
	$(DIR_O)/Indicator.o \
	$(DIR_O)/LineMarker.o \
	$(DIR_O)/MarginView.o \
	$(DIR_O)/PerLine.o \
	$(DIR_O)/PlatWin.o \
	$(DIR_O)/PositionCache.o \
	$(DIR_O)/PropSetSimple.o \
	$(DIR_O)/RESearch.o \
	$(DIR_O)/RunStyles.o \
	$(DIR_O)/ScintRes.o \
	$(DIR_O)/Selection.o \
	$(DIR_O)/Style.o \
	$(DIR_O)/UniConversion.o \
	$(DIR_O)/UniqueString.o \
	$(DIR_O)/ViewStyle.o \
	$(DIR_O)/XPM.o \
	$(DIR_O)/HanjaDic.o

SOBJS = $(DIR_O)/ScintillaDLL.o $(DIR_O)/ScintillaWin.o $(DIR_O)/ScintillaBase.o $(BASEOBJS)

# Required by lexers
LEXLIBOBJS=\
	$(DIR_O)/Accessor.o \
	$(DIR_O)/Catalogue.o \
	$(DIR_O)/ExternalLexer.o \
	$(DIR_O)/DefaultLexer.o \
	$(DIR_O)/LexerBase.o \
	$(DIR_O)/LexerModule.o \
	$(DIR_O)/LexerSimple.o \
	$(DIR_O)/StyleContext.o \
	$(DIR_O)/WordList.o \

$(COMPONENT): $(SOBJS)
	$(CXX) $(LDFLAGS) $(STRIPFLAG) -o $@ -Wl,--out-implib,$(LIBSCI) $(SOBJS) $(LIBS)


# Automatically generate dependencies for most files with "make deps"
include deps.mak

$(DIR_O)/ScintRes.o:	ScintRes.rc
	$(WINDRES) ScintRes.rc $@

