/*

	win32 assoc object

	Created August 1994, Mark Hammond (MHammond@skippinet.com.au)

Note that this source file contains embedded documentation.
This documentation consists of marked up text inside the
C comments, and is prefixed with an '@' symbol.  The source
files are processed by a tool called "autoduck" which
generates Windows .hlp files.
@doc

*/
#include "stdafx.h"

CAssocManager ui_assoc_object::handleMgr;

CAssocManager::CAssocManager()
{
	lastLookup = NULL;
	lastObject = NULL;
#ifdef _DEBUG
	cacheLookups = cacheHits = 0;
#endif
}
CAssocManager::~CAssocManager()
{
#ifdef _DEBUG
	char buf[256];
	if (cacheLookups) {
		// cant use TRACE, as CWinApp may no longer be valid.
		wsprintf(buf, "AssocManager cache hit ratio is %d percent\n", cacheHits * 100 / cacheLookups);
		OutputDebugString(buf);
	}
#endif
}

//
// CAssocManager::cleanup
//
// This should never detect objects.
void CAssocManager::cleanup(void)
{
	POSITION pos;
	ui_assoc_object *ob;
	void *assoc;
	ASSERT_VALID(&map);
	m_critsec.Lock();
	for(pos=map.GetStartPosition();pos;) {
		map.GetNextAssoc(pos, (void *&)assoc, (void *&)ob);
		ob->cleanup();
		// not sure if I should do this!!
		//PyMem_DEL(ob);
	}
	m_critsec.Unlock();
}
void CAssocManager::Assoc(void *handle, ui_assoc_object *object, void *oldHandle)
{
	m_critsec.Lock();
	if (oldHandle) {
		// if window previously closed, this may fail when the Python object
		// destructs - but this is not a problem.
		map.RemoveKey(oldHandle);
		if (oldHandle==lastLookup)
			lastLookup = 0;	// set cache invalid.
	}
	if (handle)
		map.SetAt(handle, object);
	if (handle==lastLookup)
		lastObject = object;
	m_critsec.Unlock();
}

//
// CAssocManager::GetAssocObject
//
ui_assoc_object *CAssocManager::GetAssocObject(const void * handle)
{
	if (handle==NULL) return NULL; // no possible association for NULL!
	ui_assoc_object *ret;
	m_critsec.Lock();
#ifdef _DEBUG
	cacheLookups++;
#endif
	// implement a basic 1 item cache.
	if (lastLookup==handle) {
		ret = lastObject;
#ifdef _DEBUG
		++cacheHits;
#endif
	}
	else {
		if (!map.Lookup((void *)handle, (void *&)ret))
			ret = NULL;
		lastLookup = handle;
		lastObject = ret;
	}
	m_critsec.Unlock();
	return ret;
}

/*static*/void *ui_assoc_object::GetGoodCppObject(PyObject *&self, ui_type *ui_type_check)
{
	// first, call is_uiobject, which may modify the "self" pointer.
	// this is to support a Python class instance being passed in,
	// and auto-convert it to the classes AttachedObject.
	if (ui_type_check && !is_uiobject(self, ui_type_check)) {
		CString csRet = "object is not a ";
		csRet += ui_type_check->tp_name;
		TRACE("GetGoodCppObject fails RTTI\n");
		const char *ret = csRet;
		RETURN_TYPE_ERR((char *)ret);
	}
	ui_assoc_object *s = (ui_assoc_object *)self;
	if (s->assoc==NULL)
		RETURN_ERR("The object has been destroyed.");
#ifdef _DEBUG
	// These sorts of errors are C developers problems, and
	// should not be possible to be triggered by Python.
	// Therefore we restrict the type checking code to debug
	if (!s->CheckCppObject(ui_type_check))
		return NULL;
#endif // _DEBUG
	return s->assoc;
}

void *ui_assoc_object::GetGoodCppObject(ui_type *ui_type_check) const
{
	// Get a checked association.
	PyObject *temp = (PyObject *)this;
	void *ret = GetGoodCppObject(temp, ui_type_check);
	ASSERT(this==(ui_assoc_object *)temp); // Called with this->, and this needs to be changed!
	return ret;
}

