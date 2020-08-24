# python language, type, version and compat tools for pywin

from __future__ import absolute_import

import sys
import types

PY2 = sys.version_info[0] == 2
PY3 = not PY2
if PY3:
    unichr = chr
    string_types = str,
    basestring = (str, bytes)
    integer_types = int,
    long_type = int
    class_types = type,
    unicode_type = text_type = str
    binary_type = bytes    
    input = input
    import builtins
else:
    unichr = unichr
    string_types = basestring,
    basestring = basestring
    integer_types = (int, long)
    long_type = long
    class_types = (type, types.ClassType)
    unicode_type = text_type = unicode
    binary_type = str
    input = raw_input
    import __builtin__ as builtins
print_ = __builtins__.get('print')

class DictObj(object):
	"""exposes a dictionary as object - adding optional keyword args"""
	def __init__(self, d=None, **kw):
		if d is not None: self.__dict__ = d
		if kw: self.__dict__.update(kw)
class Object(object):
    """universal object with .__dict__ for ad-hoc usage with dictionary and
    keyword init and dict proxy methods"""    
    def __init__(self, _d=None, _pop=None, **kw):
        if _d: self.__dict__.update(_d)
        if kw: self.__dict__.update(kw)
        if _pop:
            if isinstance(_pop, basestring): _pop = _pop,
            for x in _pop: self.__dict__.pop(x, None)
    def __repr__(self):
        return '%s(%r)' % (self.__class__.__name__, self.__dict__)
    def print_(self):
        print_("<Object 0x%X> --- print ----" % id(self))
        for k, v in sorted(self.__dict__.items()):
            print_("  %s = %s" % (k, v))
    def get(self, k, default=None):
        return self.__dict__.get(k, default)
    def setdefault(self, k, default=None):
        return self.__dict__.setdefault(k, default)
    def __getitem__(self, k):
        return self.__dict__[k]
    def __contains__(self, k):
        return k in self.__dict__
    def __setitem__(self, k, v):
        self.__dict__[k] = v
    ##def __eq__(self, other):
    def eq_simple(self, other):
        return self.__class__ == other.__class__ and self.__dict__ == other.__dict__

def _import_module(name):
    """Import module, returning the module after the last dot."""
    __import__(name)
    return sys.modules[name]
class _LazyDescr(object):
    def __init__(self, name):
        self.name = name
    def __get__(self, obj, tp):
        result = self._resolve()
        setattr(obj, self.name, result)  # Invokes __set__.
        try:
            # This is a bit ugly, but it avoids running this again by
            # removing this descriptor.
            delattr(obj.__class__, self.name)
        except AttributeError:
            pass
        return result
class MovedModule(_LazyDescr):
    __path__ = []
    def __init__(self, name, old, new=None):
        super(MovedModule, self).__init__(name)
        if PY3:
            if new is None:
                new = name
            self.mod = new
        else:
            self.mod = old
    def _resolve(self):
        return _import_module(self.mod)
    def __getattr__(self, attr):
        _module = self._resolve()
        value = getattr(_module, attr)
        setattr(self, attr, value)
        return value
class MovedAttribute(_LazyDescr):
    def __init__(self, name, old_mod, new_mod, old_attr=None, new_attr=None):
        super(MovedAttribute, self).__init__(name)
        if PY3:
            if new_mod is None:
                new_mod = name
            self.mod = new_mod
            if new_attr is None:
                if old_attr is None:
                    new_attr = name
                else:
                    new_attr = old_attr
            self.attr = new_attr
        else:
            self.mod = old_mod
            if old_attr is None:
                old_attr = name
            self.attr = old_attr
    def _resolve(self):
        module = _import_module(self.mod)
        return getattr(module, self.attr)
_moved_attributes = [
    MovedAttribute("input", "__builtin__", "builtins", "raw_input", "input"),
    MovedAttribute("map", "itertools", "builtins", "imap", "map"),
    MovedAttribute("range", "__builtin__", "builtins", "xrange", "range"),
    MovedAttribute("reduce", "__builtin__", "functools"),
    MovedAttribute("xrange", "__builtin__", "builtins", "xrange", "range"),
    MovedAttribute("zip", "itertools", "builtins", "izip", "zip"),
    ##MovedModule("builtins", "__builtin__"),
    MovedModule("_thread", "thread", "_thread"),
    MovedModule("queue", "Queue"),
    MovedModule("reprlib", "repr"),
]
if sys.platform == "win32":
    _moved_attributes += [
        MovedModule("winreg", "_winreg"),
    ]
class _Moves(types.ModuleType):
    def __init__(self):
        self.__path__ = []
        self.__name__ = __name__ + '.moves'
        sys.modules[self.__name__] = self
moves = _Moves()
for attr in _moved_attributes:
    setattr(_Moves, attr.name, attr)
del attr

if PY3:
    def reraise(tp, value, tb=None):
        try:
            if value is None:
                value = tp()
            if value.__traceback__ is not tb:
                raise value.with_traceback(tb)
            raise value
        finally:
            value = None
            tb = None
else:
    exec("""\
def reraise(tp, value, tb=None):
    try:
        raise tp, value, tb
    finally:
        tb = None
""")

# Turn this module into a package.
__path__ = []
__package__ = __name__
