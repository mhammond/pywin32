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
extended = 'Extended Properties="Excel 8.0;HDR=Yes;IMEX=1;"'

try:  # first command line argument will be xls file name -- default to the one written by xls_write.py
    filename = sys.argv[1]
except IndexError:
    filename = "xx.xls"

constr = f"Provider={driver};Data Source={filename};{extended}"

conn = adodbapi.connect(constr)

try:  # second command line argument will be worksheet name -- default to first worksheet
    sheet = sys.argv[2]
except IndexError:
    # use ADO feature to get the name of the first worksheet
    sheet = conn.get_table_names()[0]

print(f"Shreadsheet={filename}  Worksheet={sheet}")
print("------------------------------------------------------------")
crsr = conn.cursor()
sql = f"SELECT * from [{sheet}]"
crsr.execute(sql)
for row in crsr.fetchmany(10):
    print(repr(row))
crsr.close()
conn.close()
