#include "stdafx.h"
#include "PythonCOM.h"
#include "PyComTypeObjects.h"
// @doc

/*static*/ ITypeLib *PyITypeLib::GetI(PyObject *self)
{
	return (ITypeLib *)PyIUnknown::GetI(self);
}
/*static*/ ITypeInfo *PyITypeInfo::GetI(PyObject *self)
{
	return (ITypeInfo *)PyIUnknown::GetI(self);
}

/*static*/ ITypeComp *PyITypeComp::GetI(PyObject *self)
{
	return (ITypeComp *)PyIUnknown::GetI(self);
}

/////////////////////////////////////////////////////////////////////////////
// class PyITypeInfo

PyITypeInfo::PyITypeInfo(IUnknown *ti) :
	PyIUnknown(ti)
{
	ob_type = &type;
}

PyITypeInfo::~PyITypeInfo()
{
}

PyObject *PyITypeInfo::GetContainingTypeLib()
{
	// BUGBUG??
	// Note that since we do not check to see if there is already a Python
	// object corresponding to the returned typelib, we could theoretically
	// end up with multiple Python objects pointing to the same OLE objects
	// Maybe we should to keep a global mapping of C/C++/OLE objects by
	// their memory address onto Python objects (by their memory address).
	ITypeInfo *pMyTypeInfo = GetI(this);
	if (pMyTypeInfo==NULL) return NULL;

	ITypeLib *ptlib;
	unsigned index;
	SCODE sc = pMyTypeInfo->GetContainingTypeLib(&ptlib, &index);
	if (FAILED(sc))
		return PyCom_BuildPyException(sc, pMyTypeInfo, IID_ITypeInfo);

	PyObject *ret = PyTuple_New(2);
	PyTuple_SetItem(ret, 0, PyCom_PyObjectFromIUnknown(ptlib, IID_ITypeLib));
	PyTuple_SetItem(ret, 1, PyInt_FromLong(index));
	return ret;
}

PyObject *PyITypeInfo::GetImplTypeFlags(int index)
{
	int implFlags;
	ITypeInfo *pMyTypeInfo = GetI(this);
	if (pMyTypeInfo==NULL) return NULL;

	PY_INTERFACE_PRECALL;
	SCODE sc = pMyTypeInfo->GetImplTypeFlags(index, &implFlags);
	PY_INTERFACE_POSTCALL;
	if (FAILED(sc))
		return PyCom_BuildPyException(sc, pMyTypeInfo, IID_ITypeInfo);

	return Py_BuildValue("i", implFlags);
}

PyObject *PyITypeInfo::GetDocumentation(MEMBERID id)
{
	BSTR name, docstring, helpfile;
	unsigned long helpctx;
	ITypeInfo *pMyTypeInfo = GetI(this);
	if (pMyTypeInfo==NULL) return NULL;

	PY_INTERFACE_PRECALL;
	SCODE sc = pMyTypeInfo->GetDocumentation(id, &name, &docstring, &helpctx, &helpfile);
	PY_INTERFACE_POSTCALL;
	if (FAILED(sc))
		return PyCom_BuildPyException(sc, pMyTypeInfo, IID_ITypeInfo);

	// NOTE - These BSTR's seem not to have a reasonable length.
	// Specifically, DAO3032 leaves crap at the end if we use
	// MakeBSTRToObj.
	PyObject *obName = MakeOLECHARToObj(name);
	PyObject *obDocstring = MakeOLECHARToObj(docstring);
	PyObject *obHelpfile = MakeOLECHARToObj(helpfile);

	PyObject *ret = Py_BuildValue("(OOiO)", obName, obDocstring, helpctx, obHelpfile);

	SysFreeString(name);
	Py_XDECREF(obName);
	SysFreeString(docstring);
	Py_XDECREF(obDocstring);
	SysFreeString(helpfile);
	Py_XDECREF(obHelpfile);

	return ret;
}

static PyObject* BuildFUNCDESC(ITypeInfo* pI,FUNCDESC* desc)
{
	PyObject *ret = PyObject_FromFUNCDESC(desc);

/***
	PyObject *sca = MakeSCODEArray(desc->lprgscode, desc->cScodes);
	PyObject *args = MakeElemDescArray(desc->lprgelemdescParam, desc->cParams);
	PyObject *rettype = MakeElemDesc(&desc->elemdescFunc);
	PyObject *ret = Py_BuildValue("(iOOiiiiiOi)",
		desc->memid,        // @tupleitem 0|int|memberId|
		sca,				// @tupleitem 1|(int, ...)|scodeArray|
		args,				// @tupleitem 2|(<o ELEMDESC>, ...)|args|
		desc->funckind,		// @tupleitem 3|int|funckind|
		desc->invkind,		// @tupleitem 4|int|invkind|
		desc->callconv,		// @tupleitem 5|int|callconv|
		desc->cParamsOpt,	// @tupleitem 6|int|cParamsOpt|
		desc->oVft,			// @tupleitem 7|int|oVft|
		rettype,			// @tupleitem 8|<o ELEMDESC>|returnType|
		desc->wFuncFlags);	// @tupleitem 9|int|wFuncFlags|

	Py_DECREF(sca);
	Py_DECREF(args);
	Py_DECREF(rettype);
***/
	{
	PY_INTERFACE_PRECALL;
	pI->ReleaseFuncDesc(desc);
	PY_INTERFACE_POSTCALL;
	}
	return ret;
}


