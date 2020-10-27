# basic module browser.

# usage:
# >>> import browser
# >>> browser.Browse()
# or
# >>> browser.Browse(your_module)
import sys
import types
import __main__
import inspect

import win32ui
from pywin.mfc import dialog

from . import hierlist

special_names = ["__doc__", "__name__", "__self__"]

#
# HierList items
class HLIPythonObject(hierlist.HierListItem, object):
    def __init__(self, myobject=None, name=None):
        hierlist.HierListItem.__init__(self)
        self.myobject = myobject
        self.knownExpandable = None
        if name:
            self.name = name
        else:
            try:
                self.name = myobject.__name__
            except (AttributeError, TypeError):
                try:
                    r = repr(myobject)
                    if len(r) > 20:
                        r = r[:20] + "..."
                    self.name = r
                except (AttributeError, TypeError):
                    self.name = "???"

    def __lt__(self, other):
        return self.name < other.name

    def __eq__(self, other):
        return self.name == other.name and self.myobject is other.myobject

    def __repr__(self):
        try:
            type = self.GetHLIType()
        except:
            type = "Generic"
        return "<%s(%s) - name: %s  object: %r" % (
            self.__class__.__name__,
            type,
            self.name,
            self.myobject,
        )

    def GetText(self):
        try:
            return str(self.name) + " (" + self.GetHLIType() + ")"
        except AttributeError:
            s = repr(self.myobject)
            if len(s) > 80:
                s = s[:77] + " ..."
            return str(self.name) + " = " + s

    def InsertDocString(self, lst):
        ob = None
        try:
            ob = self.myobject.__doc__
        except (AttributeError, TypeError):
            pass
        # I don't quite grok descriptors enough to know how to
        # best hook them up. Eg:
        # >>> object.__getattribute__.__class__.__doc__
        # <attribute '__doc__' of 'wrapper_descriptor' objects>
        if ob and isinstance(ob, str):
            lst.insert(0, HLIDocString(ob, "Doc"))

    def GetSubList(self):
        ret = []
        d_inst = {}
        try:
            # Note: using dir() would yield too much with all class attrs
            # REQ: handle __slots__ , self.__dir__()
            for (key, ob) in self.myobject.__dict__.items():
                if key not in special_names:
                    ret.append(MakeHLI(ob, key))
                    d_inst[key] = 1
        except (AttributeError, TypeError):
            pass

        ret.sort()
        if ret:
            self.InsertDocString(ret)
        return ret

    # if the has a dict, it is expandable.
    def IsExpandable(self):
        if self.knownExpandable is None:
            self.knownExpandable = self.CalculateIsExpandable()
        return self.knownExpandable

    def CalculateIsExpandable(self):
        try:
            for key in self.myobject.__dict__.keys():
                if key not in special_names:
                    return 1
        except (AttributeError, TypeError):
            pass
        return 0

    def GetBitmapColumn(self):
        if self.IsExpandable():
            return 0
        else:
            return 4

    def TakeDefaultAction(self):
        ShowObject(self.myobject, self.name)

    def LocateObject(self):
        from pywin.framework import scriptutils

        loc = scriptutils.LocateObject(self.myobject, self.name)
        if loc.fn:

            def _jump():
                scriptutils.JumpToDocument(loc.fn, loc.lineno, loc.col, bScrollToTop=0)

            win32ui.GetApp().CallAfter(_jump)
            return True


class CLocateObject:
    def TakeDefaultAction(self):
        if not self.LocateObject():
            ShowObject(self.myobject, self.name)


class HLIDocString(HLIPythonObject):
    def GetHLIType(self):
        return "DocString"

    def GetText(self):
        return self.myobject.strip()

    def IsExpandable(self):
        return 0

    def GetBitmapColumn(self):
        return 6


class HLIModule(HLIPythonObject):
    def GetHLIType(self):
        return "Module"


class HLIPythonDirObject(HLIPythonObject):
    def IsExpandable(self):
        return 1

    def GetSubList(self, classtext=None):
        ret = []
        ret.append(MakeHLI(self.myobject.__class__, classtext))
        for name in dir(self.myobject):
            if name.startswith("__"):
                continue
            ret.append(MakeHLI(getattr(self.myobject, name), name))
        return ret


class HLIFrame(CLocateObject, HLIPythonDirObject):  # TODO: no effect
    def GetHLIType(self):
        return "Stack Frame"

    def GetText(self):
        return str(self.name) + " (%s of %r)" % (
            self.GetHLIType(),
            self.myobject.f_code.co_name,
        )

    def GetSubList(self):
        return HLIPythonDirObject.GetSubList(
            self, "Frame of %r" % self.myobject.f_code.co_name
        )


class HLITraceback(HLIPythonObject):
    def GetHLIType(self):
        return "Traceback"


class HLIClass(CLocateObject, HLIPythonObject):
    def GetHLIType(self):
        return "Class"

    def GetSubList(self):
        ret = []
        for base in self.myobject.__bases__:
            ret.append(MakeHLI(base, "Base class: " + base.__name__))
        ret = ret + HLIPythonObject.GetSubList(self)
        return ret


