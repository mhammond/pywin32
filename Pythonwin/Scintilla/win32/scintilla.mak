# Make file for Scintilla on Windows Visual C++ and Borland C++ version
# Copyright 1998-2001 by Neil Hodgson <neilh@scintilla.org>
# The License.txt file describes the conditions under which this software may be distributed.
# This makefile is for using Visual C++ with nmake or Borland C++ with make depending on
# the setting of the VENDOR macro. If no VENDOR is defined n the command line then
# the tool used is automatically detected.
# Usage for Microsoft:
#     nmake -f scintilla.mak
# Usage for Borland:
#     make -f scintilla.mak
# For debug versions define DEBUG on the command line, for Borland:
#     make DEBUG=1 -f scintilla.mak
# The main makefile uses mingw32 gcc and may be more current than this file.

.SUFFIXES: .cxx

DIR_O=.
DIR_BIN=..\bin

COMPONENT=$(DIR_BIN)\Scintilla.dll
LEXCOMPONENT=$(DIR_BIN)\SciLexer.dll

!IFNDEF VENDOR
!IFDEF _NMAKE_VER
#Microsoft nmake so make default VENDOR MICROSOFT
VENDOR=MICROSOFT
!ELSE
VENDOR=BORLAND
!ENDIF
!ENDIF

!IF "$(VENDOR)"=="MICROSOFT"

CC=cl
RC=rc
LD=link

INCLUDEDIRS=-I ../include -I ../src
CXXFLAGS=/TP /W4
# For something scary:/Wp64
CXXDEBUG=/Zi /Od /MDd -DDEBUG
CXXNDEBUG=/Ox /MD -DNDEBUG
NAMEFLAG=-Fo
LDFLAGS=/opt:nowin98
LDDEBUG=/DEBUG
LIBS=KERNEL32.lib USER32.lib GDI32.lib IMM32.lib OLE32.LIB

!IFDEF QUIET
CC=@$(CC)
CXXDEBUG=$(CXXDEBUG) /nologo
CXXNDEBUG=$(CXXNDEBUG) /nologo
LDFLAGS=$(LDFLAGS) /nologo
!ENDIF

!ELSE
# BORLAND

CC=bcc32
RC=brcc32 -r
LD=ilink32

INCLUDEDIRS=-I../include -I../src
CXXFLAGS =  -v
CXXFLAGS=-P -tWM -w -w-prc -w-inl -RT- -x-
# Above turns off warnings for clarfying parentheses and inlines with for not expanded
CXXDEBUG=-v -DDEBUG
CXXNDEBUG=-O1 -DNDEBUG
NAMEFLAG=-o
LDFLAGS=
LDDEBUG=-v
LIBS=import32 cw32mt

!ENDIF

!IFDEF DEBUG
CXXFLAGS=$(CXXFLAGS) $(CXXDEBUG)
LDFLAGS=$(LDDEBUG) $(LDFLAGS)
!ELSE
CXXFLAGS=$(CXXFLAGS) $(CXXNDEBUG)
!ENDIF

ALL:	$(COMPONENT) $(LEXCOMPONENT) $(DIR_O)\ScintillaWinS.obj $(DIR_O)\WindowAccessor.obj

clean:
	-del /q $(DIR_O)\*.obj $(DIR_O)\*.pdb $(COMPONENT) $(LEXCOMPONENT) $(DIR_O)\*.res $(DIR_BIN)\*.map

SOBJS=\
	$(DIR_O)\AutoComplete.obj \
	$(DIR_O)\CallTip.obj \
	$(DIR_O)\CellBuffer.obj \
	$(DIR_O)\ContractionState.obj \
	$(DIR_O)\Document.obj \
	$(DIR_O)\Editor.obj \
	$(DIR_O)\Indicator.obj \
	$(DIR_O)\KeyMap.obj \
	$(DIR_O)\LineMarker.obj \
	$(DIR_O)\PlatWin.obj \
	$(DIR_O)\RESearch.obj \
	$(DIR_O)\ScintillaBase.obj \
	$(DIR_O)\ScintillaWin.obj \
	$(DIR_O)\Style.obj \
	$(DIR_O)\UniConversion.obj \
	$(DIR_O)\ViewStyle.obj

