# Magic utility that "redirects" to pywintypesxx.dll

def __import_pywin32_system_module__(modname, globs):
    # *sigh* - non-admin installs will not have pywintypesxx.dll in the 
    # system directory, so 'import win32api' will fail looking
    # for pywintypes - the exact DLL we are trying to load!
    # So if it exists in sys.prefix, then we try and load it from
    # there, as that way we can avoid the win32api import
    import imp, sys, os
    if not sys.platform.startswith("win32"):
        # These extensions can be built on Linux via the 'mainwin' toolkit.
        # Look for a native 'lib{modname}.so'
        for ext, mode, ext_type in imp.get_suffixes():
            if ext_type==imp.C_EXTENSION:
                for path in sys.path:
                    look = os.path.join(path, "lib" + modname + ext)
                    if os.path.isfile(look):
                        mod = imp.load_module(modname, None, look,
                                              (ext, mode, ext_type))
                        # and fill our namespace with it.
                        globs.update(mod.__dict__)
                        return
        raise ImportError, "No dynamic module " + modname
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
                  "Module '%s' isn't in frozen sys.path %s" % (modname, sys.path)
    else:
        # If there is a version in our Python directory, use that
        # (it may not be on the PATH, so may fail to be loaded by win32api)
        # Non-admin installs will have the system files there.
        found = None
        if os.path.isfile(os.path.join(sys.prefix, filename)):
            found = os.path.join(sys.prefix, filename)
        # Allow Windows to find it.  We have tried various other tricks,
        # but in some cases, we ended up with *2* versions of the libraries
        # loaded - the one found by Windows when doing a later "import win32*",
        # and the one we found here.
        # A remaining trick would be to simulate LoadLibrary(), using the
        # registry to determine the system32 directory.  However, Python
        # 2.2 doesn't have sys.getwindowsversion(), which is kinda needed
        # to determine the correct places to look.

        # The downside of this is that we need to use win32api, and this
        # depends on pywintypesxx.dll, which may be what we are trying to
        # find!  If this fails, we get a dialog box, followed by the import
        # error.  The dialog box is undesirable, but should only happen
        # when something is badly broken, and is a less harmful side-effect
        # than loading the DLL twice!
        if found is None:
            import win32api # failure here means Windows can't find it either!
            found = win32api.GetModuleFileName(win32api.LoadLibrary(filename))

    # Python can load the module
    mod = imp.load_module(modname, None, found, 
                          ('.dll', 'rb', imp.C_EXTENSION))
    # and fill our namespace with it.
    globs.update(mod.__dict__)

__import_pywin32_system_module__("pywintypes", globals())
