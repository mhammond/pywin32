# This module exists to create the "best" dispatch object for a given
# object.  If "makepy" support for a given object is detected, it is
# used, otherwise a dynamic dispatch object.

# Note that if the unknown dispatch object then returns a known
# dispatch object, the known class will be used.  This contrasts
# with dynamic.Dispatch behaviour, where dynamic objects are always used.
import dynamic, CLSIDToClass, pythoncom
import pywintypes

def __WrapDispatch(dispatch, userName = None, resultCLSID = None, typeinfo = None, \
                  UnicodeToString = 1, clsctx = pythoncom.CLSCTX_SERVER):
  """
    Helper function to return a makepy generated class for a CLSID if it exists,
    otherwise cope by using CDispatch.
  """
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


def GetObject(Pathname = None, Class = None, clsctx = None):
  """
    Mimic VB's GetObject() function.

    ob = GetObject(Class = "ProgID") or GetObject(Class = clsid) will
    connect to an already running instance of the COM object.
    
    ob = GetObject(r"c:\blah\blah\foo.xls") (aka the COM moniker syntax)
    will return a ready to use Python wrapping of the required COM object.

    Note: You must specifiy one or the other of these arguments. I know
    this isn't pretty, but it is what VB does. Blech. If you don't
    I'll throw ValueError at you. :)
    
    This will most likely throw pythoncom.com_error if anything fails.
  """
  resultCLSID = None
  
  if clsctx is None:
    clsctx = pythoncom.CLSCTX_ALL
    
  if (Pathname is None and Class is None) or \
     (Pathname is not None and Class is not None):
    raise ValueError, "You must specify a value for Pathname or Class, but not both."

  if Class is not None:
    return GetActiveObject(Class, clsctx)
  else:
    return Moniker(Pathname, clsctx)    

def GetActiveObject(Class, clsctx = pythoncom.CLSCTX_ALL):
  """
    Python friendly version of GetObject's ProgID/CLSID functionality.
  """  
  resultCLSID = pywintypes.IID(Class)
  dispatch = pythoncom.GetActiveObject(resultCLSID)
  dispatch = dispatch.QueryInterface(pythoncom.IID_IDispatch)
  return __WrapDispatch(dispatch, Class, resultCLSID = resultCLSID, clsctx = pythoncom.CLSCTX_ALL)

def Moniker(Pathname, clsctx = pythoncom.CLSCTX_ALL):
  """
    Python friendly version of GetObject's moniker functionality.
  """
  moniker, i, bindCtx = pythoncom.MkParseDisplayName(Pathname)
  dispatch = moniker.BindToObject(bindCtx, None, pythoncom.IID_IDispatch)
  return __WrapDispatch(dispatch, Pathname, clsctx = clsctx)
  
def Dispatch(dispatch, userName = None, resultCLSID = None, typeinfo = None, UnicodeToString=1, clsctx = pythoncom.CLSCTX_SERVER):
  """Creates a Dispatch based COM object.
  """
  dispatch, userName = dynamic._GetGoodDispatchAndUserName(dispatch,userName,clsctx)
  return __WrapDispatch(dispatch, userName, resultCLSID, typeinfo, UnicodeToString, clsctx)

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
  """
    The dynamic class used as a last resort.
    The purpose of this overriding of dynamic.CDispatch is to perpetuate the policy
    of using the makepy generated wrapper Python class instead of dynamic.CDispatch
    if/when possible.
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
