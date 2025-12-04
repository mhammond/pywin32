// @doc
#include "PyWinTypes.h"
#include "PyWinObjects.h"
#include "structmember.h"

#include "malloc.h"

#define PyW32_BEGIN_ALLOW_THREADS PyThreadState *_save = PyEval_SaveThread()
#define PyW32_END_ALLOW_THREADS PyEval_RestoreThread(_save)
#define PyW32_BLOCK_THREADS Py_BLOCK_THREADS

// function pointers
#define CHECK_PFN(fname)    \
    if (pfn##fname == NULL) \
        return PyErr_Format(PyExc_NotImplementedError, "%s is not available on this platform", #fname);
HMODULE kernel32_dll;
typedef DWORD(WINAPI *GetConsoleProcessListfunc)(LPDWORD, DWORD);
static GetConsoleProcessListfunc pfnGetConsoleProcessList = NULL;
typedef BOOL(WINAPI *GetConsoleDisplayModefunc)(LPDWORD);
static GetConsoleDisplayModefunc pfnGetConsoleDisplayMode = NULL;
typedef BOOL(WINAPI *SetConsoleDisplayModefunc)(HANDLE, DWORD, PCOORD);
static SetConsoleDisplayModefunc pfnSetConsoleDisplayMode;
typedef BOOL(WINAPI *AttachConsolefunc)(DWORD);
static AttachConsolefunc pfnAttachConsole = NULL;
typedef BOOL(WINAPI *AddConsoleAliasfunc)(LPWSTR, LPWSTR, LPWSTR);
static AddConsoleAliasfunc pfnAddConsoleAlias = NULL;
typedef DWORD(WINAPI *GetConsoleAliasesLengthfunc)(LPWSTR);
static GetConsoleAliasesLengthfunc pfnGetConsoleAliasesLength = NULL;
typedef DWORD(WINAPI *GetConsoleAliasesfunc)(LPWSTR, DWORD, LPTSTR);
static GetConsoleAliasesfunc pfnGetConsoleAliases = NULL;
typedef DWORD(WINAPI *GetConsoleAliasExesfunc)(LPWSTR, DWORD);
static GetConsoleAliasExesfunc pfnGetConsoleAliasExes = NULL;
typedef DWORD(WINAPI *GetConsoleAliasExesLengthfunc)(VOID);
static GetConsoleAliasExesLengthfunc pfnGetConsoleAliasExesLength = NULL;
typedef HWND(WINAPI *GetConsoleWindowfunc)(void);
static GetConsoleWindowfunc pfnGetConsoleWindow = NULL;
typedef BOOL(WINAPI *GetCurrentConsoleFontfunc)(HANDLE, BOOL, PCONSOLE_FONT_INFO);
static GetCurrentConsoleFontfunc pfnGetCurrentConsoleFont;
typedef COORD(WINAPI *GetConsoleFontSizefunc)(HANDLE, DWORD);
static GetConsoleFontSizefunc pfnGetConsoleFontSize = NULL;
typedef BOOL(WINAPI *GetConsoleSelectionInfofunc)(PCONSOLE_SELECTION_INFO);
static GetConsoleSelectionInfofunc pfnGetConsoleSelectionInfo = NULL;
typedef DWORD(WINAPI *GetNumberOfConsoleFontsfunc)(VOID);
static GetNumberOfConsoleFontsfunc pfnGetNumberOfConsoleFonts = NULL;
typedef BOOL(WINAPI *SetConsoleFontfunc)(HANDLE, DWORD);
static SetConsoleFontfunc pfnSetConsoleFont = NULL;

// convert python object to array of WORDS/USHORTS
// ?????? should move this into Pywintypes, similar code used in win32security_ds.cpp
// to create an array of USHORTS
BOOL PyWinObject_AsUSHORTArray(PyObject *obushorts, USHORT **pushorts, DWORD *item_cnt, BOOL bNoneOk = TRUE)
{
    BOOL ret = TRUE;
    DWORD bufsize, tuple_index;
    long short_candidate;
    PyObject *ushorts_tuple = NULL, *tuple_item;
    *pushorts = NULL;
    if (obushorts == Py_None) {
        if (bNoneOk)
            return TRUE;
        PyErr_SetString(PyExc_ValueError, "Sequence of unsigned shorts cannot be None");
        return FALSE;
    }
    if ((ushorts_tuple = PyWinSequence_Tuple(obushorts, item_cnt)) == NULL)
        return FALSE;  // last exit without cleaning up
    bufsize = *item_cnt * sizeof(USHORT);
    *pushorts = (USHORT *)malloc(bufsize);
    if (*pushorts == NULL) {
        PyErr_Format(PyExc_MemoryError, "Unable to allocate %d bytes", bufsize);
        ret = FALSE;
    }
    else
        for (tuple_index = 0; tuple_index < *item_cnt; tuple_index++) {
            tuple_item = PyTuple_GET_ITEM(ushorts_tuple, tuple_index);
            short_candidate = PyLong_AsLong(tuple_item);
            if (short_candidate == -1 && PyErr_Occurred()) {
                ret = FALSE;
                break;
            }
            else if (short_candidate < 0) {
                PyErr_Format(PyExc_ValueError, "Unsigned short cannot be negative");
                ret = FALSE;
                break;
            }
            else if (short_candidate > USHRT_MAX) {
                PyErr_Format(PyExc_ValueError, "Unsigned short cannot exceed %d", USHRT_MAX);
                ret = FALSE;
                break;
            }
            else
                (*pushorts)[tuple_index] = (USHORT)short_candidate;
        }
    if (!ret)
        if (*pushorts != NULL) {
            free(*pushorts);
            *pushorts = NULL;
        }
    Py_XDECREF(ushorts_tuple);
    return ret;
}

// convert python object to single unicode character
// object *must* be unicode, and onechar should be allocated for a single WCHAR
// used mostly for putting a WCHAR inside an existing struct - would be nice if the
// structmember framework provided a format code for this
BOOL PyWinObject_AsSingleWCHAR(PyObject *obchar, WCHAR *onechar)
{
    if (!PyUnicode_Check(obchar) || (PyUnicode_GetLength(obchar) != 1)) {
        PyErr_SetString(PyExc_ValueError, "Object must be a single unicode character");
        return FALSE;
    }
#define PUAWC_TYPE PyObject *
    if (PyUnicode_AsWideChar((PUAWC_TYPE)obchar, onechar, 1) == -1)
        return FALSE;
    return TRUE;
}

// @object PySMALL_RECT|Wrapper for a SMALL_RECT struct
// Create using PySMALL_RECTType(Left, Top, Right, Bottom). All params optional, defaulting to 0

class PySMALL_RECT : public PyObject {
   public:
    static struct PyMemberDef members[];
    // static struct PyMethodDef methods[];
    static void tp_dealloc(PyObject *ob);
    SMALL_RECT rect;
    PySMALL_RECT(SMALL_RECT *);
    PySMALL_RECT(void);
    static PyObject *tp_new(PyTypeObject *tp, PyObject *args, PyObject *kwargs);
    static PyObject *tp_str(PyObject *self);
};

/*
static struct PyMethodDef PySMALL_RECT::methods[] =
{
    {NULL}
};
*/

struct PyMemberDef PySMALL_RECT::members[] = {
    {"Left", T_SHORT, offsetof(PySMALL_RECT, rect.Left), 0, NULL},      // @prop int|Left|Left side of rectangle
    {"Top", T_SHORT, offsetof(PySMALL_RECT, rect.Top), 0, NULL},        // @prop int|Top|Top edge of rectangle
    {"Right", T_SHORT, offsetof(PySMALL_RECT, rect.Right), 0, NULL},    // @prop int|Right|Right edge of rectangle
    {"Bottom", T_SHORT, offsetof(PySMALL_RECT, rect.Bottom), 0, NULL},  // @prop int|Bottom|Bottome edge of rectangle
    {NULL}};

static PyTypeObject PySMALL_RECTType = {
    PYWIN_OBJECT_HEAD "PySMALL_RECT",
    sizeof(PySMALL_RECT),
    0,
    PySMALL_RECT::tp_dealloc,
    0,                                                                                           // tp_print
    0,                                                                                           // tp_getattr
    0,                                                                                           // tp_setattr
    0,                                                                                           // tp_compare
    PySMALL_RECT::tp_str,                                                                        // tp_repr
    0,                                                                                           // tp_as_number
    0,                                                                                           // tp_as_sequence
    0,                                                                                           // tp_as_mapping
    0,                                                                                           // tp_hash
    0,                                                                                           // tp_call
    PySMALL_RECT::tp_str,                                                                        // tp_str
    PyObject_GenericGetAttr,                                                                     // tp_getattro
    PyObject_GenericSetAttr,                                                                     // tp_setattro
    0,                                                                                           // tp_as_buffer;
    Py_TPFLAGS_DEFAULT,                                                                          // tp_flags;
    "Wrapper for a SMALL_RECT struct. Create using PySMALL_RECTType(Left, Top, Right, Bottom)",  // tp_doc
    0,  // traverseproc tp_traverse;
    0,  // tp_clear;
    0,  // tp_richcompare;
    0,  // tp_weaklistoffset;
    0,  // tp_iter
    0,  // tp_iternext
    0,  // PySMALL_RECT::methods
    PySMALL_RECT::members,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    PySMALL_RECT::tp_new};

PyObject *PySMALL_RECT::tp_new(PyTypeObject *tp, PyObject *args, PyObject *kwargs)
{
    SMALL_RECT sr;
    ZeroMemory(&sr, sizeof(SMALL_RECT));
    static char *keywords[] = {"Left", "Top", "Right", "Bottom", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|HHHH:PySMALL_RECTType", keywords, &sr.Left, &sr.Top, &sr.Right,
                                     &sr.Bottom))
        return NULL;
    return new PySMALL_RECT(&sr);
}

PyObject *PySMALL_RECT::tp_str(PyObject *self)
{
    char buf[100];
    int chars_printed;
    SMALL_RECT sr = ((PySMALL_RECT *)self)->rect;
    chars_printed = _snprintf(buf, 100, "PySMALL_RECTType(Left=%d,Top=%d,Right=%d,Bottom=%d)", sr.Left, sr.Top,
                              sr.Right, sr.Bottom);
    if (chars_printed < 0) {
        PyErr_SetString(PyExc_SystemError, "String representation of PySMALL_RECT too long for buffer");
        return NULL;
    }
    return PyWinCoreString_FromString(buf, chars_printed);
}

PySMALL_RECT::PySMALL_RECT(SMALL_RECT *psr)
{
    ob_type = &PySMALL_RECTType;
    rect = *psr;
    _Py_NewReference(this);
}

PySMALL_RECT::PySMALL_RECT(void)
{
    ob_type = &PySMALL_RECTType;
    ZeroMemory(&rect, sizeof(SMALL_RECT));
    _Py_NewReference(this);
}

void PySMALL_RECT::tp_dealloc(PyObject *ob) { delete (PySMALL_RECT *)ob; }

BOOL PySMALL_RECT_check(PyObject *ob)
{
    if (Py_TYPE(ob) != &PySMALL_RECTType) {
        PyErr_SetString(PyExc_TypeError, "Object must be a PySMALL_RECT");
        return FALSE;
    }
    return TRUE;
}

BOOL PyWinObject_AsSMALL_RECT(PyObject *obrect, PSMALL_RECT *pprect, BOOL bNoneOk = TRUE)
{
    *pprect = NULL;
    if (obrect == Py_None) {
        if (bNoneOk)
            return TRUE;
        PyErr_SetString(PyExc_ValueError, "SMALL_RECT cannot be None");
        return FALSE;
    }
    if (!PySMALL_RECT_check(obrect))
        return FALSE;
    *pprect = &((PySMALL_RECT *)obrect)->rect;
    return TRUE;
}

PyObject *PyWinObject_FromSMALL_RECT(PSMALL_RECT psr)
{
    PyObject *ret = new PySMALL_RECT(psr);
    if (ret == NULL)
        PyErr_SetString(PyExc_MemoryError, "Unable to create PySMALL_RECT instance");
    return ret;
}

// @object PyCOORD|Wrapper for a COORD struct.  Create using PyCOORDType(X,Y)
class PyCOORD : public PyObject {
   public:
    static struct PyMemberDef members[];
    // static struct PyMethodDef methods[];
    static void deallocFunc(PyObject *ob);
    COORD coord;
    PyCOORD(COORD *);
    PyCOORD(void);
    static PyObject *tp_new(PyTypeObject *tp, PyObject *args, PyObject *kwargs);
    static PyObject *tp_str(PyObject *self);

   protected:
    ~PyCOORD();
};

/*
static struct PyMethodDef PyCOORD::methods[] =
{
    {NULL}
};
*/

struct PyMemberDef PyCOORD::members[] = {
    {"X", T_SHORT, offsetof(PyCOORD, coord.X), 0, "Horizontal coordinate"},  // @prop int|X|Horizontal coordinate
    {"Y", T_SHORT, offsetof(PyCOORD, coord.Y), 0, "Vertical coordinate"},    // @prop int|Y|Vertical coordinate
    {NULL}};

static PyTypeObject PyCOORDType = {PYWIN_OBJECT_HEAD "PyCOORD",
                                   sizeof(PyCOORD),
                                   0,
                                   PyCOORD::deallocFunc,
                                   0,                                                            // tp_print
                                   0,                                                            // tp_getattr
                                   0,                                                            // tp_setattr
                                   0,                                                            // tp_compare
                                   PyCOORD::tp_str,                                              // tp_repr
                                   0,                                                            // tp_as_number
                                   0,                                                            // tp_as_sequence
                                   0,                                                            // tp_as_mapping
                                   0,                                                            // tp_hash
                                   0,                                                            // tp_call
                                   PyCOORD::tp_str,                                              // tp_str
                                   PyObject_GenericGetAttr,                                      // tp_getattro
                                   PyObject_GenericSetAttr,                                      // tp_setattro
                                   0,                                                            // tp_as_buffer;
                                   Py_TPFLAGS_DEFAULT,                                           // tp_flags;
                                   "Wrapper for a COORD struct. Create using PyCOORDType(X,Y)",  // tp_doc
                                   0,                 // traverseproc tp_traverse;
                                   0,                 // tp_clear;
                                   0,                 // tp_richcompare;
                                   0,                 // tp_weaklistoffset;
                                   0,                 // tp_iter
                                   0,                 // tp_iternext
                                   0,                 // PyCOORD::methods			// tp_methods
                                   PyCOORD::members,  // tp_members
                                   0,                 // tp_getset
                                   0,                 // tp_base
                                   0,                 // tp_dict
                                   0,                 // tp_descr_get
                                   0,                 // tp_descr_set
                                   0,                 // tp_dictoffset
                                   0,                 // tp_init
                                   0,                 // tp_alloc
                                   PyCOORD::tp_new};

PyObject *PyCOORD::tp_new(PyTypeObject *tp, PyObject *args, PyObject *kwargs)
{
    COORD coord;
    ZeroMemory(&coord, sizeof(COORD));
    static char *keywords[] = {"X", "Y", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|HH:PyCOORDType", keywords, &coord.X, &coord.Y))
        return NULL;
    return new PyCOORD(&coord);
}

PyObject *PyCOORD::tp_str(PyObject *self)
{
    char buf[60];
    int chars_printed;
    COORD coord = ((PyCOORD *)self)->coord;
    chars_printed = _snprintf(buf, 60, "PyCOORDType(X=%d,Y=%d)", coord.X, coord.Y);
    if (chars_printed < 0) {
        PyErr_SetString(PyExc_SystemError, "String representation of PyCOORD too long for buffer");
        return NULL;
    }
    return PyWinCoreString_FromString(buf, chars_printed);
}

PyCOORD::PyCOORD(COORD *pcoord)
{
    ob_type = &PyCOORDType;
    coord = *pcoord;
    _Py_NewReference(this);
}

PyCOORD::PyCOORD(void)
{
    ob_type = &PyCOORDType;
    ZeroMemory(&coord, sizeof(COORD));
    _Py_NewReference(this);
}

PyCOORD::~PyCOORD() {}

void PyCOORD::deallocFunc(PyObject *ob) { delete (PyCOORD *)ob; }

BOOL PyCOORD_Check(PyObject *ob)
{
    if (Py_TYPE(ob) != &PyCOORDType) {
        PyErr_SetString(PyExc_TypeError, "Object must be a PyCOORD");
        return FALSE;
    }
    return TRUE;
}

BOOL PyWinObject_AsCOORD(PyObject *obcoord, COORD **ppcoord, BOOL bNoneOk = TRUE)
{
    *ppcoord = NULL;
    if (obcoord == Py_None) {
        if (bNoneOk)
            return TRUE;
        PyErr_SetString(PyExc_ValueError, "COORD must not be None in this context");
        return FALSE;
    }
    if (!PyCOORD_Check(obcoord))
        return FALSE;
    *ppcoord = &((PyCOORD *)obcoord)->coord;
    return TRUE;
}

PyObject *PyWinObject_FromCOORD(COORD *pcoord)
{
    PyObject *ret = new PyCOORD(pcoord);
    if (ret == NULL)
        PyErr_SetString(PyExc_MemoryError, "Unable to create PyCOORD object");
    return ret;
}

// @object PyINPUT_RECORD|Interface to the INPUT_RECORD struct used with console IO functions.  Create using
// PyINPUT_RECORDType(EventType)
// @comm Only attributes that apply to each particular EventType can be accessed:<nl>
//	KEY_EVENT: KeyDown, RepeatCount, VirtualKeyCode, VirtualScanCode, ControlKeyState<nl>
//	MOUSE_EVENT: MousePosition, ButtonState, ControlKeyState, EventFlags<nl>
//	WINDOW_BUFFER_SIZE_EVENT: Size<nl>
//	FOCUS_EVENT: SetFocus<nl>
//	MENU_EVENT: CommandId<nl>

class PyINPUT_RECORD : public PyObject {
   public:
    static struct PyMemberDef members[];
    // static struct PyMethodDef methods[];
    static void tp_dealloc(PyObject *self);
    INPUT_RECORD input_record;
    PyINPUT_RECORD(INPUT_RECORD *);
    PyINPUT_RECORD(WORD EventType);
    PyCOORD *obcoord;
    static PyObject *tp_new(PyTypeObject *tp, PyObject *args, PyObject *kwargs);
    static PyObject *tp_str(PyObject *self);
    static PyObject *tp_getattro(PyObject *self, PyObject *obname);
    static int tp_setattro(PyObject *self, PyObject *obname, PyObject *obvalue);
};

//	Many of these are handled manually in PyINPUT_RECORD::tp_setattro and tp_getattro,
//		but kept here so they are visible
struct PyMemberDef PyINPUT_RECORD::members[] = {
    // @prop int|EventType|One of KEY_EVENT, MOUSE_EVENT, WINDOW_BUFFER_SIZE_EVENT, MENU_EVENT, FOCUS_EVENT. Cannot be
    // changed after object is created
    {"EventType", T_USHORT, offsetof(PyINPUT_RECORD, input_record.EventType), READONLY,
     "One of KEY_EVENT, MOUSE_EVENT, WINDOW_BUFFER_SIZE_EVENT, MENU_EVENT, FOCUS_EVENT.  Cannot be changed after "
     "object is created"},
    // @prop boolean|KeyDown|True for a key press, False for key release
    {"KeyDown", T_LONG, offsetof(PyINPUT_RECORD, input_record.Event.KeyEvent.bKeyDown), 0,
     "True for a key press, False for key release"},
    // @prop int|RepeatCount|Nbr of repeats generated (key was held down if >1)
    {"RepeatCount", T_USHORT, offsetof(PyINPUT_RECORD, input_record.Event.KeyEvent.wRepeatCount), 0,
     "Nbr of repeats generated (key was held down if >1)"},
    // @prop int|VirtualKeyCode|Device-independent key code, win32con.VK_*
    {"VirtualKeyCode", T_USHORT, offsetof(PyINPUT_RECORD, input_record.Event.KeyEvent.wVirtualKeyCode), 0,
     "Device-independent key code, win32con.VK_*"},
    // @prop int|VirtualScanCode|Device-dependent scan code generated by keyboard
    {"VirtualScanCode", T_USHORT, offsetof(PyINPUT_RECORD, input_record.Event.KeyEvent.wVirtualScanCode), 0,
     "Device-dependent scan code generated by keyboard"},
    // @prop <o PyUnicode>|Char|Single unicode character generated by the keypress
    {"Char", T_LONG, 0, 0, "Single unicode character generated by the keypress"},
    // @prop int|ControlKeyState|State of modifier keys, combination of CAPSLOCK_ON, ENHANCED_KEY, LEFT_ALT_PRESSED,
    //  LEFT_CTRL_PRESSED, NUMLOCK_ON, RIGHT_ALT_PRESSED, RIGHT_CTRL_PRESSED, SCROLLLOCK_ON, SHIFT_PRESSED
    {"ControlKeyState", T_ULONG, 0, 0,
     "State of modifier keys, combination of CAPSLOCK_ON, ENHANCED_KEY, LEFT_ALT_PRESSED, LEFT_CTRL_PRESSED,"
     "NUMLOCK_ON, RIGHT_ALT_PRESSED, RIGHT_CTRL_PRESSED, SCROLLLOCK_ON, SHIFT_PRESSED"},
    // @prop int|ButtonState|Bitmask representing which mouse buttons were pressed.
    {"ButtonState", T_ULONG, offsetof(PyINPUT_RECORD, input_record.Event.MouseEvent.dwButtonState), 0,
     "Bitmask representing which mouse buttons were pressed"},
    // @prop int|EventFlags|DOUBLE_CLICK, MOUSE_MOVED or MOUSE_WHEELED, or 0.  If 0, indicates a mouse button press
    {"EventFlags", T_ULONG, offsetof(PyINPUT_RECORD, input_record.Event.MouseEvent.dwEventFlags), 0,
     "DOUBLE_CLICK, MOUSE_MOVED or MOUSE_WHEELED, or 0.  If 0, indicates a mouse button press"},
    // @prop <o PyCOORD>|MousePosition|Position in character coordinates
    {"MousePosition", T_ULONG, 0, 0, "Position in character coordinates"},
    // @prop <o PyCOORD>|Size|New size of screen buffer in character rows/columns
    {"Size", T_ULONG, 0, 0, "New size of screen buffer in character rows/columns"},
    // @prop boolean|SetFocus|Reserved - Used only with type FOCUS_EVENT.  This event is Reserved, and should be
    // ignored.
    {"SetFocus", T_ULONG, offsetof(PyINPUT_RECORD, input_record.Event.FocusEvent.bSetFocus), 0, "Reserved"},
    // @prop int|CommandId|Used only with event type MENU_EVENT, which is reserved and should not be used
    {"CommandId", T_ULONG, offsetof(PyINPUT_RECORD, input_record.Event.MenuEvent.dwCommandId), 0, "Reserved"},
    {NULL}};

PyObject *PyINPUT_RECORD::tp_getattro(PyObject *self, PyObject *obname)
{
    INPUT_RECORD *pir = &((PyINPUT_RECORD *)self)->input_record;
    char *name = PYWIN_ATTR_CONVERT(obname);
    if (name == NULL)
        return NULL;
    if (strcmp(name, "ControlKeyState") == 0) {
        DWORD *src_ptr;
        if (pir->EventType == KEY_EVENT)
            src_ptr = &pir->Event.KeyEvent.dwControlKeyState;
        else if (pir->EventType == MOUSE_EVENT)
            src_ptr = &pir->Event.MouseEvent.dwControlKeyState;
        else {
            PyErr_SetString(PyExc_AttributeError, "'ConrolKeyState' is only valid for KEY_EVENT or MOUSE_EVENT");
            return NULL;
        }
        return PyLong_FromUnsignedLong(*src_ptr);
    }

    if (strcmp(name, "Char") == 0) {
        if (pir->EventType != KEY_EVENT) {
            PyErr_SetString(PyExc_AttributeError, "'Char' is only valid for type KEY_EVENT");
            return NULL;
        }
        return PyWinObject_FromWCHAR(&pir->Event.KeyEvent.uChar.UnicodeChar, 1);
    }

    if (strcmp(name, "Size") == 0) {
        if (pir->EventType != WINDOW_BUFFER_SIZE_EVENT) {
            PyErr_SetString(PyExc_AttributeError, "'Size' is only valid for type WINDOW_BUFFER_SIZE_EVENT");
            return NULL;
        }
        Py_INCREF(((PyINPUT_RECORD *)self)->obcoord);
        return ((PyINPUT_RECORD *)self)->obcoord;
    }

    if (strcmp(name, "MousePosition") == 0) {
        if (pir->EventType != MOUSE_EVENT) {
            PyErr_SetString(PyExc_AttributeError, "'MousePosition' is only valid for type MOUSE_EVENT");
            return NULL;
        }
        Py_INCREF(((PyINPUT_RECORD *)self)->obcoord);
        return ((PyINPUT_RECORD *)self)->obcoord;
    }

    return PyObject_GenericGetAttr(self, obname);
}

int PyINPUT_RECORD::tp_setattro(PyObject *self, PyObject *obname, PyObject *obvalue)
{
    INPUT_RECORD *pir = &((PyINPUT_RECORD *)self)->input_record;
    char *name;
    name = PYWIN_ATTR_CONVERT(obname);
    if (name == NULL)
        return -1;
    if (obvalue == NULL) {
        PyErr_SetString(PyExc_AttributeError, "PyINPUT_RECORD members can't be removed");
        return -1;
    }
    // ??? should probably add some EventType/attribute validation for everything done thru
    //  the normal structmember api also ???
    if (strcmp(name, "ControlKeyState") == 0) {
        // Event union contains 2 different ConrolKeyState's at different offsets depending on event type
        DWORD *dest_ptr;
        if (pir->EventType == KEY_EVENT)
            dest_ptr = &pir->Event.KeyEvent.dwControlKeyState;
        else if (pir->EventType == MOUSE_EVENT)
            dest_ptr = &pir->Event.MouseEvent.dwControlKeyState;
        else {
            PyErr_SetString(PyExc_AttributeError, "'ConrolKeyState' is only valid for KEY_EVENT or MOUSE_EVENT");
            return -1;
        }

        *dest_ptr = PyLong_AsUnsignedLongMask(obvalue);
        if ((*dest_ptr == (DWORD)-1) && PyErr_Occurred())
            return -1;
        return 0;
    }

    if (strcmp(name, "Char") == 0) {
        if (pir->EventType != KEY_EVENT) {
            PyErr_SetString(PyExc_AttributeError, "'Char' is only valid for type KEY_EVENT");
            return -1;
        }
        if (!PyWinObject_AsSingleWCHAR(obvalue, &pir->Event.KeyEvent.uChar.UnicodeChar))
            return -1;
        return 0;
    }

    if (strcmp(name, "Size") == 0) {
        if (pir->EventType != WINDOW_BUFFER_SIZE_EVENT) {
            PyErr_SetString(PyExc_AttributeError, "'Size' is only valid for type WINDOW_BUFFER_SIZE_EVENT");
            return -1;
        }
        if (!PyCOORD_Check(obvalue))
            return -1;
        ((PyINPUT_RECORD *)self)->input_record.Event.WindowBufferSizeEvent.dwSize = ((PyCOORD *)obvalue)->coord;
        Py_DECREF(((PyINPUT_RECORD *)self)->obcoord);
        Py_INCREF(obvalue);
        ((PyINPUT_RECORD *)self)->obcoord = (PyCOORD *)obvalue;
        return 0;
    }

    if (strcmp(name, "MousePosition") == 0) {
        if (pir->EventType != MOUSE_EVENT) {
            PyErr_SetString(PyExc_AttributeError, "'MousePosition' is only valid for type MOUSE_EVENT");
            return -1;
        }
        if (!PyCOORD_Check(obvalue))
            return -1;
        ((PyINPUT_RECORD *)self)->input_record.Event.MouseEvent.dwMousePosition = ((PyCOORD *)obvalue)->coord;
        Py_DECREF(((PyINPUT_RECORD *)self)->obcoord);
        Py_INCREF(obvalue);
        ((PyINPUT_RECORD *)self)->obcoord = (PyCOORD *)obvalue;
        return 0;
    }

    return PyObject_GenericSetAttr(self, obname, obvalue);
}

static PyTypeObject PyINPUT_RECORDType = {
    PYWIN_OBJECT_HEAD "PyINPUT_RECORD",
    sizeof(PyINPUT_RECORD),
    0,
    PyINPUT_RECORD::tp_dealloc,
    0,                                                                                // tp_print
    0,                                                                                // tp_getattr
    0,                                                                                // tp_setattr
    0,                                                                                // tp_compare
    PyINPUT_RECORD::tp_str,                                                           // tp_repr
    0,                                                                                // tp_as_number
    0,                                                                                // tp_as_sequence
    0,                                                                                // tp_as_mapping
    0,                                                                                // tp_hash
    0,                                                                                // tp_call
    PyINPUT_RECORD::tp_str,                                                           // tp_str
    PyINPUT_RECORD::tp_getattro,                                                      // tp_getattro
    PyINPUT_RECORD::tp_setattro,                                                      // tp_setattro
    0,                                                                                // tp_as_buffer;
    Py_TPFLAGS_DEFAULT,                                                               // tp_flags;
    "Wrapper for a INPUT_RECORD struct. Create using PyINPUT_RECORDType(EventType)",  // tp_doc
    0,                                                                                // traverseproc tp_traverse;
    0,                                                                                // tp_clear;
    0,                                                                                // tp_richcompare;
    0,                                                                                // tp_weaklistoffset;
    0,                                                                                // tp_iter
    0,                                                                                // tp_iternext
    0,                                                                                // PySMALL_RECT::methods
    PyINPUT_RECORD::members,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    PyINPUT_RECORD::tp_new};

PyINPUT_RECORD::PyINPUT_RECORD(WORD EventType)
{
    // EventType can't be changed after object is created
    ob_type = &PyINPUT_RECORDType;
    ZeroMemory(&input_record, sizeof(INPUT_RECORD));
    input_record.EventType = EventType;
    // keep a reference to a PyCOORD, used by 2 different types of events
    if ((EventType == MOUSE_EVENT) || (EventType == WINDOW_BUFFER_SIZE_EVENT))
        obcoord = new PyCOORD();
    else
        obcoord = NULL;
    _Py_NewReference(this);
}

PyINPUT_RECORD::PyINPUT_RECORD(INPUT_RECORD *pinput_record)
{
    ob_type = &PyINPUT_RECORDType;
    input_record = *pinput_record;
    if (input_record.EventType == MOUSE_EVENT)
        obcoord = new PyCOORD(&input_record.Event.MouseEvent.dwMousePosition);
    else if (input_record.EventType == WINDOW_BUFFER_SIZE_EVENT)
        obcoord = new PyCOORD(&input_record.Event.WindowBufferSizeEvent.dwSize);
    else
        obcoord = NULL;
    _Py_NewReference(this);
}

void PyINPUT_RECORD::tp_dealloc(PyObject *self)
{
    Py_XDECREF(((PyINPUT_RECORD *)self)->obcoord);
    delete (PyINPUT_RECORD *)self;
}

BOOL PyINPUT_RECORD_Check(PyObject *ob)
{
    if (Py_TYPE(ob) != &PyINPUT_RECORDType) {
        PyErr_SetString(PyExc_TypeError, "Object must be a PyINPUT_RECORD");
        return FALSE;
    }
    return TRUE;
}

BOOL PyWinObject_AsINPUT_RECORD(PyObject *obir, INPUT_RECORD **ppir)
{
    if (!PyINPUT_RECORD_Check(obir))
        return FALSE;
    *ppir = &((PyINPUT_RECORD *)obir)->input_record;
    // pick up any changes to the PyCOORD associated with the input record
    if ((*ppir)->EventType == MOUSE_EVENT)
        (*ppir)->Event.MouseEvent.dwMousePosition = ((PyINPUT_RECORD *)obir)->obcoord->coord;
    else if ((*ppir)->EventType == WINDOW_BUFFER_SIZE_EVENT)
        (*ppir)->Event.WindowBufferSizeEvent.dwSize = ((PyINPUT_RECORD *)obir)->obcoord->coord;
    return TRUE;
}

PyObject *PyWinObject_FromINPUT_RECORD(INPUT_RECORD *pinput_record)
{
    PyObject *ret = new PyINPUT_RECORD(pinput_record);
    if (ret == NULL)
        PyErr_SetString(PyExc_MemoryError, "Unable to create PyINPUT_RECORD");
    return ret;
}

PyObject *PyINPUT_RECORD::tp_new(PyTypeObject *tp, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"EventType", NULL};
    WORD EventType;
    PyObject *ret;
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "H:PyINPUT_RECORDType", keywords, &EventType))
        return NULL;
    ret = new PyINPUT_RECORD(EventType);
    if (ret == NULL)
        PyErr_SetString(PyExc_MemoryError, "Unable to create PyINPUT_RECORD object");
    return ret;
}

