/* File : win32evtlog.i */

%module win32evtlog // A module, encapsulating the Windows Win32 event log API.
// <nl>The Evt* functions are only available on Vista and later.  Attempting to call
//	them on XP will result in the process exiting, rather than a python exception.

%include "typemaps.i"
%include "pywin32.i"

%{

#include <structmember.h>

#undef PyHANDLE
#include "PyWinObjects.h"
#include "WinEvt.h"

// Automatically freed WCHAR that can be used anywhere WCHAR * is required
class TmpWCHAR
{
public:
	WCHAR *tmp;
	TmpWCHAR() { tmp=NULL; }
	TmpWCHAR(WCHAR *t) { tmp=t; }
	WCHAR * operator= (WCHAR *t){
		PyWinObject_FreeWCHAR(tmp);
		tmp=t;
		return t;
		}
	WCHAR ** operator& () {return &tmp;}
	boolean operator== (WCHAR *t) { return tmp==t; }
	operator WCHAR *() { return tmp; }
	~TmpWCHAR() { PyWinObject_FreeWCHAR(tmp); }
};

// @object PyEVTLOG_HANDLE|Object representing a handle to the windows event log.
//   Identical to <o PyHANDLE>, but calls CloseEventLog() on destruction
class PyEVTLOG_HANDLE: public PyHANDLE
{
public:
	PyEVTLOG_HANDLE(HANDLE hInit) : PyHANDLE(hInit) {}
	virtual BOOL Close(void) {
		BOOL ok = m_handle ? CloseEventLog(m_handle) : TRUE;
		m_handle = 0;
		if (!ok)
			PyWin_SetAPIError("CloseEventLog");
		return ok;
	}
	virtual const char *GetTypeName() {
		return "PyEVTLOG_HANDLE";
	}
};

// @object PyEVT_HANDLE|Handle to an event log, session, query, or any other object used with
//	the Evt* event log functions on Vista and later.
//	When the object is destroyed, EvtClose is called.
class PyEVT_HANDLE: public PyHANDLE
{
public:
	PyEVT_HANDLE(HANDLE hInit) : PyHANDLE(hInit) {}
	virtual BOOL Close(void){
		BOOL ret=EvtClose(m_handle);
		if (!ret)
			PyWin_SetAPIError("EvtClose");
		m_handle = 0;
		return ret;
		}
	virtual const char *GetTypeName(){
		return "PyEVT_HANDLE";
		}
};

#define PyHANDLE HANDLE

PyObject *PyWinObject_FromEVTLOG_HANDLE(HANDLE h)
{
	PyObject *ret = new PyEVTLOG_HANDLE(h);
	if (!ret)
		PyErr_NoMemory();
	return ret;
}

PyObject *PyWinObject_FromEVT_HANDLE(HANDLE h)
{
	PyObject *ret=new PyEVT_HANDLE(h);
	if (ret==NULL){
		EvtClose(h);
		PyErr_NoMemory();
		}
	return ret;
}
%}

%typemap(python,except) PyEVTLOG_HANDLE {
  Py_BEGIN_ALLOW_THREADS
  $function
  Py_END_ALLOW_THREADS
  if ($source==0 || $source==INVALID_HANDLE_VALUE)  {
    $cleanup
    return PyWin_SetAPIError("$name");
  }
}

%typemap(python,out) PyEVTLOG_HANDLE {
  $target = PyWinObject_FromEVTLOG_HANDLE($source);
}

typedef HANDLE PyEVTLOG_HANDLE;
%{
#define PyEVTLOG_HANDLE HANDLE
%}

