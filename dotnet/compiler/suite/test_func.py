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

# NOTE - taken from the compiler-sig compiler test suite!
def test1(a, b=2, c=3):
    return a + c

def test2(a, **kw):
    for k, v in kw.items():
        print k, v

def test3(find, zfill, a):
    import string
    return string.zfill(a, 10)

def test4(((a, b), c), x, (d, (e, f)), y, z):
    return a + b * c - d / e + f

def test5(a=1, *rest):
    print rest
    return map(lambda x, a=a: x + a, rest)

def test():
    print test1(2)
    print test2('a', b='c', c='d', abc='42')
    print test3(None, None, "365")
    print test4(((10, 6), 2), 0, (6, (3, 22)), 0, 0)
    print test5()
    print test5(2, 0, 1, 2)
    # Some extended call tests.
    args = 0, (6, (3,22))
    kwargs = {'y':0, 'z':0}
    print "skipping test that kw args can come before the extended syntax - compiler bug :-("
    #print test4( ((10,6), 2), *args, **kwargs)
    # Ensure kw args can come before the extended syntax.
    #print test4( ((10,6), 2), y=0,z=0, *args)
    kwargs = {'a':1, 'c':4}
    #print test1( **kwargs )

#    print test5( *(2, 0, 1, 2) )
#    print test5( 2, *(0, 1, 2) )

test()

