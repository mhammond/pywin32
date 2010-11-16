
#ifndef __PYWINOBJECTS_H__
#define __PYWINTYPES_H__

#ifndef  NO_PYWINTYPES_IID
// NOTE - In general, you should not use "new PyIID", but use the
// API PyCom_PyIIDObjectFromIID
class PYWINTYPES_EXPORT PyIID : public PyObject
{
public:
	IID m_iid;

	PyIID(REFIID riid = IID_NULL);

	int IsEqual(REFIID riid);
	int IsEqual(PyObject *ob);
	int IsEqual(PyIID &iid);

	/* Python support */
	PyObject *richcompare(PyObject *other, int op);
	Py_hash_t hash(void);
	PyObject *str(void);
	PyObject *repr(void);

	static void deallocFunc(PyObject *ob);
	static int printFunc(PyObject *ob, FILE *fp, int flags);
	static PyObject *richcompareFunc(PyObject *self, PyObject *other, int op);
	static Py_hash_t hashFunc(PyObject *ob);
	static PyObject * strFunc(PyObject *ob);
	static PyObject * reprFunc(PyObject *ob);
};
#endif // NO_PYWINTYPES_IID

#ifndef NO_PYWINTYPES_TIME

class PYWINTYPES_EXPORT PyTime : public PyObject
{
public:
	DATE m_time;	/* the OLE type for representing date/times */

	PyTime(DATE t);
	PyTime(time_t t);
	PyTime(const SYSTEMTIME &t);
	PyTime(const FILETIME &t);

	/* Conversion Helpers */
	BOOL GetTime(DATE *pDate);
	BOOL GetTime(FILETIME *pDate);
	BOOL GetTime(SYSTEMTIME *pDate);

	/* Python support */
	PyObject *str();
	PyObject *repr();
	int compare(PyObject *ob);
	PyObject *PyTime::richcompare(PyObject *other, int op);

	int print(FILE *fp, int flags);
	Py_hash_t hash(void);
	//PyObject *str(void);
	long asLong(void);

	static PyObject * unaryFailureFunc(PyObject *ob);
	static PyObject * binaryFailureFunc(PyObject *ob1, PyObject *ob2);
	static PyObject * ternaryFailureFunc(PyObject *ob1, PyObject *ob2, PyObject *ob3);
	static void deallocFunc(PyObject *ob);
	static int printFunc(PyObject *ob, FILE *fp, int flags);
	static PyObject *getattro(PyObject *self, PyObject *obname);
	static int compareFunc(PyObject *ob1, PyObject *ob2);
	static PyObject *richcompareFunc(PyObject *self, PyObject *other, int op);
	static Py_hash_t hashFunc(PyObject *ob);
	//static PyObject * strFunc(PyObject *ob);
	static int nonzeroFunc(PyObject *ob);
	static PyObject * intFunc(PyObject *ob);
	static PyObject * floatFunc(PyObject *ob);
	static PyObject * strFunc(PyObject *ob);
	static PyObject * reprFunc(PyObject *ob);
	static struct PyMethodDef methods[];
	// Methods
	static PyObject *Format(PyObject *self, PyObject *args);
};
#endif // NO_PYWINTYPES_TIME

class PYWINTYPES_EXPORT PyOVERLAPPED : public PyObject
{
public:
	class PYWINTYPES_EXPORT sMyOverlapped : public OVERLAPPED
	{
	public:
		PyObject *obState;
		DWORD  dwValue;
		// set to TRUE when we bump the reference count to keep the object
		// alive while it is sitting in a completion port.
		BOOL isArtificialReference;
		sMyOverlapped() {obState=NULL;dwValue=0;isArtificialReference=0;}
		sMyOverlapped(const OVERLAPPED &o) : OVERLAPPED(o) {obState=NULL;dwValue=0;}
	};
	PyObject *obDummy;
	OVERLAPPED *GetOverlapped() {return &m_overlapped;}

	PyOVERLAPPED(void);
	PyOVERLAPPED(const sMyOverlapped *);
	~PyOVERLAPPED();

	/* Python support */
	static void deallocFunc(PyObject *ob);
	static PyObject *richcompareFunc(PyObject *ob, PyObject *other, int op);

	static PyObject *getattro(PyObject *self, PyObject *obname);
	static int setattro(PyObject *self, PyObject *obname, PyObject *v);
	static Py_hash_t hashFunc(PyObject *self);
	static struct PYWINTYPES_EXPORT PyMemberDef members[];

	static PyObject *get_hEvent(PyObject *self, void *unused);
	static int set_hEvent(PyObject *self, PyObject *v, void *unused);
	static PyObject *get_Internal(PyObject *self, void *unused);
	static int set_Internal(PyObject *self, PyObject *v, void *unused);
	static PyObject *get_InternalHigh(PyObject *self, void *unused);
	static int set_InternalHigh(PyObject *self, PyObject *v, void *unused);
	static PyGetSetDef getset[];

	sMyOverlapped m_overlapped;
	PyObject *m_obhEvent;
};

