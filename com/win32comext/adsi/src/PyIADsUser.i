%module IADsUser // A COM interface to ADSI's IADsUser interface.

%include "typemaps.i"
%include "pywin32.i"
%include "pythoncom.i"
%include "adsilib.i"

%{
#include "PyIADs.h"
#include "PyIADsUser.h"

#define SWIG_THIS_IID IID_IADsUser

PyIADsUser::PyIADsUser(IUnknown *pDisp) :
	PyIADs(pDisp)
{
	ob_type = &type;
}

PyIADsUser::~PyIADsUser()
{
}

IADsUser *PyIADsUser::GetI(PyObject *self)
{
	return (IADsUser *)PyIADs::GetI(self);
}

PyObject* PyIADsUser_getattro(PyObject *ob, PyObject *obname)
{
	char *name = PyString_AsString(obname);
	if (!name) return NULL;

	IADsUser *p = PyIADsUser::GetI(ob);
	// todo!
	return PyIADs::getattro(ob, obname);
}

%}

// @pyswig int|get_AccountDisabled|
HRESULT get_AccountDisabled(short *OUTPUT);
// @pyswig |put_AccountDisabled|
// @pyparm int|val||
HRESULT put_AccountDisabled(short val);

/**
// @pyswig int|get_AccountExpirationDate|
HRESULT get_AccountExpirationDate(DATE *OUTPUT);
// @pyswig |put_AccountExpirationDate|
// @pyparm <o PyTime>|val||
HRESULT put_AccountExpirationDate(DATE val);
**/
// @pyswig unicode|get_BadLoginAddress|
HRESULT get_BadLoginAddress(BSTR *OUTPUT);

// @pyswig int|get_BadLoginCount |
HRESULT get_BadLoginCount (long *OUTPUT);

// @pyswig unicode|get_Department|
HRESULT get_Department(BSTR *OUTPUT);
// @pyswig |put_Department|
// @pyparm unicode|val||
HRESULT put_Department(OLECHAR *val);

// @pyswig unicode|get_Description|
HRESULT get_Description(BSTR *OUTPUT);
// @pyswig |put_Description|
// @pyparm unicode|val||
HRESULT put_Description(OLECHAR *val);

// @pyswig unicode|get_Division|
HRESULT get_Division(BSTR *OUTPUT);
// @pyswig |put_Division|
// @pyparm unicode|val||
HRESULT put_Division(OLECHAR *val);

// @pyswig unicode|get_EmailAddress|
HRESULT get_EmailAddress(BSTR *OUTPUT);
// @pyswig |put_EmailAddress|
// @pyparm unicode|val||
HRESULT put_EmailAddress(OLECHAR *val);

// @pyswig unicode|get_EmployeeID|
HRESULT get_EmployeeID(BSTR *OUTPUT);
// @pyswig |put_EmployeeID|
// @pyparm unicode|val||
HRESULT put_EmployeeID(OLECHAR *val);

// FAX

// @pyswig unicode|get_FirstName|
HRESULT get_FirstName(BSTR *OUTPUT);
// @pyswig |put_FirstName|
// @pyparm unicode|val||
HRESULT put_FirstName(OLECHAR *val);

// @pyswig unicode|get_FullName|
HRESULT get_FullName(BSTR *OUTPUT);
// @pyswig |put_FullName|
// @pyparm unicode|val||
HRESULT put_FullName(OLECHAR *val);

// @pyswig unicode|get_HomeDirectory|
HRESULT get_HomeDirectory(BSTR *OUTPUT);
// @pyswig |put_HomeDirectory|
// @pyparm unicode|val||
HRESULT put_HomeDirectory(OLECHAR *val);

// @pyswig unicode|get_HomePage|
HRESULT get_HomePage(BSTR *OUTPUT);
// @pyswig |put_HomePage|
// @pyparm unicode|val||
HRESULT put_HomePage(OLECHAR *val);

// @pyswig unicode|get_LoginScript|
HRESULT get_LoginScript(BSTR *OUTPUT);
// @pyswig |put_LoginScript|
// @pyparm unicode|val||
HRESULT put_LoginScript(OLECHAR *val);

// @pyswig |SetPassword|
// @pyparm unicode|val||
HRESULT SetPassword(OLECHAR *val);

// @pyswig |ChangePassword|
// @pyparm unicode|oldval||
// @pyparm unicode|newval||
HRESULT ChangePassword(OLECHAR *val, OLECHAR *val);
