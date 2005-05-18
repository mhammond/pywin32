import win32print, pywintypes, win32con, win32gui, win32ui

pname=win32print.GetDefaultPrinter()
print pname
p=win32print.OpenPrinter(pname)
print 'Printer handle: ',p
print_processor=win32print.GetPrinter(p,2)['pPrintProcessor']
## call with last parm set to 0 to get total size needed for printer's DEVMODE
dmsize=win32print.DocumentProperties(0, p, pname, None, None, 0)
## dmDriverExtra should be total size - fixed size
driverextra=dmsize - pywintypes.DEVMODEType().Size  ## need a better way to get DEVMODE.dmSize
dm=pywintypes.DEVMODEType(driverextra)
dm.Fields=dm.Fields|win32con.DM_ORIENTATION|win32con.DM_COPIES
dm.Orientation=win32con.DMORIENT_LANDSCAPE
dm.Copies=2
win32print.DocumentProperties(0, p, pname, dm, dm, win32con.DM_IN_BUFFER|win32con.DM_OUT_BUFFER)

pDC=win32gui.CreateDC(print_processor,pname,dm)
printerwidth=win32ui.GetDeviceCaps(pDC, 110) ##PHYSICALWIDTH
printerheight=win32ui.GetDeviceCaps(pDC, 111) ##PHYSICALHEIGHT

hwnd=win32gui.GetDesktopWindow()
l,t,r,b=win32gui.GetWindowRect(hwnd)
desktopheight=b-t
desktopwidth=r-l
dDC = win32gui.GetWindowDC(hwnd)

dcDC=win32gui.CreateCompatibleDC(dDC)
dcBM = win32gui.CreateCompatibleBitmap(dDC, desktopwidth, desktopheight);
win32gui.SelectObject(dcDC, dcBM)
win32gui.StretchBlt(dcDC, 0, 0, desktopwidth, desktopheight, dDC, 0, 0, desktopwidth, desktopheight, win32con.SRCCOPY)

pcDC=win32gui.CreateCompatibleDC(pDC)
pcBM=win32gui.CreateCompatibleBitmap(pDC, printerwidth, printerheight)
win32gui.SelectObject(pcDC, pcBM)
win32gui.StretchBlt(pcDC, 0, 0, printerwidth, printerheight, dcDC, 0, 0, desktopwidth, desktopheight, win32con.SRCCOPY)

win32print.StartDoc(pDC,('desktop.bmp',None,None,0))
win32print.StartPage(pDC)
win32gui.StretchBlt(pDC, 0, 0, int(printerwidth*.9), int(printerheight*.9), pcDC, 0, 0, printerwidth, printerheight, win32con.SRCCOPY)

win32print.EndPage(pDC)
win32print.EndDoc(pDC)

win32print.ClosePrinter(p)
win32gui.DeleteDC(dDC)
win32gui.DeleteDC(dcDC)
win32gui.DeleteDC(pDC)
win32gui.DeleteDC(pcDC)


