// MiscTypes.cpp - misc Python types.
#include "stdafx.h"
#include "PythonCOM.h"

// ### FUTURE: since we have our own type, we can use it to expose values
// ###         from the PyComTypeObjects. For example, "override" the getattr
// ###         slot and be able to return the base type. If we added an IID
// ###         to the type, then we could return that, too (which would
// ###         provide a nice way to see what interface is being exposed by
// ###         an object).
static PyTypeObject PyInterfaceType_Type = {
	PyObject_HEAD_INIT(&PyType_Type)
	0,			/* Number of items for varobject */
	"interface-type",			/* Name of this type */
	sizeof(PyTypeObject),	/* Basic object size */
	0,			/* Item size for varobject */
	0,			/*tp_dealloc*/
	0,			/*tp_print*/
	PyType_Type.tp_getattr, /*tp_getattr*/
	0,			/*tp_setattr*/
	0,			/*tp_compare*/
	PyType_Type.tp_repr,	/*tp_repr*/
	0,			/*tp_as_number*/
	0,			/*tp_as_sequence*/
	0,			/*tp_as_mapping*/
	0,			/*tp_hash*/
	0,			/*tp_call*/
	0,			/*tp_str*/
	0,			/*tp_getattro */
	0,			/*tp_setattro */
	0,			/* tp_as_buffer */
	0,			/* tp_flags */
	"Define the behavior of a PythonCOM Interface type.",          /* tp_doc */
};

PyComTypeObject::PyComTypeObject( const char *name, PyComTypeObject *pBase, int typeSize, struct PyMethodDef* methodList, PyIUnknown * (* thector)(IUnknown *))
{
// originally, this copied the typeobject of the parent, but as it is impossible
// to gurantee order of static object construction, I went this way.  This is 
// probably better, as is forces _all_ python objects have the same type sig.
	static const PyTypeObject type_template = {
		PyObject_HEAD_INIT(&PyInterfaceType_Type)
		0,													/*ob_size*/
		"PythonComTypeTemplate",							/*tp_name*/
		sizeof(PyIBase), 									/*tp_basicsize*/
		0,													/*tp_itemsize*/
		/* methods */
		(destructor) PyIBase::dealloc, 						/*tp_dealloc*/
		0,													/*tp_print*/
		(getattrfunc) PyIBase::getattr, 					/*tp_getattr*/
		(setattrfunc) PyIBase::setattr,						/*tp_setattr*/
		PyIBase::cmp,										/*tp_compare*/
		(reprfunc)PyIBase::repr,							/*tp_repr*/
    	0,													/*tp_as_number*/
		0,			/*tp_as_sequence*/
		0,			/*tp_as_mapping*/
		0,			/*tp_hash*/
		0,			/*tp_call*/
		0,			/*tp_str*/
		0,			/*tp_getattro */
		0,			/*tp_setattro */
		0,			/* tp_as_buffer */
		Py_TPFLAGS_HAVE_ITER,			/* tp_flags */
		0,          /* tp_doc */
		0,    /* tp_traverse */
		0,                              /* tp_clear */
		0,                              /* tp_richcompare */
		0,                              /* tp_weaklistoffset */
		PyIBase::iter,		/* tp_iter */
		PyIBase::iternext        /* tp_iternext */
	};

	*((PyTypeObject *)this) = type_template;

	chain.methods = methodList;
	chain.link = pBase ? &pBase->chain : NULL;

	baseType = pBase;
	ctor = thector;

	// cast away const, as Python doesnt use it.
	tp_name = (char *)name;
	tp_basicsize = typeSize;
}
PyComTypeObject::~PyComTypeObject()
{
}

/* static */ BOOL PyComTypeObject::is_interface_type(const PyObject *ob)
{
	return ob->ob_type == &PyInterfaceType_Type;
}

/////////////////////////////////////////////////////////////////////////////
// class PyOleEmpty
PyOleEmpty::PyOleEmpty()
{
	ob_type = &PyOleEmptyType;
	_Py_NewReference(this);
}

static void empty_dealloc(PyOleEmpty *o)
{
	delete o;
}

PyTypeObject PyOleEmptyType =
{
	PyObject_HEAD_INIT(&PyType_Type)
	0,
	"PyOleEmpty",
	sizeof(PyOleEmpty),
	0,
	(destructor)empty_dealloc,  /*tp_dealloc*/
	0,                      /*tp_print*/
	0,                      /*tp_getattr*/
	0,                      /*tp_setattr*/
	0,                      /*tp_compare*/
	0,                      /*tp_repr*/
	0,                      /*tp_as_number*/
	0,                      /*tp_as_sequence*/
	0,                      /*tp_as_mapping*/
};

/////////////////////////////////////////////////////////////////////////////
// class PyOleMissing
PyOleMissing::PyOleMissing()
{
	ob_type = &PyOleMissingType;
	_Py_NewReference(this);
}

static void missing_dealloc(PyOleMissing *o)
{
	delete o;
}

PYCOM_EXPORT PyTypeObject PyOleMissingType =
{
	PyObject_HEAD_INIT(&PyType_Type)
	0,
	"PyOleMissing",
	sizeof(PyOleMissing),
	0,
	(destructor)missing_dealloc,  /*tp_dealloc*/
	0,                      /*tp_print*/
	0,                      /*tp_getattr*/
	0,                      /*tp_setattr*/
	0,                      /*tp_compare*/
	0,                      /*tp_repr*/
	0,                      /*tp_as_number*/
	0,                      /*tp_as_sequence*/
	0,                      /*tp_as_mapping*/
};
////////////////////////////////
