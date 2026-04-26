/* File : win32evtlog.i */

%module win32evtlog // A module, encapsulating the Windows Win32 event log API.

%include "typemaps.i"
%include "pywin32.i"

%{

#include <structmember.h>

#undef PyHANDLE
#include "PyWinObjects.h"
#include "WinEvt.h"

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
//	the Evt* event log functions.
//	When the object is destroyed, EvtClose is called.
class PyEVT_HANDLE: public PyHANDLE
{
public:
	PyEVT_HANDLE(HANDLE hInit, PyObject *context) : PyHANDLE(hInit){
		callback_objects = context;
		Py_XINCREF(callback_objects);
		}
	virtual BOOL Close(void){
		BOOL ret=EvtClose(m_handle);
		if (!ret)
			PyWin_SetAPIError("EvtClose");
		m_handle = 0;
		Py_XDECREF(callback_objects);
		callback_objects=NULL;
		return ret;
		}
	virtual const char *GetTypeName(){
		return "PyEVT_HANDLE";
		}
	// Only used with push subscription handles.  Will be a 2-tuple
	// that keeps references to the callback function and context object
	PyObject *callback_objects;
};

#define PyHANDLE HANDLE

PyObject *PyWinObject_FromEVTLOG_HANDLE(HANDLE h)
{
	PyObject *ret = new PyEVTLOG_HANDLE(h);
	if (!ret)
		PyErr_NoMemory();
	return ret;
}

PyObject *PyWinObject_FromEVT_HANDLE(HANDLE h, PyObject *context=NULL)
{
	PyObject *ret=new PyEVT_HANDLE(h, context);
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
	{"TimeGenerated",      T_OBJECT,  OFF(TimeGenerated)}, // @prop <o PyDateTime>|TimeGenerated|
	{"TimeWritten",        T_OBJECT,  OFF(TimeWritten)}, // @prop <o PyDateTime>|TimeWritten|
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

	Data = PyBytes_FromStringAndSize(((char *)pEvt)+pEvt->DataOffset, pEvt->DataLength);

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

PyObject *_MyReadEventLog(HANDLE hEventLog, DWORD dwReadFlags, DWORD dwRecordOffset, DWORD nNumberOfBytesToRead)
{
	DWORD needed = nNumberOfBytesToRead, read;
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

#define EVTLOG_READ_BUF_LEN_MAX 0x7ffff
#define EVTLOG_READ_BUF_LEN_DEFAULT 0x1000

// @pyswig [object,...]|ReadEventLog|Reads some event log records.
// @rdesc If there are no event log records available, then an empty list is returned.
PyObject *MyReadEventLog(PyObject *self, PyObject *args) {
    HANDLE hEventLog = INVALID_HANDLE_VALUE;
    DWORD dwReadFlags, dwRecordOffset, nNumberOfBytesToRead = EVTLOG_READ_BUF_LEN_DEFAULT;
    if (!PyArg_ParseTuple(args, "O&kk|k:ReadEventLog",
        PyWinObject_AsHANDLE, &hEventLog,  // @pyparm <o Py_HANDLE>|Handle||Handle to a an opened event log (see <om win32evtlog.OpenEventLog>)
        &dwReadFlags,                      // @pyparm int|Flags||Reading flags
        &dwRecordOffset,                   // @pyparm int|Offset||Record offset to read (in SEEK mode).
        &nNumberOfBytesToRead))            // @pyparm int|Size|4096|Output buffer size.
        return NULL;
    if (nNumberOfBytesToRead == 0)
        nNumberOfBytesToRead = EVTLOG_READ_BUF_LEN_DEFAULT;
    if (nNumberOfBytesToRead > EVTLOG_READ_BUF_LEN_MAX)
        nNumberOfBytesToRead = EVTLOG_READ_BUF_LEN_MAX;
    return _MyReadEventLog(hEventLog, dwReadFlags, dwRecordOffset, nNumberOfBytesToRead);
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
	PSID sid;
	if (!PyWinObject_AsSID(obSID, &sid, TRUE))
		return NULL;

	PyWinBufferView pybuf(obData, false, true);
	if (!pybuf.ok())
		return NULL;
	if (!PyWinObject_AsWCHARArray(obStrings, &pStrings, &numStrings, TRUE))
		return NULL;
	if (numStrings > USHRT_MAX){
		PyErr_Format(PyExc_ValueError, "String inserts can contain at most %d strings", USHRT_MAX);
		goto cleanup;
		}
	BOOL ok;
	Py_BEGIN_ALLOW_THREADS
	ok = ReportEventW(hEventLog, wType, wCategory,	dwEventID, sid, (WORD)numStrings, pybuf.len(), (const WCHAR **)pStrings, pybuf.ptr());
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

%native (ReadEventLog) MyReadEventLog;

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

PyObject *PyWinObject_FromEVT_VARIANT(PEVT_VARIANT val);

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
	Py_BEGIN_ALLOW_THREADS
	enum_handle=EvtOpenChannelEnum(session, flags);
	Py_END_ALLOW_THREADS
	if (enum_handle==NULL)
		return PyWin_SetAPIError("EvtOpenChannelEnum");
	return PyWinObject_FromEVT_HANDLE(enum_handle);
}
PyCFunction pfnPyEvtOpenChannelEnum = (PyCFunction) PyEvtOpenChannelEnum;


// Helper function to convert a list of strings double zero terminated
// into a Python list of strings.
// e.g. hello\0world\0\0 -> [ "hello", "world" ]
static PyObject* PyList_FromDoubleTerminatedWSTR(LPWSTR strings)
{
	PyObject* ret = PyList_New(0);
	if (ret == NULL) {
		return NULL;
	}

	WCHAR* cur = strings;
	while (*cur) {
		PyObject* keyword = PyWinObject_FromWCHAR(cur);
		PyList_Append(ret, keyword);
		Py_XDECREF(keyword);
		cur += wcslen(cur) + 1;
	}

	return ret;
}

// Used internally to format event messages
static PyObject *FormatMessageInternal(EVT_HANDLE metadata, EVT_HANDLE event, DWORD flags, DWORD resourceId)
{
	LPWSTR buf = NULL;
	PyObject *ret = NULL;
	DWORD allocated_size = 0;
	DWORD returned_size = 0;
	DWORD status = 0;
	DWORD err = 0;

	BOOL bsuccess = 0;
	Py_BEGIN_ALLOW_THREADS
	// Get the size of the buffer
	bsuccess = EvtFormatMessage(metadata, event, resourceId, 0, NULL, flags, allocated_size, buf, &returned_size);
	Py_END_ALLOW_THREADS

	err = GetLastError();

	// The above call should always return ERROR_INSUFFICIENT_BUFFER
	if (!bsuccess && err != ERROR_INSUFFICIENT_BUFFER) {
		return PyWin_SetAPIError("EvtFormatMessage");
	}

	allocated_size = returned_size;
	if (flags == EvtFormatMessageKeyword) {
		allocated_size += 1; // +1 to double terminate the keyword list
	}

	allocated_size *= sizeof(WCHAR);
	buf = (WCHAR *)malloc(allocated_size);
	if (buf == NULL) {
		PyErr_NoMemory();
		return NULL;
	}

	Py_BEGIN_ALLOW_THREADS
	bsuccess = EvtFormatMessage(metadata, event, resourceId, 0, NULL, flags, allocated_size, buf, &returned_size);
	Py_END_ALLOW_THREADS

	if (!bsuccess) {
		free(buf);
		return PyWin_SetAPIError("EvtFormatMessage");
	}

	if (flags == EvtFormatMessageKeyword) {
		buf[returned_size] = L'\0';
	}

	if (flags == EvtFormatMessageKeyword) {
		ret = PyList_FromDoubleTerminatedWSTR(buf);
	} else {
		ret = PyWinObject_FromWCHAR(buf);
	}

	free(buf);

	return ret;
}

// @pyswig str,list|EvtFormatMessage|Formats a message string.
// @rdesc Returns a formatted message string, or a list of strings if Flags=EvtFormatMessageKeyword
// @comm Accepts keyword args
static PyObject *PyEvtFormatMessage(PyObject *self, PyObject *args, PyObject *kwargs)
{
	static char *keywords[] = {"Metadata", "Event", "Flags", "ResourceId", NULL};
	EVT_HANDLE metadata_handle = NULL;
	EVT_HANDLE event_handle = NULL;
	DWORD flags = 0;
	DWORD resourceId = 0;
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O&O&k|k:EvtFormatMessage", keywords,
		PyWinObject_AsHANDLE, &metadata_handle,	// @pyparm <o PyEVT_HANDLE>|Metadata||Handle to provider metadata returned by <om win32evtlog.EvtOpenPublisherMetadata>
		PyWinObject_AsHANDLE, &event_handle,	// @pyparm <o PyEVT_HANDLE>|Event||Handle to an event
		&flags,	// @pyparm int|Flags||Type of message to format. EvtFormatMessageEvent or EvtFormatMessageLevel or EvtFormatMessageTask or EvtFormatMessageOpcode or EvtFormatMessageKeyword or EvtFormatMessageChannel or EvtFormatMessageProvider or EvtFormatMessageId or EvtFormatMessageXml.  If set to EvtFormatMessageId, callers should also set the 'ResourceId' parameter
		&resourceId))  // @pyparm int|ResourceId|0|The resource identifier of a message string returned by <om win32evtlog.EvtGetPublisherMetadataProperty>.  Only set this if flags = EvtFormatMessageId.
		return NULL;

	return FormatMessageInternal(metadata_handle, event_handle, flags, resourceId);
}
PyCFunction pfnPyEvtFormatMessage = (PyCFunction) PyEvtFormatMessage;

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
	BOOL bsuccess;
	while (true){
		if (buf)
			free(buf);
		// MSDN docs say sizes are in bytes, but it doesn't seem to be so ???
		WCHAR *buf=(WCHAR *)malloc(allocated_size * sizeof(WCHAR));
		if (!buf)
			return NULL;

		Py_BEGIN_ALLOW_THREADS
		bsuccess = EvtNextChannelPath(enum_handle, allocated_size, buf, &returned_size);
		Py_END_ALLOW_THREADS
		if (bsuccess){
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
	Py_BEGIN_ALLOW_THREADS
	log_handle=EvtOpenLog(session, path, flags);
	Py_END_ALLOW_THREADS
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
	BOOL bsuccess;
	Py_BEGIN_ALLOW_THREADS
	bsuccess = EvtClearLog(session, path, export_path, flags);
	Py_END_ALLOW_THREADS
	if (bsuccess){
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
		&obexport_path, // @pyparm str|TargetFilePath||File to create, cannot already exist
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
	BOOL bsuccess;
	Py_BEGIN_ALLOW_THREADS
	bsuccess = EvtExportLog(session, path, query, export_path, flags);
	Py_END_ALLOW_THREADS
	if (bsuccess){
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

	BOOL bsuccess;
	Py_BEGIN_ALLOW_THREADS
	bsuccess = EvtArchiveExportedLog(session, path, lcid, flags);
	Py_END_ALLOW_THREADS
	if (bsuccess){
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

	BOOL bsuccess;
	while (1){
		if (msg)
			free(msg);
		buflen=bufneeded;
		msg=(WCHAR *)malloc(buflen * sizeof(WCHAR));
		if (msg==NULL){
			PyErr_NoMemory();
			return NULL;
			}
		Py_BEGIN_ALLOW_THREADS
		bsuccess = EvtGetExtendedStatus(buflen, msg, &bufneeded);
		Py_END_ALLOW_THREADS
		if (bsuccess){
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

// @pyswig <o PyEVT_HANDLE>|EvtQuery|Opens a query over a log channel or exported log file
// @comm Accepts keyword args
static PyObject *PyEvtQuery(PyObject *self, PyObject *args, PyObject *kwargs)
{
	static char *keywords[]={"Path", "Flags", "Query", "Session", NULL};
	EVT_HANDLE ret, session=NULL;
	DWORD flags;
	TmpWCHAR path, query;
	PyObject *obpath, *obquery=Py_None;
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "Ol|OO&:EvtQuery", keywords,
		&obpath,	// @pyparm str|Path||Log channel or exported log file, depending on Flags
		&flags,		// @pyparm int|Flags||Combination of EVT_QUERY_FLAGS (EvtQuery*)
		&obquery,	// @pyparm str|Query|None|Selects events to return, None or '*' for all events
		PyWinObject_AsHANDLE, &session))	// @pyparm <o PyEVT_HANDLE>|Session|None|Handle to a remote session (see <om win32evtlog.EvtOpenSession>), or None for local machine.
		return NULL;
	if (!PyWinObject_AsWCHAR(obpath, &path, FALSE))
		return NULL;
	if (!PyWinObject_AsWCHAR(obquery, &query, TRUE))
		return NULL;
	Py_BEGIN_ALLOW_THREADS
	ret = EvtQuery(session, path, query, flags);
	Py_END_ALLOW_THREADS
	if (ret == NULL)
		return PyWin_SetAPIError("EvtQuery");
	return PyWinObject_FromEVT_HANDLE(ret);
}
PyCFunction pfnPyEvtQuery = (PyCFunction) PyEvtQuery;

// @pyswig (<o PyEVT_HANDLE>,...)|EvtNext|Returns events from a query
// @rdesc Returns a tuple of handles to events.  If no items are available, returns
//	an empty tuple instead of raising an exception.
// @comm Accepts keyword args
static PyObject *PyEvtNext(PyObject *self, PyObject *args, PyObject *kwargs)
{
	static char *keywords[]={"ResultSet", "Count", "Timeout", "Flags", NULL};
	EVT_HANDLE query;
	EVT_HANDLE *events =NULL;
	DWORD nbr_requested, nbr_returned, flags=0, timeout=(DWORD)-1;

	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O&k|kk:EvtNext", keywords,
		PyWinObject_AsHANDLE, &query,	// @pyparm <o PyEVT_HANDLE>|ResultSet||Handle to event query or subscription
		&nbr_requested,		// @pyparm int|Count||Number of events to return
		&timeout,	// @pyparm int|Timeout|-1|Time to wait in milliseconds, use -1 for infinite
		&flags))	// @pyparm int|Flags|0|Reserved, use only 0
		return NULL;
	events = (EVT_HANDLE *)malloc(nbr_requested * sizeof(EVT_HANDLE *));
	if (events==NULL){
		PyErr_NoMemory();
		return NULL;
		}
	BOOL bsuccess;
	Py_BEGIN_ALLOW_THREADS
	bsuccess = EvtNext(query, nbr_requested, events, timeout, flags, &nbr_returned);
	Py_END_ALLOW_THREADS
	if (!bsuccess){
		free(events);
		DWORD err=GetLastError();
		if (err == ERROR_NO_MORE_ITEMS || (err == ERROR_INVALID_OPERATION && nbr_returned == 0))
			return PyTuple_New(0);
		return PyWin_SetAPIError("EvtNext", err);
		}

	// If tuple construction fails, any handle not yet wrapped in a PyEVT_HANDLE
	// will be orphaned and remain open.  Should be a rare occurence, though.
	PyObject *ret=PyTuple_New(nbr_returned);
	if (ret){
		for (DWORD i=0;i<nbr_returned;i++){
			PyObject *obevt=PyWinObject_FromEVT_HANDLE(events[i]);
			if (obevt==NULL){
				Py_DECREF(ret);
				ret=NULL;
				break;
				}
			PyTuple_SET_ITEM(ret, i, obevt);
			}
		}
	free(events);
	return ret;
}
PyCFunction pfnPyEvtNext = (PyCFunction) PyEvtNext;

// @pyswig |EvtSeek|Changes the current position in a result set
// @comm Accepts keyword args
static PyObject *PyEvtSeek(PyObject *self, PyObject *args, PyObject *kwargs)
{
	static char *keywords[]={"ResultSet", "Position", "Flags", "Bookmark", "Timeout", NULL};
	EVT_HANDLE query, bookmark=NULL;
	DWORD flags, timeout=0;
	LONGLONG position;
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O&Lk|O&k:EvtSeek", keywords,
		PyWinObject_AsHANDLE, &query,	// @pyparm <o PyEVT_HANDLE>|ResultSet||Handle to event query or subscription
		&position,	// @pyparm int|Position||Offset (base from which to seek is specified by Flags)
		&flags,	// @pyparm int|Flags||EvtSeekRelative* flag indicating seek origin
		PyWinObject_AsHANDLE, &bookmark,	// @pyparm <o PyEVT_HANDLE>|Bookmark|None|Used as seek origin only if Flags contains EvtSeekRelativeToBookmark
		&timeout))	// @pyparm int|Timeout|0|Reserved, use only 0
		return NULL;
	BOOL bsuccess;
	Py_BEGIN_ALLOW_THREADS
	bsuccess = EvtSeek(query, position, bookmark, timeout, flags);
	Py_END_ALLOW_THREADS
	if (!bsuccess)
		return PyWin_SetAPIError("EvtSeek");
	Py_INCREF(Py_None);
	return Py_None;;
}
PyCFunction pfnPyEvtSeek = (PyCFunction) PyEvtSeek;

// @pyswig <o PyEVT_HANDLE>|EvtCreateRenderContext|Creates a render context
// @comm Accepts keyword args
static PyObject* PyEvtCreateRenderContext(PyObject *self, PyObject *args, PyObject *kwargs)
{
	static char *keywords[] = {"Flags", NULL};
	DWORD flags = EvtRenderContextSystem;

	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "k:EvtCreateRenderContext", keywords,
		&flags))	// @pyparm int|Flags||EvtRenderContextSystem or EvtRenderContextUser. EvtRenderContextValues not currently supported
		return NULL;

	EVT_HANDLE ret = NULL;
	Py_BEGIN_ALLOW_THREADS
	ret = EvtCreateRenderContext(0, NULL, flags);
	Py_END_ALLOW_THREADS

	if (ret == NULL) {
		return PyWin_SetAPIError("EvtCreateRenderContext", GetLastError());
	}

	return PyWinObject_FromEVT_HANDLE(ret);
}
PyCFunction pfnPyEvtCreateRenderContext = (PyCFunction) PyEvtCreateRenderContext;

