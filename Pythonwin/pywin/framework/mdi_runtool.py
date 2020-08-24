#####################################################################
#
# RunTool by kxrob - MDI-Plugin in the style of sgrepmdi.py / mdi_pychecker
#
# Runs external command-line tools (e.g. pyflakes/flake8, diff/git, test &
# build jobs, cython, grep), collects their standard & error output, and allows to
# comfortably jump to target lines or to add (noqa) ignore comments. The
# target parser currently detects: standard compiler and
# checker error formats, Python tracebacks, unified diff positions, `grep
# -nH` output. Reads non-blocking from external process via thread. Multiple
# instances with separate settings stores are exposed in Menu/File/New.
# ( HKCU\SOFTWARE\Python V.v\Python for Win32\RunTool\num_instances )
#
# REQ: extra right-click "jump to diff old source location" (--- vs
# default new +++ locations)
#
#####################################################################

from __future__ import absolute_import
from __future__ import print_function
import sys
import os
import time
import re
import traceback

import win32ui
import win32api
from pywin.mfc import docview, dialog, window
import win32con
from . import scriptutils
from pywin.xtypes.moves import range

# group(1) is the filename, group(2) is the lineno, group(3) is column, group(4) is errtext
reError = re.compile(r"^([^+-]..[^\(:]+)?[\(:](\d+)[\):](?:(\d*):?)?\s*(.*)")
reTraceback = re.compile(r'["(]([^",]+)"?[,\s]+line (\d+)()()')
# example: File "C:\Python27\Lib\site-packages\pychecker\warn.py", line 242, in _checkFunction
# example: SyntaxError: invalid syntax (C:\path\to\somemod.py, line 240)

# these are the atom numbers defined by Windows for basic dialog controls
BUTTON = 0x80
EDIT = 0x81
STATIC = 0x82
LISTBOX = 0x83
SCROLLBAR = 0x84
COMBOBOX = 0x85

class RTTemplate(docview.RichEditDocTemplate):

    def __init__(self, tname='RunTool'):
        self.tname = tname
        docview.RichEditDocTemplate.__init__(
            self, win32ui.IDR_TEXTTYPE, RTDocument, RTFrame, RTView)
        self.SetDocStrings("\n%(tname)s\n%(tname)s\nRunTool params (*.pywruntool)\n.pywruntool\n\n\n" % locals())
        win32ui.GetApp().AddDocTemplate(self)
        self.docparams = None

    def MatchDocType(self, fileName, _fileType):
        doc = self.FindOpenDocument(fileName)
        if doc:
            return doc
        ext = os.path.splitext(fileName)[1].lower()
        if ext == '.runtool':
            return win32ui.CDocTemplate_Confidence_yesAttemptNative
        return win32ui.CDocTemplate_Confidence_noAttempt

    def SetParams(self, params):
        self.docparams = params

    def ReadParams(self):
        tmp = self.docparams
        self.docparams = None
        return tmp

class RTFrame(window.MDIChildWnd):
    # The template and doc params will one day be removed.
    def __init__(self, wnd=None):
        window.MDIChildWnd.__init__(self, wnd)

def str2int(s, default=0):
    try: return int(s)
    except ValueError:
        return default


class RTParams:
    filpattern = ''
    dirpattern = ''
    remember = 0
    use_file = 1
    preset_prj_root = 0
    def GetParams(self, nc=0):
        l = [self.dirpattern, self.filpattern, self.toolcmd, str(int(self.use_file)), str(int(self.preset_prj_root))]
        for i in range(nc):
            l[i] = ''
        return '\t'.join(l)
    def SetParams(self, paramstr):
        params = paramstr.split('\t') + [''] * 5
        self.dirpattern = params[0]
        self.filpattern = params[1]
        self.toolcmd = params[2] or self.toolcmd
        self.use_file = str2int(params[3], default=1)
        self.preset_prj_root = str2int(params[4])


##DocBase = docview.RichEditDoc
##ViewBase = docview.RichEditView
##from pywin.scintilla.view import CScintillaView as ViewBase
from pywin.scintilla.document import CScintillaDocument as DocBase
from pywin.framework.editor.color.coloreditor import SyntEditView as ViewBase