PyObject *PyITypeInfo::GetFuncDesc(int index)
{
	FUNCDESC *desc;
	ITypeInfo *pMyTypeInfo = GetI(this);
	if (pMyTypeInfo==NULL) return NULL;

	PY_INTERFACE_PRECALL;
	SCODE sc = pMyTypeInfo->GetFuncDesc(index, &desc);
	PY_INTERFACE_POSTCALL;
	if (FAILED(sc))
		return PyCom_BuildPyException(sc, pMyTypeInfo, IID_ITypeInfo);
	return BuildFUNCDESC(pMyTypeInfo,desc);
}
/**********88
PyObject *PyITypeInfo::GetIDsOfNames(OLECHAR FAR* FAR* names, int count)
{
	ITypeInfo *pMyTypeInfo = GetI(this);
	if (pMyTypeInfo==NULL) return NULL;

	MEMBERID *ids = new MEMBERID[count];
	SCODE sc = pMyTypeInfo->GetIDsOfNames(names, count, ids);
	if (FAILED(sc))
	{
		delete [] ids;
		return PyCom_BuildPyException(sc, pMyTypeInfo, IID_ITypeInfo);
	}

	PyObject *ret = PyTuple_New(count);
	for (int i = 0; i < count; i++)
		PyTuple_SetItem(ret, i, PyInt_FromLong(ids[i]));
	
	delete [] ids;
	return ret;
}
**********/
PyObject *PyITypeInfo::GetNames(MEMBERID id)
{
	BSTR names[256];
	unsigned len = 0;
	ITypeInfo *pMyTypeInfo = GetI(this);
	if (pMyTypeInfo==NULL) return NULL;
	PY_INTERFACE_PRECALL;
	SCODE sc = pMyTypeInfo->GetNames(id, names, 256, &len);
	PY_INTERFACE_POSTCALL;
	if (FAILED(sc))
		return PyCom_BuildPyException(sc, pMyTypeInfo, IID_ITypeInfo);

	PyObject *ret = PyTuple_New(len);
	for (unsigned i = 0; i < len; i++)
	{
		// Again, MAkeBSTRToObj occasionally gives crap at EOS.
		PyObject *obString = MakeOLECHARToObj(names[i]);
		PyTuple_SetItem(ret, i, obString);
		SysFreeString(names[i]);
	}
	
	return ret;
}

PyObject *PyITypeInfo::GetTypeAttr()
{
	TYPEATTR *attr;
	ITypeInfo *pMyTypeInfo = GetI(this);
	if (pMyTypeInfo==NULL) return NULL;
	PY_INTERFACE_PRECALL;
	SCODE sc = pMyTypeInfo->GetTypeAttr(&attr);
	PY_INTERFACE_POSTCALL;
	if (FAILED(sc))
		return PyCom_BuildPyException(sc, pMyTypeInfo, IID_ITypeInfo);

/*	
	PyObject *obIID = PyWinObject_FromIID(attr->guid);
	PyObject *obDescAlias;
	// Some (only a few 16 bit MSOffice only one so far, and even then only occasionally!)
	// servers seem to send invalid tdescAlias when its not actually an alias.
	if (attr->typekind == TKIND_ALIAS)
		obDescAlias = MakeTypeDesc(&attr->tdescAlias);
	else {
		Py_INCREF(Py_None);
		obDescAlias=Py_None;
	}

	PyObject *obIDLDesc = MakeIDLDesc(&attr->idldescType);
	PyObject *ret = Py_BuildValue("(OiiiiiiiiiiiiiOO)",
		obIID,                   // @tupleitem 0|<o PyIID>|IID|The IID
		attr->lcid,				 // @tupleitem 1|int|lcid|The lcid
		attr->memidConstructor,	 // @tupleitem 2|int|memidConstructor|ID of constructor
		attr->memidDestructor,	 // @tupleitem 3|int|memidDestructor|ID of destructor,
		attr->cbSizeInstance,	 // @tupleitem 4|int|cbSizeInstance|The size of an instance of this type
		attr->typekind,			 // @tupleitem 5|int|typekind|The kind of type this information describes.  One of the win32con.TKIND_* constants.
		attr->cFuncs,			 // @tupleitem 6|int|cFuncs|Number of functions.
		attr->cVars,			 // @tupleitem 7|int|cVars|Number of variables/data members.
		attr->cImplTypes,		 // @tupleitem 8|int|cImplTypes|Number of implemented interfaces.
		attr->cbSizeVft,		 // @tupleitem 9|int|cbSizeVft|The size of this type's VTBL
		attr->cbAlignment,		 // @tupleitem 10|int|cbAlignment|Byte alignment for an instance of this type.
		attr->wTypeFlags,		 // @tupleitem 11|int|wTypeFlags|One of the pythoncom TYPEFLAG_
		attr->wMajorVerNum,		 // @tupleitem 12|int|wMajorVerNum|Major version number.
		attr->wMinorVerNum,		 // @tupleitem 13|int|wMinorVerNum|Minor version number.
		obDescAlias,			 // @tupleitem 14|<o TYPEDESC>|obDescAlias|If TypeKind == pythoncom.TKIND_ALIAS, specifies the type for which this type is an alias.
		obIDLDesc				 // @tupleitem 15|<o IDLDESC>|obIDLDesc|IDL attributes of the described type.
	);
	Py_XDECREF(obDescAlias);
	Py_XDECREF(obIDLDesc);
	Py_XDECREF(obIID);
***/
	PyObject *ret = PyObject_FromTYPEATTR(attr);

	{
	PY_INTERFACE_PRECALL;
	pMyTypeInfo->ReleaseTypeAttr(attr);
	PY_INTERFACE_POSTCALL;
	}

	return ret;
}

