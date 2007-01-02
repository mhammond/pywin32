# The start of a win32gui generic demo.
# Feel free to contribute more demos back ;-)

import win32gui, win32con, win32api
import time, math

def _MyCallback( hwnd, extra ):
    hwnds, classes = extra
    hwnds.append(hwnd)
    classes[win32gui.GetClassName(hwnd)] = 1

def TestEnumWindows():
    windows = []
    classes = {}
    win32gui.EnumWindows(_MyCallback, (windows, classes))
    print "Enumerated a total of %d windows with %d classes" % (len(windows),len(classes))
    if not classes.has_key("tooltips_class32"):
        print "Hrmmmm - I'm very surprised to not find a 'tooltips_class32' class."


def OnPaint(hwnd, msg, wp, lp):
    dc, ps=win32gui.BeginPaint(hwnd)
    win32gui.SetGraphicsMode(dc, win32con.GM_ADVANCED)
    br=win32gui.CreateSolidBrush(win32api.RGB(255,0,0))
    win32gui.SelectObject(dc, br)
    angle=win32gui.GetWindowLong(hwnd, win32con.GWL_USERDATA)
    win32gui.SetWindowLong(hwnd, win32con.GWL_USERDATA, angle+2)
    r_angle=angle*(math.pi/180)
    win32gui.SetWorldTransform(dc,
        {'M11':math.cos(r_angle), 'M12':math.sin(r_angle), 'M21':math.sin(r_angle)*-1, 'M22':math.cos(r_angle),'Dx':250,'Dy':250})
    win32gui.MoveToEx(dc,250,250)
    win32gui.BeginPath(dc)
    win32gui.Pie(dc, 10, 70, 200, 200, 350, 350, 75, 10)
    win32gui.Chord(dc, 200, 200, 850, 0, 350, 350, 75, 10)
    win32gui.LineTo(dc, 300,300)
    win32gui.LineTo(dc, 100, 20)
    win32gui.LineTo(dc, 20, 100)
    win32gui.LineTo(dc, 400, 0)
    win32gui.LineTo(dc, 0, 400)
    win32gui.EndPath(dc)
    win32gui.StrokeAndFillPath(dc)
    win32gui.EndPaint(hwnd, ps)
    return 0
    
def wndproc(hwnd, msg, wp, lp):
	if msg==win32con.WM_PAINT:
		return OnPaint(hwnd, msg, wp, lp)
	return win32gui.DefWindowProc(hwnd, msg, wp, lp)

def TestSetWorldTransform():
    wc = win32gui.WNDCLASS()
    wc.lpszClassName = 'test_win32gui'
    wc.style =  win32con.CS_GLOBALCLASS|win32con.CS_VREDRAW | win32con.CS_HREDRAW
    wc.hbrBackground = win32con.COLOR_WINDOW+1
    wc.lpfnWndProc=wndproc
    class_atom=win32gui.RegisterClass(wc)       
    hwnd = win32gui.CreateWindow(class_atom,'Spin the Lobster!',
        win32con.WS_CAPTION|win32con.WS_VISIBLE,
        100,100,900,900, 0, 0, 0, None)
    for x in xrange(500):
        win32gui.InvalidateRect(hwnd,None,True)
        win32gui.PumpWaitingMessages()
        time.sleep(0.01)
    win32gui.DestroyWindow(hwnd)
    win32gui.UnregisterClass(class_atom,None)

print "Enumerating all windows..."
TestEnumWindows()
print "Testing drawing functions ..."
TestSetWorldTransform()
print "All tests done!"
