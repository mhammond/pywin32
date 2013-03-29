# Configure this in order to run the testcases.
"testADOdbapiConfig.py v 2.4.3"

import platform
import sys

import is64bit
import setuptestframework
if sys.version_info >= (3,0):
    import tryconnection3 as tryconnection
else:
    import tryconnection2 as tryconnection

print(sys.version)
node = platform.node()
print('node=%s: is64bit.os()= %s, is64bit.Python()= %s' % (node, is64bit.os(), is64bit.Python()))

testfolder = setuptestframework.maketemp()

if '--package' in sys.argv:
    pth = setuptestframework.makeadopackage(testfolder)
    if pth not in sys.path:
        sys.path.insert(1,pth)
# function to clean up the temporary folder -- calling program must run this function before exit.
cleanup = setuptestframework.getcleanupfunction()

import adodbapi
# testADOdbapi.py will import this same version when we return

try:
    print(adodbapi.version) # show version
except:
    print('"adodbapi.version" not present or not working.')
print(__doc__)

doAllTests = '--all' in sys.argv
doAccessTest = node != 'novelt2' or doAllTests
doSqlServerTest = node == 'novelt2' or doAllTests
doMySqlTest = '--mysql' in sys.argv or doAllTests
doPostgresTest =  '--pg' in sys.argv or doAllTests
iterateOverTimeTests = '--time' in sys.argv or doAllTests

try: #If mx extensions are installed, use mxDateTime
    import mx.DateTime
    doMxDateTimeTest=True
except: 
    doMxDateTimeTest=False #Requires eGenixMXExtensions

doTimeTest = True # obsolete python time format

if doAccessTest:
    c = {'dsn': setuptestframework.makemdb(testfolder)}
    if is64bit.Python():
        c['driver'] = "Microsoft.ACE.OLEDB.12.0"
    else:
        c['driver'] = "Microsoft.Jet.OLEDB.4.0"
    connStrAccess = "Provider=%(driver)s;Data Source=%(dsn)s" % c
    doAccessTest = tryconnection.try_connection(connStrAccess)
   
if doSqlServerTest:
    _computername = "192.168.100.101" # name of computer with SQL Server
    _databasename = "vernon" #or something else
    #_username="guest"
    #_password="12345678"
    #if is64bit:
    #    connStrSQLServer = "Provider=SQLNCLI11;Data Source=tcp:o9our2opzq.database.windows.net;Database=ADOtest;User ID=vernondcole@o9our2opzq;Password=Az0456xxx;Encrypt=True;"
    #else:
    #    connStrSQLServer = "Provider=SQLOLEDB.1; Data Source=tcp:o9our2opzq.database.windows.net;Database=ADOtest;User ID=vernondcole@o9our2opzq;Password=Az0456xxx;"
    connStrSQLServer = r"Provider=SQLOLEDB.1; Integrated Security=SSPI; Initial Catalog=%s;Data Source=%s" %(_databasename, _computername)
    #connStrSQLServer = r"Provider=SQLOLEDB.1; User ID=%s; Password=%s; Initial Catalog=%s;Data Source=%s" %(_username,_password,_databasename, _computername)
    print('    ...Testing MS-SQL login...')
    doSqlServerTest = tryconnection.try_connection(connStrSQLServer)

if doMySqlTest:
    # import socket
    _computername = "25.116.170.194" ## socket.gethostbyname('kf7xm.ham-radio-op.net')
    _databasename='test'
    _username = 'adotest'
    _password = '12345678'
    _provider = ''
    if is64bit.Python():
        _provider = 'Provider=MSDASQL;'
        _driver="MySQL ODBC 5.2a Driver"    # or _driver="MySQL ODBC 3.51 Driver"
    else:
        _driver="MySQL ODBC 5.2a Driver"
    connStrMySql = '%sDriver={%s};Server=%s;Port=3306;Database=%s;user=%s;password=%s;Option=3;' % \
                   (_provider,_driver,_computername,_databasename,_username,_password)
    print('    ...Testing MySql login...')
    doMySqlTest = tryconnection.try_connection(connStrMySql)

if doPostgresTest:
    _computername = "25.223.161.222"
    _databasename='adotest'
    _username = 'adotestuser'
    _password = '12345678'
    _driver="PostgreSQL Unicode"
    _provider = ''
    if is64bit.Python():
        _driver += '(x64)'
        _provider = 'Provider=MSDASQL;'

# get driver from http://www.postgresql.org/ftp/odbc/versions/
    connStrPostgres = '%sDriver={%s};Server=%s;Database=%s;uid=%s;pwd=%s;' % \
                   (_provider,_driver,_computername,_databasename,_username,_password)
    print('    ...Testing PostgreSQL login...')
    doPostgresTest = tryconnection.try_connection(connStrPostgres)

assert doAccessTest or doSqlServerTest or doMySqlTest or doPostgresTest, 'No database engine found for testing'