%{

// @object PyEventLogRecord|An object containing the data in an EVENTLOGRECORD.
class PyEventLogRecord : public PyObject
{
public:
	PyEventLogRecord(EVENTLOGRECORD *pEvt);
	~PyEventLogRecord(void);

	static void deallocFunc(PyObject *ob);
	static struct PyMemberDef members[];

protected:
	DWORD Reserved;
	DWORD RecordNumber;
	PyObject *TimeGenerated;
	PyObject * TimeWritten;
	DWORD EventID;
	WORD EventType;
	PyObject *SourceName;
	PyObject *StringInserts;
	WORD EventCategory;
	WORD ReservedFlags;
	DWORD ClosingRecordNumber;
	PyObject *Sids;
	PyObject *Data;
	PyObject *ComputerName;
};

/*
PyObject *PyWinMethod_NewEventLogRecord(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ":EventLogRecord"))
		return NULL;
	return new PyEventLogRecord();
}
*/

PyTypeObject PyEventLogRecordType =
{
	PYWIN_OBJECT_HEAD
	"PyEventLogRecord",
	sizeof(PyEventLogRecord),
	0,
	PyEventLogRecord::deallocFunc,		/* tp_dealloc */
	0,						/* tp_print */
	0,						/* tp_getattr */
	0,						/* tp_setattr */
	0,						/* tp_compare */
	0,						/* tp_repr */
	0,						/* tp_as_number */
	0,						/* tp_as_sequence */
	0,						/* tp_as_mapping */
	0,						/* tp_hash */
	0,						/* tp_call */
	0,						/* tp_str */
	PyObject_GenericGetAttr,	/* tp_getattro */
	PyObject_GenericSetAttr,	/* tp_setattro */
	0,						/*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT,		/* tp_flags */
	0,						/* tp_doc */
	0,						/* tp_traverse */
	0,						/* tp_clear */
	0,						/* tp_richcompare */
	0,						/* tp_weaklistoffset */
	0,						/* tp_iter */
	0,						/* tp_iternext */
	0,						/* tp_methods */
	PyEventLogRecord::members,	/* tp_members */
	0,						/* tp_getset */
	0,						/* tp_base */
	0,						/* tp_dict */
	0,						/* tp_descr_get */
	0,						/* tp_descr_set */
	0,						/* tp_dictoffset */
	0,						/* tp_init */
	0,						/* tp_alloc */
	0,						/* tp_new */
};

#define OFF(e) offsetof(PyEventLogRecord, e)

/*static*/ struct PyMemberDef PyEventLogRecord::members[] = {
	{"Reserved",           T_INT,     OFF(Reserved)}, // @prop integer|Reserved|
	{"RecordNumber",       T_INT,	  OFF(RecordNumber)}, // @prop integer|RecordNumber|
	{"TimeGenerated",      T_OBJECT,  OFF(TimeGenerated)}, // @prop <o PyTime>|TimeGenerated|
	{"TimeWritten",        T_OBJECT,  OFF(TimeWritten)}, // @prop <o PyTime>|TimeWritten|
	{"EventID",            T_INT,	  OFF(EventID)}, // @prop integer|EventID|
	{"EventType",          T_SHORT,	  OFF(EventType)}, // @prop integer|EventType|
	{"EventCategory",      T_SHORT,   OFF(EventCategory)}, // @prop integer|EventCategory|
	{"ReservedFlags",      T_SHORT,   OFF(ReservedFlags)}, // @prop integer|ReservedFlags|
	{"ClosingRecordNumber",T_INT,     OFF(ClosingRecordNumber)}, // @prop integer|ClosingRecordNumber|
	{"SourceName",         T_OBJECT,  OFF(SourceName)}, // @prop <o PyUnicode>|SourceName|
	{"StringInserts",      T_OBJECT,  OFF(StringInserts)}, // @prop (<o PyUnicode>,...)|StringInserts|
	{"Sid",                T_OBJECT,  OFF(Sids)}, // @prop <o PySID>|Sid|
	{"Data",               T_OBJECT,  OFF(Data)}, // @prop string|Data|
	{"ComputerName",       T_OBJECT,  OFF(ComputerName)}, // @prop <o PyUnicode>|ComputerName|
	{NULL}
};

PyEventLogRecord::PyEventLogRecord(EVENTLOGRECORD *pEvt)
{
	ob_type = &PyEventLogRecordType;
	_Py_NewReference(this);
	Reserved = RecordNumber = EventID = ClosingRecordNumber = 0;
	TimeWritten = TimeGenerated = SourceName = ComputerName = StringInserts = Sids = Data = NULL;
	EventType = EventCategory = ReservedFlags = 0;
	if (pEvt==NULL) // Empty one.
		return;

	Reserved = pEvt->Reserved;
	RecordNumber = pEvt->RecordNumber;
	EventID = pEvt->EventID;
	EventType = pEvt->EventType;
	EventCategory = pEvt->EventCategory;
	ReservedFlags = pEvt->ReservedFlags;
	ClosingRecordNumber = pEvt->ClosingRecordNumber;

	if (pEvt->NumStrings==0) {
		StringInserts = Py_None;
		Py_INCREF(Py_None);
	} else {
		StringInserts = PyTuple_New(pEvt->NumStrings);
		if (StringInserts) {
			WCHAR *stringOffset = (WCHAR *) (((BYTE *)pEvt) + pEvt->StringOffset);
			for (DWORD stringNo = 0;stringNo<pEvt->NumStrings;stringNo++) {
				PyTuple_SET_ITEM( StringInserts, (int)stringNo, PyWinObject_FromWCHAR(stringOffset));
				stringOffset = stringOffset + (wcslen(stringOffset)) + 1;
			}
		}
	}

	TimeGenerated = PyWinTimeObject_Fromtime_t((time_t)pEvt->TimeGenerated);
	TimeWritten = PyWinTimeObject_Fromtime_t((time_t)pEvt->TimeWritten);

	if (pEvt->UserSidLength==0) {
		Sids = Py_None; // No SID in this record.
		Py_INCREF(Sids);
	} else {
		Sids = PyWinObject_FromSID( (PSID)(((BYTE *)pEvt) + pEvt->UserSidOffset));
	}

	Data = PyString_FromStringAndSize(((char *)pEvt)+pEvt->DataOffset, pEvt->DataLength);

	WCHAR *szSourceName = (WCHAR *)(((BYTE *)pEvt) + sizeof(EVENTLOGRECORD));
	SourceName = PyWinObject_FromWCHAR(szSourceName);

	ComputerName = PyWinObject_FromWCHAR(szSourceName + wcslen(szSourceName) + 1);
}

PyEventLogRecord::~PyEventLogRecord(void)
{
	Py_XDECREF(TimeWritten);
	Py_XDECREF(TimeGenerated);
	Py_XDECREF(SourceName);
	Py_XDECREF(StringInserts);
	Py_XDECREF(Sids);
	Py_XDECREF(Data);
	Py_XDECREF(ComputerName);
}

/*static*/ void PyEventLogRecord::deallocFunc(PyObject *ob)
{
	delete (PyEventLogRecord *)ob;
}

PyObject *MakeEventLogObject( BYTE *buf, DWORD numBytes )
{
	PyObject *ret = PyList_New(0);
	if (ret==NULL) return NULL;
	while (numBytes>0) {
		EVENTLOGRECORD *pEvt = (EVENTLOGRECORD *)buf;
		PyObject *subItem = new PyEventLogRecord(pEvt);
		if (subItem==NULL) {
			Py_DECREF(ret);
			PyErr_SetString(PyExc_MemoryError, "Allocating EventLogRecord object");
			return NULL;
		}
		PyList_Append(ret, subItem);
		Py_DECREF(subItem);
		buf = buf + pEvt->Length;
		numBytes -= pEvt->Length;
	}
	return ret;
}

PyObject *MyReadEventLog( HANDLE hEventLog, DWORD dwReadFlags, DWORD dwRecordOffset)
{
	DWORD needed, read;
	needed = 1024;
	BYTE *buf;
	BOOL ok;
	while (1) {
		buf = (BYTE *)malloc(needed);
		if (buf==NULL) {
			PyErr_SetString(PyExc_MemoryError, "Allocating initial buffer");
			return NULL;
		}
		Py_BEGIN_ALLOW_THREADS
		ok = ReadEventLogW(hEventLog, dwReadFlags, dwRecordOffset, buf, needed, &read, &needed);
		Py_END_ALLOW_THREADS
		if (!ok) {
			DWORD err = GetLastError();
			if (err==ERROR_HANDLE_EOF) {
				read = 0;// pretend everything is OK...
				break;
			}
			else if (err==ERROR_INSUFFICIENT_BUFFER) {
				free(buf);
				continue; // try again.
			} else {
				free(buf);
				return PyWin_SetAPIError("ReadEventLog");
			}
		}
		else
			break;
	}
	// Convert the object.
	PyObject *ret = MakeEventLogObject(buf, read);
	free(buf);
	return ret;
}

PyObject * MyReportEvent( HANDLE hEventLog,
    WORD wType,	// event type to log 
    WORD wCategory,	// event category 
    DWORD dwEventID,	// event identifier
    PyObject *obSID,    // user security identifier object (optional)
    PyObject *obStrings,  // insert strings
    PyObject *obData)     // raw data
{
	PyObject *rc = NULL;
	DWORD numStrings = 0;
	WCHAR **pStrings = NULL;
	DWORD dataSize = 0;
	void *pData;
	PSID sid;
	if (!PyWinObject_AsSID(obSID, &sid, TRUE))
		return NULL;
	if (!PyWinObject_AsReadBuffer(obData, &pData, &dataSize, TRUE))
		return NULL;
	if (!PyWinObject_AsWCHARArray(obStrings, &pStrings, &numStrings, TRUE))
		return NULL;
	if (numStrings > USHRT_MAX){
		PyErr_Format(PyExc_ValueError, "String inserts can contain at most %d strings", USHRT_MAX);
		goto cleanup;
		}
	BOOL ok;
	Py_BEGIN_ALLOW_THREADS
	ok = ReportEventW(hEventLog, wType, wCategory,	dwEventID, sid, (WORD)numStrings, dataSize, (const WCHAR **)pStrings, pData);
	Py_END_ALLOW_THREADS

	if (!ok) {
		PyWin_SetAPIError("ReportEvent");
		goto cleanup;
	}
	Py_INCREF(Py_None);
	rc = Py_None;
cleanup:
	PyWinObject_FreeWCHARArray(pStrings, numStrings);
	return rc;
}

%}

