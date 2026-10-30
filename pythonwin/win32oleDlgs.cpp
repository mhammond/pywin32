
#include "stdafxole.h"

#include "win32dlg.h"
#include "win32oleDlgs.h"

// @doc

// @object PyCOleDialog|An abstract class which encapsulates an MFC COleDialog object.  Derived from a <o
// PyCCommonDialog> object.
static struct PyMethodDef PyCOleDialog_methods[] = {{NULL, NULL}};

ui_type_CObject PyCOleDialog::type("PyCOleDialog", &PyCCommonDialog::type, RUNTIME_CLASS(COleDialog),
                                   sizeof(PyCOleDialog), PYOBJ_OFFSET(PyCOleDialog), PyCOleDialog_methods, NULL);
