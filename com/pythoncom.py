# Magic utility that "redirects" to pythoncomxx.dll

def __import(modname):
    import win32api, imp, sys, os
    suffix = ""
    if win32api.__file__.find("_d")>0:
        suffix = "_d"
    filename = "%s%d%d%s.dll" % (modname, sys.version_info[0], sys.version_info[1], suffix)
    if hasattr(sys, "frozen"):
        # If we are running from a frozen program (py2exe, McMillan, freeze)
        # then we try and load the DLL from our sys.path
        for look in sys.path:
            found = os.path.join(look, filename)
            if os.path.isfile(found):
                break
        else:
            raise ImportError, "Module '%s' isn't in frozen sys.path directories" % modname
        h = None
    else:
        # Normal Python needs these files in a directory somewhere on
        # %PATH%, so let Windows search it out for us
        h = win32api.LoadLibrary(filename)
        found = win32api.GetModuleFileName(h)
    # Python can load the module
    mod = imp.load_module(modname, None, found, ('.dll', 'rb', imp.C_EXTENSION))
    # and fill our namespace with it.
    globals().update(mod.__dict__)
    if h is not None:
        win32api.FreeLibrary(h)

__import("pythoncom")
del __import
