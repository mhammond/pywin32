// -*- Mode: C++; tab-width: 4 -*-
//	Author: Sam Rushing <rushing@nightmare.com>
//	$Id$

// mmapfilemodule.cpp -- map a view of a file into memory
//
// todo: need permission flags, perhaps a 'chsize' analog
//   not all functions check range yet!!!
//
//
// Note: This module currently only deals with 32-bit file
//   sizes.
//
// The latest version of mmapfile is maintained by Sam at
// ftp://squirl.nightmare.com/pub/python/python-ext

#include <windows.h>

#include "Python.h"

#include <string.h>
#include <sys/types.h>

static PyObject *mmapfile_module_error;

typedef struct {
  PyObject_HEAD
  HANDLE	map_handle;
  HFILE		file_handle;
  char *	data;
  size_t	size;
  size_t	pos;
  char *	tagname;
} mmapfile_object;

static void
mmapfile_object_dealloc(mmapfile_object * m_obj)
{
  UnmapViewOfFile (m_obj->data);
  CloseHandle (m_obj->map_handle);
  CloseHandle ((HANDLE)m_obj->file_handle);
  PyMem_DEL(m_obj);
}

static PyObject *
mmapfile_close_method (mmapfile_object * self, PyObject * args)
{
  UnmapViewOfFile (self->data);
  CloseHandle (self->map_handle);
  CloseHandle ((HANDLE)self->file_handle);
  self->map_handle = (HANDLE) NULL;
  Py_INCREF (Py_None);
  return (Py_None);
}

#define CHECK_VALID														\
do {																	\
  if (!self->map_handle) {												\
    PyErr_SetString (PyExc_ValueError, "mmapfile closed or invalid");	\
    return NULL;														\
  }																		\
} while (0)

static PyObject *
mmapfile_read_byte_method (mmapfile_object * self,
						   PyObject * args)
{
  char value;
  char * where = (self->data+self->pos);
  CHECK_VALID;
  if ((where >= 0) && (where < (self->data+self->size))) {
	value = (char) *(where);
	self->pos += 1;
	return Py_BuildValue("c", (char) *(where));
  } else {
	PyErr_SetString (PyExc_ValueError, "read byte out of range");
	return NULL;
  }
}

static PyObject *
mmapfile_read_line_method (mmapfile_object * self,
						   PyObject * args)
{
  char * start;
  char * eof = self->data+self->size;
  char * eol;

  CHECK_VALID;
  start = self->data+self->pos;

  // strchr was a bad idea here - there's no way to range
  // check it.  there is no 'strnchr'
  for (eol = start; (eol < eof) && (*eol != '\n') ; eol++)
	{ /* do nothing */ }

  PyObject * result = Py_BuildValue("s#", start, (long) (++eol - start));
  self->pos += (eol - start);
  return (result);
}

static PyObject *
mmapfile_read_method (mmapfile_object * self,
					  PyObject * args)
{
  long num_bytes;

  CHECK_VALID;
  if (!PyArg_ParseTuple (args, "l", &num_bytes))
	return(NULL);

  // silently 'adjust' out-of-range requests
  if ((self->pos + num_bytes) > self->size) {
	num_bytes -= (self->pos+num_bytes) - self->size;
  }
  PyObject * result = Py_BuildValue("s#", self->data+self->pos, num_bytes);
  self->pos += num_bytes;  
  return (result);
}

static PyObject *
mmapfile_find_method (mmapfile_object *self,
					  PyObject *args)
{
  long start = self->pos;
  char * needle;
  int len;

  CHECK_VALID;
  if (!PyArg_ParseTuple (args, "s#|l", &needle, &len, &start)) {
	return NULL;
  } else {
	char * p = self->data+self->pos;
	char * e = self->data+self->size;
	while (p < e) {
	  char * s = p;
	  char * n = needle;
	  while ((s<e) && (*n) && !(*s-*n)) {
		s++, n++;
	  }
	  if (!*n) {
		return Py_BuildValue ("l", (long) (p - (self->data + start)));
	  }
	  p++;
	}
	return Py_BuildValue ("l", (long) -1);
  }
}

static PyObject *
mmapfile_write_method (mmapfile_object * self,
					   PyObject * args)
{
  long length;
  char * data;

  CHECK_VALID;
  if (!PyArg_ParseTuple (args, "s#", &data, &length))
	return(NULL);

  if ((self->pos + length) > self->size) {
	PyErr_SetString (mmapfile_module_error, "data out of range");
	return NULL;
  }
  memcpy (self->data+self->pos, data, length);
  self->pos = self->pos+length;
  Py_INCREF (Py_None);
  return (Py_None);
}