// Returns a list of Event Values in the same order as returned by the EvtRender function
static PyObject *RenderEventValues(EVT_HANDLE render_context, EVT_HANDLE event)
{
	PyObject* ret = NULL;
	if (render_context == NULL) {
		PyWin_SetAPIError("EvtRender - Invalid render context for EvtRenderEventValues");
		return NULL;
	}

	DWORD allocated_size = 0;
	DWORD returned_size = 0;
	DWORD prop_count = 0;
	PEVT_VARIANT variants = NULL;
	BOOL bsuccess = 0;
	DWORD err = ERROR_SUCCESS;

	Py_BEGIN_ALLOW_THREADS
	bsuccess = EvtRender(render_context, event, EvtRenderEventValues, allocated_size, variants, &returned_size, &prop_count);
	Py_END_ALLOW_THREADS

	// bsuccess should always be false here, because we call it initially to get
	// the size of the buffer
	if (!bsuccess) {
		err = GetLastError();
		if (err != ERROR_INSUFFICIENT_BUFFER) {
			PyWin_SetAPIError("EvtRender", err);
			goto cleanup;
		}

		// allocate buffer size
		allocated_size = returned_size;
		variants = (PEVT_VARIANT)malloc(allocated_size);
		if (variants == NULL) {
			PyErr_NoMemory();
			goto cleanup;
		}

		Py_BEGIN_ALLOW_THREADS
		bsuccess = EvtRender(render_context, event, EvtRenderEventValues, allocated_size, variants, &returned_size, &prop_count);
		Py_END_ALLOW_THREADS
	}

	if (!bsuccess) {
		PyWin_SetAPIError("EvtRender");
		goto cleanup;
	}

	ret = PyList_New(prop_count);
	for (DWORD i = 0; i < prop_count; ++i) {
		PyObject* item = PyWinObject_FromEVT_VARIANT(&variants[i]);
		if (!item) {
			PyErr_Clear();
			Py_INCREF(Py_None);
			item = Py_None;
		}
		PyList_SetItem(ret, i, item);
	}

cleanup:

	if (variants) {
		free(variants);
	}

	return ret;

}

