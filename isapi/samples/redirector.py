# This is a sample ISAPI extension written in Python.
#
# Please see README.txt in this directory, and specifically the
# information about the "loader" DLL - installing this sample will create
# "_redirector.dll" in the current directory.  The readme explains this.

# Executing this script (or any server config script) will install the extension
# into your web server. As the server executes, the PyISAPI framework will load
# this module and create your Extension and Filter objects.

# This is the simplest possible redirector (or proxy) we can write.  The
# extension installs with a mask of '*' in the root of the site.
# As an added bonus though, we optionally show how, on IIS6 and later, we
# can use HSE_ERQ_EXEC_URL to ignore certain requests - in IIS5 and earlier
# we can only do this with an ISAPI filter - see redirector_with_filter for
# an example.  If this sample is run on IIS5 or earlier it simply ignores
# any excludes.

from isapi import isapicon, threaded_extension
import sys
import traceback
import urllib

# sys.isapidllhandle will exist when we are loaded by the IIS framework.
# In this case we redirect our output to the win32traceutil collector.
if hasattr(sys, "isapidllhandle"):
    import win32traceutil

# The site we are proxying.
proxy = "http://www.python.org"

# Urls we exclude (ie, allow IIS to handle itself) - all are lowered,
# and these entries exist by default on Vista...
excludes = ["/iisstart.htm", "/welcome.png"]

def io_callback(ecb, arg, cbIO, errcode):
    # called when our aynch request completes - there is nothing
    # more for us to do...
    print "IO callback", ecb, arg, cbIO, errcode
    ecb.DoneWithSession()

# The ISAPI extension - handles all requests in the site.
class Extension(threaded_extension.ThreadPoolExtension):
    "Python sample Extension"
    def Dispatch(self, ecb):
        # Note that our ThreadPoolExtension base class will catch exceptions
        # in our Dispatch method, and write the traceback to the client.
        # That is perfect for this sample, so we don't catch our own.
        #print 'IIS dispatching "%s"' % (ecb.GetServerVariable("URL"),)
        url = ecb.GetServerVariable("URL")
        if ecb.Version < 0x60000:
            print "IIS5 or earlier - can't do 'excludes'"
        else:
            for exclude in excludes:
                if url.lower().startswith(exclude):
                    print "excluding %s" % url
                    ecb.ReqIOCompletion(io_callback)
                    ecb.ExecURL(None, None, None, None, None, isapicon.HSE_EXEC_URL_IGNORE_CURRENT_INTERCEPTOR)
                    return isapicon.HSE_STATUS_PENDING

        new_url = proxy + url
        print "Opening %s" % new_url
        fp = urllib.urlopen(new_url)
        headers = fp.info()
        ecb.SendResponseHeaders("200 OK", str(headers) + "\r\n", False)
        ecb.WriteClient(fp.read())
        ecb.DoneWithSession()
        print "Returned data from '%s'!" % (new_url,)
        return isapicon.HSE_STATUS_SUCCESS

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
    vd = VirtualDirParameters(Name="/",
                              Description = Extension.__doc__,
                              ScriptMaps = sm,
                              ScriptMapUpdate = "replace"
                              )
    params.VirtualDirs = [vd]
    HandleCommandLine(params)
