/*
  odbcmodule.c

  Donated to the Python community by EShop, who can not
  support it!

 */
// @doc - this file contains autoduck documentation in the comments.
#include <math.h>
#include <limits.h>
#include <string.h>

#include <windows.h>
#include <sql.h>
#include <sqlext.h>

#include <Python.h>
#include <import.h>

#include <time.h>

#include "dbi.h"  //$ This is a hack
static PyObject *odbcError;

#define MAX_STR		45
static HENV Env;

typedef struct
{
	PyObject_HEAD
	HDBC hdbc;
	int  connected;
	int  connect_id;
	char dsn[MAX_STR];
	char uid[MAX_STR];
	char pwd[MAX_STR];
} connectionObject;

static connectionObject *connection(PyObject *o)
{
	return  (connectionObject *) o;
}

typedef PyObject * (* CopyFcn)(const void *, int);

typedef struct _out
{
	struct _out *next;
	long rcode;
	void *bind_area;
	CopyFcn copy_fcn;
	bool bGetData;
	short vtype;
	int pos;
	long vsize;
} OutputBinding;

typedef struct _in {
	struct _in *next;
	long len;
	long sqlBytesAvailable;
	bool bPutData;
	char bind_area[1];
} InputBinding;

typedef struct
{
	PyObject_HEAD
	HSTMT hstmt;
	OutputBinding *outputVars;
	InputBinding *inputVars;
	long max_width;
	connectionObject *my_conx;
	int  connect_id;
	PyObject *description;  
	int n_columns;
	bool bGetDataIsNeeded;
} cursorObject;

static cursorObject *cursor(PyObject *o)
{
	return  (cursorObject *) o;
}

static void cursorDealloc(PyObject *self);
static PyObject * cursorGetAttr(PyObject *self, char *name);


static PyTypeObject Cursor_Type =
{
	PyObject_HEAD_INIT (&PyType_Type)
	0,			/*ob_size */
	"odbccur",		/*tp_name */
	sizeof(cursorObject),	/*tp_basicsize */
	0,			/*tp_itemsize */
	cursorDealloc,	/*tp_dealloc */
	0,			/*tp_print */
	cursorGetAttr,	/*tp_getattr */
	/* drop the rest */
};


static void connectionDealloc(PyObject *self);
static PyObject * connectionGetAttr(PyObject *self, char *name);

static PyTypeObject Connection_Type =
{
	PyObject_HEAD_INIT (&PyType_Type)
	0,				/*ob_size */
	"odbcconn",		/*tp_name */
	sizeof (connectionObject),	/*tp_basicsize */
	0,				/*tp_itemsize */
	connectionDealloc,		/*tp_dealloc */
	0,				/*tp_print */
	connectionGetAttr,		/*tp_getattr */
	/* drop the rest */
};

static int unsuccessful(RETCODE rc)
{
	return (rc != SQL_SUCCESS) && (rc != SQL_SUCCESS_WITH_INFO);
}


int connectionDied(const char *sqlState)
{
	return !strcmp(sqlState, "08S01");
}


typedef struct {
	const char *state;
	int  index;
	int  connected;
} odbcErrorDesc;


static odbcErrorDesc *lookupError(const char *sqlState);
static PyObject *dbiErrors[6]; // 'cause I know about six DBI errors

static void odbcPrintError
(
	HENV env,
	connectionObject *conn,
	HSTMT cur,
	const char *action
)
{
	char  sqlState[256];
	long  nativeError;    
	short   pcbErrorMsg;  
	char    errorMsg[1000];
	PyObject *error;

	if (unsuccessful(SQLError(
		env,
		conn ? conn->hdbc : 0,
		cur, 
		(unsigned char *) sqlState,
		&nativeError, 
		(unsigned char *) errorMsg,
		sizeof(errorMsg), &pcbErrorMsg)))
	{
		error = odbcError;
		strcpy(errorMsg, "Could not find error ");
	}
	else
	{
		strcat(errorMsg, " in ");
		strcat(errorMsg, action);

		odbcErrorDesc *errorType = lookupError(sqlState);

		if (conn && errorType && (errorType->connected == 0))
		{
			printf("Disconnected\n");
			SQLDisconnect(conn->hdbc);
			conn->connected = 0;
		}

        // internal is the default
		int errn = errorType ? errorType->index : 5 ;  
		error = dbiErrors[errn];
	}

	PyErr_SetString(error, errorMsg);
}

static void connectionError(connectionObject *conn, const char *action)
{
	odbcPrintError(Env, conn, SQL_NULL_HSTMT, action);
}

static void cursorError(cursorObject *cur, const char *action)
{
	odbcPrintError(Env, cur->my_conx, cur->hstmt, action);
}

static int doConnect(connectionObject *conn)
{
	RETCODE rc;
	Py_BEGIN_ALLOW_THREADS
	rc = SQLConnect(
		conn->hdbc,
		(unsigned char *) conn->dsn,
		SQL_NTS,
		(unsigned char *) conn->uid,
		SQL_NTS, 
		(unsigned char *) conn->pwd,
		SQL_NTS);
	Py_END_ALLOW_THREADS
	if  (unsuccessful(rc))
	{
		odbcPrintError(Env, conn, SQL_NULL_HSTMT, "LOGIN");
		return 1;
	}
	conn->connected = 1;
	conn->connect_id++; // perturb it so cursors know to reconnect

	return 0;
}

static int attemptReconnect(cursorObject *cur)
{
	if ((cur->connect_id != cur->my_conx->connect_id) ||
		(cur->my_conx->connected == 0))
	{
		// ie the cursor was made on an old connection

		printf("Attempting reconnect\n");
		SQLFreeStmt(cur->hstmt, SQL_DROP);

		if (cur->my_conx->connected == 0)
		{
			// ie the db has not been reconnected
			if (doConnect(cur->my_conx))
			{
				return 1;
			}
		}
		if (unsuccessful(SQLAllocStmt(cur->my_conx->hdbc, &cur->hstmt)))
		{
			connectionError(cur->my_conx, "REOPEN");
			return 1;
		}
		cur->connect_id = cur->my_conx->connect_id;
		return 0;
	}

	return 0;
}

