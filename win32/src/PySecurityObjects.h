// Security objects

#ifndef MS_WINCE /* Not on CE */

// To do - rationalize PySECURITY_ATTRIBUTES and SECURITY_DESCRIPTOR
// objects.
class PYWINTYPES_EXPORT PySECURITY_ATTRIBUTES : public PyObject
{
public:
	SECURITY_ATTRIBUTES *GetSA() {return &m_sa;}

	PySECURITY_ATTRIBUTES(void);
	PySECURITY_ATTRIBUTES(const SECURITY_ATTRIBUTES &);

	/* Python support */
	int compare(PyObject *ob);

	static void deallocFunc(PyObject *ob);

	static PyObject *getattr(PyObject *self, char *name);
	static int setattr(PyObject *self, char *name, PyObject *v);

	static PyObject *Initialize(PyObject *self, PyObject *args);
	static PyObject *SetSecurityDescriptorDacl(PyObject *self, PyObject *args);

#pragma warning( disable : 4251 )
	static struct memberlist memberlist[];
#pragma warning( default : 4251 )

protected:
	SECURITY_ATTRIBUTES m_sa;
	SECURITY_DESCRIPTOR m_sd;
};


class PYWINTYPES_EXPORT PySECURITY_DESCRIPTOR : public PyObject
{
public:
	SECURITY_DESCRIPTOR *GetSD() {return m_psd;}

	PySECURITY_DESCRIPTOR(unsigned cb = 0);
	PySECURITY_DESCRIPTOR(const SECURITY_DESCRIPTOR *psd, unsigned cb=0);
	~PySECURITY_DESCRIPTOR(void);

	/* Python support */
	int compare(PyObject *ob);

	static void deallocFunc(PyObject *ob);

	static PyObject *getattr(PyObject *self, char *name);
	static int setattr(PyObject *self, char *name, PyObject *v);
	static int getreadbuf(PyObject *self, int index, const void **ptr);
	static int getsegcount(PyObject *self, int *lenp);

	static PyObject *Initialize(PyObject *self, PyObject *args);
	static PyObject *SetSecurityDescriptorDacl(PyObject *self, PyObject *args);

#pragma warning( disable : 4251 )
	static struct memberlist memberlist[];
#pragma warning( default : 4251 )

protected:
	SECURITY_DESCRIPTOR *m_psd;
};

class PYWINTYPES_EXPORT PySID : public PyObject
{
public:
	PSID GetSID() {return m_psid;}

	PySID(int bufSize, void *initBuf = NULL);
	PySID(PSID other, bool bFreewithFreeSid = false);
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

#pragma warning( disable : 4251 )
	static struct memberlist memberlist[];
#pragma warning( default : 4251 )

protected:
	PSID m_psid;
	bool m_bFreeWithFreeSid;
};

class PYWINTYPES_EXPORT PyACL : public PyObject
{
public:
	ACL *GetACL() {return (ACL *)buf;}

	PyACL(int bufSize);
	~PyACL();

	/* Python support */
	int compare(PyObject *ob);

	static void deallocFunc(PyObject *ob);

	static PyObject *getattr(PyObject *self, char *name);
	static int setattr(PyObject *self, char *name, PyObject *v);

	static PyObject *Initialize(PyObject *self, PyObject *args);
	static PyObject *AddAccessAllowedAce(PyObject *self, PyObject *args);
	static PyObject *AddAccessDeniedAce(PyObject *self, PyObject *args);

#pragma warning( disable : 4251 )
	static struct memberlist memberlist[];
#pragma warning( default : 4251 )

protected:
	void *buf;
	int bufSize;
};

#endif // MS_WINCE
