// win32net.h
//
// Declarations for the win32net module.
extern PyObject *ReturnNetError(char *fnName, long err = 0);

enum NSI_TYPE {
    NSI_WSTR,
    NSI_DWORD,
    NSI_LONG,
    NSI_BOOL,
    NSI_HOURS,
    NSI_SID,
    NSI_SECURITY_DESCRIPTOR,
};
struct PyNET_STRUCT_ITEM {
    char *attrname;
    NSI_TYPE type;
    size_t off;
    BOOL reqd;
};

struct PyNET_STRUCT {
    DWORD level;
    PyNET_STRUCT_ITEM *entries;
    size_t structsize;
};

PyObject *PyObject_FromNET_STRUCT(PyNET_STRUCT *pI, BYTE *buf);
BOOL PyObject_AsNET_STRUCT(PyObject *ob, PyNET_STRUCT *pI, BYTE **ppRet);
void PyObject_FreeNET_STRUCT(PyNET_STRUCT *pI, BYTE *pBuf);
BOOL FindNET_STRUCT(DWORD level, PyNET_STRUCT *pBase, PyNET_STRUCT **ppRet);

// Helpers functions that take a function pointer
typedef DWORD(__stdcall *PFNSIMPLEENUM)(LPCWSTR, DWORD, LPBYTE *, DWORD, LPDWORD, LPDWORD, PDWORD_PTR);
PyObject *PyDoSimpleEnum(PyObject *self, PyObject *args, PFNSIMPLEENUM pfn, char *fnname, PyNET_STRUCT *pInfos);

typedef DWORD(__stdcall *PFNNAMEDENUM)(LPCWSTR, LPCWSTR, DWORD, LPBYTE *, DWORD, LPDWORD, LPDWORD, PDWORD_PTR);
PyObject *PyDoNamedEnum(PyObject *self, PyObject *args, PFNNAMEDENUM pfn, char *fnname, PyNET_STRUCT *pInfos);

typedef DWORD(__stdcall *PFNGROUPSET)(LPCWSTR, LPCWSTR, DWORD, LPBYTE, DWORD);
PyObject *PyDoGroupSet(PyObject *self, PyObject *args, PFNGROUPSET pfn, char *fnname, PyNET_STRUCT *pInfos);

typedef DWORD(__stdcall *PFNGETINFO)(LPCWSTR, LPCWSTR, DWORD, LPBYTE *);
PyObject *PyDoGetInfo(PyObject *self, PyObject *args, PFNGETINFO pfn, char *fnname, PyNET_STRUCT *pInfos);

typedef DWORD(__stdcall *PFNGETMODALSINFO)(LPCWSTR, DWORD, LPBYTE *);
PyObject *PyDoGetModalsInfo(PyObject *self, PyObject *args, PFNGETMODALSINFO pfn, char *fnname, PyNET_STRUCT *pInfos);

typedef DWORD(__stdcall *PFNSETINFO)(LPCWSTR, LPCWSTR, DWORD, LPBYTE, DWORD *);
PyObject *PyDoSetInfo(PyObject *self, PyObject *args, PFNSETINFO pfn, char *fnname, PyNET_STRUCT *pInfos);

typedef DWORD(__stdcall *PFNSETMODALSINFO)(LPCWSTR, DWORD, LPBYTE, DWORD *);
PyObject *PyDoSetModalsInfo(PyObject *self, PyObject *args, PFNSETMODALSINFO pfn, char *fnname, PyNET_STRUCT *pInfos);

typedef DWORD(__stdcall *PFNADD)(LPCWSTR, DWORD, LPBYTE, DWORD *);
PyObject *PyDoAdd(PyObject *self, PyObject *args, PFNADD pfn, char *fnname, PyNET_STRUCT *pInfos);

typedef DWORD(__stdcall *PFNDEL)(LPCWSTR, LPCWSTR);
PyObject *PyDoDel(PyObject *self, PyObject *args, PFNDEL pfn, char *fnname);

PyObject *PyDoGroupDelMembers(PyObject *self, PyObject *args);

#if WINVER >= 0x0500
typedef NET_API_STATUS(NET_API_FUNCTION *NetValidateNamefunc)(LPCWSTR, LPCWSTR, LPCWSTR, LPCWSTR, NETSETUP_NAME_TYPE);
extern "C" NetValidateNamefunc pfnNetValidateName;

typedef NET_API_STATUS(NET_API_FUNCTION *NetGetJoinInformationfunc)(LPCWSTR, LPWSTR *, PNETSETUP_JOIN_STATUS);
extern "C" NetGetJoinInformationfunc pfnNetGetJoinInformation;

typedef NET_API_STATUS(NET_API_FUNCTION *NetValidatePasswordPolicyfunc)(LPCWSTR, LPVOID, NET_VALIDATE_PASSWORD_TYPE,
                                                                        LPVOID, LPVOID *);
extern "C" NetValidatePasswordPolicyfunc pfnNetValidatePasswordPolicy;

typedef NET_API_STATUS(NET_API_FUNCTION *NetValidatePasswordPolicyFreefunc)(LPVOID *);
extern "C" NetValidatePasswordPolicyFreefunc pfnNetValidatePasswordPolicyFree;

#endif  // WINVER
