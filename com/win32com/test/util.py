import sys, os
import win32api
import tempfile
import unittest
import gc
import pythoncom
import winerror
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

def ExecuteShellCommand(cmd, testcase,
                        expected_output = None, # Set to '' to check for nothing
                        tracebacks_ok = 0, # OK if the output contains a t/b?
                        ):
    output_name = tempfile.mktemp('win32com_test')
    cmd = cmd + ' > "%s" 2>&1' % output_name
    rc = os.system(cmd)
    output = open(output_name, "U").read().strip()
    class Failed(Exception): pass
    try:
        if rc:
            raise Failed, "exit code was " + str(rc)
        if expected_output is not None and output != expected_output:
            raise Failed, \
                  "Expected output %r (got %r)" % (expected_output, output)
        if not tracebacks_ok and \
           output.find("Traceback (most recent call last)")>=0:
            raise Failed, "traceback in program output"
        return output
    except Failed, why:
        print "Failed to exec command '%r'" % cmd
        print "Failed as", why
        print "** start of program output **"
        print output
        print "** end of program output **"
        testcase.fail("Executing '%s' failed as %s" % (cmd, why))

class CaptureWriter:
    def __init__(self):
        self.old_err = self.old_out = None
        self.clear()
    def capture(self):
        self.clear()
        self.old_out = sys.stdout
        self.old_err = sys.stderr
        sys.stdout = sys.stderr = self
    def release(self):
        if self.old_out:
            sys.stdout = self.old_out
            self.old_out = None
        if self.old_err:
            sys.stderr = self.old_err
            self.old_err = None
    def clear(self):
        self.captured = []
    def write(self, msg):
        self.captured.append(msg)
    def get_captured(self):
        return "".join(self.captured)
    def get_num_lines_captured(self):
        return len("".join(self.captured).split("\n"))

class TestCaseMixin:
    def _preTest(self):
        self.ni = _GetInterfaceCount()
        self.ng = _GetGatewayCount()
    def _postTest(self, result):
        gc.collect()
        lost_i = _GetInterfaceCount() - self.ni
        lost_g = _GetGatewayCount() - self.ng
        if lost_i or lost_g:
            msg = "%d interface objects and %d gateway objects leaked" \
                                                        % (lost_i, lost_g)
            result.addFailure(self, (AssertionError, msg, None))
    def assertRaisesCOM_HRESULT(self, hresult, func, *args, **kw):
        try:
            func(*args, **kw)
        except pythoncom.com_error, details:
            if details[0]==hresult:
                return
        self.fail("Excepected COM exception with HRESULT 0x%x" % hresult)

class TestCase(unittest.TestCase, TestCaseMixin):
    def __call__(self, result=None):
        if result is None: result = self.defaultTestResult()
        self._preTest()
        try:
            unittest.TestCase.__call__(self, result)
        finally:
            self._postTest(result)

class CapturingFunctionTestCase(unittest.FunctionTestCase, TestCaseMixin):
    def __call__(self, result=None):
        if result is None: result = self.defaultTestResult()
        writer = CaptureWriter()
        self._preTest()
        writer.capture()
        try:
            unittest.FunctionTestCase.__call__(self, result)
        finally:
            writer.release()
            self._postTest(result)
        self.checkOutput(writer.get_captured(), result)
    def checkOutput(self, output, result):
        if output.find("Traceback")>=0:
            msg = "Test output contained a traceback\n---\n%s\n---" % output
            result.errors.append((self, msg))

class ShellTestCase(unittest.TestCase):
    def __init__(self, cmd, expected_output):
        self.__cmd = cmd
        self.__eo = expected_output
        unittest.TestCase.__init__(self)
    def runTest(self):
        ExecuteShellCommand(self.__cmd, self, self.__eo)
    def __str__(self):
        max = 30
        if len(self.__cmd)>max:
            cmd_repr = self.__cmd[:max] + "..."
        else:
            cmd_repr = self.__cmd
        return "exec: " + cmd_repr

def testmain(*args, **kw):
    unittest.main(*args, **kw)
    