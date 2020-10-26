"""
Various utilities for running/importing a script
"""
import sys
import win32ui
import win32api
import win32con
import __main__
from pywin.mfc import dialog
from pywin.mfc.docview import TreeView
import os
import string
import re
import traceback
import linecache
import bdb

from .cmdline import ParseArgs

RS_DEBUGGER_NONE = 0  # Dont run under the debugger.
RS_DEBUGGER_STEP = 1  # Start stepping under the debugger
RS_DEBUGGER_GO = 2  # Just run under the debugger, stopping only at break-points.
RS_DEBUGGER_PM = 3  # Dont run under debugger, but do post-mortem analysis on exception.

debugging_options = """No debugging
Step-through in the debugger
Run in the debugger
Post-Mortem of unhandled exceptions""".split(
    "\n"
)

byte_cr = "\r".encode("ascii")
byte_lf = "\n".encode("ascii")
byte_crlf = "\r\n".encode("ascii")

# A dialog box for the "Run Script" command.
class DlgRunScript(dialog.Dialog):
    "A class for the 'run script' dialog"

    def __init__(self, bHaveDebugger):
        dialog.Dialog.__init__(self, win32ui.IDD_RUN_SCRIPT)
        self.AddDDX(win32ui.IDC_EDIT1, "script")
        self.AddDDX(win32ui.IDC_EDIT2, "args")
        self.AddDDX(win32ui.IDC_COMBO1, "debuggingType", "i")
        self.HookCommand(self.OnBrowse, win32ui.IDC_BUTTON2)
        self.bHaveDebugger = bHaveDebugger

    def OnInitDialog(self):
        rc = dialog.Dialog.OnInitDialog(self)
        cbo = self.GetDlgItem(win32ui.IDC_COMBO1)
        for o in debugging_options:
            cbo.AddString(o)
        cbo.SetCurSel(self["debuggingType"])
        if not self.bHaveDebugger:
            cbo.EnableWindow(0)

    def OnBrowse(self, id, cmd):
        openFlags = win32con.OFN_OVERWRITEPROMPT | win32con.OFN_FILEMUSTEXIST
        dlg = win32ui.CreateFileDialog(
            1, None, None, openFlags, "Python Scripts (*.py)|*.py||", self
        )
        dlg.SetOFNTitle("Run Script")
        if dlg.DoModal() != win32con.IDOK:
            return 0
        self["script"] = dlg.GetPathName()
        self.UpdateData(0)
        return 0


def GetDebugger():
    """Get the default Python debugger.  Returns the debugger, or None.

    It is assumed the debugger has a standard "pdb" defined interface.
    Currently always returns the 'pywin.debugger' debugger, or None
    (pdb is _not_ returned as it is not effective in this GUI environment)
    """
    try:
        import pywin.debugger

        return pywin.debugger
    except ImportError:
        return None


def IsOnPythonPath(path):
    "Given a path only, see if it is on the Pythonpath.  Assumes path is a full path spec."
    # must check that the command line arg's path is in sys.path
    for syspath in sys.path:
        try:
            # Python 1.5 and later allows an empty sys.path entry.
            if syspath and os.path.normcase(
                win32ui.FullPath(syspath)
            ) == os.path.normcase(path):
                return 1
        except win32ui.error as details:
            print(
                "Warning: The sys.path entry '%s' is invalid\n%s" % (syspath, details)
            )
    return 0


def GetPackageModuleName(fileName):
    """Given a filename, return (module name, new path).
    eg - given "c:\a\b\c\my.py", return ("b.c.my",None) if "c:\a" is on sys.path.
    If no package found, will return ("my", "c:\a\b\c")
    """
    path, fname = os.path.split(fileName)
    path = origPath = win32ui.FullPath(path)
    fname = os.path.splitext(fname)[0]
    modBits = []
    newPathReturn = None
    if not IsOnPythonPath(path):
        # Module not directly on the search path - see if under a package.
        while len(path) > 3:  # ie 'C:\'
            path, modBit = os.path.split(path)
            modBits.append(modBit)
            # If on path, _and_ existing package of that name loaded.
            if (
                IsOnPythonPath(path)
                and modBit in sys.modules
                and (
                    os.path.exists(os.path.join(path, modBit, "__init__.py"))
                    or os.path.exists(os.path.join(path, modBit, "__init__.pyc"))
                    or os.path.exists(os.path.join(path, modBit, "__init__.pyo"))
                )
            ):
                modBits.reverse()
                return ".".join(modBits) + "." + fname, newPathReturn
            # Not found - look a level higher
        else:
            newPathReturn = origPath

    return fname, newPathReturn


