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

import string
import os

OT_STRING = 1
OT_BOOL = 2
OT_INT = 3
OT_MULTI = 0x8000
OT_TYPE_MASK = 0xFF

class OptionsException(Exception):
    pass
class AmbiguousOption(OptionsException):
    def __init__(self, opt, poss):
        mess = "Option '/%s' could be any of %s" % (opt, string.join(poss, ", "))
        OptionsException.__init__(self, mess)

class UnknownOption(OptionsException):
    def __init__(self, opt):
        mess = "Unknown option '/%s'" % (opt,)
        OptionsException.__init__(self, mess)

options = {
    'warning-level':    ('The warning level', 2, OT_INT),
    'output-file':      ('The file holding the resulting module (and possibly assembly)', None, OT_STRING),
    'dll':              ("Creates a DLL instead of a .EXE", 0, OT_BOOL),
    'print-tree':       ("Prints the AST without compiling the source file", 0, OT_BOOL),
    'transformed-tree': ("Prints the transformed AST without compiling of the source file", 0, OT_BOOL),
    'debug-info':       ("Emits symbolic debug information", 0, OT_BOOL),
    'source-file':      ("The name of the source file to report in the debug info", None, OT_STRING),
    'verbose-level':    ("The verbosity of the compiler output", 1, OT_INT),
    'module-name':      ("The name of the .NET module generated", None, OT_STRING),
    'profile-compiler': ("Profiles the compiler over the given source file", None, OT_INT),
    'assembly-desc':    ("Description for the generated assembly", None, OT_STRING),
    'assembly-alias':   ("Default alias for the generated assembly", None, OT_STRING),
    'assembly-keyfile': ("Filename holding the assembly key pair", None, OT_STRING),
    'assembly-name' : ("The assembly name written to the assembly", None, OT_STRING),
    'assembly-fullname':   ("Full name for the generated assembly", None, OT_STRING),
    'assembly-filename': ("The assembly file name - overrides /output-file", None, OT_STRING),
    'reference': ("Reference metadata from the named file", None, OT_MULTI),
    'no-python-runtime' : ("Dont add a reference to the standard Python runtime", 0, OT_BOOL)
}

def GetOptionDescriptions():
    ret = []
    for name, (desc, default, typ) in options.items():
        ret.append("/%-20s - %s" % (name, desc))
    return ret

class Options:
    def __init__(self):
        self.__opts = {}
        for name in options.keys():
            self._ResetOption(name)

    def _ResetOption(self, name):
        desc, default, typ = options[name]
        if typ & OT_MULTI:
            assert (typ & OT_TYPE_MASK & OT_BOOL) == 0, "Can't have multi-bool!"
            if default is None:
                self.__opts[name] = []
            else:
                self.__opts[name] = default[:] # Copy the default, dont reference it!
        else:
            self.__opts[name] = default

    def ResetOptions(self, *options):
        for opt in options:
            self._ResetOption(opt)

    def SetOption(self, opt_name, opt_val):
        matching = []
        for name in options.keys():
            if string.find(name, opt_name)==0:
                matching.append(name)
        if len(matching)==0:
            raise UnknownOption(opt_name)
        if len(matching)>1:
            raise AmbiguousOption(opt_name, matching)
        setattr(self, matching[0], opt_val)

    def __getattr__(self, attr):
        try:
            attr = string.replace(attr, "_", "-")
            return self.__opts[attr]
        except KeyError:
            raise AttributeError, "Invalid option '%s'" % (attr,)

    def __setattr__(self, attr, val):
        if attr[:1]=='_':
            self.__dict__[attr] = val
            return
        attr = string.replace(attr, "_", "-")
        if not self.__opts.has_key(attr):
            raise AttributeError, "Invalid option '%s'" % (attr,)
        # XXX - Need to check the flags!
        desc, default, typ = options[attr]
        if val is None:
            # No value for a BOOL toggles it.
            if typ & OT_TYPE_MASK == OT_BOOL:
                val = not self.__opts[attr]
            # No value for an int increments it!
            elif typ & OT_TYPE_MASK == OT_INT:
                val = self.__opts.get(attr,0)+1
        else:
            if typ & OT_STRING:
                val = os.path.expandvars(val)
        if typ & OT_MULTI:
            existing = self.__opts.get(attr, [])
            existing.append(val)
            self.__opts[attr] = existing
        else:
            self.__opts[attr] = val
