// Security objects
// Much of the security support written by Roger Upole <rwupole@msn.com>

#ifndef MS_WINCE /* Not on CE */

// To do - rationalize PySECURITY_ATTRIBUTES and SECURITY_DESCRIPTOR
// objects.
class PYWINTYPES_EXPORT PySECURITY_ATTRIBUTES : public PyObject
{
public:
	SECURITY_ATTRIBUTES *GetSA() {return &m_sa;}

	PySECURITY_ATTRIBUTES(void);
	PySECURITY_ATTRIBUTES(const SECURITY_ATTRIBUTES &);
	~PySECURITY_ATTRIBUTES(void);

	/* Python support */
	int compare(PyObject *ob);

	static void deallocFunc(PyObject *ob);

	static PyObject *getattr(PyObject *self, char *name);
	static int setattr(PyObject *self, char *name, PyObject *v);

	PyObject *m_obSD;

#ifdef _MSC_VER
#pragma warning( disable : 4251 )
#endif // _MSC_VER
	static struct memberlist memberlist[];
#ifdef _MSC_VER
#pragma warning( default : 4251 )
#endif // _MSC_VER

protected:
	SECURITY_ATTRIBUTES m_sa;
	SECURITY_DESCRIPTOR m_sd;
	PyObject *m_obACL; // call to SetSecurityDescriptorDacl incref's then stores pyACL
};


class PYWINTYPES_EXPORT PySECURITY_DESCRIPTOR : public PyObject
{
public:
	PSECURITY_DESCRIPTOR GetSD() {return m_psd;}
	BOOL SetSD(PSECURITY_DESCRIPTOR psd);

	PySECURITY_DESCRIPTOR(unsigned cb = 0);
	PySECURITY_DESCRIPTOR(PSECURITY_DESCRIPTOR psd);
	~PySECURITY_DESCRIPTOR(void);

	/* Python support */
	int compare(PyObject *ob);

	static void deallocFunc(PyObject *ob);

	static PyObject *getattr(PyObject *self, char *name);
	static int setattr(PyObject *self, char *name, PyObject *v);
	static int getreadbuf(PyObject *self, int index, const void **ptr);
	static int getsegcount(PyObject *self, int *lenp);

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
	static PyObject *IsSelfRelative(PyObject *self, PyObject *args);

#ifdef _MSC_VER
#pragma warning( disable : 4251 )
#endif // _MSC_VER
	static struct memberlist memberlist[];
#ifdef _MSC_VER
#pragma warning( default : 4251 )
#endif // _MSC_VER

protected:
	PSECURITY_DESCRIPTOR m_psd;
};

class PYWINTYPES_EXPORT PySID : public PyObject
{
public:
	PSID GetSID() {return m_psid;}

	PySID(int bufSize, void *initBuf = NULL);
	PySID(PSID other);
	~PySID();

	/* Python support */
	int compare(PyObject *ob);

	static void deallocFunc(PyObject *ob);
	static int compareFunc(PyObject *ob1, PyObject *ob2);
	static PyObject *strFunc(PyObject *ob);

	static PyObject *getattr(PyObject *self, char *name);
	static int setattr(PyObject *self, char *name, PyObject *v);
	static int getreadbuf(PyObject *self, int index, const void **ptr);
	static int getsegcount(PyObject *self, int *lenp);

	static PyObject *Initialize(PyObject *self, PyObject *args);
	static PyObject *IsValid(PyObject *self, PyObject *args);
	static PyObject *SetSubAuthority(PyObject *self, PyObject *args);
	static PyObject *GetLength(PyObject *self, PyObject *args);
	static PyObject *GetSubAuthorityCount(PyObject *self, PyObject *args);
	static PyObject *GetSubAuthority(PyObject *self, PyObject *args);
	static PyObject *GetSidIdentifierAuthority(PyObject *self, PyObject *args);

#ifdef _MSC_VER
#pragma warning( disable : 4251 )
#endif // _MSC_VER
	static struct memberlist memberlist[];
#ifdef _MSC_VER
#pragma warning( default : 4251 )
#endif // _MSC_VER

protected:
	PSID m_psid;
	bool m_bFreeWithFreeSid;
};

class PYWINTYPES_EXPORT PyACL : public PyObject
{
public:
	ACL *GetACL() {return (ACL *)buf;}
	void SetACL(ACL *pacl)
	{
		if (buf)
			free(buf);
		buf = (ACL *)malloc(pacl->AclSize);
		memcpy(buf,pacl,pacl->AclSize);
	}

	PyACL(int bufSize, int aclrev);
	PyACL(PACL pacl);

	~PyACL();


	/* Python support */
	int compare(PyObject *ob);

	static void deallocFunc(PyObject *ob);

	static PyObject *getattr(PyObject *self, char *name);
	static int setattr(PyObject *self, char *name, PyObject *v);

	static PyObject *Initialize(PyObject *self, PyObject *args);
	static PyObject *IsValid(PyObject *self, PyObject *args);
	static PyObject *AddAccessAllowedAce(PyObject *self, PyObject *args);
	static PyObject *AddAccessDeniedAce(PyObject *self, PyObject *args);
	static PyObject *AddAuditAccessAce(PyObject *self, PyObject *args);
	static PyObject *GetAclSize(PyObject *self, PyObject *args);
	static PyObject *GetAclRevision(PyObject *self, PyObject *args);
	static PyObject *GetAceCount(PyObject *self, PyObject *args);
	static PyObject *GetAce(PyObject *self, PyObject *args);
	static PyObject *DeleteAce(PyObject *self, PyObject *args);
	static PyObject *PyGetExplicitEntriesFromAcl(PyObject *self, PyObject *args);
	static PyObject *PySetEntriesInAcl(PyObject *self, PyObject *args);

#ifdef _MSC_VER
#pragma warning( disable : 4251 )
#endif // _MSC_VER
	static struct memberlist memberlist[];
#ifdef _MSC_VER
#pragma warning( default : 4251 )
#endif // _MSC_VER

protected:
	void *buf;
	int bufSize;
};

#endif // MS_WINCE
