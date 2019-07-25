// -*- Mode: C++; tab-width: 4 -*-
//	Author: Sam Rushing <rushing@nightmare.com>
//	$Id$

// mmapfilemodule.cpp -- map a view of a file into memory
//
// todo: need permission flags, perhaps a 'chsize' analog
//
// The latest version of mmapfile is maintained by Sam at
// ftp://squirl.nightmare.com/pub/python/python-ext
// Above url is 404

// @doc - Contains comments for autoduck documentation

#include "PyWinTypes.h"

typedef struct {
    PyObject_HEAD HANDLE map_handle;
    HANDLE file_handle;
    char *data;
    size_t size;
    size_t pos;
    TCHAR *tagname;
    // File mapping size can be 64-bits even on win32, and does not have to span entire file
    ULARGE_INTEGER mapping_size;
    ULARGE_INTEGER offset;
    // Status returned by GetLastError after CreateFileMapping, so we can tell if an existing mapping was opened
    // ??? Should probably expose this as an attribute ???
    DWORD creation_status;
} mmapfile_object;

static void mmapfile_object_dealloc(mmapfile_object *m_obj)
{
    if (m_obj->data != NULL)
        UnmapViewOfFile(m_obj->data);
    if (m_obj->map_handle != NULL)
        CloseHandle(m_obj->map_handle);
    if (m_obj->file_handle != INVALID_HANDLE_VALUE)
        CloseHandle(m_obj->file_handle);
    PyWinObject_FreeTCHAR(m_obj->tagname);
    PyObject_Del(m_obj);
}

// @pymethod |Pymmapfile|close|Closes the file mapping handle and releases mapped view
static PyObject *mmapfile_close_method(mmapfile_object *self, PyObject *args)
{
    if (self->data != NULL)
        UnmapViewOfFile(self->data);
    if (self->map_handle != NULL)
        CloseHandle(self->map_handle);
    if (self->file_handle != INVALID_HANDLE_VALUE)
        CloseHandle(self->file_handle);
    self->data = NULL;
    self->map_handle = NULL;
    self->file_handle = INVALID_HANDLE_VALUE;

    Py_INCREF(Py_None);
    return (Py_None);
}

#define CHECK_VALID                                                          \
    do {                                                                     \
        if (!self->map_handle) {                                             \
            PyErr_SetString(PyExc_ValueError, "mmapfile closed or invalid"); \
            return NULL;                                                     \
        }                                                                    \
    } while (0)

// @pymethod str|Pymmapfile|read_byte|Reads a single character from current pos
static PyObject *mmapfile_read_byte_method(mmapfile_object *self, PyObject *args)
{
    char *where = (self->data + self->pos);
    CHECK_VALID;
    if ((where >= 0) && (where < (self->data + self->size))) {
        PyObject *ret = PyString_FromStringAndSize(where, 1);
        if (ret)
            self->pos += 1;
        return ret;
    }
    PyErr_SetString(PyExc_ValueError, "read byte out of range");
    return NULL;
}

// @pymethod str|Pymmapfile|read_line|Reads data from current pos up to next EOL.
static PyObject *mmapfile_read_line_method(mmapfile_object *self, PyObject *args)
{
    char *start;
    char *eof = self->data + self->size;
    char *eol;

    CHECK_VALID;
    start = self->data + self->pos;

    // strchr was a bad idea here - there's no way to range
    // check it.  there is no 'strnchr'
    for (eol = start; (eol < eof) && (*eol != '\n'); eol++) { /* do nothing */
    }

    PyObject *result = PyString_FromStringAndSize(start, (++eol - start));
    if (result)
        self->pos += (eol - start);
    return (result);
}

// @pymethod str|Pymmapfile|read|Returns specified number of bytes from buffer, and advances current position
static PyObject *mmapfile_read_method(mmapfile_object *self, PyObject *args)
{
    size_t num_bytes;
    PyObject *obnum_bytes;
    CHECK_VALID;
    if (!PyArg_ParseTuple(args, "O",
                          &obnum_bytes))  // @pyparm int|num_bytes||Number of bytes to read
        return NULL;
    num_bytes = PyInt_AsSsize_t(obnum_bytes);
    if (num_bytes == -1 && PyErr_Occurred())
        return NULL;

    // silently 'adjust' out-of-range requests
    if ((self->pos + num_bytes) > self->size)
        num_bytes -= (self->pos + num_bytes) - self->size;

    PyObject *result = PyString_FromStringAndSize(self->data + self->pos, num_bytes);
    if (result)
        self->pos += num_bytes;
    return (result);
}

