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

from genil_con import *
from win32com.client import constants
import types

# Reflection/Emit used to have a "descriptor"
# This object largely replaces theirs - maybe it should ultimately go,
# but it is a handy container to capture this info.
class Descriptor:
    def __init__(self, attr, ret_type, param_types):
        self.attr = attr
        self.ret_type = ret_type
        self.param_types = param_types


def find_matching_com_descriptors(generator, func):
    # The matching/signature rules.
    # * If the function has a _com_params_ attribute, you always
    #   get this single COM+ signature.  varargs or kwargs not
    #   supported and will cause a compile error.  None of the
    #   other rules are tested.

    # * If you are overloading a COM+ method (eg, ctor, virtual):
    # - If the Python function has varargs, you get a PyMethod
    #   signature, and thunks from all COM+ overrides with at least
    #   as many positional arguments (ie, zero positional args==
    #   every COM+ signature overloaded).

    # - If the function has no varargs or kwargs, a matching override
    #   with exactly that many parameters is searched for.  If found,
    #   that COM signature is used.

    self = generator
    num_args = len(func.argnames)
    if func.is_instance_method: num_args = num_args - 1
    num_positional_args = num_args
    if func.varargs: num_positional_args = num_positional_args - 1
    if func.kwargs: num_positional_args = num_positional_args - 1
    # See if we are overriding any COM+ signatures.
    cctx = generator.cctx
    # If not an instance method or no bases or interfaces, we can't have a COM+ sig.
    infos = []
    if not func.is_instance_method or len(cctx.all_cor_bases)==0:
        return infos
    if func.is_ctor:
        # If a ctor, only use sigs if we have a base
        if cctx.base_class:
            infos = glue.Type_GetConstructors(cctx.base_class)
            infos = map(COMAttributeWrapper, infos)
    else:
        # Look for methods
        for base in cctx.all_cor_bases:
            new_infos = glue.Type_GetMethods(base)
            new_infos = map(COMAttributeWrapper, new_infos)
            # Only interested in virtual methods with my name.
            check_info = lambda info, name=func.name: \
                                     info.Name == name and \
                                     info.Attributes & constants.MethodAttributes_Virtual == constants.MethodAttributes_Virtual
            new_infos = filter(check_info, new_infos)
#            print "base", base.FullName, "found", new_infos
            infos.extend(list(new_infos))
    if len(infos)>0:
        # I have at least one COM+ matching signature still, see
        # if I can find as many matching positional params.
        if func.varargs:
            check_info = lambda info, nargs=num_positional_args: \
                                     len(info.GetParameters())>=nargs
        else:
            check_info = lambda info, nargs=num_positional_args: \
                                     len(info.GetParameters())==nargs
        infos = filter(check_info, infos)
        if len(infos)>1 and not func.varargs:
                raise source_error("There is more than one method you may be overloading - either use varargs or add a _com_params_ reference.")
    # we get the descriptors for the infos left.
    ret = []
    for info in infos:
        param_types = info.GetParameters()
        param_types = map(lambda param_info: param_info.ParameterType, param_types)
        if func.is_ctor:
            ret_type = None
        else:
            ret_type = info.ReturnType
        attr = info.Attributes
        if attr & constants.MethodAttributes_Abstract:
            attr = attr & ~constants.MethodAttributes_Abstract
        ret.append(Descriptor(attr, ret_type, param_types))
    return ret
#        return map(lambda info : glue.MethodBase_GetDescriptor(info), infos)
#    return []

class MethodSignatureManager:
    def __init__(self, generator, func, descriptor):
        self.generator = generator
        self.func = func
        self.descriptor = descriptor

    def PrepareContext(self):
        # Called just before we enter the functions context
        pass 
    def EmitFuncHeader(self):
        raise NotImplementedError

    def EmitThunk(self, method_info):
        raise NotImplementedError

