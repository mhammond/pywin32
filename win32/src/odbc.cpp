/*
  odbc.cpp
  $Id$

  Donated to the Python community by EShop, who can not
  support it!

  Note that this can be built on non-Windows systems by a C (not C++)
  compiler, so should avoid C++ constructs and comments.

 */
/* @doc - this file contains autoduck documentation in the comments. */
#include <math.h>
#include <limits.h>
#include <string.h>

#include "PyWinTypes.h"
#include "PyWinObjects.h"
#include "structmember.h"

#include <sql.h>
#include <sqlext.h>

#include <import.h>

#include <time.h>

#ifndef _cplusplus
#define bool int
#define true 1
#define false 0
#endif

// #ifdef _WIN64
// # define mktime _mktime32
// #endif

static PyObject *datetime_module, *datetime_class;

// Type names
static PyObject *DbiString, *DbiRaw, *DbiNumber, *DbiDate;
// Exceptions
static PyObject *odbcError;
static PyObject *DbiNoError, *DbiOpError, *DbiProgError;
static PyObject *DbiIntegrityError, *DbiDataError, *DbiInternalError;

#define MAX_STR 256
static HENV Env;

typedef struct {
    PyObject_HEAD HDBC hdbc;
    int connected;
    int connect_id;
    TCHAR *connectionString;
    PyObject *connectionError;
} connectionObject;

static connectionObject *connection(PyObject *o) { return (connectionObject *)o; }

typedef PyObject *(*CopyFcn)(const void *, SQLLEN);

typedef struct _out {
    struct _out *next;
    SQLLEN rcode;
    void *bind_area;
    CopyFcn copy_fcn;
    bool bGetData;
    short vtype;
    int pos;
    SQLLEN vsize;
} OutputBinding;

typedef struct _in {
    struct _in *next;
    SQLLEN len;
    SQLLEN sqlBytesAvailable;
    bool bPutData;
    char bind_area[1];
} InputBinding;

typedef struct {
    PyObject_HEAD HSTMT hstmt;
    OutputBinding *outputVars;
    InputBinding *inputVars;
    long max_width;
    connectionObject *my_conx;
    int connect_id;
    PyObject *description;
    PyObject *cursorError;
    int n_columns;
} cursorObject;

static cursorObject *cursor(PyObject *o) { return (cursorObject *)o; }

static void cursorDealloc(PyObject *self);
PyMethodDef cursorMethods[];
PyMemberDef cursorMembers[];

static PyTypeObject Cursor_Type = {
    PYWIN_OBJECT_HEAD "odbccur", /*tp_name */
    sizeof(cursorObject),        /*tp_basicsize */
    0,                           /*tp_itemsize */
    cursorDealloc,               /*tp_dealloc */
    0,                           /*tp_print */
    0,                           /*tp_getattr */
    0,                           /*tp_setattr */
    0,                           /*tp_compare */
    0,                           /*tp_repr */
    0,                           /*tp_as_number */
    0,                           /* tp_as_sequence */
    0,                           /* tp_as_mapping */
    0,                           /* tp_hash */
    0,                           /* tp_call */
    0,                           /*tp_str */
    PyObject_GenericGetAttr,     /* tp_getattro dbiGetAttr */
    PyObject_GenericSetAttr,     /* tp_setattro */
    0,                           /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT,          /* tp_flags */
    0,                           /* tp_doc */
    0,                           /* tp_traverse */
    0,                           /* tp_clear */
    0,                           /* tp_richcompare */
    0,                           /* tp_weaklistoffset */
    0,                           /* tp_iter */
    0,                           /* tp_iternext */
    cursorMethods,               /* tp_methods */
    cursorMembers,               /* tp_members */
    0,                           /* tp_getset */
    0,                           /* tp_base */
    0,                           /* tp_dict */
    0,                           /* tp_descr_get */
    0,                           /* tp_descr_set */
    0,                           /* tp_dictoffset */
    0,                           /* tp_init */
    0,                           /* tp_alloc */
    0,                           /* tp_new */
};

static void connectionDealloc(PyObject *self);
PyMethodDef connectionMethods[];
PyMemberDef connectionMembers[];
static PyTypeObject Connection_Type = {
    PYWIN_OBJECT_HEAD "odbcconn", /*tp_name */
    sizeof(connectionObject),     /*tp_basicsize */
    0,                            /*tp_itemsize */
    connectionDealloc,            /*tp_dealloc */
    0,                            /*tp_print */
    0,                            /*tp_getattr */
    0,                            /*tp_setattr */
    0,                            /*tp_compare */
    0,                            /*tp_repr */
    0,                            /*tp_as_number */
    0,                            /* tp_as_sequence */
    0,                            /* tp_as_mapping */
    0,                            /* tp_hash */
    0,                            /* tp_call */
    0,                            /*tp_str */
    PyObject_GenericGetAttr,      /* tp_getattro dbiGetAttr */
    PyObject_GenericSetAttr,      /* tp_setattro */
    0,                            /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT,           /* tp_flags */
    0,                            /* tp_doc */
    0,                            /* tp_traverse */
    0,                            /* tp_clear */
    0,                            /* tp_richcompare */
    0,                            /* tp_weaklistoffset */
    0,                            /* tp_iter */
    0,                            /* tp_iternext */
    connectionMethods,            /* tp_methods */
    connectionMembers,            /* tp_members */
    0,                            /* tp_getset */
    0,                            /* tp_base */
    0,                            /* tp_dict */
    0,                            /* tp_descr_get */
    0,                            /* tp_descr_set */
    0,                            /* tp_dictoffset */
    0,                            /* tp_init */
    0,                            /* tp_alloc */
    0,                            /* tp_new */
};

static int unsuccessful(RETCODE rc) { return (rc != SQL_SUCCESS) && (rc != SQL_SUCCESS_WITH_INFO); }

int connectionDied(const char *sqlState) { return !strcmp(sqlState, "08S01"); }

typedef struct {
    const TCHAR *state;
    int index;
    int connected;
} odbcErrorDesc;

static odbcErrorDesc *lookupError(const TCHAR *sqlState);
static PyObject *dbiErrors[6]; /* 'cause I know about six DBI errors */

static void odbcPrintError(HENV env, connectionObject *conn, HSTMT cur, const TCHAR *action)
{
    TCHAR sqlState[256];
    long nativeError;
    short pcbErrorMsg;
    TCHAR errorMsg[1000];
    PyObject *error;

    if (unsuccessful(SQLError(env, conn ? conn->hdbc : 0, cur, (SQLTCHAR *)sqlState, &nativeError, (SQLTCHAR *)errorMsg,
                              sizeof(errorMsg) / sizeof(errorMsg[0]), &pcbErrorMsg))) {
        error = odbcError;
        _tcscpy(errorMsg, _T("Could not find error "));
    }
    else {
        _tcscat(errorMsg, _T(" in "));
        _tcscat(errorMsg, action);

        odbcErrorDesc *errorType = lookupError(sqlState);

        if (conn && errorType && (errorType->connected == 0)) {
            SQLDisconnect(conn->hdbc);
            conn->connected = 0;
        }

        /* internal is the default */
        int errn = errorType ? errorType->index : 5;
        error = dbiErrors[errn];
    }

    PyErr_SetObject(error, PyWinObject_FromTCHAR(errorMsg));
}

static void connectionError(connectionObject *conn, const TCHAR *action)
{
    odbcPrintError(Env, conn, SQL_NULL_HSTMT, action);
}

static void cursorError(cursorObject *cur, const TCHAR *action)
{
    odbcPrintError(Env, cur->my_conx, cur->hstmt, action);
}

static int doConnect(connectionObject *conn)
{
    RETCODE rc;
    short connectionStringLength;
    Py_BEGIN_ALLOW_THREADS rc = SQLDriverConnect(conn->hdbc, NULL, (SQLTCHAR *)conn->connectionString, SQL_NTS, NULL, 0,
                                                 &connectionStringLength, SQL_DRIVER_NOPROMPT);
    Py_END_ALLOW_THREADS if (unsuccessful(rc))
    {
        odbcPrintError(Env, conn, SQL_NULL_HSTMT, _T("LOGIN"));
        return 1;
    }
    conn->connected = 1;
    conn->connect_id++; /* perturb it so cursors know to reconnect */

    return 0;
}

