# debugger.py

# A debugger for Pythonwin.  Built from pdb.

# Mark Hammond (MHammond@skippinet.com.au) - Dec 94.

# usage:
# >>> import pywin.debugger
# >>> pywin.debugger.GetDebugger().run("command")

import pdb
import bdb
import sys
import string
import os
import types

import win32ui
import win32api
import win32con
import pywin.docking.DockingBar
from pywin.mfc import dialog, object, afxres, window
from pywin.framework import app, interact, editor, scriptutils
from pywin.framework.editor.color.coloreditor import MARKER_CURRENT, MARKER_BREAKPOINT
from pywin.tools import browser, hierlist
import commctrl
import traceback

# import win32traceutil
if win32ui.UNICODE:
    LVN_ENDLABELEDIT = commctrl.LVN_ENDLABELEDITW
else:
    LVN_ENDLABELEDIT = commctrl.LVN_ENDLABELEDITA

##from dbgcon import *  # flake8 doesn't see
from .dbgcon import (
    DBGSTATE_NOT_DEBUGGING,
    DBGSTATE_BREAK,
    DBGSTATE_RUNNING,
    DBGSTATE_QUITTING,
    DBGSTATE_FREETRACING,
    LINESTATE_BREAKPOINT,
    LINESTATE_CURRENT,
    OPT_STOP_EXCEPTIONS,
    OPT_HIDE,
    LoadDebuggerOptions,
)

error = "pywin.debugger.error"


def _LineStateToMarker(ls):
    if ls == LINESTATE_CURRENT:
        return MARKER_CURRENT
    # 	elif ls == LINESTATE_CALLSTACK:
    # 		return MARKER_CALLSTACK
    return MARKER_BREAKPOINT


class HierListItem(browser.HLIPythonObject):
    pass


class HierFrameItem(HierListItem):
    def __init__(self, frame, debugger):
        HierListItem.__init__(self, frame, repr(frame))
        self.debugger = debugger

    def GetText(self):
        name = self.myobject.f_code.co_name
        if not name or name == "?":
            # See if locals has a '__name__' (ie, a module)
            if "__name__" in self.myobject.f_locals:
                name = str(self.myobject.f_locals["__name__"]) + " module"
            else:
                name = "<Debugger Context>"

        return "%s   (%s:%d)" % (
            name,
            os.path.split(self.myobject.f_code.co_filename)[1],
            self.myobject.f_lineno,
        )

    def GetBitmapColumn(self):
        if self.debugger.curframe is self.myobject:
            return 7
        else:
            return 8

    def GetSubList(self):
        ret = []
        ret.append(HierFrameDict(self.myobject.f_locals, "Locals", 2))
        ret.append(HierFrameDict(self.myobject.f_globals, "Globals", 1))
        return ret

    def IsExpandable(self):
        return 1

    def TakeDefaultAction(self):
        # Set the default frame to be this frame.
        win32ui.GetApp().CallAfter(lambda: self.debugger.set_cur_frame(self.myobject))
        return 1


class HierFrameDict(browser.HLIDict):
    def __init__(self, dict, name, bitmapColumn):
        self.bitmapColumn = bitmapColumn
        browser.HLIDict.__init__(self, dict, name)

    def GetBitmapColumn(self):
        return self.bitmapColumn


class NoStackAvailableItem(HierListItem):
    def __init__(self, why):
        HierListItem.__init__(self, None, why)

    def IsExpandable(self):
        return 0

    def GetText(self):
        return self.name

    def GetBitmapColumn(self):
        return 8


class HierStackRoot(HierListItem):
    def __init__(self, debugger):
        HierListItem.__init__(self, debugger, None)
        self.last_stack = []

    def GetSubList(self):
        debugger = self.myobject
        # 		print self.debugger.stack, self.debugger.curframe
        ret = []
        if debugger.debuggerState == DBGSTATE_BREAK:
            stackUse = debugger.stack[:]
            stackUse.reverse()
            self.last_stack = []
            for frame, lineno in stackUse:
                if frame.f_locals.get("__debuggerframe__"):
                    continue
                self.last_stack.append((frame, lineno))
                if (
                    frame is debugger.userbotframe
                ):  # Dont bother showing frames below our bottom frame.
                    break
        for frame, lineno in self.last_stack:
            ret.append(HierFrameItem(frame, debugger))
        return ret

    def GetText(self):
        return "root item"

    def IsExpandable(self):
        return 1


class HierListDebugger(hierlist.HierListWithItems):
    """Hier List of stack frames, breakpoints, whatever"""

    def __init__(self):
        hierlist.HierListWithItems.__init__(
            self, None, win32ui.IDB_DEBUGGER_HIER, None, win32api.RGB(255, 0, 0)
        )

    def Setup(self, debugger):
        root = HierStackRoot(debugger)
        self.AcceptRoot(root)

    # 	def Refresh(self):
    # 		self.Setup()
    def OnTreeItemExpanding(self, info, extra):
        (hwndFrom, idFrom, code) = info
        if idFrom != self.listBoxId:
            return None
        action, itemOld, itemNew, pt = extra
        itemHandle = itemNew[0]
        if itemHandle not in self.filledItemHandlesMap:
            item = self.itemHandleMap[itemHandle]
            subItems = self.GetSubList(item)
            lh = []
            for item in subItems:
                lh.append(self.AddItem(itemHandle, item))
            self.filledItemHandlesMap[itemHandle] = None
            if subItems and isinstance(subItems[0], HierFrameDict):
                self.Expand(lh[0], commctrl.TVE_EXPAND)
        return 0


