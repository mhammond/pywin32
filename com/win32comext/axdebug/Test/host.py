import os
import traceback

import pythoncom
import win32api
import win32com.server.util
import winerror
from win32com.axdebug import adb, axdebug, codecontainer, contexts, documents, gateways
from win32com.axdebug.util import _wrap, _wrap_remove, trace
from win32com.axscript import axscript
from win32com.client.util import Enumerator
from win32com.server.exception import Exception


class ExternalConnection:
    _public_methods_ = ["AddConnection", "ReleaseConnection"]
    _com_interfaces_ = [pythoncom.IID_IExternalConnection]
    numExtRefs = 0

    def AddConnection(self, extconn, reserved):
        self.numExtRefs = self.numExtRefs + 1
        return self.numExtRefs

    def ReleaseConnection(self, extconn, reserved, fLastReleaseCloses):
        self.numExtRefs = self.numExtRefs - 1
        return self.numExtRefs


# externalConnectionManager = ExternalConnection()
# wrappedExternalConnectionManager = _wrap(externalConnectionManager, pythoncom.IID_IExternalConnection)


def DelegatedExternalConnectionQI(iid):
    # PyIExternalConnection (do I need this?  Keep getting QI'd for it, anyway?)
    if iid == pythoncom.IID_IExternalConnection:
        return wrappedExternalConnectionManager
    return 0


class PySourceModuleDebugDocumentHost(gateways.DebugDocumentHost):
    """A DebugDocumentHost that works with Python source files."""

    def __init__(self, module):
        self.module = module
        gateways.DebugDocumentHost.__init__(self)
        self.codeContainer = None

    def _query_interface_(self, iid):
        from win32com.util import IIDToInterfaceName

        trace(
            "PySourceModuleDebugDocumentHost QI with %s (%s)"
            % (IIDToInterfaceName(iid), str(iid))
        )
        return 0

    def _GetCodeContainer(self):
        if self.codeContainer is None:
            try:
                codeText = open(self.module.__file__, "rt").read()
            except IOError as details:
                codeText = "# Exception opening file\n# %s" % (details)
            from win32com.axdebug import codecontainer

            self.codeContainer = codecontainer.SourceCodeContainer(
                codeText, self.module.__file__
            )
        return self.codeContainer

    def GetDeferredText(self, dwTextStartCookie, maxChars, bWantAttr):
        try:
            trace("GetDeferredText", dwTextStartCookie, maxChars, bWantAttr)
            cont = self._GetCodeContainer()
            if bWantAttr:
                attr = cont.GetSyntaxColorAttributes()
            else:
                attr = None
            return cont.text, attr
        except:
            traceback.print_exc()

    def GetScriptTextAttributes(self, codeText, delimterText, flags):
        # Result must be an attribute sequence of same "length" as the code.
        trace("GetScriptTextAttributes", delimterText, flags)
        raise Exception(scode=winerror.E_NOTIMPL)

    def OnCreateDocumentContext(self):
        # Result must be a PyIUnknown
        trace("OnCreateDocumentContext")
        raise Exception(scode=winerror.E_NOTIMPL)

    def GetPathName(self):
        # Result must be (string, int) where the int is a BOOL
        # - TRUE if the path refers to the original file for the document.
        # - FALSE if the path refers to a newly created temporary file.
        # - raise Exception(scode=E_FAIL) if no source file can be created/determined.
        trace("GetPathName")
        try:
            return win32api.GetFullPathName(self.module.__file__), 1
        except (AttributeError, win32api.error):
            raise Exception(scode == E_FAIL)

    def GetFileName(self):
        # Result is a string with just the name of the document, no path information.
        trace("GetFileName")
        return os.path.split(module.__file__)

    def NotifyChanged():
        trace("NotifyChanged")
        raise Exception(scode=winerror.E_NOTIMPL)


def TestSmartHelper():
    pdm = pythoncom.CoCreateInstance(
        axdebug.CLSID_ProcessDebugManager,
        None,
        pythoncom.CLSCTX_ALL,
        axdebug.IID_IProcessDebugManager,
    )
    app = pdm.CreateApplication()
    app.SetName("Python Process")

    pydebugger = adb.Debugger()

    nodes = BuildModuleTree()

    all_real_nodes = CreateDebugDocumentHelperNodes(pdm, app, nodes)
    root = app.GetRootNode()
    AttachParentNodes(root, nodes, all_real_nodes)

    pydebugger.AttachApp(app)
    cookie = pdm.AddApplication(app)
    input("Waiting...")
    ttest.test()

    pdm.RemoveApplication(cookie)
    print("Done")


def testdumb():
    pdm = pythoncom.CoCreateInstance(
        axdebug.CLSID_ProcessDebugManager,
        None,
        pythoncom.CLSCTX_ALL,
        axdebug.IID_IProcessDebugManager,
    )
    app = pdm.GetDefaultApplication()

    nodes = BuildModuleTree()
    all_real_nodes = CreateDebugDocumentHelperNodes(pdm, app, nodes)
    AttachParentNodes(None, nodes, all_real_nodes)

    parentNode = None
    all_real_nodes = {}
    input("Waiting...")
    print("Done")


def TestSmartProvider():
    import ttest
    from win32com.axdebug import debugger

    d = debugger.AXDebugger()
    # 	d.StartDebugger()
    # 	d.Attach()
    d.Break()
    input("Waiting...")
    ttest.test()
    d.Close()
    print("Done")


def test():
    try:
        # 		app = TestSmartHelper()
        app = TestSmartProvider()
    # 		app = testdumb()
    except:
        traceback.print_exc()


# 	_wrap_remove(externalConnectionManager)
# 	wrappedExternalConnectionManager = None

if __name__ == "__main__":
    test()
    import win32com.axdebug.util

    win32com.axdebug.util._dump_wrapped()
    print(
        " %d/%d com objects still alive"
        % (pythoncom._GetInterfaceCount(), pythoncom._GetGatewayCount())
    )