PyObject *PyITypeInfo::GetVarDesc(int index)
{
	VARDESC *desc;
	ITypeInfo *pMyTypeInfo = GetI(this);
	if (pMyTypeInfo==NULL) return NULL;
	PY_INTERFACE_PRECALL;
	SCODE sc = pMyTypeInfo->GetVarDesc(index, &desc);
	PY_INTERFACE_POSTCALL;
	if (FAILED(sc))
		return PyCom_BuildPyException(sc, pMyTypeInfo, IID_ITypeInfo);
	PyObject *ret = PyObject_FromVARDESC(desc);
	{
	PY_INTERFACE_PRECALL;
	pMyTypeInfo->ReleaseVarDesc(desc);
	PY_INTERFACE_POSTCALL;
	}
	return ret;
}

PyObject *PyITypeInfo::GetRefTypeInfo(HREFTYPE href)
{
	ITypeInfo *pti;
	ITypeInfo *pMyTypeInfo = GetI(this);
	if (pMyTypeInfo==NULL) return NULL;
	PY_INTERFACE_PRECALL;
	SCODE sc = pMyTypeInfo->GetRefTypeInfo(href, &pti);
	PY_INTERFACE_POSTCALL;
	if (FAILED(sc))
		return PyCom_BuildPyException(sc, pMyTypeInfo, IID_ITypeInfo);
	return new PyITypeInfo(pti);
}

PyObject *PyITypeInfo::GetRefTypeOfImplType(int index)
{
	HREFTYPE href;
	ITypeInfo *pMyTypeInfo = GetI(this);
	if (pMyTypeInfo==NULL) return NULL;
	PY_INTERFACE_PRECALL;
	SCODE sc = pMyTypeInfo->GetRefTypeOfImplType(index, &href);
	PY_INTERFACE_POSTCALL;
	if (FAILED(sc))
		return PyCom_BuildPyException(sc, pMyTypeInfo, IID_ITypeInfo);
	return Py_BuildValue("i", href);
}

PyObject *PyITypeInfo::GetTypeComp()
{
	ITypeInfo *pMyTypeInfo = GetI(this);
	ITypeComp *ptc;
	if (pMyTypeInfo==NULL) return NULL;
	PY_INTERFACE_PRECALL;
	SCODE sc = pMyTypeInfo->GetTypeComp(&ptc);
	PY_INTERFACE_POSTCALL;
	if (FAILED(sc))
		return PyCom_BuildPyException(sc, pMyTypeInfo, IID_ITypeInfo);

	return PyCom_PyObjectFromIUnknown(ptc, IID_ITypeComp);
}

// @pymethod <o PyITypeLib>, int|PyITypeInfo|GetContainingTypeLib|Retrieves the containing type library and the index of the type description within that type library.
static PyObject *typeinfo_getlib(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ""))
		return NULL;
	return ((PyITypeInfo*)self)->GetContainingTypeLib();
}

// @pymethod <o TYPEATTR>|PyITypeInfo|GetTypeAttr|Retrieves a <o TYPEATTR> object that contains the attributes of the type description.
static PyObject *typeinfo_getattr(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ""))
		return NULL;
	return ((PyITypeInfo*)self)->GetTypeAttr();
}

// @pymethod (name, docstring, helpContext, helpFile)|PyITypeInfo|GetDocumentation|Retrieves the documentation string, the complete Help file name and path, and the context ID for the Help topic for a specified type description.
static PyObject *typeinfo_getdocs(PyObject *self, PyObject *args)
{
	int pos;
	// @pyparm int|memberId||
	if (!PyArg_ParseTuple(args, "i", &pos))
		return NULL;
	return ((PyITypeInfo*)self)->GetDocumentation(pos);
}

