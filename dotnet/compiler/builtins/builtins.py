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

# COM+ builtins
#import COR.Python.Runtime
import COR.System

#None = COR.Python.Runtime.Py_None

## Builtin functions in alpha order
##

def abs(val):
    return COR.Python.Runtime.PyNumber_Absolute(val)

def apply(fn, args, kw = None):
    # Nice and simple with extended call support :-)
    # EEEK - but the test_builtins fail .
    # it WILL be simple once it works correctly :-)
    # return fn(*args, **kw)

    # Args are PyObject[], so get deref'd by the compiler - need to wrap.
    args_ob = COR.Python.Converters.PyTuple_AsArray(args)
    return COR.Python.Runtime.PyObject_Call(fn, args_ob, kw)

# buffer
# callable
# chr

def cmp(x, y):
    return COR.Python.Runtime.PyObject_Compare(x,y)

# coerce
# compile
# complex
# delattr
# dir
# divmod
# eval
# execfile
# exit
# filter
def float(val):
    return COR.Python.Runtime.PyNumber_Float(val)

def getattr(ob, name, default):
    try:
        return COR.Python.Runtime.PyObject_GetAttr(ob, name)
    except AttributeError:
        return default

# globals

def hasattr(ob, name):
    try:
        COR.Python.Runtime.PyObject_GetAttr(ob, name)
        return 1
    except AttributeError:
        return 0
        
# hash
# hex

def id(ob):
    return COR.Python.Runtime.PyObject_GetId(ob)

# input

def int(val, base=-1):
    if type(val)==type(''):
        if base==-1: base = 10
        return COR.System.Convert.ToInt32(val, base)
    if base!=-1:
        raise TypeError("can't convert non-string with explicit base")
    return COR.Python.Runtime.PyNumber_Int(val)

def intern(s):
    return s

# isinstance
# issubclass

def len(o):
    return COR.Python.Runtime.PyObject_Length(o)

def list(s):
    return COR.Python.Runtime.PySequence_List(s)

# locals

def long(val):
    # XXX - need long ints!
    return int(val)

def map(fn, *seqs):
    if len(seqs)==0:
        raise TypeError("map() requires at least 2 args")
    if fn is None and len(seqs)==0:
        # map(None, S) is the same as list(S).
        return COR.Python.Runtime.PySequence_List(seqs[0])

    if fn is None:
        fn = lambda *args : args
    ret = []
    index = 0
    while 1:
        finished = 1
        args = []
        for seq in seqs:
            if index>=len(seq):
                args.append(None)
            else:
                args.append(seq[index])
                finished = 0
        if finished:
            break
        tup_args = COR.Python.Runtime.PySequence_Tuple(args)
        ret.append(apply(fn, tup_args))
        index = index + 1
    return ret

# max
# min
# oct
# open
def ord(c):
    return c.ord(0)

def pow(x, y, z=None):
    return COR.Python.Runtime.PyNumber_Power(x, y, z)

# quit

def range(*args):
    nargs = len(args)
    if nargs<=0 or len(args)>3:
        raise TypeError("range expects 1-3 integer args")
    if len(args)==1:
        start = 0; stop = args[0]; step = 1
    else:
        start = args[0]
        stop = args[1]
        if nargs==3:
          step = args[2]
        else:
          step = 1
    ret = []
    while start < stop:
        ret.append(start)
        start = start + step
    return ret

# raw_input
# reduce
# reload

def repr(s):
    return COR.Python.Runtime.PyObject_Repr(s)

# round

def setattr(ob, name, val):
    COR.Python.Runtime.PyObject_SetAttr(ob, name, val)

# setattr
# slice

def str(ob):
    return COR.Python.Runtime.PyObject_Str(ob)

def tuple(s):
    return COR.Python.Runtime.PySequence_Tuple(s)

# type
# vars
# xrange


# These should go to exceptions.py
IndexError = COR.Python.Builtins.Exceptions.IndexError
RuntimeError = COR.Python.Builtins.Exceptions.RuntimeError
AttributeError = COR.Python.Builtins.Exceptions.AttributeError
