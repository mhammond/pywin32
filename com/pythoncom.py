# Magic utility that "redirects" to pythoncomxx.dll

def __import(modname):
    import win32api, imp, sys
    suffix = ""
    if win32api.__file__.find("_d")>0:
        suffix = "_d"
    filename = "%s%d%d%s.dll" % (modname, sys.version_info[0], sys.version_info[1], suffix)
    # win32 can find the DLL name.
    h = win32api.LoadLibrary(filename)
    found = win32api.GetModuleFileName(h)
    # Python can load the module
    mod = imp.load_module(modname, None, found, ('.dll', 'rb', imp.C_EXTENSION))
    # and fill our namespace with it.
    globals().update(mod.__dict__)
    win32api.FreeLibrary(h)

__import("pythoncom")
del __import