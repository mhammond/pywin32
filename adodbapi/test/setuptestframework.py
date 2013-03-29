#!/usr/bin/python2 Configure this in order to run the testcases.
"setuptestframework.py v 2.4.3"

import os
import sys
import tempfile
import shutil

def maketemp():
    tempdir = tempfile.mkdtemp(prefix='adotest_')
    return tempdir

def _cleanup_function(testfolder):
    shutil.rmtree(testfolder)

def getcleanupfunction():
    return _cleanup_function

# make a new package directory for the test copy of ado
def makeadopackage(testfolder):
    adoName = os.path.normpath(os.getcwd() + '/../adodbapi.py')
    adoPath = os.path.dirname(adoName)
    if os.path.exists(adoName):
        newpackage = os.path.join(testfolder,'adodbapi')
        os.mkdir(newpackage)
        for f in os.listdir(adoPath):
            if f.endswith('.py'):
                shutil.copy(os.path.join(adoPath, f), newpackage)
        if sys.version_info >= (3,0): # only when running Py3.n
            save = sys.stdout
            sys.stdout = None
            from lib2to3.main import main  # use 2to3 to make test package
            main("lib2to3.fixes",args=['-n','-w', newpackage])
            sys.stdout = save
        return testfolder
    else:
        raise EnvironmentError('Connot find source of adodbapi to test.')

def makemdb(testfolder):
    # following setup code borrowed from pywin32 odbc test suite
    # kindly contributed by Frank Millman.
    import tempfile
    import os

    _accessdatasource = tempfile.mktemp(suffix='.mdb', prefix='ado_test_', dir=testfolder)
    if os.path.isfile(_accessdatasource):
        os.unlink(_accessdatasource)
    try:
        from win32com.client.gencache import EnsureDispatch
        from win32com.client import constants
        win32 = True
    except ImportError: #perhaps we are running IronPython
        win32 = False #iron Python
        from System import Activator, Type

    # Create a brand-new database - what is the story with these?
    dbe = None
    for suffix in (".36", ".35", ".30"):
        try:
            if win32:
                dbe = EnsureDispatch("DAO.DBEngine" + suffix)
            else:
                type= Type.GetTypeFromProgID("DAO.DBEngine" + suffix)
                dbe =  Activator.CreateInstance(type)
            break
        except:
            pass
    if dbe:
        print(('    ...Creating ACCESS db at '+_accessdatasource))
        if win32:
            workspace = dbe.Workspaces(0)
            newdb = workspace.CreateDatabase(_accessdatasource, 
                                            constants.dbLangGeneral,
                                            constants.dbEncrypt)
        else:
            newdb = dbe.CreateDatabase(_accessdatasource,';LANGID=0x0409;CP=1252;COUNTRY=0')
        newdb.Close()
    else:
        print(('    ...copying test ACCESS db to '+_accessdatasource))
        mdbName = os.path.normpath(os.getcwd() + '/../examples/test.mdb')
        import shutil
        shutil.copy(mdbName, _accessdatasource)

    return  _accessdatasource


