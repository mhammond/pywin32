/*

	win32 context code

	This is _not_ a Python type, just a utility class.

	Created January 1996, Mark Hammond (MHammond@skippinet.com.au)
*/

#include "stdafx.h"
#include "win32template.h"
/////////////////////////////////////////////////////////////////////
//
// PythonCreateContext class.
//
//////////////////////////////////////////////////////////////////////
PythonCreateContext::PythonCreateContext()
{
	m_PythonObject = NULL;
	m_pNewViewClass	= NULL;
	m_pCurrentDoc = NULL;
	m_pNewDocTemplate = NULL;
}
PythonCreateContext::~PythonCreateContext()
{
	ReleasePythonObject();
}
#define TEMPLATE_ATTR "template"
void PythonCreateContext::SetPythonObject(PyObject *ob)
{
	ASSERT(ob);
	m_PythonObject = ob;
	Py_INCREF(m_PythonObject);
	// now try and get the template object.

	if (ob!=Py_None) {
		PyObject *pTempl = PyObject_GetAttrString(ob, TEMPLATE_ATTR);
		if (pTempl==NULL)
			PyErr_SetString(ui_module_error, "Warning - CreateContext object has no " TEMPLATE_ATTR " attribute");
		else {
			m_pNewDocTemplate = PyCDocTemplate::GetTemplate(pTempl);
		}
	}
	if (PyErr_Occurred())
		gui_print_error();
}
void PythonCreateContext::ReleasePythonObject()
{
	Py_XDECREF(m_PythonObject);
	m_PythonObject = NULL;
	m_pNewDocTemplate = NULL;
}