// @pymethod int|Pymmapfile|find|Finds a string in the buffer.
// @rdesc Returns pos of string, or -1 if not found
static PyObject *mmapfile_find_method(mmapfile_object *self, PyObject *args)
{
    size_t start = self->pos;
    char *needle;
    Py_ssize_t len;
    PyObject *obneedle, *obstart = Py_None;
    CHECK_VALID;
    if (!PyArg_ParseTuple(
            args, "O|O",
            &obneedle,  // @pyparm str|needle||String to be located
            &obstart))  // @pyparm int|start||Pos at which to start search, current pos assumed if not specified
        return NULL;
    if (PyString_AsStringAndSize(obneedle, &needle, &len) == -1)
        return NULL;

    if (obstart != Py_None) {
        start = PyInt_AsSsize_t(obstart);
        if (start == -1 && PyErr_Occurred())
            return NULL;
    }

    char *p = self->data + self->pos;
    char *e = self->data + self->size;
    while (p < e) {
        char *s = p;
        char *n = needle;
        while ((s < e) && (*n) && !(*s - *n)) {
            s++, n++;
        }
        if (!*n) {
            return PyLong_FromLongLong(p - (self->data + start));
        }
        p++;
    }
    return PyInt_FromLong(-1);
}

// @pymethod |Pymmapfile|write|Places data at current pos in buffer.
static PyObject *mmapfile_write_method(mmapfile_object *self, PyObject *args)
{
    Py_ssize_t length;
    char *data;
    PyObject *obdata;
    CHECK_VALID;
    if (!PyArg_ParseTuple(args, "O",
                          &obdata))  // @pyparm str|data||Data to be written
        return NULL;
    if (PyString_AsStringAndSize(obdata, &data, &length) == -1)
        return NULL;

    if ((self->pos + length) > self->size) {
        PyErr_SetString(PyExc_ValueError, "data out of range");
        return NULL;
    }
    memcpy(self->data + self->pos, data, length);
    self->pos = self->pos + length;
    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod |Pymmapfile|write_byte|Writes a single character of data
static PyObject *mmapfile_write_byte_method(mmapfile_object *self, PyObject *args)
{
    char value;

    CHECK_VALID;
    if (!PyArg_ParseTuple(args, "c:write_byte",
                          &value))  // @pyparm str|char||Single byte to be placed in buffer
        return NULL;

    // read and write methods can leave pos = size, technically past end of buffer
    if (self->pos < self->size) {
        *(self->data + self->pos) = value;
        self->pos += 1;
        Py_INCREF(Py_None);
        return Py_None;
    }
    PyErr_SetString(PyExc_ValueError, "write_byte past end of buffer");
    return NULL;
}

// @pymethod long|Pymmapfile|size|Returns size of current view
static PyObject *mmapfile_size_method(mmapfile_object *self, PyObject *args)
{
    CHECK_VALID;
    // Size of buffer is not always same as file size
    return PyLong_FromUnsignedLongLong(self->size);
}

// Is this really necessary?  This could easily be done
// from python by just closing and re-opening with the
// new size?
// @pymethod |Pymmapfile|resize|Resizes the file mapping and view.
// @comm If MaximumSize is 0, only the mapped view will be affected.
// @comm Accepts keyword args.
static PyObject *mmapfile_resize_method(mmapfile_object *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"MaximumSize", "FileOffset", "NumberOfBytesToMap", NULL};
    LARGE_INTEGER new_mapping_size;
    ULARGE_INTEGER new_offset = {0, 0};
    SSIZE_T new_view_size = 0;
    PyObject *obview_size = Py_None;
    CHECK_VALID;

    if (!PyArg_ParseTupleAndKeywords(
            args, kwargs, "L|KO", keywords,
            &new_mapping_size.QuadPart,  // @pyparm long|MaximumSize||New size for file mapping. Use a multiple of
                                         // system page size (see <om win32api.GetSystemInfo>)
            &new_offset.QuadPart,        // @pyparm long|FileOffset|0|Offset into file mapping.  Must be multiple of
                                         // allocation granularity.
            &obview_size))  // @pyparm long|NumberOfBytesToMap|0|New view size.  Specify a multiple of system page size.
        return NULL;
    if (obview_size != Py_None) {
        new_view_size = PyInt_AsSsize_t(obview_size);
        if (new_view_size == -1 && PyErr_Occurred())
            return NULL;
    }

    // First, unmap the file view
    UnmapViewOfFile(self->data);
    self->data = NULL;

    /* These 2 steps are not necessary since CreateFileMapping expands file as needed,
        and this can accidentally truncate the mapped file when a smaller view is requested.
        // Move to the desired EOF position
        SetFilePointer (self->file_handle, new_size, NULL, FILE_BEGIN);
        // Change the size of the file
        SetEndOfFile (self->file_handle);
    */
    // If new mapping size isn't given, only need to create new view
    if (new_mapping_size.QuadPart) {
        // Close the mapping object
        CloseHandle(self->map_handle);
        // Create another mapping object and remap the file view
        self->map_handle = CreateFileMapping(self->file_handle, NULL, PAGE_READWRITE, new_mapping_size.HighPart,
                                             new_mapping_size.LowPart, self->tagname);
        if (self->map_handle == NULL)
            return PyWin_SetAPIError("CreateFileMapping");
        self->creation_status = GetLastError();
        self->mapping_size.QuadPart = new_mapping_size.QuadPart;
    }

    self->data =
        (char *)MapViewOfFile(self->map_handle, FILE_MAP_WRITE, new_offset.HighPart, new_offset.LowPart, new_view_size);
    if (self->data == NULL)
        return PyWin_SetAPIError("MapViewOfFile");

    // If view size not given, use VirtualQuery to determine it
    if (!new_view_size) {
        MEMORY_BASIC_INFORMATION mb;
        if (!VirtualQuery(self->data, &mb, sizeof(mb)))
            return PyWin_SetAPIError("VirtualQuery");
        self->size = mb.RegionSize;
    }
    else
        self->size = new_view_size;

    // When downsizing a view, old pos may be greater than currently allowed
    if (self->pos >= self->size)
        self->pos = self->size - 1;
    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod |Pymmapfile|flush|Flushes memory buffer to disk
static PyObject *mmapfile_flush_method(mmapfile_object *self, PyObject *args)
{
    PyObject *oboffset = Py_None, *obsize = Py_None;
    size_t offset = 0;
    size_t size = 0;
    CHECK_VALID;
    if (!PyArg_ParseTuple(
            args, "|OO",
            &oboffset,  // @pyparm int|offset|0|Position in buffer at which to flush
            &obsize))   // @pyparm int|size|0|Number of bytes to flush, 0 to flush remainder of buffer past the offset
        return NULL;
    if (oboffset != Py_None) {
        offset = PyInt_AsSsize_t(oboffset);
        if (offset == -1 && PyErr_Occurred())
            return NULL;
    }
    if (obsize != Py_None) {
        size = PyInt_AsSsize_t(obsize);
        if (size == -1 && PyErr_Occurred())
            return NULL;
    }
    if ((offset + size) > self->size) {
        PyErr_SetString(PyExc_ValueError, "flush values out of range");
        return NULL;
    }
    if (!FlushViewOfFile(self->data + offset, size))
        return PyWin_SetAPIError("FlushViewOfFile");
    // Previously the BOOL result was returned without raising an error, return 1 on success
    return PyInt_FromLong(1);
}

// @pymethod int|Pymmapfile|tell|Returns current position in buffer
static PyObject *mmapfile_tell_method(mmapfile_object *self, PyObject *args)
{
    CHECK_VALID;
    return PyLong_FromLongLong(self->pos);
}

// @pymethod |Pymmapfile|seek|Changes current position
static PyObject *mmapfile_seek_method(mmapfile_object *self, PyObject *args)
{
    size_t dist;
    PyObject *obdist;
    int how = 0;
    CHECK_VALID;
    if (!PyArg_ParseTuple(args, "O|i",
                          &obdist,  // @pyparm int|dist||Distance to seek
                          &how))    // @pyparm int|how|0|Pos from which to seek
        return (NULL);
    dist = PyInt_AsSsize_t(obdist);
    if (dist == -1 && PyErr_Occurred())
        return NULL;

    // @flagh how|meaning
    size_t where;
    switch (how) {
        // @flag 0|Seek from start of buffer
        case 0:
            where = dist;
            break;
        // @flag 1|Seek from current position
        case 1:
            where = self->pos + dist;
            break;
        // @flag 2|Seek backwards from end of buffer
        case 2:
            where = self->size - dist;
            break;
        default:
            PyErr_SetString(PyExc_ValueError, "unknown seek type");
            return NULL;
    }
    if ((where >= 0) && (where < (self->size))) {
        self->pos = where;
        Py_INCREF(Py_None);
        return (Py_None);
    }
    PyErr_SetString(PyExc_ValueError, "seek out of range");
    return NULL;
}

// @pymethod |Pymmapfile|move|Moves data from one place in buffer to another
static PyObject *mmapfile_move_method(mmapfile_object *self, PyObject *args)
{
    size_t dest, src, count;
    PyObject *obdest, *obsrc, *obcount;
    CHECK_VALID;
    if (!PyArg_ParseTuple(args, "OOO",
                          &obdest,    // @pyparm int|dest||Destination position in buffer
                          &obsrc,     // @pyparm int|src||Source position in buffer
                          &obcount))  // @pyparm int|count||Number of bytes to move
        return NULL;
    dest = PyInt_AsSsize_t(obdest);
    if (dest == (size_t)-1 && PyErr_Occurred())
        return NULL;
    src = PyInt_AsSsize_t(obsrc);
    if (src == (size_t)-1 && PyErr_Occurred())
        return NULL;
    count = PyInt_AsSsize_t(obcount);
    if (count == (size_t)-1 && PyErr_Occurred())
        return NULL;

    // bounds check the values
    if (  // end of source after end of data??
        ((src + count) > self->size)
        // dest will fit?
        || (dest + count > self->size)) {
        PyErr_SetString(PyExc_ValueError, "source or destination out of range");
        return NULL;
    }
    memmove(self->data + dest, self->data + src, count);
    Py_INCREF(Py_None);
    return Py_None;
}

// @object Pymmapfile|Object that provides access to memory-mapped file operations.
static struct PyMethodDef mmapfile_object_methods[] = {
    // @pymeth close|Closes the file mapping handle and releases mapped view
    {"close", (PyCFunction)mmapfile_close_method, METH_NOARGS},
    // @pymeth find|Finds a string in the buffer.
    {"find", (PyCFunction)mmapfile_find_method, METH_VARARGS},
    // @pymeth flush|Flushes memory buffer to disk
    {"flush", (PyCFunction)mmapfile_flush_method, METH_VARARGS},
    // @pymeth move|Moves data from one place in buffer to another
    {"move", (PyCFunction)mmapfile_move_method, METH_VARARGS},
    // @pymeth read|Returns specified number of bytes from buffer, and advances current position
    {"read", (PyCFunction)mmapfile_read_method, METH_VARARGS},
    // @pymeth read_byte|Reads a single character from current pos
    {"read_byte", (PyCFunction)mmapfile_read_byte_method, METH_NOARGS},
    // @pymeth read_line|Reads data from current pos up to next EOL.
    {"readline", (PyCFunction)mmapfile_read_line_method, METH_NOARGS},
    // @pymeth resize|Resizes the file mapping and view
    {"resize", (PyCFunction)mmapfile_resize_method, METH_KEYWORDS | METH_VARARGS},
    // @pymeth seek|Changes current position
    {"seek", (PyCFunction)mmapfile_seek_method, METH_VARARGS},
    // @pymeth size|Returns size of file mapping
    {"size", (PyCFunction)mmapfile_size_method, METH_NOARGS},
    // @pymeth tell|Returns current position in buffer
    {"tell", (PyCFunction)mmapfile_tell_method, METH_NOARGS},
    // @pymeth write|Places data at current pos in buffer.
    {"write", (PyCFunction)mmapfile_write_method, METH_VARARGS},
    // @pymeth write_byte|Writes a single character of data
    {"write_byte", (PyCFunction)mmapfile_write_byte_method, METH_VARARGS},
    {NULL, NULL} /* sentinel */
};

static PyTypeObject mmapfile_object_type = {
    PYWIN_OBJECT_HEAD "mmapfile",             // tp_name
    sizeof(mmapfile_object),                  // tp_size
    0,                                        // tp_itemsize
    (destructor)mmapfile_object_dealloc,      // tp_dealloc
    0,                                        // tp_print
    0,                                        // tp_getatt
    0,                                        // tp_setattr
    0,                                        // tp_compare
    0,                                        // tp_repr
    0,                                        // tp_as_number
    0,                                        /* tp_as_sequence */
    0,                                        /* tp_as_mapping */
    0,                                        /* tp_hash */
    0,                                        /* tp_call */
    0,                                        /* tp_str */
    PyObject_GenericGetAttr,                  /* tp_getattro */
    0,                                        /* tp_setattro */
    0,                                        /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags */
    0,                                        /* tp_doc */
    0,                                        /* tp_traverse */
    0,                                        /* tp_clear */
    0,                                        /* tp_richcompare */
    0,                                        /* tp_weaklistoffset */
    0,                                        /* tp_iter */
    0,                                        /* tp_iternext */
    mmapfile_object_methods,                  /* tp_methods */
    0,                                        /* tp_members */
    0,                                        /* tp_getset */
    0,                                        /* tp_base */
    0,                                        /* tp_dict */
    0,                                        /* tp_descr_get */
    0,                                        /* tp_descr_set */
    0,                                        /* tp_dictoffset */
    0,                                        /* tp_init */
    0,                                        /* tp_alloc */
    0,                                        /* tp_new */
};

// @pymethod <o Pymmapfile>|mmapfile|mmapfile|Creates or opens a memory mapped file.
//	This method uses the following API functions: CreateFileMapping, MapViewOfFile, VirtualQuery
// @comm Accepts keyword args.
// @pyseeapi CreateFileMapping
// @pyseeapi MapViewOfFile
// @pyseeapi VirtualQuery
static PyObject *new_mmapfile_object(PyObject *self, PyObject *args, PyObject *kwargs)
{
    mmapfile_object *m_obj;
    TCHAR *filename;
    PyObject *obfilename, *obtagname, *obview_size = Py_None;
    PSECURITY_ATTRIBUTES psa = NULL;  // Not accepted as a parameter yet

    m_obj = PyObject_New(mmapfile_object, &mmapfile_object_type);
    if (m_obj == NULL)
        return NULL;
    m_obj->file_handle = INVALID_HANDLE_VALUE;
    m_obj->map_handle = NULL;
    m_obj->data = NULL;
    m_obj->pos = 0;
    m_obj->size = 0;
    m_obj->mapping_size.QuadPart = 0;
    m_obj->offset.QuadPart = 0;
    m_obj->tagname = NULL;
    m_obj->creation_status = 0;

    static char *keywords[] = {"File", "Name", "MaximumSize", "FileOffset", "NumberOfBytesToMap", NULL};
    if (!PyArg_ParseTupleAndKeywords(
            args, kwargs, "OO|KKO", keywords,
            &obfilename,  // @pyparm str|File||Name of file.  Use None or '' when opening an existing named mapping, or
                          // to use system pagefile.
            &obtagname,   // @pyparm str|Name||Name of mapping object to create or open, can be None
            &m_obj->mapping_size
                 .QuadPart,  // @pyparm int|MaximumSize|0|Size of file mapping to create, should be specified as a
                             // multiple of system page size (see <om win32api.GetSystemInfo>).  Defaults to size of
                             // existing file if 0. If an existing named mapping is opened, the returned object will
                             // have the same size as the original mapping.
            &m_obj->offset.QuadPart,  // @pyparm int|FileOffset|0|Offset into the file at which to create view.  This
                                      // should be specified as a multiple of system allocation granularity. (see <om
                                      // win32api.GetSystemInfo>)
            &obview_size)) {  // @pyparm int|NumberOfBytesToMap|0|Size of view to create, also a multiple of system page
                              // size. If 0, view will span from offset to end of file mapping.
        Py_DECREF(m_obj);
        return NULL;
    }

    if (!PyWinObject_AsTCHAR(obtagname, &m_obj->tagname, TRUE)) {
        Py_DECREF(m_obj);
        return NULL;
    }
    if (!PyWinObject_AsTCHAR(obfilename, &filename, TRUE)) {
        Py_DECREF(m_obj);
        return NULL;
    }
    if (obview_size != Py_None) {
        m_obj->size = PyInt_AsSsize_t(obview_size);
        if (m_obj->size == -1 && PyErr_Occurred()) {
            Py_DECREF(m_obj);
            PyWinObject_FreeTCHAR(filename);
            return NULL;
        }
    }

    // if an actual filename has been specified
    if (filename && _tcslen(filename)) {
        m_obj->file_handle = CreateFile(filename, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, psa,
                                        OPEN_ALWAYS, 0, NULL);
        if (m_obj->file_handle == INVALID_HANDLE_VALUE) {
            Py_DECREF(m_obj);
            PyWinObject_FreeTCHAR(filename);
            return PyWin_SetAPIError("CreateFile");
        }
    }
    PyWinObject_FreeTCHAR(filename);

    // If mapping size was not specified, use existing file size
    if ((!m_obj->mapping_size.QuadPart) && (m_obj->file_handle != INVALID_HANDLE_VALUE)) {
        m_obj->mapping_size.LowPart = GetFileSize(m_obj->file_handle, &m_obj->mapping_size.HighPart);
        if (m_obj->mapping_size.LowPart == INVALID_FILE_SIZE) {
            DWORD err = GetLastError();
            if (err != NO_ERROR) {
                Py_DECREF(m_obj);
                return PyWin_SetAPIError("GetFileSize", err);
            }
        }
        // Must specify either mapping size or a non-empty file
        if (!m_obj->mapping_size.QuadPart) {
            PyErr_SetString(PyExc_ValueError, "Specified file is empty, and no mapping size given");
            return NULL;
        }
        // Round file size up to a multiple of system page size
        SYSTEM_INFO si;
        GetSystemInfo(&si);
        if (m_obj->mapping_size.QuadPart % si.dwPageSize)
            m_obj->mapping_size.QuadPart += si.dwPageSize - (m_obj->mapping_size.QuadPart % si.dwPageSize);
    }

    m_obj->map_handle = CreateFileMapping(m_obj->file_handle, psa, PAGE_READWRITE, m_obj->mapping_size.HighPart,
                                          m_obj->mapping_size.LowPart, m_obj->tagname);
    if (m_obj->map_handle == NULL) {
        Py_DECREF(m_obj);
        return PyWin_SetAPIError("CreateFileMapping");
    }

    /* ??? If an existing named mapping was opened, but a filename was also specified,
            we've created a superfluous HANDLE that needs to be closed.  Should probably also delete
            the file if one was created.  Maybe issue a warning also ???
    */
    m_obj->creation_status = GetLastError();
    if (m_obj->creation_status == ERROR_ALREADY_EXISTS)
        if (m_obj->file_handle != INVALID_HANDLE_VALUE) {
            CloseHandle(m_obj->file_handle);
            m_obj->file_handle = INVALID_HANDLE_VALUE;
        }

    m_obj->data = (char *)MapViewOfFile(m_obj->map_handle, FILE_MAP_WRITE, m_obj->offset.HighPart,
                                        m_obj->offset.LowPart, m_obj->size);
    if (m_obj->data == NULL) {
        Py_DECREF(m_obj);
        return PyWin_SetAPIError("MapViewOfFile");
    }

    // If view size was not given, use VirtualQuery to determine actual memory available in view
    if (!m_obj->size) {
        MEMORY_BASIC_INFORMATION mb;
        if (!VirtualQuery(m_obj->data, &mb, sizeof(mb))) {
            Py_DECREF(m_obj);
            return PyWin_SetAPIError("VirtualQuery");
        }
        m_obj->size = mb.RegionSize;
    }
    return ((PyObject *)m_obj);
}

// @module mmapfile|Compiled extension module that provides access to the memory mapped file API
static struct PyMethodDef mmapfile_functions[] = {
    // @pymeth mmapfile|Creates or opens a file mapping, and maps a view into memory
    {"mmapfile", (PyCFunction)new_mmapfile_object, METH_KEYWORDS | METH_VARARGS,
     "Pymmapfile=mmapfile(File,Name,MaximumSize=0,FileOffset=0,NumberOfBytesToMap=0)  Creates a memory mapped file "
     "view"},
    {NULL, NULL}  // Sentinel
};

PYWIN_MODULE_INIT_FUNC(mmapfile)
{
    PYWIN_MODULE_INIT_PREPARE(mmapfile, mmapfile_functions,
                              "Compiled extension module that provides access to the memory mapped file API");

    if (PyDict_SetItemString(dict, "error", PyWinExc_ApiError) == -1)
        PYWIN_MODULE_INIT_RETURN_ERROR;
    if (PyType_Ready(&mmapfile_object_type) == -1)
        PYWIN_MODULE_INIT_RETURN_ERROR;

    PYWIN_MODULE_INIT_RETURN_SUCCESS;
}
