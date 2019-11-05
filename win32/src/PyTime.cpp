//
// PyTime.cpp -- date/time type for Python
//
// @doc
#include "PyWinTypes.h"
#include "PyWinObjects.h"

#ifdef PYWIN_HAVE_DATETIME_CAPI
#include "datetime.h"  // python's datetime header.
#endif

#include "time.h"
#include "tchar.h"
#include "math.h"

// Each second as stored in a DATE.
const double ONETHOUSANDMILLISECONDS = 0.00001157407407407407407407407407;

PyObject *PyWin_NewTime(PyObject *timeOb);

BOOL PyWinTime_Check(PyObject *ob)
{
    return 0 ||
#ifndef NO_PYWINTYPES_TIME
           PyWinTime_CHECK(ob) ||
#endif
#ifdef PYWIN_HAVE_DATETIME_CAPI
           PyDateTime_Check(ob) ||
#endif
           PyObject_HasAttrString(ob, "timetuple");
}

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

// @pymethod <o PyTime>|pywintypes|Time|Creates a new time object.
PyObject *PyWinMethod_NewTime(PyObject *self, PyObject *args)
{
    PyObject *timeOb;
    // @pyparm object|timeRepr||An integer/float/tuple time representation.
    // @comm Note that the parameter can be any object that supports
    // int(object) - for example , another PyTime object.
    // <nl>The integer should be as defined by the Python time module.
    // See the description of the <o PyTime> object for more information.
    if (!PyArg_ParseTuple(args, "O", &timeOb))
        return NULL;

    return PyWin_NewTime(timeOb);
}

// @pymethod <o PyTime>|pywintypes|TimeStamp|Creates a new time object.
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

#ifndef NO_PYWINTYPES_TIME

BOOL PyWinObject_AsDATE(PyObject *ob, DATE *pDate)
{
    PyObject *newref = NULL;
    BOOL rc;
    if (!PyWinTime_CHECK(ob)) {
        if (!(ob = PyWin_NewTime(ob)))
            return FALSE;
        newref = ob;
    }
    rc = ((PyTime *)ob)->GetTime(pDate);
    Py_XDECREF(newref);
    return rc;
}

BOOL PyWinObject_AsFILETIME(PyObject *ob, FILETIME *pDate)
{
    PyObject *newref = NULL;
    BOOL rc;
    if (!PyWinTime_CHECK(ob)) {
        if (!(ob = PyWin_NewTime(ob)))
            return FALSE;
        newref = ob;
    }
    rc = ((PyTime *)ob)->GetTime(pDate);
    Py_XDECREF(newref);
    return rc;
}
BOOL PyWinObject_AsSYSTEMTIME(PyObject *ob, SYSTEMTIME *pDate)
{
    PyObject *newref = NULL;
    BOOL rc;
    if (!PyWinTime_CHECK(ob)) {
        if (!(ob = PyWin_NewTime(ob)))
            return FALSE;
        newref = ob;
    }
    rc = ((PyTime *)ob)->GetTime(pDate);
    Py_XDECREF(newref);
    return rc;
}

/* the following code is taken from Python 2.3 Modules/datetimemodule.c
 * it is used for calculating day of the year for PyTime::Format
 */
#define SECS_PER_DAY (24.0 * 60.0 * 60.0)

static int _days_before_month[] = {0, /* unused; this vector uses 1-based indexing */
                                   0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};

/* year -> 1 if leap year, else 0. */
static int is_leap(int year)
{
    /* Cast year to unsigned.  The result is the same either way, but
     * C can generate faster code for unsigned mod than for signed
     * mod (especially for % 4 -- a good compiler should just grab
     * the last 2 bits when the LHS is unsigned).
     */
    const unsigned int ayear = (unsigned int)year;
    return ayear % 4 == 0 && (ayear % 100 != 0 || ayear % 400 == 0);
}

/* year, month -> number of days in year preceeding first day of month */
static int days_before_month(int year, int month)
{
    int days;

    assert(month >= 1);
    assert(month <= 12);
    days = _days_before_month[month];
    if (month > 2 && is_leap(year))
        ++days;
    return days;
}

// @pymethod <o PyUnicode>|PyTime|Format|Formats the time value.

