# Common AutoDuck make file

$(GENDIR):
	@if not exist $(GENDIR)\. md $(GENDIR)

cleanad:
    if exist "$(GENDIR)\*" del "$(GENDIR)\*" /q

# Generate a Help file

"$(GENDIR)\$(TARGET).rtf" "$(GENDIR)\$(TARGET).hpj" : "$(GENDIR)\$(TARGET).hlog" $(SOURCE) pythonwin.fmt
    @echo Running autoduck for the .rtf
    @$(AD) $(ADHLP) /t$(ADTAB) $(SOURCE)

"..\$(TARGET).hlp": "$(GENDIR)\$(TARGET).rtf" "$(GENDIR)\$(TARGET).hpj"
    if exist "$(GENDIR)\$(TARGET).ph" del "$(GENDIR)\$(TARGET).ph"
    $(PYTHON) fixHelpCompression.py "$(GENDIR)\$(TARGET).hpj"
    cd "$(GENDIR)"
    $(HC) $(TARGET).hpj
    if exist "..\..\..\$(TARGET).hlp" del "..\..\..\$(TARGET).hlp" 
    move "$(TARGET).hlp" "..\..\..\$(TARGET).hlp"
    cd $(MYDIR_FROM_GENDIR)

# Generate a topic log file

"$(GENDIR)\$(TARGET).hlog" : $(SOURCE)  pythonwin.fmt
    @echo Running autoduck for the .hlog
    @$(AD) $(ADLOG) $(SOURCE)

"$(GENDIR)\$(TARGET).hhlog" : $(SOURCE)  pythonwin.fmt
    @echo Running autoduck for the .hhlog
    @$(ADHTMLFMT) $(ADHTMLLOG) $(SOURCE)

# Generate a print documentation file

"$(TARGET).doc" :  $(SOURCE)
    @echo Running autoduck for the .doc
    @$(AD) $(ADDOC) /t$(ADTAB) $(SOURCE)

# Generate an HTML Help file.

"$(GENDIR)\$(TARGET).hhp" : BuildHHP.py $(DOCUMENT_FILE) $(HTML_FILES)
	$(PYTHON) BuildHHP.py "$(GENDIR)\$(TARGET)" "$(TARGET)" "$(GENDIR)" $(HTML_FILES)

"$(GENDIR)\$(TARGET).html" "$(GENDIR)\$(TARGET).dump" "$(GENDIR)\$(TARGET).idx" : $(SOURCE) pyhtml.fmt "$(GENDIR)\$(TARGET).hhlog" InsertExternalOverviews.py $(DOCUMENT_FILE) 
	@echo Running autoduck for the .html
	@$(ADHTMLFMT) $(ADHTML) /t$(ADTAB) $(SOURCE)
	$(PYTHON) InsertExternalOverviews.py "$(GENDIR)\$(TARGET).html"

"$(GENDIR)\$(TARGET).hhk" : "$(GENDIR)\$(TARGET).idx" "$(GENDIR)\$(TARGET).idx" TOCToHHK.py
	$(PYTHON) TOCToHHK.py "$(GENDIR)\$(TARGET).idx" "$(GENDIR)\$(TARGET).hhk"

#"$(GENDIR)\$(TARGET).hhc" : "$(GENDIR)\$(TARGET).dump" "$(GENDIR)\$(TARGET).dump" Dump2HHC.py $(EXT_TOPICS)
#	Dump2HHC.py "$(GENDIR)\$(TARGET).dump" "$(GENDIR)\$(TARGET).hhc" "$(TITLE)" "$(TARGET)" $(EXT_TOPICS)

"..\$(TARGET).chm" : $(SOURCE) "$(GENDIR)\$(TARGET).html" "$(GENDIR)\$(TARGET).hhc" "$(GENDIR)\$(TARGET).hhk" "$(GENDIR)\$(TARGET).hhp"
	-$(HHC) "$(GENDIR)\$(TARGET).hhp"
	if exist "..\$(TARGET).chm" del "..\$(TARGET).chm"
	move "$(GENDIR)\$(TARGET).chm" "..\$(TARGET).chm"


