# Portions Copyright 1999-2000 Microsoft Corporation.
# Portions Copyright 1997-1999 Greg Stein and Bill Tutt.
#
# This source code may be freely distributed, as long as all
# copyright information remains in place.
#
# See also the copyrights for the version of Python you are using.
#
# Implemented 1999-2000 by Mark Hammond (MarkH@ActiveState.com)
# and Greg Stein (gstein@lyra.org)
#
# See http://www.ActiveState.com/.NET for the latest versions.

#
# IL compiler
#


import compiler # The "standard" Python compiler package.
from compiler.consts import *

import string
import os
import sys
import types
from pywintypes import UnicodeType
from win32com import client
import winerror
import pythoncom
import gen_namespaces
import linecache

# Hack into the implementation - later win32com.client.dynamic versions have this
import winerror
client.dynamic.ERRORS_BAD_CONTEXT.append(winerror.E_INVALIDARG)
# HACK - almost all "_oleobj_" references in here are to fix win32all-139/ActivePy210 and earlier.

from genil_con import * # Bring all the constants in.

from win32com.client import gencache, constants

glue = gencache.EnsureDispatch("P2IL2.CORGlue", bForDemand = 1)
#glue = client.Dispatch("P2IL2.CORGlue")

NoWhere = "dont show the source file!"
ENOENT = 2

g_typelib = None

def LoadCOMPlusTypeLib():
    # The COM+ type lib isnt actually registered!
    try:
        global g_typelib
        tlb = g_typelib = pythoncom.LoadTypeLib("mscorlib.tlb")
    except pythoncom.com_error, details:
        print "ERROR:  Can not load the COM+ type library"
        print details
        sys.exit(1)
    try:
        # Only version of the win32com support with this function are suitable :-(
        check = gencache.ForgetAboutTypelibInterface
    except AttributeError:
        check = None
    if check is not None:
        # If "ForgetAbout..." is there, this is always there...
        return gencache.EnsureModuleForTypelibInterface(tlb, bForDemand = 1)
    # OK - running with an old win32com module - do this work ourselves.
    # (This can be removed later, when we can insist on later win32com builds)
    tla = tlb.GetLibAttr()
    guid = tla[0]
    lcid = tla[1]
    major = tla[3]
    minor = tla[4]
    modName = gencache.GetGeneratedFileName(guid, lcid, major, minor)
    try:
        mod = gencache.GetModuleForTypelib(guid, lcid, major, minor)
    except ImportError:
        # OK - build it.
        print "Loading and caching .NET type library.  This may take a few seconds.  Please wait..."
        gen_path = gencache.GetGeneratedFileName(guid, lcid, major, minor)
        # Ensure we generate in the correct dir.
        gen_path = os.path.join(gencache.GetGeneratePath(), gen_path)
        try:
            os.mkdir(gen_path)
        except os.error:
            pass
        this_name = os.path.join( gen_path, "__init__.py")
        f = open(this_name, "wt")
        try:
            from win32com.client import genpy, makepy
            progress = makepy.SimpleProgress(0)
            gen = genpy.Generator(tlb, "mscorlib.tlb", progress)
            gen.generate(f, 1)

        finally:
            f.close()

        gencache.AddModuleToCache(guid, lcid, major, minor)

        # And do the children we need now (the win32com support can actually
        # demand-generate these...
        generate_children = string.split("""_Assembly _AppDomain _AssemblyName
                                                               _AssemblyBuilder _ModuleBuilder _Type
                                                               _TypeBuilder _FieldBuilder _MethodBuilder
                                                               _ILGenerator _FieldInfo _MethodInfo _LocalBuilder
                                                               _ParameterInfo _ConstructorInfo _PropertyInfo
                                                               _ConstructorBuilder _PropertyBuilder _ParameterBuilder
                                                               _StrongNameKeyPair
                                                               """)
        for child in generate_children:

            # Need to create a new generator per run :-(
            gen = genpy.Generator(tlb, "mscorlib.tlb", progress)
            dir_name = gencache.GetGeneratedFileName(guid, lcid, major, minor)
            dir_path_name = os.path.join(gencache.GetGeneratePath(), dir_name)

            gen.generate_child(child, dir_path_name)
            __import__("win32com.gen_py." + dir_name + "." + child)

        # All finished.
        progress.Close()

LoadCOMPlusTypeLib()

def get_node_lineno(node):
    if node.lineno is None:
        try:
            return node._Node__children[1][0].lineno
        except IndexError:
            return None
        except TypeError:
            import pywin.debugger;pywin.debugger.pm()
    else:
        return node.lineno

class error(Exception):
    def __init__(self, msg):
        self.msg = msg
        Exception.__init__(self, msg)

class source_error(error):
    def __init__(self, msg):
        error.__init__(self, msg)

class emit_error(source_error):
    def __init__(self, desc, com_error, com_tb):
        self.com_tb = com_tb
        hr, msg, exc, arg = com_error
        if exc is not None:
            msg = exc[2]
        source_error.__init__(self, desc  + "\n" + msg)

def FlattenTuple(tup):
    # Used to flatten arguments lists containing tuples.  Eg:
    # def Foo( a, (b, (c,) ) ), x, y, z )
    ret = []
    for i in tup:
        if type(i)==types.TupleType:
            ret.extend(FlattenTuple(i))
        else:
            ret.append(i)
    return ret

# Debug helper - return a string signature for the info.
def format_method_info(info):
    param_infos = info.GetParameters()
    param_infos = map(lambda info : info.ParameterType.FullName, param_infos)
    param_sig = string.join(param_infos, ", ")
    name = info.Name
    try:
        ret_type = info.ReturnType.FullName
    except pythoncom.com_error:
        ret_type = None
    return "%s(%s)->%s" % (name, param_sig, ret_type)


# dottedname helpers.
class __NotADottedName(Exception): pass
def _build_dotted_name(node):
    if node[0] == "getattr":
        ob = node[1]
        attr = node[2]
        return _build_dotted_name(ob) + [attr]
    elif node[0] == "name":
        return [node[1]]
    else:
        raise __NotADottedName

# Return a list of strings if the node refers to a dotted name.
# eg, transforms: ('getattr', ('getattr', ('name', 'COR'), 'System'), 'Exception')
# into: ["COR", "System", "Exception"]
def build_dotted_name(node):
    try:
        return _build_dotted_name(node)
    except __NotADottedName:
        return []


# These lists should be with the other constants?
native_types = [T_COR_INT, T_COR_BOOL, T_COR_STRING, T_COR_CHAR, T_COR_DOUBLE, T_COR_OBJECT, T_PYOBJECT]
def find_best_methods(name, infos, input_arg_types, only_static = 0, exact = 0):
    trace_results = 0
    exact_match = None
    best_matches = []
    num_input_args = len(input_arg_types)
    if name is not None:
        infos = filter(lambda info, name=name: info.Name == name, infos)

    if trace_results:
        print "Matching function '%s' for types %s:" % (name, map(lambda t: t.FullName,input_arg_types))
        print "Candidates are:"
        for m in infos:
            print format_method_info(m)

    # Filter static
    if only_static:
        flt = lambda info: info.IsStatic
    else:
        flt = lambda info: not info.IsStatic
    infos = filter(flt, infos)

    if trace_results:
        print "After static filter:"
        for m in infos:
            print format_method_info(m)

    for info in infos:
        pis = info.GetParameters()
        # Do basic param number matching, allowing for default args.
        needed_params = 0
        for pi in pis:
                if not pi.IsOptional:
                    needed_params = needed_params + 1

        if num_input_args > len(pis) or num_input_args < needed_params:
                continue
        pis_look = pis[:num_input_args]
        for pi, at in map(None, pis_look, input_arg_types):
            pt = pi.ParameterType
            if not glue.Type_Equals(pt, at):
                    break
        else:
            exact_match = info
            break

        # need better "best match" handling.
        # for now, best means "all native types"
        for pi, at in map(None, pis_look, input_arg_types):
            pt = pi.ParameterType
            ptname = pt.FullName

            if not pt.IsArray and \
                 ptname not in native_types:
                break
        else:
#            print "Found a good match:", format_method_info(m)
            best_matches.append(info)

    if trace_results:
        if exact_match is not None:
            print "Have exact match", format_method_info(exact_match)
        else:
            if exact:
                print "No exact match, but exact required - returning no infos"
            else:
                print "No exact match - returning best matches:"
                for m in best_matches:
                    print format_method_info(m)
                print "--"

    if exact_match is not None:
        return [exact_match]
    if not exact and best_matches:
        return best_matches
    return []
#    return infos

def find_compiler_directive(node, directive):
    # Node is the "stmt' node.
    assert node[0] is 'stmt', "Unexpected node"
    stmts = node[1]
    if len(stmts)==0: return
    # allow a docstring first.
    stmt = stmts[0]
    index = 0
    if len(stmt)==2 and stmt[0]=='discard' and stmt[1][0]=='const':
        index = 1
    while index < len(stmts):
        stmt = stmts[index]
        if stmt[0] != "assign" or stmt[1][0] [0]!= 'ass_name':
            return # Must be in the initial name assignments
        lhs = stmt[1]
        if len(lhs)==1 and \
             lhs[0][1]==directive and \
             lhs[0][2]=="OP_ASSIGN":
            rhs = stmt[2]
            if rhs[0]!='const':
                raise source_error("Directive '%s' is not a literal constant" % (directive))
            # whew - found it - remove the node.
            del node[1][index]
            return rhs[1]
        index = index + 1

def _fixresult(arg):
    if type(arg) in [type(()), type([])]:
        new_arg = []
        for a in arg:
            new_arg.append(_fixresult(a))
        arg = new_arg
    elif hasattr(arg, "_oleobj_") and not isinstance(arg, COMAttributeWrapper):
        arg = COMAttributeWrapper(arg)
    return arg

class COMAttributeWrapperMethod:
    def __init__(self, ob, name):
        self._obj_ = ob
        self._name_ = name
    def __call__(self, *args):
        ret = glue.call(self._obj_, self._name_, args)
        return _fixresult(ret)
        
class COMAttributeWrapper:
    def __init__(self, ob):
        assert hasattr(ob, "QueryInterface") or (ob.__class__<> self.__class__), "wrapping a wrapper!"
        self.__dict__['_obj_'] = ob
        self.__dict__['_oleobj_'] = getattr(ob, '_oleobj_', ob) # So COM wrappers pass the original as a param
    def __getattr__(self, attr):
        if attr[0]=='_':
            raise AttributeError, attr
        try:
            ret = getattr(self._obj_, attr)
        except AttributeError:
            try:
                ret = glue.getattr(self._obj_, attr)
            except pythoncom.com_error, details:
                if not details[2] or details[2][5] != winerror.DISP_E_BADPARAMCOUNT:
                    raise AttributeError, attr
                ret = COMAttributeWrapperMethod(self._obj_, attr)
                self.__dict__[attr] = ret
                return ret # no need to check result
        return _fixresult(ret)
    def __setattr__(self, attr, val):
        try:
            return setattr(self._obj_, attr, val)
        except AttributeError:
            try:
                return glue.setattr(self._obj_, attr, val)
            except pythoncom.com_error, details:
                raise AttributeError, attr

class Compiler:
    # There is a single compiler object for a single compilation step.
    # This compilation may process many source files.
    # Each source file processed will have a new generator object and
    # a new module context.  There will be exactly one assmbly context
    # per compiler.
    # This class is also the container/cache for COM+ objects.
    def __init__(self, options):
        self.actx = self.gen = None
        self.options = options
        self.corTypes = {} # Cache of COR Type objects for standard types.
        self.cachedMethodInfos = {}
        self.cachedFieldInfos = {}
        self.cachedConstructorInfos = {}

        self.extra_assemblies = None

    def getCORType(self, typestring, really_need_it = 1):
        # First see if we have cached it.
        try:
            # Fastest case since we expect many cache hits.
            return self.corTypes[typestring]
        except KeyError:
            pass
        if self.extra_assemblies is None:
            self.extra_assemblies = []
            if not self.options.no_python_runtime:
                try:
#                    self.extra_assemblies.append(glue.Assembly_LoadFrom("..\\PyRuntime\\Python.dll"))
                    self.extra_assemblies.append(glue.Assembly_LoadWithPartialName("Python"))
                except pythoncom.com_error, (hr, msg, exc, arg):
                    if exc and exc[2]: msg = exc[2]
                    raise EnvironmentError(ENOENT, "The Python runtime assembly ('Python.dll') can not be loaded: %s" % (msg,))
##            try:
##                self.extra_assemblies.append(glue.Assembly_Load("System.dll")) # Needed for System.Diagnostics.Debug
##            except pythoncom.com_error, (hr, msg, exc, arg):
##                    if exc and exc[2]: msg = exc[2]
##                    raise EnvironmentError(ENOENT, "The COM+ system assembly ('System.dll') can not be loaded: %s" % (msg,))
            for name in self.options.reference:
                try:
                    self.extra_assemblies.append(glue.Assembly_LoadFrom(name))
                except pythoncom.com_error, (hr, msg, exc, arg):
                    if exc and exc[2]: msg = exc[2]
                    raise EnvironmentError(ENOENT, "The specified DLL ('%s') can not be loaded: %s" % (name, msg))
        # OK - fetch it from COR, then cache it.
        # First see if in my assembly.
#        typ = self.actx.ass_builder.GetType_2(typestring)
    
        typ = COMAttributeWrapper(self.actx.ass_builder).GetType(typestring)
        if typ is None:
            typ = glue.GetTypeX(typestring) # WTF - why the X - see corglue.cpp?
        if typ is None:
            for a in self.extra_assemblies:
#                print "looking for", typestring, "in assembly", a.FullName
                typ = a.GetType_2(typestring)
                if typ is not None:
                    break
        if really_need_it and typ is None:
            raise source_error("Can not find the COR type '%s'" % (typestring,))
        self.corTypes[typestring] = typ
        return typ

    def _msg(self, prefix, msg, where = None):
        if where == NoWhere:
            file_info_1 = file_info_2 = ""
        else:
            if where is None:
                if self.gen is not None:
                    fname = self.gen.input
                    lineno = self.gen.lineno
                else:
                    fname = lineno = None
            else:
                fname, lineno = where
            if fname is None:
                file_info_1 = ""
                file_info_2 = "\n>[no source information available]"
            else:
                file_info_1 = "%s (%s):" % (fname, lineno)
                file_info_2 = "\n>%s" % (linecache.getline(fname, lineno)[:-1],)
        print "%s: %s %s%s" % (prefix, file_info_1, msg, file_info_2)

    def warning(self, level, msg, where = None):
        if level <= self.options.warning_level:
            prefix = "Level %d Warning" % (level,)
            self._msg(prefix, msg, where)

    def verbose(self, level, msg, where = None):
        if level <= self.options.verbose_level:
            prefix = "Note"
            self._msg(prefix, msg, where)

    def getMethodInfo(self, corObjectName, methodName, arg_types):
        try:
            return self.cachedMethodInfos[corObjectName, methodName, arg_types]
        except KeyError:
            pass
        corType = self.getCORType(corObjectName)