PyObject *PyTime::Format(PyObject *self, PyObject *args)
{
    PyObject *obFormat = NULL;
    // @pyparm string|format|%c|The format.  See the comments section for a description of the supported flags.
    if (!PyArg_ParseTuple(args, "|O:Format", &obFormat))
        return NULL;
    TCHAR *fmt = TEXT("%c");
    BOOL bFreeString = FALSE;
    if (obFormat) {
        bFreeString = TRUE;
        if (!PyWinObject_AsTCHAR(obFormat, &fmt))
            return NULL;
    }
    TCHAR szBuffer[256];
    PyTime *pTime = (PyTime *)self;

    // _tcsftime tries to be "helpful" by dieing with a too early date in
    // some CRT implementations (eg, vs2008 64bit - and probably others)
    SYSTEMTIME st;
    if (!VariantTimeToSystemTime(pTime->m_time, &st) || st.wYear < 1900) {
        PyErr_SetString(PyExc_ValueError, "can't format dates this early");
        return NULL;
    }

    struct tm tm = {0};
    tm.tm_sec = st.wSecond;
    tm.tm_min = st.wMinute;
    tm.tm_hour = st.wHour;
    tm.tm_mday = st.wDay;
    tm.tm_mon = st.wMonth - 1;
    tm.tm_year = st.wYear - 1900;

    /* Ask windows for the current is_dst flag */
    TIME_ZONE_INFORMATION tzinfo;
    switch (GetTimeZoneInformation(&tzinfo)) {
        case TIME_ZONE_ID_STANDARD:
            tm.tm_isdst = 0;
            break;
        case TIME_ZONE_ID_DAYLIGHT:
            tm.tm_isdst = 1;
            break;
        default:
            tm.tm_isdst = -1;
            break;
    }
    /* tm_wday: day of week (0-6) sunday=0 : weekday(y, m, d) */
    /* tm_yday: day of year (0-365) january 1=0: days_before_month(y, m) + d */
    tm.tm_wday = st.wDayOfWeek;
    tm.tm_yday = days_before_month(st.wYear, st.wMonth) + st.wDay - 1;

    if (!_tcsftime(szBuffer, 256 /*_countof()*/, fmt, &tm))
        szBuffer[0] = '\0';  // Better error?
    PyObject *rc = PyWinCoreString_FromString(szBuffer);
    if (bFreeString)
        PyWinObject_FreeTCHAR(fmt);
    return rc;
    // @comm The following format characters are supported.
    // @flagh Character|Description
    // @flag %a|Abbreviated weekday name
    // @flag %A|Full weekday name
    // @flag %b|Abbreviated month name
    // @flag %B|Full month name
    // @flag %c|Date and time representation appropriate for locale
    // @flag %d|Day of month as decimal number (01 - 31)
    // @flag %H|Hour in 24-hour format (00 - 23)
    // @flag %I|Hour in 12-hour format (01 - 12)
    // @flag %j|Day of year as decimal number (001 - 366)
    // @flag %m|Month as decimal number (01 - 12)
    // @flag %M|Minute as decimal number (00 - 59)
    // @flag %p|Current locale's A.M./P.M. indicator for 12-hour clock
    // @flag %S|Second as decimal number (00 - 59)
    // @flag %U|Week of year as decimal number, with Sunday as first day of week (00 - 51)
    // @flag %w|Weekday as decimal number (0 - 6; Sunday is 0)
    // @flag %W|Week of year as decimal number, with Monday as first day of week (00 - 51)
    // @flag %x|Date representation for current locale
    // @flag %X|Time representation for current locale
    // @flag %y|Year without century, as decimal number (00 - 99)
    // @flag %Y|Year with century, as decimal number
    // @flag %z, %Z|Time-zone name or abbreviation; no characters if time zone is unknown
    // @flag %%|Percent sign
    // @comm As in the printf function, the # flag may prefix any formatting code. In that case, the meaning of the
    // format code is changed as follows.
    // @flagh Format Code|Meaning
    // @flag %#a, %#A, %#b, %#B, %#p, %#X, %#z, %#Z, %#%|# flag is ignored.
    // @flag %#c|Long date and time representation, appropriate for current locale. For example: "Tuesday, March 14,
    // 1995, 12:41:29".
    // @flag %#x|Long date representation, appropriate to current locale. For example: "Tuesday, March 14, 1995".
    // @flag %#d, %#H, %#I, %#j, %#m, %#M, %#S, %#U, %#w, %#W, %#y, %#Y|Remove leading zeros (if any).
}

