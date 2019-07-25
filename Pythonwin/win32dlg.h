// dialog objects

// hack the associations a bit.  When the dialog window closes, I dont
// destroy the association.

/////////////////////////////////////////////////////////
//
//	dialog
class PYW_EXPORT PyCDialog : public PyCWnd {
   public:
    MAKE_PY_CTOR(PyCDialog)
    static PyObject *PyCDialog::create(PyObject *self, PyObject *args);
    static PyObject *PyCDialog::createIndirect(PyObject *self, PyObject *args);
    PyObject *ddlist;
    PyObject *dddict;
    HGLOBAL hTemplate;
    HINSTANCE hInstance;  // If known, the DLL we loaded from.
    HGLOBAL hSaved;
    static ui_type_CObject type;

   protected:
    PyCDialog();
    virtual ~PyCDialog();
    virtual void SetAssocInvalid() { return; }  // ignore
   public:
    virtual PyObject *getattro(PyObject *obname);
};
////////////////////////////////////////////////////////
//
//	Common dialog base.
class PYW_EXPORT PyCCommonDialog : public PyCDialog {
   public:
    static ui_type_CObject type;

   protected:
};

////////////////////////////////////////////////////////
//
//	file dialog
class PyCFileDialog : public PyCCommonDialog {
   public:
    MAKE_PY_CTOR(PyCFileDialog)
    static PyObject *ui_file_dialog_create(PyObject *self, PyObject *args);  // create an actual object.
    static ui_type_CObject type;

   protected:
    PyCFileDialog();
    ~PyCFileDialog();
};

////////////////////////////////////////////////////////
//
//	font dialog
class PyCFontDialog : public PyCCommonDialog {
   public:
    MAKE_PY_CTOR(PyCFontDialog)
    static PyObject *ui_font_dialog_create(PyObject *self, PyObject *args);  // create an actual object.
    LOGFONT *pInitLogFont;
    static ui_type_CObject type;

   protected:
    PyCFontDialog();
    ~PyCFontDialog();
};

////////////////////////////////////////////////////////
//
//	color dialog
class PyCColorDialog : public PyCCommonDialog {
   public:
    MAKE_PY_CTOR(PyCColorDialog)
    static PyObject *create(PyObject *self, PyObject *args);  // create an actual object.
    static ui_type_CObject type;

   protected:
    PyCColorDialog();
    ~PyCColorDialog();
};

////////////////////////////////////////////////////////
//
//	print dialog
class PyCPrintDialog : public PyCCommonDialog {
   public:
    MAKE_PY_CTOR(PyCPrintDialog)
    static PyObject *create(PyObject *self, PyObject *args);
    static ui_type_CObject type;

   protected:
    PyCPrintDialog();
    virtual ~PyCPrintDialog();
};
