//#define DEBUG_TRACE
//#define DEBUG_TRACE_PYFUNCTION
//#define DEBUG_TRACE_PYCLASS
//#define DEBUG_TRACE_PYMETHOD

using System;
using System.Reflection;
using System.Collections;
using System.Text;
using System.IO;
using Python.Builtins.exceptions;
using Python.Builtins.types;

[assembly: System.Reflection.AssemblyVersion("1.0.0.0")]
[assembly:AssemblyKeyFileAttribute("../../ManagedPython.key")]
[assembly:AssemblyTitleAttribute("Python")]
[assembly:AssemblyDefaultAliasAttribute("Python.dll")]

namespace Python {

namespace Builtins {

namespace exceptions {

/******************************************************
*
* The Python exception hierarchy
*
*******************************************************/
// Exceptions need re-thinking.  We may be able to reuse standard exception 
// (eg, IndexError = IndexOutOfRangeException, if we can get
// the hierarchies lined up.  We may even be able to implement the
// exceptions module in
public class Exception : System.Exception {
	public Exception( String msg ) : base(msg) {;}
	public Exception( String msg, System.Exception inner ) : base( msg, inner ) {;}
	public Exception() : base() {;}
};

public class KeyError : Exception {
	public KeyError( String msg ) : base( msg ) {;}
	public KeyError( String msg, System.Exception inner ) : base( msg, inner ) {;}
};

public class AttributeError : Exception {
	public AttributeError( String msg ) : base( msg ) {;}
	public AttributeError( String msg, System.Exception inner ) : base( msg, inner ) {;}
};

public class RuntimeError : Exception {
	public RuntimeError() : base() {;}
	public RuntimeError( String msg ) : base( msg ) {;}
	public RuntimeError( String msg, System.Exception inner ) : base( msg, inner ) {;}
};

public class TypeError : Exception {
	public TypeError() : base() {;}
	public TypeError( String msg ) : base( msg ) {;}
	public TypeError( String msg, System.Exception inner ) : base( msg, inner ) {;}
};

public class IndexError : Exception {
	public IndexError() : base() {;}
	public IndexError( String msg ) : base( msg ) {;}
	public IndexError( String msg, System.Exception inner ) : base( msg, inner ) {;}
};

public class ImportError : Exception {
	public ImportError() : base() {;}
	public ImportError( String msg ) : base( msg ) {;}
	public ImportError( String msg, System.Exception inner ) : base( msg, inner ) {;}
};
public class NameError : Exception {
	public NameError() : base() {;}
	public NameError( String msg ) : base( msg ) {;}
	public NameError( String msg, System.Exception inner ) : base( msg, inner ) {;}
};
public class ValueError : Exception {
	public ValueError() : base() {;}
	public ValueError( String msg ) : base( msg ) {;}
	public ValueError( String msg, System.Exception inner ) : base( msg, inner ) {;}
};

}; // End of namespace exceptions

namespace types {
/******************************************************
*
* Python Objects.
*
*******************************************************/
using Python.Builtins.exceptions;

public class PyDict {
	public
	PyDict() {
		dict = new Hashtable();
	}
	public Hashtable
	dict;
}

public class PyList {
	public
	PyList(ArrayList l) {
		list = l;
	}
	public
	PyList() {
		list = new ArrayList();
	}
	public
	PyList(int capacity) {
		list = new ArrayList(capacity);
	}
/***
	// Standard arrays dont provide hash functions that allow useful comparisons
	// in hash tables.
	public override 
	int GetHashCode() {
		long x, y;
		int len = list.Count;
		x = 0x345678L;
		while (--len >= 0) {
			y = list[len].GetHashCode();
			x = (1000003*x) ^ y;
		}
		x ^= len;
		if (x==0) x=-1;
		return (int)x;
	}
***/
//	public override 
//	bool Equals(Object other) {
//		return GetHashCode()==other.GetHashCode();
//	}

	public ArrayList
	list;
}

public class PyFunction {
	public PyFunction(Type t, String n)
	{
		type = t;
		name = n;
		mi = null;
	}
	public PyFunction(Delegate d)
	{
		type = null;
		mi = d.Method;
		name = mi.Name;
	}
	public PyFunction(MethodInfo methodInfo)
	{
		type = null;
		mi = methodInfo;
		name = methodInfo.Name;
	}
	public override
	String ToString() {
		return String.Format("PyFunction {0}::{1}()", type.Name, name);
	}
	internal Type type;
	internal String name;
	internal MethodInfo mi;
};

public class PyMethod {
	public
	PyMethod(Type t, Object thisob, String n) {
		type = t;
		ob = thisob;
		name = n;
		mi = null;
	}
	public 
	PyMethod( Delegate d) {
		mi = d.Method;
		type = mi.DeclaringType;
		name = mi.Name;
		ob = d.Target;
	}
	public
	PyMethod(MethodInfo methodInfo, Object thisob) {
		type = methodInfo.DeclaringType;
		mi = methodInfo;
		ob = thisob;
		name = mi.Name;
	}
	internal Object ob;
	internal String name;
	internal Type type;
	internal MethodInfo mi;
};

// Handles a static C method with a "self" first param.
public class PyBuiltinMethod {
	public
	PyBuiltinMethod(PyBuiltinMethodDelegate d, PyObject self, PyObject bound) {
		this.d = d;
		this.self = self;
		this.bound = bound;
	}
	internal PyBuiltinMethodDelegate d;
	internal PyObject self; // If not None, passed as the first "self" param.
	internal PyObject bound; // If not None, stuck at the front of the args.
};

// A Python module is not closely related to a COM+ 20 module
// A Python module is closer to a COM+ "namespace" - however, namespaces
// do not have any runtime representation, so we fudge it.
public class PyNamespace {
	public PyNamespace(String fqn, String rn)
	{
		full_name = fqn;
		name = rn;
		subobjects = new Hashtable();
	}
	internal Hashtable subobjects;
	internal String full_name;
	internal String name;
};

public class PyTuple {
	public 
	PyTuple(PyObject[] ar) {
		a = ar;
	}
	// Standard arrays dont provide hash functions that allow useful comparisons
	// in hash tables.  As tuples are immutable, we can calc the hash once.
	public override 
	int GetHashCode() {
		if (hash!=0)
			return hash;
		long x, y;
		int len = a.Length;
		x = 0x345678L;
		while (--len >= 0) {
			y = a.GetValue(len).GetHashCode();
			x = (1000003*x) ^ y;
		}
		x ^= len;
		if (x==0) x=-1;
		return hash=(int)x;
	}
	public override 
	bool Equals(Object other) {
		return GetHashCode()==other.GetHashCode();
	}
	internal PyObject[] a;
	private int hash = 0;
}

/******************************************************
*
* The Python built-in types
*
*******************************************************/
// We use a value type for the main PyObject.
public struct PyObject : IComparable {
	public Object ob;
	public IPyType typ;
	// The constructor
	public 
	PyObject(Object o, IPyType t) {ob=o;typ=t;}
	public override
	String ToString() {
//		String obstr = ob==null ? "<null>" : ob.ToString();
//		String tstr = typ==null ? "<null>" : typ.ToString();
		return String.Format("<PyObject: {0}, {1}>", ob, typ==null ? "<null type!>" : typ.tp_name());
	}
	// XXX - is this correct?
	public override
	int GetHashCode() {
		return ob==null ? 0 : ob.GetHashCode();
	}
	public override
	bool Equals(Object other) {
		return GetHashCode()==other.GetHashCode();
	}
	public
	int CompareTo(Object other) {
		try {
			return Runtime.PyObject_Compare(this, (PyObject)other);
//			return ((IComparable)ob).CompareTo(((PyObject)other).ob);
		}
		catch (InvalidCastException) {
			String msg = String.Format("Objects '{0}' (type '{1}') and '{2}' are not comparable", ob, typ.tp_name(), other);
			throw new RuntimeError(msg);
		}
	}
}

public interface IPyNumber {
	PyObject nb_add(PyObject self, PyObject other);
	PyObject nb_subtract(PyObject self, PyObject other);
	PyObject nb_multiply(PyObject self, PyObject other);
	PyObject nb_divide(PyObject self, PyObject other);
	PyObject nb_remainder(PyObject self, PyObject other);
	PyObject nb_divmod(PyObject self, PyObject other);
	PyObject nb_power(PyObject self, PyObject v, PyObject w);
	PyObject nb_negative(PyObject self);
	PyObject nb_positive(PyObject self);
	PyObject nb_absolute(PyObject self);
	bool 	nb_nonzero(PyObject self);
	PyObject	nb_invert(PyObject self);
	PyObject	nb_lshift(PyObject self, PyObject n);
	PyObject	nb_rshift(PyObject self, PyObject n);
	PyObject	nb_and(PyObject self, PyObject n);
	PyObject	nb_xor(PyObject self, PyObject n);
	PyObject	nb_or(PyObject self, PyObject n);
	bool         nb_coerce(PyObject self, ref PyObject cvt);
	PyObject	nb_int(PyObject self);
	PyObject	nb_long(PyObject self);
	PyObject	nb_float(PyObject self);
	PyObject	nb_oct(PyObject self);
	PyObject	nb_hex(PyObject self);
};

public interface IPySequence {
	int sq_length(PyObject self);
	PyObject sq_concat(PyObject self, PyObject other);
	PyObject sq_repeat(PyObject self, int n);
	PyObject sq_item(PyObject self, int n);
	PyObject sq_slice(PyObject self, int s, int e);
	void 	sq_ass_item(PyObject self, int n, PyObject v);
	void 	sq_del_item(PyObject self, int n);
	void 	sq_ass_slice(PyObject self, int s, int e, PyObject v);
};

public interface IPyMapping {
	int 	mp_length(PyObject self);
	PyObject	mp_subscript(PyObject self, PyObject ss);
	void	mp_ass_subscript(PyObject self, PyObject ss, PyObject v);
	void	mp_del_subscript(PyObject self, PyObject ss); // Not in the Python C API
};

public interface IPyType {
	String tp_name();
	Type tp_type(); // Added for COM+ - get the underlying type we expect in our .ob
	Object tp_as_type(PyObject self, Type t); // Added for COM+ - get the object as this type (may not be the same as tp_type)
	void 	tp_print(PyObject self, System.IO.TextWriter w, int flags);
	PyObject	tp_getattr(PyObject self, String a);
	void	tp_setattr(PyObject self, String a, PyObject v);
	int	tp_compare(PyObject self, PyObject other);
	PyObject tp_repr(PyObject self);
	IPyNumber tp_as_number();
	IPySequence tp_as_sequence();
	IPyMapping tp_as_mapping();
	PyObject tp_call(PyObject self, PyObject [] args, PyObject kw);
    	PyObject tp_str(PyObject self);
	PyObject tp_getattro(PyObject self, PyObject a);
	void	tp_setattro(PyObject self, PyObject a, PyObject b);
	// Buffer procs slot??
	String tp_doc(PyObject self);
}; // end of interface 'IPyType'

public delegate PyObject PyBuiltinMethodDelegate(PyObject self, PyObject[]args, PyObject kw);

// It is not necessary to derive from PyType, but this object provides default
// implementations for the interfaces.
public abstract class PyType : IPyType {
	public PyType()
	{
		methods = new Hashtable();
		_load_methods();
	}

	protected virtual void _load_methods()
	{
	}

	protected void _load_method(String py_name, PyBuiltinMethodDelegate func)
	{
		methods[py_name] = func;
	}

	protected 
	Hashtable methods;
	// IPyType methods
	public virtual 
	String tp_name() {
		throw new NotSupportedException();
	}

	public abstract
	Type tp_type();

	public virtual 
	Object tp_as_type(PyObject self, Type t) {
		if (t==tp_type() || t==typeof(Object)) {
			return self.ob;
		}
		throw new NotSupportedException();
	}

	public virtual 
	void tp_print(PyObject self, System.IO.TextWriter s, int flags) {
		throw new NotSupportedException();
	}

	public virtual 
	PyObject tp_getattr(PyObject self, System.String name) {
		PyBuiltinMethodDelegate d = (PyBuiltinMethodDelegate)methods[name];
		if (d != null)
			return new PyObject(new PyBuiltinMethod(d, self, Runtime.Py_None), Runtime.PyBuiltinMethod_Type);
		if (name=="__object__")
			return new PyObject( self.ob, Runtime.PyInstance_Type);
		throw new AttributeError("'" + tp_name() + "' object has no attribute '" + name + "'");
	}

	public virtual 
	void	tp_setattr(PyObject self, String attr, PyObject val) {
		throw new NotSupportedException();
	}

	public virtual 
	int	tp_compare(PyObject self, PyObject other) {
		IComparable ic;
		try {
			ic = (IComparable)(self.ob);
		}
		catch (InvalidCastException)	{
			throw new NotSupportedException();
		}
		return ic.CompareTo(other.ob);
	}

	public virtual
	PyObject tp_repr(PyObject self) {
		throw new NotSupportedException();
	}

	public virtual 
	IPyNumber tp_as_number() {
		return null;
	}

	public virtual 
	IPySequence tp_as_sequence() {
		return null;
	}

	public virtual 
	IPyMapping tp_as_mapping() {
		return null;
	}

	public virtual 
	PyObject tp_str(PyObject self) {
		throw new NotSupportedException();
	}
	public virtual 
	PyObject tp_call(PyObject self, PyObject [] args, PyObject kw) {
		throw new NotSupportedException();
	}
	public virtual 
	PyObject tp_getattro(PyObject self, PyObject attr) {
		throw new NotSupportedException();
	}
	public virtual 
	void tp_setattro(PyObject self, PyObject attr, PyObject val) {
		throw new NotSupportedException();
	}
	// Buffer procs slot??
	public virtual 
	String tp_doc(PyObject self) {
		throw new NotSupportedException();
	}
};

public class PyNothingType : PyType
{
	public override 
	String tp_name() {
		return "None";
	}

	public override
	Object tp_as_type(PyObject self, Type t) {
		if (t.IsValueType)
			throw new TypeError( String.Format("None can not be used as a value type ('{0}')", t.FullName) );
		return null;			
	}

	public override 
	Type tp_type() {
		return typeof(Object);
	}

	public override 
	PyObject tp_repr(PyObject self) {
		return Converters.PyString_FromString("None");
	}

