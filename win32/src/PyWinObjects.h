
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
	int compare(PyObject *ob);
	long hash(void);
	PyObject *str(void);
	PyObject *repr(void);

	static void deallocFunc(PyObject *ob);
	static int printFunc(PyObject *ob, FILE *fp, int flags);
	static int compareFunc(PyObject *ob1, PyObject *ob2);
	static long hashFunc(PyObject *ob);
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
	int print(FILE *fp, int flags);
	PyObject *getattr(char *name);
	long hash(void);
	//PyObject *str(void);
	long asLong(void);

	static PyObject * unaryFailureFunc(PyObject *ob);
	static PyObject * binaryFailureFunc(PyObject *ob1, PyObject *ob2);
	static PyObject * ternaryFailureFunc(PyObject *ob1, PyObject *ob2, PyObject *ob3);
	static void deallocFunc(PyObject *ob);
	static int printFunc(PyObject *ob, FILE *fp, int flags);
	static PyObject *getattrFunc(PyObject *ob, char *attr);
	static int compareFunc(PyObject *ob1, PyObject *ob2);
	static long hashFunc(PyObject *ob);
	//static PyObject * strFunc(PyObject *ob);
	static int nonzeroFunc(PyObject *ob);
	static PyObject * intFunc(PyObject *ob);
	static PyObject * floatFunc(PyObject *ob);
	static PyObject * strFunc(PyObject *ob);
	static PyObject * reprFunc(PyObject *ob);
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
	int compare(PyObject *ob);

	static void deallocFunc(PyObject *ob);
	static int compareFunc(PyObject *ob1, PyObject *ob2);

	static PyObject *getattr(PyObject *self, char *name);
	static int setattr(PyObject *self, char *name, PyObject *v);
	static long hashFunc(PyObject *self);
#ifdef _MSC_VER
#pragma warning( disable : 4251 )
#endif // _MSC_VER
	static struct memberlist memberlist[];
#ifdef _MSC_VER
#pragma warning( default : 4251 )
#endif // _MSC_VER
	sMyOverlapped m_overlapped;
	PyObject *m_obHandle;
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
	int compare(PyObject *ob);
	int print(FILE *fp, int flags);
	PyObject *asStr(void);
	long hash(void);

	static void deallocFunc(PyObject *ob);
	static int printFunc(PyObject *ob, FILE *fp, int flags);
	static int compareFunc(PyObject *ob1, PyObject *ob2);
	static int nonzeroFunc(PyObject *ob);
	static long hashFunc(PyObject *ob);

	static PyObject * strFunc(PyObject *ob);
	static PyObject * intFunc(PyObject *ob);
	static PyObject * longFunc(PyObject *ob);
	static PyObject * unaryFailureFunc(PyObject *ob);
	static PyObject * binaryFailureFunc(PyObject *ob1, PyObject *ob2);
	static PyObject * ternaryFailureFunc(PyObject *ob1, PyObject *ob2, PyObject *ob3);

	static PyObject *Close(PyObject *self, PyObject *args);
	static PyObject *Detach(PyObject *self, PyObject *args);

	static PyObject *getattr(PyObject *self, char *name);
	static int setattr(PyObject *self, char *name, PyObject *v);
#ifdef _MSC_VER
#pragma warning( disable : 4251 )
#endif // _MSC_VER
	static struct memberlist memberlist[];
#ifdef _MSC_VER
#pragma warning( default : 4251 )
#endif // _MSC_VER

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

class PYWINTYPES_EXPORT PyDEVMODE : public PyObject
{
public:
#ifdef _MSC_VER
#pragma warning( disable : 4251 )
#endif // _MSC_VER
	static struct PyMemberDef members[];
	static struct PyMethodDef methods[];
#ifdef _MSC_VER
#pragma warning( default : 4251 )
#endif // _MSC_VER

	static void deallocFunc(PyObject *ob);
	PyDEVMODE(PDEVMODE);
	PyDEVMODE(void);
	PyDEVMODE(USHORT);
	static PyObject *getattro(PyObject *self, PyObject *name);
	static int setattro(PyObject *self, PyObject *obname, PyObject *obvalue);
	static PyObject *Clear(PyObject *self, PyObject *args);
	static PyObject *tp_new(PyTypeObject *, PyObject *, PyObject *);
	// use this where a function modifies a passed-in PyDEVMODE to make changes visible to Python
	void modify_in_place(void)
		{memcpy(&devmode, pdevmode, pdevmode->dmSize);}
	PDEVMODE GetDEVMODE(void);
	PyObject *obdummy;
protected:
	// Pointer to variable length DEVMODE with dmDriverExtra bytes allocated at end, always use this externally
	PDEVMODE pdevmode;
	// copy of fixed portion of DEVMODE for structmember api to access
	DEVMODE  devmode;   
	~PyDEVMODE();
};

// Unicode version of DEVMODE
class PYWINTYPES_EXPORT PyDEVMODEW : public PyObject
{
public:
#ifdef _MSC_VER
#pragma warning( disable : 4251 )
#endif // _MSC_VER
	static struct PyMemberDef members[];
	static struct PyMethodDef methods[];
#ifdef _MSC_VER
#pragma warning( default : 4251 )
#endif // _MSC_VER

	static void deallocFunc(PyObject *ob);
	PyDEVMODEW(PDEVMODEW);
	PyDEVMODEW(void);
	PyDEVMODEW(USHORT);
	static PyObject *getattro(PyObject *self, PyObject *name);
	static int setattro(PyObject *self, PyObject *obname, PyObject *obvalue);
	static PyObject *Clear(PyObject *self, PyObject *args);
	static PyObject *tp_new(PyTypeObject *, PyObject *, PyObject *);
	// use this where a function modifies a passed-in PyDEVMODE to make changes visible to Python
	void modify_in_place(void)
		{memcpy(&devmode, pdevmode, pdevmode->dmSize);}
	PDEVMODEW GetDEVMODE(void);
	PyObject *obdummy;
protected:
	// Pointer to variable length DEVMODE with dmDriverExtra bytes allocated at end, always use this externally
	PDEVMODEW pdevmode;
	// copy of fixed portion of DEVMODE for structmember api to access
	DEVMODEW devmode;   
	~PyDEVMODEW();
};
