/* win32ctrllist : header

	List control object.  

	Created May 1996, Mark Hammond (MHammond@skippinet.com.au)

Note that this source file contains embedded documentation.
This documentation consists of marked up text inside the
C comments, and is prefixed with an '@' symbol.  The source
files are processed by a tool called "autoduck" which
generates Windows .hlp files.
@doc

*/
///////////////////////////////////////////////////////////////////////
// Control objects.
//
// PyCListCtrl
//
class PyCListCtrl : public PyCWnd {

public:
	static ui_type_CObject type;
	MAKE_PY_CTOR(PyCListCtrl)

protected:
	PyCListCtrl();
	virtual ~PyCListCtrl();
};

class PythonImageList : public CImageList
{
public:
	PythonImageList();
	~PythonImageList();
#ifdef _DEBUG
	virtual void Dump( CDumpContext &dc ) const;
#endif
};

///////////////////////////////////////////////////////////////////////
// ImageList
//

class PYW_EXPORT PyCImageList : public ui_assoc_CObject{
public:
	MAKE_PY_CTOR(PyCImageList)
	static CImageList *GetImageList(PyObject *self);
	static ui_type_CObject type;
protected:
	PyCImageList();
	virtual ~PyCImageList();
};
