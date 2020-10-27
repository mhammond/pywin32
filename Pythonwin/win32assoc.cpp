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

#ifdef DEBUG
#define ASSERT_GIL_HELD                           \
    {                                             \
        PyGILState_STATE s = PyGILState_Ensure(); \
        ASSERT(s == PyGILState_LOCKED);           \
        PyGILState_Release(s);                    \
    }
#else
#define ASSERT_GIL_HELD
#endif

CAssocManager ui_assoc_object::handleMgr;

CAssocManager::CAssocManager()
{
    lastLookup = NULL;
    lastObjectWeakRef = NULL;
#ifdef _DEBUG
    cacheLookups = cacheHits = 0;
#endif
}
CAssocManager::~CAssocManager()
{
#ifdef _DEBUG
    TCHAR buf[256];
    if (cacheLookups) {
        // cant use TRACE, as CWinApp may no longer be valid.
        wsprintf(buf, _T("AssocManager cache hit ratio is %d percent\n"), cacheHits * 100 / cacheLookups);
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
    TRACE("CAssocManager cleaning up %d objects\n", map.GetCount());
    CEnterLeavePython _celp;
    for (pos = map.GetStartPosition(); pos;) {
        map.GetNextAssoc(pos, (void *&)assoc, (void *&)ob);
        RemoveAssoc(assoc);
    }
}

void CAssocManager::RemoveAssoc(void *handle)
{
    // nuke any existing items.
    PyObject *weakref;
    if (map.Lookup(handle, (void *&)weakref)) {
        PyObject *ob = PyWeakref_GetObject(weakref);
        map.RemoveKey(handle);
        if (ob != Py_None)
            // The object isn't necessarily dead (ie, its refcount may
            // not be about to hit zero), but its 'dead' from our POV, so
            // let it free any MFC etc resources the object owns.
            // XXX - this kinda sucks - just relying on the object
            // destructor *should* be OK...
            ((ui_assoc_object *)ob)->cleanup();
        Py_DECREF(weakref);
    }
    lastObjectWeakRef = 0;
    lastLookup = 0;  // set cache invalid.
}

void CAssocManager::Assoc(void *handle, ui_assoc_object *object)
{
    ASSERT_GIL_HELD;  // we rely on the GIL to serialize access to our map...
    ASSERT(handle);
#ifdef DEBUG
    // overwriting an existing entry probably means we are failing to
    // detect the death of the old object and its address has been reused.
    if (object) {  // might just be nuking ours, so we expect to find outself!
        PyObject *existing_weakref;
        if (map.Lookup(handle, (void *&)existing_weakref)) {
            TRACE("CAssocManager::Assoc overwriting existing assoc\n");
            DebugBreak();
        }
    }
#endif
    RemoveAssoc(handle);
    if (object) {
        PyObject *weakref = PyWeakref_NewRef(object, NULL);
        if (weakref)
            // reference owned by the map.
            map.SetAt(handle, weakref);
        else {
            TRACE("Failed to create weakref\n");
            gui_print_error();
            DebugBreak();
        }
    }
}

//
// CAssocManager::GetAssocObject
// Returns an object *with a new reference*.  NULL is not an error return - it just means "no object"
ui_assoc_object *CAssocManager::GetAssocObject(void *handle)
{
    if (handle == NULL)
        return NULL;  // no possible association for NULL!
    ASSERT_GIL_HELD;  // we rely on the GIL to serialize access to our map...
    PyObject *weakref;
#ifdef _DEBUG
    cacheLookups++;
#endif
    // implement a basic 1 item cache.
    if (lastLookup == handle) {
        weakref = lastObjectWeakRef;
#ifdef _DEBUG
        ++cacheHits;
#endif
    }
    else {
        if (!map.Lookup((void *)handle, (void *&)weakref))
            weakref = NULL;
        lastLookup = handle;
        lastObjectWeakRef = weakref;
    }
    if (weakref == NULL)
        return NULL;
    // convert the weakref object into a real object.
    PyObject *ob = PyWeakref_GetObject(weakref);
    if (ob == NULL) {
        // an error - but a NULL return from us just means "no assoc"
        // so print the error and ignore it, treating it as if the
        // weak-ref target has died.
        gui_print_error();
        ob = Py_None;
    }
    ui_assoc_object *ret;
    if (ob == Py_None) {
        // weak-ref target has died.  Remove it from the map.
        Assoc(handle, NULL);
        ret = NULL;
    }
    else {
        ret = (ui_assoc_object *)ob;
        Py_INCREF(ret);
    }
    return ret;
}

/*static*/ void *ui_assoc_object::GetGoodCppObject(PyObject *&self, ui_type *ui_type_check)
{
    // first, call is_uiobject, which may modify the "self" pointer.
    // this is to support a Python class instance being passed in,
    // and auto-convert it to the classes AttachedObject.
    if (ui_type_check && !is_uiobject(self, ui_type_check)) {
        TRACE("GetGoodCppObject fails RTTI\n");
        return PyErr_Format(PyExc_TypeError, "object is not a %s", ui_type_check->tp_name);
    }
    ui_assoc_object *s = (ui_assoc_object *)self;
    if (s->assoc == NULL)
        RETURN_ERR("The object has been destroyed.");
#ifdef _DEBUG
    // These sorts of errors are C developers problems, and
    // should not be possible to be triggered by Python.
    // Therefore we restrict the type checking code to debug
    if (!s->CheckCppObject(ui_type_check))
        return NULL;
#endif  // _DEBUG
    return s->assoc;
}

void *ui_assoc_object::GetGoodCppObject(ui_type *ui_type_check) const
{
    // Get a checked association.
    PyObject *temp = (PyObject *)this;
    void *ret = GetGoodCppObject(temp, ui_type_check);
    ASSERT(this == (ui_assoc_object *)temp);  // Called with this->, and this needs to be changed!
    return ret;
}

bool ui_assoc_CObject::CheckCppObject(ui_type *ui_type_check) const
{
    if (!ui_assoc_object::CheckCppObject(ui_type_check))
        return false;
    CObject *pObj = (CObject *)assoc;
    // Assert triggers occasionally for brand new window objects -
    // Removing this ASSERT cant hurt too much (as I have never seen it
    // fire legitimately
    //	ASSERT_VALID(pObj); // NULL has already been handled before now.
    if (ui_type_check == NULL)
        return true;  // Cant check anything!

    ui_type_CObject *pTyp = (ui_type_CObject *)ui_type_check;
    if (pTyp->pCObjectClass == NULL) {
        // Type must be intermediate - ie, has child classes that
        // all objects should be one of.  This may indicate we are
        // missing a child type (eg, a CommonDialog derived class)
        RETURN_ERR("Internal error - attempt to create an object of an abstract class");
    }
    if (!pObj->IsKindOf(pTyp->pCObjectClass)) {
        TRACE2("ui_assoc_CObject::GetGoodCppObject fails due to RTTI - looking for %s, got %s\n",
               pTyp->pCObjectClass->m_lpszClassName, pObj->GetRuntimeClass()->m_lpszClassName);
        RETURN_ERR("Internal error - C++ RTTI failed");
    }
    return true;
}

// @pymethod |PyAssocObject|AttachObject|Attaches a Python object for lookup of "virtual" functions.
PyObject *ui_assoc_object::AttachObject(PyObject *self, PyObject *args)
{
    PyObject *ob;
    ui_assoc_object *pAssoc = (ui_assoc_object *)self;
    if (pAssoc == NULL)
        return NULL;
    if (!PyArg_ParseTuple(args, "O:AttachObject", &ob))
        return NULL;
    // Possibility for recursion here if we re-attach the
    // same instance to the same win32ui type object.
    // decref of the instance may trigger instance delete,
    // which may trigger AttachObject(None), which will
    // attempt to decref etc.
    // So set the instance to NULL _before_ we decref it, and only
    // do the decref after we've incref'd the new object - if it is the
    // same object we may otherwise transition it via a refcount of 0.
    PyObject *old = pAssoc->virtualInst;
    pAssoc->virtualInst = NULL;
    if (ob != Py_None) {
        pAssoc->virtualInst = ob;
        DOINCREF(ob);
    }
    XDODECREF(old);
    RETURN_NONE;
}

// @pymethod object|PyAssocObject|GetAttachedObject|Returned the attached Python object, or None.
PyObject *ui_assoc_object::GetAttachedObject(PyObject *self, PyObject *args)
{
    ui_assoc_object *pAssoc = (ui_assoc_object *)self;
    if (pAssoc == NULL)
        return NULL;
    if (!PyArg_ParseTuple(args, ":GetAttachedObject"))
        return NULL;
    PyObject *ob = pAssoc->virtualInst;
    if (!ob)
        ob = Py_None;
    Py_INCREF(ob);
    return ob;
}

// @object PyAssocObject|An internal class.
static struct PyMethodDef PyAssocObject_methods[] = {
    {"AttachObject", ui_assoc_object::AttachObject,
     1},  // @pymeth AttachObject|Attaches a Python object for lookup of "virtual" functions.
    {"GetAttachedObject", ui_assoc_object::GetAttachedObject,
     1},  // @pymeth GetAttachedObject|Returned the attached Python object, or None.
    {NULL, NULL}};

ui_type ui_assoc_object::type("(abstract) PyAssocObject", &ui_base_class::type, sizeof(ui_assoc_object),
                              PYOBJ_OFFSET(ui_assoc_object), PyAssocObject_methods, NULL);

ui_assoc_object::ui_assoc_object()
{
    assoc = 0;
    virtualInst = NULL;
}
ui_assoc_object::~ui_assoc_object()
{
#ifdef TRACE_ASSOC
    CString rep = repr();
    const char *szRep = rep;
    TRACE("Destroying association with %p and %s", this, szRep);
#endif
    Py_CLEAR(virtualInst);
    //	virtuals.DeleteAll();
    if (assoc) {
        handleMgr.Assoc(assoc, 0);
        SetAssocInvalid();  // let child do whatever to detect
    }
}

PyObject *ui_assoc_object::GetGoodRet()
{
    if (this == NULL)
        return NULL;
    if (virtualInst) {
        PyObject *vi = virtualInst;
        DOINCREF(vi);
        DODECREF(this);
        return vi;
    }
    else
        return this;
}

/*static*/ ui_assoc_object *ui_assoc_object::make(ui_type &makeType, void *search, bool skipLookup)
{
    ASSERT(search);  // really only a C++ problem.
    CEnterLeavePython _celp;
    ui_assoc_object *ret = NULL;
    if (!skipLookup)
        ret = (ui_assoc_object *)handleMgr.GetAssocObject(search);
    if (ret) {
        if (!ret->is_uiobject(&makeType)) {
            PyErr_Format(ui_module_error, "Internal error - existing object has type '%s', but '%s' was requested.",
                         ret->ob_type->tp_name, makeType.tp_name);
            return NULL;
        }
        return ret;
    }
    ret = (ui_assoc_object *)ui_base_class::make(makeType);  // may fail if unknown class.
    if (ret) {
        ASSERT(ret->ob_type == &makeType);  // Created object must be of the type we expect.
                                            // do NOT keep a reference to the Python object, or it will
                                            // remain forever.  The destructor must remove itself from the map.
#ifdef TRACE_ASSOC
        TRACE_ASSOC("  Associating 0x%x with 0x%x", search, ret);
#endif
        handleMgr.Assoc(search, ret);
        ret->assoc = search;
    }
    return ret;
}

CString ui_assoc_object::repr()
{
    CString csRet;
    static TCHAR *no_repr = _T("<None>");
    TCHAR *py_repr = NULL;
    BOOL bfree_repr = FALSE;

    if (virtualInst == NULL)
        py_repr = no_repr;
    else {
        PyObject *vi_repr = PyObject_Str(virtualInst);
        if (vi_repr == NULL || !PyWinObject_AsTCHAR(vi_repr, &py_repr, FALSE)) {
            PyErr_Clear();
            py_repr = no_repr;
        }
        else
            bfree_repr = TRUE;
        Py_XDECREF(vi_repr);
    }
    csRet.Format(_T(" - assoc is %p, vi=%s"), assoc, py_repr);
    if (bfree_repr)
        PyWinObject_FreeTCHAR(py_repr);
    return ui_base_class::repr() + csRet;
}

#ifdef _DEBUG
void ui_assoc_object::Dump(CDumpContext &dc) const
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
static struct PyMethodDef PyAssocCObject_methods[] = {{NULL, NULL}};

ui_type_CObject ui_assoc_CObject::type("PyAssocCObject", &ui_assoc_object::type, RUNTIME_CLASS(CObject),
                                       sizeof(ui_assoc_CObject), PYOBJ_OFFSET(ui_assoc_CObject), PyAssocCObject_methods,
                                       NULL);

ui_assoc_CObject::ui_assoc_CObject()
{
    bManualDelete = FALSE;  // default not explicit delete on object.
}

ui_assoc_CObject::~ui_assoc_CObject()
{
    if (bManualDelete) {
        bManualDelete = FALSE;
        CObject *pO = (CObject *)GetGoodCppObject(&type);  // get pointer before killing it.
        ASSERT(!PyErr_Occurred());                         // PyErr_Clear() is bogus?????
        if (!pO)
            PyErr_Clear();
        else
            delete pO;
    }
}
#ifdef _DEBUG

void ui_assoc_CObject::Dump(CDumpContext &dc) const
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
    catch (int code) {
        // short form for trashed objects
        afxDump << "<Bad! (" << code << ")>";
    }
    catch (...) {
        // short form for trashed objects
        afxDump << "<Bad!>";
    }
#endif
}
#endif
