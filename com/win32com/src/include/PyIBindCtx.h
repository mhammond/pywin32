
/////////////////////////////////////////////////////////////////////////////
// class PyIBindCtx

class PYCOM_EXPORT PyIBindCtx : public PyIUnknown
{
public:
	MAKE_PYCOM_CTOR(PyIBindCtx);
	static PyComTypeObject type;
	static IBindCtx *GetI(PyObject *self);

	static PyObject *GetRunningObjectTable(PyObject *self, PyObject *args);

protected:
	PyIBindCtx(IUnknown *);
	~PyIBindCtx();
};
