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

# test_methods.py - test class, instance and method semantics.
class TestClass:
    def TestMethod(self, arg):
        return [self, arg] # Dont have tuples yet :-)

def TestInstanceSemantics():
    if TestClass.__name__ != "TestClass":
        raise RuntimeError, "Class __name__ attribute is wrong! (%s)" % (TestClass.__name__,)
    instance = TestClass()
    if instance.TestMethod.im_self is not instance:
        raise RuntimeError, "Bound method has the wrong instance!"
    if instance.TestMethod.im_class is not TestClass:
        raise RuntimeError, "Bound method has the wrong class!"
    rc = instance.TestMethod(3)
    ob, arg = instance.TestMethod(3)
    if ob is not instance:
        raise RuntimeError, "did not get self back from the method"
    if arg != 3:
        raise RuntimeError, "did not get the arg back from the method"
    # Call the unbound method
    ob, arg = TestClass.TestMethod(instance, 4)
    if ob is not instance:
        raise RuntimeError, "did not get self back from the unbound method"
    # Note : XXX - TestClass.TestMethod is not instance.TestMethod.im_func - does this matter?
    print "Class/Instance semantic checks passed"

class TestClass1:
    class1_attr = "class1 attr"
    def __init__(self):
        self.attr = "TestClass1"
    def method(self):
        return self
    def another(self):
        return self

class TestClass2(TestClass1):
    class2_attr = "class2 attr"
    def __init__(self, attr = None):
        TestClass1.__init__(self)
        self.attr = attr
    def method(self):
        return self.attr
    def another(self):
        return [TestClass1.another(self)]
    def not_another(self):
        return TestClass1.another(self)
    def methodwithintdefault(self, arg=99):
        return arg
    def methodwithstringdefault(self, arg = "weeee"):
        return arg
    def methodwithnonedefault(self, arg = None):
        return arg

class TestClass2_1(TestClass2):
    def __init__(self, attr = "foo"):
    	TestClass2.__init__(self, attr)

class TestClass2_2(TestClass2_1):
    def __init__(self, attr = 1):
    	TestClass2_1.__init__(self, attr)

def TestOverloads():
    t = TestClass1()
    if t.attr != "TestClass1":
        raise RuntimeError, "TestClass1 attribute didnt work"
    if t.method() is not t:
        raise RuntimeError, "TestClass1 method didnt work"
    if t.another() != t:
        raise RuntimeError, "TestClass1 method called by base class didnt work"
##    if t.class1_attr != "class1 attr":
##        raise RuntimeError, "TestClass1 class attribute didnt work"

    t = TestClass2(None)
    print "got t"
    if t.attr != None:
        raise RuntimeError, "TestClass2 attribute didnt work"
    if t.method() is not None:
        raise RuntimeError, "TestClass2 method didnt work"
    if t.another()[0] != t:
        raise RuntimeError, "TestClass2 method that calls base class didnt work"
    if t.not_another() != t:
        raise RuntimeError, "TestClass2 method that calls different base class method didnt work"
    try:
        if t.class1_attr != "class1 attr":
            raise RuntimeError, "TestClass2 class attribute1 didnt work"
        print "Problem solved - base-class class attribute worked!"
    except AttributeError:
        print "Known problem - can't access base-class class attributes"

    t = TestClass2("bar")
    if t.attr != "bar":
        raise RuntimeError, "TestClass2 attribute didnt work"
    t = TestClass2(0)
    if t.attr != 0:
        raise RuntimeError, "TestClass2 attribute didnt work"
    # Classes with different default types.
    print "Known problem - default args don't work"
#    if TestClass2_1().attr != "foo":
#        raise RuntimeError, "TestClass2_1() attribute didnt work"
    if TestClass2_1(None).attr != None:
        raise RuntimeError, "TestClass2_1(None) attribute didnt work"
    if TestClass2_1("bar").attr != "bar":
        raise RuntimeError, "TestClass2_1('bar') attribute didnt work"
    if TestClass2_1(99).attr != 99:
        raise RuntimeError, "TestClass2_1(99) attribute didnt work"

    print "Skipping trying to use default integer param - compiler can't work out how to get COM+ UDT values!?"
