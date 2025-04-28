/******************************************************************************

win32helpmodule.cpp - An interface to the win32 WinHelp and HtmlHelp API's

Note that this source file contains embedded documentation.
This documentation consists of marked up text inside the
C comments, and is prefixed with an '@' symbol.  The source
files are processed by a tool called "autoduck" which
generates Windows .hlp files.

  Author: Roger Burnham, June 1999, rburnham@cri-inc.com

  @doc

******************************************************************************/

#include "PyWinTypes.h"
#include "htmlhelp.h"

#define DllExport _declspec(dllexport)
#define PyW32_BEGIN_ALLOW_THREADS PyThreadState *_save = PyEval_SaveThread()
#define PyW32_END_ALLOW_THREADS PyEval_RestoreThread(_save)

PyObject *ReturnAPIError(char *fnName, long err = 0) { return PyWin_SetAPIError(fnName, err); }

// Conventional WinHelp:

//*****************************************************************************
//
// @pymethod |win32help|WinHelp|Invokes the Windows Help system.

static PyObject *PyWinHelp(PyObject *self, PyObject *args)
{
    // @pyparm int|hwnd||The handle of the window requesting help.
    // @pyparm string|hlpFile||The name of the help file.
    // @pyparm int|cmd||The type of help.  See the api for full details.
    // @pyparm None/int/string|data|None|Additional data specific to the help call. Can be a buffer or pointer-sized
    // int.
    HWND hwnd;
    TCHAR *hlpFile = NULL;
    PyObject *obhlpFile;
    UINT cmd;
    PyObject *obData = Py_None;
    ULONG_PTR data;
    PyWinBufferView pybuf;

    if (!PyArg_ParseTuple(args, "O&Oi|O:WinHelp", PyWinObject_AsHANDLE, &hwnd, &obhlpFile, &cmd, &obData))
        return NULL;

    if (!pybuf.init(obData, false, true)) {
        PyErr_Clear();
        if (!PyWinLong_AsULONG_PTR(obData, &data)) {
            PyErr_SetString(PyExc_TypeError, "Data must be a buffer, None, or pointer-sized number");
            return NULL;
        }
    }
    else
        data = (ULONG_PTR)pybuf.ptr();
    if (!PyWinObject_AsTCHAR(obhlpFile, &hlpFile, FALSE))
        return NULL;
    PyW32_BEGIN_ALLOW_THREADS;
    BOOL ok = ::WinHelp(hwnd, hlpFile, cmd, data);
    PyW32_END_ALLOW_THREADS;
    PyWinObject_FreeTCHAR(hlpFile);
    if (!ok)
        return ReturnAPIError("WinHelp");
    Py_INCREF(Py_None);
    return Py_None;

    // @pyseeapi WinHelp
    // @rdesc The method raises an exception if an error occurs.
}

// Support for an HH_AKLINK object.
class PyHH_AKLINK : public PyObject {
   public:
    HH_AKLINK *GetAKLINK() { return &m_HH_AKLINK; }

    PyHH_AKLINK(void);
    // PyHH_AKLINK(const HH_AKLINK *pAKLINK);
    ~PyHH_AKLINK();

    /* Python support */

    static void deallocFunc(PyObject *ob);
    static int setattro(PyObject *self, PyObject *obname, PyObject *v);
    static struct PyMemberDef members[];

   protected:
    HH_AKLINK m_HH_AKLINK;
    PyObject *m_pszKeywords;  // LPCTSTR       pszKeywords;
    PyObject *m_pszUrl;       // LPCTSTR       pszUrl;
    PyObject *m_pszMsgText;   // LPCTSTR       pszMsgText;
    PyObject *m_pszMsgTitle;  // LPCTSTR       pszMsgTitle;
    PyObject *m_pszWindow;    // LPCTSTR       pszWindow;
};

#define PyHH_AKLINK_Check(ob) ((ob)->ob_type == &PyHH_AKLINKType)

// @object PyHH_AKLINK|A Python object, representing an HH_AKLINK structure
// @comm Typically you create a PyHH_AKLINK (via <om win32help.HH_AKLINK>)
// object, and set its properties.
// The object can then be passed to any function which takes an HH_AKLINK
// object.<nl>
//<nl>
// Use this structure to specify one or more ALink names or KLink keywords
// that you want to search for.<nl>
//<nl>
// If the lookup yields no matching topics, HtmlHelp() checks the values of
// the following HH_AKLINK members to determine what alternative action to
// take:<nl>
//<nl>
// indexOnFail. If indexOnFail is TRUE, the Index tab is selected in the
// help window specified in window, and the keyword specified in
// keyword is selected in the entry field.<nl>
//<nl>
// url. If indexOnFail is FALSE, the topic file specified in url
// appears in the help window specified in window.<nl>
// msgText and msgTitle. If indexOnFail is FALSE and url is NULL,
// a message box appears using the text and caption specified in
// msgText and msgTitle.<nl>
//<nl>
// Used by<nl>
//<c HH_ALINK_LOOKUP><nl>
//<c HH_KEYWORD_LOOKUP><nl>

PyTypeObject PyHH_AKLINKType = {
    PYWIN_OBJECT_HEAD "PyHH_AKLINK",                        /* tp_name */
    sizeof(PyHH_AKLINK),                                    /* tp_basicsize */
    0,                                                      /* tp_itemsize */
    PyHH_AKLINK::deallocFunc,                               /* tp_dealloc */
    0,                                                      /* tp_print */
    0,                                                      /* tp_getattr */
    0,                                                      /* tp_setattr */
    0,                                                      /* tp_compare */
    0,                                                      /* tp_repr */
    0,                                                      /* tp_as_number */
    0,                                                      /* tp_as_sequence */
    0,                                                      /* tp_as_mapping */
    0,                                                      /* tp_hash */
    0,                                                      /* tp_call */
    0,                                                      /* tp_str */
    PyObject_GenericGetAttr,                                /* tp_getattro */
    PyHH_AKLINK::setattro,                                  /* tp_setattro */
    0,                                                      /* tp_as_buffer */
    0,                                                      /* tp_flags */
    "A Python object, representing an HH_AKLINK structure", /* tp_doc */
    0,                                                      /* tp_traverse */
    0,                                                      /* tp_clear */
    0,                                                      /* tp_richcompare */
    0,                                                      /* tp_weaklistoffset */
    0,                                                      /* tp_iter */
    0,                                                      /* tp_iternext */
    0,                                                      /* tp_methods */
    PyHH_AKLINK::members,                                   /* tp_members */
    0,                                                      /* tp_getset */
    0,                                                      /* tp_base */
    0,                                                      /* tp_dict */
    0,                                                      /* tp_descr_get */
    0,                                                      /* tp_descr_set */
    0,                                                      /* tp_dictoffset */
    0,                                                      /* tp_init */
    0,                                                      /* tp_alloc */
    0,                                                      /* tp_new */
};

#undef OFF
#define OFF(e) offsetof(PyHH_AKLINK, e)

/*static*/ struct PyMemberDef PyHH_AKLINK::members[] = {

    // BOOL       fIndexOnFail;
    // @prop int|indexOnFail|Specifies whether to display the keyword in the
    // Index tab of the HTML Help Viewer if the lookup fails. The value of
    // window specifies the Help Viewer.
    {"indexOnFail", T_INT, OFF(m_HH_AKLINK.fIndexOnFail)},

    //**************************************************************************
    //**************************************************************************
    // The following are added _ONLY_ so that they show up in a
    // dir() of the object, they are never handled via the memberlist.

    // LPCTSTR       pszKeywords;
    // @prop string|keywords|Specifies one or more ALink names or KLink
    // keywords to look up. Multiple entries are delimited by a semicolon.
    {"keywords", T_OBJECT, OFF(m_pszKeywords)},

    // LPCTSTR       pszUrl;
    // @prop string|url|Specifies the topic file to navigate to if the lookup
    // fails. url refers to a valid topic within the specified compiled help
    //(.chm) file and does not support Internet protocols that point to an
    // HTML file.
    {"url", T_OBJECT, OFF(m_pszUrl)},

    // LPCTSTR       pszMsgText;
    // @prop string|msgText|Specifies the text to display in a message box if
    // the lookup fails and indexOnFail is FALSE and url is NULL.
    {"msgText", T_OBJECT, OFF(m_pszMsgText)},

    // LPCTSTR       pszMsgTitle;
    // @prop string|msgTitle|Specifies the caption of the message box in which
    // the msgText parameter appears.
    {"msgTitle", T_OBJECT, OFF(m_pszMsgTitle)},

    // LPCTSTR       pszWindow;
    // @prop string|window|Specifies the name of the window type in which to
    // display one of the following:<nl>
    //<nl>
    // The selected topic, if the lookup yields one or more matching topics.
    // The topic specified in url, if the lookup fails and a topic is specified
    // in url.<nl>
    //<nl>
    // The Index tab, if the lookup fails and indexOnFail is specified as TRUE.
    {"window", T_OBJECT, OFF(m_pszWindow)},
    //**************************************************************************
    //**************************************************************************

    {NULL} /* Sentinel */
};

PyHH_AKLINK::PyHH_AKLINK()
{
    ob_type = &PyHH_AKLINKType;
    _Py_NewReference(this);
    memset(&m_HH_AKLINK, 0, sizeof(m_HH_AKLINK));

    m_HH_AKLINK.cbStruct = sizeof(m_HH_AKLINK);
    m_pszKeywords = m_pszUrl = m_pszMsgText = NULL;
    m_pszMsgTitle = m_pszWindow = NULL;
}

/* This is not necessary, as HH_AKLINK is not used as output
    from any functions.  Plus, it never actually copies the
    strings into the Python object
PyHH_AKLINK::PyHH_AKLINK(const HH_AKLINK *pAKLINK)
{
  ob_type = &PyHH_AKLINKType;
  _Py_NewReference(this);
  memcpy(&m_HH_AKLINK, pAKLINK, sizeof(m_HH_AKLINK));

  m_pszKeywords = pAKLINK->pszKeywords
    ? PyWinObject_FromTCHAR((TCHAR*)pAKLINK->pszKeywords)
    : NULL;
  m_pszUrl = pAKLINK->pszUrl
    ? PyWinObject_FromTCHAR((TCHAR*)pAKLINK->pszUrl)
    : NULL;
  m_pszMsgText = pAKLINK->pszMsgText
    ? PyWinObject_FromTCHAR((TCHAR*)pAKLINK->pszMsgText)
    : NULL;
  m_pszMsgTitle = pAKLINK->pszMsgTitle
    ? PyWinObject_FromTCHAR((TCHAR*)pAKLINK->pszMsgTitle)
    : NULL;
  m_pszWindow = pAKLINK->pszWindow
    ? PyWinObject_FromTCHAR((TCHAR*)pAKLINK->pszWindow)
    : NULL;
}
*/

PyHH_AKLINK::~PyHH_AKLINK(void)
{
    Py_XDECREF(m_pszKeywords);
    Py_XDECREF(m_pszUrl);
    Py_XDECREF(m_pszMsgText);
    Py_XDECREF(m_pszMsgTitle);
    Py_XDECREF(m_pszWindow);
}

int PyHH_AKLINK::setattro(PyObject *self, PyObject *obname, PyObject *v)
{
    char *name = PYWIN_ATTR_CONVERT(obname);
    if (name == NULL)
        return -1;
    PyHH_AKLINK *pO = (PyHH_AKLINK *)self;

    if (strcmp("keywords", name) == 0) {
        if (PyWinObject_AsTCHAR(v, (TCHAR **)&pO->m_HH_AKLINK.pszKeywords)) {
            Py_XDECREF(pO->m_pszKeywords);
            pO->m_pszKeywords = v;
            Py_INCREF(v);
            return 0;
        }
        else
            return -1;
    }
    if (strcmp("url", name) == 0) {
        if (PyWinObject_AsTCHAR(v, (TCHAR **)&pO->m_HH_AKLINK.pszUrl)) {
            Py_XDECREF(pO->m_pszUrl);
            pO->m_pszUrl = v;
            Py_INCREF(v);
            return 0;
        }
        else
            return -1;
    }
    if (strcmp("msgText", name) == 0) {
        if (PyWinObject_AsTCHAR(v, (TCHAR **)&pO->m_HH_AKLINK.pszMsgText)) {
            Py_XDECREF(pO->m_pszMsgText);
            pO->m_pszMsgText = v;
            Py_INCREF(v);
            return 0;
        }
        else
            return -1;
    }
    if (strcmp("msgTitle", name) == 0) {
        if (PyWinObject_AsTCHAR(v, (TCHAR **)&pO->m_HH_AKLINK.pszMsgTitle)) {
            Py_XDECREF(pO->m_pszMsgTitle);
            pO->m_pszMsgTitle = v;
            Py_INCREF(v);
            return 0;
        }
        else
            return -1;
    }
    if (strcmp("window", name) == 0) {
        if (PyWinObject_AsTCHAR(v, (TCHAR **)&pO->m_HH_AKLINK.pszWindow)) {
            Py_XDECREF(pO->m_pszWindow);
            pO->m_pszWindow = v;
            Py_INCREF(v);
            return 0;
        }
        else
            return -1;
    }
    return PyObject_GenericSetAttr(self, obname, v);
}

/*static*/ void PyHH_AKLINK::deallocFunc(PyObject *ob) { delete (PyHH_AKLINK *)ob; }

// A converter.
BOOL PyWinObject_AsHH_AKLINK(PyObject *ob, HH_AKLINK **ppAKLINK, BOOL bNoneOK)
{
    if (bNoneOK && ob == Py_None) {
        *ppAKLINK = NULL;
    }
    else if (!PyHH_AKLINK_Check(ob)) {
        PyErr_SetString(PyExc_TypeError, "The object is not a PyHH_AKLINK object");
        return FALSE;
    }
    else {
        *ppAKLINK = ((PyHH_AKLINK *)ob)->GetAKLINK();
    }
    return TRUE;
}

/* This struct is only used as input, and constructor does not copy string members correctly anyway
PyObject *PyWinObject_FromHH_AKLINK(const HH_AKLINK *pAKLINK)
{
  if (pAKLINK==NULL) {
    Py_INCREF(Py_None);
    return Py_None;
  }
  PyObject *ret = new PyHH_AKLINK(pAKLINK);
  if(ret==NULL)
    PyErr_SetString(PyExc_MemoryError, "PyHH_AKLINK");
  return ret;
}
*/

//*****************************************************************************
//
// @pymethod <o PyHH_AKLINK>|win32help|HH_AKLINK|
// Creates a new HH_AKLINK object.

static PyObject *myHH_AKLINK(PyObject *self, PyObject *args)
{
    if (!PyArg_ParseTuple(args, ":HH_AKLINK"))
        return NULL;
    return new PyHH_AKLINK();
}

// Support for an HH_FTS_QUERY object.

class PyHH_FTS_QUERY : public PyObject {
   public:
    HH_FTS_QUERY *GetFTS_QUERY() { return &m_HH_FTS_QUERY; }

    PyHH_FTS_QUERY(void);
    PyHH_FTS_QUERY(const HH_FTS_QUERY *pFTS_QUERY);
    ~PyHH_FTS_QUERY();

    /* Python support */

    static void deallocFunc(PyObject *ob);

    static PyObject *getattro(PyObject *self, PyObject *obname);
    static int setattro(PyObject *self, PyObject *obname, PyObject *v);
    static struct PyMemberDef members[];

   protected:
    HH_FTS_QUERY m_HH_FTS_QUERY;
    PyObject *m_pszSearchQuery;  // LPCTSTR       pszSearchQuery;
};

#define PyHH_FTS_QUERY_Check(ob) ((ob)->ob_type == &PyHH_FTS_QUERYType)

// @object PyHH_FTS_QUERY|A Python object, representing an HH_FTS_QUERY
// structure
// @comm Typically you create a PyHH_FTS_QUERY
//(via <om win32help.HH_FTS_QUERY>) object, and set its properties.
// The object can then be passed to any function which takes an HH_FTS_QUERY
// object.<nl>
//<nl>
// Use this structure for full-text search.