static PyObject *
mmapfile_write_byte_method (mmapfile_object * self,
							PyObject * args)
{
  char value;

  CHECK_VALID;
  if (!PyArg_ParseTuple (args, "c", &value))
	return(NULL);

  *(self->data+self->pos) = value;
  self->pos += 1;
  Py_INCREF (Py_None);
  return (Py_None);
}

static PyObject *
mmapfile_size_method (mmapfile_object * self,
					  PyObject * args)
{
  CHECK_VALID;
  if (self->file_handle != (HFILE) 0xFFFFFFFF) {
	return (Py_BuildValue ("l", GetFileSize ((HANDLE)self->file_handle, NULL)));
  } else {
	return (Py_BuildValue ("l", self->size) );
  }
}

// This assumes that you want the entire file mapped,
// and when recreating the map will make the new file
// have the new size
//
// Is this really necessary?  This could easily be done
// from python by just closing and re-opening with the
// new size?

static PyObject *
mmapfile_resize_method (mmapfile_object * self,
						PyObject * args)
{
  unsigned long new_size;
  CHECK_VALID;
  if (!PyArg_ParseTuple (args, "l", &new_size)) {
	return NULL;
  } else { 
	// First, unmap the file view
	UnmapViewOfFile (self->data);
	// Close the mapping object
	CloseHandle ((HANDLE)self->map_handle);
	// Move to the desired EOF position
	SetFilePointer ((HANDLE)self->file_handle, new_size, NULL, FILE_BEGIN);
	// Change the size of the file
	SetEndOfFile ((HANDLE)self->file_handle);
	// Create another mapping object and remap the file view
	self->map_handle = CreateFileMapping ((HANDLE) self->file_handle,
										   NULL,
										   PAGE_READWRITE,
										   0,
										   new_size,
										   self->tagname);
	char complaint[256];
	if (self->map_handle != NULL) {
	  self->data = (char *) MapViewOfFile (self->map_handle,
											FILE_MAP_WRITE,
											0,
											0,
											0);
	  if (self->data != NULL) {
		self->size = new_size;
		Py_INCREF (Py_None);
		return Py_None;
	  } else {
		sprintf (complaint, "MapViewOfFile failed: %ld", GetLastError());
	  }
	} else {
	  sprintf (complaint, "CreateFileMapping failed: %ld", GetLastError());
	}
	PyErr_SetString (mmapfile_module_error, complaint);
	return (NULL);
  }
}

static PyObject *
mmapfile_flush_method (mmapfile_object * self, PyObject * args)
{
  size_t offset	= 0;
  size_t size	= self->size;
  CHECK_VALID;
  if (!PyArg_ParseTuple (args, "|ll", &offset, &size)) {
	return NULL;
  } else if ((offset + size) > self->size) {
	PyErr_SetString (PyExc_ValueError, "flush values out of range");
	return NULL;
  } else {
	return (Py_BuildValue ("l", FlushViewOfFile (self->data+offset, size)));
  }
}

static PyObject *
mmapfile_tell_method (mmapfile_object * self, PyObject * args)
{
  CHECK_VALID;
  return (Py_BuildValue ("l", self->pos) );
}

static PyObject *
mmapfile_seek_method (mmapfile_object * self, PyObject * args)
{
  // ptrdiff_t dist;
  long dist;
  int how=0;
  CHECK_VALID;
  if (!PyArg_ParseTuple (args, "l|i", &dist, &how)) {
	return(NULL);
  } else {
	unsigned long where;
	switch (how) {
	case 0:
	  where = dist;
	  break;
	case 1:
	  where = self->pos + dist;
	  break;
	case 2:
	  where = self->size - dist;
	  break;
	default:
	  PyErr_SetString (PyExc_ValueError, "unknown seek type");
	  return NULL;
	}
	if ((where >= 0) && (where < (self->size))) {
	  self->pos = where;
	  Py_INCREF (Py_None);
	  return (Py_None);
	} else {
	  PyErr_SetString (PyExc_ValueError, "seek out of range");
	  return NULL;
	}
  }
}

static PyObject *
mmapfile_move_method (mmapfile_object * self, PyObject * args)
{
  unsigned long dest, src, count;
  CHECK_VALID;
  if (!PyArg_ParseTuple (args, "iii", &dest, &src, &count)) {
	return NULL;
  } else {
	// bounds check the values
	if (// end of source after end of data??
		((src+count) > self->size)
		// dest will fit?
		|| (dest+count > self->size)) {
	  PyErr_SetString (PyExc_ValueError,
					   "source or destination out of range");
	  return NULL;
	} else {
	  memmove (self->data+dest, self->data+src, count);
	  Py_INCREF (Py_None);
	  return Py_None;
	}
  }
}

