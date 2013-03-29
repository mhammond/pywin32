""" db_print.py -- a simple demo for ADO database reads."""
import adodbapi
import adodbapi.ado_consts as adc
import adodbapi.is64bit as is64bit

databasename = "Test.mdb"

if is64bit.Python():
    driver = "Microsoft.ACE.OLEDB.12.0"
else:
    driver = "Microsoft.Jet.OLEDB.4.0"
constr = "Provider=%s;Data Source=%s" % (driver, databasename)

table_name= 'Products'

# ------------------------ START HERE -------------------------------------
#create the connection
con = adodbapi.connect(constr)

#make a cursor on the connection
c = con.cursor()

#run an SQL statement on the cursor
sql = 'select * from %s' % table_name
c.execute(sql)

#check the results
print('result rowcount shows as= %d. (Note: -1 means "not known")' \
      % (c.rowcount,))
print()
print('result data description is:')
print('            NAME Type         DispSize IntrnlSz Prec Scale Null?')
for d in c.description:
    print(('%16s %-12s %8d %8d %4d %5d %s') % \
          (d[0], adc.adTypeNames[d[1]], d[2],   d[3],  d[4],d[5], bool(d[6])))
print()
print('str() of first five records are...') 

#get the results
db = c.fetchmany(5)

#print them
for rec in db:
    print(rec)

print()
print('repr() of next row is...')
print(repr(next(c)))
print()

c.close()
con.close()
