# Magic utility that "redirects" to pywintypesxx.dll

def __import(modname):
    # *sigh* - non-admin installs will not have pywintypesxx.dll in the 
    # system directory, so 'import win32api' will fail looking
    # for pywintypes - the exact DLL we are trying to load!
    # So if it exists in sys.prefix, then we try and load it from
    # there, as that way we can avoid the win32api import
    import imp, sys, os
    # See if this is a debug build.
    for suffix_item in imp.get_suffixes():
        if suffix_item[0]=='_d.pyd':
            suffix = '_d'
            break
    else:
        suffix = ""
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
    else:
        if os.path.isfile(os.path.join(sys.prefix, filename)):
            found = os.path.join(sys.prefix, filename)
        else:
            # We could still avoid win32api here, but...
            import win32api
            # Normal Python needs these files in a directory somewhere on
            # %PATH%, so let Windows search it out for us
            h = win32api.LoadLibrary(filename)
            found = win32api.GetModuleFileName(h)
    # Python can load the module
    mod = imp.load_module(modname, None, found, ('.dll', 'rb', imp.C_EXTENSION))
    # and fill our namespace with it.
    globals().update(mod.__dict__)

__import("pywintypes")
del __import
