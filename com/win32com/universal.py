# Code that packs and unpacks the Univgw structures.

# See if we have a special directory for the binaries (for developers)
import types
import pythoncom
from win32com.client import gencache

com_error = pythoncom.com_error
_univgw = pythoncom._univgw

# Make it clear to the user they are playing with fire ATM ;-)
msg = "win32com.universal is a very new module - it probably has bugs, and the interface may change in the future.  Use at your own risk!"
try:
    import warnings
    warnings.warn(msg)
except ImportError:
    print msg


def RegisterInterfaces(typelibGUID, lcid, major, minor, interface_names = None):
    # First see if we have makepy support.  If so, we can probably satisfy the request without loading the typelib.
    try:
        mod = gencache.GetModuleForTypelib(typelibGUID, lcid, major, minor)
    except ImportError:
        mod = None
    if mod is None:
        import win32com.client.build
        # Load up the typelib and build (but don't cache) it now
        tlb = pythoncom.LoadRegTypeLib(typelibGUID, major, minor, lcid)
        typecomp_lib = tlb.GetTypeComp()
        for name in interface_names:
            type_info, type_comp = typecomp_lib.BindType(name, )
            # If we got back a Dispatch interface, convert to the real interface.
            attr = type_info.GetTypeAttr()
            if attr.typekind == pythoncom.TKIND_DISPATCH:
                refhtype = type_info.GetRefTypeOfImplType(-1)
                type_info = type_info.GetRefTypeInfo(refhtype)
                attr = type_info.GetTypeAttr()
            item = win32com.client.build.VTableItem(type_info, attr, type_info.GetDocumentation(-1))
            _doCreateVTable(item.clsid, item.python_name, item.bIsDispatch, item.vtableFuncs)
    else:
        # Cool - can used cached info.
        if not interface_names:
            interface_names = mod.VTablesNamesToCLSIDMap.keys()
        for name in interface_names:
            try:
                iid = mod.VTablesNamesToCLSIDMap[name]
            except KeyError:
                raise ValueError, "Interface '%s' does not exist in this cached typelib" % (name,)
#            print "Processing interface", name
            sub_mod = gencache.GetModuleForCLSID(iid)
            is_dispatch = getattr(sub_mod, name + "_vtables_dispatch_")
            method_defs = getattr(sub_mod, name + "_vtables_")
            # And create the univgw defn
            _doCreateVTable(iid, name, is_dispatch, method_defs)

def _doCreateVTable(iid, interface_name, is_dispatch, method_defs):
    defn = Definition(iid, is_dispatch, method_defs)
    vtbl = _univgw.CreateVTable(defn, is_dispatch)
    _univgw.RegisterVTable(vtbl, iid, interface_name)

def _CalcTypeSize(typeTuple):
    t = typeTuple[0]
    if t & pythoncom.VT_BYREF:
        # Its a pointer.
        cb = _univgw.SizeOfVT(pythoncom.VT_PTR)[1]
    elif t == pythoncom.VT_RECORD:
        try:
            import warnings
            warnings.warn("assuming records always pass pointers (they wont work for other reasons anyway!")
        except ImportError:
            print "warning: assuming records always pass pointers (they wont work for other reasons anyway!"
        cb = _univgw.SizeOfVT(pythoncom.VT_PTR)[1]
        #cb = typeInfo.GetTypeAttr().cbSizeInstance
    else:
        cb = _univgw.SizeOfVT(t)[1]
    return cb

class Arg:
    def __init__(self, arg_info, name = None):
        self.name = name
        self.vt, self.inOut, self.GUID = arg_info
        self.size = _CalcTypeSize(arg_info)
        # Offset from the beginning of the arguments of the stack.
        self.offset = 0

class Method:
    def __init__(self, method_info, isEventSink=0):
        name, dispid, arg_defs, ret_def, names = method_info
        self.dispid = dispid
        # We dont use this ATM.
#        self.ret = Arg(ret_def)
        if isEventSink and name[:2] != "On":
            name = "On%s" % name
        self.name = name
        cbArgs = 0
        self.args = []
        for argDesc in arg_defs:
            arg = Arg(argDesc)
            arg.offset = cbArgs
            cbArgs = cbArgs + arg.size
            self.args.append(arg)
        self.cbArgs = cbArgs
        self._gw_in_args = self._GenerateInArgTuple()
        self._gw_out_args = self._GenerateOutArgTuple()

    def _GenerateInArgTuple(self):
        # Given a method, generate the in argument tuple
        l = []
        for arg in self.args:
            if arg.inOut & pythoncom.PARAMFLAG_FIN or \
                 arg.inOut == 0:
                l.append((arg.vt, arg.offset, arg.size))
        return tuple(l)

    def _GenerateOutArgTuple(self):
        # Given a method, generate the out argument tuple
        l = []
        for arg in self.args:
            if arg.inOut & pythoncom.PARAMFLAG_FOUT or \
               arg.inOut & pythoncom.PARAMFLAG_FRETVAL or \
               arg.inOut == 0:
                l.append((arg.vt, arg.offset, arg.size))
        return tuple(l)

class Definition:
    def __init__(self, iid, is_dispatch, method_defs):
        self._iid = iid
        self._methods = []
        self._is_dispatch = is_dispatch
        for info in method_defs:
            entry = Method(info)
            self._methods.append(entry)
    def iid(self):
        return self._iid
    def vtbl_argsizes(self):
        return map(lambda m: m.cbArgs, self._methods)
    def dispatch(self, ob, index, argPtr,
                 ReadFromInTuple=_univgw.ReadFromInTuple,
                 WriteFromOutTuple=_univgw.WriteFromOutTuple):
        "Dispatch a call to an interface method."
#        import pywin.debugger;pywin.debugger.brk()
        meth = self._methods[index]
        # Infer S_OK if they don't return anything bizarre.
        hr = 0 
        args = ReadFromInTuple(meth._gw_in_args, argPtr)
        # Ensure the correct dispid is setup
        ob._dispid_to_func_[meth.dispid] = meth.name
        retVal = ob._InvokeEx_(meth.dispid, 0, pythoncom.DISPATCH_METHOD, args, None, None)
        # None is an allowed return value stating that
        # the code doesn't want to touch any output arguments.
        if type(retVal) == types.TupleType: # Like win32com, we special case a tuple.
            # However, if they want to return a specific HRESULT,
            # then they have to return all of the out arguments
            # AND the HRESULT.
            if len(retVal) == len(meth._gw_out_args) + 1:
                hr = retVal[0]
                retVal = retVal[1:]
            else:
                raise TypeError, "Expected %s return values, got: %s" % (len(meth._gw_out_args) + 1, len(retVal))
        else:
            retVal = [retVal]
            retVal.extend([None] * (len(meth._gw_out_args)-1))
            retVal = tuple(retVal)
        WriteFromOutTuple(retVal, meth._gw_out_args, argPtr)
        return hr
