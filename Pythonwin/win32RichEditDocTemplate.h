
class PyCRichEditDocTemplate : public PyCDocTemplate {
protected:
public:

	static PyObject *create(PyObject *self, PyObject *args);
	static PyObject *DoCreateRichEditDoc(PyObject *self, PyObject *args);

	static ui_type_CObject type;
	MAKE_PY_CTOR(PyCRichEditDocTemplate)
};
