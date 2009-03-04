//
// Association classes.
//
#pragma once
// afxmt.h is often not included by default in stdafx.h
// Try and include it here - it wont hurt if stfafx.h has already done it!
#include <afxmt.h>          // CCriticalSection, etc

// Handle Manager maps between pointers of some sort, and an associated
// Python objects.

typedef void *ASSOC_CPP;

class ui_assoc_object;
////////////////////

class CAssocManager 
#ifdef _DEBUG
						: public CObject	// CObject for diag only.
#endif
{
public:
	CAssocManager();
	~CAssocManager();
	void Assoc(void *assoc, ui_assoc_object *PyObject);
	ui_assoc_object *GetAssocObject(void * handle);

	void cleanup(void);	// only to be called at the _very_ end
private:
	void RemoveAssoc(void *handle);
	// A "map" of weak-references to Python objects.  Now we use weakrefs
	// this really should be a regular Python dict...
	CMapPtrToPtr map;
	const void *lastLookup;
	PyObject *lastObjectWeakRef;
#ifdef _DEBUG
	int cacheLookups;
	int cacheHits;
#endif
};

//
// ui_assoc_object 
//
class PYW_EXPORT ui_assoc_object : public ui_base_class{
public:	// some probably shouldnt be, but...
	PyObject *GetGoodRet();
	static ui_assoc_object *make( ui_type &makeType, void * search, bool skipLookup=false );

	// Given a C++ object, return a PyObject associated (map lookup)
	static ui_assoc_object *GetAssocObject(void *search) {
		return ui_assoc_object::handleMgr.GetAssocObject(search);
	}

	// Return the C++ object associated with this Python object.
	// Do as much type checking as possible.
	// Static version may have "self" pointer changed if it does
	// auto conversion from Instance to Object.
	static void *GetGoodCppObject(PyObject *&self, ui_type *ui_type_check);

	// virtuals for Python support
	virtual CString repr();

	// methods
	static PyObject *AttachObject(PyObject *self, PyObject *args);
	static PyObject *GetAttachedObject(PyObject *self, PyObject *args);

	PyObject *virtualInst;

	static ui_type type;
	static CAssocManager handleMgr;
#ifdef _DEBUG
	virtual void Dump( CDumpContext &dc ) const;
#endif
protected:
	void *GetGoodCppObject(ui_type *ui_type_check=NULL) const;
	virtual bool CheckCppObject(ui_type *ui_type_check) const {return true;}
	virtual void SetAssocInvalid() { assoc = 0; } // XXX - bogus - called during destruction???

	ui_assoc_object(); // ctor/dtor
	virtual ~ui_assoc_object();
	void *assoc;
};

class PYW_EXPORT ui_assoc_CObject : public ui_assoc_object {
	// create an object
public:
	static ui_type_CObject type;
#ifdef _DEBUG
	virtual void Dump( CDumpContext &dc ) const;
#endif
	BOOL bManualDelete; // set to TRUE if the C++ object should be deleted when finished.
protected:
	ui_assoc_CObject();
	~ui_assoc_CObject();
	// Perform some basic type checking
	virtual bool CheckCppObject(ui_type *ui_type_check) const;
};
