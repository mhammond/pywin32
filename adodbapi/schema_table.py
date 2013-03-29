""" coll using an open ADO connection --> list of table names"""
import adodbapi

def names(connection_object):
    ado = connection_object.adoConn
    schema = ado.OpenSchema(20) # constant = adSchemaTables

    tables = []
    while not schema.EOF:
        name = adodbapi.getIndexedValue(schema.Fields,'TABLE_NAME').Value
        type = adodbapi.getIndexedValue(schema.Fields,'TABLE_TYPE').Value
        schema.MoveNext()
        tables.append(name)
    del schema
    return tables
