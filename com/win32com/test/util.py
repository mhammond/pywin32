import sys, os
import win32api
from pythoncom import _GetInterfaceCount, _GetGatewayCount

def CheckClean():
    # Ensure no lingering exceptions - Python should have zero outstanding
    # COM objects
    sys.exc_traceback = sys.exc_value = sys.exc_type = None
    c = _GetInterfaceCount()
    if c:
        print "Warning - %d com interface objects still alive" % c
    c = _GetGatewayCount()
    if c:
        print "Warning - %d com gateway objects still alive" % c

def RegisterPythonServer(filename, verbose=0):
    cmd = '%s "%s" > nul' % (win32api.GetModuleFileName(0), filename)
    if verbose:
        print "Registering engine", filename
#       print cmd
    rc = os.system(cmd)
    if rc:
        raise RuntimeError, "Registration of engine '%s' failed" % filename

class CaptureWriter:
    def __init__(self):
        self.old = None
        self.clear()
    def capture(self):
        self.clear()
        self.old = sys.stdout
        sys.stdout = self
    def release(self):
        if self.old:
            sys.stdout = self.old
            self.old = None
    def clear(self):
        self.captured = []
    def write(self, msg):
        self.captured.append(msg)
    def get_captured(self):
        return "".join(self.captured)
    def get_num_lines_captured(self):
        return len("".join(self.captured).split("\n"))