// @pyswig str|EvtRender|Formats an event into XML text or a Python Dict of key/values
// @comm Accepts keyword args
// @comm Rendering event values
static PyObject *PyEvtRender(PyObject *self, PyObject *args, PyObject *kwargs)
{
	static char *keywords[]={"Event", "Flags", "Context", NULL};
	EVT_HANDLE event;
	EVT_HANDLE render_context = NULL;
	void *buf=NULL;
	DWORD flags, bufsize=2048, bufneeded, propcount;
	PyObject *ret=NULL;

	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O&k|O&:EvtRender", keywords,
		PyWinObject_AsHANDLE, &event,	// @pyparm <o PyEVT_HANDLE>|Event||Handle to an event or bookmark
		&flags,	// @pyparm int|Flags||EvtRenderEventValues or EvtRenderEventXml or EvtRenderBookmark indicating type of handle
		PyWinObject_AsHANDLE, &render_context)) // @pyparm <o PyEVT_HANDLE>|Context|None|Handle to a render context returned by <om win32evtlog.EvtCreateRenderContext>
		return NULL;
	if (flags==EvtRenderEventValues){
		// pass this off to an internal function
		return RenderEventValues(render_context, event);
	}
	BOOL bsuccess;
	while(1){
		if (buf)
			free(buf);
		buf=malloc(bufsize);
		if (buf==NULL){
			PyErr_NoMemory();
			return NULL;
			}

		Py_BEGIN_ALLOW_THREADS
		bsuccess = EvtRender(NULL, event, flags, bufsize, buf, &bufneeded, &propcount);
		Py_END_ALLOW_THREADS
		if (bsuccess){
			ret=PyWinObject_FromWCHAR((WCHAR *)buf);
			break;
			}
		DWORD err=GetLastError();
		if (err==ERROR_INSUFFICIENT_BUFFER)
			bufsize=bufneeded;
		else{
			PyWin_SetAPIError("EvtRender", err);
			break;
			}
		}
	free(buf);
	return ret;
}
PyCFunction pfnPyEvtRender = (PyCFunction) PyEvtRender;


DWORD CALLBACK PyEvtSubscribe_callback(
	EVT_SUBSCRIBE_NOTIFY_ACTION action,
	void *context,
	EVT_HANDLE event)
{
	CEnterLeavePython celp;
	DWORD err=0;
	PyObject *func = PyTuple_GET_ITEM((PyObject *)context, 0);
	PyObject *obcontext = PyTuple_GET_ITEM((PyObject *)context, 1);
	PyObject *args=Py_BuildValue("kOO", action, obcontext, PyWinLong_FromHANDLE(event));
	if (args==NULL){
		// ??? Docs don't specify what happens when you return an error from callback
		// Need to check if subscription handle is closed ???
		PyErr_Print();
		return ERROR_OUTOFMEMORY;
		}
	PyObject *ret=PyObject_Call(func, args, NULL);
	if (ret==NULL){
		// Nothing to be done about an exception raised by the python callback
		PyErr_Print();
		err = ERROR_OUTOFMEMORY;
		}
	else if (ret!=Py_None){
		// Allow the callback to return an error
		err=PyLong_AsUnsignedLong(ret);
		if (err==(DWORD)-1 && PyErr_Occurred()){
			PyErr_Print();
			err = 0;
			}
		}

	Py_DECREF(args);
	Py_XDECREF(ret);
	return err;
}

// @pyswig <o PyEVT_HANDLE>|EvtSubscribe|Requests notification for events
// @comm Accepts keyword args
// @comm The method used to receive events is determined by the parameters passed in.
//	To create a push subscription, define a callback function that will be called with each event.
//	The function will receive 3 args:
//		First is an integer specifying why the function was called (EvtSubscribeActionError or EvtSubscribeActionDeliver)
//		Second is the context object passed to EvtSubscribe.
//		Third is the handle to an event log record (if not called due to an error)
//	If an event handle is passed in, a pull subscription is created.  The event handle will be
//	signalled when events are available, and the subscription handle can be
//	passed to <om win32evtlog.EvtNext> to obtain the events.

static PyObject *PyEvtSubscribe(PyObject *self, PyObject *args, PyObject *kwargs)
{
	static char *keywords[]={"ChannelPath", "Flags", "SignalEvent", "Callback", "Context",
		"Query", "Session", "Bookmark", NULL};
	EVT_HANDLE session=NULL, bookmark=NULL, ret;
	HANDLE signalevent=NULL;
	TmpWCHAR path, query;
	PyObject *obpath, *obcallback=Py_None, *obquery=Py_None, *obcontext=Py_None;
	TmpPyObject obuserdata;	// actual context passed to C++ callback - tuple of (function, context object)
	DWORD flags;
	EVT_SUBSCRIBE_CALLBACK pfncallback=NULL;
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "Ok|O&OOOO&O&:EvtSubscribe", keywords,
		&obpath,	// @pyparm str|ChannelPath||Name of an event log channel
		&flags,		// @pyparm int|Flags||Combination of EvtSubscribe* flags determining how subscription is initiated
		PyWinObject_AsHANDLE, &signalevent,	// @pyparm <o Py_HANDLE>|SignalEvent|None|An event handle to be set when events are available (see <om win32event.CreateEvent>)
		&obcallback,	// @pyparm function|Callback|None|Python function to be called with each event
		&obcontext,		// @pyparm object|Context|None|Arbitrary object to be passed to the callback function
		&obquery,		// @pyparm str|Query|None|XML query used to select specific events, use None or '*' for all events
		PyWinObject_AsHANDLE, &session,		// @pyparm <o PyEVT_HANDLE>|Session|None|Handle to a session on another machine, or None for local
		PyWinObject_AsHANDLE, &bookmark))	// @pyparm <o PyEVT_HANDLE>|Bookmark|None|If Flags contains EvtSubscribeStartAfterBookmark, used as starting point
		return NULL;

	if (!PyWinObject_AsWCHAR(obpath, &path, FALSE))
		return NULL;
	if (!PyWinObject_AsWCHAR(obquery, &query, TRUE))
		return NULL;
	if (obcallback != Py_None){
		pfncallback=PyEvtSubscribe_callback;
		obuserdata = Py_BuildValue("OO", obcallback, obcontext);
		if (obuserdata==NULL)
			return NULL;
		}
	Py_BEGIN_ALLOW_THREADS
	ret = EvtSubscribe(session, signalevent, path, query, bookmark,
		(void *)obuserdata, pfncallback, flags);
	Py_END_ALLOW_THREADS
	if (ret==NULL)
		return PyWin_SetAPIError("EvtSubscribe");
	return PyWinObject_FromEVT_HANDLE(ret, obuserdata);
}
PyCFunction pfnPyEvtSubscribe = (PyCFunction) PyEvtSubscribe;

// @pyswig <o PyEVT_HANDLE>|EvtCreateBookmark|Creates a bookmark
// @comm Accepts keyword args
static PyObject *PyEvtCreateBookmark(PyObject *self, PyObject *args, PyObject *kwargs)
{
	static char *keywords[]={"BookmarkXML", NULL};
	EVT_HANDLE ret;
	TmpWCHAR xml;
	PyObject *obxml=Py_None;
	// @pyparm str|BookmarkXML|None|XML representation of a bookmark as returned by <om win32evtlog.EvtRender>, or None for a new bookmark
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|O:EvtCreateBookmark", keywords,
		&obxml))
		return NULL;
	if (!PyWinObject_AsWCHAR(obxml, &xml, TRUE))
		return NULL;
	Py_BEGIN_ALLOW_THREADS
	ret = EvtCreateBookmark(xml);
	Py_END_ALLOW_THREADS
	if (ret == NULL)
		return PyWin_SetAPIError("EvtCreateBookmark");
	return PyWinObject_FromEVT_HANDLE(ret);
}
PyCFunction pfnPyEvtCreateBookmark = (PyCFunction) PyEvtCreateBookmark;

// @pyswig <o PyEVT_HANDLE>|EvtUpdateBookmark|Repositions a bookmark to an event
// @comm Accepts keyword args
static PyObject *PyEvtUpdateBookmark(PyObject *self, PyObject *args, PyObject *kwargs)
{
	static char *keywords[]={"Bookmark", "Event", NULL};
	EVT_HANDLE bookmark, evt;
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O&O&:EvtUpdateBookmark", keywords,
		PyWinObject_AsHANDLE, &bookmark,	// @pyparm <o PyEVT_HANDLE>|Bookmark||Handle to a bookmark
		PyWinObject_AsHANDLE, &evt))	// @pyparm <o PyEVT_HANDLE>|Event||Handle to an event
		return NULL;

	BOOL bsuccess;
	Py_BEGIN_ALLOW_THREADS
	bsuccess = EvtUpdateBookmark(bookmark, evt);
	Py_END_ALLOW_THREADS
	if (!bsuccess)
		return PyWin_SetAPIError("EvtUpdateBookmark");
	Py_INCREF(Py_None);
	return Py_None;
}
PyCFunction pfnPyEvtUpdateBookmark = (PyCFunction) PyEvtUpdateBookmark;

