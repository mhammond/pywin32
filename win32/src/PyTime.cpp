//
// PyTime.cpp -- date/time type for Python
//
// @doc
#include "PyWinTypes.h"
#include "PyWinObjects.h"

#include "datetime.h"  // python's datetime header.

#include "time.h"
#include "tchar.h"
#include "math.h"

// @object PyTime|An alias for the builtin datetime object.

// Each second as stored in a DATE.
const double ONETHOUSANDMILLISECONDS = 0.00001157407407407407407407407407;

PyObject *PyWin_NewTime(PyObject *timeOb);

BOOL PyWinTime_Check(PyObject *ob) { return PyDateTime_Check(ob) || PyObject_HasAttrString(ob, "timetuple"); }

// Timezone helpers...
// Returns a timezone object representing UTC.  Implementation currently
// calls into win32timezone for an object then caches it forever.
static PyObject *GetTZUTC()
{
    static PyObject *got = NULL;
    if (got == NULL) {
        // assumes we have the gil so no races...
        PyObject *mod = PyImport_ImportModule("win32timezone");
        if (!mod)
            return NULL;
        PyObject *klass = PyObject_GetAttrString(mod, "TimeZoneInfo");
        Py_DECREF(mod);
        if (!klass)
            return NULL;
        PyObject *args = PyTuple_New(0);
        if (args) {
            got = PyObject_CallMethod(klass, "utc", "");
            Py_DECREF(args);
        }
        Py_DECREF(klass);
        assert(got);
        assert(got != Py_None);  // this would silently create a tz-naive object.
    }
    Py_XINCREF(got);
    return got;
}

// @pymethod <o PyDateTime>|pywintypes|Time|Creates a new time object.
PyObject *PyWinMethod_NewTime(PyObject *self, PyObject *args)
{
    PyObject *timeOb;
    // @pyparm object|timeRepr||An integer/float/tuple time representation.
    // @comm Note that the parameter can be any object that supports
    // int(object) or another PyDateTime object.
    // <nl>The integer should be as defined by the Python time module.
    // See the description of the <o PyDateTime> object for more information.
    if (!PyArg_ParseTuple(args, "O", &timeOb))
        return NULL;

    return PyWin_NewTime(timeOb);
}

// @pymethod <o PyDateTime>|pywintypes|TimeStamp|Creates a new time object.
PyObject *PyWinMethod_NewTimeStamp(PyObject *self, PyObject *args)
{
    PyObject *obts;
    LARGE_INTEGER ts;
    // @pyparm int|timestamp||An integer timestamp representation.
    if (!PyArg_ParseTuple(args, "O", &obts))
        return NULL;

    if (!PyWinObject_AsLARGE_INTEGER(obts, &ts))
        return NULL;

    return PyWinObject_FromTimeStamp(ts);
}

// @pymethod str|PyDateTime|Format|
// @comm This method is an alias for the datetime strftime method, using
// %c as the default value for the format string.
static PyObject *PyWinDateTimeType_Format(PyObject *self, PyObject *args)
{
    PyObject *new_args = NULL;
    PyObject *method = NULL;
    PyObject *ret = NULL;
    if (PyTuple_Size(args) == 0) {
        if (!(new_args = Py_BuildValue("(s)", "%c")))
            return NULL;
        args = new_args;
    }
    if (!(method = PyObject_GetAttrString(self, "strftime")))
        goto done;

    ret = PyObject_Call(method, args, NULL);
done:
    Py_XDECREF(new_args);
    Py_XDECREF(method);
    return ret;
}

// @object PyDateTime|A Python object, representing an instant in time.
// @comm PyDateTime is a sub-class of the regular datetime.datetime object.
// It is subclassed so it can provide a somewhat backwards compatible
// <om PyDateTime.Format> method, but is otherwise identical. Functions accepting
// a PyDateTime object also accept a datetime.datetime object. A PyDateTime
// object can be created via <om pywintypes.Time>.
// @comm Migration note: pywin32 builds for Python 2 used an (incompatible)
// PyTime object instad of datetime.

struct PyMethodDef PyWinDateTimeType_methods[] = {
    {"Format", PyWinDateTimeType_Format,
     1},  // @pymeth Format|Formats the time value - an alias for strftime with a default param.
    {NULL}};