PyObject *PyINPUT_RECORD::tp_str(PyObject *self)
{
    char buf[100];
    char *rec_type;
    int chars_printed;

    if (((PyINPUT_RECORD *)self)->input_record.EventType == KEY_EVENT)
        rec_type = "KEY_EVENT";
    else if (((PyINPUT_RECORD *)self)->input_record.EventType == MOUSE_EVENT)
        rec_type = "MOUSE_EVENT";
    else if (((PyINPUT_RECORD *)self)->input_record.EventType == WINDOW_BUFFER_SIZE_EVENT)
        rec_type = "WINDOW_BUFFER_SIZE_EVENT";
    else if (((PyINPUT_RECORD *)self)->input_record.EventType == MENU_EVENT)
        rec_type = "MENU_EVENT";
    else if (((PyINPUT_RECORD *)self)->input_record.EventType == FOCUS_EVENT)
        rec_type = "FOCUS_EVENT";
    else
        rec_type = "<Unknown>";

    chars_printed = _snprintf(buf, 100, "PyINPUT_RECORD(EventType=%d) (%s)",
                              ((PyINPUT_RECORD *)self)->input_record.EventType, rec_type);
    if (chars_printed < 0) {
        PyErr_SetString(PyExc_SystemError, "String representation of PyINPUT_RECORD too long for buffer");
        return NULL;
    }
    return PyWinCoreString_FromString(buf, chars_printed);
}