class PYWINTYPES_EXPORT PyHANDLE : public PyObject
{
public:
	operator HANDLE() {return m_handle;}

	PyHANDLE(HANDLE hInit);
	virtual ~PyHANDLE(void);

	virtual BOOL Close(void);
	virtual const char *GetTypeName() {return "PyHANDLE";}

	/* Python support */
	PyObject *richcompare(PyObject *other, int op);

	int print(FILE *fp, int flags);
	PyObject *asStr(void);
	Py_hash_t hash(void);

	static void deallocFunc(PyObject *ob);
	static int printFunc(PyObject *ob, FILE *fp, int flags);
	static PyObject *richcompareFunc(PyObject *ob, PyObject *other, int op);
	static int nonzeroFunc(PyObject *ob);
	static Py_hash_t hashFunc(PyObject *ob);

	static PyObject * strFunc(PyObject *ob);
	static PyObject * intFunc(PyObject *ob);
	static PyObject * longFunc(PyObject *ob);
	static PyObject * unaryFailureFunc(PyObject *ob);
	static PyObject * binaryFailureFunc(PyObject *ob1, PyObject *ob2);
	static PyObject * ternaryFailureFunc(PyObject *ob1, PyObject *ob2, PyObject *ob3);

	static PyObject *get_handle(PyObject *self, void *unused);
	static PyGetSetDef getset[];

	static PyObject *Close(PyObject *self, PyObject *args);
	static PyObject *Detach(PyObject *self, PyObject *args);
	static struct PyMethodDef methods[];

protected:
	HANDLE m_handle;
};

class PYWINTYPES_EXPORT PyHKEY : public PyHANDLE
{
public:
	PyHKEY(HANDLE hInit) : PyHANDLE(hInit) {}
	virtual BOOL Close(void);
	virtual const char *GetTypeName() {return "PyHKEY";}
};
#endif /* __PYWINTYPES_H__ */

class PYWINTYPES_EXPORT PyDEVMODEA : public PyObject
{
public:
	static struct PyMemberDef members[];
	static struct PyMethodDef methods[];

	static PyObject *get_DeviceName(PyObject *self, void *unused);
	static int set_DeviceName(PyObject *self, PyObject *obsd, void *unused);
	static PyObject *get_FormName(PyObject *self, void *unused);
	static int set_FormName(PyObject *self, PyObject *obsd, void *unused);
	static PyObject *get_DriverData(PyObject *self, void *unused);
	static int set_DriverData(PyObject *self, PyObject *obsd, void *unused);
	static PyGetSetDef getset[];

	static void deallocFunc(PyObject *ob);
	PyDEVMODEA(PDEVMODEA);
	PyDEVMODEA(void);
	PyDEVMODEA(USHORT);
	static PyObject *Clear(PyObject *self, PyObject *args);
	static PyObject *tp_new(PyTypeObject *, PyObject *, PyObject *);
	// use this where a function modifies a passed-in PyDEVMODE to make changes visible to Python
	void modify_in_place(void)
		{memcpy(&devmode, pdevmode, pdevmode->dmSize);}
	PDEVMODEA GetDEVMODE(void);
protected:
	// Pointer to variable length DEVMODE with dmDriverExtra bytes allocated at end, always use this externally
	PDEVMODEA pdevmode;
	// copy of fixed portion of DEVMODE for structmember api to access
	DEVMODEA  devmode;   
	~PyDEVMODEA();
};

// Unicode version of DEVMODE
class PYWINTYPES_EXPORT PyDEVMODEW : public PyObject
{
public:
	static struct PyMemberDef members[];
	static struct PyMethodDef methods[];

	static PyObject *get_DeviceName(PyObject *self, void *unused);
	static int set_DeviceName(PyObject *self, PyObject *obsd, void *unused);
	static PyObject *get_FormName(PyObject *self, void *unused);
	static int set_FormName(PyObject *self, PyObject *obsd, void *unused);
	static PyObject *get_DriverData(PyObject *self, void *unused);
	static int set_DriverData(PyObject *self, PyObject *obsd, void *unused);
	static PyGetSetDef getset[];

	static void deallocFunc(PyObject *ob);
	PyDEVMODEW(PDEVMODEW);
	PyDEVMODEW(void);
	PyDEVMODEW(USHORT);
	static PyObject *Clear(PyObject *self, PyObject *args);
	static PyObject *tp_new(PyTypeObject *, PyObject *, PyObject *);
	// use this where a function modifies a passed-in PyDEVMODE to make changes visible to Python
	void modify_in_place(void)
		{memcpy(&devmode, pdevmode, pdevmode->dmSize);}
	PDEVMODEW GetDEVMODE(void);
protected:
	// Pointer to variable length DEVMODE with dmDriverExtra bytes allocated at end, always use this externally
	PDEVMODEW pdevmode;
	// copy of fixed portion of DEVMODE for structmember api to access
	DEVMODEW devmode;   
	~PyDEVMODEW();
};

#ifdef UNICODE
#define PyDEVMODE PyDEVMODEW
#else
#define PyDEVMODE PyDEVMODEA
#endif