#    if TestClass2_2().attr != 1:
#        raise RuntimeError, "TestClass2_2() attribute didnt work"
    if TestClass2_2(None).attr != None:
        raise RuntimeError, "TestClass2_2 attribute didnt work"
    if TestClass2_2("bar").attr != "bar":
        raise RuntimeError, "TestClass2_2 attribute didnt work"
    if TestClass2_2(99).attr != 99:
        raise RuntimeError, "TestClass2_2 attribute didnt work"

#    if t.methodwithintdefault() != 99:
#        raise RuntimeError, "Default value (methodwithintdefault) was wrong!"
    if t.methodwithintdefault(100) != 100:
        raise RuntimeError, "Default value (methodwithintdefault without default!) was wrong!"
#    if t.methodwithstringdefault() != "weeee":
#        raise RuntimeError, "Default value (methodwithstringdefault) was wrong!"
    if t.methodwithstringdefault("spam") != "spam":
        raise RuntimeError, "Default value (methodwithstringdefault without default!) was wrong!"
    print "skipping test for default=None - bug in COM+ (see PyMethodType::tp_call()?"
#    if t.methodwithnonedefault() != None:
#        raise RuntimeError, "Default value (methodwithnonedefault) was wrong - " + repr(t.methodwithnonedefault())
    if t.methodwithnonedefault(3) != 3:
        raise RuntimeError, "Default value (methodwithnonedefault without default!) was wrong - " + t.methodwithnonedefault(3)

    t = TestClass2("Foo")
    if t.attr != "Foo" or t.method() != "Foo":
        raise RuntimeError, "TestClass2 failed with none None param"

    print "skipping test for default=None - bug in COM+ (see PyMethodType::tp_call()?"
##    t = TestClass2()
##    if t.attr != None:
##        raise RuntimeError, "TestClass2 attribute didnt work"
    t.attr = "wow"
    if t.attr != "wow":
        raise RuntimeError, "TestClass2 attribute didnt work after change."
    TC = TestClass2 # trick the compiler into doing reflection based creation
    t = TC()
    if t.attr != None:
        raise RuntimeError, "TestClass2 attribute didnt work"
    if t.method() != None:
        raise RuntimeError, "TestClass2 method didnt work"
    print "Class Overloading tests passed."

class TestClass3:
    def __init__(self, foo, bar):
        self.foo = foo
        self.bar = bar
class TestClass4(TestClass3):
    def hi(self): pass

def TestConstructors():
    t = TestClass4(1,"bar")
    if t.foo != 1 or t.bar != "bar":
        raise RuntimeError, "constructor on base class as not called"
    print "Class constructor tests passed."

# Do some basic interface tests
class MyCustomAttributeProvider(COR.System.Reflection.ICustomAttributeProvider):
    def GetCustomAttributes(self, typ, bInherit):
        return None
    def GetCustomAttributes(self, bInherit):
        return None
    def IsDefined(self, typ, bInherit):
        return 0

class NotACustomAttributeProvider:
    pass

def PassInterface(i):
    _com_params_ = "System.Reflection.ICustomAttributeProvider"
    _com_return_type_ = "System.Reflection.ICustomAttributeProvider"
    return i

def TestInterfaces():
#    a = MyCustomAttributeProvider()
#    if PassInterface(a) is not a:
#        raise RuntimeError("Unexpected result from PassInterface")
    a = NotACustomAttributeProvider()
    try:
        PassInterface(a)
        raise RuntimeError("Could pass an object that wasnt the interface")
    except TypeError:
        pass
    print "Interface tests worked."
    
# Some pure-python classes
class PurePython:
	def __init__(self, **args): # Will force no COM+ base-class
		self.args = args
	def getarg(self, name):
		return self.args[name]

class PurePythonChild(PurePython):
	def getarg(self, name):
		return "child " + str(PurePython.getarg(self, name))

def TestPurePython():
	b = PurePython(hello="hello", number=1)
	if b.getarg("hello") != "hello" or b.getarg("number") != 1:
		raise RuntimeError("Can't get the properties")

##	b = PurePythonChild(hello="hello", number=1)
##	if b.getarg("hello") != "child hello" or b.getarg("number") != "child 1":
##		raise RuntimeError("Can't get the properties from the child.")
	print "Pure-python class tests worked."

TestInstanceSemantics()
TestOverloads()
TestConstructors()
TestInterfaces()
#TestPurePython()