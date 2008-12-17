//
// @doc

#include "PyWinTypes.h"
#include "PyWinObjects.h"
#include "PySoundObjects.h"
#include "structmember.h"
#include "directsound_pch.h"

// @pymethod <o PyDSCAPS>|directsound|DSCAPS|Creates a new PyDSCAPS object.
PyObject *PyWinMethod_NewDSCAPS(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ":DSCAPS"))
		return NULL;
	return new PyDSCAPS();
}

PyObject *PyWinObject_FromDSCAPS(const DSCAPS &caps)
{
	return new PyDSCAPS(caps);
}

BOOL PyWinObject_AsDSCAPS(PyObject *ob, DSCAPS **ppDSCAPS, BOOL bNoneOK /*= TRUE*/)
{
	if (bNoneOK && ob==Py_None) {
		*ppDSCAPS = NULL;
	} else if (!PyDSCAPS_Check(ob)) {
		PyErr_SetString(PyExc_TypeError, "The object is not a PyDSCAPS object");
		return FALSE;
	} else {
		PyDSCAPS *pycaps= (PyDSCAPS *)ob;
		*ppDSCAPS = pycaps->GetCAPS();
	}
	return TRUE;
}


// @object PyDSCAPS|A Python object, representing a DSCAPS structure
static struct PyMethodDef PyDSCAPS_methods[] = {
	{NULL}
};

PyTypeObject PyDSCAPSType =
{
	PYWIN_OBJECT_HEAD
	"PyDSCAPSType",
	sizeof(PyDSCAPSType),
	0,
	PyDSCAPS::deallocFunc,
	0,			// tp_print;
	0,			// tp_getattr
	0,			// tp_setattr
	0,			// tp_compare
	0,			// tp_repr
	0,			// tp_as_number
	0,			// tp_as_sequence
	0,			// tp_as_mapping
	0,
	0,						/* tp_call */
	0,		/* tp_str */
	PyObject_GenericGetAttr,
	PyObject_GenericSetAttr,
	0,			// tp_as_buffer;
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,	// tp_flags;
	0,			// tp_doc; /* Documentation string */
	0,			// traverseproc tp_traverse;
	0,			// tp_clear;
	0,			// tp_richcompare;
	0,			// tp_weaklistoffset;
	0,			// tp_iter
	0,			// iternextfunc tp_iternext
	0,			// methods
	PyDSCAPS::members,
	0,			// tp_getset;
	0,			// tp_base;
	0,			// tp_dict;
	0,			// tp_descr_get;
	0,			// tp_descr_set;
	0,			// tp_dictoffset;
	0,			// tp_init;
	0,			// tp_alloc;
	0			// newfunc tp_new;
};


#define OFF(e) offsetof(PyDSCAPS, e)

