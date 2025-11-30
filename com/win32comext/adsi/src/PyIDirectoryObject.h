class PyIDirectoryObject : public PyIUnknown
{
public:
MAKE_PYCOM_CTOR(PyIDirectoryObject);
static PyComTypeObject type;
static IDirectoryObject *GetI(PyObject *self);
	static PyObject *DeleteDSObject(PyObject *self, PyObject *args);
	static PyObject *CreateDSObject(PyObject *self, PyObject *args);
	static PyObject *SetObjectAttributes(PyObject *self, PyObject *args);
	static PyObject *GetObjectAttributes(PyObject *self, PyObject *args);
	static PyObject *GetObjectInformation(PyObject *self, PyObject *args);
protected:
	PyIDirectoryObject(IUnknown *);
	~PyIDirectoryObject();
};

