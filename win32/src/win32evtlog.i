/* File : win32evtlog.i */

%module win32evtlog // A module, encapsulating the Windows Win32 event log API.


%include "typemaps.i"
%include "pywin32.i"

%{

#include <structmember.h>

// @object PyEventLogRecord|An object containing the data in an EVENTLOGRECORD.
class PyEventLogRecord : public PyObject
{
public:
	PyEventLogRecord(EVENTLOGRECORD *pEvt);
	~PyEventLogRecord(void);

	static void deallocFunc(PyObject *ob);

	static PyObject *getattr(PyObject *self, char *name);
	static int setattr(PyObject *self, char *name, PyObject *v);
//#pragma warning( disable : 4251 )
	static struct memberlist memberlist[];
//#pragma warning( default : 4251 )

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
	PyObject_HEAD_INIT(&PyType_Type)
	0,
	"PyEventLogRecord",
	sizeof(PyEventLogRecord),
	0,
	PyEventLogRecord::deallocFunc,		/* tp_dealloc */
	0,		/* tp_print */
	PyEventLogRecord::getattr,				/* tp_getattr */
	PyEventLogRecord::setattr,				/* tp_setattr */
	0,	/* tp_compare */
	0,						/* tp_repr */
	0,						/* tp_as_number */
	0,	/* tp_as_sequence */
	0,						/* tp_as_mapping */
	0,
	0,						/* tp_call */
	0,		/* tp_str */
};

#define OFF(e) offsetof(PyEventLogRecord, e)

/*static*/ struct memberlist PyEventLogRecord::memberlist[] = {
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

	TimeGenerated = PyWinTimeObject_FromLong(pEvt->TimeGenerated);
	TimeWritten = PyWinTimeObject_FromLong(pEvt->TimeWritten);

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

PyObject *PyEventLogRecord::getattr(PyObject *self, char *name)
{
	return PyMember_Get((char *)self, memberlist, name);
}

int PyEventLogRecord::setattr(PyObject *self, char *name, PyObject *v)
{
	if (v == NULL) {
		PyErr_SetString(PyExc_AttributeError, "can't delete EventLogRecord attributes");
		return -1;
	}
	return PyMember_Set((char *)self, memberlist, name, v);
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
	WORD numStrings = 0;
	BSTR *pStrings = NULL;
	int i;
	DWORD dataSize = 0;
	char *pData;
	PSID sid;
	if (!PyWinObject_AsSID(obSID, &sid, TRUE))
		return NULL;
	if (obStrings==Py_None) {
		pStrings = NULL;
	} else if (PySequence_Check(obStrings)) {
		numStrings = PySequence_Length(obStrings);
		pStrings = new BSTR [numStrings];
		if (pStrings==NULL) {
			PyErr_SetString(PyExc_MemoryError, "Allocating string arrays");
			goto cleanup;
		}
		memset(pStrings, 0, sizeof(BSTR *)*numStrings);
		for (i=0;i<numStrings;i++) {
			PyObject *obString = PySequence_GetItem(obStrings, i);
			if (obString==NULL) {
				goto cleanup;
			}
			BOOL ok = PyWinObject_AsBstr(obString, pStrings+i);
			Py_XDECREF(obString);
			if (!ok)
				goto cleanup;
		}
	} else {
		PyErr_SetString(PyExc_TypeError, "strings must be None or a sequence");
		goto cleanup;
	}
	if (obData==Py_None)
		pData = NULL;
	else if (PyString_Check(obData)){
		pData = PyString_AsString(obData);
		dataSize = PyString_Size(obData);
	} else {
		PyErr_SetString(PyExc_TypeError, "data must be None or a string");
		return NULL;
	}
	BOOL ok;
	Py_BEGIN_ALLOW_THREADS
	ok = ReportEventW(hEventLog, wType, wCategory,	dwEventID, sid, numStrings, dataSize, (const WCHAR **)pStrings, pData);
	Py_END_ALLOW_THREADS

	if (!ok) {
		PyWin_SetAPIError("ReportEvent");
		goto cleanup;
	}
	Py_INCREF(Py_None);
	rc = Py_None;
cleanup:
	if (pStrings) {
		for (i=0;i<numStrings;i++)
			SysFreeString(pStrings[i]);
		delete [] pStrings;
	}
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

// @pyswig int|OpenEventLog|Opens an event log.
%name (OpenEventLog) HANDLE OpenEventLogW (
    %val PyWin_AutoFreeBstr &inNullWideString, // @pyparm <o PyUnicode>|serverName||The server name, or None
    %val PyWin_AutoFreeBstr &inWideString  // @pyparm <o PyUnicode>|sourceName||specifies the name of the source that the returned handle will reference. The source name must be a subkey of a logfile entry under the EventLog key in the registry. 
    );

// @pyswig int|RegisterEventSource|Registers an Event Source
%name (RegisterEventSource) HANDLE
RegisterEventSourceW (
    %val PyWin_AutoFreeBstr &inNullWideString, // @pyparm <o PyUnicode>|serverName||The server name, or None
    %val PyWin_AutoFreeBstr &inWideString  // @pyparm <o PyUnicode>|sourceName||The source name
    );


// @pyswig int|OpenBackupEventLog|Opens a previously saved event log.
%name (OpenBackupEventLog) HANDLE OpenBackupEventLogW (
    %val PyWin_AutoFreeBstr &inNullWideString, // @pyparm <o PyUnicode>|serverName||The server name, or None
    %val PyWin_AutoFreeBstr &inWideString      // @pyparm <o PyUnicode>|fileName||The filename to open
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
     HANDLE     hEventLog,
     WORD       wType,
     WORD       wCategory,
     DWORD      dwEventID,
     PyObject   *pyobject,
     PyObject   *pyobject,
     PyObject   *pyobject
    );