class HLICode(CLocateObject, HLIPythonDirObject):
    def GetHLIType(self):
        return "Code"

    def GetText(self):
        return str(self.name) + " (%s of %r)" % (
            self.GetHLIType(),
            self.myobject.co_name,
        )

    def IsExpandable(self):
        return self.myobject


class HLIInstance(CLocateObject, HLIPythonObject):
    def GetHLIType(self):
        return "Instance"

    def GetText(self):
        return (
            str(self.name)
            + " (Instance of class "
            + str(self.myobject.__class__.__name__)
            + ")"
        )

    def IsExpandable(self):
        return 1

    def GetSubList(self):
        ret = []
        ret.append(MakeHLI(self.myobject.__class__))
        ret = ret + HLIPythonObject.GetSubList(self)
        return ret


class HLIBuiltinFunction(HLIPythonDirObject):
    def GetHLIType(self):
        return "Builtin Function"


class HLIFunction(CLocateObject, HLIPythonDirObject):
    def GetHLIType(self):
        return "Function"

    def GetText(self):
        return str(self.name) + " (%s %r)" % (
            self.GetHLIType(),
            self.myobject.__code__.co_name,
        )

    def IsExpandable(self):
        return 1

    def GetSubList(self):
        ret = HLIPythonDirObject.GetSubList(self)
        # 		ret.append( MakeHLI( self.myobject.func_argcount, "Arg Count" ))
        try:
            spec = inspect.getargspec(self.myobject)
            ret.append(MakeHLI(spec.args, ":Args"))
            ret.append(MakeHLI(spec.defaults, ":Defaults"))
            ret.append(MakeHLI(spec.varargs, ":Varargs"))
            ret.append(MakeHLI(spec.keywords, ":Keywords"))
        except AttributeError:
            pass
        try:
            code = self.myobject.__code__
            globs = self.myobject.__globals__
        except AttributeError:
            # must be py2.5 or earlier...
            code = self.myobject.__code__
            globs = self.myobject.__globals__
        self.InsertDocString(ret)
        return ret


class HLIMethod(CLocateObject, HLIPythonObject):
    # myobject is just a string for methods.
    def GetHLIType(self):
        return "Method"

    def GetText(self):
        return "Method: " + str(self.myobject) + "()"


class HLISeq(HLIPythonObject):
    def GetHLIType(self):
        return "Sequence (abstract!)"

    def IsExpandable(self):
        return len(self.myobject) > 0

    def GetSubList(self):
        ret = []
        pos = 0
        for item in self.myobject:
            ret.append(MakeHLI(item, "[" + str(pos) + "]"))
            pos = pos + 1
        self.InsertDocString(ret)
        return ret


class HLIList(HLISeq):
    def GetHLIType(self):
        return "List"


class HLITuple(HLISeq):
    def GetHLIType(self):
        return "Tuple"


class HLIDict(HLIPythonObject):
    def GetHLIType(self):
        return "Dict"

    def IsExpandable(self):
        try:
            self.myobject.__doc__
            return 1
        except (AttributeError, TypeError):
            return len(self.myobject) > 0

    def GetSubList(self):
        ret = []
        keys = list(self.myobject.keys())
        keys.sort()
        for key in keys:
            ob = self.myobject[key]
            ret.append(MakeHLI(ob, str(key)))
        self.InsertDocString(ret)
        return ret


# In Python 1.6, strings and Unicode have builtin methods, but we dont really want to see these
class HLIString(HLIPythonObject):
    def IsExpandable(self):
        return 0


TypeMap = {
    type: HLIClass,
    types.FunctionType: HLIFunction,
    types.MethodType: HLIMethod,  # types.MethodType is types.UnboundMethodType
    tuple: HLITuple,
    dict: HLIDict,
    list: HLIList,
    types.ModuleType: HLIModule,
    types.CodeType: HLICode,
    types.BuiltinFunctionType: HLIBuiltinFunction,
    types.FrameType: HLIFrame,
    types.TracebackType: HLITraceback,
    str: HLIString,
    str: HLIString,
    int: HLIPythonObject,
    int: HLIPythonObject,
    bool: HLIPythonObject,
    float: HLIPythonObject,
    type(None): HLIPythonObject,
}
if sys.version_info < (3, 0):
    TypeMap[type] = HLIClass


class HLIListBased(HLIInstance, HLIList):
    def GetHLIType(self):
        return "List based Instance"

    def GetSubList(self):
        return HLIInstance.GetSubList(self) + HLIList.GetSubList(self)


class HLIDictBased(HLIInstance, HLIDict):
    def GetHLIType(self):
        return "Dict based Instance"

    def GetSubList(self):
        return HLIInstance.GetSubList(self) + HLIDict.GetSubList(self)


def MakeHLI(ob, name=None):
    try:
        cls = TypeMap[type(ob)]
    except KeyError:
        if isinstance(ob, (list, tuple)):
            cls = HLIListBased
        elif isinstance(ob, dict):
            cls = HLIDictBased
        # hrmph - this check gets more and more bogus as Python
        # improves.  Its possible we should just *always* use
        # HLIInstance?
        elif hasattr(ob, "__class__"):  # 'new style' class
            cls = HLIInstance
        else:
            cls = HLIPythonObject
    return cls(ob, name)