#    if not arg_types:
#      method_info = glue.Type_GetMethod(corType, methodName)# corType.GetMethod(methodName)
#    else:
        real_arg_types = []
        for arg in arg_types:
            cor_arg_type = self.getCORType(arg)
            real_arg_types.append(cor_arg_type)
        method_info = glue.Type_GetMethodArgs(corType, methodName, real_arg_types)
        assert method_info is not None, "Can not find the method '%s.%s(%s)" % (corObjectName, methodName, string.join(arg_types, ","))
        self.cachedMethodInfos[corObjectName, methodName, arg_types] = method_info
        return method_info

    def getFieldInfo(self, corObjectName, fieldName):
        try:
            return self.cachedFieldInfos[corObjectName, fieldName]
        except KeyError:
            pass
        corType = self.getCORType(corObjectName)
        field_info = glue.Type_GetField(corType, fieldName)
        assert field_info is not None, "Can not find the field '%s.%s" % (corObjectName, fieldName)
        self.cachedFieldInfos[corObjectName, fieldName] = field_info
        return field_info

    def getConstructorInfo(self, type_name, args):
        try:
            return self.cachedConstructorInfos[(type_name, args)]
        except KeyError:
            pass
        obj_type = self.getCORType(type_name)
        cor_args = map(self.getCORType, args)
        ctor_info = glue.Type_GetConstructor(obj_type, cor_args)
        assert ctor_info is not None, "Can't locate the constructor '%s%s'" % (type_name,args)
        self.cachedConstructorInfos[(type_name, args)] = ctor_info
        return ctor_info

    def createModule(self, source_file):
        # Work out the output file names etc.
        output_file = self.options.output_file
        if output_file is None or os.path.isdir(output_file):
            base = self.options.module_name
            if base is None:
                base = os.path.splitext(os.path.basename(self.options.source_file))[0]
            if self.actx is None: # We are creating the assembly
                if self.options.dll:
                    ext = ".dll"
                else:
                    ext=  ".exe"
            else:
                ext = ".mod" # A second or subsequent module in the assembly.
            if output_file is not None: # must be a dir!
                # Hrm - not sure what to do about module paths.  Likely that we need to force
                # them to the same dir as the assembly...
                base = os.path.abspath(os.path.join(output_file, base))
            output_file = base + ext
            self.options.output_file = output_file

        if self.actx is None: # First module - define the assembly.
            self.assembly_output_filename = self.options.assembly_filename or output_file
            self.actx = AssemblyContext(self)
        else:
            # Subsequent output names can not have paths.
            if os.path.split(self.options.output_file)[0]:
                raise error, "Module output files can not specify the path"

        self.verbose(1, "Compiling source file '%s'" % (source_file,), NoWhere)
        self.gen = Generator(self, source_file)
        try:
            self.gen.go()
        finally:
            self.gen.finalize()
            # Leave the last gen hanging around for error locations.

    def save(self):
        self.actx.finalize()
        print "Created assembly:", self.assembly_output_filename

    def finalize(self):
        self.gen = None
        self.actx = None
        global g_typelib
        if g_typelib is not None:
            if hasattr(gencache, "ForgetAboutTypelibInterface"):
                gencache.ForgetAboutTypelibInterface(g_typelib)
            g_typelib = None

    def makeModuleContext(self, filename):
        url = os.path.abspath(filename)
        module_name = self.options.module_name
        if module_name is None:
            if self.options.dll:
                module_name = os.path.splitext(os.path.basename(url))[0]
            else:
                module_name = '__main__'

        # Modules can not have paths.
        output_file = os.path.basename(self.options.output_file)
        mod = glue.AssemblyBuilder_DefineDynamicModule(self.actx.ass_builder, module_name, output_file, self.options.debug_info)
#        mod = self.actx.ass_builder.DefineDynamicModule(module_name, output_file, self.options.debug_info)
        return ModuleContext(self, mod, url, module_name, self.assembly_output_filename)

class _Opcodes:
    def __init__(self):
        self.glue = glue # client.Dispatch('P2IL.CORGlue')
    def __getattr__(self, name):
        if name[:2] == '__':
            raise AttributeError, name
        try:
            value = self.glue.GetOpCode(name)
        except pythoncom.com_error, details:
            raise AttributeError, "Can't find '%s': %s" % (name, details)
        setattr(self, name, value)
        return value

Opcodes = _Opcodes()

import funcsigs
funcsigs.glue = glue
funcsigs.Opcodes = Opcodes
funcsigs.COMAttributeWrapper = COMAttributeWrapper

# A map of integer opcodes optimized for value.
Opcodes_int = {
    0: Opcodes.Ldc_I4_0,
    1: Opcodes.ldc_i4_1,
    2: Opcodes.Ldc_I4_2,
    3: Opcodes.ldc_i4_3,
    4: Opcodes.ldc_i4_4,
    5: Opcodes.ldc_i4_5,
    6: Opcodes.ldc_i4_6,
    7: Opcodes.ldc_i4_7,
    8: Opcodes.ldc_i4_8,
    -1: Opcodes.ldc_i4_M1,
}

Opcodes_ldarg = {
    0 : Opcodes.Ldarg_0,
    1 : Opcodes.Ldarg_1,
    2 : Opcodes.Ldarg_2,
    3 : Opcodes.Ldarg_3,
}
# Value on stack already is result of compare, _then_ constant pushed.
# So comparson opcode is "val against constant" rather than "constant against val"
Compare_map = {
#        The opcode, literal to cmp against
    "==" : (Opcodes.ceq, 0),
    "<=" : (Opcodes.clt, 1),
    "<"  : (Opcodes.clt, 0),
    ">=" : (Opcodes.cgt, -1),
    ">"  : (Opcodes.cgt, 0),
    "!=" : (Opcodes.cgt_un, 0),
}

AbstractMethods_map = {
    "PyObject_Call" : ("Python.Builtins.types.PyObject", "Python.Builtins.types.PyObject[]", "Python.Builtins.types.PyObject"),
    "PyObject_FromObject" : ("System.Object",),
    "PyObject_IsTrue" : ("Python.Builtins.types.PyObject",),
    "PyObject_Print" : ("Python.Builtins.types.PyObject", "System.IO.TextWriter", "System.Int32"),
    "PyObject_DelItem" : ("Python.Builtins.types.PyObject", "Python.Builtins.types.PyObject"),
    "PyObject_SetItem" : ("Python.Builtins.types.PyObject", "Python.Builtins.types.PyObject", "Python.Builtins.types.PyObject"),
    "PyObject_Compare" : ("Python.Builtins.types.PyObject", "Python.Builtins.types.PyObject"),
    "PyObject_GetItem" : ("Python.Builtins.types.PyObject", "Python.Builtins.types.PyObject"),
    "PyObject_Length" : ("Python.Builtins.types.PyObject",),
    "PyObject_GetAttrString" : ("Python.Builtins.types.PyObject", "System.String"),
    "PyObject_GetAttr" : ("Python.Builtins.types.PyObject", "Python.Builtins.types.PyObject"),
    "PyObject_SetAttrString" : ("Python.Builtins.types.PyObject", "System.String", "Python.Builtins.types.PyObject"),
    "PyObject_SetAttr" : ("Python.Builtins.types.PyObject", "Python.Builtins.types.PyObject", "Python.Builtins.types.PyObject"),
    "PyObject_GetEnumerator" : ("Python.Builtins.types.PyObject",),
    "PyObject_Not" : ("Python.Builtins.types.PyObject",),
    "PyObject_Str" : ("Python.Builtins.types.PyObject",),
    "PyObject_Repr" : ("Python.Builtins.types.PyObject",),
    "PyObject_AsExternalObject" : ("Python.Builtins.types.PyObject", "System.Type"),
    "PyNumber_Or" : ("Python.Builtins.types.PyObject", "Python.Builtins.types.PyObject"),
    "PyNumber_Xor" : ("Python.Builtins.types.PyObject", "Python.Builtins.types.PyObject"),
    "PyNumber_And" : ("Python.Builtins.types.PyObject", "Python.Builtins.types.PyObject"),
    "PyNumber_Lshift" : ("Python.Builtins.types.PyObject", "Python.Builtins.types.PyObject"),
    "PyNumber_Rshift" : ("Python.Builtins.types.PyObject", "Python.Builtins.types.PyObject"),
    "PyNumber_Add" : ("Python.Builtins.types.PyObject", "Python.Builtins.types.PyObject"),
    "PyNumber_Subtract" : ("Python.Builtins.types.PyObject", "Python.Builtins.types.PyObject"),
    "PyNumber_Multiply" : ("Python.Builtins.types.PyObject", "Python.Builtins.types.PyObject"),
    "PyNumber_Divide" : ("Python.Builtins.types.PyObject", "Python.Builtins.types.PyObject"),
    "PyNumber_Remainder" : ("Python.Builtins.types.PyObject", "Python.Builtins.types.PyObject"),
    "PyNumber_Negative" : ("Python.Builtins.types.PyObject",),
    "PyNumber_Positive" : ("Python.Builtins.types.PyObject",),
    "PyNumber_Invert" : ("Python.Builtins.types.PyObject",),
    "PyNumber_Int" : ("Python.Builtins.types.PyObject",),
    "PyNumber_Power" : ("Python.Builtins.types.PyObject","Python.Builtins.types.PyObject","Python.Builtins.types.PyObject"),
    "PyImport_ImportModule" : ("System.String",),
    "PySequence_GetItem": ("Python.Builtins.types.PyObject", "System.Int32"),
    "PySequence_GetSlice": ("Python.Builtins.types.PyObject", "System.Int32", "System.Int32"),
    "PySequence_SetSlice": (T_PYOBJECT, "System.Int32", "System.Int32", T_PYOBJECT),
    "__CombineVarArgs":(T_PYOBJECT_ARRAY, T_PYOBJECT),
    "__Assign" : (T_PYOBJECT, T_PYOBJECT, T_PYOBJECT),
    "__SetItem" : ("Python.Builtins.types.PyObject", "Python.Builtins.types.PyObject", "Python.Builtins.types.PyObject"),
    "__SetAttr" : ("Python.Builtins.types.PyObject", "Python.Builtins.types.PyObject", "System.String"),
    "__LookupGlobal" : (T_PYOBJECT, T_COR_STRING),
}

Convert_cor_to_py_map = {
    # Some value types (eg T_COR_BOOL) are widened  before being looked up here.
    T_COR_STRING : "PyString_Type",
    T_COR_INT : "PyInt_Type",
    T_COR_DOUBLE : "PyFloat_Type",
    T_COR_TYPE : "PyClass_Type",
    T_COR_INSTANCE : "PyInstance_Type",
}

cor_integers_narrower_map = {
    T_COR_BOOL : Opcodes.Conv_I2,
    "System.Int16" : Opcodes.Conv_I2,
}
cor_wider_map = {
    (T_COR_INT, "System.Int64") : Opcodes.Conv_I8,
    ("System.Float", "System.Double") : Opcodes.Conv_R8,
}

cor_integer_convert_opcode = Opcodes.Conv_I4

Convert_override_map = {
    "System.Int64" : "PyInt_FromInt",
}

Constructor_map = {
    "tuple" : ("Python.Builtins.types.PyTuple", (T_PYOBJECT_ARRAY,), "PyTuple_Type"),
    "dict" : ("Python.Builtins.types.PyDict", (), "PyDict_Type"),
    "list" : ("Python.Builtins.types.PyList", (), "PyList_Type"),
    "sized list" : ("Python.Builtins.types.PyList", ("System.Int32",), "PyList_Type"),
    "inited list" : ("Python.Builtins.types.PyList", ("System.Collections.List",), "PyList_Type"),
    "method" : ("Python.Builtins.types.PyMethod", ("System.Type", "System.Object", "System.String"), "PyMethod_Type"),
    "builtin-method" : ("Python.Builtins.types.PyBuiltinMethod", ("Python.Builtins.types.PyBuiltinMethodDelegate", T_PYOBJECT, T_PYOBJECT), "PyBuiltinMethod_Type"),
    "function" : ("Python.Builtins.types.PyFunction", ("System.Type", "System.String"), "PyFunction_Type"),
    "pyobject" : (T_PYOBJECT, ("System.Object", "Python.Builtins.types.IPyType"), None),
}

Exception_map = {
    ValueError : "Python.Builtins.exceptions.ValueError",
    RuntimeError : "Python.Builtins.exceptions.RuntimeError",
    TypeError : "Python.Builtins.exceptions.TypeError",
}

COR_MAXINT = sys.maxint # xxx - should get this from int32::MaxValue

class ILGenerator:
    def __init__(self, methBuilder, compiler):
#    assert mctx is not None, "where is the mctx!"
        if methBuilder.__class__ != COMAttributeWrapper:
            self.methBuilder = COMAttributeWrapper(methBuilder)
        else:
            self.methBuilder = methBuilder
        self.ilgen = self.methBuilder.GetILGenerator()
        self.compiler = compiler
        self.emit_symbolic_info = compiler.options.debug_info
        self.ilwrap = client.Dispatch("P2IL2.ILGenWrapper")
        self.ilwrap.SetILGenerator(self.ilgen)
        self.type_pyobject = compiler.getCORType(T_PYOBJECT)
        self.type_pyobject_array = compiler.getCORType(T_PYOBJECT_ARRAY)
        self.local_pyobject_deref = None # declared on demand.
#        self.boxer_locals = {}

    def __getattr__(self, attr):
        try:
            rc = getattr(self.ilwrap, attr)
        except AttributeError:
            rc = getattr(self.ilgen, attr)
        self.__dict__[attr] = rc
        return rc
    def __call__(self):
        return self.ilgen

    def DeclareLocal(self, typ):
        # LocalBuilder objects need magic attribute support.
        return self.ilgen.DeclareLocal(typ)

    def EmitConstructor(self, opcode, ctor_info):
        ctor_info = getattr(ctor_info, "_oleobj_", ctor_info)
        return self.ilwrap.EmitConstructor(opcode, ctor_info)

    def EmitMethod(self, opcode, info):
        info = getattr(info, "_oleobj_", info)
        return self.ilwrap.EmitMethod(opcode, info)

    def _BoxValueType(self, type_name):
        cor_type = self.compiler.getCORType(type_name, 0)
        # XXX - not sure we should call with false here.
        if cor_type is None or not cor_type.IsValueType:
            return
##        boxer_local = self.boxer_locals.get(type_name)
##        if boxer_local is None:
##            boxer_local = self.DeclareLocal(cor_type)
##            if self.emit_symbolic_info:
##                boxer_local.SetLocalSymInfo("box$%s" % (string.replace(type_name, ".", "_"),))
##            self.boxer_locals[type_name] = boxer_local
##        self.EmitLocal(Opcodes.stloc, boxer_local)
##        self.EmitLocal(Opcodes.ldloca_s, boxer_local)
        self.EmitType(Opcodes.box, cor_type)

    def _EmitCORObjectToPyObject(self, type_name):
        # If it has an override, use it.
        override_abstract = Convert_override_map.get(type_name)
        if override_abstract is not None:
            self.emitAbstractCall(override_abstract)
            return

        # See if a special type we can narrow
        if cor_integers_narrower_map.has_key(type_name):
                self.Emit(cor_integer_convert_opcode)
                type_name = T_COR_INT

        pytype_name = Convert_cor_to_py_map.get(type_name)

        # If it can be boxed, box it
        self._BoxValueType(type_name)
        # an object all ready to go - create the PyObject.
        if pytype_name is None:
            # Just a generic object - let the runtime find it.
            self.EmitMethodCall("Python.Converters", "PyObject_FromObject", ("System.Object",) )
            return

        self.EmitField(Opcodes.ldsfld, "Python.Runtime", pytype_name)
        self.EmitNewObject("pyobject")