// @pymethod |connection|setautocommit|Sets the autocommit mode.
static PyObject *odbcSetAutoCommit(PyObject *self, PyObject *args)
{
	int c;
	connectionObject *conn;
	// @pyparm int|c||The boolean autocommit mode.
	if (!PyArg_ParseTuple(args, "i",&c))
		return NULL;
	conn=connection(self);
	if (c==0)
	{
		if (unsuccessful(SQLSetConnectOption(
			conn->hdbc,
			SQL_AUTOCOMMIT,
			SQL_AUTOCOMMIT_OFF)))
		{
			connectionError(conn, "SETAUTOCOMMIT");
			return NULL;
		}
	}
	else
	{
		if (unsuccessful(SQLSetConnectOption(
			conn->hdbc,
			SQL_AUTOCOMMIT,
			SQL_AUTOCOMMIT_ON)))
		{
			connectionError(conn, "SETAUTOCOMMIT");
			return NULL;
		};
	}

	Py_INCREF(Py_None);
	return Py_None;
}


// @pymethod |connection|commit|Commits a transaction.
static PyObject *odbcCommit(PyObject *self, PyObject *args)
{
	RETCODE rc;
	Py_BEGIN_ALLOW_THREADS
	rc = SQLTransact(
		Env,
		connection(self)->hdbc,
		SQL_COMMIT);
	Py_END_ALLOW_THREADS
	if (unsuccessful(rc))
	{
		connectionError(connection(self), "COMMIT");
		return 0;
	}
	else
	{
		Py_INCREF(Py_None);
		return Py_None;
	}
}

// @pymethod |connection|rollback|Rollsback a transaction.
static PyObject *odbcRollback(PyObject *self, PyObject *args)
{
	RETCODE rc;
	Py_BEGIN_ALLOW_THREADS
	rc = SQLTransact(
		Env,
		connection(self)->hdbc,
		SQL_ROLLBACK);
	Py_END_ALLOW_THREADS
	if (unsuccessful(rc))
	{
		connectionError(connection(self), "ROLLBACK");
		return 0;
	}
	else {
		Py_INCREF(Py_None);
		return Py_None;
	}
}

// @pymethod |connection|cursor|Creates a <o cursor> object
static PyObject *odbcCursor(PyObject *self, PyObject *args)
{
	connectionObject *conn = connection(self);
	if (conn->connected == 0)
	{
		if (doConnect(conn))
		{
			return 0;
		}
	}

	cursorObject *cur = PyObject_NEW(cursorObject, &Cursor_Type);
	if (cur == NULL)
		return NULL;

	cur->outputVars = 0;
	cur->inputVars = 0;
	cur->description = 0;
	cur->max_width = 65536L;
	cur->my_conx = 0;
	cur->bGetDataIsNeeded = false;
	if (unsuccessful(SQLAllocStmt(conn->hdbc, &cur->hstmt)))
	{
		connectionError(cur->my_conx, "OPEN");
		PyMem_DEL(cur);
		return 0;
	}
	cur->my_conx = conn;
	cur->connect_id = cur->my_conx->connect_id;
	Py_INCREF(self); /* the cursors owns a reference to the connection */
	return (PyObject*) cur;
}

// @pymethod |connection|close|Closes the connection.
static PyObject *odbcClose(PyObject *self, PyObject *args)
{
  Py_INCREF(Py_None);
  return Py_None;
}

// @object connection|An object representing an ODBC connection
static PyMethodDef connectionMethods[] = {
	{ "setautocommit", odbcSetAutoCommit, 1 }, // @pymeth setautocommit|Sets the autocommit mode.
	{ "commit", odbcCommit, 1 } , // @pymeth commit|Commits a transaction.
	{ "rollback", odbcRollback, 1 } , // @pymeth rollback|Rollsback a transaction.
	{ "cursor", odbcCursor, 1 } , // @pymeth cursor|Creates a <o cursor> object
	{ "close", odbcClose, 1 } , // @pymeth close|Closes the connection.
	{0,     0}
};

static PyObject *connectionGetAttr(PyObject *self,
       char *name)
{
	if (!strcmp(name, "error"))
	{
		Py_INCREF(odbcError);
		return odbcError;
	}

	return Py_FindMethod(connectionMethods, self, name);
}

static void connectionDealloc(PyObject *self)
{
	SQLDisconnect(connection(self)->hdbc);
	SQLFreeConnect(connection(self)->hdbc);
	PyMem_DEL(self);
}