class RTDocument(RTParams, DocBase):
    result = None
    toolcmd = ''
    def __init__(self, template):
        self.template = template
        self.tname = template.tname
        DocBase.__init__(self, template)

    def CheckExternalDocumentUpdated(self): 
        return False  # required by SyntEditView
    
    def OnOpenDocument(self, fnm):
        #this bizarre stuff with params is so right clicking in a result window
        #and starting a new run can communicate the default parameters
        try:
            params = open(fnm, 'r').read()
        except EnvironmentError:
            params = None
        self.SetInitParams(params)
        return self.OnNewDocument()
    
    kill_cnt = 0
    def OnCloseDocument(self):
        try:
            win32ui.GetApp().DeleteIdleHandler(self.IdleHandler)
        except ValueError:
            pass
        p = self.process
        if p and p.poll() is None:
            self.kill_cnt += 1
            msg = "-- terminating child process %s (%s) --\n" % (p.pid, self.cmd)
            self.GetFirstView().Append(msg)
            sys.stderr.write(msg)
            ##p.terminate()  # doesn't kill childs when shell=1
            os.popen('TASKKILL /T /PID %s /F' % p.pid).read()
            if self.kill_cnt <= 2:
                return 1
            time.sleep(0.7)
        return self._obj_.OnCloseDocument()

    def SaveInitParams(self):
        if self.remember:
            paramstr = self.GetParams(nc=2)
            win32ui.WriteProfileVal(self.tname, 'Params', paramstr)

    num_instances = 2
    def SetInitParams(self, paramstr=None):
        if paramstr is None:
            paramstr = win32ui.GetProfileVal(self.tname, 'Params', '')
        self.SetParams(paramstr)
        
        # setup some reasonable defaults.
        if not self.filpattern:
            try:
                editor = win32ui.GetMainFrame().MDIGetActive()[0].GetEditorView()
                self.filpattern = os.path.basename(editor.GetDocument().GetPathName())
            except (AttributeError, win32ui.error):
                self.filpattern = '*.py'

        if not self.dirpattern:
            try:
                editor = win32ui.GetMainFrame().MDIGetActive()[0].GetEditorView()
                self.dirpattern = os.path.abspath(
                    os.path.dirname(editor.GetDocument().GetPathName()))
            except (AttributeError, win32ui.error):
                self.dirpattern = os.getcwd()

        if self.preset_prj_root:
            prjroot = find_prj_root(self.dirpattern)
            if prjroot:
                self.dirpattern = prjroot

    def FindRunableModuleOrProgCmd(self, name='flake8'):
        py = os.path.join(sys.prefix, 'python.exe')
        if not os.path.isfile(py):
            if "64 bit" in sys.version:
                py = os.path.join(sys.prefix, 'PCBuild', 'amd64', 'python.exe')
            else:
                py = os.path.join(sys.prefix, 'PCBuild', 'python.exe')
        try:
            py = win32api.GetShortPathName(py)
        except win32api.error:
            py = ""
            
        # check if that package supports -m 
        from distutils.sysconfig import get_python_lib
        mainscript = os.path.join(get_python_lib(), name, '__main__.py')
        err = None
        # default command examples
        if name == '4.RunTool':
            name = 'cython'
        cmd = py + ' -m ' + name
        if name == 'flake8':
            cmd += ' --ignore W,E'  # non pep warnings by default
        elif name == '2.RunTool':
            cmd = os.path.join(sys.exec_prefix, 'python setup.py --help')
            self.use_file = 0
        elif name == '3.RunTool':
            cmd = 'git diff'
            self.use_file = 0
        elif 'RunTool' in name:
            cmd = 'make --help'
            self.use_file = 0
        if not os.path.isfile(py):
            if sys.version > '3.3':
                import shutil
                py = shutil.which("python")
            else:
                py = 'python'
            return cmd, "Can't find python.exe! (at %s)\n" % py
        elif not os.path.isfile(mainscript):
            if sys.version > '3.3':
                import shutil
                if shutil.which(name):
                    return name, None
                return cmd, "%s not installed as runnable Python module or on PATH?" % (name)
            return cmd, "Can't find %s as runnable Python module" % (name)
        return cmd, err

    warncmd = None
    def OnNewDocument(self):
        if self.dirpattern == '':
            self.SetInitParams(thetemplate.ReadParams())
            
        if not self.toolcmd.strip():
            self.toolcmd, self.warncmd = self.FindRunableModuleOrProgCmd(self.tname)

        d = RTDialog(self, name=self.tname)
        if d.DoModal() == win32con.IDOK:
            for name in list(d.keys()):
                setattr(self, name, d[name])
            if not self.toolcmd.strip():
                self.toolcmd, self.warncmd = self.FindRunableModuleOrProgCmd(self.tname)
            self.DoRun()
            self.SaveInitParams()
            return 1
        return 0  # cancelled - return zero to stop frame creation.

    def DoRun(self):
        self.SetTitle("Run '%s' in '%s'" % (self.toolcmd, self.dirpattern))
        #self.text = []
        Append = self.GetFirstView().Append
        Append("# Run in " + self.dirpattern + " at %s\n" % time.asctime())
        Append("# Files:   " + self.filpattern + '\n')
        if not os.path.isdir(self.dirpattern):
            Append("# ERROR: directory '%s' doesn't exist " % self.dirpattern)
            self.SetModifiedFlag(0)
            return

        files = self.use_file and self.filpattern or ''
        if re.search(r'\b(pyflakes|flake\d)\b', self.toolcmd):
            # pre-expand  ( pyflakes issue #566 )
            import glob, shlex
            dn = self.dirpattern
            try:
                cwd = os.getcwd()
                if os.path.isdir(dn):
                    os.chdir(dn)
                l = sum([glob.glob(x) or [x] for x in shlex.split(files)], [])
                files = ' '.join([' ' in x and '"%s"' % x or x for x in l])
            finally:
                os.chdir(cwd)
        
        self.cmd = '%s %s' % (self.toolcmd, files)
        Append("# Command: %s\n" % self.cmd)
        win32ui.SetStatusText("Running ...", 0)
        self.StartRun()

    def StartRun(self):
        self.result = None
        win32api.SetCursor(win32api.LoadCursor(0, win32con.IDC_APPSTARTING))
        from pywin.xtypes.moves import _thread
        ##self.ThreadRun()  # in main thread for debugging
        _thread.start_new(self.ThreadRun, ())

    process = None
    def ThreadRun(self):
        import sys
        result = ''
        Append = self.GetFirstView().Append        
        try:
            import subprocess
            t0 = time.time()
            p = self.process = subprocess.Popen(
                self.cmd,  # [self.toolcmd, self.tooloptions, files],
                shell=1,
                bufsize=1,  # 0=unbuffered, 1=line buffered, -1=system default
                cwd=self.dirpattern,
                stdout=subprocess.PIPE, stderr=subprocess.STDOUT,   # STDOUT : stderr goes to same pipe as stdout
            )
            Append("# == Process %s running ... ==\n\n" % p.pid)
            
            enc = getattr(p.stdout, 'encoding', None)
            if not enc:
                from pywin.xtypes.moves import winreg
                k = winreg.OpenKey(winreg.HKEY_LOCAL_MACHINE, r'SYSTEM\CurrentControlSet\Control\Nls\CodePage')
                enc = 'cp' + winreg.QueryValueEx(k, 'OEMCP')[0]
                k.Close()
                try: u''.encode(enc)
                except LookupError:
                    sys.stdout.write("-- unknown console code page: %s\n" % enc)
                    enc = 'ascii'

            ##result = p.stdout.read()
            lines = []
            new = []
            t_last = t0
            ##for line in p.stdout:
            while 1:
                line = p.stdout.readline()
                new.append(line)
                if time.time() - t_last > 1 or not line:
                    t_last = time.time()
                    s = (b''.join(new)).decode(enc, 'replace')
                    Append(s)
                    lines.append(s)
                if not line:
                    break
            p.stdout.close()
            t1 = time.time()
            result = ''.join(lines)
            ##Append(result)
            
            msg = "# == %s run finished after %.1fs ==" % (self.tname, t1 - t0)
            sys.stdout.write(msg + '\r\n')
            Append('\n' + msg + "\n# (Double-click on warning lines to jump to source. Right-click for more)")
            self.SetModifiedFlag(0)
        except Exception:
            global _exc
            _exc = self._exc = sys.exc_info()
            msg = "# == %s ENDs with exception ==\n%s" % (
                self.tname, ''.join(traceback.format_exception_only(*_exc[:2])))
            traceback.print_exc()
            sys.stdout.write(msg)
            Append('\n' + msg)
            self.SetModifiedFlag(0)
        self.result = result

    def IdleHandler(self, _handler, _count):
        time.sleep(0.001)
        if self.result != None:
            win32ui.GetApp().DeleteIdleHandler(self.IdleHandler)
            return 0
        return 1  # more

    def OnSaveDocument(self, filename):
        savefile = open(filename, 'wb')
        txt = self.GetParams() + '\n'
        savefile.write(txt)
        savefile.close()
        self.SetModifiedFlag(0)
        return 1
    # end of RTDocument