bool ui_assoc_CObject::CheckCppObject(ui_type *ui_type_check) const
{
	if (!ui_assoc_object::CheckCppObject(ui_type_check)) return false;
	CObject *pObj = (CObject *)assoc;
// Assert triggers occasionally for brand new window objects - 
// Removing this ASSERT cant hurt too much (as I have never seen it
// fire legitimately
//	ASSERT_VALID(pObj); // NULL has already been handled before now.
	if (ui_type_check==NULL) return true; // Cant check anything!

	ui_type_CObject *pTyp = (ui_type_CObject *)ui_type_check;
	if (pTyp->pCObjectClass==NULL) {
		// Type must be intermediate - ie, has child classes that
		// all objects should be one of.  This may indicate we are
		// missing a child type (eg, a CommonDialog derived class)
		RETURN_ERR("Internal error - attempt to create an object of an abstract class");
	}
	if (!pObj->IsKindOf(pTyp->pCObjectClass)) {
		TRACE2("ui_assoc_CObject::GetGoodCppObject fails due to RTTI - looking for %s, got %s\n", pTyp->pCObjectClass->m_lpszClassName, pObj->GetRuntimeClass()->m_lpszClassName);
		RETURN_ERR("Internal error - C++ RTTI failed");
	}
	return true;
}

// @pymethod |PyAssocObject|AttachObject|Attaches a Python object for lookup of "virtual" functions.
PyObject *
ui_assoc_object::AttachObject(PyObject *self, PyObject *args)
{
	PyObject *ob;
	ui_assoc_object *pAssoc = (ui_assoc_object *)self;
	if (pAssoc==NULL) return NULL;
	if (!PyArg_ParseTuple(args, "O:AttachObject", &ob ))
		return NULL;
	// Possibility for recursion here if we re-attach the
	// same instance to the same win32ui type object.
	// decref of the instance may trigger instance delete,
	// which may trigger AttachObject(None), which will
	// attempt to decref etc.  
	// So set the instance to NULL _before_ we decref it!
	PyObject *old = pAssoc->virtualInst;
	pAssoc->virtualInst = NULL;
	XDODECREF(old);
	if (ob!=Py_None) {
		pAssoc->virtualInst = ob;
		DOINCREF(ob);
	}
	RETURN_NONE;
}

// @object PyAssocObject|An internal class.
static struct PyMethodDef PyAssocObject_methods[] = {
	{"AttachObject",    ui_assoc_object::AttachObject, 1 }, // @pymeth AttachObject|Attaches a Python object for lookup of "virtual" functions.
	{NULL, NULL}
};

ui_type ui_assoc_object::type("(abstract) PyAssocObject", 
							  &ui_base_class::type, 
							  sizeof(ui_assoc_object), 
							  PyAssocObject_methods, 
							  NULL);

ui_assoc_object::ui_assoc_object()
{
	assoc=0;
	virtualInst=NULL;
}
ui_assoc_object::~ui_assoc_object()
{
	KillAssoc();
}

// handle is invalid - therefore release all refs I am holding for it.
// ASSUMES WE HOLD THE PYTHON LOCK as for all Python object destruction.
void ui_assoc_object::KillAssoc()
{
#ifdef TRACE_ASSOC
	CString rep = repr();
	const char *szRep = rep;
	TRACE("Destroying association with %p and %s",this,szRep);
#endif
	// note that _any_ of these may cause this to be deleted, as the reference
	// count may drop to zero.  If any one dies, and later ones will fail.  Therefore
	// I incref first, and decref at the end.
	// Note that this _always_ recurses when this happens as the destructor also
	// calls us to cleanup.  Forcing an INCREF/DODECREF in that situation causes death
	// by recursion, as each dec back to zero causes a delete.
	BOOL bDestructing = ob_refcnt==0;
	if (!bDestructing)
		Py_INCREF(this);
	DoKillAssoc(bDestructing);	// kill all map entries, etc.
	SetAssocInvalid();			// let child do whatever to detect
	if (!bDestructing)
		DODECREF(this);
}
// the virtual version...
// ASSUMES WE HOLD THE PYTHON LOCK as for all Python object destruction.
void ui_assoc_object::DoKillAssoc( BOOL bDestructing /*= FALSE*/ )
{
	// In Python debug builds, this can get recursive -
	// Python temporarily increments the refcount of the dieing
	// object - this object death will attempt to use the dieing object.
	PyObject *vi = virtualInst;
	virtualInst = NULL;
	Py_XDECREF(vi);
//	virtuals.DeleteAll();
	handleMgr.Assoc(0,this,assoc);
}

