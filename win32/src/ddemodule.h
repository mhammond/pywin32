extern PyObject * BASED_CODE dde_module_error;

// #define RETURN_NONE				do {Py_INCREF(Py_None);return Py_None;} while (0)
#define RETURN_DDE_ERR(err)		do {PyErr_SetString(dde_module_error,err);return NULL;} while (0)


class PythonDDEServer : public CDDEServer
{
public:
	PythonDDEServer() : m_obSystemTopic(NULL) {;}
    virtual BOOL OnCreate();
	virtual void Status(const char* pszFormat, ...);
	virtual CDDEServerSystemTopic *CreateSystemTopic();
	PyObject *m_obSystemTopic;
};

class PyDDEServer : public ui_assoc_CObject
{
public:
	~PyDDEServer() {Python_delete_assoc(this);}
	static ui_type_CObject type;
	static PythonDDEServer *GetServer (PyObject *self);
	MAKE_PY_CTOR(PyDDEServer);
};


template <class T>
class PythonDDETopicFramework : public T
{
public:
	~PythonDDETopicFramework() {Python_delete_assoc(this);}
    virtual BOOL Exec(void* pData, DWORD dwSize)
	{
		PyObject *args = Py_BuildValue("(s)", pData);
		BOOL rc = TRUE;
		CVirtualHelper helper("Exec", this);
		if (helper.call_args(args) )
			helper.retval(rc);
		return !rc;
	}
    virtual BOOL NSRequest(const char * szItem, void** ppData, DWORD* dwSize)
	{
		PyObject *args = Py_BuildValue("(s)", szItem);
		BOOL rc = TRUE;
		CVirtualHelper helper("Request", this);
		if (helper.call_args(args) ) {
			char * strret ;
			if (helper.retval(strret)) {
				PyObject * look ;
				helper.retval(look) ;
				*dwSize = PyObject_Length(look)+1 ; 
				*ppData = (void*)(const char*)strret ;
				return TRUE ;
			}
		}
		return !rc;
	}

    virtual BOOL NSPoke(const char * szItem, void* pData, DWORD dwSize)
	{
		PyObject *args = Py_BuildValue("(sz#)", szItem, pData, dwSize);
		BOOL rc = TRUE;
		CVirtualHelper helper("Poke", this);
		if (helper.call_args(args) ) {
			return TRUE ;
		}
		return !rc;
	}
};

typedef PythonDDETopicFramework<CDDETopic> PythonDDETopic;
typedef PythonDDETopicFramework<CDDEServerSystemTopic> PythonDDEServerSystemTopic;


class PyDDETopic : public ui_assoc_CObject
{
public:
	static ui_type_CObject type;
	static PythonDDETopic *GetTopic (PyObject *self);
	MAKE_PY_CTOR(PyDDETopic);
};

class PyDDEServerSystemTopic : public PyDDETopic
{
public:
	static ui_type_CObject type;
	static PythonDDEServerSystemTopic *GetTopic (PyObject *self);
	MAKE_PY_CTOR(PyDDEServerSystemTopic);
};

class PythonDDEStringItem : public CDDEStringItem
{
public:
	PythonDDEStringItem::~PythonDDEStringItem() {Python_delete_assoc(this);}
};

class PyDDEStringItem : public ui_assoc_CObject
{
public:
	static ui_type_CObject type;
	static PythonDDEStringItem *GetItem (PyObject *self);
	MAKE_PY_CTOR(PyDDEStringItem);
};

class PythonDDEConv : public CDDEConv
{
public:
	PythonDDEConv(CDDEServer* pServer) : CDDEConv(pServer) {;}
	PythonDDEConv::~PythonDDEConv() {Python_delete_assoc(this);}
};

class PyDDEConv : public ui_assoc_CObject
{
public:
	static ui_type_CObject type;
	static PythonDDEConv *GetConv (PyObject *self);
	MAKE_PY_CTOR(PyDDEConv);
};
