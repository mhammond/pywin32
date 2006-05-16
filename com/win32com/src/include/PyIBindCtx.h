
/////////////////////////////////////////////////////////////////////////////
// class PyIBindCtx

class PYCOM_EXPORT PyIBindCtx : public PyIUnknown
{
public:
	MAKE_PYCOM_CTOR(PyIBindCtx);
	static PyComTypeObject type;
	static IBindCtx *GetI(PyObject *self);

	static PyObject *GetRunningObjectTable(PyObject *self, PyObject *args);
	static PyObject *GetBindOptions(PyObject *self, PyObject *args);
	static PyObject *SetBindOptions(PyObject *self, PyObject *args);
	static PyObject *RegisterObjectParam(PyObject *self, PyObject *args);
	static PyObject *RevokeObjectParam(PyObject *self, PyObject *args);
	static PyObject *GetObjectParam(PyObject *self, PyObject *args);
	static PyObject *EnumObjectParam(PyObject *self, PyObject *args);
protected:
	PyIBindCtx(IUnknown *);
	~PyIBindCtx();
};
