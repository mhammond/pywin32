Notes about GenIL:
-------------------------
These are internal, developer notes.

* The test suite demonstrates the complete functionality we can
handle.  We have good coverage of the language and runtime, but
still poor coverage of the built modules and functions.

* Variable handling is quite different from genc - instead of returning
a "dict object" where the variable is to be stored, I introduced the
concept of "W_GLOBAL" and "W_LOCAL" for the "where" part.  In either of
these cases, the corresponding "type" value is the name of the variable.

* Global variables are stored in __dict__.  Local variables use
Lightning locals.  In all cases, these are System.Object, and never the
"native" type.  Eg, "a=1" will create a System.Object with a boxed
integer.

* Created a "Python.Runtime" module which is used by the generated IL.
This has a first implementation of a Dictionary, List and a few
exceptions.  Simply change into the "PyRuntime" directory and run nmake.

* The definition of a function still needs some work.
* What I write before: ... causes a "System.Delegate" to be stored.
No optimization of function calls is made (I did have "direct" calls,
but decided to remove them for the sake of code sanity).  All calls are
made indirectly through the delegate.  This needs some serious thought -
creating delegates is a PITA - you need a new delegate type for each
function you need to call.

* Classes havent really been handled at all, but basic, non-functioning
support exists.  A class definition causes a "System.Type" to be stored.

* Exception handling is pretty basic, but does work.  In particular, need
better support for determining the exception to catch.

* Would be nice to be able to hide entries from the traceback.

* How do other languages use a Python class instance attributes.  Eg:
# Implemented in Python
class Foo:
  def __init__(self):
    self.bar = "bar"
// Called From C++
Foo *f = new Foo();
Console::WriteLine(f->bar);

ie, how do I perform dynamic reflection??

* VarArgs, default arg values.  Something isnt working correctly here - either
a design fault in COM+ (less likely) or I dont understand it (more likely)

* global functions vs static methods - all my funcs are static methods.



  