class DebuggerWindow(window.Wnd):
    def __init__(self, ob):
        window.Wnd.__init__(self, ob)
        self.debugger = None

    def Init(self, debugger):
        self.debugger = debugger

    def GetDefRect(self):
        defRect = app.LoadWindowSize("Debugger Windows\\" + self.title)
        if defRect[2] - defRect[0] == 0:
            defRect = 0, 0, 150, 150
        return defRect

    def OnDestroy(self, msg):
        newSize = self.GetWindowPlacement()[4]
        pywin.framework.app.SaveWindowSize("Debugger Windows\\" + self.title, newSize)
        return window.Wnd.OnDestroy(self, msg)

    def OnKeyDown(self, msg):
        key = msg[2]
        if key in [13, 27, 32]:
            return 1
        if key in [46, 8]:  # delete/BS key
            self.DeleteSelected()
            return 0
        view = scriptutils.GetActiveView()
        try:
            firer = view.bindings.fire_key_event
        except AttributeError:
            firer = None
        if firer is not None:
            return firer(msg)
        else:
            return 1

    def DeleteSelected(self):
        win32api.MessageBeep()

    def EditSelected(self):
        win32api.MessageBeep()


class DebuggerStackWindow(DebuggerWindow):
    title = "Stack"

    def __init__(self):
        DebuggerWindow.__init__(self, win32ui.CreateTreeCtrl())
        self.list = HierListDebugger()
        self.listOK = 0

    def SaveState(self):
        self.list.DeleteAllItems()
        self.listOK = 0
        win32ui.WriteProfileVal(
            "Debugger Windows\\" + self.title, "Visible", self.IsWindowVisible()
        )

    def CreateWindow(self, parent):
        style = (
            win32con.WS_CHILD
            | win32con.WS_VISIBLE
            | win32con.WS_BORDER
            | commctrl.TVS_HASLINES
            | commctrl.TVS_LINESATROOT
            | commctrl.TVS_HASBUTTONS
        )
        self._obj_.CreateWindow(style, self.GetDefRect(), parent, win32ui.IDC_LIST1)
        self.HookMessage(self.OnKeyDown, win32con.WM_KEYDOWN)
        self.HookMessage(self.OnKeyDown, win32con.WM_SYSKEYDOWN)
        self.list.HierInit(parent, self)
        self.listOK = 0  # delayed setup
        # self.list.Setup()

    def RespondDebuggerState(self, state):
        assert self.debugger is not None, "Init not called"
        if not self.listOK:
            self.listOK = 1
            self.list.Setup(self.debugger)
        else:
            self.list.Refresh()

    def RespondDebuggerData(self):
        try:
            handle = self.GetChildItem(0)
        except win32ui.error:
            return  # No items
        while 1:
            item = self.list.ItemFromHandle(handle)
            col = self.list.GetBitmapColumn(item)
            selCol = self.list.GetSelectedBitmapColumn(item)
            if selCol is None:
                selCol = col
            if self.list.GetItemImage(handle) != (col, selCol):
                self.list.SetItemImage(handle, col, selCol)
            try:
                handle = self.GetNextSiblingItem(handle)
            except win32ui.error:
                break


class DebuggerListViewWindow(DebuggerWindow):
    def __init__(self):
        DebuggerWindow.__init__(self, win32ui.CreateListCtrl())

    def CreateWindow(self, parent):
        list = self
        style = (
            win32con.WS_CHILD
            | win32con.WS_VISIBLE
            | win32con.WS_BORDER
            | commctrl.LVS_EDITLABELS
            | commctrl.LVS_REPORT
        )
        self._obj_.CreateWindow(style, self.GetDefRect(), parent, win32ui.IDC_LIST1)
        self.HookMessage(self.OnKeyDown, win32con.WM_KEYDOWN)
        self.HookMessage(self.OnKeyDown, win32con.WM_SYSKEYDOWN)
        list = self
        title, width = self.columns[0]
        itemDetails = (commctrl.LVCFMT_LEFT, width, title, 0)
        list.InsertColumn(0, itemDetails)
        col = 1
        for title, width in self.columns[1:]:
            col = col + 1
            itemDetails = (commctrl.LVCFMT_LEFT, width, title, 0)
            list.InsertColumn(col, itemDetails)
        parent.HookNotify(self.OnListEndLabelEdit, LVN_ENDLABELEDIT)
        parent.HookNotify(self.OnItemRightClick, commctrl.NM_RCLICK)
        parent.HookNotify(self.OnItemDoubleClick, commctrl.NM_DBLCLK)

    def RespondDebuggerData(self):
        pass

    def RespondDebuggerState(self, state):
        pass

    def EditSelected(self):
        try:
            sel = self.GetNextItem(-1, commctrl.LVNI_SELECTED)
        except win32ui.error:
            return
        self.EditLabel(sel)

    def OnKeyDown(self, msg):
        key = msg[2]
        # If someone starts typing, they probably are trying to edit the text!
        if chr(key) in string.ascii_uppercase:
            self.EditSelected()
            return 0
        return DebuggerWindow.OnKeyDown(self, msg)

    def OnItemDoubleClick(self, notify_data, extra):
        self.EditSelected()

    def OnItemRightClick(self, notify_data, extra):
        # First select the item we right-clicked on.
        pt = self.ScreenToClient(win32api.GetCursorPos())
        flags, hItem, subitem = self.HitTest(pt)
        if hItem == -1 or commctrl.TVHT_ONITEM & flags == 0:
            return None
        self.SetItemState(hItem, commctrl.LVIS_SELECTED, commctrl.LVIS_SELECTED)

        dockbar = self.GetParent()
        if dockbar.IsFloating():
            hook_parent = win32ui.GetMainFrame()
        else:
            hook_parent = self.GetParentFrame()
        menu = win32ui.CreatePopupMenu()
        if hasattr(self, "OnJumpToSource"):
            menu.AppendMenu(
                win32con.MF_STRING | win32con.MF_ENABLED, 1002, "Jump to Source"
            )
            hook_parent.HookCommand(self.OnJumpToSource, 1002)
        menu.AppendMenu(win32con.MF_STRING | win32con.MF_ENABLED, 1000, "Edit item")
        menu.AppendMenu(win32con.MF_STRING | win32con.MF_ENABLED, 1001, "Delete item")
        hook_parent.HookCommand(self.OnEditItem, 1000)
        hook_parent.HookCommand(self.OnDeleteItem, 1001)
        menu.TrackPopupMenu(win32api.GetCursorPos())  # track at mouse position.
        return None

    def OnDeleteItem(self, command, code):
        self.DeleteSelected()

    def OnEditItem(self, command, code):
        self.EditSelected()