// we sub-class the datetime.datetime object so we can provide a b/w compat
// 'Format' method.
// Note that this is a temporary strategy - 'Format()' will soon become
// deprecated - but we must give some period for people to adjust before
// breaking this fundamental method.
PyTypeObject PyWinDateTimeType = {
    PYWIN_OBJECT_HEAD "pywintypes.datetime",
    0,  // tp_basicsize filled in at runtime
    0,
    // dealloc filled at runtime
    0,                                        /* tp_dealloc */
    0,                                        /* tp_print */
    0,                                        /* tp_getattr */
    0,                                        /* tp_setattr */
    0,                                        /* tp_compare */
    0,                                        /* tp_repr */
    0,                                        /* tp_as_number */
    0,                                        /* tp_as_sequence */
    0,                                        /* tp_as_mapping */
    0,                                        /* tp_hash */
    0,                                        /* tp_call */
    0,                                        /* tp_str */
    PyObject_GenericGetAttr,                  /* tp_getattro */
    PyObject_GenericSetAttr,                  /* tp_setattro */
    0,                                        /* tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags */
    0,                                        /* tp_doc */
    0,                                        /* tp_traverse */
    0,                                        /* tp_clear */
    0,                                        /* tp_richcompare */
    0,                                        /* tp_weaklistoffset */
    0,                                        /* tp_iter */
    0,                                        /* tp_iternext */
    PyWinDateTimeType_methods,                /* tp_methods */
    0,                                        /* tp_members */
    0,                                        /* tp_getset */
    // we fill tp_base in at runtime; it's not available statically.
    0, /* tp_base */
    0, /* tp_dict */
    0, /* tp_descr_get */
    0, /* tp_descr_set */
    0, /* tp_dictoffset */
    0, /* tp_init */
    0, /* tp_alloc */
    // tp_new filled at runtime
    0, /* tp_new */
};

BOOL PyWinObject_AsDATE(PyObject *ob, DATE *pDate)
{
    SYSTEMTIME st;
    if (!PyWinObject_AsSYSTEMTIME(ob, &st))
        return FALSE;
    // Extra work to get milliseconds, via
    // https://www.codeproject.com/Articles/17576/SystemTime-to-VariantTime-with-Milliseconds
    WORD wMilliseconds = st.wMilliseconds;
    // not clear why we need to zero this since we always seem to get ms ignored
    // but...
    st.wMilliseconds = 0;

    double dWithoutms;
    if (!SystemTimeToVariantTime(&st, &dWithoutms)) {
        PyWin_SetAPIError("SystemTimeToVariantTime");
        return FALSE;
    }
    // manually convert the millisecond information into variant
    // fraction and add it to system converted value
    double OneMilliSecond = ONETHOUSANDMILLISECONDS / 1000;
    *pDate = dWithoutms + (OneMilliSecond * wMilliseconds);
    return TRUE;
}

BOOL PyWinObject_AsFILETIME(PyObject *ob, FILETIME *ft)
{
    SYSTEMTIME st;
    if (!PyWinObject_AsSYSTEMTIME(ob, &st))
        return FALSE;
    // and to the FILETIME
    if (!SystemTimeToFileTime(&st, ft)) {
        PyWin_SetAPIError("SystemTimeToFileTime");
        return FALSE;
    }
    return TRUE;
}

BOOL PyWinObject_AsSYSTEMTIME(PyObject *ob, SYSTEMTIME *st)
{
    if (!PyDateTime_Check(ob)) {
        PyErr_Format(PyExc_TypeError, "must be a pywintypes time object (got %s)", Py_TYPE(ob)->tp_name);
        return NULL;
    }
    // convert the date to a UTC date.
    PyObject *utc = PyObject_CallMethod(ob, "astimezone", "O", GetTZUTC());
    // likely error is "ValueError: astimezone() cannot be applied to a naive datetime"
    if (!utc)
        return NULL;
    st->wYear = PyDateTime_GET_YEAR(utc);
    st->wMonth = PyDateTime_GET_MONTH(utc);
    st->wDay = PyDateTime_GET_DAY(utc);
    st->wHour = PyDateTime_DATE_GET_HOUR(utc);
    st->wMinute = PyDateTime_DATE_GET_MINUTE(utc);
    st->wSecond = PyDateTime_DATE_GET_SECOND(utc);
    st->wMilliseconds = PyDateTime_DATE_GET_MICROSECOND(utc) / 1000;
    Py_DECREF(utc);
    return TRUE;
}

