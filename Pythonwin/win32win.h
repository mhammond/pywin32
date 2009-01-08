// win32win.h - support for Python/MFC Windows objects.

//
// Window Objects
//
class PYW_EXPORT PyCWnd : public PyCCmdTarget{
public:	// probably shouldnt be, but...
	MAKE_PY_CTOR(PyCWnd)
	CMapWordToPtr *pMessageHookList;
	CMapWordToPtr *pKeyHookList;
	PyObject      *obKeyStrokeHandler;
	BOOL bDidSubclass;

	// ensure that the pointer is permanent
	static PyCWnd *make( ui_type_CObject &makeType, CWnd *pSearch, HWND wnd=NULL );
protected:
	PyCWnd();
	~PyCWnd();
public:
	static CWnd *GetPythonGenericWnd(PyObject *self, ui_type_CObject *pType = &type);
	static PyObject *get_window(PyObject *self, PyObject *args);
	static PyObject *get_top_window(PyObject *self, PyObject *args);

	static PyObject *FindWindow(PyObject *self, PyObject *args);
	static PyObject *FindWindowEx(PyObject *self, PyObject *args);
	static PyObject *CreateWindowFromHandle(PyObject *self, PyObject *args);
	static PyObject *CreateControl(PyObject *self, PyObject *args);
	static PyObject *GetActiveWindow(PyObject *self, PyObject *args);
	static PyObject *GetForegroundWindow(PyObject *self, PyObject *args);
	static PyObject *GetFocus(PyObject *self, PyObject *args);


	// virtuals for Python support
	virtual CString repr();

	BOOL check_key_stroke(WPARAM ch);

	static ui_type_CObject type;
};

//
// views
//
class PYW_EXPORT PyCView : public PyCWnd {
protected:
	PyCView() {}
	~PyCView(){}
public:
	static CView *GetViewPtr(PyObject *self);
	static ui_type_CObject type;
};

class PYW_EXPORT PyCScrollView : public PyCView {
protected:
	PyCScrollView() {}
	~PyCScrollView(){}
public:
	static PyObject *create(PyObject *self, PyObject *args);
	static CScrollView *GetViewPtr(PyObject *self);
	static ui_type_CObject type;
	MAKE_PY_CTOR(PyCScrollView)
};

class PYW_EXPORT PyCFormView : public PyCView {
protected:
public:
	static PyObject *create(PyObject *self, PyObject *args);
	static ui_type_CObject type;
	MAKE_PY_CTOR(PyCFormView)
};

//
// frame windows
//
class PYW_EXPORT PyCFrameWnd : public PyCWnd {
protected:
	PyCFrameWnd() {}
	~PyCFrameWnd(){}
public:
	MAKE_PY_CTOR(PyCFrameWnd)
	static ui_type_CObject type;
};

class PYW_EXPORT PyCMDIFrameWnd : public PyCFrameWnd {
protected:
	PyCMDIFrameWnd() {return;}
public:
	MAKE_PY_CTOR(PyCMDIFrameWnd)
	static ui_type_CObject type;
};

class PYW_EXPORT PyCMDIChildWnd : public PyCFrameWnd {
protected:
	PyCMDIChildWnd() {return;}
public:
	MAKE_PY_CTOR(PyCMDIChildWnd)
//	static PyObject *create(PyObject *self, PyObject *args);
	static ui_type_CObject type;
};

extern PYW_EXPORT CWnd *GetWndPtr(PyObject *);

/////////////////////////////////////////////////////////
//
// Additional helpers
//
// Allow objects to simply provide OnCommand to call the
// base handler without recursing to death.
template <class ClassFramework>
PyObject *__DoBaseOnCommand(ui_type_CObject *type, PyObject *self, PyObject *args)
{
	CObject *ob = (CObject *)ui_assoc_CObject::GetGoodCppObject( self, type );
	if (ob==NULL)
		return NULL;
	PyObject *obwparam, *oblparam;
	if (!PyArg_ParseTuple(args, "OO", &obwparam, &oblparam))
		return NULL;
	WPARAM wparam;
	LPARAM lparam;
	if (!PyWinObject_AsPARAM(obwparam, &wparam))
		return NULL;
	if (!PyWinObject_AsPARAM(oblparam, &lparam))
		return NULL;
	GUI_BGN_SAVE;
	ClassFramework *pcf = (ClassFramework *)ob;
	BOOL rc = pcf->_BaseOnCommand(wparam, lparam);
	GUI_END_SAVE;
	return PyInt_FromLong(rc);
}
#define DoBaseOnCommand(Class, type, self, args) \
	__DoBaseOnCommand<Class>(type, self, args)

template <class ClassFramework>
PyObject *__DoBaseOnClose(ui_type_CObject *type, PyObject *self, PyObject *args)
{
	CObject *ob = (CObject *)ui_assoc_CObject::GetGoodCppObject( self, type );
	if (ob==NULL)
		return NULL;
	if (!PyArg_ParseTuple(args, "")) return NULL;
	GUI_BGN_SAVE;
	ClassFramework *pcf = (ClassFramework *)ob;
	pcf->_BaseOnClose();
	GUI_END_SAVE;
	RETURN_NONE;
}
#define DoBaseOnClose(Class, type, self, args) \
	__DoBaseOnClose<Class>(type, self, args)