PyTypeObject PyHH_FTS_QUERYType = {
    PYWIN_OBJECT_HEAD "PyHH_FTS_QUERY",                     /* tp_name */
    sizeof(PyHH_FTS_QUERY),                                 /* tp_basicsize */
    0,                                                      /* tp_itemsize */
    PyHH_FTS_QUERY::deallocFunc,                            /* tp_dealloc */
    0,                                                      /* tp_print */
    0,                                                      /* tp_getattr */
    0,                                                      /* tp_setattr */
    0,                                                      /* tp_compare */
    0,                                                      /* tp_repr */
    0,                                                      /* tp_as_number */
    0,                                                      /* tp_as_sequence */
    0,                                                      /* tp_as_mapping */
    0,                                                      /* tp_hash */
    0,                                                      /* tp_call */
    0,                                                      /* tp_str */
    PyHH_FTS_QUERY::getattro,                               /* tp_getattro */
    PyHH_FTS_QUERY::setattro,                               /* tp_setattro */
    0,                                                      /* tp_as_buffer */
    0,                                                      /* tp_flags */
    "A Python object, representing an HH_FTS_QUERY struct", /* tp_doc */
    0,                                                      /* tp_traverse */
    0,                                                      /* tp_clear */
    0,                                                      /* tp_richcompare */
    0,                                                      /* tp_weaklistoffset */
    0,                                                      /* tp_iter */
    0,                                                      /* tp_iternext */
    0,                                                      /* tp_methods */
    PyHH_FTS_QUERY::members,                                /* tp_members */
    0,                                                      /* tp_getset */
    0,                                                      /* tp_base */
    0,                                                      /* tp_dict */
    0,                                                      /* tp_descr_get */
    0,                                                      /* tp_descr_set */
    0,                                                      /* tp_dictoffset */
    0,                                                      /* tp_init */
    0,                                                      /* tp_alloc */
    0,                                                      /* tp_new */
};

#undef OFF
#define OFF(e) offsetof(PyHH_FTS_QUERY, e)

/*static*/ struct PyMemberDef PyHH_FTS_QUERY::members[] = {

    // BOOL     fUniCodeStrings;
    // @prop int|uniCodeStrings|TRUE if all strings are Unicode.
    {"uniCodeStrings", T_INT, OFF(m_HH_FTS_QUERY.fUniCodeStrings)},

    // LONG     iProximity;
    // @prop long|proximity|Word proximity.
    {"proximity", T_LONG, OFF(m_HH_FTS_QUERY.iProximity)},

    // BOOL     fStemmedSearch;
    // @prop int|stemmedSearch|TRUE for StemmedSearch only.
    {"stemmedSearch", T_INT, OFF(m_HH_FTS_QUERY.fStemmedSearch)},

    // BOOL     fTitleOnly;
    // @prop int|titleOnly|TRUE for Title search only.
    {"titleOnly", T_INT, OFF(m_HH_FTS_QUERY.fTitleOnly)},

    // BOOL     fExecute;
    // @prop int|execute|TRUE to initiate the search.
    {"execute", T_INT, OFF(m_HH_FTS_QUERY.fExecute)},

    //**************************************************************************
    //**************************************************************************
    // The following are added _ONLY_ so that they show up in a
    // dir() of the object, they are never handled via the memberlist.

    // LPCTSTR       pszSearchQuery;
    // @prop string|searchQuery|String containing the search query.
    {"searchQuery", T_STRING, OFF(m_HH_FTS_QUERY.pszSearchQuery)},
    //**************************************************************************
    //**************************************************************************

    {NULL} /* Sentinel */
};

PyHH_FTS_QUERY::PyHH_FTS_QUERY()
{
    ob_type = &PyHH_FTS_QUERYType;
    _Py_NewReference(this);
    memset(&m_HH_FTS_QUERY, 0, sizeof(m_HH_FTS_QUERY));

    m_HH_FTS_QUERY.cbStruct = sizeof(m_HH_FTS_QUERY);
    m_pszSearchQuery = NULL;
}

PyHH_FTS_QUERY::PyHH_FTS_QUERY(const HH_FTS_QUERY *pFTS_QUERY)
{
    ob_type = &PyHH_FTS_QUERYType;
    _Py_NewReference(this);
    memcpy(&m_HH_FTS_QUERY, pFTS_QUERY, sizeof(m_HH_FTS_QUERY));

    m_pszSearchQuery = pFTS_QUERY->pszSearchQuery ? PyWinObject_FromTCHAR((TCHAR *)pFTS_QUERY->pszSearchQuery) : NULL;
}

PyHH_FTS_QUERY::~PyHH_FTS_QUERY(void) { Py_XDECREF(m_pszSearchQuery); }

PyObject *PyHH_FTS_QUERY::getattro(PyObject *self, PyObject *obname)
{
    char *name = PYWIN_ATTR_CONVERT(obname);
    if (name == NULL)
        return NULL;
    PyHH_FTS_QUERY *pO = (PyHH_FTS_QUERY *)self;

    if (strcmp("searchQuery", name) == 0) {
        PyObject *rc = pO->m_pszSearchQuery ? pO->m_pszSearchQuery : Py_None;
        Py_INCREF(rc);
        return rc;
    }

    return PyObject_GenericGetAttr(self, obname);
}

int PyHH_FTS_QUERY::setattro(PyObject *self, PyObject *obname, PyObject *v)
{
    if (v == NULL) {
        PyErr_SetString(PyExc_AttributeError, "can't delete HH_FTS_QUERY attributes");
        return -1;
    }
    char *name = PYWIN_ATTR_CONVERT(obname);
    if (name == NULL)
        return -1;

    PyHH_FTS_QUERY *pO = (PyHH_FTS_QUERY *)self;

    if (strcmp("searchQuery", name) == 0) {
        if (PyWinObject_AsTCHAR(v, (TCHAR **)&pO->m_HH_FTS_QUERY.pszSearchQuery)) {
            Py_XDECREF(pO->m_pszSearchQuery);
            pO->m_pszSearchQuery = v;
            Py_INCREF(v);
            return 0;
        }
        else
            return -1;
    }
    return PyObject_GenericSetAttr(self, obname, v);
}

/*static*/ void PyHH_FTS_QUERY::deallocFunc(PyObject *ob) { delete (PyHH_FTS_QUERY *)ob; }

// A converter.
BOOL PyWinObject_AsHH_FTS_QUERY(PyObject *ob, HH_FTS_QUERY **ppFTS_QUERY, BOOL bNoneOK)
{
    if (bNoneOK && ob == Py_None) {
        *ppFTS_QUERY = NULL;
    }
    else if (!PyHH_FTS_QUERY_Check(ob)) {
        PyErr_SetString(PyExc_TypeError, "The object is not a PyHH_FTS_QUERY object");
        return FALSE;
    }
    else {
        *ppFTS_QUERY = ((PyHH_FTS_QUERY *)ob)->GetFTS_QUERY();
    }
    return TRUE;
}

PyObject *PyWinObject_FromHH_FTS_QUERY(const HH_FTS_QUERY *pFTS_QUERY)
{
    if (pFTS_QUERY == NULL) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    PyObject *ret = new PyHH_FTS_QUERY(pFTS_QUERY);
    if (ret == NULL)
        PyErr_SetString(PyExc_MemoryError, "PyHH_FTS_QUERY");
    return ret;
}

//*****************************************************************************
//
// @pymethod <o PyHH_FTS_QUERY>|win32help|HH_FTS_QUERY|
// Creates a new HH_FTS_QUERY object.

static PyObject *myHH_FTS_QUERY(PyObject *self, PyObject *args)
{
    if (!PyArg_ParseTuple(args, ":HH_FTS_QUERY"))
        return NULL;
    return new PyHH_FTS_QUERY();
}

// HH_LAST_ERROR object has _NOT_ been implemented in the Html Help engine.

// Support for an HH_POPUP object.

class PyHH_POPUP : public PyObject {
   public:
    HH_POPUP *GetPOPUP() { return &m_HH_POPUP; }

    PyHH_POPUP(void);
    PyHH_POPUP(const HH_POPUP *pPOPUP);
    ~PyHH_POPUP();

    /* Python support */

    static void deallocFunc(PyObject *ob);

    static PyObject *getattro(PyObject *self, PyObject *obname);
    static int setattro(PyObject *self, PyObject *obname, PyObject *v);
    static struct PyMemberDef members[];

   protected:
    HH_POPUP m_HH_POPUP;
    PyObject *m_pszText;    // LPCTSTR       pszText;
    PyObject *m_pt;         // POINT         pt;
    PyObject *m_rcMargins;  // RECT          rcMargins;
    PyObject *m_pszFont;    // LPCTSTR       pszFont;
};

#define PyHH_POPUP_Check(ob) ((ob)->ob_type == &PyHH_POPUPType)

// @object PyHH_POPUP|A Python object, representing an HH_POPUP structure
// @comm Typically you create a PyHH_POPUP (via <om win32help.HH_POPUP>)
// object, and set its properties.
// The object can then be passed to any function which takes an HH_POPUP
// object.<nl>
//<nl>
// Use this structure to specify or modify the attributes of a pop-up
// window.<nl>
//<nl>
// Used by<nl>
//<c HH_DISPLAY_TEXT_POPUP><nl>

PyTypeObject PyHH_POPUPType = {
    PYWIN_OBJECT_HEAD "PyHH_POPUP",                        /* tp_name */
    sizeof(PyHH_POPUP),                                    /* tp_basicsize */
    0,                                                     /* tp_itemsize */
    PyHH_POPUP::deallocFunc,                               /* tp_dealloc */
    0,                                                     /* tp_print */
    0,                                                     /* tp_getattr */
    0,                                                     /* tp_setattr */
    0,                                                     /* tp_compare */
    0,                                                     /* tp_repr */
    0,                                                     /* tp_as_number */
    0,                                                     /* tp_as_sequence */
    0,                                                     /* tp_as_mapping */
    0,                                                     /* tp_hash */
    0,                                                     /* tp_call */
    0,                                                     /* tp_str */
    PyHH_POPUP::getattro,                                  /* tp_getattro */
    PyHH_POPUP::setattro,                                  /* tp_setattro */
    0,                                                     /* tp_as_buffer */
    0,                                                     /* tp_flags */
    "A Python object, representing an HH_POPUP structure", /* tp_doc */
    0,                                                     /* tp_traverse */
    0,                                                     /* tp_clear */
    0,                                                     /* tp_richcompare */
    0,                                                     /* tp_weaklistoffset */
    0,                                                     /* tp_iter */
    0,                                                     /* tp_iternext */
    0,                                                     /* tp_methods */
    PyHH_POPUP::members,                                   /* tp_members */
    0,                                                     /* tp_getset */
    0,                                                     /* tp_base */
    0,                                                     /* tp_dict */
    0,                                                     /* tp_descr_get */
    0,                                                     /* tp_descr_set */
    0,                                                     /* tp_dictoffset */
    0,                                                     /* tp_init */
    0,                                                     /* tp_alloc */
    0,                                                     /* tp_new */
};

#undef OFF
#define OFF(e) offsetof(PyHH_POPUP, e)

/*static*/ struct PyMemberDef PyHH_POPUP::members[] = {

    // HINSTANCE   hinst;
    // @prop long|hinst|Instance handle of the program or DLL to retrieve the
    // string resource from. Ignored if idString is zero.
    {"hinst", T_LONG, OFF(m_HH_POPUP.hinst)},

    // UINT idString;
    // @prop unsigned int|idString|Specifies zero, or a resource ID in the
    // program or DLL specified in hinst.
    {"idString", T_UINT, OFF(m_HH_POPUP.idString)},

    // COLORREF    clrForeground;
    // @prop int|clrForeground|Specifies the RGB value to use for the
    // foreground color of the pop-up window. To use the system color for the
    // window text, specify -1.
    {"clrForeground", T_INT, OFF(m_HH_POPUP.clrForeground)},

    // COLORREF    clrBackground;
    // @prop int|clrBackground|Specifies the RGB value to use for the
    // background color of the pop-up window. To use the system color for the
    // window background, specify -1.
    {"clrBackground", T_INT, OFF(m_HH_POPUP.clrBackground)},

    //**************************************************************************
    //**************************************************************************
    // The following are added _ONLY_ so that they show up in a
    // dir() of the object, they are never handled via the memberlist.

    // LPCTSTR       pszText;
    // @prop string|text|Specifies the text to display if idString is zero.
    {"text", T_STRING, OFF(m_HH_POPUP.pszText)},

    // LPCTSTR       pszFont;
    // @prop string|font|Specifies the font attributes to use for the text in
    // the pop-up window.<nl>
    // Use the following format to specify font family, point size, character
    // set, and font format:<nl>
    // facename[, point size[, charset[ BOLD ITALIC UNDERLINE]]]<nl>
    // To omit an attribute, enter a comma. For example, to specify bold, 10-pt,
    // MS Sans Serif font, font would be:<nl>
    // MS Sans Serif, 10, , BOLD
    {"font", T_STRING, OFF(m_HH_POPUP.pszFont)},

    // POINT  pt;
    // @prop tuple|pt|(x,y). Specifies (in pixels) where the top center of the
    // pop-up window should be located.
    {"pt", T_STRING, OFF(m_HH_POPUP.pt)},

    // RECT   rcMargins;
    // @prop tuple|margins|(left,top,right,bottom). Specifies (in pixels) the
    // margins to use on the left, top, right, and bottom sides of the pop-up
    // window. The default for all rectangle members is -1.
    {"margins", T_STRING, OFF(m_HH_POPUP.rcMargins)},
    //**************************************************************************
    //**************************************************************************

    {NULL} /* Sentinel */
};

PyHH_POPUP::PyHH_POPUP()
{
    ob_type = &PyHH_POPUPType;
    _Py_NewReference(this);
    memset(&m_HH_POPUP, 0, sizeof(m_HH_POPUP));

    m_HH_POPUP.cbStruct = sizeof(m_HH_POPUP);
    m_pszText = m_pt = m_rcMargins = m_pszFont = NULL;
}

PyHH_POPUP::PyHH_POPUP(const HH_POPUP *pPOPUP)
{
    ob_type = &PyHH_POPUPType;
    _Py_NewReference(this);
    memcpy(&m_HH_POPUP, pPOPUP, sizeof(m_HH_POPUP));

    m_pszText = pPOPUP->pszText ? PyWinObject_FromTCHAR((TCHAR *)pPOPUP->pszText) : NULL;
    m_pszFont = pPOPUP->pszFont ? PyWinObject_FromTCHAR((TCHAR *)pPOPUP->pszFont) : NULL;

    m_pt = PyTuple_New(2);
    PyTuple_SetItem(m_pt, 0, PyLong_FromLong(pPOPUP->pt.x));
    PyTuple_SetItem(m_pt, 1, PyLong_FromLong(pPOPUP->pt.y));

    m_rcMargins = PyTuple_New(4);
    PyTuple_SetItem(m_rcMargins, 0, PyLong_FromLong(pPOPUP->rcMargins.left));
    PyTuple_SetItem(m_rcMargins, 1, PyLong_FromLong(pPOPUP->rcMargins.right));
    PyTuple_SetItem(m_rcMargins, 2, PyLong_FromLong(pPOPUP->rcMargins.top));
    PyTuple_SetItem(m_rcMargins, 3, PyLong_FromLong(pPOPUP->rcMargins.bottom));
}

PyHH_POPUP::~PyHH_POPUP(void)
{
    Py_XDECREF(m_pszText);
    Py_XDECREF(m_pt);
    Py_XDECREF(m_rcMargins);
    Py_XDECREF(m_pszFont);
}

PyObject *PyHH_POPUP::getattro(PyObject *self, PyObject *obname)
{
    char *name = PYWIN_ATTR_CONVERT(obname);
    if (name == NULL)
        return NULL;

    PyHH_POPUP *pO = (PyHH_POPUP *)self;

    if (strcmp("text", name) == 0) {
        PyObject *rc = pO->m_pszText ? pO->m_pszText : Py_None;
        Py_INCREF(rc);
        return rc;
    }
    if (strcmp("font", name) == 0) {
        PyObject *rc = pO->m_pszFont ? pO->m_pszFont : Py_None;
        Py_INCREF(rc);
        return rc;
    }
    if (strcmp("pt", name) == 0) {
        PyObject *rc = pO->m_pt ? pO->m_pt : Py_None;
        Py_INCREF(rc);
        return rc;
    }
    if (strcmp("margins", name) == 0) {
        PyObject *rc = pO->m_rcMargins ? pO->m_rcMargins : Py_None;
        Py_INCREF(rc);
        return rc;
    }

    return PyObject_GenericGetAttr(self, obname);
}

