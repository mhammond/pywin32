# postinstall script for pywin32
#
# copies PyWinTypesxx.dll and PythonCOMxx.dll into the system directory,
# and creates a pth file
import os, sys, glob, shutil, time
import _winreg

com_modules = [
    # module_name,                      class_names
    ("win32com.servers.interp",         "Interpreter"),
    ("win32com.servers.dictionary",     "DictionaryPolicy"),
    ("win32com.axscript.client.pyscript","PyScript"),
]

# Is this a 'silent' install - ie, avoid all dialogs.
# Different than 'verbose'
silent = 0

# Verbosity of output messages.
verbose = 1

ver_string = "%d.%d" % (sys.version_info[0], sys.version_info[1])
root_key_name = "Software\\Python\\PythonCore\\" + ver_string

try:
    # When this script is run from inside the bdist_wininst installer,
    # file_created() and directory_created() are additional builtin
    # functions which write lines to Python23\pywin32-install.log. This is
    # a list of actions for the uninstaller, the format is inspired by what
    # the Wise installer also creates.
    file_created
except NameError:
    def file_created(file):
        pass
    def directory_created(directory):
        pass
    def get_root_hkey():
        try:
            _winreg.OpenKey(_winreg.HKEY_LOCAL_MACHINE,
                            root_key_name, _winreg.KEY_CREATE_SUB_KEY)
            return _winreg.HKEY_LOCAL_MACHINE
        except OSError, details:
            # Either not exist, or no permissions to create subkey means
            # must be HKCU
            return _winreg.HKEY_CURRENT_USER

def CopyTo(desc, src, dest):
    import win32api, win32con
    while 1:
        try:
            win32api.CopyFile(src, dest, 0)
            return
        except win32api.error, details:
            if details[0]==5: # access denied - user not admin.
                raise
            if silent:
                # Running silent mode - just re-raise the error.
                raise
            err_msg = details[2]
            tb = None
            full_desc = "Error %s\n\n" \
                        "If you have any Python applications running, " \
                        "please close them now\nand select 'Retry'\n\n%s" \
                        % (desc, err_msg)
            rc = win32api.MessageBox(0,
                                     full_desc,
                                     "Installation Error",
                                     win32con.MB_ABORTRETRYIGNORE)
            if rc == win32con.IDABORT:
                raise
            elif rc == win32con.IDIGNORE:
                return
            # else retry - around we go again.

# We need to import win32api to determine the Windows system directory,
# so we can copy our system files there - but importing win32api will
# load the pywintypes.dll already in the system directory preventing us
# from updating them!
# So, we pull the same trick pywintypes.py does, but it loads from
# our pywintypes_system32 directory.
def LoadSystemModule(lib_dir, modname):
    # See if this is a debug build.
    import imp
    for suffix_item in imp.get_suffixes():
        if suffix_item[0]=='_d.pyd':
            suffix = '_d'
            break
    else:
        suffix = ""
    filename = "%s%d%d%s.dll" % \
               (modname, sys.version_info[0], sys.version_info[1], suffix)
    filename = os.path.join(lib_dir, "pywin32_system32", filename)
    mod = imp.load_module(modname, None, filename, 
                          ('.dll', 'rb', imp.C_EXTENSION))


def SetPyKeyVal(key_name, value_name, value):
    root_hkey = get_root_hkey()
    root_key = _winreg.OpenKey(root_hkey, root_key_name)
    try:
        my_key = _winreg.CreateKey(root_key, key_name)
        try:
            _winreg.SetValueEx(my_key, value_name, 0, _winreg.REG_SZ, value)
        finally:
            my_key.Close()
    finally:
        root_key.Close()
    if verbose:
        print "-> %s\\%s[%s]=%r" % (root_key_name, key_name, value_name, value)

def RegisterCOMObjects(register = 1):
    import win32com.server.register
    if register:
        func = win32com.server.register.RegisterClasses
    else:
        func = win32com.server.register.UnregisterClasses
    flags = {}
    if not verbose:
        flags['quiet']=1
    for module, klass_name in com_modules:
        __import__(module)
        mod = sys.modules[module]
        flags["finalize_register"] = getattr(mod, "DllRegisterServer", None)
        flags["finalize_unregister"] = getattr(mod, "DllUnregisterServer", None)
        klass = getattr(mod, klass_name)
        func(klass, **flags)

