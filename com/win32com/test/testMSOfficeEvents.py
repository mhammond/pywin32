# OfficeEvents - test/demonstrate events with Word and Excel.
from win32com.client import DispatchWithEvents, Dispatch
import msvcrt, pythoncom
import time

def TestExcel():
    class ExcelEvents:
        def OnNewWorkbook(self, wb):
            print "OnNewWorkbook event fired", wb
        def OnWindowActivate(self, wb, wn):
            print "OnWindowActivate", wb, wn
        def OnSheetBeforeDoubleClick(self, Sh, Target, Cancel):
            if Target.Column % 2 == 0:
                print "You can double-click there..."
            else:
                print "You can not double-click there..."
            # This function is a void, so the result ends up in
            # the only ByRef - Cancel.
                return 1

    e = DispatchWithEvents("Excel.Application", ExcelEvents)
    e.Visible=1
    e.Workbooks.Add()
    print "Double-click in a few of the Excel cells..."
    print "Press any key when finished with Excel!"
    while not msvcrt.kbhit():
        pythoncom.PumpWaitingMessages()
        time.sleep(.2)
    msvcrt.getch()

def TestWord():
    class WordEvents:
        def OnDocumentChange(self):
            print "OnDocumentChange"
        def OnQuit(self):
            print "Word is quitting"

    w = DispatchWithEvents("Word.Application", WordEvents)
    w.Visible = 1
    w.Documents.Add()
    print "Press any key when finished with Word!"
    while not msvcrt.kbhit():
        pythoncom.PumpWaitingMessages()
        time.sleep(.2)
    msvcrt.getch()

TestWord()
TestExcel()
