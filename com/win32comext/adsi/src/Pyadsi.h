class Pyadsi : public PyIDispatch
{
public:
MAKE_PYCOM_CTOR(Pyadsi);
static PyComTypeObject type;
static adsi *GetI(PyObject *self);
	static PyObject *StringAsDS_SELECTION_LIST(PyObject *self, PyObject *args);
	static PyObject *ADsGetLastError(PyObject *self, PyObject *args);
	static PyObject *ADsEnumerateNext(PyObject *self, PyObject *args);
	static PyObject *ADsBuildEnumerator(PyObject *self, PyObject *args);
	static PyObject *ADsOpenObject(PyObject *self, PyObject *args);
	static PyObject *ADsGetObject(PyObject *self, PyObject *args);
protected:
	Pyadsi(IUnknown *);
	~Pyadsi();
};