def install():
    import distutils.sysconfig
    import traceback
    # Create the .pth file in the site-packages dir, and use only relative paths
    lib_dir = distutils.sysconfig.get_python_lib(plat_specific=1)
    # Used to write this directly to sys.prefix - clobber it.
    if os.path.isfile(os.path.join(sys.prefix, "pywin32.pth")):
        os.unlink(os.path.join(sys.prefix, "pywin32.pth"))
    fname = os.path.join(lib_dir, "pywin32.pth")
    if verbose:
        print "Creating .PTH file %s" % fname
    pthfile = open(fname, "w")
    # Register the file with the uninstaller
    file_created(fname)
    for name in "win32 win32\\lib Pythonwin".split():
        # Create entries for the PTH file, and at the same time
        # add the directory to sys.path so we can load win32api below.
        pthfile.write(name + "\n")
        sys.path.append(os.path.join(lib_dir, name))
    # It is possible people with old versions installed with still have 
    # pywintypes and pythoncom registered.  We no longer need this, and stale
    # entries hurt us.
    for name in "pythoncom pywintypes".split():
        keyname = "Software\\Python\\PythonCore\\" + sys.winver + "\\Modules\\" + name
        for root in _winreg.HKEY_LOCAL_MACHINE, _winreg.HKEY_CURRENT_USER:
            try:
                _winreg.DeleteKey(root, keyname + "\\Debug")
            except WindowsError:
                pass
            try:
                _winreg.DeleteKey(root, keyname)
            except WindowsError:
                pass
    LoadSystemModule(lib_dir, "pywintypes")
    LoadSystemModule(lib_dir, "pythoncom")
    import win32api
    # and now we can get the system directory:
    files = glob.glob(os.path.join(lib_dir, "pywin32_system32\\*.*"))
    if not files:
        raise RuntimeError, "No system files to copy!!"
    # Try the system32 directory first - if that fails due to "access denied",
    # it implies a non-admin user, and we use sys.prefix
    for dest_dir in [win32api.GetSystemDirectory(), sys.prefix]:
        # and copy some files over there
        worked = 0
        try:
            for fname in files:
                base = os.path.basename(fname)
                dst = os.path.join(dest_dir, base)
                CopyTo("installing %s" % base, fname, dst)
                if verbose:
                    print "Copied %s to %s" % (base, dst)
                # Register the files with the uninstaller
                file_created(dst)
                worked = 1
                # If this isn't sys.prefix (ie, System32), then nuke 
                # any versions that may exist in sys.prefix - having
                # duplicates causes major headaches.
                if dest_dir != sys.prefix:
                    bad_fname = os.path.join(sys.prefix, base)
                    if os.path.exists(bad_fname):
                        # let exceptions go here - delete must succeed
                        os.unlink(bad_fname)
            if worked:
                break
        except win32api.error, details:
            if details[0]==5:
                # access denied - user not admin - try sys.prefix dir,
                # but first check that a version doesn't already exist
                # in that place - otherwise that one will still get used!
                if os.path.exists(dst):
                    msg = "The file '%s' exists, but can not be replaced " \
                          "due to insufficient permissions.  You must " \
                          "reinstall this software as an Administrator" \
                          % dst
                    print msg
                    raise RuntimeError, msg
                continue
            raise
    else:
        raise RuntimeError, \
              "You don't have enough permissions to install the system files"

    # Pythonwin 'compiles' config files - record them for uninstall.
    pywin_dir = os.path.join(lib_dir, "Pythonwin", "pywin")
    for fname in glob.glob(os.path.join(pywin_dir, "*.cfg")):
        file_created(fname[:-1] + "c") # .cfg->.cfc

    # Register our demo COM objects.
    try:
        try:
            RegisterCOMObjects()
        except win32api.error, details:
            if details[0]!=5: # ERROR_ACCESS_DENIED
                raise
            print "You do not have the permissions to install COM objects."
            print "The sample COM objects were not registered."
    except:
        print "FAILED to register the Python COM objects"
        traceback.print_exc()

    # There may be no main Python key in HKCU if, eg, an admin installed
    # python itself.
    _winreg.CreateKey(get_root_hkey(), root_key_name)

    # Register the .chm help file.
    chm_file = os.path.join(lib_dir, "PyWin32.chm")
    if os.path.isfile(chm_file):
        # This isn't recursive, so if 'Help' doesn't exist, we croak
        SetPyKeyVal("Help", None, None)
        SetPyKeyVal("Help\\Pythonwin Reference", None, chm_file)
    else:
        print "NOTE: PyWin32.chm can not be located, so has not " \
              "been registered"
    # Create the win32com\gen_py directory.
    make_dir = os.path.join(lib_dir, "win32com", "gen_py")
    if not os.path.isdir(make_dir):
        if verbose:
            print "Creating directory", make_dir
        directory_created(make_dir)
        os.mkdir(make_dir)

    try:
        create_shortcut
    except NameError:
        # todo: create shortcut with win32all
        pass
    else:
        try:
            # use bdist_wininst builtins to create a shortcut.
            # CSIDL_COMMON_PROGRAMS only available works on NT/2000/XP, and
            # will fail there if the user has no admin rights.
            if get_root_hkey()==_winreg.HKEY_LOCAL_MACHINE:
                try:
                    fldr = get_special_folder_path("CSIDL_COMMON_PROGRAMS")
                except OSError:
                    # No CSIDL_COMMON_PROGRAMS on this platform
                    fldr = get_special_folder_path("CSIDL_PROGRAMS")
            else:
                # non-admin install - always goes in this user's start menu.
                fldr = get_special_folder_path("CSIDL_PROGRAMS")

            try:
                install_group = _winreg.QueryValue(get_root_hkey(),
                                                   root_key_name + "\\InstallPath\\InstallGroup")
            except OSError:
                vi = sys.version_info
                install_group = "Python %d.%d" % (vi[0], vi[1])
            fldr = os.path.join(fldr, install_group)
            if not os.path.isdir(fldr):
                os.mkdir(fldr)

            dst = os.path.join(fldr, "PythonWin.lnk")
            create_shortcut(os.path.join(lib_dir, "Pythonwin\\Pythonwin.exe"),
                            "The Pythonwin IDE", dst, "", sys.prefix)
            file_created(dst)
            if verbose:
                print "Shortcut for Pythonwin created"
            # And the docs.
            dst = os.path.join(fldr, "Python for Windows Documentation.lnk")
            doc = "Documentation for the PyWin32 extensions"
            create_shortcut(chm_file, doc, dst)
            file_created(dst)
            if verbose:
                print "Shortcut to documentation created"
        except Exception, details:
            if verbose:
                print details

    # Check the MFC dll exists - it is doesn't, point them at it
    # (I should install it, but its a bit tricky with distutils)
    # Unfortunately, this is quite likely on Windows XP and MFC71.dll
    if sys.hexversion < 0x2040000:
        mfc_dll = "mfc42.dll"
    else:
        mfc_dll = "mfc71.dll"
    try:
        win32api.SearchPath(None, mfc_dll)
    except win32api.error:
        print "*" * 20, "WARNING", "*" * 20
        print "It appears that the MFC DLL '%s' is not installed" % (mfc_dll,)
        print "Pythonwin will not work without this DLL, and I haven't had the"
        print "time to package it in with the installer."
        print
        print "You can download this DLL from:"
        print "http://starship.python.net/crew/mhammond/win32/"
        print "*" * 50

    print "The pywin32 extensions were successfully installed."