class DebuggerBreakpointsWindow(DebuggerListViewWindow):
    title = "Breakpoints"
    columns = [("Condition", 70), ("Location", 1024)]

    def OnJumpToSource(self, command, code):
        try:
            num = self.GetNextItem(-1, commctrl.LVNI_SELECTED)
            item_id = self.GetItem(num)[6]
            from bdb import Breakpoint

            for bplist in list(Breakpoint.bplist.values()):
                for bp in bplist:
                    if id(bp) == item_id:
                        scriptutils.JumpToDocument(bp.file, bp.line)
                        break
        except win32ui.error:
            win32api.MessageBeep()

    def SaveState(self):
        items = []
        for i in range(self.GetItemCount()):
            items.append(self.GetItemText(i, 0))
            items.append(self.GetItemText(i, 1))
        win32ui.WriteProfileVal(
            "Debugger Windows\\" + self.title, "BreakpointList", "\t".join(items)
        )
        win32ui.WriteProfileVal(
            "Debugger Windows\\" + self.title, "Visible", self.IsWindowVisible()
        )
        return 1

    def OnListEndLabelEdit(self, std, extra):
        item = extra[0]
        text = item[4]
        if text is None:
            return

        item_id = self.GetItem(item[0])[6]

        from bdb import Breakpoint

        for bplist in Breakpoint.bplist.values():
            for bp in bplist:
                if id(bp) == item_id:
                    if text.strip().lower() == "none":
                        text = None
                    bp.cond = text
                    break
        self.RespondDebuggerData()

    def DeleteSelected(self):
        try:
            num = self.GetNextItem(-1, commctrl.LVNI_SELECTED)
            item_id = self.GetItem(num)[6]
            from bdb import Breakpoint

            for bplist in list(Breakpoint.bplist.values()):
                for bp in bplist:
                    if id(bp) == item_id:
                        self.debugger.clear_break(bp.file, bp.line)
                        break
        except win32ui.error:
            win32api.MessageBeep()
        self.RespondDebuggerData()

    def RespondDebuggerData(self):
        l = self
        l.DeleteAllItems()
        index = -1
        from bdb import Breakpoint

        for bplist in Breakpoint.bplist.values():
            for bp in bplist:
                baseName = os.path.split(bp.file)[1]
                cond = bp.cond
                item = index + 1, 0, 0, 0, str(cond), 0, id(bp)
                index = l.InsertItem(item)
                l.SetItemText(index, 1, "%s: %s" % (baseName, bp.line))


class DebuggerWatchWindow(DebuggerListViewWindow):
    title = "Watch"
    columns = [("Expression", 70), ("Value", 1024)]

    def CreateWindow(self, parent):
        DebuggerListViewWindow.CreateWindow(self, parent)
        items = win32ui.GetProfileVal(
            "Debugger Windows\\" + self.title, "Items", ""
        ).split("\t")
        index = -1
        for item in items:
            if item:
                index = self.InsertItem(index + 1, item)
        self.InsertItem(index + 1, "<New Item>")

    def SaveState(self):
        items = []
        for i in range(self.GetItemCount() - 1):
            items.append(self.GetItemText(i, 0))
        win32ui.WriteProfileVal(
            "Debugger Windows\\" + self.title, "Items", "\t".join(items)
        )
        win32ui.WriteProfileVal(
            "Debugger Windows\\" + self.title, "Visible", self.IsWindowVisible()
        )
        return 1

    def OnListEndLabelEdit(self, std, extra):
        item = extra[0]
        itemno = item[0]
        text = item[4]
        if text is None:
            return
        self.SetItemText(itemno, 0, text)
        if itemno == self.GetItemCount() - 1:
            self.InsertItem(itemno + 1, "<New Item>")
        self.RespondDebuggerData()

    def DeleteSelected(self):
        try:
            num = self.GetNextItem(-1, commctrl.LVNI_SELECTED)
            if num < self.GetItemCount() - 1:  # We cant delete the last
                self.DeleteItem(num)
        except win32ui.error:
            win32api.MessageBeep()

    def RespondDebuggerData(self):
        state = self.debugger.debuggerState
        globs = locs = None
        if state == DBGSTATE_BREAK:
            if self.debugger.curframe:
                globs = self.debugger.curframe.f_globals
                locs = self.debugger.curframe.f_locals
        else:
            globs, locs = interact.edit.currentView.GetContext()
        for i in range(self.GetItemCount() - 1):
            text = self.GetItemText(i, 0)
            if globs is None:
                val = ""
            else:
                try:
                    val = repr(eval(text, globs, locs))
                except SyntaxError:
                    val = "Syntax Error"
                except:
                    t, v, tb = sys.exc_info()
                    val = traceback.format_exception_only(t, v)[0].strip()
                    tb = None  # prevent a cycle.
            self.SetItemText(i, 1, val)


def CreateDebuggerDialog(parent, klass):
    control = klass()
    control.CreateWindow(parent)
    return control


DebuggerDialogInfos = (
    (0xE810, DebuggerStackWindow, None),
    (0xE811, DebuggerBreakpointsWindow, (10, 10)),
    (0xE812, DebuggerWatchWindow, None),
)