class PyMethodSignatureManager(MethodSignatureManager):
    def PrepareContext(self):
        # We emit the default args to a static array.  This is primarily to allow
        # mutable defaults, and defaults that are actually expressions that must
        # be evaluated at runtime.
        il = self.generator.il # Must still be the context where the defaults are evaluated.
        func = self.func
        defaults = func.defaults
        mctx = self.generator.mctx
        compiler = self.generator.compiler
        parent_ctx = self.generator.cctx or mctx

        if defaults:
            array_type = compiler.getCORType(T_PYOBJECT_ARRAY)
            array_name = self.generator.make_unique_name(func.name + "$default_args")
            self.fb_args = glue.TypeBuilder_DefineField(parent_ctx.typeBuilder, array_name, array_type, constants.FieldAttributes_Private | constants.FieldAttributes_Static)
            emitters = []
            for default in defaults:
                emitters.append( (self.generator.dispatch_and_convert, (default,) ) )
            il.emitArray(emitters)
            il.EmitField(Opcodes.stsfld, self.fb_args)

    # Unpack a single argument declared of the form (a, b, (c,d))
    def _DoEmitTupleArgUnpack(self, names, il, fctx):
        for i in range(len(names)):
            name = names[i]
            il.Emit(Opcodes.dup)
            il.pushConstant(i)
            il.emitAbstractCall("PySequence_GetItem")
            if type(name)==types.TupleType:
                self._DoEmitTupleArgUnpack(name, il, fctx)
            else:
                tok, typ = fctx.getDeclaredLocal(name)
                il.EmitConversion(T_PYOBJECT, typ)
                il.EmitLocal(Opcodes.stloc, tok)
        # XXX - really need a runtime check that the sequence isnt too large!!!
        il.Emit(Opcodes.pop)
        
    def DoEmitArgUnpack(self, il, fctx, arg_names):
        for i in range(len(arg_names)):
            il.EmitLdArg(1)
            il.pushConstant(i)
            il.EmitType(Opcodes.ldelema, il.type_pyobject)
            il.EmitType(Opcodes.ldobj, il.type_pyobject)
            this_name = arg_names[i]
            if type(this_name)==types.TupleType:
                self._DoEmitTupleArgUnpack(this_name, il, fctx)
            else:
                tok, typ = fctx.getDeclaredLocal(this_name)
                il.EmitConversion(T_PYOBJECT, typ)
                il.EmitLocal(Opcodes.stloc, tok)

    def _LoadPositionalArgs(self, func, arg_names, num_needed_args, num_positional_args, local_num_args):
        il = self.generator.il
        fctx = self.generator.fctx
        il.EmitLdArg(1)
        il.Emit(Opcodes.Ldlen)
        il.EmitLocal(Opcodes.stloc, local_num_args)
        if num_needed_args > 0:
            il.EmitLocal(Opcodes.ldloc, local_num_args)
            il.pushConstant(num_needed_args-1)
            il.Emit(Opcodes.cgt)
            label_load_nondefaults = il.CreateLabel2()
            il.EmitLabel(Opcodes.brtrue, label_load_nondefaults)
            il.pushConstant(func.name)
            il.pushConstant(num_needed_args)
            il.EmitLocal(Opcodes.ldloc,local_num_args)
            il.EmitMethod(Opcodes.call, self.generator.mctx.GetHelperNotEnoughArgsException() )
            il.Emit(Opcodes.throw)
            il.MarkLabel2(label_load_nondefaults)
            self.DoEmitArgUnpack(il, fctx, arg_names[:num_needed_args])
        # OK - all non-default params ready to roll.
        # Load up those with defaults.
        for i in range(len(func.defaults)):
            label_use_default = il.CreateLabel2()
            label_end_default = il.CreateLabel2()
            tok, typ = fctx.getDeclaredLocal(arg_names[i + num_needed_args])
            il.EmitLocal(Opcodes.ldloc, local_num_args)
            il.pushConstant(i + num_needed_args + 1)
            il.Emit(Opcodes.clt)
            il.EmitLabel(Opcodes.brtrue, label_use_default)
            # Get this one from the args.
            il.EmitLdArg(1)
            il.pushConstant(i + num_needed_args)

            il.EmitLabel(Opcodes.br, label_end_default)
            il.MarkLabel2(label_use_default)
            # Get this from the  default array.
            il.EmitField(Opcodes.ldsfld, self.fb_args)
            il.pushConstant(i)

            il.MarkLabel2(label_end_default)
            il.EmitType(Opcodes.ldelema, il.type_pyobject)
            il.EmitType(Opcodes.ldobj, il.type_pyobject)
            il.EmitConversion(T_PYOBJECT, typ)
            il.EmitLocal(Opcodes.stloc, tok)

    def _LoadVariableArgs(self, func, arg_names, num_needed_args, num_positional_args, local_num_args):
        il = self.generator.il
        fctx = self.generator.fctx
        if func.varargs:
            index_vararg = len(arg_names)-1
            if func.kwargs: index_vararg = index_vararg - 1
            label_end_varargs = il.CreateLabel2()
            label_empty_varargs = il.CreateLabel2()
            local_tok, typ = fctx.getDeclaredLocal(func.argnames[index_vararg])
            assert typ==T_PYOBJECT, "varargs should always be PyObjects!"
            il.EmitLocal(Opcodes.ldloc, local_num_args)
            
            il.pushConstant(num_needed_args + len(func.defaults))
            il.Emit(Opcodes.sub)
            il.Emit(Opcodes.dup)
            il.pushConstant(1)
            il.Emit(Opcodes.clt)
            il.EmitLabel(Opcodes.brtrue, label_empty_varargs)

            # We have some varargs
            # Create the new array
            local_num_copy = fctx.ensureLocalDeclared("$num_varargs", T_COR_INT)
            local_temp_array = fctx.ensureLocalDeclared("$varargs", T_PYOBJECT_ARRAY)
            il.Emit(Opcodes.dup)
            il.EmitType(Opcodes.newarr, il.type_pyobject)
            il.EmitLocal(Opcodes.stloc, local_temp_array)
            # Stash away how many to copy.
            il.EmitLocal(Opcodes.stloc, local_num_copy)
            # Copy the array - setup args
            il.EmitLdArg(1) # the source array.
            il.pushConstant(num_needed_args + len(func.defaults)) # start index
            il.EmitLocal(Opcodes.ldloc, local_temp_array) # The destination array
            il.pushConstant(0) # dest index
            il.EmitLocal(Opcodes.ldloc, local_num_copy) # how many to copy.
            # Copy the array - make the call
            il.EmitMethodCall("System.Array", "Copy", ("System.Array", "System.Int32", "System.Array", "System.Int32", "System.Int32"))
            il.EmitLocal(Opcodes.ldloc, local_temp_array)
            il.EmitLabel(Opcodes.br, label_end_varargs)
            il.MarkLabel2(label_empty_varargs)

            # No var args - pop the (0 or less) length from the stack, and make an empty one.
            il.Emit(Opcodes.pop)
            il.pushConstant(0)
            il.EmitType(Opcodes.newarr, il.type_pyobject)
            il.MarkLabel2(label_end_varargs)

            il.EmitNewObject("tuple")
            il.EmitLocal(Opcodes.stloc, local_tok)
        else: # no varargs - check they didnt give too many.
            il.EmitLocal(Opcodes.ldloc,local_num_args)
            il.pushConstant(num_positional_args+1)
            il.Emit(Opcodes.clt)
            label_max_ok = il.CreateLabel2()
            il.EmitLabel(Opcodes.brtrue, label_max_ok)

            il.pushConstant(func.name)
            il.pushConstant(num_positional_args)
            il.EmitLocal(Opcodes.ldloc,local_num_args)
            il.EmitMethod(Opcodes.call, self.generator.mctx.GetHelperTooManyArgsException() )
            il.Emit(Opcodes.throw)

            il.MarkLabel2(label_max_ok)

    def _EmitPrivateCtor(self):
        # XXX - this is a hack - the default ctor should not be generated here!
        cctx = self.generator.cctx
        from genil import ILGenerator # damn - circular imports :-(
        attr = constants.MethodAttributes_Private