// @pymethod <o FUNCDESC>|PyITypeInfo|GetFuncDesc|Retrieves the <o FUNCDESC> object that contains information about a specified function.
static PyObject *typeinfo_getfuncdesc(PyObject *self, PyObject *args)
{
	int pos;
	// @pyparm int|memberId||
	if (!PyArg_ParseTuple(args, "i", &pos))
		return NULL;
	return ((PyITypeInfo*)self)->GetFuncDesc(pos);
}

// @pymethod int|PyITypeInfo|GetImplTypeFlags|Retrieves the IMPLTYPEFLAGS enumeration for one implemented interface or base interface in a type description.
static PyObject *typeinfo_getimpltypeflags(PyObject *self, PyObject *args)
{
	int index;
	// @pyparm int|index||
	if (!PyArg_ParseTuple(args, "i", &index))
		return NULL;
	return ((PyITypeInfo*)self)->GetImplTypeFlags(index);
}

// @pymethod (tuple of strings)|PyITypeInfo|GetNames|Retrieves the variable with the specified member ID (or the name of the property or method and its parameters) that correspond to the specified function ID.
static PyObject *typeinfo_getnames(PyObject *self, PyObject *args)
{
	int pos;
	// @pyparm int|memberId||
	if (!PyArg_ParseTuple(args, "i", &pos))
		return NULL;
	return ((PyITypeInfo*)self)->GetNames(pos);
}

// @pymethod <o PyITypeInfo>|PyITypeInfo|GetRefTypeInfo|If a type description references other type descriptions, it retrieves the referenced type descriptions.
static PyObject *typeinfo_getreftypeinfo(PyObject *self, PyObject *args)
{
	int href;
	// @pyparm int|hRefType||
	if (!PyArg_ParseTuple(args, "i", &href))
		return NULL;
	return ((PyITypeInfo*)self)->GetRefTypeInfo((HREFTYPE)href);
}

// @pymethod int|PyITypeInfo|GetRefTypeOfImplType|Retrieves the type description of the implemented interface types.
static PyObject *typeinfo_getreftypeofimpltype(PyObject *self, PyObject *args)
{
	int index;
	// @pyparm int|hRefType||
	if (!PyArg_ParseTuple(args, "i:GetRefTypeOfImplType", &index))
		return NULL;
	return ((PyITypeInfo*)self)->GetRefTypeOfImplType(index);
	// @comm If a type description describes a COM class, it retrieves the type 
	// description of the implemented interface types. For an interface, 
	// GetRefTypeOfImplType returns the type information for inherited 
	// interfaces, if any exist.
}


// @pymethod <o VARDESC>|PyITypeInfo|GetVarDesc|Retrieves a <o VARDESC> object that describes the specified variable.
static PyObject *typeinfo_getvardesc(PyObject *self, PyObject *args)
{
	int pos;
	// @pyparm int|memberId||
	if (!PyArg_ParseTuple(args, "i", &pos))
		return NULL;
	return ((PyITypeInfo*)self)->GetVarDesc(pos);
}

// @pymethod int|PyITypeInfo|GetIDsOfNames|Maps between member names and member IDs, and parameter names and parameter IDs.
static PyObject *typeinfo_getidsofnames(PyObject *self, PyObject *args)
{
	// XXX - todo - merge this code with PyIDispatch::GetIDsOfNames
	UINT i;

	int argc = PyObject_Length(args);
	if ( argc == -1 )
		return NULL;
	if ( argc < 1 ) {
		PyErr_SetString(PyExc_TypeError, "At least one argument must be supplied");
		return NULL;
	}
	LCID lcid = LOCALE_SYSTEM_DEFAULT;
	UINT offset = 0;
	if ( argc > 1 )
	{
		PyObject *ob = PySequence_GetItem(args, 0);
		if ( !ob )
			return NULL;
		if ( PyInt_Check(ob) )
		{
			lcid = PyInt_AS_LONG((PyIntObject *)ob);
			if ( lcid == -1 )
				return NULL;
			offset = 1;
		}
	}

	UINT cNames = argc - offset;
	OLECHAR FAR* FAR* rgszNames = new LPOLESTR[cNames];

	for ( i = 0 ; i < cNames; ++i )
	{
		PyObject *ob = PySequence_GetItem(args, i + offset);
		if ( !ob )
		{
			for (;i>0;i--)
				PyWinObject_FreeBstr(rgszNames[i-1]);
			delete [] rgszNames;
			return NULL;
		}
		if (!PyWinObject_AsBstr(ob, rgszNames+i)) {
			for (;i>0;i--)
				PyWinObject_FreeBstr(rgszNames[i-1]);
			delete [] rgszNames;
			return NULL;
		}
		Py_DECREF(ob);
	}

	DISPID FAR* rgdispid = new DISPID[cNames];
	ITypeInfo *pti = PyITypeInfo::GetI(self);
	if (pti==NULL) return NULL;
	PY_INTERFACE_PRECALL;
	HRESULT hr = pti->GetIDsOfNames(rgszNames, cNames, rgdispid);
	PY_INTERFACE_POSTCALL;

	delete [] rgszNames;

	if ( FAILED(hr) )
		return PyCom_BuildPyException(hr, pti, IID_ITypeInfo);

	PyObject *result;

	/* if we have just one name, then return a single DISPID (int) */
	if ( cNames == 1 )
	{
		result = PyInt_FromLong(rgdispid[0]);
	}
	else
	{
		result = PyTuple_New(cNames);
		if ( result )
		{
			for ( i = 0; i < cNames; ++i )
			{
				PyObject *ob = PyInt_FromLong(rgdispid[i]);
				if ( !ob )
				{
					delete [] rgdispid;
					return NULL;
				}
				PyTuple_SET_ITEM(result, i, ob);
			}
		}
	}

	delete [] rgdispid;
	return result;
}

