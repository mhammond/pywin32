# Portions Copyright 1999-2000 Microsoft Corporation.
# Portions Copyright 1997-1999 Greg Stein and Bill Tutt.
#
# This source code may be freely distributed, as long as all
# copyright information remains in place.
#
# See also the copyrights for the version of Python you are using.
#
# Implemented 1999-2000 by Mark Hammond (MarkH@ActiveState.com)
#
# See http://www.ActiveState.com/.NET for the latest versions.


# COM+ no longer has Box functions on value types.
# There are no other obvious candidates for testing
# all these types (we need, eg, a method with an int16
# param, but no overloads that also take other sizes)
def TestIntConversions():
    int_type = type(0)
    # The compiler sees these signatures, so therefore has to convert
    # from a PyObject to an Int16.
    i=COR.System.Int16.Box(1+1)
    if type(i) is not int_type:
        print i, type(i)
        raise RuntimeError("Couldnt convert PyInt object to Int16")
    # Compiler handles literals specially - in this case, it needs to
    # convert directly from Int32 to Int16.
    i=COR.System.Int16.Box(2)
    if type(i) is not int_type:
        raise RuntimeError("Couldnt convert int literal to Int16")

    # This wont work until we get long integers - the result
    # of this should not be silently narrowed into an int
    i=COR.System.Int64.Box(1+1)
    if type(i) is not int_type:
        print "Int64 expression conversions wont work properly until we get longs!"
        if i.__class__.__name__ != "System.Int64":
            raise RuntimeError("Expected back an int64 object!")
#        raise RuntimeError("Couldnt convert PyInt object to Int64")
    # Compiler handles literals specially
    i=COR.System.Int64.Box(2)
    if type(i) is not int_type:
        print "Int64 literal conversions wont work properly until we get longs!"
        if i.__class__.__name__ != "System.Int64":
            raise RuntimeError("Expected back an int64 object!")
#        raise RuntimeError("Couldnt convert int literal to Int64")

#TestIntConversions()
print "Sorry - havent updated this test suite yet"
#print "Conversions tests passed"