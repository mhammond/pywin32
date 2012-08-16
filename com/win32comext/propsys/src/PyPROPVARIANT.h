// Common definitions for propsys module, should probably rename

#include "PythonCOM.h"
#include "structmember.h"
#include "propsys.h"


extern BOOL PyWin_NewPROPVARIANT(PyObject *ob, VARTYPE vt, PROPVARIANT *ppv);
extern BOOL PyWinObject_AsPROPVARIANT(PyObject *ob, PROPVARIANT **pppv);
extern PyObject *PyWinObject_FromPROPVARIANT(PROPVARIANT *ppv);
extern PyObject *PyWinObject_FromPROPVARIANT(REFPROPVARIANT rpv);
extern BOOL PyWinObject_AsPROPERTYKEY(PyObject *obkey, PROPERTYKEY *pkey);
extern PyObject *PyWinObject_FromPROPERTYKEY(REFPROPERTYKEY key);
extern BOOL PyWinObject_AsUSHORT(PyObject *ob, PUSHORT pushort);

extern __declspec(dllexport) PyTypeObject PyPROPVARIANTType;

class __declspec(dllexport) PyPROPVARIANT : public PyObject
{
public:
	// The Python methods
	// static int setattr(PyObject *self, char *name, PyObject *value);
	BOOLEAN ClearOnDestruction;
	static struct PyMemberDef members[];
	static struct PyMethodDef methods[];
	static void deallocFunc(PyObject *ob);
	static PyObject *tp_new(PyTypeObject *, PyObject *, PyObject *);
	PROPVARIANT Py_propvariant;
	PyPROPVARIANT(PROPVARIANT *);
	PyPROPVARIANT(REFPROPVARIANT rpv);
	PyPROPVARIANT(void);
	static PyObject *GetValue(PyObject *, PyObject *);
	static PyObject *ToString(PyObject *, PyObject *);
	static PyObject *ChangeType(PyObject *, PyObject *);
protected:
	~PyPROPVARIANT();

};