# Prepare all the "control bars" for this package.
# If control bars are not all loaded when the toolbar-state functions are
# called, things go horribly wrong.
def PrepareControlBars(frame):
    style = (
        win32con.WS_CHILD
        | afxres.CBRS_SIZE_DYNAMIC
        | afxres.CBRS_TOP
        | afxres.CBRS_TOOLTIPS
        | afxres.CBRS_FLYBY
    )
    tbd = win32ui.CreateToolBar(frame, style, win32ui.ID_VIEW_TOOLBAR_DBG)
    tbd.ModifyStyle(0, commctrl.TBSTYLE_FLAT)
    tbd.LoadToolBar(win32ui.IDR_DEBUGGER)
    tbd.EnableDocking(afxres.CBRS_ALIGN_ANY)
    tbd.SetWindowText("Debugger")
    frame.DockControlBar(tbd)

    # and the other windows.
    for id, klass, float in DebuggerDialogInfos:
        try:
            frame.GetControlBar(id)
            exists = 1
        except win32ui.error:
            exists = 0
        if exists:
            continue
        bar = pywin.docking.DockingBar.DockingBar()
        style = win32con.WS_CHILD | afxres.CBRS_LEFT  # don't create visible.
        bar.CreateWindow(
            frame,
            CreateDebuggerDialog,
            klass.title,
            id,
            style,
            childCreatorArgs=(klass,),
        )
        bar.SetBarStyle(
            bar.GetBarStyle()
            | afxres.CBRS_TOOLTIPS
            | afxres.CBRS_FLYBY
            | afxres.CBRS_SIZE_DYNAMIC
        )
        bar.EnableDocking(afxres.CBRS_ALIGN_ANY)
        if float is None:
            frame.DockControlBar(bar)
        else:
            frame.FloatControlBar(bar, float, afxres.CBRS_ALIGN_ANY)


SKIP_NONE = 0
SKIP_STEP = 1
SKIP_RUN = 2

debugger_parent = pdb.Pdb


