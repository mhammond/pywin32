//
// Command target header file
//

//
// Command target
//
class PYW_EXPORT PyCCmdTarget : public ui_assoc_CObject {
    friend CVirtualHelper::CVirtualHelper(const char *iname, void *iassoc, EnumVirtualErrorHandling veh);

   public:  // some probably shouldnt be, but...
    CMapWordToPtr *pNotifyHookList;
    CMapWordToPtr *pCommandHookList;
    CMapWordToPtr *pOleEventHookList;
    CMapWordToPtr *pCommandUpdateHookList;

    // virtuals for Python support
    virtual CString repr();

    static ui_type_CObject type;

   protected:
    PyCCmdTarget();
    virtual ~PyCCmdTarget();
};
extern void free_hook_list(PyObject *objectHooked, CMapWordToPtr **ppList);
extern PyObject *add_hook_list(PyObject *objectHooked, PyObject *args, CMapWordToPtr **ppList);