// @object PyConsoleScreenBuffer|Handle to a console screen buffer
// Create using <om win32console.CreateConsoleScreenBuffer> or <om win32console.GetStdHandle>
// Use PyConsoleScreenBufferType(Handle) to wrap a pre-existing handle as returned by <om win32api.GetStdHandle>.
// Will also accept a handle created by <om win32file.CreateFile> for CONIN$ or CONOUT$.
// When an existing handle is wrapped, a copy is made using DuplicateHandle, and caller is still responsible
// for any cleanup of original handle.
class PyConsoleScreenBuffer : public PyHANDLE {
   public:
    PyConsoleScreenBuffer(HANDLE hconsole);
    ~PyConsoleScreenBuffer(void);
    static void tp_dealloc(PyObject *ob);
    const char *GetTypeName() { return "PyConsoleScreenBuffer"; }
    // static struct PyMemberDef members[];
    static struct PyMethodDef methods[];
    static PyObject *tp_new(PyTypeObject *tp, PyObject *args, PyObject *kwargs);
    static PyObject *PySetConsoleActiveScreenBuffer(PyObject *self, PyObject *args);
    static PyObject *PyGetConsoleCursorInfo(PyObject *self, PyObject *args);
    static PyObject *PySetConsoleCursorInfo(PyObject *self, PyObject *args, PyObject *kwargs);
    static PyObject *PyGetConsoleMode(PyObject *self, PyObject *args);
    static PyObject *PySetConsoleMode(PyObject *self, PyObject *args, PyObject *kwargs);
    static PyObject *PyReadConsole(PyObject *self, PyObject *args, PyObject *kwargs);
    static PyObject *PyWriteConsole(PyObject *self, PyObject *args, PyObject *kwargs);
    static PyObject *PyFlushConsoleInputBuffer(PyObject *self, PyObject *args);
    static PyObject *PySetConsoleTextAttribute(PyObject *self, PyObject *args, PyObject *kwargs);
    static PyObject *PySetConsoleCursorPosition(PyObject *self, PyObject *args, PyObject *kwargs);
    static PyObject *PySetConsoleScreenBufferSize(PyObject *self, PyObject *args, PyObject *kwargs);
    static PyObject *PySetConsoleWindowInfo(PyObject *self, PyObject *args, PyObject *kwargs);
    static PyObject *PyGetConsoleScreenBufferInfo(PyObject *self, PyObject *args);
    static PyObject *PyGetLargestConsoleWindowSize(PyObject *self, PyObject *args);
    static PyObject *PyFillConsoleOutputAttribute(PyObject *self, PyObject *args, PyObject *kwargs);
    static PyObject *PyFillConsoleOutputCharacter(PyObject *self, PyObject *args, PyObject *kwargs);
    static PyObject *PyReadConsoleOutputCharacter(PyObject *self, PyObject *args, PyObject *kwargs);
    static PyObject *PyReadConsoleOutputAttribute(PyObject *self, PyObject *args, PyObject *kwargs);
    static PyObject *PyWriteConsoleOutputCharacter(PyObject *self, PyObject *args, PyObject *kwargs);
    static PyObject *PyWriteConsoleOutputAttribute(PyObject *self, PyObject *args, PyObject *kwargs);
    static PyObject *PyScrollConsoleScreenBuffer(PyObject *self, PyObject *args, PyObject *kwargs);
    static PyObject *PyGetCurrentConsoleFont(PyObject *self, PyObject *args, PyObject *kwargs);
    static PyObject *PyGetConsoleFontSize(PyObject *self, PyObject *args, PyObject *kwargs);
    static PyObject *PySetConsoleFont(PyObject *self, PyObject *args, PyObject *kwargs);
    static PyObject *PySetStdHandle(PyObject *self, PyObject *args, PyObject *kwargs);
    static PyObject *PySetConsoleDisplayMode(PyObject *self, PyObject *args, PyObject *kwargs);
    static PyObject *PyWriteConsoleInput(PyObject *self, PyObject *args, PyObject *kwargs);
    static PyObject *PyReadConsoleInput(PyObject *self, PyObject *args, PyObject *kwargs);
    static PyObject *PyPeekConsoleInput(PyObject *self, PyObject *args, PyObject *kwargs);
    static PyObject *PyGetNumberOfConsoleInputEvents(PyObject *self, PyObject *args);
};

struct PyMethodDef PyConsoleScreenBuffer::methods[] = {
    //@pymeth Detach|Releases reference to handle without closing it
    {"Detach", PyHANDLE::Detach, METH_VARARGS, "Releases reference to handle without closing it"},
    //@pymeth Close|Closes the handle
    {"Close", PyHANDLE::Close, METH_VARARGS, "Closes the handle"},
    // @pymeth SetConsoleActiveScreenBuffer|Sets this handle as the currently display screen buffer
    {"SetConsoleActiveScreenBuffer", PyConsoleScreenBuffer::PySetConsoleActiveScreenBuffer, METH_VARARGS,
     "Sets this handle as the currently displayed screen buffer"},
    // @pymeth GetConsoleCursorInfo|Retrieves size and visibility of console's cursor
    {"GetConsoleCursorInfo", PyConsoleScreenBuffer::PyGetConsoleCursorInfo, METH_VARARGS,
     "Retrieves size and visibility of console's cursor"},
    // @pymeth SetConsoleCursorInfo|Sets the size and visibility of console's cursor
    {"SetConsoleCursorInfo", (PyCFunction)PyConsoleScreenBuffer::PySetConsoleCursorInfo, METH_VARARGS | METH_KEYWORDS,
     "Sets the size and visibility of console's cursor"},
    // @pymeth GetConsoleMode|Returns the input or output mode of the console buffer
    {"GetConsoleMode", PyConsoleScreenBuffer::PyGetConsoleMode, METH_VARARGS,
     "Returns the input or output mode of the console buffer"},
    // @pymeth SetConsoleMode|Sets the input or output mode of the console buffer
    {"SetConsoleMode", (PyCFunction)PyConsoleScreenBuffer::PySetConsoleMode, METH_VARARGS | METH_KEYWORDS,
     "Sets the input or output mode of the console buffer"},
    // @pymeth ReadConsole|Reads characters from the console input buffer
    {"ReadConsole", (PyCFunction)PyConsoleScreenBuffer::PyReadConsole, METH_VARARGS | METH_KEYWORDS,
     "Reads characters from the console input buffer"},
    // @pymeth WriteConsole|Writes characters at current cursor position
    {"WriteConsole", (PyCFunction)PyConsoleScreenBuffer::PyWriteConsole, METH_VARARGS | METH_KEYWORDS,
     "Writes characters at current cursor position"},
    // @pymeth FlushConsoleInputBuffer|Flush input buffer for console
    {"FlushConsoleInputBuffer", PyConsoleScreenBuffer::PyFlushConsoleInputBuffer, METH_VARARGS,
     "Flush input buffer for console"},
    // @pymeth SetConsoleTextAttribute|Sets character attributes for subsequent write operations
    {"SetConsoleTextAttribute", (PyCFunction)PyConsoleScreenBuffer::PySetConsoleTextAttribute,
     METH_VARARGS | METH_KEYWORDS, "Sets character attributes for subsequent write operations"},
    // @pymeth SetConsoleCursorPosition|Sets the console screen buffer's cursor position
    {"SetConsoleCursorPosition", (PyCFunction)PyConsoleScreenBuffer::PySetConsoleCursorPosition,
     METH_VARARGS | METH_KEYWORDS, "Sets the console screen buffer's cursor position"},
    // @pymeth SetConsoleScreenBufferSize|Sets the size of the console screen buffer
    {"SetConsoleScreenBufferSize", (PyCFunction)PyConsoleScreenBuffer::PySetConsoleScreenBufferSize,
     METH_VARARGS | METH_KEYWORDS, "Sets the size of the console screen buffer"},
    // @pymeth SetConsoleWindowInfo|Changes size and position of a console's window
    {"SetConsoleWindowInfo", (PyCFunction)PyConsoleScreenBuffer::PySetConsoleWindowInfo, METH_VARARGS | METH_KEYWORDS,
     "Changes size and position of a console's window"},
    // @pymeth GetConsoleScreenBufferInfo|Returns the state of the screen buffer
    {"GetConsoleScreenBufferInfo", PyConsoleScreenBuffer::PyGetConsoleScreenBufferInfo, METH_VARARGS,
     "Returns the state of the screen buffer"},
    // @pymeth GetLargestConsoleWindowSize|Returns the largest possible size for the console's window
    {"GetLargestConsoleWindowSize", PyConsoleScreenBuffer::PyGetLargestConsoleWindowSize, METH_VARARGS,
     "Returns the largest possible size for the console's window"},
    // @pymeth FillConsoleOutputAttribute|Set text attributes for a consecutive series of characters
    {"FillConsoleOutputAttribute", (PyCFunction)PyConsoleScreenBuffer::PyFillConsoleOutputAttribute,
     METH_VARARGS | METH_KEYWORDS, "Sets text attributes for a consecutive series of characters"},
    // @pymeth FillConsoleOutputCharacter|Sets consecutive character positions to a specified character
    {"FillConsoleOutputCharacter", (PyCFunction)PyConsoleScreenBuffer::PyFillConsoleOutputCharacter,
     METH_VARARGS | METH_KEYWORDS, "Sets consecutive character positions to a specified character"},
    // @pymeth ReadConsoleOutputCharacter|Reads consecutive characters from a starting position
    {"ReadConsoleOutputCharacter", (PyCFunction)PyConsoleScreenBuffer::PyReadConsoleOutputCharacter,
     METH_VARARGS | METH_KEYWORDS, "Reads consecutive characters from a starting position"},
    // @pymeth ReadConsoleOutputAttribute|Retrieves attributes from consecutive character cells
    {"ReadConsoleOutputAttribute", (PyCFunction)PyConsoleScreenBuffer::PyReadConsoleOutputAttribute,
     METH_VARARGS | METH_KEYWORDS, "Retrieves attributes from consecutive character cells"},
    // @pymeth WriteConsoleOutputCharacter|Writes a string of characters at a specified position
    {"WriteConsoleOutputCharacter", (PyCFunction)PyConsoleScreenBuffer::PyWriteConsoleOutputCharacter,
     METH_VARARGS | METH_KEYWORDS, "Writes a string of characters at a specified position"},
    // @pymeth WriteConsoleOutputAttribute|Sets the attributes of a range of character cells
    {"WriteConsoleOutputAttribute", (PyCFunction)PyConsoleScreenBuffer::PyWriteConsoleOutputAttribute,
     METH_VARARGS | METH_KEYWORDS, "Sets the attributes of a range of character cells"},
    // @pymeth ScrollConsoleScreenBuffer|Scrolls a region of the display
    {"ScrollConsoleScreenBuffer", (PyCFunction)PyConsoleScreenBuffer::PyScrollConsoleScreenBuffer,
     METH_VARARGS | METH_KEYWORDS, "Scrolls a region of the display"},
    // @pymeth GetCurrentConsoleFont|Returns the currently displayed font
    {"GetCurrentConsoleFont", (PyCFunction)PyConsoleScreenBuffer::PyGetCurrentConsoleFont, METH_VARARGS | METH_KEYWORDS,
     "Returns the currently displayed font"},
    // @pymeth GetConsoleFontSize|Returns size of specified font for the console
    {"GetConsoleFontSize", (PyCFunction)PyConsoleScreenBuffer::PyGetConsoleFontSize, METH_VARARGS | METH_KEYWORDS,
     "Returns size of specified font for the console"},
    // @pymeth SetConsoleFont|Changes the font used by the screen buffer
    {"SetConsoleFont", (PyCFunction)PyConsoleScreenBuffer::PySetConsoleFont, METH_VARARGS | METH_KEYWORDS,
     "Changes the font used by the screen buffer"},
    // @pymeth SetStdHandle|Replaces one of calling process's standard handles with this handle
    {"SetStdHandle", (PyCFunction)PyConsoleScreenBuffer::PySetStdHandle, METH_VARARGS | METH_KEYWORDS,
     "Replaces one of calling process's standard handles with this handle"},
    // @pymeth SetConsoleDisplayMode|Sets the display mode of the console buffer
    {"SetConsoleDisplayMode", (PyCFunction)PyConsoleScreenBuffer::PySetConsoleDisplayMode, METH_VARARGS | METH_KEYWORDS,
     "Sets the display mode of the console buffer"},
    // @pymeth WriteConsoleInput|Places input records in the console's input queue
    {"WriteConsoleInput", (PyCFunction)PyConsoleScreenBuffer::PyWriteConsoleInput, METH_VARARGS | METH_KEYWORDS,
     "Places input records in the console's input queue"},
    // @pymeth ReadConsoleInput|Reads input records and removes them from the input queue
    {"ReadConsoleInput", (PyCFunction)PyConsoleScreenBuffer::PyReadConsoleInput, METH_VARARGS | METH_KEYWORDS,
     "Reads input records and removes them from the input queue"},
    // @pymeth PeekConsoleInput|Returns pending input records without removing them from the input queue
    {"PeekConsoleInput", (PyCFunction)PyConsoleScreenBuffer::PyPeekConsoleInput, METH_VARARGS | METH_KEYWORDS,
     "Returns pending input records without removing them from the input queue"},
    // @pymeth GetNumberOfConsoleInputEvents|Returns the number of unread records in the input queue
    {"GetNumberOfConsoleInputEvents", PyConsoleScreenBuffer::PyGetNumberOfConsoleInputEvents, METH_VARARGS,
     "Returns the number of unread records in the input queue"},
    {NULL}};