#        def_ctor_info = cctx.typeBuilder.DefineConstructor(attr, 0, None)
        def_ctor_info = glue.TypeBuilder_DefineConstructor(cctx.typeBuilder, attr, 0, [])
        def_ctor_il = ILGenerator( def_ctor_info, self.generator.compiler )
        def_ctor_il.EmitLdArg(0)
        base_name = cctx.base_class
        if base_name is None: base_name = "System.Object"
        base_ctor_info = glue.Type_GetConstructor(self.generator.compiler.getCORType(base_name), ())
        if base_ctor_info is None:
            raise source_error("Can't find the default constructor!")
        def_ctor_il.EmitConstructor(Opcodes.call, base_ctor_info)
        cctx.EmitInstanceInit(def_ctor_il)
        def_ctor_il.Emit(Opcodes.ret)
        return def_ctor_info

    def EmitFuncHeader(self):
        func = self.func
        il = self.generator.il
        fctx = self.generator.fctx
        cctx = self.generator.cctx
        arg_names = func.argnames[:]
        
        if func.is_instance_method:
            self_local = fctx.ensureLocalDeclared(arg_names[0])

        # This is pretty painful to handle all the cases, and bloats the code somewhat!
        # So for the common case, or all positional args, no defaults, we optimize...
        if 0: # XXXXX - not func.varargs and not func.kwargs and len(func.defaults)==0:
            il.EmitLdArg(1)
            il.Emit(Opcodes.Ldlen)
            il.pushConstant(len(arg_names))
            il.Emit(Opcodes.ceq)
            label_args_ok = il.CreateLabel2()
            il.EmitLabel(Opcodes.brtrue, label_args_ok)
            il.pushConstant(func.name)
            il.pushConstant(len(arg_names))
            il.EmitMethod(Opcodes.call, self.generator.mctx.GetHelperNotExactArgsException() )
            il.Emit(Opcodes.throw)
            il.MarkLabel2(label_args_ok)
            self.DoEmitArgUnpack(il, fctx, arg_names)
        else:
            num_positional_args = len(arg_names)
            if func.varargs: num_positional_args = num_positional_args - 1
            if func.kwargs: num_positional_args = num_positional_args - 1
            num_needed_args = num_positional_args - len(func.defaults)
        
            local_num_args = fctx.ensureLocalDeclared("$num_args", T_COR_INT)
            self._LoadPositionalArgs(func, arg_names, num_needed_args, num_positional_args, local_num_args)
            # Defaults copied in - copy rest of varargs, or check not too many args.
            self._LoadVariableArgs(func, arg_names, num_needed_args, num_positional_args, local_num_args)
         
            if func.kwargs:
                local_tok, typ = fctx.getDeclaredLocal(arg_names[-1])
                assert typ==T_PYOBJECT, "keywords should always be PyObjects!"
                il.EmitLdArg(2)
                il.EmitLocal(Opcodes.stloc, local_tok)

        # Now do some "self" transformations for the __init__ (ie, the pretend constructor!)
        # XXX - probably should check self is indeed an instance (if its not, user will
        # see InvalidCast exceptions all over the place!)
        if func.is_ctor:
            il.EmitLocal(Opcodes.ldloc, self_local)
            il.EmitPyObjectDeref("ob")
            il.pushConstant(None)
            il.Emit(Opcodes.ceq)
            lab_end_self = il.CreateLabel2()
            il.EmitLabel(Opcodes.brfalse, lab_end_self)
            # Do the real object construction
            # XXX - this is a hack - the default ctor should not be generated here!
            def_ctor_info = self._EmitPrivateCtor()
            # Call this ctor - load the "this"
            il.EmitConstructor(Opcodes.newobj, def_ctor_info)
            il.EmitConversion(T_COR_OBJECT, T_PYOBJECT)
            il.EmitLocal(Opcodes.stloc, self_local)
            # Finally set the new self up as the result.
            il.EmitLocal(Opcodes.ldloc, self_local)
            il.EmitLocal(Opcodes.stloc, fctx.local_return)

            il.MarkLabel2(lab_end_self)


    def EmitFunctionObject(self, method_info):
        il = self.generator.il
        mctx = self.generator.mctx
        getCORType = self.generator.compiler.getCORType
        # Create the delegate object
        il.Emit(Opcodes.ldnull)
        il.EmitMethod(Opcodes.ldftn, method_info)

        delegate_type = getCORType("Python.Builtins.types.PyBuiltinMethodDelegate")
        ctor_info = glue.Type_GetConstructor(delegate_type, (getCORType("System.Object"), getCORType("System.IntPtr")))
        assert ctor_info is not None, "Can't find a matching constructor for the method delegate!"
        il.EmitConstructor(Opcodes.Newobj, ctor_info)
        il.pushConstant(None, T_PYOBJECT)
        il.pushConstant(None, T_PYOBJECT)
        il.EmitNewObject("builtin-method")