int PyHH_POPUP::setattro(PyObject *self, PyObject *obname, PyObject *v)
{
    if (v == NULL) {
        PyErr_SetString(PyExc_AttributeError, "can't delete HH_POPUP attributes");
        return -1;
    }
    char *name = PYWIN_ATTR_CONVERT(obname);
    if (name == NULL)
        return -1;

    PyHH_POPUP *pO = (PyHH_POPUP *)self;

    if (strcmp("text", name) == 0) {
        if (PyWinObject_AsTCHAR(v, (TCHAR **)&pO->m_HH_POPUP.pszText)) {
            Py_XDECREF(pO->m_pszText);
            pO->m_pszText = v;
            Py_INCREF(v);
            return 0;
        }
        else
            return -1;
    }
    if (strcmp("font", name) == 0) {
        if (PyWinObject_AsTCHAR(v, (TCHAR **)&pO->m_HH_POPUP.pszFont)) {
            Py_XDECREF(pO->m_pszFont);
            pO->m_pszFont = v;
            Py_INCREF(v);
            return 0;
        }
        else
            return -1;
    }
    if (strcmp("pt", name) == 0) {
        int x, y;
        if (PyArg_ParseTuple(v, "ii", &x, &y)) {
            Py_XDECREF(pO->m_pt);
            pO->m_pt = v;
            Py_INCREF(v);
            pO->m_HH_POPUP.pt.x = x;
            pO->m_HH_POPUP.pt.y = y;
            return 0;
        }
        else
            return -1;
    }
    if (strcmp("margins", name) == 0) {
        int left, top, right, bottom;
        if (PyArg_ParseTuple(v, "iiii", &left, &top, &right, &bottom)) {
            Py_XDECREF(pO->m_rcMargins);
            pO->m_rcMargins = v;
            Py_INCREF(v);
            pO->m_HH_POPUP.rcMargins.left = left;
            pO->m_HH_POPUP.rcMargins.top = top;
            pO->m_HH_POPUP.rcMargins.right = top;
            pO->m_HH_POPUP.rcMargins.bottom = top;
            return 0;
        }
        else
            return -1;
    }
    return PyObject_GenericSetAttr(self, obname, v);
}

/*static*/ void PyHH_POPUP::deallocFunc(PyObject *ob) { delete (PyHH_POPUP *)ob; }

// A converter.
BOOL PyWinObject_AsHH_POPUP(PyObject *ob, HH_POPUP **ppPOPUP, BOOL bNoneOK)
{
    if (bNoneOK && ob == Py_None) {
        *ppPOPUP = NULL;
    }
    else if (!PyHH_POPUP_Check(ob)) {
        PyErr_SetString(PyExc_TypeError, "The object is not a PyHH_POPUP object");
        return FALSE;
    }
    else {
        *ppPOPUP = ((PyHH_POPUP *)ob)->GetPOPUP();
    }
    return TRUE;
}

PyObject *PyWinObject_FromHH_POPUP(const HH_POPUP *pPOPUP)
{
    if (pPOPUP == NULL) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    PyObject *ret = new PyHH_POPUP(pPOPUP);
    if (ret == NULL)
        PyErr_SetString(PyExc_MemoryError, "PyHH_POPUP");
    return ret;
}

//*****************************************************************************
//
// @pymethod <o PyHH_POPUP>|win32help|HH_POPUP|Creates a new HH_POPUP object.

static PyObject *myHH_POPUP(PyObject *self, PyObject *args)
{
    if (!PyArg_ParseTuple(args, ":HH_POPUP"))
        return NULL;
    return new PyHH_POPUP();
}

// Support for an HH_WINTYPE object.

class PyHH_WINTYPE : public PyObject {
   public:
    HH_WINTYPE *GetWINTYPE() { return &m_HH_WINTYPE; }

    PyHH_WINTYPE(void);
    PyHH_WINTYPE(const HH_WINTYPE *pWINTYPE);
    ~PyHH_WINTYPE();

    /* Python support */

    static void deallocFunc(PyObject *ob);

    static PyObject *getattro(PyObject *self, PyObject *obname);
    static int setattro(PyObject *self, PyObject *obname, PyObject *v);
    static struct PyMemberDef members[];

   protected:
    HH_WINTYPE m_HH_WINTYPE;
};

#define PyHH_WINTYPE_Check(ob) ((ob)->ob_type == &PyHH_WINTYPEType)

// @object PyHH_WINTYPE|A Python object, representing an HH_WINTYPE structure
// @comm Typically you create a PyHH_WINTYPE (via <om win32help.HH_WINTYPE>)
// object, and set its properties.
// The object can then be passed to any function which takes an HH_WINTYPE
// object.<nl>
//<nl>
// Use this structure to specify or modify the attributes of a window type.
// Window types can be defined by an author in a project (.hhp) file, or they
// can be defined programmatically using the HTML Help API.<nl>
// When a HH_WINTYPE structure is passed to HtmlHelp() using the
//<c HH_SET_WIN_TYPE> command, the HTML Help API makes a private copy of the
// contents of the structure. The help developer is therefore responsible for
// freeing memory used by the HH_WINTYPE structure or character arrays
// within it. The help developer can free memory after calling
//<c HH_SET_WIN_TYPE>.<nl>
//<nl>
// Used by<nl>
//<c HH_SET_WIN_TYPE><nl>
//<c HH_GET_WIN_TYPE><nl>

PyTypeObject PyHH_WINTYPEType = {
    PYWIN_OBJECT_HEAD "PyHH_WINTYPE",                        /* tp_name */
    sizeof(PyHH_WINTYPE),                                    /* tp_basicsize */
    0,                                                       /* tp_itemsize */
    PyHH_WINTYPE::deallocFunc,                               /* tp_dealloc */
    0,                                                       /* tp_print */
    0,                                                       /* tp_getattr */
    0,                                                       /* tp_setattr */
    0,                                                       /* tp_compare */
    0,                                                       /* tp_repr */
    0,                                                       /* tp_as_number */
    0,                                                       /* tp_as_sequence */
    0,                                                       /* tp_as_mapping */
    0,                                                       /* tp_hash */
    0,                                                       /* tp_call */
    0,                                                       /* tp_str */
    PyHH_WINTYPE::getattro,                                  /* tp_getattro */
    PyHH_WINTYPE::setattro,                                  /* tp_setattro */
    0,                                                       /* tp_as_buffer */
    0,                                                       /* tp_flags */
    "A Python object, representing an HH_WINTYPE structure", /* tp_doc */
    0,                                                       /* tp_traverse */
    0,                                                       /* tp_clear */
    0,                                                       /* tp_richcompare */
    0,                                                       /* tp_weaklistoffset */
    0,                                                       /* tp_iter */
    0,                                                       /* tp_iternext */
    0,                                                       /* tp_methods */
    PyHH_WINTYPE::members,                                   /* tp_members */
    0,                                                       /* tp_getset */
    0,                                                       /* tp_base */
    0,                                                       /* tp_dict */
    0,                                                       /* tp_descr_get */
    0,                                                       /* tp_descr_set */
    0,                                                       /* tp_dictoffset */
    0,                                                       /* tp_init */
    0,                                                       /* tp_alloc */
    0,                                                       /* tp_new */
};

#undef OFF
#define OFF(e) offsetof(PyHH_WINTYPE, e)

/*static*/ struct PyMemberDef PyHH_WINTYPE::members[] = {

    // BOOL   fUniCodeStrings;
    // @prop int|uniCodeStrings|Specifies whether the strings used in this
    // structure are UNICODE.
    {"uniCodeStrings", T_INT, OFF(m_HH_WINTYPE.fUniCodeStrings)},

    // DWORD  fsValidMembers;
    // @prop int|validMembers|Specifies which members in the structure are valid.
    {"validMembers", T_ULONG, OFF(m_HH_WINTYPE.fsValidMembers)},

    // DWORD  fsWinProperties;
    // @prop int|winProperties|Specifies the properties of the window, such as
    // whether it is the standard HTML Help Viewer or whether it includes a
    // Search tab.
    {"winProperties", T_ULONG, OFF(m_HH_WINTYPE.fsWinProperties)},

    // DWORD  dwStyles;
    // @prop int|styles|Specifies the styles used to create the window. These
    // styles can be ignored, combined with extended styles, or used exclusively
    // depending on the value of the validMembers and winProperties parameters.
    {"styles", T_ULONG, OFF(m_HH_WINTYPE.dwStyles)},

    // DWORD  dwExStyles;
    // @prop int|exStyles|Specifies the extended styles used to create the
    // window. These styles can be ignored, combined with default styles, or used
    // exclusively depending on the value of the validMembers and winProperties
    // parameters.
    {"exStyles", T_ULONG, OFF(m_HH_WINTYPE.dwExStyles)},

    // int    nShowState;
    // @prop int|showState|Specifies the initial display state of the window.
    // Valid values are the same as those for the Win32 API ShowWindow function.
    {"showState", T_INT, OFF(m_HH_WINTYPE.nShowState)},

    // These params handled below...
    // HWND   hwndHelp;
    // @prop int|hwndHelp|Specifies the handle of the window if the window has
    // been created.

    // HWND   hwndCaller;
    // @prop int|hwndCaller|Specifies the window that will receive HTML Help
    // notification messages. Notification messages are sent via Windows
    // WM_NOTIFY messages.

    // HWND   hwndToolBar;
    // @prop int|hwndToolBar|Specifies the handle of the toolbar.

    // HWND   hwndNavigation;
    // @prop int|hwndNavigation|Specifies the handle of the Navigation pane.

    // HWND   hwndHTML;
    // @prop int|hwndHTML|Specifies the handle of the Topic pane, which hosts
    // Shdocvw.dll.

    // int    iNavWidth;
    // @prop int|navWidth|Specifies the width of the Navigation pane when the
    // Help Viewer is expanded.
    {"navWidth", T_INT, OFF(m_HH_WINTYPE.iNavWidth)},

    // DWORD  fsToolBarFlags;
    // @prop int|toolBarFlags|Specifies which buttons to include on the toolbar.
    {"toolBarFlags", T_ULONG, OFF(m_HH_WINTYPE.fsToolBarFlags)},

    // BOOL   fNotExpanded;
    // @prop int|notExpanded|Specifies that the Help Viewer open with the
    // Navigation pane closed.
    {"notExpanded", T_INT, OFF(m_HH_WINTYPE.fNotExpanded)},

    // int    curNavType;
    // @prop int|curNavType|Specifies the default tab to display on the
    // Navigation pane.
    {"curNavType", T_INT, OFF(m_HH_WINTYPE.curNavType)},

    // int    idNotify;
    // @prop int|idNotify|Specifies a non-zero ID for enabling HTML Help
    // notification messages. This ID is passed as the wParam value of Windows
    // WM_NOTIFY messages.
    {"idNotify", T_INT, OFF(m_HH_WINTYPE.idNotify)},

    //**************************************************************************
    //**************************************************************************
    // The following are added _ONLY_ so that they show up in a
    // dir() of the object, they are never handled via the memberlist.

    // LPCTSTR       pszType;
    // @prop string|typeName|A null-terminated string that specifies the name
    // of the window type.
    {"typeName", T_STRING, OFF(m_HH_WINTYPE.pszType)},

    // LPCTSTR       pszCaption;
    // @prop string|caption|A null-terminated string that specifies the caption
    // to display in the title bar of the window.
    {"caption", T_STRING, OFF(m_HH_WINTYPE.pszCaption)},

    // RECT   rcWindowPos;
    // @prop tuple|windowPos|(left,top,right,bottom). Specifies the coordinates
    // of the window in pixels.
    {"windowPos", T_STRING, OFF(m_HH_WINTYPE.rcWindowPos)},

    // RECT   rcHTML;
    // @prop tuple|HTMLPos|(left,top,right,bottom). Specifies the coordinates
    // of the Topic pane.
    {"HTMLPos", T_STRING, OFF(m_HH_WINTYPE.rcHTML)},

    // LPCTSTR       pszToc;
    // @prop string|toc|Specifies the contents (.hhc) file to display in the
    // Navigation pane.
    {"toc", T_STRING, OFF(m_HH_WINTYPE.pszToc)},

    // LPCTSTR       pszIndex;
    // @prop string|index|Specifies the index (.hhk) file to display in the
    // Navigation pane.
    {"index", T_STRING, OFF(m_HH_WINTYPE.pszIndex)},

    // LPCTSTR       pszFile;
    // @prop string|file|Specifies the default HTML file to display in the
    // Topic pane.
    {"file", T_STRING, OFF(m_HH_WINTYPE.pszFile)},

    // LPCTSTR       pszHome;
    // @prop string|home|Specifies the file or URL to display in the Topic pane
    // when the Home button is clicked.
    {"home", T_STRING, OFF(m_HH_WINTYPE.pszHome)},

    // LPCTSTR       pszJump1;
    // @prop string|jump1|Specifies the text to display underneath the Jump1
    // button.
    {"jump1", T_STRING, OFF(m_HH_WINTYPE.pszJump1)},

    // LPCTSTR       pszJump2;
    // @prop string|jump2|Specifies the text to display underneath the Jump2
    // button.
    {"jump2", T_STRING, OFF(m_HH_WINTYPE.pszJump2)},

    // LPCTSTR       pszUrlJump1;
    // @prop string|urlJump1|Specifies the URL to jump to when the Jump1 button
    // is clicked.
    {"urlJump1", T_STRING, OFF(m_HH_WINTYPE.pszUrlJump1)},

    // LPCTSTR       pszUrlJump2;
    // @prop string|urlJump2|Specifies the URL to jump to when the Jump2 button
    // is clicked.
    {"urlJump2", T_STRING, OFF(m_HH_WINTYPE.pszUrlJump2)},
    //**************************************************************************
    //**************************************************************************

    {NULL} /* Sentinel */
};

PyHH_WINTYPE::PyHH_WINTYPE()
{
    ob_type = &PyHH_WINTYPEType;
    _Py_NewReference(this);
    memset(&m_HH_WINTYPE, 0, sizeof(m_HH_WINTYPE));
    m_HH_WINTYPE.cbStruct = sizeof(m_HH_WINTYPE);
}

PyHH_WINTYPE::PyHH_WINTYPE(const HH_WINTYPE *pWINTYPE)
{
    ob_type = &PyHH_WINTYPEType;
    _Py_NewReference(this);
    memcpy(&m_HH_WINTYPE, pWINTYPE, sizeof(m_HH_WINTYPE));

    // as the API doc says: Deep copy the structure to which dwData points
    // before modifying the structure.
    // ??? This used to copy the string right back to itself ???
    if (pWINTYPE->pszType)
        m_HH_WINTYPE.pszType = PyWin_CopyString(pWINTYPE->pszType);
    if (pWINTYPE->pszCaption)
        m_HH_WINTYPE.pszCaption = PyWin_CopyString(pWINTYPE->pszCaption);
    if (pWINTYPE->pszToc)
        m_HH_WINTYPE.pszToc = PyWin_CopyString(pWINTYPE->pszToc);
    if (pWINTYPE->pszIndex)
        m_HH_WINTYPE.pszIndex = PyWin_CopyString(pWINTYPE->pszIndex);
    if (pWINTYPE->pszFile)
        m_HH_WINTYPE.pszFile = PyWin_CopyString(pWINTYPE->pszFile);
    if (pWINTYPE->pszHome)
        m_HH_WINTYPE.pszHome = PyWin_CopyString(pWINTYPE->pszHome);
    if (pWINTYPE->pszJump1)
        m_HH_WINTYPE.pszJump1 = PyWin_CopyString(pWINTYPE->pszJump1);
    if (pWINTYPE->pszJump2)
        m_HH_WINTYPE.pszJump2 = PyWin_CopyString(pWINTYPE->pszJump2);
    if (pWINTYPE->pszUrlJump1)
        m_HH_WINTYPE.pszUrlJump1 = PyWin_CopyString(pWINTYPE->pszUrlJump1);
    if (pWINTYPE->pszUrlJump2)
        m_HH_WINTYPE.pszUrlJump2 = PyWin_CopyString(pWINTYPE->pszUrlJump2);
}

