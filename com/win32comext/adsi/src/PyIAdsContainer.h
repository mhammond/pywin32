class PyIADsContainer : public PyIDispatch
{
public:
MAKE_PYCOM_CTOR(PyIADsContainer);
static PyComTypeObject type;
static IADsContainer *GetI(PyObject *self);
	static PyObject *put_Hints(PyObject *self, PyObject *args);
	static PyObject *get_Hints(PyObject *self, PyObject *args);
	static PyObject *put_Filter(PyObject *self, PyObject *args);
	static PyObject *get_Filter(PyObject *self, PyObject *args);
	static PyObject *get_Count(PyObject *self, PyObject *args);
	static PyObject *GetObject(PyObject *self, PyObject *args);
protected:
	PyIADsContainer(IUnknown *);
	~PyIADsContainer();
};

