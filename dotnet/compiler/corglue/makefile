REGASM=regasm.exe

CORGLUE_SRC=corglue.cs

..\corglue.dll: $(CORGLUE_SRC)
	csc /nologo /W:4 /target:module /out:..\corglue.mod $(CORGLUE_SRC)
    al /out:..\corglue.dll /keyf:..\..\..\ManagedPython.key ..\corglue.mod
	$(REGASM) ..\corglue.dll /tlb:..\corglue.tlb
    gacutil /i ..\corglue.dll

clean:
	- $(REGASM) /u ..\corglue.dll /tlb:..\corglue.tlb
	- del ..\corglue.tlb
	- del ..\corglue.dll
	- del ..\corglue.pdb
