import win32print, pywintypes, win32con, win32ui, win32gui

pname=win32print.GetDefaultPrinter()
print pname
p=win32print.OpenPrinter(pname)
print 'Printer handle: ',p

## call with last parm set to 0 to get total size needed for printer's DEVMODE
dmsize=win32print.DocumentProperties(0, p, pname, None, None, 0)
## dmDriverExtra should be total size - fixed size
driverextra=dmsize - pywintypes.DEVMODEType().Size  ## need a better way to get DEVMODE.dmSize
dm=pywintypes.DEVMODEType(driverextra)
dm.Fields=dm.Fields|win32con.DM_ORIENTATION|win32con.DM_COPIES
dm.Orientation=win32con.DMORIENT_LANDSCAPE
dm.Copies=2
win32print.DocumentProperties(0, p, pname, dm, dm, win32con.DM_IN_BUFFER|win32con.DM_OUT_BUFFER)

pDC=win32gui.CreateDC('WINSPOOL',pname,dm)
printerDC=win32ui.CreateDCFromHandle(pDC)

printerwidth=printerDC.GetDeviceCaps(110)  ##PHYSICALWIDTH
printerheight=printerDC.GetDeviceCaps(111) ##PHYSICALHEIGHT

hwnd=win32gui.GetDesktopWindow()
## hwnd=win32gui.GetForegroundWindow()
l,t,r,b=win32gui.GetWindowRect(hwnd)
desktopheight=b-t
desktopwidth=r-l
dDC = win32gui.GetWindowDC(hwnd)
desktopDC=win32ui.CreateDCFromHandle(dDC)

printerDC.StartDoc('desktop.bmp')
printerDC.StartPage()
printerDC.StretchBlt((0,0),(int(printerwidth*.9),int(printerheight*.9)),  ## allow for paper margin
           desktopDC,(0,0),(desktopwidth,desktopheight),win32con.SRCCOPY)
printerDC.EndPage()
printerDC.EndDoc()