def GetActiveView():
    """Gets the edit control (eg, EditView) with the focus, or None"""
    try:
        childFrame, bIsMaximised = win32ui.GetMainFrame().MDIGetActive()
        return childFrame.GetActiveView()
    except win32ui.error:
        return None


def GetActiveEditControl():
    view = GetActiveView()
    if view is None:
        return None
    if hasattr(view, "SCIAddText"):  # Is it a scintilla control?
        return view
    try:
        return view.GetRichEditCtrl()
    except AttributeError:
        pass
    try:
        return view.GetEditCtrl()
    except AttributeError:
        pass


def GetActiveEditorDocument():
    """Returns the active editor document and view, or (None,None) if no
    active document or its not an editor document.
    """
    view = GetActiveView()
    if view is None or isinstance(view, TreeView):
        return (None, None)
    doc = view.GetDocument()
    if hasattr(doc, "MarkerAdd"):  # Is it an Editor document?
        return doc, view
    return (None, None)


def GetActiveFileName(bAutoSave=1):
    """Gets the file name for the active frame, saving it if necessary.

    Returns None if it cant be found, or raises KeyboardInterrupt.
    """
    pathName = None
    active = GetActiveView()
    if active is None:
        return None
    try:
        doc = active.GetDocument()
        pathName = doc.GetPathName()

        if bAutoSave and (
            len(pathName) > 0
            or doc.GetTitle()[:8] == "Untitled"
            or doc.GetTitle()[:6] == "Script"
        ):  # if not a special purpose window
            if doc.IsModified():
                try:
                    doc.OnSaveDocument(pathName)
                    pathName = doc.GetPathName()

                    # clear the linecache buffer
                    linecache.clearcache()

                except win32ui.error:
                    raise KeyboardInterrupt

    except (win32ui.error, AttributeError):
        pass
    if not pathName:
        return None
    return pathName


lastScript = ""
lastArgs = ""
lastDebuggingType = RS_DEBUGGER_NONE


