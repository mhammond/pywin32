/*
  dbimodule.h

  Donated to the Python community by EShop, who can not
  support it!

  this is the general interface to copperman-compliant databases.

  In particular, types and type numbers are defined
 */
#ifndef DBI_H
#define DBI_H

PyAPI_FUNC(int) dbiIsDate(const PyObject *o);
PyAPI_FUNC(int) dbiIsRaw(const PyObject *o);
PyAPI_FUNC(int) dbiIsRowId(const PyObject *o);

/* These do not INCREF */
PyAPI_FUNC(PyObject) *dbiValue(PyObject *o);  
PyAPI_FUNC(PyObject) *dbiMakeDate(PyObject *contents);
PyAPI_FUNC(PyObject) *dbiMakeRaw(PyObject *contents);
PyAPI_FUNC(PyObject) *dbiMakeRowId(PyObject *contents);

PyAPI_FUNC(PyObject)*DbiString;
PyAPI_FUNC(PyObject)*DbiRaw;
PyAPI_FUNC(PyObject)*DbiRowId;
PyAPI_FUNC(PyObject)*DbiNumber;
PyAPI_FUNC(PyObject)*DbiDate;

PyAPI_FUNC(PyObject)*DbiNoError;
PyAPI_FUNC(PyObject)*DbiOpError;
PyAPI_FUNC(PyObject)*DbiProgError;
PyAPI_FUNC(PyObject)*DbiIntegrityError;
PyAPI_FUNC(PyObject)*DbiDataError;
PyAPI_FUNC(PyObject)*DbiInternalError;


#endif