// @pymethod <o PyITypeComp>|PyITypeInfo|GetTypeComp|Retrieves the corrsponding type to DESCKIND mapping.
static PyObject *typeinfo_gettypecomp(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ":GetTypeComp"))
		return NULL;
	return ((PyITypeInfo*)self)->GetTypeComp();
}

// @object PyITypeInfo|An OLE automation type info object.  Derived from <o PyIUnknown>
static struct PyMethodDef PyITypeInfo_methods[] =
{
	{ "GetContainingTypeLib", typeinfo_getlib, 1 }, // @pymeth GetContainingTypeLib|Retrieves the containing type library and the index of the type description within that type library.
	{ "GetDocumentation", typeinfo_getdocs, 1 }, // @pymeth GetDocumentation|Retrieves the documentation string, the complete Help file name and path, and the context ID for the Help topic for a specified type description.
	{ "GetFuncDesc",      typeinfo_getfuncdesc, 1 }, // @pymeth GetFuncDesc|Retrieves the <o FUNCDESC> object that contains information about a specified function.
	{ "GetImplTypeFlags", typeinfo_getimpltypeflags, 1}, // @pymeth GetImplTypeFlags|Retrieves the IMPLTYPEFLAGS enumeration for one implemented interface or base interface in a type description.
	{ "GetIDsOfNames",    typeinfo_getidsofnames, 1 }, // @pymeth GetIDsOfNames|Maps between member names and member IDs, and parameter names and parameter IDs.
	{ "GetNames",         typeinfo_getnames, 1 }, // @pymeth GetNames|Retrieves the variable with the specified member ID (or the name of the property or method and its parameters) that correspond to the specified function ID.
	{ "GetTypeAttr",      typeinfo_getattr, 1 }, // @pymeth GetTypeAttr|Retrieves a <o TYPEATTR> object that contains the attributes of the type description.
	{ "GetRefTypeInfo",   typeinfo_getreftypeinfo, 1}, // @pymeth GetRefTypeInfo|If a type description references other type descriptions, it retrieves the referenced type descriptions.
	{ "GetRefTypeOfImplType",typeinfo_getreftypeofimpltype, 1}, // @pymeth GetRefTypeOfImplType|Retrieves the type description of the implemented interface types.
	{ "GetVarDesc",       typeinfo_getvardesc, 1 }, // @pymeth GetVarDesc|Retrieves a <o VARDESC> object that describes the specified variable.
	{ "GetTypeComp",      typeinfo_gettypecomp, 1 }, // @pymeth GetTypeComp|Retrieves a <o ITypeComp> object for Name to VARDESC/FUNCDESC mapping.
	{NULL,  NULL} 
};

PyComTypeObject PyITypeInfo::type("PyITypeInfo",
				&PyIUnknown::type, // @base PyITypeInfo|PyIUnknown
                 sizeof(PyITypeInfo),
                 PyITypeInfo_methods,
				 GET_PYCOM_CTOR(PyITypeInfo));

/////////////////////////////////////////////////////////////////////////////
// class PyITypeLib

PyITypeLib::PyITypeLib(IUnknown *tl) :
	PyIUnknown(tl)
{
	ob_type = &type;
}

PyITypeLib::~PyITypeLib()
{
}

// @object TLIBATTR|Type library attributes are represented as a tuple of:
PyObject *PyITypeLib::GetLibAttr()
{
	TLIBATTR *attr;
	ITypeLib *pMyTypeLib = GetI(this);
	if (pMyTypeLib==NULL) return NULL;
	PY_INTERFACE_PRECALL;
	SCODE sc = pMyTypeLib->GetLibAttr(&attr);
	PY_INTERFACE_POSTCALL;
	if (FAILED(sc))
		return PyCom_BuildPyException(sc, pMyTypeLib, IID_ITypeLib);

	PyObject *obIID = PyWinObject_FromIID(attr->guid);
	PyObject *ret = Py_BuildValue("Oiiiii",
		obIID,              // @tupleitem 0|<o PyIID>|IID|The IID for the library
		attr->lcid,         // @tupleitem 1|int|lcid|The default locale ID for the library
		attr->syskind,      // @tupleitem 2|int|syskind|Identifies the target operating system platform
		attr->wMajorVerNum,	// @tupleitem 3|int|majorVersion|The major version number of the library
		attr->wMinorVerNum,	// @tupleitem 4|int|minorVersion|The minor version number of the library
		attr->wLibFlags);	// @tupleitem 5|int|flags|Flags for the library.

	Py_DECREF(obIID);
	{
	PY_INTERFACE_PRECALL;
	pMyTypeLib->ReleaseTLibAttr(attr);
	PY_INTERFACE_POSTCALL;
	}

	return ret;
}