static int attemptReconnect(cursorObject *cur)
{
    if ((cur->connect_id != cur->my_conx->connect_id) || (cur->my_conx->connected == 0)) {
        /* ie the cursor was made on an old connection */
        /* Do not need to free HSTMT here, since any statements attached to the connection
            are automatically invalidated when SQLDisconnect is called in odbcPrintError.
            (only place where connected is set to 0)
            SQLFreeStmt(cur->hstmt, SQL_DROP);
        */
        cur->hstmt = NULL;
        if (cur->my_conx->connected == 0) {
            /* ie the db has not been reconnected */
            if (doConnect(cur->my_conx)) {
                return 1;
            }
        }
        if (unsuccessful(SQLAllocStmt(cur->my_conx->hdbc, &cur->hstmt))) {
            connectionError(cur->my_conx, _T("REOPEN"));
            return 1;
        }
        cur->connect_id = cur->my_conx->connect_id;
        return 0;
    }

    return 0;
}

/* @pymethod |connection|setautocommit|Sets the autocommit mode. */
static PyObject *odbcSetAutoCommit(PyObject *self, PyObject *args)
{
    int c;
    connectionObject *conn;
    /* @pyparm int|c||The boolean autocommit mode. */
    if (!PyArg_ParseTuple(args, "i", &c))
        return NULL;
    conn = connection(self);
    if (c == 0) {
        if (unsuccessful(SQLSetConnectOption(conn->hdbc, SQL_AUTOCOMMIT, SQL_AUTOCOMMIT_OFF))) {
            connectionError(conn, _T("SETAUTOCOMMIT"));
            return NULL;
        }
    }
    else {
        if (unsuccessful(SQLSetConnectOption(conn->hdbc, SQL_AUTOCOMMIT, SQL_AUTOCOMMIT_ON))) {
            connectionError(conn, _T("SETAUTOCOMMIT"));
            return NULL;
        };
    }

    Py_INCREF(Py_None);
    return Py_None;
}

/* @pymethod |connection|commit|Commits a transaction. */
static PyObject *odbcCommit(PyObject *self, PyObject *args)
{
    RETCODE rc;
    Py_BEGIN_ALLOW_THREADS rc = SQLTransact(Env, connection(self)->hdbc, SQL_COMMIT);
    Py_END_ALLOW_THREADS if (unsuccessful(rc))
    {
        connectionError(connection(self), _T("COMMIT"));
        return 0;
    }
    else
    {
        Py_INCREF(Py_None);
        return Py_None;
    }
}

/* @pymethod |connection|rollback|Rollsback a transaction. */
static PyObject *odbcRollback(PyObject *self, PyObject *args)
{
    RETCODE rc;
    Py_BEGIN_ALLOW_THREADS rc = SQLTransact(Env, connection(self)->hdbc, SQL_ROLLBACK);
    Py_END_ALLOW_THREADS if (unsuccessful(rc))
    {
        connectionError(connection(self), _T("ROLLBACK"));
        return 0;
    }
    else
    {
        Py_INCREF(Py_None);
        return Py_None;
    }
}

/* @pymethod |connection|cursor|Creates a <o cursor> object */
static PyObject *odbcCursor(PyObject *self, PyObject *args)
{
    connectionObject *conn = connection(self);
    if (conn->connected == 0) {
        if (doConnect(conn)) {
            return 0;
        }
    }

    cursorObject *cur = PyObject_New(cursorObject, &Cursor_Type);
    if (cur == NULL)
        return NULL;

    cur->outputVars = 0;
    cur->inputVars = 0;
    cur->description = 0;
    cur->max_width = 65536L;
    cur->my_conx = 0;
    cur->hstmt = NULL;
    cur->cursorError = odbcError;
    Py_INCREF(odbcError);
    if (unsuccessful(SQLAllocStmt(conn->hdbc, &cur->hstmt))) {
        connectionError(cur->my_conx, _T("OPEN"));
        Py_DECREF(cur);
        return NULL;
    }
    cur->my_conx = conn;
    cur->connect_id = cur->my_conx->connect_id;
    Py_INCREF(self); /* the cursors owns a reference to the connection */
    return (PyObject *)cur;
}

/* @pymethod |connection|close|Closes the connection. */
static PyObject *odbcClose(PyObject *self, PyObject *args)
{
    Py_INCREF(Py_None);
    return Py_None;
}

/* @object connection|An object representing an ODBC connection */
static struct PyMethodDef connectionMethods[] = {
    {"setautocommit", odbcSetAutoCommit, 1}, /* @pymeth setautocommit|Sets the autocommit mode. */
    {"commit", odbcCommit, 1},               /* @pymeth commit|Commits a transaction. */
    {"rollback", odbcRollback, 1},           /* @pymeth rollback|Rollsback a transaction. */
    {"cursor", odbcCursor, 1},               /* @pymeth cursor|Creates a <o cursor> object */
    {"close", odbcClose, 1},                 /* @pymeth close|Closes the connection. */
    {0, 0}};

static PyMemberDef connectionMembers[] = {{"error", T_OBJECT, offsetof(connectionObject, connectionError), READONLY},
                                          {NULL}};

static void connectionDealloc(PyObject *self)
{
    Py_XDECREF(connection(self)->connectionError);
    SQLDisconnect(connection(self)->hdbc);
    SQLFreeConnect(connection(self)->hdbc);
    if (connection(self)->connectionString) {
        free(connection(self)->connectionString);
    }
    PyObject_Del(self);
}

static void deleteOutput(cursorObject *cur)
{
    OutputBinding *ob = cur->outputVars;
    while (ob) {
        OutputBinding *next = ob->next;
        free(ob->bind_area);
        free(ob);
        ob = next;
    }
    cur->outputVars = 0;
}

static void deleteInput(cursorObject *cur)
{
    InputBinding *ib = cur->inputVars;
    while (ib) {
        InputBinding *next = ib->next;
        /*$ free(ib->bind_area); */
        free(ib);
        ib = next;
    }
    cur->inputVars = 0;
}

static void deleteBinding(cursorObject *cur)
{
    deleteInput(cur);
    deleteOutput(cur);
}

static void cursorDealloc(PyObject *self)
{
    cursorObject *cur = cursor(self);
    /* Only free HSTMT if database connection hasn't been disconnected */
    if (cur->my_conx && cur->my_conx->connected && cur->hstmt)
        SQLFreeHandle(SQL_HANDLE_STMT, cur->hstmt);

    deleteBinding(cur);
    if (cur->my_conx) {
        Py_DECREF((PyObject *)cur->my_conx);
    }
    Py_XDECREF(cur->description);
    Py_XDECREF(cur->cursorError);
    PyObject_Del(self);
}

/* @pymethod |cursor|close|Closes the cursor */
static PyObject *odbcCurClose(PyObject *self, PyObject *args)
{
    /* @comm This method does nothing!!  I presume it should!?!?! */
    Py_INCREF(Py_None);
    return Py_None;
}

static BOOL bindOutputVar(cursorObject *cur, CopyFcn fcn, short vtype, SQLLEN vsize, int pos, bool bUseGet)
{
    OutputBinding *ob = (OutputBinding *)malloc(sizeof(OutputBinding));
    if (ob == NULL) {
        PyErr_NoMemory();
        return FALSE;
    }
    OutputBinding *current = NULL;

    ob->bGetData = bUseGet;
    ob->pos = pos;
    ob->vtype = vtype;
    ob->vsize = vsize;

    /* Stick the new column on the end of the linked list.
       We do this because we call SQLGetData() while walking the linked list.
       Some ODBC drivers require all BLOB columns to be at the end of the column list.
       So preserve the order our consumer called us with. */
    ob->next = NULL;
    if (cur->outputVars == NULL) {
        cur->outputVars = ob;
    }
    else {
        current = cur->outputVars;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = ob;
    }

    ob->copy_fcn = fcn;
    ob->bind_area = malloc(vsize);
    if (ob->bind_area == NULL) {
        PyErr_NoMemory();
        return FALSE;
    }
    ob->rcode = vsize;
    if (ob->bGetData == false) {
        if (unsuccessful(SQLBindCol(cur->hstmt, pos, vtype, ob->bind_area, vsize, &ob->rcode))) {
            cursorError(cur, _T("BIND"));
            return FALSE;
        }
    }
    return TRUE;
}

static PyObject *wcharCopy(const void *v, SQLLEN sz) { return PyWinObject_FromWCHAR((WCHAR *)v, sz / sizeof(WCHAR)); }

static PyObject *stringCopy(const void *v, SQLLEN sz) { return PyString_FromStringAndSize((char *)v, sz); }

static PyObject *longCopy(const void *v, SQLLEN sz) { return PyInt_FromLong(*(unsigned long *)v); }

static PyObject *doubleCopy(const void *v, SQLLEN sz)
{
    double d = *(double *)v;

    return (d == floor(d)) ? PyLong_FromDouble(d) : PyFloat_FromDouble(d);
}