// @pymethod |PyConsoleScreenBuffer|SetConsoleActiveScreenBuffer|Sets this handle as the currently displayed screen
// buffer
PyObject *PyConsoleScreenBuffer::PySetConsoleActiveScreenBuffer(PyObject *self, PyObject *args)
{
    if (!PyArg_ParseTuple(args, ":SetConsoleActiveScreenBuffer"))
        return NULL;
    if (!SetConsoleActiveScreenBuffer(((PyConsoleScreenBuffer *)self)->m_handle))
        return PyWin_SetAPIError("SetConsoleActiveScreenBuffer");
    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod (Size, bVisible)|PyConsoleScreenBuffer|GetConsoleCursorInfo|Retrieves size and visibility of console's
// cursor
// @rdesc Returns the size of the console's cursor expressed as a percentage of character size, and a boolen indicating
// if cursor is visible
PyObject *PyConsoleScreenBuffer::PyGetConsoleCursorInfo(PyObject *self, PyObject *args)
{
    CONSOLE_CURSOR_INFO cci;
    if (!PyArg_ParseTuple(args, ":GetConsoleCursorInfo"))
        return NULL;
    if (!GetConsoleCursorInfo(((PyConsoleScreenBuffer *)self)->m_handle, &cci))
        return PyWin_SetAPIError("GetConsoleCursorInfo");
    return Py_BuildValue("ll", cci.dwSize, cci.bVisible);
}

// @pymethod |PyConsoleScreenBuffer|SetConsoleCursorInfo|Sets the size and visibility of console's cursor
PyObject *PyConsoleScreenBuffer::PySetConsoleCursorInfo(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"Size", "Visible", NULL};
    CONSOLE_CURSOR_INFO cci;
    if (!PyArg_ParseTupleAndKeywords(
            args, kwargs, "kk:SetConsoleCursorInfo", keywords,
            &cci.dwSize,     // @pyparm int|Size||Percentage of character size that cursor will occupy
            &cci.bVisible))  // @pyparm boolen|Visible||Determines if cursor is visible
        return NULL;
    if (!SetConsoleCursorInfo(((PyConsoleScreenBuffer *)self)->m_handle, &cci))
        return PyWin_SetAPIError("SetConsoleCursorInfo");
    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod int|PyConsoleScreenBuffer|GetConsoleMode|Returns the input or output mode of the console buffer
// @rdesc Returns a combination of ENABLE_*_INPUT or ENABLE_*_OUTPUT constants
PyObject *PyConsoleScreenBuffer::PyGetConsoleMode(PyObject *self, PyObject *args)
{
    if (!PyArg_ParseTuple(args, ":GetConsoleMode"))
        return NULL;
    DWORD mode;
    if (!GetConsoleMode(((PyConsoleScreenBuffer *)self)->m_handle, &mode))
        return PyWin_SetAPIError("GetConsoleMode");
    return PyLong_FromLong(mode);
}

// @pymethod |PyConsoleScreenBuffer|SetConsoleMode|Sets the input or output mode of the console buffer
PyObject *PyConsoleScreenBuffer::PySetConsoleMode(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"Mode", NULL};
    DWORD mode;
    if (!PyArg_ParseTupleAndKeywords(
            args, kwargs, "k:SetConsoleMode", keywords,
            &mode))  // @pyparm int|Mode||Combination of ENABLE_*_INPUT or ENABLE_*_OUTPUT constants
        return NULL;

    if (!SetConsoleMode(((PyConsoleScreenBuffer *)self)->m_handle, mode))
        return PyWin_SetAPIError("SetConsoleMode");
    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod <o PyUNICODE>|PyConsoleScreenBuffer|ReadConsole|Reads characters from the console input buffer
PyObject *PyConsoleScreenBuffer::PyReadConsole(PyObject *self, PyObject *args, PyObject *kwargs)
{
    PyObject *ret = NULL;
    WCHAR *buf = NULL;
    LPVOID reserved = NULL;
    DWORD nbrtoread, nbrread;
    static char *keywords[] = {"NumberOfCharsToRead", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "l:ReadConsole", keywords,
                                     &nbrtoread))  // @pyparm int|NumberOfCharsToRead||Characters to read
        return NULL;
    buf = (WCHAR *)malloc(nbrtoread * sizeof(WCHAR));
    if (buf == NULL)
        return PyErr_Format(PyExc_MemoryError, "ReadConsole: Unable to allocate buffer of %d bytes",
                            nbrtoread * sizeof(WCHAR));
    if (!ReadConsole(((PyConsoleScreenBuffer *)self)->m_handle, (LPVOID)buf, nbrtoread, &nbrread,
                     (PCONSOLE_READCONSOLE_CONTROL)reserved))
        PyWin_SetAPIError("ReadConsole");
    else
        ret = PyWinObject_FromWCHAR(buf, nbrread);
    free(buf);
    return ret;
}

// @pymethod int|PyConsoleScreenBuffer|WriteConsole|Writes characters at current cursor position
// @rdesc Returns the number of characters written
PyObject *PyConsoleScreenBuffer::PyWriteConsole(PyObject *self, PyObject *args, PyObject *kwargs)
{
    WCHAR *buf = NULL;
    PyObject *obbuf, *ret = NULL;
    LPVOID reserved = NULL;
    DWORD nbrtowrite, nbrwritten;
    static char *keywords[] = {"Buffer", NULL};
    if (!PyArg_ParseTupleAndKeywords(
            args, kwargs, "O:WriteConsole", keywords,
            &obbuf))  // @pyparm <o PyUNICODE>|Buffer||String or Unicode to be written to console
        return NULL;
    if (!PyWinObject_AsWCHAR(obbuf, &buf, FALSE, &nbrtowrite))
        return NULL;

    if (!WriteConsole(((PyConsoleScreenBuffer *)self)->m_handle, (LPVOID)buf, nbrtowrite, &nbrwritten, reserved))
        PyWin_SetAPIError("WriteConsole");
    else
        ret = PyLong_FromLong(nbrwritten);
    PyWinObject_FreeWCHAR(buf);
    return ret;
}

// @pymethod |PyConsoleScreenBuffer|FlushConsoleInputBuffer|Flush input buffer
PyObject *PyConsoleScreenBuffer::PyFlushConsoleInputBuffer(PyObject *self, PyObject *args)
{
    if (!PyArg_ParseTuple(args, ":FlushConsoleInputBuffer"))
        return NULL;
    if (!FlushConsoleInputBuffer(((PyConsoleScreenBuffer *)self)->m_handle))
        return PyWin_SetAPIError("FlushConsoleInputBuffer");
    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod |PyConsoleScreenBuffer|SetConsoleTextAttribute|Sets character attributes for subsequent write operations
PyObject *PyConsoleScreenBuffer::PySetConsoleTextAttribute(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"Attributes", NULL};
    WORD Attributes;
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "H:SetConsoleTextAttribute", keywords,
                                     &Attributes))  // @pyparm int|Attributes||Attributes to be set, combination of
                                                    // FOREGROUND_*, BACKGROUND_*, and COMMON_LVB_* constants
        return NULL;
    if (!SetConsoleTextAttribute(((PyConsoleScreenBuffer *)self)->m_handle, Attributes))
        return PyWin_SetAPIError("SetConsoleTextAttribute");
    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod |PyConsoleScreenBuffer|SetConsoleCursorPosition|Sets the console screen buffer's cursor position
PyObject *PyConsoleScreenBuffer::PySetConsoleCursorPosition(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"CursorPosition", NULL};
    PyObject *obcoord;
    PCOORD pcoord;
    if (!PyArg_ParseTupleAndKeywords(
            args, kwargs, "O:SetConsoleCursorPosition", keywords,
            &obcoord))  // @pyparm <o PyCOORD>|CursorPosition||A PyCOORD containing the new cursor position
        return NULL;
    if (!PyWinObject_AsCOORD(obcoord, &pcoord, FALSE))
        return NULL;
    if (!SetConsoleCursorPosition(((PyConsoleScreenBuffer *)self)->m_handle, *pcoord))
        return PyWin_SetAPIError("SetConsoleCursorPosition");
    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod |PyConsoleScreenBuffer|SetConsoleScreenBufferSize|Sets the size of the console screen buffer
PyObject *PyConsoleScreenBuffer::PySetConsoleScreenBufferSize(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"Size", NULL};
    PCOORD pcoord;
    PyObject *obcoord;
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O:SetConsoleScreenBufferSize", keywords,
                                     &obcoord))  // @pyparm <o PyCOORD>|Size||COORD object containing the new dimensions
        return NULL;
    if (!PyWinObject_AsCOORD(obcoord, &pcoord, FALSE))
        return NULL;
    if (!SetConsoleScreenBufferSize(((PyConsoleScreenBuffer *)self)->m_handle, *pcoord))
        return PyWin_SetAPIError("SetConsoleScreenBufferSize");
    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod |PyConsoleScreenBuffer|SetConsoleWindowInfo|Changes size and position of a console's window
PyObject *PyConsoleScreenBuffer::PySetConsoleWindowInfo(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"Absolute", "ConsoleWindow", NULL};
    BOOL absolut;
    PyObject *obrect;
    PSMALL_RECT prect;
    if (!PyArg_ParseTupleAndKeywords(
            args, kwargs, "lO:SetConsoleWindowInfo", keywords,
            &absolut,  // @pyparm boolean|Absolute||If False, coordinates are relative to current position
            &obrect))  // @pyparm <o PySMALL_RECT>|ConsoleWindow||A SMALL_RECT containing the new window coordinates
        return NULL;
    if (!PyWinObject_AsSMALL_RECT(obrect, &prect, FALSE))
        return NULL;
    if (!SetConsoleWindowInfo(((PyConsoleScreenBuffer *)self)->m_handle, absolut, prect))
        return PyWin_SetAPIError("SetConsoleWindowInfo");
    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod dict|PyConsoleScreenBuffer|GetConsoleScreenBufferInfo|Returns the state of the screen buffer
PyObject *PyConsoleScreenBuffer::PyGetConsoleScreenBufferInfo(PyObject *self, PyObject *args)
{
    CONSOLE_SCREEN_BUFFER_INFO info;
    if (!PyArg_ParseTuple(args, ":GetConsoleScreenBufferInfo"))
        return NULL;
    if (!GetConsoleScreenBufferInfo(((PyConsoleScreenBuffer *)self)->m_handle, &info))
        return PyWin_SetAPIError("GetConsoleScreenBufferInfo");
    return Py_BuildValue("{s:N,s:N,s:H,s:N,s:N}", "Size", PyWinObject_FromCOORD(&info.dwSize), "CursorPosition",
                         PyWinObject_FromCOORD(&info.dwCursorPosition), "Attributes", info.wAttributes, "Window",
                         PyWinObject_FromSMALL_RECT(&info.srWindow), "MaximumWindowSize",
                         PyWinObject_FromCOORD(&info.dwMaximumWindowSize));
}

