# A sample shell namespace view

# To demostrate:
# * Execute this script to register the namespace.
# * Open Windows Explorer
# * Note the new folder hanging off "My Computer"
# This is still incomplete - but we *do* create a view window - we just do
# nothing with it.
import sys, os
import pythoncom
from win32com.shell import shell, shellcon
import win32gui
import win32con
import winerror
from win32com.server.util import wrap

debug=1
if debug:
    import win32com.server.dispatcher
    defaultDispatcher = win32com.server.dispatcher.DefaultDebugDispatcher

IOleWindow_Methods = "GetWindow ContextSensitiveHelp".split()
IShellView_Methods = IOleWindow_Methods + \
                    """TranslateAccelerator EnableModeless UIActivate
                       Refresh CreateViewWindow DestroyViewWindow
                       GetCurrentInfo AddPropertySheetPages SaveViewState
                       SelectItem GetItemObject""".split()
IShellFolder_Methods = """ParseDisplayName EnumObjects BindToObject
                          BindToStorage CompareIDs CreateViewObject
                          GetAttributesOf GetUIObjectOf GetDisplayNameOf
                          SetNameOf""".split()

IBrowserFrame_Methods = ["GetFrameOptions"]

IPersist_Methods = ["GetClassID"]
IPersistFolder_Methods = IPersist_Methods + ["Initialize"]

# Our shell extension.
class ShellView:
#    _reg_progid_ = "Python.ShellExtension.View"
#    _reg_desc_ = "Python Sample Shell Extension (View)"
    _public_methods_ = IShellView_Methods
    _com_interfaces_ = [pythoncom.IID_IOleWindow,
                        shell.IID_IShellView,
                        ]
    def __init__(self, hwnd):
        self.hwnd_parent = hwnd
   # IShellView
    def CreateViewWindow(self, prev, settings, browser, rect):
        print "CreateViewWindow", prev, settings, browser, rect
        import win32ui
        style = win32con.WS_VISIBLE|win32con.WS_CHILD
        l = win32ui.CreateListCtrl()
        l.CreateWindow(style, rect, self.hwnd_parent, 1)
        l.InsertItem(0, "Hello")
        return l.GetSafeHwnd()
    def TranslateAccelerator(self, msg):
        return winerror.S_FALSE

class ShellFolder:
    _reg_progid_ = "Python.ShellExtension.Folder"
    _reg_desc_ = "Python Sample Shell Extension (Folder)"
    _reg_clsid_ = "{f6287035-3074-4cb5-a8a6-d3c80e206944}"
    _com_interfaces_ = [shell.IID_IBrowserFrameOptions,
                        pythoncom.IID_IPersist,
                        shell.IID_IPersistFolder,
                        shell.IID_IShellFolder,
                        ]

    _public_methods_ = IBrowserFrame_Methods + \
                       IPersistFolder_Methods + \
                       IShellFolder_Methods
    # IPersistFolder    
    def GetFrameOptions(self, mask):
        return 0
    def GetClassID(self):
        return self._reg_clsid_
    # IPersist
    def Initialize(self, pidl):
        print "Got pidl", repr(pidl)
    # IShellFolder
    def CreateViewObject(self, hwnd, iid):
        print "CreateViewObject", hwnd, iid
        return wrap(ShellView(hwnd), useDispatcher=defaultDispatcher)
 
def DllRegisterServer():
    import _winreg
    key = _winreg.CreateKey(_winreg.HKEY_LOCAL_MACHINE,
                            "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\" \
                            "Explorer\\Desktop\\Namespace\\" + \
                            ShellFolder._reg_clsid_)
    _winreg.SetValueEx(key, None, 0, _winreg.REG_SZ, ShellFolder._reg_desc_)
    # And special shell keys under our CLSID
    key = _winreg.CreateKey(_winreg.HKEY_CLASSES_ROOT,
                        "CLSID\\" + ShellFolder._reg_clsid_ + "\\ShellFolder")
    # 'Attributes' is an int stored as a binary! use struct
    attr = shellcon.SFGAO_FOLDER | shellcon.SFGAO_HASSUBFOLDER | \
           shellcon.SFGAO_BROWSABLE
    print attr
    import struct
    s = struct.pack("I", attr)
    _winreg.SetValueEx(key, "Attributes", 0, _winreg.REG_BINARY, s)
    print ShellFolder._reg_desc_, "registration complete."

def DllUnregisterServer():
    import _winreg
    try:
        key = _winreg.DeleteKey(_winreg.HKEY_LOCAL_MACHINE,
                            "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\" \
                            "Explorer\\Desktop\\Namespace\\" + \
                            ShellFolder._reg_clsid_)
    except WindowsError, details:
        import errno
        if details.errno != errno.ENOENT:
            raise
    print ShellFolder._reg_desc_, "unregistration complete."

if __name__=='__main__':
    from win32com.server import register
    register.UseCommandLine(ShellFolder,
                   finalize_register = DllRegisterServer,
                   finalize_unregister = DllUnregisterServer)
