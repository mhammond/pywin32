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

# gen_namespaces - utility namespace classes for GenIL (but hopefully to become base classes for both genc and genIL.

from genil_con import *

class Namespace:
  "Represents a set of local/global namespaces."
  def __init__(self, context):
    self.variables = {}
    self.context = context

  def canUseGlobalDefaultVar(self):
    """Return 1 if the caller can use global defaults for functions
    Return 0 if the caller must create a glue lambda."""
    # As we're in the global namespace, always allow static defaults
    return 1

  def allowPotentialSpecialBuiltin(self, name):
    """Determine if our host can implement special handling
    for the built-in named 'name'"""
    # If 'name' is a declared global, or exists in the root namespace
    # then we're not a builtin
    return not self.variables.has_key(name)

  def lookupInternal(self, name, scope):
    return scope
  
  def lookup(self, name):
    """Lookup a name for access.

    Returns where the variable lives (ie, local or global)
    """
    return self.lookupInternal(name, W_GLOBAL)

  def assignInternal(self, name, scope):
    self.variables[name] = 1
    return scope

  def assign(self, name):
    """Assign a value to the given name.

   The variable category (local, global) is returned.
    """
    return self.assignInternal(name, W_GLOBAL)

  def makeglobal(self, name):
    pass

class LocalNamespace(Namespace):
  def __init__(self, global_ns, context):
    Namespace.__init__(self, context)
    self.declared_globals = {}
    self.global_ns = global_ns

  def allowPotentialSpecialBuiltin(self, name):
    "Determine if 'name' is a potential builtin that we care about"
    if not self.global_ns.allowPotentialSpecialBuiltin(name):
      return 0
    return Namespace.allowPotentialSpecialBuiltin(self, name)

  def lookup(self, name):
    if self.declared_globals.has_key(name):
      return W_GLOBAL

    if self.variables.has_key(name):
      return W_LOCAL
    # Havent seen this - assume global
    # should lookup the parent namespace and offer some sort of
    # warning when not found?
    return W_GLOBAL

  def assign(self, name):
    if self.declared_globals.has_key(name):
      return W_GLOBAL
    return Namespace.assignInternal(self, name, W_LOCAL)

  def makeglobal(self, name):
    self.declared_globals[name] = 1
