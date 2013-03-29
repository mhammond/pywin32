"""call using an open ADO connection --> list of table names"""
import adodbapi

def names(connection_object):
    ado = connection_object.adoConn
    schema = ado.OpenSchema(20) # constant = adSchemaTables

    tables = []
    while not schema.EOF:
        name = adodbapi.getIndexedValue(schema.Fields,'TABLE_NAME').Value
        tables.append(name)
        schema.MoveNext()
    del schema
    return tables