class Debugger(pdb.Pdb):
    def __init__(self):
        self.inited = 0
        self.skipBotFrame = SKIP_NONE
        self.userbotframe = None
        self.frameShutdown = 0
        self.pumping = 0
        self.debuggerState = DBGSTATE_NOT_DEBUGGING  # Assume so, anyway.
        self.shownLineCurrent = None  # The last filename I highlighted.
        self.shownLineCallstack = None  # The last filename I highlighted.
        self.last_cmd_debugged = ""
        self.isInitialBreakpoint = 0
        self.bAtException = self.bAtPostMortem = 0
        debugger_parent.__init__(self)

        # See if any break-points have been set in the editor
        for doc in editor.editorTemplate.GetDocumentList():
            lineNo = -1
            while 1:
                lineNo = doc.MarkerGetNext(lineNo + 1, MARKER_BREAKPOINT)
                if lineNo <= 0:
                    break
                self.set_break(doc.GetPathName(), lineNo)

        self.reset()
        self.inForcedGUI = win32ui.GetApp().IsInproc()
        self.options = LoadDebuggerOptions()
        self.dummyframe = sys._getframe()  # will never match in the future

    def __del__(self):
        self.close()

    curframe = None

    def SetInteractiveContext(self, globs=None, locs=None, curframe=None):
        if interact.edit is not None and interact.edit.currentView is not None:
            ia = interact.edit.currentView
            if globs == "current":
                globs, locs = ia.GetContext()
            ia.SetContext(
                globs,
                locs,
                name=("D" * (self.pumping))
                + (self.debuggerState == DBGSTATE_BREAK)  # depth of recursive debugging
                * "B"
                + (self.debuggerState == DBGSTATE_FREETRACING) * "F"
                + (sys.version_info >= (2, 6) and bool(sys.gettrace()) * "T" or "")
                + self.bAtPostMortem * "P"
                + self.bAtException * "E",
                curframe=self.curframe,
            )

    def close(self, frameShutdown=0):
        # frameShutdown: if we have total shutdown (ie, main window is dieing)
        if self.pumping:
            if not self.GUIAboutToFinishInteract():
                # User cancelled the close.
                return 0
            # We can stop the message pump, as only WM_QUIT is posted, which returns
            # immediately. However StopDebuggerPump() / posting WM_QUIT is done at
            # the end of this method now to not be disturbed by messages (bug fix)
        self.frameShutdown = frameShutdown
        if not self.inited:
            return 1
        self.inited = 0

        frame = win32ui.GetMainFrame()
        # Hide the debuger toolbars (as they wont normally form part of the main toolbar state.
        for id, klass, float in DebuggerDialogInfos:
            try:
                tb = frame.GetControlBar(id)
                if tb.dialog is not None:  # We may never have actually been shown.
                    tb.dialog.SaveState()
                    frame.ShowControlBar(tb, 0, 1)
            except win32ui.error:
                pass

        self._UnshowCurrentLine()
        self.set_quit()
        self.SetInteractiveContext(None, None)
        if self.pumping:
            # send WM_QUIT - must be last to not be disturbed by messages (bug fix)
            self.StopDebuggerPump()
        return 1

    def StopDebuggerPump(self):
        assert self.pumping, "Can't stop the debugger pump if I'm not pumping!"
        # After stopping a pump, I may never return.
        ##if self.GUIAboutToFinishInteract():
        win32api.PostQuitMessage()
        return 1

    def get_option(self, option):
        """Public interface into debugger options"""
        try:
            return self.options[option]
        except KeyError:
            raise error("Option %s is not a valid option" % option)

    def prep_run(self, cmd):
        pass

    def done_run(self, cmd=None):
        self.RespondDebuggerState(DBGSTATE_NOT_DEBUGGING)
        self.close()

    def canonic(self, fname):
        return os.path.abspath(fname).lower()

    def reset(self):
        debugger_parent.reset(self)
        self.userbotframe = None
        self.UpdateAllLineStates()
        self._UnshowCurrentLine()

    def setup(self, f, t):
        debugger_parent.setup(self, f, t)
        self.bAtException = t is not None

    def set_break(self, filename, lineno, temporary=0, cond=None):
        filename = self.canonic(filename)
        self.SetLineState(filename, lineno, LINESTATE_BREAKPOINT)
        if not self.breaks and self.debuggerState == DBGSTATE_FREETRACING:
            print("-- debugger free-tracing: reactivation of tracing overhead --")
            sys.settrace(self.trace_dispatch)
        self.SetInteractiveContext("current")
        return debugger_parent.set_break(self, filename, lineno, temporary, cond)

    def clear_break(self, filename, lineno):
        filename = self.canonic(filename)
        self.ResetLineState(filename, lineno, LINESTATE_BREAKPOINT)
        r = debugger_parent.clear_break(self, filename, lineno)
        ##if not self.breaks and self.debuggerState != DBGSTATE_BREAK and sys.gettrace():	# py2.6+
        if not self.breaks and self.debuggerState == DBGSTATE_FREETRACING:
            print("-- debugger: deactivation of tracing overhead (0 breakpoints) --")
            sys.settrace(None)
        self.SetInteractiveContext("current")
        return r

    def cmdloop(self):
        if self.frameShutdown:
            return  # App in the process of closing - never break in!
        self.GUIAboutToBreak()

    def print_stack_entry(self, frame):
        # We dont want a stack printed - our GUI is better :-)
        pass

    def user_return(self, frame, return_value):
        # Same as parent, just no "print"
        # This function is called when a return trap is set here
        frame.f_locals["__return__"] = return_value
        self.interaction(frame, None)

    def user_call(self, frame, args):
        # base class has an annoying 'print' that adds no value to us...
        if self.stop_here(frame):
            self.interaction(frame, None)

    def user_exception(self, frame, exc_info):
        # This function is called if an exception occurs,
        # but only if we are to stop at or just below this level
        (exc_type, exc_value, exc_traceback) = exc_info
        if self.get_option(OPT_STOP_EXCEPTIONS):
            frame.f_locals["__exception__"] = exc_type, exc_value
            print("Unhandled exception while debugging...")
            # on both py2k and py3k, we may be called with exc_value
            # being the args to the exception, or it may already be
            # instantiated (IOW, PyErr_Normalize() hasn't been
            # called on the args).  In py2k this is fine, but in
            # py3k, traceback.print_exception fails.  So on py3k
            # we instantiate an exception instance to print.
            if sys.version_info > (3,) and not isinstance(exc_value, BaseException):
                # they are args - may be a single item or already a tuple
                if not isinstance(exc_value, tuple):
                    exc_value = (exc_value,)
                exc_value = exc_type(*exc_value)

            traceback.print_exception(exc_type, exc_value, exc_traceback)
            self.frame_returning = (
                frame  # so set_next etc. take care about falling down
            )
            self.interaction(frame, exc_traceback)
            self.frame_returning = None

    def user_line(self, frame):
        if frame.f_lineno == 0:
            return
        debugger_parent.user_line(self, frame)

    def run_recursive(self, code, globs=None, locs=None):
        # enter recursive debugging (on same instance / GUI)
        d = self
        print("-- enter recursive debugging --")
        old_debstate = (
            d.debuggerState,
            d.bAtPostMortem,
            d.bAtException,
            d.stack,
            d.curindex,
            d.curframe,
            getattr(d, "curframe_locals", None),
            d.frame_returning,
        )

        def _recursive_debugging_func():
            f = sys._getframe()
            while f and f != d.curframe:
                # hide debugger internal frames in stack view
                f.f_locals["__debuggerframe__"] = True  # hides theses fames
                f = f.f_back
            d.set_step()
            sys.settrace(d.trace_dispatch)
            exec(code, globs, locs)

        sys.settrace(None)
        try:
            sys.call_tracing(_recursive_debugging_func, ())
        finally:
            print("-- leave recursive debugging --")
            sys.settrace(d.trace_dispatch)

            # now we need to restore the debugger state for as if
            # entered d.interact() .
            (
                d.bAtPostMortem,
                d.bAtException,
                d.stack,
                d.curindex,
                d.curframe,
                d.curframe_locals,
                d.frame_returning,
            ) = old_debstate[1:]
            d.quitting = 0

            # like GUIAboutToBreak()
            d.GUICheckInit()
            d.RespondDebuggerState(old_debstate[0])
            d.GUIAboutToInteract()  # window showing, interactive context, RespondDebuggerData
            d.ShowCurrentLine()

    def run(self, cmd, globals=None, locals=None, start_stepping=1):
        if not isinstance(cmd, (str, types.CodeType)):
            raise TypeError("Only strings can be run")
        self.last_cmd_debugged = cmd
        if start_stepping:
            self.isInitialBreakpoint = 0
        else:
            self.isInitialBreakpoint = 1
        try:
            if globals is None:
                import __main__

                globals = __main__.__dict__
            if locals is None:
                locals = globals
            sys.settrace(None)  # because DBGSTATE_FREETRACING could be active
            self.reset()
            self.prep_run(cmd)
            _frame = sys._getframe()
            if hasattr(_frame, "_frame"):
                _frame = _frame._frame  # original frame from psyco proxy frame
            __debuggerframe__ = (
                True  # blocks set_step() and stack view etc out here # noqa
            )
            self.userbotframe = (
                _frame  # -> hidding FW in stack & detecting DBGSTATE_FREETRACING
            )
            sys.settrace(self.trace_dispatch)
            if type(cmd) != types.CodeType:
                cmd = cmd + "\n"
            try:
                try:
                    if start_stepping:
                        self.skipBotFrame = SKIP_STEP
                    else:
                        self.skipBotFrame = SKIP_RUN
                        ##self.set_continue()		# sets .stopframe = .botframe # which may not exist
                        self.stopframe = _frame
                    exec(cmd, globals, locals)
                except bdb.BdbQuit:
                    pass
            finally:
                self.skipBotFrame = SKIP_NONE
                self.quitting = 1
                sys.settrace(None)
                self.userbotframe = None
        finally:
            self.done_run(cmd)

    def runeval(self, expr, globals=None, locals=None):
        self.prep_run(expr)
        try:
            debugger_parent.runeval(self, expr, globals, locals)
        finally:
            self.done_run(expr)

    def runexec(self, what, globs=None, locs=None):
        self.reset()
        sys.settrace(self.trace_dispatch)
        try:
            try:
                exec(what, globs, locs)
            except bdb.BdbQuit:
                pass
        finally:
            self.quitting = 1
            sys.settrace(None)

    def do_set_quit(self):
        if self.GUIAboutToRun():  # stop msgpump-> return to previous pump/mainloop
            self.set_quit()

    def do_set_step(self):
        if self.GUIAboutToRun():
            self.set_step()

    def do_set_next(self):
        if self.GUIAboutToRun():
            self.set_next(self.curframe)

    def do_set_until(self):
        if self.GUIAboutToRun():
            self.set_until(self.curframe)

    def do_set_return(self):
        if self.GUIAboutToRun():
            self.set_return(self.curframe)

    def do_set_continue(self):
        if self.GUIAboutToRun():
            self.set_continue()

    skip = None  # support PY2.6-

    def stop_here(self, frame):  # Bdb speed optimized:
        # (CT) stopframe may now also be None, see dispatch_call.
        # (CT) the former test for None is therefore removed from here.
        if self.stopframe is self.dummyframe:  # highest probability
            # fast continue, just breakpoints
            return False
        if frame is self.stopframe:  # next / returning step
            if self.stoplineno == -1:
                return False
            if frame.f_lineno >= self.stoplineno:
                if self.skip and self.is_skipped_module(
                    frame.f_globals.get("__name__")
                ):
                    return False
                return True
            return False

        if self.stopframe is None:
            if self.skip and self.is_skipped_module(frame.f_globals.get("__name__")):
                return False
            return True
        return False

    if sys.version_info < (2, 6):

        def _set_stopinfo(self, stopframe, returnframe, stoplineno=0):
            self.stopframe = stopframe
            self.returnframe = returnframe
            self.quitting = 0
            # stoplineno >= 0 means: stop at line >= the stoplineno
            # stoplineno -1 means: don't stop at all
            self.stoplineno = stoplineno

    def set_step(self):
        """Stop after one line of code."""
        # Bdb copy PLUS: warn & block stepping into bottomless abyss (free tracing / event handler)
        # Issue #13183: pdb skips frames after hitting a breakpoint and running
        # step commands.
        # Restore the trace function in the caller (that may not have been set
        # for performance reasons) when returning from the current frame.
        if self.frame_returning:
            caller_frame = self.frame_returning.f_back
            if (
                caller_frame
                and not caller_frame.f_trace
                and not caller_frame.f_locals.get("__debuggerframe__")
            ):
                caller_frame.f_trace = self.trace_dispatch
            if not caller_frame:
                print("-- left bottom frame -- ")
            self._set_stopinfo(
                None, None
            )  # bottomless -> re-appear on next emerging frame
        else:
            self._set_stopinfo(None, None)

    frame_returning = None  # support PY2.6-

    def set_next(self, frame):
        """Stop on the next line in or below the given frame."""
        if self.frame_returning and frame is self.curframe:
            self.set_step()  # same care about caller_frame stop & .f_trace
        else:
            self._set_stopinfo(frame, None)

    def set_return(self, frame):  # caller_frame aware
        """Stop when returning from the given frame."""
        if self.frame_returning and frame is self.curframe:
            self.set_step()  # same care about caller_frame.f_trace
        else:
            self._set_stopinfo(frame.f_back, frame, 0)

    def set_until(self, frame, lineno=None):
        if self.frame_returning and frame is self.curframe:
            self.set_step()
        else:
            pdb.Pdb.set_until(self, self.curframe)

    def do_set_untilline(self, frame, lineno=None):
        if self.GUIAboutToRun():
            if self.frame_returning and frame is self.curframe:
                self.set_step()
            else:
                self._set_stopinfo(frame, frame, lineno)

    def set_continue(self):
        # fast continue mode. And don't clear sys.settrace() when in free
        # tracing mode - hot state is visible in mainframe caption

        # Don't stop except at breakpoints or when finished
        self._set_stopinfo(self.dummyframe, None, -1)  # NotImplemented -> fast continue
        if not self.breaks:
            # No breakpoints - run without debugger overhead.
            # TODO: DBGSTATE_FREETRACING: With more coding we could switch off trace
            # too and re-activate it on next .set_break() .
            print(
                "-- debugger continue: deactivation of tracing overhead (0 breakpoints) --"
            )
            sys.settrace(None)
            self.SetInteractiveContext(None, None)
            frame = sys._getframe().f_back
            while frame and frame is not self.botframe:
                del frame.f_trace
                frame = frame.f_back

    def _dump_frame_(self, frame, name=None):
        if name is None:
            name = ""
        if frame:
            if frame.f_code and frame.f_code.co_filename:
                fname = os.path.split(frame.f_code.co_filename)[1]
            else:
                fname = "??"
            print(repr(name), fname, frame.f_lineno, frame)
        else:
            print(repr(name), "None")

    def set_trace(self, frame=None):
        if frame is None:
            frame = sys._getframe(1)
            if hasattr(frame, "_frame"):
                # get original frame from psyco proxy as the .trace_dispatch is fed with
                # real frames and will compare against .botframe
                frame = frame._frame
        self.reset()
        self.userbotframe = None
        while frame:
            # scriptutils.py creates a local variable with name
            # '_debugger_stop_frame_', and we dont go past it
            # (everything above this is Pythonwin framework code)
            if "_debugger_stop_frame_" in frame.f_locals:
                self.userbotframe = frame
                break

            frame.f_trace = self.trace_dispatch
            self.botframe = frame
            frame = frame.f_back
        self.set_step()
        sys.settrace(self.trace_dispatch)

    def set_cur_frame(self, frame):
        # Sets the "current" frame - ie, the frame with focus.  This is the
        # frame on which "step out" etc actions are taken.
        # This may or may not be the top of the stack.
        assert frame is not None, "You must pass a valid frame"
        self.curframe = frame
        for f, index in self.stack:
            if f is frame:
                self.curindex = index
                break
        else:
            print(
                "--  this stack frame is a goner from the past retained for inspection --"
            )
        self.SetInteractiveContext(frame.f_globals, frame.f_locals)
        self.GUIRespondDebuggerData()
        self.ShowCurrentLine()

    def IsBreak(self):
        return self.debuggerState == DBGSTATE_BREAK

    def IsDebugging(self):
        return self.debuggerState != DBGSTATE_NOT_DEBUGGING

    def RespondDebuggerState(self, state):
        if state == self.debuggerState:
            return
        if state == DBGSTATE_NOT_DEBUGGING:  # Debugger exists, but not doing anything
            title = ""
        elif state == DBGSTATE_FREETRACING:  # Code is running under the debugger.
            title = " - debugger is free-tracing breakpoints"
        elif state == DBGSTATE_RUNNING:  # Code is running under the debugger.
            title = " - running"
        elif state == DBGSTATE_BREAK:  # We are at a breakpoint or stepping or whatever.
            if self.bAtException:
                if self.bAtPostMortem:
                    title = " - post mortem exception"
                else:
                    title = " - exception"
            else:
                title = " - break"
        else:
            raise error("Invalid debugger state passed!")
        win32ui.GetMainFrame().SetWindowText(
            win32ui.LoadString(win32ui.IDR_MAINFRAME) + title
        )
        if self.debuggerState == DBGSTATE_QUITTING and state != DBGSTATE_NOT_DEBUGGING:
            print("Ignoring state change cos Im trying to stop!", state)
            return
        self.debuggerState = state
        try:
            frame = win32ui.GetMainFrame()
        except win32ui.error:
            frame = None
        if frame is not None:
            for id, klass, float in DebuggerDialogInfos:
                cb = win32ui.GetMainFrame().GetControlBar(id).dialog
                cb.RespondDebuggerState(state)
        # Tell each open editor window about the state transition
        for doc in editor.editorTemplate.GetDocumentList():
            doc.OnDebuggerStateChange(state)
        self.ShowCurrentLine()

    #
    # GUI debugger interface.
    #
    def GUICheckInit(self):
        if self.inited:
            return
        self.inited = 1
        frame = win32ui.GetMainFrame()

        # Ensure the debugger windows are attached to the debugger.
        for id, klass, float in DebuggerDialogInfos:
            w = frame.GetControlBar(id)
            w.dialog.Init(self)
            # Show toolbar if it was visible during last debug session
            # This would be better done using a CDockState, but that class is not wrapped yet
            if win32ui.GetProfileVal(
                "Debugger Windows\\" + w.dialog.title, "Visible", 0
            ):
                frame.ShowControlBar(w, 1, 1)

        # ALWAYS show debugging toolbar, regardless of saved state
        tb = frame.GetControlBar(win32ui.ID_VIEW_TOOLBAR_DBG)
        frame.ShowControlBar(tb, 1, 1)
        self.GUIRespondDebuggerData()

    # 		frame.RecalcLayout()

    def GetDebuggerBar(self, barName):
        frame = win32ui.GetMainFrame()
        for id, klass, float in DebuggerDialogInfos:
            if klass.title == barName:
                return frame.GetControlBar(id)
        assert 0, "Can't find a bar of that name!"

    def GUIRespondDebuggerData(self):
        if not self.inited:  # GUI not inited - no toolbars etc.
            return

        for id, klass, float in DebuggerDialogInfos:
            cb = win32ui.GetMainFrame().GetControlBar(id).dialog
            cb.RespondDebuggerData()

    def GUIAboutToRun(self):
        if not self.GUIAboutToFinishInteract():
            return 0
        self.RespondDebuggerState(
            self.userbotframe and DBGSTATE_RUNNING or DBGSTATE_FREETRACING
        )
        self._UnshowCurrentLine()
        self.bAtException = self.bAtPostMortem = 0
        # StopDebuggerPump() posts WM_QUIT - Must be last action here to not be
        # disturbed by other messages. Fixes bug: SetInteractiveContext() etc. w
        # further message handling somehow eliminated WM_QUIT from the msg queue -
        # which happened e.g. when the last line in interactive was not clean at
        # debugger steps and resulted in mixed up debugging call stack & state and
        # the need to restart the IDE.
        self.StopDebuggerPump()
        return 1

    def GUIAboutToBreak(self):
        "Called as the GUI debugger is about to get context, and take control of the running program."
        if self.pumping > 2:  # allow 3 recursive debuggers
            print("!!! GUIAboutToBreak: too many recursive debuggings - outa here")
            self.set_continue()
            return
        self.GUICheckInit()
        self.RespondDebuggerState(DBGSTATE_BREAK)

        # run a nested Windows message loop right here on top of the call stack
        # during the debugger user interaction

        last_pumping = self.pumping
        self.pumping += 1
        self.GUIAboutToInteract()

        # Run a message loop until a WM_QUIT message is received.
        ##print "-- Start PumpMessages --", self.pumping
        win32ui.GetApp().PumpMessages()  # NOTE - This will NOT return until the user is finished interacting
        ##print "-- Return PumpMessages --", self.pumping	# levels should match!
        self.pumping -= 1
        assert self.pumping == last_pumping, "Fatal: debugger pump level mixed up!"
        self.SetInteractiveContext(None, None)
        if self.frameShutdown:  # User shut down app while debugging
            print("-- SHUT DOWN DEBUGGER INTERACTION --")
        if self.quitting:
            if self.debuggerState == DBGSTATE_FREETRACING:
                self.quitting = 0
                print("-- Debugger Interrupt (FREETRACING) --")
                # When KeyboardInterrupt breaks down through the .trace_dispatch()
                # handler, inevitably sys.gettrace() becomes None by Python internals!
                # So we file a CallAfter (run from message loop below the trace) which
                # re-activates the trace, so that free-tracing can continue.
                win32ui.GetApp().CallAfter(
                    lambda: (
                        sys.settrace(self.trace_dispatch),
                        self.SetInteractiveContext(None, None),
                    )
                )
                # PY3.1+ catches and ignores KeyboardInterrupt in Pdb._cmdloop(), so we
                # use BdbQuit again.
                ##raise KeyboardInterrupt("Debugger KeyboardInterrupt (FREETRACING)")
                raise bdb.BdbQuit("Debugger Interrupt (FREETRACING)")
            raise bdb.BdbQuit  # this can not be raised from beyond the pump by do_set_quit

    def GUIAboutToInteract(self):
        "Called as the GUI is about to perform any interaction with the user"
        frame = win32ui.GetMainFrame()
        # Remember the enabled state of our main frame
        # may be disabled primarily if a modal dialog is displayed.
        # Only get at enabled via GetWindowLong.
        self.oldFrameEnableState = frame.IsWindowEnabled()
        self.oldForeground = win32ui.GetForegroundWindow()
        if not self.oldFrameEnableState:
            # 			fw.EnableWindow(0) Leave enabled for now?
            frame.EnableWindow(1)
        if self.inForcedGUI and not frame.IsWindowVisible():
            frame.ShowWindow(win32con.SW_SHOW)
            frame.UpdateWindow()
        if self.curframe:
            self.SetInteractiveContext(self.curframe.f_globals, self.curframe.f_locals)
        else:
            self.SetInteractiveContext(None, None)
        self.GUIRespondDebuggerData()

    def GUIAboutToFinishInteract(self):
        """Called as the GUI is about to finish any interaction with the user
        Returns non zero if we are allowed to stop interacting"""
        try:
            win32ui.GetMainFrame().EnableWindow(self.oldFrameEnableState)
            self.oldForeground.EnableWindow(1)
        except win32ui.error:
            # old window may be dead.
            pass
        # 			self.oldForeground.SetForegroundWindow() - fails??
        if not self.inForcedGUI:
            return 1  # Never a problem, and nothing else to do.
        # If we are running a forced GUI, we may never get an opportunity
        # to interact again.  Therefore we perform a "SaveAll", to makesure that
        # any documents are saved before leaving.
        for template in win32ui.GetApp().GetDocTemplateList():
            for doc in template.GetDocumentList():
                if not doc.SaveModified():
                    return 0
        # All documents saved - now hide the app and debugger.
        if self.get_option(OPT_HIDE):
            frame = win32ui.GetMainFrame()
            frame.ShowWindow(win32con.SW_HIDE)
        return 1

    #
    # Pythonwin interface - all stuff to do with showing source files,
    # changing line states etc.
    #
    def ShowLineState(self, fileName, lineNo, lineState):
        # Set the state of a line, open if not already
        self.ShowLineNo(fileName, lineNo)
        self.SetLineState(fileName, lineNo, lineState)

    def SetLineState(self, fileName, lineNo, lineState):
        # Set the state of a line if the document is open.
        doc = editor.editorTemplate.FindOpenDocument(fileName)
        if doc is not None:
            marker = _LineStateToMarker(lineState)
            if not doc.MarkerCheck(lineNo, marker):
                doc.MarkerAdd(lineNo, marker)

    def ResetLineState(self, fileName, lineNo, lineState):
        # Set the state of a line if the document is open.
        doc = editor.editorTemplate.FindOpenDocument(fileName)
        if doc is not None:
            marker = _LineStateToMarker(lineState)
            doc.MarkerDelete(lineNo, marker)

    def UpdateDocumentLineStates(self, doc):
        # Show all lines in their special status color.  If the doc is open
        # all line states are reset.
        doc.MarkerDeleteAll(MARKER_BREAKPOINT)
        doc.MarkerDeleteAll(MARKER_CURRENT)
        fname = self.canonic(doc.GetPathName())
        # Now loop over all break-points
        for line in self.breaks.get(fname, []):
            doc.MarkerAdd(line, MARKER_BREAKPOINT)
        # And the current line if in this document.
        if self.shownLineCurrent and fname == self.shownLineCurrent[0]:
            lineNo = self.shownLineCurrent[1]
            if not doc.MarkerCheck(lineNo, MARKER_CURRENT):
                doc.MarkerAdd(lineNo, MARKER_CURRENT)

    # 		if self.shownLineCallstack and fname == self.shownLineCallstack[0]:
    # 			doc.MarkerAdd(self.shownLineCallstack[1], MARKER_CURRENT)

    def UpdateAllLineStates(self):
        for doc in editor.editorTemplate.GetDocumentList():
            self.UpdateDocumentLineStates(doc)

    def ShowCurrentLine(self):
        # Show the current line.  Only ever 1 current line - undoes last current
        # The "Current Line" is self.curframe.
        # The "Callstack Line" is the top of the stack.
        # If current == callstack, only show as current.
        self._UnshowCurrentLine()  # un-highlight the old one.
        if self.curframe:
            fileName = self.canonic(self.curframe.f_code.co_filename)
            lineNo = self.curframe.f_lineno
            self.shownLineCurrent = fileName, lineNo
            self.ShowLineState(fileName, lineNo, LINESTATE_CURRENT)

    def _UnshowCurrentLine(self):
        "Unshow the current line, and forget it"
        if self.shownLineCurrent is not None:
            fname, lineno = self.shownLineCurrent
            self.ResetLineState(fname, lineno, LINESTATE_CURRENT)
            self.shownLineCurrent = None

    def ShowLineNo(self, filename, lineno):
        wasOpen = editor.editorTemplate.FindOpenDocument(filename) is not None
        if os.path.isfile(filename) and scriptutils.JumpToDocument(filename, lineno):
            if not wasOpen:
                doc = editor.editorTemplate.FindOpenDocument(filename)
                if doc is not None:
                    self.UpdateDocumentLineStates(doc)
                    return 1
                return 0
            return 1
        else:
            # Can't find the source file - linecache may have it?
            import linecache

            line = linecache.getline(filename, lineno)
            print(
                "%s(%d): %s"
                % (os.path.basename(filename), lineno, line[:-1].expandtabs(4))
            )
            return 0


try:
    _reload += 1  # noqa
except NameError:
    _reload = 0
else:
    # dev reload support : changes effective mostly immediately
    exec("from .. import debugger")  # SyntaxError in old Pythons
    obj = debugger.currentDebugger  # noqa
    if obj:
        obj.__class__ = getattr(
            sys.modules[obj.__class__.__module__], obj.__class__.__name__
        )
        print("reload-reclassed %s" % obj)
