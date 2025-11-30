class PyIADsUser : public PyIADs
{
public:
MAKE_PYCOM_CTOR(PyIADsUser);
static PyComTypeObject type;
static IADsUser *GetI(PyObject *self);
	static PyObject *ChangePassword(PyObject *self, PyObject *args);
	static PyObject *SetPassword(PyObject *self, PyObject *args);
	static PyObject *put_LoginScript(PyObject *self, PyObject *args);
	static PyObject *get_LoginScript(PyObject *self, PyObject *args);
	static PyObject *put_HomePage(PyObject *self, PyObject *args);
	static PyObject *get_HomePage(PyObject *self, PyObject *args);
	static PyObject *put_HomeDirectory(PyObject *self, PyObject *args);
	static PyObject *get_HomeDirectory(PyObject *self, PyObject *args);
	static PyObject *put_FullName(PyObject *self, PyObject *args);
	static PyObject *get_FullName(PyObject *self, PyObject *args);
	static PyObject *put_FirstName(PyObject *self, PyObject *args);
	static PyObject *get_FirstName(PyObject *self, PyObject *args);
	static PyObject *put_EmployeeID(PyObject *self, PyObject *args);
	static PyObject *get_EmployeeID(PyObject *self, PyObject *args);
	static PyObject *put_EmailAddress(PyObject *self, PyObject *args);
	static PyObject *get_EmailAddress(PyObject *self, PyObject *args);
	static PyObject *put_Division(PyObject *self, PyObject *args);
	static PyObject *get_Division(PyObject *self, PyObject *args);
	static PyObject *put_Description(PyObject *self, PyObject *args);
	static PyObject *get_Description(PyObject *self, PyObject *args);
	static PyObject *put_Department(PyObject *self, PyObject *args);
	static PyObject *get_Department(PyObject *self, PyObject *args);
	static PyObject *get_BadLoginCount(PyObject *self, PyObject *args);
	static PyObject *get_BadLoginAddress(PyObject *self, PyObject *args);
	static PyObject *put_AccountDisabled(PyObject *self, PyObject *args);
	static PyObject *get_AccountDisabled(PyObject *self, PyObject *args);
protected:
	PyIADsUser(IUnknown *);
	~PyIADsUser();
};

