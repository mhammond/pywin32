// This file declares the IDirectSoundCapture Interface for Python.
// ---------------------------------------------------
//
// Interface Declaration

class PyIDirectSoundCapture : public PyIUnknown
{
public:
	MAKE_PYCOM_CTOR(PyIDirectSoundCapture);
	static IDirectSoundCapture *GetI(PyObject *self);
	static PyComTypeObject type;

	static PyObject *QueryInterface(PyObject *self, PyObject *args);

	// The Python methods

	static PyObject *Initialize(PyObject *self, PyObject *args);
	static PyObject *CreateCaptureBuffer(PyObject *self, PyObject *args);
	static PyObject *GetCaps(PyObject *self, PyObject *args);

	PyIDirectSoundCapture(IUnknown *pdisp);
	~PyIDirectSoundCapture();

	PyObject *m_DS;
};