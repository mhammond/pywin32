# frame.py - The MDI frame window for an editor.
import pywin.framework.window
import win32ui
import win32con
import afxres

import ModuleBrowser

class EditorFrame(pywin.framework.window.MDIChildWnd):
    def OnCreateClient(self, cp, context):
        splitter = win32ui.CreateSplitter()
        splitter.CreateStatic (self, 1, 2)
        # Create the default view as specified by the template (ie, the editor view)
        view = context.template.MakeView(context.doc)
        # Create the browser view.
        otherView = ModuleBrowser.BrowserView(context.doc)
        # Note we must add the default view first, so that doc.GetFirstView() returns the editor view.
        splitter.CreateView(view, 0, 1, (0,0)) # size ignored.
        splitter.CreateView (otherView, 0, 0, (0, 0))
        # Restrict the size of the browser splitter (and we can avoid filling
        # it until it is shown)
        splitter.SetColumnInfo(0, 10, 20)
        # And the active view is our default view (so it gets initial focus)
        self.SetActiveView(view)

    def GetEditorView(self):
        # In a multi-view (eg, splitter) environment, get
        # an editor (ie, scintilla) view
        return self.GetActiveDocument().GetFirstView()
    def OnClose(self):
        # Must force the module browser to close itself here (OnDestroy for the view itself is too late!)
        self.GetActiveDocument().GetAllViews()[1].DestroyBrowser()
        return self._obj_.OnClose()