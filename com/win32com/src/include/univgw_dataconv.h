/*
** Data conversion functions (between native and Python data types)
*/

#ifndef __DATACONV_H__
#define __DATACONV_H__

/*
** Define a pointer type that can be used to work with va_list values.
**
** On the Alpha, the va_list type is a structure, so we need to use a
** pointer to the va_list.  On all other platforms, the va_list is already
** a pointer, so we don't actually need to do anything.
**
** This va_list reference will get stashed into a PyCObject as it passes
** through Python.
*/
#ifdef _M_ALPHA

typedef va_list *	dataconv_t;
#define VA_LIST(v)		(*(v))
#define VA_LIST_PTR(v)	(&(v))

#else // _M_ALPHA

typedef va_list			dataconv_t;
#define VA_LIST(v)		(v)
#define VA_LIST_PTR(v)	(v)

#endif // _M_ALPHA

PyObject * dataconv_L64(PyObject *self, PyObject *args);
PyObject * dataconv_UL64(PyObject *self, PyObject *args);
PyObject * dataconv_strL64(PyObject *self, PyObject *args);
PyObject * dataconv_strUL64(PyObject *self, PyObject *args);
PyObject * dataconv_interface(PyObject *self, PyObject *args);
PyObject * dataconv_SizeOfVT(PyObject *self, PyObject *args);
PyObject * dataconv_WriteFromOutTuple(PyObject *self, PyObject *args);
PyObject * dataconv_ReadFromInTuple(PyObject *self, PyObject *args);

#endif // __DATACONV_H__
