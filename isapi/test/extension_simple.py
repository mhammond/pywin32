# This is an ISAPI extension purely for testing purposes.  It is NOT
# a 'demo' (even though it may be useful!)
#
# Install this extension, then point your browser to:
# "http://localhost/pyisapi_test/test1"
# This will execute the method 'test1' below.  You can specify any method name
# at all, but currently there is only 1.

from isapi import isapicon, threaded_extension, ExtensionError
from isapi.simple import SimpleFilter
import traceback
import urllib
import winerror

# If we have no console (eg, am running from inside IIS), redirect output
# somewhere useful - in this case, the standard win32 trace collector.
import win32api
try:
    win32api.GetConsoleTitle()
except win32api.error:
    # No console - redirect
    import win32traceutil

# The ISAPI extension - handles requests in our virtual dir, and sends the
# response to the client.
class Extension(threaded_extension.ThreadPoolExtension):
    "Python ISAPI Tester"
    def Dispatch(self, ecb):
        print 'Tester dispatching "%s"' % (ecb.GetServerVariable("URL"),)
        url = ecb.GetServerVariable("URL")
        test_name = url.split("/")[-1]
        meth = getattr(self, test_name, None)
        if meth is None:
            raise AttributeError, "No test named '%s'" % (test_name,)
        result = meth(ecb)
        ecb.SendResponseHeaders("200 OK", "Content-type: text/html\r\n\r\n", 
                                False)
        print >> ecb, "<HTML><BODY>OK"
        if result:
            print >> ecb, "<pre>"
            print >> ecb, result
            print >> ecb, "</pre>"
        print >> ecb, "</BODY></HTML>"
        ecb.DoneWithSession()

    def test1(self, ecb):
        try:
            ecb.GetServerVariable("foo bar")
            raise RuntimeError, "should have failed!"
        except ExtensionError, err:
            assert err.errno == winerror.ERROR_INVALID_INDEX, err
        return "worked!"
# The entry points for the ISAPI extension.
def __ExtensionFactory__():
    return Extension()

if __name__=='__main__':
    # If run from the command-line, install ourselves.
    from isapi.install import *
    params = ISAPIParameters()
    # Setup the virtual directories - this is a list of directories our
    # extension uses - in this case only 1.
    # Each extension has a "script map" - this is the mapping of ISAPI
    # extensions.
    sm = [
        ScriptMapParams(Extension="*", Flags=0)
    ]
    vd = VirtualDirParameters(Name="pyisapi_test",
                              Description = Extension.__doc__,
                              ScriptMaps = sm,
                              ScriptMapUpdate = "replace"
                              )
    params.VirtualDirs = [vd]
    HandleCommandLine(params)
