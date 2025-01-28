# Common AutoDuck make file

AD        = bin\autoduck.exe /Spythonwin.fmt
ADHTMLFMT = bin\autoduck.exe /Spyhtml.fmt
ADHTMLLOG = /R html "/L$(GENDIR)\$(TARGET).HHLOG" /N
ADLOG     = /R help "/L$(GENDIR)\$(TARGET).HLOG" /N
ADHTML    = /R html "/G$(GENDIR)\$(TARGET).DUMP" "/C$(GENDIR)\$(TARGET).HHLOG" "/I$(GENDIR)\$(TARGET).IDX" "/O$(GENDIR)\$(TARGET).HTML" /D "title=$(TITLE)"
ADHLP     = /R help "/C$(GENDIR)\$(TARGET).HLOG" "/O$(GENDIR)\$(TARGET).RTF" /D "title=$(TITLE)"
ADDOC     = /RD "/O$(GENDIR)\$(TARGET).DOC" /D "doc_header=$(DOCHDR)"
ADTAB     = 8
HC        = hcw /a /c /e
HHC       = hhc
# PYTHON is set in make.py, defaulting to the one found on PATH
PYTHON    = python
