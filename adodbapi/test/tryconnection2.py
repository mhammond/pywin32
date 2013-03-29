def try_connection(constr):
    import adodbapi
    try:
        s = adodbapi.connect(constr) #connect to server
        s.close()
    except adodbapi.DatabaseError, inst:
        print(inst.args[0])    # should be the error message
        return False
    print "  (successful)"
    return True