PyObject *PyList_FromEVT_VARIANTArray(PEVT_VARIANT val)
{
	if ((val->Type & EVT_VARIANT_TYPE_ARRAY) == 0) {
		PyErr_SetString(PyExc_TypeError, "Trying to create a list from an EVT_VARIANT that is not an array");
		return NULL;
	}

	PyObject* ret = PyList_New(val->Count);
	DWORD val_type = val->Type & EVT_VARIANT_TYPE_MASK;
	for (DWORD i = 0; i < val->Count; ++i) {
		PyObject *obval = NULL;
		switch (val_type) {
			case EvtVarTypeString:
				obval = PyWinObject_FromWCHAR(val->StringArr[i]);
				break;
			case EvtVarTypeAnsiString:
				obval = PyWinCoreString_FromString(val->AnsiStringArr[i]);
				break;
			case EvtVarTypeSByte:
				obval = PyLong_FromLong(val->SByteArr[i]);
				break;
			case EvtVarTypeByte:
				obval = PyLong_FromUnsignedLong(val->ByteArr[i]);
				break;
			case EvtVarTypeInt16:
				obval = PyLong_FromLong(val->Int16Arr[i]);
				break;
			case EvtVarTypeUInt16:
				obval = PyLong_FromUnsignedLong(val->UInt16Arr[i]);
				break;
			case EvtVarTypeInt32:
				obval = PyLong_FromLong(val->Int32Arr[i]);
				break;
			case EvtVarTypeUInt32:
				obval = PyLong_FromUnsignedLong(val->UInt32Arr[i]);
				break;
			case EvtVarTypeInt64:
				obval = PyLong_FromLongLong(val->Int64Val);
				break;
			case EvtVarTypeUInt64:
				obval = PyLong_FromUnsignedLongLong(val->UInt64Arr[i]);
				break;
			case EvtVarTypeSingle:
				obval = PyFloat_FromDouble(val->SingleArr[i]);
				break;
			case EvtVarTypeDouble:
				obval = PyFloat_FromDouble(val->DoubleArr[i]);
				break;
			case EvtVarTypeBoolean:
				obval = PyBool_FromLong(val->BooleanArr[i]);
				break;
			case EvtVarTypeGuid:
				obval = PyWinObject_FromIID(val->GuidArr[i]);
				break;
			case EvtVarTypeSizeT:
				obval = PyLong_FromSsize_t(val->SizeTArr[i]);
				break;
			case EvtVarTypeFileTime:
				{
				// FileTimeVal is defined as ULONGLONG but
				// FileTimeArr is defined as an array of FILETIME
				LARGE_INTEGER timestamp;
				timestamp.LowPart = val->FileTimeArr[i].dwLowDateTime;
				timestamp.HighPart = val->FileTimeArr[i].dwHighDateTime;
				obval = PyWinObject_FromTimeStamp(timestamp);
				break;
				}
			case EvtVarTypeSysTime:
				obval = PyWinObject_FromSYSTEMTIME(val->SysTimeArr[i]);
				break;
			case EvtVarTypeSid:
				obval = PyWinObject_FromSID(val->SidArr[i]);
				break;
			case EvtVarTypeHexInt32:
				{
				PyObject* number = PyLong_FromUnsignedLong(val->UInt32Arr[i]);
				obval = PyNumber_ToBase(number, 16);
				Py_XDECREF(number);
				break;
				}
			case EvtVarTypeHexInt64:
				{
				PyObject* number = PyLong_FromUnsignedLongLong(val->UInt64Arr[i]);
				obval = PyNumber_ToBase(number, 16);
				Py_XDECREF(number);
				break;
				}
			case EvtVarTypeEvtXml:
				obval = PyWinObject_FromWCHAR(val->XmlValArr[i]);
				break;
			default:
				return PyErr_Format(PyExc_NotImplementedError, "EVT_VARIANT_TYPE %d not supported yet", val_type);
				break;
		    }

		if (obval == NULL) {
			Py_INCREF(Py_None);
			obval = Py_None;
		}

		PyList_SetItem(ret, i, obval);
	}

	return ret;
}

PyObject *PyWinObject_FromEVT_VARIANT(PEVT_VARIANT val)
{
	if (val->Type & EVT_VARIANT_TYPE_ARRAY) {
		return PyList_FromEVT_VARIANTArray(val);
	}
	DWORD val_type = val->Type & EVT_VARIANT_TYPE_MASK;
	PyObject *obval = NULL;
	switch (val_type){
		case EvtVarTypeNull:
			Py_INCREF(Py_None);
			obval = Py_None;
			break;
		case EvtVarTypeString:
			obval = PyWinObject_FromWCHAR(val->StringVal);
			break;
		case EvtVarTypeAnsiString:
			obval = PyWinCoreString_FromString(val->AnsiStringVal);
			break;
		case EvtVarTypeSByte:
			obval = PyLong_FromLong(val->SByteVal);
			break;
		case EvtVarTypeByte:
			obval = PyLong_FromUnsignedLong(val->ByteVal);
			break;
		case EvtVarTypeInt16:
			obval = PyLong_FromLong(val->Int16Val);
			break;
		case EvtVarTypeUInt16:
			obval = PyLong_FromUnsignedLong(val->UInt16Val);
			break;
		case EvtVarTypeInt32:
			obval = PyLong_FromLong(val->Int32Val);
			break;
		case EvtVarTypeUInt32:
			obval = PyLong_FromUnsignedLong(val->UInt32Val);
			break;
		case EvtVarTypeInt64:
			obval = PyLong_FromLongLong(val->Int64Val);
			break;
		case EvtVarTypeUInt64:
			obval = PyLong_FromUnsignedLongLong(val->UInt64Val);
			break;
		case EvtVarTypeSingle:
			obval = PyFloat_FromDouble(val->SingleVal);
			break;
		case EvtVarTypeDouble:
			obval = PyFloat_FromDouble(val->DoubleVal);
			break;
		case EvtVarTypeBoolean:
			obval = PyBool_FromLong(val->BooleanVal);
			break;
		case EvtVarTypeBinary:
			obval = PyBytes_FromStringAndSize((char *)val->BinaryVal, val->Count);
			break;
		case EvtVarTypeGuid:
			obval = PyWinObject_FromIID(*val->GuidVal);
			break;
		case EvtVarTypeSizeT:
			obval = PyLong_FromSsize_t(val->SizeTVal);
			break;
		case EvtVarTypeFileTime:
			{
			// FileTimeVal is defined as ULONGLONG for unknown reasons
			LARGE_INTEGER timestamp;
			timestamp.QuadPart = val->FileTimeVal;
			obval = PyWinObject_FromTimeStamp(timestamp);
			break;
			}
		case EvtVarTypeSysTime:
			obval = PyWinObject_FromSYSTEMTIME(*val->SysTimeVal);
			break;
		case EvtVarTypeSid:
			obval = PyWinObject_FromSID(val->SidVal);
			break;
		case EvtVarTypeHexInt32:
			{
			PyObject* number = PyLong_FromUnsignedLong(val->UInt32Val);
			obval = PyNumber_ToBase(number, 16);
			Py_XDECREF(number);
			break;
			}
		case EvtVarTypeHexInt64:
			{
			PyObject* number = PyLong_FromUnsignedLongLong(val->UInt64Val);
			obval = PyNumber_ToBase(number, 16);
			Py_XDECREF(number);
			break;
			}
		case EvtVarTypeEvtHandle:
			obval = PyWinObject_FromEVT_HANDLE(val->EvtHandleVal);
			break;
		case EvtVarTypeEvtXml:
			obval = PyWinObject_FromWCHAR(val->XmlVal);
			break;
		default:
			PyErr_Format(PyExc_NotImplementedError, "EVT_VARIANT_TYPE %d not supported yet", val_type);
		}
	if (obval == NULL)
		return NULL;
	return Py_BuildValue("Nk", obval, val->Type);
}

// @pyswig (object, int)|EvtGetChannelConfigProperty|Retreives channel configuration information
// @comm Accepts keyword args
// @comm Returns the value and type of value (EvtVarType*)
static PyObject *PyEvtGetChannelConfigProperty(PyObject *self, PyObject *args, PyObject *kwargs)
{
	static char *keywords[]={"ChannelConfig", "PropertyId", "Flags", NULL};
	EVT_HANDLE config_handle;
	EVT_CHANNEL_CONFIG_PROPERTY_ID prop_id;
	DWORD flags = 0;
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O&i|k:EvtGetChannelConfigProperty", keywords,
		PyWinObject_AsHANDLE, &config_handle,	// @pyparm <o PyEVT_HANDLE>|ChannelConfig||Config handle as returned by <om win32evtlog.EvtOpenChannelConfig>
		&prop_id,	// @pyparm int|PropertyId||Property to retreive, one of EvtChannel* constants
		&flags))	// @pyparm int|Flags|0|Reserved, use only 0
		return NULL;

	PEVT_VARIANT val = NULL;
	DWORD buf_size=0, buf_needed, err;
	Py_BEGIN_ALLOW_THREADS
	EvtGetChannelConfigProperty(config_handle, prop_id, flags, buf_size, val, &buf_needed);
	Py_END_ALLOW_THREADS
	err = GetLastError();
	if (err != ERROR_INSUFFICIENT_BUFFER)
		return PyWin_SetAPIError("EvtGetChannelConfigProperty", err);
	val = (PEVT_VARIANT)malloc(buf_needed);
	if (val == NULL)
		return PyErr_Format(PyExc_MemoryError, "Unable to allocate %d bytes", buf_needed);
	buf_size = buf_needed;
	BOOL bsuccess;
	Py_BEGIN_ALLOW_THREADS
	bsuccess = EvtGetChannelConfigProperty(config_handle, prop_id, flags, buf_size, val, &buf_needed);
	Py_END_ALLOW_THREADS
	PyObject *ret = NULL;
	if (!bsuccess)
		PyWin_SetAPIError("EvtGetChannelConfigProperty");
	else
		ret = PyWinObject_FromEVT_VARIANT(val);
	free(val);
	return ret;
}
PyCFunction pfnPyEvtGetChannelConfigProperty = (PyCFunction) PyEvtGetChannelConfigProperty;

// @pyswig <o PyEVT_HANDLE>|EvtOpenChannelConfig|Opens channel configuration
// @comm Accepts keyword args
static PyObject *PyEvtOpenChannelConfig(PyObject *self, PyObject *args, PyObject *kwargs)
{
	static char *keywords[]={"ChannelPath", "Session", "Flags", NULL};
	EVT_HANDLE session = NULL, ret;
	PyObject *obchannel;
	TmpWCHAR channel;
	DWORD flags = 0;
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O|O&k:EvtOpenChannelConfig", keywords,
		&obchannel,	// @pyparm str|ChannelPath||Channel to be opened
		PyWinObject_AsHANDLE, &session,	// @pyparm <o PyEVT_HANDLE>|Session|None|Session handle as returned by <om win32evtlog.EvtOpenSession>, or None for local machine
		&flags))	// @pyparm int|Flags|0|Reserved, use only 0
		return NULL;
	if (!PyWinObject_AsWCHAR(obchannel, &channel, FALSE))
		return NULL;
	Py_BEGIN_ALLOW_THREADS
	ret = EvtOpenChannelConfig(session, channel, flags);
	Py_END_ALLOW_THREADS
	if (ret == NULL)
		return PyWin_SetAPIError("EvtOpenChannelConfig");
	return PyWinObject_FromEVT_HANDLE(ret);
}
PyCFunction pfnPyEvtOpenChannelConfig = (PyCFunction) PyEvtOpenChannelConfig;