	public override
	int tp_compare(PyObject self, PyObject other) {
		throw new NotSupportedException();
	}
};

public class PyTypeType : PyType
{
	public override 
	String tp_name() {
		return "type";
	}
	public override 
	Type tp_type() {
		return typeof(IPyType);
	}
	public override 
	PyObject tp_getattr(PyObject self, String attr) {
		IPyType t = (IPyType)(self.ob);
		if (attr=="__name__")
			return Converters.PyString_FromString(t.tp_name());
		// XXX - need to add __doc__ and __members__
		return base.tp_getattr(self, attr);
	}
	public override 
	PyObject tp_repr(PyObject self) {
		IPyType t = (IPyType)(self.ob);
		return Converters.PyString_FromString(String.Format("<type '{0}'>", t.tp_name()));
	}
};

public class PyStringType : PyType, IPySequence {
	protected override
	void _load_methods()
	{
		_load_method("join", new PyBuiltinMethodDelegate(PyString_Join));
		_load_method("split", new PyBuiltinMethodDelegate(PyString_Split));
		_load_method("lower", new PyBuiltinMethodDelegate(PyString_Lower));
		_load_method("upper", new PyBuiltinMethodDelegate(PyString_Upper));
		// lower
		//upper
		//islower
		//isupper
		//isspace
		//isdigit
		//istitle
		//capitalize
		//count
		//endswith
		//find
		//index
		//lstrip
		// replace
		// rfind
		// rindex
		// rstrip
		// startswith
		// strip
		// swapcase
		// translate
		// title
		// ljust
		// rjust
		// center
		// expandtabs
		// splitlines
		
		_load_method("ord", new PyBuiltinMethodDelegate(PyString_Ord)); // easiest way to implement 'ord' builtin :-)
	}
	public static
	PyObject PyString_Split(PyObject self, PyObject[] args, PyObject kw)
	{
		String s = (String)(self.ob);
		if (kw.ob != null) throw new TypeError("Keyword args not supported");
		String[] a;
		if (args.Length==0)
			a = s.Split(null);
		else if (args.Length==1) {
			char[] sep =  Converters.PyString_AsString(args[0]).ToCharArray();
			a = s.Split(sep);
		} else if (args.Length==2) {
			char[] sep =  Converters.PyString_AsString(args[0]).ToCharArray();
			a = s.Split(sep, Converters.PyInt_AsInt32(args[1]));
		} else 
			throw new TypeError("too many args");
		return Converters.PyList_FromCollection(a);
	}
	public static
	PyObject PyString_Join(PyObject self, PyObject[] args, PyObject kw)
	{
		String s = (String)(self.ob);
		if (kw.ob != null) throw new TypeError("Keyword args not supported");
		if (args.Length!=1) throw new TypeError("Expected 1 arg");
		IList l = Converters.PyList_AsList(args[0]);
		String[] items = new String[l.Count];
		for (int i=0;i<l.Count;i++) {
			items[i] = Converters.PyString_AsString((PyObject)(l[i]));
		}
		return Converters.PyString_FromString(String.Join(s, items));
	}
	public static
	PyObject PyString_Lower(PyObject self, PyObject[] args, PyObject kw)
	{
		String s = (String)(self.ob);
		if (kw.ob != null) throw new TypeError("Keyword args not supported");
		if (args.Length!=0) throw new TypeError("Expected no args");
		return Converters.PyString_FromString(s.ToLower());
	}
	public static
	PyObject PyString_Upper(PyObject self, PyObject[] args, PyObject kw)
	{
		String s = (String)(self.ob);
		if (kw.ob != null) throw new TypeError("Keyword args not supported");
		if (args.Length!=0) throw new TypeError("Expected no args");
		return Converters.PyString_FromString(s.ToUpper());
	}
	public static
	PyObject PyString_Ord(PyObject self, PyObject[] args, PyObject kw)
	{
		String s = (String)(self.ob);
		if (kw.ob != null) throw new TypeError("Keyword args not supported");
		int index;
		if (args.Length==0) 
			index = 0;
		else if (args.Length==1) 
			index = Converters.PyInt_AsInt32(args[0]);
		else
			throw new TypeError("Expected 0 or 1 args");
		return Converters.PyInt_FromInt(s[index]);
	}

	// IPyType methods
	public override 
	String tp_name() {
		return "string";
	}
	public override 
	Type tp_type() {
		return typeof(String);
	}
	public override 
	void tp_print(PyObject self, System.IO.TextWriter s, int flags) {
		String ss = (String )(self.ob);
		if ((flags & Runtime.Py_PRINT_RAW) != 0) {
			s.Write(ss);
			return;
		}
		// Figure out which quote to use later!
		char quote = '"';
		s.Write(quote);
		for (int i = 0; i < ss.Length; i++) {
			char c = ss[i];
			if (c == quote || c == '\\') {
				s.Write("\\");
				s.Write(c);
			} else if (c < ' ' || c >= 0177) {
				// XXX - rethink in terms of Unicode!
				s.Write("\\");
				s.Write(System.Convert.ToString(c & 0377, 8));
			} else
				s.Write(c);
		}
		s.Write(quote);
	}
	public override 
	PyObject tp_str(PyObject self) {
		return self;
	}
	public override 
	PyObject tp_repr(PyObject self) {
		String ss = (String )(self.ob);
		// XXX - not the correct semantics - just copied from tp_print.
		StringBuilder s = new StringBuilder();
		// Figure out which quote to use later!
		char quote = '\'';
		s.Append(quote);
		for (int i = 0; i < ss.Length; i++) {
			char c = ss[i];
			if (c == quote || c == '\\') {
				s.Append("\\");
				s.Append(c);
			} else if (c < ' ' || c >= 0177) {
				// XXX - rethink in terms of Unicode!
				s.Append("\\");
				s.Append(System.Convert.ToString(c & 0377, 8));
			} else
				s.Append(c);
		}
		s.Append(quote);
		return Converters.PyString_FromString(s.ToString());
	}
	public override 
	IPySequence tp_as_sequence() {
		return this;
	}
	// IPySequence methods
	public 
	int sq_length(PyObject ob) {
		return ((String)(ob.ob)).Length;
	}
	public 
	PyObject sq_concat(PyObject self, PyObject other) {
		String snew = (String )(self.ob) + (String )(other.ob);
		return Converters.PyString_FromString(snew);
	}
	public 
	PyObject sq_repeat(PyObject self, int cnt) {
		StringBuilder sb = new StringBuilder();
		String s = (String )(self.ob);
		for (int i=0;i<cnt;i++)
			sb.Append(s);
		return Converters.PyString_FromString(sb.ToString());
	}
	public 
	PyObject sq_item(PyObject s, int index) {
		String self = (String )(s.ob);
		if (index<0 || index >= self.Length)
			throw new IndexError("string index out of range");
		return Converters.PyString_FromString(self.Substring(index, 1));
	}
	public 
	PyObject sq_slice(PyObject s, int start, int end) {
		String self = (String )(s.ob);
		StringBuilder sb = new StringBuilder();
		end = Math.Min(end, self.Length);
		for (int i=start;i<end;i++)
			sb.Append(self[i]);
		return Converters.PyString_FromString(sb.ToString());
	}
	public 
	void sq_ass_item(PyObject self, int i, PyObject ob) {
		throw new NotSupportedException();
	}
	public 
	void sq_del_item(PyObject self, int i) {
		throw new NotSupportedException();
	}
	public 
	void sq_ass_slice(PyObject self, int s, int e, PyObject v) {
		throw new NotSupportedException();
	}
	// Formatting support
	public static PyObject
	Format(PyObject self, PyObject args) {
		PyObject arg_repr = Runtime.PyObject_Repr(args);
		return Converters.PyString_FromString(String.Format("{0}(args={1}", self.ob, arg_repr.ob));
	}
};

public class PyIntType : PyType, IPyNumber {
	// IPyType methods
	public override 
	String tp_name() {
		return "int";
	}
	public override 
	Type tp_type() {
		return typeof(Int32);
	}
	public override
	Object tp_as_type(PyObject self, Type t) {
		if (t==tp_type() || t==typeof(Object)) {
			return self.ob;
		}
		if (t==typeof(Int64))
			return (Int64)(Int32)self.ob;
		throw new NotSupportedException();
	}
	public override 
	void tp_print(PyObject self, System.IO.TextWriter s, int flags) {
		s.Write(self.ob.ToString());
	}
	public override 
	PyObject tp_repr(PyObject self) {
		return Converters.PyString_FromString(self.ob.ToString());
	}
	public override 
	IPyNumber tp_as_number() {
		return this;
	}
	// IPyNumber methods.
	public 
	PyObject nb_add( PyObject self, PyObject other ) {
		return Converters.PyInt_FromInt(((int)self.ob) + ((int)other.ob));
	}
	public 
	PyObject nb_subtract(PyObject self, PyObject other) {
		return Converters.PyInt_FromInt(((int)self.ob) - ((int)other.ob));
	}
	public 
	PyObject nb_multiply(PyObject self, PyObject other) {
		return Converters.PyInt_FromInt(((int)self.ob) * ((int)other.ob));
	}
	public 
	PyObject nb_divide(PyObject self, PyObject other) {
		int d=0, m=0;
		divmod(((int)self.ob), ((int)other.ob), ref d, ref m);
		return Converters.PyInt_FromInt(d);
	}
	public 
	PyObject nb_remainder(PyObject self, PyObject other) {
		int d=0, m=0;
		divmod(((int)self.ob), ((int)other.ob), ref d, ref m);
		return Converters.PyInt_FromInt(m);
	}
	public 
	PyObject nb_divmod(PyObject self, PyObject other) {
		int d=0, m=0;
		divmod(((int)self.ob), ((int)other.ob), ref d, ref m);
		return Converters.PyInt_FromInt(d);
	}
	public 
	PyObject nb_power(PyObject self, PyObject other, PyObject v) {
		return Converters.PyInt_FromInt( (int)Math.Pow((double)(int)self.ob, (double)(int)other.ob));
	}
	public 
	PyObject nb_negative(PyObject self) {
		return Converters.PyInt_FromInt(-((int)self.ob));
	}
	public 
	PyObject nb_positive(PyObject self) {
		return self;
	}
	public 
	PyObject nb_absolute(PyObject self) {
		int val = (int)self.ob;
		if (val<0) val = -val;
		return Converters.PyInt_FromInt(val);
	}
	public 
	bool nb_nonzero(PyObject self) {
		return (int)(self.ob) != 0;
	}
	public 
	PyObject nb_invert(PyObject self) 
	{
		return Converters.PyInt_FromInt(~((int)self.ob) );
	}
	public 
	PyObject nb_lshift(PyObject self, PyObject v) {
		throw new NotSupportedException();
	}
	public 
	PyObject	nb_rshift(PyObject self, PyObject v) {
		throw new NotSupportedException();
	}
	public 
	PyObject nb_and(PyObject self, PyObject v) {
		return Converters.PyInt_FromInt(((int)self.ob) & ((int)v.ob));
	}
	public 
	PyObject nb_xor(PyObject self, PyObject v) {
		return Converters.PyInt_FromInt( ((int)self.ob) ^ ((int)v.ob));
	}
	public 
	PyObject nb_or(PyObject self, PyObject v) {
		return Converters.PyInt_FromInt( ((int)self.ob) | ((int)v.ob));
	}
	public 
	bool nb_coerce(PyObject self, ref PyObject cvt) {
		throw new NotSupportedException();
	}
	public 
	PyObject nb_int(PyObject self) {
		return self;
	}
	public 
	PyObject nb_long(PyObject self) {
		throw new NotSupportedException();
	}
	public 
	PyObject nb_float(PyObject self) {
		throw new NotSupportedException();
	}
	public 
	PyObject nb_oct(PyObject self) {
		return Converters.PyString_FromString(Convert.ToString(((int)self.ob), 8));
	}
	public 
	PyObject nb_hex(PyObject self) {
		return Converters.PyString_FromString(Convert.ToString(((int)self.ob), 16));
	}
	// Helper to maintain Python semantics for div and mod ops.
	protected 
	void divmod(int xi, int yi, ref int pxdivy, ref int pxmody)
	{
		int xdivy, xmody;
		if (yi < 0) {
			if (xi < 0) {
				xdivy = -xi / -yi;
			}
			else
				xdivy = - (xi / -yi);
		}
		else {
			if (xi < 0)
				xdivy = - (-xi / yi);
			else
				xdivy = xi / yi;
		}
		xmody = xi - xdivy*yi;
		if ((xmody < 0 && yi > 0) || (xmody > 0 && yi < 0)) {
			xmody += yi;
			xdivy -= 1;
		}
		pxdivy = xdivy;
		pxmody = xmody;
	}
}; // End of PyIntType

public class PyFloatType : PyType, IPyNumber {
	// IPyType methods
	public override String
	tp_name() {
		return "float";
	}

	public override Type
	tp_type() {
		return typeof(Double);
	}

	public override Object
	tp_as_type(PyObject self, Type t) {
		if (t==tp_type() || t==typeof(Object)) {
			return self.ob;
		}
		if (t==typeof(float))
			return (float)(double)self.ob;
		throw new NotSupportedException();
	}

	private String
	AsStringEx(PyObject v, int precision)
	{
		/* Subroutine for float_repr and float_print.
		   We want float numbers to be recognizable as such,
		   i.e., they should contain a decimal point or an exponent.
		   However, %g may print the number as an integer;
		   in such cases, we append ".0" to the string. */
		// XXX - Cant find the docs for any builtin precision support!?!?!?!
		String ret = Convert.ToString((double)v.ob);
		int ret_len = ret.Length;
		int offset = 0;
		if (ret[0]=='-')
			offset++;
		for (; offset < ret_len; offset++) {
			/* Any non-digit means it's not an integer;
			   this takes care of NAN and INF as well. */
			if (!Char.IsDigit(ret[offset]))
				break;
		}
		if (offset==ret_len) {
			ret = ret + ".0";
		}
		return ret;
	}
	
	public override void
	tp_print(PyObject self, System.IO.TextWriter s, int flags) {
		int prec = (flags&Runtime.Py_PRINT_RAW) != 0 ? 12 : 17;
		s.Write(AsStringEx(self, prec));
	}

	public override PyObject
	tp_repr(PyObject self) {
		return Converters.PyString_FromString(AsStringEx(self, 17));
	}

	public override IPyNumber
	tp_as_number() {
		return this;
	}

	// IPyNumber methods.
	public PyObject
	nb_add( PyObject self, PyObject other ) {
		return Converters.PyFloat_FromDouble(((double)self.ob) + ((double)other.ob));
	}

	public PyObject
	nb_subtract(PyObject self, PyObject other) {
		return Converters.PyFloat_FromDouble(((double)self.ob) - ((double)other.ob));
	}

	public PyObject
	nb_multiply(PyObject self, PyObject other) {
		return Converters.PyFloat_FromDouble(((double)self.ob) * ((double)other.ob));
	}

	public PyObject
	nb_divide(PyObject self, PyObject other) {
		return Converters.PyFloat_FromDouble((double)self.ob / (double)other.ob);
	}

	public PyObject 
	nb_remainder(PyObject self, PyObject other) {
		float v = (float)self.ob;
		float w = (float)other.ob;
		if (w==0.0)
			throw new DivideByZeroException();
		double mod = Math.IEEERemainder(v, w);
		/* note: checking mod*wx < 0 is incorrect -- underflows to
		   0 if wx < sqrt(smallest nonzero double) */
		if (mod != 0.0 && ((w < 0) != (mod < 0))) {
			mod += w;
		}
		return Converters.PyFloat_FromDouble(mod);
	}

	public PyObject
	nb_divmod(PyObject self, PyObject other) {
		throw new NotSupportedException();
/*
		float v = (float)self.ob;
		float w = (float)other.ob;
		double mod = Math.IEEERemainder(v, w);
*/
		/* fmod is typically exact, so vx-mod is *mathemtically* an
		   exact multiple of wx.  But this is fp arithmetic, and fp
		   vx - mod is an approximation; the result is that div may
		   not be an exact integral value after the division, although
		   it will always be very close to one.
		*/
/*
		double div = (w - mod) / w;
		// note: checking mod*wx < 0 is incorrect -- underflows to
		// 0 if wx < sqrt(smallest nonzero double) 
		if (mod && ((w < 0) != (mod < 0))) {
			mod += x;
			div -= 1.0;
		}
		// snap quotient to nearest integral value
		double floordiv = floor(div);
		if (div - floordiv > 0.5)
			floordiv += 1.0;
*/
	}

	public PyObject
	nb_power(PyObject self, PyObject other, PyObject v) {
		throw new NotSupportedException();
	}

	public PyObject
	nb_negative(PyObject self) {
		return Converters.PyFloat_FromDouble(-((double)self.ob));
	}

	public PyObject
	nb_positive(PyObject self) {
		return self;
	}

	public PyObject
	nb_absolute(PyObject self) {
		double val = (double)self.ob;
		if (val<0) val = -val;
		return Converters.PyFloat_FromDouble(val);
	}

	public bool
	nb_nonzero(PyObject self) {
		return (double)(self.ob) != 0.0;
	}

