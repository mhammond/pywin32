""" db_print.py -- a simple demo for ADO database reads."""
import sys
import adodbapi
import adodbapi.is64bit as is64bit
import adodbapi.schema_table as schema_table

try:
    databasename = sys.argv[1]
except IndexError:
    databasename = "test.mdb"

if is64bit.Python():
    driver = "Microsoft.ACE.OLEDB.12.0"
else:
    driver = "Microsoft.Jet.OLEDB.4.0"
constr = "Provider=%s;Data Source=%s" % (driver, databasename)

#create the connection
con = adodbapi.connect(constr)

print('Table names in= %s' % databasename)

for table in schema_table.names(con):
    print(table)