// @pymethod <o PyCOORD>|PyConsoleScreenBuffer|GetLargestConsoleWindowSize|Returns the largest possible size for the
// console's window
PyObject *PyConsoleScreenBuffer::PyGetLargestConsoleWindowSize(PyObject *self, PyObject *args)
{
    if (!PyArg_ParseTuple(args, ":GetLargestConsoleWindowSize"))
        return NULL;
    COORD coord;
    coord = ::GetLargestConsoleWindowSize(((PyConsoleScreenBuffer *)self)->m_handle);
    return PyWinObject_FromCOORD(&coord);
}

// @pymethod int|PyConsoleScreenBuffer|FillConsoleOutputAttribute|Set text attributes for a consecutive series of
// characters
// @rdesc Returns the number of character cells whose attributes were set
PyObject *PyConsoleScreenBuffer::PyFillConsoleOutputAttribute(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"Attribute", "Length", "WriteCoord", NULL};
    DWORD len, nbrwritten;
    PyObject *obcoord;
    PCOORD pcoord;
    WORD attr;
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "HlO:FillConsoleOutputAttribute", keywords,
                                     &attr,      // @pyparm int|Attribute||Text attributes to be set, combination of
                                                 // FOREGROUND_*, BACKGROUND_*, and COMMON_LVB_* constants
                                     &len,       // @pyparm int|Length||The number of characters to set
                                     &obcoord))  // @pyparm <o PyCOORD>|WriteCoord||The screen position to begin at
        return NULL;
    if (!PyWinObject_AsCOORD(obcoord, &pcoord, FALSE))
        return NULL;
    if (!FillConsoleOutputAttribute(((PyConsoleScreenBuffer *)self)->m_handle, attr, len, *pcoord, &nbrwritten))
        return PyWin_SetAPIError("FillConsoleOutputAttribute");
    return PyLong_FromLong(nbrwritten);
}

// @pymethod int|PyConsoleScreenBuffer|FillConsoleOutputCharacter|Sets consecutive character positions to a specified
// character
// @rdesc Returns the number of characters actually written
PyObject *PyConsoleScreenBuffer::PyFillConsoleOutputCharacter(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"Character", "Length", "WriteCoord", NULL};
    DWORD len, nbrwritten;
    PyObject *obfillchar, *obcoord;
    WCHAR fillchar;
    PCOORD pcoord;
    if (!PyArg_ParseTupleAndKeywords(
            args, kwargs, "OlO:FillConsoleOutputCharacter", keywords,
            &obfillchar,  // @pyparm <o PyUNICODE>|Character||A single character to be used to fill the specified range
            &len,         // @pyparm int|Length||The number of characters positions to fill
            &obcoord))    // @pyparm <o PyCOORD>|WriteCoord||The screen position to begin at
        return NULL;
    if (!PyWinObject_AsCOORD(obcoord, &pcoord, FALSE))
        return NULL;
    if (!PyWinObject_AsSingleWCHAR(obfillchar, &fillchar))
        return NULL;
    if (!FillConsoleOutputCharacter(((PyConsoleScreenBuffer *)self)->m_handle, fillchar, len, *pcoord, &nbrwritten))
        return PyWin_SetAPIError("FillConsoleOutputCharacter");
    return PyLong_FromLong(nbrwritten);
}

// @pymethod <o PyUnicode>|PyConsoleScreenBuffer|ReadConsoleOutputCharacter|Reads consecutive characters from a starting
// position
PyObject *PyConsoleScreenBuffer::PyReadConsoleOutputCharacter(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"Length", "ReadCoord", NULL};
    DWORD len, nbrread;
    PyObject *obcoord, *ret = NULL;
    WCHAR *buf = NULL;
    PCOORD pcoord;
    if (!PyArg_ParseTupleAndKeywords(
            args, kwargs, "lO:ReadConsoleOutputCharacter", keywords,
            &len,       // @pyparm int|Length||The number of characters positions to read
            &obcoord))  // @pyparm <o PyCOORD>|ReadCoord||The screen position start reading from
        return NULL;
    if (!PyWinObject_AsCOORD(obcoord, &pcoord, FALSE))
        return NULL;
    buf = (WCHAR *)malloc(len * sizeof(WCHAR));
    if (buf == NULL)
        return PyErr_Format(PyExc_MemoryError, "Unable to unicode buffer of %d characters", len);
    if (!ReadConsoleOutputCharacter(((PyConsoleScreenBuffer *)self)->m_handle, buf, len, *pcoord, &nbrread))
        PyWin_SetAPIError("ReadConsoleOutputCharacter");
    else
        ret = PyWinObject_FromWCHAR(buf, nbrread);
    free(buf);
    return ret;
}

// @pymethod (int,...)|PyConsoleScreenBuffer|ReadConsoleOutputAttribute|Retrieves attributes from consecutive character
// cells
// @rdesc Returns a sequence of ints containing the attributes of a range of characters
PyObject *PyConsoleScreenBuffer::PyReadConsoleOutputAttribute(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"Length", "ReadCoord", NULL};
    DWORD len, nbrread, tuple_ind;
    PyObject *obcoord, *ret = NULL, *ret_item;
    WORD *buf = NULL;
    PCOORD pcoord;
    if (!PyArg_ParseTupleAndKeywords(
            args, kwargs, "lO:ReadConsoleOutputAttribute", keywords,
            &len,       // @pyparm int|Length||The number of attributes to read
            &obcoord))  // @pyparm <o PyCOORD>|ReadCoord||The screen position from which to start reading
        return NULL;
    if (!PyWinObject_AsCOORD(obcoord, &pcoord, FALSE))
        return NULL;
    buf = (WORD *)malloc(len * sizeof(WORD));
    if (buf == NULL)
        return PyErr_Format(PyExc_MemoryError, " Unable to allocate array of %d WORDs", len);
    if (!ReadConsoleOutputAttribute(((PyConsoleScreenBuffer *)self)->m_handle, buf, len, *pcoord, &nbrread))
        PyWin_SetAPIError("ReadConsoleOutputAttribute");
    else {
        ret = PyTuple_New(nbrread);
        if (ret != NULL)
            for (tuple_ind = 0; tuple_ind < nbrread; tuple_ind++) {
                ret_item = PyLong_FromLong(buf[tuple_ind]);
                if (ret_item == NULL) {
                    Py_DECREF(ret);
                    ret = NULL;
                    break;
                }
                PyTuple_SET_ITEM(ret, tuple_ind, ret_item);
            }
    }
    free(buf);
    return ret;
}

// @pymethod int|PyConsoleScreenBuffer|WriteConsoleOutputCharacter|Writes a string of characters at a specified position
// @rdesc Returns the number of characters actually written
PyObject *PyConsoleScreenBuffer::PyWriteConsoleOutputCharacter(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"Characters", "WriteCoord", NULL};
    DWORD buflen, nbrwritten;
    PyObject *obbuf, *obcoord, *ret = NULL;
    WCHAR *buf = NULL;
    PCOORD pcoord;
    if (!PyArg_ParseTupleAndKeywords(
            args, kwargs, "OO:WriteConsoleOutputCharacter", keywords,
            &obbuf,     // @pyparm <o PyUNICODE>|Characters||Characters to be written
            &obcoord))  // @pyparm <o PyCOORD>|WriteCoord||The screen position at which to start writing
        return NULL;
    if (!PyWinObject_AsCOORD(obcoord, &pcoord, FALSE))
        return NULL;
    if (!PyWinObject_AsWCHAR(obbuf, &buf, FALSE, &buflen))
        return NULL;

    if (!WriteConsoleOutputCharacter(((PyConsoleScreenBuffer *)self)->m_handle, buf, buflen, *pcoord, &nbrwritten))
        PyWin_SetAPIError("WriteConsoleOutputCharacter");
    else
        ret = PyLong_FromLong(nbrwritten);
    PyWinObject_FreeWCHAR(buf);
    return ret;
}

// @pymethod int|PyConsoleScreenBuffer|WriteConsoleOutputAttribute|Sets the attributes of a range of character cells
// @rdesc Returns the number of attributes set
PyObject *PyConsoleScreenBuffer::PyWriteConsoleOutputAttribute(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"Attributes", "WriteCoord", NULL};
    WORD *attributes = NULL;
    DWORD attributecnt, nbrwritten;
    PyObject *obattributes, *obcoord, *ret = NULL;
    PCOORD pcoord;
    if (!PyArg_ParseTupleAndKeywords(
            args, kwargs, "OO:WriteConsoleOutputAttribute", keywords,
            &obattributes,  // @pyparm (int,...)|Attributes||A sequence of ints containing the attributes to be set
            &obcoord))      // @pyparm <o PyCOORD>|WriteCoord||The screen position at which to start writing
        return NULL;
    if (!PyWinObject_AsCOORD(obcoord, &pcoord, FALSE))
        return NULL;
    if (!PyWinObject_AsUSHORTArray(obattributes, &attributes, &attributecnt, FALSE))
        return NULL;
    if (!WriteConsoleOutputAttribute(((PyConsoleScreenBuffer *)self)->m_handle, attributes, attributecnt, *pcoord,
                                     &nbrwritten))
        PyWin_SetAPIError("WriteConsoleOutputAttribute");
    else
        ret = PyLong_FromLong(nbrwritten);
    free(attributes);
    return ret;
}

// @pymethod |PyConsoleScreenBuffer|ScrollConsoleScreenBuffer|Scrolls a region of the display
PyObject *PyConsoleScreenBuffer::PyScrollConsoleScreenBuffer(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"ScrollRectangle", "ClipRectangle", "DestinationOrigin",
                               "FillCharacter",   "FillAttribute", NULL};
    PyObject *obscrollrect, *obcliprect, *obdestcoord, *obfillchar;
    PSMALL_RECT pscrollrect, pcliprect;
    PCOORD pdestcoord;
    CHAR_INFO char_info;

    if (!PyArg_ParseTupleAndKeywords(
            args, kwargs, "OOOOH:ScrollConsoleScreenBuffer", keywords,
            &obscrollrect,  // @pyparm <o PySMALL_RECT>|ScrollRectangle||The region to be scrolled
            &obcliprect,  // @pyparm <o PySMALL_RECT>|ClipRectangle||Rectangle that limits display area affected, can be
                          // None
            &obdestcoord,  // @pyparm <o PyCOORD>|DestinationOrigin||The position to which ScrollRectangle will be moved
            &obfillchar,   // @pyparm <o PyUNICODE>|FillCharacter||Character to fill in the area left blank by scrolling
                           // operation
            &char_info.Attributes))  // @pyparm int|FillAttribute||Text attributes to apply to FillCharacter
        return NULL;
    if (PyWinObject_AsSMALL_RECT(obscrollrect, &pscrollrect, FALSE) &&
        PyWinObject_AsSMALL_RECT(obcliprect, &pcliprect, TRUE) &&
        PyWinObject_AsCOORD(obdestcoord, &pdestcoord, FALSE) &&
        PyWinObject_AsSingleWCHAR(obfillchar, &char_info.Char.UnicodeChar)) {
        if (!ScrollConsoleScreenBuffer(((PyConsoleScreenBuffer *)self)->m_handle, pscrollrect, pcliprect, *pdestcoord,
                                       &char_info)) {
            PyWin_SetAPIError("ScrollConsoleScreenBuffer");
        }
        else {
            Py_INCREF(Py_None);
            return Py_None;
        }
    }
    return NULL;
}

// @pymethod (int, <o PyCOORD>)|PyConsoleScreenBuffer|GetCurrentConsoleFont|Returns currently displayed font
// @rdesc Returns the index of current font and window size
// @comm Only exists on XP or later.<nl>
// MSDN docs claim the returned COORD is the font size, but it's actually the window size.<nl>
// Use <om PyConsoleScreenBuffer.GetConsoleFontSize> for the font size.
PyObject *PyConsoleScreenBuffer::PyGetCurrentConsoleFont(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"MaximumWindow", NULL};
    CONSOLE_FONT_INFO cfi;
    BOOL bmax = FALSE;
    CHECK_PFN(GetCurrentConsoleFont);
    if (!PyArg_ParseTupleAndKeywords(
            args, kwargs, "|l:GetCurrentConsoleFont", keywords,
            &bmax))  // @pyparm boolean|MaximumWindow|False|If True, retrieves font size for maximum window size
        return NULL;
    if (!(*pfnGetCurrentConsoleFont)(((PyConsoleScreenBuffer *)self)->m_handle, bmax, &cfi))
        return PyWin_SetAPIError("GetCurrentConsoleFont");
    return Py_BuildValue("lO", cfi.nFont, PyWinObject_FromCOORD(&cfi.dwFontSize));
}

// @pymethod <o PyCOORD>|PyConsoleScreenBuffer|GetConsoleFontSize|Returns size of specified font for the console
// @comm Only exists on XP or later.
PyObject *PyConsoleScreenBuffer::PyGetConsoleFontSize(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"Font", NULL};
    DWORD font;
    COORD fontsize;
    CHECK_PFN(GetConsoleFontSize);
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "l:GetConsoleFontSize", keywords,
                                     &font))  // @pyparm int|Font||Index of font as returned by GetCurrentConsoleFont
        return NULL;
    fontsize = (*pfnGetConsoleFontSize)(((PyConsoleScreenBuffer *)self)->m_handle, font);
    if (fontsize.X == 0 && fontsize.Y == 0)
        return PyWin_SetAPIError("GetConsoleFontSize");
    return PyWinObject_FromCOORD(&fontsize);
}

// @pymethod |PyConsoleScreenBuffer|SetConsoleFont|Changes the font used by the screen buffer
// @comm Function is not documented on MSDN
PyObject *PyConsoleScreenBuffer::PySetConsoleFont(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"Font", NULL};
    DWORD font;
    CHECK_PFN(SetConsoleFont);
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "l:SetConsoleFont", keywords,
                                     &font))  // @pyparm int|Font||The number of the font to be set
        return NULL;
    if (!(*pfnSetConsoleFont)(((PyConsoleScreenBuffer *)self)->m_handle, font))
        return PyWin_SetAPIError("SetConsoleFont");
    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod |PyConsoleScreenBuffer|SetStdHandle|Replaces one of calling process's standard handles with this handle