static PyObject *dateCopy(const void *v, SQLLEN sz)
{
    const TIMESTAMP_STRUCT *dt = (const TIMESTAMP_STRUCT *)v;
    // Units for fraction is billionths, python datetime uses microseconds
    unsigned long usec = dt->fraction / 1000;
    return PyObject_CallFunction(datetime_class, "hhhhhhk", dt->year, dt->month, dt->day, dt->hour, dt->minute,
                                 dt->second, usec);
}

static PyObject *rawCopy(const void *v, SQLLEN sz)
{
    PyObject *ret = PyBuffer_New(sz);
    if (!ret)
        return NULL;
    void *buf;
    DWORD buflen;
    // Should not fail, but check anyway
    if (!PyWinObject_AsWriteBuffer(ret, &buf, &buflen)) {
        Py_DECREF(ret);
        return NULL;
    }
    memcpy(buf, v, sz);
    return ret;
}

typedef struct {
    const TCHAR *ptr;
    int parmCount;
    int parmIdx;
    int isParm;
    TCHAR state;
    TCHAR prev;
} parseContext;

static void initParseContext(parseContext *ct, const TCHAR *c)
{
    ct->state = 0;
    ct->ptr = c;
    ct->parmCount = 0;
}

static TCHAR doParse(parseContext *ct)
{
    ct->isParm = 0;
    if (ct->state == *ct->ptr) {
        ct->state = 0;
    }
    else if (ct->state == 0) {
        if ((*ct->ptr == '\'') || (*ct->ptr == '"')) {
            ct->state = *ct->ptr;
        }
        else if (*ct->ptr == '?') {
            ct->parmIdx = ct->parmCount;
            ct->parmCount++;
            ct->isParm = 1;
        }
        else if ((*ct->ptr == ':') && !isalnum(ct->prev)) {
            const TCHAR *m = ct->ptr + 1;
            int n = 0;
            while (isdigit(*m)) {
                n *= 10;
                n += *m - '0';
                m++;
            }
            if (n) {
                ct->parmIdx = n - 1;
                ct->parmCount++;
                ct->ptr = m;
                ct->isParm = 1;
                ct->prev = '0';
                return '?';
            }
        }
    }
    ct->prev = *ct->ptr;
    return *ct->ptr++;
}

static SQLLEN NTS = SQL_NTS;

static InputBinding *initInputBinding(cursorObject *cur, Py_ssize_t len)
{
    InputBinding *ib = (InputBinding *)malloc(sizeof(InputBinding) + len);
    if (ib) {
        ib->bPutData = false;
        ib->next = cur->inputVars;
        cur->inputVars = ib;
        ib->len = len;
    }
    else {
        PyErr_NoMemory();
    }
    return ib;
}

static int ibindInt(cursorObject *cur, int column, PyObject *item)
{
    int len = sizeof(long);
    long val = PyInt_AsLong(item);

    InputBinding *ib = initInputBinding(cur, len);
    if (!ib)
        return 0;

    memcpy(ib->bind_area, &val, len);

    if (unsuccessful(SQLBindParameter(cur->hstmt, column, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, len, 0,
                                      ib->bind_area, len, &ib->len))) {
        cursorError(cur, _T("input-binding"));
        return 0;
    }

    return 1;
}

static int ibindLong(cursorObject *cur, int column, PyObject *item)
{
    /* This will always be called in Py3k, so differentiate between an int
        that fits in a long, and one that requires a 64=bit datatype. */
    int len;
    InputBinding *ib;
    SQLSMALLINT ParamType = SQL_PARAM_INPUT, CType, SqlType;
    long longval = PyLong_AsLong(item);
    if (longval != -1 || !PyErr_Occurred()) {
        CType = SQL_C_LONG;
        SqlType = SQL_INTEGER;
        len = sizeof(long);
        ib = initInputBinding(cur, len);
        if (!ib)
            return 0;
        memcpy(ib->bind_area, &longval, len);
    }
    else {
        __int64 longlongval = PyLong_AsLongLong(item);
        if (longlongval == -1 && PyErr_Occurred())
            return 0;
        CType = SQL_C_SBIGINT;
        SqlType = SQL_BIGINT;
        len = sizeof(longlongval);
        ib = initInputBinding(cur, len);
        if (!ib)
            return 0;
        memcpy(ib->bind_area, &longlongval, len);
    }

    if (unsuccessful(
            SQLBindParameter(cur->hstmt, column, ParamType, CType, SqlType, len, 0, ib->bind_area, len, &ib->len))) {
        cursorError(cur, _T("input-binding"));
        return 0;
    }

    return 1;
}

static int ibindNull(cursorObject *cur, int column)
{
    static SQLLEN nl;
    /* apparently, ODBC does not read the last parameter
       until EXEC time, i.e., after this function is
       out of scope, hence nl must be static */

    nl = SQL_NULL_DATA;
    /* I don't know if ODBC resets the value of the parameter.
       It shouldn't but god knows... */

    if (unsuccessful(SQLBindParameter(cur->hstmt, column, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, 0, 0, 0, 0, &nl))) {
        cursorError(cur, _T("input-binding"));
        return 0;
    }

    return 1;
}

static int ibindDate(cursorObject *cur, int column, PyObject *item)
{
    /* Sql server apparently determines the precision and type of date based
        on length of input, according to the character size required for column
        storage.  This is completely bogus when passing a TIMESTAMP_STRUCT, whose
        length is always 16.  This apparently causes Sql Server to treat it as a
        SMALLDATETIME, and truncates seconds as well as fraction of second, and
        also limits the range of acceptable dates.
        Tell it we have enough room for 3 decimals, since this is all that
        SYSTEMTIME affords, and all that Sql Server 2005 will accept.
        Sql Server 2008 has a datetime2 with up to 7 decimals.
        Might need to use SqlDescribeCol to get length and precision to support this.
    */
    SQLLEN len = 23;  // length of character storage for yyyy-mm-dd hh:mm:ss.ddd
    assert(len >= sizeof(TIMESTAMP_STRUCT));
    InputBinding *ib = initInputBinding(cur, len);
    if (!ib)
        return 0;
    TIMESTAMP_STRUCT *dt = (TIMESTAMP_STRUCT *)ib->bind_area;
    ZeroMemory(dt, len);
    // Accept either a PyTime or datetime object
#ifndef NO_PYWINTYPES_TIME
    if (PyWinTime_CHECK(item)) {
        SYSTEMTIME st;
        if (!((PyTime *)item)->GetTime(&st))
            return 0;
        dt->year = st.wYear;
        dt->month = st.wMonth;
        dt->day = st.wDay;
        dt->hour = st.wHour;
        dt->minute = st.wMinute;
        dt->second = st.wSecond;
        // Fraction is in nanoseconds
        dt->fraction = st.wMilliseconds * 1000000;
    }
    else {
#endif  // NO_PYWINTYPES_TIME
        // Python 2.3 doesn't have C Api for datetime
        TmpPyObject timeseq = PyObject_CallMethod(item, "timetuple", NULL);
        if (timeseq == NULL)
            return 0;
        timeseq = PySequence_Tuple(timeseq);
        if (timeseq == NULL)
            return 0;
        // Last 3 items are ignored.
        PyObject *obwday, *obyday, *obdst;
        if (!PyArg_ParseTuple(timeseq, "hhh|hhhOOO:TIMESTAMP_STRUCT", &dt->year, &dt->month, &dt->day, &dt->hour,
                              &dt->minute, &dt->second, &obwday, &obyday, &obdst))
            return 0;

        TmpPyObject usec = PyObject_GetAttrString(item, "microsecond");
        if (usec == NULL)
            PyErr_Clear();
        else {
            dt->fraction = PyLong_AsUnsignedLong(usec);
            if (dt->fraction == -1 && PyErr_Occurred())
                return 0;
            // Convert to nanoseconds
            dt->fraction *= 1000;
        }
#ifndef NO_PYWINTYPES_TIME
    }
#endif  // NO_PYWINTYPES_TIME

    if (unsuccessful(SQLBindParameter(cur->hstmt, column, SQL_PARAM_INPUT, SQL_C_TIMESTAMP, SQL_TIMESTAMP, len,
                                      3,  // Decimal digits of precision, appears to be ignored for datetime
                                      ib->bind_area, len, &ib->len))) {
        cursorError(cur, _T("input-binding"));
        return 0;
    }

    return 1;
}