def find_prj_root(dn):
    while 1:
        for x in ('.git', '.svn', 'Makefile', 'setup.py'):
            if os.path.exists(os.path.join(dn, x)):
                return dn
        old = dn
        dn = os.path.dirname(dn)
        if dn != old:
            continue
        # no project root found
        return None


ID_OPEN_FILE = 0xe500
ID_SAVERESULTS = 0x502
ID_TRYAGAIN = 0x503
ID_NUMINSTANCES = 0x510
ID_ADDCOMMENT = 0x504

class RTView(ViewBase, object):
    def __init__(self, doc):
        self.doc = doc
        ViewBase.__init__(self, doc)
        ##self.SetWordWrap(win32ui.CRichEditView_WrapNone)
    def OnInitialUpdate(self):
        ##self.HookHandlers()
        rc = ViewBase.OnInitialUpdate(self)  # includes HookHandlers()
        ##fmt = (-402653169, 0, 200, 0, 0, 0, 49, 'Courier New')
        ##self.SetDefaultCharFormat(fmt)
        return rc
    def _on_reclass(self):
        # smart reloader post fix for fast debugging
        print("RTView._on_reclass")
        self.HookHandlers()
    def HookHandlers(self):
        ViewBase.HookHandlers(self)
        self.HookMessage(self.OnRClick, win32con.WM_RBUTTONDOWN)
        self.HookCommand(self.OnCmdOpenFile, ID_OPEN_FILE)
        self.HookCommand(self.OnCmdOpenFile, ID_OPEN_FILE + 1)
        self.HookCommand(self.OnCmdSave, ID_SAVERESULTS)
        self.HookCommand(self.OnTryAgain, ID_TRYAGAIN)
        self.HookCommandRange(self.OnAddComment, ID_ADDCOMMENT, ID_ADDCOMMENT + 4)
        self.HookMessage(self.OnLDblClick, win32con.WM_LBUTTONDBLCLK)
        self.HookMessage(self.OnLButtonUp, win32con.WM_LBUTTONUP)

    lbu_action = None
    def OnLButtonUp(self, *args):
        if self.lbu_action:
            try:
                ##self.lbu_action()
                app.CallAfter(self.lbu_action)
            finally:
                self.lbu_action = None
            return 1
        return 1

    def FindTarget(self, line=None):
        """Detects jump target file & lineno (&col) of: standard warning /
        error formats, python tracebacks, diff / git diff output, grep -nH
        """

        lnno = None
        if line is None:
            lnno = self.LineFromChar(-1)  # selection or current line
            line = self.GetLine(lnno)
            
        # check standard warning / error lines and Python tracebacks
        
        m = reError.match(line) or \
            reTraceback.search(line) or \
            (lnno and reTraceback.search(self.GetLine(lnno - 1)))  # python traceback style
        if m:
            col = m.group(3)
            col = col and int(col) or 0
            return scriptutils.DictObj(
                fname=m.group(1),
                lineno=int(m.group(2)),
                col=col,
                errtext=m.group(4))
        
        # check for diff / git diff output
        
        _a, end = self.GetSel()
        lnno = self.LineFromChar(end)
        ##lnno = self.GetCurLineNumber()
        linestart = self.LineIndex(lnno)
        lineend = self.LineIndex(lnno + 1)
        col = end - linestart
        lineno = lineno_old = 0  # of jump target
        s = self.GetTextRange(0, lineend)
        l = list(re.finditer(r'(?m)^\+\+\+ ("[^"]+"|\S+)', s))
        if l:
            
            # found diff new file signature
            
            m = l[-1]
            fname = m.group(1)
            if fname[:2] in ('b/', 'a/'):
                fname = './' + fname[2:]
            fn1 = fname
            root = self.doc.dirpattern
            if not os.path.isabs(fname):
                fname = os.path.normpath(os.path.join(root, fname))
            if not os.path.isfile(fname):
                prjroot = find_prj_root(root)
                if prjroot:
                    root = prjroot
                    fname = os.path.normpath(os.path.join(root, fn1))
            if not os.path.isfile(fname):
                print("-- file '%s' not found --" % fname)
                return None
            l = list(re.compile(r'(?m)^@@ -(\d+),\d+ \+(\d+),\d+ @@').finditer(s, m.start()))
            if l:
                # hunk start found
                m = l[-1]
                lineno_old = int(m.group(1))
                lineno = int(m.group(2))
                hunklines = s[m.start():]
                for line in hunklines.split('\n')[2:]:
                    if line[:1] in (' ', '+'):
                        lineno += 1
                    if line[:1] in (' ', '-'):
                        lineno_old += 1
            ##print "-- found: '%(fname)s' line %(lineno)s:%(col)s' --" % locals()
            o = scriptutils.DictObj(
                fname=fname,
                lineno=lineno,
                col=col,
                errtext='')
            l_oldfile = list(re.finditer(r'(?m)^--- ("[^"]+"|\S+)', s))
            if l_oldfile:
                m = l_oldfile[-1]
                o.fname_old = m.group(1)
                if not os.path.isabs(o.fname_old):
                    o.fname_old = os.path.normpath(os.path.join(root, o.fname_old))
                o.lineno_old = lineno_old
            return o
        
        return None

    def OnLDblClick(self, _params=None, delay_action=True, ft=None):
        ft = ft or self.FindTarget()
        if ft:
            if not os.path.dirname(ft.fname):
                ft.fname = os.path.normpath(os.path.join(self.doc.dirpattern, ft.fname))
            def action():
                view = scriptutils.JumpToDocument(ft.fname, ft.lineno, ft.col)
                if view:
                    view.AddLastPosEvent()
            ##app.CallAfter(_action)
            if delay_action:
                self.lbu_action = action  # delayed action or dbl click will mess mark/sel
            else:
                action()
            return 0    # done
        return 1    # pass it on by default.

    def OnCmdOpenFile(self, cmd, _code):
        ft = self.ft
        if cmd == ID_OPEN_FILE + 1:
            ft.fname = ft.fname_old
            ft.lineno = ft.lineno_old
        return self.OnLDblClick(delay_action=False, ft=ft)

    def OnRClick(self, params):
        menu = win32ui.CreatePopupMenu()
        flags = win32con.MF_STRING | win32con.MF_ENABLED
        self.ft = ft = self.FindTarget()
        if ft:
            menu.AppendMenu(flags | win32con.MF_DEFAULT, ID_OPEN_FILE, "&Jump to %s:%s" % (
                os.path.basename(ft.fname), ft.lineno))
            if hasattr(ft, 'lineno_old'):
                menu.AppendMenu(flags | win32con.MF_DEFAULT, ID_OPEN_FILE + 1, "&Jump to --- %s:%s" % (
                    os.path.basename(ft.fname_old), ft.lineno_old))
            menu.AppendMenu(flags, ID_ADDCOMMENT + 0, "&Add to source: # noqa")
            menu.AppendMenu(flags, ID_ADDCOMMENT + 1, "&Add to source: # noqa:<WARNCODE>")
            menu.AppendMenu(flags, ID_ADDCOMMENT + 2, "&Add to source: # noqa=<WARNREGEX>")
            menu.AppendMenu(flags, ID_ADDCOMMENT + 3, "&Add to source: #pylint:disable=W")
            menu.AppendMenu(flags, ID_ADDCOMMENT + 4, "&Add to source: #pylint:disable=<WARNCODE>")
            menu.AppendMenu(win32con.MF_SEPARATOR)
        menu.AppendMenu(flags, ID_TRYAGAIN, "&Try Again")
        menu.AppendMenu(flags, win32con.MF_SEPARATOR)
        menu.AppendMenu(flags, win32ui.ID_EDIT_CUT, "Cu&t")
        menu.AppendMenu(flags, win32ui.ID_EDIT_COPY, "&Copy")
        menu.AppendMenu(flags, win32ui.ID_EDIT_PASTE, "&Paste")
        menu.AppendMenu(flags, win32con.MF_SEPARATOR)
        menu.AppendMenu(flags, win32ui.ID_EDIT_SELECT_ALL, "&Select all")
        menu.AppendMenu(flags, win32con.MF_SEPARATOR)
        menu.AppendMenu(flags, ID_SAVERESULTS, "Sa&ve results")
        menu.TrackPopupMenu(params[5])
        return 0

    def OnAddComment(self, cmd, _code):
        icm = cmd - ID_ADDCOMMENT
        lcm = [
            '# noqa',
            '# noqa:<WARNCODE>',
            '# noqa=<WARNREGEX>',
            '#pylint:disable=W',
            '#pylint:disable=<WARNCODE>',
        ]
        cm = lcm[icm]
        addspecific = cmd & 1
        sel = list(self.GetSel())
        sel.sort()
        start, end = sel
        line_start, line_end = self.LineFromChar(start), self.LineFromChar(end)
        first = 1
        for i in range(line_start, line_end + 1):
            line = self.GetLine(i)
            ft = self.FindTarget(line)
            if ft:
                if not os.path.dirname(ft.fname):
                    ft.fname = os.path.join(self.doc.dirpattern, ft.fname)
                view = scriptutils.JumpToDocument(ft.fname, ft.lineno)
                pos = view.LineIndex(ft.lineno) - 1
                if view.GetTextRange(pos - 1, pos) in ('\r', '\n'):
                    pos -= 1
                view.SetSel(pos, pos)
                errtext = ft.errtext.strip()
                errcode = 'W'
                m = re.search(r'([A-Z]\d\d\d\d?\b)', errtext)
                if m:
                    errcode = m.group(1)
                if start != end and line_start == line_end:
                    errcode = self.GetSelText()
                errcode = re.escape(errcode).replace('\ ', ' ')
                cmnt = cm.replace('<WARNREGEX>', repr(
                    re.escape(errtext).replace('\ ', ' ')))
                cmnt = cmnt.replace('<WARNCODE>', errcode)
                ##if cmnt != cm:
                ##    cmnt = dialog.GetSimpleInput("Add", cmnt)
                if not cmnt:
                    return 0

                cmnt = cmnt % locals()
                view.ReplaceSel('  ' + cmnt)
        return 0

    def OnTryAgain(self, _cmd, _code):
        thetemplate.SetParams(self.GetDocument().GetParams())
        thetemplate.OpenDocumentFile()
        return 0

    def OnCmdSave(self, _cmd, _code):
        flags = win32con.OFN_OVERWRITEPROMPT
        dlg = win32ui.CreateFileDialog(0, None, None, flags, "Text Files (*.txt)|*.txt||", self)
        dlg.SetOFNTitle("Save Results As")
        if dlg.DoModal() == win32con.IDOK:
            pn = dlg.GetPathName()
            self._obj_.SaveFile(pn)
        return 0

    def Append(self, strng):
        numlines = self.GetLineCount()
        endpos = self.LineIndex(numlines - 1) + len(self.GetLine(numlines - 1))
        self.SetSel(endpos, endpos)
        self.ReplaceSel(strng)


