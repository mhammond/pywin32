// Cloned from PyIEnumSTATPROPSTG.h


class PyIEnumSTATPROPSETSTG : public PyIUnknown
{
public:
	MAKE_PYCOM_CTOR(PyIEnumSTATPROPSETSTG);
	static IEnumSTATPROPSETSTG *GetI(PyObject *self);
	static PyComEnumTypeObject type;
	// The Python methods
	static PyObject *Next(PyObject *self, PyObject *args);
	static PyObject *Skip(PyObject *self, PyObject *args);
	static PyObject *Reset(PyObject *self, PyObject *args);
	static PyObject *Clone(PyObject *self, PyObject *args);

protected:
	PyIEnumSTATPROPSETSTG(IUnknown *pdisp);
	~PyIEnumSTATPROPSETSTG();
};