PyObject *PyITypeLib::GetDocumentation(int pos)
{
	BSTR name, docstring, helpfile;
	unsigned long helpctx;

	ITypeLib *pMyTypeLib = GetI(this);
	if (pMyTypeLib==NULL) return NULL;
	PY_INTERFACE_PRECALL;
	SCODE sc = pMyTypeLib->GetDocumentation(pos, &name, &docstring, &helpctx, &helpfile);
	PY_INTERFACE_POSTCALL;
	if (FAILED(sc))
		return PyCom_BuildPyException(sc, pMyTypeLib, IID_ITypeLib);

	PyObject *obName = MakeOLECHARToObj(name);
	PyObject *obDocstring = MakeOLECHARToObj(docstring);
	PyObject *obHelpfile = MakeOLECHARToObj(helpfile);
	PyObject *ret = Py_BuildValue("(OOiO)", obName, obDocstring, helpctx, obHelpfile);

	SysFreeString(name);
	Py_XDECREF(obName);
	SysFreeString(docstring);
	Py_XDECREF(obDocstring);
	SysFreeString(helpfile);
	Py_XDECREF(obHelpfile);
	return ret;
}

PyObject *PyITypeLib::GetTypeInfo(int pos)
{
	ITypeInfo *pti;
	ITypeLib *pMyTypeLib = GetI(this);
	if (pMyTypeLib==NULL) return NULL;
	PY_INTERFACE_PRECALL;
	SCODE sc = pMyTypeLib->GetTypeInfo(pos, &pti);
	PY_INTERFACE_POSTCALL;
	if (FAILED(sc))
		return PyCom_BuildPyException(sc, pMyTypeLib, IID_ITypeLib);

	return PyCom_PyObjectFromIUnknown(pti, IID_ITypeInfo);
}

PyObject *PyITypeLib::GetTypeInfoCount()
{
	ITypeLib *pMyTypeLib = GetI(this);
	if (pMyTypeLib==NULL) return NULL;
	PY_INTERFACE_PRECALL;
	long rc = pMyTypeLib->GetTypeInfoCount();
	PY_INTERFACE_POSTCALL;
	return PyInt_FromLong(rc);
}

PyObject *PyITypeLib::GetTypeInfoOfGuid(REFGUID guid)
{
	ITypeLib *pMyTypeLib = GetI(this);
	if (pMyTypeLib==NULL) return NULL;
	ITypeInfo *pti = NULL;
	PY_INTERFACE_PRECALL;
	HRESULT hr = pMyTypeLib->GetTypeInfoOfGuid(guid, &pti);
	PY_INTERFACE_POSTCALL;
	if (FAILED(hr))
		return PyCom_BuildPyException(hr, pMyTypeLib, IID_ITypeLib);
	return PyCom_PyObjectFromIUnknown(pti, IID_ITypeInfo);
}

PyObject *PyITypeLib::GetTypeInfoType(int pos)
{
	TYPEKIND tkind;
	ITypeLib *pMyTypeLib = GetI(this);
	if (pMyTypeLib==NULL) return NULL;
	PY_INTERFACE_PRECALL;
	SCODE sc = pMyTypeLib->GetTypeInfoType(pos, &tkind);
	PY_INTERFACE_POSTCALL;
	if (FAILED(sc))
		return PyCom_BuildPyException(sc, pMyTypeLib, IID_ITypeLib);

	return PyInt_FromLong(tkind);
}

// @pymethod <o TLIBATTR>|PyITypeLib|GetLibAttr|Retrieves the libraries attributes
static PyObject *typelib_getattr(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ":GetLibAttr"))
		return NULL;
	return ((PyITypeLib*)self)->GetLibAttr();
}

// @pymethod tuple|PyITypeLib|GetDocumentation|Retrieves documentation information about the library.
static PyObject *typelib_getdocs(PyObject *self, PyObject *args)
{
	int pos;
	// @pyparm int|index||The index of the type description within the library
	if (!PyArg_ParseTuple(args, "i:GetDocumentation", &pos))
		return NULL;
	// @rdesc The return type is a tuple of (name of item, documentation string, help context integer, help file name)
	return ((PyITypeLib*)self)->GetDocumentation(pos);
}

// @pymethod <o PyITypeInfo>|PyITypeLib|GetTypeInfo|Retrieves the specified type description in the library.
static PyObject *typelib_getinfo(PyObject *self, PyObject *args)
{
	int pos;
	// @pyparm int|index||The index of the type description within the library
	if (!PyArg_ParseTuple(args, "i:GetTypeInfo", &pos))
		return NULL;
	return ((PyITypeLib*)self)->GetTypeInfo(pos);
}