class RTDialog(dialog.Dialog):
    def __init__(self, doc, name='RunTool'):
        self.doc = doc
        wc = win32con
        style = wc.DS_MODALFRAME | wc.WS_POPUP | wc.WS_VISIBLE | wc.WS_CAPTION | wc.WS_SYSMENU | wc.DS_SETFONT
        CS = wc.WS_CHILD | wc.WS_VISIBLE
        tmp = [
            [name, (0, 0, 310, 90), style, None, (8, 'MS Sans Serif')],
            [STATIC, "Command:", -1, (7, 7, 50, 9), CS],
            [EDIT, doc.toolcmd, 100, (50, 7, 250, 11), CS | wc.WS_TABSTOP
             | wc.ES_AUTOHSCROLL | wc.WS_BORDER],

            ##[STATIC, "File(s):",  -1, (20, 20, 30 , 9), CS],
            [BUTTON, "&File(s):",     104, (7,  20,  40,  9), CS | wc.BS_AUTOCHECKBOX | wc.BS_LEFTTEXT | wc.WS_TABSTOP],
            [EDIT, '-', 103, (50, 20, 250, 11), CS | wc.WS_TABSTOP | wc.ES_AUTOHSCROLL | wc.WS_BORDER],
            [STATIC, "&Directory:",  -1, (7, 34, 40, 9), CS],
            [EDIT, '-', 102, (50, 34, 250, 11), CS | wc.WS_TABSTOP | wc.ES_AUTOHSCROLL | wc.WS_BORDER],
            [BUTTON, "&Prefill project root directory containing .git / Makefile / setup.py (next time)",
             105, (7,  48,  300,  9), CS | wc.BS_AUTOCHECKBOX | wc.WS_TABSTOP],
            #    [BUTTON, '...',                 110, (182,34,  16,  11), CS | wc.BS_PUSHBUTTON | wc.WS_TABSTOP],
            ##[STATIC, "Options:",            -1, (7,  48,  50,  9), CS ],
            ##[EDIT,   gp,                    101, (52, 48, 128,  11), CS | wc.WS_TABSTOP | wc.ES_AUTOHSCROLL | wc.WS_BORDER ],
            #    [BUTTON, '...',                 111, (182,48,  16,  11), CS | wc.BS_PUSHBUTTON | wc.WS_TABSTOP],
            [BUTTON, "&Remember", 106, (7, 70, 128, 9), CS | wc.BS_AUTOCHECKBOX | wc.WS_TABSTOP],
            [BUTTON, "&OK", wc.IDOK, (190, 70, 50, 12), CS | wc.BS_DEFPUSHBUTTON | wc.WS_TABSTOP],
            [BUTTON, "&Cancel", wc.IDCANCEL, (250, 70, 50, 12), CS | wc.BS_PUSHBUTTON | wc.WS_TABSTOP],
        ]
        dialog.Dialog.__init__(self, tmp)
        self.AddDDX(100, 'toolcmd')
        ##self.AddDDX(101,'tooloptions')
        self.AddDDX(102, 'dirpattern')
        self.AddDDX(103, 'filpattern')
        self.AddDDX(104,'use_file')
        self.HookCommand(self.OnUpdate, 104)
        self.AddDDX(105,'preset_prj_root')
        self.HookCommand(self.OnUpdatePrePrj, 105)
        self.AddDDX(106, 'remember')
        self._obj_.data['toolcmd'] = doc.toolcmd
        self._obj_.data['dirpattern'] = doc.dirpattern
        self._obj_.data['filpattern'] = doc.filpattern
        self._obj_.data['use_file']  = doc.use_file
        self._obj_.data['preset_prj_root'] = doc.preset_prj_root
        self._obj_.data['remember'] = 0
        ##self.HookCommand(self.OnMoreDirectories, 110)
        ##self.HookCommand(self.OnMoreFiles, 111)

    def OnInitDialog(self):
        self.UpdateData(0)
        self.files = self.GetDlgItem(103)
        self.OnUpdate()
        if self.doc.warncmd:
            self.SetWindowText(self.doc.warncmd)
        return 1  # focus on first control

    def OnUpdate(self, *args):
        self.UpdateData(1)
        self.files.EnableWindow(not not self['use_file'])
    def OnUpdatePrePrj(self, *args):
        self.GetDlgItem(106).SetCheck(1)

    def OnMoreDirectories(self, _cmd, _code):
        self.getMore('RunTool\\Directories', 'dirpattern')

    def OnMoreFiles(self, _cmd, _code):
        self.getMore('RunTool\\File Types', 'filpattern')

    def getMore(self, section, key):
        self.UpdateData(1)
        #get the items out of the ini file
        ini = win32ui.GetProfileFileName()
        secitems = win32api.GetProfileSection(section, ini)
        items = []
        for secitem in secitems:
            items.append(secitem.split('=')[1])
        dlg = ParamsDialog(items)
        if dlg.DoModal() == win32con.IDOK:
            itemstr = ';'.join(dlg.getItems())
            self._obj_.data[key] = itemstr
            #update the ini file with dlg.getNew()
            i = 0
            newitems = dlg.getNew()
            if newitems:
                items = items + newitems
                for item in items:
                    win32api.WriteProfileVal(section, repr(i), item, ini)
                    i = i + 1
            self.UpdateData(0)

    def OnOK(self):
        self.UpdateData(1)
        for theid, name in [
            (100, 'toolcmd'),
            (102, 'dirpattern'),
            ##(103,'filpattern'),
        ]:
            if not self[name]:
                self.GetDlgItem(theid).SetFocus()
                win32api.MessageBeep()
                win32ui.SetStatusText("%s: Please enter a value" % name)
                return
        self._obj_.OnOK()


