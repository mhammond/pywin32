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
	void Assoc(void *assoc, ui_assoc_object *PyObject, void *oldAssoc=NULL);
	ui_assoc_object *GetAssocObject(const void * handle);

	void cleanup(void);	// only to be called at the _very_ end
private:
	CMapPtrToPtr map;
	const void *lastLookup;
	ui_assoc_object *lastObject;
	CCriticalSection m_critsec;
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
	static ui_assoc_object *make( ui_type &makeType, void * search );

	// Given a C++ object, return a PyObject associated (map lookup)
	static ui_assoc_object *GetPyObject(void *search);

	// Return the C++ object associated with this Python object.
	// Do as much type checking as possible.
	// Static version may have "self" pointer changed if it does
	// auto conversion from Instance to Object.
	static void *GetGoodCppObject(PyObject *&self, ui_type *ui_type_check);

	// Call this when the C++ object dies, or otherwise becomes invalid.
	void KillAssoc();	// maps to a virtual with some protection wrapping.

	// virtuals for Python support
	virtual CString repr();

	// methods
	static PyObject *AttachObject(PyObject *self, PyObject *args);

	PyObject *virtualInst;

	static ui_type type;
	static CAssocManager handleMgr;
#ifdef _DEBUG
	virtual void Dump( CDumpContext &dc ) const;
#endif
protected:
	void *GetGoodCppObject(ui_type *ui_type_check=NULL) const;
	virtual bool CheckCppObject(ui_type *ui_type_check) const {return true;}
	// Does the actual killing.
	virtual void DoKillAssoc( BOOL bDestructing = FALSE ); // does the actual work.
	// Called during KillAssoc - normally zeroes association.
	// Override to keep handle after destruction (eg, the association
	// with a dialog is valid after the Window's window has closed).
	virtual void SetAssocInvalid() { assoc = 0; }

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
