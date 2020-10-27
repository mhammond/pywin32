// PyMapiUtil.h

#include "mapix.h"

// We should not be using this!
#define OleSetOleError PyCom_BuildPyException

PyObject *PyMAPIObject_FromTypedUnknown(ULONG typ, IUnknown *pUnk, BOOL bAddRef);

PyObject *PyObject_FromMAPIERROR(MAPIERROR *e, BOOL bIsUnicode, BOOL free_buffer);

/* Create (and free) a SBinaryArray from a PyObject */
BOOL PyMAPIObject_AsSBinaryArray(PyObject *ob, SBinaryArray *pv);
void PyMAPIObject_FreeSBinaryArray(SBinaryArray *pv);

/* Create (and free) a SPropValue from a PyObject */
BOOL PyMAPIObject_AsSPropValue(PyObject *ob, SPropValue *pv, void *pAllocMoreLinkBlock);
PyObject *PyMAPIObject_FromSPropValue(SPropValue *pv);

/* Create a PyObject to/from a SPropValue Array*/
BOOL PyMAPIObject_AsSPropValueArray(PyObject *ob, SPropValue **ppv, ULONG *pcValues);
PyObject *PyMAPIObject_FromSPropValueArray(SPropValue *pv, ULONG nvalues);

/* Create a PyObject from a SRow/SRowSet */
PyObject *PyMAPIObject_FromSRow(SRow *pr);
PyObject *PyMAPIObject_FromSRowSet(SRowSet *prs);

/* Create (and free) a SRowSet from a PyObject */
BOOL PyMAPIObject_AsSRowSet(PyObject *obSeq, SRowSet **ppResult, BOOL bNoneOK);
void PyMAPIObject_FreeSRowSet(SRowSet *pResult);

/* ADRLIST structures are really just SRowSet */
#define PyMAPIObject_FromADRLIST(prs) PyMAPIObject_FromSRowSet((SRowSet *)(prs))

#define PyMAPIObject_AsADRLIST(obSeq, ppResult, bNoneOK) PyMAPIObject_AsSRowSet(obSeq, (SRowSet **)(ppResult), bNoneOK)
#define PyMAPIObject_FreeADRLIST(p) PyMAPIObject_FreeSRowSet((SRowSet *)(p))

/* Create (and free) a SSortOrderSet from a PyObject */
BOOL PyMAPIObject_AsSSortOrderSet(PyObject *obsos, SSortOrderSet **ppsos, BOOL bNoneOK = TRUE);
void PyMAPIObject_FreeSSortOrderSet(SSortOrderSet *ppsos);

/* Create (and free) a SRestriction from a PyObject */
BOOL PyMAPIObject_AsSRestriction(PyObject *ob, SRestriction **pRest, BOOL bNoneOK = TRUE);
void PyMAPIObject_FreeSRestriction(SRestriction *pr);

/* Create (and free) a SPropTagArray from a PyObject */
BOOL PyMAPIObject_AsSPropTagArray(PyObject *obsos, SPropTagArray **ppta);
void PyMAPIObject_FreeSPropTagArray(SPropTagArray *pta);

/* Create a PyObject from a SPropTagArray */
PyObject *PyMAPIObject_FromSPropTagArray(SPropTagArray *pta);

/* Create (and free) a MAPINAMEID array from a PyObject */
BOOL PyMAPIObject_AsMAPINAMEIDArray(PyObject *ob, MAPINAMEID ***pppNameId, ULONG *pNumIds, BOOL bNoneOK = FALSE);
void PyMAPIObject_FreeMAPINAMEIDArray(MAPINAMEID **pv);

/* Create a PyObject from a MAPINAMEID Array */
PyObject *PyMAPIObject_FromMAPINAMEIDArray(MAPINAMEID **ppNameId, ULONG numIds);

/* Create a PyObject from a SPropProblemArray */
PyObject *PyMAPIObject_FromSPropProblemArray(SPropProblemArray *ppa);

PyObject *PyWinObject_FromMAPIStr(LPTSTR str, BOOL isUnicode);
BOOL PyWinObject_AsMAPIStr(PyObject *stringObject, LPTSTR *pResult, BOOL asUnicode, BOOL bNoneOK = FALSE,
                           DWORD *pResultLen = NULL);
