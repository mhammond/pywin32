// Portions Copyright 1999-2000 Microsoft Corporation.
// Portions Copyright 1997-1999 Greg Stein and Bill Tutt.
//
// This source code may be freely distributed, as long as all
// copyright information remains in place.
//
// See also the copyrights for the version of Python you are using.
//
// Implemented 1999-2000 by Mark Hammond (MarkH@ActiveState.com)
// and Greg Stein (gstein@lyra.org)
//
// See http://www.ActiveState.com/.NET for the latest versions.

using System;
using System.Reflection;
using System.Reflection.Emit;
using System.Runtime.InteropServices;

[assembly:ClassInterface(ClassInterfaceType.AutoDual)]

namespace P2IL2
{
public class CORGlue
{
	public Type m_OpCodesType;
	public CORGlue()
	{
		m_OpCodesType = Type.GetType("System.Reflection.Emit.OpCodes");
	}

	// For some reason the later .NET builds (eg, 2204.20)
	// it seems impossible to pass "Type []" via the com interop
	// layers.  Therefore, they are all declared "Object[]", and
	// use this converter.
	internal static Type[] _ConvertTypeArray(Object[] args) {
		Type[] ret = new Type[args.Length];
		int n=args.Length;
		for (int i=0;i<n;i++)
			ret[i] = (Type)args[i];
		return ret;
	}

	public Object getattr(Object ob, String name) {
		System.Type t = ob.GetType();
		bool isMethod = false;
		try{
			isMethod = t.GetMethod(name) != null;
		} catch (AmbiguousMatchException) {
			isMethod = true; // still a method
		}
		if (isMethod) {
			int hresult;
			unchecked {hresult = (int)0x8002000e;}  // DISP_E_BADPARAMCOUNT
			throw new COMException("Really a method", hresult);
		}
		FieldInfo fi = t.GetField(name);
		if (fi != null) {
			// This object does have a property of that name - use it.
			return fi.GetValue(ob);
		}
		PropertyInfo pi = t.GetProperty(name);
		if (pi != null) {
			// This object does have a property of that name - use it.
			return pi.GetValue(ob, null);
		}
		throw new COMException("No such attribute");
	}
	public void setattr(Object ob, String name, Object value) {
		System.Type t = ob.GetType();
		FieldInfo fi = t.GetField(name);
		if (fi != null) {
			// This object does have a property of that name - use it.
			fi.SetValue(ob, value);
			return;
		}
		PropertyInfo pi = t.GetProperty(name);
		if (pi != null) {
			// This object does have a property of that name - use it.
			pi.SetValue(ob, value, null);
			return;
		}
		throw new COMException("No such attribute");
	}

	public Object call(Object ob, String name, Object[] args) {
		System.Type t = ob.GetType();
		BindingFlags flags = BindingFlags.InvokeMethod | BindingFlags.Default | BindingFlags.Instance | BindingFlags.Public | BindingFlags.FlattenHierarchy | BindingFlags.OptionalParamBinding;
		return t.InvokeMember(name, flags, null, ob, args);
	}

	public Object[] MakeObjectArray(Object [] args) {
		return args;
	}

	public Object GetOpCode(String name)
	{
		FieldInfo fi = m_OpCodesType.GetField(name, BindingFlags.IgnoreCase | BindingFlags.Static | BindingFlags.Public | BindingFlags.GetField);
		if (fi==null)
			throw new Exception(String.Format("The opcode '{0}' can not be located", name));
		return fi.GetValue(null);
	}
	public Object CreateStaticDelegate(Type type, Type target, String name)
	{
		return Delegate.CreateDelegate(type, target, name);
	}
	public Object CreateInstanceDelegate(Type type, Object target, String name)
	{
		return Delegate.CreateDelegate(type, target, name);
	}

	// Something in this implementation doesnt work from Python (probably does now though!)
	public Type CreateDelegateType(ModuleBuilder module, String name)
	{
		Type delegateType = Type.GetType("System.Delegate");
		TypeAttributes attr = (TypeAttributes)(TypeAttributes.Public|TypeAttributes.Sealed);
		TypeBuilder tb = module.DefineType(name, attr, delegateType);
	
		// Define the delegate ctor
		Type[] args = new Type [2];
		args[0] = Type.GetType("System.Object");
		args[1] = Type.GetType("System.Int32*");
		
		MethodAttributes mattr = MethodAttributes.RTSpecialName | MethodAttributes.SpecialName | MethodAttributes.Public;
		MethodBuilder b = tb.DefineMethod(".ctor", mattr, null, args);
		b.SetImplementationFlags(MethodImplAttributes.Runtime);
		
		// Now the "Invoke" function.
		mattr = MethodAttributes.Virtual | MethodAttributes.Public;
		b = tb.DefineMethod("Invoke", mattr, null, null);	
		b.SetImplementationFlags(MethodImplAttributes.Runtime);
	
		return tb.CreateType();
	}

