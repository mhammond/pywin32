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

# test_exceptions.py

import COR.Python.Builtins

if COR.Python.Builtins.exceptions.NameError is not NameError:
    raise RuntimeError, "Builtin NameError is not the same as the runtime NameError"

def TestRaising():
    # Check that I cant get past a raise!
    bHitException = 1
    try:
        raise RuntimeError, "Hello"
        print "*** Error:  got past a raise!"
    except RuntimeError:
        bHitException = 1
    except:
        raise RuntimeError, "Wrong exception block entered"
    if not bHitException:
        raise RuntimeError, "Didnt appear to hit an exception!"

    print "Exception (raising) tests passed"

def TestFinally():
    bHitFinally = 0
    bHitExcept = 0
    dict = {}
    try:
        try:
            dict.foobar
            raise RuntimeError, "Could get a non-existent attribute"
        finally:
            bHitFinally = 1
        raise RuntimeError, "This code should not be reachable"
    except AttributeError:
        bHitExcept = 1
    if not bHitFinally:
        raise RuntimeError, "Did not hit the finally block"
    if not bHitExcept:
        raise RuntimeError, "Did not hit the except block"
    print "Exception (finally) tests passed"

def TestCatch():
    ok=0
    try:
        foo() # NameError raised
    except AttributeError, value:
        raise RuntimeError, "Wrong exception handler is invoked"
    except:
        ok = 1
    if not ok: raise RuntimeError, "Did not enter the default exception handler"

    # XXX - Should be able to derive from _my_ exception - but I can't!
    class MyException(COR.System.Exception):
        # As we have no ctor, this class gets all base ctors.
        pass
    ok = 0
    try:
        raise MyException
    except MyException, value:
        ok = 1
    if not ok: raise RuntimeError, "Did not catch my derived exception"
    try:
        raise MyException, "Wow"
    except MyException, value:
        if value.Message != "Wow":
            raise RuntimeError, "The string constructor didnt appear to be called!"

    class MyException2(COR.System.Exception):
            # As we define the ctor, this (ie, no params) is the only ctor we get
            def __init__(self):
                COR.System.Exception.__init__(self)
                self.hit = "yes"
    try:
        raise MyException2
    except MyException2, value:
        if value.hit != "yes":
            raise RuntimeError, "The exception instance didnt initialize correctly:" + value.hit

    ok = 0
    try:
        1/0
    except COR.System.DivideByZeroException, value:
        ok = 1
    if not ok: raise RuntimeError, "Did not catch the system thrown exception"
    print "Exception (catch) tests passed"

TestFinally()
TestRaising()
TestCatch()