static int ibindRaw(cursorObject *cur, int column, PyObject *item)
{
    void *val;
    DWORD len;
    if (!PyWinObject_AsReadBuffer(item, &val, &len))
        return 0;
    InputBinding *ib = initInputBinding(cur, len);
    if (!ib)
        return 0;
    ib->bPutData = true;

    memcpy(ib->bind_area, val, len);

    RETCODE rc = SQL_SUCCESS;
    ib->sqlBytesAvailable = SQL_LEN_DATA_AT_EXEC(ib->len);
    rc = SQLBindParameter(cur->hstmt, column, SQL_PARAM_INPUT, SQL_C_BINARY, SQL_LONGVARBINARY, len, 0, ib->bind_area,
                          len, &ib->len);
    if (unsuccessful(rc)) {
        cursorError(cur, _T("input-binding"));
        return 0;
    }

    return 1;
}

static int ibindFloat(cursorObject *cur, int column, PyObject *item)
{
    double d = PyFloat_AsDouble(item);
    InputBinding *ib = initInputBinding(cur, sizeof(double));

    if (!ib)
        return NULL;

    memcpy(ib->bind_area, &d, ib->len);

    if (unsuccessful(SQLBindParameter(cur->hstmt, column, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DOUBLE, 15, 0,
                                      ib->bind_area, sizeof(double), &ib->len))) {
        cursorError(cur, _T("input-binding"));
        return 0;
    }

    return 1;
}

static int ibindString(cursorObject *cur, int column, PyObject *item)
{
    const char *val = PyString_AsString(item);
    size_t len = strlen(val);

    InputBinding *ib = initInputBinding(cur, len);
    if (!ib)
        return 0;

    strcpy(ib->bind_area, val);
    int sqlType = SQL_VARCHAR; /* SQL_CHAR can cause padding in some drivers.. */
    if (len > 255)             /* should remove hardcoded value and actually implement setinputsize method */
    {
        ib->sqlBytesAvailable = SQL_LEN_DATA_AT_EXEC(ib->len);
        sqlType = SQL_LONGVARCHAR;
        ib->bPutData = true;
    }
    else {
        ib->sqlBytesAvailable = ib->len;
        ib->bPutData = false;
    }

    RETCODE rc =
        SQLBindParameter(cur->hstmt, column, SQL_PARAM_INPUT, SQL_C_CHAR, sqlType, len, 0, ib->bind_area, len, &NTS);
    if (unsuccessful(rc)) {
        cursorError(cur, _T("input-binding"));
        return 0;
    }

    return 1;
}

static int ibindUnicode(cursorObject *cur, int column, PyObject *item)
{
    const WCHAR *wval = (WCHAR *)PyUnicode_AsUnicode(item);
    Py_ssize_t nchars = PyUnicode_GetSize(item) + 1;
    Py_ssize_t nbytes = nchars * sizeof(WCHAR);

    InputBinding *ib = initInputBinding(cur, nbytes);
    if (!ib)
        return 0;

    memcpy(ib->bind_area, wval, nbytes);
    /* See above re SQL_VARCHAR */
    int sqlType = SQL_WVARCHAR;
    if (nchars > 255) {
        ib->sqlBytesAvailable = SQL_LEN_DATA_AT_EXEC(ib->len);
        sqlType = SQL_WLONGVARCHAR;
        ib->bPutData = true;
    }
    else {
        ib->sqlBytesAvailable = ib->len;
        ib->bPutData = false;
    }

    RETCODE rc = SQLBindParameter(cur->hstmt, column, SQL_PARAM_INPUT, SQL_C_WCHAR, sqlType, nchars, 0, ib->bind_area,
                                  nbytes, &NTS);
    if (unsuccessful(rc)) {
        cursorError(cur, _T("input-binding"));
        return 0;
    }

    return 1;
}

static int rewriteQuery(TCHAR *out, const TCHAR *in)
{
    parseContext ctx;

    initParseContext(&ctx, in);
    while (*out++ = doParse(&ctx))
        ;
    return ctx.parmCount;
}

static int bindInput(cursorObject *cur, PyObject *vars, int columns)
{
    int i;
    PyObject *item;
    int rv;
    int iCol;

    if (columns == 0) {
        return 1;
    }

    for (i = 0; i < PySequence_Length(vars); i++) {
        item = PySequence_GetItem(vars, i);
        iCol = i + 1;
        if (PyLong_Check(item)) {
            rv = ibindLong(cur, iCol, item);
        }
        else if (PyInt_Check(item)) {
            rv = ibindInt(cur, iCol, item);
        }
        else if (PyString_Check(item)) {
            rv = ibindString(cur, iCol, item);
        }
        else if (PyUnicode_Check(item)) {
            rv = ibindUnicode(cur, iCol, item);
        }
        else if (item == Py_None) {
            rv = ibindNull(cur, iCol);
        }
        else if (PyFloat_Check(item)) {
            rv = ibindFloat(cur, iCol, item);
        }
        else if (PyWinTime_Check(item)) {
            rv = ibindDate(cur, iCol, item);
        }
#if (PY_VERSION_HEX < 0x03000000)
        else if (PyBuffer_Check(item))
#else
        else if (PyObject_CheckBuffer(item))
#endif
        {
            rv = ibindRaw(cur, iCol, item);
        }
        else {
            OutputDebugString(_T("bindInput - using repr conversion for type: '"));
            OutputDebugStringA(item->ob_type->tp_name);
            OutputDebugString(_T("'\n"));
            PyObject *sitem = PyObject_Str(item);
            if (sitem == NULL)
                rv = 0;
            else if (PyString_Check(sitem))
                rv = ibindString(cur, iCol, sitem);
            else if
                PyUnicode_Check(sitem) rv = ibindUnicode(cur, iCol, sitem);
            else {  // Just in case some object doesn't follow the rules
                PyErr_Format(PyExc_SystemError, "??? Repr for type '%s' returned type '%s' ???", item->ob_type,
                             sitem->ob_type);
                rv = 0;
            }
            Py_XDECREF(sitem);
        }
        Py_DECREF(item);
        if (rv == 0) {
            return 0;
        }
    }

    return 1;
}

static int display_size(short coltype, int collen, const TCHAR *colname)
{
    switch (coltype) {
        case SQL_CHAR:
        case SQL_VARCHAR:
        case SQL_DATE:
        case SQL_TIMESTAMP:
        case SQL_BIT:
            return (max(collen, (int)_tcslen(colname)));
        case SQL_SMALLINT:
        case SQL_INTEGER:
        case SQL_TINYINT:
            return (max(collen + 1, (int)_tcslen(colname)));
        case SQL_DECIMAL:
        case SQL_NUMERIC:
            return (max(collen + 2, (int)_tcslen(colname)));
        case SQL_REAL:
        case SQL_FLOAT:
        case SQL_DOUBLE:
            return (max(20, (int)_tcslen(colname)));
        case SQL_BINARY:
        case SQL_VARBINARY:
            return (max(2 * collen, (int)_tcslen(colname)));
        case SQL_LONGVARBINARY:
        case SQL_LONGVARCHAR:
        default:
            return (0);
    }
}

