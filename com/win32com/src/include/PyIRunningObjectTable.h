
class PyIRunningObjectTable : public PyIUnknown
{
public:
	MAKE_PYCOM_CTOR(PyIRunningObjectTable);
	static PyComEnumProviderTypeObject type;
	static IRunningObjectTable *GetI(PyObject *self);

	static PyObject *IsRunning(PyObject *self, PyObject *args);
	static PyObject *GetObject(PyObject *self, PyObject *args);
	static PyObject *EnumRunning(PyObject *self, PyObject *args);
	static PyObject *Register(PyObject *self, PyObject *args);
	static PyObject *Revoke(PyObject *self, PyObject *args);

protected:
	PyIRunningObjectTable(IUnknown *);
	~PyIRunningObjectTable();
};
