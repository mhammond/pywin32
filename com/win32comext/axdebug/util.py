# Utility function for wrapping objects.  Centralising allows me to turn
# debugging on and off for the entire package in a single spot.

import os
import sys

import win32api
import win32com.server.util
import winerror
from win32com.server.exception import Exception

try:
    os.environ["DEBUG_AXDEBUG"]
    debugging = 1
except KeyError:
    debugging = 0


def trace(*args):
    if not debugging:
        return
    print(str(win32api.GetCurrentThreadId()) + ":", end=" ")
    for arg in args:
        print(arg, end=" ")
    print()


# The AXDebugging implementation assumes that the returned COM pointers are in
# some cases identical.  Eg, from a C++ perspective:
# p->GetSomeInterface( &p1 );
# p->GetSomeInterface( &p2 );
# p1==p2
# By default, this is _not_ true for Python.
# (Now this is only true for Document objects, and Python
# now does ensure this.


def _wrap_nodebug(object, iid):
    return win32com.server.util.wrap(object, iid)


def _wrap_debug(object, iid):
    import win32com.server.policy

    dispatcher = win32com.server.policy.DispatcherWin32trace
    return win32com.server.util.wrap(object, iid, useDispatcher=dispatcher)


if debugging:
    _wrap = _wrap_debug
else:
    _wrap = _wrap_nodebug


def RaiseNotImpl(who=None):
    if who is not None:
        print(f"********* Function {who} Raising E_NOTIMPL  ************")

    # Print a sort-of "traceback", dumping all the frames leading to here.
    try:
        1 / 0
    except:
        frame = sys.exc_info()[2].tb_frame
    while frame:
        print(f"File: {frame.f_code.co_filename}, Line: {frame.f_lineno}")
        frame = frame.f_back

    # and raise the exception for COM
    raise Exception(scode=winerror.E_NOTIMPL)


import win32com.server.policy


class Dispatcher(win32com.server.policy.DispatcherWin32trace):
    def __init__(self, policyClass, object):
        win32com.server.policy.DispatcherTrace.__init__(self, policyClass, object)
        import win32traceutil  # Sets up everything.

    # print(f"Object with win32trace dispatcher created ({object})")

    def _QueryInterface_(self, iid):
        rc = win32com.server.policy.DispatcherBase._QueryInterface_(self, iid)
        # if not rc:
        #     self._trace_(f"in _QueryInterface_ with unsupported IID {IIDToInterfaceName(iid)} ({iid})\n")
        return rc

    def _Invoke_(self, dispid, lcid, wFlags, args):
        print(
            "In Invoke with",
            dispid,
            lcid,
            wFlags,
            args,
            "with object",
            self.policy._obj_,
        )
        try:
            rc = win32com.server.policy.DispatcherBase._Invoke_(
                self, dispid, lcid, wFlags, args
            )
            # print("Invoke of", dispid, "returning", rc)
            return rc
        except Exception:
            t, v, tb = sys.exc_info()
            tb = None  # A cycle
            scode = v.scode
            try:
                desc = " (" + str(v.description) + ")"
            except AttributeError:
                desc = ""
            print(f"*** Invoke of {dispid} raised COM exception 0x{scode:x}{desc}")
        except:
            print(f"*** Invoke of {dispid} failed:")
            typ, val, tb = sys.exc_info()
            import traceback

            traceback.print_exception(typ, val, tb)
            raise