class CORMethodSignatureManager(MethodSignatureManager):
    def EmitFuncHeader(self):
        func = self.func
        fctx = self.generator.fctx
        mctx = self.generator.mctx
        arg_names = func.argnames[:]
        il = self.generator.il
        compiler = self.generator.compiler

        first_cor_arg = 0
        if func.is_instance_method:
            tok = fctx.ensureLocalDeclared(arg_names[0], T_COR_INSTANCE) # type-spec is a special hack needed for ctors!
            del arg_names[0]
            first_cor_arg = 1
            if func.is_ctor:
                fctx.ctor_self_tok = tok
            else:
                il.EmitLdArg(0)
                il.EmitLocal(Opcodes.stloc, tok)

        param_types = self.descriptor.param_types
        param_types = map(lambda t: t.FullName, param_types)
        assert len(arg_names) == len(param_types), "Arg number mismatch - %d, %d!" % (len(arg_names), len(param_types))
        for i, name, param_typ in map(None, range(len(arg_names)), arg_names, param_types):
            local_tok, local_typ = fctx.getDeclaredLocal(name)
            il.EmitLdArg(i+first_cor_arg)
            il.EmitConversion(param_typ, local_typ)
            il.EmitLocal(Opcodes.stloc, local_tok)

            param_attr = constants.ParameterAttributes_In
            # Setup the default value if specified.
            default_index = i - (len(arg_names) - len(func.defaults))
            have_default = 0
            if default_index >= 0:
                param_attr = param_attr | constants.ParameterAttributes_Optional | constants.ParameterAttributes_HasDefault
                default_node = func.defaults[default_index]
                if default_node[0]=="const":
                    have_default = 1
                    default_val = default_node[1]
                elif default_node[0]=="name" and default_node[1]=="None":
                    have_default = 1
                    default_val = None
                else:
                    compiler.warning(1, "Only constants can be default values for a COM+ signatures (%s)" % (`default_node`,))