LEXOBJS=\
	$(DIR_O)\LexAda.obj \
	$(DIR_O)\LexAVE.obj \
	$(DIR_O)\LexConf.obj \
	$(DIR_O)\LexCPP.obj \
	$(DIR_O)\LexHTML.obj \
	$(DIR_O)\LexLua.obj \
	$(DIR_O)\LexOthers.obj \
	$(DIR_O)\LexPascal.obj \
	$(DIR_O)\LexPerl.obj \
	$(DIR_O)\LexPython.obj \
	$(DIR_O)\LexSQL.obj \
	$(DIR_O)\LexVB.obj

LOBJS=\
	$(DIR_O)\AutoComplete.obj \
	$(DIR_O)\CallTip.obj \
	$(DIR_O)\CellBuffer.obj \
	$(DIR_O)\ContractionState.obj \
	$(DIR_O)\Document.obj \
	$(DIR_O)\DocumentAccessor.obj \
	$(DIR_O)\Editor.obj \
	$(DIR_O)\Indicator.obj \
	$(DIR_O)\KeyMap.obj \
	$(DIR_O)\KeyWords.obj \
	$(DIR_O)\LineMarker.obj \
	$(DIR_O)\PlatWin.obj \
	$(DIR_O)\RESearch.obj \
	$(DIR_O)\PropSet.obj \
	$(DIR_O)\ScintillaBaseL.obj \
	$(DIR_O)\ScintillaWinL.obj \
	$(DIR_O)\Style.obj \
	$(DIR_O)\UniConversion.obj \
	$(DIR_O)\ViewStyle.obj \
	$(LEXOBJS)

!IF "$(VENDOR)"=="MICROSOFT"

$(COMPONENT): $(SOBJS) $(DIR_O)\ScintRes.res
	$(LD) $(LDFLAGS) /DLL /OUT:$@ $(SOBJS) $(DIR_O)\ScintRes.res $(LIBS)

$(DIR_O)\ScintRes.res : ScintRes.rc
	$(RC) /fo$@ $(*B).rc

$(LEXCOMPONENT): $(LOBJS) $(DIR_O)\ScintRes.res
	$(LD) $(LDFLAGS) /DLL /OUT:$@ $(LOBJS) $(DIR_O)\ScintRes.res $(LIBS)

!ELSE

$(COMPONENT): $(SOBJS) ScintRes.res
	$(LD) $(LDFLAGS) -Tpd -c c0d32 $(SOBJS), $@, , $(LIBS), , ScintRes.res

$(DIR_O)\ScintRes.res: ScintRes.rc
	$(RC) $*.rc

$(LEXCOMPONENT): $(LOBJS)
	$(LD) $(LDFLAGS) -Tpd -c c0d32 $(LOBJS), $@, , $(LIBS), , ScintRes.res

!ENDIF

# Define how to build all the objects and what they depend on

# Most of the source is in ..\src with a couple in this directory
{..\src}.cxx{$(DIR_O)}.obj:
	$(CC) $(INCLUDEDIRS) $(CXXFLAGS) -c $(NAMEFLAG)$@ $<
{.}.cxx{$(DIR_O)}.obj:
	$(CC) $(INCLUDEDIRS) $(CXXFLAGS) -c $(NAMEFLAG)$@ $<

# Some source files are compiled into more than one object because of different conditional compilation
$(DIR_O)\ScintillaBaseL.obj: ..\src\ScintillaBase.cxx
	$(CC) $(INCLUDEDIRS) $(CXXFLAGS) -DSCI_LEXER -c $(NAMEFLAG)$@ ..\src\ScintillaBase.cxx

$(DIR_O)\ScintillaWinL.obj: ScintillaWin.cxx
	$(CC) $(INCLUDEDIRS) $(CXXFLAGS) -DSCI_LEXER -c $(NAMEFLAG)$@ ScintillaWin.cxx