static void deleteOutput(cursorObject *cur)
{
	OutputBinding *ob = cur->outputVars;
	while (ob)
	{
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
	while (ib)
	{
		InputBinding *next = ib->next;
		//$ free(ib->bind_area);
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
	if (SQLFreeStmt(cur->hstmt, SQL_DROP))
	{
		cursorError(cur, "CLOSE");
	}
	deleteBinding(cur);
	if (cur->my_conx)
	{
		Py_DECREF((PyObject*)cur->my_conx);
	}
	if (cur->description)
	{
		Py_DECREF(cur->description);
	}
	PyMem_DEL(self);
}


// @pymethod |cursor|close|Closes the cursor
static PyObject *odbcCurClose(PyObject *self, PyObject *args)
{
	// @comm This method does nothing!!  I presume it should!?!?!
	Py_INCREF(Py_None);
	return Py_None;
}

static void bindOutputVar
(
	cursorObject *cur,
	CopyFcn fcn,
	short vtype,
	long vsize,
	int pos,
	bool bUseGet
)
{
	OutputBinding *ob = (OutputBinding *) malloc(sizeof(OutputBinding));
	OutputBinding *current = NULL;

	ob->bGetData = bUseGet;
	ob->pos = pos;
	ob->vtype = vtype;
	ob->vsize = vsize;
	
	// Stick the new column on the end of the linked list.
	// We do this because we call SQLGetData() while walking the linked list.
	// Some ODBC drivers require all BLOB columns to be at the end of the column list.
	// So preserve the order our consumer called us with.
	ob->next = NULL;
	if (cur->outputVars == NULL)
	{
		cur->outputVars = ob;
	}
	else
	{
		current = cur->outputVars;
		while (current->next != NULL)
		{
			current = current->next;
		}
		current->next = ob;
	}
	
	ob->copy_fcn = fcn;
	ob->bind_area = malloc(vsize);
	ob->rcode = vsize;
	if (ob->bGetData == false)
	{
		if (unsuccessful(SQLBindCol(
			cur->hstmt,
			pos,
			vtype,
			ob->bind_area,
			vsize,
			&ob->rcode)))
		{
			cursorError(cur, "BIND");
		}
	}
}

static PyObject *stringCopy(const void *v, int sz)
{
	return Py_BuildValue("s", v);
}
static PyObject *longCopy(const void *v, int sz)
{
	return PyInt_FromLong(*(unsigned long *)v);
}

static PyObject *doubleCopy(const void *v, int sz)
{
	double d = *(double *)v;

	return (d == floor(d)) ? PyLong_FromDouble(d) : PyFloat_FromDouble(d);
}

static PyObject *dateCopy(const void *v, int sz)
{
	const TIMESTAMP_STRUCT  *dt = (const TIMESTAMP_STRUCT *) v;
	struct tm gt;
	gt.tm_isdst = -1; /* figure out DST */
	gt.tm_year = dt->year-1900;
	gt.tm_mon = dt->month-1;
	gt.tm_mday = dt->day;
	gt.tm_hour = dt->hour;
	gt.tm_min = dt->minute;
	gt.tm_sec = dt->second;
	return dbiMakeDate(PyInt_FromLong(mktime(&gt)));
}

static PyObject *rawCopy(const void *v, int sz)
{
	return dbiMakeRaw(PyString_FromStringAndSize((char *)v, sz));
}

typedef struct {
	const char *ptr;
	int parmCount;
	int parmIdx;
	int isParm;
	char state;
	char prev;
} parseContext;

static void initParseContext(parseContext *ct, const char *c)
{
  ct->state = 0;
  ct->ptr = c;
  ct->parmCount = 0;
}

static char doParse(parseContext *ct) 
{
	ct->isParm = 0;
	if (ct->state == *ct->ptr)
	{
		ct->state = 0;
	}
	else if (ct->state == 0)
	{
		if ((*ct->ptr == '\'') || (*ct->ptr == '"'))
		{
			ct->state = *ct->ptr;
		}
		else if (*ct->ptr == '?')
		{
			ct->parmIdx = ct->parmCount;
			ct->parmCount++;
			ct->isParm = 1;
		}
		else if ((*ct->ptr == ':') && !isalnum(ct->prev))
		{
			const char *m = ct->ptr + 1;
			int n = 0;
			while (isdigit(*m))
			{
				n *= 10;
				n += *m - '0';
				m++;
			}
			if (n)
			{
				ct->parmIdx = n-1;
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


static void OutOfMemory()
{
	PyErr_SetString(odbcError, "out of memory");
}

static long NTS = SQL_NTS;

static InputBinding *initInputBinding(cursorObject *cur, int len)
{
	InputBinding *ib = (InputBinding *)malloc(sizeof(InputBinding) + len);
	if (ib)
	{
		ib->bPutData = false;
		ib->next = cur->inputVars;
		cur->inputVars = ib;
		ib->len = len;
	}
	else
	{
		OutOfMemory();
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

	if (unsuccessful(SQLBindParameter(
		cur->hstmt,
		column,
		SQL_PARAM_INPUT,
		SQL_C_LONG,
		SQL_INTEGER,
		len,
		0, 
		ib->bind_area,
		len,
		&ib->len)))
	{
		cursorError(cur, "input-binding");
		return 0;
	}

	return 1;
}

static int ibindLong(cursorObject*cur,int column, PyObject *item) 
{
	int len = sizeof(double);
	double val = PyLong_AsDouble(item);

	InputBinding *ib = initInputBinding(cur, len);
	if (!ib)
		return 0;

	memcpy(ib->bind_area, &val, len);

	if (unsuccessful(SQLBindParameter(
		cur->hstmt,
		column,
		SQL_PARAM_INPUT,
		SQL_C_DOUBLE,
		SQL_FLOAT,
		len,
		0, 
		ib->bind_area,
		len,
		&ib->len)))
	{
		cursorError(cur, "input-binding");
		return 0;
	}

	return 1;
}

static int ibindNull(cursorObject*cur, int column)
{
  static SDWORD nl; 
  /* apparently, ODBC does not read the last parameter
     until EXEC time, i.e., after this function is
     out of scope, hence nl must be static */

  nl = SQL_NULL_DATA;
  /* I don't know if ODBC resets the value of the parameter.
     It shouldn't but god knows... */

  if (unsuccessful(SQLBindParameter(
	  cur->hstmt,
	  column,
	  SQL_PARAM_INPUT,
	  SQL_C_CHAR,
	  SQL_CHAR,
	  0,
	  0, 
	  0,
	  0,
	  &nl)))
  {
      cursorError(cur, "input-binding");
      return 0;
  }

  return 1;
}

static int ibindDate(cursorObject*cur, int column, PyObject *item) 
{
	long val = PyInt_AsLong(item);
	int len = sizeof(TIMESTAMP_STRUCT);

	InputBinding *ib = initInputBinding(cur, len);
	if (!ib)
		return 0;

	TIMESTAMP_STRUCT *dt = (TIMESTAMP_STRUCT*) ib->bind_area ;
	struct tm *gt = localtime(&val);

	dt->year = 1900 + gt->tm_year;
	dt->month = gt->tm_mon + 1;
	dt->day = gt->tm_mday;
	dt->hour = gt->tm_hour;
	dt->minute = gt->tm_min;
	dt->second = gt->tm_sec;
	dt->fraction = 0;

	if (unsuccessful(SQLBindParameter(
		cur->hstmt,
		column,
		SQL_PARAM_INPUT,
		SQL_C_TIMESTAMP,
		SQL_TIMESTAMP,
		len,
		0, 
		ib->bind_area,
		len,
		&ib->len)))
	{
		cursorError(cur, "input-binding");
		return 0;
	}

	return 1;
}

static int ibindRaw(cursorObject *cur, int column, PyObject *item)
{
  const char *val = PyString_AsString(item);
  int len = PyObject_Length(item);

  InputBinding *ib = initInputBinding(cur, len);
  if (!ib)
      return 0;
  ib->bPutData = true;
  
  memcpy(ib->bind_area, val, len);

  RETCODE rc = SQL_SUCCESS;
  ib->sqlBytesAvailable = SQL_LEN_DATA_AT_EXEC(ib->len);
  rc = SQLBindParameter(
	  cur->hstmt,
	  column,
	  SQL_PARAM_INPUT,
	  SQL_C_BINARY,
	  SQL_LONGVARBINARY,
	  len,
	  0, 
	  ib->bind_area,
	  len,
	  &ib->len);
  if (unsuccessful(rc))
  {
      cursorError(cur, "input-binding");
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

	if (unsuccessful(SQLBindParameter(
		cur->hstmt,
		column,
		SQL_PARAM_INPUT,
		SQL_C_DOUBLE,
		SQL_DOUBLE,
		15,
		0,
		ib->bind_area,
		sizeof(double),
		&ib->len)))
	{
		cursorError(cur, "input-binding");
		return 0;
	}

	return 1;
}

static int ibindString(cursorObject *cur, int column, PyObject *item)
{
  const char *val = PyString_AsString(item);
  int len = strlen(val);

  InputBinding *ib = initInputBinding(cur, len);
  if (!ib)
      return 0;

  strcpy(ib->bind_area, val);
  int sqlType = SQL_CHAR;
  if (len > 255)
  {
	  ib->sqlBytesAvailable = SQL_LEN_DATA_AT_EXEC(ib->len);
	  sqlType = SQL_LONGVARCHAR;
	  ib->bPutData = true;
  }
  else
  {
	  ib->sqlBytesAvailable = ib->len;
	  ib->bPutData = false;
  }

  RETCODE rc = SQLBindParameter(
	  cur->hstmt,
	  column,
	  SQL_PARAM_INPUT,
	  SQL_C_CHAR, 
	  sqlType,
	  len,
	  0, 
	  ib->bind_area,
	  len,
	  &NTS);
  if (unsuccessful(rc))
  {
      cursorError(cur, "input-binding");
      return 0;
  }

  return 1;
}

static int rewriteQuery
(
	char *out,
	const char *in
)
{
	parseContext ctx;
	
	initParseContext(&ctx, in);
	while (*out++ = doParse(&ctx))
		;
	return ctx.parmCount;
}

static int bindInput
(
	cursorObject *cur,
	PyObject *vars, 
	int columns
)
{
	int i;
	PyObject *item;
	int rv;
	int iCol;
	
	if (columns == 0)
	{
		return 1;
	}

	for(i = 0; i < PySequence_Length(vars); i++)
	{
		item = PySequence_GetItem(vars, i);
		iCol = i + 1;
		if (dbiIsRaw(item))
		{
			rv = ibindRaw(cur, iCol, dbiValue(item));
		} 
		else if (dbiIsDate(item))
		{
			rv = ibindDate(cur, iCol, dbiValue(item));
		}
		else if (PyLong_Check(item))
		{
			rv = ibindLong(cur, iCol, item);
		}
		else if (PyInt_Check(item))
		{
			rv = ibindInt(cur, iCol, item);
		}
		else if (PyString_Check(item))
		{
			rv = ibindString(cur, iCol, item);
		}
		else if (item==Py_None)
		{
			rv = ibindNull(cur, iCol);
		}
		else if (PyFloat_Check(item))
		{
			rv = ibindFloat(cur, iCol, item);
		}
		else
		{
			PyObject *sitem = PyObject_Str(item);
			rv = ibindString(cur, iCol, sitem);
			Py_DECREF(sitem);
		}
		Py_DECREF(item);
		if (rv == 0)
		{
			return 0;
		}
	}
	
	return 1;
}

static int display_size(short coltype, int collen, const char *colname) 
{

  switch (coltype)
    {
    case SQL_CHAR:
    case SQL_VARCHAR:
    case SQL_DATE:
    case SQL_TIMESTAMP:
    case SQL_BIT:
      return(max(collen, (int)strlen(colname)));
    case SQL_SMALLINT:
    case SQL_INTEGER:
    case SQL_TINYINT:
      return(max(collen+1, (int)strlen(colname)));
    case SQL_DECIMAL:
    case SQL_NUMERIC:
      return(max(collen+2, (int)strlen(colname)));
    case SQL_REAL:
    case SQL_FLOAT:
    case SQL_DOUBLE:
      return(max(20, (int)strlen(colname)));
    case SQL_BINARY:
    case SQL_VARBINARY:
      return(max(2*collen, (int)strlen(colname)));
    case SQL_LONGVARBINARY:
    case SQL_LONGVARCHAR:
    default:
      return (0);
    } 
}


static int bindOutput(cursorObject *cur)
{
	short vtype;
	unsigned long vsize;
	char name[256];
	int pos = 1;
	short n_columns;
	SQLNumResultCols(cur->hstmt, &n_columns);
	cur->n_columns = n_columns; 
	for (pos = 1; pos <= cur->n_columns; pos++)
	{
		PyObject *new_tuple;
		PyObject *typeOf;
		long dsize;
		unsigned long prec;
		short nullok;
		short nsize = sizeof(nsize);
		short scale = 0;
		SQLDescribeCol(
			cur->hstmt,
			pos,
			(unsigned char *) name,
			(short) sizeof(name),
			&nsize,
			&vtype,
			&vsize,
			&scale,
			&nullok);
		name[nsize] = 0;
		_strlwr(name);
		dsize = display_size(vtype, vsize, name);
		prec = 0;

		switch(vtype) {
		case SQL_SMALLINT:
		case SQL_INTEGER:
		case SQL_TINYINT:
			bindOutputVar(
				cur,
				longCopy,
				SQL_C_LONG,
				sizeof(unsigned long),
				pos,
				false);
			typeOf = DbiNumber;
			break;
		case SQL_NUMERIC:
		case SQL_DECIMAL:
		case SQL_FLOAT:
		case SQL_DOUBLE:
		case SQL_REAL:
		case SQL_BIGINT:
			bindOutputVar(
				cur,
				doubleCopy,
				SQL_C_DOUBLE,
				sizeof(double),
				pos,
				false);
			typeOf = DbiNumber;
			prec = vsize;
			break;
		case SQL_DATE:
		case SQL_TIMESTAMP:
			bindOutputVar(
				cur,
				dateCopy,
				SQL_C_TIMESTAMP,
				sizeof(TIMESTAMP_STRUCT),
				pos,
				false);
			typeOf = DbiDate;
			break;
		case SQL_LONGVARBINARY:
			cur->bGetDataIsNeeded = true;
			bindOutputVar(
				cur,
				rawCopy,
				SQL_C_BINARY,
				cur->max_width,
				pos,
				true);
			typeOf = DbiRaw;
			break;
		case SQL_BINARY:
		case SQL_VARBINARY:
			bindOutputVar(
				cur,
				rawCopy,
				SQL_C_BINARY,
				cur->max_width,
				pos,
				false);
			typeOf = DbiRaw;
			break;
		case SQL_LONGVARCHAR:
			bindOutputVar(cur, stringCopy, SQL_C_CHAR, cur->max_width, pos, true);
			typeOf = DbiString;
			break;
		default:
			bindOutputVar(cur, stringCopy, SQL_C_CHAR, vsize+1, pos, false);
			typeOf = DbiString;
			break;
		}
		if (PyErr_Occurred())
		{
			return 0;
		}
		new_tuple = Py_BuildValue(
			"(sOiiiii)",
			name, typeOf, dsize,
			(int)vsize, prec, scale, nullok);

		if (!new_tuple)
		{
			return 0;
		}

		PyList_Append(cur->description, new_tuple);
		Py_DECREF(new_tuple);
	}

	/* success */
	return 1;
}


// This lame function is here for backward compatibility with some
// very old ODBC drivers that got naively ported from Windows 3.1.
// So says: Chris Ingram [chris.ingram@synchrologic.com]
static RETCODE sendSQLInputData
(
	cursorObject *cur
)
{
	RETCODE rc = SQL_SUCCESS;
	char   *pIndx;
	
	Py_BEGIN_ALLOW_THREADS
	rc = SQLParamData(cur->hstmt, (void **)&pIndx);
	while (rc == SQL_NEED_DATA) 
	{
		InputBinding* pInputBinding = cur->inputVars;

		// find the input to put
		while (pInputBinding)
		{
			if (pIndx != pInputBinding->bind_area)
			{
				pInputBinding = pInputBinding->next;
			}
			else
			{
				break;
			}
		}

		if (pInputBinding)
		{
			size_t   putTimes = pInputBinding->len / 1024;
			size_t   remainder = pInputBinding->len % 1024;
			rc = SQL_SUCCESS;
					
			if (!putTimes && !remainder)
			{
				rc = SQLPutData(cur->hstmt, pInputBinding->bind_area, 0);
			}
			for (size_t i = 0; i < putTimes && rc == SQL_SUCCESS; i++)
			{
				rc = SQLPutData(cur->hstmt, (void*)(&pInputBinding->bind_area[i *  1024]), 1024);
			}

			if (remainder && rc == SQL_SUCCESS)
			{
				rc = SQLPutData(cur->hstmt, (void*)(&pInputBinding->bind_area[i *  1024]), remainder);
			}                    
		}
				
		// see if additional data is needed.
		rc = SQLParamData(cur->hstmt, (void **)&pIndx);
	}
	Py_END_ALLOW_THREADS
	
	return rc;
}

// @pymethod int|cursor|execute|Execute some SQL
static PyObject *odbcCurExec(PyObject *self, PyObject *args)
{
	cursorObject *cur = cursor(self);
	PyObject *temp = NULL;
	const char *sql;
	char *sqlbuf;
	PyObject *inputvars = 0;
	PyObject *rv = 0;
	PyObject *rows = 0;
	long t;
	int n_columns = 0;
	long n_rows = 0;

	if (attemptReconnect(cur))
	{
		return 0;
	}

	// @pyparm string|sql||The SQL to execute
	// @pyparm sequence|[var, ...]|[]|Input variables.
	if (!PyArg_ParseTuple(args, "s|O", &sql, &inputvars))
	{
		return NULL;
	}

	if (inputvars && !PySequence_Check(inputvars))
	{
		PyErr_SetString(odbcError, "expected sequence as second parameter");
		return NULL;
	}
	else if (inputvars && PySequence_Length(inputvars) > 0)
	{
		temp = PySequence_GetItem(inputvars, 0);
		// Strings don't count as a list in this case.
		if (PySequence_Check(temp) && !PyString_Check(temp))
		{
			rows = inputvars;
			inputvars = NULL;
		}
		Py_DECREF(temp);
	}

	deleteBinding(cur);

	if (cur->description)
	{
		Py_DECREF(cur->description);
	}
	cur->description = PyList_New(0);
	if (!cur->description)
	{
		return NULL;
	}

	cur->n_columns = 0;

	sqlbuf = (char *) malloc(strlen(sql) + 100);
	if (!sqlbuf)
	{
		Py_DECREF(cur->description);
		OutOfMemory();
		return NULL;
	}

	SQLFreeStmt(cur->hstmt, SQL_CLOSE); /* ignore errors here */
	RETCODE rc = SQL_SUCCESS;
	n_columns = rewriteQuery(sqlbuf, sql);
	Py_BEGIN_ALLOW_THREADS
	rc = SQLPrepare(cur->hstmt, (unsigned char *)sqlbuf, SQL_NTS);
	Py_END_ALLOW_THREADS
	if (unsuccessful(rc))
	{
		cursorError(cur, "EXEC");
		goto Error;
	}

	if (rows)
	{
		// handle insert cases...
		for(int i = 0; i < PySequence_Length(rows); i++)
		{
			inputvars = PySequence_GetItem(rows, i);
			if (!PySequence_Check(inputvars))
			{
				PyErr_SetString(odbcError, "expected sequence of sequences for bulk inserts");
				goto Error;
			}
			if (PySequence_Length(inputvars) != n_columns)
			{
				PyErr_Format(odbcError, "Found an insert row that didn't have %d columns", n_columns);
				goto Error;
			}
			if (!bindInput(cur, inputvars, n_columns))
			{
				goto Error;
			}
			Py_BEGIN_ALLOW_THREADS
			rc = SQLExecDirect(cur->hstmt, (unsigned char *) sqlbuf,
							   SQL_NTS);
			Py_END_ALLOW_THREADS
			// move data here.
			if (rc == SQL_NEED_DATA)
			{
				rc = sendSQLInputData(cur);
			}
			if (unsuccessful(rc))
			{
				cursorError(cur, "EXEC");
				goto Error;
			}
			/* Success! */
			/* Note: multiple result sets aren't supported here, just bulk inserts... */
			Py_BEGIN_ALLOW_THREADS
			SQLRowCount(cur->hstmt, &t);
			Py_END_ALLOW_THREADS
			n_rows += t;
			deleteBinding(cur);
			Py_DECREF(inputvars);
			inputvars = NULL;
		}
	}
	else
	{
		if (!bindInput(cur, inputvars, n_columns))
		{
			goto Error;
		}
		Py_BEGIN_ALLOW_THREADS
		rc = SQLExecDirect(cur->hstmt, (unsigned char *) sqlbuf,
						   SQL_NTS);
		Py_END_ALLOW_THREADS
		if (rc == SQL_NEED_DATA)
		{
			rc = sendSQLInputData(cur);
		}
		if (unsuccessful(rc))
		{
			cursorError(cur, "EXEC");
			goto Error;
		}
		else if (!bindOutput(cur))
		{
			goto Error;
		}
		/* success */
		if (cur->n_columns > 0)
		{
			/* it was a select */
			Py_INCREF(Py_None);
			rv = Py_None;
		}
		else
		{
			n_rows = 1; /* just in case it does not work */
			SQLRowCount(cur->hstmt, &n_rows);
		}
	}
	
	rv = PyInt_FromLong(n_rows);
Cleanup:
	free(sqlbuf);
	return rv;
Error:
	Py_DECREF(cur->description);
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
		long cbRequired;
		RETCODE rc;
		long cbRead = 0;
        // Use SQLGetData to retrieve data for blob (or long varchar) type columns.
        if (ob->bGetData)
        {
            // Initialize memory (offsets, etc.)
            // (use bind_area for buffer, and dynamically allocate in the loop below)
            cbRequired = ob->vsize;
            ob->rcode = 0;
            cbRead = 0;      // Count of bytes read (running total and offset into buffer).

            // Loop until SQLGetData tells us that there are no more chunks
            // of the blob to retrieve.
            do
            {
                // Check to see if bind_area is big enough
                //    if cbRequired > vsize
                //       re-allocate bind_area to cbRequired
                //       set ob->vsize = cbRequired
                if (cbRequired > ob->vsize)
                {
                    void *pTemp;
					//
					// Some BLOBs can be huge, be paranoid about allowing
					// other threads to run.
					//
					Py_BEGIN_ALLOW_THREADS
                    pTemp = malloc (cbRequired);
                    memcpy(pTemp, ob->bind_area, ob->vsize);
                    free (ob->bind_area);
					Py_END_ALLOW_THREADS
                    ob->bind_area = pTemp;
                    ob->vsize = cbRequired;
                }

                // rc = GetData( ... , bind_area + offset, vsize - offset, &rcode )
				Py_BEGIN_ALLOW_THREADS
                rc = SQLGetData(cur->hstmt,
                                       ob->pos,
                                       ob->vtype,
                                       (char *)ob->bind_area + cbRead,
                                       ob->vsize - cbRead,
                                       &ob->rcode);
				Py_END_ALLOW_THREADS
				if (unsuccessful(rc))
				{
					Py_DECREF(row);
	 				cursorError(cur, "FETCH");
					return NULL;
				}

                if ((ob->rcode != SQL_NO_TOTAL) && (ob->rcode <= ob->vsize - cbRead))
                {
                    // If we get here, then this should be the last iteration
                    // through the loop.
                    cbRead += ob->rcode;
                }
                else
                {
                    // Grow buffer by (32k minus 1 byte) each for each chunk.
                    // If not for the SQL Anywhere 5.0 problem (driver version
                    // 5.05.041867), we could probably grow by 50% each time
                    // or the remaining size (as determined by ob->rcode).

                    //cbRequired += ob->rcode - ob->vsize;
                    cbRequired += 32767;    // Fix that works for SQL Anywhere 5.0 driver.
                    if (ob->vtype == SQL_C_CHAR)
					{
						// We want to ignore the intermediate
						// NULL characters SQLGetData() gives us.
						// (silly, silly)
						cbRead = ob->vsize - 1;
					}
					else
					{
						cbRead = ob->vsize;
					}
                }

            } while (rc == SQL_SUCCESS_WITH_INFO); 
        }
		
		PyObject *v;
		if (ob->rcode == SQL_NULL_DATA)
		{
			v = Py_None;
			Py_INCREF(v);
		}
		else
		{
			if (ob->bGetData == false)
			{
				v = ob->copy_fcn(
					ob->bind_area,
					(ob->rcode < cur->max_width) ?
					ob->rcode : cur->max_width);
			}
			else
			{
				v = ob->copy_fcn(
					ob->bind_area,
					cbRead);
			}
			if (!v)
			{
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
  Py_BEGIN_ALLOW_THREADS
  rc = SQLFetch(cur->hstmt);
  Py_END_ALLOW_THREADS
  if (rc == SQL_NO_DATA_FOUND)
  {
    Py_INCREF(Py_None);
    return Py_None;
  }
  else if (unsuccessful(rc))
  {
    cursorError(cur, "FETCH");
    return 0;
  }
  return processOutput(cur);
}

static PyObject *fetchN(cursorObject *cur, long n_rows)
{
	int row;
	PyObject *list = PyList_New(0);
	for (row = 0; row < n_rows; row++)
	{
		PyObject *entry = fetchOne(cur);
		if (entry)
		{
			if (entry == Py_None)
			{
				Py_DECREF(entry);
				return list;
			}
			else
			{
				PyList_Append(list, entry);
				Py_DECREF(entry);
			}
		}
		else
		{
			Py_DECREF(list); // thwow it away
			return NULL;
		}
	}
	return list;
}


// @pymethod data|cursor|fetchone|Fetch one row of data
static PyObject *odbcCurFetchOne(PyObject *self, PyObject *args)
{
	return fetchOne(cursor(self));
}


// @pymethod [data, ...]|cursor|fetchmany|Fetch many rows of data
static PyObject *odbcCurFetchMany(PyObject *self, PyObject *args)
{
  long n_rows = 1;

  if (!PyArg_ParseTuple(args, "|l", &n_rows))
  {
      return NULL;
  }

  return fetchN(cursor(self), n_rows);
}

// @pymethod [data, ...]|cursor|fetchall|Fetch all rows of data
static PyObject *odbcCurFetchAll(PyObject *self, PyObject *args)
{
	return fetchN(cursor(self), LONG_MAX);
}

// @pymethod |cursor|setinputsizes|
static PyObject *odbcCurSetInputSizes(PyObject *self, PyObject *args)
{
	Py_INCREF(Py_None);
	return Py_None;
}
// @pymethod |cursor|setoutputsize|
static PyObject *odbcCurSetOutputSize(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, "l", &cursor(self)->max_width))
	{
		return NULL;
	}

	Py_INCREF(Py_None);
	return Py_None;
}

// @object cursor|An object representing an ODBC cursor.
static PyMethodDef cursorMethods[] = {
  { "close", odbcCurClose, 1} , // @pymeth close|Closes the cursor
  { "execute", odbcCurExec, 1} , // @pymeth execute|Execute some SQL
  { "fetchone", odbcCurFetchOne, 1} , // @pymeth fetchone|Fetch one row of data
  { "fetchmany", odbcCurFetchMany, 1} , // @pymeth fetchmany|Fetch many rows of data
  { "fetchall", odbcCurFetchAll, 1} , // @pymeth fetchall|Fetch all the rows of data
  { "setinputsizes", odbcCurSetInputSizes, 1} , // @pymeth setinputsizes|
  { "setoutputsize", odbcCurSetOutputSize, 1} ,// @pymeth setoutputsize|
  {0,     0}        /* Sentinel */
};

static PyObject *cursorGetAttr(PyObject *self,
        char *name)
{
	if (!strcmp(name, "error"))
	{
		Py_INCREF(odbcError);
		return odbcError;
	}
	if (!strcmp(name, "description"))
	{
		Py_INCREF(cursor(self)->description);
		return cursor(self)->description;
	}
	return Py_FindMethod(cursorMethods, self, name);
}

static void parseInfo(connectionObject *conn, const char *c)
{
	char *p;
	char buf[255];
	strncpy(buf, c, sizeof(buf));
	p = strtok(buf, "/");
	if (p)
	{
		strncpy(conn->dsn, p, sizeof(conn->dsn));
		p = strtok(0, "/");
		if (p)
		{
			strncpy(conn->uid, p, sizeof(conn->uid));
			p = strtok(0, "/");
			if (p)
			{
				strncpy(conn->pwd, p, sizeof(conn->pwd));
			}
			else
			{
				conn->pwd[0] = 0;
			}
		}
		else
		{
			conn->uid[0] = 0;
			conn->pwd[0] = 0;
		}
	}
	else
	{
		strncpy(conn->dsn, c, sizeof(conn->dsn));
		conn->uid[0] = 0;
		conn->pwd[0] = 0;
	}
}

// @pymethod <o connection>|odbc|odbc|Creates an ODBC connection
static PyObject *odbcLogon(PyObject *self, PyObject *args)
{
	const char *connectionString;
	connectionObject *conn;

	// @pyparm string|connectionString||An ODBC connection string.
	if (!PyArg_ParseTuple(args, "s", &connectionString))
	{
		return NULL;
	}

	conn = PyObject_NEW(connectionObject, &Connection_Type);
	if (!conn)
	{
		return NULL;
	}

	conn->connect_id = 0; // initialize it to anything
	conn->hdbc = SQL_NULL_HDBC;
	if (unsuccessful(SQLAllocConnect(Env, &conn->hdbc)))
	{
		connectionError(conn, "ALLOCATION");
		PyMem_DEL(conn);
		return 0;
	}

	parseInfo(conn, connectionString);

	if (doConnect(conn))
	{
		PyMem_DEL(conn);
		return 0;
	}

	return (PyObject*)conn;
}

// @module odbc|A Python wrapper around the ODBC API.
static PyMethodDef globalMethods[] = {
  { "odbc", odbcLogon, 1} , // @pymeth odbc|Creates an <o connection> object.
  {0,     0}        /* Sentinel */
};


extern "C" __declspec(dllexport) void initodbc()
{
	odbcError = PyString_FromString("OdbcError");

    if (unsuccessful(SQLAllocEnv(&Env)))
	{
		odbcPrintError(SQL_NULL_HENV, 0, SQL_NULL_HSTMT, "INIT");
    }
    else if (PyImport_ImportModule("dbi"))
	{
		PyObject *m = Py_InitModule("odbc", globalMethods);
		if (!m) /* Eeek - some serious error! */
			return;
		if (m)
		{
			/* The indices go to indices in the ODBC error table */
			dbiErrors[0] = DbiNoError;
			dbiErrors[1] = DbiOpError;
			dbiErrors[2] = DbiProgError;
			dbiErrors[3] = DbiIntegrityError;
			dbiErrors[4] = DbiDataError;
			dbiErrors[5] = DbiInternalError;
			PyDict_SetItemString(PyModule_GetDict (m), "error", odbcError);
		}
    }
    else
	{
		PyErr_SetString(PyString_FromString("odbc"),
						"Cannot import dbi module");
    }
}

static odbcErrorDesc errorTable[] = {
	{ "01000", 5, 0 } , // General warning
	{ "01002", 1, 1 } , // Disconnect error
	{ "01004", 0, 1 } , // Data truncated
	{ "01006", 5, 1 } , // Privilege not revoked
	{ "01S00", 2, 1 } , // Invalid connection string attribute
	{ "01S01", 5, 1 } , // Error in row
	{ "01S02", 5, 1 } , // Option value changed
	{ "01S03", 0, 1 } , // No rows updated or deleted
	{ "01S04", 0, 1 } , // More than one row updated or deleted
	{ "01S05", 0, 1 } , // Cancel treated as SQLFreeStmt with the SQL_CLOSE
	{ "01S06", 2, 1 } , // Attempt to fetch before the result set returned
	{ "07001", 2, 1 } , // Wrong number of parameters
	{ "07006", 2, 1 } , // Restricted data type attribute violation
	{ "07S01", 2, 1 } , // Invalid use of default parameter
	{ "08001", 1, 1 } , // Unable to connect to data source
	{ "08002", 1, 1 } , // Connection in use
	{ "08003", 1, 1 } , // Connection not open
	{ "08004", 1, 1 } , // Data source rejected establishment of connection
	{ "08007", 1, 1 } , // Connection failure during transaction
	{ "08S01", 1, 0 } , // Communication link failure
	{ "21S01", 2, 1 } , // Insert value list does not match column list
	{ "21S02", 2, 1 } , // Degree of derived table does not match column list
	{ "22001", 0, 1 } , // String data right truncation
	{ "22002", 5, 1 } , // Indicator variable required but not supplied
	{ "22003", 4, 1 } , // Numeric value out of range
	{ "22005", 4, 1 } , // Error in assignment
	{ "22008", 4, 1 } , // Datetime field overflow
	{ "22012", 4, 1 } , // Division by zero
	{ "22026", 4, 1 } , // String data, length mismatch
	{ "23000", 3, 1 } , // Integrity constraint violation
	{ "24000", 5, 1 } , // Invalid cursor state
	{ "25000", 5, 1 } , // Invalid transaction state
	{ "28000", 1, 1 } , // Invalid authorization specification
	{ "34000", 5, 1 } , // Invalid cursor name
	{ "37000", 2, 1 } , // Syntax error or access violation
	{ "3C000", 5, 1 } , // Duplicate cursor name
	{ "40001", 5, 1 } , // Serialization failure
	{ "42000", 2, 1 } , // Syntax error or access violation
	{ "70100", 1, 1 } , // Operation aborted
	{ "IM001", 1, 1 } , // Driver does not support this function
	{ "IM002", 1, 1 } , // Data source name not found and no default driver 
	{ "IM003", 1, 1 } , // Specified driver could not be loaded
	{ "IM004", 1, 1 } , // Driver's SQLAllocEnv failed
	{ "IM005", 1, 1 } , // Driver's SQLAllocConnect failed
	{ "IM006", 1, 1 } , // Driver's SQLSetConnect-Option failed
	{ "IM007", 1, 1 } , // No data source or driver specified; dialog prohibited
	{ "IM008", 1, 1 } , // Dialog failed
	{ "IM009", 1, 1 } , // Unable to load translation DLL
	{ "IM010", 1, 1 } , // Data source name too long
	{ "IM011", 1, 1 } , // Driver name too long
	{ "IM012", 1, 1 } , // DRIVER keyword syntax error
	{ "IM013", 1, 1 } , // Trace file error
	{ "S0001", 2, 1 } , // Base table or view already exists
	{ "S0002", 2, 1 } , // Base table not found
	{ "S0011", 2, 1 } , // Index already exists
	{ "S0012", 2, 1 } , // Index not found
	{ "S0021", 2, 1 } , // Column already exists
	{ "S0022", 2, 1 } , // Column not found
	{ "S0023", 2, 1 } , // No default for column
	{ "S1000", 1, 1 } , // General error
	{ "S1001", 1, 1 } , // Memory allocation failure
	{ "S1002", 5, 1 } , // Invalid column number
	{ "S1003", 5, 1 } , // Program type out of range
	{ "S1004", 5, 1 } , // SQL data type out of range
	{ "S1008", 1, 1 } , // Operation canceled
	{ "S1009", 5, 1 } , // Invalid argument value
	{ "S1010", 5, 1 } , // Function sequence error
	{ "S1011", 5, 1 } , // Operation invalid at this time
	{ "S1012", 5, 1 } , // Invalid transaction operation code specified
	{ "S1015", 5, 1 } , // No cursor name available
	{ "S1090", 5, 1 } , // Invalid string or buffer length
	{ "S1091", 5, 1 } , // Descriptor type out of range
	{ "S1092", 5, 1 } , // Option type out of range
	{ "S1093", 5, 1 } , // Invalid parameter number
	{ "S1095", 5, 1 } , // Function type out of range
	{ "S1096", 5, 1 } , // Information type out of range
	{ "S1097", 5, 1 } , // Column type out of range
	{ "S1098", 5, 1 } , // Scope type out of range
	{ "S1099", 5, 1 } , // Nullable type out of range
	{ "S1100", 5, 1 } , // Uniqueness option type out of range
	{ "S1101", 5, 1 } , // Accuracy option type out of range
	{ "S1103", 5, 1 } , // Direction option out of range
	{ "S1105", 5, 1 } , // Invalid parameter type
	{ "S1106", 5, 1 } , // Fetch type out of range
	{ "S1107", 5, 1 } , // Row value out of range
	{ "S1108", 5, 1 } , // Concurrency option out of range
	{ "S1109", 5, 1 } , // Invalid cursor position
	{ "S1110", 5, 1 } , // Invalid driver completion
	{ "S1111", 5, 1 } , // Invalid bookmark value
	{ "S1C00", 1, 1 } , // Driver not capable
	{ "S1T00", 1, 1 }   // Timeout expired
};

static int odbcCompare(const void * v1, const void * v2)
{
	return strcmp(((const odbcErrorDesc *) v1)->state,
				  ((const odbcErrorDesc *) v2)->state);
}



static odbcErrorDesc *lookupError(const char *sqlState)
{
	odbcErrorDesc key;

	key.state = sqlState;
	return (odbcErrorDesc*) 
		bsearch(
			&key, 
			errorTable, 
			sizeof(errorTable)/ sizeof(odbcErrorDesc), // number of elems
			sizeof(odbcErrorDesc), 
			odbcCompare);
}

