# postinstall script for pywin32
#
# copies PyWinTypesxx.dll and PythonCOMxx.dll into the system directory,
# and creates a pth file
import os, sys, glob, shutil

if sys.argv[1] == "-install":
    import distutils.sysconfig
    lib_dir = distutils.sysconfig.get_python_lib(plat_specific=1)
    fname = os.path.join(sys.prefix, "pywin32.pth")
    print "Creating PTH FILE %s" % fname
    pthfile = open(fname, "w")
    # Register the file with the uninstaller
    file_created(fname)
    for name in "win32 win32com Pythonwin".split():
        # Create entries for the PTH file, and at the same time
        # add the directory to sys.path so we can load win32api below.
        path = os.path.join(lib_dir, name)
        pthfile.write(path + "\n")
        sys.path.append(path)
    # To be able to import win32api, PyWinTypesxx.dll must be on the PATH
    # We must be carefull to use the one we just installed, not one already
    # in the system directory, otherwise we will not be able to copy the one
    # just installed into the system dir.
    os.environ["PATH"] = "%s;%s" % (os.path.join(lib_dir, "system32"), os.environ["PATH"])
    import win32api
    # and now we can get the system directory:
    sysdir = win32api.GetSystemDirectory()
    # and copy some files over there
    for fname in glob.glob(os.path.join(lib_dir, "system32\\*.*")):
        base = os.path.basename(fname)
        dst = os.path.join(sysdir, base)
        print "Copy %s to %s" % (base, sysdir)
        shutil.copyfile(fname, dst)
        # Register the files with the uninstaller
        file_created(dst)
        
