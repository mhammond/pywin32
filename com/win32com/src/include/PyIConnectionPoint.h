class PyIConnectionPoint : public PyIUnknown
{
public:
	MAKE_PYCOM_CTOR(PyIConnectionPoint);
	static IConnectionPoint *GetI(PyObject *self);
	static PyComTypeObject type;

	// The Python methods
	static PyObject *GetConnectionInterface(PyObject *self, PyObject *args);
	static PyObject *GetConnectionPointContainer(PyObject *self, PyObject *args);
	static PyObject *Advise(PyObject *self, PyObject *args);
	static PyObject *Unadvise(PyObject *self, PyObject *args);
	static PyObject *EnumConnections(PyObject *self, PyObject *args);

protected:
	PyIConnectionPoint(IUnknown *pdisp);
	~PyIConnectionPoint();
};