def RunScript(defName=None, defArgs=None, bShowDialog=1, debuggingType=None):
    global lastScript, lastArgs, lastDebuggingType
    _debugger_stop_frame_ = 1  # Magic variable so the debugger will hide me!

    # Get the debugger - may be None!
    debugger = GetDebugger()

    if defName is None:
        try:
            pathName = GetActiveFileName()
        except KeyboardInterrupt:
            return  # User cancelled save.
    else:
        pathName = defName
    if not pathName:
        pathName = lastScript
    if defArgs is None:
        args = ""
        if pathName == lastScript:
            args = lastArgs
    else:
        args = defArgs
    if debuggingType is None:
        debuggingType = lastDebuggingType

    if not pathName or bShowDialog:
        dlg = DlgRunScript(debugger is not None)
        dlg["script"] = pathName
        dlg["args"] = args
        dlg["debuggingType"] = debuggingType
        if dlg.DoModal() != win32con.IDOK:
            return
        script = dlg["script"]
        args = dlg["args"]
        debuggingType = dlg["debuggingType"]
        if not script:
            return
        if debuggingType == RS_DEBUGGER_GO and debugger is not None:
            # This may surprise users - they select "Run under debugger", but
            # it appears not to!  Only warn when they pick from the dialog!
            # First - ensure the debugger is activated to pickup any break-points
            # set in the editor.
            try:
                # Create the debugger, but _dont_ init the debugger GUI.
                rd = debugger._GetCurrentDebugger()
            except AttributeError:
                rd = None
            if rd is not None and len(rd.breaks) == 0:
                msg = "There are no active break-points.\r\n\r\nSelecting this debug option without any\r\nbreak-points is unlikely to have the desired effect\r\nas the debugger is unlikely to be invoked..\r\n\r\nWould you like to step-through in the debugger instead?"
                rc = win32ui.MessageBox(
                    msg,
                    win32ui.LoadString(win32ui.IDR_DEBUGGER),
                    win32con.MB_YESNOCANCEL | win32con.MB_ICONINFORMATION,
                )
                if rc == win32con.IDCANCEL:
                    return
                if rc == win32con.IDYES:
                    debuggingType = RS_DEBUGGER_STEP

        lastDebuggingType = debuggingType
        lastScript = script
        lastArgs = args
    else:
        script = pathName

    # try and open the script.
    if (
        len(os.path.splitext(script)[1]) == 0
    ):  # check if no extension supplied, and give one.
        script = script + ".py"
    # If no path specified, try and locate the file
    path, fnameonly = os.path.split(script)
    if len(path) == 0:
        try:
            os.stat(fnameonly)  # See if it is OK as is...
            script = fnameonly
        except os.error:
            fullScript = LocatePythonFile(script)
            if fullScript is None:
                win32ui.MessageBox("The file '%s' can not be located" % script)
                return
            script = fullScript
    else:
        path = win32ui.FullPath(path)
        if not IsOnPythonPath(path):
            sys.path.append(path)

    # py3k fun: If we use text mode to open the file, we get \r\n
    # translated so Python allows the syntax (good!), but we get back
    # text already decoded from the default encoding (bad!) and Python
    # ignores any encoding decls (bad!).  If we use binary mode we get
    # the raw bytes and Python looks at the encoding (good!) but \r\n
    # chars stay in place so Python throws a syntax error (bad!).
    # So: so the binary thing and manually normalize \r\n.
    try:
        f = open(script, "rb")
    except IOError as exc:
        win32ui.MessageBox(
            "The file could not be opened - %s (%d)" % (exc.strerror, exc.errno)
        )
        return

    # Get the source-code - as above	##, normalize \r\n
    code = f.read()  ##.replace(byte_crlf, byte_lf).replace(byte_cr, byte_lf) + byte_lf

    # Remember and hack sys.argv for the script.
    oldArgv = sys.argv
    sys.argv = ParseArgs(args)
    sys.argv.insert(0, script)
    # sys.path[0] is the path of the script
    oldPath0 = sys.path[0]
    newPath0 = os.path.split(script)[0]
    if not oldPath0:  # if sys.path[0] is empty
        sys.path[0] = newPath0
        insertedPath0 = 0
    else:
        sys.path.insert(0, newPath0)
        insertedPath0 = 1
    bWorked = 0
    win32ui.DoWaitCursor(1)
    base = os.path.split(script)[1]
    # Allow windows to repaint before starting.
    win32ui.PumpWaitingMessages()
    win32ui.SetStatusText("Running script %s..." % base, 1)
    exitCode = 0
    from pywin.framework import interact

    # Check the debugger flags
    if debugger is None and (debuggingType != RS_DEBUGGER_NONE):
        win32ui.MessageBox(
            "No debugger is installed.  Debugging options have been ignored!"
        )
        debuggingType = RS_DEBUGGER_NONE

    # Get a code object - ignore the debugger for this, as it is probably a syntax error
    # at this point
    try:
        codeObject = compile(code, script, "exec", dont_inherit=True)
    except:
        # Almost certainly a syntax error!
        _HandlePythonFailure("run script", script)
        # No code object which to run/debug.
        return
    __main__.__file__ = script
    try:
        if debuggingType == RS_DEBUGGER_STEP:
            debugger.run(codeObject, __main__.__dict__, start_stepping=1)
        elif debuggingType == RS_DEBUGGER_GO and debugger._GetCurrentDebugger().breaks:
            debugger.run(codeObject, __main__.__dict__, start_stepping=0)
        else:
            # Post mortem or no debugging
            exec(codeObject, __main__.__dict__)
        bWorked = 1
    except bdb.BdbQuit:
        # Dont print tracebacks when the debugger quit, but do print a message.
        print("Debugging session cancelled.")
        exitCode = 1
        bWorked = 1
    except SystemExit as code:
        exitCode = code
        bWorked = 1
    except KeyboardInterrupt:
        # Consider this successful, as we dont want the debugger.
        # (but we do want a traceback!)
        if interact.edit and interact.edit.currentView:
            interact.edit.currentView.EnsureNoPrompt()
        traceback.print_exc()
        if interact.edit and interact.edit.currentView:
            interact.edit.currentView.AppendToPrompt([])
        bWorked = 1
        sys.last_type, sys.last_value, sys.last_traceback = sys.exc_info()
    except:
        if interact.edit and interact.edit.currentView:
            interact.edit.currentView.EnsureNoPrompt()
        traceback.print_exc()
        if interact.edit and interact.edit.currentView:
            interact.edit.currentView.AppendToPrompt([])

        sys.last_type, sys.last_value, sys.last_traceback = sys.exc_info()
        if debuggingType != RS_DEBUGGER_NONE:
            debugger.pm()

    ##del __main__.__file__   # last run __file__ should better stay for interaction
    sys.argv = oldArgv
    if insertedPath0:
        del sys.path[0]
    else:
        sys.path[0] = oldPath0
    f.close()
    if bWorked:
        win32ui.SetStatusText("Script '%s' returned exit code %s" % (script, exitCode))
    else:
        win32ui.SetStatusText("Exception raised while running script  %s" % base)
    try:
        sys.stdout.flush()
    except AttributeError:
        pass

    win32ui.DoWaitCursor(0)