#define EVENTLOG_FORWARDS_READ EVENTLOG_FORWARDS_READ
#define EVENTLOG_BACKWARDS_READ EVENTLOG_BACKWARDS_READ
#define EVENTLOG_SEEK_READ EVENTLOG_SEEK_READ
#define EVENTLOG_SEQUENTIAL_READ EVENTLOG_SEQUENTIAL_READ

#define EVENTLOG_SUCCESS EVENTLOG_SUCCESS
#define EVENTLOG_ERROR_TYPE EVENTLOG_ERROR_TYPE
#define EVENTLOG_WARNING_TYPE EVENTLOG_WARNING_TYPE
#define EVENTLOG_INFORMATION_TYPE EVENTLOG_INFORMATION_TYPE
#define EVENTLOG_AUDIT_SUCCESS EVENTLOG_AUDIT_SUCCESS
#define EVENTLOG_AUDIT_FAILURE EVENTLOG_AUDIT_FAILURE

#define EVENTLOG_START_PAIRED_EVENT EVENTLOG_START_PAIRED_EVENT
#define EVENTLOG_END_PAIRED_EVENT EVENTLOG_END_PAIRED_EVENT
#define EVENTLOG_END_ALL_PAIRED_EVENTS EVENTLOG_END_ALL_PAIRED_EVENTS
#define EVENTLOG_PAIRED_EVENT_ACTIVE EVENTLOG_PAIRED_EVENT_ACTIVE
#define EVENTLOG_PAIRED_EVENT_INACTIVE EVENTLOG_PAIRED_EVENT_INACTIVE

