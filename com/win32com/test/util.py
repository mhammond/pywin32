import sys
import string
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
        return string.join(self.captured,"")
    def get_num_lines_captured(self):
        return len(string.split(string.join(self.captured, ""),"\n"))