// @object PyTime|A Python object, representing an instant in time.
// @comm A PyTime object is used primarily when exchanging date/time information
// with COM objects or other win32 functions.
// <nl>Using int(timeObject) will return an integer compatible with
// the standard Python time module.
// @ex First import the time module|import time
// @ex To return a simple string|time.ctime(int(timeObject))
// @ex To return a string formatted as the long date in control panel|time.strftime("%#c",
// time.localtime(int(timeObject)))
// @xref <om pywintypes.Time>

struct PyMethodDef PyTime::methods[] = {{"Format", PyTime::Format, 1},  // @pymeth Format|Formats the time value
                                        {NULL}};

static PyNumberMethods PyTime_NumberMethods = {
    PyTime::binaryFailureFunc, /* nb_add */
    PyTime::binaryFailureFunc, /* nb_subtract */
    PyTime::binaryFailureFunc, /* nb_multiply */
#if (PY_VERSION_HEX < 0x03000000)
    PyTime::binaryFailureFunc, /* nb_divide - removed in Py3k */
#endif
    PyTime::binaryFailureFunc,  /* nb_remainder */
    PyTime::binaryFailureFunc,  /* nb_divmod */
    PyTime::ternaryFailureFunc, /* nb_power */
    PyTime::unaryFailureFunc,   /* nb_negative */
    PyTime::unaryFailureFunc,   /* nb_positive */
    PyTime::unaryFailureFunc,   /* nb_absolute */
    PyTime::nonzeroFunc,        /* nb_nonzero */
    PyTime::unaryFailureFunc,   /* nb_invert */
    PyTime::binaryFailureFunc,  /* nb_lshift */
    PyTime::binaryFailureFunc,  /* nb_rshift */
    PyTime::binaryFailureFunc,  /* nb_and */
    PyTime::binaryFailureFunc,  /* nb_xor */
    PyTime::binaryFailureFunc,  /* nb_or */
#if (PY_VERSION_HEX < 0x03000000)
    0, /* nb_coerce (allowed to be zero) */
#endif
    PyTime::intFunc,          /* nb_int */
    PyTime::unaryFailureFunc, /* nb_long */
    PyTime::floatFunc,        /* nb_float */
#if (PY_VERSION_HEX < 0x03000000)
    PyTime::unaryFailureFunc, /* nb_oct */
    PyTime::unaryFailureFunc, /* nb_hex */
#endif
};
// @pymeth __int__|Used when an integer representation of the time object is required.
// @pymeth __float__|Used when a floating point representation of the time object is required.

