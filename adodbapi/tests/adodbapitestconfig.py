# Configure this in order to run the testcases.
"adodbapitestconfig.py v 2.2.2"

import os
import sys
print sys.version
#attempt to find adodbapi in this directory's parent
cwd = os.getcwd()
adoPath = os.path.normpath(cwd + '/../adodbapi.py')
if os.path.exists(adoPath):
    if adoPath not in sys.path:
        sys.path.insert(1,os.path.dirname(adoPath))
import adodbapi
# adodbapitest.py will import this same version when we return

try:
    print adodbapi.version # show version
except:
    print '"adodbapi.version" not present or not working.'
print __doc__

doAccessTest = True
doSqlServerTest = True
try: #If mx extensions are installed, use mxDateTime
    import mx.DateTime
    doMxDateTimeTest=True
except: 
    doMxDateTimeTest=False #Requires eGenixMXExtensions
doAS400Test = False
doMySqlTest = False

import sys
if float(sys.version[:3])>2.29:
    doDateTimeTest=True #Requires Python 2.3 Alpha2
else:
    doDateTimeTest=False
    print 'Error: adodbapi 2.1 requires Python 2.3'
    
iterateOverTimeTests=False 


if doSqlServerTest:
    _computername="franklin" #or name of computer with SQL Server
    _databasename="Northwind" #or something else
    _username="guest"
    _password="12345678"
    #connStrSQLServer = r"Provider=SQLOLEDB.1; Integrated Security=SSPI; Initial Catalog=%s;Data Source=%s" %(_databasename, _computername)
    connStrSQLServer = r"Provider=SQLOLEDB.1; User ID=%s; Password=%s; Initial Catalog=%s;Data Source=%s" %(_username,_password,_databasename, _computername)
    print '    ...Testing MS-SQL login...'
    try:
        s = adodbapi.connect(connStrSQLServer) #connect to server
        s.close()
    except adodbapi.DatabaseError, inst:
        print inst.args[0][2]    # should be the error message
        doSqlServerTest = False

if doMySqlTest:
    _computername='10.100.5.249'
    _databasename='test'
   
    #_driver="MySQL ODBC 3.51 Driver"
    _driver="MySQL ODBC 5.1 Driver"
    connStrMySql = 'Driver={%s};Server=%s;Port=3306;Database=%s;Option=3;' % \
                   (_driver,_computername,_databasename)
    print '    ...Testing MySql login...'
    try:
        s = adodbapi.connect(connStrMySql) #connect to server
        s.close()
    except adodbapi.DatabaseError, inst:
        print inst.args[0][2]    # should be the error message
        doMySqlTest = False

if doAS400Test:
    #OLE DB -> "PROVIDER=IBMDA400; DATA SOURCE=MY_SYSTEM_NAME;USER ID=myUserName;PASSWORD=myPwd;DEFAULT COLLECTION=MY_LIBRARY;"
    connStrAS400skl = "Provider=IBMDA400; DATA SOURCE=%s;DEFAULT COLLECTION=%s;User ID=%s;Password=%s"
    # NOTE! user's PC must have OLE support installed in IBM Client Access Express
    _computername='SPICE'
    _databasename="DPVERNON"
    _username = 'DNTWCI'
      # or raw_input('  AS400 User ID for data retrieval [%s]:' % defaultUser)
    _password =  'DNTWCI'
      # or getpass.getpass('  AS400 password:') #read the password
    connStrAS400 = connStrAS400skl % (_computername,_databasename,_username,_password) #build the connection string
    print '    ...Testing AS400 login...'
    try:
        s = adodbapi.connect(connStrAS400) #connect to server
        s.close()
    except adodbapi.DatabaseError, inst:
        print inst.args[0][2]    # should be the AS400 error message
        doAS400Test = False
    