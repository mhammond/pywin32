# postinstall script for pywin32
#
# copies PyWinTypesxx.dll and PythonCOMxx.dll into the system directory,
# and creates a pth file
import os, sys, glob, shutil
import _winreg

com_modules = [
    # module_name,                      class_names
    ("win32com.servers.interp",         "Interpreter"),
    ("win32com.servers.dictionary",     "DictionaryPolicy"),
]

# Is this a 'silent' install - ie, avoid all dialogs.
# Different than 'verbose'
silent = 0

# Verbosity of output messages.
verbose = 1

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

def AbortRetryIgnore(desc, func, *args):
    import win32api, win32con
    while 1:
        try:
            return func(*args)
        except:
            if silent:
                # Running silent mode - just re-raise the error.
                raise
            exc_type, exc_val, tb = sys.exc_info()
            tb = None
            full_desc = "Error %s\n\n" \
                        "If you have any Python applications running, " \
                        "please close them now\nand select 'Retry'\n\n%s" \
                        % (desc, exc_val)
            rc = win32api.MessageBox(0,
                                     full_desc,
                                     "Installation Error",
                                     win32con.MB_ABORTRETRYIGNORE)
            if rc == win32con.IDABORT:
                raise
            elif rc == win32con.IDIGNORE:
                return None
            # else retry - around we go again.

def SetPyKeyVal(key_name, value_name, value):
    ver_string = "%d.%d" % (sys.version_info[0], sys.version_info[1])
    root_key_name = "Software\\Python\\PythonCore\\" + ver_string
    try:
        root_key = _winreg.OpenKey(_winreg.HKEY_LOCAL_MACHINE, root_key_name)
    except OSError:
        root_key = _winreg.OpenKey(_winreg.HKEY_CURRENT_USER_MACHINE,
                                   root_key_name)
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
        klass = getattr(mod, klass_name)
        func(klass, **flags)

def install():
    import distutils.sysconfig
    lib_dir = distutils.sysconfig.get_python_lib(plat_specific=1)
    fname = os.path.join(sys.prefix, "pywin32.pth")
    if verbose:
        print "Creating .PTH file %s" % fname
    pthfile = open(fname, "w")
    # Register the file with the uninstaller
    file_created(fname)
    for name in "win32 win32\\lib Pythonwin".split():
        # Create entries for the PTH file, and at the same time
        # add the directory to sys.path so we can load win32api below.
        path = os.path.join(lib_dir, name)
        pthfile.write(path + "\n")
        sys.path.append(path)
    # To be able to import win32api, PyWinTypesxx.dll must be on the PATH
    # We must be careful to use the one we just installed, not one already
    # in the system directory, otherwise we will not be able to copy the one
    # just installed into the system dir.
    os.environ["PATH"] = "%s;%s" % (os.path.join(lib_dir, "pywin32_system32"), os.environ["PATH"])
    # importing pywintypes explicitly before win32api means our one in sys.path
    # is found, rather than whatever Windows implicitly finds as a side-effect
    # of importing win32api.
    import pywintypes
    import win32api
    # and now we can get the system directory:
    sysdir = win32api.GetSystemDirectory()
    # and copy some files over there
    files = glob.glob(os.path.join(lib_dir, "pywin32_system32\\*.*"))
    if not files:
        raise RuntimeError, "No system files to copy!!"
    for fname in files:
        base = os.path.basename(fname)
        dst = os.path.join(sysdir, base)
        if verbose:
            print "Copy %s to %s" % (base, sysdir)
        AbortRetryIgnore("installing %s" % base,
                         shutil.copyfile, fname, dst)
        # Register the files with the uninstaller
        file_created(dst)
    # Register our demo COM objects.
    RegisterCOMObjects()
    # Register the .chm help file.
    chm_file = os.path.join(lib_dir, "PyWin32.chm")
    if os.path.isfile(chm_file):
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
            # XXX CSIDL_COMMON_PROGRAMS only available works on NT/2000/XP:
            fldr = get_special_folder_path("CSIDL_COMMON_PROGRAMS")
            dst = os.path.join(fldr, "Python %d.%d\\PythonWin.lnk" % \
                               (sys.version_info[0], sys.version_info[1]))
            create_shortcut(os.path.join(lib_dir, "Pythonwin\\Pythonwin.exe"),
                            "The Pythonwin IDE",
                            dst)
            file_created(dst)
        except Exception, details:
            print details
        else:
            print "Shortcut for Pythonwin created"

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
            break # we do nothing for now
        else:
            print "Unknown option:", arg
            usage()
            sys.exit(0)
        arg_index += 1
