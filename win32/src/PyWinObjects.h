
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
	int print(FILE *fp, int flags);
	long hash(void);
	PyObject *str(void);

	static void deallocFunc(PyObject *ob);
	static int printFunc(PyObject *ob, FILE *fp, int flags);
	static int compareFunc(PyObject *ob1, PyObject *ob2);
	static long hashFunc(PyObject *ob);
	static PyObject * strFunc(PyObject *ob);
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
	// Methods
	static PyObject *Format(PyObject *self, PyObject *args);
};
#endif // NO_PYWINTYPES_TIME

typedef struct PYWINTYPES_EXPORT _sMyOverlapped : public OVERLAPPED
{
	PyObject *obState;
} sMyOverlapped;

class PYWINTYPES_EXPORT PyOVERLAPPED : public PyObject
{
public:
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
#pragma warning( disable : 4251 )
	static struct memberlist memberlist[];
#pragma warning( default : 4251 )

protected:
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
#pragma warning( disable : 4251 )
	static struct memberlist memberlist[];
#pragma warning( default : 4251 )

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