static BOOL bindOutput(cursorObject *cur)
{
    short vtype;
    SQLULEN vsize;
    TCHAR name[256];
    int pos = 1;
    short n_columns;
    SQLNumResultCols(cur->hstmt, &n_columns);
    cur->n_columns = n_columns;
    for (pos = 1; pos <= cur->n_columns; pos++) {
        PyObject *typeOf;
        long dsize;
        unsigned long prec;
        short nullok;
        short nsize;
        short scale = 0;
        SQLDescribeCol(cur->hstmt, pos, (SQLTCHAR *)name, sizeof(name) / sizeof(name[0]), &nsize, &vtype,
                       &vsize,  // This is column size in characters
                       &scale, &nullok);
        name[nsize] = 0;
        dsize = display_size(vtype, vsize, name);
        prec = 0;

        switch (vtype) {
            case SQL_BIT:
            case SQL_SMALLINT:
            case SQL_INTEGER:
            case SQL_TINYINT:
                if (!bindOutputVar(cur, longCopy, SQL_C_LONG, sizeof(unsigned long), pos, false))
                    return FALSE;
                typeOf = DbiNumber;
                break;
            case SQL_NUMERIC:
            case SQL_DECIMAL:
            case SQL_FLOAT:
            case SQL_DOUBLE:
            case SQL_REAL:
            case SQL_BIGINT:
                if (!bindOutputVar(cur, doubleCopy, SQL_C_DOUBLE, sizeof(double), pos, false))
                    return FALSE;
                typeOf = DbiNumber;
                prec = vsize;
                break;
            case SQL_DATE:
            case SQL_TIMESTAMP:
                if (!bindOutputVar(cur, dateCopy, SQL_C_TIMESTAMP, vsize, pos, false))
                    return FALSE;
                typeOf = DbiDate;
                break;
            case SQL_LONGVARBINARY:
                if (!bindOutputVar(cur, rawCopy, SQL_C_BINARY, cur->max_width, pos, true))
                    return FALSE;
                typeOf = DbiRaw;
                break;
            case SQL_BINARY:
            case SQL_VARBINARY:
                if (!bindOutputVar(cur, rawCopy, SQL_C_BINARY, cur->max_width, pos, false))
                    return FALSE;
                typeOf = DbiRaw;
                break;
            case SQL_VARCHAR:
            case SQL_WVARCHAR:
                if (!bindOutputVar(cur, wcharCopy, SQL_C_WCHAR, (vsize + 1) * sizeof(WCHAR), pos, false))
                    return FALSE;
                typeOf = DbiString;
                break;
            case SQL_LONGVARCHAR:
            case SQL_WLONGVARCHAR:
                if (!bindOutputVar(cur, wcharCopy, SQL_C_WCHAR, cur->max_width, pos, true))
                    return FALSE;
                typeOf = DbiString;
                break;
            default:
                if (!bindOutputVar(cur, stringCopy, SQL_C_CHAR, vsize + 1, pos, false))
                    return FALSE;
                typeOf = DbiString;
                break;
        }

        TmpPyObject new_tuple =
            Py_BuildValue("(NOiiiii)", PyWinObject_FromTCHAR(name), typeOf, dsize, (int)vsize, prec, scale, nullok);

        if ((new_tuple == NULL) || PyList_Append(cur->description, new_tuple) == -1)
            return FALSE;
    }

    /* success */
    return TRUE;
}

/* This lame function is here for backward compatibility with some
   very old ODBC drivers that got naively ported from Windows 3.1.
   So says: Chris Ingram [chris.ingram@synchrologic.com] */
static RETCODE sendSQLInputData(cursorObject *cur)
{
    RETCODE rc = SQL_SUCCESS;
    char *pIndx;

    Py_BEGIN_ALLOW_THREADS rc = SQLParamData(cur->hstmt, (void **)&pIndx);
    while (rc == SQL_NEED_DATA) {
        InputBinding *pInputBinding = cur->inputVars;

        /* find the input to put */
        while (pInputBinding) {
            if (pIndx != pInputBinding->bind_area) {
                pInputBinding = pInputBinding->next;
            }
            else {
                break;
            }
        }

        if (pInputBinding) {
            size_t putTimes = pInputBinding->len / 1024;
            size_t remainder = pInputBinding->len % 1024;
            rc = SQL_SUCCESS;

            if (!putTimes && !remainder) {
                rc = SQLPutData(cur->hstmt, pInputBinding->bind_area, 0);
            }
            size_t i;
            for (i = 0; i < putTimes && rc == SQL_SUCCESS; i++) {
                rc = SQLPutData(cur->hstmt, (void *)(&pInputBinding->bind_area[i * 1024]), 1024);
            }

            if (remainder && rc == SQL_SUCCESS) {
                rc = SQLPutData(cur->hstmt, (void *)(&pInputBinding->bind_area[i * 1024]), remainder);
            }
        }

        /* see if additional data is needed. */
        rc = SQLParamData(cur->hstmt, (void **)&pIndx);
    }
    Py_END_ALLOW_THREADS

        return rc;
}

/* @pymethod int|cursor|execute|Execute some SQL */
static PyObject *odbcCurExec(PyObject *self, PyObject *args)
{
    cursorObject *cur = cursor(self);
    TCHAR *sql = NULL;
    PyObject *obsql;
    TCHAR *sqlbuf;
    PyObject *inputvars = 0;
    PyObject *rv = 0;
    PyObject *rows = 0;
    SQLLEN t;
    int n_columns = 0;
    SQLLEN n_rows = 0;

    if (attemptReconnect(cur)) {
        return 0;
    }

    /* @pyparm string|sql||The SQL to execute */
    /* @pyparm sequence|[var, ...]|[]|Input variables. */
    /* If the first element is itself a sequence (other than a string)
        the input will be interpreted as a sequence of sequences to be
        used to execute the statement multiple times.
    */
    if (!PyArg_ParseTuple(args, "O|O:execute", &obsql, &inputvars)) {
        return NULL;
    }

    if (inputvars) {
        if (PyString_Check(inputvars) || PyUnicode_Check(inputvars) || !PySequence_Check(inputvars))
            return PyErr_Format(odbcError, "Values must be a sequence, not %s", inputvars->ob_type->tp_name);
        if (PySequence_Length(inputvars) > 0) {
            PyObject *temp = PySequence_GetItem(inputvars, 0);
            if (temp == NULL)
                return NULL;
            /* Strings don't count as a list in this case. */
            if (PySequence_Check(temp) && !PyString_Check(temp) && !PyUnicode_Check(temp)) {
                rows = inputvars;
                inputvars = NULL;
            }
            Py_DECREF(temp);
        }
    }
    if (!PyWinObject_AsTCHAR(obsql, &sql, FALSE))
        return NULL;

    deleteBinding(cur);

    if (cur->description) {
        Py_DECREF(cur->description);
    }
    cur->description = PyList_New(0);
    if (!cur->description) {
        PyWinObject_FreeTCHAR(sql);
        return NULL;
    }

    cur->n_columns = 0;

    sqlbuf = (TCHAR *)malloc((_tcslen(sql) + 100) * sizeof(TCHAR));
    if (!sqlbuf) {
        Py_DECREF(cur->description);
        cur->description = NULL;
        PyWinObject_FreeTCHAR(sql);
        return PyErr_NoMemory();
    }

    SQLFreeStmt(cur->hstmt, SQL_CLOSE); /* ignore errors here */
    RETCODE rc = SQL_SUCCESS;
    n_columns = rewriteQuery(sqlbuf, sql);
    Py_BEGIN_ALLOW_THREADS rc = SQLPrepare(cur->hstmt, (SQLTCHAR *)sqlbuf, SQL_NTS);
    Py_END_ALLOW_THREADS if (unsuccessful(rc))
    {
        cursorError(cur, _T("EXEC"));
        goto Error;
    }

    if (rows) {
        int i;
        /* handle insert cases... */
        for (i = 0; i < PySequence_Length(rows); i++) {
            inputvars = PySequence_GetItem(rows, i);
            if (!PySequence_Check(inputvars)) {
                PyErr_SetString(odbcError, "expected sequence of sequences for bulk inserts");
                goto Error;
            }
            if (PySequence_Length(inputvars) != n_columns) {
                PyErr_Format(odbcError, "Found an insert row that didn't have %d columns", n_columns);
                goto Error;
            }
            if (!bindInput(cur, inputvars, n_columns)) {
                goto Error;
            }
            Py_BEGIN_ALLOW_THREADS rc = SQLExecDirect(cur->hstmt, (SQLTCHAR *)sqlbuf, SQL_NTS);
            Py_END_ALLOW_THREADS
                /* move data here. */
                if (rc == SQL_NEED_DATA)
            {
                rc = sendSQLInputData(cur);
            }
            if (unsuccessful(rc)) {
                cursorError(cur, _T("EXEC"));
                goto Error;
            }
            /* Success! */
            /* Note: multiple result sets aren't supported here, just bulk inserts... */
            Py_BEGIN_ALLOW_THREADS SQLRowCount(cur->hstmt, &t);
            Py_END_ALLOW_THREADS n_rows += t;
            deleteBinding(cur);
            Py_DECREF(inputvars);
            inputvars = NULL;
        }
    }
    else {
        if (!bindInput(cur, inputvars, n_columns)) {
            goto Error;
        }
        Py_BEGIN_ALLOW_THREADS rc = SQLExecDirect(cur->hstmt, (SQLTCHAR *)sqlbuf, SQL_NTS);
        Py_END_ALLOW_THREADS if (rc == SQL_NEED_DATA) { rc = sendSQLInputData(cur); }
        if (unsuccessful(rc)) {
            cursorError(cur, _T("EXEC"));
            goto Error;
        }
        else if (!bindOutput(cur)) {
            goto Error;
        }
        /* success */
        if (cur->n_columns > 0) {
            /* it was a select */
            Py_INCREF(Py_None);
            rv = Py_None;
        }
        else {
            n_rows = 1; /* just in case it does not work */
            SQLRowCount(cur->hstmt, &n_rows);
        }
    }

    rv = PyLong_FromLongLong(n_rows);
Cleanup:
    PyWinObject_FreeTCHAR(sql);
    free(sqlbuf);
    return rv;
Error:
    Py_DECREF(cur->description);
    cur->description = NULL;
    rv = NULL;
    goto Cleanup;
}