class ParamsDialog(dialog.Dialog):
    def __init__(self, items):
        self.items = items
        self.newitems = []
        self.selections = []
        style = win32con.DS_MODALFRAME | win32con.WS_POPUP | win32con.WS_VISIBLE | win32con.WS_CAPTION | win32con.WS_SYSMENU | win32con.DS_SETFONT
        CS = win32con.WS_CHILD | win32con.WS_VISIBLE
        tmp = [["RunTool Parameters", (0, 0, 205, 100), style, None, (8, "MS Sans Serif")], ]
        tmp.append([LISTBOX, '', 107, (7, 7, 150, 72), CS | win32con.LBS_MULTIPLESEL |
                    win32con.LBS_STANDARD | win32con.LBS_HASSTRINGS | win32con.WS_TABSTOP | win32con.LBS_NOTIFY])
        tmp.append([BUTTON, 'OK', win32con.IDOK, (167, 7, 32, 12), CS
                    | win32con.BS_DEFPUSHBUTTON | win32con.WS_TABSTOP])
        tmp.append([BUTTON, 'Cancel', win32con.IDCANCEL, (167, 23, 32, 12),
                    CS | win32con.BS_PUSHBUTTON | win32con.WS_TABSTOP])
        tmp.append([STATIC, 'New:', -1, (2, 83, 15, 12), CS])
        tmp.append([EDIT, '', 108, (18, 83, 139, 12), CS | win32con.WS_TABSTOP
                    | win32con.ES_AUTOHSCROLL | win32con.WS_BORDER])
        tmp.append([BUTTON, 'Add', 109, (167, 83, 32, 12), CS
                    | win32con.BS_PUSHBUTTON | win32con.WS_TABSTOP])
        dialog.Dialog.__init__(self, tmp)
        self.HookCommand(self.OnAddItem, 109)
        self.HookCommand(self.OnListDoubleClick, 107)
        self.Hook

    def OnInitDialog(self):
        lb = self.GetDlgItem(107)
        for item in self.items:
            lb.AddString(item)
        return self._obj_.OnInitDialog()

    def OnAddItem(self, _cmd, _code):
        eb = self.GetDlgItem(108)
        item = eb.GetLine(0)
        self.newitems.append(item)
        lb = self.GetDlgItem(107)
        i = lb.AddString(item)
        lb.SetSel(i, 1)
        return 1

    def OnListDoubleClick(self, _cmd, code):
        if code == win32con.LBN_DBLCLK:
            self.OnOK()
            return 1

    def OnOK(self):
        lb = self.GetDlgItem(107)
        self.selections = lb.GetSelTextItems()
        self._obj_.OnOK()

    def getItems(self):
        return self.selections

    def getNew(self):
        return self.newitems

app = win32ui.GetApp()
try:    
    for t in app.GetDocTemplateList():
        if t.__class__.__name__ == 'RTTemplate':
            app.RemoveDocTemplate(t)
except NameError:
    pass

thetemplate = RTTemplate('flake8')
thetemplate = RTTemplate('RunTool')
if 1:
    # more instances
    # HKCU\SOFTWARE\Python V.v\Python for Win32\RunTool\num_instances
    s = win32ui.GetProfileVal('RunTool', 'num_instances', '5')
    try: num_instances = int(s)
    except ValueError:
        num_instances = 5
    for i in range(1, num_instances):
        RTTemplate('%s.RunTool' % (i + 1))
    #TODO: possibly expose num_instances in MainMenu/View/Options Dialog

##_reload_smart = None  # suppress smart reloader