PyHH_WINTYPE::~PyHH_WINTYPE(void)
{
    PyWinObject_FreeTCHAR((TCHAR *)m_HH_WINTYPE.pszType);
    PyWinObject_FreeTCHAR((TCHAR *)m_HH_WINTYPE.pszCaption);
    PyWinObject_FreeTCHAR((TCHAR *)m_HH_WINTYPE.pszToc);
    PyWinObject_FreeTCHAR((TCHAR *)m_HH_WINTYPE.pszIndex);
    PyWinObject_FreeTCHAR((TCHAR *)m_HH_WINTYPE.pszFile);
    PyWinObject_FreeTCHAR((TCHAR *)m_HH_WINTYPE.pszHome);
    PyWinObject_FreeTCHAR((TCHAR *)m_HH_WINTYPE.pszJump1);
    PyWinObject_FreeTCHAR((TCHAR *)m_HH_WINTYPE.pszJump2);
    PyWinObject_FreeTCHAR((TCHAR *)m_HH_WINTYPE.pszUrlJump1);
    PyWinObject_FreeTCHAR((TCHAR *)m_HH_WINTYPE.pszUrlJump2);
}

PyObject *PyHH_WINTYPE::getattro(PyObject *self, PyObject *obname)
{
    char *name = PYWIN_ATTR_CONVERT(obname);
    if (name == NULL)
        return NULL;
    PyHH_WINTYPE *pO = (PyHH_WINTYPE *)self;

    if (strcmp("typeName", name) == 0)
        return PyWinObject_FromTCHAR(pO->m_HH_WINTYPE.pszType);
    if (strcmp("caption", name) == 0)
        return PyWinObject_FromTCHAR(pO->m_HH_WINTYPE.pszCaption);
    if (strcmp("toc", name) == 0)
        return PyWinObject_FromTCHAR(pO->m_HH_WINTYPE.pszToc);
    if (strcmp("index", name) == 0)
        return PyWinObject_FromTCHAR(pO->m_HH_WINTYPE.pszIndex);
    if (strcmp("file", name) == 0)
        return PyWinObject_FromTCHAR(pO->m_HH_WINTYPE.pszFile);
    if (strcmp("home", name) == 0)
        return PyWinObject_FromTCHAR(pO->m_HH_WINTYPE.pszHome);
    if (strcmp("jump1", name) == 0)
        return PyWinObject_FromTCHAR(pO->m_HH_WINTYPE.pszJump1);
    if (strcmp("jump2", name) == 0)
        return PyWinObject_FromTCHAR(pO->m_HH_WINTYPE.pszJump2);
    if (strcmp("urlJump1", name) == 0)
        return PyWinObject_FromTCHAR(pO->m_HH_WINTYPE.pszUrlJump1);
    if (strcmp("urlJump2", name) == 0)
        return PyWinObject_FromTCHAR(pO->m_HH_WINTYPE.pszUrlJump2);

    if (strcmp("windowPos", name) == 0)
        return PyWinObject_FromRECT(&pO->m_HH_WINTYPE.rcWindowPos);
    if (strcmp("HTMLPos", name) == 0)
        return PyWinObject_FromRECT(&pO->m_HH_WINTYPE.rcHTML);

    if (strcmp("hwndHelp", name) == 0)
        return PyWinLong_FromHANDLE(pO->m_HH_WINTYPE.hwndHelp);
    if (strcmp("hwndCaller", name) == 0)
        return PyWinLong_FromHANDLE(pO->m_HH_WINTYPE.hwndCaller);
    if (strcmp("hwndToolBar", name) == 0)
        return PyWinLong_FromHANDLE(pO->m_HH_WINTYPE.hwndToolBar);
    if (strcmp("hwndNavigation", name) == 0)
        return PyWinLong_FromHANDLE(pO->m_HH_WINTYPE.hwndNavigation);
    if (strcmp("hwndHTML", name) == 0)
        return PyWinLong_FromHANDLE(pO->m_HH_WINTYPE.hwndHTML);
    return PyObject_GenericGetAttr(self, obname);
}

int PyHH_WINTYPE::setattro(PyObject *self, PyObject *obname, PyObject *v)
{
    if (v == NULL) {
        PyErr_SetString(PyExc_AttributeError, "can't delete HH_WINTYPE attributes");
        return -1;
    }
    char *name = PYWIN_ATTR_CONVERT(obname);
    if (name == NULL)
        return -1;
    PyHH_WINTYPE *pO = (PyHH_WINTYPE *)self;

    TCHAR *tchar_val;
    HANDLE hwnd_val;
    if (strcmp("typeName", name) == 0) {
        if (!PyWinObject_AsTCHAR(v, &tchar_val))
            return -1;
        pO->m_HH_WINTYPE.pszType = tchar_val;
        return 0;
    }
    if (strcmp("caption", name) == 0) {
        if (!PyWinObject_AsTCHAR(v, &tchar_val))
            return -1;
        pO->m_HH_WINTYPE.pszCaption = tchar_val;
        return 0;
    }
    if (strcmp("toc", name) == 0) {
        if (!PyWinObject_AsTCHAR(v, &tchar_val))
            return -1;
        pO->m_HH_WINTYPE.pszToc = tchar_val;
        return 0;
    }
    if (strcmp("index", name) == 0) {
        if (!PyWinObject_AsTCHAR(v, &tchar_val))
            return -1;
        pO->m_HH_WINTYPE.pszIndex = tchar_val;
        return 0;
    }
    if (strcmp("file", name) == 0) {
        if (!PyWinObject_AsTCHAR(v, &tchar_val))
            return -1;
        pO->m_HH_WINTYPE.pszFile = tchar_val;
        return 0;
    }
    if (strcmp("home", name) == 0) {
        if (!PyWinObject_AsTCHAR(v, &tchar_val))
            return -1;
        pO->m_HH_WINTYPE.pszHome = tchar_val;
        return 0;
    }
    if (strcmp("jump1", name) == 0) {
        if (!PyWinObject_AsTCHAR(v, &tchar_val))
            return -1;
        pO->m_HH_WINTYPE.pszJump1 = tchar_val;
        return 0;
    }
    if (strcmp("jump2", name) == 0) {
        if (!PyWinObject_AsTCHAR(v, &tchar_val))
            return -1;
        pO->m_HH_WINTYPE.pszJump2 = tchar_val;
        return 0;
    }
    if (strcmp("urlJump1", name) == 0) {
        if (!PyWinObject_AsTCHAR(v, &tchar_val))
            return -1;
        pO->m_HH_WINTYPE.pszUrlJump1 = tchar_val;
        return 0;
    }
    if (strcmp("urlJump2", name) == 0) {
        if (!PyWinObject_AsTCHAR(v, &tchar_val))
            return -1;
        pO->m_HH_WINTYPE.pszUrlJump2 = tchar_val;
        return 0;
    }
    if (strcmp("windowPos", name) == 0) {
        RECT rc;
        if (!PyWinObject_AsRECT(v, &rc))
            return -1;
        pO->m_HH_WINTYPE.rcWindowPos = rc;
        return 0;
    }
    if (strcmp("HTMLPos", name) == 0) {
        RECT rc;
        if (!PyWinObject_AsRECT(v, &rc))
            return -1;
        pO->m_HH_WINTYPE.rcHTML = rc;
        return 0;
    }

    if (strcmp("hwndHelp", name) == 0) {
        if (!PyWinObject_AsHANDLE(v, &hwnd_val))
            return -1;
        pO->m_HH_WINTYPE.hwndHelp = (HWND)hwnd_val;
        return 0;
    }
    if (strcmp("hwndCaller", name) == 0) {
        if (!PyWinObject_AsHANDLE(v, &hwnd_val))
            return -1;
        pO->m_HH_WINTYPE.hwndCaller = (HWND)hwnd_val;
        return 0;
    }
    if (strcmp("hwndToolBar", name) == 0) {
        if (!PyWinObject_AsHANDLE(v, &hwnd_val))
            return -1;
        pO->m_HH_WINTYPE.hwndToolBar = (HWND)hwnd_val;
        return 0;
    }
    if (strcmp("hwndNavigation", name) == 0) {
        if (!PyWinObject_AsHANDLE(v, &hwnd_val))
            return -1;
        pO->m_HH_WINTYPE.hwndNavigation = (HWND)hwnd_val;
        return 0;
    }
    if (strcmp("hwndHTML", name) == 0) {
        if (!PyWinObject_AsHANDLE(v, &hwnd_val))
            return -1;
        pO->m_HH_WINTYPE.hwndHTML = (HWND)hwnd_val;
        return 0;
    }
    return PyObject_GenericSetAttr(self, obname, v);
}

/*static*/ void PyHH_WINTYPE::deallocFunc(PyObject *ob) { delete (PyHH_WINTYPE *)ob; }

// A converter.
BOOL PyWinObject_AsHH_WINTYPE(PyObject *ob, HH_WINTYPE **ppWINTYPE, BOOL bNoneOK)
{
    if (bNoneOK && ob == Py_None) {
        *ppWINTYPE = NULL;
    }
    else if (!PyHH_WINTYPE_Check(ob)) {
        PyErr_SetString(PyExc_TypeError, "The object is not a PyHH_WINTYPE object");
        return FALSE;
    }
    else {
        *ppWINTYPE = ((PyHH_WINTYPE *)ob)->GetWINTYPE();
    }
    return TRUE;
}

PyObject *PyWinObject_FromHH_WINTYPE(const HH_WINTYPE *pWINTYPE)
{
    if (pWINTYPE == NULL) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    PyObject *ret = new PyHH_WINTYPE(pWINTYPE);
    if (ret == NULL)
        PyErr_SetString(PyExc_MemoryError, "PyHH_WINTYPE");
    return ret;
}

//*****************************************************************************
//
// @pymethod <o PyHH_WINTYPE>|win32help|HH_WINTYPE|
// Creates a new HH_WINTYPE object.

static PyObject *myHH_WINTYPE(PyObject *self, PyObject *args)
{
    if (!PyArg_ParseTuple(args, ":HH_WINTYPE"))
        return NULL;
    return new PyHH_WINTYPE();
}

// Support for an NMHDR object.

class PyNMHDR : public PyObject {
   public:
    NMHDR *GetNMHDR() { return &m_NMHDR; }

    PyNMHDR(void);
    PyNMHDR(const NMHDR *pNMHDR);
    ~PyNMHDR();

    /* Python support */

    static void deallocFunc(PyObject *ob);
    static struct PyMemberDef members[];

   protected:
    NMHDR m_NMHDR;
};

#define PyNMHDR_Check(ob) ((ob)->ob_type == &PyNMHDRType)

// @object PyNMHDR|A Python object, representing an NMHDR
// structure
// @comm Typically you create a PyNMHDR
//(via <om win32help.NMHDR>) object, and set its properties.
// The object can then be passed to any function which takes an NMHDR
// object.<nl>
//<nl>
// Contains information about a notification message.

PyTypeObject PyNMHDRType = {
    PYWIN_OBJECT_HEAD "PyNMHDR",                        /* tp_name */
    sizeof(PyNMHDR),                                    /* tp_basicsize */
    0,                                                  /* tp_itemsize */
    PyNMHDR::deallocFunc,                               /* tp_dealloc */
    0,                                                  /* tp_print */
    0,                                                  /* tp_getattr */
    0,                                                  /* tp_setattr */
    0,                                                  /* tp_compare */
    0,                                                  /* tp_repr */
    0,                                                  /* tp_as_number */
    0,                                                  /* tp_as_sequence */
    0,                                                  /* tp_as_mapping */
    0,                                                  /* tp_hash */
    0,                                                  /* tp_call */
    0,                                                  /* tp_str */
    PyObject_GenericGetAttr,                            /* tp_getattro */
    PyObject_GenericSetAttr,                            /* tp_setattro */
    0,                                                  /* tp_as_buffer */
    0,                                                  /* tp_flags */
    "A Python object, representing an NMHDR structure", /* tp_doc */
    0,                                                  /* tp_traverse */
    0,                                                  /* tp_clear */
    0,                                                  /* tp_richcompare */
    0,                                                  /* tp_weaklistoffset */
    0,                                                  /* tp_iter */
    0,                                                  /* tp_iternext */
    0,                                                  /* tp_methods */
    PyNMHDR::members,                                   /* tp_members */
    0,                                                  /* tp_getset */
    0,                                                  /* tp_base */
    0,                                                  /* tp_dict */
    0,                                                  /* tp_descr_get */
    0,                                                  /* tp_descr_set */
    0,                                                  /* tp_dictoffset */
    0,                                                  /* tp_init */
    0,                                                  /* tp_alloc */
    0,                                                  /* tp_new */
};

#undef OFF
#define OFF(e) offsetof(PyNMHDR, e)

/*static*/ struct PyMemberDef PyNMHDR::members[] = {

    // HWND hwndFrom;
    // @prop int|hwndFrom|Window handle to the control sending a message.
    // ??? 64-bit problem here ???
    {"hwndFrom", T_INT, OFF(m_NMHDR.hwndFrom)},

    // UINT idFrom;
    // @prop unsigned int|idFrom|Identifier of the control sending a message.
    {"idFrom", T_INT, OFF(m_NMHDR.idFrom)},

    // UINT code;
    // @prop unsigned int|code|Notification code. This member can be a
    // control-specific notification code or it can be one of the common
    // notification codes.
    {"code", T_INT, OFF(m_NMHDR.code)},

    //**************************************************************************
    //**************************************************************************
    // The following are added _ONLY_ so that they show up in a
    // dir() of the object, they are never handled via the memberlist.

    //**************************************************************************
    //**************************************************************************

    {NULL} /* Sentinel */
};

PyNMHDR::PyNMHDR()
{
    ob_type = &PyNMHDRType;
    _Py_NewReference(this);
    memset(&m_NMHDR, 0, sizeof(m_NMHDR));
}

PyNMHDR::PyNMHDR(const NMHDR *pNMHDR)
{
    ob_type = &PyNMHDRType;
    _Py_NewReference(this);
    memcpy(&m_NMHDR, pNMHDR, sizeof(m_NMHDR));
}

PyNMHDR::~PyNMHDR(void) {};

/*static*/ void PyNMHDR::deallocFunc(PyObject *ob) { delete (PyNMHDR *)ob; }

// A converter.
BOOL PyWinObject_AsNMHDR(PyObject *ob, NMHDR **ppNMHDR, BOOL bNoneOK)
{
    if (bNoneOK && ob == Py_None) {
        *ppNMHDR = NULL;
    }
    else if (!PyNMHDR_Check(ob)) {
        PyErr_SetString(PyExc_TypeError, "The object is not a PyNMHDR object");
        return FALSE;
    }
    else {
        *ppNMHDR = ((PyNMHDR *)ob)->GetNMHDR();
    }
    return TRUE;
}

PyObject *PyWinObject_FromNMHDR(const NMHDR *pNMHDR)
{
    if (pNMHDR == NULL) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    PyObject *ret = new PyNMHDR(pNMHDR);
    if (ret == NULL)
        PyErr_SetString(PyExc_MemoryError, "PyNMHDR");
    return ret;
}

//*****************************************************************************
//
// @pymethod <o PyNMHDR>|win32help|NMHDR|
// Creates a new NMHDR object.

static PyObject *myNMHDR(PyObject *self, PyObject *args)
{
    if (!PyArg_ParseTuple(args, ":NMHDR"))
        return NULL;
    return new PyNMHDR();
}

// Support for an HHN_NOTIFY object.

class PyHHN_NOTIFY : public PyObject {
   public:
    HHN_NOTIFY *GetN_NOTIFY() { return &m_HHN_NOTIFY; }

    PyHHN_NOTIFY(void);
    PyHHN_NOTIFY(const HHN_NOTIFY *pN_NOTIFY);
    ~PyHHN_NOTIFY();

    /* Python support */

    static void deallocFunc(PyObject *ob);

    static PyObject *getattro(PyObject *self, PyObject *obname);
    static int setattro(PyObject *self, PyObject *obname, PyObject *v);
    static struct PyMemberDef members[];

   protected:
    HHN_NOTIFY m_HHN_NOTIFY;
    PyObject *m_hdr;     // NMHDR  hdr;
    PyObject *m_pszUrl;  // PCSTR  pszUrl;
};

#define PyHHN_NOTIFY_Check(ob) ((ob)->ob_type == &PyHHN_NOTIFYType)