static PyObject *processOutput(cursorObject *cur)
{
    OutputBinding *ob = cur->outputVars;
    int column = 0;
    PyObject *row = PyTuple_New(cur->n_columns);
    if (!row)
        return NULL;

    while (ob) {
        if (ob->bGetData) {
            /* Use SQLGetData to retrieve data for blob (or long varchar) type columns.
                Loop until return code indicates all remaining data fit into buffer.
            */
            RETCODE rc;
            SQLLEN cbRead = 0;
            ob->rcode = 0;
            do {
                /* Increase buffer size by cursor chunk size on second and subsequent calls
                    If not for the SQL Anywhere 5.0 problem (driver version
                    5.05.041867), we could probably grow by 50% each time
                    or the remaining size (as determined by ob->rcode).
                    Regarding above note, caller can now use cursor.setoutputsize
                    to work around any such bug in a driver */
                if (ob->rcode) {
                    void *pTemp = ob->bind_area;
                    ob->vsize += cur->max_width;
                    /* Some BLOBs can be huge, be paranoid about allowing
                       other threads to run. */
                    Py_BEGIN_ALLOW_THREADS ob->bind_area = realloc(ob->bind_area, ob->vsize);
                    Py_END_ALLOW_THREADS if (ob->bind_area == NULL)
                    {
                        PyErr_NoMemory();
                        ob->vsize -= cur->max_width;
                        ob->bind_area = pTemp;
                        Py_DECREF(row);
                        return NULL;
                    }
                }

                Py_BEGIN_ALLOW_THREADS rc = SQLGetData(cur->hstmt, ob->pos, ob->vtype, (char *)ob->bind_area + cbRead,
                                                       ob->vsize - cbRead, &ob->rcode);
                Py_END_ALLOW_THREADS if (unsuccessful(rc))
                {
                    Py_DECREF(row);
                    cursorError(cur, _T("SQLGetData"));
                    return NULL;
                }
                /* Return code can be a negative status code:
                    SQL_NO_TOTAL if length is not known, SQL_NULL_DATA if nothing to retreive
                    Otherwise will be total bytes remaining including current read.
                */
                if (ob->rcode >= 0 && ob->rcode <= ob->vsize - cbRead) {
                    /* If we get here, then this should be the last iteration through the loop. */
                    ob->rcode += cbRead;
                }
                else {
                    cbRead = ob->vsize;
                    /* We want to ignore the intermediate
                          NULL characters SQLGetData() gives us.
                           (silly, silly) */
                    if (ob->vtype == SQL_C_CHAR)
                        cbRead--;
                    else if (ob->vtype == SQL_C_WCHAR)
                        /* Buffer is not guaranteed to be an exact multiple of sizeof(WCHAR),
                            leaving an extra byte and throwing the next get off by 1. */
                        cbRead -= sizeof(WCHAR) + ob->vsize % sizeof(WCHAR);
                }

            } while (rc == SQL_SUCCESS_WITH_INFO);
        }

        PyObject *v;
        if (ob->rcode == SQL_NULL_DATA) {
            v = Py_None;
            Py_INCREF(v);
        }
        else {
            if (ob->bGetData == false) {
                v = ob->copy_fcn(ob->bind_area, (ob->rcode < cur->max_width) ? ob->rcode : cur->max_width);
            }
            else {
                v = ob->copy_fcn(ob->bind_area, ob->rcode);
            }
            if (!v) {
                Py_DECREF(row);
                return NULL;
            }
        }

        PyTuple_SET_ITEM(row, column++, v);
        ob = ob->next;
    }
    return row;
}

static PyObject *fetchOne(cursorObject *cur)
{
    RETCODE rc;
    Py_BEGIN_ALLOW_THREADS rc = SQLFetch(cur->hstmt);
    Py_END_ALLOW_THREADS if (rc == SQL_NO_DATA_FOUND)
    {
        Py_INCREF(Py_None);
        return Py_None;
    }
    else if (unsuccessful(rc))
    {
        cursorError(cur, _T("FETCH"));
        return 0;
    }
    return processOutput(cur);
}

static PyObject *fetchN(cursorObject *cur, long n_rows)
{
    int row;
    PyObject *list = PyList_New(0);
    for (row = 0; row < n_rows; row++) {
        PyObject *entry = fetchOne(cur);
        if (entry) {
            if (entry == Py_None) {
                Py_DECREF(entry);
                return list;
            }
            else {
                if (PyList_Append(list, entry) == -1) {
                    Py_DECREF(list);
                    Py_DECREF(entry);
                    return NULL;
                }
                Py_DECREF(entry);
            }
        }
        else {
            Py_DECREF(list); /* throw it away */
            return NULL;
        }
    }
    return list;
}

/* @pymethod data|cursor|fetchone|Fetch one row of data */
static PyObject *odbcCurFetchOne(PyObject *self, PyObject *args) { return fetchOne(cursor(self)); }

/* @pymethod [data, ...]|cursor|fetchmany|Fetch many rows of data */
static PyObject *odbcCurFetchMany(PyObject *self, PyObject *args)
{
    long n_rows = 1;

    if (!PyArg_ParseTuple(args, "|l", &n_rows)) {
        return NULL;
    }

    return fetchN(cursor(self), n_rows);
}

/* @pymethod [data, ...]|cursor|fetchall|Fetch all rows of data */
static PyObject *odbcCurFetchAll(PyObject *self, PyObject *args) { return fetchN(cursor(self), LONG_MAX); }