// @pyswig |ClearEventLog|Clears the event log
%name (ClearEventLog) BOOLAPI
ClearEventLogW (
    HANDLE hEventLog,	// @pyparm int|handle||Handle to the event log to clear.
    WCHAR *INPUT_NULLOK // @pyparm <o PyUnicode>|eventLogName||The name of the event log to save to, or None
    );

// @pyswig |BackupEventLog|Backs up the event log
%name (BackupEventLog) BOOLAPI
BackupEventLogW (
    HANDLE hEventLog, // @pyparm int|handle||Handle to the event log to backup.
    WCHAR *lpBackupFileName // @pyparm <o PyUnicode>|eventLogName||The name of the event log to save to
    );


// @pyswig |CloseEventLog|Closes the eventlog
BOOLAPI
CloseEventLog (
    HANDLE hEventLog // @pyparm int|handle||Handle to the event log to close
    );

// @pyswig |DeregisterEventSource|Deregisters an Event Source
BOOLAPI
DeregisterEventSource (
    HANDLE hEventLog // @pyparm int|handle||Identifies the event log whose handle was returned by <om win32evtlog.RegisterEventSource.>
    );

// @pyswig |NotifyChangeEventLog|Lets an application receive notification when an event is written to the event log file specified by the hEventLog parameter. When the event is written to the event log file, the function causes the event object specified by the hEvent parameter to become signaled.
BOOLAPI
NotifyChangeEventLog(
    HANDLE  hEventLog, // @pyparm int|handle||Handle to an event log file, obtained by calling <om win32evtlog.OpenEventLog> function. When an event is written to this log file, the event specified by hEvent becomes signaled.
    PyHANDLE  hEvent // @pyparm int|handle||A handle to a Win32 event. This is the event that becomes signaled when an event is written to the event log file specified by the hEventLog parameter.

    );