// @object PyHHN_NOTIFY|A Python object, representing an HHN_NOTIFY
// structure
// @comm Typically you create a PyHHN_NOTIFY
//(via <om win32help.HHN_NOTIFY>) object, and set its properties.
// The object can then be passed to any function which takes an HHN_NOTIFY
// object.<nl>
//<nl>
// Use this structure to return the file name of the topic that has been
// navigated to, or to return the window type name of the help window that
// has been created.<nl>
//<nl>
// Used by<nl>
//<c HHN_NAVCOMPLETE><nl>
//<c HHN_WINDOW_CREATE><nl>

PyTypeObject PyHHN_NOTIFYType = {
    PYWIN_OBJECT_HEAD "PyHHN_NOTIFY",                        /* tp_name */
    sizeof(PyHHN_NOTIFY),                                    /* tp_basicsize */
    0,                                                       /* tp_itemsize */
    PyHHN_NOTIFY::deallocFunc,                               /* tp_dealloc */
    0,                                                       /* tp_print */
    0,                                                       /* tp_getattr */
    0,                                                       /* tp_setattr */
    0,                                                       /* tp_compare */
    0,                                                       /* tp_repr */
    0,                                                       /* tp_as_number */
    0,                                                       /* tp_as_sequence */
    0,                                                       /* tp_as_mapping */
    0,                                                       /* tp_hash */
    0,                                                       /* tp_call */
    0,                                                       /* tp_str */
    PyHHN_NOTIFY::getattro,                                  /* tp_getattro */
    PyHHN_NOTIFY::setattro,                                  /* tp_setattro */
    0,                                                       /* tp_as_buffer */
    0,                                                       /* tp_flags */
    "A Python object, representing an HHN_NOTIFY structure", /* tp_doc */
    0,                                                       /* tp_traverse */
    0,                                                       /* tp_clear */
    0,                                                       /* tp_richcompare */
    0,                                                       /* tp_weaklistoffset */
    0,                                                       /* tp_iter */
    0,                                                       /* tp_iternext */
    0,                                                       /* tp_methods */
    PyHHN_NOTIFY::members,                                   /* tp_members */
    0,                                                       /* tp_getset */
    0,                                                       /* tp_base */
    0,                                                       /* tp_dict */
    0,                                                       /* tp_descr_get */
    0,                                                       /* tp_descr_set */
    0,                                                       /* tp_dictoffset */
    0,                                                       /* tp_init */
    0,                                                       /* tp_alloc */
    0,                                                       /* tp_new */
};

#undef OFF
#define OFF(e) offsetof(PyHHN_NOTIFY, e)

/*static*/ struct PyMemberDef PyHHN_NOTIFY::members[] = {

    //**************************************************************************
    //**************************************************************************
    // The following are added _ONLY_ so that they show up in a
    // dir() of the object, they are never handled via the memberlist.

    // NMHDR  hdr;
    // @prop NMHDR|hdr|Standard WM_NOTIFY header.(<om win32help.NMHDR>)
    {"hdr", T_STRING, OFF(m_HHN_NOTIFY.hdr)},

    // PCSTR  pszUrl;
    // @prop string|url|A multi-byte, zero-terminated string that specifies
    // the topic navigated to, or the name of the help window being created.
    {"url", T_STRING, OFF(m_HHN_NOTIFY.pszUrl)},
    //**************************************************************************
    //**************************************************************************

    {NULL} /* Sentinel */
};

PyHHN_NOTIFY::PyHHN_NOTIFY()
{
    ob_type = &PyHHN_NOTIFYType;
    _Py_NewReference(this);
    memset(&m_HHN_NOTIFY, 0, sizeof(m_HHN_NOTIFY));

    m_hdr = m_pszUrl = NULL;
}

PyHHN_NOTIFY::PyHHN_NOTIFY(const HHN_NOTIFY *pN_NOTIFY)
{
    ob_type = &PyHHN_NOTIFYType;
    _Py_NewReference(this);
    memcpy(&m_HHN_NOTIFY, pN_NOTIFY, sizeof(m_HHN_NOTIFY));

    m_hdr = new PyNMHDR(&pN_NOTIFY->hdr);
    // ??? This doesn't copy the string into the new struct ???
    m_pszUrl = pN_NOTIFY->pszUrl ? PyWinObject_FromTCHAR((TCHAR *)pN_NOTIFY->pszUrl) : NULL;
}

PyHHN_NOTIFY::~PyHHN_NOTIFY(void)
{
    Py_XDECREF(m_hdr);
    Py_XDECREF(m_pszUrl);
}

PyObject *PyHHN_NOTIFY::getattro(PyObject *self, PyObject *obname)
{
    char *name = PYWIN_ATTR_CONVERT(obname);
    if (name == NULL)
        return NULL;
    PyHHN_NOTIFY *pO = (PyHHN_NOTIFY *)self;

    if (strcmp("hdr", name) == 0) {
        PyObject *rc = pO->m_hdr ? pO->m_hdr : Py_None;
        Py_INCREF(rc);
        return rc;
    }
    if (strcmp("url", name) == 0) {
        PyObject *rc = pO->m_pszUrl ? pO->m_pszUrl : Py_None;
        Py_INCREF(rc);
        return rc;
    }

    return PyObject_GenericGetAttr(self, obname);
}

int PyHHN_NOTIFY::setattro(PyObject *self, PyObject *obname, PyObject *v)
{
    if (v == NULL) {
        PyErr_SetString(PyExc_AttributeError, "can't delete HHN_NOTIFY attributes");
        return -1;
    }
    char *name = PYWIN_ATTR_CONVERT(obname);
    if (name == NULL)
        return -1;

    PyHHN_NOTIFY *pO = (PyHHN_NOTIFY *)self;

    if (strcmp("hdr", name) == 0) {
        if (PyWinObject_AsNMHDR(v, (NMHDR **)&pO->m_HHN_NOTIFY.hdr, 0)) {
            Py_XDECREF(pO->m_hdr);
            pO->m_hdr = v;
            Py_INCREF(v);
            return 0;
        }
        else
            return -1;
    }
    if (strcmp("url", name) == 0) {
        if (PyWinObject_AsTCHAR(v, (TCHAR **)&pO->m_HHN_NOTIFY.pszUrl)) {
            Py_XDECREF(pO->m_pszUrl);
            pO->m_pszUrl = v;
            Py_INCREF(v);
            return 0;
        }
        else
            return -1;
    }
    return PyObject_GenericSetAttr(self, obname, v);
}

/*static*/ void PyHHN_NOTIFY::deallocFunc(PyObject *ob) { delete (PyHHN_NOTIFY *)ob; }

// A converter.
BOOL PyWinObject_AsHHN_NOTIFY(PyObject *ob, HHN_NOTIFY **ppN_NOTIFY, BOOL bNoneOK)
{
    if (bNoneOK && ob == Py_None) {
        *ppN_NOTIFY = NULL;
    }
    else if (!PyHHN_NOTIFY_Check(ob)) {
        PyErr_SetString(PyExc_TypeError, "The object is not a PyHHN_NOTIFY object");
        return FALSE;
    }
    else {
        *ppN_NOTIFY = ((PyHHN_NOTIFY *)ob)->GetN_NOTIFY();
    }
    return TRUE;
}

PyObject *PyWinObject_FromHHN_NOTIFY(const HHN_NOTIFY *pN_NOTIFY)
{
    if (pN_NOTIFY == NULL) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    PyObject *ret = new PyHHN_NOTIFY(pN_NOTIFY);
    if (ret == NULL)
        PyErr_SetString(PyExc_MemoryError, "PyHHN_NOTIFY");
    return ret;
}

//*****************************************************************************
//
// @pymethod <o PyHHN_NOTIFY>|win32help|HHN_NOTIFY|
// Creates a new HHN_NOTIFY object.

static PyObject *myHHN_NOTIFY(PyObject *self, PyObject *args)
{
    if (!PyArg_ParseTuple(args, ":HHN_NOTIFY"))
        return NULL;
    return new PyHHN_NOTIFY();
}

// Support for an HHNTRACK object.

class PyHHNTRACK : public PyObject {
   public:
    HHNTRACK *GetTRACK() { return &m_HHNTRACK; }

    PyHHNTRACK(void);
    PyHHNTRACK(const HHNTRACK *pTRACK);
    ~PyHHNTRACK();

    /* Python support */

    static void deallocFunc(PyObject *ob);

    static PyObject *getattro(PyObject *self, PyObject *obname);
    static int setattro(PyObject *self, PyObject *obname, PyObject *v);
    static struct PyMemberDef members[];

   protected:
    HHNTRACK m_HHNTRACK;
    PyObject *m_hdr;         // NMHDR       hdr;
    PyObject *m_pszCurUrl;   // PCSTR       pszCurUrl;
    PyObject *m_phhWinType;  // HH_WINTYPE* phhWinType;
};

#define PyHHNTRACK_Check(ob) ((ob)->ob_type == &PyHHNTRACKType)

// @object PyHHNTRACK|A Python object, representing an HHNTRACK
// structure
// @comm Typically you create a PyHHNTRACK
//(via <om win32help.HHNTRACK>) object, and set its properties.
// The object can then be passed to any function which takes an HHNTRACK
// object.<nl>
//<nl>
// This structure returns the file name of the current topic and a constant
// that specfies the user action that is about to occur, such as hiding the
// Navigation pane by clicking the Hide button on the toolbar.<nl>
//<nl>
// Used by<nl>
//<c HHN_TRACK><nl>

PyTypeObject PyHHNTRACKType = {
    PYWIN_OBJECT_HEAD "PyHHNTRACK",                         /* tp_name */
    sizeof(PyHHNTRACK),                                     /* tp_basicsize */
    0,                                                      /* tp_itemsize */
    PyHHNTRACK::deallocFunc,                                /* tp_dealloc */
    0,                                                      /* tp_print */
    0,                                                      /* tp_getattr */
    0,                                                      /* tp_setattr */
    0,                                                      /* tp_compare */
    0,                                                      /* tp_repr */
    0,                                                      /* tp_as_number */
    0,                                                      /* tp_as_sequence */
    0,                                                      /* tp_as_mapping */
    0,                                                      /* tp_hash */
    0,                                                      /* tp_call */
    0,                                                      /* tp_str */
    PyHHNTRACK::getattro,                                   /* tp_getattro */
    PyHHNTRACK::setattro,                                   /* tp_setattro */
    0,                                                      /* tp_as_buffer */
    0,                                                      /* tp_flags */
    "A Python object, representing an HHNTRACK structure.", /* tp_doc */
    0,                                                      /* tp_traverse */
    0,                                                      /* tp_clear */
    0,                                                      /* tp_richcompare */
    0,                                                      /* tp_weaklistoffset */
    0,                                                      /* tp_iter */
    0,                                                      /* tp_iternext */
    0,                                                      /* tp_methods */
    PyHHNTRACK::members,                                    /* tp_members */
    0,                                                      /* tp_getset */
    0,                                                      /* tp_base */
    0,                                                      /* tp_dict */
    0,                                                      /* tp_descr_get */
    0,                                                      /* tp_descr_set */
    0,                                                      /* tp_dictoffset */
    0,                                                      /* tp_init */
    0,                                                      /* tp_alloc */
    0,                                                      /* tp_new */
};

#undef OFF
#define OFF(e) offsetof(PyHHNTRACK, e)

/*static*/ struct PyMemberDef PyHHNTRACK::members[] = {

    // int         idAction;
    // @prop int|action|Specifies the action the user is about to take. This
    // is an HHACT_ constant.
    {"action", T_INT, OFF(m_HHNTRACK.idAction)},

    //**************************************************************************
    //**************************************************************************
    // The following are added _ONLY_ so that they show up in a
    // dir() of the object, they are never handled via the memberlist.

    // NMHDR  hdr;
    // @prop NMHDR|hdr|Standard WM_NOTIFY header(<om win32help.NMHDR>).
    {"hdr", T_STRING, OFF(m_HHNTRACK.hdr)},

    // PCSTR  pszCurUrl;
    // @prop string|curUrl|A multi-byte, zero-terminated string that specifies
    // the topic navigated to, or the name of the help window being created.
    {"curUrl", T_STRING, OFF(m_HHNTRACK.pszCurUrl)},

    // HH_WINTYPE* phhWinType;
    // @prop HH_WINTYPE|winType|A pointer to the current HH_WINTYPE structure
    //(<om win32help.HH_WINTYPE>).
    {"winType", T_STRING, OFF(m_HHNTRACK.phhWinType)},

    //**************************************************************************
    //**************************************************************************

    {NULL} /* Sentinel */
};

PyHHNTRACK::PyHHNTRACK()
{
    ob_type = &PyHHNTRACKType;
    _Py_NewReference(this);
    memset(&m_HHNTRACK, 0, sizeof(m_HHNTRACK));

    m_hdr = m_pszCurUrl = m_phhWinType = NULL;
}

PyHHNTRACK::PyHHNTRACK(const HHNTRACK *pTRACK)
{
    ob_type = &PyHHNTRACKType;
    _Py_NewReference(this);
    memcpy(&m_HHNTRACK, pTRACK, sizeof(m_HHNTRACK));

    m_hdr = new PyNMHDR(&pTRACK->hdr);

    m_pszCurUrl = pTRACK->pszCurUrl ? PyWinObject_FromTCHAR((TCHAR *)pTRACK->pszCurUrl) : NULL;

    m_phhWinType = new PyHH_WINTYPE(pTRACK->phhWinType);
}

PyHHNTRACK::~PyHHNTRACK(void)
{
    Py_XDECREF(m_hdr);
    Py_XDECREF(m_pszCurUrl);
    Py_XDECREF(m_phhWinType);
}

PyObject *PyHHNTRACK::getattro(PyObject *self, PyObject *obname)
{
    char *name = PYWIN_ATTR_CONVERT(obname);
    if (name == NULL)
        return NULL;

    PyHHNTRACK *pO = (PyHHNTRACK *)self;
    if (strcmp("hdr", name) == 0) {
        PyObject *rc = pO->m_hdr ? pO->m_hdr : Py_None;
        Py_INCREF(rc);
        return rc;
    }
    if (strcmp("curUrl", name) == 0) {
        PyObject *rc = pO->m_pszCurUrl ? pO->m_pszCurUrl : Py_None;
        Py_INCREF(rc);
        return rc;
    }
    if (strcmp("winType", name) == 0) {
        PyObject *rc = pO->m_phhWinType ? pO->m_phhWinType : Py_None;
        Py_INCREF(rc);
        return rc;
    }

    return PyObject_GenericGetAttr(self, obname);
}

int PyHHNTRACK::setattro(PyObject *self, PyObject *obname, PyObject *v)
{
    if (v == NULL) {
        PyErr_SetString(PyExc_AttributeError, "can't delete HHNTRACK attributes");
        return -1;
    }
    char *name = PYWIN_ATTR_CONVERT(obname);
    if (name == NULL)
        return -1;
    PyHHNTRACK *pO = (PyHHNTRACK *)self;

    if (strcmp("hdr", name) == 0) {
        if (PyWinObject_AsNMHDR(v, (NMHDR **)&pO->m_HHNTRACK.hdr, 0)) {
            Py_XDECREF(pO->m_hdr);
            pO->m_hdr = v;
            Py_INCREF(v);
            return 0;
        }
        else
            return -1;
    }
    if (strcmp("curUrl", name) == 0) {
        if (PyWinObject_AsTCHAR(v, (TCHAR **)&pO->m_HHNTRACK.pszCurUrl)) {
            Py_XDECREF(pO->m_pszCurUrl);
            pO->m_pszCurUrl = v;
            Py_INCREF(v);
            return 0;
        }
        else
            return -1;
    }
    if (strcmp("winType", name) == 0) {
        if (PyWinObject_AsHH_WINTYPE(v, (HH_WINTYPE **)&pO->m_HHNTRACK.phhWinType, 0)) {
            Py_XDECREF(pO->m_phhWinType);
            pO->m_phhWinType = v;
            Py_INCREF(v);
            return 0;
        }
        else
            return -1;
    }
    return PyObject_GenericSetAttr(self, obname, v);
}

/*static*/ void PyHHNTRACK::deallocFunc(PyObject *ob) { delete (PyHHNTRACK *)ob; }

