// This file declares the IDirectSoundNotify Interface for Python.
// ---------------------------------------------------
//
// Interface Declaration

class PyIDirectSoundNotify : public PyIUnknown {
   public:
    MAKE_PYCOM_CTOR(PyIDirectSoundNotify);
    static IDirectSoundNotify *GetI(PyObject *self);
    static PyComTypeObject type;

    // The Python methods
    static PyObject *SetNotificationPositions(PyObject *self, PyObject *args);

    PyIDirectSoundNotify(IUnknown *pdisp);
    ~PyIDirectSoundNotify();

    PyObject *m_DS;
};