	public PyObject
	nb_invert(PyObject self) 
	{
		throw new NotSupportedException();
	}

	public PyObject
	nb_lshift(PyObject self, PyObject v) {
		throw new NotSupportedException();
	}

	public PyObject
	nb_rshift(PyObject self, PyObject v) {
		throw new NotSupportedException();
	}

	public PyObject
	nb_and(PyObject self, PyObject v) {
		throw new NotSupportedException();
	}

	public PyObject
	nb_xor(PyObject self, PyObject v) {
		throw new NotSupportedException();
	}

	public PyObject
	nb_or(PyObject self, PyObject v) {
		throw new NotSupportedException();
	}

	public bool
	nb_coerce(PyObject self, ref PyObject cvt) {
		if (Converters.PyInt_Check(cvt)) {
			cvt = Converters.PyFloat_FromDouble((double)Converters.PyInt_AsInt32(cvt));
			return true;
		}
		// Can't do it.
		return false;
	}

	public PyObject
	nb_int(PyObject self) {
		return Converters.PyInt_FromInt((int)(double)self.ob);
	}

	public PyObject
	nb_long(PyObject self) {
		throw new NotSupportedException();
	}

	public PyObject
	nb_float(PyObject self) {
		return self;
	}

	public PyObject
	nb_oct(PyObject self) {
		throw new NotSupportedException();
	}

	public PyObject
	nb_hex(PyObject self) {
		throw new NotSupportedException();
	}
}; // end of PyFloatType

public class PyDictType : PyType, IPyMapping {
	protected override void _load_methods()
	{
		_load_method("has_key", new PyBuiltinMethodDelegate(PyDict_HasKey));
		_load_method("keys", new PyBuiltinMethodDelegate(PyDict_Keys));
		_load_method("items", new PyBuiltinMethodDelegate(PyDict_Items));
		_load_method("values", new PyBuiltinMethodDelegate(PyDict_Values));
/**
		_load_method("update", "PyDict_Update");
		_load_method("clear", "PyDict_Clear");
		_load_method("copy", "PyDict_Copy");
		_load_method("get", "PyDict_Get");
**/
	}
	public static
	PyObject PyDict_HasKey(PyObject self, PyObject[] args, PyObject kw) {
		Hashtable dict = ((PyDict)self.ob).dict;
		if (kw.ob != null) throw new TypeError("Keyword args not supported");
		if (args.Length!=1) throw new TypeError("Expected 1 arg");
		Object value = ((PyObject)args[0]).ob;
		return Converters.PyInt_FromBool(dict.ContainsKey(value));
	}
	public static
	PyObject PyDict_Keys(PyObject self, PyObject[] args, PyObject kw) {
		Hashtable dict = ((PyDict)self.ob).dict;
		if (kw.ob != null) throw new TypeError("Keyword args not supported");
		if (args.Length!=0) throw new TypeError("Expected 0 args");
		return Converters.PyList_FromCollection(dict.Keys);
	}
	public static
	PyObject PyDict_Items(PyObject self, PyObject[] args, PyObject kw) {
		Hashtable dict = ((PyDict)self.ob).dict;
		if (kw.ob != null) throw new TypeError("Keyword args not supported");
		if (args.Length!=0) throw new TypeError("Expected 0 args");

		IDictionaryEnumerator en = dict.GetEnumerator();
		ArrayList l = new ArrayList(dict.Count);
		while (en.MoveNext()) {
			PyObject[] items = new PyObject[2];
			items[0] = (PyObject)(en.Key);
			items[1] = (PyObject)(en.Value);
			PyObject tuple = Converters.PyTuple_FromArray(items);
			l.Add(tuple);
		}
		return Converters.PyList_FromList(l);
	}
	public static
	PyObject PyDict_Values(PyObject self, PyObject[] args, PyObject kw) {
		Hashtable dict = ((PyDict)self.ob).dict;
		if (kw.ob != null) throw new TypeError("Keyword args not supported");
		if (args.Length!=0) throw new TypeError("Expected 0 args");
		return Converters.PyList_FromCollection(dict.Values);
	}
	// IPyType methods
	public override 
	String tp_name() {
		return "dictionary";
	}
	public override 
	Type tp_type() {
		return typeof(PyDict);
	}
	public override 
	void tp_print(PyObject self, System.IO.TextWriter s, int flags) {
		Hashtable ht = ((PyDict)self.ob).dict;

		bool ok = Runtime.Py_ReprEnter(self);
		if (!ok) {
			s.Write("{...}");
			return;
		}
		try {
			s.Write("{");
			int any = 0;
			IEnumerator kenum = ht.Keys.GetEnumerator();
			while (kenum.MoveNext()) {
				if (any++ > 0)
					s.Write(", ");
				PyObject key = (PyObject)kenum.Current;
				Runtime.PyObject_Print(key, s, 0);
				s.Write(": ");
				Runtime.PyObject_Print((PyObject)ht[key], s, 0);
			}
			s.Write("}");
		}
		finally {
			Runtime.Py_ReprLeave(self);
		}
	}
	public override 
	PyObject tp_repr(PyObject self) {
		Hashtable ht = ((PyDict)self.ob).dict;

		bool ok = Runtime.Py_ReprEnter(self);
		if (!ok) {
			return Converters.PyString_FromString("{...}");
		}
		try {
			StringBuilder sb = new StringBuilder("{");
			int any = 0;
			IEnumerator kenum = ht.Keys.GetEnumerator();
			while (kenum.MoveNext()) {
				if (any++ > 0)
					sb.Append(", ");
				PyObject key = (PyObject)kenum.Current;
				PyObject obs = Runtime.PyObject_Repr(key);
				sb.Append(Converters.PyString_AsString(obs));
				sb.Append(": ");
				obs = Runtime.PyObject_Repr((PyObject)ht[key]);
				sb.Append(Converters.PyString_AsString(obs));
			}
			sb.Append("}");
			return Converters.PyString_FromString(sb.ToString());
		}
		finally {
			Runtime.Py_ReprLeave(self);
		}
	}
	public override IPyMapping tp_as_mapping()
	{
		return this;
	}
	// IPyMapping methods
	public int 	mp_length(PyObject self)
	{
		return ((PyDict)self.ob).dict.Count;
	}
	public PyObject	mp_subscript(PyObject self, PyObject key)
	{
//		if (key==null)
//			key = Null.Value;
		Object check = ((PyDict)self.ob).dict[key];
		if (check==null)
			// XXX - should probably use tp_str() for the exception.
			throw new KeyError(Converters.PyString_AsString(Runtime.PyObject_Str(key)));
		return (PyObject)check;
//		if (ret==Null.Value)
//			ret = null;
	}
	public void	mp_ass_subscript(PyObject self, PyObject key, PyObject val) {
		Hashtable dict = ((PyDict)self.ob).dict;
		dict[key] = val;
	}
	public void mp_del_subscript(PyObject self, PyObject key) {
		Hashtable dict = ((PyDict)self.ob).dict;
		dict.Remove(key);
	}
};

public class PyTupleType : PyType, IPySequence {
	public override
	String tp_name() {
		return "tuple";
	}
	public override 
	Type tp_type() {
		return typeof(PyTuple);
	}
	public override
	void tp_print(PyObject self, System.IO.TextWriter s, int flags) {
		PyObject[]  a = ((PyTuple)(self.ob)).a;
		s.Write("(");
		int size = a.Length;
		for (int i = 0; i < size; i++) {
			if (i > 0)
				s.Write(", ");
			PyObject element = (PyObject)(a[i]);
			Runtime.PyObject_Print(element, s, 0) ;
		}
		if (size == 1)
			s.Write(",");
		s.Write(")");
	}
	public override
	PyObject tp_repr(PyObject self) {
		PyObject[]  a = ((PyTuple)(self.ob)).a;
		StringBuilder s = new StringBuilder("(");
	
		String comma = ", ";
		int size = a.Length;
		for (int i = 0; i < size; i++) {
			if (i > 0)
				s.Append(comma);
			PyObject element = (PyObject)(a.GetValue(i));
			s.Append( Runtime.PyObject_Repr(element) );
		}
		if (size == 1)
			s.Append(",");
		s.Append(")");
		return Converters.PyString_FromString(s.ToString());
	}
	public override 
	int tp_compare(PyObject self, PyObject other) {
		PyObject[] aself = ((PyTuple)(self.ob)).a;
		PyObject[] aother = ((PyTuple)(other.ob)).a;
		int num_self = aself.Length;
		int num_other = aother.Length;
		int len = (num_self < num_other) ? num_self : num_other;
		for (int i = 0; i < len; i++) {
			int cmp = Runtime.PyObject_Compare(aself[i], aother[i]);
			if (cmp != 0)
				return cmp;
		}
		return num_self - num_other;
	}
	public override 
	IPySequence tp_as_sequence() {
		return this;
	}
	// IPySequence methods
	public int 
	sq_length(PyObject self) {
		return ((PyTuple)(self.ob)).a.Length;
	}

	public PyObject 
	sq_concat(PyObject self, PyObject other) {
		PyObject[] aself = ((PyTuple)(self.ob)).a;
		PyObject[] aother = ((PyTuple)(other.ob)).a;
		int num_self = aself.Length;
		int num_other = aother.Length;
		PyObject[] anew = new PyObject[num_self + num_other];
		Array.Copy(aself, anew, num_self);
		Array.Copy(aother, 0, anew, num_self, num_other);
		return Converters.PyTuple_FromArray(anew);
	}

	public PyObject
	sq_repeat(PyObject self, int num) {
		Array a = ((PyTuple)(self.ob)).a;
		if (num<0) num=0;
		if (a.Length == 0 || num == 1)
			/* Since tuples are immutable, we can return a shared
			   copy in this case */
			return self;
		int num_self = a.Length;
		PyObject[] ret = new PyObject[num * num_self];
		int index = 0;
		for (int i=0;i<num;i++) {
			Array.Copy(a, 0, ret, index, num_self);
			index += num_self;
		}
		return Converters.PyTuple_FromArray(ret);
	}

	public PyObject
	sq_item(PyObject self, int index) {
		PyObject[] a = ((PyTuple)(self.ob)).a;
		if (index<0 || index >= a.Length)
			throw new IndexError("tuple index out of range");
		return a[index];
	}

	public PyObject
	sq_slice(PyObject self, int ilow, int ihigh) {
		Array a = ((PyTuple)(self.ob)).a;
		int size = a.Length;
		if (ilow<0) ilow = 0;
		if (ihigh>size) ihigh = size;
		if (ihigh<ilow) ihigh = ilow;
		if (ilow == 0 && ihigh == size)
			/* XXX can only do this if tuples are immutable! */
			return self;
		PyObject[] ret= new PyObject[ihigh-ilow];
		Array.Copy(a, ilow, ret, 0, ihigh-ilow);
		return Converters.PyTuple_FromArray(ret);
	}

	public void	sq_ass_item(PyObject self, int n, PyObject v) {throw new NotSupportedException();}
	public void 	sq_del_item(PyObject self, int n) {throw new NotSupportedException();}
	public void 	sq_ass_slice(PyObject self, int s, int e, PyObject v) {throw new NotSupportedException();}
}

// A type for a COM+ array.  Was the tuple implementation until I discovered that
// the GetHashCode() of an array doesnt walk the children.
public class PyArrayType : PyType, IPySequence {