PyObject *PyConsoleScreenBuffer::PySetStdHandle(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"StdHandle", NULL};
    DWORD StdHandle;
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "k:SetStdHandle", keywords,
                                     &StdHandle))  // @pyparm int|StdHandle||Specifies handle to be replaced -
                                                   // STD_INPUT_HANDLE, STD_OUTPUT_HANDLE, or STD_ERROR_HANDLE
        return NULL;
    if (!SetStdHandle(StdHandle, ((PyConsoleScreenBuffer *)self)->m_handle))
        return PyWin_SetAPIError("SetStdHandle");
    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod |PyConsoleScreenBuffer|SetConsoleDisplayMode|Sets the display mode of the console buffer
PyObject *PyConsoleScreenBuffer::PySetConsoleDisplayMode(PyObject *self, PyObject *args, PyObject *kwargs)
{
    CHECK_PFN(SetConsoleDisplayMode);
    static char *keywords[] = {"Flags", "NewScreenBufferDimensions", NULL};
    DWORD flags;
    PCOORD dim;
    PyObject *obdim;
    if (!PyArg_ParseTupleAndKeywords(
            args, kwargs, "kO:SetConsoleDisplayMode", keywords,
            &flags,   // @pyparm int|Flags||CONSOLE_FULLSCREEN_MODE or CONSOLE_WINDOWED_MODE
            &obdim))  // @pyparm <o PyCOORD>|NewScreenBufferDimensions||New size of the screen buffer in characters
        return NULL;
    if (!PyWinObject_AsCOORD(obdim, &dim, FALSE))
        return NULL;
    if (!(*pfnSetConsoleDisplayMode)(((PyConsoleScreenBuffer *)self)->m_handle, flags, dim))
        return PyWin_SetAPIError("SetConsoleDisplayMode");
    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod int|PyConsoleScreenBuffer|WriteConsoleInput|Places input records in the console's input queue
// @rdesc Returns the number of records written
PyObject *PyConsoleScreenBuffer::PyWriteConsoleInput(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"Buffer", NULL};
    INPUT_RECORD *pinput_records = NULL, *pinput_record;
    DWORD nbrofrecords, nbrwritten, tuple_index;
    PyObject *obbuf, *obtuple = NULL, *obinput_record, *ret = NULL;
    if (!PyArg_ParseTupleAndKeywords(
            args, kwargs, "O:WriteConsoleInput", keywords,
            &obbuf))  // @pyparm (<o PyINPUT_RECORD>,...)|Buffer||A sequence of <o PyINPUT_RECORD> objects
        return NULL;
    obtuple = PyWinSequence_Tuple(obbuf, &nbrofrecords);
    if (obtuple == NULL)
        return NULL;
    pinput_records = (INPUT_RECORD *)malloc(nbrofrecords * sizeof(INPUT_RECORD));
    if (pinput_records == NULL) {
        PyErr_Format(PyExc_MemoryError, "Unable to allocate %d bytes", nbrofrecords * sizeof(INPUT_RECORD));
        goto done;
    }
    for (tuple_index = 0; tuple_index < nbrofrecords; tuple_index++) {
        obinput_record = PyTuple_GET_ITEM(obtuple, tuple_index);
        if (!PyWinObject_AsINPUT_RECORD(obinput_record, &pinput_record))
            goto done;
        pinput_records[tuple_index] = *pinput_record;
    }
    if (!WriteConsoleInput(((PyConsoleScreenBuffer *)self)->m_handle, pinput_records, nbrofrecords, &nbrwritten))
        PyWin_SetAPIError("WriteConsoleInput");
    else
        ret = PyLong_FromUnsignedLong(nbrwritten);
done:
    if (pinput_records != NULL)
        free(pinput_records);
    Py_XDECREF(obtuple);
    return ret;
}

// @pymethod (<o PyINPUT_RECORD>,...)|PyConsoleScreenBuffer|ReadConsoleInput|Reads input records and removes them from
// the input queue
// @rdesc Returns a sequence of <o PyINPUT_RECORD> objects
// @comm This functions blocks until at least one record is read.<nl>
// The number of records returned may be less than the nbr requested
PyObject *PyConsoleScreenBuffer::PyReadConsoleInput(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"Length", NULL};
    INPUT_RECORD *pinput_records = NULL;
    DWORD nbrofrecords, nbrread, tuple_index;
    PyObject *ret = NULL, *ret_item;
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "k:ReadConsoleInput", keywords,
                                     &nbrofrecords))  // @pyparm int|Length||The number of input records to read
        return NULL;
    pinput_records = (INPUT_RECORD *)malloc(nbrofrecords * sizeof(INPUT_RECORD));
    if (pinput_records == NULL)
        return PyErr_Format(PyExc_MemoryError, "Unable to allocate %d bytes", nbrofrecords * sizeof(INPUT_RECORD));
    if (!ReadConsoleInput(((PyConsoleScreenBuffer *)self)->m_handle, pinput_records, nbrofrecords, &nbrread))
        PyWin_SetAPIError("ReadConsoleInput");
    else {
        ret = PyTuple_New(nbrread);
        if (ret != NULL)
            for (tuple_index = 0; tuple_index < nbrread; tuple_index++) {
                ret_item = PyWinObject_FromINPUT_RECORD(&pinput_records[tuple_index]);
                if (ret_item == NULL) {
                    Py_DECREF(ret);
                    ret = NULL;
                    break;
                }
                PyTuple_SET_ITEM(ret, tuple_index, ret_item);
            }
    }
    free(pinput_records);
    return ret;
}

// @pymethod (<o PyINPUT_RECORD>,...)|PyConsoleScreenBuffer|PeekConsoleInput|Returns pending input records without
// removing them from the input queue
// @rdesc Returns a sequence of <o PyINPUT_RECORD> objects
// @comm This function does not block as ReadConsoleInput does.<nl>
//	The number of records returned may be less than the nbr requested
PyObject *PyConsoleScreenBuffer::PyPeekConsoleInput(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"Length", NULL};
    INPUT_RECORD *pinput_records = NULL;
    DWORD nbrofrecords, nbrread, tuple_index;
    PyObject *ret = NULL, *ret_item;
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "k:PeekConsoleInput", keywords,
                                     &nbrofrecords))  // @pyparm int|Length||The number of input records to read
        return NULL;
    pinput_records = (INPUT_RECORD *)malloc(nbrofrecords * sizeof(INPUT_RECORD));
    if (pinput_records == NULL)
        return PyErr_Format(PyExc_MemoryError, "Unable to allocate %d bytes", nbrofrecords * sizeof(INPUT_RECORD));
    if (!PeekConsoleInput(((PyConsoleScreenBuffer *)self)->m_handle, pinput_records, nbrofrecords, &nbrread))
        PyWin_SetAPIError("PeekConsoleInput");
    else {
        ret = PyTuple_New(nbrread);
        if (ret != NULL)
            for (tuple_index = 0; tuple_index < nbrread; tuple_index++) {
                ret_item = PyWinObject_FromINPUT_RECORD(&pinput_records[tuple_index]);
                if (ret_item == NULL) {
                    Py_DECREF(ret);
                    ret = NULL;
                    break;
                }
                PyTuple_SET_ITEM(ret, tuple_index, ret_item);
            }
    }
    free(pinput_records);
    return ret;
}

// @pymethod int|PyConsoleScreenBuffer|GetNumberOfConsoleInputEvents|Returns the number of unread records in the input
// queue
PyObject *PyConsoleScreenBuffer::PyGetNumberOfConsoleInputEvents(PyObject *self, PyObject *args)
{
    if (!PyArg_ParseTuple(args, ":GetNumberOfConsoleInputEvents"))
        return NULL;
    DWORD nbrofevents;
    if (!GetNumberOfConsoleInputEvents(((PyConsoleScreenBuffer *)self)->m_handle, &nbrofevents))
        return PyWin_SetAPIError("GetNumberOfConsoleInputEvents");
    return PyLong_FromUnsignedLong(nbrofevents);
}

PyTypeObject PyConsoleScreenBufferType = {
    PYWIN_OBJECT_HEAD "PyConsoleScreenBuffer",
    sizeof(PyConsoleScreenBuffer),
    0,
    PyConsoleScreenBuffer::tp_dealloc,  // tp_dealloc
    0,                                  // tp_print
    0,                                  // tp_getattr
    0,                                  // tp_setattr
    0,                                  // tp_compare
    PyHANDLE::strFunc,                  // tp_repr
    PyHANDLEType.tp_as_number,          // tp_as_number
    0,                                  // tp_as_sequence
    0,                                  // tp_as_mapping
    0,                                  // tp_hash
    0,                                  // tp_call
    PyHANDLE::strFunc,                  // tp_str
    PyObject_GenericGetAttr,            // tp_getattro
    PyObject_GenericSetAttr,            // tp_setattro
    0,                                  // tp_as_buffer
    Py_TPFLAGS_DEFAULT,                 // tp_flags
    "Handle to a console screen buffer.\nCreate using CreateConsoleScreenBuffer or "
    "PyConsoleScreenBufferType(Handle)",  // tp_doc
    0,                                    // tp_traverse
    0,                                    // tp_clear
    0,                                    // tp_richcompare
    0,                                    // tp_weaklistoffset
    0,                                    // tp_iter
    0,                                    // tp_iternext
    PyConsoleScreenBuffer::methods,       // tp_methods
    0,                                    //	PyConsoleScreenBuffer::members,		// tp_members
    0,                                    // tp_getset
    0,                                    // tp_base
    0,                                    // tp_dict
    0,                                    // tp_descr_get
    0,                                    // tp_descr_set
    0,                                    // tp_dictoffset
    0,                                    // tp_init
    0,                                    // tp_alloc
    PyConsoleScreenBuffer::tp_new         // tp_new
};

#define PyConsoleScreenBuffer_Check(ob) ((ob)->ob_type == &PyConsoleScreenBufferType)

PyObject *PyWinObject_FromConsoleScreenBuffer(HANDLE h, BOOL bDuplicate)
{
    HANDLE hdup;
    if (!bDuplicate)
        hdup = h;
    else {
        HANDLE hprocess = GetCurrentProcess();
        if (!DuplicateHandle(hprocess, h, hprocess, &hdup, 0, FALSE, DUPLICATE_SAME_ACCESS))
            return PyWin_SetAPIError("DuplicateHandle");
    }
    PyObject *ret = new PyConsoleScreenBuffer(hdup);
    if (ret == NULL)
        PyErr_NoMemory();
    return ret;
}

PyObject *PyConsoleScreenBuffer::tp_new(PyTypeObject *tp, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"Handle", NULL};
    HANDLE h;
    PyObject *obh;
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O", keywords, &obh))
        return NULL;
    if (!PyWinObject_AsHANDLE(obh, &h))
        return NULL;
    // Handle will be duplicated so caller is still responsible for original handle
    return PyWinObject_FromConsoleScreenBuffer(h, TRUE);
}

PyConsoleScreenBuffer::PyConsoleScreenBuffer(HANDLE hconsole) : PyHANDLE(hconsole)
{
    ob_type = &PyConsoleScreenBufferType;
}

PyConsoleScreenBuffer::~PyConsoleScreenBuffer(void)
{
    // Close happens in tp_dealloc below
}

void PyConsoleScreenBuffer::tp_dealloc(PyObject *ob)
{
    // use same error logic as in PyHANDLE::deallocFunc
    PyObject *typ, *val, *tb;
    PyErr_Fetch(&typ, &val, &tb);
    ((PyConsoleScreenBuffer *)ob)->Close();
    delete (PyConsoleScreenBuffer *)ob;
    PyErr_Restore(typ, val, tb);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// module functions start here
/////////////////////////////////////////////////////////////////////////////////////////////////////////
// @pymethod <o PyConsoleScreenBuffer>|win32console|CreateConsoleScreenBuffer|Creates a new console screen buffer
static PyObject *PyCreateConsoleScreenBuffer(PyObject *self, PyObject *args, PyObject *kwargs)
{
    DWORD access = GENERIC_READ | GENERIC_WRITE, sharemode = FILE_SHARE_READ | FILE_SHARE_WRITE,
          flags = CONSOLE_TEXTMODE_BUFFER;
    SECURITY_ATTRIBUTES *psa = NULL;
    LPVOID reserved = NULL;
    HANDLE hconsole;
    PyObject *obsa = Py_None;
    static char *keywords[] = {"DesiredAccess", "ShareMode", "SecurityAttributes", "Flags", 0};
    if (!PyArg_ParseTupleAndKeywords(
            args, kwargs, "|kkOk", keywords,
            &access,     // @pyparm int|DesiredAccess|GENERIC_READ and GENERIC_WRITE|GENERIC_READ and/or GENERIC_WRITE
            &sharemode,  // @pyparm int|ShareMode|FILE_SHARE_READ and FILE_SHARE_WRITE|FILE_SHARE_READ and/or
                         // FILE_SHARE_WRITE
            &obsa,       // @pyparm <o PySECURITY_ATTRIBUTES>|SecurityAttributes|None|Specifies security descriptor and
                         // inheritance for handle
            &flags))  // @pyparm int|Flags|CONSOLE_TEXTMODE_BUFFER|CONSOLE_TEXTMODE_BUFFER is currently only valid flag
        return NULL;
    if (!PyWinObject_AsSECURITY_ATTRIBUTES(obsa, &psa, TRUE))
        return NULL;

    hconsole = CreateConsoleScreenBuffer(access, sharemode, psa, flags, reserved);
    if (hconsole == INVALID_HANDLE_VALUE)
        return PyWin_SetAPIError("CreateConsoleScreenBuffer");
    // Newly created handle doesn't need to be duplicated
    return PyWinObject_FromConsoleScreenBuffer(hconsole, FALSE);
}

// @pymethod int|win32console|GetConsoleDisplayMode|Returns the current console's display mode
// @comm Only exists on Wix XP and later
// @rdesc CONSOLE_FULLSCREEN,CONSOLE_FULLSCREEN_HARDWARE
static PyObject *PyGetConsoleDisplayMode(PyObject *self, PyObject *args)
{
    DWORD mode;
    if (!PyArg_ParseTuple(args, ":GetConsoleDisplayMode"))
        return NULL;
    CHECK_PFN(GetConsoleDisplayMode);
    if (!(*pfnGetConsoleDisplayMode)(&mode))
        return PyWin_SetAPIError("GetConsoleDisplayMode");
    return PyLong_FromLong(mode);
}

// @pymethod |win32console|AttachConsole|Attaches to console of another process
// @comm Calling process must not already be attached to another console
static PyObject *PyAttachConsole(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"ProcessId", NULL};
    DWORD pid;  // @pyparm int|ProcessId||Pid of another process, or ATTACH_PARENT_PROCESS
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "l", keywords, &pid))
        return NULL;
    CHECK_PFN(AttachConsole);
    if (!(*pfnAttachConsole)(pid))
        return PyWin_SetAPIError("AttachConsole");
    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod |win32console|AllocConsole|Creates a new console for the calling process
