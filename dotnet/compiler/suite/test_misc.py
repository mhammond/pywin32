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


b = "Global Variable B"

def TestFunc2(arg):
    return arg

# Testing locals and a few misc things.
def TestLocals():
    b=1
    if b != 1:
        raise RuntimeError, "local variable b should be 1 - is" + str(b)
    b=2
    if b != 2:
        raise RuntimeError, "local variable b should be 2 - is" + str(b)
    if b*b != 4:
        raise RuntimeError, "Couldnt multiple b*b!"
    b = "Hello There"
    if b != "Hello There":
        raise RuntimeError, "local variable b should be 'Hello There' - is" + str(b)
    # Call a function with a local var as an arg.
    if TestFunc2(b) != b:
        raise RuntimeError, "TestFunc2 yielded the wrong result given a variable"
    # Call a function with a literal string as an arg.
    if TestFunc2("Hi there") != "Hi there":
        raise RuntimeError, "TestFunc2 yielded the wrong result given a string literal"
    print "Local variable test passed"

def TestStrings():
    if "hello" + " there" != "hello there":
        raise RuntimeError, "couldnt add strings!"
    if "hello" + str(" there") != "hello there":
        raise RuntimeError, "couldnt add str'd strings!"
    if "hi" * 3 != "hihihi":
        raise RuntimeError, "ERROR: String multiplication failed!"
    if "hi"[0] != 'h' or "hi"[-1] != 'i':
        raise RuntimeError, "ERROR: String indexing failed!"
    b = "Hello There"
    if b[0] != "H":
        raise RuntimeError, "simple string indexing did not work"
    if b[0:2] != "He":
        raise RuntimeError, "simple string slicing did not work"
    if b[-1] != "e":
        raise RuntimeError, "simple string indexing (negative) did not work"
    if b[1:-1] != "ello Ther":
        print b[1:-1]
        raise RuntimeError, "string slicing (negative) did not work"
    if b[-2:-1] != "r":
        raise RuntimeError, "string slicing (both negative) did not work"
    if b[-2:] != "re":
        raise RuntimeError, "string slicing (first negative, second omitted) did not work"
    try:
        b[100]
        raise RuntimeError, "Could index past the length of the string"
    except IndexError:
        pass
    print "String tests passed"

def TestBoolean():
    if 1==1:
        pass
    else:
        raise RuntimeError, "1==1 failed!"
    if 1 != 1:
        raise RuntimeError, "1!=1 failed!"
    if 1<=1:
        pass
    else:
        raise RuntimeError, "1<=1 failed"
    if 1 != 2:
        pass
    else:
        raise RuntimeError, "1!= 2 failed"
    if 1<2:
        pass
    else:
        raise RuntimeError, "1<2 failed"
    if 1<=2:
        pass
    else:
        raise RuntimeError, "1<=2 failed"
    if 1>2:
        raise RuntimeError, "1>2 failed"

    if '':
        raise RuntimeError, "ERROR: Empty string returns true"
    if "Hi":
        pass
    else:
        raise RuntimeError, "ERROR: Non empty string is false"
    if "Hi"=="Hi":
        pass
    else:
        raise RuntimeError, "ERROR: equal strings arent equal"
    if "a" < "b":
        pass
    else:
        raise RuntimeError, "ERROR: simple strings dodnt compare equal"
    d = {}
    if d:
        raise RuntimeError, "ERROR: empty dictionary is true"
    else:
        pass
    d[0]=1
    if d:
        pass
    else:
        raise RuntimeError, "ERROR: non-empty dictionary is false"
    # Test that bools work OK from functions.
    def TestLessThan(a,b): return a<b
    if TestLessThan(1,0) or not TestLessThan(0, 1):
        raise RuntimeError, "Bool result from function failed"
    print "Boolean tests passed"

def TestWhile():
    i=0
    bHitElse = 0
    while i < 3:
        i = i + 1
    else:
        bHitElse = 1
    if not bHitElse:
        raise RuntimeError, "while loop did not enter else clause"
    i=0
    while 1 < 1000:
        i = i + 1
        if i % 2==0:
            continue