PYWINTYPES_EXPORT PyTypeObject PyTimeType = {
    PYWIN_OBJECT_HEAD "time", sizeof(PyTime), 0, PyTime::deallocFunc, /* tp_dealloc */
    NULL,                                                             /* tp_print */
    0,                                                                /* tp_getattr */
    0,                                                                /* tp_setattr */
    // @pymeth __cmp__|Used when time objects are compared.
    PyTime::compareFunc, /* tp_compare */
    // @pymeth __repr__|Used for repr(ob)
    PyTime::reprFunc,      /* tp_repr */
    &PyTime_NumberMethods, /* tp_as_number */
    0,                     /* tp_as_sequence */
    0,                     /* tp_as_mapping */
    // @pymeth __hash__|Used when the hash value of an time object is required
    PyTime::hashFunc, /* tp_hash */
    0,                /* tp_call */
    // @pymeth __str__|Used for str(ob)
    PyTime::strFunc,                          /* tp_str */
    PyTime::getattro,                         /* tp_getattro */
    0,                                        /* tp_setattro */
    0,                                        /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags */
    0,                                        /* tp_doc */
    0,                                        /* tp_traverse */
    0,                                        /* tp_clear */
    PyTime::richcompareFunc,                  /* tp_richcompare */
    0,                                        /* tp_weaklistoffset */
    0,                                        /* tp_iter */
    0,                                        /* tp_iternext */
    PyTime::methods,                          /* tp_methods */
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

PyTime::PyTime(DATE t)
{
    ob_type = &PyTimeType;
    _Py_NewReference(this);
    m_time = t;
}

PyTime::PyTime(time_t t)
{
    ob_type = &PyTimeType;
    _Py_NewReference(this);
    m_time = 0;
    struct tm *ptm = localtime(&t);
    if (ptm != NULL) {  // otherwise an invalid integer

        SYSTEMTIME st = {ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_wday, ptm->tm_mday,
                         ptm->tm_hour,        ptm->tm_min,     ptm->tm_sec,  0};
        (void)SystemTimeToVariantTime(&st, &m_time);
    }
}

PyTime::PyTime(const SYSTEMTIME &t)
{
    ob_type = &PyTimeType;
    _Py_NewReference(this);
    m_time = 0;
    // Not declared as const systemtime -
    // better not take the chance!
    SYSTEMTIME nt = t;
    (void)SystemTimeToVariantTime(&nt, &m_time);
}

PyTime::PyTime(const FILETIME &t)
{
    ob_type = &PyTimeType;
    _Py_NewReference(this);
    SYSTEMTIME st;
    m_time = 0;
    FileTimeToSystemTime(&t, &st);
    (void)SystemTimeToVariantTime(&st, &m_time);
}

BOOL PyTime::GetTime(DATE *pDate)
{
    *pDate = m_time;
    return TRUE;
}

BOOL PyTime::GetTime(FILETIME *pDate)
{
    SYSTEMTIME time;
    if (!VariantTimeToSystemTime(m_time, &time)) {
        PyWin_SetAPIError("VariantTimeToSystemTime");
        return FALSE;
    }
    if (!SystemTimeToFileTime(&time, pDate)) {
        PyWin_SetAPIError("SystemTimeToFileTime");
        return FALSE;
    }
    return TRUE;
}

BOOL PyTime::GetTime(SYSTEMTIME *pDate)
{
    if (!VariantTimeToSystemTime(m_time, pDate)) {
        PyWin_SetAPIError("VariantTimeToSystemTime");
        return FALSE;
    }
    return TRUE;
}

PyObject *PyTime::str()
{
    // Just re-use 'Format()'.
    PyObject *args = PyTuple_New(0);
    PyObject *ret = Format(this, args);
    Py_XDECREF(args);
    return ret;
}

int PyTime::compare(PyObject *ob)
{
    DATE time2 = ((PyTime *)ob)->m_time;

    if (m_time < time2)
        return -1;
    if (m_time > time2)
        return 1;
    return 0;
}

// Py3k requires that objects implement richcompare to be used as dict keys
PyObject *PyTime::richcompare(PyObject *other, int op)
{
    if (!PyWinTime_CHECK(other)) {
        Py_INCREF(Py_NotImplemented);
        return Py_NotImplemented;
    }
    DATE other_time = ((PyTime *)other)->m_time;
    PyObject *ret;

    switch (op) {
        case Py_EQ:
            ret = (m_time == other_time) ? Py_True : Py_False;
            break;
        case Py_NE:
            ret = (m_time != other_time) ? Py_True : Py_False;
            break;
        case Py_LT:
            ret = (m_time < other_time) ? Py_True : Py_False;
            break;
        case Py_GT:
            ret = (m_time > other_time) ? Py_True : Py_False;
            break;
        case Py_LE:
            ret = (m_time <= other_time) ? Py_True : Py_False;
            break;
        case Py_GE:
            ret = (m_time >= other_time) ? Py_True : Py_False;
            break;
        default:
            ret = NULL;
            PyErr_SetString(PyExc_SystemError, "Invalid richcompare operation");
    }
    Py_XINCREF(ret);
    return ret;
}

Py_hash_t PyTime::hash(void)
{
    /* arbitrarily use seconds as the hash value */
    return (Py_hash_t)(m_time * SECS_PER_DAY);
}

long PyTime::asLong(void)
{
    SYSTEMTIME st;

    if (!VariantTimeToSystemTime(m_time, &st)) {
        PyErr_SetString(PyExc_ValueError, "illegal internal value");
        return -1;
    }

    struct tm tm = {0};
    tm.tm_sec = st.wSecond;
    tm.tm_min = st.wMinute;
    tm.tm_hour = st.wHour;
    tm.tm_mday = st.wDay;
    tm.tm_mon = st.wMonth - 1;
    tm.tm_year = st.wYear - 1900;
    tm.tm_isdst = -1; /* have the library figure it out */
    long result = (long)mktime(&tm);
    if (result == -1) {
        PyErr_SetString(PyExc_ValueError, "illegal time value");
        return -1;
    }
    return result;
}

PyObject *PyTime::repr()
{
    TCHAR dateBuf[128];
    TCHAR timeBuf[128];
    SYSTEMTIME st;

    if (!VariantTimeToSystemTime(m_time, &st))
        _tcscpy(timeBuf, _T("!illegal internal value!"));
    else {
        if (0 ==
            GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &st, NULL, dateBuf, sizeof(dateBuf) / sizeof(TCHAR)))
            wsprintf(dateBuf, _T("!GetDateFormat failed (%ld)!"), GetLastError());
        if (0 == GetTimeFormat(LOCALE_USER_DEFAULT, 0, &st, NULL, timeBuf, sizeof(timeBuf)))
            wsprintf(timeBuf, _T("!GetTimeFormat failed (%ld)!"), GetLastError());
    }

    TCHAR resBuf[160];
    wsprintf(resBuf, _T("<PyTime:%s %s>"), dateBuf, timeBuf);
    return PyWinCoreString_FromString(resBuf);
}