/* @pymethod |cursor|setinputsizes| */
static PyObject *odbcCurSetInputSizes(PyObject *self, PyObject *args)
{
    Py_INCREF(Py_None);
    return Py_None;
}
/* @pymethod |cursor|setoutputsize| */
static PyObject *odbcCurSetOutputSize(PyObject *self, PyObject *args)
{
    if (!PyArg_ParseTuple(args, "l", &cursor(self)->max_width)) {
        return NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

/* @object cursor|An object representing an ODBC cursor. */
static PyMethodDef cursorMethods[] = {
    {"close", odbcCurClose, 1},                 /* @pymeth close|Closes the cursor */
    {"execute", odbcCurExec, 1},                /* @pymeth execute|Execute some SQL */
    {"fetchone", odbcCurFetchOne, 1},           /* @pymeth fetchone|Fetch one row of data */
    {"fetchmany", odbcCurFetchMany, 1},         /* @pymeth fetchmany|Fetch many rows of data */
    {"fetchall", odbcCurFetchAll, 1},           /* @pymeth fetchall|Fetch all the rows of data */
    {"setinputsizes", odbcCurSetInputSizes, 1}, /* @pymeth setinputsizes| */
    {"setoutputsize", odbcCurSetOutputSize, 1}, /* @pymeth setoutputsize| */
    {0, 0}};

static PyMemberDef cursorMembers[] = {{"description", T_OBJECT, offsetof(cursorObject, description), READONLY},
                                      {"error", T_OBJECT, offsetof(cursorObject, cursorError), READONLY},
                                      {NULL}};

static void parseInfo(connectionObject *conn, const TCHAR *c)
{
    TCHAR *p;
    TCHAR buf[255];
    const TCHAR *firstEqualsSign;
    const TCHAR *firstSlash;
    TCHAR dsn[MAX_STR];
    TCHAR uid[MAX_STR];
    TCHAR pwd[MAX_STR];
    size_t connectionStringLength;

    firstEqualsSign = _tcschr(c, _T('='));
    firstSlash = _tcschr(c, _T('/'));

    if (!firstEqualsSign || (firstSlash && firstSlash < firstEqualsSign)) {
        _tcsncpy(buf, c, sizeof(buf) / sizeof(TCHAR));
        p = _tcstok(buf, _T("/"));
        if (p) {
            _tcsncpy(dsn, p, sizeof(dsn) / sizeof(TCHAR));
            p = _tcstok(0, _T("/"));
            if (p) {
                _tcsncpy(uid, p, sizeof(uid) / sizeof(TCHAR));
                p = _tcstok(0, _T("/"));
                if (p) {
                    _tcsncpy(pwd, p, sizeof(pwd) / sizeof(TCHAR));
                }
                else {
                    pwd[0] = 0;
                }
            }
            else {
                uid[0] = 0;
                pwd[0] = 0;
            }
        }
        else {
            _tcsncpy(dsn, c, sizeof(dsn));
            uid[0] = 0;
            pwd[0] = 0;
        }

        connectionStringLength = _tcslen(dsn) + _tcslen(uid) + _tcslen(pwd) + 15; /* add room for DSN=;UID=;PWD=\0 */
        conn->connectionString = (TCHAR *)malloc(connectionStringLength);
        _tcscpy(conn->connectionString, _T("DSN="));
        _tcscat(conn->connectionString, dsn);
        if (_tcslen(uid)) {
            _tcscat(conn->connectionString, _T(";UID="));
            _tcscat(conn->connectionString, uid);
        }
        if (_tcslen(pwd)) {
            _tcscat(conn->connectionString, _T(";PWD="));
            _tcscat(conn->connectionString, pwd);
        }
    }
    else {
        conn->connectionString = (TCHAR *)malloc((_tcslen(c) + 1) * sizeof(TCHAR));
        _tcscpy(conn->connectionString, c);
    }
}

/* @pymethod <o connection>|odbc|odbc|Creates an ODBC connection */
static PyObject *odbcLogon(PyObject *self, PyObject *args)
{
    TCHAR *connectionString = NULL;
    PyObject *obconnectionString;
    connectionObject *conn;

    /* @pyparm string|connectionString||An ODBC connection string.
       For backwards-compatibility, this parameter can be of the form
       DSN[/username[/password]] (e.g. "myDSN/myUserName/myPassword").
       Alternatively, a full ODBC connection string can be used (e.g.,
       "Driver={SQL Server};Server=(local);Database=myDatabase"). */
    if (!PyArg_ParseTuple(args, "O", &obconnectionString)) {
        return NULL;
    }
    if (!PyWinObject_AsTCHAR(obconnectionString, &connectionString, FALSE))
        return NULL;

    conn = PyObject_New(connectionObject, &Connection_Type);
    if (!conn) {
        PyWinObject_FreeTCHAR(connectionString);
        return NULL;
    }

    conn->connectionError = odbcError;
    Py_INCREF(odbcError);
    conn->connect_id = 0; /* initialize it to anything */
    conn->hdbc = SQL_NULL_HDBC;
    conn->connectionString = NULL;
    if (unsuccessful(SQLAllocConnect(Env, &conn->hdbc))) {
        connectionError(conn, _T("ALLOCATION"));
        Py_DECREF(conn);
        PyWinObject_FreeTCHAR(connectionString);
        return NULL;
    }

    parseInfo(conn, connectionString);

    if (doConnect(conn)) {
        PyWinObject_FreeTCHAR(connectionString);
        Py_DECREF(conn);
        return NULL;
    }
    PyWinObject_FreeTCHAR(connectionString);
    return (PyObject *)conn;
}

/* @pymethod (name, desc)/None|odbc|SQLDataSources|Enumerates ODBC data sources */
/* @rdesc The result is None when SQL_NO_DATA is returned from ODBC. */
static PyObject *odbcSQLDataSources(PyObject *self, PyObject *args)
{
    int direction;
    /* @pyparm int|direction||One of SQL_FETCH_* flags indicating how to retrieve data sources */
    if (!PyArg_ParseTuple(args, "i:SQLDataSources", &direction))
        return NULL;

    PyObject *ret;
    SQLTCHAR svr[256];
    SQLTCHAR desc[1024];
    SQLSMALLINT svr_size = sizeof(svr) / sizeof(svr[0]);
    SQLSMALLINT desc_size = sizeof(desc) / sizeof(desc[0]);
    RETCODE rc;
    Py_BEGIN_ALLOW_THREADS rc = SQLDataSources(Env, direction, svr, svr_size, &svr_size, desc, desc_size, &desc_size);
    Py_END_ALLOW_THREADS

        if (rc == SQL_NO_DATA)
    {
        ret = Py_None;
        Py_INCREF(Py_None);
    }
    else if (unsuccessful(rc))
    {
        connectionError(NULL, _T("SQLDataSources"));
        ret = NULL;
    }
    else ret = Py_BuildValue("NN", PyWinObject_FromTCHAR((TCHAR *)svr, svr_size),
                             PyWinObject_FromTCHAR((TCHAR *)desc, desc_size));
    return ret;
}

/* @module odbc|A Python wrapper around the ODBC API. */
static PyMethodDef globalMethods[] = {
    {"odbc", odbcLogon, 1},                    /* @pymeth odbc|Creates an <o connection> object. */
    {"SQLDataSources", odbcSQLDataSources, 1}, /* @pymeth SQLDataSources|Enumerates ODBC data sources. */
    {0, 0}};

#define ADD_CONSTANT(tok)                                 \
    if (PyModule_AddIntConstant(module, #tok, tok) == -1) \
        PYWIN_MODULE_INIT_RETURN_ERROR;

PYWIN_MODULE_INIT_FUNC(odbc)
{
    PYWIN_MODULE_INIT_PREPARE(odbc, globalMethods, "A Python wrapper around the ODBC API.");

    if (PyType_Ready(&Cursor_Type) == -1)
        PYWIN_MODULE_INIT_RETURN_ERROR;
    if (PyType_Ready(&Connection_Type) == -1)
        PYWIN_MODULE_INIT_RETURN_ERROR;

    // Sql dates are now returned as python's datetime object.
    //	C Api for datetime didn't exist in 2.3, stick to dynamic semantics for now.
    datetime_module = PyImport_ImportModule("datetime");
    if (datetime_module == NULL)
        PYWIN_MODULE_INIT_RETURN_ERROR;
    datetime_class = PyObject_GetAttrString(datetime_module, "datetime");
    if (datetime_class == NULL)
        PYWIN_MODULE_INIT_RETURN_ERROR;

    if (unsuccessful(SQLAllocEnv(&Env))) {
        odbcPrintError(SQL_NULL_HENV, 0, SQL_NULL_HSTMT, _T("INIT"));
        PYWIN_MODULE_INIT_RETURN_ERROR;
    }

    /* Names of various sql datatypes.
        's' format of Py_BuildValue creates unicode on py3k, and char string on 2.x
    */
    char *szDbiString = "STRING";
    char *szDbiRaw = "RAW";
    char *szDbiNumber = "NUMBER";
    char *szDbiDate = "DATE";
    PyObject *obtypes = Py_BuildValue("(ssss)", szDbiString, szDbiRaw, szDbiNumber, szDbiDate);
    // Steals a ref to obtypes, so it doesn't need to be DECREF'ed.
    if (obtypes == NULL || PyModule_AddObject(module, "TYPES", obtypes) == -1)
        PYWIN_MODULE_INIT_RETURN_ERROR;
    DbiString = PyTuple_GET_ITEM(obtypes, 0);
    DbiRaw = PyTuple_GET_ITEM(obtypes, 1);
    DbiNumber = PyTuple_GET_ITEM(obtypes, 2);
    DbiDate = PyTuple_GET_ITEM(obtypes, 3);
    /* ??? These are also added to the module with attribute name same as value,
            not sure what the point of this is ???
    */
    if (PyDict_SetItem(dict, DbiString, DbiString) == -1 || PyDict_SetItem(dict, DbiRaw, DbiRaw) == -1 ||
        PyDict_SetItem(dict, DbiNumber, DbiNumber) == -1 || PyDict_SetItem(dict, DbiDate, DbiDate) == -1)
        PYWIN_MODULE_INIT_RETURN_ERROR;

    // Initialize various exception types
    odbcError = PyErr_NewException("odbc.odbcError", NULL, NULL);
    if (odbcError == NULL || PyDict_SetItemString(dict, "error", odbcError) == -1)
        PYWIN_MODULE_INIT_RETURN_ERROR;
    DbiNoError = PyErr_NewException("dbi.noError", NULL, NULL);
    if (DbiNoError == NULL || PyDict_SetItemString(dict, "noError", DbiNoError) == -1)
        PYWIN_MODULE_INIT_RETURN_ERROR;
    DbiOpError = PyErr_NewException("dbi.opError", NULL, NULL);
    if (DbiOpError == NULL || PyDict_SetItemString(dict, "opError", DbiOpError) == -1)
        PYWIN_MODULE_INIT_RETURN_ERROR;
    DbiProgError = PyErr_NewException("dbi.progError", NULL, NULL);
    if (DbiProgError == NULL || PyDict_SetItemString(dict, "progError", DbiProgError) == -1)
        PYWIN_MODULE_INIT_RETURN_ERROR;
    DbiIntegrityError = PyErr_NewException("dbi.integrityError", NULL, NULL);
    if (DbiIntegrityError == NULL || PyDict_SetItemString(dict, "integrityError", DbiIntegrityError) == -1)
        PYWIN_MODULE_INIT_RETURN_ERROR;
    DbiDataError = PyErr_NewException("dbi.dataError", NULL, NULL);
    if (DbiDataError == NULL || PyDict_SetItemString(dict, "dataError", DbiDataError) == -1)
        PYWIN_MODULE_INIT_RETURN_ERROR;
    DbiInternalError = PyErr_NewException("dbi.internalError", NULL, NULL);
    if (DbiInternalError == NULL || PyDict_SetItemString(dict, "internalError", DbiInternalError) == -1)
        PYWIN_MODULE_INIT_RETURN_ERROR;
    /* The indices go to indices in the ODBC error table */
    dbiErrors[0] = DbiNoError;
    dbiErrors[1] = DbiOpError;
    dbiErrors[2] = DbiProgError;
    dbiErrors[3] = DbiIntegrityError;
    dbiErrors[4] = DbiDataError;
    dbiErrors[5] = DbiInternalError;

    ADD_CONSTANT(SQL_FETCH_NEXT);
    ADD_CONSTANT(SQL_FETCH_FIRST);
    ADD_CONSTANT(SQL_FETCH_LAST);
    ADD_CONSTANT(SQL_FETCH_PRIOR);
    ADD_CONSTANT(SQL_FETCH_ABSOLUTE);
    ADD_CONSTANT(SQL_FETCH_RELATIVE);
    ADD_CONSTANT(SQL_FETCH_FIRST_USER);
    ADD_CONSTANT(SQL_FETCH_FIRST_SYSTEM);

    PYWIN_MODULE_INIT_RETURN_SUCCESS;
}

static odbcErrorDesc errorTable[] = {
    {_T("01000"), 5, 0}, /* General warning */
    {_T("01002"), 1, 1}, /* Disconnect error */
    {_T("01004"), 0, 1}, /* Data truncated */
    {_T("01006"), 5, 1}, /* Privilege not revoked */
    {_T("01S00"), 2, 1}, /* Invalid connection string attribute */
    {_T("01S01"), 5, 1}, /* Error in row */
    {_T("01S02"), 5, 1}, /* Option value changed */
    {_T("01S03"), 0, 1}, /* No rows updated or deleted */
    {_T("01S04"), 0, 1}, /* More than one row updated or deleted */
    {_T("01S05"), 0, 1}, /* Cancel treated as SQLFreeStmt with the SQL_CLOSE */
    {_T("01S06"), 2, 1}, /* Attempt to fetch before the result set returned */
    {_T("07001"), 2, 1}, /* Wrong number of parameters */
    {_T("07006"), 2, 1}, /* Restricted data type attribute violation */
    {_T("07S01"), 2, 1}, /* Invalid use of default parameter */
    {_T("08001"), 1, 1}, /* Unable to connect to data source */
    {_T("08002"), 1, 1}, /* Connection in use */
    {_T("08003"), 1, 1}, /* Connection not open */
    {_T("08004"), 1, 1}, /* Data source rejected establishment of connection */
    {_T("08007"), 1, 1}, /* Connection failure during transaction */
    {_T("08S01"), 1, 0}, /* Communication link failure */
    {_T("21S01"), 2, 1}, /* Insert value list does not match column list */
    {_T("21S02"), 2, 1}, /* Degree of derived table does not match column list */
    {_T("22001"), 0, 1}, /* String data right truncation */
    {_T("22002"), 5, 1}, /* Indicator variable required but not supplied */
    {_T("22003"), 4, 1}, /* Numeric value out of range */
    {_T("22005"), 4, 1}, /* Error in assignment */
    {_T("22008"), 4, 1}, /* Datetime field overflow */
    {_T("22012"), 4, 1}, /* Division by zero */
    {_T("22026"), 4, 1}, /* String data, length mismatch */
    {_T("23000"), 3, 1}, /* Integrity constraint violation */
    {_T("24000"), 5, 1}, /* Invalid cursor state */
    {_T("25000"), 5, 1}, /* Invalid transaction state */
    {_T("28000"), 1, 1}, /* Invalid authorization specification */
    {_T("34000"), 5, 1}, /* Invalid cursor name */
    {_T("37000"), 2, 1}, /* Syntax error or access violation */
    {_T("3C000"), 5, 1}, /* Duplicate cursor name */
    {_T("40001"), 5, 1}, /* Serialization failure */
    {_T("42000"), 2, 1}, /* Syntax error or access violation */
    {_T("70100"), 1, 1}, /* Operation aborted */
    {_T("IM001"), 1, 1}, /* Driver does not support this function */
    {_T("IM002"), 1, 1}, /* Data source name not found and no default driver  */
    {_T("IM003"), 1, 1}, /* Specified driver could not be loaded */
    {_T("IM004"), 1, 1}, /* Driver's SQLAllocEnv failed */
    {_T("IM005"), 1, 1}, /* Driver's SQLAllocConnect failed */
    {_T("IM006"), 1, 1}, /* Driver's SQLSetConnect-Option failed */
    {_T("IM007"), 1, 1}, /* No data source or driver specified; dialog prohibited */
    {_T("IM008"), 1, 1}, /* Dialog failed */
    {_T("IM009"), 1, 1}, /* Unable to load translation DLL */
    {_T("IM010"), 1, 1}, /* Data source name too long */
    {_T("IM011"), 1, 1}, /* Driver name too long */
    {_T("IM012"), 1, 1}, /* DRIVER keyword syntax error */
    {_T("IM013"), 1, 1}, /* Trace file error */
    {_T("S0001"), 2, 1}, /* Base table or view already exists */
    {_T("S0002"), 2, 1}, /* Base table not found */
    {_T("S0011"), 2, 1}, /* Index already exists */
    {_T("S0012"), 2, 1}, /* Index not found */
    {_T("S0021"), 2, 1}, /* Column already exists */
    {_T("S0022"), 2, 1}, /* Column not found */
    {_T("S0023"), 2, 1}, /* No default for column */
    {_T("S1000"), 1, 1}, /* General error */
    {_T("S1001"), 1, 1}, /* Memory allocation failure */
    {_T("S1002"), 5, 1}, /* Invalid column number */
    {_T("S1003"), 5, 1}, /* Program type out of range */
    {_T("S1004"), 5, 1}, /* SQL data type out of range */
    {_T("S1008"), 1, 1}, /* Operation canceled */
    {_T("S1009"), 5, 1}, /* Invalid argument value */
    {_T("S1010"), 5, 1}, /* Function sequence error */
    {_T("S1011"), 5, 1}, /* Operation invalid at this time */
    {_T("S1012"), 5, 1}, /* Invalid transaction operation code specified */
    {_T("S1015"), 5, 1}, /* No cursor name available */
    {_T("S1090"), 5, 1}, /* Invalid string or buffer length */
    {_T("S1091"), 5, 1}, /* Descriptor type out of range */
    {_T("S1092"), 5, 1}, /* Option type out of range */
    {_T("S1093"), 5, 1}, /* Invalid parameter number */
    {_T("S1095"), 5, 1}, /* Function type out of range */
    {_T("S1096"), 5, 1}, /* Information type out of range */
    {_T("S1097"), 5, 1}, /* Column type out of range */
    {_T("S1098"), 5, 1}, /* Scope type out of range */
    {_T("S1099"), 5, 1}, /* Nullable type out of range */
    {_T("S1100"), 5, 1}, /* Uniqueness option type out of range */
    {_T("S1101"), 5, 1}, /* Accuracy option type out of range */
    {_T("S1103"), 5, 1}, /* Direction option out of range */
    {_T("S1105"), 5, 1}, /* Invalid parameter type */
    {_T("S1106"), 5, 1}, /* Fetch type out of range */
    {_T("S1107"), 5, 1}, /* Row value out of range */
    {_T("S1108"), 5, 1}, /* Concurrency option out of range */
    {_T("S1109"), 5, 1}, /* Invalid cursor position */
    {_T("S1110"), 5, 1}, /* Invalid driver completion */
    {_T("S1111"), 5, 1}, /* Invalid bookmark value */
    {_T("S1C00"), 1, 1}, /* Driver not capable */
    {_T("S1T00"), 1, 1}  /* Timeout expired */
};

static int odbcCompare(const void *v1, const void *v2)
{
    return _tcscmp(((const odbcErrorDesc *)v1)->state, ((const odbcErrorDesc *)v2)->state);
}

static odbcErrorDesc *lookupError(const TCHAR *sqlState)
{
    odbcErrorDesc key;

    key.state = sqlState;
    return (odbcErrorDesc *)bsearch(&key, errorTable, sizeof(errorTable) / sizeof(odbcErrorDesc), /* number of elems */
                                    sizeof(odbcErrorDesc), odbcCompare);
}