##  def _EmitCorObjectConverter(self, type_name):
##    # XXX - todo - we should look up the PyType object in the type map, and
##    # raise a type error if wrong.  Worst than can happen now is an invalid cast exception.
##    # Maybe a compiler option?
##
##    converter = Convert_py_to_cor_map.get(type_name)
##    if converter is not None:
##      cor_type, factory_func_name = converter
##      self.EmitMethodCall(cor_type, factory_func_name, (T_PYOBJECT,))
##      return 1
##    return
    def EmitPyObjectDeref(self, field_name):
        if self.local_pyobject_deref is None:
            # Declare a special local for object de-references.
            self.local_pyobject_deref = self.DeclareLocal(self.compiler.getCORType(T_PYOBJECT))
            if self.compiler.options.debug_info:
                self.local_pyobject_deref.SetLocalSymInfo("$pyob_deref")
        self.EmitLocal(Opcodes.stloc, self.local_pyobject_deref)
        self.EmitLocal(Opcodes.ldloca_s, self.local_pyobject_deref)
        self.EmitField(Opcodes.ldfld, T_PYOBJECT, field_name)

    # NOTE - we only attempt to handle "implied" internal conversions, _not_ explicit conversions
    # eg, if you ask for something to be converted from a T_PYOBJECT to a T_COR_STRING, then
    # it had better be a PyString object - eg, there is no attempt to go via PyObject_Str()
    def EmitConversion(self, type_from, type_to):
        # Note: "void" and "null" should never get here!

        if type_from==type_to:
            return
        if type_to in T_PY_ALL:
            if type_from in T_PY_ALL:
                return # Already is
            self._EmitCORObjectToPyObject(type_from)
            return

        elif type_to == T_IPYTYPE:
            # Someone wants an IPyType interface pointer.
            # If Im not converting directly from a PyObject, I need to
            # get the PyObject
            if type_from not in T_PY_ALL:
                self.EmitConversion(type_from, T_PYOBJECT)
                type_from = T_PYOBJECT
            if type_from in T_PY_ALL:
                self.EmitPyObjectDeref("typ")
                return
        else:
            if type_from in T_PY_ALL:
                type_to_ob = self.compiler.getCORType(type_to, 0)
                if type_to_ob is not None:
                    # We have a real COR type to convert to.
                    # XXX - we could avoid an object box if we really wanted to :-)
                    # Push the type on the stack
                    convert_opcode = cor_integers_narrower_map.get(type_to)
                    if convert_opcode is not None:
                        self.emitAbstractCall("PyNumber_Int")
                        self.EmitMethodCall("Python.Converters", "PyInt_AsInt32", (T_PYOBJECT,) )
                        self.Emit(convert_opcode)
                        return
                    self.EmitGetType(type_to_ob)
                    self.emitAbstractCall("PyObject_AsExternalObject") # Throws exception if null.
                    # Cast to the correct object or un-box
                    if type_to_ob.IsValueType:
                        # These unbox instructions cause things to blow up :-)
                        self.EmitType(Opcodes.unbox, type_to_ob)
                        self.EmitType(Opcodes.ldobj, type_to_ob)
#                        self.EmitMethodCall(type_to, "UnBox", (T_COR_OBJECT,))
                    else:
                        self.EmitType(Opcodes.castclass, type_to_ob)
                    return
            else:
                # Check for simple box requests (ie, COR value type to generic Object.
                if type_to == T_COR_OBJECT:
                    # If it is NULL or an instance, we are all ready!
                    if type_from in [T_COR_NULL, T_COR_INSTANCE]:
                            return
                    # check I can get the type from COR - ie, check it is an object.
                    t = self.compiler.getCORType(type_from, 0)
                    if t is not None:
                        # Already an object - just box if necessary, and we are ready.
                        self._BoxValueType(type_from)
                        return
                else:
                    # Check it is not a simple narrow/widen request
                    try:
                        self.Emit(cor_wider_map[type_from, type_to])
                        return
                    except KeyError:
                        pass
                    # Handle string/char conversions.
                    if type_to == T_COR_CHAR and type_from == T_COR_STRING:
                        # XXX - should check for 1 char string!
                        self.pushConstant(0)
                        self.EmitPropertyGet("System.String", "Chars")
                        return
                    if cor_integers_narrower_map.has_key(type_to):
                        self.Emit(cor_integer_convert_opcode)
                        return

        raise source_error("Can't convert from type '%s' to '%s'" % (type_from, type_to))

    def pushConstant(self, val, as_type = None):
        """Push a constant value on the stack."""
        if type(val) in [types.StringType, UnicodeType]:
            self.EmitString(Opcodes.ldstr, val)
            ret_t = T_COR_STRING
        elif type(val)==types.FloatType:
            self.EmitDouble(Opcodes.ldc_R8, val)
            ret_t = T_COR_DOUBLE
        elif type(val)==types.IntType:
            # Use optimized integer constant load opcodes
            custom = Opcodes_int.get(val)
            if custom is None:
                self.EmitInt(Opcodes.ldc_i4, val)
            else:
                self.Emit(custom)
            ret_t = T_COR_INT
        elif val is None:
            # Special handling for NULL
            if as_type is None or as_type == T_COR_OBJECT:
                # Just want a normal COM null
                self.Emit(Opcodes.ldnull)
                ret_t = T_COR_NULL
                as_type = None # no conversion below.
            else:
                # We want None
                assert as_type == T_PYOBJECT, "Should not be asked to push None as type '%s'" % (as_type,)
                self.EmitField(Opcodes.ldsfld, "Python.Runtime", "Py_None")
                ret_t = as_type # avoid the conversion below.
        else:
            raise error, "Can not push constant '%s' (of type %s)" % (val, type(val))
        if as_type is not None:
            self.EmitConversion(ret_t, as_type)
            ret_t = as_type
        return ret_t

    def EmitGetType(self, type_ob):
        self.EmitType(Opcodes.ldtoken, type_ob)
        self.EmitMethodCall("System.Type", "GetTypeFromHandle", ("System.RuntimeTypeHandle",))
        return T_COR_TYPE
        
    def emitArray(self, emitters):
        numvars = len(emitters)
        # Create an array of Objects.
        self.pushConstant(numvars) # Size of the array.
        self.EmitType(Opcodes.newarr, self.type_pyobject)
        i = 0
        for func, args in emitters:
            self.Emit(Opcodes.dup)
            # The array element we are setting
            self.pushConstant(i)
            # Get the address of the array element.
            self.EmitType(Opcodes.ldelema, self.type_pyobject)
            # Calculate the value to be set.
            t = apply( func, args )
            self.EmitConversion(t, T_PYOBJECT)
            # Save to the array
            self.EmitType(Opcodes.stobj, self.type_pyobject)
#      self.Emit(Opcodes.stelem_ref)
            i=i+1

    def emitAbstractCall(self, name):
        args = AbstractMethods_map[name]
        self.EmitMethodCall("Python.Runtime", name, args)

    def EmitNewObject(self, object_id):
        cor_type, cor_args, pytype_name = Constructor_map[object_id]
        # Actually a ctor
#        cor_args = map(self.compiler.getCORType, cor_args)
        ctor_info = self.compiler.getConstructorInfo(cor_type, cor_args)
        self.EmitConstructor(Opcodes.newobj, ctor_info)
        if pytype_name is not None:
            self.EmitField(Opcodes.ldsfld, "Python.Runtime", pytype_name)
            ctor_info = self.compiler.getConstructorInfo(T_PYOBJECT, (T_COR_OBJECT, "Python.Builtins.types.IPyType"))
            self.EmitConstructor(Opcodes.newobj, ctor_info)

    def GetRuntimeExceptionConstructor(self, exc):
        if type(exc)!=types.ClassType:
            assert len(exc.args)==1, "Expecting a simple exception with one arg"
            self.pushConstant(exc.args[0])
            exc_class = exc.__class__
        else:
            exc_class = exc
        exc_type_name = Exception_map[exc_class]
        return self.compiler.getConstructorInfo(exc_type_name, ("System.String",))

    def EmitRaiseException(self, exc):
        ctor_info = self.GetRuntimeExceptionConstructor(exc)
        self.EmitConstructor(Opcodes.newobj, ctor_info)
        self.Emit(Opcodes.throw)
        
    def EmitLdArg(self, num):
        try:
            self.Emit(Opcodes_ldarg[num])
            return
        except KeyError:
            self.EmitInt(Opcodes.LdArg, num)

    def EmitMethodCall( self, typ, func, args, opcode = Opcodes.Call):
        method_info = self.compiler.getMethodInfo(typ, func, args)
        self.EmitMethod(opcode, method_info)

    def EmitPropertyGet(self, typ_name, propName):
        typ = self.compiler.getCORType(typ_name)
        prop_info = glue.Type_GetProperty(typ, propName)
        meth_info = glue.PropertyInfo_GetGetMethod(prop_info._oleobj_)
#        meth_info = prop_info.GetGetMethod()
        self.EmitMethod(Opcodes.Callvirt, meth_info)

    def EmitField(self, opcode, *args):
        if len(args)==1:
            fi = args[0]
        else:
            type, name = args
            fi = self.compiler.getFieldInfo(type, name)
        self.ilwrap.EmitField(opcode, fi._oleobj_)

class Generator:
    def __init__(self, compiler, input):
        self.mctx = self.fctx = self.cctx = self.namespace_local = None
        self.il = None
        self.lineno = 0
        self.loop_labels = []
        self.context_stack = []
        self.options = compiler.options
        self.compiler = compiler

        self.input = input
        self.unique_names_used = {} # broken - see below

        d = self._dispatch = { }
        for name, func in _node_names.items():
            d[name] = getattr(self, func)

    def go(self):
        tree = compiler.parseFile(self.input)

#    try:
        self.dispatch(tree)
#    except pythoncom.com_error, details:
#      raise emit_error("Internal error", details, sys.exc_info()[2])

    def finalize(self):
        self._dispatch = {} # Cycles with bound methods stored in self.__dict__.
        self.compiler = None
        self.actx = self.mctx = self.fctx = None
        self.namespace_global = self.namespace_local = None

    def get_cor_literal_type(self, node, bWantTail = 0, cor_optional = 0):
        # If it is a type, return the type object.
        items = build_dotted_name(node)
        # By not insisting we have the "COR" prefix, we also get things
        # in _our_ namespace - eg, previous classes, etc
        bNeedIt = 0
        ret = tail = None
        if len(items)>0:
            if items[0]=="COR":
                del items[0]
                bNeedIt = not cor_optional
            if bWantTail: tail = items.pop()
            if len(items)>0:
                base_name = string.join(items, ".")
                ret = self.compiler.getCORType(base_name, 0)
                if ret is None: # See if a built-in exception.
                    if bNeedIt and len(items): # Hack - items only zero when looking up builting names - but need better name management!
                        raise source_error("The COM+ type '%s' can not be located" % (base_name,))
                    ret = self.compiler.getCORType("Python.Builtins.exceptions." + base_name, 0)
        if bWantTail:
            return ret, tail
        return ret

    def enter_contexts(self, mctx, cctx, fctx, namespace):
        self.context_stack.append( (self.mctx, self.cctx, self.fctx, self.namespace_local) )
        self.mctx = mctx
        self.cctx = cctx
        self.fctx = fctx
        self.namespace_local = namespace
        if fctx is not None:
            self.il = fctx.il
        elif cctx is not None:
            self.il = cctx.il
        else:
            self.il = mctx.il
    def leave_contexts(self):
        self.mctx, self.cctx, self.fctx, self.namespace_local = self.context_stack.pop()
        if self.fctx is not None:
            self.il = self.fctx.il
        elif self.cctx is not None:
            self.il = self.cctx.il
        elif self.mctx is not None:
            self.il = self.mctx.il
        else:
            self.il = None

    def make_unique_name(self, suggested):
        # This should be at the namespace level, but can be done later...
        while self.unique_names_used.has_key(suggested):
            suggested = suggested + "_"
        self.unique_names_used[suggested] = None
        return suggested

    def dispatch(self, node):
        "Dispatch the compile of a node."
        try:
            lineno = node.lineno
        except AttributeError: # Can happen with hand-crafted nodes (eg, lambda)
            lineno = self.lineno
        if self.options.debug_info and \
             self.il is not None and \
             lineno is not None and \
             lineno != self.lineno:
#                print "Sequence point at", node.lineno
                self.il.MarkSequencePoint(self.mctx.docwriter, lineno, 1, lineno+1, 0) # Dont have a column number available yet!
        if lineno is not None:
            self.lineno = lineno
        fn = self._dispatch[node[0]]
        ret = fn(node)
        assert type(ret) in [types.StringType, UnicodeType], "node did not return a valid value: '%s'" % (node,)
        return ret

    def dispatch_and_convert(self, node, as_type = None):
        if as_type is None: as_type = T_PYOBJECT
        t = self.dispatch(node)
        if t == T_VOID:
            raise source_error("Can't use the result of a void function")
        self.il.EmitConversion(t, as_type)
        return as_type

    def dispatchvoid(self, node):
        "Dispatch the compile of a node, expecting no return values."
        ret = self.dispatch(node)
        assert ret==T_VOID, "void node ('%s') returned a resul - %s" % (node,ret)

    def n_module(self, node):
        # Create the module context that manages the COR module
        mctx = self.compiler.makeModuleContext(self.input)
        self.namespace_global = gen_namespaces.Namespace(mctx)
        self.enter_contexts(mctx, None, None, self.namespace_global)

        # Now start running over the parse tree.
        self.dispatchvoid(node[2])

        mctx.finalize()
        self.leave_contexts()
        return T_VOID

    def n_stmt(self, node):
        map(self.dispatchvoid, node[1])
        return T_VOID

    def n_function(self, node):
        func = node
        func.is_instance_method = self.cctx is not None and self.fctx is None
        func.is_ctor = func.is_instance_method and func.name=='__init__' 

        user_sig = find_compiler_directive(func.code, "_com_params_")
        ret_type = find_compiler_directive(func.code, "_com_return_type_")
        if user_sig or ret_type:
            if user_sig:
                if func.varargs or func.kwargs: raise source_error("Dont support variable or kw args with a user-specified signature")
                arg_types = string.split(str(user_sig), ",")
                arg_types = map(string.strip, arg_types)
                arg_types = map(self.compiler.getCORType, arg_types)
                if (not func.is_instance_method and len(arg_types) != len(func.argnames)) or \
                     (func.is_instance_method and len(arg_types)!=len(func.argnames)-1):
                    raise source_error("The number of params does not match those specified in '_com_params_'")
            else:
                arg_types = ()
            cor_ret_type = None
            if ret_type is None:
                if not func.is_ctor:
                        self.compiler.warning(2, "_com_params_ specified, but no return type - assuming 'void'")
            else:
                if func.is_ctor:
                    raise source_error("You can not define the return type for a constructor")
                cor_ret_type = self.compiler.getCORType(ret_type)
                if cor_ret_type is None:
                    self.compiler.warning(2, "_com_return_type_ specified as '%s' - no such type" % (ret_type,))
            method_attributes = constants.MethodAttributes_Public
            if func.is_instance_method:
                if not func.is_ctor:
                    method_attributes = method_attributes | constants.MethodAttributes_Virtual
            else:
                method_attributes = method_attributes | constants.MethodAttributes_Static
            # This is the native descriptor, and no thunks.
            descriptors = [funcsigs.Descriptor(method_attributes, cor_ret_type, arg_types)]
            use_cor_sig = 1
        else:
            if self.cctx is not None and self.cctx.is_pyonly:
                descriptors = []
                use_cor_sig = 0
            else:
                descriptors = funcsigs.find_matching_com_descriptors(self, func)
                if len(descriptors)>1:
                    use_cor_sig = 0
                else:
                    # COR SIG for instance methods without varargs/kwargs.
                    use_cor_sig = self.cctx is not None and not (func.varargs or func.kwargs)

        if func.is_ctor and not use_cor_sig:
            self.cctx.is_pyonly = 1 # XXX - this should be able to be set when the cctx is created - the ctor may not be the first func!
        if func.is_ctor and use_cor_sig: # __init__ with a COR sig gets a fn context.
            ContextFactory = ConstructorContext
        else:
            ContextFactory = FunctionContext

        ret_t = self.basicfunc(node, ContextFactory, use_cor_sig, descriptors)
        assert ret_t == T_VOID, "Function nodes must return void"

        # Assign the function object
        return self.name_assign(node.name)

    def n_lambda(self, node):
        func = node
        func.is_instance_method = func.is_ctor = 0
        func.is_ctor = func.is_instance_method and name=='__init__'
        node.name = self.make_unique_name("lambda")
        node.doc = None
        ret_t = self.basicfunc(node, LambdaContext, 0, [])
        assert ret_t in T_PY_ALL, "lambda must return a PyObject (%s)" % (ret_t,)
        return ret_t

    def basicfunc(self, func, ContextFactory, use_cor_sig, descriptors):
        "Generate basic function/lambda code."
        num_args = len(func.argnames)
        il = self.il
        self_name = None

        if func.is_instance_method and len(func.defaults) >= func.argnames:
            # We will probably blow up below if we do :-)
            raise source_error("You may not specify a default param for the 'self/this' param")

        if func.is_instance_method:
            if len(func.argnames)==0: raise source_error("Instance method doesnt have enough args")
            self_name = func.argnames[0] # Will almost always be "self"
            if self_name != 'self':
                self.compiler.warning(2, "First parameter of method is not 'self' - did you forget it?")

        if not use_cor_sig:
            # Emitting a native PyMethod
            thunks = descriptors
            method_attributes = constants.MethodAttributes_Public | constants.MethodAttributes_Static
            arg_types = il.type_pyobject, il.type_pyobject_array, il.type_pyobject
            descriptor = funcsigs.Descriptor(method_attributes, il.type_pyobject, arg_types)
            func_generator = funcsigs.PyMethodSignatureManager(self, func, descriptor)
        else:
            # Using a COR signature - make sure we have one.
            assert len(descriptors)<=1, "Shouldnt be able to have more than one descriptor here"
            thunks = []
            if len(descriptors)==0:
                if func.is_instance_method:
                    num_cor_args = num_args - 1
                    method_attributes = constants.MethodAttributes_Public
                    if not func.is_ctor: # constructors dont appear to be marked virtual
                        method_attributes = method_attributes | constants.MethodAttributes_Virtual
                else:
                    num_cor_args = num_args
                    method_attributes = constants.MethodAttributes_Public | constants.MethodAttributes_Static
                ret_type = None
                type_ob = self.compiler.getCORType(T_COR_OBJECT)
                if not func.is_ctor:
                    ret_type = type_ob
                arg_types = (type_ob,) * num_cor_args
                descriptor = funcsigs.Descriptor(method_attributes, ret_type, arg_types)
            else:
                descriptor = descriptors[0]
            func_generator = funcsigs.CORMethodSignatureManager(self, func, descriptor)

        func_generator.PrepareContext()

        fctx = ContextFactory(self.compiler, self.mctx, self.cctx, func.name, descriptor)
        fctx.self_name = self_name
        namespace  = gen_namespaces.LocalNamespace(self.namespace_global, fctx)
        self.enter_contexts(self.mctx, self.cctx, fctx, namespace)

        arg_names = func.argnames[:]
        # We dont declare self here, as its type depends on the signature we are generating...
        if func.is_instance_method:
            self.namespace_local.assign(arg_names[0]) # the self
            del arg_names[0]
        for argname in arg_names:
            if type(argname)==types.TupleType:
                names = FlattenTuple(argname)
            else:
                names = (argname,)
            for n in names:
                self.namespace_local.assign(n)
                self.fctx.ensureLocalDeclared(n)

        func_generator.EmitFuncHeader()

        # Generate the code for the body of the function
        ret_t = self.dispatch(func.code)

        method_info = fctx.methBuilder
        ret_t = fctx.finalize(ret_t)
        # Put the old context back where it belongs
        self.leave_contexts()

        # Emit any thunks.
        for descriptor in thunks:
            fctx = ContextFactory(self.mctx, self.cctx, func.name, descriptor)
            self.enter_contexts(self.mctx, self.cctx, fctx, namespace)
            func_generator = funcsigs.CORMethodSignatureManager(self, func, descriptor)
            func_generator.EmitThunk(method_info)
            fctx.finalize()
            self.leave_contexts()

        func_generator.EmitFunctionObject(method_info)
        return ret_t

    def emitAbstractCall(self, name):
        return self.il.emitAbstractCall(name)

    def n_class(self, node):
        class_name = node[1]
        base_classes = node[2]
        # We need to be able to get the base class at compile time, rather than runtime.
        type_base_type = None
        type_base_interfaces = []
        python_base_types_nodes = []
        for base in base_classes:
            this_base_type = self.get_cor_literal_type(base)
            if this_base_type is not None:
                if this_base_type.IsClass:
                    if type_base_type is not None:
                        # Second class listed.
                        msg = "Class '%s' is using Python derivation from '%s': second or suqsequent base" % (class_name, this_base_type.FullName)
                        self.compiler.verbose(2, msg, NoWhere)
                        this_base_type = None # Force into the Python class list
                    else:
                        # Make sure I can find a public constructor.
                        ctors = glue.Type_GetConstructors(this_base_type)
                        if len(ctors)==0:
                            # Should check it really _is_ a Python class!
                            msg = "Class '%s' is using Python derivation from '%s': ctor is protected" % (class_name, this_base_type.FullName)
                            self.compiler.verbose(1, msg, NoWhere)
                            this_base_type = None # Force into the Python class list
                        else:
                            msg = "Class '%s' is using COM+ derivation from '%s'" % (class_name, this_base_type.FullName)
                            self.compiler.verbose(1, msg, NoWhere)
                            type_base_type = this_base_type
                elif this_base_type.IsInterface:
                    type_base_interfaces.append(this_base_type)
                else:
                    raise source_error("Can not derive from this object")
            if this_base_type is None:
                msg = "Class '%s' is using Python derivation from '%s': can not locate base type" % (class_name, base)
                self.compiler.verbose(2, msg, NoWhere)
                python_base_types_nodes.append(base)

        doc_string = node[3]
        if doc_string: self.compiler.verbose(1, "Dont support docstrings for classes yet")
        cctx = ClassContext(self.compiler, self.mctx, class_name, type_base_type, type_base_interfaces)
        namespace = gen_namespaces.LocalNamespace(self.namespace_global, cctx)
        # Save the existing contexts.
        self.enter_contexts(self.mctx, cctx, None, namespace)

        # Emit the code for the base classes into the class ctor.
        il = cctx.il
        il.pushConstant(len(python_base_types_nodes))
        il.EmitNewObject("sized list")
        il.EmitField(Opcodes.stsfld, cctx.fb_bases)
        for base in python_base_types_nodes:
            il.EmitField(Opcodes.ldsfld, cctx.fb_bases)
            self.dispatch_and_convert(base)
            il.EmitMethodCall( "Python.Runtime", "PyList_Append", (T_PYOBJECT,T_PYOBJECT))
        # Now compile the class body.
        self.dispatchvoid(node[4])
#    try:
        cctx.finalize()
#    except pythoncom.com_error, details:
#      raise emit_error("Error defining class object", details, sys.exc_info()[2])
        # Restore the cctx
        self.leave_contexts()

        # Now stick the class in the namespace.
        # Emit the class object itself
        il = self.il
##    il.pushConstant(class_name)
##    il.EmitMethodCall("System.Type", "GetType", ("System.String",))
        il.EmitGetType(cctx.typeBuilder)
        il.EmitConversion(T_COR_TYPE, T_PYOBJECT)
        # Emit the assignment
        return self.name_assign( class_name )

    def n_pass(self, node):
        return T_VOID

    def n_break(self, node):
        assert self.loopInLoop(), "break outside loop!"
        self.il.EmitLabel(Opcodes.br, self.loopGetLabels()[1])
        return T_VOID

    def n_continue(self, node):
        assert self.loopInLoop(), "continue outside loop!"
        self.il.EmitLabel(Opcodes.br, self.loopGetLabels()[0])
        return T_VOID

    def loopEnter(self):
        # Create 3 labels - one for the start of the loop body,
        # one for normal termination (which enters the "else", if any)
        # and one for "break" termination (which is past the "else" label.)
        il = self.il
        loop_label = il.CreateLabel2()
        loop_end_label = il.CreateLabel2()
        loop_else_label = il.CreateLabel2()
        self.loop_labels.append( (loop_label, loop_end_label) )
        return loop_label, loop_end_label, loop_else_label
    def loopInLoop(self):
        return len(self.loop_labels)>0
    def loopGetLabels(self):
        return self.loop_labels[-1]
    def loopEnd(self):
        self.loop_labels.pop()

    def n_for(self, node):
        il = self.il
        index = node[1]
        seq = node[2]
        body = node[3]
        elsepart = node[4]
        # Create the sequence.
        self.dispatch_and_convert(seq)
        is_sequence_label = il.CreateLabel2()
        self.emitAbstractCall("PyObject_GetEnumerator")
        # Damn - can NOT simply keep the enumerator on the stack,
        # as if the block contains exception handlers, the verifier
        # gets upset!  So store the enum in a local.
        local_enum = il.DeclareLocal(self.compiler.getCORType("Python.Builtins.types.IPyEnumerator"))
        il.EmitLocal(Opcodes.stloc, local_enum)
        il.EmitLocal(Opcodes.ldloc, local_enum)
        il.EmitLabel(Opcodes.brtrue, is_sequence_label)
        il.EmitRaiseException(TypeError("loop over non-sequence"))
        il.MarkLabel2(is_sequence_label)
        loop_begin_label, loop_end_label, loop_else_label = self.loopEnter()
        il.MarkLabel2(loop_begin_label)
        il.EmitLocal(Opcodes.ldloc, local_enum)
        il.EmitMethodCall("Python.Builtins.types.IPyEnumerator", "MoveNext", (), Opcodes.callvirt)
        il.EmitLabel(Opcodes.brfalse, loop_else_label)
        il.EmitLocal(Opcodes.ldloc, local_enum)
        # Call the enumerator to get the next index value
        il.EmitPropertyGet("Python.Builtins.types.IPyEnumerator", "Current")
        # And the assignment
        t = self.dispatch(index)
        assert t==T_VOID, "assignment must not leave anything on the stack"
        # the loop body
        self.dispatchvoid(body)
        il.EmitLabel(Opcodes.br, loop_begin_label)
        il.MarkLabel2(loop_else_label)
        if elsepart is not None:
            self.dispatchvoid(elsepart)
        il.MarkLabel2(loop_end_label)
        self.loopEnd()
        return T_VOID

    def n_while(self, node):
        test_node = node[1]
        body_node = node[2]
        else_node = node[3]
        il = self.il
        loop_begin_label, loop_end_label, loop_else_label = self.loopEnter()
        il.MarkLabel2(loop_begin_label)
        self.dispatch_and_convert(test_node)
        self.emitAbstractCall("PyObject_IsTrue")
        il.EmitLabel(Opcodes.brfalse, loop_else_label)
        self.dispatchvoid(body_node)
        il.EmitLabel(Opcodes.br, loop_begin_label)
        il.MarkLabel2(loop_else_label)
        if else_node is not None:
            self.dispatchvoid(else_node)
        il.MarkLabel2(loop_end_label)
        self.loopEnd()
        return T_VOID

    def n_if(self, node):
        il = self.il
        if_parts = node[1]
        else_part = node[2]
        end_label = il.CreateLabel2()
        for test, block in if_parts:
            next_label = il.CreateLabel2()
            self.dispatch_and_convert(test)
            il.emitAbstractCall("PyObject_IsTrue")
            il.EmitLabel(Opcodes.brfalse, next_label)
            self.dispatchvoid(block)
            il.EmitLabel(Opcodes.br, end_label)
            il.MarkLabel2(next_label)
        if else_part is not None:
            self.dispatchvoid(else_part)
        il.MarkLabel2(end_label)
        return T_VOID

    def n_exec(self, node):
        raise source_error('not yet implemented')

    def n_assert(self, node):
        if self.options.debug_info:
            test = node[1]
            text = node[2]
            il = self.il
            self.dispatch_and_convert(test)
            il.emitAbstractCall("PyObject_IsTrue")
            il.pushConstant("File %s, Line %s" % (self.input, self.lineno))
            self.dispatch_and_convert(text)
            il.emitAbstractCall("PyObject_Str")
            il.EmitConversion(T_PYOBJECT, T_COR_STRING)
            il.EmitMethodCall("System.Diagnostics.Debug", "Assert", (T_COR_BOOL, T_COR_STRING, T_COR_STRING) )
        else:
            self.compiler.warning(4, "assertion code not emitted as the 'debug-information' option is disabled.")
        return T_VOID

    def n_from(self, node):
        mod_name = node[1]
        items = node[2]
        il = self.il

        il.pushConstant(mod_name)
        self.emitAbstractCall("PyImport_ImportModule")

        for name in items:
            if type(name)==type(()):
                ass_name = name[1]
                name = name[0]
            else:
                ass_name = name
            il.Emit(Opcodes.Dup)
            il.pushConstant(name)
            il.emitAbstractCall("PyObject_GetAttrString")
            self.name_assign(ass_name)
        il.Emit(Opcodes.Pop)
        return T_VOID

    def n_import(self, node):
        for data in node[1]:
            if type(data)==type(()):
                mod_name, ass_name = data
            else:
                mod_name = data
                ass_name =  string.split(mod_name, ".")[0]
            # emit the module object
            self.il.pushConstant(mod_name)
            self.emitAbstractCall("PyImport_ImportModule")
            # and assignment
            self.name_assign(ass_name)
        return T_VOID

    def n_raise(self, node):
        exc = node[1]
        val = node[2]
        arg_list = node[3]
##    print "exc=", exc
##    print "excval=", val
##    print "arg_list=",arg_list
        il = self.il
        if arg_list is not None: # arg_list[0] != 'name' or arg_list[1] != 'None':
            print arg_list
            raise source_error("Dont support complex raise statements")
        # Push the exception type object to the stack
        if exc is None:
            # re-raise
            il.Emit(Opcodes.Rethrow)
        else:
            # Create the exception instance, and throw it.
            self.dispatch_and_convert(exc)
            # dup it so we can test for Class (ie, Type) or instance
            il.Emit(Opcodes.dup)
            il.EmitConversion(T_PYOBJECT, T_COR_OBJECT) # get the .ob out.
            instance_label = il.CreateLabel2()
            il.EmitType(Opcodes.isinst, self.compiler.getCORType("System.Type"))
            il.EmitLabel(Opcodes.brfalse, instance_label)
            # Instantiate the class.
            if val is not None:
                    # The value is the arg to the ctor
                il.emitArray( [(self.dispatch_and_convert, (val,))] )
    #      self.dispatch_and_convert(val)
            else:
                il.pushConstant(None)
            # NULL keywords
            il.pushConstant(None, T_PYOBJECT)
            self.emitAbstractCall("PyObject_Call")
            # fall through to instance_label
            il.MarkLabel2(instance_label)

            il.EmitConversion(T_PYOBJECT, T_COR_OBJECT) # get the .ob out.
            # Cast it to system.exception for the verifier.  If it is not an Exception, raise
            # a Python Runtime error
            label_throw = il.CreateLabel2()
            il.Emit(Opcodes.dup)
            il.EmitType(Opcodes.isinst, self.compiler.getCORType("System.Exception"))
            il.EmitLabel(Opcodes.brtrue, label_throw)
            # Eeek - not an exception.
            il.Emit(Opcodes.pop)
            il.EmitRaiseException(RuntimeError("Exception thrown by Python is not derived from System.Exception"))
            il.MarkLabel2(label_throw)
            il.EmitType(Opcodes.castclass, self.compiler.getCORType("System.Exception"))
            # Must now be an instance - raise it.
            il.Emit(Opcodes.throw)
        return T_VOID

    def n_reraise(self, node):
        self.il.Emit(Opcodes.Rethrow)
        return T_VOID

    def n_tryfinally(self, node):
        il = self.il
        il.BeginExceptionBlockX()
        self.dispatch(node[1])
        il.BeginFinallyBlockX()
        self.dispatch(node[2])
        il.EndExceptionBlockX()
        return T_VOID

    def n_tryexcept(self, node):
        il = self.il
        label_ends = []
        local_temp = il.DeclareLocal( self.compiler.getCORType("System.Object"))
        if self.options.debug_info:
            local_temp.SetLocalSymInfo("$exception_temp")

        # We create "nested" exception handlers here -
        # begin one block for each "except" clause, then the
        # try clause, then unwind checking our filters.
        for n in node[2]:
            label_ends.append(self.il.BeginExceptionBlockX())
        self.dispatch(node[1])
        for test, result, suite in node[2]:
#      il.EmitLabel(Opcodes.br, label_ends.pop())
            il.BeginExceptFilterBlockX()
            if test is None:
                # Always catch!
                il.Emit(Opcodes.pop)
                il.pushConstant(1)
            else:
                # The exception we are matching must be user_type->IsInstanceOfType(exc_val)
                # No swap instruction!!!!!  Store in a local
#        il.EmitWriteLineString("In filter")
                il.EmitLocal(Opcodes.stloc, local_temp)
                t = self.dispatch_and_convert(test, T_COR_TYPE)
#        il.EmitConversion(t, T_COR_TYPE)
                il.EmitLocal(Opcodes.ldloc, local_temp)
                il.EmitMethodCall("System.Type", "IsInstanceOfType", ("System.Object",) )
                lab_leave = self.il.CreateLabel2()
                il.Emit(Opcodes.dup)
                il.EmitLabel(Opcodes.brfalse, lab_leave) # leaving the dup'd 0 on the stack.
                il.Emit(Opcodes.pop) # remove the val we duped.
                il.pushConstant(1)
#        il.EmitWriteLineString("Exception filter is ours")
                il.MarkLabel2(lab_leave)

            # And now the Exception handler for this filter.
            il.BeginCatchBlockNull()
            if result is None:
                il.Emit(Opcodes.Pop) # Remove the value.
            else:
                il.EmitConversion(T_COR_INSTANCE, T_PYOBJECT)
                self.dispatch(result) # Store the exception value
            self.dispatch(suite)
            il.EndExceptionBlockX()
        il.Emit(Opcodes.nop) # prevent verification errors
        return T_VOID

    def n_tryexcept_other(self, node):
        # Code that caught exceptions by name - could possibly integrate
        # with above if filters turn out to have a big overhead.
        self.il.BeginExceptionBlockX()
        self.dispatch(node[1])
        # This is a pain - we need to know the "Type *" of the exception at compile time.
        # So we use exception filters instead of specifying the actual exception type.
        # We could get smarter and do both.
        for test, result, suite in node[2]:
            if test is None:
                exc_type = self.compiler.getCORType("System.Exception")
            else:
                if len(test) != 2 or test[0] != "name":
                    raise source_error("Must catch exceptions by name")
                exc_name = "Python.Builtins.exceptions." + test[1]
                exc_type = self.compiler.getCORType(exc_name)
            self.il.BeginCatchBlock(exc_type)
            if result is None:
                self.il.Emit(Opcodes.Pop)
            else:
                raise source_error("Cant get exception values yet!")
            # For now, pop the exception value
            self.dispatch(suite)
        self.il.EndExceptionBlock()
        return T_VOID

    def n_return(self, node):
        # IL does not allow a return statement in a try block
        # therefore we use a local variable
        if self.fctx.return_type_id == T_VOID:
            if node[1][0] != "const" or node[1][1] != None:
                raise source_error("Can not specify a return value for a void function")
        else:
            self.dispatch_and_convert(node[1], self.fctx.return_type_id)
            self.il.EmitLocal(Opcodes.stloc, self.fctx.local_return)
        self.il.EmitLabel(Opcodes.leave, self.fctx.label_end_method)
        return T_VOID

    def n_const(self, node):
        k = node[1]
        if type(k) in [types.IntType, types.FloatType, types.StringType, types.NoneType]:
            t = self.il.pushConstant(k)
        elif k == {}:
            self.il.EmitNewObject("dict")
            t = T_PY_DICT
        elif k == []:
            self.il.EmitNewObject("list")
            t = T_PY_LIST
        elif k == ():
            self.emitTuple([])
            t = T_PY_TUPLE
        else:
            raise source_error('constant type not supported')
        return t

    def n_print(self, node):
        "Print the values with NO newline."
        il = self.il
        ### This doesnt work for some reason.  Causes a crash.  WTF!!!
##    console_type = self.compiler.getCORType("System.Console")
##    console_info = console_type.GetField("Out")
##    print "console info is", console_info
##    console_token = self.mctx.mod.GetFieldTokenX(console_info)
##    # Never gets to here - Illegal instruction exception silently terminates the process!
##    print "token is", console_token

        for expr in node[1]:
            self.dispatch_and_convert(expr)

            il.EmitMethodCall("Python.Runtime", "__GetStdOut", ())

            self.il.pushConstant(1) # Py_PRINT_RAW

            # Make the call to PyObject_Print
            self.emitAbstractCall("PyObject_Print")

            ### this behavior doesn't perfectly match Python's print semantics
            ### Also need to write the space to the same file as the print went.
            il.EmitString(Opcodes.ldstr, " ")
            il.EmitMethodCall("System.Console", "Write", ("System.String",))

        return T_VOID

    def n_printnl(self, node):
        "Print the values, followed by a newline."
        self.n_print(node)
        ### Need to write the space to the same file as the print went.
        self.il.EmitWriteLineString('')# self.mctx.mod, newline)
        return T_VOID

    def n_discard(self, node):
        if node[1][0] == 'const':
            # Don't evaluate constant's that aren't going anywhere.. duh. :)
            return T_VOID

        t = self.dispatch(node[1])
        if t != T_VOID:
            self.il.Emit(Opcodes.pop)
        return T_VOID

    def n_assign(self, node):
        # Evaluate the RHS putting the object on the stack
        self.dispatch_and_convert(node[2])
        lhs = node[1]
        if len(lhs)==1:
            # Simple assignment - avoid extra stack work
            self.dispatchvoid(lhs[0])
        else:
            il = self.il
            for ass in lhs:
                il.Emit(Opcodes.Dup)
                self.dispatchvoid(ass)
            il.Emit(Opcodes.Pop)
        return T_VOID

    def n_ass_sequence(self, node):
        il = self.il

        is_sequence_label = il.CreateLabel2()
        self.emitAbstractCall("PyObject_GetEnumerator")
        il.Emit(Opcodes.Dup)
        il.EmitLabel(Opcodes.brtrue, is_sequence_label)
        il.Emit(Opcodes.pop)
        il.EmitRaiseException(TypeError("unpack non-sequence"))
        il.MarkLabel2(is_sequence_label)
        # Loop over the assignments, iterating and checking as we go.
        method_info_getnext = self.compiler.getMethodInfo("Python.Builtins.types.IPyEnumerator", "MoveNext", ())
        label_exit = il.CreateLabel2()
        label_exhausted = il.CreateLabel2()
        for ass in node[1]:
            il.Emit(Opcodes.dup) # For Enum::GetNext() call
            il.EmitMethod(Opcodes.callvirt, method_info_getnext)
            il.EmitLabel(Opcodes.brfalse, label_exhausted)
            il.Emit(Opcodes.dup) # For Enum::GetObject() call
            il.EmitPropertyGet("Python.Builtins.types.IPyEnumerator", "Current")
            # Object now on stack - perform assignment
            self.dispatch(ass)
        # Check sequence exhausted
        il.EmitMethod(Opcodes.callvirt, method_info_getnext)
        il.EmitLabel(Opcodes.brfalse, label_exit)
        il.EmitRaiseException(ValueError("unpack tuple of wrong size"))
#    self.il.EmitLabel(Opcodes.br, label_exit)
        il.MarkLabel2(label_exhausted)
        il.Emit(Opcodes.pop) # The exhausted enum
        il.EmitRaiseException(ValueError("unpack tuple of wrong size"))
        il.MarkLabel2(label_exit)
        return T_VOID

    def n_ass_tuple(self, node):
        return self.n_ass_sequence(node)

    def n_ass_list(self, node):
        return self.n_ass_sequence(node)

    def n_ass_name(self, node):
        if node[2] == OP_ASSIGN:
            return self.name_assign(node.name)
        elif node[2] == OP_DELETE:
            self.name_delete(node.name)
        else:
            raise source_error("Unknown ass_name flag: %s" % node[2])
        return T_VOID

    def n_subscript(self, node):
        # The object being subscripted
        self.dispatch_and_convert(node[1])
        # The subscript.
        if len(node[3]) > 1:
            self.emitTuple(node[3])
        else:
            self.dispatch_and_convert(node[3][0])
        ass_type = node[2]
        # Call the method
        if ass_type ==OP_ASSIGN:
            self.il.emitAbstractCall("__SetItem")
            return T_VOID
        elif ass_type == OP_APPLY: # subscript assignment.
            self.il.emitAbstractCall("PyObject_GetItem")
            return T_PYOBJECT
        elif ass_type == OP_DELETE:
            self.il.emitAbstractCall("PyObject_DelItem")
            return T_VOID
        else:
            # Note that OP_ASSIGN should never get here, but handled by n_assign.
            raise source_error("Unknown subscript type - %s" % (ass_type,))

##  def n_subscript(self, node, rhs_node):
##    else:
##      ass_type = node[2]

    def n_ass_attr(self, node):
        ass_typ = node[3]
        if ass_typ == OP_ASSIGN:
            # value already on the stack.
            self.dispatch_and_convert(node[1])
            self.il.pushConstant(node[2])
            self.il.emitAbstractCall("__SetAttr")
            # Check if it is a literal "self." reference, and if so, do some property magic.
            if node[1][0]=="name" \
                 and self.fctx is not None \
                 and node[1][1]==self.fctx.self_name:
                self.cctx.register_property(node[2])
        elif ass_typ == OP_DELETE:
            raise error("not yet implemented - no way of deleting attributes yet!")
        else:
            raise error("Unknown assignment type: %s" % (ass_typ,))
        return T_VOID

    def emitTuple(self, nodes):
        emitters = []
        for v in nodes:
            emitters.append( (self.dispatch_and_convert, (v,) ) )
        self.il.emitArray(emitters)
        self.il.EmitNewObject("tuple")
        return T_PY_TUPLE

    def n_tuple(self, node):
        return self.emitTuple(node[1])

    def n_list(self, node):
        # Create a new list object
        il = self.il
        il.pushConstant(len(node[1]))
        il.EmitNewObject("sized list")
        # Loop manually adding each item.
        for n in node[1]:
            il.Emit(Opcodes.dup)
            self.dispatch_and_convert(n)
            il.EmitMethodCall("Python.Runtime", "PyList_Append", (T_PYOBJECT,T_PYOBJECT))
        return T_PY_LIST

    def n_dict(self, node):
        # Create a new dict object
        il = self.il
        il.EmitNewObject("dict")
        # Loop manually adding each item.
        for key_node, val_node in node[1]:
            il.Emit(Opcodes.dup)
            self.dispatch_and_convert(key_node)
            self.dispatch_and_convert(val_node)
            self.emitAbstractCall("PyObject_SetItem")
        return T_PY_DICT

    def n_or(self, node):
        il = self.il
        lab_end_test = il.CreateLabel2()
        lab_result_true = il.CreateLabel2()
        for cmp_node in node[1]:
            self.dispatch_and_convert(cmp_node)
            self.emitAbstractCall("PyObject_IsTrue")
            il.EmitLabel(Opcodes.brtrue, lab_result_true)

        il.pushConstant(0)
        il.EmitLabel(Opcodes.br, lab_end_test)
        il.MarkLabel2(lab_result_true)
        il.pushConstant(1)
        il.MarkLabel2(lab_end_test)
        return T_COR_BOOL

    def n_and(self, node):
        il = self.il
        lab_end_test = il.CreateLabel2()
        lab_result_false = il.CreateLabel2()
        for cmp_node in node[1]:
            self.dispatch_and_convert(cmp_node)
            self.emitAbstractCall("PyObject_IsTrue")
            il.EmitLabel(Opcodes.brfalse, lab_result_false)

        il.pushConstant(1)
        il.EmitLabel(Opcodes.br, lab_end_test)
        il.MarkLabel2(lab_result_false)
        il.pushConstant(0)
        il.MarkLabel2(lab_end_test)
        return T_COR_BOOL

    def n_not(self, node):
        self.dispatch_and_convert(node[1])
        self.emitAbstractCall("PyObject_Not");
        return T_COR_BOOL

    def _do_in_loop(self, rhs, is_in_val, not_in_val):
        il = self.il
        # Create an enumerator and loop looking for the item.
        # stash away the value
        temp_local = il.DeclareLocal(il.type_pyobject)
        if self.options.debug_info:
            temp_local.SetLocalSymInfo("$loop_temp")
        il.EmitLocal(Opcodes.stloc, temp_local)
        self.dispatch_and_convert(rhs)

        is_sequence_label = il.CreateLabel2()
        self.emitAbstractCall("PyObject_GetEnumerator")
        il.Emit(Opcodes.Dup)
        il.EmitLabel(Opcodes.brtrue, is_sequence_label)
        il.Emit(Opcodes.pop)
        il.EmitRaiseException(TypeError("'in' or 'not in' needs a sequence for the right argument"))
        il.MarkLabel2(is_sequence_label)

        method_info_getnext = self.compiler.getMethodInfo("Python.Builtins.types.IPyEnumerator", "MoveNext", ())
        loop_begin_label = il.CreateLabel2()
        loop_end_label = il.CreateLabel2()
        loop_false_label = il.CreateLabel2()
        il.MarkLabel2(loop_begin_label)
        il.Emit(Opcodes.dup) # For Enum::GetNext() call
        il.EmitMethod(Opcodes.callvirt, method_info_getnext)
        il.EmitLabel(Opcodes.brfalse, loop_false_label)
        il.Emit(Opcodes.dup) # For Enum::GetObject() call
        # Call the enumerator to get the next index value
        il.EmitPropertyGet("Python.Builtins.types.IPyEnumerator", "Current")
        # And the comparison
        il.EmitLocal(Opcodes.ldloc, temp_local)
        il.emitAbstractCall("PyObject_Compare")
        il.EmitLabel(Opcodes.brtrue, loop_begin_label)
        il.Emit(Opcodes.pop) # the enumerator left on the stack
        il.pushConstant(is_in_val)
        il.EmitLabel(Opcodes.br, loop_end_label)
        il.MarkLabel2(loop_false_label)
        il.Emit(Opcodes.pop) # the enumerator left on the stack
        il.pushConstant(not_in_val)
        il.MarkLabel2(loop_end_label)

    def n_compare(self, node):
        il = self.il
        self.dispatch_and_convert(node[1])
        for op, rhs in node[2]:
            if op in ['==', '<=', '<', '>=', '>', '!=']:
                self.dispatch_and_convert(rhs)
                il.emitAbstractCall("PyObject_Compare")
                opcode, cmp_against = Compare_map[op]
                il.pushConstant(cmp_against)
                il.Emit(opcode)
            elif op=='is':
                il.EmitConversion(T_PYOBJECT, T_COR_OBJECT)
                self.dispatch_and_convert(rhs)
                il.EmitConversion(T_PYOBJECT, T_COR_OBJECT)
                il.Emit(Opcodes.ceq)
            elif op=='is not':
                # Get the object out
                il.EmitConversion(T_PYOBJECT, T_COR_OBJECT)
                self.dispatch_and_convert(rhs)
                il.EmitConversion(T_PYOBJECT, T_COR_OBJECT)
                il.Emit(Opcodes.ceq)
                il.pushConstant(0)
                il.Emit(Opcodes.ceq)
            elif op=='in':
                self._do_in_loop(rhs, 1, 0)
            elif op=='not in':
                self._do_in_loop(rhs, 0, 1)
            else:
                raise source_error("The comparison operator '%s' is not yet supported" % (op,))
        # Result is on the stack as a "bool"
        return T_COR_BOOL

    def n_bitor(self, node):
        return self.binary("PyNumber_Or", node)

    def n_bitxor(self, node):
        return self.binary("PyNumber_Xor", node)

    def n_bitand(self, node):
        return self.binary("PyNumber_And", node)

    def n_lshift(self, node):
        return self.binary("PyNumber_Lshift", node)

    def n_rshift(self, node):
        return self.binary("PyNumber_Rshift", node)

    def n_plus(self, node):
        return self.binary("PyNumber_Add", node)

    def n_minus(self, node):
        return self.binary("PyNumber_Subtract", node)

    def n_star(self, node):
        return self.binary("PyNumber_Multiply", node)

    def n_slash(self, node):
        return self.binary("PyNumber_Divide", node)

    def n_percent(self, node):
        return self.binary("PyNumber_Remainder", node)

    def n_uplus(self, node):
        return self.unary("PyNumber_Positive", node)

    def n_uminus(self, node):
        return self.unary("PyNumber_Negative", node)

    def n_invert(self, node):
        return self.unary("PyNumber_Invert", node)

    def builtin_optimize_variables(self, name):
        if name=="None":
            return 1, self.il.pushConstant(None, T_PYOBJECT)
        return 0, None

    def n_name(self, node):
        # Resolve a name.
        name = node[1]
        if self.namespace_local.allowPotentialSpecialBuiltin(name):
            # Specially optimize builtins we can
            typ = self.get_cor_literal_type(node)
            if typ is not None:
                return self.il.EmitGetType(typ)
            else:
                ok, t = self.builtin_optimize_variables(name)
                if ok:
                    return t
        return self.name_load(name)

    def n_global(self, node):
        for n in node[1]:
            self.namespace_local.makeglobal(n)
        return T_VOID

    def n_power(self, node):
        ops = node[1]
        self.dispatch_and_convert(ops[0])
        for op in ops[1:]:
            self.dispatch_and_convert(op)
            self.il.pushConstant(None, T_PYOBJECT)
            self.il.emitAbstractCall("PyNumber_Power")
        return T_PYOBJECT

    def n_backquote(self, node):
        self.dispatch_and_convert(node[1])
        self.il.emitAbstractCall("PyObject_Repr")
        return T_PYOBJECT

    def n_getattr(self, node):
