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

# test_builtins.py - test the COM+ builtins.

# Note that I copy all the builtin names to globals to avoid the compiler getting too smart :-)

def test_int(int):
    if int("10") != 10:
        raise RuntimeError, "simple string conversion failed"
    if int("-10") != -10:
        raise RuntimeError, "simple negative string conversion failed"
    if int("A", 16) != 10:
        raise RuntimeError, "hex string conversion failed"

def test_float(float):
    if float("10") != 10.0:
        raise RuntimeError, "simple string conversion failed"
    if float("-10") != -10.0:
        raise RuntimeError, "simple negative string conversion failed"

def test_str(str):
    if str(1) != "1":
        raise RuntimeError, "str() of int didnt work"

def test_len(len):
    if len([1,2,3]) != 3:
        raise RuntimeError, "len of simple list failed"

    items = [1, None]
    for i in items:
        try:
            len(i)
            raise RuntimeError, "len of invalid type didnt throw type error" % (i,)
        except TypeError:
            pass

def test_apply(apply):
    func = lambda a, b: a+b
    if apply(func, (1,2)) != 3:
        raise RuntimeError, "simple apply result was wrong"

    func = lambda a, b=100: a+b
    if apply(func, (1,)) != 101:
        raise RuntimeError, "apply with default result was wrong"

    func = lambda a, b=100: a+b
    if apply(func, (1,2)) != 3:
        raise RuntimeError, "apply with default (supplied) result was wrong"

    def sum(*args):
        ret = 0
        for arg in args:
            ret = ret + arg
        return ret
    if apply(sum, ()) != 0:
        raise RuntimeError, "apply with sum() result was wrong"
    if apply(sum, (1,)) != 1:
        raise RuntimeError, "apply with sum(1) result was wrong"
    if apply(sum, (1,2)) != 3:
        raise RuntimeError, "apply with sum(1,2) result was wrong"

    func = lambda f, *args, **kw: (f, args, kw)
    pass_args = (3,2,1)
    dict = {'kw1' : 1, 'kw2' : 2}
    if apply(func, pass_args, dict) != (3, (2,1), dict):
        raise RuntimeError, "complex apply result was wrong"

def test_tuple(tuple):
    if tuple([1,2,3]) != (1,2,3):
        raise RuntimeError, "Simple tuple test failed"
    if tuple((1,2,3)) != (1,2,3):
        raise RuntimeError, "Simple tuple test failed"
    try:
        tuple(None)
        raise RuntimeError, "tuple of None didnt cause type error"
    except TypeError:
        pass

def test_list(list):
    if list([1,2,3]) != [1,2,3]:
        raise RuntimeError, "Simple tuple test failed"
    if list((1,2,3)) != [1,2,3]:
        raise RuntimeError, "Simple tuple test failed"
    try:
        list(None)
        raise RuntimeError, "tuple of None didnt cause type error"
    except TypeError:
        pass

def test_map(map):
    if map(None, [1,2,3], [4,5,6]) != [(1,4), (2,5), (3,6)]:
        raise RuntimeError, "simple map test didnt work"
    if map(str, [1,2,3]) != ['1', '2', '3']:
        raise RuntimeError, "map using builtin didnt work"
    f = lambda arg : arg+1
    if map(f, (1,2,3)) != [2,3,4]:
        raise RuntimeError, "map using lambda didnt work"

def test_range(range):
    if range(3) != [0,1,2]:
        raise RuntimeError, "single arg range failed"
    if range(2,3) != [2]:
        raise RuntimeError,"2 arg range failed"
    if range(2,6,2) != [2,4]:
        raise RuntimeError,"3 arg range failed"

def test_abs(abs):
    if abs(-3) != 3:
        raise RuntimeError, "builtin abs() failed"

test_str(str)
test_int(int)
test_float(float)
test_list(list)
test_tuple(tuple)
test_len(len)
test_apply(apply)
test_map(map)
test_range(range)
test_abs(abs)
print "Builtins passed"