static struct PyMethodDef mmapfile_object_methods[] = {
  {"close",		(PyCFunction) mmapfile_close_method,		1},
  {"find",		(PyCFunction) mmapfile_find_method,			1},
  {"flush",		(PyCFunction) mmapfile_flush_method,		1},
  {"move",		(PyCFunction) mmapfile_move_method,			1},
  {"read",		(PyCFunction) mmapfile_read_method,			1},
  {"read_byte", (PyCFunction) mmapfile_read_byte_method,	1},
  {"readline",	(PyCFunction) mmapfile_read_line_method,	1},
  {"resize",	(PyCFunction) mmapfile_resize_method,		1},
  {"seek",		(PyCFunction) mmapfile_seek_method,			1},
  {"size",		(PyCFunction) mmapfile_size_method,			1},
  {"tell",		(PyCFunction) mmapfile_tell_method,			1},
  {"write",		(PyCFunction) mmapfile_write_method,		1},
  {"write_byte",(PyCFunction) mmapfile_write_byte_method,	1},
  {NULL,	   NULL}	   /* sentinel */
};

static PyObject *
mmapfile_object_getattr(mmapfile_object * self, char * name)
{
  return Py_FindMethod (mmapfile_object_methods, (PyObject *)self, name);
}

static PyTypeObject mmapfile_object_type = {
  PyObject_HEAD_INIT(&PyType_Type)
  0,									// ob_size
  "mmapfile",							// tp_name
  sizeof(mmapfile_object),				// tp_size
  0,									// tp_itemsize
  // methods
  (destructor) mmapfile_object_dealloc, // tp_dealloc
  0,									// tp_print
  (getattrfunc) mmapfile_object_getattr,// tp_getatt
  0,									// tp_setattr
  0,									// tp_compare
  0,									// tp_repr
  0,									// tp_as_number
};

static PyObject *
new_mmapfile_object (PyObject * self, PyObject * args)
{
  mmapfile_object * m_obj;
  unsigned long map_size;
  char * filename;
  int namelen;
  char * tagname;
  char complaint[256];
  HFILE fh = 0;
  OFSTRUCT file_info;

  if (!PyArg_Parse (args,
					"(s#zl)",
					&filename,
					&namelen,
					&tagname,
					&map_size)
	  )
	return NULL;
  
  // if an actual filename has been specified
  if (namelen != 0) {
	fh = OpenFile (filename, &file_info, OF_READWRITE);
	if (fh == HFILE_ERROR) {
	  sprintf (complaint, "OpenFile failed: %ld", GetLastError());
	  PyErr_SetString(mmapfile_module_error, complaint);
	  return NULL;
	}
  }

  m_obj = PyObject_NEW (mmapfile_object, &mmapfile_object_type);
	
  if (fh) {
	m_obj->file_handle = fh;
	if (!map_size) {
	  m_obj->size = GetFileSize ((HANDLE)fh, NULL);
	} else {
	  m_obj->size = map_size;
	}
  }
  else {
	m_obj->file_handle = (HFILE) 0xFFFFFFFF;
	m_obj->size = map_size;
  }

  // set the initial position
  m_obj->pos = (size_t) 0;

  m_obj->map_handle = CreateFileMapping ((HANDLE) m_obj->file_handle,
										 NULL,
										 PAGE_READWRITE,
										 0,
										 m_obj->size,
										 tagname);
  if (m_obj->map_handle != NULL) {
	m_obj->data = (char *) MapViewOfFile (m_obj->map_handle,
										  FILE_MAP_WRITE,
										  0,
										  0,
										  0);
	if (m_obj->data != NULL) {
	  return ((PyObject *) m_obj);
	} else {
	  sprintf (complaint, "MapViewOfFile failed: %ld", GetLastError());
	}
  } else {
	sprintf (complaint, "CreateFileMapping failed: %ld", GetLastError());
  }
  PyErr_SetString (mmapfile_module_error, complaint);
  return (NULL);
}

// List of functions exported by this module
static struct PyMethodDef mmapfile_functions[] = {
	{"mmapfile",		(PyCFunction) new_mmapfile_object},
	{NULL,			NULL}		 // Sentinel
};

extern"C" __declspec(dllexport) void
initmmapfile(void)
{
	PyObject *dict, *module;
	module = Py_InitModule ("mmapfile", mmapfile_functions);
	if (!module) /* Eeek - some serious error! */
		return;
	dict = PyModule_GetDict (module);
	if (!dict) return; /* Another serious error!*/
	mmapfile_module_error = PyString_FromString ("mmapfile error");
	PyDict_SetItemString (dict, "error", mmapfile_module_error);
}