def ImportFile():
    """This code looks for the current window, and determines if it can be imported.  If not,
    it will prompt for a file name, and allow it to be imported."""
    try:
        pathName = GetActiveFileName()
    except KeyboardInterrupt:
        pathName = None

    if pathName is not None:
        if os.path.splitext(pathName)[1].lower() not in (".py", ".pyw", ".pyx"):
            pathName = None

    if pathName is None:
        openFlags = win32con.OFN_OVERWRITEPROMPT | win32con.OFN_FILEMUSTEXIST
        dlg = win32ui.CreateFileDialog(
            1, None, None, openFlags, "Python Scripts (*.py;*.pyw)|*.py;*.pyw;*.pyx||"
        )
        dlg.SetOFNTitle("Import Script")
        if dlg.DoModal() != win32con.IDOK:
            return 0

        pathName = dlg.GetPathName()

    # If already imported, dont look for package
    path, modName = os.path.split(pathName)
    modName, modExt = os.path.splitext(modName)
    newPath = None
    # note that some packages (*cough* email *cough*) use "lazy importers"
    # meaning sys.modules can change as a side-effect of looking at
    # module.__file__ - so we must take a copy (ie, items() in py2k,
    # list(items()) in py3k)
    for key, mod in list(sys.modules.items()):
        if getattr(mod, "__file__", None):
            fname = mod.__file__
            base, ext = os.path.splitext(fname)
            if ext.lower() in [".pyo", ".pyc"]:
                ext = ".py"
            fname = base + ext
            if win32ui.ComparePath(fname, pathName) and mod.__name__ != "__main__":
                modName = key
                break
    else:  # for not broken
        modName, newPath = GetPackageModuleName(pathName)
        if newPath:
            sys.path.append(newPath)

    if modName in sys.modules:
        bNeedReload = 1
        what = "reload"
    else:
        what = "import"
        bNeedReload = 0

    win32ui.SetStatusText(what.capitalize() + "ing module...", 1)
    win32ui.DoWaitCursor(1)
    # 	win32ui.GetMainFrame().BeginWaitCursor()

    try:
        # always do an import, as it is cheap if it's already loaded.  This ensures
        # it is in our name space.
        codeObj = compile("import " + modName, "<auto import>", "exec")
    except SyntaxError:
        win32ui.SetStatusText('Invalid filename for import: "' + modName + '"')
        return
    try:
        exec(codeObj, __main__.__dict__)
        mod = sys.modules.get(modName)
        if bNeedReload:
            if sys.version_info < (3,):
                from __builtin__ import reload
            else:
                from importlib import reload
            mod = reload(sys.modules[modName])
        win32ui.SetStatusText(
            "Successfully "
            + what
            + "ed module '"
            + modName
            + "': %s" % getattr(mod, "__file__", "<unkown file>")
        )
    except:
        _HandlePythonFailure(what)
    win32ui.DoWaitCursor(0)


