""" db_print.py -- a simple demo for ADO database reads."""

import adodbapi
adodbapi.adodbapi.verbose = True # adds details to the sample printout
import adodbapi.ado_consts as adc
#tell the server  we are not planning to update...
adodbapi.adodbapi.defaultIsolationLevel = adc.adXactBrowse
#----------------- Create connection string -----------------------------------
# connection string templates from http://www.connectionstrings.com
# Switch test providers by changing the "if True" below

# connection string for an Access data table:
if True:
    _databasename = "Test.mdb"
    # generic -> 'Provider=Microsoft.Jet.OLEDB.4.0;Data Source=%s; User Id=%s; Password=%s;' % (_databasename, _username, _password)
    constr = 'Provider=Microsoft.Jet.OLEDB.4.0;Data Source=%s' \
             % _databasename
    _table_name= 'Products'
#----------
else:
# connection string for an SQL server
    _computername="127.0.0.1" #or name of computer with SQL Server
    _databasename="Northwind" #or something else
    _table_name= 'Products'

    if True:    
        # this will open a MS-SQL table with Windows authentication
        constr = r"Initial Catalog=%s; Data Source=%s; Provider=SQLOLEDB.1; Integrated Security=SSPI" \
                 %(_databasename, _computername)
    else:
        _username="guest"
        _password='12345678'
        # this set opens a MS-SQL table with SQL authentication 
        constr = r"Provider=SQLOLEDB.1; Initial Catalog=%s; Data Source=%s; user ID=%s; Password=%s; " \
             % (_databasename, _computername, _username, _password)
#-----------------------
# connection string for MySQL
if False:
    # this will open a MySQL table (assuming you have the ODBC driver from MySQL.org
    _computername = 'star.vernonscomputershop.com'
    _databasename = 'test'
    constr = 'Driver={MySQL ODBC 3.51 Driver};Server=%s;Port=3306;Database=%s;Option=3;' \
        % (_computername,_databasename)
    _table_name= 'Test_tbl'
#-----------    
# connection string for AS400
if False:
    constr = "Provider=IBMDA400; DATA SOURCE=%s;DEFAULT COLLECTION=%s;User ID=%s;Password=%s" \
                      %  (_computername, _databasename, _username, _password)
    # NOTE! user's PC must have OLE support installed in IBM Client Access Express
#----------------------------------

# ------------------------ START HERE -------------------------------------
#create the connection
con = adodbapi.connect(constr)

#make a cursor on the connection
c = con.cursor()

#run an SQL statement on the cursor
sql = 'select * from %s' % _table_name
c.execute(sql)

#check the results
print 'result rowcount shows as= %d. (Note: -1 means "not known")' \
      % (c.rowcount,)
print
print 'result data description is:'
print '            NAME Type         DispSize IntrnlSz Prec Scale Null?'
for d in c.description:
    print ('%16s %-12s %8d %8d %4d %5d %s') % \
          (d[0], adc.adTypeNames[d[1]], d[2],   d[3],  d[4],d[5], bool(d[6]))
print
print 'str() of first five records are...' 

#get the results
db = c.fetchmany(5)

#print them
for rec in db:
    print rec

print
print 'repr() of next row is...'
print repr(c.next())
print

c.close()
con.close()
