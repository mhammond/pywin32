// This file declares the IEmptyVolumeCacheCallBack Interface for Python.
// ---------------------------------------------------
//
// Interface Declaration

class PyIEmptyVolumeCacheCallBack : public PyIUnknown
{
public:
	MAKE_PYCOM_CTOR(PyIEmptyVolumeCacheCallBack);
	static IEmptyVolumeCacheCallBack *GetI(PyObject *self);
	static PyComTypeObject type;

	// The Python methods
	static PyObject *ScanProgress(PyObject *self, PyObject *args);
	static PyObject *PurgeProgress(PyObject *self, PyObject *args);

protected:
	PyIEmptyVolumeCacheCallBack(IUnknown *pdisp);
	~PyIEmptyVolumeCacheCallBack();
};