PyObject *PyTime::getattro(PyObject *self, PyObject *obname)
{
    PyObject *res;

    res = PyObject_GenericGetAttr(self, obname);
    if (res != NULL)
        return res;
    PyErr_Clear();
    SYSTEMTIME st;
    PyTime *This = (PyTime *)self;
    if (!VariantTimeToSystemTime(This->m_time, &st)) {
        PyWin_SetAPIError("VariantTimeToSystemTime");
        return NULL;
    }

    double intpart;
    st.wMilliseconds = modf(This->m_time * SECS_PER_DAY, &intpart) * 1000000;

    char *name = PYWIN_ATTR_CONVERT(obname);
    if (name == NULL)
        return NULL;

    // @prop int|year|
    if (!strcmp(name, "year")) {
        return PyInt_FromLong(st.wYear);
    }
    // @prop int|month|
    else if (!strcmp(name, "month")) {
        return PyInt_FromLong(st.wMonth);
    }
    // @prop int|weekday|
    else if (!strcmp(name, "weekday")) {
        return PyInt_FromLong(st.wDayOfWeek);
    }
    // @prop int|day|
    else if (!strcmp(name, "day")) {
        return PyInt_FromLong(st.wDay);
    }
    // @prop int|hour|
    else if (!strcmp(name, "hour")) {
        return PyInt_FromLong(st.wHour);
    }
    // @prop int|minute|
    else if (!strcmp(name, "minute")) {
        return PyInt_FromLong(st.wMinute);
    }
    // @prop int|second|
    else if (!strcmp(name, "second")) {
        return PyInt_FromLong(st.wSecond);
    }
    // @prop int|msec|
    else if (!strcmp(name, "msec")) {
        return PyInt_FromLong(st.wMilliseconds);
    }
    PyErr_SetString(PyExc_AttributeError, name);
    return NULL;
}

/*static*/ PyObject *PyTime::unaryFailureFunc(PyObject *ob)
{
    PyErr_SetString(PyExc_TypeError, "bad operand type");
    return NULL;
}
/*static*/ PyObject *PyTime::binaryFailureFunc(PyObject *ob1, PyObject *ob2)
{
    PyErr_SetString(PyExc_TypeError, "bad operand type");
    return NULL;
}
/*static*/ PyObject *PyTime::ternaryFailureFunc(PyObject *ob1, PyObject *ob2, PyObject *ob3)
{
    PyErr_SetString(PyExc_TypeError, "bad operand type");
    return NULL;
}

/*static*/ void PyTime::deallocFunc(PyObject *ob) { delete (PyTime *)ob; }

// @pymethod string|PyTime|__str__|Used when a (8-bit) string representation of the time object is required.
PyObject *PyTime::strFunc(PyObject *ob) { return ((PyTime *)ob)->str(); }

// @pymethod int|PyTime|__cmp__|Used when time objects are compared.
int PyTime::compareFunc(PyObject *ob1, PyObject *ob2) { return ((PyTime *)ob1)->compare(ob2); }

PyObject *PyTime::richcompareFunc(PyObject *self, PyObject *other, int op)
{
    return ((PyTime *)self)->richcompare(other, op);
}

// @pymethod int|PyTime|__hash__|Used when the hash value of an time object is required
Py_hash_t PyTime::hashFunc(PyObject *ob) { return ((PyTime *)ob)->hash(); }

// @pymethod |PyTime|__nonzero__|Used for detecting true/false.
/*static*/ int PyTime::nonzeroFunc(PyObject *ob)
{
    /* always non-zero */
    return 1;
}

