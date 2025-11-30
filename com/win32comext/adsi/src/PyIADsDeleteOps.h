class PyIADsDeleteOps : public PyIDispatch
{
public:
MAKE_PYCOM_CTOR(PyIADsDeleteOps);
static PyComTypeObject type;
static IADsDeleteOps *GetI(PyObject *self);
	static PyObject *DeleteObject(PyObject *self, PyObject *args);
protected:
	PyIADsDeleteOps(IUnknown *);
	~PyIADsDeleteOps();
};