#    print "n_getattr", node
        # Push the object we want the attribute from
        self.dispatch_and_convert(node[1])
        il = self.il
        # The name of the attribute
        il.pushConstant(node[2], T_COR_STRING)
        self.emitAbstractCall("PyObject_GetAttrString")
        return T_PYOBJECT

    def builtin_optimize_functions(self, builtin, args):
        # Most of these are too small to bother with functions for
        # (yet, anyway :-)
        il = self.il
        if builtin=='cast' and len(args)==2:
            self.dispatch_and_convert(args[0], T_COR_OBJECT)
            typ = self.get_cor_literal_type(args[1])
            il.EmitType(Opcodes.castclass, typ)
            t = T_COR_OBJECT
        elif builtin=='array' and len(args)==2:
            self.dispatch_and_convert(args[1], T_COR_INT) # Size of the array
            # The type of the array
            if args[0][0] != "const":
                raise source_error("builtin array() function must have string literal as first arg")
            typ = self.compiler.getCORType(args[0][1])
            il.EmitType(Opcodes.newarr, typ)
            t = T_COR_OBJECT
        elif builtin=='chr' and len(args)==1:
            self.dispatch_and_convert(args[0], T_COR_INT)
            il.pushConstant(1);
            ctor_info = self.compiler.getConstructorInfo(T_COR_STRING, (T_COR_CHAR,T_COR_INT))
            il.EmitConstructor(Opcodes.newobj, ctor_info)
            t = T_COR_STRING
        elif builtin=='len' and len(args)==1:
            self.dispatch_and_convert(args[0])
            il.emitAbstractCall("PyObject_Length")
            t = T_COR_INT
        elif builtin=='str' and len(args)==1:
            self.dispatch_and_convert(args[0])
            il.emitAbstractCall("PyObject_Str")
            t = T_PY_STRING
        elif builtin=='repr' and len(args)==1:
            self.dispatch_and_convert(args[0])
            il.emitAbstractCall("PyObject_Repr")
            t = T_PY_STRING
        elif builtin=='getattr' and len(args)==2:
            self.dispatch_and_convert(args[0])
            self.dispatch_and_convert(args[1], T_COR_STRING)
            il.emitAbstractCall("PyObject_GetAttrString")
            t = T_PY_STRING
        elif builtin=='setattr' and len(args)==3:
            self.dispatch_and_convert(args[0])
            self.dispatch_and_convert(args[1])
            self.dispatch_and_convert(args[2])
            il.emitAbstractCall("PyObject_SetAttr")
            t = T_VOID
        elif builtin=='type' and len(args)==1:
            # Emit the node, then get the IPyType on the stack.
            self.dispatch_and_convert(args[0], T_IPYTYPE)
            # Cast the IPyType to a System.Object
            il.EmitType(Opcodes.castclass, self.compiler.getCORType(T_COR_OBJECT))
            # Wrap the system.object up as a class instance.
            il.EmitConversion(T_COR_OBJECT, T_PYOBJECT)
            t = T_PYOBJECT
        else:
            return 0, None
        return 1, t

    def _EmitArgsCOMCall(self, info, args, star_args, dstar_args):
        il = self.il
        if star_args is not None or dstar_args is not None:
            raise source_error("Can't support extended call syntax when making direct COM+ calls")
        param_types = info.GetParameters()
        param_types = map(lambda info: info.ParameterType, param_types)
#        assert len(args) <= len(param_types), "Too many args for this function - expected %d, got %d!" % (len(param_types), len(args))
        for arg, typ in map(None, args, param_types[:len(args)]):
            if arg[0]=='keyword':
                raise source_error("Keyword args not supported when making direct COM+ calls")
            self.dispatch_and_convert(arg, typ.FullName)
        # Emit any default values.
        if len(param_types) - len(args) > 0:
            pis = info.GetParameters()[len(args):]
            for pi in pis[len(args):]:
                assert pi.IsOptional, "Setting default value for param, but its not optional!"
                def_val = glue.ParameterInfo_DefaultValue(pi)
                il.pushConstant(def_val, pi.ParameterType.FullName)

    def _EmitArgsPyCall(self, args, star_args, dstar_args, dummy_self = 0):
        # Only pushes args and kw - not self!
        il = self.il
        # Push the args as an array
        emitters = []
        kw_infos = []
        if dummy_self:
            emitters.append( (il.pushConstant, (None, T_PYOBJECT)) )
        for arg in args:
            if arg[0]=='keyword':
                kw_infos.append( (arg[1], arg[2]) )
            else:
                emitters.append( (self.dispatch_and_convert, (arg,)) )
        il.emitArray(emitters)
        # Could try and optimize vararg combination when
        # there are no user args specified, although we would still need
        # to handle non-tuple star_args (so I didnt bother :-)
        if star_args is not None:
            self.dispatch(star_args)
            il.emitAbstractCall("__CombineVarArgs")

        if len(kw_infos)!=0:
            il.EmitNewObject("dict")
            for key, val_node in kw_infos:
                il.Emit(Opcodes.dup)
                il.pushConstant(key, T_PYOBJECT)
                self.dispatch_and_convert(val_node)
                self.emitAbstractCall("PyObject_SetItem")
        else:
            # No keywords
            il.pushConstant(None, T_PYOBJECT)

    def n_call(self, node):
        il = self.il
        # The standard "native" Python signature.
        pysig_arg_types = map(self.compiler.getCORType, [T_PYOBJECT, T_PYOBJECT_ARRAY, T_PYOBJECT])

        # Catch special case ctor and other literals (and later special names)
        if self.cctx is not None:
            cor_base_type, tail = self.get_cor_literal_type(node[1], 1)
            # XXX - we need to get smarter about looking for base classes
            # XXX   the method we seek may be in an indirect base!
            if cor_base_type is not None and \
                     self.cctx.base_class is not None and \
                     glue.Type_Equals(cor_base_type, self.cctx.base_class):
                # It is a literal call to a base class member.
                args = node[2][1:] # Drop the "self" - if it aint there, we are gunna barf!
                if tail=='__init__':
                    # Calling the base ctor
                    ctors = glue.Type_GetConstructors(cor_base_type)
                    ctors = map(COMAttributeWrapper, ctors)
                    # Until we get better type searching, any params supplied to the parent func
                    # must be of the same type as the params to this func
                    # Look for a ctor with my args

                    base_types = self.fctx.descriptor.param_types
                    input_args = base_types[:len(args)]

                    ctor_infos = find_best_methods(None, ctors, input_args)
                    if not ctor_infos:
                        raise source_error("Can not find the base class constructor being called")
                    else:
                        if len(ctor_infos)>1:
                            param_infos = ctor_infos[0].GetParameters()
                            param_infos = map(lambda info : info.ParameterType.FullName, param_infos)
                            sig = string.join(param_infos, ", ")
                            self.compiler.warning(2, "%d possible constructor matches - using %s(%s)" % (len(ctor_infos), cor_base_type.FullName,sig))
                        ctor_info = ctor_infos[0]

                    # Call this ctor.
#                    tok, typ = self.fctx.getDeclaredLocal(self.fctx.self_name)
                    il.EmitLdArg(0)
                    self._EmitArgsCOMCall(ctor_info, args, node.star_args, node.dstar_args)
                    # And make the call.
                    il.EmitConstructor(Opcodes.Call, ctor_info)
                    self.fctx.called_cor_base = 1
                    return T_VOID
                else: # Not the ctor - just any old base-class method.
                    method_info = None
                    if tail == self.fctx.name: # We are calling _our_ base implementation.
                        # Look for a base with my args
                        base_types = self.fctx.descriptor.param_types
                        method_info = glue.Type_GetMethodArgs( cor_base_type, tail, base_types )
                    else:
                        # We are calling some other base class method using "base.method(self, ...)" syntax
                        # (probably to avoid calling our method of the same name.
                        poss_infos = glue.Type_GetMethods(cor_base_type)
                        poss_infos = map(COMAttributeWrapper, poss_infos)
                        poss_infos = find_best_methods(tail, poss_infos, (il.type_pyobject,) * len(args) )
                        if len(poss_infos)==1:
                            method_info = poss_infos[0]
                            params = method_info.GetParameters()
                            base_types = map(lambda info: info.ParameterType, params)

                    if method_info is None:
                        raise source_error("Can't find the matching base-class to call")
                    method_info = COMAttributeWrapper(method_info)
                    il.EmitLdArg(0) # push 'this'
                    if len(args) != len(base_types):
                        raise source_error("Arg count/type mismatch (if calling your base implementation, you can only call the base method with the exact same signature.")
                    for arg_node, to_type_ob in map(None, args, base_types):
                        to_type = to_type_ob.FullName
                        self.dispatch_and_convert(arg_node, to_type)
#                    ob_method_info = getattr(method_info, "_oleobj_", method_info)
                    ret_type = method_info.ReturnType
                    ret_type = ret_type.FullName
                    # And make the call.
                    il.EmitMethod(Opcodes.Call, method_info)
                    return ret_type

        args = node[2]
        # See if a COR literal we can call.
        poss_infos = []
        cor_type, tail = self.get_cor_literal_type(node[1], 1, 1)
        if cor_type is not None and tail:
            poss_infos = glue.Type_GetMethods(cor_type)
            name_match = tail
            is_ctor = 0
        else:
            cor_type = self.get_cor_literal_type(node[1])
            if cor_type is not None:
                # Before looking for real ctors, look for a static __init__ function!
                # (Later the real ctor will be private, so the order of checking wont matter)
                poss_infos = map(COMAttributeWrapper, glue.Type_GetMethods(cor_type))
                poss_infos = filter( lambda info: info.IsStatic, poss_infos)
                poss_infos = filter(lambda info: info.Name == "__init__", poss_infos)
                if not poss_infos:
                    # No special ctor - use a regular one.
                    poss_infos = glue.Type_GetConstructors(cor_type)
                name_match = None
                is_ctor = 1

        method_info = None # May actually be a ctor_info!
        is_py_sig = 0

        # Finally, get the exact method info we will use to make the call
        # (or None to get runtime binding)
        if poss_infos:
            poss_infos = map(COMAttributeWrapper, poss_infos)
            # And a hack for Python classes or functions - if we have a signature of the
            # form "static foo(PyObject, PyObject[], PyObject)", we use this!
            pysig_infos = find_best_methods(name_match, poss_infos, pysig_arg_types, 1, 1)
            if len(pysig_infos)>0:
                is_py_sig = 1
                poss_infos = pysig_infos
                assert len(poss_infos)==1, "Cant have more than one sig for this method!"
            else:
                input_args = [self.compiler.getCORType(T_COR_OBJECT)] * len(args)
                poss_infos = find_best_methods(name_match, poss_infos, input_args, not is_ctor)

            if not poss_infos:
                self.compiler.verbose(2,"Can't find a matching method - deferring call to runtime")
            elif len(poss_infos)>1:
##                for i in poss_infos:
##                    print "have info", format_method_info(i)
                self.compiler.warning(2,"Too many overloaded methods '%s' - deferring bind until runtime" % (name_match,))
            else:
                method_info = poss_infos[0]
        # Finally, if we have the method info, make the call.
        if method_info:
            if is_py_sig:
                # Call the method.
                if is_ctor:
                    il.pushConstant(None, T_PYOBJECT)
                    # Emit a NULL self.
                    self._EmitArgsPyCall(args, node.star_args, node.dstar_args, 1)
                    il.EmitMethod(Opcodes.Call, method_info)
                    return T_PYOBJECT
                else:
                    # XXX - here is a gross hack :-)
                    # Our builtins take advantage of being able to call Runtime functions directly.
                    # However, PyObject_Call has the same sig as a generated Python function, so gets handled
                    # here.  However, the arg handling for this function is different.
                    if method_info.Name == "PyObject_Call":
                        assert len(args) == 3, "You must call this with 3 args"
                        self.dispatch_and_convert(args[0], T_PYOBJECT)
                        self.dispatch_and_convert(args[1], T_PYOBJECT_ARRAY)
                        self.dispatch_and_convert(args[2], T_PYOBJECT)
                    else:
                        il.pushConstant(None, T_PYOBJECT)
                        self._EmitArgsPyCall(args, node.star_args, node.dstar_args)
                    il.EmitMethod(Opcodes.Call, method_info)
                    return T_PYOBJECT
            else:
                # A standard COM sig
                # Setup the args.
                self._EmitArgsCOMCall(method_info, args, node.star_args, node.dstar_args)
                # And make the call or construct the object.
                if is_ctor:
                    self.il.EmitConstructor(Opcodes.Newobj, method_info)
                    return T_COR_INSTANCE
                else:
                    if method_info.IsVirtual:
                        opcode = Opcodes.Callvirt
                    else:
                        opcode = Opcodes.Call
                    self.il.EmitMethod(opcode, method_info)
                    return method_info.ReturnType.FullName

        # Catch special case builtins:
        if node[1][0] == 'name' and self.namespace_local.allowPotentialSpecialBuiltin(node[1][1]):
            builtin = node[1][1]
            # Specially optimize builtins we can
            ok, t = self.builtin_optimize_functions(builtin, args)
            if ok: return t