#        print "i should be odd:", i
        if i > 6:
            break
    else:
        raise RuntimeError, "Should not enter this else clause"
    print "while loop tests passed"

def TestFor():
    check = 0
    hitelse=0
    for i in [0,1,2,3,4]:
        if i != check:
            raise RuntimeError, "looping over range failed?"
        check = check + 1
    else:
        hitelse = 1
    if not hitelse:
        raise RuntimeError, "For loop failed to hit the else block"
    hitelse = 0
    for i in [1,2]:
        break
    else:
        hitelse = 1
    if hitelse:
        raise RuntimeError, "For loop incorrectly hit the else block"
    print "for loop tests passed"

def TestLists():
    a = [1,2,3]
    if a != a or a != [1,2,3]:
        raise RuntimeError, "List doesnt compare correctly"
    if a == [1,2,3,4]:
        raise RuntimeError, "List doesnt fail comparison correctly"
    if a[0] != 1 or a[1] != 2 or a[2] != 3:
        raise RuntimeError, "Array index is wrong"
    a = [1,2,[3,4]]
    if a != a or a != [1,2,[3,4]]:
        raise RuntimeError, "List with list doesnt compare correctly"
    a = [1,2,(3,4)]
    if a != a or a != [1,2,(3,4)]:
        raise RuntimeError, "List with tuple doesnt compare correctly"

    [a,b,[c]]=[1,2,[3]]
    if a != 1 or b != 2 or c != 3:
        raise RuntimeError, "List unpack didnt work"
    try:
        a,b = [1,2,3]
        raise RuntimeError, "Assignment of list too big for assignments did not fail!"
    except ValueError:
        pass
    try:
        a,b,c = [1,2]
        raise RuntimeError, "Assignment of list too big for assignments did not fail!"
    except ValueError:
        pass
    if 1 not in [1,2,3] or not 1 in [1,2,3]:
        raise RuntimeError, "'in' test for list failed"
    if 4 in [1,2,3] or not 4 not in [1,2,3]:
        raise RuntimeError, "'in' test for list failed"
    a = [1,2,3,4,5]
    del a[0]
    if a != [2,3,4,5]:
        raise RuntimeError, "Deleting from a list failed"
    del a[1]
    if a != [2,4,5]:
        raise RuntimeError, "Deleting 2nd from a list failed"
    del a[-1]
    if a != [2,4]:
        raise RuntimeError, "Deleting last from a list failed"
    a = [0]
    a[0] = 1
    if a[0] != 1:
        raise RuntimeError, "list replacement failed"
    a = [0]
    a[0] = 1,2
    if a[0] != (1,2):
        raise RuntimeError, "list replacement of tuple failed"
    # woo hoo - have list methods.
    a = ["Hello", 3, 2, 1, 1]
    if len(a) != 5:
        raise RuntimeError, "list.length failed"
    if a.count(1) != 2:
        raise RuntimeError, "list.count failed"
    if a.pop() != 1:
        raise RuntimeError, "list.pop failed"
    if a != ["Hello", 3, 2, 1]:
        raise RuntimeError, "list.pop failed (list contents wrong)"
    a.remove("Hello")
    if a != [3,2,1]:
        raise RuntimeError, "list.remove failed (list contents wrong)"
    try:
        a.remove(9)
        raise RuntimeError, "list.remove failed (no exception)"
    except ValueError:
        pass
    if a.index(3) != 0:
        raise RuntimeError, "list.index failed (list contents wrong)"
    a.sort()
    if a != [1,2,3]:
        raise RuntimeError, "list.sort failed (list contents wrong)"
    a.reverse()
    if a != [3,2,1]:
        raise RuntimeError, "list.reverse failed (list contents wrong)"

    # Test slices
    a = [1,2,3]
    if a[:1] != [1]:
        raise RuntimeError, "List slicing failed"
    if a[:] != a:
        raise RuntimeError, "List slicing failed"
    if a[:-1] != [1,2]:
        raise RuntimeError, "List slicing failed"
    # Test slice assignment
    a = [1]
    a[1:] = [2]
    if a != [1,2]:
        raise RuntimeError, "List slice assignment failed"
    a = [1,2,3]
    a[:-1] = ["moo"]
    if a != ["moo", 3]:
        raise RuntimeError, "List slice assignment failed"

    a=[1,2,3]
    a[1:2] = [9,8,7]
    if a != [1, 9, 8, 7, 3]:
        raise RuntimeError, "List slice assignment failed"

    # Some other misc tests for things that bit me in the past, and now wont again :-)
    a = [('1',1), ('2', 2)]
    a.sort()
    if not (len(a)==2 and len(a[0])==2 and len(a[1])==2):
        raise RuntimeError, "Sorting the nested list didnt work!"
    a = [['1',1], ['2', 2]]
    a.sort()
    if not (len(a)==2 and len(a[0])==2 and len(a[1])==2):
        raise RuntimeError, "Sorting the nested list didnt work 2nd time!"

    print "List tests passed"