// a slightly modified version from Python's time module.
static BOOL gettmarg(PyObject *ob, struct tm *p, int *pmsec)
{
    int y;
    memset((void *)p, '\0', sizeof(struct tm));

    if (!PyArg_ParseTuple(ob, "iiiiiiiii|i", &y, &p->tm_mon, &p->tm_mday, &p->tm_hour, &p->tm_min, &p->tm_sec,
                          &p->tm_wday, &p->tm_yday, &p->tm_isdst, pmsec))
        return FALSE;
    if (y < 1900) {
        // we always accept 2digit years
        if (69 <= y && y <= 99)
            y += 1900;
        else if (0 <= y && y <= 68)
            y += 2000;
        else {
            PyErr_SetString(PyExc_ValueError, "year out of range");
            return FALSE;
        }
    }
    p->tm_year = y - 1900;
    p->tm_mon--;
    p->tm_wday = (p->tm_wday + 1) % 7;
    p->tm_yday--;
    return TRUE;
}

static WORD SequenceIndexAsWORD(PyObject *seq, int index)
{
    PyObject *t = PySequence_GetItem(seq, index);
    int ret = t ? PyLong_AsLong(t) : -1;
    Py_XDECREF(t);
    return (WORD)ret;
}

PyObject *PyWin_NewTime(PyObject *timeOb)
{
    // If it already a datetime object, just return it as-is.
    if (PyDateTime_Check(timeOb)) {
        Py_INCREF(timeOb);
        return timeOb;
    }

    PyObject *result = NULL;
    PyObject *cleanupOb = NULL;  // must be xdefref'd.

    // Support other objects with a "timetuple" method.
    PyObject *method = PyObject_GetAttrString(timeOb, "timetuple");
    if (method == NULL)
        PyErr_Clear();
    else {
        timeOb = PyObject_CallObject(method, NULL);
        Py_DECREF(method);
        if (!timeOb)
            return NULL;
        cleanupOb = timeOb;  // new reference that must be nuked.
                             // now we should fall into the sequence check!
    }
    if (PyNumber_Check(timeOb)) {
        PyObject *longob = PyNumber_Long(timeOb);
        if (longob) {
            long t = PyLong_AsLong(longob);
            if (t == -1) {
                if (!PyErr_Occurred())
                    PyErr_BadArgument();
            }
            else
                result = PyWinTimeObject_Fromtime_t(t);
            Py_DECREF(longob);
        }
    }
    else if (PySequence_Check(timeOb)) {
        assert(!PyErr_Occurred());  // should be no stale errors!
        // convert a timetuple, with optional millisecond extension,
        // into a datetime object. ie:
        // >>> datetime.datetime.fromtimestamp(time.mktime(timetuple))
        // but we 'inline' the time.mktime step...
        struct tm buf;
        time_t tt;
        int millisec = 0;
        // must use a tuple as we use ParseTuple with an optional arg.
        PyObject *tuple_args = PySequence_Tuple(timeOb);
        if (!tuple_args)
            return NULL;
        BOOL ok = gettmarg(tuple_args, &buf, &millisec);
        Py_DECREF(tuple_args);
        if (!ok)
            return NULL;
        tt = mktime(&buf);
        if (tt == (time_t)(-1)) {
            PyErr_SetString(PyExc_OverflowError, "mktime argument out of range");
            return NULL;
        }
        double dval = (double)tt + (millisec / 1000.0);
        PyObject *args = Py_BuildValue("(d)", dval);
        if (!args)
            return NULL;
        result = PyDateTimeAPI->DateTime_FromTimestamp((PyObject *)(&PyWinDateTimeType), args, NULL);
        Py_DECREF(args);
    }
    else
        // result stays NULL.
        PyErr_Format(PyExc_TypeError, "Objects of type '%s' can not be used as a time object",
                     Py_TYPE(timeOb)->tp_name);
    Py_XDECREF(cleanupOb);
    return result;
}

PyObject *PyWinObject_FromSYSTEMTIME(const SYSTEMTIME &t)
{
    // SYSTEMTIME structures explicitly use UTC.
    PyObject *obtz = GetTZUTC();
    if (!obtz)
        return NULL;
    // If the value is larger than the datetime module can handle, we return
    // the max datetime value.
    PyObject *ret;
    if (t.wYear > 9999) {  // sadly this constant isn't exposed.
        ret = PyObject_GetAttrString((PyObject *)PyDateTimeAPI->DateTimeType, "max");
    }
    else {
        ret = PyDateTimeAPI->DateTime_FromDateAndTime(t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond,
                                                      t.wMilliseconds * 1000, obtz, &PyWinDateTimeType);
    }
    Py_DECREF(obtz);
    return ret;
}