#      self.compiler.verbose(1, "have potential builtin to optimize: '%s'" % (builtin))

        # Push the callable object on the stack.
        self.dispatch_and_convert(node[1])
        self._EmitArgsPyCall(args, node.star_args, node.dstar_args)
        self.emitAbstractCall("PyObject_Call")
        return T_PYOBJECT

    def n_ellipsis(self, node):
        raise source_error('not yet implemented')

    def n_sliceobj(self, node):
        raise source_error('not yet implemented')

    def n_slice(self, node):
        il = self.il
        if node[2] == OP_APPLY:
            # Push the object
            self.dispatch_and_convert(node[1])
            # The first index
            if node[3] is None:
                il.pushConstant(0)
            else:
                self.dispatch_and_convert(node[3], T_COR_INT)
            if node[4] is None:
                il.pushConstant(COR_MAXINT) 
            else:
                self.dispatch_and_convert(node[4], T_COR_INT)
            self.emitAbstractCall("PySequence_GetSlice")
            return T_PYOBJECT
        elif node[2] == OP_ASSIGN:
            # Value on stack - store in a local.
            local_token = self.fctx.ensureLocalDeclared("$sliceass")
            il.EmitLocal(Opcodes.stloc, local_token)
            self.dispatch_and_convert(node[1])
            # Indexes.
            if node[3] is None:
                il.pushConstant(0)
            else:
                self.dispatch_and_convert(node[3], T_COR_INT)
            if node[4] is None:
                il.pushConstant(COR_MAXINT) 
            else:
                self.dispatch_and_convert(node[4], T_COR_INT)
            il.EmitLocal(Opcodes.ldloc, local_token)
            il.emitAbstractCall("PySequence_SetSlice")
            return T_VOID
        else:
            raise source_error("Can't do slices of type '%s'" % (node[2],))

    def binary(self, method_name, node):
        self.dispatch_and_convert(node[1][0])
        for n in node[1][1:]:
            self.dispatch_and_convert(n)
            # and call the method
            self.il.emitAbstractCall(method_name)
        return T_PYOBJECT

    def unary(self, method_name, node):
        self.dispatch_and_convert(node[1])
        # Call the method
        self.il.emitAbstractCall(method_name)
        return T_PYOBJECT

    def name_load(self, name):
        where = self.namespace_local.lookup(name)
        if where == W_GLOBAL:
            context = self.namespace_global.context
        elif where==W_LOCAL:
            context = self.namespace_local.context
        else:
            raise error, "dont know how to push a value from '%s'" % (where,)
        if context.uses_com_locals:
            tok, typ = context.getDeclaredLocal(name)
            if self.fctx and self.fctx.ctor_self_tok==tok:
                self.il.EmitLdArg(0)
            else:
                self.il.EmitLocal(Opcodes.ldloc, tok)
            ret_t = typ
        else:
            self.il.EmitField(Opcodes.ldsfld, context.fb_dict)
            self.il.pushConstant(name)
            self.emitAbstractCall("__LookupGlobal")
            ret_t = T_PYOBJECT
        return ret_t

    def name_assign(self, name):
        # Emit the code to assign the top of the stack to the given name.
        w = self.namespace_local.assign(name)
        il = self.il
        assert type(name)==types.StringType, "Variable name must be a string!"
        if w==W_GLOBAL:
            context = self.namespace_global.context
        elif w==W_LOCAL:
            # This is likely to fail with our use of self!
            if self.cctx is not None and self.fctx is not None and self.fctx.self_name == name:
                self.compiler.warning(1, "Attempt to re-assign the 'self' parameter is likely to fail verification or execution")
            context = self.namespace_local.context
        else:
            raise error, "Dont know how to assign from location '%s'" % (w,)
#      print "Assign", w, t, context
        if context.uses_com_locals:
            local_token = context.ensureLocalDeclared(name)
            # Make the assignment.
            il.EmitLocal(Opcodes.stloc, local_token)
        else:
            # Emit the code to assign to the module dictionary.
            # Perform the assignment
            il.EmitField(Opcodes.ldsfld, context.fb_dict)
            # The dict key - ie, the variable name
            il.pushConstant(name, T_PYOBJECT)
            self.emitAbstractCall("__Assign")
        return T_VOID

    def name_delete(self, name):
            # This would remove a local, or a global
            # If a local, raise an exception
            il = self.il
            if self.namespace_local.lookup(name) == W_LOCAL:
                raise source_error("Can't delete local variable: %s - don't really know what that means in IL!!" % node[1])
            # If a declared global, delete it
            if self.namespace_local.lookup(name) == W_GLOBAL:
                il.EmitField(Opcodes.ldsfld, self.namespace_global.context.fb_dict)
                il.pushConstant(name, T_PYOBJECT)
                il.emitAbstractCall("PyObject_DelItem")
            # Otherwise we don't know where it is.
            # Throw an exception, to complain about it.
            else:
                raise source_error("can't find variable to delete: %s" % name)

class AssemblyContext:
    def __init__(self, compiler):
        self.compiler = compiler

#        assembly_access = constants.AssemblyBuilderAccess_Run
        assembly_access = constants.AssemblyBuilderAccess_RunAndSave

        output_path, output_file = os.path.split(compiler.assembly_output_filename)
        options = compiler.options
        app_domain = glue.Thread_GetDomain()
        ass_name = COMAttributeWrapper(glue.CreateAssemblyName())
        ass_name.Name = options.assembly_name or os.path.splitext(os.path.basename(output_file))[0]
#        ass_name.FullName = options.assembly_fullname or output_file
#        ass_name.Description = options.assembly_desc or output_file # XXX - should use the doc-string for this!
#        ass_name.DefaultAlias = options.assembly_alias or output_file
# seem to have dropped the Description and DefaultAlias attributes!
        if options.assembly_keyfile:
            key = glue.LoadStrongNameKeyPair(options.assembly_keyfile)
            ass_name.KeyPair = key
        if output_path:
            self.ass_builder = glue.AppDomain_DefineDynamicAssemblyPath(app_domain, ass_name, assembly_access, output_path)
        else:
            self.ass_builder = glue.AppDomain_DefineDynamicAssembly(app_domain, ass_name, assembly_access)
#        self.ass_builder = app_domain.DefineDynamicAssembly(ass_name, assembly_access) # Broke in 1626 (and still in 2728?

    def finalize(self):
        try:
            COMAttributeWrapper(self.ass_builder).Save(os.path.basename(self.compiler.assembly_output_filename))
        except pythoncom.com_error, (hr, msg, exc, arg):
            if exc:
                hr = exc[0] or exc[-1]
                msg = exc[2] or msg
            raise EnvironmentError(hr, "Error creating output file - " + msg, self.compiler.assembly_output_filename)

##  def run(self):
##    if self.outputFile is not None:
##      raise error, "You can only run code that is not persisted."
##    typ = glue.GetTypeX("$__main__")
##    mi = typ.GetMethod_2("$__main__")
##    mi.Invoke(None, None)
#    typ.InvokeMember("$__main__", BindingFlags.InvokeMethod, None, None, None)

class CORModuleBuilder:
    """A class used to hide the COM integration problems with COM+.
    Hides the fact we need a real module object, and a "helper" module
    object
    """
    def __init__(self, mod):
        self.mod = mod
        # Add an "_oleobj_" so the PythonCOM framework
        # passes the correct object along.
        self._oleobj_ = self.mod._oleobj_
        self.mhelp = client.Dispatch('P2IL2.ModuleHelper')
        self.mhelp.SetModule(self.mod)
    def __getattr__(self, name):
        try:
            # Give the helper first look-in
            return getattr(self.mhelp, name)
        except AttributeError:
            rc = getattr(self.mod, name)
            setattr(self, name, rc)
            return rc

class Context:
    def __init__(self):
        self.methBuilder = None

class ModuleContext(Context):
    uses_com_locals = 0
    def __init__(self, compiler, cor_module, source_url, module_name, output_file):
        Context.__init__(self)
        self.compiler = compiler
        self.mod = CORModuleBuilder(cor_module)
#    guid_null = pythoncom.IID_NULL
        if compiler.options.debug_info:
            self.docwriter = self.mod.DefineDocument(source_url) # , guid_null, guid_null, guid_null)

        # Define the dictionary to be used as the module namespace.
        self.type_dict = compiler.getCORType(T_PYOBJECT)
        self.type_list = compiler.getCORType(T_PYOBJECT)

        # Define a TypeBuilder for the static class I use to generate the code.
        self.typeBuilder = COMAttributeWrapper(self.mod.DefineType(module_name + "$main", constants.TypeAttributes_Public))
#        self.fb_dict = self.typeBuilder.DefineField("__dict__", self.type_dict, constants.FieldAttributes_Public | constants.FieldAttributes_Static)
        self.fb_dict = glue.TypeBuilder_DefineField(self.typeBuilder, "__dict__", self.type_dict, constants.FieldAttributes_Public | constants.FieldAttributes_Static)

        # MethodBuilder for the module init code.
        attr = constants.MethodAttributes_Static | constants.MethodAttributes_Public
        self.methBuilder = glue.TypeBuilder_DefineMethod(self.typeBuilder, "$main", attr, None, [])
#        self.methBuilder = self.typeBuilder.DefineMethod("$main", attr, None, None)
        self.il = ILGenerator( self.methBuilder, compiler )

        # Emit the special initializer code for __dict__
        self.il.EmitNewObject("dict")
        self.il.Emit(Opcodes.dup) # For the __name__ set
        self.il.EmitField(Opcodes.stsfld, self.fb_dict)
        # Add __name__ to __dict__
        self.il.pushConstant("__name__", T_PYOBJECT)
        self.il.pushConstant(module_name, T_PYOBJECT)
        self.il.emitAbstractCall("PyObject_SetItem")

        self.mb_helper_not_enough_args = None
        self.mb_helper_too_many_args = None
        self.mb_helper_not_exact_args = None

    def finalize(self):
        self.il.Emit(Opcodes.Ret) # Emit the void return from our module init code.

        # If we are writing an EXE, then we dont bother with a class constructor - just define
        # an entry point.  If building a module, then we have a class ctor, but no entry point.
        if self.compiler.options.dll:
            # Define a class initializer, which simply calls our entry-point.
            cctor_builder = self.typeBuilder.DefineTypeInitializer()
            cctor_il = ILGenerator( cctor_builder, self.compiler )
            cctor_il.EmitMethod(Opcodes.Call, self.methBuilder)
            cctor_il.Emit(Opcodes.Ret) # Emit the void return.
        else:
            # set the .exe entry point
            COMAttributeWrapper(self.compiler.actx.ass_builder).SetEntryPoint(self.methBuilder)
#            self.mod.SetEntryPoint(self.methBuilder)

        # Bake the type.
        self.typeBuilder.CreateType()
        self.typeBuilder = None

        self.il = None # Clean up a circle!

    def _MakeHelperArgsException(self, func_name, err_prefix):
        attr = constants.MethodAttributes_Static | constants.MethodAttributes_Assembly
        ret_type = self.compiler.getCORType("System.Exception")
        arg_types = map(self.compiler.getCORType, ["System.String", "System.Int32", "System.Int32"])
        mb = glue.TypeBuilder_DefineMethod(self.typeBuilder, func_name, attr, ret_type, arg_types)
#        mb = self.typeBuilder.DefineMethod(func_name, attr, ret_type, arg_types)
        il = ILGenerator( mb, self.compiler )
        il.pushConstant("%s arguments for '{0}' - expected {1}, got {2}" % (err_prefix,) )
        il.EmitLdArg(0)
        il.EmitLdArg(1) # Num needed
        il.EmitConversion(T_COR_INT, T_COR_OBJECT)
        il.EmitLdArg(2) # Num got
        il.EmitConversion(T_COR_INT, T_COR_OBJECT)
        il.EmitMethodCall("System.String", "Format", (T_COR_STRING, T_COR_OBJECT, T_COR_OBJECT, T_COR_OBJECT))
        ctor_info = il.GetRuntimeExceptionConstructor(TypeError)
        il.EmitConstructor(Opcodes.newobj, ctor_info)
        il.Emit(Opcodes.ret)
        return mb

    def GetHelperTooManyArgsException(self):
        if self.mb_helper_too_many_args is None:
            self.mb_helper_too_many_args = self._MakeHelperArgsException("$TooManyArgs", "too many")
        return self.mb_helper_too_many_args

    def GetHelperNotEnoughArgsException(self):
        if self.mb_helper_not_enough_args is None:
            self.mb_helper_not_enough_args = self._MakeHelperArgsException("$NotEnoughArgs", "not enough")
        return self.mb_helper_not_enough_args

    def GetHelperNotExactArgsException(self):
        if self.mb_helper_not_exact_args is None:
            attr = constants.MethodAttributes_Static | constants.MethodAttributes_Assembly
            ret_type = self.compiler.getCORType("System.Exception")
            arg_types = map(self.compiler.getCORType, ["System.String", "System.Int32"])
            mb = glue.TypeBuilder_DefineMethod(self.typeBuilder,"$NotExactArgs", attr, ret_type, arg_types)
#            mb = self.typeBuilder.DefineMethod("$NotExactArgs", attr, ret_type, arg_types)
            il = ILGenerator( mb, self.compiler )
            il.pushConstant("'{0}' requires exactly {1} arguments")
            il.EmitLdArg(0)
            il.EmitLdArg(1) # Num needed
            il.EmitConversion(T_COR_INT, T_COR_OBJECT)
            il.EmitMethodCall("System.String", "Format", (T_COR_STRING, T_COR_OBJECT, T_COR_OBJECT))
            ctor_info = il.GetRuntimeExceptionConstructor(TypeError)
            il.EmitConstructor(Opcodes.newobj, ctor_info)
            il.Emit(Opcodes.ret)
            self.mb_helper_not_exact_args = mb
        return self.mb_helper_not_exact_args

class ClassContext(Context):
    uses_com_locals = 0
    def __init__(self, compiler, mctx, name, base_class, base_interfaces):
        Context.__init__(self)
        self.is_pyonly = 0 # Is this a Python only class (ie, everything using static Python style sigs)
        self.compiler = compiler
        self.registered_properties = {}
        self.typeBuilder = COMAttributeWrapper(mctx.mod.DefineTypeWithBases(name, constants.TypeAttributes_Public, base_class, base_interfaces))