	// *****************************
	// AppDomain delegation, static method, ctor support etc
	// *****************************
	public AssemblyBuilder AppDomain_DefineDynamicAssembly(AppDomain a, AssemblyName n, int access)
	{
		// Broke in 1626
		return a.DefineDynamicAssembly(n, (AssemblyBuilderAccess)access);
	}
	public AssemblyBuilder AppDomain_DefineDynamicAssemblyPath(AppDomain a, AssemblyName n, int access, string path)
	{
		return a.DefineDynamicAssembly(n, (AssemblyBuilderAccess)access, path);
	}
	// *****************************
	// Assembly delegation, static method, ctor support etc
	// *****************************
	public AssemblyName CreateAssemblyName()
	{
		return new AssemblyName();
	}
	public Assembly Assembly_Load( String name )
	{
		return Assembly.Load(name);
	}
	public Assembly Assembly_LoadName( AssemblyName name )
	{
		return Assembly.Load(name);
	}
	public Assembly Assembly_LoadFrom( String name )
	{
		return Assembly.LoadFrom(name);
	}
	public Assembly Assembly_LoadWithPartialName( String name )
	{
		return Assembly.LoadWithPartialName(name);
	}
	// *****************************
	// AssemblyBuilder delegation, static method, ctor support etc
	// *****************************
	public ModuleBuilder AssemblyBuilder_DefineDynamicModule(AssemblyBuilder b, string name, string fname, bool debugInfo)
	{
		return b.DefineDynamicModule(name, fname, debugInfo);
//		return new ModuleHelper(b.DefineDynamicModule(name, fname, debugInfo));
	}

	// *************************
	// ConstructorBuilder delegation, static method, ctor support etc
	// *************************

	// *************************
	// MethodBuilder delegation, static method, ctor support etc
	// *************************
/*    
	public ParameterBuilder MethodBuilder_DefineParameter(MethodBuilder mb, int position, ParameterAttributes attributes, string strParamName)
    {
        return mb.DefineParameter(position, attributes, strParamName);
    }
*/
	public ParameterBuilder MethodBuilder_DefineParameter(Object cb, int position, ParameterAttributes attributes, string strParamName)
    {
        try {
            return ((MethodBuilder)cb).DefineParameter(position, attributes, strParamName);
        } catch (InvalidCastException) {
            return ((ConstructorBuilder)cb).DefineParameter(position, attributes, strParamName);
        }
    }
	// *************************
	// MethodInfo delegation, static method, ctor support etc
	// *************************
	public void ParameterBuilder_SetConstantNull(ParameterBuilder pb)
	{
		pb.SetConstant(null);
	}
	public void ParameterBuilder_SetConstantInt(ParameterBuilder pb, int val)
	{
		pb.SetConstant((Object)val);
	}
	public object ParameterInfo_DefaultValue(object pi)
	{
		return ((ParameterInfo)pi).DefaultValue;
	}

	// *************************
	// PropertyInfo delegation, static method, ctor support etc
	// *************************
	public MethodInfo PropertyInfo_GetSetMethod(Object pi)
	{
		return ((PropertyInfo)pi).GetSetMethod();
	}
	public MethodInfo PropertyInfo_GetGetMethod(Object pi)
	{
		return ((PropertyInfo)pi).GetGetMethod();
	}
	// *************************
	// Thread delegation, static method, ctor support etc
	// *************************
	public AppDomain Thread_GetDomain()
	{
		return System.Threading.Thread.GetDomain();
	}

	// *****************************
	// Type delegation, static method, ctor support etc
	// *****************************
	public Type GetType(String typeName)
	{
		return Type.GetType(typeName);
	}
	// WTF - Python wont let "GetType" through??????
	public Type GetTypeX(String typeName)
	{
		return Type.GetType(typeName);
	}
	public bool Type_Equals( Type t, Type o )
	{
		return t.Equals(o);
	}
	public MethodInfo Type_GetMethod( Type t, String name)
	{
		return t.GetMethod(name);
	}
	public MethodInfo []Type_GetMethods( Type t)
	{
		return t.GetMethods();
	}
	public MethodInfo Type_GetMethodArgs( Type t, String name, Object [] args)
	{
		return t.GetMethod(name, _ConvertTypeArray(args));
	}
	public PropertyInfo Type_GetProperty( Type t, String name)
	{
		return t.GetProperty(name);
	}
	public FieldInfo Type_GetField( Type t, String name)
	{
		return t.GetField(name);
	}
	