def TestTuples():
    a = (1,2,3)
    if a != a or a != (1,2,3):
        raise RuntimeError, "Tuple doesnt compare correctly"
    if a == (1,2,3,4):
        raise RuntimeError, "Tuple doesnt fail comparison correctly"
    if a[0] != 1 or a[1] != 2 or a[2] != 3:
        raise RuntimeError, "Array index is wrong"
    a = (1,2,(3,4))
    if a != a or a != (1,2,(3,4)):
        raise RuntimeError, "Tuple with tuple doesnt compare correctly"
    a = (1,2,[3,4])
    if a != a or a != (1,2,[3,4]):
        raise RuntimeError, "Tuple with list doesnt compare correctly"

    a,b,(c,)=1,2,(3,)
    if a != 1 or b != 2 or c != 3:
        raise RuntimeError, "Tuple unpack didnt work"
    try:
        a,b = (1,2,3)
        raise RuntimeError, "Assignment of tuple too big for assignments did not fail!"
    except ValueError:
        pass
    try:
        a,b,c = (1,2)
        raise RuntimeError, "Assignment of tuple too big for assignments did not fail!"
    except ValueError:
        pass

    if 1 not in (1,2,3) or not 1 in (1,2,3):
        raise RuntimeError, "'in' test for tuple failed"
    if 4 in (1,2,3) or not 4 not in (1,2,3):
        raise RuntimeError, "'in' test for tuple failed"

    a = 1,2,3
    if len(a) != 3:
        raise RuntimeError, "Tuple length is not correct"
    try:
        del a[1]
        raise RuntimeError, "Could delete from a tuple"
    except TypeError:
        pass
    a = [1,2,3],4,5
    if a[0] != [1,2,3]:
        raise RuntimeError, "Tuple with first element a tuple failed"

    # Test slices
    a = (1,2,3)
    if a[:1] != (1,):
        raise RuntimeError, "Tuple slicing failed"
    if a[:] != a:
        raise RuntimeError, "Tuple slicing failed"
    if a[:-1] != (1,2):
        raise RuntimeError, "Tuple slicing failed"

    print "Tuple tests passed"

def TestDicts():
    # Now dictionary assignment
    dict = {}
    dict['key'] = "foo"
    if dict['key']!= 'foo':
        raise RuntimeError, "Dict indexing failed"
    dict[1] = 2
    try:
        dict["foo"]
        raise RuntimeError, "Could get an invalid dict key"
    except KeyError:
        pass
    dict[None] = None
    if dict[None] != None:
        raise RuntimeError, "Dict[None] didnt work!"
    dict[1,2] = 3,4
    if dict[1,2] != (3,4):
        raise RuntimeError, "dict with tuple index failed."
    if TestFunc2(dict["key"]) != "foo":
        raise RuntimeError, "TestFunc2 passed a dict item failed"
    if TestFunc2(dict[1]) != 2:
        raise RuntimeError, "TestFunc2 passed a dict item failed"

    # Sub-dict assignment
    dict["subdict"] = {}
    dict["subdict"]["subvalue"] = "subdict value"
    if dict["subdict"]["subvalue"] != "subdict value":
        raise RuntimeError, "sub-dict value is wrong"

    dict = {'0' : 0, '1' : 1, '2' : 2, '3' : 3, '4' : 4}
    ok = 0
    # Some basic exceptions
    try:
        # This used to work - it is correct now :-)
        num = dict.Count
        raise RuntimeError,  "Could get a 'protected' attribute"
    except AttributeError:
        ok= 1
    if not ok:
        raise RuntimeError, "didnt hit the attribute error"
    for i in range(5):
        if dict[str(i)] != i:
            raise RuntimeError, "dict index failed"
    v = dict.keys()
    v.sort()
    if v != map(str, range(5)):
        raise RuntimeError, "dict keys() failed - '%s'" % (dict.keys(),)
    v = dict.values()
    v.sort()
    if v != range(5):
        raise RuntimeError, "dict values() failed - '%s'" % (dict.values(),)
    v = dict.items()
    v.sort()
    if v != map(None, map(str, range(5)), range(5)):
        raise RuntimeError, "dict items() failed - '%s'" % (dict.items(),)

    print "Dictionary tests passed"