	// IPyType methods
	public override
	String tp_name() {
		return "com+array";
	}
	public override 
	Type tp_type() {
		return typeof(Array);
	}
	public override Object
	tp_as_type(PyObject self, Type t) {
		if (t.IsArray)
			return self.ob;
		return base.tp_as_type(self, t);
	}
	public override 
	void tp_print(PyObject self, System.IO.TextWriter s, int flags)
	{
		Array a = (Array)(self.ob);
		s.Write("(");
		int size = a.Length;
		for (int i = 0; i < size; i++) {
			if (i > 0)
				s.Write(", ");
			PyObject element = Converters.PyObject_FromObject(a.GetValue(i));
			Runtime.PyObject_Print(element, s, 0) ;
		}
		if (size == 1)
			s.Write(",");
		s.Write(")");
	}
	public override
	PyObject tp_repr(PyObject self)
	{
		Array a = (Array)(self.ob);
		StringBuilder s = new StringBuilder("(");
	
		String comma = ", ";
		int size = a.Length;
		for (int i = 0; i < size; i++) {
			if (i > 0)
				s.Append(comma);
			PyObject element = Converters.PyObject_FromObject(a.GetValue(i));
			s.Append( Runtime.PyObject_Repr(element) );
		}
		if (size == 1)
			s.Append(",");
		s.Append(")");
		return Converters.PyString_FromString(s.ToString());
	}
	public override 
	int tp_compare(PyObject self, PyObject other)
	{
		Array aself = (Array)(self.ob);
		Array aother = (Array)(other.ob);
		int num_self = aself.Length;
		int num_other = aother.Length;
		int len = (num_self < num_other) ? num_self : num_other;
		for (int i = 0; i < len; i++) {
			int cmp = System.Collections.Comparer.Default.Compare(aself.GetValue(i), aother.GetValue(i));
			if (cmp != 0)
				return cmp;
		}
		return num_self - num_other;
	}
	public override IPySequence tp_as_sequence()
	{
		return this;
	}
	// IPySequence methods
	public int sq_length(PyObject self)
	{
		return ((Array)(self.ob)).Length;
	}
	public PyObject sq_concat(PyObject self, PyObject other)
	{
		Array aself = (Array)(self.ob);
		Array aother = (Array)(other.ob);
		int num_self = aself.Length;
		int num_other = aother.Length;
		Object[] anew = new Object[num_self + num_other];
		Array.Copy(aself, anew, num_self);
		Array.Copy(aother, 0, anew, num_self, num_other);
		return Converters.PyArray_FromArray(anew);
	}
	public PyObject sq_repeat(PyObject self, int num) 
	{
		Array a = (Array)(self.ob);
		if (num<0) num=0;
		if (a.Length == 0 || num == 1)
			/* Since tuples are immutable, we can return a shared
			   copy in this case */
			return self;
		int num_self = a.Length;
		Object[] ret = new Object[num * num_self];
		int index = 0;
		for (int i=0;i<num;i++) {
			Array.Copy(a, 0, ret, index, num_self);
			index += num_self;
		}
		return Converters.PyArray_FromArray(ret);
	}
	public PyObject sq_item(PyObject self, int index)
	{
		Array a = (Array)(self.ob);
		if (index<0 || index >= a.Length)
			throw new IndexError("array index out of range");
		return Converters.PyObject_FromObject(a.GetValue(index));
	}
	public PyObject sq_slice(PyObject self, int ilow, int ihigh)
	{
		Array a = (Array)(self.ob);
		int size = a.Length;
		if (ilow<0) ilow = 0;
		if (ihigh>size) ihigh = size;
		if (ihigh<ilow) ihigh = ilow;
		if (ilow == 0 && ihigh == size)
			/* XXX can only do this if tuples are immutable! */
			return self;
		Object[] ret= new Object[ihigh-ilow];
		Array.Copy(a, ilow, ret, 0, ihigh-ilow);
		return Converters.PyArray_FromArray(ret);
	}
	public void 	sq_ass_item(PyObject self, int n, PyObject v) 
	{
		Array a = (Array)(self.ob);
//		if (index<0 || index >= a.Length)
//			throw new IndexError("array index out of range");
		a.SetValue(v.ob, n);
	}
	public void 	sq_del_item(PyObject self, int n) {throw new NotSupportedException();}
	public void 	sq_ass_slice(PyObject self, int s, int e, PyObject v) {throw new NotSupportedException();}
};

public class PyListType : PyType, IPySequence {
	protected override void 
	_load_methods() {
		_load_method("append", new PyBuiltinMethodDelegate(PyList_Append));
		_load_method("insert", new PyBuiltinMethodDelegate(PyList_Insert));
		_load_method("extend", new PyBuiltinMethodDelegate(PyList_Extend));
		_load_method("pop", new PyBuiltinMethodDelegate(PyList_Pop));
		_load_method("remove", new PyBuiltinMethodDelegate(PyList_Remove));
		_load_method("index", new PyBuiltinMethodDelegate(PyList_Index));
		_load_method("count", new PyBuiltinMethodDelegate(PyList_Count));
		_load_method("reverse", new PyBuiltinMethodDelegate(PyList_Reverse));
		_load_method("sort", new PyBuiltinMethodDelegate(PyList_Sort));
	}
	public static PyObject 
	PyList_Append(PyObject self, PyObject[] args, PyObject kw) {
		ArrayList op = ((PyList)self.ob).list;
		if (kw.ob != null) throw new TypeError("Keyword args not supported");
		if (args.Length!=1) throw new TypeError("Expected 1 arg");
		PyObject val = (PyObject)(args[0]);
		op.Add(val);
		return Runtime.Py_None;
	}
	public static PyObject 
	PyList_Insert(PyObject self, PyObject[] args, PyObject kw) {
		ArrayList op = ((PyList)self.ob).list;
		if (kw.ob != null) throw new TypeError("Keyword args not supported");
		if (args.Length!=2) throw new TypeError("Expected 2 args");
		int index = Converters.PyInt_AsInt32(args[0]);
		PyObject val = (PyObject)(args[1]);
		op.Insert(index, val);
		return Runtime.Py_None;
	}
	public static PyObject 
	PyList_Extend(PyObject self, PyObject[] args, PyObject kw) {
		ArrayList op = ((PyList)self.ob).list;
		if (kw.ob != null) throw new TypeError("Keyword args not supported");
		if (args.Length!=1) throw new TypeError("Expected 1 arg");
		ArrayList other = (ArrayList)Converters.PyList_AsList(args[0]);
		op.InsertRange(op.Count, other);
		return Runtime.Py_None;
	}
	public static PyObject 
	PyList_Pop(PyObject self, PyObject[] args, PyObject kw) {
		ArrayList op = ((PyList)self.ob).list;
		if (kw.ob != null) throw new TypeError("Keyword args not supported");
		if (args.Length!=0) throw new TypeError("Expected 0 args");
		// XXX - need thread safety??
		int n = op.Count;
		if (n==0)
			throw new TypeError("pop() from empty list");
		PyObject ret = (PyObject)op[n-1];
		op.RemoveAt(n-1);
		return ret;
	}
	public static PyObject 
	PyList_Remove(PyObject self, PyObject[] args, PyObject kw) {
		ArrayList op = ((PyList)self.ob).list;
		if (kw.ob != null) throw new TypeError("Keyword args not supported");
		if (args.Length!=1) throw new TypeError("Expected 1 arg");
		PyObject oblook = (PyObject)args[0];
		int index = op.IndexOf(oblook);
		if (index==-1)
			throw new ValueError("list.remove(x): x not in list");
//		try {
			op.Remove(oblook);
//		}
//		catch (ArgumentException) {
//			throw new ValueError("list.remove(x): x not in list");
//		}
		return Runtime.Py_None;
	}
	public static PyObject
	PyList_Index(PyObject self, PyObject[] args, PyObject kw) {
		ArrayList op = ((PyList)self.ob).list;
		if (kw.ob != null) throw new TypeError("Keyword args not supported");
		if (args.Length!=1) throw new TypeError("Expected 1 arg");
		int index = op.IndexOf(args[0]);
		if (index==-1)
			throw new ValueError("list.index(x): x not in list");
		return Converters.PyInt_FromInt(index);
	}
	public static PyObject
	PyList_Count(PyObject self, PyObject[] args, PyObject kw) {
		ArrayList op = ((PyList)self.ob).list;
		if (kw.ob != null) throw new TypeError("Keyword args not supported");
		if (args.Length!=1) throw new TypeError("Expected 1 arg");
		Object val = args[0];
		int index = -1;
		int cnt = 0;
		while ( (index=op.IndexOf(val, index+1)) !=-1)
			cnt++;
		return Converters.PyInt_FromInt(cnt);
	}
	public static PyObject
	PyList_Reverse(PyObject self, PyObject[] args, PyObject kw) {
		ArrayList op = ((PyList)self.ob).list;
		if (kw.ob != null) throw new TypeError("Keyword args not supported");
		if (args.Length!=0) throw new TypeError("Expected 0 args");
		op.Reverse();
		return Runtime.Py_None;
	}
	public static PyObject
	PyList_Sort(PyObject self, PyObject[] args, PyObject kw) {
		ArrayList op = ((PyList)self.ob).list;
		if (kw.ob != null) throw new TypeError("Keyword args not supported");
		if (args.Length!=0) throw new TypeError("Expected 0 args");
		op.Sort();
		return Runtime.Py_None;
	}
	// IPyType methods
	public override String 
	tp_name() {
		return "list";
	}
	public override 
	Type tp_type() {
		return typeof(PyList);
	}
	public override void
	tp_print(PyObject self, System.IO.TextWriter s, int flags) {
		ArrayList op = ((PyList)self.ob).list;
		bool ok = Runtime.Py_ReprEnter(self);
		if (!ok) {
			s.Write("[...]");
			return;
		}
		try {
			s.Write("[");
			int num_self = op.Count;
			for (int i = 0; i < num_self; i++) {
				if (i > 0)
					s.Write(", ");
				Runtime.PyObject_Print((PyObject)(op[i]), s, 0);
			}
			s.Write("]");
		}
		finally {
			Runtime.Py_ReprLeave(self);
		}
	}
	public override PyObject 
	tp_repr(PyObject self) {
		ArrayList op = ((PyList)self.ob).list;
		bool ok = Runtime.Py_ReprEnter(self);
		if (!ok) {
			return Converters.PyString_FromString("[...]");
		}
		try {
			StringBuilder s = new StringBuilder("[");
			int num_self = op.Count;
			for (int i = 0; i < num_self; i++) {
				if (i > 0)
					s.Append(", ");
				PyObject pob = (PyObject)(op[i]);
				s.Append( Converters.PyString_AsString( Runtime.PyObject_Repr( pob ) ) );
			}
			s.Append("]");
			return Converters.PyString_FromString(s.ToString());
		}
		finally {
			Runtime.Py_ReprLeave(self);
		}
	}
	public override int 
	tp_compare(PyObject self, PyObject other) {
		ArrayList aself = ((PyList)self.ob).list;
		ArrayList aother = ((PyList)other.ob).list;
		int num_self = aself.Count;
		int num_other = aother.Count;

		for (int i = 0; i < num_self && i < num_other; i++) {
			int cmp = Runtime.PyObject_Compare((PyObject)aself[i], (PyObject)aother[i]);
			if (cmp != 0)
				return cmp;
		}
		return num_self - num_other;
	}
	public override IPySequence 
	tp_as_sequence() {
		return this;
	}
	// IPySequence methods
	public int
	sq_length(PyObject self) {
		return ((PyList)self.ob).list.Count;
	}
	public PyObject
	sq_concat(PyObject self, PyObject other) {
		throw new NotSupportedException();
	}
	public PyObject
	sq_repeat(PyObject self, int num) {
		ArrayList a = ((PyList)self.ob).list;
		if (num<0) num=0;
		ArrayList ret = new ArrayList(num * a.Count);
		for (int i=0;i<num;i++)
			ret.AddRange(a);
		return Converters.PyList_FromList(ret);
	}

	public PyObject
	sq_item(PyObject self, int index) {
		ArrayList a = ((PyList)self.ob).list;
		if (index<0 || index >= a.Count)
			throw new IndexError("list index out of range");
		return (PyObject)(a[index]);
	}
	public PyObject
	sq_slice(PyObject self, int ilow, int ihigh) {
		ArrayList a = ((PyList)self.ob).list;
		if (ilow < 0)
			ilow = 0;
		else if (ilow > a.Count)
			ilow = a.Count;
		if (ihigh < ilow)
			ihigh = ilow;
		else if (ihigh > a.Count)
			ihigh = a.Count;
		int n = ihigh-ilow;
		ArrayList ret = new ArrayList(n);
		IEnumerator ienum = a.GetEnumerator(ilow, n);
		while (ienum.MoveNext())
			ret.Add(ienum.Current);
		return Converters.PyList_FromList(ret);
	}

	public void
	sq_ass_item(PyObject self, int index, PyObject  value) {
		ArrayList a = ((PyList)self.ob).list;
		if (index<0 || index >= a.Count)
			throw new IndexError("list assignment index out of range");
		a[index] = value;
	}

	public void
	sq_del_item(PyObject self, int index) {
		ArrayList a = ((PyList)self.ob).list;
		if (index<0 || index >= a.Count)
			throw new IndexError("list assignment index out of range");
		a.RemoveAt(index);
	}

	public void
	sq_ass_slice(PyObject self, int ilow, int ihigh, PyObject val) {
		/* XXX - NEED TO Special case "a[i:j] = a" -- copy b first */
		ArrayList a = ((PyList)self.ob).list;
		IList replace = Converters.PyList_AsList(val);
		if (ilow < 0)
			ilow = 0;
		else if (ilow > a.Count)
			ilow = a.Count;
		if (ihigh < ilow)
			ihigh = ilow;
		else if (ihigh > a.Count)
			ihigh = a.Count;
		a.RemoveRange(ilow, ihigh-ilow);
		a.InsertRange(ilow, replace);
	}
};

public class PyClassType : PyType {
	public override String tp_name() {
		return "class";
	}
	public override 
	Type tp_type() {
		return typeof(Type);
	}
	public override PyObject
	tp_repr(PyObject s) {
		System.Type self = (System.Type)(s.ob);
		return Converters.PyString_FromString(String.Format("<class {0} at {1}>", self.FullName, Runtime.PyObject_GetId(s)));
	}
	public override PyObject tp_getattr(PyObject s, System.String name)
	{
		System.Type self = (System.Type)(s.ob);
		if (name=="__name__")
			return Converters.PyString_FromString(self.FullName);
		if (name=="__object__")
			return new PyObject(self, Runtime.PyInstance_Type);
		// First see if we have a field of that name.
		FieldInfo fi = self.GetField(name);
		if (fi != null) {
			// This object does have a property of that name - use it.
			Object ob = fi.GetValue( null);
			return Converters.PyObject_FromObject(ob);
		}
		// No field - let's try static property.
		// This causes a reflection invocation, so we need protection!
		try {
			PropertyInfo pi = self.GetProperty(name, BindingFlags.Public | BindingFlags.GetProperty | BindingFlags.Static /*| BindingFlags.InvokeMethod */);
			if (pi != null) {
				// This object does have a property of that name - use it.
				Object var = pi.GetValue( null, (object[])null );
				return Converters.PyObject_FromObject(var);
			}
		}
		catch (System.Exception e) {
			throw Runtime.PyErr_TransformReflectionException(e);
		}
		// No field or property- see if we have a method of that name.
		// These dont actually trigger invoke's, so no exception protection.
		try {
			PyBuiltinMethodDelegate d = (PyBuiltinMethodDelegate)
			                                              Delegate.CreateDelegate(self, typeof(PyBuiltinMethodDelegate), name);
			return new PyObject(new PyBuiltinMethod(d, Runtime.Py_None, Runtime.Py_None), Runtime.PyBuiltinMethod_Type);
		}
		catch (ArgumentException) {
			// no Python signature of that name.
			// do nothing.
		}
		try {
			MethodInfo info = self.GetMethod(name);
			if (info != null)
				if (info.IsStatic)
					return new PyObject(new PyFunction( info ), Runtime.PyFunction_Type);
				else
					return new PyObject(new PyMethod( info, null), Runtime.PyMethod_Type); // unbound method
		}
		catch (AmbiguousMatchException) {
			// Damn - lots of methods with that name :-(
			// Assume static - XXX - should check if they all are static or non!
			return new PyObject(new PyFunction( self, name ), Runtime.PyFunction_Type);
		}
		// OK - lets try doing the Python thing - look for __dict__.
		throw new AttributeError(name);
/***
		BindingFlags flags = (BindingFlags.GetField | BindingFlags.Static);
		Object ob_dict = self.InvokeMember("__class_dict__", flags, null, null, null);
		Hashtable dict = null;
		try {
			if (ob_dict != null)
				dict = (Hashtable )(ob_dict);
		}
		catch (InvalidCastException) {
			// Not a hashtable - give up.
			;
		}
		if (dict==null)
			throw new AttributeError(name);
		if (name=="__dict__")
			return ob_dict;
		// Need to to the bases thing.
		Object ret = dict[name];
		if (ret==null)
			throw new AttributeError(name);
		if (ret==Null.Value)
			ret = null;
		return ret;
***/
	}
	public override PyObject tp_call(PyObject s, PyObject[] args, PyObject kw)
	{
		System.Type self = (System.Type)(s.ob);
		int numArgs = 0;
		Object[] varArgs;
		// See if it has a special Python ctor.
		Type[] pyctor_args = new Type[3];
		pyctor_args[0] = typeof(PyObject);
		pyctor_args[1] = typeof(PyObject[]);
		pyctor_args[2] = typeof(PyObject);
		ConstructorInfo ci = self.GetConstructor(pyctor_args);
		if (ci != null) {
			varArgs = new Object[3];
			varArgs[0] = s;
			varArgs[1] = args;
			varArgs[2] = kw;
			object ret = ci.Invoke(varArgs);
			return Converters.PyInstance_FromInstance(ret);
		}
		// Just use reflection to call a default constructor.
		if (args != null) numArgs = args.Length;

		BindingFlags flags = BindingFlags.CreateInstance | BindingFlags.Default;
		ConstructorInfo[] cinfos = self.GetConstructors();
		ConstructorInfo cinfo = null;
		if (cinfos.Length==1) // XXX - need to get smarter with finding the ctor for defaults?
			cinfo = cinfos[0];
		int neededArgs = cinfo==null ? numArgs : cinfo.GetParameters().Length;
		if (neededArgs < numArgs)
			throw new TypeError(String.Format("Too many args - expected {0}, got {1}", neededArgs, numArgs));
		varArgs = new Object [neededArgs]; // args can not be NULL!
		for (int i=0;i<numArgs;i++) {
			varArgs[i] = args[i].ob;
		}
		if (numArgs < neededArgs) {
			for (int i=numArgs;i<neededArgs;i++)
				varArgs[i] = null; // Missing.Value;
			// Using this flag when we dont have "Missing" (now "none")can cause AmbiguousMatch exceptions 
			// (although in the cases I have seen it, it shouldnt!)
			// Not using this flag when we do means we get an exception that "Missing" cant be converted
			// to the target type.  Variants are being removed from COM+ later anyway, so presumably
			// this "Missing" will need rework...
			flags |= BindingFlags.OptionalParamBinding;
		}

#if DEBUG_TRACE_PYCLASS
		Console.WriteLine("PyClassType constructing type '{0}' with {1} args", self, numArgs);
#endif
		return Converters.PyInstance_FromInstance(Activator.CreateInstance(self, flags, null, varArgs, null));
	}
};

public class PyInstanceType : PyType, IPyNumber, IPyMapping {
	public override
	String tp_name() {
		return "instance";
	}
	public override 
	Type tp_type() {
		return typeof(Object);
	}

	public override 
	Object tp_as_type(PyObject self, Type t) {
		return self.ob; // we will give it up for any type!
	}

