// bits_pch.h : header file for PCH generation for the BITS COM extension

#include <PythonCOM.h>
#include <Bits.h>

BOOL PyObject_AsBG_FILE_INFO_LIST(PyObject *ob, ULONG *pnum, BG_FILE_INFO **fi);
void PyObject_FreeBG_FILE_INFO_LIST(ULONG pnum, BG_FILE_INFO *fi);

BOOL PyObject_AsBG_FILE_RANGE_LIST(PyObject *ob, DWORD *pnum, BG_FILE_RANGE **fr);
void PyObject_FreeBG_FILE_RANGE_LIST(DWORD num, BG_FILE_RANGE *fr);

PyObject *PyObject_FromBG_FILE_PROGRESS(BG_FILE_PROGRESS *fp);
PyObject *PyObject_FromBG_JOB_PROGRESS(BG_JOB_PROGRESS *jp);
PyObject *PyObject_FromBG_JOB_REPLY_PROGRESS(BG_JOB_REPLY_PROGRESS *jrs);
PyObject *PyObject_FromBG_JOB_TIMES(BG_JOB_TIMES *jt);

BOOL PyObject_AsBG_AUTH_CREDENTIALS(PyObject *ob, BG_AUTH_CREDENTIALS *pRet);

BOOL PyObject_AsBG_JOB_TYPE(PyObject *ob, BG_JOB_TYPE *jt);
void PyObject_FreeBG_JOB_TYPE(BG_JOB_TYPE *jt);