// @pymethod |PyTime|__int__|Used when an integer representation of the time object is required.
PyObject *PyTime::intFunc(PyObject *ob)
{
    long result = ((PyTime *)ob)->asLong();
    if (result == -1)
        return NULL;
    return PyInt_FromLong(result);
    // @rdesc The integer result can be used with the time module.
    // Please see the main description for the <o PyTime> object.
}

// @pymethod |PyTime|__float__|Used when a floating point representation of the time object is required.
PyObject *PyTime::floatFunc(PyObject *ob)
{
    double result;
    ((PyTime *)ob)->GetTime(&result);
    return PyFloat_FromDouble(result);
}

// @pymethod |PyTime|__repr__|
PyObject *PyTime::reprFunc(PyObject *ob) { return ((PyTime *)ob)->repr(); }

#endif  // NO_PYWINTYPES_TIME

///////////////////////////////////////////////////////////////////////////
//
// The pywin32 time API using datetime objects
//
///////////////////////////////////////////////////////////////////////////
#ifdef PYWIN_HAVE_DATETIME_CAPI
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
// @comm pywin32 builds for Python 3.0 use datetime objects instead of the
// old PyTime object.
// @comm PyDateTime is a sub-class of the regular datetime.datetime object.
// It is subclassed so it can provide a somewhat backwards compatible
// <om PyDateTime.Format> method, but is otherwise identical.

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
    // we fill tp_base in at runtime; its not available statically.
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
        PyErr_Format(PyExc_TypeError, "must be a pywintypes time object (got %s)", ob->ob_type->tp_name);
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

#endif  // PYWIN_HAVE_DATETIME_CAPI

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
    int ret = t ? PyInt_AsLong(t) : -1;
    Py_XDECREF(t);
    return (WORD)ret;
}

