// dll holder class
// by Dave Brennan (brennan@hal.com)

class dll_object : public ui_assoc_object {
   public:
    MAKE_PY_CTOR(dll_object)
    static ui_type type;
    static PyObject *create(PyObject *self, PyObject *args);
    AFX_EXTENSION_MODULE *pMFCExt;
    CDynLinkLibrary *pCDLL;
    HINSTANCE GetDll() { return (HINSTANCE)GetGoodCppObject(&type); }

   protected:
    dll_object();
    ~dll_object();
    virtual CString repr();

   private:
    BOOL bDidLoadLibrary;
};
