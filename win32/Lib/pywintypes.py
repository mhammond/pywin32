# Magic utility that "redirects" to pywintypesxx.dll

def __import_pywin32_system_module__(modname, globs):
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
    filename = "%s%d%d%s.dll" % \
               (modname, sys.version_info[0], sys.version_info[1], suffix)
    if hasattr(sys, "frozen"):
        # If we are running from a frozen program (py2exe, McMillan, freeze)
        # then we try and load the DLL from our sys.path
        for look in sys.path:
            # If the sys.path entry is a (presumably) .zip file, use the
            # directory 
            if os.path.isfile(look):
                look = os.path.dirname(look)            
            found = os.path.join(look, filename)
            if os.path.isfile(found):
                break
        else:
            raise ImportError, \
                  "Module '%s' isn't in frozen sys.path directories" % modname
    else:
        search_dirs = [sys.prefix] + \
                      os.environ.get("PATH", "").split(os.pathsep)
        for d in search_dirs:
            found = os.path.join(d, filename)
            if os.path.isfile(found):
                break
        else:
            # Eeek - can't find on the path.  Try "LoadLibrary", as it
            # has slightly different semantics than a simple sys.path search
            # XXX - OK, we *don't* try LoadLibrary - if we can't find it, 
            # there is an excellent change Windows can't find it, and
            # the attempt to bring in win32api *needs* Windows to find it.
            # If Windows can't find it, it displays a dialog trying to 
            # import win32api, which is not what we want!
            raise ImportError, "Can not locate " + filename
    # Python can load the module
    mod = imp.load_module(modname, None, found, 
                          ('.dll', 'rb', imp.C_EXTENSION))
    # and fill our namespace with it.
    globs.update(mod.__dict__)

__import_pywin32_system_module__("pywintypes", globals())
