# An example of hosting an IE app (without using Pythonwin/MFC)
# A nod to the Code Project's article "Embed an HTML control in your own
# window using plain C"
import sys

import pythoncom
import win32api
import win32con
import win32gui
import winerror
from win32com.axcontrol import axcontrol
from win32com.client import Dispatch
from win32com.server.exception import COMException
from win32com.server.util import wrap

# Set to True to see debug output in the 'trace collector' window.
debugging = False

# If you wanted events or better type info, you'd probably do:
# gencache.EnsureModule('{EAB22AC0-30C1-11CF-A7EB-0000C05BAE0B}', 0, 1, 1)
# which is the "Microsoft Internet Controls" typelib defining interfaces
# such as IWebBrowser2 and the associated events.

IOleClientSite_methods = """SaveObject GetMoniker GetContainer ShowObject
                            OnShowWindow RequestNewObjectLayout""".split()

IOleInPlaceSite_methods = """GetWindow ContextSensitiveHelp CanInPlaceActivate
                             OnInPlaceActivate OnUIActivate GetWindowContext
                             Scroll OnUIDeactivate OnInPlaceDeactivate
                             DiscardUndoState DeactivateAndUndo
                             OnPosRectChange""".split()

IOleInPlaceFrame_methods = """GetWindow ContextSensitiveHelp GetBorder
                              RequestBorderSpace SetBorderSpace
                              SetActiveObject InsertMenus SetMenu
                              RemoveMenus SetStatusText EnableModeless
                              TranslateAccelerator""".split()


class SimpleSite:
    _com_interfaces_ = [axcontrol.IID_IOleClientSite, axcontrol.IID_IOleInPlaceSite]
    _public_methods_ = IOleClientSite_methods + IOleInPlaceSite_methods

    def __init__(self, host_window):
        self.hw = host_window

    # IID_IOleClientSite methods
    def SaveObject(self):
        raise COMException(hresult=winerror.E_NOTIMPL)

    def GetMoniker(self, dwAssign, which):
        raise COMException(hresult=winerror.E_NOTIMPL)

    def GetContainer(self):
        raise COMException(hresult=winerror.E_NOINTERFACE)

    def ShowObject(self):
        pass

    def OnShowWindow(self, fShow):
        pass

    def RequestNewObjectLayout(self):
        raise COMException(hresult=winerror.E_NOTIMPL)

    # IID_IOleInPlaceSite methods
    def GetWindow(self):
        return self.hw.hwnd

    def ContextSensitiveHelp(self, fEnter):
        raise COMException(hresult=winerror.E_NOTIMPL)

    def CanInPlaceActivate(self):
        pass  # we can

    def OnInPlaceActivate(self):
        pass

    def OnUIActivate(self):
        pass

    def GetWindowContext(self):
        # return IOleInPlaceFrame, IOleInPlaceUIWindow, rect, clip_rect, frame_info
        # where frame_info is (fMDIApp, hwndFrame, hAccel, nAccel)
        return (
            self.hw.ole_frame,
            None,
            (0, 0, 0, 0),
            (0, 0, 0, 0),
            (True, self.hw.hwnd, None, 0),
        )

    def Scroll(self, size):
        raise COMException(hresult=winerror.E_NOTIMPL)

    def OnUIDeactivate(self, fUndoable):
        pass

    def OnInPlaceDeactivate(self):
        pass

    def DiscardUndoState(self):
        raise COMException(hresult=winerror.E_NOTIMPL)

    def DeactivateAndUndo(self):
        raise COMException(hresult=winerror.E_NOTIMPL)

    def OnPosRectChange(self, rect):
        browser_ob = self.hw.browser.QueryInterface(axcontrol.IID_IOleInPlaceObject)
        browser_ob.SetObjectRects(rect, rect)


