class CPythonDocTemplate;
//
// Document Template Object.
//

class PYW_EXPORT PyCDocTemplate : public PyCCmdTarget {
protected:
	virtual void cleanup();
	PyCDocTemplate();
	~PyCDocTemplate();
public:
	static CPythonDocTemplate *GetTemplate(PyObject *self);
	static BOOL RemoveDocTemplateFromApp( CDocTemplate *pTemplate );

	static PyObject *create(PyObject *self, PyObject *args);
	static PyObject *DoCreateDocHelper(PyObject *self, PyObject *args, CRuntimeClass *pClass, ui_type_CObject &new_type);
	static PyObject *DoCreateDoc(PyObject *self, PyObject *args);
	static PyObject *AddDocTemplate(PyObject *self, PyObject *args);
	static PyObject *SetDocStrings(PyObject *self, PyObject *args);
	static PyObject *SetContainerInfo(PyObject *self, PyObject *args);
	static PyObject *CreateNewFrame(PyObject *self, PyObject *args);
	static PyObject *OpenDocumentFile(PyObject *self, PyObject *args);
	static PyObject *GetResourceID(PyObject *self, PyObject *args);
	static PyObject *GetSharedMenu(PyObject *self, PyObject *args);
	static PyObject *GetDocumentList(PyObject *self, PyObject *args);
	static PyObject *FindOpenDocument(PyObject *self, PyObject *args);
	static PyObject *GetDocString(PyObject *self, PyObject *args);
	static PyObject *InitialUpdateFrame(PyObject *self, PyObject *args);

	static ui_type_CObject type;
	MAKE_PY_CTOR(PyCDocTemplate)
};

// The MFC derived class.
class PYW_EXPORT CPythonDocTemplate : public CMultiDocTemplate {
friend class PyCDocTemplate;
public:
	CPythonDocTemplate(UINT idResource);
	virtual ~CPythonDocTemplate();
	virtual CDocument* CreateNewDocument();
	virtual CDocument* OpenDocumentFile(LPCTSTR lpszPathName, BOOL bMakeVisible = TRUE );

	virtual CFrameWnd* CreateNewFrame( CDocument* pDoc, CFrameWnd* pOther );
	virtual void InitialUpdateFrame( CFrameWnd* pFrame, CDocument* pDoc, BOOL bMakeVisible = TRUE);
#ifndef _MAC
	virtual CDocTemplate::Confidence MatchDocType(LPCTSTR lpszPathName,CDocument*& rpDocMatch);
#else
	virtual CDocTemplate::Confidence MatchDocType(LPCTSTR lpszPathName,DWORD dwFileType, CDocument*& rpDocMatch);
#endif
	UINT GetResourceID() {return m_nIDResource;}
};