#########################################
#
# Dialog related.


class DialogShowObject(dialog.Dialog):
    def __init__(self, object, title):
        self.object = object
        self.title = title
        dialog.Dialog.__init__(self, win32ui.IDD_LARGE_EDIT)

    def OnInitDialog(self):
        import re

        self.SetWindowText(self.title)
        self.edit = self.GetDlgItem(win32ui.IDC_EDIT1)
        try:
            strval = str(self.object)
        except:
            t, v, tb = sys.exc_info()
            strval = "Exception getting object value\n\n%s:%s" % (t, v)
            tb = None
        strval = re.sub("\n", "\r\n", strval)
        self.edit.ReplaceSel(strval)


def ShowObject(object, title):
    dlg = DialogShowObject(object, title)
    dlg.DoModal()


# And some mods for a sizable dialog from Sam Rushing!
import win32con
import win32api
import commctrl


class dynamic_browser(dialog.Dialog):
    style = win32con.WS_OVERLAPPEDWINDOW | win32con.WS_VISIBLE
    cs = (
        win32con.WS_CHILD
        | win32con.WS_VISIBLE
        | commctrl.TVS_HASLINES
        | commctrl.TVS_LINESATROOT
        | commctrl.TVS_HASBUTTONS
    )

    dt = [
        ["Python Object Browser", (0, 0, 200, 200), style, None, (8, "MS Sans Serif")],
        ["SysTreeView32", None, win32ui.IDC_LIST1, (0, 0, 200, 200), cs],
    ]

    def __init__(self, hli_root):
        dialog.Dialog.__init__(self, self.dt)
        self.hier_list = hierlist.HierListWithItems(hli_root, win32ui.IDB_BROWSER_HIER)
        self.HookMessage(self.on_size, win32con.WM_SIZE)

    def OnInitDialog(self):
        self.hier_list.HierInit(self)
        return dialog.Dialog.OnInitDialog(self)

    def OnOK(self):
        self.hier_list.HierTerm()
        self.hier_list = None
        return self._obj_.OnOK()

    def OnCancel(self):
        self.hier_list.HierTerm()
        self.hier_list = None
        return self._obj_.OnCancel()

    def on_size(self, params):
        lparam = params[3]
        w = win32api.LOWORD(lparam)
        h = win32api.HIWORD(lparam)
        self.GetDlgItem(win32ui.IDC_LIST1).MoveWindow((0, 0, w, h))


def Browse(ob=__main__):
    "Browse the argument, or the main dictionary"
    root = MakeHLI(ob, repr(ob))
    if not root.IsExpandable():
        raise TypeError(
            "Browse() argument must have __dict__ attribute, or be a Browser supported type"
        )

    dlg = dynamic_browser(root)
    dlg.CreateWindow()
    dlg.SetWindowText(root.GetText())
    return dlg


#
#
# Classes for using the browser in an MDI window, rather than a dialog
#
from pywin.mfc import docview


class BrowserTemplate(docview.DocTemplate):
    def __init__(self):
        docview.DocTemplate.__init__(
            self, win32ui.IDR_PYTHONTYPE, BrowserDocument, None, BrowserView
        )

    def OpenObject(self, root):  # Use this instead of OpenDocumentFile.
        # Look for existing open document
        for doc in self.GetDocumentList():
            if doc.root == root:
                doc.GetFirstView().ActivateFrame()
                return doc
        # not found - new one.
        doc = BrowserDocument(self, root)
        frame = self.CreateNewFrame(doc)
        doc.OnNewDocument()
        self.InitialUpdateFrame(frame, doc, 1)
        return doc


class BrowserDocument(docview.Document):
    def __init__(self, template, root):
        docview.Document.__init__(self, template)
        self.root = root
        self.SetTitle("Browser: " + root.name)

    def OnOpenDocument(self, name):
        raise TypeError("This template can not open files")
        return 0


class BrowserView(docview.TreeView):
    def OnInitialUpdate(self):
        import commctrl

        rc = self._obj_.OnInitialUpdate()
        list = hierlist.HierListWithItems(
            self.GetDocument().root,
            win32ui.IDB_BROWSER_HIER,
            win32ui.AFX_IDW_PANE_FIRST,
        )
        list.HierInit(self.GetParent())
        list.SetStyle(
            commctrl.TVS_HASLINES | commctrl.TVS_LINESATROOT | commctrl.TVS_HASBUTTONS
        )
        return rc


template = None


def MakeTemplate():
    global template
    if template is None:
        template = (
            BrowserTemplate()
        )  # win32ui.IDR_PYTHONTYPE, BrowserDocument, None, BrowserView)


def BrowseMDI(ob=__main__):
    """Browse an object using an MDI window."""

    MakeTemplate()
    root = MakeHLI(ob, repr(ob))
    if not root.IsExpandable():
        raise TypeError(
            "Browse() argument must have __dict__ attribute, or be a Browser supported type"
        )

    template.OpenObject(root)