PyObject *PyWin_NewTime(PyObject *timeOb)
{
    // If it already a datetime object, just return it as-is.
#ifndef NO_PYWINTYPES_TIME
    if (PyWinTime_CHECK(timeOb)) {
#endif
#ifdef PYWIN_HAVE_DATETIME_CAPI
        if (PyDateTime_Check(timeOb)) {
#endif
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
            timeOb = PyEval_CallObject(method, NULL);
            Py_DECREF(method);
            if (!timeOb)
                return NULL;
            cleanupOb = timeOb;  // new reference that must be nuked.
                                 // now we should fall into the sequence check!
        }
        if (PyNumber_Check(timeOb)) {
            // XXX - should possibly check for long_long, as sizeof(time_t) > sizeof(long)
            // on x64
            long t = PyInt_AsLong(timeOb);
            if (t == -1) {
                if (!PyErr_Occurred())
                    PyErr_BadArgument();
            }
            else
                result = PyWinTimeObject_Fromtime_t((time_t)t);
        }
        else if (PySequence_Check(timeOb)) {
            assert(!PyErr_Occurred());  // should be no stale errors!
#ifdef PYWIN_HAVE_DATETIME_CAPI
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
#else
    // Let's try and convert to a SYSTEMTIME and convert from
    // there (which is what pywintypes always did pre datetime)
    if (PySequence_Length(timeOb) < 6) {
        Py_XDECREF(cleanupOb);
        return PyErr_Format(PyExc_ValueError, "time tuple must have at least 6 elements");
    }
    SYSTEMTIME st = {SequenceIndexAsWORD(timeOb, 0),
                     SequenceIndexAsWORD(timeOb, 1),
                     0,
                     SequenceIndexAsWORD(timeOb, 2),
                     SequenceIndexAsWORD(timeOb, 3),
                     SequenceIndexAsWORD(timeOb, 4),
                     SequenceIndexAsWORD(timeOb, 5),
                     0};
    // A Python time tuple has 9 entries.  We allow a 10th to specify ms
    if (PySequence_Length(timeOb) > 9)
        st.wMilliseconds = SequenceIndexAsWORD(timeOb, 9);
    if (!PyErr_Occurred())
        result = PyWinObject_FromSYSTEMTIME(st);
#endif  // PYWIN_HAVE_DATETIME_CAPI
        }
        else
            // result stays NULL.
            PyErr_Format(PyExc_TypeError, "Objects of type '%s' can not be used as a time object",
                         timeOb->ob_type->tp_name);
        Py_XDECREF(cleanupOb);
        return result;
    }

    PyObject *PyWinObject_FromSYSTEMTIME(const SYSTEMTIME &t)
    {
#ifdef PYWIN_HAVE_DATETIME_CAPI
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
#endif  // PYWIN_HAVE_DATETIME_CAPI

#ifndef NO_PYWINTYPES_TIME
        return new PyTime(t);
#endif
    }

    PyObject *PyWinObject_FromFILETIME(const FILETIME &t)
    {
#ifdef PYWIN_HAVE_DATETIME_CAPI
        // XXX - We should create a datetime object using the localtz here,
        // but for now we only have a utc tz available, so convert to a
        // systemtime and go from there.
        SYSTEMTIME st;
        if (!FileTimeToSystemTime(&t, &st))
            return PyWin_SetAPIError("FileTimeToSystemTime");
        return PyWinObject_FromSYSTEMTIME(st);
#endif  // PYWIN_HAVE_DATETIME_CAPI

#ifndef NO_PYWINTYPES_TIME
        return new PyTime(t);
#endif
    }

#ifdef PYWIN_HAVE_DATETIME_CAPI
    static double round(double Value, int Digits)
    {
        assert(Digits >= -4 && Digits <= 4);
        int Idx = Digits + 4;
        double v[] = {1e-4, 1e-3, 1e-2, 1e-1, 1, 10, 1e2, 1e3, 1e4};
        return floor(Value * v[Idx] + 0.5) / (v[Idx]);
    }
#endif

    PyObject *PyWinObject_FromDATE(DATE t)
    {
#ifdef PYWIN_HAVE_DATETIME_CAPI
        // via https://www.codeproject.com/Articles/17576/SystemTime-to-VariantTime-with-Milliseconds
        // (in particular, see the comments)
        double fraction = t - (int)t;  // extracts the fraction part
        double hours = (fraction - (int)fraction) * 24.0;
        double minutes = (hours - (int)hours) * 60.0;
        double seconds = round((minutes - (int)minutes) * 60.0, 4);
        double milliseconds = round((seconds - (int)seconds) * 1000.0, 0);
        // assert(milliseconds>=0.0 && milliseconds<=999.0);

        // Strip off the msec part of time
        double TimeWithoutMsecs = t - (ONETHOUSANDMILLISECONDS / 1000.0 * milliseconds);

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
#endif  // PYWIN_HAVE_DATETIME_CAPI

#ifndef NO_PYWINTYPES_TIME
        return new PyTime(t);
#endif
    }

    PyObject *PyWinTimeObject_Fromtime_t(time_t t)
    {
#ifdef PYWIN_HAVE_DATETIME_CAPI
        PyObject *args = Py_BuildValue("(i)", (int)t);
        if (!args)
            return NULL;
        PyObject *ret = PyDateTimeAPI->DateTime_FromTimestamp((PyObject *)(&PyWinDateTimeType), args, NULL);
        Py_DECREF(args);
        return ret;
#endif  // PYWIN_HAVE_DATETIME_CAPI

#ifndef NO_PYWINTYPES_TIME
        return new PyTime(t);
#endif
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
#ifdef PYWIN_HAVE_DATETIME_CAPI
        PyDateTime_IMPORT;
        if (!PyDateTimeAPI)
            return NULL;
        PyWinDateTimeType.tp_base = PyDateTimeAPI->DateTimeType;
        PyWinDateTimeType.tp_basicsize = PyDateTimeAPI->DateTimeType->tp_basicsize;
        PyWinDateTimeType.tp_new = PyDateTimeAPI->DateTimeType->tp_new;
        PyWinDateTimeType.tp_dealloc = PyDateTimeAPI->DateTimeType->tp_dealloc;
        if (PyType_Ready(&PyWinDateTimeType) == -1)
            return FALSE;
#endif
        return TRUE;
    }

    BOOL _PyWinDateTime_PrepareModuleDict(PyObject * dict)
    {
#ifndef NO_PYWINTYPES_TIME
        if (PyType_Ready(&PyTimeType) == -1 || PyDict_SetItemString(dict, "TimeType", (PyObject *)&PyTimeType) == -1)
            return FALSE;
#endif

#ifdef PYWIN_HAVE_DATETIME_CAPI
        if (PyDict_SetItemString(dict, "TimeType", (PyObject *)&PyWinDateTimeType) == -1)
            return FALSE;
#endif
        return TRUE;
    }
