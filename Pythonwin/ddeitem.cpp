// @doc
#include "stdafxdde.h"
#include "ddemodule.h"


PythonDDEStringItem *PyDDEStringItem::GetItem (PyObject *self)
{
	return (PythonDDEStringItem *)ui_assoc_object::GetGoodCppObject( self, &type);
}

// @pymethod |PyDDEStringItem|SetData|Sets an items data, and causes any underlying notification.
PyObject *PyDDEStringItem_SetData(PyObject *self, PyObject *args)
{
	char *val;
	PythonDDEStringItem *pItem = PyDDEStringItem::GetItem(self);
	if (!pItem) return NULL;
	// @pyparm string|data||The data to set.
	if (!PyArg_ParseTuple(args, "s:SetData", &val))
		return NULL;
	GUI_BGN_SAVE;
	pItem->SetData(val);
	GUI_END_SAVE;
	RETURN_NONE;
}

// @pymethod |PyDDEStringItem|Destroy|Destroys an item
PyObject *PyDDEStringItem_Destroy(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ":Destroy"))
		return NULL;
	GUI_BGN_SAVE;
	PythonDDEStringItem *pItem = PyDDEStringItem::GetItem(self);
	GUI_END_SAVE;
	if (!pItem) return NULL;
	delete pItem;
	RETURN_NONE;
}

// @object PyDDEStringItem|A DDE string item.
static struct PyMethodDef PyDDEStringItem_methods[] = {
	{"Destroy",    PyDDEStringItem_Destroy, 1},
	{"SetData",    PyDDEStringItem_SetData, 1}, // @pymeth SetData|Sets an items data, and causes any underlying notification.
	{NULL,			NULL}		// sentinel
};

ui_type_CObject PyDDEStringItem::type("PyDDEStringItem", 
							   &ui_assoc_CObject::type, 
							   RUNTIME_CLASS(CDDEStringItem), 
							   sizeof(PyDDEStringItem), 
							   PyDDEStringItem_methods,
							   GET_PY_CTOR(PyDDEStringItem));
