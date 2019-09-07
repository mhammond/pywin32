#ifndef __PYIMONIKER_H__
#define __PYIMONIKER_H__

#include "PyIPersistStream.h"

class PYCOM_EXPORT PyIEnumMoniker : public PyIUnknown {
   public:
    MAKE_PYCOM_CTOR(PyIEnumMoniker);
    static PyComEnumTypeObject type;
    static IEnumMoniker *GetI(PyObject *self);

    static PyObject *Next(PyObject *self, PyObject *args);
    static PyObject *Skip(PyObject *self, PyObject *args);
    static PyObject *Reset(PyObject *self, PyObject *args);
    static PyObject *Clone(PyObject *self, PyObject *args);

   protected:
    PyIEnumMoniker(IUnknown *);
    ~PyIEnumMoniker();
};

class PYCOM_EXPORT PyIMoniker : public PyIPersistStream {
   public:
    MAKE_PYCOM_CTOR(PyIMoniker);
    static PyComEnumProviderTypeObject type;
    static IMoniker *GetI(PyObject *self);

    static PyObject *BindToObject(PyObject *self, PyObject *args);
    static PyObject *BindToStorage(PyObject *self, PyObject *args);
    static PyObject *Reduce(PyObject *self, PyObject *args);
    static PyObject *ComposeWith(PyObject *self, PyObject *args);
    static PyObject *Enum(PyObject *self, PyObject *args);
    static PyObject *IsEqual(PyObject *self, PyObject *args);
    static PyObject *Hash(PyObject *self, PyObject *args);
    static PyObject *IsRunning(PyObject *self, PyObject *args);
    static PyObject *GetTimeOfLastChange(PyObject *self, PyObject *args);
    static PyObject *Inverse(PyObject *self, PyObject *args);
    static PyObject *CommonPrefixWith(PyObject *self, PyObject *args);
    static PyObject *RelativePathTo(PyObject *self, PyObject *args);
    static PyObject *GetDisplayName(PyObject *self, PyObject *args);
    static PyObject *ParseDisplayName(PyObject *self, PyObject *args);
    static PyObject *IsSystemMoniker(PyObject *self, PyObject *args);

   protected:
    PyIMoniker(IUnknown *);
    ~PyIMoniker();
};

#endif /* __PYIMONIKER_H__ */