// @pyswig int|GetNumberOfEventLogRecords|Returns the number of event log records.
BOOLAPI
GetNumberOfEventLogRecords (
    HANDLE hEventLog, // @pyparm int|handle||Handle to the event log to query.
    unsigned long *OUTPUT
    );

// @pyswig int|GetOldestEventLogRecord|Returns the number of event log records.
// @rdesc The result is the absolute record number of the oldest record in the given event log.
BOOLAPI
GetOldestEventLogRecord (
    HANDLE hEventLog,
    unsigned long *OUTPUT
    );

// @pyswig <o PyEVTLOG_HANDLE>|OpenEventLog|Opens an event log.
%name (OpenEventLog) PyEVTLOG_HANDLE OpenEventLogW (
    WCHAR *INPUT_NULLOK, // @pyparm <o PyUnicode>|serverName||The server name, or None
    WCHAR *sourceName    // @pyparm <o PyUnicode>|sourceName||specifies the name of the source that the returned handle will reference. The source name must be a subkey of a logfile entry under the EventLog key in the registry. 
    );

// @pyswig int|RegisterEventSource|Registers an Event Source
%name (RegisterEventSource) HANDLE
RegisterEventSourceW (
    WCHAR *INPUT_NULLOK, // @pyparm <o PyUnicode>|serverName||The server name, or None
    WCHAR *sourceName  // @pyparm <o PyUnicode>|sourceName||The source name
    );


// @pyswig <o PyEVTLOG_HANDLE>|OpenBackupEventLog|Opens a previously saved event log.
%name (OpenBackupEventLog) HANDLE OpenBackupEventLogW (
    WCHAR *INPUT_NULLOK, // @pyparm <o PyUnicode>|serverName||The server name, or None
    WCHAR *fileName      // @pyparm <o PyUnicode>|fileName||The filename to open
    );

// @pyswig [object,...]|ReadEventLog|Reads some event log records.
// @rdesc If there are no event log records available, then an empty list is returned.
%name (ReadEventLog) PyObject *MyReadEventLog (
     HANDLE     hEventLog, // @pyparm int|handle||The handle of the event log to read.
     DWORD      dwReadFlags, // @pyparm int|flags||The read flags
     DWORD      dwRecordOffset // @pyparm int|offset||The offset
    );
    

// @pyswig |ReportEvent|Reports an event
%name (ReportEvent) PyObject *MyReportEvent (
     HANDLE     hEventLog,	// @pyparm <o PyHANDLE>|EventLog||Handle to an event log
     WORD       wType,		// @pyparm int|Type||win32con.EVENTLOG_* value
     WORD       wCategory,	// @pyparm int|Category||Source-specific event category
     DWORD      dwEventID,	// @pyparm int|EventID||Source-specific event identifier
     PyObject   *obUserSid,	// @pyparm <o PySID>|UserSid||Sid of current user, can be None 
     PyObject   *obStrings,	// @pyparm sequence|Strings||Sequence of unicode strings to be inserted in message
     PyObject   *obRawData	// @pyparm str|RawData||Binary data for event, can be None
    );

