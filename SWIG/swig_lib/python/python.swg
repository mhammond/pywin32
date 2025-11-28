/* ------------------------------------------------------------
 * python.swg
 *
 * Python configuration module.
 * ------------------------------------------------------------ */

/* ------------------------------------------------------------
 *  Inner macros
 * ------------------------------------------------------------ */
%include <pymacros.swg>


/* ------------------------------------------------------------
 *  The runtime part
 * ------------------------------------------------------------ */
%include <pyruntime.swg>

/* ------------------------------------------------------------
 *  Special user directives
 * ------------------------------------------------------------ */
%include <pyuserdir.swg>

/* ------------------------------------------------------------
 *  Typemap specializations
 * ------------------------------------------------------------ */
%include <pytypemaps.swg>

/* ------------------------------------------------------------
 *  Overloaded operator support
 * ------------------------------------------------------------ */
%include <pyopers.swg>

/* ------------------------------------------------------------
 * Warnings for Python keywords 
 * ------------------------------------------------------------ */
%include <pythonkw.swg>

/* ------------------------------------------------------------
 * The Python autodoc support
 * ------------------------------------------------------------ */
%include <pydocs.swg>

/* ------------------------------------------------------------
 * The Python classes, for C++
 * ------------------------------------------------------------ */
%include <pyclasses.swg>

/* ------------------------------------------------------------
 * The Python initialization function 
 * ------------------------------------------------------------ */
%include <pyinit.swg>


/* ------------------------------------------------------------
 * For backward compatibility
 * ------------------------------------------------------------ */
%include <pybackward.swg>


  int i = 0;
  flags = flags;
  fprintf(fp,"Global variables { ");
  while (v->vars[i]) {
    fprintf(fp,"%s", v->vars[i]->name);
    i++;
    if (v->vars[i]) fprintf(fp,", ");
  }
  fprintf(fp," }\n");
  return 0;
}

/* --------------------------------------------------------------------
   swig_varlink_getattr

   This function gets the value of a variable and returns it as a
   PyObject.   In our case, we'll be looking at the datatype and
   converting into a number or string
   -------------------------------------------------------------------- */

static PyObject *
swig_varlink_getattr(swig_varlinkobject *v, char *n)
{
  int i = 0;
  char temp[128];

  while (v->vars[i]) {
    if (strcmp(v->vars[i]->name,n) == 0) {
      return (*v->vars[i]->get_attr)();
    }
    i++;
  }
  sprintf(temp,"C global variable %s not found.", n);
  PyErr_SetString(PyExc_NameError,temp);
  return NULL;
}

/* -------------------------------------------------------------------
   swig_varlink_setattr()

   This function sets the value of a variable.
   ------------------------------------------------------------------- */

static int
swig_varlink_setattr(swig_varlinkobject *v, char *n, PyObject *p)
{
  char temp[128];
  int i = 0;
  while (v->vars[i]) {
    if (strcmp(v->vars[i]->name,n) == 0) {
      return (*v->vars[i]->set_attr)(p);
    }
    i++;
  }
  sprintf(temp,"C global variable %s not found.", n);
  PyErr_SetString(PyExc_NameError,temp);
  return 1;
}

statichere PyTypeObject varlinktype = {
/*  PyObject_HEAD_INIT(&PyType_Type)  Note : This doesn't work on some machines */
  PyObject_HEAD_INIT(0)
  0,
  "varlink",                          /* Type name    */
  sizeof(swig_varlinkobject),         /* Basic size   */
  0,                                  /* Itemsize     */
  0,                                  /* Deallocator  */
  (printfunc) swig_varlink_print,     /* Print        */
  (getattrfunc) swig_varlink_getattr, /* get attr     */
  (setattrfunc) swig_varlink_setattr, /* Set attr     */
  0,                                  /* tp_compare   */
  (reprfunc) swig_varlink_repr,       /* tp_repr      */
  0,                                  /* tp_as_number */
  0,                                  /* tp_as_mapping*/
  0,                                  /* tp_hash      */
};

/* Create a variable linking object for use later */

SWIGSTATIC PyObject *
SWIG_newvarlink(void)
{
  swig_varlinkobject *result = 0;
  result = PyMem_NEW(swig_varlinkobject,1);
  varlinktype.ob_type = &PyType_Type;    /* Patch varlinktype into a PyType */
  Py_SET_TYPE(result, &varlinktype);
  /*  _Py_NewReference(result);  Does not seem to be necessary */
  result->nvars = 0;
  result->maxvars = 64;
  result->vars = (swig_globalvar **) malloc(64*sizeof(swig_globalvar *));
  result->vars[0] = 0;
  Py_XINCREF((PyObject *) result);
  return ((PyObject*) result);
}

SWIGSTATIC void
SWIG_addvarlink(PyObject *p, char *name,
	   PyObject *(*get_attr)(void), int (*set_attr)(PyObject *p))
{
  swig_varlinkobject *v;
  v= (swig_varlinkobject *) p;

  if (v->nvars >= v->maxvars -1) {
    v->maxvars = 2*v->maxvars;
    v->vars = (swig_globalvar **) realloc(v->vars,v->maxvars*sizeof(swig_globalvar *));
    if (v->vars == NULL) {
      fprintf(stderr,"SWIG : Fatal error in initializing Python module.\n");
      exit(1);
    }
  }
  v->vars[v->nvars] = (swig_globalvar *) malloc(sizeof(swig_globalvar));
  v->vars[v->nvars]->name = (char *) malloc(strlen(name)+1);
  strcpy(v->vars[v->nvars]->name,name);
  v->vars[v->nvars]->get_attr = get_attr;
  v->vars[v->nvars]->set_attr = set_attr;
  v->nvars++;
  v->vars[v->nvars] = 0;
}

#else
#define SWIG_newvarlink() Py_None
#endif /* SWIG_NEED_VARLINK */