// @pymethod int|PyITypeLib|GetTypeInfoCount|Retrieves the number of <o PyITypeInfo>s in the type library.
static PyObject *typelib_getinfocnt(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ":GetTypeInfoCount"))
		return NULL;
	return ((PyITypeLib*)self)->GetTypeInfoCount();
}

// @pymethod <o PyITypeInfo>|PyITypeLib|GetTypeInfoOfGuid|Retrieves the type info of the specified GUID.
static PyObject *typelib_gettypeinfoofguid(PyObject *self, PyObject *args)
{
	PyObject *obguid;
	// @pyparm <o PyIID>|iid||GUID of the type description.
	if (!PyArg_ParseTuple(args, "O:GetTypeInfoOfGuid", &obguid))
		return NULL;
	GUID guid;
	if (!PyWinObject_AsIID(obguid, &guid))
		return NULL;
	return ((PyITypeLib*)self)->GetTypeInfoOfGuid(guid);
}

// @pymethod <o TYPEKIND>|PyITypeLib|GetTypeInfoType|Retrieves the type of a type description.
static PyObject *typelib_getinfotype(PyObject *self, PyObject *args)
{
	int pos;
	// @pyparm int|index||The index of the type description within the library
	if (!PyArg_ParseTuple(args, "i:GetTypeInfoType", &pos))
		return NULL;
	return ((PyITypeLib*)self)->GetTypeInfoType(pos);
}

// @object PyITypeLib|An object that implements the ITypeLib interface.
static struct PyMethodDef PyITypeLib_methods[] =
{
	{ "GetDocumentation", typelib_getdocs, 1 }, // @pymeth GetDocumentation|Retrieves documentation information about the library.
	{ "GetLibAttr",       typelib_getattr, 1 }, // @pymeth GetLibAttr|Retrieves the libraries attributes
	{ "GetTypeInfo",      typelib_getinfo, 1 }, // @pymeth GetTypeInfo|Retrieves the specified type description in the library.
	{ "GetTypeInfoCount", typelib_getinfocnt, 1 }, // @pymeth GetTypeInfoCount|Retrieves the number of <o PyITypeInfo>s in the type library.
	{ "GetTypeInfoOfGuid",typelib_gettypeinfoofguid,1}, // @pymeth GetTypeInfoOfGuid|Retrieves the type info of the specified GUID.
	{ "GetTypeInfoType",  typelib_getinfotype, 1 }, // @pymeth GetTypeInfoType|Retrieves the type of a type description.
	{NULL,  NULL}          /* sentinel */
};

PyComTypeObject PyITypeLib::type("PyITypeLib",
				&PyIUnknown::type, // @base PyITypeLib|PyIUnknown
                 sizeof(PyITypeLib),
                 PyITypeLib_methods,
				 GET_PYCOM_CTOR(PyITypeLib));

// @pymethod <o PyITypeLib>|pythoncom|LoadTypeLib|Loads a registered type library.
PyObject *pythoncom_loadtypelib(PyObject *self, PyObject *args)
{
	PyObject *obName;
	// @pyparm string|libFileName||The path to the file containing the type information.
	if (!PyArg_ParseTuple(args, "O:LoadTypeLib", &obName))
		return NULL;

	BSTR bstrName;
	if (!PyWinObject_AsBstr(obName, &bstrName))
		return NULL;

	ITypeLib *ptl;
	PY_INTERFACE_PRECALL;
	SCODE sc = LoadTypeLib(bstrName, &ptl);
	PyWinObject_FreeBstr(bstrName);
	PY_INTERFACE_POSTCALL;
	if (FAILED(sc))
		return PyCom_BuildPyException(sc);

	return PyCom_PyObjectFromIUnknown(ptl, IID_ITypeLib);
}

// @pymethod <o PyITypeLib>|pythoncom|LoadRegTypeLib|Loads a registered type library.
PyObject *pythoncom_loadregtypelib(PyObject *self, PyObject *args)
{
	PyObject *obIID;
	int major, minor;
	LCID lcid = LOCALE_USER_DEFAULT;
	// @pyparm <o PyIID>|iid||The IID of the type library.
	// @pyparm int|versionMajor||The major version number of the library
	// @pyparm int|versionMinor||The minor version number of the library
	// @pyparm int|lcid|LOCALE_USER_DEFAULT|The locale ID to use.
	if (!PyArg_ParseTuple(args, "Oii|i:LoadRegTypeLib", &obIID, &major, &minor, &lcid))
		return NULL;

	CLSID clsid;
	if (!PyWinObject_AsIID(obIID, &clsid))
		return NULL;

	ITypeLib *ptl;
	PY_INTERFACE_PRECALL;
	SCODE sc = LoadRegTypeLib(clsid, major, minor, lcid, &ptl);
	PY_INTERFACE_POSTCALL;
	if (FAILED(sc))
		return PyCom_BuildPyException(sc);

	return PyCom_PyObjectFromIUnknown(ptl, IID_ITypeLib);
	// @comm LoadRegTypeLib compares the requested version numbers against those found in the system registry, and takes one of the following actions:<nl>
	// If one of the registered libraries exactly matches both the requested major and minor version numbers, then that type library is loaded. <nl>
	// If one or more registered type libraries exactly match the requested major version number, and has a greater minor version number than that requested, the one with the greatest minor version number is loaded. <nl>
	// If none of the registered type libraries exactly match the requested major version number (or if none of those that do exactly match the major version number also have a minor version number greater than or equal to the requested minor version number), then LoadRegTypeLib returns an error.
}