// return an object, given an association, if we have one.
/* static */ ui_assoc_object *ui_assoc_object::GetPyObject(void *search)
{
	return (ui_assoc_object *)handleMgr.GetAssocObject(search);
}

PyObject *ui_assoc_object::GetGoodRet()
{
	if (this==NULL) return NULL;
	if (virtualInst) {
		DODECREF(this);
		DOINCREF(virtualInst);
		return virtualInst;
	} else
		return this;
}

/*static*/ ui_assoc_object *ui_assoc_object::make( ui_type &makeType, void *search, bool skipLookup )
{
	ASSERT(search); // really only a C++ problem.
	ui_assoc_object* ret=NULL;
	if (!skipLookup)
		ret = (ui_assoc_object*) handleMgr.GetAssocObject(search);
	if (ret) {
		if (!ret->is_uiobject(&makeType))
			RETURN_ERR("Internal error - existing object is not of same type as requested new object");
		DOINCREF( ret );
		return ret;
	}
	ret = (ui_assoc_object*) ui_base_class::make( makeType );	// may fail if unknown class.
	if (ret) {
		ASSERT(ret->ob_type == &makeType); // Created object must be of the type we expect.
		// do NOT keep a reference to the Python object, or it will
		// remain forever.  The destructor must remove itself from the map.
#ifdef TRACE_ASSOC
		TRACE_ASSOC ("  Associating 0x%x with 0x%x", search, ret);
#endif
		// if I have an existing handle, remove it.
		handleMgr.Assoc(search, ret,NULL);
		ret->assoc = search;
	}
	return ret;
}

CString ui_assoc_object::repr()
{
	CString csRet;
	char *buf = csRet.GetBuffer(128);
	PyObject *vi_repr = virtualInst ? PyObject_Repr(virtualInst) : NULL;
	sprintf(buf, " - assoc is %p, vi=%s", assoc, vi_repr ? PyString_AsString(vi_repr) : "<None>" );
	csRet.ReleaseBuffer();
	Py_XDECREF(vi_repr);
	return ui_base_class::repr() + csRet;
}
#ifdef _DEBUG
void ui_assoc_object::Dump( CDumpContext &dc ) const
{
	ui_base_class::Dump(dc);
	dc << "assoc=" << assoc;
}
#endif

/*int ui_assoc_object::setattr(char *name, PyObject *v)
{
	// v may be NULL or None.
	return virtuals.AddVirtualHandler(name, v);
}*/

// @object PyAssocCObject|An internal class.
static struct PyMethodDef PyAssocCObject_methods[] = {
	{NULL, NULL}
};

ui_type_CObject ui_assoc_CObject::type("PyAssocCObject", 
									   &ui_assoc_object::type, 
									   RUNTIME_CLASS(CObject), 
									   sizeof(ui_assoc_CObject), 
									   PyAssocCObject_methods, 
									   NULL);

ui_assoc_CObject::ui_assoc_CObject()
{
	bManualDelete = FALSE;	// default not explicit delete on object.
}

ui_assoc_CObject::~ui_assoc_CObject()
{
	if (bManualDelete) {
		bManualDelete = FALSE;
		CObject *pO = (CObject *)GetGoodCppObject(&type);	// get pointer before killing it.
		KillAssoc(); // stop recursion - disassociate now.
		if (!pO)
			PyErr_Clear();
		else
			delete pO;
	}
}
#ifdef _DEBUG

void ui_assoc_CObject::Dump( CDumpContext &dc ) const
{
	// skip over ui_assoc, as we print the assoc in a much better format!
	ui_base_class::Dump(dc);
#if !defined(_MAC) && !defined(_AFX_PORTABLE)
	// use SEH (structured exception handling) to catch even GPFs
	//  that result from partially valid objects.
	try
#endif
	{
		CObject *pOb = (CObject *)GetGoodCppObject(NULL);
		dc << ", CObject is ";
		if (pOb) {
			if (AfxIsValidAddress(pOb, sizeof(CObject)))
				pOb->Dump(dc);
			else
				afxDump << "<at invalid address!>";
		}
		else
			dc << "<NULL>";
	}
#if !defined(_MAC) && !defined(_AFX_PORTABLE)
		catch(int code) {
			// short form for trashed objects
			afxDump << "<Bad! (" << code << ")>";
		}
		catch(...) {
			// short form for trashed objects
			afxDump << "<Bad!>";
		}
#endif
}
#endif
