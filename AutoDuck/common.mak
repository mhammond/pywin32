# Common AutoDuck make file

$(GENDIR):
	@if not exist $(GENDIR)\. md $(GENDIR)

cleanad:
    if exist $(GENDIR)\*.rtf del $(GENDIR)\*.rtf
    if exist $(GENDIR)\*.hpj del $(GENDIR)\*.hpj
    if exist $(GENDIR)\"$(TARGET)".log del $(GENDIR)\"$(TARGET)".log
    if exist $(GENDIR)\"$(TARGET)".doc del $(GENDIR)\"$(TARGET)".doc
    if exist "$(TARGET)".hlp del "$(TARGET)".hlp

# Generate a Help file

"$(GENDIR)\$(TARGET).rtf" : "$(GENDIR)\$(TARGET).log" $(SOURCE) PythonWin.fmt
    @$(AD) $(ADHLP) /t$(ADTAB) $(SOURCE)

"..\$(TARGET).hlp" : "$(GENDIR)\$(TARGET).rtf"
    if exist "$(GENDIR)\$(TARGET).ph" del "$(GENDIR)\$(TARGET).ph"
    fixHelpCompression.py "$(GENDIR)\$(TARGET).hpj"
    cd $(GENDIR)
    $(HC) $(TARGET).hpj
    if exist "..\..\..\$(TARGET).hlp" del "..\..\..\$(TARGET).hlp"
    move "$(TARGET).hlp" "..\..\..\$(TARGET).hlp"

# Generate a topic log file

"$(GENDIR)\$(TARGET).log" : $(SOURCE)  PythonWin.fmt
    $(AD) $(ADLOG) $(SOURCE)

# Generate a print documentation file

"$(TARGET).doc" :  $(SOURCE)
    $(AD) $(ADDOC) /t$(ADTAB) $(SOURCE)