// @comm Calling process must not already be attached to another console
static PyObject *PyAllocConsole(PyObject *self, PyObject *args)
{
    if (!PyArg_ParseTuple(args, ":AllocConsole"))
        return NULL;
    if (!AllocConsole())
        return PyWin_SetAPIError("AllocConsole");
    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod |win32console|FreeConsole|Detaches process from its current console
static PyObject *PyFreeConsole(PyObject *self, PyObject *args)
{
    if (!PyArg_ParseTuple(args, ":FreeConsole"))
        return NULL;
    if (!FreeConsole())
        return PyWin_SetAPIError("FreeConsole");
    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod (int,...)|win32console|GetConsoleProcessList|Returns pids of all processes attached to current console
static PyObject *PyGetConsoleProcessList(PyObject *self, PyObject *args)
{
    DWORD pids_returned, pids_allocated = 10, pid_ind;
    DWORD *pids = NULL;
    PyObject *ret = NULL, *ret_item;
    if (!PyArg_ParseTuple(args, ":GetConsoleProcessList"))
        return NULL;
    CHECK_PFN(GetConsoleProcessList);
    // if return count is greater than count passed in, buffer is too small
    do {
        if (pids != NULL) {
            free(pids);
            pids_allocated *= 2;
        }
        pids = (DWORD *)malloc(pids_allocated * sizeof(DWORD));
        if (pids == NULL)
            return PyErr_Format(PyExc_MemoryError, "Unable to allocate %d pids", pids_allocated);
        pids_returned = (*pfnGetConsoleProcessList)(pids, pids_allocated);
    } while (pids_returned > pids_allocated);
    // returns 0 if it fails for any other reason
    if (pids_returned == 0)
        PyWin_SetAPIError("GetConsoleProcessList");
    else {
        ret = PyTuple_New(pids_returned);
        if (ret != NULL) {
            for (pid_ind = 0; pid_ind < pids_returned; pid_ind++) {
                ret_item = PyLong_FromLong(pids[pid_ind]);
                if (ret_item == NULL) {
                    Py_DECREF(ret);
                    ret = NULL;
                    break;
                }
                PyTuple_SET_ITEM(ret, pid_ind, ret_item);
            }
        }
    }
    if (pids != NULL)
        free(pids);
    return ret;
}

// @pymethod int|win32console|GetConsoleCP|Returns the input code page for calling process's console
static PyObject *PyGetConsoleCP(PyObject *self, PyObject *args)
{
    UINT codepage;
    if (!PyArg_ParseTuple(args, ":GetConsoleCP"))
        return NULL;
    codepage = GetConsoleCP();
    return PyLong_FromUnsignedLong(codepage);
}

// @pymethod int|win32console|GetConsoleOutputCP|Returns the output code page for calling process's console
static PyObject *PyGetConsoleOutputCP(PyObject *self, PyObject *args)
{
    UINT codepage;
    if (!PyArg_ParseTuple(args, ":GetConsoleOutputCP"))
        return NULL;
    codepage = GetConsoleOutputCP();
    return PyLong_FromUnsignedLong(codepage);
}

// @pymethod |win32console|SetConsoleCP|Sets the input code page for calling process's console
static PyObject *PySetConsoleCP(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"CodePageID", NULL};
    UINT codepage;
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "k:SetConsoleCP", keywords,
                                     &codepage))  // @pyparm int|CodePageId||The code page to set
        return NULL;
    if (!SetConsoleCP(codepage))
        return PyWin_SetAPIError("SetConsoleCP");
    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod |win32console|SetConsoleOutputCP|Sets the output code page for calling process's console
static PyObject *PySetConsoleOutputCP(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"CodePageID", NULL};
    UINT codepage;
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "k:SetConsoleOutputCP", keywords,
                                     &codepage))  // @pyparm int|CodePageID||The code page to set
        return NULL;
    if (!SetConsoleOutputCP(codepage))
        return PyWin_SetAPIError("SetConsoleOutputCP");
    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod dict|win32console|GetConsoleSelectionInfo|Returns info on text selection within the current console
// @rdesc Returns a dictionary containing {Flags:int, SelectionAnchor: <o PyCOORD>, Selection:<o PySMALL_RECT>}
// Flags will contain a combination of
// CONSOLE_NO_SELECTION,CONSOLE_SELECTION_IN_PROGRESS,CONSOLE_SELECTION_NOT_EMPTY,CONSOLE_MOUSE_SELECTION,CONSOLE_MOUSE_DOWN
static PyObject *PyGetConsoleSelectionInfo(PyObject *self, PyObject *args)
{
    CONSOLE_SELECTION_INFO csi;
    if (!PyArg_ParseTuple(args, ":GetConsoleSelectionInfo"))
        return NULL;
    CHECK_PFN(GetConsoleSelectionInfo);
    if (!(*pfnGetConsoleSelectionInfo)(&csi))
        return PyWin_SetAPIError("GetConsoleSelectionInfo");
    return Py_BuildValue("{s:l,s:N,s:N}", "Flags", csi.dwFlags, "SelectionAnchor",
                         PyWinObject_FromCOORD(&csi.dwSelectionAnchor), "Selection",
                         PyWinObject_FromSMALL_RECT(&csi.srSelection));
}

// @pymethod |win32console|AddConsoleAlias|Creates a new console alias
static PyObject *PyAddConsoleAlias(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"Source", "Target", "ExeName", NULL};
    PyObject *ret = NULL, *obsource, *obtarget, *obexename;
    LPWSTR source = NULL, target = NULL, exename = NULL;
    if (!PyArg_ParseTupleAndKeywords(
            args, kwargs, "OOO:AddConsoleAlias", keywords,
            &obsource,  // @pyparm <o PyUNICODE>|Source||The string to be mapped to the target string
            &obtarget,  // @pyparm <o PyUNICODE>|Target||String to be substituted for Source.  If None, alias is removed
            &obexename))  // @pyparm <o PyUNICODE>|ExeName||Name of executable that will use alias
        return NULL;
    CHECK_PFN(AddConsoleAlias);
    if (PyWinObject_AsWCHAR(obsource, &source, FALSE) && PyWinObject_AsWCHAR(obtarget, &target, TRUE) &&
        PyWinObject_AsWCHAR(obexename, &exename, FALSE)) {
        if (!(*pfnAddConsoleAlias)(source, target, exename)) {
            PyWin_SetAPIError("AddConsoleAlias");
        }
        else {
            Py_INCREF(Py_None);
            ret = Py_None;
        }
    }
    PyWinObject_FreeWCHAR(source);
    PyWinObject_FreeWCHAR(target);
    PyWinObject_FreeWCHAR(exename);
    return ret;
}

// @pymethod <o PyUNICODE>|win32console|GetConsoleAliases|Retrieves aliases defined under specified executable
// @rdesc Returns a unicode string containing null-terminated pairs of aliases and their target text
// of the form "alias1=replacementtext1\0alias2=replacementtext2\0"
static PyObject *PyGetConsoleAliases(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"ExeName", NULL};
    PyObject *ret = NULL, *obexename;
    LPWSTR exename = NULL, buf = NULL;
    DWORD buflen;
    if (!PyArg_ParseTupleAndKeywords(
            args, kwargs, "O:GetConsoleAliases", keywords,
            &obexename))  // @pyparm <o PyUNICODE>|ExeName||Name of executable for which to return aliases
        return NULL;
    CHECK_PFN(GetConsoleAliases);
    CHECK_PFN(GetConsoleAliasesLength);
    if (PyWinObject_AsWCHAR(obexename, &exename, FALSE)) {
        buflen = (*pfnGetConsoleAliasesLength)(exename);
        if (buflen == 0)
            ret = PyWinObject_FromWCHAR(L"");
        else {
            buf = (LPWSTR)malloc(buflen);
            if (buf == NULL)
                PyErr_Format(PyExc_MemoryError, "GetConsoleAliases: Unable to allocate %d bytes", buflen);
            else if ((*pfnGetConsoleAliases)(buf, buflen, exename) == 0)
                PyWin_SetAPIError("GetConsoleAliases");
            else
                ret = PyWinObject_FromWCHAR(buf, buflen / sizeof(WCHAR));
        }
    }
    if (buf)
        free(buf);
    PyWinObject_FreeWCHAR(exename);
    return ret;
}

// @pymethod <o PyUNICODE>|win32console|GetConsoleAliasExes|Lists all executables that have console aliases defined
// @rdesc Returns a unicode string containing executable names separated by NULLS
static PyObject *PyGetConsoleAliasExes(PyObject *self, PyObject *args)
{
    PyObject *ret = NULL;
    LPWSTR buf = NULL;
    DWORD buflen;
    if (!PyArg_ParseTuple(args, ":GetConsoleAliasExes"))
        return NULL;
    CHECK_PFN(GetConsoleAliasExes);
    CHECK_PFN(GetConsoleAliasExesLength);
    buflen = (*pfnGetConsoleAliasExesLength)();
    if (buflen == 0)
        ret = PyWinObject_FromWCHAR(L"");
    else {
        buf = (LPWSTR)malloc(buflen);
        if (buf == NULL)
            PyErr_Format(PyExc_MemoryError, "GetConsoleAliasExes: Unable to allocate %d bytes", buflen);
        else if ((*pfnGetConsoleAliasExes)(buf, buflen) == 0)
            PyWin_SetAPIError("GetConsoleAliasExes");
        else
            ret = PyWinObject_FromWCHAR(buf, buflen / sizeof(WCHAR));
    }

    if (buf)
        free(buf);
    return ret;
}

// @pymethod int|win32console|GetConsoleWindow|Returns a handle to the console's window, or 0 if none exists
// @rdesc This function may raise NotImplementedError if it does not exist on
// the platform, or a <o PyHANDLE> object with a value of 0.  It will never
// raise a win32 exception.
static PyObject *PyGetConsoleWindow(PyObject *self, PyObject *args)
{
    HWND h;
    CHECK_PFN(GetConsoleWindow);
    if (!PyArg_ParseTuple(args, ":GetConsoleWindow"))
        return NULL;
    h = (*pfnGetConsoleWindow)();
    return PyWinLong_FromHANDLE(h);
}

// @pymethod int|win32console|GetNumberOfConsoleFonts|Returns the number of fonts available to the console
// @comm Function is not documented in MSDN
PyObject *PyGetNumberOfConsoleFonts(PyObject *self, PyObject *args)
{
    DWORD nbroffonts;
    CHECK_PFN(GetNumberOfConsoleFonts);
    if (!PyArg_ParseTuple(args, ":GetNumberOfConsoleFonts"))
        return NULL;
    nbroffonts = (*pfnGetNumberOfConsoleFonts)();
    return PyLong_FromLong(nbroffonts);
}

// @pymethod |win32console|SetConsoleTitle|Sets the title of the console window
PyObject *PySetConsoleTitle(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"ConsoleTitle", NULL};
    WCHAR *title = NULL;
    PyObject *obtitle, *ret = NULL;
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O:SetConsoleTitle", keywords,
                                     &obtitle))  // @pyparm <o PyUNICODE>|ConsoleTitle||New title for the console
        return NULL;
    if (!PyWinObject_AsWCHAR(obtitle, &title, FALSE))
        return NULL;
    if (!SetConsoleTitle(title))
        PyWin_SetAPIError("SetConsoleTitle");
    else {
        Py_INCREF(Py_None);
        ret = Py_None;
    }
    PyWinObject_FreeWCHAR(title);
    return ret;
}

// @pymethod <o PyUNICODE>|win32console|GetConsoleTitle|Returns the title of the console window
PyObject *PyGetConsoleTitle(PyObject *self, PyObject *args)
{
    WCHAR *title = NULL;
    DWORD chars_allocated = 64, chars_returned;
    PyObject *ret = NULL;
    if (!PyArg_ParseTuple(args, ":GetConsoleTitle"))
        return NULL;

    // if buffer is too small, function still copies as much of title as will fit,
    //  so loop until fewer characters returned than were allocated
    while (TRUE) {
        if (title != NULL) {
            free(title);
            chars_allocated *= 2;
        }
        title = (WCHAR *)malloc(chars_allocated * sizeof(WCHAR));
        if (title == NULL)
            return PyErr_Format(PyExc_MemoryError, "GetConsoleTitle: unable to allocate %d bytes",
                                chars_allocated * sizeof(WCHAR));
        chars_returned = GetConsoleTitle(title, chars_allocated);
        if (chars_returned == 0) {
            PyWin_SetAPIError("GetConsoleTitle");
            break;
        }
        if ((chars_returned + 1) < chars_allocated) {  // returned length does *not* includes the NULL terminator
            ret = PyWinObject_FromWCHAR(title);
            break;
        }
    }
    free(title);
    return ret;
}

// @pymethod |win32console|GenerateConsoleCtrlEvent|Sends a control signal to a group of processes attached to a common
// console
PyObject *PyGenerateConsoleCtrlEvent(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"CtrlEvent", "ProcessGroupId", NULL};
    DWORD evt, pid = 0;
    if (!PyArg_ParseTupleAndKeywords(
            args, kwargs, "k|k:GenerateConsoleCtrlEvent", keywords,
            &evt,   // @pyparm int|CtrlEvent||Signal to be sent to specified process group - CTRL_C_EVENT or
                    // CTRL_BREAK_EVENT
            &pid))  // @pyparm int|ProcessGroupId|0|Pid of a process group, use 0 for calling process
        return NULL;
    if (!GenerateConsoleCtrlEvent(evt, pid))
        return PyWin_SetAPIError("GenerateConsoleCtrlEvent");
    Py_INCREF(Py_None);
    return Py_None;
}

// @pymethod <o PyConsoleScreenBuffer>|win32console|GetStdHandle|Returns one of calling process's standard handles
// @rdesc Returns a <o PyConsoleScreenBuffer> wrapping the handle, or None if specified handle does not exist
PyObject *PyGetStdHandle(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *keywords[] = {"StdHandle", NULL};
    DWORD StdHandle;
    HANDLE h;
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "k:GetStdHandle", keywords,
                                     &StdHandle))  // @pyparm int|StdHandle||Specifies the handle to return -
                                                   // STD_INPUT_HANDLE, STD_OUTPUT_HANDLE, or STD_ERROR_HANDLE
        return NULL;
    h = GetStdHandle(StdHandle);
    if (h == INVALID_HANDLE_VALUE)
        return PyWin_SetAPIError("GetStdHandle");
    if (h == NULL) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    // Duplicate the handle so the processwide std handles aren't closed prematurely
    return PyWinObject_FromConsoleScreenBuffer(h, TRUE);
}

