import sys, _winreg, os

versions = {
    "2.1" : (r"..\python-2.1","Python21"),
    "2.2" : (r"..\python-2.2-cvs", "Python22"),
    "2.3" : (r"..\python-cvs", None),
}

path_infos = (
    ("Include Dirs", ("pc", "include")),
    ("Library Dirs", ("pcbuild",)),
)

def SetDevStudioVersion(version, verbose):
    root = versions[version][0]
    for value_name, look_dirs in path_infos:
        key = _winreg.OpenKeyEx(_winreg.HKEY_CURRENT_USER, r"Software\Microsoft\Devstudio\6.0\Build System\Components\Platforms\Win32 (x86)\Directories", 0, _winreg.KEY_ALL_ACCESS)
        val, typ = _winreg.QueryValueEx(key, value_name)
        existing_dirs = val.split(";")
        for look_dir in look_dirs:
            this_look = os.path.abspath(os.path.join(root, look_dir))
            for i in range(len(existing_dirs)):
                dir = existing_dirs[i].lower()
                for pos_dir, cvs in versions.values():
                    pos_dir = os.path.abspath(pos_dir).lower()
                    this_pos = os.path.join(root, pos_dir, look_dir).lower()
                    if dir == this_pos:
                        existing_dirs[i] = this_look
        new_val = ";".join(existing_dirs)
        if val.lower() == new_val.lower():
            print "No change detected for '%s'" % (value_name,)
        else:
            _winreg.SetValueEx(key, value_name, 0, _winreg.REG_SZ, new_val)
        if verbose:
            print "%s:" % (value_name,)
            for p in new_val.split(";"):
                print "", p

def PullCVS(version):
    files = "com/win32com.dsp", "win32/PyWinTypes.dsp"
    tag = versions[version][1]
    if tag is None:
        flag = "-A"
    else:
        flag = "-r " + tag
    for file in files:
        if not os.path.isfile(file):
            raise RuntimeError("Can't find file I need to update: %s" % (file,))
        cmd = "cvs -z5 update " + flag + " " + file
        print cmd
        rc = os.system(cmd)
        if rc != 0:
            return False
    return True

def main(version, verbose = False):
    SetDevStudioVersion(version, verbose)
    PullCVS(version)

if __name__=='__main__':
    main(sys.argv[1], True)