void PyWinObject_FreeEVT_RPC_LOGIN(EVT_RPC_LOGIN *erl)
{
	PyWinObject_FreeWCHAR(erl->Server);
	PyWinObject_FreeWCHAR(erl->User);
	PyWinObject_FreeWCHAR(erl->Domain);
	PyWinObject_FreeWCHAR(erl->Password);
}

// @object PyEVT_RPC_LOGIN|Tuple containing login credentials for a remote Event Log connection
// @comm To use current login credentials, pass None for User, Domain, and Password
// @tupleitem 0|string|Server|Machine to connect to (only required item)
// @tupleitem 1|string|User|User account to login with, defaults to None
// @tupleitem 2|string|Domain|Domain of account, defaults to None
// @tupleitem 3|string|Password|Password, defaults to None
// @tupleitem 4|int|Flags|Type of authentication, EvtRpcLogin*.  Default is EvtRpcLoginAuthDefault
BOOL PyWinObject_AsEVT_RPC_LOGIN(PyObject *ob, EVT_RPC_LOGIN *erl)
{
	ZeroMemory(erl, sizeof(*erl));
	if (!PyTuple_Check(ob)){
		PyErr_Format(PyExc_TypeError, "PyEVT_RPC_LOGIN must be a tuple instead of %s", ob->ob_type->tp_name);
		return FALSE;
		}
	PyObject *observer, *obuser=Py_None, *obdomain=Py_None, *obpassword=Py_None;
	if (!PyArg_ParseTuple(ob, "O|OOOk", &observer, &obuser, &obdomain, &obpassword, &erl->Flags))
		return FALSE;
	if (PyWinObject_AsWCHAR(observer, &erl->Server, FALSE) &&
		PyWinObject_AsWCHAR(obuser, &erl->User, TRUE) &&
		PyWinObject_AsWCHAR(obdomain, &erl->Domain, TRUE) &&
		PyWinObject_AsWCHAR(obpassword, &erl->Password, TRUE))
		return TRUE;

	PyWinObject_FreeEVT_RPC_LOGIN(erl);
	return FALSE;
}

// @pyswig <o PyEVT_HANDLE>|EvtOpenSession|Creates a session used to access the Event Log on another machine
// @comm Accepts keyword args
static PyObject *PyEvtOpenSession(PyObject *self, PyObject *args, PyObject *kwargs)
{
	static char *keywords[]={"Login", "LoginClass", "Timeout", "Flags", NULL};
	EVT_RPC_LOGIN login = {NULL};
	EVT_HANDLE ret;
	PyObject *oblogin;
	EVT_LOGIN_CLASS loginclass = EvtRpcLogin;
	DWORD flags = 0, timeout = 0;
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O|ikk:EvtOpenSession", keywords,
		&oblogin,	// @pyparm <o PyEVT_RPC_LOGIN>|Login||Credentials to be used to access remote machine
		&loginclass,	// @pyparm int|LoginClass|EvtRpcLogin|Type of login to perform, EvtRpcLogin is only defined value
		&timeout,	// @pyparm int|Timeout|0|Reserved, use only 0
		&flags))	// @pyparm int|Flags|0|Reserved, use only 0
		return NULL;

	if (!PyWinObject_AsEVT_RPC_LOGIN(oblogin, &login))
		return NULL;
	Py_BEGIN_ALLOW_THREADS
	ret = EvtOpenSession(loginclass, &login, timeout, flags);
	Py_END_ALLOW_THREADS
	PyWinObject_FreeEVT_RPC_LOGIN(&login);
	if (ret == NULL)
		return PyWin_SetAPIError("EvtOpenSession");
	return PyWinObject_FromEVT_HANDLE(ret);
}
PyCFunction pfnPyEvtOpenSession = (PyCFunction) PyEvtOpenSession;

// @pyswig <o PyEVT_HANDLE>|EvtOpenPublisherEnum|Begins an enumeration of event publishers
// @comm Accepts keyword args
static PyObject *PyEvtOpenPublisherEnum(PyObject *self, PyObject *args, PyObject *kwargs)
{
	static char *keywords[]={"Session", "Flags", NULL};
	EVT_HANDLE session=NULL, enum_handle;
	DWORD flags=0;
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|O&k:EvtOpenPublisherEnum", keywords,
		PyWinObject_AsHANDLE, &session,	// @pyparm <o PyEVT_HANDLE>|Session|None|Handle to a remote session (see <om win32evtlog.EvtOpenSession>), or None for local machine.
		&flags))						// @pyparm int|Flags|0|Reserved, use only 0
		return NULL;
	Py_BEGIN_ALLOW_THREADS
	enum_handle=EvtOpenPublisherEnum(session, flags);
	Py_END_ALLOW_THREADS
	if (enum_handle==NULL)
		return PyWin_SetAPIError("EvtOpenPublisherEnum");
	return PyWinObject_FromEVT_HANDLE(enum_handle);
}
PyCFunction pfnPyEvtOpenPublisherEnum = (PyCFunction) PyEvtOpenPublisherEnum;

// @pyswig str|EvtNextPublisherId|Returns the next publisher from an enumeration
// @rdesc Returns None at end of enumeration
// @comm Accepts keyword args
static PyObject *PyEvtNextPublisherId(PyObject *self, PyObject *args, PyObject *kwargs)
{
	static char *keywords[]={"PublisherEnum", NULL};
	EVT_HANDLE enum_handle;
	DWORD allocated_size=256, returned_size, err;
	WCHAR *buf=NULL;
	PyObject *ret=NULL;
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O&:EvtNextPublisherId", keywords,
		PyWinObject_AsHANDLE, &enum_handle))	// @pyparm <o PyEVT_HANDLE>|PublisherEnum||Handle to an enumeration as returned by <om win32evtlog.EvtOpenPublisherEnum>
		return NULL;
	BOOL bsuccess;
	while (true){
		if (buf)
			free(buf);
		WCHAR *buf=(WCHAR *)malloc(allocated_size * sizeof(WCHAR));
		if (!buf)
			return NULL;

		Py_BEGIN_ALLOW_THREADS
		bsuccess = EvtNextPublisherId(enum_handle, allocated_size, buf, &returned_size);
		Py_END_ALLOW_THREADS
		if (bsuccess){
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
		PyWin_SetAPIError("EvtNextPublisherId", err);
		break;
	}
	if (buf)
		free(buf);
	return ret;
}
PyCFunction pfnPyEvtNextPublisherId = (PyCFunction) PyEvtNextPublisherId;

// @pyswig <o PyEVT_HANDLE>|EvtOpenPublisherMetadata|Opens a publisher to retrieve properties using <om win32evtlog.EvtGetPublisherMetadataProperty>
// @comm Accepts keyword args
static PyObject *PyEvtOpenPublisherMetadata(PyObject *self, PyObject *args, PyObject *kwargs)
{
	static char *keywords[]={"PublisherIdentity", "Session", "LogFilePath", "Locale", "Flags", NULL};
	PyObject *obpublisher, *oblogfile=Py_None;
	TmpWCHAR publisher, logfile;
	EVT_HANDLE session = NULL;
	LCID locale = 0;
	DWORD flags = 0;
	EVT_HANDLE ret;

	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O|O&Okk:EvtOpenPublisherMetadata", keywords,
		&obpublisher,	// @pyparm str|PublisherIdentity||Publisher id as returned by <om win32evtlog.EvtNextPublisherId>
		PyWinObject_AsHANDLE, &session,	// @pyparm <o PyEVT_HANDLE>|Session|None|Handle to remote session, or None for local machine
		&oblogfile,	// @pyparm str|LogFilePath|None|Log file from which to retrieve publisher, or None for locally registered publisher
		&locale,	// @pyparm int|Locale|0|Locale to use for retrieved properties, use 0 for current locale
		&flags))	// @pyparm int|Flags|0|Reserved, use only 0
		return NULL;
	if (!PyWinObject_AsWCHAR(obpublisher, &publisher, FALSE))
		return NULL;
	if (!PyWinObject_AsWCHAR(oblogfile, &logfile, TRUE))
		return NULL;

	Py_BEGIN_ALLOW_THREADS
	ret = EvtOpenPublisherMetadata(session, publisher, logfile, locale, flags);
	Py_END_ALLOW_THREADS
	if (ret == NULL)
		return PyWin_SetAPIError("EvtOpenPublisherMetadata");
	return PyWinObject_FromEVT_HANDLE(ret);
}
PyCFunction pfnPyEvtOpenPublisherMetadata = (PyCFunction) PyEvtOpenPublisherMetadata;

// @pyswig (object, int)|EvtGetPublisherMetadataProperty|Retrieves a property from an event publisher
// @comm Accepts keyword args
// @rdesc Returns the value and type of value (EvtVarType*)
// Some properties return a handle (type EvtVarTypeEvtHandle) which can be iterated using
// <om win32evtlog.EvtGetObjectArraySize> and <om win32evtlog.EvtGetObjectArrayProperty>.
static PyObject *PyEvtGetPublisherMetadataProperty(PyObject *self, PyObject *args, PyObject *kwargs)
{
	static char *keywords[]={"PublisherMetadata", "PropertyId", "Flags", NULL};
	EVT_HANDLE hpublisher;
	EVT_PUBLISHER_METADATA_PROPERTY_ID prop_id;
	DWORD flags = 0;
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O&i|k:EvtGetPublisherMetadataProperty", keywords,
		PyWinObject_AsHANDLE, &hpublisher,	// @pyparm <o PyEVT_HANDLE>|PublisherMetadata||Publisher handle as returned by <om win32evtlog.EvtOpenPublisherMetadata>
		&prop_id,	// @pyparm int|PropertyId||Property to retreive, EvtPublisherMetadata*
		&flags))	// @pyparm int|Flags|0|Reserved, use only 0
		return NULL;

	PEVT_VARIANT val = NULL;
	DWORD buf_size=0, buf_needed, err;
	Py_BEGIN_ALLOW_THREADS
	EvtGetPublisherMetadataProperty(hpublisher, prop_id, flags, buf_size, val, &buf_needed);
	Py_END_ALLOW_THREADS
	err = GetLastError();
	if (err != ERROR_INSUFFICIENT_BUFFER)
		return PyWin_SetAPIError("EvtGetPublisherMetadataProperty", err);
	val = (PEVT_VARIANT)malloc(buf_needed);
	if (val == NULL)
		return PyErr_Format(PyExc_MemoryError, "Unable to allocate %d bytes", buf_needed);
	buf_size = buf_needed;
	BOOL bsuccess;
	Py_BEGIN_ALLOW_THREADS
	bsuccess = EvtGetPublisherMetadataProperty(hpublisher, prop_id, flags, buf_size, val, &buf_needed);
	Py_END_ALLOW_THREADS
	PyObject *ret = NULL;
	if (!bsuccess)
		PyWin_SetAPIError("EvtGetPublisherMetadataProperty");
	else
		ret = PyWinObject_FromEVT_VARIANT(val);
	free(val);
	return ret;
}
PyCFunction pfnPyEvtGetPublisherMetadataProperty = (PyCFunction) PyEvtGetPublisherMetadataProperty;

