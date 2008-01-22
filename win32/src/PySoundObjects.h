#ifndef __PYSOUNDOBJECTS_H__
#define __PYSOUNDOBJECTS_H__

#include <windows.h>
#include <mmsystem.h>

class PYWINTYPES_EXPORT PyWAVEFORMATEX : public PyObject
{
public:

	PyWAVEFORMATEX(void);
	PyWAVEFORMATEX(const WAVEFORMATEX &);
	~PyWAVEFORMATEX();

	/* Python support */
	static void deallocFunc(PyObject *ob);

#ifdef _MSC_VER
#pragma warning( disable : 4251 )
#endif // _MSC_VER
	static struct PyMemberDef members[];
#ifdef _MSC_VER
#pragma warning( default : 4251 )
#endif // _MSC_VER
	WAVEFORMATEX m_wfx;
};

#endif