	public override
	IPyNumber tp_as_number() {
		return this;
	}
	public override
	IPyMapping tp_as_mapping() {
		return this;
	}
	public override
	PyObject tp_getattr(PyObject self, System.String name) {
//		Console.WriteLine("getattr of instance object {0} for attr {1}", self, name);
		// First see if we have a field of that name.
		System.Type t = self.ob.GetType();
		if (name=="__class__")
			return Converters.PyObject_FromObject(self.ob.GetType());
		FieldInfo fi = t.GetField(name);
		try {
			if (fi != null) {
				// This object does have a property of that name - use it.
				Object var = fi.GetValue(self.ob);
				return Converters.PyObject_FromObject(var);
			}
			// Next see if we have a property of that name.
			PropertyInfo pi = t.GetProperty(name);
			if (pi != null) {
				// This object does have a property of that name - use it.
				Object var = pi.GetValue(self.ob, (object [])null);
				return Converters.PyObject_FromObject(var);
			}
		}
		catch (System.Exception e) {
			throw Runtime.PyErr_TransformReflectionException(e);
		}
		// No field - see if we have a method of that name.
		try {
			PyBuiltinMethodDelegate d = (PyBuiltinMethodDelegate)
							      Delegate.CreateDelegate(typeof(PyBuiltinMethodDelegate), t, name);
			return new PyObject(new PyBuiltinMethod(d, Runtime.Py_None, self), Runtime.PyBuiltinMethod_Type);
		}
		catch (ArgumentException) {
			// no Python signature of that name.
			// do nothing.
		}
		try {
			MethodInfo info = t.GetMethod(name);
			if (info != null)
				if (info.IsStatic)
					return new PyObject(new PyFunction( t, name ), Runtime.PyFunction_Type);
				else
					return new PyObject(new PyMethod(info, self.ob), Runtime.PyMethod_Type);
		}
		catch (AmbiguousMatchException) {
			// Damn - lots of methods with that name :-(
			// Assume non-static
			return new PyObject(new PyMethod(t, self.ob, name), Runtime.PyMethod_Type);
		}
		throw new AttributeError(name);
	}
	public override
	void tp_setattr(PyObject self, System.String name, PyObject value) {
		// First see if we have a field of that name.
		System.Type t = self.ob.GetType();
		FieldInfo fi = t.GetField(name);
		try {
			if (fi != null) {
				// This object does have a property of that name - use it.
				fi.SetValue(self.ob, value.ob);
				return;
			}
			PropertyInfo pi = t.GetProperty(name);
			if (pi != null) {
				// This object does have a property of that name - use it.
				pi.SetValue(self.ob, value.ob, null);
				return;
			}
		}
		catch (System.Exception e) {
			throw Runtime.PyErr_TransformReflectionException(e);
		}
		throw new AttributeError(String.Format("'{0}' instance has no attribute '{1}'", t.FullName, name));
	}
	public override
	PyObject tp_repr(PyObject self) {
		String s = String.Format("<{0} instance at {1}>", self.ob.GetType().FullName, Runtime.PyObject_GetId(self));
		return Converters.PyString_FromString(s);
	}
	// IPyNumber methods.
	public PyObject 
	nb_add( PyObject self, PyObject other ) {
		throw new NotSupportedException();
	}
	public PyObject 
	nb_subtract(PyObject self, PyObject other) {
		throw new NotSupportedException();
	}
	public PyObject 
	nb_multiply(PyObject self, PyObject other) {
		throw new NotSupportedException();
	}
	public PyObject 
	nb_divide(PyObject self, PyObject other) {
		throw new NotSupportedException();
	}
	public PyObject 
	nb_remainder(PyObject self, PyObject other) {
		throw new NotSupportedException();
	}
	public PyObject 
	nb_divmod(PyObject self, PyObject other) {
		throw new NotSupportedException();
	}
	public PyObject 
	nb_power(PyObject self, PyObject other, PyObject v) {
		throw new NotSupportedException();
	}
	public PyObject 
	nb_negative(PyObject self) {
		throw new NotSupportedException();
	}
	public PyObject 
	nb_positive(PyObject self) {
		throw new NotSupportedException();
	}
	public PyObject 
	nb_absolute(PyObject self) {
		throw new NotSupportedException();
	}
	public bool 
	nb_nonzero(PyObject self) {
		throw new NotSupportedException();
	}
	public PyObject 
	nb_invert(PyObject self) {
		throw new NotSupportedException();
	}
	public PyObject 
	nb_lshift(PyObject self, PyObject v) {
		throw new NotSupportedException();
	}
	public PyObject
	nb_rshift(PyObject self, PyObject v) {
		throw new NotSupportedException();
	}
	public PyObject 
	nb_and(PyObject self, PyObject v) {
		throw new NotSupportedException();
	}
	public PyObject 
	nb_xor(PyObject self, PyObject v) {
		throw new NotSupportedException();
	}
	public PyObject 
	nb_or(PyObject self, PyObject v) {
		throw new NotSupportedException();
	}
	public bool
	nb_coerce(PyObject self, ref PyObject cvt) {
		throw new NotSupportedException();
	}
	public PyObject
	nb_int(PyObject self) {
		try {
			return Converters.PyInt_FromInt(Convert.ToInt32(self.ob));
		}
		catch (System.ArithmeticException) {
			throw new NotSupportedException();
		}
	}
	public PyObject
	nb_long(PyObject self) {
		throw new NotSupportedException();
	}
	public PyObject
	nb_float(PyObject self) {
		throw new NotSupportedException();
	}
	public PyObject
	nb_oct(PyObject self) {
		return Converters.PyString_FromString(Convert.ToString(Convert.ToInt32(self.ob), 8));
	}
	public PyObject
	nb_hex(PyObject self) {
		return Converters.PyString_FromString(Convert.ToString(Convert.ToInt32(self.ob), 16));
	}
	// Mapping methods
	public int
	mp_length(PyObject self) {
		// Allow a property called "count"
		System.Type t = self.ob.GetType();
		BindingFlags flags = BindingFlags.Default | BindingFlags.IgnoreCase | BindingFlags.GetProperty | BindingFlags.InvokeMethod;
		try {
			Object v = t.InvokeMember("Count", flags, null, self.ob, (object [])null);
			return Convert.ToInt32(v);
		}
		catch (MissingMethodException) {
			throw new NotSupportedException();
		}
		catch (ArgumentException) {
			throw new NotSupportedException();
		}
		catch (System.Exception e) {
			throw Runtime.PyErr_TransformReflectionException(e);
		}
	}
	public PyObject
	mp_subscript(PyObject self, PyObject ss) {
		// Allow the default property to be got.
		System.Type t = self.ob.GetType();
		MemberInfo[] infos = t.GetDefaultMembers();
		foreach (MemberInfo info in infos) {
			// Do I need to check anything else?
			if (info.MemberType == MemberTypes.Property) {
				PropertyInfo pi = (PropertyInfo)info;
				Object []args = new Object[1];
				args[0] = ss.ob;
				Object ret = pi.GetValue(self.ob, args);
				return Converters.PyObject_FromObject(ret);
			}
		}
		throw new NotSupportedException();
	}
	public void
	mp_ass_subscript(PyObject self, PyObject ss, PyObject v) {
		// Allow the default property to be set.
		System.Type t = self.ob.GetType();
		MemberInfo[] infos = t.GetDefaultMembers();
		foreach (MemberInfo info in infos) {
			// Do I need to check anything else?
			if (info.MemberType == MemberTypes.Property) {
				PropertyInfo pi = (PropertyInfo)info;
				Object []args = new Object[1];
				args[0] = ss.ob;
				pi.SetValue(self.ob, v.ob, args);
				return;
			}
		}
		throw new NotSupportedException();
	}
	public void mp_del_subscript(PyObject self, PyObject ss) {
		throw new NotSupportedException();
	}
};

public class PyMethodType : PyType {

	// IPyType interface
	public override String tp_name() {
		return "method";
	}
	public override 
	Type tp_type() {
		return typeof(PyMethod);
	}
	public override PyObject tp_getattr(PyObject s, System.String name)
	{
		PyMethod self = (PyMethod)(s.ob);
		if (self.ob != null) { // only exist for bound methods
			if (name.CompareTo("im_func")==0)
				return new PyObject(new PyFunction(self.type, self.name), Runtime.PyFunction_Type);
			else if (name.CompareTo("im_self")==0)
				return new PyObject(self.ob, Runtime.PyInstance_Type);
			else if (name.CompareTo("im_class")==0)
				return new PyObject(self.type, Runtime.PyClass_Type);
		}
		throw new AttributeError("No method attribute named " + name);
	}
	public override
	PyObject tp_repr(PyObject s) {
		PyMethod self = (PyMethod)(s.ob);
		String ret;
		if (self.ob == null)
			ret = String.Format("<unbound method {0}.{1}>", self.type, self.name);
		else {
			Object[] obs = new Object[4];
			obs[0] = self.type;
			obs[1] = self.name;
			obs[2] = self.ob.GetType();
			obs[3] = Runtime.PyObject_GetId(s);
//			ret = String.Format("<method {0}.{1} of {2} instance at {3}>", self.type, self.name, self.ob.GetType(), Runtime.PyObject_GetId(s));
			ret = String.Format("<method {0}.{1} of {2} instance at {3}>", obs);
		}
		return Converters.PyString_FromString(ret);
	}
	public override 
	PyObject tp_call(PyObject s, PyObject[] args, PyObject kw) {
		PyMethod self = (PyMethod)(s.ob);
		int numArgs = args.Length;
		Object object_call = self.ob;
		int numRealArgs = numArgs;
		int firstArg = 0;
		ParameterInfo[] pinfos = (self.mi == null) ? null : self.mi.GetParameters();
		if (object_call==null) {
			if (numArgs==0)
				throw new TypeError("unbound method must be called with class instance 1st argument");
			object_call = args[0].ob;
			firstArg = 1;
			numRealArgs--;
		}
		int numNeededArgs = numRealArgs;
		if (pinfos != null) {
			if (pinfos.Length < numRealArgs)
				throw new TypeError(String.Format("Too many arguments - expected {0}, but got {1}", pinfos.Length, numRealArgs));
			numNeededArgs = pinfos.Length;
		}
		object[] varArgs = new object[numNeededArgs];
		for (int i=firstArg;i<numArgs;i++)
			varArgs[i-firstArg] = args[i].ob;
		for (int i=numRealArgs; i<numNeededArgs;i++)
			// XXX - what is the story with default args!?!?
			// I can't see to win here.
			// Using Missing.Value appears to work in every case _except_
			// when there is an explicit default of null!
			varArgs[i] = Missing.Value;

#if DEBUG_TRACE_PYMETHOD
		if (object_call!=null)
			Console.WriteLine("PyMethodType.tp_call calling {0}::{1}({2} args)", object_call.GetType().FullName, self.name, numNeededArgs );
		Console.WriteLine("PyMethodType.tp_call making call on {0} via {1}", object_call, self.mi==null? "Type::InvokeMember" : "MethodInfo::Invoke");
#endif
		BindingFlags flags = BindingFlags.InvokeMethod | BindingFlags.Default | BindingFlags.OptionalParamBinding;
		object ret;
		try {
			if (self.mi==null) 
				ret = self.type.InvokeMember(self.name, flags, null, object_call, varArgs);
			else
				ret = self.mi.Invoke(object_call, flags, null, varArgs, null);
#if DEBUG_TRACE_PYMETHOD
			Console.WriteLine("PyMethodType.tp_call returning {0}", ret);
#endif
			return Converters.PyObject_FromObject(ret);
		}
		catch (System.Exception e) {
			throw Runtime.PyErr_TransformReflectionException(e);
		}
	}
};

public class PyBuiltinMethodType : PyType {
	// IPyType interface
	public override String tp_name() {
		return "builtin method";
	}

	public override 
	Type tp_type() {
		return typeof(PyBuiltinMethod);
	}

	public override PyObject
	tp_call(PyObject s, PyObject[] args, PyObject kw)
	{
		PyBuiltinMethod self = (PyBuiltinMethod)(s.ob);
		PyObject self_use = self.self.ob==null ? s : self.self;
		PyObject[] args_use = args;
		if (self.bound.ob != null) {
			int num_real_args = args.Length;
			args_use = new PyObject[num_real_args+1];
			args_use[0] = self.bound;
			Array.Copy(args, 0, args_use, 1, num_real_args);
		}
		return self.d(self_use, args_use, kw);
	}
};

public class PyFunctionType : PyType {
	// IPyType interface
	public override
	String tp_name() {
		return "function";
	}
	public override 
	Type tp_type() {
		return typeof(PyFunction);
	}
	public override
	PyObject tp_repr(PyObject s) {
		PyFunction self = (PyFunction)(s.ob);
		String ret = String.Format("<method {0} of {1} object at {2}>", self.name, self.type, Runtime.PyObject_GetId(s));
		return Converters.PyString_FromString(ret);
	}
	public override 
	PyObject tp_call(PyObject s, PyObject[] args, PyObject kw) {
		PyFunction self = (PyFunction)(s.ob);
		int numArgs = args.Length;
		object[] varArgs = numArgs!=0 ? new object [numArgs] : null;
		for (int i=0;i<numArgs;i++) {
			varArgs[i] = args[i].ob;
		}
#if DEBUG_TRACE_PYFUNCTION
		Console.WriteLine("Invoke on PyFunction " + self.name + " has num args " + Convert.ToString(numArgs));
		Console.WriteLine("Invoke has type " + self.type.FullName + "/" + self.type.ToString() + "/" + self.type.GUID.ToString() + "/" + self.type.Module.FullyQualifiedName);
		Console.WriteLine("Invoking via {0}", self.mi==null ? "type" : "method info");
#endif
//		BindingFlags flags = BindingFlags.InvokeMethod | BindingFlags.Static | BindingFlags.DefaultValueFull;
		BindingFlags flags = BindingFlags.Default | BindingFlags.Static | BindingFlags.InvokeMethod;
		object ret;
		try {
			if (self.mi == null)
				ret = self.type.InvokeMember(self.name, flags, null, null, varArgs);
			else
				ret = self.mi.Invoke(null, flags, null, varArgs, null);
#if DEBUG_TRACE_PYFUNCTION
			Console.WriteLine("Invoke returned " + ret.ToString());
#endif
			return Converters.PyObject_FromObject(ret);
		}
		catch (System.Exception e) {
			throw Runtime.PyErr_TransformReflectionException(e);
		}
	}
};

// This is for a COR module - _not_ a COR namespace
public class PyModuleType : PyType {
	// IPyType interface
	public override
	String tp_name() {
		return "module";
	}
//	void 	tp_print(PyObject , System.IO.TextWriter *, int);
	public override 
	Type tp_type() {
		return typeof(Module);
	}
	public override
	PyObject	tp_getattr(PyObject s, String this_name) {
		Module self = (Module)(s.ob);
		// First see if it is a Python module - if so, just delegate to it's __dict__.
		// XXX - this is slow - we dont really want to take this reflection hit
		// each attribute reference...
		int base_len = self.Name.Length - System.IO.Path.GetExtension(self.Name).Length;
		String py_type_name = self.Name.Substring(0, base_len) + "$main";
		Type t = self.GetType(py_type_name);
		try {
			if (t != null) {
				FieldInfo fi = t.GetField("__dict__");
				PyObject ob = (PyObject)fi.GetValue(null);
				return Runtime.PyObject_GetItem(ob, Converters.PyString_FromString(this_name));
			}
	
			// Do we have a class?
			t = self.GetType(this_name);
			if (t != null)
				return Converters.PyClass_FromClass(t);
		}
		catch (System.Exception e) {
			throw Runtime.PyErr_TransformReflectionException(e);
		}
		throw new AttributeError(String.Format("module '{0}' has no attribute '{1}'", self.FullyQualifiedName, this_name));
	}
	public override
	PyObject tp_repr(PyObject s) {
		Module self = (Module)(s.ob);
		String ret = String.Format("<module '{0}' from '{1}'>", self.Name, self.FullyQualifiedName);
		return Converters.PyString_FromString(ret);
	}
};

// This is for a COR namespace - _not_ a COR module
public class PyNamespaceType : PyType {
	// IPyType interface
	public override
	String tp_name() {
		return "namespace";
	}
	public override 
	Type tp_type() {
		return typeof(PyNamespace);
	}
	public override
	PyObject tp_getattr(PyObject s, String this_name) {
		PyNamespace self = (PyNamespace)(s.ob);
		Object oret = (self.subobjects[this_name]);
		if (oret != null) {
#if DEBUG_TRACE
			Console.WriteLine("Namespace.getattr for {0} returning cached type {1}", this_name, (PyObject)oret);
#endif
			return (PyObject)(oret);
		}
		// Try and load it from COM+
		String fullName = "";
		if (self.full_name.Length != 0) fullName = self.full_name + "." ;
		fullName = fullName + this_name;

#if DEBUG_TRACE
		Console.WriteLine("Namespace.getattr looking for type '" + fullName + "'");
#endif
		Type t = Type.GetType(fullName);
		if (t != null) {
			PyObject ret = Converters.PyClass_FromClass(t);
			self.subobjects[this_name] = ret;
#if DEBUG_TRACE
			Console.WriteLine("Namespace.getattr returning type '{0}' - {1}", fullName, ret);
#endif
			return ret;
		}
		// Otherwise, assume it is a sub name, so just store it away.
		PyObject newNS = new PyObject(new PyNamespace(fullName, this_name), Runtime.PyNamespace_Type);
		self.subobjects[this_name] = newNS;
#if DEBUG_TRACE
		Console.WriteLine("Namespace.getattr returning sub-namespace '{0}' ('{1}')", this_name, fullName);
#endif
		return newNS;
	}
	public override
	PyObject tp_repr(PyObject s) {
		PyNamespace self = (PyNamespace)(s.ob);
		String ret = String.Format("<{0} namespace>", self.full_name);
		return Converters.PyString_FromString(ret);
	}
};

// A helper object for performing simple enumeration over Python types.
// much easier in the COM+ world.
public interface IPyEnumerator {
	bool MoveNext();
	PyObject Current { get; }
	void Reset();
};

public class EnumeratorCOREnumerator : IPyEnumerator {
	public EnumeratorCOREnumerator(IEnumerator ienum) {this.ienum=ienum;}
	public bool MoveNext() {return ienum.MoveNext();}
	public PyObject Current {
		get {
			return Converters.PyObject_FromObject(ienum.Current);
		}
	}
	public void Reset() {ienum.Reset();}
	private IEnumerator ienum;
}
	
public class EnumeratorPyObject : IPyEnumerator {
	public
	EnumeratorPyObject(PyObject seq) {
		current = Runtime.Py_None;
		sequence = seq;
		typ_seq = seq.typ.tp_as_sequence();
		Reset();
	}