$(DIR_O)\ScintillaWinS.obj: ScintillaWin.cxx
	$(CC) $(INCLUDEDIRS) $(CXXFLAGS) -DSTATIC_BUILD -c $(NAMEFLAG)$@ ScintillaWin.cxx

# Dependencies

# All lexers depend on this set of headers
LEX_HEADERS=..\include\Platform.h ..\include\PropSet.h \
 ..\include\SString.h ..\include\Accessor.h ..\include\KeyWords.h \
 ..\include\Scintilla.h ..\include\SciLexer.h

$(DIR_O)\AutoComplete.obj: ..\src\AutoComplete.cxx ..\include\Platform.h ..\src\AutoComplete.h

$(DIR_O)\CallTip.obj: ..\src\CallTip.cxx ..\include\Platform.h ..\src\CallTip.h

$(DIR_O)\CellBuffer.obj: ..\src\CellBuffer.cxx ..\include\Platform.h ..\include\Scintilla.h ..\src\CellBuffer.h

$(DIR_O)\ContractionState.obj: ..\src\ContractionState.cxx ..\include\Platform.h ..\src\ContractionState.h

$(DIR_O)\Document.obj: ..\src\Document.cxx ..\include\Platform.h ..\include\Scintilla.h \
 ..\src\RESearch.h ..\src\CellBuffer.h ..\src\Document.h

$(DIR_O)\DocumentAccessor.obj: ..\src\DocumentAccessor.cxx ..\include\Platform.h ..\include\PropSet.h \
 ..\include\SString.h ..\include\Accessor.h ..\src\DocumentAccessor.h ..\include\Scintilla.h

$(DIR_O)\Editor.obj: ..\src\Editor.cxx ..\include\Platform.h ..\include\Scintilla.h ..\src\ContractionState.h \
 ..\src\CellBuffer.h ..\src\KeyMap.h ..\src\Indicator.h ..\src\LineMarker.h ..\src\Style.h ..\src\ViewStyle.h \
 ..\src\Document.h ..\src\Editor.h

$(DIR_O)\Indicator.obj: ..\src\Indicator.cxx ..\include\Platform.h ..\include\Scintilla.h ..\src\Indicator.h

$(DIR_O)\KeyMap.obj: ..\src\KeyMap.cxx ..\include\Platform.h ..\include\Scintilla.h ..\src\KeyMap.h

$(DIR_O)\KeyWords.obj: ..\src\KeyWords.cxx ..\include\Platform.h ..\include\PropSet.h \
 ..\include\SString.h ..\include\Accessor.h ..\include\KeyWords.h \
 ..\include\Scintilla.h ..\include\SciLexer.h

$(DIR_O)\LexAda.obj: ..\src\LexAda.cxx $(LEX_HEADERS)

$(DIR_O)\LexAVE.obj: ..\src\LexAVE.cxx $(LEX_HEADERS)

$(DIR_O)\LexConf.obj: ..\src\LexConf.cxx $(LEX_HEADERS)

$(DIR_O)\LexCPP.obj: ..\src\LexCPP.cxx $(LEX_HEADERS)

$(DIR_O)\LexHTML.obj: ..\src\LexHTML.cxx $(LEX_HEADERS)

$(DIR_O)\LexLua.obj: ..\src\LexLua.cxx $(LEX_HEADERS)

$(DIR_O)\LexOthers.obj: ..\src\LexOthers.cxx $(LEX_HEADERS)

$(DIR_O)\LexPerl.obj: ..\src\LexPerl.cxx $(LEX_HEADERS)

$(DIR_O)\LexPascal.obj: ..\src\LexPascal.cxx $(LEX_HEADERS)

$(DIR_O)\LexPython.obj: ..\src\LexPython.cxx $(LEX_HEADERS)

$(DIR_O)\LexSQL.obj: ..\src\LexSQL.cxx $(LEX_HEADERS)

$(DIR_O)\LexVB.obj: ..\src\LexVB.cxx $(LEX_HEADERS)

$(DIR_O)\LineMarker.obj: ..\src\LineMarker.cxx ..\include\Platform.h ..\include\Scintilla.h ..\src\LineMarker.h

