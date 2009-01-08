// @doc
#include "stdafxdde.h"
#include "ddemodule.h"



PythonDDETopic *PyDDETopic::GetTopic (PyObject *self)
{
	return (PythonDDETopic *)ui_assoc_object::GetGoodCppObject( self, &type);
}

// @pymethod |PyDDETopic|AddItem|Add an item to the topic.
PyObject *PyDDETopic_AddItem(PyObject *self, PyObject *args)
{
	PyObject *obItem;
	PythonDDETopic *pTopic = PyDDETopic::GetTopic(self);
	if (!pTopic) return NULL;
	// @pyparm <o PyDDEItem>|item||The item to add
	if (!PyArg_ParseTuple(args, "O:AddItem", &obItem))
		return NULL;
	PythonDDEStringItem *pItem = PyDDEStringItem::GetItem(obItem);
	if (!pItem) return NULL;
	GUI_BGN_SAVE;
	BOOL ok = pTopic->AddItem(pItem);
	GUI_END_SAVE;
	if (!ok)
		RETURN_DDE_ERR("AddItem failed");
	RETURN_NONE;
}

// @pymethod |PyDDETopic|Destroy|Destroys an item
PyObject *PyDDETopic_Destroy(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ":Destroy"))
		return NULL;
	PythonDDETopic *pTopic = PyDDETopic::GetTopic(self);
	if (!pTopic) return NULL;
	GUI_BGN_SAVE;
	delete pTopic;
	GUI_END_SAVE;
	RETURN_NONE;
}

// @object PyDDETopic|A DDE topic.
static struct PyMethodDef PyDDETopic_methods[] = {
	{"AddItem",    PyDDETopic_AddItem, 1}, // @pymeth AddItem|Add an item to the topic.
	{"Destroy",    PyDDETopic_Destroy, 1}, // @pymeth Destroy|Destroys an item
	{NULL,			NULL}		// sentinel
};



ui_type_CObject PyDDETopic::type("PyDDETopic", 
							   &ui_assoc_CObject::type, 
							   RUNTIME_CLASS(CDDETopic), 
							   sizeof(PyDDETopic), 
							   PYOBJ_OFFSET(PyDDETopic), 
							   PyDDETopic_methods,
   							   GET_PY_CTOR(PyDDETopic));


PythonDDEServerSystemTopic *PyDDEServerSystemTopic::GetTopic (PyObject *self)
{
	return (PythonDDEServerSystemTopic *)ui_assoc_object::GetGoodCppObject( self, &type);
}


static struct PyMethodDef PyDDEServerSystemTopic_methods[] = {
	{NULL,			NULL}		// sentinel
};

ui_type_CObject PyDDEServerSystemTopic::type("PyDDEServerSystemTopic", 
							   &PyDDETopic::type, 
							   RUNTIME_CLASS(CDDEServerSystemTopic), 
							   sizeof(PyDDEServerSystemTopic), 
							   PYOBJ_OFFSET(PyDDEServerSystemTopic), 
							   PyDDEServerSystemTopic_methods,
   							   GET_PY_CTOR(PyDDEServerSystemTopic));
