class PyIDirectorySearch : public PyIUnknown
{
public:
MAKE_PYCOM_CTOR(PyIDirectorySearch);
static PyComTypeObject type;
static IDirectorySearch *GetI(PyObject *self);
	static PyObject *GetNextColumnName(PyObject *self, PyObject *args);
	static PyObject *GetColumn(PyObject *self, PyObject *args);
	static PyObject *AbandonSearch(PyObject *self, PyObject *args);
	static PyObject *CloseSearchHandle(PyObject *self, PyObject *args);
	static PyObject *GetPreviousRow(PyObject *self, PyObject *args);
	static PyObject *GetFirstRow(PyObject *self, PyObject *args);
	static PyObject *GetNextRow(PyObject *self, PyObject *args);
	static PyObject *ExecuteSearch(PyObject *self, PyObject *args);
	static PyObject *SetSearchPreference(PyObject *self, PyObject *args);
protected:
	PyIDirectorySearch(IUnknown *);
	~PyIDirectorySearch();
};

