/*
  dbimodule.h

  Donated to the Python community by EShop, who can not
  support it!

  this is the general interface to copperman-compliant databases.

  In particular, types and type numbers are defined
 */
#ifndef DBI_H
#define DBI_H

#ifdef MS_WIN32
#ifdef DBI_EXPORT
#define DBI_FUNC(x) __declspec(dllexport) x
#else
#define DBI_FUNC(x) __declspec(dllimport) x
#endif /* DBI_EXPORT */
#else /* MS_WIN32 */
#define DBI_FUNC(x) x
#endif

DBI_FUNC(int) dbiIsDate(const PyObject *o);
DBI_FUNC(int) dbiIsRaw(const PyObject *o);
DBI_FUNC(int) dbiIsRowId(const PyObject *o);

/* These do not INCREF */
DBI_FUNC(PyObject) *dbiValue(PyObject *o);  
DBI_FUNC(PyObject) *dbiMakeDate(PyObject *contents);
DBI_FUNC(PyObject) *dbiMakeRaw(PyObject *contents);
DBI_FUNC(PyObject) *dbiMakeRowId(PyObject *contents);

DBI_FUNC(PyObject)*DbiString;
DBI_FUNC(PyObject)*DbiRaw;
DBI_FUNC(PyObject)*DbiRowId;
DBI_FUNC(PyObject)*DbiNumber;
DBI_FUNC(PyObject)*DbiDate;

DBI_FUNC(PyObject)*DbiNoError;
DBI_FUNC(PyObject)*DbiOpError;
DBI_FUNC(PyObject)*DbiProgError;
DBI_FUNC(PyObject)*DbiIntegrityError;
DBI_FUNC(PyObject)*DbiDataError;
DBI_FUNC(PyObject)*DbiInternalError;


#endif