def CheckFile():
    """This code looks for the current window, and gets Python to check it
    without actually executing any code (ie, by compiling only)
    """
    try:
        pathName = GetActiveFileName()
    except KeyboardInterrupt:
        return

    what = "check"
    win32ui.SetStatusText(what.capitalize() + "ing module...", 1)
    win32ui.DoWaitCursor(1)
    try:
        f = open(pathName, "rb")
    except IOError as details:
        print("Cant open file '%s' - %s" % (pathName, details))
        return
    try:
        code = f.read()
    finally:
        f.close()
    try:
        codeObj = compile(code, pathName, "exec", dont_inherit=True)
        if RunTabNanny(pathName):
            win32ui.SetStatusText(
                "Python and the TabNanny successfully checked the file '"
                + os.path.basename(pathName)
                + "'"
            )
    except SyntaxError:
        _HandlePythonFailure(what, pathName)
    except:
        traceback.print_exc()
        _HandlePythonFailure(what)
    win32ui.DoWaitCursor(0)


def RunTabNanny(filename):
    import io as io

    tabnanny = FindTabNanny()
    if tabnanny is None:
        win32ui.MessageBox("The TabNanny is not around, so the children can run amok!")
        return

    # Capture the tab-nanny output
    newout = io.StringIO()
    old_out = sys.stderr, sys.stdout
    sys.stderr = sys.stdout = newout
    try:
        tabnanny.check(filename)
    finally:
        # Restore output
        sys.stderr, sys.stdout = old_out
    data = newout.getvalue()
    if data:
        try:
            lineno = data.split()[1]
            lineno = int(lineno)
            _JumpToPosition(filename, lineno)
            try:  # Try and display whitespace
                GetActiveEditControl().SCISetViewWS(1)
            except:
                pass
            win32ui.SetStatusText("The TabNanny found trouble at line %d" % lineno)
        except (IndexError, TypeError, ValueError):
            print("The tab nanny complained, but I cant see where!")
            print(data)
        return 0
    return 1


def _JumpToPosition(fileName, lineno, col=1):
    JumpToDocument(fileName, lineno, col)


def JumpToDocument(fileName, lineno=0, col=1, nChars=0, bScrollToTop=0):
    # Jump to the position in a file.
    # If lineno is <= 0, dont move the position - just open/restore.
    # if nChars > 0, select that many characters.
    # if bScrollToTop, the specified line will be moved to the top of the window
    #  (eg, bScrollToTop should be false when jumping to an error line to retain the
    #  context, but true when jumping to a method defn, where we want the full body.
    # Return the view which is editing the file, or None on error.
    doc = win32ui.GetApp().OpenDocumentFile(fileName)
    if doc is None:
        return None
    frame = doc.GetFirstView().GetParentFrame()
    try:
        view = frame.GetEditorView()
        if frame.GetActiveView() != view:
            frame.SetActiveView(view)
        frame.AutoRestore()
    except AttributeError:  # Not an editor frame??
        view = doc.GetFirstView()
    if lineno > 0:
        charNo = view.LineIndex(lineno - 1)
        start = charNo + (col or 1) - 1
        size = view.GetTextLength()
        try:
            view.EnsureCharsVisible(charNo)
        except AttributeError:
            print("Doesnt appear to be one of our views?")
        view.SetSel(min(start, size), min(start + nChars, size))
    if bScrollToTop:
        curTop = view.GetFirstVisibleLine()
        nScroll = (lineno - 1) - curTop
        view.LineScroll(nScroll, 0)
    view.SetFocus()
    return view


def _HandlePythonFailure(what, syntaxErrorPathName=None):
    typ, details, tb = (
        sys.last_type,
        sys.last_value,
        sys.last_traceback,
    ) = sys.exc_info()
    if isinstance(details, SyntaxError):
        try:
            if (
                not details.filename or details.filename == "<string>"
            ) and syntaxErrorPathName:
                fileName = syntaxErrorPathName
            _JumpToPosition(details.filename, details.lineno, details.offset or 1)
        except (TypeError, ValueError):
            msg = str(details)
        traceback.print_exc()
        win32ui.SetStatusText(
            "Failed to " + what + " - syntax error - %s" % details.msg
        )
    else:
        traceback.print_exc()
        win32ui.SetStatusText("Failed to " + what + " - " + str(details))
    tb = None  # Clean up a cycle.