	public bool
	MoveNext() {
		bool rc;
		try {
			current = typ_seq.sq_item(sequence, index);
//			Console.WriteLine("sequence returned {0}/{1}", current.ob, (Object)(current.typ));
			index++;
			rc = true;
		}
		catch (IndexError) {
//			current = null;
			rc = false;
		}
		return rc;
	}

	public PyObject
	Current {
		get {
			return current;
		}
	}

	public void
	Reset() {
		index = 0;
//		current = null;
	}

	private PyObject sequence;
	private PyObject current;
	private IPySequence typ_seq;
	private int index;
};

}; // end of namespace types

}; // end of namespace Builtins

// Utility functions for converting to and from COR objects and PyObjects
//
// XXX - IMO (In Mark's opinion :-) we should declare InvalidCastException as the official error when using
// these API's - this will be the natural exception thrown by most conversion failures - save wrapping them all
// in TypeErrors at this low level.  The higher layers should probably transform the exceptions before Python sees them.
public class Converters {
	static Converters() {
		TypeMap = new Hashtable();
		int n = builtin_types.Length;
		for(int i=0;i<n;i++) {
			IPyType pyt = builtin_types[i];
			TypeMap[pyt.tp_type()] = pyt;
		}
		// Now add an extry for each of our types (as we cant just look up IPyObject
		for(int i=0;i<n;i++) {
			IPyType pyt = builtin_types[i];
			TypeMap[pyt.GetType()] = Runtime.PyType_Type;
		}
/**
		TypeMap[typeof(String)] = Runtime.PyString_Type;
		TypeMap[typeof(Int32)] = Runtime.PyInt_Type;
		TypeMap[typeof(PyDict)] = Runtime.PyDict_Type;
		TypeMap[typeof(PyList)] = Runtime.PyList_Type;
		TypeMap[typeof(PyFunction)] = Runtime.PyFunction_Type;
		TypeMap[typeof(PyMethod)] = Runtime.PyMethod_Type;
		TypeMap[typeof(PyBuiltinMethod)] = Runtime.PyBuiltinMethod_Type;
		TypeMap[typeof(PyNamespace)] = Runtime.PyNamespace_Type;
		TypeMap[typeof(Module)] = Runtime.PyModule_Type;
		TypeMap[typeof(PyTuple)] = Runtime.PyTuple_Type;
		TypeMap[typeof(IPyType)] = Runtime.PyType_Type;
**/
	}
	private static IPyType[] builtin_types = {
		Runtime.PyString_Type,
		Runtime.PyInt_Type,
		Runtime.PyFloat_Type,
		Runtime.PyDict_Type,
		Runtime.PyList_Type,
		Runtime.PyFunction_Type,
		Runtime.PyMethod_Type,
		Runtime.PyBuiltinMethod_Type,
		Runtime.PyNamespace_Type,
		Runtime.PyModule_Type,
		Runtime.PyTuple_Type,
		Runtime.PyInstance_Type,
	};
	private static
	Hashtable TypeMap;
	private static Type type_bool = typeof(bool);
	private static Type type_int16 = typeof(Int16);
	private static Type type_single = typeof(float);
	private static Type type_pyobject = typeof(PyObject);

	public static PyObject
	PyObject_FromObject(Object ob) {
		if (ob==null)
			return Runtime.Py_None;
		Type t = ob.GetType();
//		Console.WriteLine("Have type {0} for {1}", t, ob);
		IPyType pyt = null;
		if (t==type_bool) {
			ob = Convert.ToInt32((bool)ob);
			pyt = Runtime.PyInt_Type;
		} else if (t==type_int16) {
			ob = Convert.ToInt32((Int16)ob);
			pyt = Runtime.PyInt_Type;
		} else if (t==type_single) {
			ob = Convert.ToDouble((float)ob);
			pyt = Runtime.PyFloat_Type;
		} else {
			pyt = (IPyType) TypeMap[t];
			if (pyt == null) {
				if (t.IsArray)
					pyt = Runtime.PyArray_Type;
				else if (t==Type.GetType("System.RuntimeType")) // RuntimeType is protected!?
					pyt = Runtime.PyClass_Type;
			}
			if (t==type_pyobject)
				return (PyObject)ob;
			if (pyt==null)
				pyt = Runtime.PyInstance_Type;
		}
		return new PyObject(ob, pyt);
	}

	// String converters
	public static bool
	PyString_Check(PyObject ob) {
		return ob.typ == Runtime.PyString_Type;
	}
	public static PyObject 
	PyString_FromString(String s) {
		return new PyObject(s, Runtime.PyString_Type);
	}
	public static String 
	PyString_AsString(PyObject o) {
		return (String)(o.ob);
	}
	public static Char 
	PyString_AsChar(PyObject o) {
		String s = PyString_AsString(o);
		if (s.Length != 1)
			throw new TypeError("Expected a string with exactly one character");
		return s[0];
	}
	// Int converters
	public static bool
	PyInt_Check(PyObject ob) {
		return ob.typ == Runtime.PyInt_Type;
	}
	public static PyObject 
	PyInt_FromInt(Int32 i) {
		return new PyObject(i, Runtime.PyInt_Type);
	}
	public static PyObject 
	PyInt_FromInt64(Int64 i) {
		return new PyObject((Int32)i, Runtime.PyInt_Type);
	}
	public static PyObject 
	PyInt_FromBool(bool i) {
		return new PyObject(i ? 1 : 0, Runtime.PyInt_Type);
	}
	public static int 
	PyInt_AsInt32(PyObject o) {
		return (Int32)(o.ob);
	}
	public static int
	PyInt_AsInt64(PyObject o) {
		return (Int32)(o.ob);
	}

	// Float
	public static PyObject
	PyFloat_FromDouble(Double d) {
		return new PyObject(d, Runtime.PyFloat_Type);
	}
	public static Double
	PyFloat_AsDouble(PyObject o)
	{
		return (Double)o.ob;
	}

	// List objects
	public static PyObject 
	PyList_FromCollection(ICollection c) {
		IEnumerator en = c.GetEnumerator();
		ArrayList l = new ArrayList(c.Count);
		while (en.MoveNext()) {
			l.Add(Converters.PyObject_FromObject(en.Current));
		}
		return PyList_FromList(l);
	}
	public static PyObject
	PyList_FromList(ArrayList l) {
		return new PyObject(new PyList(l), Runtime.PyList_Type);
	}
	public static IList 
	PyList_AsList(PyObject p) {
		return ((PyList)p.ob).list;
	}
	// Tuple objects
	public static bool
	PyTuple_Check(PyObject ob) {
		return ob.typ == Runtime.PyTuple_Type;
	}
	
	public static PyObject 
	PyTuple_FromArray(PyObject[] anew) {
		return new PyObject( new PyTuple(anew), Runtime.PyTuple_Type);
	}
	public static PyObject[]
	PyTuple_AsArray(PyObject self) {
		return ((PyTuple)self.ob).a;
	}
	// Array object
	public static PyObject 
	PyArray_FromArray(Object[] a) {
		return new PyObject(a, Runtime.PyArray_Type);
	}
	// Module objects
	public static PyObject 
	PyModule_FromModule(Module m) {
		return new PyObject(m, Runtime.PyModule_Type);
	}
	// Class objects
	public static PyObject 
	PyClass_FromClass(Type ob) {
		return new PyObject(ob, Runtime.PyClass_Type);
	}
	public static Type 
	PyClass_AsType(PyObject ob) {
		return (Type)(ob.ob);
	}
	// Instance objects
	public static PyObject 
	PyInstance_FromInstance(Object ob) {
		return new PyObject(ob, Runtime.PyInstance_Type);
	}
	public static bool
	PyInstance_Check(PyObject ob) {
		return false; // XXX - todo!
	}
}

// The actual runtime engine.
public class Runtime {

	static public int 
	Py_PRINT_RAW = 1;
	public static IPyEnumerator PyObject_GetEnumerator(PyObject ob)
	{
		// If the type is a sequence, we can wrap it.
		IPySequence seq = PyObject_Type(ob).tp_as_sequence();
		if (seq != null) {
			return new EnumeratorPyObject(ob);
		}
		// Give the object next crack.
		try {
			return new EnumeratorCOREnumerator((IEnumerator)(ob.ob));
		}
		catch (InvalidCastException) {
			// do nothing
		}
		// If it is an enumerable object, then get its enumerator.
		try {
			// XXX - COR doc has IEnumerable, but I cant seem to see it!?
			ICollection eable = (ICollection)(ob.ob);
			return new EnumeratorCOREnumerator(eable.GetEnumerator());
		}
		catch (InvalidCastException) {
			// do nothing
		}
		return null;
//		throw new TypeError("loop over non-sequence");
	}

	public static PyObject
	PyObject_Repr(PyObject o)
	{
		IPyType t = PyObject_Type(o);
		try {
			return t.tp_repr(o);
		}
		catch (NotSupportedException) {
			// do nothing.
		}
		StringBuilder sb = new StringBuilder();
		String ret = String.Format("<{0} at {1}>", t.tp_name(), PyObject_GetId(o));
		return Converters.PyString_FromString(ret);
	}

	public static PyObject
	PyObject_Str(PyObject o)
	{
		IPyType t = PyObject_Type(o);
		try {
			return t.tp_str(o);
		}
		catch (NotSupportedException) {
			// do nothing.
		}
		return PyObject_Repr(o);
	}

	public static void
	PyObject_Print(PyObject ob, System.IO.TextWriter stream, int flags)
	{
		IPyType t = PyObject_Type(ob);
		try {
			// Try the print slot.
			t.tp_print(ob, stream, flags);
			return;
		}
		catch (NotSupportedException) {
			// do nothing.
		}
		PyObject strval;
		// Try the tp_repr or tp_str slot.
		if ((flags & Py_PRINT_RAW) != 0)
			strval = PyObject_Str(ob); // t.tp_str(ob);
		else
			strval = PyObject_Repr(ob); // t.tp_repr(ob);
		PyObject_Print(strval, stream, Py_PRINT_RAW);
		return;
	}

	public static
	int PyObject_GetId(PyObject self) {
		return self.ob.GetHashCode(); // System.Remoting.RemotingServices.Marshal(ob, 0).URI;
	}

	private static PyObject do_cmp(PyObject v, PyObject w)
	{
		throw new TypeError("Cant do instance comparisons yet!");
		
//		long c;
//		/* __rcmp__ actually won't be called unless __cmp__ isn't defined,
//		   because the check in cmpobject() reverses the objects first.
//		   This is intentional -- it makes no sense to define cmp(x,y)
//		   different than -cmp(y,x). */
//		if (PyInstance_Check(v) || PyInstance_Check(w))
//			return PyInstance_DoBinOp(v, w, "__cmp__", "__rcmp__", do_cmp);
//		c = PyObject_Compare(v, w);
//		if (c && PyErr_Occurred())
//			return null;
//		return Converters.PyInt_FromInt(c);
	}

	public static int
	PyObject_Compare(PyObject v, PyObject w)
	{
		IPyType vtp, wtp;
//		if (v.ob == w.ob) // XXX - is this valid???
//			return 0;
		if (Converters.PyInstance_Check(v) || Converters.PyInstance_Check(w)) {
			Console.WriteLine("instances!!");
			PyObject res;
			if (!Converters.PyInstance_Check(v))
				return -PyObject_Compare(w, v);
			res = do_cmp(v, w);
			if (!Converters.PyInt_Check(res))
				throw new TypeError("comparison did not return an int");
			int c = Converters.PyInt_AsInt32(res);
			return (c < 0) ? -1 : (c > 0) ? 1 : 0;
		}
		if ((vtp = PyObject_Type(v)) != (wtp = PyObject_Type(w))) {
			String vname = vtp.tp_name();
			String wname = wtp.tp_name();
			if (vtp.tp_as_number() != null && wtp.tp_as_number() != null) {
				if (PyNumber_CoerceEx(ref v, ref w)) {
					vtp = PyObject_Type(v);
					try {
						return vtp.tp_compare(v, w);
					}
					catch (NotSupportedException) {
						// do nothing!
					}
				}
				return v.GetHashCode() - w.GetHashCode(); // XXX - good enough!?!?
//					throw new TypeError("These objects of different type need to be compared by identity and Im not sure how yet!");
			}
			else if (vtp.tp_as_number() != null)
				vname = "";
			else if (wtp.tp_as_number() != null)
				wname = "";
			/* Numerical types compare smaller than all other types */
			return vname.CompareTo(wname);
		}
		try {
//			Console.WriteLine("same type compare {0}, {1}, {2}", v,w,vtp);
			return vtp.tp_compare(v, w);
		}
		catch (NotSupportedException) {
			// do nothing!
		}
		return v.GetHashCode() - w.GetHashCode(); // XXX - good enough
//		throw new TypeError("These objects of type that doesnt define a comparison function need by identity and Im not sure how yet!");
	}

