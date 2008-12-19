
// Our public converters.
PyObject *PyObject_FromFUNCDESC(FUNCDESC *desc);
PyObject *PyObject_FromTYPEATTR(TYPEATTR *desc);
PyObject *PyObject_FromELEMDESC(const ELEMDESC *ed);
PyObject *PyObject_FromELEMDESCArray(ELEMDESC *ed, int len);
PyObject *PyObject_FromTYPEDESC(const TYPEDESC *td);
PyObject *PyObject_FromVARDESC(VARDESC *desc);


BOOL PyObject_AsELEMDESC( PyObject *ob, ELEMDESC *ppDesc, void *pMore );
BOOL PyObject_AsELEMDESCArray( PyObject *ob, ELEMDESC **ppDesc, short *pNum, void *pMore );
BOOL PyObject_AsTYPEDESC( PyObject *ob, TYPEDESC **ppDesc);

BOOL PyObject_AsFUNCDESC(PyObject *ob, FUNCDESC **ppfd);
void PyObject_FreeFUNCDESC(FUNCDESC *);

BOOL PyObject_AsTYPEDESC( PyObject *ob, TYPEDESC **ppDesc);
void PyObject_FreeTYPEDESC(TYPEDESC *);

BOOL PyObject_AsVARDESC(PyObject *ob, VARDESC **pp);
void PyObject_FreeVARDESC(VARDESC *p);


// The object definitions.
class PyFUNCDESC : public PyObject
{
public:
	PyFUNCDESC(void);
	PyFUNCDESC(const FUNCDESC *pFD);
	~PyFUNCDESC();

	static void deallocFunc(PyObject *ob);
	static PyObject *getitem(PyObject *self, Py_ssize_t index);
	static Py_ssize_t getlength(PyObject *self);

	static struct PyMemberDef members[];
	static struct PyMethodDef methods[];
	static PyTypeObject Type;

	int memid;
	PyObject *scodeArray;
	PyObject *args;
	int funckind;
	int invkind;
	int callconv;
	int cParamsOpt;
	int oVft;
	PyObject *rettype;
	int wFuncFlags;
};

class PyTYPEATTR : public PyObject
{
public:
	PyTYPEATTR(void);
	PyTYPEATTR(const TYPEATTR *pFD);
	~PyTYPEATTR();

	static void deallocFunc(PyObject *ob);

	static PyObject *getitem(PyObject *self, Py_ssize_t index);
	static Py_ssize_t getlength(PyObject *self);

	static struct PyMemberDef members[];
	static struct PyMethodDef methods[];
	static PyTypeObject Type;

protected:
	PyObject *iid;
	int lcid;
	int memidConstructor;
	int memidDestructor;
	int cbSizeInstance;
	int typekind;
	int cFuncs;
	int cVars;
	int cImplTypes;
	int cbSizeVft;
	int cbAlignment;
	int wTypeFlags;
	int wMajorVerNum;
	int wMinorVerNum;
	PyObject *obDescAlias;
	PyObject *obIDLDesc;
};


class PyVARDESC : public PyObject
{
public:
	PyVARDESC(void);
	PyVARDESC(const VARDESC *pVD);
	~PyVARDESC();

	static void deallocFunc(PyObject *ob);

	static PyObject *getitem(PyObject *self, Py_ssize_t index);
	static Py_ssize_t getlength(PyObject *self);
	static struct PyMemberDef members[];
	static struct PyMethodDef methods[];
	static PyTypeObject Type;

	int memid;
	PyObject *value;
	PyObject *elemdescVar;
	int wVarFlags;
	int varkind;
};

class PYCOM_EXPORT PySTGMEDIUM : public PyObject
{
public:
	PySTGMEDIUM(STGMEDIUM *pS = NULL);
	~PySTGMEDIUM(void);

	void DropOwnership(void);
	void Close(void);
	BOOL CopyTo(STGMEDIUM *pDest);
	static void deallocFunc(PyObject *ob);
	static PyObject *getattro(PyObject *self, PyObject *obname);
	STGMEDIUM medium;
	static struct PyMethodDef methods[];
	static PyTypeObject Type;
};

PYCOM_EXPORT PySTGMEDIUM *PyObject_FromSTGMEDIUM(STGMEDIUM *desc = NULL);
#define PySTGMEDIUM_Check(x) ((x)->ob_type==&PySTGMEDIUM::Type)