# Find the Python TabNanny in either the standard library or the Python Tools/Scripts directory.
def FindTabNanny():
    try:
        return __import__("tabnanny")
    except ImportError:
        pass
    # OK - not in the standard library - go looking.
    filename = "tabnanny.py"
    try:
        path = win32api.RegQueryValue(
            win32con.HKEY_LOCAL_MACHINE,
            "SOFTWARE\\Python\\PythonCore\\%s\\InstallPath" % (sys.winver),
        )
    except win32api.error:
        print("WARNING - The Python registry does not have an 'InstallPath' setting")
        print("          The file '%s' can not be located" % (filename))
        return None
    fname = os.path.join(path, "Tools\\Scripts\\%s" % filename)
    try:
        os.stat(fname)
    except os.error:
        print(
            "WARNING - The file '%s' can not be located in path '%s'" % (filename, path)
        )
        return None

    tabnannyhome, tabnannybase = os.path.split(fname)
    tabnannybase = os.path.splitext(tabnannybase)[0]
    # Put tab nanny at the top of the path.
    sys.path.insert(0, tabnannyhome)
    try:
        return __import__(tabnannybase)
    finally:
        # remove the tab-nanny from the path
        del sys.path[0]


def LocatePythonFile(fileName, bBrowseIfDir=1):
    "Given a file name, return a fully qualified file name, or None"
    # first look for the exact file as specified
    if not os.path.isfile(fileName):
        # Go looking!
        baseName = fileName
        for path in sys.path:
            fileName = os.path.abspath(os.path.join(path, baseName))
            if os.path.isdir(fileName):
                if bBrowseIfDir:
                    d = win32ui.CreateFileDialog(
                        1, "*.py", None, 0, "Python Files (*.py)|*.py|All files|*.*"
                    )
                    d.SetOFNInitialDir(fileName)
                    rc = d.DoModal()
                    if rc == win32con.IDOK:
                        fileName = d.GetPathName()
                        break
                    else:
                        raise KeyboardInterrupt
            else:
                fileName = fileName.replace(".", "/") + ".py"
                import glob

                fileNames = glob.glob(fileName)  # allow 1st of "pywin/fr*/int*(.py)"
                if fileNames:
                    fileName = fileNames[0]
                    break  # Found it!

        else:  # for not broken out of
            return None
    return win32ui.FullPath(fileName)


_func_code = "__code__"
if not hasattr(LocatePythonFile, _func_code):
    _func_code = "func_code"  # <Py2.6


