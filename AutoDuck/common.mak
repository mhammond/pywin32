# Common AutoDuck make file

$(GENDIR):
	@if not exist $(GENDIR)\. md $(GENDIR)

cleanad:
    if exist "$(GENDIR)\*.rtf" del "$(GENDIR)\*.rtf"
    if exist "$(GENDIR)\*.hpj" del "$(GENDIR)\*.hpj"
    if exist "$(GENDIR)\*.log" del "$(GENDIR)\*.log"
	if exist "$(GENDIR)\*.hlog" del "$(GENDIR)\*.hlog"
	if exist "$(GENDIR)\*.hhlog" del "$(GENDIR)\*.hhlog"
    if exist "$(GENDIR)\*.doc" del "$(GENDIR)\*.doc"
    if exist "$(GENDIR)\*.hlp" del "$(GENDIR)\*.hlp"
	if exist "$(GENDIR)\*.html" del "$(GENDIR)\*.html"
	if exist "$(GENDIR)\*.idx"  del "$(GENDIR)\*.idx"
	if exist "$(GENDIR)\*.dump" del "$(GENDIR)\*.dump"
	if exist "$(GENDIR)\*.hhk" del "$(GENDIR)\*.hhk"
	if exist "$(GENDIR)\*.hhc" del "$(GENDIR)\*.hhc"

# Generate a Help file

"$(GENDIR)\$(TARGET).rtf" "$(GENDIR)\$(TARGET).hpj" : "$(GENDIR)\$(TARGET).hlog" $(SOURCE) pythonwin.fmt
    $(AD) $(ADHLP) /t$(ADTAB) $(SOURCE)

"..\$(TARGET).hlp": "$(GENDIR)\$(TARGET).rtf" "$(GENDIR)\$(TARGET).hpj"
    if exist "$(GENDIR)\$(TARGET).ph" del "$(GENDIR)\$(TARGET).ph"
    fixHelpCompression.py "$(GENDIR)\$(TARGET).hpj"
	cd "$(GENDIR)"
    $(HC) $(TARGET).hpj
    if exist "..\..\..\$(TARGET).hlp" del "..\..\..\$(TARGET).hlp" 
    move "$(TARGET).hlp" "..\..\..\$(TARGET).hlp" 

# Generate a topic log file

"$(GENDIR)\$(TARGET).hlog" : $(SOURCE)  pythonwin.fmt
    $(AD) $(ADLOG) $(SOURCE)

"$(GENDIR)\$(TARGET).hhlog" : $(SOURCE)  pythonwin.fmt
    $(ADHTMLFMT) $(ADHTMLLOG) $(SOURCE)

# Generate a print documentation file

"$(TARGET).doc" :  $(SOURCE)
    $(AD) $(ADDOC) /t$(ADTAB) $(SOURCE)

# Generate an HTML Help file.

"$(GENDIR)\$(TARGET).hhp" : BuildHHP.py
	BuildHHP.py "$(GENDIR)\$(TARGET)" "$(TARGET)"

"$(GENDIR)\$(TARGET).html" "$(GENDIR)\$(TARGET).dump" "$(GENDIR)\$(TARGET).idx" : $(SOURCE) pyhtml.fmt "$(GENDIR)\$(TARGET).hhlog"
	$(ADHTMLFMT) $(ADHTML) /t$(ADTAB) $(SOURCE)

"$(GENDIR)\$(TARGET).hhk" : "$(GENDIR)\$(TARGET).idx" "$(GENDIR)\$(TARGET).idx" TOCToHHK.py
	TOCToHHK.py "$(GENDIR)\$(TARGET).idx" "$(GENDIR)\$(TARGET).hhk"

"$(GENDIR)\$(TARGET).hhc" : "$(GENDIR)\$(TARGET).dump" "$(GENDIR)\$(TARGET).dump" Dump2HHC.py
	Dump2HHC.py "$(GENDIR)\$(TARGET).dump" "$(GENDIR)\$(TARGET).hhc" "$(TITLE)" "$(TARGET)" 

"..\$(TARGET).chm" : $(SOURCE) "$(GENDIR)\$(TARGET).html" "$(GENDIR)\$(TARGET).hhc" "$(GENDIR)\$(TARGET).hhk" "$(GENDIR)\$(TARGET).hhp"
	-$(HHC) "$(GENDIR)\$(TARGET).hhp"
	if exist "..\$(TARGET).chm" del "..\$(TARGET).chm"
	move "$(GENDIR)\$(TARGET).chm" "..\$(TARGET).chm" 

