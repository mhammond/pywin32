#ifndef __PYRECORD_H__
#define __PYRECORD_H__

class PyRecordBuffer;

// @object PyRecord|An object that represents a COM User Defined Type.
// @comm Once created or obtained from other methods, you can simply
// get and set attributes.
class PyRecord : public PyObject {
   public:
    PyRecord(IRecordInfo *ri, PVOID data, PyRecordBuffer *owner);
    ~PyRecord();

    static PyRecord *new_record(IRecordInfo *ri, PVOID data, PyRecordBuffer *owner, PyTypeObject *type = NULL);
    static PyObject *tp_new(PyTypeObject *type, PyObject *args, PyObject *kwds);
    static int tp_init(PyObject *self, PyObject *args, PyObject *kwds);
    static void tp_dealloc(PyRecord *ob);
    static PyObject *getattro(PyObject *self, PyObject *obname);
    static int setattro(PyObject *self, PyObject *obname, PyObject *v);
    static PyObject *tp_repr(PyObject *self);
    static PyObject *tp_richcompare(PyObject *ob1, PyObject *ob2, int op);
    static struct PyMethodDef methods[];

    static PyTypeObject Type;
    IRecordInfo *pri;
    void *pdata;
    PyRecordBuffer *owner;
};

#endif  // __PYRECORD_H__