$(DIR_O)\PlatWin.obj: PlatWin.cxx ..\include\Platform.h PlatformRes.h ..\src\UniConversion.h

$(DIR_O)\RESearch.obj: ..\src\RESearch.cxx ..\src\RESearch.h

$(DIR_O)\PropSet.obj: ..\src\PropSet.cxx ..\include\Platform.h ..\include\PropSet.h \
 ..\include\SString.h

$(DIR_O)\ScintillaBase.obj: ..\src\ScintillaBase.cxx ..\include\Platform.h ..\include\Scintilla.h \
 ..\src\ContractionState.h ..\src\CellBuffer.h ..\src\CallTip.h ..\src\KeyMap.h ..\src\Indicator.h \
 ..\src\LineMarker.h ..\src\Style.h ..\src\ViewStyle.h ..\src\AutoComplete.h ..\src\Document.h ..\src\Editor.h \
 ..\src\ScintillaBase.h

$(DIR_O)\ScintillaBaseL.obj: ..\src\ScintillaBase.cxx ..\include\Platform.h ..\include\Scintilla.h ..\include\SciLexer.h \
 ..\src\ContractionState.h ..\src\CellBuffer.h ..\src\CallTip.h ..\src\KeyMap.h ..\src\Indicator.h \
 ..\src\LineMarker.h ..\src\Style.h ..\src\AutoComplete.h ..\src\ViewStyle.h ..\src\Document.h ..\src\Editor.h \
 ..\src\ScintillaBase.h ..\include\PropSet.h \
 ..\include\SString.h ..\include\Accessor.h ..\src\DocumentAccessor.h ..\include\KeyWords.h

$(DIR_O)\ScintillaWin.obj: ScintillaWin.cxx ..\include\Platform.h ..\include\Scintilla.h \
 ..\src\ContractionState.h ..\src\CellBuffer.h ..\src\CallTip.h ..\src\KeyMap.h ..\src\Indicator.h \
 ..\src\LineMarker.h ..\src\Style.h ..\src\AutoComplete.h ..\src\ViewStyle.h ..\src\Document.h ..\src\Editor.h \
 ..\src\ScintillaBase.h ..\src\UniConversion.h

$(DIR_O)\ScintillaWinL.obj: ScintillaWin.cxx ..\include\Platform.h ..\include\Scintilla.h ..\include\SciLexer.h \
 ..\src\ContractionState.h ..\src\CellBuffer.h ..\src\CallTip.h ..\src\KeyMap.h ..\src\Indicator.h \
 ..\src\LineMarker.h ..\src\Style.h ..\src\AutoComplete.h ..\src\ViewStyle.h ..\src\Document.h ..\src\Editor.h \
 ..\src\ScintillaBase.h ..\include\PropSet.h \
 ..\include\SString.h ..\include\Accessor.h ..\include\KeyWords.h ..\src\UniConversion.h

$(DIR_O)\ScintillaWinS.obj: ScintillaWin.cxx ..\include\Platform.h ..\include\Scintilla.h \
 ..\src\ContractionState.h ..\src\CellBuffer.h ..\src\CallTip.h ..\src\KeyMap.h ..\src\Indicator.h \
 ..\src\LineMarker.h ..\src\Style.h ..\src\AutoComplete.h ..\src\ViewStyle.h ..\src\Document.h ..\src\Editor.h \
 ..\src\ScintillaBase.h ..\src\UniConversion.h

$(DIR_O)\Style.obj: ..\src\Style.cxx ..\include\Platform.h ..\src\Style.h

$(DIR_O)\ViewStyle.obj: ..\src\ViewStyle.cxx ..\include\Platform.h ..\include\Scintilla.h ..\src\Indicator.h \
 ..\src\LineMarker.h ..\src\Style.h ..\src\ViewStyle.h

$(DIR_O)\UniConversion.obj: ..\src\UniConversion.cxx ..\src\UniConversion.h

$(DIR_O)\WindowAccessor.obj: ..\src\WindowAccessor.cxx ..\include\Platform.h ..\include\PropSet.h \
 ..\include\SString.h ..\include\Accessor.h ..\include\WindowAccessor.h ..\include\Scintilla.h