%{


// New event log functions available on Vista and later
// @pyswig <o PyEVT_HANDLE>|EvtOpenChannelEnum|Begins an enumeration of event channels
// @comm Accepts keyword args
static PyObject *PyEvtOpenChannelEnum(PyObject *self, PyObject *args, PyObject *kwargs)
{
	static char *keywords[]={"Session", "Flags", NULL};
	EVT_HANDLE session=NULL, enum_handle;
	DWORD flags=0;
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|O&k:EvtOpenChannelEnum", keywords,
		PyWinObject_AsHANDLE, &session,	// @pyparm <o PyEVT_HANDLE>|Session|None|Handle to a remote session (see <om win32evtlog.EvtOpenSession>), or None for local machine.
		&flags))						// @pyparm int|Flags|0|Reserved, use only 0
		return NULL;
	enum_handle=EvtOpenChannelEnum(session, flags);
	if (enum_handle==NULL)
		return PyWin_SetAPIError("EvtOpenChannelEnum");
	return PyWinObject_FromEVT_HANDLE(enum_handle);
}
PyCFunction pfnPyEvtOpenChannelEnum = (PyCFunction) PyEvtOpenChannelEnum;

// @pyswig str|EvtNextChannelPath|Retrieves a channel path from an enumeration
// @rdesc Returns None at end of enumeration
// @comm Accepts keyword args
static PyObject *PyEvtNextChannelPath(PyObject *self, PyObject *args, PyObject *kwargs)
{
	static char *keywords[]={"ChannelEnum", NULL};
	EVT_HANDLE enum_handle;
	DWORD allocated_size=256, returned_size, err;
	WCHAR *buf=NULL;
	PyObject *ret=NULL;
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O&:EvtNextChannelPath", keywords,
		PyWinObject_AsHANDLE, &enum_handle))	// @pyparm <o PyEVT_HANDLE>|ChannelEnum||Handle to an enumeration as returned by <om win32evtlog.EvtOpenChannelEnum>
		return NULL;
	while (true){
		if (buf)
			free(buf);
		// MSDN docs say sizes are in bytes, but it doesn't seem to be so ???
		WCHAR *buf=(WCHAR *)malloc(allocated_size * sizeof(WCHAR));
		if (!buf)
			return NULL;
		if (EvtNextChannelPath(enum_handle, allocated_size, buf, &returned_size)){
			ret=PyWinObject_FromWCHAR(buf);
			break;
			}
		err=GetLastError();
		if (err==ERROR_INSUFFICIENT_BUFFER){
			allocated_size=returned_size;
			continue;
			}
		if (err==ERROR_NO_MORE_ITEMS){
			Py_INCREF(Py_None);
			ret=Py_None;
			break;
			}
		PyWin_SetAPIError("EvtNextChannelPath", err);
		break;
	}
	if (buf)
		free(buf);
	return ret;
}
PyCFunction pfnPyEvtNextChannelPath = (PyCFunction) PyEvtNextChannelPath;

// @pyswig <o PyEVT_HANDLE>|EvtOpenLog|Opens an event log or exported log archive
// @comm Accepts keyword args
static PyObject *PyEvtOpenLog(PyObject *self, PyObject *args, PyObject *kwargs)
{
	static char *keywords[]={"Path", "Flags", "Session", NULL};
	EVT_HANDLE session=NULL, log_handle;
	DWORD flags=0;
	WCHAR *path;
	PyObject *obpath;
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "Ok|O&:EvtOpenLog", keywords,
		&obpath,						// @pyparm str|Path||Event log name or Path of an export file
		&flags,						// @pyparm int|Flags||EvtOpenChannelPath (1) or EvtOpenFilePath (2)
		PyWinObject_AsHANDLE, &session))	// @pyparm <o PyEVT_HANDLE>|Session|None|Handle to a remote session (see <om win32evtlog.EvtOpenSession>), or None for local machine.
		return NULL;
	if (!PyWinObject_AsWCHAR(obpath, &path, FALSE))
		return NULL;
	log_handle=EvtOpenLog(session, path, flags);
	PyWinObject_FreeWCHAR(path);
	if (log_handle==NULL)
		return PyWin_SetAPIError("EvtOpenLog");
	return PyWinObject_FromEVT_HANDLE(log_handle);
}
PyCFunction pfnPyEvtOpenLog = (PyCFunction) PyEvtOpenLog;

