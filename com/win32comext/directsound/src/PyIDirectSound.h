// This file declares the IDirectSound Interface for Python.
// ---------------------------------------------------
//
// Interface Declaration

class PyIDirectSound : public PyIUnknown
{
public:
	MAKE_PYCOM_CTOR(PyIDirectSound);
	static IDirectSound *GetI(PyObject *self);
	static PyComTypeObject type;

	// The Python methods
	static PyObject *Initialize(PyObject *self, PyObject *args);
	static PyObject *SetCooperativeLevel(PyObject *self, PyObject *args);
	static PyObject *CreateSoundBuffer(PyObject *self, PyObject *args);
	static PyObject *Compact(PyObject *self, PyObject *args);
	static PyObject *GetCaps(PyObject *self, PyObject *args);
	static PyObject *GetSpeakerConfig(PyObject *self, PyObject *args);
	static PyObject *SetSpeakerConfig(PyObject *self, PyObject *args);

	PyIDirectSound(IUnknown *pdisp);
	~PyIDirectSound();
};