# Demonstrates some advanced menu concepts using win32gui.
# This creates a taskbar icon which has some fancy menus (but note that
# selecting the menu items does nothing useful - see win32gui_taskbar.py
# for examples of this.

# NOTE: This is a work in progress.  Todo:
# * The "Checked" menu items don't work correctly - I'm not sure why.
# * No support for GetMenuItemInfo.

# Based on Andy McKay's demo code.
from win32api import *
from win32gui import *
from win32gui_struct import *
import win32con
import sys, os
import struct
import array

this_dir = os.path.split(sys.argv[0])[0]

class MainWindow:
    def __init__(self):
        message_map = {
                win32con.WM_DESTROY: self.OnDestroy,
                win32con.WM_COMMAND: self.OnCommand,
                win32con.WM_USER+20 : self.OnTaskbarNotify,
        }
        # Register the Window class.
        wc = WNDCLASS()
        hinst = wc.hInstance = GetModuleHandle(None)
        wc.lpszClassName = "PythonTaskbarDemo"
        wc.lpfnWndProc = message_map # could also specify a wndproc.
        classAtom = RegisterClass(wc)
        # Create the Window.
        style = win32con.WS_OVERLAPPED | win32con.WS_SYSMENU
        self.hwnd = CreateWindow( classAtom, "Taskbar Demo", style, \
                0, 0, win32con.CW_USEDEFAULT, win32con.CW_USEDEFAULT, \
                0, 0, hinst, None)
        UpdateWindow(self.hwnd)
        iconPathName = os.path.abspath(os.path.join( sys.prefix, "pyc.ico" ))
        if not os.path.isfile(iconPathName):
            # Look in the source tree.
            iconPathName = os.path.abspath(os.path.join( os.path.split(sys.executable)[0], "..\\PC\\pyc.ico" ))
        if os.path.isfile(iconPathName):
            icon_flags = win32con.LR_LOADFROMFILE | win32con.LR_DEFAULTSIZE
            hicon = LoadImage(hinst, iconPathName, win32con.IMAGE_ICON, 0, 0, icon_flags)
        else:
            print "Can't find a Python icon file - using default"
            hicon = LoadIcon(0, win32con.IDI_APPLICATION)
        
        flags = NIF_ICON | NIF_MESSAGE | NIF_TIP
        nid = (self.hwnd, 0, flags, win32con.WM_USER+20, hicon, "Python Demo")
        Shell_NotifyIcon(NIM_ADD, nid)
        print "Please right-click on the Python icon in the taskbar"

    def OnDestroy(self, hwnd, msg, wparam, lparam):
        nid = (self.hwnd, 0)
        Shell_NotifyIcon(NIM_DELETE, nid)
        PostQuitMessage(0) # Terminate the app.

    def OnTaskbarNotify(self, hwnd, msg, wparam, lparam):
        if lparam==win32con.WM_RBUTTONUP:
            print "You right clicked me."
            menu = CreatePopupMenu()
            # Create our 'Exit' item with the standard, ugly 'close' icon.
            item, extras = PackMENUITEMINFO(text = "Exit",
                                            hbmpItem=win32con.HBMMENU_MBAR_CLOSE,
                                            wID=1000)
            InsertMenuItem(menu, 0, 1, item)
            # Create a 'text only' menu via InsertMenuItem rather then
            # AppendMenu, just to prove we can!
            item, extras = PackMENUITEMINFO(text = "Text only item",
                                            wID=1001)
            InsertMenuItem(menu, 0, 1, item)

            load_bmp_flags=win32con.LR_LOADFROMFILE | \
                           win32con.LR_LOADTRANSPARENT
            # These images are "over sized", so we load them scaled.
            hbmp = LoadImage(0, os.path.join(this_dir, "images/smiley.bmp"),
                             win32con.IMAGE_BITMAP, 20, 20, load_bmp_flags)

            # Create a top-level menu with a bitmap
            item, extras = PackMENUITEMINFO(text="Menu with icon",
                                            hbmpItem=hbmp,
                                            wID=1002)
            InsertMenuItem(menu, 0, 1, item)

            # Create a sub-menu, and put a few funky ones there.
            self.sub_menu = sub_menu = CreatePopupMenu()
            # A 'checkbox' menu.
            item, extras = PackMENUITEMINFO(fState=win32con.MFS_CHECKED,
                                            text="Checkbox menu",
                                            hbmpItem=hbmp,
                                            wID=1003)
            InsertMenuItem(sub_menu, 0, 1, item)
            # A 'radio' menu.
            InsertMenu(sub_menu, 0, win32con.MF_BYPOSITION, win32con.MF_SEPARATOR, None)
            item, extras = PackMENUITEMINFO(fType=win32con.MFT_RADIOCHECK,
                                            fState=win32con.MFS_CHECKED,
                                            text="Checkbox menu - bullet 1",
                                            hbmpItem=hbmp,
                                            wID=1004)
            InsertMenuItem(sub_menu, 0, 1, item)
            item, extras = PackMENUITEMINFO(fType=win32con.MFT_RADIOCHECK,
                                            fState=win32con.MFS_UNCHECKED,
                                            text="Checkbox menu - bullet 2",
                                            hbmpItem=hbmp,
                                            wID=1005)
            InsertMenuItem(sub_menu, 0, 1, item)
            # And add the sub-menu to the top-level menu.
            item, extras = PackMENUITEMINFO(text="Sub-Menu",
                                            hSubMenu=sub_menu)
            InsertMenuItem(menu, 0, 1, item)

            # Set 'Exit' as the default option.
            SetMenuDefaultItem(menu, 1000, 0)

            # And display the menu at the cursor pos.
            pos = GetCursorPos()
            SetForegroundWindow(self.hwnd)
            TrackPopupMenu(menu, win32con.TPM_LEFTALIGN, pos[0], pos[1], 0, self.hwnd, None)
            PostMessage(self.hwnd, win32con.WM_NULL, 0, 0)
            self.hmenu = menu
        return 1

    def OnCommand(self, hwnd, msg, wparam, lparam):
        id = LOWORD(wparam)
        print "OnCommand for control ID", id
    def OnCommand(self, hwnd, msg, wparam, lparam):
        id = LOWORD(wparam)
        if id == 1000:
            print "Goodbye"
            DestroyWindow(self.hwnd)
        elif id==1003:
            # Our 'checkbox' item
            state = GetMenuState(self.sub_menu, id, win32con.MF_BYCOMMAND)
            if state==-1:
                raise RuntimeError, "No item found"
            if state & win32con.MF_CHECKED:
                check_flags = win32con.MF_UNCHECKED
                print "Menu was checked - unchecking"
            elif state & win32con.MF_UNCHECKED:
                check_flags = win32con.MF_CHECKED
                print "Menu was unchecked - checking"
            else:
                raise RuntimeError, "Menu is neither checked nor unchecked!"

            CheckMenuItem(self.sub_menu, id, win32con.MF_BYCOMMAND & check_flags)
            if GetMenuState(self.sub_menu, id, win32con.MF_BYCOMMAND) & win32con.MF_CHECKED != check_flags:
                raise RuntimeError, "The new item didn't get the new checked state!"
        elif id==1004 or id==1005:
            # Our 'checkbox' item
            state = GetMenuState(self.sub_menu, id, win32con.MF_BYCOMMAND)
            if state==-1:
                raise RuntimeError, "No item found"
            CheckMenuItem(self.sub_menu, id, win32con.MF_BYCOMMAND & win32con.MF_CHECKED)
            if GetMenuState(self.sub_menu, id, win32con.MF_BYCOMMAND) & win32con.MF_CHECKED == 0:
                raise RuntimeError, "The new item didn't get the new checked state!"
        else:
            print "OnCommand for ID", id

def main():
    w=MainWindow()
    PumpMessages()

if __name__=='__main__':
    main()
