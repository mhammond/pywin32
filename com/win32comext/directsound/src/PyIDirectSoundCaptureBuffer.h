// This file declares the IDirectSoundCaptureBuffer Interface for Python.
// ---------------------------------------------------
//
// Interface Declaration

class PyIDirectSoundCaptureBuffer : public PyIUnknown
{
public:
	MAKE_PYCOM_CTOR(PyIDirectSoundCaptureBuffer);
	static IDirectSoundCaptureBuffer *GetI(PyObject *self);
	static PyComTypeObject type;

	static PyObject *QueryInterface(PyObject *self, PyObject *args);

	// The Python methods

	// Information methods
	static PyObject *GetCaps(PyObject *self, PyObject *args);
	static PyObject *GetFormat(PyObject *self, PyObject *args);
	static PyObject *GetStatus(PyObject *self, PyObject *args);
	static PyObject *GetCurrentPosition(PyObject *self, PyObject *args);

	// Memory management
	static PyObject *Initialize(PyObject *self, PyObject *args);

	// Capture management
	static PyObject *Start(PyObject *self, PyObject *args);
	static PyObject *Stop(PyObject *self, PyObject *args);
	static PyObject *Update(PyObject *self, PyObject *args);

	PyIDirectSoundCaptureBuffer(IUnknown *pdisp);
	~PyIDirectSoundCaptureBuffer();

	PyObject *m_DS;
};