PyObject *PyWinObject_FromFILETIME(const FILETIME &t)
{
    // XXX - We should create a datetime object using the localtz here,
    // but for now we only have a utc tz available, so convert to a
    // systemtime and go from there.
    SYSTEMTIME st;
    if (!FileTimeToSystemTime(&t, &st))
        return PyWin_SetAPIError("FileTimeToSystemTime");
    return PyWinObject_FromSYSTEMTIME(st);
}

static double round(double Value, int Digits)
{
    assert(Digits >= -4 && Digits <= 4);
    int Idx = Digits + 4;
    double v[] = {1e-4, 1e-3, 1e-2, 1e-1, 1, 10, 1e2, 1e3, 1e4};
    return floor(Value * v[Idx] + 0.5) / (v[Idx]);
}

PyObject *PyWinObject_FromDATE(DATE t)
{
    // via https://www.codeproject.com/Articles/17576/SystemTime-to-VariantTime-with-Milliseconds
    // (in particular, see the comments)
    double fraction = t - (int)t;  // extracts the fraction part
    double hours = (fraction - (int)fraction) * 24.0;
    double minutes = (hours - (int)hours) * 60.0;
    double seconds = round((minutes - (int)minutes) * 60.0, 4);
    double milliseconds = round((seconds - (int)seconds) * 1000.0, 0);
    // Strip off the msec part of time
    double TimeWithoutMsecs = t - (ONETHOUSANDMILLISECONDS / 1000.0 * milliseconds);

    // We might have rounded ms to 1000 which blows up datetime. Round up
    // to the next second.
    if (milliseconds >= 1000) {
        TimeWithoutMsecs += ONETHOUSANDMILLISECONDS;
        milliseconds = 0;
    }

    // Let the OS translate the variant date/time
    SYSTEMTIME st;
    if (!VariantTimeToSystemTime(TimeWithoutMsecs, &st)) {
        return PyWin_SetAPIError("VariantTimeToSystemTime");
    }
    if (milliseconds > 0.0) {
        // add the msec part to the systemtime object
        st.wMilliseconds = (WORD)milliseconds;
    }
    return PyWinObject_FromSYSTEMTIME(st);
}

PyObject *PyWinTimeObject_Fromtime_t(time_t t)
{
    PyObject *args = Py_BuildValue("(i)", (int)t);
    if (!args)
        return NULL;
    PyObject *ret = PyDateTimeAPI->DateTime_FromTimestamp((PyObject *)(&PyWinDateTimeType), args, NULL);
    if (ret == NULL) {
        // datetime throws an OSError on failure, but for compatibility with
        // Python 2, we turn that into a ValueError.
        PyErr_Clear();
        PyErr_SetString(PyExc_ValueError, "invalid timestamp");
    }
    Py_DECREF(args);
    return ret;
}

// Converts a TimeStamp, which is in 100 nanosecond units like a FILETIME
// See comments in pywintypes.h re LARGE_INTEGER vs TimeStamp
PyObject *PyWinObject_FromTimeStamp(const LARGE_INTEGER &ts)
{
    FILETIME ft;
    ft.dwHighDateTime = ts.HighPart;
    ft.dwLowDateTime = ts.LowPart;
    return PyWinObject_FromFILETIME(ft);
}

// A couple of public functions used by the module init
BOOL _PyWinDateTime_Init()
{
    PyDateTime_IMPORT;
    if (!PyDateTimeAPI)
        return NULL;
    PyWinDateTimeType.tp_base = PyDateTimeAPI->DateTimeType;
    PyWinDateTimeType.tp_basicsize = PyDateTimeAPI->DateTimeType->tp_basicsize;
    PyWinDateTimeType.tp_new = PyDateTimeAPI->DateTimeType->tp_new;
    PyWinDateTimeType.tp_dealloc = PyDateTimeAPI->DateTimeType->tp_dealloc;
    if (PyType_Ready(&PyWinDateTimeType) == -1)
        return FALSE;
    return TRUE;
}

BOOL _PyWinDateTime_PrepareModuleDict(PyObject *dict)
{
    if (PyDict_SetItemString(dict, "TimeType", (PyObject *)&PyWinDateTimeType) == -1)
        return FALSE;
    return TRUE;
}
