# Common AutoDuck make file

AD      = ad2\autoduck.exe /SPythonWin.fmt
ADLOG   = "/L$(GENDIR)\$(TARGET).LOG" /N
ADHLP   = /RH "/C$(GENDIR)\$(TARGET).LOG" "/O$(GENDIR)\$(TARGET).RTF" /D "title=$(TITLE)"
ADDOC   = /RD "/O$(GENDIR)\$(TARGET).DOC" /D "doc_header=$(DOCHDR)"
ADTAB   = 8
HC      = hcw /a /c 