// A converter.
BOOL PyWinObject_AsHHNTRACK(PyObject *ob, HHNTRACK **ppTRACK, BOOL bNoneOK)
{
    if (bNoneOK && ob == Py_None) {
        *ppTRACK = NULL;
    }
    else if (!PyHHNTRACK_Check(ob)) {
        PyErr_SetString(PyExc_TypeError, "The object is not a PyHHNTRACK object");
        return FALSE;
    }
    else {
        *ppTRACK = ((PyHHNTRACK *)ob)->GetTRACK();
    }
    return TRUE;
}

PyObject *PyWinObject_FromHHNTRACK(const HHNTRACK *pTRACK)
{
    if (pTRACK == NULL) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    PyObject *ret = new PyHHNTRACK(pTRACK);
    if (ret == NULL)
        PyErr_SetString(PyExc_MemoryError, "PyHHNTRACK");
    return ret;
}

//*****************************************************************************
//
// @pymethod <o PyHHNTRACK>|win32help|HHNTRACK|
// Creates a new HHNTRACK object.

static PyObject *myHHNTRACK(PyObject *self, PyObject *args)
{
    if (!PyArg_ParseTuple(args, ":HHNTRACK"))
        return NULL;
    return new PyHHNTRACK();
}

// HtmlHelp:

//*****************************************************************************
//
// @pymethod int|win32help|HtmlHelp|Invokes the Windows Html Help system.

static PyObject *PyHtmlHelp(PyObject *self, PyObject *args)
{
    // @pyparm int|hwnd||The handle of the window requesting help.
    // @pyparm string/None|file||The name of the help file, or None.
    // @pyparm int|cmd||The type of help.  Valid values are:<nl>
    //<nl>
    //<c HH_ALINK_LOOKUP>: Looks up one or more Associative link (ALink) names
    // in a compiled help (.chm) file.<nl>
    // The ALink names to search for, and the action to be taken if no matches
    // are found, are specified in the <om win32help.HH_AKLINK> structure.<nl>
    //<nl>
    //   <c file>: Specifies a compiled help (.chm) file, or a specific topic
    // within a compiled help file.<nl>
    //   <c data>: Specifies NULL or a pointer to a topic within a compiled help
    // file.<nl>
    //<nl>
    //<c HH_CLOSE_ALL>: Closes all windows opened directly or indirectly by the
    // calling program.  The args are not checked for type, values are set as
    // they "Must" be.<nl>
    //<nl>
    //   <c hwnd>: Must be None.<nl>
    //   <c file>: Must be None.<nl>
    //   <c data>: Must be zero.<nl>
    //<nl>
    //<c HH_DISPLAY_INDEX>: Selects the Index tab in the Navigation pane of the
    // HTML Help Viewer and searches for the keyword specified in the data
    // parameter.<nl>
    //<nl>
    //   <c file>: Specifies a compiled help (.chm) file, or a specific topic
    // within a compiled help file.<nl>
    //   <c data>: Specifies the keyword to select in the index (.hhk) file.<nl>
    //<nl>
    //<c HH_DISPLAY_SEARCH>: Selects the Search tab in the Navigation pane of
    // the HTML Help Viewer and performs a search for the term specified in the
    // searchQuery parameter of the <om win32help.HH_FTS_QUERY> structure.<nl>
    //<nl>
    //   <c file>: Specifies a compiled help (.chm) file, or a specific topic
    // within a compiled help file.<nl>
    //   <c data>: Specifies a pointer to an <om win32help.HH_FTS_QUERY>
    // structure.<nl>
    //<nl>
    //<c HH_DISPLAY_TEXT_POPUP>: Opens a pop-up window that displays the
    // contents of one of the following:<nl>
    // An explicit text string.<nl>
    // A text string based on a resource ID.<nl>
    // A text string ID based on a text file contained in a compiled help (.chm)
    // file.<nl>
    //<nl>
    //   <c file>: To use an explicit text string, use None.  To use a text
    // string from a resource, use None. To use text string from a text file
    // contained in a compiled help file, specify the .chm file and the text
    // file within the .chm file.<nl>
    //   <c data>: Specifies a pointer to an <om win32help.HH_POPUP> structure.
    //<nl>
    //<nl>
    //<c HH_DISPLAY_TOC>: Selects the Contents tab in the Navigation pane of
    // the HTML Help Viewer.<nl>
    //<nl>
    //   <c file>: Specifies a compiled help (.chm) file, or a specific topic
    // within a compiled help file.<nl>
    //   <c data>: Specifies None or a pointer to a topic within a compiled help
    // file.<nl>
    //<nl>
    //<c HH_DISPLAY_TOPIC>: Opens a help topic in a specified help window.<nl>
    // If a window type is not specified, a default window type is used. If the
    // window type or default window type is open, the help topic replaces the
    // current topic in the window.<nl>
    //<nl>
    //   <c file>: Specifies a compiled help (.chm) file, or a specific topic
    // within a compiled help file. To specify a defined window type, insert a
    // greater-than (>) character followed by the name of the window type.<nl>
    //   <c data>: Specifies None or a pointer to a topic within a compiled help
    // file.<nl>
    //<nl>
    //<c HH_GET_LAST_ERROR>: Returns information about the last error that
    // occurred in the HTML Help ActiveX control (Hhctrl.ocx).<nl>
    //<nl>
    //   <c file>: Must be None<nl>
    //   <c data>: A pointer to a HH_LAST_ERROR structure.<nl>
    //<nl>
    //   <c Has not been implemented by Microsoft yet.><nl>
    //<nl>
    //<c HH_GET_WIN_HANDLE>: Returns the handle (hwnd) of a specified window
    // type.<nl>
    //<nl>
    //   <c file>: Specifies the name of the compiled help (.chm) file in which
    // the window type is defined.<nl>
    //   <c data>: Specifies the name of the window type whose handle you want to
    // return.<nl>
    //<nl>
    //<c HH_GET_WIN_TYPE>: Retrieves a pointer to the <om win32help.HH_WINTYPE>
    // structure associated with a specified window type.<nl>
    //<nl>
    //   <c file>: Specifies the name of the window type whose information you
    // want to get and the name of the compiled help (.chm) file in which the
    // window type is defined. The window name must begin with a greater-than (>)
    // character and must be preceded by the name of the compiled help file it
    // is defined in.<nl>
    //   <c data>: Ignored.<nl>
    //<nl>
    //<c HH_HELP_CONTEXT>: Displays a help topic based on a mapped topic ID.
    // If a window type is not specified, a default window type is used. If the
    // window type or default window type is open, the help topic replaces the
    // current topic in the window.<nl>
    //<nl>
    //   <c file>: Specifies the compiled help (.chm) file that contains the
    // mapping information. To specify a defined window type, insert a
    // greater-than (>) character followed by the name of the window type.<nl>
    //   <c data>: Specifies the numeric ID of the topic to display. You must map
    // symbolic IDs of dialog boxes to numeric IDs in the [MAP] section of your
    // project (.hhp) file.<nl>
    //<nl>
    //<c HH_INITIALIZE>: This command initializes the help system for use and
    // must be the first HTML Help command called. It returns a cookie which must
    // be used in the <c HH_UNINITIALIZE> call. HH_INITIALIZE configures HTML Help
    // to run on the same thread as the calling application instead of a
    // secondary thread by setting the global property <c HH_GPROPID_SINGLETHREAD>
    // to VARIANT_TRUE. Running HTML Help on the same thread as the calling
    // application requires the calling application to send messages to HTML Help
    // by calling the <c HH_PRETRANSLATEMESSAGE> command.<nl>
    //<nl>
    //   <c file>: Must be None.<nl>
    //   <c data>: Ignored.<nl>
    //<nl>
    //<c HH_KEYWORD_LOOKUP>: Looks up one or more keywords in a compiled help
    //(.chm) file. The keywords to search for and the action to be taken if
    // no matches are found are specified in the <om win32help.HH_AKLINK>
    // structure.<nl>
    //<nl>
    //   <c file>: Specifies the compiled help (.chm) file that contains
    // keywords.<nl>
    //   <c data>: Points to an <om win32help.HH_AKLINK> structure.<nl>
    //<nl>
    //<c HH_PRETRANSLATEMESSAGE>: This command is called in the message loop
    // of your Windows application to ensure proper handling of Windows
    // messages, especially keyboard messages when running HTML Help single
    // thread. The HTML Help API is not thread safe and must be called from one
    // and only one thread in a process.<nl>
    //<nl>
    //   <c file>
    //   <c data>: Points to a Win32 MSG structure.<nl>
    //<nl>
    //   <c Has not been implemented yet.><nl>
    //<nl>
    //<c HH_SET_WIN_TYPE>: Creates a new help window or modifies an existing
    // help window at run time.<nl>
    //<nl>
    //   <c file>: Specifies the name of the window type that you want to
    // create or modify and the name of the compiled help (.chm) file in which
    // the window type is defined. The window type name must begin with a
    // greater-than (>) character and must be preceded by the name of the
    // compiled help file in which it is defined.<nl>
    //   <c data>: Points to an <om win32help.HH_WINTYPE> structure.<nl>
    //<nl>
    //<c HH_SYNC>: Locates and selects the contents entry for the help topic
    // that is open in the Topic pane of the HTML Help Viewer.<nl>
    //<nl>
    //   <c file>: Specifies the name of the window type that you want to
    // sync and the name of the compiled help (.chm) file in which the window
    // type is defined. The window type name must begin with a greater-than (>)
    // character and must be preceded by the name of the compiled help file in
    // which it is defined.<nl>
    //   <c data>: Specifies a pointer to a topic within a compiled help file.
    // This value is the topic file to which the contents will synchronize.<nl>
    //<nl>
    //<c HH_TP_HELP_CONTEXTMENU>: Opens a pop-up context menu. Generally used
    // in response to the Windows WM_CONTEXTMENU message. For example, this
    // message is sent when a user right-clicks a dialog box control.<nl>
    //<nl>
    //   <c hwnd>: Specifies the window handle of the dialog box control for
    // which you want pop-up help to appear. This is typically the control
    // that has focus. <nl>
    //   <c file>: Specifies the compiled help (.chm) file, and the text
    // file that contains the pop-up help topics. By default, the text file is
    // named Cshelp.txt. If Cshelp.txt is located in the root of the compiled
    // help file, then you only need to specify the help file name. If not,
    // you must also specify the relative path.<nl>
    //   <c data>: Specifies an array of DWORDs containing pairs of dialog
    // box control IDs and help topic IDs. The array must be terminated by zero,
    // as in the following example:<nl>
    // DWORD ids[3];<nl>
    //      ids[0] = ControlId;<nl>
    //      ids[1] = HelpId;<nl>
    //      ids[2] = 0;<nl>
    //<nl>
    //<c HH_TP_HELP_WM_HELP>: Opens a pop-up help topic. Generally used in
    // response to the Windows WM_HELP message. For example, this message is
    // sent when a user presses F1.<nl>
    //<nl>
    //   <c hwnd>: Specifies the window handle of the dialog box control for
    // which you want pop-up help to appear. This is typically the control that
    // has focus. <nl>
    //   <c file>: Specifies the compiled help (.chm) file, and the text file
    // that contains the pop-up help topics. By default, the text file is named
    // Cshelp.txt. If Cshelp.txt is located in the root of the compiled help
    // file, then you only need to specify the help file name. If not, you must
    // also specify the relative path.<nl>
    //   <c data>: Specifies an array of DWORDs containing pairs of dialog
    // box control IDs and help topic IDs. The array must be terminated by 0,
    // as in the following example:<nl>
    // DWORD ids[3];<nl>
    //      ids[0] = ControlId;<nl>
    //      ids[1] = HelpId;<nl>
    //      ids[2] = 0;<nl>
    //<nl>
    //<c HH_UNINITIALIZE>: This command is called to properly shut down HTML
    // Help. This function should be the last help command the application
    // calls. HH_UNINITIALIZE should not be called during DLL process detach,
    // but during the normal application shutdown process.  The type of the
    // file arg is not checked, just set to the value it "Must" be.<nl>
    //<nl>
    //   <c file>: Must be None.<nl>
    //   <c data>: Specifies a cookie. This is the cookie returned by
    //<c HH_INITIALIZE>. <nl>
    //<nl>
    // @pyparm None/int/string/int tuple/<om win32help.HH_AKLINK>/
    //<om win32help.HH_FTS_QUERY>/<om win32help.HH_POPUP>/
    //<om win32help.HH_WINTYPE>|
    // data|0|Additional data specific to the help call.

    HWND hwnd;
    TCHAR *file;
    UINT cmd;
    PyObject *fileOb;
    PyObject *dataOb = Py_None;
    DWORD_PTR data;
    TCHAR *dataObAsTCHAR = NULL;
    HH_AKLINK *pAKLink;
    HH_FTS_QUERY *pFTS_Query;
    HH_POPUP *pPopup;
    HH_WINTYPE *pWinType;
    DWORD dwCookie = 0;
    DWORD *ctlIDs = NULL;
    int i;
    BOOL error = FALSE;
    DWORD len;
    PyObject *item = NULL;

    if (!PyArg_ParseTuple(args, "O&Oi|O:HtmlHelp", PyWinObject_AsHANDLE, &hwnd, &fileOb, &cmd, &dataOb))
        return NULL;

    if (!PyWinObject_AsTCHAR(fileOb, &file, TRUE, &len))
        return NULL;
    if (len >= _MAX_PATH)
        return PyErr_Format(PyExc_ValueError, "string of length %d is too large for this function", len);

    switch (cmd) {
        case HH_ALINK_LOOKUP:
        case HH_KEYWORD_LOOKUP:
            if (!file) {
                PyErr_SetString(PyExc_TypeError,
                                "HH_ALINK_LOOKUP and HH_KEYWORD_LOOKUP \
file must be a string");
                return NULL;
            }
            if (!PyWinObject_AsHH_AKLINK(dataOb, &pAKLink, 0)) {
                PyErr_SetString(PyExc_TypeError,
                                "HH_ALINK_LOOKUP and HH_KEYWORD_LOOKUP \
cmd data must be a PyHH_AKLINK object");
                return NULL;
            }
            data = (DWORD_PTR)pAKLink;
            break;

        case HH_CLOSE_ALL:
            hwnd = NULL;
            file = NULL;
            data = NULL;
            break;

        case HH_DISPLAY_INDEX:
        case HH_GET_WIN_HANDLE:
        case HH_SYNC:
            if (!file) {
                PyErr_SetString(PyExc_TypeError,
                                "HH_DISPLAY_INDEX, HH_GET_WIN_HANDLE and HH_SYNC \
file must be a string");
                return NULL;
            }
            if (!PyWinObject_AsTCHAR(dataOb, &dataObAsTCHAR, FALSE, NULL))
                return NULL;
            data = (DWORD_PTR)dataObAsTCHAR;
            break;

        case HH_DISPLAY_SEARCH:
            if (!file) {
                PyErr_SetString(PyExc_TypeError, "HH_DISPLAY_SEARCH file must be a string");
                return NULL;
            }
            if (!PyWinObject_AsHH_FTS_QUERY(dataOb, &pFTS_Query, 0)) {
                PyErr_SetString(PyExc_TypeError, "HH_DISPLAY_SEARCH data must be a PyHH_FTS_QUERY object");
                return NULL;
            }
            data = (DWORD_PTR)pFTS_Query;
            break;

        case HH_DISPLAY_TEXT_POPUP:
            if (!PyWinObject_AsHH_POPUP(dataOb, &pPopup, 0)) {
                PyErr_SetString(PyExc_TypeError, "HH_DISPLAY_TEXT_POPUP data must be a PyHH_POPUP object");
                return NULL;
            }
            data = (DWORD_PTR)pPopup;
            break;

        case HH_DISPLAY_TOC:
        case HH_DISPLAY_TOPIC:
            if (!file) {
                PyErr_SetString(PyExc_TypeError,
                                "HH_DISPLAY_TOC and HH_DISPLAY_TOPIC \
file must be a string");
                return NULL;
            }
            if (dataOb == Py_None) {
                data = 0;
            }
            else {
                if (!PyWinObject_AsTCHAR(dataOb, &dataObAsTCHAR, FALSE, NULL))
                    return NULL;
                data = (DWORD_PTR)dataObAsTCHAR;
            }
            break;

        case HH_GET_LAST_ERROR:
            PyErr_SetString(PyExc_NotImplementedError,
                            "HH_GET_LAST_ERROR not implemented \
in the Html Help engine yet.");
            return NULL;
            break;

        case HH_GET_WIN_TYPE:
            if (!file) {
                PyErr_SetString(PyExc_TypeError, "HH_GET_WIN_TYPE file must be a string");
                return NULL;
            }
            data = (DWORD_PTR)&pWinType;
            break;

        case HH_HELP_CONTEXT:
            if (!file) {
                PyErr_SetString(PyExc_TypeError, "HH_HELP_CONTEXT file must be a string");
                return NULL;
            }
            if (!PyLong_Check(dataOb)) {
                PyErr_SetString(PyExc_TypeError, "HH_HELP_CONTEXT data must be an integer");
                return NULL;
            }
            data = (DWORD_PTR)PyLong_AsLong(dataOb);
            break;

        case HH_INITIALIZE:
            file = NULL;
            data = (DWORD_PTR)&dwCookie;
            break;

        case HH_PRETRANSLATEMESSAGE:
            PyErr_SetString(PyExc_NotImplementedError, "HH_PRETRANSLATEMESSAGE not implemented in win32help yet");
            return NULL;
            break;

        case HH_SET_WIN_TYPE:
            if (!file) {
                PyErr_SetString(PyExc_TypeError, "HH_SET_WIN_TYPE file must be a string");
                return NULL;
            }
            if (!PyWinObject_AsHH_WINTYPE(dataOb, &pWinType, 0)) {
                PyErr_SetString(PyExc_TypeError, "HH_SET_WIN_TYPE data must be a PyHH_WINTYPE object");
                return NULL;
            }
            data = (DWORD_PTR)pWinType;
            break;

        case HH_TP_HELP_CONTEXTMENU:
        case HH_TP_HELP_WM_HELP:
            if (!file) {
                PyErr_SetString(PyExc_TypeError,
                                "HH_TP_HELP_CONTEXTMENU and HH_TP_HELP_WM_HELP "
                                "file must be a string");
                return NULL;
            }
            if (!PyTuple_Check(dataOb)) {
                PyErr_SetString(PyExc_TypeError,
                                "HH_TP_HELP_CONTEXTMENU and HH_TP_HELP_WM_HELP "
                                "data must be a tuple");
                return NULL;
            }
            len = PyTuple_Size(dataOb);
            if ((len % 2) != 0) {
                PyErr_SetString(PyExc_TypeError,
                                "HH_TP_HELP_CONTEXTMENU and HH_TP_HELP_WM_HELP "
                                "data tuple length must be even");
                return NULL;
            }
            ctlIDs = new DWORD[len + 1];
            for (i = 0; i < len; i++) {
                item = PyTuple_GetItem(dataOb, i);
                if (!PyLong_Check(item))
                    error = TRUE;
                else
                    ctlIDs[i] = PyLong_AsLong(item);
            }
            if (error) {
                delete[] ctlIDs;
                PyErr_SetString(PyExc_TypeError,
                                "HH_TP_HELP_CONTEXTMENU and HH_TP_HELP_WM_HELP \
data tuple items must be integers");
                return NULL;
            }
            data = (DWORD_PTR)ctlIDs;
            break;

        case HH_UNINITIALIZE:
            file = NULL;
            if (!PyLong_Check(dataOb)) {
                PyErr_SetString(PyExc_TypeError, "HH_UNINITIALIZE data must be an integer");
                return NULL;
            }
            data = (DWORD_PTR)PyLong_AsLong(dataOb);
            break;

        default:
            PyErr_SetString(PyExc_TypeError, "Unrecognized cmd");
            return NULL;
            break;
    }

    HWND helpWnd;
    PyW32_BEGIN_ALLOW_THREADS;
    helpWnd = ::HtmlHelp(hwnd, file, cmd, data);
    PyW32_END_ALLOW_THREADS;

    PyWinObject_FreeTCHAR(dataObAsTCHAR);
    PyWinObject_FreeTCHAR(file);

    PyObject *ret;

    switch (cmd) {
        case HH_GET_WIN_TYPE:
            ret = PyTuple_New(2);
            PyTuple_SetItem(ret, 0, Py_BuildValue("i", helpWnd));
            PyTuple_SetItem(ret, 1, new PyHH_WINTYPE(pWinType));
            break;

        case HH_INITIALIZE:
            ret = PyTuple_New(2);
            PyTuple_SetItem(ret, 0, Py_BuildValue("i", helpWnd));
            PyTuple_SetItem(ret, 1, Py_BuildValue("i", dwCookie));
            break;

        default:
            ret = Py_BuildValue("i", helpWnd);
            break;
    }

    if (cmd == HH_TP_HELP_CONTEXTMENU || cmd == HH_TP_HELP_WM_HELP) {
        delete[] ctlIDs;
    }

    return ret;

    // @pyseeapi HtmlHelp

    // @rdesc Depending on the specified cmd and the result:<nl>
    //<nl>
    //<c HH_GET_WIN_TYPE>:<nl>
    // tuple: (hwnd as below, and the <om win32help.HH_WINTYPE> object).<nl>
    // Deep copy the structure to which dwData points before modifying the
    // structure.<nl>
    //<nl>
    //<c HH_INITIALIZE>:<nl>
    // tuple: (hwnd as below, and the cookie).<nl>
    // This call returns a cookie that you must pass as the value of
    // data when you call <c HH_UNINITIALIZE>.<nl>
    //<nl>
    //<nl>
    //<c All other commands>:<nl>
    //<nl>
    // HtmlHelp() returns one or both of the following:<nl>
    // The handle (hwnd) of the help window.<nl>
    // NULL. In some cases, NULL indicates failure; in other cases, NULL
    // indicates that the help window has not yet been created.
}

