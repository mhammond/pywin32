class PyIShellLinkDataList : public PyIUnknown
{
public:
	MAKE_PYCOM_CTOR(PyIShellLinkDataList);
	static IShellLinkDataList *GetI(PyObject *self);
	static PyComTypeObject type;

	// The Python methods
	static PyObject *AddDataBlock(PyObject *self, PyObject *args);
	static PyObject *CopyDataBlock(PyObject *self, PyObject *args);
	static PyObject *RemoveDataBlock(PyObject *self, PyObject *args);
	static PyObject *GetFlags(PyObject *self, PyObject *args);
	static PyObject *SetFlags(PyObject *self, PyObject *args);

protected:
	PyIShellLinkDataList(IUnknown *pdisp);
	~PyIShellLinkDataList();
};