	public ConstructorInfo []Type_GetConstructors( Type t)
	{
		return t.GetConstructors();
	}
	public Object Type_GetConstructor(Type typ, Object[] args)
	{
		ConstructorInfo info = typ.GetConstructor(_ConvertTypeArray(args));
		return (Object)info;
	}
	// *****************************
	// TypeBuilder delegation, static method, ctor support etc
	// *****************************
	public ConstructorBuilder TypeBuilder_DefineConstructor(TypeBuilder tb, MethodAttributes attr, CallingConventions cc, Object []pt)
	{
		return tb.DefineConstructor(attr, cc, _ConvertTypeArray(pt));
	}
	public MethodBuilder TypeBuilder_DefineMethod(TypeBuilder tb, String name, MethodAttributes attr, Type returnType, Object [] pt) {
		return tb.DefineMethod(name, attr, returnType, _ConvertTypeArray(pt));
	}
	public FieldBuilder TypeBuilder_DefineField(TypeBuilder tb, String name, Type returnType, FieldAttributes attr) {
		return tb.DefineField(name, returnType, attr);
	}
	public PropertyBuilder TypeBuilder_DefineProperty(TypeBuilder tb, String name, PropertyAttributes attr, Type returnType, Object [] pt) {
		return tb.DefineProperty(name, attr, returnType, _ConvertTypeArray(pt));
	}

	// *****************************
	// StrongKeyNamePair delegation, static method, ctor support etc
	// *****************************
	public StrongNameKeyPair LoadStrongNameKeyPair(String filename)
	{
		System.IO.FileStream fs = new System.IO.FileStream(filename, System.IO.FileMode.Open);
		return new StrongNameKeyPair(fs);
	}
};

public class ILGenWrapper
{
	System.Reflection.Emit.ILGenerator gen;
	public void SetILGenerator(System.Reflection.Emit.ILGenerator g)
	{
		gen = g;
	}
	public Object BeginExceptionBlock2()
	{
		return (Object )gen.BeginExceptionBlock();
	}
	public Object CreateLabel2()
	{
		return (Object )gen.DefineLabel();
	}
	public void MarkLabel2(Object label)
	{
		gen.MarkLabel( (Label)label);
	}
	public void Emit(Object instruction)
	{
		OpCode i = (OpCode)instruction;
		gen.Emit(i);
	}
	public void EmitLocal(Object instruction, LocalBuilder local)
	{
		System.Reflection.Emit.OpCode i = (OpCode)instruction;
		gen.Emit(i, local);
	}
	public void EmitString(Object instruction, String str)
	{
		gen.Emit((OpCode)instruction, str);
	}
	
	public void EmitInt(Object  instruction, int val)
	{
		gen.Emit((OpCode)instruction, val);
	}
	public void EmitDouble(Object  instruction, double val)
	{
		gen.Emit((OpCode)instruction, val);
	}
	public void EmitField(Object  instruction, Object field)
	{
		gen.Emit((OpCode)instruction, (FieldInfo)field);
	}
	public void EmitType(Object  instruction, Type info)
	{
		gen.Emit((OpCode)instruction, info);
	}
	public void EmitMethod(Object  instruction, Object meth)
	{
		gen.Emit((OpCode)instruction, (MethodInfo)meth);
	}
	public void EmitConstructor(Object  instruction, Object ci)
	{
		gen.Emit((OpCode)instruction, (ConstructorInfo)ci);
	}
	public void EmitLabel(Object  instruction, Object label)
	{
		gen.Emit((OpCode)instruction, (Label)label);
	}
	public void EmitWriteLineString(String val)
	{
		gen.EmitWriteLine(val);
	}
	public void EmitWriteLineField(FieldInfo fld)
	{
		gen.EmitWriteLine(fld);
	}
	public void EmitWriteLineLocal(LocalBuilder lcl)
	{
		gen.EmitWriteLine(lcl);
	}
	public Object BeginExceptionBlockX()
	{
		return (Object )gen.BeginExceptionBlock();
	}
	public void EndExceptionBlockX()
	{
		gen.EndExceptionBlock();
	}
	public void BeginCatchBlockX(Object t)
	{
		gen.BeginCatchBlock((Type)t);
	}
	public void BeginCatchBlockNull()
	{
		gen.BeginCatchBlock(null);
	}
	public void BeginExceptFilterBlockX()
	{
		gen.BeginExceptFilterBlock();
	}
	public void BeginFinallyBlockX()
	{
		gen.BeginFinallyBlock();
	}
};

public class ModuleHelper
{
	public ModuleHelper() {
		mod = null;
	}
	public ModuleHelper(ModuleBuilder _mod) {
		mod = _mod;
    }
	ModuleBuilder mod;

	public void SetModule(ModuleBuilder module)
	{
		mod = (ModuleBuilder)module;
	}
	public ModuleBuilder GetModule()
	{
		return mod;
	}
	public Object DefineDocument(String name)
	{
		Guid g = System.Guid.Empty;
		return mod.DefineDocument(name, g, g, g);
	}
	public TypeBuilder DefineType(String name, TypeAttributes attr)
	{
		return mod.DefineType(name, attr);
	}
	public TypeBuilder DefineTypeWithBases(String name, TypeAttributes attr, Type parent, Object [] interfaces)
	{
		return mod.DefineType(name, attr, parent, CORGlue._ConvertTypeArray(interfaces));
	}
};

}