// Module constants:
#define ADD_CONSTANT(tok)                                \
    if (rc = PyModule_AddIntConstant(module, #tok, tok)) \
    return rc

int AddConstants(PyObject *module)
{
    int rc;

#ifdef _DEBUG
    int debug = 1;
#else
    int debug = 0;
#endif

    ADD_CONSTANT(debug);
    // @const win32help|debug|1 if we are current using a _DEBUG build of
    // win32help, else 0.

    ADD_CONSTANT(HH_DISPLAY_TOPIC);
    // @const win32help|HH_DISPLAY_TOPIC|

    ADD_CONSTANT(HH_HELP_FINDER);
    // @const win32help|HH_HELP_FINDER|WinHelp equivalent

    ADD_CONSTANT(HH_DISPLAY_TOC);
    // @const win32help|HH_DISPLAY_TOC|not currently implemented

    ADD_CONSTANT(HH_DISPLAY_INDEX);
    // @const win32help|HH_DISPLAY_INDEX|not currently implemented

    ADD_CONSTANT(HH_DISPLAY_SEARCH);
    // @const win32help|HH_DISPLAY_SEARCH|not currently implemented

    ADD_CONSTANT(HH_SET_WIN_TYPE);
    // @const win32help|HH_SET_WIN_TYPE|

    ADD_CONSTANT(HH_GET_WIN_TYPE);
    // @const win32help|HH_GET_WIN_TYPE|

    ADD_CONSTANT(HH_GET_WIN_HANDLE);
    // @const win32help|HH_GET_WIN_HANDLE|

    ADD_CONSTANT(HH_ENUM_INFO_TYPE);
    // @const win32help|HH_ENUM_INFO_TYPE|Get Info type name, call
    // repeatedly to enumerate, -1 at end

    ADD_CONSTANT(HH_SET_INFO_TYPE);
    // @const win32help|HH_SET_INFO_TYPE|Add Info type to filter.

    ADD_CONSTANT(HH_SYNC);
    // @const win32help|HH_SYNC|

    ADD_CONSTANT(HH_RESERVED1);
    // @const win32help|HH_RESERVED1|

    ADD_CONSTANT(HH_RESERVED2);
    // @const win32help|HH_RESERVED2|

    ADD_CONSTANT(HH_RESERVED3);
    // @const win32help|HH_RESERVED3|

    ADD_CONSTANT(HH_KEYWORD_LOOKUP);
    // @const win32help|HH_KEYWORD_LOOKUP|

    ADD_CONSTANT(HH_DISPLAY_TEXT_POPUP);
    // @const win32help|HH_DISPLAY_TEXT_POPUP|display string resource id
    // or text in a popup window

    ADD_CONSTANT(HH_HELP_CONTEXT);
    // @const win32help|HH_HELP_CONTEXT|display mapped numeric value in
    // dwData

    ADD_CONSTANT(HH_TP_HELP_CONTEXTMENU);
    // @const win32help|HH_TP_HELP_CONTEXTMENU|text popup help, same as
    // WinHelp HELP_CONTEXTMENU

    ADD_CONSTANT(HH_TP_HELP_WM_HELP);
    // @const win32help|HH_TP_HELP_WM_HELP|text popup help, same as
    // WinHelp HELP_WM_HELP

    ADD_CONSTANT(HH_CLOSE_ALL);
    // @const win32help|HH_CLOSE_ALL|close all windows opened directly or
    // indirectly by the caller

    ADD_CONSTANT(HH_ALINK_LOOKUP);
    // @const win32help|HH_ALINK_LOOKUP|ALink version of HH_KEYWORD_LOOKUP

    ADD_CONSTANT(HH_GET_LAST_ERROR);
    // @const win32help|HH_GET_LAST_ERROR|not currently implemented
    // See HHERROR.h

    ADD_CONSTANT(HH_ENUM_CATEGORY);
    // @const win32help|HH_ENUM_CATEGORY|Get category name, call
    // repeatedly to enumerate, -1 at end

    ADD_CONSTANT(HH_ENUM_CATEGORY_IT);
    // @const win32help|HH_ENUM_CATEGORY_IT|Get category info type members,
    // call repeatedly to enumerate, -1 at end

    ADD_CONSTANT(HH_RESET_IT_FILTER);
    // @const win32help|HH_RESET_IT_FILTER|Clear the info type filter of
    // all info types.

    ADD_CONSTANT(HH_SET_INCLUSIVE_FILTER);
    // @const win32help|HH_SET_INCLUSIVE_FILTER|set inclusive filtering
    // method for untyped topics to be included in display

    ADD_CONSTANT(HH_SET_EXCLUSIVE_FILTER);
    // @const win32help|HH_SET_EXCLUSIVE_FILTER|set exclusive filtering
    // method for untyped topics to be excluded from display

    ADD_CONSTANT(HH_INITIALIZE);
    // @const win32help|HH_INITIALIZE|Initializes the help system.

    ADD_CONSTANT(HH_UNINITIALIZE);
    // @const win32help|HH_UNINITIALIZE|Uninitializes the help system.

    ADD_CONSTANT(HH_PRETRANSLATEMESSAGE);
    // @const win32help|HH_PRETRANSLATEMESSAGE|Pumps messages.
    //(NULL, NULL, MSG*).

    ADD_CONSTANT(HH_SET_GLOBAL_PROPERTY);
    // @const win32help|HH_SET_GLOBAL_PROPERTY|Set a global property.
    //(NULL, NULL, HH_GPROP).

    ADD_CONSTANT(HHWIN_PROP_TAB_AUTOHIDESHOW);
    // @const win32help|HHWIN_PROP_TAB_AUTOHIDESHOW|Automatically hide/show
    // tri-pane window

    ADD_CONSTANT(HHWIN_PROP_ONTOP);
    // @const win32help|HHWIN_PROP_ONTOP|Top-most window

    ADD_CONSTANT(HHWIN_PROP_NOTITLEBAR);
    // @const win32help|HHWIN_PROP_NOTITLEBAR|no title bar

    ADD_CONSTANT(HHWIN_PROP_NODEF_STYLES);
    // @const win32help|HHWIN_PROP_NODEF_STYLES|no default window styles
    //(only HH_WINTYPE.styles)

    ADD_CONSTANT(HHWIN_PROP_NODEF_EXSTYLES);
    // @const win32help|HHWIN_PROP_NODEF_EXSTYLES|no default extended
    // window styles (only HH_WINTYPE.exStyles)

    ADD_CONSTANT(HHWIN_PROP_TRI_PANE);
    // @const win32help|HHWIN_PROP_TRI_PANE|use a tri-pane window

    ADD_CONSTANT(HHWIN_PROP_NOTB_TEXT);
    // @const win32help|HHWIN_PROP_NOTB_TEXT|no text on toolbar buttons

    ADD_CONSTANT(HHWIN_PROP_POST_QUIT);
    // @const win32help|HHWIN_PROP_POST_QUIT|post WM_QUIT message when
    // window closes

    ADD_CONSTANT(HHWIN_PROP_AUTO_SYNC);
    // @const win32help|HHWIN_PROP_AUTO_SYNC|automatically ssync contents
    // and index

    ADD_CONSTANT(HHWIN_PROP_TRACKING);
    // @const win32help|HHWIN_PROP_TRACKING|send tracking notification
    // messages

    ADD_CONSTANT(HHWIN_PROP_TAB_SEARCH);
    // @const win32help|HHWIN_PROP_TAB_SEARCH|include search tab in
    // navigation pane

    ADD_CONSTANT(HHWIN_PROP_TAB_HISTORY);
    // @const win32help|HHWIN_PROP_TAB_HISTORY|include history tab in
    // navigation pane

    ADD_CONSTANT(HHWIN_PROP_TAB_FAVORITES);
    // @const win32help|HHWIN_PROP_TAB_FAVORITES|include favorites tab in
    // navigation pane

    ADD_CONSTANT(HHWIN_PROP_CHANGE_TITLE);
    // @const win32help|HHWIN_PROP_CHANGE_TITLE|Put current HTML title in
    // title bar

    ADD_CONSTANT(HHWIN_PROP_NAV_ONLY_WIN);
    // @const win32help|HHWIN_PROP_NAV_ONLY_WIN|Only display the navigation
    // window

    ADD_CONSTANT(HHWIN_PROP_NO_TOOLBAR);
    // @const win32help|HHWIN_PROP_NO_TOOLBAR|Don't display a toolbar

    ADD_CONSTANT(HHWIN_PROP_MENU);
    // @const win32help|HHWIN_PROP_MENU|Menu

    ADD_CONSTANT(HHWIN_PROP_TAB_ADVSEARCH);
    // @const win32help|HHWIN_PROP_TAB_ADVSEARCH|Advanced FTS UI.

    ADD_CONSTANT(HHWIN_PROP_USER_POS);
    // @const win32help|HHWIN_PROP_USER_POS|After initial creation, user
    // controls window size/position

    ADD_CONSTANT(HHWIN_PROP_TAB_CUSTOM1);
    // @const win32help|HHWIN_PROP_TAB_CUSTOM1|Use custom tab #1

    ADD_CONSTANT(HHWIN_PROP_TAB_CUSTOM2);
    // @const win32help|HHWIN_PROP_TAB_CUSTOM2|Use custom tab #2

    ADD_CONSTANT(HHWIN_PROP_TAB_CUSTOM3);
    // @const win32help|HHWIN_PROP_TAB_CUSTOM3|Use custom tab #3

    ADD_CONSTANT(HHWIN_PROP_TAB_CUSTOM4);
    // @const win32help|HHWIN_PROP_TAB_CUSTOM4|Use custom tab #4

    ADD_CONSTANT(HHWIN_PROP_TAB_CUSTOM5);
    // @const win32help|HHWIN_PROP_TAB_CUSTOM5|Use custom tab #5

    ADD_CONSTANT(HHWIN_PROP_TAB_CUSTOM6);
    // @const win32help|HHWIN_PROP_TAB_CUSTOM6|Use custom tab #6

    ADD_CONSTANT(HHWIN_PROP_TAB_CUSTOM7);
    // @const win32help|HHWIN_PROP_TAB_CUSTOM7|Use custom tab #7

    ADD_CONSTANT(HHWIN_PROP_TAB_CUSTOM8);
    // @const win32help|HHWIN_PROP_TAB_CUSTOM8|Use custom tab #8

    ADD_CONSTANT(HHWIN_PROP_TAB_CUSTOM9);
    // @const win32help|HHWIN_PROP_TAB_CUSTOM9|Use custom tab #9

    ADD_CONSTANT(HHWIN_TB_MARGIN);
    // @const win32help|HHWIN_TB_MARGIN|the window type has a margin

    ADD_CONSTANT(HHWIN_PARAM_PROPERTIES);
    // @const win32help|HHWIN_PARAM_PROPERTIES|valid winProperties

    ADD_CONSTANT(HHWIN_PARAM_STYLES);
    // @const win32help|HHWIN_PARAM_STYLES|valid styles

    ADD_CONSTANT(HHWIN_PARAM_EXSTYLES);
    // @const win32help|HHWIN_PARAM_EXSTYLES|valid exStyles

    ADD_CONSTANT(HHWIN_PARAM_RECT);
    // @const win32help|HHWIN_PARAM_RECT|valid windowPos

    ADD_CONSTANT(HHWIN_PARAM_NAV_WIDTH);
    // @const win32help|HHWIN_PARAM_NAV_WIDTH|valid navWidth

    ADD_CONSTANT(HHWIN_PARAM_SHOWSTATE);
    // @const win32help|HHWIN_PARAM_SHOWSTATE|valid showState

    ADD_CONSTANT(HHWIN_PARAM_INFOTYPES);
    // @const win32help|HHWIN_PARAM_INFOTYPES|valid apInfoTypes

    ADD_CONSTANT(HHWIN_PARAM_TB_FLAGS);
    // @const win32help|HHWIN_PARAM_TB_FLAGS|valid toolBarFlags

    ADD_CONSTANT(HHWIN_PARAM_EXPANSION);
    // @const win32help|HHWIN_PARAM_EXPANSION|valid notExpanded

    ADD_CONSTANT(HHWIN_PARAM_TABPOS);
    // @const win32help|HHWIN_PARAM_TABPOS|valid tabpos

    ADD_CONSTANT(HHWIN_PARAM_TABORDER);
    // @const win32help|HHWIN_PARAM_TABORDER|valid taborder

    ADD_CONSTANT(HHWIN_PARAM_HISTORY_COUNT);
    // @const win32help|HHWIN_PARAM_HISTORY_COUNT|valid cHistory

    ADD_CONSTANT(HHWIN_PARAM_CUR_TAB);
    // @const win32help|HHWIN_PARAM_CUR_TAB|valid curNavType

    ADD_CONSTANT(HHWIN_BUTTON_EXPAND);
    // @const win32help|HHWIN_BUTTON_EXPAND|Expand/contract button

    ADD_CONSTANT(HHWIN_BUTTON_BACK);
    // @const win32help|HHWIN_BUTTON_BACK|Back button

    ADD_CONSTANT(HHWIN_BUTTON_FORWARD);
    // @const win32help|HHWIN_BUTTON_FORWARD|Forward button

    ADD_CONSTANT(HHWIN_BUTTON_STOP);
    // @const win32help|HHWIN_BUTTON_STOP|Stop button

    ADD_CONSTANT(HHWIN_BUTTON_REFRESH);
    // @const win32help|HHWIN_BUTTON_REFRESH|Refresh button

    ADD_CONSTANT(HHWIN_BUTTON_HOME);
    // @const win32help|HHWIN_BUTTON_HOME|Home button

    ADD_CONSTANT(HHWIN_BUTTON_BROWSE_FWD);
    // @const win32help|HHWIN_BUTTON_BROWSE_FWD|not implemented

    ADD_CONSTANT(HHWIN_BUTTON_BROWSE_BCK);
    // @const win32help|HHWIN_BUTTON_BROWSE_BCK|not implemented

    ADD_CONSTANT(HHWIN_BUTTON_NOTES);
    // @const win32help|HHWIN_BUTTON_NOTES|not implemented

    ADD_CONSTANT(HHWIN_BUTTON_CONTENTS);
    // @const win32help|HHWIN_BUTTON_CONTENTS|not implemented

    ADD_CONSTANT(HHWIN_BUTTON_SYNC);
    // @const win32help|HHWIN_BUTTON_SYNC|Sync button

    ADD_CONSTANT(HHWIN_BUTTON_OPTIONS);
    // @const win32help|HHWIN_BUTTON_OPTIONS|Options button

    ADD_CONSTANT(HHWIN_BUTTON_PRINT);
    // @const win32help|HHWIN_BUTTON_PRINT|Print button

    ADD_CONSTANT(HHWIN_BUTTON_INDEX);
    // @const win32help|HHWIN_BUTTON_INDEX|not implemented

    ADD_CONSTANT(HHWIN_BUTTON_SEARCH);
    // @const win32help|HHWIN_BUTTON_SEARCH|not implemented

    ADD_CONSTANT(HHWIN_BUTTON_HISTORY);
    // @const win32help|HHWIN_BUTTON_HISTORY|not implemented

    ADD_CONSTANT(HHWIN_BUTTON_FAVORITES);
    // @const win32help|HHWIN_BUTTON_FAVORITES|not implemented

    ADD_CONSTANT(HHWIN_BUTTON_JUMP1);
    // @const win32help|HHWIN_BUTTON_JUMP1|

    ADD_CONSTANT(HHWIN_BUTTON_JUMP2);
    // @const win32help|HHWIN_BUTTON_JUMP2|

    ADD_CONSTANT(HHWIN_BUTTON_ZOOM);
    // @const win32help|HHWIN_BUTTON_ZOOM|

    ADD_CONSTANT(HHWIN_BUTTON_TOC_NEXT);
    // @const win32help|HHWIN_BUTTON_TOC_NEXT|

    ADD_CONSTANT(HHWIN_BUTTON_TOC_PREV);
    // @const win32help|HHWIN_BUTTON_TOC_PREV|

    ADD_CONSTANT(HHWIN_DEF_BUTTONS);
    // @const win32help|HHWIN_DEF_BUTTONS|

    ADD_CONSTANT(IDTB_EXPAND);
    // @const win32help|IDTB_EXPAND|

    ADD_CONSTANT(IDTB_CONTRACT);
    // @const win32help|IDTB_CONTRACT|

    ADD_CONSTANT(IDTB_STOP);
    // @const win32help|IDTB_STOP|

    ADD_CONSTANT(IDTB_REFRESH);
    // @const win32help|IDTB_REFRESH|

    ADD_CONSTANT(IDTB_BACK);
    // @const win32help|IDTB_BACK|

    ADD_CONSTANT(IDTB_HOME);
    // @const win32help|IDTB_HOME|

    ADD_CONSTANT(IDTB_SYNC);
    // @const win32help|IDTB_SYNC|

    ADD_CONSTANT(IDTB_PRINT);
    // @const win32help|IDTB_PRINT|

    ADD_CONSTANT(IDTB_OPTIONS);
    // @const win32help|IDTB_OPTIONS|

    ADD_CONSTANT(IDTB_FORWARD);
    // @const win32help|IDTB_FORWARD|

    ADD_CONSTANT(IDTB_NOTES);
    // @const win32help|IDTB_NOTES|not implemented

    ADD_CONSTANT(IDTB_BROWSE_FWD);
    // @const win32help|IDTB_BROWSE_FWD|

    ADD_CONSTANT(IDTB_BROWSE_BACK);
    // @const win32help|IDTB_BROWSE_BACK|

    ADD_CONSTANT(IDTB_CONTENTS);
    // @const win32help|IDTB_CONTENTS|not implemented

    ADD_CONSTANT(IDTB_INDEX);
    // @const win32help|IDTB_INDEX|not implemented

    ADD_CONSTANT(IDTB_SEARCH);
    // @const win32help|IDTB_SEARCH|not implemented

    ADD_CONSTANT(IDTB_HISTORY);
    // @const win32help|IDTB_HISTORY|not implemented

    ADD_CONSTANT(IDTB_FAVORITES);
    // @const win32help|IDTB_FAVORITES|not implemented

    ADD_CONSTANT(IDTB_JUMP1);
    // @const win32help|IDTB_JUMP1|

    ADD_CONSTANT(IDTB_JUMP2);
    // @const win32help|IDTB_JUMP2|

    ADD_CONSTANT(IDTB_CUSTOMIZE);
    // @const win32help|IDTB_CUSTOMIZE|

    ADD_CONSTANT(IDTB_ZOOM);
    // @const win32help|IDTB_ZOOM|

    ADD_CONSTANT(IDTB_TOC_NEXT);
    // @const win32help|IDTB_TOC_NEXT|

    ADD_CONSTANT(IDTB_TOC_PREV);
    // @const win32help|IDTB_TOC_PREV|

    ADD_CONSTANT(HHN_FIRST);
    // @const win32help|HHN_FIRST|

    ADD_CONSTANT(HHN_LAST);
    // @const win32help|HHN_LAST|

    ADD_CONSTANT(HHN_NAVCOMPLETE);
    // @const win32help|HHN_NAVCOMPLETE|

    ADD_CONSTANT(HHN_TRACK);
    // @const win32help|HHN_TRACK|

    ADD_CONSTANT(HHN_WINDOW_CREATE);
    // @const win32help|HHN_WINDOW_CREATE|

    ADD_CONSTANT(HHWIN_NAVTAB_TOP);
    // @const win32help|HHWIN_NAVTAB_TOP|

    ADD_CONSTANT(HHWIN_NAVTAB_LEFT);
    // @const win32help|HHWIN_NAVTAB_LEFT|

    ADD_CONSTANT(HHWIN_NAVTAB_BOTTOM);
    // @const win32help|HHWIN_NAVTAB_BOTTOM|

    ADD_CONSTANT(HH_TAB_CONTENTS);
    // @const win32help|HH_TAB_CONTENTS|

    ADD_CONSTANT(HH_TAB_INDEX);
    // @const win32help|HH_TAB_INDEX|

    ADD_CONSTANT(HH_TAB_SEARCH);
    // @const win32help|HH_TAB_SEARCH|

    ADD_CONSTANT(HH_TAB_FAVORITES);
    // @const win32help|HH_TAB_FAVORITES|

    ADD_CONSTANT(HH_TAB_HISTORY);
    // @const win32help|HH_TAB_HISTORY|

    ADD_CONSTANT(HH_TAB_AUTHOR);
    // @const win32help|HH_TAB_AUTHOR|

    ADD_CONSTANT(HH_TAB_CUSTOM_FIRST);
    // @const win32help|HH_TAB_CUSTOM_FIRST|

    ADD_CONSTANT(HH_TAB_CUSTOM_LAST);
    // @const win32help|HH_TAB_CUSTOM_LAST|

    ADD_CONSTANT(HH_MAX_TABS_CUSTOM);
    // @const win32help|HH_MAX_TABS_CUSTOM|

    ADD_CONSTANT(HH_FTS_DEFAULT_PROXIMITY);
    // @const win32help|HH_FTS_DEFAULT_PROXIMITY|

    ADD_CONSTANT(HHACT_TAB_CONTENTS);
    // @const win32help|HHACT_TAB_CONTENTS|

    ADD_CONSTANT(HHACT_TAB_INDEX);
    // @const win32help|HHACT_TAB_INDEX|

    ADD_CONSTANT(HHACT_TAB_SEARCH);
    // @const win32help|HHACT_TAB_SEARCH|

    ADD_CONSTANT(HHACT_TAB_HISTORY);
    // @const win32help|HHACT_TAB_HISTORY|

    ADD_CONSTANT(HHACT_TAB_FAVORITES);
    // @const win32help|HHACT_TAB_FAVORITES|

    ADD_CONSTANT(HHACT_EXPAND);
    // @const win32help|HHACT_EXPAND|

    ADD_CONSTANT(HHACT_CONTRACT);
    // @const win32help|HHACT_CONTRACT|

    ADD_CONSTANT(HHACT_BACK);
    // @const win32help|HHACT_BACK|

    ADD_CONSTANT(HHACT_FORWARD);
    // @const win32help|HHACT_FORWARD|

    ADD_CONSTANT(HHACT_STOP);
    // @const win32help|HHACT_STOP|

    ADD_CONSTANT(HHACT_REFRESH);
    // @const win32help|HHACT_REFRESH|

    ADD_CONSTANT(HHACT_HOME);
    // @const win32help|HHACT_HOME|

    ADD_CONSTANT(HHACT_SYNC);
    // @const win32help|HHACT_SYNC|

    ADD_CONSTANT(HHACT_OPTIONS);
    // @const win32help|HHACT_OPTIONS|

    ADD_CONSTANT(HHACT_PRINT);
    // @const win32help|HHACT_PRINT|

    ADD_CONSTANT(HHACT_HIGHLIGHT);
    // @const win32help|HHACT_HIGHLIGHT|

    ADD_CONSTANT(HHACT_CUSTOMIZE);
    // @const win32help|HHACT_CUSTOMIZE|

    ADD_CONSTANT(HHACT_JUMP1);
    // @const win32help|HHACT_JUMP1|

    ADD_CONSTANT(HHACT_JUMP2);
    // @const win32help|HHACT_JUMP2|

    ADD_CONSTANT(HHACT_ZOOM);
    // @const win32help|HHACT_ZOOM|

    ADD_CONSTANT(HHACT_TOC_NEXT);
    // @const win32help|HHACT_TOC_NEXT|

    ADD_CONSTANT(HHACT_TOC_PREV);
    // @const win32help|HHACT_TOC_PREV|

    ADD_CONSTANT(HHACT_NOTES);
    // @const win32help|HHACT_NOTES|

    ADD_CONSTANT(HHACT_LAST_ENUM);
    // @const win32help|HHACT_LAST_ENUM|

    ADD_CONSTANT(HH_GPROPID_SINGLETHREAD);
    // @const win32help|HH_GPROPID_SINGLETHREAD|VARIANT_BOOL: True for single
    // thread

    ADD_CONSTANT(HH_GPROPID_TOOLBAR_MARGIN);
    // @const win32help|HH_GPROPID_TOOLBAR_MARGIN|long: Provides a left/right
    // margin around the toolbar.

    ADD_CONSTANT(HH_GPROPID_UI_LANGUAGE);
    // @const win32help|HH_GPROPID_UI_LANGUAGE|long: LangId of the UI.

    ADD_CONSTANT(HH_GPROPID_CURRENT_SUBSET);
    // @const win32help|HH_GPROPID_CURRENT_SUBSET|BSTR: Current subset.

    ADD_CONSTANT(HH_GPROPID_CONTENT_LANGUAGE);
    // @const win32help|HH_GPROPID_CONTENT_LANGUAGE|long: LandId for desired
    // content.

    return rc;
}

/* List of functions exported by this module */

// @module win32help|A module, encapsulating the Win32 help API's.
static struct PyMethodDef win32help_functions[] = {

    // @pymeth WinHelp|Invokes the Windows Help system.
    {"WinHelp", PyWinHelp, 1},

    // @pymeth HH_AKLINK|Create and returns an HH_AKLINK structure
    {"HH_AKLINK", myHH_AKLINK, 1},

    // @pymeth HH_FTS_QUERY|Create and returns an HH_FTS_QUERY structure
    {"HH_FTS_QUERY", myHH_FTS_QUERY, 1},

    // @pymeth HH_POPUP|Create and returns an HH_POPUP structure
    {"HH_POPUP", myHH_POPUP, 1},

    // @pymeth HH_WINTYPE|Create and returns an HH_WINTYPE structure
    {"HH_WINTYPE", myHH_WINTYPE, 1},

    // @pymeth NMHDR|Create and returns an NMHDR structure
    {"NMHDR", myNMHDR, 1},

    // @pymeth HHN_NOTIFY|Create and returns an HHN_NOTIFY structure
    {"HHN_NOTIFY", myHHN_NOTIFY, 1},

    // @pymeth HHNTRACK|Create and returns an HHNTRACK structure
    {"HHNTRACK", myHHNTRACK, 1},

    // @pymeth HtmlHelp|Invokes the Windows HTML Help system.
    {"HtmlHelp", PyHtmlHelp, 1},

    {NULL, NULL}

};

// Module initialization:

PYWIN_MODULE_INIT_FUNC(win32help)
{
    PYWIN_MODULE_INIT_PREPARE(win32help, win32help_functions, "A module, encapsulating the Win32 help API's.");
    if (AddConstants(module) != 0)
        PYWIN_MODULE_INIT_RETURN_ERROR;

    if (PyType_Ready(&PyHH_AKLINKType) == -1 || PyType_Ready(&PyHH_FTS_QUERYType) == -1 ||
        PyType_Ready(&PyHH_POPUPType) == -1 || PyType_Ready(&PyHH_WINTYPEType) == -1 ||
        PyType_Ready(&PyNMHDRType) == -1 || PyType_Ready(&PyHHN_NOTIFYType) == -1 ||
        PyType_Ready(&PyHHNTRACKType) == -1)
        PYWIN_MODULE_INIT_RETURN_ERROR;

    PYWIN_MODULE_INIT_RETURN_SUCCESS;
}
