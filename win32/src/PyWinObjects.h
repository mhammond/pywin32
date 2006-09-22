
#ifndef __PYWINOBJECTS_H__
#define __PYWINTYPES_H__


#ifndef PYWIN_USE_PYUNICODE

class PYWINTYPES_EXPORT PyUnicode : public PyObject
{
public:
	BSTR	m_bstrValue;

	PyUnicode(void);
	PyUnicode(const char *value);
	PyUnicode(const char *value, unsigned int numBytes);
	PyUnicode(const OLECHAR *value);
	PyUnicode(const OLECHAR *value, int numChars);
	PyUnicode(const BSTR value, BOOL takeOwnership=FALSE);
	PyUnicode(PyObject *value);
	~PyUnicode();

	/* Python support */
	int compare(PyObject *ob);
	PyObject * concat(PyObject *ob);
	PyObject * repeat(int count);
	PyObject * item(int index);
	PyObject * slice(int start, int end);
	PyObject * getattr(char *name);
	long hash(void);
	PyObject *asStr(void);
	int print(FILE *fp, int flags);
	PyObject *repr();
	PyObject * upper(void);
	PyObject * lower(void);

	static void deallocFunc(PyObject *ob);
	static int compareFunc(PyObject *ob1, PyObject *ob2);
	static long hashFunc(PyObject *ob);
	static PyObject * strFunc(PyObject *ob);
	static int printFunc(PyObject *ob, FILE *fp, int flags);
	static PyObject * reprFunc(PyObject *ob);
	static int lengthFunc(PyObject *ob);
	static PyObject * concatFunc(PyObject *ob1, PyObject *ob2);
	static PyObject * repeatFunc(PyObject *ob1, int count);
	static PyObject * itemFunc(PyObject *ob1, int index);
	static PyObject * sliceFunc(PyObject *ob1, int start, int end);
	static PyObject * getattrFunc(PyObject *ob, char *name);
	static PyObject * upperFunc(PyObject *ob, PyObject *args);
	static PyObject * lowerFunc(PyObject *ob, PyObject *args);
};

#endif // PYWIN_USE_PYUNICODE

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
	PyTime(long t);
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
	long asLong(void);
	long hash(void);

	static void deallocFunc(PyObject *ob);
	static int printFunc(PyObject *ob, FILE *fp, int flags);
	static int compareFunc(PyObject *ob1, PyObject *ob2);
	static int nonzeroFunc(PyObject *ob);
	static long hashFunc(PyObject *ob);
	static PyObject * strFunc(PyObject *ob);
	static PyObject * intFunc(PyObject *ob);
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
