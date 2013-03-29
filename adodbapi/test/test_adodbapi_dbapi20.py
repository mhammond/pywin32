print("This module depends on the dbapi20 compliance tests created by Stuart Bishop")
print("(see db-sig mailing list history for info)")
import platform
import unittest
import sys

import dbapi20
import is64bit
import setuptestframework

testfolder = setuptestframework.maketemp()
pth = setuptestframework.makeadopackage(testfolder)
if pth not in sys.path:
    sys.path.insert(1,pth)
# function to clean up the temporary folder -- calling program must run this function before exit.
cleanup = setuptestframework.getcleanupfunction()

import adodbapi

print(adodbapi.version)
print("Tested with dbapi20 %s" % dbapi20.__version__)


node = platform.node()
if node == "novelt2":
    _computername = '127.0.0.1'
    _databasename = 'vernon'
    connStr = "Provider=SQLOLEDB.1; Integrated Security=SSPI; Initial Catalog=%s;Data Source=%s" %(_databasename, _computername)
elif node == "z-PC":
    _computername = "25.223.161.222"
    _databasename='adotest'
    _username = 'adotestuser'
    _password = '12345678'
    _driver="PostgreSQL Unicode"
    _provider = ''
    if is64bit.Python():
        _driver += '(x64)'
        _provider = 'Provider=MSDASQL;'
    connStr = '%sDriver={%s};Server=%s;Database=%s;uid=%s;pwd=%s;' % \
                  (_provider,_driver,_computername,_databasename,_username,_password)
else:  # ACCESS data base is known to fail some tests.
    testmdb = setuptestframework.makemdb(testfolder)
    connStr = r"Provider=Microsoft.Jet.OLEDB.4.0;Data Source=" + testmdb
print('Using Connection String='+connStr)

class test_adodbapi(dbapi20.DatabaseAPI20Test):
    driver = adodbapi
    connect_args =  (connStr,)
    connect_kw_args = {}

    def __init__(self,arg):
        dbapi20.DatabaseAPI20Test.__init__(self,arg)

    def testMethodName(self):
        return self.id().split('.')[-1]

    def setUp(self):
        # Call superclass setUp In case this does something in the
        # future
        dbapi20.DatabaseAPI20Test.setUp(self)
        con = self._connect()
        con.close()
        if self.testMethodName()=='test_callproc':
            sql="""
                create procedure templower
                    @theData varchar(50)
                as
                    select lower(@theData)
            """
            con = self._connect()       
            cur = con.cursor()
            try:
                cur.execute(sql)
                con.commit()
            except:
                pass
            self.lower_func='templower'


    def tearDown(self):
        if self.testMethodName()=='test_callproc':
            con = self._connect()
            cur = con.cursor()
            cur.execute("drop procedure templower")
            con.commit()
        dbapi20.DatabaseAPI20Test.tearDown(self)
        

    def help_nextset_setUp(self,cur):
        'Should create a procedure called deleteme '
        'that returns two result sets, first the number of rows in booze then "name from booze"'
        sql="""
            create procedure deleteme as
            begin
                select count(*) from %sbooze
                select name from %sbooze
            end
        """ %(self.table_prefix,self.table_prefix)
        cur.execute(sql)

    def help_nextset_tearDown(self,cur):
        'If cleaning up is needed after nextSetTest'
        try:
            cur.execute("drop procedure deleteme")
        except:
            pass

    def test_nextset(self):
        con = self._connect()
        try:            
            cur = con.cursor()

            stmts=[self.ddl1] + self._populate()
            for sql in stmts:
                cur.execute(sql)

            self.help_nextset_setUp(cur)

            cur.callproc('deleteme')
            numberofrows=cur.fetchone()
            assert numberofrows[0]== 6
            assert cur.nextset()
            names=cur.fetchall()
            assert len(names) == len(self.samples)
            s=cur.nextset()
            assert s == None,'No more return sets, should return None'           
        finally:
            try:
                self.help_nextset_tearDown(cur)
            finally:
                con.close()
        
    def test_setoutputsize(self): pass

if __name__ == '__main__':
    unittest.main()
    cleanup(testfolder)
