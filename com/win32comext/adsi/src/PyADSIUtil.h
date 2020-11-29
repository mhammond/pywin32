// PyADSIUtil.h
#include "iads.h"
#include "Adshlp.h"

PyObject *PyADSIObject_FromADS_OBJECT_INFO(ADS_OBJECT_INFO *);

PyObject *PyADSIObject_FromADS_ATTR_INFOs(ADS_ATTR_INFO *infos, DWORD cinfos);
BOOL PyADSIObject_AsADS_ATTR_INFOs(PyObject *ob, ADS_ATTR_INFO **ppret, DWORD *pcinfos);
void PyADSIObject_FreeADS_ATTR_INFOs(ADS_ATTR_INFO *pattr, DWORD cattr);

// Helpers for passing arrays of Unicode around.
BOOL PyADSI_MakeNames(PyObject *obNames, WCHAR ***names, DWORD *pcnames);
void PyADSI_FreeNames(WCHAR **names, DWORD cnames);

BOOL PyADSIObject_AsADS_SEARCHPREF_INFOs(PyObject *ob, ADS_SEARCHPREF_INFO **ppret, DWORD *pcinfos);
void PyADSIObject_FreeADS_SEARCHPREF_INFOs(ADS_SEARCHPREF_INFO *pattr, DWORD cattr);

PyObject *PyADSIObject_FromADSVALUE(ADSVALUE &v);
