# This module exists to create the "best" dispatch object for a given
# object.  If "makepy" support for a given object is detected, it is
# used, otherwise a dynamic dispatch object.

# Note that if the unknown dispatch object then returns a known
# dispatch object, the known class will be used.  This contrasts
# with dynamic.Dispatch behaviour, where dynamic objects are always used.
import dynamic, CLSIDToClass, pythoncom

def Dispatch(dispatch, userName = None, resultCLSID = None, typeinfo = None, UnicodeToString=1, clsctx = pythoncom.CLSCTX_SERVER):
  """Creates a Dispatch based COM object.
  """
  dispatch, userName = dynamic._GetGoodDispatchAndUserName(dispatch,userName,clsctx)
  if resultCLSID is None:
    try:
      typeinfo = dispatch.GetTypeInfo()
      resultCLSID = str(typeinfo.GetTypeAttr()[0])
    except pythoncom.com_error:
      pass
  if resultCLSID is not None:
    try:
      return CLSIDToClass.GetClass(resultCLSID)(dispatch)
    except KeyError: # We dont know this CLSID yet
      # Attempt to load generated module support
      # This may load the module, and make it available
      try:
        import gencache
        if gencache.GetModuleForCLSID(resultCLSID) is not None:
          try:
            return CLSIDToClass.GetClass(resultCLSID)(dispatch)
          except KeyError: # still dont know it?
            pass
      except ImportError:
        # no gencache avail - thats OK!
        pass

  # Return a "dynamic" object - best we can do!
  return dynamic.Dispatch(dispatch, userName, CDispatch, typeinfo, UnicodeToString=UnicodeToString,clsctx=clsctx)

def DispatchEx(clsid, machine=None, userName = None, resultCLSID = None, typeinfo = None, UnicodeToString=1, clsctx = None):
  """Creates a Dispatch based COM object on a specific machine.
  """
  # If InProc is registered, DCOM will use it regardless of the machine name 
  # (and regardless of the DCOM config for the object.)  So unless the user
  # specifies otherwise, we exclude inproc apps when a remote machine is used.
  if clsctx is None:
    clsctx = pythoncom.CLSCTX_SERVER
    if machine is not None: clsctx = clsctx & ~pythoncom.CLSCTX_INPROC
  if machine is None:
    serverInfo = None
  else:
    serverInfo = (machine,)          
  if userName is None: userName = clsid
  dispatch = pythoncom.CoCreateInstanceEx(clsid, None, clsctx, serverInfo, (pythoncom.IID_IDispatch,))[0]
  return Dispatch(dispatch, userName, resultCLSID, typeinfo, UnicodeToString=UnicodeToString, clsctx=clsctx)

class CDispatch(dynamic.CDispatch):
  """The dynamic class used as a last resort.
  """
  def _wrap_dispatch_(self, ob, userName = None, returnCLSID = None, UnicodeToString = 1):
    return Dispatch(ob, userName, returnCLSID,None,UnicodeToString)

class Constants:
  """A container for generated COM constants.
  """
  def __init__(self):
    self.__dicts__ = [] # A list of dictionaries
  def __getattr__(self, a):
    for d in self.__dicts__:
      if d.has_key(a):
        return d[a]
    raise AttributeError, a

# And create an instance.
constants = Constants()