// @pyswig |EvtClearLog|Clears an event log and optionally exports events to an archive
// @comm Accepts keyword args
static PyObject *PyEvtClearLog(PyObject *self, PyObject *args, PyObject *kwargs)
{
	static char *keywords[]={"ChannelPath", "TargetFilePath", "Session", "Flags", NULL};
	EVT_HANDLE session=NULL;
	DWORD flags=0;
	TmpWCHAR path, export_path;
	PyObject *obpath, *obexport_path=Py_None;
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O|OO&k:EvtClearLog", keywords,
		&obpath,	// @pyparm str|ChannelPath||Name of event log to be cleared
		&obexport_path, // @pyparm str|TargetFilePath|None|Name of file in which cleared events will be archived, or None
		PyWinObject_AsHANDLE, &session,	// @pyparm <o PyEVT_HANDLE>|Session|None|Handle to a remote session (see <om win32evtlog.EvtOpenSession>), or None for local machine.
		&flags))		// @pyparm int|Flags|0|Reserved, use only 0
		return NULL;
	if (!PyWinObject_AsWCHAR(obpath, &path, FALSE))
		return NULL;
	if (!PyWinObject_AsWCHAR(obexport_path, &export_path, TRUE))
		return NULL;
	if (EvtClearLog(session, path, export_path, flags)){
		Py_INCREF(Py_None);
		return Py_None;
		}
	return PyWin_SetAPIError("EvtClearLog");
}
PyCFunction pfnPyEvtClearLog = (PyCFunction) PyEvtClearLog;

// @pyswig |EvtExportLog|Exports events from a channel or log file
// @comm Accepts keyword args
static PyObject *PyEvtExportLog(PyObject *self, PyObject *args, PyObject *kwargs)
{
	static char *keywords[]={"Path", "TargetFilePath", "Flags", "Query", "Session", NULL};
	EVT_HANDLE session=NULL;
	DWORD flags=0;
	TmpWCHAR path, query, export_path;
	PyObject *obpath, *obexport_path, *obquery=Py_None;
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "OOk|OO&:EvtExportLog", keywords,
		&obpath,	// @pyparm str|Path||Path of a live event log channel or exported log file
		&obexport_path, // @pyparm str|TargetFilePath|None|Name of file in which cleared events will be archived, or None
		&flags,	// @pyparm int|Flags||Combination of EvtExportLog* flags specifying the type of path
		&obquery,	// @pyparm str|Query|None|Selects specific events to export
		PyWinObject_AsHANDLE, &session))	// @pyparm <o PyEVT_HANDLE>|Session|None|Handle to a remote session (see <om win32evtlog.EvtOpenSession>), or None for local machine.
		return NULL;
	if (!PyWinObject_AsWCHAR(obpath, &path, FALSE))
		return NULL;
	if (!PyWinObject_AsWCHAR(obexport_path, &export_path, FALSE))
		return NULL;
	if (!PyWinObject_AsWCHAR(obquery, &query, TRUE))
		return NULL;
	if (EvtExportLog(session, path, query, export_path, flags)){
		Py_INCREF(Py_None);
		return Py_None;
		}
	return PyWin_SetAPIError("EvtExportLog");
}
PyCFunction pfnPyEvtExportLog = (PyCFunction) PyEvtExportLog;

