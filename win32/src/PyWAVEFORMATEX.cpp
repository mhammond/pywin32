//
// @doc

#include "PyWinTypes.h"
#include "PyWinObjects.h"
#include "PySoundObjects.h"
#include "structmember.h"

// @pymethod <o PyWAVEFORMATEX>|pywintypes|WAVEFORMATEX|Creates a new WAVEFORMATEX object
PyObject *PyWinMethod_NewWAVEFORMATEX(PyObject *self, PyObject *args)
{
    if (!PyArg_ParseTuple(args, ":WAVEFORMATEX"))
        return NULL;
    return new PyWAVEFORMATEX();
}

PyObject *PyWinObject_FromWAVEFORMATEX(const WAVEFORMATEX &wfx) { return new PyWAVEFORMATEX(wfx); }

BOOL PyWinObject_AsWAVEFORMATEX(PyObject *ob, WAVEFORMATEX **ppWAVEFORMATEX, BOOL bNoneOK /*= TRUE*/)
{
    if (bNoneOK && ob == Py_None) {
        *ppWAVEFORMATEX = NULL;
    }
    else if (!PyWAVEFORMATEX_Check(ob)) {
        PyErr_SetString(PyExc_TypeError, "The object is not a PyWAVEFORMATEX object");
        return FALSE;
    }
    else {
        PyWAVEFORMATEX *pywfx = (PyWAVEFORMATEX *)ob;
        *ppWAVEFORMATEX = &pywfx->m_wfx;
    }
    return TRUE;
}

// @object PyWAVEFORMATEX|A Python object, representing a WAVEFORMATEX structure
static struct PyMethodDef PyWAVEFORMATEX_methods[] = {{NULL}};

PYWINTYPES_EXPORT PyTypeObject PyWAVEFORMATEXType = {
    PYWIN_OBJECT_HEAD "PyWAVEFORMATEX",
    sizeof(PyWAVEFORMATEX),
    0,
    PyWAVEFORMATEX::deallocFunc,
    0,  // tp_print;
    0,  // tp_getattr
    0,  // tp_setattr
    0,  // tp_compare
    0,  // tp_repr
    0,  // tp_as_number
    0,  // tp_as_sequence
    0,  // tp_as_mapping
    0,
    0, /* tp_call */
    0, /* tp_str */
    PyObject_GenericGetAttr,
    PyObject_GenericSetAttr,
    0,                                         // tp_as_buffer;
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,  // tp_flags;
    0,                                         // tp_doc; /* Documentation string */
    0,                                         // traverseproc tp_traverse;
    0,                                         // tp_clear;
    0,                                         // tp_richcompare;
    0,                                         // tp_weaklistoffset;
    0,                                         // tp_iter
    0,                                         // iternextfunc tp_iternext
    0,                                         // methods
    PyWAVEFORMATEX::members,
    0,  // tp_getset;
    0,  // tp_base;
    0,  // tp_dict;
    0,  // tp_descr_get;
    0,  // tp_descr_set;
    0,  // tp_dictoffset;
    0,  // tp_init;
    0,  // tp_alloc;
    0   // newfunc tp_new;
};

#define OFF(e) offsetof(PyWAVEFORMATEX, e)

/*static*/ struct PyMemberDef PyWAVEFORMATEX::members[] = {
    {"wFormatTag", T_SHORT, OFF(m_wfx.wFormatTag), 0,
     "Format as an integer. WAVE_FORMAT_PCM (1) is very common."},  // @prop integer|wFormatTag|Waveform-audio format
                                                                    // type. pywintypes only defines WAVE_FORMAT_PCM as
                                                                    // a constant. Other values must be looked up in the
                                                                    // mmsystem.h header file.
    {"nChannels", T_SHORT, OFF(m_wfx.nChannels), 0,
     "Number of channels"},  // @prop integer|nChannels|Number of channels. 1 for Mono, 2 for Stereo, anything, but
                             // never 5.1.
    {"nSamplesPerSec", T_INT, OFF(m_wfx.nSamplesPerSec), 0,
     "Sample rate in seconds"},  // @prop integer|nSamplesPerSec|Sample rate, in samples per second (hertz), that each
                                 // channel should be played or recorded. If wFormatTag is WAVE_FORMAT_PCM, then common
                                 // values for nSamplesPerSec are 8000, 11025, 22050, and 44100 Hz
    {"nAvgBytesPerSec", T_INT, OFF(m_wfx.nAvgBytesPerSec), 0,
     "Required average data-transfer rate, in bytes per second, for the format tag. If wFormatTag is WAVE_FORMAT_PCM, "
     "nAvgBytesPerSec should be equal to the product of nSamplesPerSec and nBlockAlign."},  // @prop
                                                                                            // integer|nAvgBytesPerSec|Required
                                                                                            // average data-transfer
                                                                                            // rate, in bytes per
                                                                                            // second, for the format
                                                                                            // tag. If wFormatTag is
                                                                                            // WAVE_FORMAT_PCM,
                                                                                            // nAvgBytesPerSec should be
                                                                                            // equal to the product of
                                                                                            // nSamplesPerSec and
                                                                                            // nBlockAlign.
    {"nBlockAlign", T_SHORT, OFF(m_wfx.nBlockAlign), 0,
     "Block alignment, in bytes. The block alignment is the minimum atomic unit of data for the wFormatTag format "
     "type. If wFormatTag is WAVE_FORMAT_PCM, nBlockAlign should be equal to the product of nChannels and "
     "wBitsPerSample divided by 8 (bits per byte)."},  // @prop integer|nBlockAlign|Block alignment, in bytes. The block
                                                       // alignment is the minimum atomic unit of data for the
                                                       // wFormatTag format type. If wFormatTag is WAVE_FORMAT_PCM,
                                                       // nBlockAlign should be equal to the product of nChannels and
                                                       // wBitsPerSample divided by 8 (bits per byte). For non-PCM
                                                       // formats, this member must be computed according to the
                                                       // manufacturer’s specification of the format tag.
    {"wBitsPerSample", T_SHORT, OFF(m_wfx.wBitsPerSample), 0,
     "Bits per sample for the wFormatTag format type. If wFormatTag is WAVE_FORMAT_PCM, then wBitsPerSample should be "
     "equal to 8 or 16."},  // @prop integer|wBitsPerSample|Bits per sample for the wFormatTag format type. If
                            // wFormatTag is WAVE_FORMAT_PCM, then wBitsPerSample should be equal to 8 or 16.
    {NULL}                  /* Sentinel */
};

PyWAVEFORMATEX::PyWAVEFORMATEX(void)
{
    ob_type = &PyWAVEFORMATEXType;
    _Py_NewReference(this);
    memset(&m_wfx, 0, sizeof(m_wfx));
}

PyWAVEFORMATEX::PyWAVEFORMATEX(const WAVEFORMATEX &wfx)
{
    ob_type = &PyWAVEFORMATEXType;
    _Py_NewReference(this);
    m_wfx = wfx;
    m_wfx.cbSize = 0;
}

PyWAVEFORMATEX::~PyWAVEFORMATEX() {}

/*static*/ void PyWAVEFORMATEX::deallocFunc(PyObject *ob) { delete (PyWAVEFORMATEX *)ob; }