def usage():
    msg = \
"""%s: A post-install script for the pywin32 extensions.
    
This should be run automatically after installation, but if it fails you
can run it again with a '-install' parameter, to ensure the environment
is setup correctly.

Additional Options:
  -wait pid : Wait for the specified process to terminate before starting.
  -silent   : Don't display the "Abort/Retry/Ignore" dialog for files in use.
  -quiet    : Don't display progress messages.
"""
    print msg.strip() % os.path.basename(sys.argv[0])

# NOTE: If this script is run from inside the bdist_wininst created
# binary installer or uninstaller, the command line args are either
# '-install' or '-remove'.

# Important: From inside the binary installer this script MUST NOT
# call sys.exit() or raise SystemExit, otherwise not only this script
# but also the installer will terminate! (Is there a way to prevent
# this from the bdist_wininst C code?)

if __name__=='__main__':
    if len(sys.argv)==1:
        usage()
        sys.exit(1)

    arg_index = 1
    while arg_index < len(sys.argv):
        arg = sys.argv[arg_index]
        # Hack for installing while we are in use.  Just a simple wait so the
        # parent process can terminate.
        if arg == "-wait":
            arg_index += 1
            pid = int(sys.argv[arg_index])
            try:
                os.waitpid(pid, 0)
            except AttributeError:
                # Python 2.2 - no waitpid - just sleep.
                time.sleep(3)
            except os.error:
                # child already dead
                pass
        elif arg == "-install":
            install()
        elif arg == "-silent":
            silent = 1
        elif arg == "-quiet":
            verbose = 0
        elif arg == "-remove":
            # Nothing to do here - we can't unregister much, as we have
            # already been uninstalled.
            pass
        else:
            print "Unknown option:", arg
            usage()
            sys.exit(0)
        arg_index += 1