#            pi = fctx.methBuilder.DefineParameter(i+1, glue.ParameterAttributes(param_attr), name)
            pi = glue.MethodBuilder_DefineParameter(fctx.methBuilder, i+1, param_attr, name)
            if have_default:
                # *sigh*
                if default_val is not None:
                    print "Skipping correct default arg constant"
##                print "Default val is", default_val, type(default_val), param_typ
##                if default_val is None:
                glue.ParameterBuilder_SetConstantNull(pi)
##                else:
##                    glue.ParameterBuilder_SetConstantInt(pi, default_val)
#                    pi.SetConstant(default_val)

    def EmitFunctionObject(self, method_info):
        il = self.generator.il
        if self.func.is_instance_method:
            # Emitting unbound method
            il.EmitGetType(self.generator.cctx.typeBuilder)
            il.Emit(Opcodes.Ldnull)
            il.pushConstant(self.func.name)
            il.EmitNewObject("method")
        else:
            # Function object.
            il.EmitGetType(self.generator.mctx.typeBuilder)
            il.pushConstant(self.func.name)
            il.EmitNewObject("function")

        return T_PYOBJECT

    def EmitThunk(self, method_info):
        # Pretty simple -
        # * If instance method, pass self.
        # * All positional args copied to PyObject array.
        # * Keywords is always None
        il = self.generator.il
        descriptor = self.descriptor
        mctx = self.generator.mctx
        if self.func.is_instance_method:
            il.EmitLdArg(0)
            il.EmitConversion(T_COR_INSTANCE, T_PYOBJECT)
            next_arg = 1
        else:
            il.pushConstant(None, T_PYOBJECT)
            next_arg = 0
        param_types = self.descriptor.param_types
        emitters = []
        for t in param_types:
            type_name = t.FullName
            emitters.append( (self._ParamEmitter, (next_arg, type_name)) )
            next_arg = next_arg + 1
        il.emitArray(emitters)
        # The Keywords
        il.pushConstant(None, T_PYOBJECT)

        # The call.
        try:
            il.EmitMethod(Opcodes.call, method_info)
        except pythoncom.com_error:
            il.EmitConstructor(Opcodes.call, method_info)

        ret_type = descriptor.ret_type
        ret_type_name = ret_type.FullName
        if not ret_type_name == T_VOID:
            il.EmitConversion(T_PYOBJECT, ret_type_name)
            il.EmitLocal(Opcodes.stloc, self.generator.fctx.local_return)

    def _ParamEmitter(self, arg_num, t):
        self.generator.il.EmitLdArg(arg_num)
        return t
