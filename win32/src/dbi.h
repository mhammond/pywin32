/*
  dbimodule.h

  Donated to the Python community by EShop, who can not
  support it!

  this is the general interface to copperman-compliant databases.

  In particular, types and type numbers are defined
 */
#ifndef DBI_H
#define DBI_H

int dbiIsDate(const PyObject *o);
int dbiIsRaw(const PyObject *o);
int dbiIsRowId(const PyObject *o);

/* These do not INCREF */
PyObject *dbiValue(PyObject *o);  
PyObject *dbiMakeDate(PyObject *contents);
PyObject *dbiMakeRaw(PyObject *contents);
PyObject *dbiMakeRowId(PyObject *contents);

#ifdef DBI_EXPORT
    #define CALLCONV(RTYPE) __declspec(dllexport) RTYPE
#else
    #define CALLCONV(RTYPE) __declspec(dllimport) RTYPE
#endif

CALLCONV(PyObject)*DbiString;
CALLCONV(PyObject)*DbiRaw;
CALLCONV(PyObject)*DbiRowId;
CALLCONV(PyObject)*DbiNumber;
CALLCONV(PyObject)*DbiDate;

CALLCONV(PyObject)*DbiNoError;
CALLCONV(PyObject)*DbiOpError;
CALLCONV(PyObject)*DbiProgError;
CALLCONV(PyObject)*DbiIntegrityError;
CALLCONV(PyObject)*DbiDataError;
CALLCONV(PyObject)*DbiInternalError;


#endif
