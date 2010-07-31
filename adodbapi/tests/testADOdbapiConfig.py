# Configure this in order to run the testcases.
"testADOdbapiConfig.py v 2.4.0"

import os
import sys
print(sys.version)
#attempt to use the copy of adodbapi in this directory's parent
if sys.version[0] < '3': # only when running Py2.n
    cwd = os.getcwd()
    if 'P3k' not in cwd: # and not in the Python 3 test folder
        adoPath = os.path.normpath(cwd + '/../adodbapi.py')
        if os.path.exists(adoPath):
            if adoPath not in sys.path:
                sys.path.insert(1,os.path.dirname(adoPath))
import adodbapi
# testADOdbapi.py will import this same version when we return

try:
    print adodbapi.version # show version
except:
    print '"adodbapi.version" not present or not working.'
print __doc__

doAllTests = True
doAccessTest = True or doAllTests
doSqlServerTest = False or doAllTests
doMySqlTest = False or doAllTests
doPostgresTest = False # or doAllTests

try: #If mx extensions are installed, use mxDateTime
    import mx.DateTime
    doMxDateTimeTest=True
except: 
    doMxDateTimeTest=False #Requires eGenixMXExtensions
doDateTimeTest=True #Requires Python 2.3 Alpha2
    
iterateOverTimeTests = False or doAllTests

if doAccessTest:
    _accessdatasource = "test.mdb"  #set to None for automatic creation
    if not os.access(_accessdatasource,os.F_OK):
        _accessdatasource = None
    if _accessdatasource == None:
        # following setup code borrowed from pywin32 odbc test suite
        # kindly contributed by Frank Millman.
        import tempfile
        import os
        try:
            from win32com.client.gencache import EnsureDispatch
            from win32com.client import constants
            win32 = True
        except ImportError: #perhaps we are running IronPython
            win32 = False
        if not win32: #iron Python
            from System import Activator, Type
        _accessdatasource = os.path.join(tempfile.gettempdir(), "test_odbc.mdb")
        if os.path.isfile(_accessdatasource):
            os.unlink(_accessdatasource)
        # Create a brand-new database - what is the story with these?
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
        else:
            raise RuntimeError("Can't find a DB engine")
        print '    ...Creating ACCESS db at',_accessdatasource    
        if win32:
            workspace = dbe.Workspaces(0)
            newdb = workspace.CreateDatabase(_accessdatasource, 
                                            constants.dbLangGeneral,
                                            constants.dbEncrypt)
        else:
            newdb = dbe.CreateDatabase(_accessdatasource,';LANGID=0x0409;CP=1252;COUNTRY=0')
        newdb.Close()
    connStrAccess = r"Provider=Microsoft.Jet.OLEDB.4.0;Data Source=" + _accessdatasource
    # for ODBC connection try...
    # connStrAccess = "Driver={Microsoft Access Driver (*.mdb)};db=%s;Uid=;Pwd=;" + _accessdatasource

if doSqlServerTest:
    _computername=r".\SQLEXPRESS" #or name of computer with SQL Server
    _databasename="Northwind" #or something else
    #_username="guest"
    #_password="12345678"
    connStrSQLServer = r"Provider=SQLOLEDB.1; Integrated Security=SSPI; Initial Catalog=%s;Data Source=%s" %(_databasename, _computername)
    #connStrSQLServer = r"Provider=SQLOLEDB.1; User ID=%s; Password=%s; Initial Catalog=%s;Data Source=%s" %(_username,_password,_databasename, _computername)
    print '    ...Testing MS-SQL login...'
    try:
        s = adodbapi.connect(connStrSQLServer) #connect to server
        s.close()
    except adodbapi.DatabaseError, inst:
        print inst.args[0]    # should be the error message
        doSqlServerTest = False

if doMySqlTest:
    import socket
    try:
        _computername = socket.gethostbyname('kf7xm.ham-radio-op.net')
    except:
        _computername = '127.0.0.1'
    _databasename='test'
    _username = 'Test'
    _password = '12345678'
    _driver="MySQL ODBC 5.1 Driver"     # or _driver="MySQL ODBC 3.51 Driver"
    connStrMySql = 'Driver={%s};Server=%s;Port=3306;Database=%s;user=%s;password=%s;Option=3;' % \
                   (_driver,_computername,_databasename,_username,_password)
    print '    ...Testing MySql login...'
    try:
        s = adodbapi.connect(connStrMySql) #connect to server
        s.close()
    except adodbapi.DatabaseError,  inst:
        print inst.args[0]    # should be the error message
        doMySqlTest = False

if doPostgresTest:
    import socket
    try:
        _computername = socket.gethostbyname('kf7xm.ham-radio-op.net')
    except:
        _computername = '127.0.0.1'
    _databasename='test'
    _username = 'Test'
    _password = '12345678'
    _driver="PostgreSQL Unicode"
    connStrPostgres = 'Driver={%s};Server=%s;Database=%s;user=%s;password=%s;' % \
                   (_driver,_computername,_databasename,_username,_password)
    print '    ...Testing PostgreSQL login...'
    try:
        s = adodbapi.connect(connStrPostgres) #connect to server
        s.close()
    except adodbapi.DatabaseError,  inst:
        print inst.args[0]    # should be the error message
        doPostgresTest = False