// @pyswig <o PyEVT_HANDLE>|EvtOpenEventMetadataEnum|Enumerates the events that a publisher provides
// @comm Accepts keyword args
static PyObject *PyEvtOpenEventMetadataEnum(PyObject *self, PyObject *args, PyObject *kwargs)
{
	static char *keywords[]={"PublisherMetadata", "Flags", NULL};
	EVT_HANDLE hpublisher, enum_handle;
	DWORD flags=0;
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O&|k:EvtOpenEventMetadataEnum", keywords,
		PyWinObject_AsHANDLE, &hpublisher,	// @pyparm <o PyEVT_HANDLE>|PublisherMetadata||Publisher handle as returned by <om win32evtlog.EvtOpenPublisherMetadata>
		&flags))							// @pyparm int|Flags|0|Reserved, use only 0
		return NULL;
	Py_BEGIN_ALLOW_THREADS
	enum_handle=EvtOpenEventMetadataEnum(hpublisher, flags);
	Py_END_ALLOW_THREADS
	if (enum_handle==NULL)
		return PyWin_SetAPIError("EvtOpenEventMetadataEnum");
	return PyWinObject_FromEVT_HANDLE(enum_handle);
}
PyCFunction pfnPyEvtOpenEventMetadataEnum = (PyCFunction) PyEvtOpenEventMetadataEnum;

// @pyswig <o PyEVT_HANDLE>|EvtNextEventMetadata|Retrieves the next item from an event metadata enumeration
// @comm Accepts keyword args
static PyObject *PyEvtNextEventMetadata(PyObject *self, PyObject *args, PyObject *kwargs)
{
	static char *keywords[]={"EventMetadataEnum", "Flags", NULL};
	EVT_HANDLE henum, ret;
	DWORD flags = 0;
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O&|k:EvtNextEventMetadata", keywords,
		PyWinObject_AsHANDLE, &henum,	// @pyparm <o PyEVT_HANDLE>|EventMetadataEnum||Enumeration handle as returned by <om win32evtlog.EvtOpenEventMetadataEnum>
		&flags))						// @pyparm int|Flags|0|Reserved, use only 0
		return NULL;

	Py_BEGIN_ALLOW_THREADS
	ret = EvtNextEventMetadata(henum, flags);
	Py_END_ALLOW_THREADS
	if (ret != NULL)
		return PyWinObject_FromEVT_HANDLE(ret);
	DWORD err=GetLastError();
	if (err==ERROR_NO_MORE_ITEMS){
		Py_INCREF(Py_None);
		return Py_None;
		}
	return PyWin_SetAPIError("EvtNextEventMetadata");
}
PyCFunction pfnPyEvtNextEventMetadata = (PyCFunction) PyEvtNextEventMetadata;

// @pyswig (object, int)|EvtGetEventMetadataProperty|Retrieves a property from an event publisher
// @comm Accepts keyword args
// @rdesc Returns the value and type of value (EvtVarType*).
static PyObject *PyEvtGetEventMetadataProperty(PyObject *self, PyObject *args, PyObject *kwargs)
{
	static char *keywords[]={"EventMetadata", "PropertyId", "Flags", NULL};
	EVT_HANDLE hevent;
	EVT_EVENT_METADATA_PROPERTY_ID prop_id;
	DWORD flags = 0;
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O&i|k:EvtGetEventMetadataProperty", keywords,
		PyWinObject_AsHANDLE, &hevent,	// @pyparm <o PyEVT_HANDLE>|EventMetadata||Event metadata handle as returned by <om win32evtlog.EvtNextEventMetadata>
		&prop_id,	// @pyparm int|PropertyId||Property to retreive, EventMetadata*
		&flags))	// @pyparm int|Flags|0|Reserved, use only 0
		return NULL;

	PEVT_VARIANT val = NULL;
	DWORD buf_size=0, buf_needed, err;
	Py_BEGIN_ALLOW_THREADS
	EvtGetEventMetadataProperty(hevent, prop_id, flags, buf_size, val, &buf_needed);
	Py_END_ALLOW_THREADS
	err = GetLastError();
	if (err != ERROR_INSUFFICIENT_BUFFER)
		return PyWin_SetAPIError("EvtGetEventMetadataProperty", err);
	val = (PEVT_VARIANT)malloc(buf_needed);
	if (val == NULL)
		return PyErr_Format(PyExc_MemoryError, "Unable to allocate %d bytes", buf_needed);
	buf_size = buf_needed;
	BOOL bsuccess;
	Py_BEGIN_ALLOW_THREADS
	bsuccess = EvtGetEventMetadataProperty(hevent, prop_id, flags, buf_size, val, &buf_needed);
	Py_END_ALLOW_THREADS
	PyObject *ret = NULL;
	if (!bsuccess)
		PyWin_SetAPIError("EvtGetEventMetadataProperty");
	else
		ret = PyWinObject_FromEVT_VARIANT(val);
	free(val);
	return ret;
}
PyCFunction pfnPyEvtGetEventMetadataProperty = (PyCFunction) PyEvtGetEventMetadataProperty;

// @pyswig (object, int)|EvtGetLogInfo|Retrieves log file or channel information
// @comm Accepts keyword args
// @comm Returns the value and type of value (EvtVarType*)
static PyObject *PyEvtGetLogInfo(PyObject *self, PyObject *args, PyObject *kwargs)
{
	static char *keywords[]={"Log", "PropertyId", NULL};
	EVT_HANDLE hlog;
	EVT_LOG_PROPERTY_ID prop_id;
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O&i:EvtGetLogInfo", keywords,
		PyWinObject_AsHANDLE, &hlog,	// @pyparm <o PyEVT_HANDLE>|Log||Event log handle as returned by <om win32evtlog.EvtOpenLog>
		&prop_id))	// @pyparm int|PropertyId||Property to retreive, EvtLog*
		return NULL;

	PEVT_VARIANT val = NULL;
	DWORD buf_size=0, buf_needed, err;
	Py_BEGIN_ALLOW_THREADS
	EvtGetLogInfo(hlog, prop_id, buf_size, val, &buf_needed);
	Py_END_ALLOW_THREADS
	err = GetLastError();
	if (err != ERROR_INSUFFICIENT_BUFFER)
		return PyWin_SetAPIError("EvtGetLogInfo", err);
	val = (PEVT_VARIANT)malloc(buf_needed);
	if (val == NULL)
		return PyErr_Format(PyExc_MemoryError, "Unable to allocate %d bytes", buf_needed);
	buf_size = buf_needed;
	BOOL bsuccess;
	Py_BEGIN_ALLOW_THREADS
	bsuccess = EvtGetLogInfo(hlog, prop_id, buf_size, val, &buf_needed);
	Py_END_ALLOW_THREADS
	PyObject *ret = NULL;
	if (!bsuccess)
		PyWin_SetAPIError("EvtGetLogInfo");
	else
		ret = PyWinObject_FromEVT_VARIANT(val);
	free(val);
	return ret;
}
PyCFunction pfnPyEvtGetLogInfo = (PyCFunction) PyEvtGetLogInfo;

// @pyswig (object, int)|EvtGetEventInfo|Retrieves information about the source of an event
// @comm Accepts keyword args
// @comm Returns the value and type of value (EvtVarType*)
static PyObject *PyEvtGetEventInfo(PyObject *self, PyObject *args, PyObject *kwargs)
{
	static char *keywords[]={"Event", "PropertyId", NULL};
	EVT_HANDLE hevent;
	EVT_EVENT_PROPERTY_ID prop_id;
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O&i:EvtGetEventInfo", keywords,
		PyWinObject_AsHANDLE, &hevent,	// @pyparm <o PyEVT_HANDLE>|Event||Handle to an event
		&prop_id))	// @pyparm int|PropertyId||Property to retreive, EvtEvent*
		return NULL;

	PEVT_VARIANT val = NULL;
	DWORD buf_size=0, buf_needed, err;
	Py_BEGIN_ALLOW_THREADS
	EvtGetEventInfo(hevent, prop_id, buf_size, val, &buf_needed);
	Py_END_ALLOW_THREADS
	err = GetLastError();
	if (err != ERROR_INSUFFICIENT_BUFFER)
		return PyWin_SetAPIError("EvtGetEventInfo", err);
	val = (PEVT_VARIANT)malloc(buf_needed);
	if (val == NULL)
		return PyErr_Format(PyExc_MemoryError, "Unable to allocate %d bytes", buf_needed);
	buf_size = buf_needed;
	BOOL bsuccess;
	Py_BEGIN_ALLOW_THREADS
	bsuccess = EvtGetEventInfo(hevent, prop_id, buf_size, val, &buf_needed);
	Py_END_ALLOW_THREADS
	PyObject *ret = NULL;
	if (!bsuccess)
		PyWin_SetAPIError("EvtGetEventInfo");
	else
		ret = PyWinObject_FromEVT_VARIANT(val);
	free(val);
	return ret;
}
PyCFunction pfnPyEvtGetEventInfo = (PyCFunction) PyEvtGetEventInfo;

// @pyswig int|EvtGetObjectArraySize|Returns the size of an array of event objects
// @comm Accepts keyword args
static PyObject *PyEvtGetObjectArraySize(PyObject *self, PyObject *args, PyObject *kwargs)
{
	static char *keywords[]={"ObjectArray", NULL};
	EVT_HANDLE harray;
	DWORD size;
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O&:EvtGetObjectArraySize", keywords,
		PyWinObject_AsHANDLE, &harray))	// @pyparm <o PyEVT_HANDLE>|ObjectArray||Handle to an array of objects as returned by <om win32evtlog.EvtGetPublisherMetadataProperty> for some ProperyId's
		return NULL;
	BOOL bsuccess;
	Py_BEGIN_ALLOW_THREADS
	bsuccess = EvtGetObjectArraySize(harray, &size);
	Py_END_ALLOW_THREADS
	if (!bsuccess)
		return PyWin_SetAPIError("EvtGetObjectArraySize");
	return PyLong_FromUnsignedLong(size);
}
PyCFunction pfnPyEvtGetObjectArraySize = (PyCFunction) PyEvtGetObjectArraySize;