#        print "base class is", `base_class`, base_class.FullName
        self.name = name
        self.seen_ctor = 0
        if base_class is None:
            self.all_cor_bases = base_interfaces
        else:
            self.all_cor_bases = [base_class] + base_interfaces
        self.base_class = base_class

        # Define the __dict__ for the Class itself (_not_ the instance!)
        self.type_dict = mctx.type_dict
        # Hrm - cant give classes and instances the same name.  Maybe make it a property?  Euw.
        self.fb_dict = glue.TypeBuilder_DefineField(self.typeBuilder, "__class_dict__", self.type_dict, constants.FieldAttributes_Public | constants.FieldAttributes_Static)

        self.fb_bases = glue.TypeBuilder_DefineField(self.typeBuilder, "__bases__", compiler.getCORType(T_PYOBJECT), constants.FieldAttributes_Public | constants.FieldAttributes_Static)

        # Create the class constructor, and create our IL generator.
        cctor_builder = self.typeBuilder.DefineTypeInitializer()
        il = self.il = ILGenerator( cctor_builder, self.compiler )
        # Emit the special initializer code for __dict__
        il.EmitNewObject("dict")
        il.EmitField(Opcodes.stsfld, self.fb_dict)

        # And a type builder for __dict__ for any instances.
        self.fb_inst_dict = glue.TypeBuilder_DefineField(self.typeBuilder, "__dict__", self.type_dict, constants.FieldAttributes_Public)
        
    def register_property(self, prop_name):
        # If the parent knows about it, don't re-define it.
        for base in self.all_cor_bases:
            if glue.Type_GetProperty(base, prop_name) is not None:
                break
        else: # for not broken
            self.registered_properties[prop_name] = None
            
    def EmitInstanceInit(self, il):
        il.EmitLdArg(0)
        il.EmitNewObject("dict")
        il.EmitField(Opcodes.stfld, self.fb_inst_dict)

    def EmitBaseCtors(self):
        base_class = self.base_class
        if base_class is None:
            base_class = self.compiler.getCORType("System.Object")
        ctor_infos = glue.Type_GetConstructors(base_class)
        ctor_infos = map(COMAttributeWrapper, ctor_infos)
        # Only want ctors from my _direct_ base, not indirect bases.
#    ctor_infos = filter(lambda info, base=base: glue.Type_Equals(base, :
        for ctor_info in ctor_infos:
            param_types = ctor_info.GetParameters()
            param_types = map(lambda info: info.ParameterType, param_types)
            attr = ctor_info.Attributes
            if attr & (constants.MethodAttributes_Public|constants.MethodAttributes_Static) != constants.MethodAttributes_Public:
                continue
            methBuilder = glue.TypeBuilder_DefineConstructor(self.typeBuilder, attr, 0, param_types)
#            methBuilder = self.typeBuilder.DefineConstructor(attr, 0, param_types)
            il = ILGenerator( methBuilder, self.compiler )
            # Call this ctor - load the "this"
            il.EmitLdArg(0)
            # And each other param
            # XXX - faster way to get just the number of params?
            pis = ctor_info.GetParameters()
            for i in range(len(pis)):
                il.EmitLdArg(i+1)
            # And make the call.
            il.EmitConstructor(Opcodes.Call, ctor_info)
            # Init our instance
            self.EmitInstanceInit(il)
            il.Emit(Opcodes.ret)

    def create_properties(self):
        # Create all the properties as property accessor functions.
        if self.registered_properties: # and not self.is_pyonly:
            # Define the generic property getter/setter.
            obj_type = self.compiler.getCORType("System.Object")
            str_type = self.compiler.getCORType("System.String")
            method_attributes = constants.MethodAttributes_Private
#            methBuilder = self.typeBuilder.DefineMethod("get$", method_attributes, obj_type, (str_type,))
            methBuilder = glue.TypeBuilder_DefineMethod(self.typeBuilder, "get$", method_attributes, obj_type, (str_type,))
            il = ILGenerator( methBuilder, self.compiler )
#      local_ret = glue.ConstructorBuilder_DeclareLocal(self.methBuilder, obj_type)
            local_token = il.DeclareLocal(obj_type)
            if il.emit_symbolic_info:
                local_token.SetLocalSymInfo("$ret")
            il.pushConstant(None)
            il.EmitLocal(Opcodes.stloc, local_token)

            # get the __dict__ for the class.
            il.BeginExceptionBlockX()

            il.EmitLdArg(0)
            il.EmitField(Opcodes.ldfld, self.fb_inst_dict)
            il.EmitLdArg(1)
            il.EmitConversion(T_COR_STRING, T_PY_STRING)
            il.emitAbstractCall("PyObject_GetItem")
            il.EmitConversion(T_PYOBJECT, T_COR_OBJECT)
            il.EmitLocal(Opcodes.stloc, local_token)

#      il.BeginCatchBlock(self.compiler.getCORType("Python.Builtins.exceptions.KeyError"))
            il.BeginCatchBlock(self.compiler.getCORType("System.Exception")) # xxx - wierd errors if I try and use builtin errors!
            il.Emit(Opcodes.pop)
#            il.pushConstant(None)
            il.EndExceptionBlockX()
            il.EmitLocal(Opcodes.ldloc, local_token)
            il.Emit(Opcodes.ret)
            # Now the generic property setter.
#            methBuilderSetter = self.typeBuilder.DefineMethod("set$", method_attributes, None, (str_type, obj_type))
            methBuilderSetter = glue.TypeBuilder_DefineMethod(self.typeBuilder, "set$", method_attributes, None, (str_type, obj_type))
            il = ILGenerator( methBuilderSetter, self.compiler )
            il.EmitLdArg(0)
            il.EmitField(Opcodes.ldfld, self.fb_inst_dict)
            il.EmitLdArg(1)
            il.EmitConversion(T_COR_STRING, T_PY_STRING)
            il.EmitLdArg(2)
            il.EmitConversion(T_COR_OBJECT, T_PYOBJECT)
            il.emitAbstractCall("PyObject_SetItem")
            il.Emit(Opcodes.ret)
            # Now each property.
            method_attributes = constants.MethodAttributes_Public | constants.MethodAttributes_SpecialName
            prop_attr = 0
            for prop_name in self.registered_properties.keys():
                pb = glue.TypeBuilder_DefineProperty(self.typeBuilder, prop_name, prop_attr, obj_type, [])
                pb = COMAttributeWrapper(pb)
#                pb = self.typeBuilder.DefineProperty(prop_name, prop_attr, obj_type, [])
                # The Getter
                propMethBuilder = glue.TypeBuilder_DefineMethod(self.typeBuilder, "get_" + prop_name, method_attributes, obj_type, [])
#                propMethBuilder = self.typeBuilder.DefineMethod("get_" + prop_name, method_attributes, obj_type, None)
                il = ILGenerator( propMethBuilder, self.compiler )
                il.EmitLdArg(0)
                il.pushConstant(prop_name)
                il.EmitMethod(Opcodes.call, methBuilder)
                il.Emit(Opcodes.ret)
                pb.SetGetMethod(propMethBuilder)
                # And the Setter.
                propMethBuilder = glue.TypeBuilder_DefineMethod(self.typeBuilder, "set_" + prop_name, method_attributes, None, (obj_type,))
#                propMethBuilder = self.typeBuilder.DefineMethod("set_" + prop_name, method_attributes, None, (obj_type,))
                il = ILGenerator( propMethBuilder, self.compiler )
                il.EmitLdArg(0)
                il.pushConstant(prop_name)
                il.EmitLdArg(1)
                il.EmitMethod(Opcodes.call, methBuilderSetter)
                il.Emit(Opcodes.ret)
                pb.SetSetMethod(propMethBuilder)

    def finalize(self):
        self.il.Emit(Opcodes.Ret) # Emit the void return for our class ctor
        # User hasnt specified a base - or no ctor provided.
        if self.base_class is not None and \
             not self.seen_ctor:
            # If the user has not provided a ctor, we provide ctors that do nothing other than
            # call the base.
            self.EmitBaseCtors()

        self.create_properties()
        return self.typeBuilder.CreateType()

class FunctionContextBase(Context):
    uses_com_locals = 1
    def __init__(self, compiler, mctx, cctx, name, descriptor):
        Context.__init__(self)
        self.ctor_self_tok = None
        self.name = name
        self.compiler = compiler
        self.mctx = mctx
        self.cctx = cctx
        self.descriptor = descriptor
        self.emit_symbolic_info = compiler.options.debug_info

        self.CreateMethodBuilder(name, descriptor)
        self.il = ILGenerator( self.methBuilder, compiler )

        self.locals = {} # Map of locals for this function.
        self.initialize()
        cor_ret_type = descriptor.ret_type
        if cor_ret_type is None: cor_ret_type = compiler.getCORType(T_VOID)
        self.return_type_id = cor_ret_type.FullName
        if self.return_type_id != T_VOID:
            self.local_return = self.ensureLocalDeclared("$ret", self.return_type_id)
            if self.return_type_id == T_PYOBJECT:
                self.il.pushConstant(None, self.return_type_id)
            else:
                # XXX - could optimize ints etc away (but wouldnt the JIT do it anyway?)
                # XXX - WTF - doesnt work anyway!
                if cor_ret_type.IsValueType:
                    if self.return_type_id in [T_COR_BOOL, T_COR_INT]:
                        self.il.pushConstant(0)
                    else:
##          ctor = glue.Type_GetConstructor(cor_ret_type, ())
##          if ctor is None:
                        raise source_error("Dont know how to initialize the return value for an object of type '%s'" % (self.return_type_id,))
##          il.EmitType(Opcodes.newobj, cor_ret_type)
                else:
                    # Just init to NULL.
                    self.il.pushConstant(None)
            self.il.EmitLocal(Opcodes.stloc, self.local_return)
        self.label_end_method = self.il.CreateLabel2()
    def CreateMethodBuilder(self, cctx, name, descriptor):
        assert 0, "You must override me"
    def __repr__(self):
        return "<%s for func '%s'>" % (self.__class__.__name__, self.name)

    def finalize(self, ret_t):
        self.cctx = self.il = None
        return ret_t

    def initialize(self):
        pass
#    self.methBuilder.CreateMethodBody_2(self.il())

    def ensureLocalDeclared(self, name, typ = T_PYOBJECT):
        """Ensure the local variable is declared.  Returns the token for the variable"""
        tok = self.locals.get(name)
        if tok is not None:
            # already declared.
            assert tok[1] == typ, "Local variable requested with different type"
            return tok[0]
        if typ==T_COR_INSTANCE: typ = T_COR_OBJECT # XXX - this sucks!
        cor_typ = self.compiler.getCORType(typ)
#    tok = glue.MethodBuilder_DeclareLocal( self.methBuilder, typ)
        loc = self.il.DeclareLocal(cor_typ)
        if self.emit_symbolic_info:
            loc.SetLocalSymInfo(name)
        self.locals[name] = loc, typ
        return loc

    def getDeclaredLocal(self, name):
        ret = self.locals.get(name)
        if ret is None:
            raise source_error("Reference to unbound local variable '%s'" % (name,))
        return ret

class FunctionContext(FunctionContextBase):
    def CreateMethodBuilder(self, name, descriptor):
        if self.cctx is None:
            builder = self.mctx.typeBuilder
#      self.methBuilder = self.mctx.mod.DefineGlobalMethod(name, descriptor)
        else:
            builder = self.cctx.typeBuilder
#        self.methBuilder = builder.DefineMethod(name, descriptor.attr, descriptor.ret_type, descriptor.param_types)
        self.methBuilder = COMAttributeWrapper(glue.TypeBuilder_DefineMethod(builder, name, descriptor.attr, descriptor.ret_type, descriptor.param_types))

    def finalize(self, ret_t):
        assert ret_t == T_VOID, "Expecting the function node to be void!"
        self.il.MarkLabel2(self.label_end_method)
        if self.return_type_id != T_VOID:
            self.il.EmitLocal(Opcodes.ldloc, self.local_return)
        self.il.Emit(Opcodes.ret)
        return FunctionContextBase.finalize(self, ret_t)

class LambdaContext(FunctionContext):
    def finalize(self, ret_t):
        # Skip the pushing of a default return type - for lambda its already there.
        self.il.EmitConversion(ret_t, T_PYOBJECT)
        self.il.Emit(Opcodes.Ret)
        FunctionContextBase.finalize(self, ret_t)
        return T_PYOBJECT

class ConstructorContext(FunctionContextBase):
    def CreateMethodBuilder(self, name, descriptor):
        builder = self.cctx.typeBuilder
        attr = constants.MethodAttributes_Public | constants.MethodAttributes_RTSpecialName | constants.MethodAttributes_SpecialName
#        self.methBuilder = builder.DefineConstructor(attr, 0 , descriptor.param_types)
        self.methBuilder = COMAttributeWrapper(glue.TypeBuilder_DefineConstructor(builder, attr, 0, descriptor.param_types))

    def initialize(self):
        self.cctx.seen_ctor = 1
        self.called_cor_base = 0
        # ensure __dict__ is setup.
        il = self.il
        self.cctx.EmitInstanceInit(il)
        base_class = self.cctx.base_class
        if base_class is None:
            # If user has not specified any base classes, then emit the ctor now.
            base_class = self.compiler.getCORType("System.Object")
            # call the base.
            ctor_info = glue.Type_GetConstructor(base_class, ())
            # Call this ctor.
            il.EmitLdArg(0)
            il.EmitConstructor(Opcodes.Call, ctor_info)

    def finalize(self, ret_t):
        if self.cctx.base_class is None:
            assert not self.called_cor_base, "Should not have been able to call a COM+ base class when we dont have a parent."
        else:
            if not self.called_cor_base:
                self.compiler.warning(1, "Explicit constructor does not call COM+ base constructor")
#        raise source_error("Class with constructor derives from COM+ class, but base constructor was not called")
        self.il.MarkLabel2(self.label_end_method)
        self.il.Emit(Opcodes.Ret)
        return FunctionContextBase.finalize(self, ret_t)

_node_names = {
    'module' : 'n_module',
    'stmt' : 'n_stmt',
    'function' : 'n_function',
    'lambda' : 'n_lambda',
    'class' : 'n_class',
    'pass' : 'n_pass',
    'break' : 'n_break',
    'continue' : 'n_continue',
    'for' : 'n_for',
    'while' : 'n_while',
    'if' : 'n_if',
    'exec' : 'n_exec',
    'assert' : 'n_assert',
    'from' : 'n_from',
    'import' : 'n_import',
    'raise' : 'n_raise',
    'reraise' : 'n_reraise',
    'tryfinally' : 'n_tryfinally',
    'tryexcept' : 'n_tryexcept',
    'return' : 'n_return',
    'const' : 'n_const',
    'print' : 'n_print',
    'printnl' : 'n_printnl',
    'discard' : 'n_discard',
    'assign' : 'n_assign',
    'ass_tuple' : 'n_ass_tuple',
    'ass_list' : 'n_ass_list',
    'ass_name' : 'n_ass_name',
    'ass_attr' : 'n_ass_attr',
    'tuple' : 'n_tuple',
    'list' : 'n_list',
    'dict' : 'n_dict',
    'or' : 'n_or',
    'and' : 'n_and',
    'not' : 'n_not',
    'compare' : 'n_compare',
    'bitor' : 'n_bitor',
    'bitxor' : 'n_bitxor',
    'bitand' : 'n_bitand',
    '<<' : 'n_lshift',
    '>>' : 'n_rshift',
    '+' : 'n_plus',
    '-' : 'n_minus',
    '*' : 'n_star',
    '/' : 'n_slash',
    '%' : 'n_percent',
    'unary+' : 'n_uplus',
    'unary-' : 'n_uminus',
    'invert' : 'n_invert',
    'power' : 'n_power',
    'name' : 'n_name',
    'global' : 'n_global',
    'backquote' : 'n_backquote',
    'getattr' : 'n_getattr',
    'call_func' : 'n_call',
##  'keyword' : 'n_keyword',
    'subscript' : 'n_subscript',
    'ellipsis' : 'n_ellipsis',
    'sliceobj' : 'n_sliceobj',
    'slice' : 'n_slice',
}

if __name__=='__main__':
    print "Run gencode.py, not this!"