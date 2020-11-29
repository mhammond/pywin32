// Security objects
// Much of the security support written by Roger Upole <rwupole@msn.com>

#ifdef MS_WINCE
#define NO_PYWINTYPES_SECURITY /* This source is not included for WinCE */
#endif

#ifndef NO_PYWINTYPES_SECURITY
typedef BOOL(WINAPI *addacefunc)(PACL, DWORD, DWORD, PSID);
typedef BOOL(WINAPI *addaceexfunc)(PACL, DWORD, DWORD, DWORD, PSID);
typedef BOOL(WINAPI *addobjectacefunc)(PACL, DWORD, DWORD, DWORD, GUID *, GUID *, PSID);
extern addacefunc addaccessallowedace;
extern addacefunc addaccessdeniedace;
extern addaceexfunc addaccessallowedaceex;
extern addaceexfunc addaccessdeniedaceex;
extern addaceexfunc addmandatoryace;
extern addobjectacefunc addaccessallowedobjectace;
extern addobjectacefunc addaccessdeniedobjectace;
extern BOOL(WINAPI *addauditaccessaceex)(PACL, DWORD, DWORD, DWORD, PSID, BOOL, BOOL);
extern BOOL(WINAPI *addauditaccessobjectace)(PACL, DWORD, DWORD, DWORD, GUID *, GUID *, PSID, BOOL, BOOL);
extern BOOL(WINAPI *setsecuritydescriptorcontrol)(PSECURITY_DESCRIPTOR, SECURITY_DESCRIPTOR_CONTROL,
                                                  SECURITY_DESCRIPTOR_CONTROL);

// To do - rationalize PySECURITY_ATTRIBUTES and SECURITY_DESCRIPTOR
// objects.
class PYWINTYPES_EXPORT PySECURITY_ATTRIBUTES : public PyObject {
   public:
    SECURITY_ATTRIBUTES *GetSA() { return &m_sa; }

    PySECURITY_ATTRIBUTES(void);
    PySECURITY_ATTRIBUTES(const SECURITY_ATTRIBUTES &);
    ~PySECURITY_ATTRIBUTES(void);

    /* Python support */
    int compare(PyObject *ob);
    static void deallocFunc(PyObject *ob);
    static PyObject *getattro(PyObject *self, PyObject *obname);
    static int setattro(PyObject *self, PyObject *obname, PyObject *v);
    static struct PYWINTYPES_EXPORT PyMemberDef members[];
    static struct PyMethodDef methods[];

    static PyObject *get_SECURITY_DESCRIPTOR(PyObject *self, void *unused);
    static int set_SECURITY_DESCRIPTOR(PyObject *self, PyObject *obsd, void *unused);
    static PyGetSetDef getset[];
    PyObject *m_obSD;

   protected:
    SECURITY_ATTRIBUTES m_sa;
};

class PYWINTYPES_EXPORT PySECURITY_DESCRIPTOR : public PyObject {
   public:
    PSECURITY_DESCRIPTOR GetSD() { return m_psd; }
    BOOL SetSD(PSECURITY_DESCRIPTOR psd);

    PySECURITY_DESCRIPTOR(Py_ssize_t cb = 0);
    PySECURITY_DESCRIPTOR(PSECURITY_DESCRIPTOR psd);
    ~PySECURITY_DESCRIPTOR(void);

    /* Python support */
    int compare(PyObject *ob);
    static void deallocFunc(PyObject *ob);

#if (PY_VERSION_HEX < 0x03000000)
    static Py_ssize_t getreadbuf(PyObject *self, Py_ssize_t index, void **ptr);
    static Py_ssize_t getsegcount(PyObject *self, Py_ssize_t *lenp);
#else
    static int getbufferinfo(PyObject *self, Py_buffer *view, int flags);
#endif

    static PyObject *Initialize(PyObject *self, PyObject *args);
    static PyObject *GetSecurityDescriptorOwner(PyObject *self, PyObject *args);
    static PyObject *GetSecurityDescriptorGroup(PyObject *self, PyObject *args);
    static PyObject *GetSecurityDescriptorDacl(PyObject *self, PyObject *args);
    static PyObject *GetSecurityDescriptorSacl(PyObject *self, PyObject *args);
    static PyObject *SetSecurityDescriptorOwner(PyObject *self, PyObject *args);
    static PyObject *SetSecurityDescriptorGroup(PyObject *self, PyObject *args);
    static PyObject *SetSecurityDescriptorDacl(PyObject *self, PyObject *args);
    static PyObject *SetSecurityDescriptorSacl(PyObject *self, PyObject *args);
    static PyObject *IsValid(PyObject *self, PyObject *args);
    static PyObject *GetLength(PyObject *self, PyObject *args);
    static PyObject *GetSecurityDescriptorControl(PyObject *self, PyObject *args);
    static PyObject *SetSecurityDescriptorControl(PyObject *self, PyObject *args);
    static PyObject *IsSelfRelative(PyObject *self, PyObject *args);
    static struct PyMethodDef methods[];

