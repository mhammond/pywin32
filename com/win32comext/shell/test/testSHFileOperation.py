from win32com.shell import shell, shellcon
import win32api
import os

def testSHFileOperation(file_cnt):
    temp_dir=os.environ['temp']
    orig_fnames=[win32api.GetTempFileName(temp_dir,'sfo')[0] for x in range(file_cnt)]
    new_fnames=[os.path.join(temp_dir,'copy of '+os.path.split(orig_fnames[x])[1]) for x in range(file_cnt)]

    pFrom='\0'.join(orig_fnames)
    pTo='\0'.join(new_fnames)

    shell.SHFileOperation((0, shellcon.FO_MOVE, pFrom, pTo, shellcon.FOF_MULTIDESTFILES|shellcon.FOF_NOCONFIRMATION))
    for fname in orig_fnames:
        assert not os.path.isfile(fname)
        
    for fname in new_fnames:
        assert os.path.isfile(fname)
        shell.SHFileOperation((0, shellcon.FO_DELETE, fname, None, shellcon.FOF_NOCONFIRMATION|shellcon.FOF_NOERRORUI))


testSHFileOperation(10)
testSHFileOperation(1)