// @pyswig (object, int)|EvtGetObjectArrayProperty|Retrieves an item from an object array
// @comm Accepts keyword args
// @rdesc Returns the value and type of value (EvtVarType*)
static PyObject *PyEvtGetObjectArrayProperty(PyObject *self, PyObject *args, PyObject *kwargs)
{
	static char *keywords[]={"ObjectArray", "PropertyId", "ArrayIndex", "Flags", NULL};
	EVT_HANDLE harray;
	DWORD prop_id, index, flags = 0;
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O&kk|k:EvtGetObjectArrayProperty", keywords,
		PyWinObject_AsHANDLE, &harray,	// @pyparm <o PyEVT_HANDLE>|ObjectArray||Handle to an array of objects as returned by <om win32evtlog.EvtGetPublisherMetadataProperty> for some ProperyId's
		&prop_id,	// @pyparm int|PropertyId||Type of property contained in the array
		&index,		// @pyparm int|ArrayIndex||Zero-based index of item to retrieve
		&flags))	// @pyparm int|Flags|0|Reserved, use only 0
		return NULL;

	PEVT_VARIANT val = NULL;
	DWORD buf_size=0, buf_needed, err;
	Py_BEGIN_ALLOW_THREADS
	EvtGetObjectArrayProperty(harray, prop_id, index, flags, buf_size, val, &buf_needed);
	Py_END_ALLOW_THREADS
	err = GetLastError();
	if (err != ERROR_INSUFFICIENT_BUFFER)
		return PyWin_SetAPIError("EvtGetObjectArrayProperty", err);
	val = (PEVT_VARIANT)malloc(buf_needed);
	if (val == NULL)
		return PyErr_Format(PyExc_MemoryError, "Unable to allocate %d bytes", buf_needed);
	buf_size = buf_needed;
	BOOL bsuccess;
	Py_BEGIN_ALLOW_THREADS
	bsuccess = EvtGetObjectArrayProperty(harray, prop_id, index, flags, buf_size, val, &buf_needed);
	Py_END_ALLOW_THREADS
	PyObject *ret = NULL;
	if (!bsuccess)
		PyWin_SetAPIError("EvtGetObjectArrayProperty");
	else
		ret = PyWinObject_FromEVT_VARIANT(val);
	free(val);
	return ret;
}
PyCFunction pfnPyEvtGetObjectArrayProperty = (PyCFunction) PyEvtGetObjectArrayProperty;
%}


%native (EvtCreateRenderContext) pfnPyEvtCreateRenderContext;
%native (EvtFormatMessage) pfnPyEvtFormatMessage;
%native (EvtOpenChannelEnum) pfnPyEvtOpenChannelEnum;
%native (EvtNextChannelPath) pfnPyEvtNextChannelPath;
%native (EvtOpenLog) pfnPyEvtOpenLog;
%native (EvtClearLog) pfnPyEvtClearLog;
%native (EvtExportLog) pfnPyEvtExportLog;
%native (EvtArchiveExportedLog) pfnPyEvtArchiveExportedLog;
%native (EvtGetExtendedStatus) PyEvtGetExtendedStatus;
%native (EvtQuery) pfnPyEvtQuery;
%native (EvtNext) pfnPyEvtNext;
%native (EvtSeek) pfnPyEvtSeek;
%native (EvtRender) pfnPyEvtRender;
%native (EvtSubscribe) pfnPyEvtSubscribe;
%native (EvtCreateBookmark) pfnPyEvtCreateBookmark;
%native (EvtUpdateBookmark) pfnPyEvtUpdateBookmark;
%native (EvtGetChannelConfigProperty) pfnPyEvtGetChannelConfigProperty;
%native (EvtOpenChannelConfig) pfnPyEvtOpenChannelConfig;
%native (EvtOpenSession) pfnPyEvtOpenSession;
%native (EvtOpenPublisherEnum) pfnPyEvtOpenPublisherEnum;
%native (EvtNextPublisherId) pfnPyEvtNextPublisherId;
%native (EvtOpenPublisherMetadata) pfnPyEvtOpenPublisherMetadata;
%native (EvtGetPublisherMetadataProperty) pfnPyEvtGetPublisherMetadataProperty;
%native (EvtOpenEventMetadataEnum) pfnPyEvtOpenEventMetadataEnum;
%native (EvtNextEventMetadata) pfnPyEvtNextEventMetadata;
%native (EvtGetEventMetadataProperty) pfnPyEvtGetEventMetadataProperty;
%native (EvtGetLogInfo) pfnPyEvtGetLogInfo;
%native (EvtGetEventInfo) pfnPyEvtGetEventInfo;
%native (EvtGetObjectArraySize) pfnPyEvtGetObjectArraySize;
%native (EvtGetObjectArrayProperty) pfnPyEvtGetObjectArrayProperty;


