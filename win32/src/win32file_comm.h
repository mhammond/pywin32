// The communications related functions.
// The COMM port enhancements were added by Mark Hammond, and are
// (c) 2000-2001, ActiveState Tools Corp.

// The comms port helpers.
extern PyObject *PyWinObject_FromCOMSTAT(const COMSTAT *pCOMSTAT);
extern BOOL PyWinObject_AsCOMSTAT(PyObject *ob, COMSTAT **ppCOMSTAT, BOOL bNoneOK = TRUE);
extern BOOL PyWinObject_AsDCB(PyObject *ob, DCB **ppDCB, BOOL bNoneOK = TRUE);
extern PyObject *PyWinObject_FromDCB(const DCB *pDCB);
extern PyObject *PyWinMethod_NewDCB(PyObject *self, PyObject *args);
extern PyObject *PyWinObject_FromCOMMTIMEOUTS( COMMTIMEOUTS *p);
extern BOOL PyWinObject_AsCOMMTIMEOUTS( PyObject *ob, COMMTIMEOUTS *p);

class PyDCB : public PyObject
{
public:
	DCB *GetDCB() {return &m_DCB;}

	PyDCB(void);
	PyDCB(const DCB &);
	~PyDCB();

	/* Python support */
	int compare(PyObject *ob);

	static void deallocFunc(PyObject *ob);
	static int compareFunc(PyObject *ob1, PyObject *ob2);

	static PyObject *getattro(PyObject *self, PyObject *obname);
	static int setattro(PyObject *self, PyObject *obname, PyObject *v);
	static struct PyMemberDef members[];
	static PyTypeObject type;

protected:
	DCB m_DCB;
};

#define PyDCB_Check(x) ((x)->ob_type==&PyDCB::type)

////////////////////////////////////////////////////////////////
//
// COMSTAT object.
//
////////////////////////////////////////////////////////////////
class PyCOMSTAT : public PyObject
{
public:
	COMSTAT *GetCOMSTAT() {return &m_COMSTAT;}

	PyCOMSTAT(void);
	PyCOMSTAT(const COMSTAT &);
	~PyCOMSTAT();

	/* Python support */
	int compare(PyObject *ob);

	static void deallocFunc(PyObject *ob);
	static int compareFunc(PyObject *ob1, PyObject *ob2);

	static PyObject *getattro(PyObject *self, PyObject *obname);
	static int setattro(PyObject *self, PyObject *obname, PyObject *v);
	static struct PyMemberDef members[];
	static PyTypeObject type;

protected:
	COMSTAT m_COMSTAT;
};

#define PyCOMSTAT_Check(x) ((x)->ob_type==&PyCOMSTAT::type)