def LocateObject(obj, expr=None, editor_current=None, ns_extra=None):
    """Locate the source definition or container definition of `obj`.
    expr:  optional expression for obj; e.g. 'scriptutils.myvariable'
    ns_extra: extra namespace to search (in addition to sys.modules / __main__ / current-interact...)
    return: DictObj(locals()) exposing: fn, lineno, col, typ, error, obj
    """
    import inspect

    ed = editor_current

    # main return attributes

    lineno = col = 1
    error = ""
    fn = None
    typ = None  # 'class', 'import', 'def', '=' ... ; not always set

    # try to get location directly from obj inspection - or info for further parsing search

    expr_open = ""  # when non-zero -> detail search to continue in source text later
    for _retry in 0, 1, 2:
        try:
            if hasattr(obj, "__wrapped__"):
                obj = inspect.unwrap(obj)
            try:
                fn = inspect.getsourcefile(obj)
            except TypeError:
                error = str(sys.exc_info()[1])
            else:
                if fn is None:
                    error = "no file found"
                elif fn.startswith("<"):
                    fn = None
                    error = "frozen module"

            if not fn:

                # its not a module, class, method, function, traceback, frame, or code object

                if not inspect.isclass(obj) and (
                    hasattr(obj, "__dict__") or hasattr(obj, "__slots__")
                ):

                    # user instance -> locate class definition
                    obj = obj.__class__
                    continue

                if "." in expr:

                    # probably some builtin type - lets check for pre-dot containerobject and
                    # setup for text search of attribute
                    parts = expr.split(".")  # .rsplit not in py2.3-
                    expr, expr_open = ".".join(parts[:-1]), parts[-1]
                    if not expr:
                        break
                    if expr in ("self", "cls"):
                        r = FindClassDef(editor_current)
                        if r is None:
                            break  # break _retry loop
                        # (classname, pos, iline, indentstr) = r
                        expr = r[0] + "." + expr_open
                        expr_open = ""
                    obj = GetXNamespace(expr, ns_extra)
                    if obj is not None:
                        continue  # repeat search with pre-dot object and expr_open

            elif inspect.isclass(obj):

                # finds class source faster than inspect.getsourcelines() for classes
                # (locating classes is frequent)

                if sys.version_info > (3,):
                    s = open(fn, "rb").read()
                    # PEP 263 decode
                    l2 = b"\n".join(s[:256].split(b"\n", 2)[:2])
                    m = re.search(
                        b"(?m)^[ \\t\\f]*#.*?coding[:=][ \\t]*([-_.a-zA-Z0-9]+)", l2
                    )
                    s = s.decode(m and m.group(1).decode("ascii") or "utf-8", "replace")
                else:
                    s = open(fn).read()
                ms = list(
                    re.finditer(r"(?m)^([ \t]*)class\s*" + obj.__name__ + r"\b", s)
                )
                if ms:
                    # found in source code
                    typ = "class"
                    if sys.version_info < (2, 4):
                        ms.sort(lambda a, b: cmp(a.group(1), b.group(1)))
                    else:
                        ms.sort(key=lambda m: m.group(1))
                    lineno = s[: ms[0].start()].count("\n") + 1
                    _lines = [ms[0].group()]
                    col = len(ms[0].group()) + 1
                else:
                    # then search for code location of __init__ or other methods
                    keys = list(obj.__dict__.keys())
                    keys.sort()
                    keys.insert(0, "__init__")
                    for k in keys:
                        v = obj.__dict__.get(k)
                        if callable(v) and hasattr(v, _func_code):  # py23 no __code__
                            obj = v
                            break  # retry with code object
                    else:
                        for k in keys:
                            v = getattr(obj, k, None)
                            if callable(v) and hasattr(v, _func_code):
                                obj = v
                                break
                        else:
                            expr_open = expr  # trigger further text search if fn
                            if hasattr(obj, "__module__"):
                                fn = getattr(
                                    sys.modules[obj.__module__], "__file__", None
                                )
                                if fn and fn.endswith(".pyc"):
                                    fn = fn[:-4] + ".py"
                                lineno = 1
                                break
                            break  # found nothing
                    continue  # cont with method
            else:

                # getsourcelines speed is ok for non-classes
                _lines, lineno = inspect.getsourcelines(obj)
                line = _lines[0]
                name = getattr(obj, "__name__", None)
                if name and name in line:
                    col = line.find(name) + 1 or 1
                elif expr:
                    col = line.find(expr.split(".")[-1]) + 1 or 1
            # found!

        except EnvironmentError:
            error = str(sys.exc_info()[1])

        break

    if fn and not expr_open:
        return DictObj(locals())

    # More parsing search for `expr` in the source code text.
    # Search for "EXPR = ..." ; "def|class|import EXPR" ; "class CONTAININGCLS:"

    if expr_open:
        expr = expr_open
    if fn:
        doc = win32ui.GetApp().OpenDocumentFile(fn)
        ed = doc.GetFirstView()
        ed.SetSel(ed.LineIndex(lineno - 1))
        path = fn

    expr0 = expr  # initial expr
    if ed:
        fn_ed = ed.GetDocument().GetPathName()
        pos = ed.GetSel()[1]
        txt = ed.GetTextRange()
        # search for "EXPR = ..." or "def|class|import EXPR"
        t_regex = (
            t_0
        ) = r"(?m)\b(?:%(_expr)s)\s*=[^=]|(^\s*def|^\s*class|\bimport)\s+(%(_expr)s)\b"
        while True:
            _expr = re.escape(expr)
            regex = t_regex % locals()
            plast = None
            for m in re.finditer(regex, txt):
                p = m.end()
                if p > pos and plast is not None:
                    break
                plast = p
                mlast = m
            if plast is not None:
                typ = (m.group(1) or "=").strip()
                # inspect the last match before or first after pos of expr
                lineno = txt[:plast].count("\n") + 1
                if t_regex is t_0 and lineno == ed.GetCurLineNumber() + 1:
                    # same line again and original template
                    if m.group(1) == "class":
                        print("WARN: same inner class? --", m.groups())
                    md = re.search(r"^(\s+)def .*\bself\b", ed.GetLine())
                    if md:
                        nwhite = len(md.group(1))
                        # def <method> -> search for "class CONTAININGCLS:"
                        # (useful for navigating in huge classes)
                        t_regex = r"(?m)^\s{0,%s}(class)\s+(\w)" % (
                            nwhite - 1
                        )  # no inner class from here
                        continue
                    t_regex = r"\b()(%(_expr)s),"  # tuple assignment / usage possibly
                    continue
                # found a typical definition
                fn = fn_ed
                col = mlast.start(mlast.lastindex or 0) - txt[:plast].rfind("\n")
                break
            if "." in expr:
                expr = expr.split(".", 1)[-1]  # drop 1st part and retry
            else:
                # so we only found the predot object/file and not a typical plain or
                # nested (def/class/import/=) definition. Now we could search just for
                # the bare string `expr` anywhere down - or give up and serve the predot
                # part location (file line 0)
                m = re.search(r"\b%s\b" % re.escape(expr), txt)
                if m:
                    pos = m.start()
                    lineno = txt[:pos].count("\n") + 1
                    # found the bare string as last hope to be useful
                    fn = fn_ed
                    col = pos - txt[:pos].rfind("\n") + 1
                break

    return DictObj(locals())