%init %{
    if (PyType_Ready(&PyEventLogRecordType) == -1)
        PYWIN_MODULE_INIT_RETURN_ERROR;

    for (PyMethodDef *pmd = win32evtlogMethods;pmd->ml_name;pmd++)
        if   ((strcmp(pmd->ml_name, "EvtOpenChannelEnum")==0)
			||(strcmp(pmd->ml_name, "EvtCreateRenderContext")==0)
			||(strcmp(pmd->ml_name, "EvtFormatMessage")==0)
			||(strcmp(pmd->ml_name, "EvtNextChannelPath")==0)
			||(strcmp(pmd->ml_name, "EvtOpenLog")==0)
			||(strcmp(pmd->ml_name, "EvtClearLog")==0)
			||(strcmp(pmd->ml_name, "EvtOpenSession")==0)
			||(strcmp(pmd->ml_name, "EvtExportLog")==0)
			||(strcmp(pmd->ml_name, "EvtArchiveExportedLog")==0)
			||(strcmp(pmd->ml_name, "EvtQuery")==0)
			||(strcmp(pmd->ml_name, "EvtNext")==0)
			||(strcmp(pmd->ml_name, "EvtSeek")==0)
			||(strcmp(pmd->ml_name, "EvtRender")==0)
			||(strcmp(pmd->ml_name, "EvtSubscribe")==0)
			||(strcmp(pmd->ml_name, "EvtCreateBookmark")==0)
			||(strcmp(pmd->ml_name, "EvtUpdateBookmark")==0)
			||(strcmp(pmd->ml_name, "EvtGetChannelConfigProperty")==0)
			||(strcmp(pmd->ml_name, "EvtOpenChannelConfig")==0)
			||(strcmp(pmd->ml_name, "EvtOpenSession")==0)
			||(strcmp(pmd->ml_name, "EvtOpenPublisherEnum")==0)
			||(strcmp(pmd->ml_name, "EvtNextPublisherId")==0)
			||(strcmp(pmd->ml_name, "EvtOpenPublisherMetadata")==0)
			||(strcmp(pmd->ml_name, "EvtGetPublisherMetadataProperty")==0)
			||(strcmp(pmd->ml_name, "EvtOpenEventMetadataEnum")==0)
			||(strcmp(pmd->ml_name, "EvtNextEventMetadata")==0)
			||(strcmp(pmd->ml_name, "EvtGetEventMetadataProperty")==0)
			||(strcmp(pmd->ml_name, "EvtGetLogInfo")==0)
			||(strcmp(pmd->ml_name, "EvtGetEventInfo")==0)
			||(strcmp(pmd->ml_name, "EvtGetObjectArraySize")==0)
			||(strcmp(pmd->ml_name, "EvtGetObjectArrayProperty")==0)
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

// EVT_FORMAT_MESSAGE_FLAGS, used with EvtFormatMessage
#define EvtFormatMessageEvent EvtFormatMessageEvent
#define EvtFormatMessageLevel EvtFormatMessageLevel
#define EvtFormatMessageTask EvtFormatMessageTask
#define EvtFormatMessageOpcode EvtFormatMessageOpcode
#define EvtFormatMessageKeyword EvtFormatMessageKeyword
#define EvtFormatMessageChannel EvtFormatMessageChannel
#define EvtFormatMessageProvider EvtFormatMessageProvider
#define EvtFormatMessageId EvtFormatMessageId
#define EvtFormatMessageXml EvtFormatMessageXml

//fields available when rendering events using EvtRenderEventValues with a EvtRenderContextSystem
#define EvtSystemProviderName EvtSystemProviderName
#define EvtSystemProviderGuid EvtSystemProviderGuid
#define EvtSystemEventID EvtSystemEventID
#define EvtSystemQualifiers EvtSystemQualifiers
#define EvtSystemLevel EvtSystemLevel
#define EvtSystemTask EvtSystemTask
#define EvtSystemOpcode EvtSystemOpcode
#define EvtSystemKeywords EvtSystemKeywords
#define EvtSystemTimeCreated EvtSystemTimeCreated
#define EvtSystemEventRecordId EvtSystemEventRecordId
#define EvtSystemActivityID EvtSystemActivityID
#define EvtSystemRelatedActivityID EvtSystemRelatedActivityID
#define EvtSystemProcessID EvtSystemProcessID
#define EvtSystemThreadID EvtSystemThreadID
#define EvtSystemChannel EvtSystemChannel
#define EvtSystemComputer EvtSystemComputer
#define EvtSystemUserID EvtSystemUserID
#define EvtSystemVersion EvtSystemVersion
#define EvtSystemPropertyIdEND EvtSystemPropertyIdEND

// EVT_QUERY_FLAGS used with EvtQuery
#define EvtQueryChannelPath EvtQueryChannelPath
#define EvtQueryFilePath EvtQueryFilePath
#define EvtQueryForwardDirection EvtQueryForwardDirection
#define EvtQueryReverseDirection EvtQueryReverseDirection
#define EvtQueryTolerateQueryErrors EvtQueryTolerateQueryErrors

// EVT_SEEK_FLAGS used with EvtSeek
#define EvtSeekRelativeToFirst EvtSeekRelativeToFirst
#define EvtSeekRelativeToLast EvtSeekRelativeToLast
#define EvtSeekRelativeToCurrent EvtSeekRelativeToCurrent
#define EvtSeekRelativeToBookmark EvtSeekRelativeToBookmark
#define EvtSeekOriginMask EvtSeekOriginMask
#define EvtSeekStrict EvtSeekStrict

// EVT_RENDER_FLAGS
#define EvtRenderEventValues EvtRenderEventValues
#define EvtRenderEventXml EvtRenderEventXml
#define EvtRenderBookmark EvtRenderBookmark

// EVT_RENDER_CONTEXT_FLAGS
#define EvtRenderContextValues EvtRenderContextValues
#define EvtRenderContextSystem EvtRenderContextSystem
#define EvtRenderContextUser EvtRenderContextUser

// EvtSubscribe flags
#define EvtSubscribeToFutureEvents EvtSubscribeToFutureEvents
#define EvtSubscribeStartAtOldestRecord EvtSubscribeStartAtOldestRecord
#define EvtSubscribeStartAfterBookmark EvtSubscribeStartAfterBookmark
#define EvtSubscribeOriginMask EvtSubscribeOriginMask
#define EvtSubscribeTolerateQueryErrors EvtSubscribeTolerateQueryErrors
#define EvtSubscribeStrict EvtSubscribeStrict

// EVT_SUBSCRIBE_NOTIFY_ACTION - passed as first parm to EvtSubscribe callback
#define EvtSubscribeActionError EvtSubscribeActionError
#define EvtSubscribeActionDeliver EvtSubscribeActionDeliver

// EVT_VARIANT_TYPE
#define EvtVarTypeNull EvtVarTypeNull
#define EvtVarTypeString EvtVarTypeString
#define EvtVarTypeAnsiString EvtVarTypeAnsiString
#define EvtVarTypeSByte EvtVarTypeSByte
#define EvtVarTypeByte EvtVarTypeByte
#define EvtVarTypeInt16 EvtVarTypeInt16
#define EvtVarTypeUInt16 EvtVarTypeUInt16
#define EvtVarTypeInt32 EvtVarTypeInt32
#define EvtVarTypeUInt32 EvtVarTypeUInt32
#define EvtVarTypeInt64 EvtVarTypeInt64
#define EvtVarTypeUInt64 EvtVarTypeUInt64
#define EvtVarTypeSingle EvtVarTypeSingle
#define EvtVarTypeDouble EvtVarTypeDouble
#define EvtVarTypeBoolean EvtVarTypeBoolean
#define EvtVarTypeBinary EvtVarTypeBinary
#define EvtVarTypeGuid EvtVarTypeGuid
#define EvtVarTypeSizeT EvtVarTypeSizeT
#define EvtVarTypeFileTime EvtVarTypeFileTime
#define EvtVarTypeSysTime EvtVarTypeSysTime
#define EvtVarTypeSid EvtVarTypeSid
#define EvtVarTypeHexInt32 EvtVarTypeHexInt32
#define EvtVarTypeHexInt64 EvtVarTypeHexInt64
#define EvtVarTypeEvtHandle EvtVarTypeEvtHandle
#define EvtVarTypeEvtXml EvtVarTypeEvtXml

// EVT_CHANNEL_CONFIG_PROPERTY_ID
#define EvtChannelConfigEnabled EvtChannelConfigEnabled
#define EvtChannelConfigIsolation EvtChannelConfigIsolation
#define EvtChannelConfigType EvtChannelConfigType
#define EvtChannelConfigOwningPublisher EvtChannelConfigOwningPublisher
#define EvtChannelConfigClassicEventlog EvtChannelConfigClassicEventlog
#define EvtChannelConfigAccess EvtChannelConfigAccess
#define EvtChannelLoggingConfigRetention EvtChannelLoggingConfigRetention
#define EvtChannelLoggingConfigAutoBackup EvtChannelLoggingConfigAutoBackup
#define EvtChannelLoggingConfigMaxSize EvtChannelLoggingConfigMaxSize
#define EvtChannelLoggingConfigLogFilePath EvtChannelLoggingConfigLogFilePath
#define EvtChannelPublishingConfigLevel EvtChannelPublishingConfigLevel
#define EvtChannelPublishingConfigKeywords EvtChannelPublishingConfigKeywords
#define EvtChannelPublishingConfigControlGuid EvtChannelPublishingConfigControlGuid
#define EvtChannelPublishingConfigBufferSize EvtChannelPublishingConfigBufferSize
#define EvtChannelPublishingConfigMinBuffers EvtChannelPublishingConfigMinBuffers
#define EvtChannelPublishingConfigMaxBuffers EvtChannelPublishingConfigMaxBuffers
#define EvtChannelPublishingConfigLatency EvtChannelPublishingConfigLatency
#define EvtChannelPublishingConfigClockType EvtChannelPublishingConfigClockType
#define EvtChannelPublishingConfigSidType EvtChannelPublishingConfigSidType
#define EvtChannelPublisherList EvtChannelPublisherList
#ifdef EvtChannelPublishingConfigFileMax // this is only in SDK versions 7 and up
#define EvtChannelPublishingConfigFileMax EvtChannelPublishingConfigFileMax
#endif
#define EvtChannelConfigPropertyIdEND EvtChannelConfigPropertyIdEND

// Login type used with EvtOpenSession
#define EvtRpcLogin EvtRpcLogin

// Login flags using in Login param of EvtOpenSession
#define EvtRpcLoginAuthDefault EvtRpcLoginAuthDefault
#define EvtRpcLoginAuthNegotiate EvtRpcLoginAuthNegotiate
#define EvtRpcLoginAuthKerberos EvtRpcLoginAuthKerberos
#define EvtRpcLoginAuthNTLM EvtRpcLoginAuthNTLM

// EVT_PUBLISHER_METADATA_PROPERTY_ID
#define EvtPublisherMetadataPublisherGuid EvtPublisherMetadataPublisherGuid
#define EvtPublisherMetadataResourceFilePath EvtPublisherMetadataResourceFilePath
#define EvtPublisherMetadataParameterFilePath EvtPublisherMetadataParameterFilePath
#define EvtPublisherMetadataMessageFilePath EvtPublisherMetadataMessageFilePath
#define EvtPublisherMetadataHelpLink EvtPublisherMetadataHelpLink
#define EvtPublisherMetadataPublisherMessageID EvtPublisherMetadataPublisherMessageID
#define EvtPublisherMetadataChannelReferences EvtPublisherMetadataChannelReferences
#define EvtPublisherMetadataChannelReferencePath EvtPublisherMetadataChannelReferencePath
#define EvtPublisherMetadataChannelReferenceIndex EvtPublisherMetadataChannelReferenceIndex
#define EvtPublisherMetadataChannelReferenceID EvtPublisherMetadataChannelReferenceID
#define EvtPublisherMetadataChannelReferenceFlags EvtPublisherMetadataChannelReferenceFlags
#define EvtPublisherMetadataChannelReferenceMessageID EvtPublisherMetadataChannelReferenceMessageID
#define EvtPublisherMetadataLevels EvtPublisherMetadataLevels
#define EvtPublisherMetadataLevelName EvtPublisherMetadataLevelName
#define EvtPublisherMetadataLevelValue EvtPublisherMetadataLevelValue
#define EvtPublisherMetadataLevelMessageID EvtPublisherMetadataLevelMessageID
#define EvtPublisherMetadataTasks EvtPublisherMetadataTasks
#define EvtPublisherMetadataTaskName EvtPublisherMetadataTaskName
#define EvtPublisherMetadataTaskEventGuid EvtPublisherMetadataTaskEventGuid
#define EvtPublisherMetadataTaskValue EvtPublisherMetadataTaskValue
#define EvtPublisherMetadataTaskMessageID EvtPublisherMetadataTaskMessageID
#define EvtPublisherMetadataOpcodes EvtPublisherMetadataOpcodes
#define EvtPublisherMetadataOpcodeName EvtPublisherMetadataOpcodeName
#define EvtPublisherMetadataOpcodeValue EvtPublisherMetadataOpcodeValue
#define EvtPublisherMetadataOpcodeMessageID EvtPublisherMetadataOpcodeMessageID
#define EvtPublisherMetadataKeywords EvtPublisherMetadataKeywords
#define EvtPublisherMetadataKeywordName EvtPublisherMetadataKeywordName
#define EvtPublisherMetadataKeywordValue EvtPublisherMetadataKeywordValue
#define EvtPublisherMetadataKeywordMessageID EvtPublisherMetadataKeywordMessageID
#define EvtPublisherMetadataPropertyIdEND EvtPublisherMetadataPropertyIdEND

// EVT_EVENT_METADATA_PROPERTY_ID used with EvtGetEventMetadataProperty
#define EventMetadataEventID EventMetadataEventID
#define EventMetadataEventVersion EventMetadataEventVersion
#define EventMetadataEventChannel EventMetadataEventChannel
#define EventMetadataEventLevel EventMetadataEventLevel
#define EventMetadataEventOpcode EventMetadataEventOpcode
#define EventMetadataEventTask EventMetadataEventTask
#define EventMetadataEventKeyword EventMetadataEventKeyword
#define EventMetadataEventMessageID EventMetadataEventMessageID
#define EventMetadataEventTemplate EventMetadataEventTemplate
#define EvtEventMetadataPropertyIdEND EvtEventMetadataPropertyIdEND

// EVT_LOG_PROPERTY_ID, used with EvtGetLogInfo
#define EvtLogCreationTime EvtLogCreationTime
#define EvtLogLastAccessTime EvtLogLastAccessTime
#define EvtLogLastWriteTime EvtLogLastWriteTime
#define EvtLogFileSize EvtLogFileSize
#define EvtLogAttributes EvtLogAttributes
#define EvtLogNumberOfLogRecords EvtLogNumberOfLogRecords
#define EvtLogOldestRecordNumber EvtLogOldestRecordNumber
#define EvtLogFull EvtLogFull

// EVT_EVENT_PROPERTY_ID used with EvtGetEventInfo
#define EvtEventQueryIDs EvtEventQueryIDs
#define EvtEventPath EvtEventPath
#define EvtEventPropertyIdEND EvtEventPropertyIdEND