	// XXX - need some sort of map here
	static public IPyType PyNothing_Type = new PyNothingType();
	static public IPyType PyString_Type = new PyStringType();
	static public IPyType PyInt_Type = new PyIntType();
	static public IPyType PyFloat_Type = new PyFloatType();
	static public IPyType PyDict_Type = new PyDictType();
	static public IPyType PyList_Type = new PyListType();
	static public IPyType PyArray_Type = new PyArrayType();
//	static IPyType PyOb_Type = new PyType;
	static public IPyType PyModule_Type = new PyModuleType();
	static public IPyType PyNamespace_Type = new PyNamespaceType();
	static public IPyType PyFunction_Type = new PyFunctionType();
	static public IPyType PyMethod_Type = new PyMethodType();
	static public IPyType PyBuiltinMethod_Type = new PyBuiltinMethodType();
	static public IPyType PyClass_Type = new PyClassType();
	static public IPyType PyInstance_Type = new PyInstanceType();
	static public IPyType PyTuple_Type = new PyTupleType();
	static public IPyType PyType_Type = new PyTypeType();

	static public PyObject Py_None = new PyObject(null, PyNothing_Type);

	public static IPyType PyObject_Type(PyObject ob)
	{
		IPyType t = ob.typ;
		return t==null ? PyNothing_Type : t;
	}
	// Abstract sequence methods
	public static PyObject 
	PySequence_GetItem(PyObject s, int i) {
		IPySequence m = PyObject_Type(s).tp_as_sequence();
		if (m != null) {
			if (i < 0) {
				try {
					i += m.sq_length(s);
				}
				catch (NotSupportedException) {
					// Do nothing!
				}
			}
			try {
				return m.sq_item(s, i);
			}
			catch (NotSupportedException) {
				// Do nothing!
			}
		}
		throw new TypeError("unindexable object");
	}

	public static void 
	PySequence_SetItem(PyObject s, int i, PyObject o) {
		IPySequence m = PyObject_Type(s).tp_as_sequence();
		if (m != null) {
			if (i < 0) {
				try {
					i += m.sq_length(s);
				}
				catch (NotSupportedException) {
					// Do nothing!
				}
			}
			try {
				m.sq_ass_item(s, i, o);
				return;
			}
			catch (NotSupportedException) {
				// Do nothing!
			}
		}
		throw new TypeError("object doesn't support item assignment");
	}

	public static void 
	PySequence_DelItem(PyObject s, int i) {
		IPySequence m = PyObject_Type(s).tp_as_sequence();
		if (m != null) {
			if (i < 0) {
				try {
					i += m.sq_length(s);
				}
				catch (NotSupportedException) {
					// Do nothing!
				}
			}
			try {
				m.sq_del_item(s, i);
				return;
			}
			catch (NotSupportedException) {
				// Do nothing!
			}
		}
		throw new TypeError("object doesn't support item deletion");
	}

	public static PyObject
	PySequence_GetSlice(PyObject s, int i1, int i2) {
		IPySequence m = PyObject_Type(s).tp_as_sequence();
		if (m != null) {
			if (i1 < 0 || i2 < 0) {
				try {
					int l = m.sq_length(s);
					if (i1 < 0)
						i1 += l;
					if (i2 < 0)
						i2 += l;
				}
				catch (NotSupportedException) {
					// do nothing...
				}
			}
			try {
				return m.sq_slice(s, i1, i2);
			}
			catch (NotSupportedException) {
				// do nothing...
			}
		}
		throw new TypeError(String.Format("'{0}' objects are not sliceable", PyObject_Type(s).tp_name()));
	}

	public static void
	PySequence_SetSlice(PyObject s, int i1, int i2, PyObject o) {
		IPySequence m = PyObject_Type(s).tp_as_sequence();
		if (m != null) {
			if (i1 < 0 || i2 < 0) {
				try {
					int l = m.sq_length(s);
					if (i1 < 0)
						i1 += l;
					if (i2 < 0)
						i2 += l;
				}
				catch (NotSupportedException) {
					// do nothing...
				}
			}
			try {
				m.sq_ass_slice(s, i1, i2, o);
				return;
			}
			catch (NotSupportedException) {
				// do nothing...
			}
		}
		throw new TypeError("object doesnt support slice assignment");
	}

	public static PyObject 
	PySequence_List(PyObject v) {
		IPySequence s = PyObject_Type(v).tp_as_sequence();
		if (s != null) {
			try {
				int len = s.sq_length(v);
				ArrayList al = new ArrayList(len);
				for (int i=0; ;i++) {
					try {
						al.Add(s.sq_item(v, i));
					}
					catch (IndexError) {
						break;
					}
				}
				return Converters.PyList_FromList(al);
			}
			catch (NotSupportedException) {
				// do nothing...
			}
		}
		throw new TypeError("list() argument must be a sequence");
	}
	public static PyObject 
	PySequence_Tuple(PyObject v) {
		IPySequence s = PyObject_Type(v).tp_as_sequence();
		if (s != null) {
			try {
				int len = s.sq_length(v);
				PyObject[]a = new PyObject[len];
				int i;
				for (i=0; ;i++) {
					PyObject item;
					try {
						item = s.sq_item(v, i);
					}
					catch (IndexError) {
						break;
					}
					if (i>=len) {
						int old_len = len;
						len = len < 500 ? len+10 : len + 100;
						PyObject[] anew = new PyObject[len];
						Array.Copy(a, anew, old_len);
						len = old_len;
						a = anew;
					}
					a[i] = item;
				}
				if (i<len) {
					PyObject[] anew = new PyObject[i];
					Array.Copy(a, anew, i);
					a = anew;
				}
				return Converters.PyTuple_FromArray(a);
			}
			catch (NotSupportedException) {
				// do nothing...
			}
		}
		throw new TypeError("tuple() argument must be a sequence");
	}
	//
	// Generic abstract methods
	//
	public static bool
	PyObject_IsTrue(PyObject v)
	{
		if (v.ob == null)
			return false;
		IPyType t = PyObject_Type(v);
		IPyNumber n = t.tp_as_number();
		if (n != null) {
			try {
				return n.nb_nonzero(v);
			}
			catch (NotSupportedException) {
				// Do nothing!
			}
		}
		IPyMapping m = t.tp_as_mapping();
		if (m != null) {
			try {
				return m.mp_length(v) > 0;
			}
			catch (NotSupportedException) {
				// Do nothing!
			}
		}
		IPySequence s = t.tp_as_sequence();
		if (s != null) {
			try {
				return s.sq_length(v) > 0;
			}
			catch (NotSupportedException) {
				// Do nothing!
			}
		}
		return true;
	}

	public static bool
	PyObject_Not(PyObject v) {
		return !PyObject_IsTrue(v);
	}

	public static int 
	PyObject_Length(PyObject o) {
		IPySequence m;
		m = PyObject_Type(o).tp_as_sequence();
		if (m != null) {
			try {
				return m.sq_length(o);
			}
			catch (NotSupportedException) {
				// Do nothing!
			}
		}
		return PyMapping_Length(o);
	}

	public static PyObject
	PyObject_GetItem(PyObject o, PyObject key) {
		IPyType t = PyObject_Type(o);
		IPyMapping m = t.tp_as_mapping();
		if (m != null) {
			try {
					return m.mp_subscript(o, key);
			}
			catch (NotSupportedException) {
				// Do nothing!
			}
		}
		if (t.tp_as_sequence() != null) {
			if (Converters.PyInt_Check(key))
				return PySequence_GetItem(o, Converters.PyInt_AsInt32(key));
			throw new TypeError("sequence index must be integer");
		}
		String name = t.tp_name();
		throw new TypeError(String.Format("'{0}' object is unsubscriptable", name));
	}

	public static void 
	PyObject_SetItem(PyObject o, PyObject key, PyObject value) {
		IPyType t = PyObject_Type(o);
		IPyMapping m = t.tp_as_mapping();
		if (m != null) {
			try {
				m.mp_ass_subscript(o, key, value);
				return;
			}
			catch (NotSupportedException) {
				// Do nothing!
			}
		}
		if (t.tp_as_sequence() != null) {
			if (Converters.PyInt_Check(key)) {
				PySequence_SetItem(o, Converters.PyInt_AsInt32(key), value);
				return;
			}
			throw new TypeError("sequence index must be integer");
		}
		String name = t.tp_name();
		throw new TypeError(String.Format("'{0}' object does not support item assignment", name));
	}

	public static void
	PyObject_DelItem(PyObject o, PyObject key) {
		IPyType t = PyObject_Type(o);
		IPyMapping m = t.tp_as_mapping();
		if (m != null) {
			try {
				m.mp_del_subscript(o, key);
				return;
			}
			catch (NotSupportedException) {
				// Do nothing!
			}
		}
		if (t.tp_as_sequence() != null) {
			if (Converters.PyInt_Check(key)) {
				PySequence_DelItem(o, Converters.PyInt_AsInt32(key));
				return;
			}
			throw new TypeError("sequence index must be integer");
		}
		String name = t.tp_name();
		throw new TypeError(String.Format("'{0}' object does not support item deletion", name));
	}

	public static PyObject
	PyObject_GetAttrString(PyObject ob, String attr) {
		IPyType t = PyObject_Type(ob);
/*		try {
			return t.tp_getattro(ob, attr);
		}
		catch (NotSupportedException) {
			// Do nothing.
		}
*/
		try {
#if DEBUG_TRACE
			Console.WriteLine("Calling getattr for '{0}' on {1})", attr, ob);
#endif			
			PyObject rc = t.tp_getattr(ob, attr);
#if DEBUG_TRACE
			Console.WriteLine("getattr('{0}')->{1}", attr, rc);
#endif
			return rc;
		}
		catch (NotSupportedException) {
			// Do nothing.
		}
		String name = t.tp_name();
		throw new AttributeError(String.Format("'{0}' object has no attribute '{1}'", name, attr));
	}

	public static PyObject
	PyObject_GetAttr(PyObject ob, PyObject attr) {
		return PyObject_GetAttrString(ob, Converters.PyString_AsString(attr));
	}

	public static void
	PyObject_SetAttrString(PyObject ob, String attr, PyObject value) {
		IPyType t = PyObject_Type(ob);
/*
		try {
			t.tp_setattro(ob, attr,value);
			return;
		}
		catch (NotSupportedException) {
			// Do nothing.
		}
*/
		try {
			t.tp_setattr(ob, attr,value);
			return;
		}
		catch (NotSupportedException) {
			// Do nothing.
		}
		String name = t.tp_name();
		throw new AttributeError(String.Format("'{0}' object has either no attributes, or read-only attributes", name));
	}

	public static void
	PyObject_SetAttr(PyObject ob, PyObject attr, PyObject value) {
		PyObject_SetAttrString(ob, Converters.PyString_AsString(attr), value);
	}

	public static PyObject
	PyObject_Wrap(PyObject ob) {
		// Create the "normal" PyObject
		return new PyObject(ob, PyInstance_Type);
	}

	public static System.Exception
	PyErr_TransformReflectionException(System.Exception e) {
//		return e;
		// The Reflection API caught an inner exception - we
		// need to present this inner exception to the Python code.
		if (e is TargetInvocationException) {
			System.Exception new_e = e;
			// If the inner exception is a Python exception, re-throw a new
			// Python exception of the same type
			if (e.InnerException is Python.Builtins.exceptions.Exception) {
				object[] args = new object[2];
				args[0] = e.InnerException.Message;
				args[1] = e;
				new_e = (System.Exception)Activator.CreateInstance(e.InnerException.GetType(), args);
			}
			return new_e;
		} 
		if (e is MissingMethodException) {
			return new TypeError(e.Message, e);
		}
		if (e is ArgumentException) {
			return new TypeError(e.Message, e);
		}
		return e;
	}

	public static PyObject
	PyObject_Call(PyObject ob, PyObject[] args, PyObject kw)
	{
#if DEBUG_TRACE
		Console.WriteLine("PyObject_Call: calling {0}({1} args)", ob, args.Length);
#endif
		IPyType t = PyObject_Type(ob);
		try {
			PyObject rc = t.tp_call(ob, args, kw);
#if DEBUG_TRACE
			Console.WriteLine("PyObject_Call: Result of {0}(): {1}", ob, rc);
#endif
			return rc;
		}
		catch (NotSupportedException)
		{
			// do nothing
		}
		String msg = String.Format("call of non-function (type {0})", t.tp_name());
		throw new TypeError(msg);
	}
	public static Object
	PyObject_AsExternalObject(PyObject self, Type t) {
		IPyType pyt = PyObject_Type(self);
		try {
			return pyt.tp_as_type(self, t);
		}
		catch (NotSupportedException) {
			// do nothing!
		}
		throw new TypeError(String.Format("Can not convert Python '{0}' objects to type '{1}'", pyt.tp_name(), t.FullName));
	}
	//
	// Abstract mapping methods
	//
	public static int
	PyMapping_Length(PyObject o) {
		IPyType t = PyObject_Type(o);
		try {
			IPyMapping m = t.tp_as_mapping();
			if (m!=null)
				return m.mp_length(o);
		}
		catch (NotSupportedException) {
			// Do nothing!
		}
		throw new TypeError(String.Format("len() of unsized object (type '{0}')", t.tp_name()));
	}

	public static bool
	PyMapping_Check(PyObject o) {
		IPyType t = PyObject_Type(o);
		try {
			IPyMapping m = t.tp_as_mapping();
			return m != null;
		}
		catch (NotSupportedException) {
			return false;
		}
	}

	//
	// Abstract number methods.
	//
	
	public static PyObject
	PyNumber_Int(PyObject s) {
		if (s.typ == PyString_Type) {
			return Converters.PyInt_FromInt(Convert.ToInt32( (String)(s.ob), 10));
		}
		IPyNumber nb = PyObject_Type(s).tp_as_number();
		try {
			if (nb != null)
				return nb.nb_int(s);
		}
		catch (NotSupportedException) {
			// Do nothing
		}
		throw new TypeError(String.Format("'{0}' object can't be converted to int", PyObject_Type(s).tp_name()));
	}

	public static PyObject
	PyNumber_Float(PyObject s) {
		if (s.typ == PyString_Type) {
			return Converters.PyFloat_FromDouble(Convert.ToDouble( (String)(s.ob)));
		}
		IPyNumber nb = PyObject_Type(s).tp_as_number();
		try {
			if (nb != null)
				return nb.nb_float(s);
		}
		catch (NotSupportedException) {
			// Do nothing
		}
		throw new TypeError(String.Format("'{0}' object can't be converted to float", PyObject_Type(s).tp_name()));
	}

	public static void
	PyNumber_Coerce(ref PyObject v, ref PyObject w) {
		if (!PyNumber_CoerceEx(ref v, ref w))
			throw new TypeError("Number coercion failed");
	}

	public static bool
	PyNumber_CoerceEx(ref PyObject v, ref PyObject w) {

		if (v.typ  == w.typ && !Converters.PyInstance_Check(v)) {
			return true;
		}
		try {
			IPyNumber nb = v.typ.tp_as_number();
			if (nb != null && nb.nb_coerce(v, ref w))
				return true;
		}
		catch (NotSupportedException) {
			// do nothing
		}
		try {
			IPyNumber nb = w.typ.tp_as_number();
			if (nb != null && nb.nb_coerce(w, ref v))
				return true;
		}
		catch (NotSupportedException) {
			// do nothing
		}
		return false;
	}

