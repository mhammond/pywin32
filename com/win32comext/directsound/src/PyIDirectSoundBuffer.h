// This file declares the IDirectSoundBuffer Interface for Python.
// ---------------------------------------------------
//
// Interface Declaration

class PyIDirectSoundBuffer : public PyIUnknown
{
public:
	MAKE_PYCOM_CTOR(PyIDirectSoundBuffer);
	static IDirectSoundBuffer *GetI(PyObject *self);
	static PyComTypeObject type;

	static PyObject *QueryInterface(PyObject *self, PyObject *args);

	// The Python methods

	// Information methods
	static PyObject *GetCaps(PyObject *self, PyObject *args);
	static PyObject *GetFormat(PyObject *self, PyObject *args);
	static PyObject *GetStatus(PyObject *self, PyObject *args);
	static PyObject *SetFormat(PyObject *self, PyObject *args);

	// Memory management
	static PyObject *Initialize(PyObject *self, PyObject *args);
	static PyObject *Restore(PyObject *self, PyObject *args);

	// Play management
	static PyObject *GetCurrentPosition(PyObject *self, PyObject *args);
	static PyObject *Play(PyObject *self, PyObject *args);
	static PyObject *SetCurrentPosition(PyObject *self, PyObject *args);
	static PyObject *Stop(PyObject *self, PyObject *args);
	static PyObject *Update(PyObject *self, PyObject *args);

	// Sound management
	static PyObject *GetFrequency(PyObject *self, PyObject *args);
	static PyObject *GetPan(PyObject *self, PyObject *args);
	static PyObject *GetVolume(PyObject *self, PyObject *args);
	static PyObject *SetFrequency(PyObject *self, PyObject *args);
	static PyObject *SetPan(PyObject *self, PyObject *args);
	static PyObject *SetVolume(PyObject *self, PyObject *args);

	PyIDirectSoundBuffer(IUnknown *pdisp);
	~PyIDirectSoundBuffer();

	PyObject *m_DS;
};