// @pyswig |EvtArchiveExportedLog|Localizes an exported event log file
// @comm Accepts keyword args
static PyObject *PyEvtArchiveExportedLog(PyObject *self, PyObject *args, PyObject *kwargs)
{
	static char *keywords[]={"LogFilePath", "Locale", "Session", "Flags", NULL};
	EVT_HANDLE session=NULL;
	DWORD flags=0;
	TmpWCHAR path;
	LCID lcid;
	PyObject *obpath;
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "Ol|O&k:EvtArchiveExportedLog", keywords,
		&obpath,	// @pyparm str|LogFilePath||Filename of an exported log file
		&lcid,	// @pyparm int|Locale||Locale id
		PyWinObject_AsHANDLE, &session,	// @pyparm <o PyEVT_HANDLE>|Session|None|Handle to a remote session (see <om win32evtlog.EvtOpenSession>), or None for local machine.
		&flags))	// @pyparm int|Flags|0|Reserved
		return NULL;
	if (!PyWinObject_AsWCHAR(obpath, &path, FALSE))
		return NULL;
	if (EvtArchiveExportedLog(session, path, lcid, flags)){
		Py_INCREF(Py_None);
		return Py_None;
		}
	return PyWin_SetAPIError("EvtArchiveExportedLog");
}
PyCFunction pfnPyEvtArchiveExportedLog = (PyCFunction) PyEvtArchiveExportedLog;

// @pyswig str|EvtGetExtendedStatus|Returns additional error info from last Evt* call
static PyObject *PyEvtGetExtendedStatus(PyObject *self, PyObject *args)
{
	DWORD buflen=0, bufneeded=1024;
	WCHAR *msg=NULL;
	PyObject *ret=NULL;
	if (!PyArg_ParseTuple(args, ":EvtGetExtendedStatus"))
		return NULL;
	while (1){
		if (msg)
			free(msg);
		buflen=bufneeded;
		msg=(WCHAR *)malloc(buflen * sizeof(WCHAR));
		if (msg==NULL){
			PyErr_NoMemory();
			return NULL;
			}
		if (EvtGetExtendedStatus(buflen, msg, &bufneeded)){
			ret=PyWinObject_FromWCHAR(msg, bufneeded);
			break;
			}
		if (bufneeded <= buflen){
			PyWin_SetAPIError("EvtGetExtendedStatus");
			break;
			}
		}
	if (msg)
		free(msg);
	return ret;
}
%}

%native (EvtOpenChannelEnum) pfnPyEvtOpenChannelEnum;
%native (EvtNextChannelPath) pfnPyEvtNextChannelPath;
%native (EvtOpenLog) pfnPyEvtOpenLog;
%native (EvtClearLog) pfnPyEvtClearLog;
%native (EvtExportLog) pfnPyEvtExportLog;
%native (EvtArchiveExportedLog) pfnPyEvtArchiveExportedLog;
%native (EvtGetExtendedStatus) PyEvtGetExtendedStatus;


%init %{
    for (PyMethodDef *pmd = win32evtlogMethods;pmd->ml_name;pmd++)
        if   ((strcmp(pmd->ml_name, "EvtOpenChannelEnum")==0)
			||(strcmp(pmd->ml_name, "EvtNextChannelPath")==0) 
			||(strcmp(pmd->ml_name, "EvtOpenLog")==0)
			||(strcmp(pmd->ml_name, "EvtClearLog")==0)
			||(strcmp(pmd->ml_name, "EvtOpenSession")==0)
			||(strcmp(pmd->ml_name, "EvtExportLog")==0)
			||(strcmp(pmd->ml_name, "EvtArchiveExportedLog")==0)
			){
			pmd->ml_flags = METH_VARARGS | METH_KEYWORDS;
			}
%}

// Used with EvtOpenLog
#define EvtOpenChannelPath EvtOpenChannelPath
#define EvtOpenFilePath EvtOpenFilePath

// EVT_EXPORTLOG_FLAGS, used with EvtExportLog
#define EvtExportLogChannelPath EvtExportLogChannelPath
#define EvtExportLogFilePath EvtExportLogFilePath
#define EvtExportLogTolerateQueryErrors EvtExportLogTolerateQueryErrors