   protected:
    PSECURITY_DESCRIPTOR m_psd;
};

class PYWINTYPES_EXPORT PySID : public PyObject {
   public:
    PSID GetSID() { return m_psid; }

    PySID(int bufSize, void *initBuf = NULL);
    PySID(PSID other);
    ~PySID();

    /* Python support */
    PyObject *richcompare(PyObject *other, int op);

    static void deallocFunc(PyObject *ob);
    static PyObject *richcompareFunc(PyObject *ob1, PyObject *ob2, int op);
    static PyObject *strFunc(PyObject *ob);

    // Buffer interface changed in 3.0
#if (PY_VERSION_HEX < 0x03000000)
    static Py_ssize_t getreadbuf(PyObject *self, Py_ssize_t index, void **ptr);
    static Py_ssize_t getsegcount(PyObject *self, Py_ssize_t *lenp);
#else
    static int getbufferinfo(PyObject *self, Py_buffer *view, int flags);
#endif

    static PyObject *Initialize(PyObject *self, PyObject *args);
    static PyObject *IsValid(PyObject *self, PyObject *args);
    static PyObject *SetSubAuthority(PyObject *self, PyObject *args);
    static PyObject *GetLength(PyObject *self, PyObject *args);
    static PyObject *GetSubAuthorityCount(PyObject *self, PyObject *args);
    static PyObject *GetSubAuthority(PyObject *self, PyObject *args);
    static PyObject *GetSidIdentifierAuthority(PyObject *self, PyObject *args);
    static struct PyMethodDef PySID::methods[];

   protected:
    PSID m_psid;
};

class PYWINTYPES_EXPORT PyACL : public PyObject {
   public:
    ACL *GetACL() { return (ACL *)buf; }
    BOOL SetACL(ACL *pacl)
    {
        WORD origbufsize = ((ACL *)buf)->AclSize;
        if (pacl->AclSize <= origbufsize) {
            ZeroMemory(buf, origbufsize);
            memcpy(buf, pacl, pacl->AclSize);
            ((ACL *)buf)->AclSize = origbufsize;
            return TRUE;
        }
        void *buf_save = buf;  // so we can restore state if allocation fails
        buf = realloc(buf, pacl->AclSize);
        if (buf == NULL) {
            PyErr_Format(PyExc_MemoryError, "SetACL: Unable to reallocate ACL to size %d", pacl->AclSize);
            buf = buf_save;
            return FALSE;
        }
        memcpy(buf, pacl, pacl->AclSize);
        return TRUE;
    }

    PyACL(int bufSize, int aclrev);
    PyACL(PACL pacl);

    ~PyACL();

    /* Python support */
    int compare(PyObject *ob);
    static void deallocFunc(PyObject *ob);
    static struct PyMethodDef PyACL::methods[];

    static PyObject *Initialize(PyObject *self, PyObject *args);
    static PyObject *IsValid(PyObject *self, PyObject *args);
    static PyObject *AddAccessAllowedAce(PyObject *self, PyObject *args);
    static PyObject *AddAccessAllowedAceEx(PyObject *self, PyObject *args);
    static PyObject *AddAccessAllowedObjectAce(PyObject *self, PyObject *args);
    static PyObject *AddAccessDeniedAce(PyObject *self, PyObject *args);
    static PyObject *AddAccessDeniedAceEx(PyObject *self, PyObject *args);
    static PyObject *AddMandatoryAce(PyObject *self, PyObject *args);
    static PyObject *AddAccessDeniedObjectAce(PyObject *self, PyObject *args);
    static PyObject *AddAuditAccessAce(PyObject *self, PyObject *args);
    static PyObject *AddAuditAccessAceEx(PyObject *self, PyObject *args);
    static PyObject *AddAuditAccessObjectAce(PyObject *self, PyObject *args);
    static PyObject *GetAclSize(PyObject *self, PyObject *args);
    static PyObject *GetAclRevision(PyObject *self, PyObject *args);
    static PyObject *GetAceCount(PyObject *self, PyObject *args);
    static PyObject *GetAce(PyObject *self, PyObject *args);
    static PyObject *DeleteAce(PyObject *self, PyObject *args);
    static PyObject *PyGetExplicitEntriesFromAcl(PyObject *self, PyObject *args);
    static PyObject *PySetEntriesInAcl(PyObject *self, PyObject *args);
    static PyObject *PyGetEffectiveRightsFromAcl(PyObject *self, PyObject *args);
    static PyObject *PyGetAuditedPermissionsFromAcl(PyObject *self, PyObject *args);

   protected:
    void *buf;
};

#endif  // NO_PYWINTYPES_SECURITY
