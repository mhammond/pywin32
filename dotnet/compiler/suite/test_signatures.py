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


class SignatureTest:
    def __init__(self, str):
        _com_params_="System.String"
        self.attr = str
    def func(self, arg):
        _com_params_="System.Int32"
        _com_return_type_="System.Int32"
        return arg + 1
    def func(self, arg):
        _com_params_="System.String"
        _com_return_type_="System.String"
        return arg[1:-1]
    def void_func(self):
        _com_return_type_="System.Void"
    def str_func(self, arg):
        _com_params_="System.String"
        _com_return_type_="System.String"
        return "whatever"
    def str_func_fail(self, arg):
        _com_params_="System.String"
        _com_return_type_="System.String"
        return 1+1 # Compiler is too smart if we just use a literal :-)
    def str_func_null(self, arg):
        _com_params_="System.String"
        _com_return_type_="System.String"
        return None

def TestSignatures():
    a=SignatureTest("Hello")
    if a.attr != "Hello":
        raise RuntimeError, "Attribute not correct"
    if a.func(2) != 3:
        raise RuntimeError, "Int func not correct"
    if a.func("Hello") != "ell":
        raise RuntimeError, "String func not correct"
    try:
        a.str_func(2)
        raise RuntimeError("Call to str_func with int param did not fail!")
    except TypeError:
        pass

    if a.str_func_null("hi") is not None:
        raise RuntimeError("Function returning NULL didnt!")
    try:
        print a.str_func_fail("hi")
        raise RuntimeError("Call to str_func_fail did not fail!")
    except TypeError:
        pass
    print "Testing explicit COM+ signatures succeeded"

TestSignatures()