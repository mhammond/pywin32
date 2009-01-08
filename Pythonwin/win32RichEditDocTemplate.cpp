#include "stdafx.h"

#include "win32win.h"
#include "win32doc.h"
#include "win32control.h"
#include "win32RichEdit.h"
#include "win32template.h"
#include "win32RichEditDocTemplate.h"
#include "pythondoc.h"
#include "pythonRichEditDoc.h"

// @doc

// @pymethod <o PyCRichEditDocTemplate>|win32ui|CreateRichEditDocTemplate|Creates a document template object.
PyObject *
PyCRichEditDocTemplate::create(PyObject *self, PyObject *args)
{
	UINT idResource;
	// @pyparm int|idRes||The ID for resources for documents of this type.
	if (!PyArg_ParseTuple(args,"i:CreateRichEditDocTemplate", &idResource))
		return NULL;

	CPythonDocTemplate *pMFCTemplate = new CPythonDocTemplate(idResource);
	return ui_assoc_object::make(PyCRichEditDocTemplate::type, pMFCTemplate);
}


// @pymethod <o PyCRichEditDoc>|PyCRichEditDocTemplate|DoCreateRichEditDoc|Creates an underlying document object.
PyObject *
PyCRichEditDocTemplate::DoCreateRichEditDoc(PyObject *self, PyObject *args)
{
	// @pyparm string|fileName|None|The name of the file to load.
	return DoCreateDocHelper(self, args, RUNTIME_CLASS(CPythonRichEditDoc), PyCRichEditDoc::type);
}


// @object PyCRichEditDocTemplate|A document template class for OLE functionality.  Encapsulates an MFC <c CDocTemplate> class
static struct PyMethodDef PyCRichEditDocTemplate_methods[] = {
	{"DoCreateRichEditDoc",PyCRichEditDocTemplate::DoCreateRichEditDoc, 1}, // @pymeth DoCreateRichEditDoc|Creates an underlying document object.
	{NULL,			NULL}
};

ui_type_CObject PyCRichEditDocTemplate::type("PyCRichEditDocTemplate", 
									 &PyCDocTemplate::type, 
									 RUNTIME_CLASS(CDocTemplate), 
									 sizeof(PyCRichEditDocTemplate), 
									 PYOBJ_OFFSET(PyCRichEditDocTemplate), 
									 PyCRichEditDocTemplate_methods, 
									 GET_PY_CTOR(PyCRichEditDocTemplate) );

