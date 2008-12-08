/***********************************************************

win32lz.cpp -- module for interface into win32 LZ routines.

Note that this source file contains embedded documentation.
This documentation consists of marked up text inside the
C comments, and is prefixed with an '@' symbol.  The source
files are processed by a tool called "autoduck" which
generates Windows .hlp files.
@doc

******************************************************************/

#include "Pywintypes.h"
#include "lzexpand.h"

static PyObject *obHandleMap = NULL;

/* error helper */
void SetError(char *msg, char *fnName = NULL, DWORD code = 0)
{
	PyObject *v = Py_BuildValue("(izs)", 0, fnName, msg);
	if (v != NULL) {
		PyErr_SetObject(PyWinExc_ApiError, v);
		Py_DECREF(v);
	}
}
PyObject *ReturnError(char *msg, char *fnName = NULL, DWORD code = 0)
{
	SetError(msg, fnName, code);
	return NULL;
}
PyObject *ReturnLZError(char *fnName, long err = 0)
{
	char *pMsg;
	switch (err) {
	case LZERROR_BADINHANDLE:
		pMsg = "The handle identifying the source file is not valid. The file cannot be read.";
		break;
	case LZERROR_BADOUTHANDLE:
		pMsg = "DLE	The handle identifying the destination file is not valid. The file cannot be written.";
		break;
	case LZERROR_GLOBALLOC:
		pMsg = "The maximum number of open compressed files has been exceeded or local memory cannot be allocated.";
		break;
	case LZERROR_GLOBLOCK:
		pMsg = "The LZ file handle cannot be locked down.";
		break;
	case LZERROR_UNKNOWNALG:
		pMsg = "The file is compressed with an unrecognized compression algorithm.";
		break;
	case LZERROR_BADVALUE:
		pMsg = "A parameter was bad";
		break;
	case LZERROR_READ:
		pMsg = "The source file format is not valid.";
		break;
	default:
		pMsg = "No error message is available";
		break;
	}
	PyObject *v = Py_BuildValue("(iss)", err, fnName, pMsg);
	if (v != NULL) {
		PyErr_SetObject(PyWinExc_ApiError, v);
		Py_DECREF(v);
	}
	return NULL;
}
// @pymethod string|win32lz|GetExpandedName|Retrieves the original name of an expanded file,
static PyObject *
PyGetExpandedName(PyObject *self, PyObject *args)
{
	TCHAR outName[_MAX_PATH+1];
	TCHAR *nameIn;
	PyObject *obnameIn;
	if (!PyArg_ParseTuple(args, "O:GetExpandedName", &obnameIn )) // @pyparm str|Source||Name of a compressed file
		return NULL;
	if (!PyWinObject_AsTCHAR(obnameIn, &nameIn, FALSE))
		return NULL;
	// @pyseeapi GetExpandedName
	int ret = GetExpandedName(nameIn, outName);
	PyWinObject_FreeTCHAR(nameIn);
	if (ret!=1)
		return ReturnLZError("GetExpandedName", ret);
	return PyWinObject_FromTCHAR(outName);
}

// @pymethod |win32lz|Close|Closes a handle to an LZ file.
static PyObject *
PyLZClose(PyObject *self, PyObject *args)
{
	int h;
	if (!PyArg_ParseTuple(args, "i:Close", &h )) // @pyparm int|handle||The handle of the LZ file to close.
		return NULL;

	// @pyseeapi LZClose
	LZClose(h);
	Py_INCREF(Py_None);
	return (Py_None);
}

// @pymethod int|win32lz|Copy|Copies a source file to a destination file.
static PyObject *
PyLZCopy(PyObject *self, PyObject *args)
{
	int hSrc, hDest;
	if (!PyArg_ParseTuple(args, "ii:Copy", &hSrc, &hDest )) 
		// @pyparm int|hSrc||The handle of the source file to copy.
		// @pyparm int|hDest||The handle of the destination file.
		return NULL;
	// @comm If the source file is compressed with the Microsoft File Compression Utility
	// (COMPRESS.EXE), this function creates a decompressed destination file.
	// If the source file is not compressed, this function duplicates the original file. 
	// @pyseeapi LZCopy
	long ret = LZCopy( hSrc, hDest);
	if (ret < 0)
		return ReturnLZError("LZCopy",ret);
	return PyInt_FromLong(ret);
}

// @pymethod |win32lz|Init|Allocates memory for the internal data structures required to decompress files, and then creates and initializes them. 
static PyObject *
PyLZInit(PyObject *self, PyObject *args)
{
	int h;
	if (!PyArg_ParseTuple(args, "i:Init", &h )) // @pyparm int|handle||handle of source file
		return NULL;

	// @pyseeapi LZInit
	INT ret = LZInit(h);
	if (ret<0)
		return ReturnLZError("LZInit",ret);
	return PyInt_FromLong(ret);
}

// @pymethod int,(tuple)|win32lz|OpenFile|Creates, opens, reopens, or deletes the specified file. 
static PyObject *
PyLZOpenFile(PyObject *self, PyObject *args)
{
	TCHAR *fname;
	PyObject *obfname;
	int op;
	if (!PyArg_ParseTuple(args, "Oi:OpenFile", &obfname, &op ))
		// @pyparm string|fileName||Name of file to open
		// @pyparm int|action||Can be one of the wi32con.OF_ constants (OF_CREATE, OF_DELETE, etc)
		return NULL;
	if (!PyWinObject_AsTCHAR(obfname, &fname, FALSE))
		return NULL;
	// @pyseeapi LZOpenFile
	OFSTRUCT of;
	of.cBytes = sizeof(OFSTRUCT);
	INT ret = LZOpenFile(fname, &of, op);
	PyWinObject_FreeTCHAR(fname);
	if (ret<0)
		return ReturnLZError("LZOpenFile",ret);
	return Py_BuildValue("i(iiiis)",ret, of.fFixedDisk, of.nErrCode, of.Reserved1, of.Reserved2, of.szPathName);
}


/* List of functions exported by this module */
// @module win32lz|A module encapsulating the Windows LZ compression routines.
static struct PyMethodDef win32lz_functions[] = {
	{"GetExpandedName",             PyGetExpandedName,  METH_VARARGS}, // @pymeth GetExpandedName|Retrieves the original name of an expanded file,
	{"Close",                       PyLZClose,  METH_VARARGS}, // @pymeth Close|Closes a handle to an LZ file.
	{"Copy",                        PyLZCopy,  METH_VARARGS}, // @pymeth Copy|Copies a source file to a destination file.
	{"Init",                        PyLZInit,  METH_VARARGS}, // @pymeth Init|Allocates memory for the internal data structures required to decompress files, and then creates and initializes them. 
	{"OpenFile",                    PyLZOpenFile,  METH_VARARGS}, // @pymeth OpenFile|Creates, opens, reopens, or deletes the specified file. 
	{NULL,			NULL}
};

PYWIN_MODULE_INIT_FUNC(win32lz)
{
	PYWIN_MODULE_INIT_PREPARE(win32lz, win32lz_functions,
		"A module encapsulating the Windows LZ compression routines.");

	PyDict_SetItemString(dict, "error", PyWinExc_ApiError);

	PYWIN_MODULE_INIT_RETURN_SUCCESS;
}