class DictObj(object):
    """exposes dictionary as object - adding optional keyword args"""

    def __init__(self, d=None, **kw):
        if d is not None:
            self.__dict__ = d
        if kw:
            self.__dict__.update(kw)


def FindClassDef(ed, pos=None):
    """find containing class defintion in source code from current editor
    position.

    return: (classname, pos, iline, indentstr)  OR None=no class found
    example position resulting in class `XY`:
         class XY:
            def meth(self):
              abc = self.GetSomeT|hing()   # <--- pos
    """
    if pos is None:
        pos = ed.GetSel()[0]
    txt = ed.GetTextRange()

    # find indent of current expression. simplified: no ml string, but \ in last line ..

    iline = ed.LineFromChar(pos)
    lastline = ed.GetLine(iline - 1)
    while lastline.endswith("\\"):
        iline -= 1
        lastline = ed.GetLine(iline - 1)
    line = ed.GetLine(iline)
    indent_expr = len(re.match(r"(\s*)", line).group(1))  # .replace('\t', '    ')

    # find last def according indent

    plast = None
    for m in re.finditer(r"(?m)^(\s{0,%s})def\s" % (indent_expr - 1), txt):
        p = m.start()
        if p > pos:  # and plast is not None:
            break
        plast = p
        mlast = m
    if plast is None:
        return None

    indent_def = len(mlast.group(1))
    pos = plast

    # find last class according indent

    plast = None
    for m in re.finditer(r"(?m)^(\s{0,%s})class\s+(\w+)" % (indent_def - 1), txt):
        p = m.start()
        if p > pos:  # and plast is not None:
            break
        plast = p
        mlast = m
    if plast is None:
        return None

    pos = mlast.end(1)
    return mlast.group(2), pos, ed.LineFromChar(pos), mlast.group(1)


def GetXNamespace(expr="", ns_extra={}):
    """Get or eval `expr` in combined namespace of sys.modules, __builtins__,
    __main__, ns_extra, and interactive (debugging) context. For calltips,
    auto-complete, object location etc.

    When expr != '' then evaluate `expr` in that namespace and return the
    result object - or None on failure.
    """

    namespace = sys.modules.copy()
    namespace.update(__builtins__)
    namespace.update(__main__.__dict__)
    namespace.update(ns_extra)
    # Get the debugger's context.
    try:
        from pywin.framework import interact

        if interact.edit is not None and interact.edit.currentView is not None:
            globs, locs = interact.edit.currentView.GetContext()[:2]
            if globs is not __main__.__dict__:
                namespace.update(globs)
            if locs is not __main__.__dict__:
                namespace.update(locs)
            else:
                # again - ns_extra with higher prio
                namespace.update(ns_extra)
    except ImportError:
        print("GetXNamespace ImportError interact")
    if not expr:
        return namespace
    try:
        return eval(expr, namespace)
    except:
        return None