class SimpleFrame:
    # _com_interfaces_ = [axcontrol.IID_IOleInPlaceFrame]
    _public_methods_ = IOleInPlaceFrame_methods

    def __init__(self, host_window):
        self.hw = host_window

    def GetWindow(self):
        return self.hw.hwnd

    def ContextSensitiveHelp(self, fEnterMode):
        raise COMException(hresult=winerror.E_NOTIMPL)

    def GetBorder(self):
        raise COMException(hresult=winerror.E_NOTIMPL)

    def RequestBorderSpace(self, widths):
        raise COMException(hresult=winerror.E_NOTIMPL)

    def SetBorderSpace(self, widths):
        raise COMException(hresult=winerror.E_NOTIMPL)

    def SetActiveObject(self, ob, name):
        pass

    def InsertMenus(self, hmenuShared, menuWidths):
        raise COMException(hresult=winerror.E_NOTIMPL)

    def SetMenu(self, hmenuShared, holemenu, hwndActiveObject):
        pass

    def RemoveMenus(self, hmenuShared):
        pass

    def SetStatusText(self, statusText):
        pass

    def EnableModeless(self, fEnable):
        pass

    def TranslateAccelerator(self, msg, wID):
        raise COMException(hresult=winerror.E_NOTIMPL)


# A class that manages the top-level window.
class IEHost:
    wnd_class_name = "EmbeddedBrowser"

    def __init__(self):
        self.hwnd = None
        self.ole_frame = None

    def __del__(self):
        try:
            win32gui.UnregisterClass(self.wnd_class_name, None)
        except win32gui.error:
            pass

    def create_window(self):
        message_map = {
            win32con.WM_SIZE: self.OnSize,
            win32con.WM_DESTROY: self.OnDestroy,
        }

        wc = win32gui.WNDCLASS()
        wc.lpszClassName = self.wnd_class_name
        # wc.style =  win32con.CS_GLOBALCLASS|win32con.CS_VREDRAW | win32con.CS_HREDRAW
        # wc.hbrBackground = win32con.COLOR_WINDOW+1
        wc.lpfnWndProc = message_map
        class_atom = win32gui.RegisterClass(wc)
        self.hwnd = win32gui.CreateWindow(
            wc.lpszClassName,
            "Embedded browser",
            win32con.WS_OVERLAPPEDWINDOW | win32con.WS_VISIBLE,
            win32con.CW_USEDEFAULT,
            win32con.CW_USEDEFAULT,
            win32con.CW_USEDEFAULT,
            win32con.CW_USEDEFAULT,
            0,
            0,
            0,
            None,
        )
        browser = pythoncom.CoCreateInstance(
            "{8856F961-340A-11D0-A96B-00C04FD705A2}",
            None,
            pythoncom.CLSCTX_INPROC_SERVER | pythoncom.CLSCTX_INPROC_HANDLER,
            axcontrol.IID_IOleObject,
        )
        self.browser = browser
        site = wrap(
            SimpleSite(self), axcontrol.IID_IOleClientSite, useDispatcher=debugging
        )

        browser.SetClientSite(site)
        browser.SetHostNames("IE demo", "Hi there")
        axcontrol.OleSetContainedObject(self.browser, True)
        rect = win32gui.GetWindowRect(self.hwnd)
        browser.DoVerb(axcontrol.OLEIVERB_SHOW, None, site, -1, self.hwnd, rect)
        b2 = Dispatch(browser.QueryInterface(pythoncom.IID_IDispatch))
        self.browser2 = b2
        b2.Left = 0
        b2.Top = 0
        b2.Width = rect[2]
        b2.Height = rect[3]

    def OnSize(self, hwnd, msg, wparam, lparam):
        self.browser2.Width = win32api.LOWORD(lparam)
        self.browser2.Height = win32api.HIWORD(lparam)

    def OnDestroy(self, hwnd, msg, wparam, lparam):
        self.browser.Close(axcontrol.OLECLOSE_NOSAVE)
        self.browser = None
        self.browser2 = None
        win32gui.PostQuitMessage(0)


if __name__ == "__main__":
    h = IEHost()
    h.create_window()
    if len(sys.argv) < 2:
        h.browser2.Navigate2("about:blank")
        doc = h.browser2.Document
        doc.write(
            'This is an IE page hosted by <a href="https://www.python.org">python</a>'
        )
        doc.write("<br>(you can also specify a URL on the command-line...)")
    else:
        h.browser2.Navigate2(sys.argv[1])

    win32gui.PumpMessages()