def TestLambdas():
    lst=[]
    la=lambda lst=lst:lst.append("x")
    la()
    if lst != ['x']:
      raise RuntimeError, "Lambda default arg didnt work"
    la()
    if lst != ['x', 'x']:
      raise RuntimeError, "Lambda default arg didnt work"
    new_lst = []
    la(new_lst)
    if len(new_lst)!=1 or new_lst[0]!='x':
      raise RuntimeError, "Lambda non-default arg didnt work"
    la = lambda number: number==0
    if not la(0):
        raise RuntimeError, "boolean lambda failed!"
    if la(1):
        raise RuntimeError, "boolean lambda failed!"
    print "Lambda tests passed"

def TestMisc():
    # simple arithmatic and logic
    if 67*3-60*3+(15%8)*3 != 42:
        raise RuntimeError, "The secret value is wrong!"
    #
    if 1.1 * 2.0 != 2.2:
        raise RuntimeError, "Floats didnt multiply"
    # Check coercion each way...
    if 1.1 * 2 != 2.2:
        raise RuntimeError, "Floats didnt multiply"
    if 2 * 1.1  != 2.2:
        raise RuntimeError, "Floats didnt multiply"

    if 1 | 2 | 4 != 7:
        raise RuntimeError, "chained binary 'or' ops did not work correctly"
    if 7 & 2 != 2:
        raise RuntimeError, "'and' did not work correctly"
    if 123 ^ 64 ^ 64 != 123:
        raise RuntimeError, "chained binary 'xor' ops did not work correctly"

    a = b = 1
    if a != 1 and b != 1:
        raise RuntimeError, "multuple assignment failed"

    if "Hi" and 1 and 1:
        pass
    else:
        raise RuntimeError, "ERROR: 'and' test failed"
    if 0 or 0 or 0 or "Hi":
        pass
    else:
        raise RuntimeError, "ERROR: 'or' test failed"

    if type(0) is not type(1) or type(0) != type(1):
        raise RuntimeError, "Same types dont compare"
    if type(type(0)) is not type(type("1")) or type(type(0)) != type(type("1")):
        raise RuntimeError, "Type type doesnt compare"
    if type(1) is type("1") or type(1)==type("1"):
        raise RuntimeError, "Different types do compare"
    
    print "misc tests passed"

# Simple global variable tests
a=1
if a != 1:
    raise RuntimeError, "Global variable a is not 1!!!"
a = "Hi there"
if a != "Hi there":
    raise RuntimeError, "Global variable a is not 'Hi there'!!!"

TestLocals()
TestStrings()
TestBoolean()
TestWhile()
TestFor()
TestLists()
TestTuples()
TestDicts()
TestLambdas()
TestMisc()

############# Not working until we sort out delegates etc in the generate phase.
##testMethodCalled = 0
##class TestClass:
##    def TestMethod(self):
##        global testMethodCalled
##        testMethodCalled = 1
##    def TestMethod2(self, arg):
##        return arg
##
##TestClass().TestMethod()
##if not testMethodCalled:
##    raise RuntimeError, "TestMethod didnt set the global!"
##if TestClass().TestMethod2("Hi there") != "Hi there":
##    raise RuntimeError, "TestMethod2 result was wrong!"
