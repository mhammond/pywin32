from __future__ import print_function
import sys
import adodbapi
try:
    import adodbapi.is64bit as is64bit
    is64 = is64bit.Python()
except ImportError:
    is64 = False

if is64:
    driver = "Microsoft.ACE.OLEDB.12.0"
else:
    driver = "Microsoft.Jet.OLEDB.4.0"
extended =  'Extended Properties="Excel 8.0;HDR=Yes;IMEX=1;"'

try:  # first command line argument will be xls file name -- default to the one written by xls_write.py
    filename = sys.argv[1]
except IndexError:
    filename = 'xx.xls'

constr = "Provider=%s;Data Source=%s;%s" % (driver, filename, extended)

conn = adodbapi.connect(constr)

try:  # second command line argument will be worksheet name -- default to first worksheet
    sheet = sys.argv[2]
except IndexError:
    # use ADO internals to get the name of the first worksheet
    ado = conn.adoConn  # the actual ADO connection object
    adSchemaTables = 20  # known constant from Microsoft
    schema = ado.OpenSchema(adSchemaTables) # get list of tables
    f = adodbapi.getIndexedValue(schema.Fields,'TABLE_NAME')
    sheet = f.Value

print('Shreadsheet=%s  Worksheet=%s' % (filename, sheet))
print('------------------------------------------------------------')
crsr = conn.cursor()
sql = "SELECT * from [%s]" % sheet
crsr.execute(sql)
for row in crsr.fetchmany(10):
    print(repr(row))
crsr.close()
conn.close()
