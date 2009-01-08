// property page/sheet objects

// property sheets are a "clone"
// hack the associations a bit.  When the dialog window closes, I dont
// destroy the association.

/////////////////////////////////////////////////////////
//
//	
class PyCPropertySheet : public PyCWnd {
public:
	static PyObject *create( PyObject *self, PyObject *args );
	static ui_type_CObject type;
	MAKE_PY_CTOR(PyCPropertySheet)
protected:
	PyCPropertySheet();
	virtual ~PyCPropertySheet();
};

class PyCPropertyPage : public PyCDialog {
public:
	static PyObject *create( PyObject *self, PyObject *args );
	static PyObject *createIndirect( PyObject *self, PyObject *args );
	static ui_type_CObject type;
	MAKE_PY_CTOR(PyCPropertyPage)
protected:
	PyCPropertyPage();
	virtual ~PyCPropertyPage();
};

class ui_tabctrl_object : public PyCWnd {
public:
	static PyObject *create( PyObject *self, PyObject *args );
protected:
	ui_tabctrl_object();
	virtual ~ui_tabctrl_object();
public:
	static ui_type_CObject type;
	MAKE_PY_CTOR(ui_tabctrl_object)
};
