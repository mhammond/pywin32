class PyIDsObjectPicker : public PyIUnknown
{
public:
MAKE_PYCOM_CTOR(PyIDsObjectPicker);
static PyComTypeObject type;
static IDsObjectPicker *GetI(PyObject *self);
	static PyObject *InvokeDialog(PyObject *self, PyObject *args);
	static PyObject *Initialize(PyObject *self, PyObject *args);
protected:
	PyIDsObjectPicker(IUnknown *);
	~PyIDsObjectPicker();
};