/*static*/ struct PyMemberDef PyDSCAPS::members[] = {
	{"dwFlags",  T_INT,  OFF(m_caps.dwFlags), 0, "Specifies device capabilities."}, 
	// @prop integer|dwFlags|Specifies device capabilities. Can be one or more of the following:
	// @flagh Flag|Description
	// @flag DSCAPS_PRIMARYMONO|The device supports monophonic primary buffers. 
	// @flag DSCAPS_PRIMARYSTEREO|The device supports stereo primary buffers. 
	// @flag DSCAPS_PRIMARY8BIT|The device supports hardware-mixed secondary buffers with 8-bit samples. 
	// @flag DSCAPS_PRIMARY16BIT|The device supports primary sound buffers with 16-bit samples.
	// @flag DSCAPS_CONTINUOUSRATE|The device supports all sample rates between the dwMinSecondarySampleRate and dwMaxSecondarySampleRate member values. Typically, this means that the actual output rate will be within +/- 10 hertz (Hz) of the requested frequency. 
	// @flag DSCAPS_EMULDRIVER|The device does not have a DirectSound driver installed, so it is being emulated through the waveform-audio functions. Performance degradation should be expected. 
	// @flag DSCAPS_CERTIFIED|This driver has been tested and certified by Microsoft. 
	// @flag DSCAPS_SECONDARYMONO|The device supports hardware-mixed monophonic secondary buffers.
	// @flag DSCAPS_SECONDARYSTEREO|The device supports hardware-mixed stereo secondary buffers. 
	// @flag DSCAPS_SECONDARY8BIT|The device supports hardware-mixed secondary buffers with 8-bit samples. 
	// @flag DSCAPS_SECONDARY16BIT|The device supports hardware-mixed secondary sound buffers with 16-bit samples. 
	{"dwMinSecondarySampleRate",  T_INT,  OFF(m_caps.dwMinSecondarySampleRate), 0, "Minimum sample rate supported by this device's hardware secondary sound buffers."}, 
	// @prop integer|dwMinSecondarySampleRate|Minimum sample rate supported by this device's hardware secondary sound buffers.
	{"dwMaxSecondarySampleRate",  T_INT,  OFF(m_caps.dwMaxSecondarySampleRate), 0, "Maximum sample rate supported by this device's hardware secondary sound buffers."}, 
	// @prop integer|dwMaxSecondarySampleRate|Maximum sample rate supported by this device's hardware secondary sound buffers.
	{"dwPrimaryBuffers",  T_INT,  OFF(m_caps.dwPrimaryBuffers), 0, "Number of primary buffers supported. This value will always be 1."}, 
	// @prop integer|dwPrimaryBuffers|Number of primary buffers supported. This value will always be 1.
	{"dwMaxHwMixingAllBuffers",  T_INT,  OFF(m_caps.dwMaxHwMixingAllBuffers), 0, "Specifies the total number of buffers that can be mixed in hardware. This member can be less than the sum of dwMaxHwMixingStaticBuffers and dwMaxHwMixingStreamingBuffers. Resource tradeoffs frequently occur."}, 
	// @prop integer|dwMaxHwMixingAllBuffers|Specifies the total number of buffers that can be mixed in hardware. This member can be less than the sum of dwMaxHwMixingStaticBuffers and dwMaxHwMixingStreamingBuffers. Resource tradeoffs frequently occur.
	{"dwMaxHwMixingStaticBuffers",  T_INT,  OFF(m_caps.dwMaxHwMixingStaticBuffers), 0, "Specifies the maximum number of static sound buffers."}, 
	// @prop integer|dwMaxHwMixingStaticBuffers|Specifies the maximum number of static sound buffers.
	{"dwMaxHwMixingStreamingBuffers",  T_INT,  OFF(m_caps.dwMaxHwMixingStreamingBuffers), 0, "Specifies the maximum number of streaming sound buffers."}, 
	// @prop integer|dwMaxHwMixingStreamingBuffers|Specifies the maximum number of streaming sound buffers.
	{"dwFreeHwMixingAllBuffers",  T_INT,  OFF(m_caps.dwFreeHwMixingAllBuffers), 0, "Description of the free mixing hardware capabilities of the device. An application can use these values to determine whether hardware resources are available for allocation to a secondary sound buffer. Also, by comparing this value to the members that specify maximum mixing capabilities, the resources that are already allocated can be determined. "}, 
	// @prop integer|dwFreeHwMixingAllBuffers|Description of the free hardware mixing capabilities of the device. An application can use this value to determine whether hardware resources are available for allocation to a secondary sound buffer. Also, by comparing these values to the members that specify maximum mixing capabilities, the resources that are already allocated can be determined. 
	{"dwFreeHwMixingStaticBuffers",  T_INT,  OFF(m_caps.dwFreeHwMixingStaticBuffers), 0, "Description of the free hardware mixing capabilities of the device. An application can use this value to determine whether hardware resources are available for allocation to a secondary sound buffer. Also, by comparing these values to the members that specify maximum mixing capabilities, the resources that are already allocated can be determined."}, 
	// @prop integer|dwFreeHwMixingStaticBuffers|Description of the free hardware mixing capabilities of the device. An application can use this value to determine whether hardware resources are available for allocation to a secondary sound buffer. Also, by comparing these values to the members that specify maximum mixing capabilities, the resources that are already allocated can be determined.
	{"dwFreeHwMixingStreamingBuffers",  T_INT,  OFF(m_caps.dwFreeHwMixingStreamingBuffers), 0, "Description of the free hardware mixing capabilities of the device. An application can use this value to determine whether hardware resources are available for allocation to a secondary sound buffer. Also, by comparing these values to the members that specify maximum mixing capabilities, the resources that are already allocated can be determined."}, 
	// @prop integer|dwFreeHwMixingStreamingBuffers|Description of the free hardware mixing capabilities of the device. An application can use this value to determine whether hardware resources are available for allocation to a secondary sound buffer. Also, by comparing these values to the members that specify maximum mixing capabilities, the resources that are already allocated can be determined.
	{"dwMaxHw3DAllBuffers",  T_INT,  OFF(m_caps.dwMaxHw3DAllBuffers), 0, "Description of the hardware 3-D positional capabilities of the device."}, 
	// @prop integer|dwMaxHw3DAllBuffers|Description of the hardware 3-D positional capabilities of the device. 
	{"dwMaxHw3DStaticBuffers",  T_INT,  OFF(m_caps.dwMaxHw3DStaticBuffers), 0, "Description of the hardware 3-D positional capabilities of the device. "}, 
	// @prop integer|dwMaxHw3DStaticBuffers|Description of the hardware 3-D positional capabilities of the device. 
	{"dwMaxHw3DStreamingBuffers",  T_INT,  OFF(m_caps.dwMaxHw3DStreamingBuffers), 0, "Description of the hardware 3-D positional capabilities of the device."}, 
	// @prop integer|dwMaxHw3DStreamingBuffers|Description of the hardware 3-D positional capabilities of the device. 
	{"dwFreeHw3DAllBuffers",  T_INT,  OFF(m_caps.dwFreeHw3DAllBuffers), 0, "Description of the free, or unallocated, hardware 3-D positional capabilities of the device."}, 
	// @prop integer|dwFreeHw3DAllBuffers|Description of the free, or unallocated, hardware 3-D positional capabilities of the device.
	{"dwFreeHw3DStaticBuffers",  T_INT,  OFF(m_caps.dwFreeHw3DStaticBuffers), 0, "Description of the free, or unallocated, hardware 3-D positional capabilities of the device."}, 
	// @prop integer|dwFreeHw3DStaticBuffers|Description of the free, or unallocated, hardware 3-D positional capabilities of the device.
	{"dwFreeHw3DStreamingBuffers",  T_INT,  OFF(m_caps.dwFreeHw3DStreamingBuffers), 0, "Description of the free, or unallocated, hardware 3-D positional capabilities of the device."}, 
	// @prop integer|dwFreeHw3DStreamingBuffers|Description of the free, or unallocated, hardware 3-D positional capabilities of the device.
	{"dwTotalHwMemBytes",  T_INT,  OFF(m_caps.dwTotalHwMemBytes), 0, "Size, in bytes, of the amount of memory on the sound card that stores static sound buffers."}, 
	// @prop integer|dwTotalHwMemBytes|Size, in bytes, of the amount of memory on the sound card that stores static sound buffers.
	{"dwFreeHwMemBytes",  T_INT,  OFF(m_caps.dwFreeHwMemBytes), 0, "Size, in bytes, of the free memory on the sound card."}, 
	// @prop integer|dwFreeHwMemBytes|Size, in bytes, of the free memory on the sound card.
	{"dwMaxContigFreeHwMemBytes",  T_INT,  OFF(m_caps.dwMaxContigFreeHwMemBytes), 0, "Size, in bytes, of the largest contiguous block of free memory on the sound card."}, 
	// @prop integer|dwMaxContigFreeHwMemBytes|Size, in bytes, of the largest contiguous block of free memory on the sound card.
	{"dwUnlockTransferRateHwBuffers",  T_INT,  OFF(m_caps.dwUnlockTransferRateHwBuffers), 0, "Description of the rate, in kilobytes per second, at which data can be transferred to hardware static sound buffers. This and the number of bytes transferred determines the duration of a call to the IDirectSoundBuffer::Update method."}, 
	// @prop integer|dwUnlockTransferRateHwBuffers|Description of the rate, in kilobytes per second, at which data can be transferred to hardware static sound buffers. This and the number of bytes transferred determines the duration of a call to the IDirectSoundBuffer::Update method.
	{"dwPlayCpuOverheadSwBuffers",  T_INT,  OFF(m_caps.dwPlayCpuOverheadSwBuffers), 0, "Description of the processing overhead, as a percentage of the central processing unit, needed to mix software buffers (those located in main system memory). This varies according to the bus type, the processor type, and the clock speed. The unlock transfer rate for software buffers is 0 because the data need not be transferred anywhere. Similarly, the play processing overhead for hardware buffers is 0 because the mixing is done by the sound device."}, 
	// @prop integer|dwPlayCpuOverheadSwBuffers|Description of the processing overhead, as a percentage of the central processing unit, needed to mix software buffers (those located in main system memory). This varies according to the bus type, the processor type, and the clock speed. The unlock transfer rate for software buffers is 0 because the data need not be transferred anywhere. Similarly, the play processing overhead for hardware buffers is 0 because the mixing is done by the sound device.
	{NULL}
};

PyDSCAPS::PyDSCAPS(void)
{
	ob_type = &PyDSCAPSType;
	_Py_NewReference(this);
	memset(&m_caps, 0, sizeof(m_caps));
}

PyDSCAPS::PyDSCAPS(const DSCAPS &caps)
{
	ob_type = &PyDSCAPSType;
	_Py_NewReference(this);
	m_caps = caps;
	m_caps.dwSize = sizeof(DSCAPS);
}

PyDSCAPS::~PyDSCAPS()
{
}

/*static*/ void PyDSCAPS::deallocFunc(PyObject *ob)
{
	delete (PyDSCAPS *)ob;
}