	public static bool
	PyNumber_Check(PyObject v) {
		return PyObject_Type(v).tp_as_number() != null;
	}
	public static PyObject
	PyNumber_Or(PyObject v, PyObject w) {
//		BINOP(v, w, "__or__", "__ror__", PyNumber_Xor);
		if (PyObject_Type(v).tp_as_number() != null) {
			PyNumber_Coerce(ref v, ref w);
			try {
				return PyObject_Type(v).tp_as_number().nb_or(v, w);
			}
			catch (NotSupportedException) {
				// Do nothing.
			}
		}
		throw new TypeError("bad operand type(s) for |");
	}
	public static PyObject 
	PyNumber_Xor(PyObject v, PyObject w) {
//		BINOP(v, w, "__xor__", "__rxor__", PyNumber_Xor);
		if (PyObject_Type(v).tp_as_number() != null) {
			PyNumber_Coerce(ref v, ref w);
			try {
				return PyObject_Type(v).tp_as_number().nb_xor(v, w);
			}
			catch (NotSupportedException) {
				// Do nothing.
			}
		}
		throw new TypeError("bad operand type(s) for ^");
	}
	public static PyObject 
	PyNumber_And(PyObject v, PyObject w) {
//		BINOP(v, w, "__and__", "__rand__", PyNumber_And);
		if (PyObject_Type(v).tp_as_number() != null) {
			PyNumber_Coerce(ref v, ref w);
			try {
				return PyObject_Type(v).tp_as_number().nb_and(v, w);
			}
			catch (NotSupportedException) {
				// Do nothing.
			}
		}
		throw new TypeError("bad operand type(s) for &");
	}
	public static PyObject 
	PyNumber_Lshift(PyObject v, PyObject w) {
//		BINOP(v, w, "__lshift__", "__rlshift__", PyNumber_Lshift);
		if (PyObject_Type(v).tp_as_number() != null) {
			PyNumber_Coerce(ref v, ref w);
			try {
				return PyObject_Type(v).tp_as_number().nb_lshift(v, w);
			}
			catch (NotSupportedException) {
				// Do nothing.
			}
		}
		throw new TypeError("bad operand type(s) for <<");
	}
	public static PyObject
	PyNumber_Rshift(PyObject v, PyObject w) {
//		BINOP(v, w, "__rshift__", "__rrshift__", PyNumber_Rshift);
		if (PyObject_Type(v).tp_as_number() != null) {
			PyNumber_Coerce(ref v, ref w);
			try {
				return PyObject_Type(v).tp_as_number().nb_rshift(v, w);
			}
			catch (NotSupportedException) {
				// Do nothing.
			}
		}
		throw new TypeError("bad operand type(s) for >>");
	}
	public static PyObject
	PyNumber_Add(PyObject v, PyObject w) {
		IPyType tpv = PyObject_Type(v);
		IPySequence m;

//		BINOP(v, w, "__add__", "__radd__", PyNumber_Add);
		m = tpv.tp_as_sequence();
		if (m != null) {
			try {
				return m.sq_concat(v, w);
			}
			catch (NotSupportedException) {
				// Do nothing.
			}
		} else if (tpv.tp_as_number() != null) {
			PyNumber_Coerce(ref v, ref w);
			try {
				return PyObject_Type(v).tp_as_number().nb_add(v, w);
			}
			catch (NotSupportedException) {
				// Do nothing.
			}
		}
		throw new TypeError("bad operand type(s) for +");
	}
	public static PyObject
	PyNumber_Subtract(PyObject v, PyObject w) {
		IPyType tpv = PyObject_Type(v);
		IPyNumber nbv;
//		BINOP(v, w, "__sub__", "__rsub__", PyNumber_Subtract);
		if ((nbv=tpv.tp_as_number()) != null) {
			PyNumber_Coerce(ref v, ref w);
			try {
				return PyObject_Type(v).tp_as_number().nb_subtract(v, w);
			}
			catch (NotSupportedException) {
				// Do nothing.
			}
		}
		throw new TypeError("bad operand type(s) for -");
	}
	public static PyObject
	PyNumber_Multiply(PyObject v, PyObject w) {
		IPyType tpv = PyObject_Type(v);
		IPyType tpw = PyObject_Type(w);
//		BINOP(v, w, "__mul__", "__rmul__", PyNumber_Multiply);
		if (tpv.tp_as_number() != null &&
		    tpw.tp_as_sequence() != null &&
		    !Converters.PyInstance_Check(v)) {
			/* number*sequence -- swap v and w */
			PyObject tmp = v;
			v = w;
			w = tmp;
			IPyType tptemp = tpv;
			tpv = tpw;
			tpw = tptemp;
		}
		if (tpv.tp_as_number() != null) {
			PyNumber_Coerce(ref v, ref w);
			try {
				return PyObject_Type(v).tp_as_number().nb_multiply(v, w);
			}
			catch (NotSupportedException) {
				// do nothing!
			}
		}
		IPySequence sqv = tpv.tp_as_sequence();
		if (sqv != null) {
			if (!Converters.PyInt_Check(w))
				throw new TypeError("can't multiply sequence with non-int");
			try {
				return sqv.sq_repeat(v, Converters.PyInt_AsInt32(w));
			}
			catch (NotSupportedException) {
				// Do nothing;
			}
		}
		throw new TypeError("bad operand type(s) for *");
	}
	public static PyObject
	PyNumber_Divide(PyObject v, PyObject w) {
//		BINOP(v, w, "__div__", "__rdiv__", PyNumber_Divide);
		if (PyObject_Type(v).tp_as_number() != null) {
			PyNumber_Coerce(ref v, ref w);
			try {
				return PyObject_Type(v).tp_as_number().nb_divide(v, w);
			}
			catch (NotSupportedException) {
				// Do nothing.
			}
		}
		throw new TypeError("bad operand type(s) for /");
	}
	public static PyObject
	PyNumber_Remainder(PyObject v, PyObject w) {
//		BINOP(v, w, "__mod__", "__rmod__", PyNumber_Remainder);
		if (Converters.PyString_Check(v))
			return PyStringType.Format(v, w);
		if (PyObject_Type(v).tp_as_number() != null) {
			PyNumber_Coerce(ref v, ref w);
			try {
				return PyObject_Type(v).tp_as_number().nb_remainder(v, w);
			}
			catch (NotSupportedException) {
				// Do nothing.
			}
		}
		throw new TypeError("bad operand type(s) for %");
	}
	public static PyObject
	PyNumber_Divmod(PyObject v, PyObject w) {
//		BINOP(v, w, "__divmod__", "__rdivmod__", PyNumber_Divmod);
		if (PyObject_Type(v).tp_as_number() != null) {
			PyNumber_Coerce(ref v, ref w);
			try {
				return PyObject_Type(v).tp_as_number().nb_divmod(v, w);
			}
			catch (NotSupportedException) {
				// Do nothing.
			}
		}
		throw new TypeError("bad operand type(s) for divmod()");
	}
	private static PyObject 
	do_pow(PyObject v, PyObject w) {
//		BINOP(v, w, "__pow__", "__rpow__", do_pow);
		if (PyObject_Type(v).tp_as_number() == null ||
		    PyObject_Type(w).tp_as_number() == null) {
			throw new TypeError("pow(x, y) requires numeric arguments");
		}
		PyNumber_Coerce(ref v, ref w);
		try {
			return PyObject_Type(v).tp_as_number().nb_power(v, w, Py_None);
		}
		catch (NotSupportedException) {
			// do nothing!
		}
		throw new TypeError("pow(x, y) not defined for these operands");
	}
	public static PyObject
	PyNumber_Power(PyObject v, PyObject w, PyObject z) {
		PyObject v1, z1, w2, z2;

		if (z.ob == null)
			return do_pow(v, w);
		/* XXX The ternary version doesn't do class instance coercions */
		if (Converters.PyInstance_Check(v))
			return PyObject_Type(v).tp_as_number().nb_power(v, w, z);
		if (PyObject_Type(v).tp_as_number() == null ||
		    PyObject_Type(z).tp_as_number() == null ||
		    PyObject_Type(w).tp_as_number() == null) {
			throw new TypeError("pow(x, y, z) requires numeric arguments");
		}
		PyNumber_Coerce(ref v, ref w);
		v1 = v;
		z1 = z;
		PyNumber_Coerce(ref v1, ref z1);
		w2 = w;
		z2 = z1;
	 	PyNumber_Coerce(ref w2, ref z2);
		try {
			return PyObject_Type(v1).tp_as_number().nb_power(v1, w2, z2);
		}
		catch (NotSupportedException) {
			// Do nothing.
		}
		throw new TypeError("pow(x, y, z) not defined for these operands");
	}
	/* Unary operators and functions */
	public static PyObject
	PyNumber_Negative(PyObject o) {
		IPyNumber m = PyObject_Type(o).tp_as_number();
		if (m != null) {
			try {
				return m.nb_negative(o);
			}
			catch (NotSupportedException) {
				// Do nothing.
			}
		}
		throw new TypeError("bad operand type for unary -");
	}
	public static PyObject
	PyNumber_Positive(PyObject o) {
		IPyNumber m = PyObject_Type(o).tp_as_number();
		if (m != null) {
			try {
				return m.nb_positive(o);
			}
			catch (NotSupportedException) {
				// Do nothing.
			}
		}
		throw new TypeError("bad operand type for unary +");
	}
	public static PyObject
	PyNumber_Invert(PyObject o) {
		IPyNumber m = PyObject_Type(o).tp_as_number();
		if (m != null) {
			try {
				return m.nb_invert(o);
			}
			catch (NotSupportedException) {
				// Do nothing.
			}
		}
		throw new TypeError("bad operand type for unary ~");
	}
	public static PyObject
	PyNumber_Absolute(PyObject o) {
		IPyNumber m = PyObject_Type(o).tp_as_number();
		if (m != null) {
			try {
				return m.nb_absolute(o);
			}
			catch (NotSupportedException) {
				// Do nothing.
			}
		}
		throw new TypeError("bad operand type for unary abs()");
	}
	// list helpers
	public static void 
	PyList_Append(PyObject l, PyObject ob) {
		((PyList)l.ob).list.Add(ob);
	}
	internal static
	bool Py_ReprEnter(PyObject self) {
		// XXX - to do - add recursion logic!
		return true;
	}
	internal static
	void Py_ReprLeave(PyObject self)
	{
		// XXX - to do - add recursion logic!
		return;
	}
	private static Module
	GetModuleFromAssembly(String aname, String mname) {
        Assembly a;
        try {
            a = Assembly.LoadWithPartialName(aname);
            if (a == null)
                return null;
        } catch (FileNotFoundException) {
            return null;
        }
		Module[] mods = a.GetModules();
		// An assembly with a single module - no probs
		int num_mods = mods.Length;
		for (int i=0;i<num_mods;i++) {
			Module m = mods[i];
			string name_cmp = m.Name;
			if (name_cmp.EndsWith(".mod"))
				name_cmp = name_cmp.Remove(name_cmp.Length-4,4);
			if (name_cmp == mname)
				return m;
		}
		return null;
	}
	static PyObject root = Py_None;
	public static PyObject PyImport_ImportModule(String name)
	{
		char[] seps = {'.'};
		String[] bits = name.Split(seps);
		if (bits[0].CompareTo("COR")==0) {
			if (root.ob==null)
				root = new PyObject(new PyNamespace("", bits[0]), PyNamespace_Type);
			PyObject sub = root;
			IPyType mod_type = new PyNamespaceType();
	//		String full_name = bits[0];
			for (int i=1;i<bits.Length;i++) {
	//			full_name = full_name + "." + bits[i];
				sub = mod_type.tp_getattr(sub, bits[i]);
			}
			return root;
		} else {
			// XXX - What is our story with Assemblies/Modules?
			// See if in our builtin assembly
			Module m = null;
			try {
				m = GetModuleFromAssembly("Python.Builtins.Builtins", name);
			}
			catch (System.IO.IOException) {
				; // m remains null.
			}
			if (m==null) {
				// Try an assembly of its own name.
				try {
					m = GetModuleFromAssembly(name, name);
				}
				catch (System.IO.IOException) {
					; // m remains null.
				}
			}
			if (m != null)
				return Converters.PyModule_FromModule(m);
		}
		throw new ImportError("No module named " + name);
	}
	private static Hashtable LoadBuiltins() {
//		Module mod = Module.GetModule("Python.Builtins.Builtins.dll");
//		Type mainType = mod.GetType("builtins$main");
		Assembly ass = Assembly.LoadWithPartialName("Python.Builtins.Builtins");
		if (ass==null)
			throw new SystemException("Can not load the builtins");

		Type mainType = ass.GetType("builtins$main");
		// Execute the startup code for the builtin module.
//		mainType.InvokeMember("$main", static_cast<BindingFlags>(BindingFlags.InvokeMethod | BindingFlags.Static), null, null, null);
		// Get the __dict__ field.
		FieldInfo fi = mainType.GetField("__dict__");
		PyObject pob = (PyObject)fi.GetValue(null);
		return ((PyDict)pob.ob).dict;
	}
	private static Hashtable builtins = null;

	public static PyObject __LookupGlobal(PyObject pyglobs, String name)
	{
		Hashtable globs = ((PyDict)pyglobs.ob).dict;
		PyObject obname = Converters.PyString_FromString(name);
		Object obret = globs[obname];
		if (obret==null) {
			if (builtins==null) {
				builtins = LoadBuiltins();
#if DEBUG_TRACE
				IEnumerator en = builtins.Keys.GetEnumerator();
				Console.WriteLine("Loaded the following items from builtins...");
				while (en.MoveNext())
					Console.WriteLine("Object {0}", en.Current);
				Console.WriteLine("Builtins dumped.");
#endif
			}
			obret = builtins[obname];
		}
		if (obret==null)
			throw new NameError(name);
//		if (ret==Null.Value)
//			ret = null;
		
		return (PyObject)(obret);
	}
	// Given a MethodInfo * and an array of args, make the call.
	public static void __Assign(PyObject value, PyObject dict, PyObject name)
	{
		PyObject_SetItem(dict, name, value);
//		if (value==null) value=Null.Value;
//		ht[name] = value;
	}
	public static void __SetItem(PyObject value, PyObject ob, PyObject key)
	{
		PyObject_SetItem(ob, key, value);
	}
	public static void __SetAttr(PyObject value, PyObject ob, String key)
	{
//		Console.WriteLine("__SetAttr {0}[{1}]={2}", ob, key, value);
		PyObject_SetAttrString(ob, key, value);
	}
	public static System.IO.TextWriter __GetStdOut()
	{
		return System.Console.Out;
	}
	public static PyObject[]
	__CombineVarArgs(PyObject[] userArgs, PyObject starArgs)
	{
		int user_len = userArgs.Length;
		PyObject[] ret;
		if (Converters.PyTuple_Check(starArgs)) {
			// quick path for tuples.
			PyObject[] other = Converters.PyTuple_AsArray(starArgs);
			int other_len = other.Length;
			ret = new PyObject[user_len + other_len];
			Array.Copy(userArgs, 0, ret, 0, user_len);
			Array.Copy(other, 0, ret, user_len, other_len);
		} else {
			IPyEnumerator pyenum = PyObject_GetEnumerator(starArgs);
			int num_other = 0;
			while (pyenum.MoveNext())
				num_other++;
			ret = new PyObject[user_len + num_other];
			Array.Copy(userArgs, 0, ret, 0, num_other);
			// Can enumerators change size?  Hope not, but be safe and count by index.
			pyenum.Reset();
			for (int i=0;i<num_other;i++) {
				if (pyenum.MoveNext())
					ret[user_len+i] = pyenum.Current;
				// else if just remains NULL/None
			}
		}
		return ret;
	}

};

}; // End "Python" namespace