// @module win32console|Interface to the Windows Console functions for dealing with character-mode applications
static struct PyMethodDef win32console_functions[] = {
    // @pymeth CreateConsoleScreenBuffer|Creates a new console handle
    {"CreateConsoleScreenBuffer", (PyCFunction)PyCreateConsoleScreenBuffer, METH_VARARGS | METH_KEYWORDS,
     "Creates a new console screen buffer"},
    // @pymeth GetConsoleDisplayMode|Returns the current console's display mode
    {"GetConsoleDisplayMode", PyGetConsoleDisplayMode, METH_VARARGS, "Returns the current console's display mode"},
    // @pymeth AttachConsole|Attaches calling process to console of another process
    {"AttachConsole", (PyCFunction)PyAttachConsole, METH_VARARGS | METH_KEYWORDS,
     "Attaches calling process to console of another process"},
    // @pymeth AllocConsole|Creates a new console for the calling process
    {"AllocConsole", PyAllocConsole, METH_VARARGS, "Creates a new console for the calling process"},
    // @pymeth FreeConsole|Detaches process from its console
    {"FreeConsole", PyFreeConsole, METH_VARARGS, "Detaches process from its console"},
    // @pymeth GetConsoleProcessList|Returns pids of all processes attached to current console
    {"GetConsoleProcessList", PyGetConsoleProcessList, METH_VARARGS,
     "Returns pids of all processes attached to current console"},
    // @pymeth GetConsoleCP|Returns the input code page for calling process's console
    {"GetConsoleCP", PyGetConsoleCP, METH_VARARGS, "Returns the input code page for calling process's console"},
    // @pymeth GetConsoleOutputCP|Returns the output code page for calling process's console
    {"GetConsoleOutputCP", PyGetConsoleOutputCP, METH_VARARGS,
     "Returns the output code page for calling process's console"},
    // @pymeth SetConsoleCP|Sets the input code page for calling process's console
    {"SetConsoleCP", (PyCFunction)PySetConsoleCP, METH_VARARGS | METH_KEYWORDS,
     "Sets the input code page for calling process's console"},
    // @pymeth SetConsoleOutputCP|Sets the output code page for calling process's console
    {"SetConsoleOutputCP", (PyCFunction)PySetConsoleOutputCP, METH_VARARGS | METH_KEYWORDS,
     "Sets the output code page for calling process's console"},
    // @pymeth GetConsoleSelectionInfo|Returns info on text selection within the current console
    {"GetConsoleSelectionInfo", PyGetConsoleSelectionInfo, METH_VARARGS,
     "Returns info on text selection within the current console"},
    // @pymeth AddConsoleAlias|Creates a new console alias
    {"AddConsoleAlias", (PyCFunction)PyAddConsoleAlias, METH_VARARGS | METH_KEYWORDS, "Creates a new console alias"},
    // @pymeth GetConsoleAliases|Retrieves aliases defined under specified executable
    {"GetConsoleAliases", (PyCFunction)PyGetConsoleAliases, METH_VARARGS | METH_KEYWORDS,
     "Retrieves aliases defined under specified executable"},
    // @pymeth GetConsoleAliasExes|Lists all executables that have console aliases defined
    {"GetConsoleAliasExes", PyGetConsoleAliasExes, METH_VARARGS,
     "Lists all executables that have console aliases defined"},
    // @pymeth GetConsoleWindow|Returns a handle to the console's window, or 0 if none exists
    {"GetConsoleWindow", PyGetConsoleWindow, METH_VARARGS,
     "Returns a handle to the console's window, or 0 if none exists"},
    // @pymeth GetNumberOfConsoleFonts|Returns the number of fonts available to the console
    {"GetNumberOfConsoleFonts", PyGetNumberOfConsoleFonts, METH_VARARGS,
     "Returns the number of fonts available to the console"},
    // @pymeth SetConsoleTitle|Sets the title of calling process's console
    {"SetConsoleTitle", (PyCFunction)PySetConsoleTitle, METH_VARARGS | METH_KEYWORDS,
     "Sets the title of calling process's console"},
    // @pymeth GetConsoleTitle|Returns the title of console to which calling process is attached
    {"GetConsoleTitle", PyGetConsoleTitle, METH_VARARGS,
     "Returns the title of console to which calling process is attached"},
    // @pymeth GenerateConsoleCtrlEvent|Sends a control signal to a group of processes attached to a common console
    {"GenerateConsoleCtrlEvent", (PyCFunction)PyGenerateConsoleCtrlEvent, METH_VARARGS | METH_KEYWORDS,
     "Sends a control signal to a group of processes attached to a common console"},
    // @pymeth GetStdHandle|Returns one of calling process's standard handles
    {"GetStdHandle", (PyCFunction)PyGetStdHandle, METH_VARARGS | METH_KEYWORDS,
     "Returns one of calling process's standard handles"},
    {NULL, NULL}};

PYWIN_MODULE_INIT_FUNC(win32console)
{
    PYWIN_MODULE_INIT_PREPARE(
        win32console, win32console_functions,
        "Interface to the Windows Console functions for dealing with character-mode applications.");

    PyDict_SetItemString(dict, "error", PyWinExc_ApiError);

    // load function pointers
    kernel32_dll = PyWin_GetOrLoadLibraryHandle("kernel32.dll");
    if (kernel32_dll != NULL) {
        pfnGetConsoleProcessList = (GetConsoleProcessListfunc)GetProcAddress(kernel32_dll, "GetConsoleProcessList");
        pfnGetConsoleDisplayMode = (GetConsoleDisplayModefunc)GetProcAddress(kernel32_dll, "GetConsoleDisplayMode");
        pfnSetConsoleDisplayMode = (SetConsoleDisplayModefunc)GetProcAddress(kernel32_dll, "SetConsoleDisplayMode");
        pfnAttachConsole = (AttachConsolefunc)GetProcAddress(kernel32_dll, "AttachConsole");
        pfnAddConsoleAlias = (AddConsoleAliasfunc)GetProcAddress(kernel32_dll, "AddConsoleAliasW");
        pfnGetConsoleAliases = (GetConsoleAliasesfunc)GetProcAddress(kernel32_dll, "GetConsoleAliasesW");
        pfnGetConsoleAliasesLength =
            (GetConsoleAliasesLengthfunc)GetProcAddress(kernel32_dll, "GetConsoleAliasesLengthW");
        pfnGetConsoleAliasExes = (GetConsoleAliasExesfunc)GetProcAddress(kernel32_dll, "GetConsoleAliasExesW");
        pfnGetConsoleAliasExesLength =
            (GetConsoleAliasExesLengthfunc)GetProcAddress(kernel32_dll, "GetConsoleAliasExesLengthW");
        pfnGetConsoleWindow = (GetConsoleWindowfunc)GetProcAddress(kernel32_dll, "GetConsoleWindow");
        pfnGetCurrentConsoleFont = (GetCurrentConsoleFontfunc)GetProcAddress(kernel32_dll, "GetCurrentConsoleFont");
        pfnGetConsoleFontSize = (GetConsoleFontSizefunc)GetProcAddress(kernel32_dll, "GetConsoleFontSize");
        pfnGetConsoleSelectionInfo =
            (GetConsoleSelectionInfofunc)GetProcAddress(kernel32_dll, "GetConsoleSelectionInfo");
        pfnGetNumberOfConsoleFonts =
            (GetNumberOfConsoleFontsfunc)GetProcAddress(kernel32_dll, "GetNumberOfConsoleFonts");
        pfnSetConsoleFont = (SetConsoleFontfunc)GetProcAddress(kernel32_dll, "SetConsoleFont");
    }

    if (PyType_Ready(&PyConsoleScreenBufferType) == -1)
        PYWIN_MODULE_INIT_RETURN_ERROR;
    if (PyDict_SetItemString(dict, "PyConsoleScreenBufferType", (PyObject *)&PyConsoleScreenBufferType) == -1)
        PYWIN_MODULE_INIT_RETURN_ERROR;

    if (PyType_Ready(&PySMALL_RECTType) == -1)
        PYWIN_MODULE_INIT_RETURN_ERROR;
    if (PyDict_SetItemString(dict, "PySMALL_RECTType", (PyObject *)&PySMALL_RECTType) == -1)
        PYWIN_MODULE_INIT_RETURN_ERROR;

    if (PyType_Ready(&PyCOORDType) == -1)
        PYWIN_MODULE_INIT_RETURN_ERROR;
    if (PyDict_SetItemString(dict, "PyCOORDType", (PyObject *)&PyCOORDType) == -1)
        PYWIN_MODULE_INIT_RETURN_ERROR;

    if (PyType_Ready(&PyINPUT_RECORDType) == -1)
        PYWIN_MODULE_INIT_RETURN_ERROR;
    if (PyDict_SetItemString(dict, "PyINPUT_RECORDType", (PyObject *)&PyINPUT_RECORDType) == -1)
        PYWIN_MODULE_INIT_RETURN_ERROR;

    PyModule_AddIntConstant(module, "CONSOLE_TEXTMODE_BUFFER", CONSOLE_TEXTMODE_BUFFER);
    PyModule_AddIntConstant(module, "CONSOLE_FULLSCREEN", CONSOLE_FULLSCREEN);
    PyModule_AddIntConstant(module, "CONSOLE_FULLSCREEN_HARDWARE", CONSOLE_FULLSCREEN_HARDWARE);
    PyModule_AddIntConstant(module, "ATTACH_PARENT_PROCESS", ATTACH_PARENT_PROCESS);

    PyModule_AddIntConstant(module, "ENABLE_LINE_INPUT", ENABLE_LINE_INPUT);
    PyModule_AddIntConstant(module, "ENABLE_ECHO_INPUT", ENABLE_ECHO_INPUT);
    PyModule_AddIntConstant(module, "ENABLE_PROCESSED_INPUT", ENABLE_PROCESSED_INPUT);
    PyModule_AddIntConstant(module, "ENABLE_WINDOW_INPUT", ENABLE_WINDOW_INPUT);
    PyModule_AddIntConstant(module, "ENABLE_MOUSE_INPUT", ENABLE_MOUSE_INPUT);
    PyModule_AddIntConstant(module, "ENABLE_PROCESSED_OUTPUT", ENABLE_PROCESSED_OUTPUT);
    PyModule_AddIntConstant(module, "ENABLE_WRAP_AT_EOL_OUTPUT", ENABLE_WRAP_AT_EOL_OUTPUT);

    // text attribute flags - ?????? COMMON_* flags don't seem to do anything ??????
    PyModule_AddIntConstant(module, "FOREGROUND_BLUE", FOREGROUND_BLUE);
    PyModule_AddIntConstant(module, "FOREGROUND_GREEN", FOREGROUND_GREEN);
    PyModule_AddIntConstant(module, "FOREGROUND_RED", FOREGROUND_RED);
    PyModule_AddIntConstant(module, "FOREGROUND_INTENSITY", FOREGROUND_INTENSITY);
    PyModule_AddIntConstant(module, "BACKGROUND_BLUE", BACKGROUND_BLUE);
    PyModule_AddIntConstant(module, "BACKGROUND_GREEN", BACKGROUND_GREEN);
    PyModule_AddIntConstant(module, "BACKGROUND_RED", BACKGROUND_RED);
    PyModule_AddIntConstant(module, "BACKGROUND_INTENSITY", BACKGROUND_INTENSITY);
    PyModule_AddIntConstant(module, "COMMON_LVB_LEADING_BYTE", COMMON_LVB_LEADING_BYTE);
    PyModule_AddIntConstant(module, "COMMON_LVB_TRAILING_BYTE", COMMON_LVB_TRAILING_BYTE);
    PyModule_AddIntConstant(module, "COMMON_LVB_GRID_HORIZONTAL", COMMON_LVB_GRID_HORIZONTAL);
    PyModule_AddIntConstant(module, "COMMON_LVB_GRID_LVERTICAL", COMMON_LVB_GRID_LVERTICAL);
    PyModule_AddIntConstant(module, "COMMON_LVB_GRID_RVERTICAL", COMMON_LVB_GRID_RVERTICAL);
    PyModule_AddIntConstant(module, "COMMON_LVB_REVERSE_VIDEO", COMMON_LVB_REVERSE_VIDEO);
    PyModule_AddIntConstant(module, "COMMON_LVB_UNDERSCORE", COMMON_LVB_UNDERSCORE);

    // selection flags for GetConsoleSelectionInfo
    PyModule_AddIntConstant(module, "CONSOLE_NO_SELECTION", CONSOLE_NO_SELECTION);
    PyModule_AddIntConstant(module, "CONSOLE_SELECTION_IN_PROGRESS", CONSOLE_SELECTION_IN_PROGRESS);
    PyModule_AddIntConstant(module, "CONSOLE_SELECTION_NOT_EMPTY", CONSOLE_SELECTION_NOT_EMPTY);
    PyModule_AddIntConstant(module, "CONSOLE_MOUSE_SELECTION", CONSOLE_MOUSE_SELECTION);
    PyModule_AddIntConstant(module, "CONSOLE_MOUSE_DOWN", CONSOLE_MOUSE_DOWN);
    PyModule_AddIntConstant(module, "LOCALE_USER_DEFAULT", LOCALE_USER_DEFAULT);

    // event types for INPUT_RECORD
    PyModule_AddIntConstant(module, "KEY_EVENT", KEY_EVENT);
    PyModule_AddIntConstant(module, "MOUSE_EVENT", MOUSE_EVENT);
    PyModule_AddIntConstant(module, "WINDOW_BUFFER_SIZE_EVENT", WINDOW_BUFFER_SIZE_EVENT);
    PyModule_AddIntConstant(module, "MENU_EVENT", MENU_EVENT);
    PyModule_AddIntConstant(module, "FOCUS_EVENT", FOCUS_EVENT);

    // Control events for GenerateConsoleCtrlEvent
    PyModule_AddIntConstant(module, "CTRL_C_EVENT", CTRL_C_EVENT);
    PyModule_AddIntConstant(module, "CTRL_BREAK_EVENT", CTRL_BREAK_EVENT);

    // std handles
    PyModule_AddIntConstant(module, "STD_INPUT_HANDLE", STD_INPUT_HANDLE);
    PyModule_AddIntConstant(module, "STD_OUTPUT_HANDLE", STD_OUTPUT_HANDLE);
    PyModule_AddIntConstant(module, "STD_ERROR_HANDLE", STD_ERROR_HANDLE);

    // flags used with SetConsoleDisplayMode
    // ?????? these aren't in my SDK headers
    PyModule_AddIntConstant(module, "CONSOLE_FULLSCREEN_MODE", 1);
    PyModule_AddIntConstant(module, "CONSOLE_WINDOWED_MODE", 2);

    PYWIN_MODULE_INIT_RETURN_SUCCESS;
}
