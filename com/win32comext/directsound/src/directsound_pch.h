// directsound_pch.h : header file for PCH generation for the directsound COM extension

#include <windows.h>
#include <Python.h>
#include <PythonCOM.h>
#include <dsound.h>

/*
** DSBUFFERDESC support
*/

PyObject *PyWinMethod_NewDSBUFFERDESC(PyObject *self, PyObject *args);
PyObject *PyWinObject_FromWAVEFROMATEX(const DSBUFFERDESC &dsbd);
BOOL PyWinObject_AsDSBUFFERDESC(PyObject *ob, DSBUFFERDESC **ppDSBUFFERDESC, BOOL bNoneOK = TRUE);
extern PyTypeObject PyDSBUFFERDESCType;
#define PyDSBUFFERDESC_Check(ob)		((ob)->ob_type == &PyDSBUFFERDESCType)

/*
** DSCAPS support
*/

PyObject *PyWinMethod_NewDSCAPS(PyObject *self, PyObject *args);
PyObject *PyWinObject_FromDSCAPS(const DSBUFFERDESC &dsbd);
BOOL PyWinObject_AsDSCAPS(PyObject *ob, DSCAPS **ppDSCAPS, BOOL bNoneOK = TRUE);
extern PyTypeObject PyDSCAPSType;
#define PyDSCAPS_Check(ob)		((ob)->ob_type == &PyDSCAPSType)

/*
** DSBCAPS support
*/

PyObject *PyWinMethod_NewDSBCAPS(PyObject *self, PyObject *args);
PyObject *PyWinObject_FromDSBCAPS(const DSBUFFERDESC &dsbd);
BOOL PyWinObject_AsDSBCAPS(PyObject *ob, DSBCAPS **ppDSBCAPS, BOOL bNoneOK = TRUE);
extern PyTypeObject PyDSBCAPSType;
#define PyDSBCAPS_Check(ob)		((ob)->ob_type == &PyDSBCAPSType)


class PyDSBUFFERDESC : public PyObject
{
public:

	PyDSBUFFERDESC(void);
	PyDSBUFFERDESC(const DSBUFFERDESC &);
	~PyDSBUFFERDESC();

	/* Python support */
	static void deallocFunc(PyObject *ob);

	static int setattro(PyObject *self, PyObject *obname, PyObject *obvalue);

	PyObject *m_obWFX;

#ifdef _MSC_VER
#pragma warning( disable : 4251 )
#endif // _MSC_VER
	static struct PyMemberDef members[];
#ifdef _MSC_VER
#pragma warning( default : 4251 )
#endif // _MSC_VER
	DSBUFFERDESC m_dsbd;
};


class PyDSCAPS : public PyObject
{
public:

	DSCAPS *GetCAPS() {return &m_caps;}

	PyDSCAPS(void);
	PyDSCAPS(const DSCAPS &);
	~PyDSCAPS();

	/* Python support */
	static void deallocFunc(PyObject *ob);

#ifdef _MSC_VER
#pragma warning( disable : 4251 )
#endif // _MSC_VER
	static struct PyMemberDef members[];
#ifdef _MSC_VER
#pragma warning( default : 4251 )
#endif // _MSC_VER
	DSCAPS m_caps;
};

class PyDSBCAPS : public PyObject
{
public:

	DSBCAPS *GetCAPS() {return &m_caps;}

	PyDSBCAPS(void);
	PyDSBCAPS(const DSBCAPS &);
	~PyDSBCAPS();

	/* Python support */
	static void deallocFunc(PyObject *ob);

#ifdef _MSC_VER
#pragma warning( disable : 4251 )
#endif // _MSC_VER
	static struct PyMemberDef members[];
#ifdef _MSC_VER
#pragma warning( default : 4251 )
#endif // _MSC_VER
	DSBCAPS m_caps;
};