#ifndef MS_WINCE
// @pymethod <o PyUnicode>|pythoncom|QueryPathOfRegTypeLib|Retrieves the path of a registered type library.
PyObject *pythoncom_querypathofregtypelib(PyObject *self, PyObject *args)
{
	PyObject *obIID;
	int major, minor;
	LCID lcid = LOCALE_USER_DEFAULT;
	// @pyparm <o PyIID>|iid||The IID of the type library.
	// @pyparm int|versionMajor||The major version number of the library
	// @pyparm int|versionMinor||The minor version number of the library
	// @pyparm int|lcid|LOCALE_USER_DEFAULT|The locale ID to use.
	if (!PyArg_ParseTuple(args, "Oii|i",
		&obIID,
		&major,
		&minor,
		&lcid))
		return NULL;

	CLSID clsid;
	if (!PyWinObject_AsIID(obIID, &clsid))
		return NULL;

	BSTR result;
	PY_INTERFACE_PRECALL;
	HRESULT hr = QueryPathOfRegTypeLib(clsid, major, minor, lcid, &result);
	PY_INTERFACE_POSTCALL;
	if (FAILED(hr))
		return PyCom_BuildPyException(hr);
	return PyWinObject_FromBstr( result, TRUE );
}
#endif
/////////////////////////////////////////////////////////////////////////////
// class PyITypeComp

PyITypeComp::PyITypeComp(IUnknown *ti) :
	PyIUnknown(ti)
{
	ob_type = &type;
}

PyITypeComp::~PyITypeComp()
{
}

// @pymethod <o DESCKIND>|PyITypeComp|Bind|binds to a variable/type
static PyObject *typecomp_bind(PyObject *self, PyObject *args)
{
	PyObject *obS;
	int		w=0;
	// @pyparm string|szName||The name to bind to
	// @pyparm int|wflags||the bind flags
	if (!PyArg_ParseTuple(args, "O|i:Bind", &obS, &w))
		return NULL;
	BSTR bstrS;
	if (!PyWinObject_AsBstr(obS, &bstrS))
		return NULL;
	PyObject *rc = ((PyITypeComp*)self)->Bind(bstrS,w);
	PyWinObject_FreeBstr(bstrS);
	return rc;
}

// @object PyITypeComp|An object that implements the ITypeComp interface.
static struct PyMethodDef PyITypeComp_methods[] =
{
	{ "Bind", typecomp_bind, 1 }, // @pymeth bind|Retrieves specified binding description.
	{NULL,  NULL}          /* sentinel */
};

PyComTypeObject PyITypeComp::type("PyITypeComp",
				&PyIUnknown::type, // @base PyITypeLib|PyIUnknown
                 sizeof(PyITypeComp),
                 PyITypeComp_methods,
				 GET_PYCOM_CTOR(PyITypeComp));

static PyObject* ITypeCompBind( ITypeComp* pTC, OLECHAR* S, unsigned short w )
{
	ITypeInfo*		pI;
	DESCKIND  		DK;
	BINDPTR			BP;
	PyObject*		ret;
	unsigned long	hashval = 0;

	PY_INTERFACE_PRECALL;
#ifndef MS_WINCE
	// appears in the headers for CE, but wont link!?
	hashval = LHashValOfNameSys(SYS_WIN32,LOCALE_USER_DEFAULT,S);
#endif
	SCODE sc = pTC->Bind(S, hashval,w, &pI, &DK, &BP);
	PY_INTERFACE_POSTCALL;
	if (FAILED(sc))
		return PyCom_BuildPyException(sc);
	switch(DK){
		case DESCKIND_NONE:
			Py_INCREF(Py_None);
			ret = Py_None;
			break;
		case DESCKIND_FUNCDESC:
			ret = PyObject_FromFUNCDESC(BP.lpfuncdesc);
			break;
		case DESCKIND_VARDESC:
			ret = PyObject_FromVARDESC(BP.lpvardesc);
			break;
		case DESCKIND_TYPECOMP:
			ret = PyCom_PyObjectFromIUnknown(BP.lptcomp, IID_ITypeComp);
			break;
		case DESCKIND_IMPLICITAPPOBJ:
			ITypeComp* pTC2;
			pI->GetTypeComp(&pTC2);
			ret = Py_BuildValue("(OO)", PyObject_FromVARDESC(BP.lpvardesc), ITypeCompBind(pTC2,S,w));
			break;
		}
	return Py_BuildValue( "(iO)", (int)DK, ret );
}

PyObject *PyITypeComp::Bind(OLECHAR* s,unsigned short w)
{
	return ITypeCompBind(GetI(this),s,w);
}
