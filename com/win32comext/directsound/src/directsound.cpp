// directsound.cpp :
// $Id$

// directsound wrapper contributed by Lars Immisch <lars@ibp.de>

/***
Note that this source file contains embedded documentation.
This documentation consists of marked up text inside the
C comments, and is prefixed with an '@' symbol.  The source
files are processed by a tool called "autoduck" which
generates Windows .hlp files.
@doc
***/

#include "directsound_pch.h"
#include "stddef.h"             // for offsetof
#include "PythonCOMRegister.h"  // For simpler registration of IIDs etc.
#include "PyIDirectSound.h"
#include "PyIDirectSoundBuffer.h"
#include "PyIDirectSoundNotify.h"
#include "PyIDirectSoundCapture.h"
#include "PyIDirectSoundCaptureBuffer.h"

// @pymethod <o PyIUnknown>|directsound|DirectSoundCreate|Creates and initializes a new object that supports the
// IDirectSound interface.
static PyObject *directsound_DirectSoundCreate(PyObject *, PyObject *args)
{
    PyObject *ret = NULL;
    PyObject *obGUID = NULL, *obUnk = NULL;
    IUnknown *pUnkIn = NULL;
    GUID guid, *pguid = NULL;
    LPDIRECTSOUND ds;
    HRESULT hr;

    if (!PyArg_ParseTuple(args, "|OO:DirectSoundCreate",
                          &obGUID,  // @pyparm <o PyIID>|guid|None|Address of the GUID that identifies the sound device.
                                    // The value of this parameter must be one of the GUIDs returned by
                                    // DirectSoundEnumerate, or None for the default device.
                          &obUnk))  // @pyparm <o PyIUknown>|unk|None|The IUnknown for COM aggregation.
    {
        return NULL;
    }

    if (obUnk) {
        if (!PyCom_InterfaceFromPyInstanceOrObject(obUnk, IID_IUnknown, (void **)&pUnkIn, TRUE))
            goto done;
    }

    if (obGUID && obGUID != Py_None) {
        if (!PyWinObject_AsIID(obGUID, &guid))
            goto done;

        pguid = &guid;
    }

    Py_BEGIN_ALLOW_THREADS hr = ::DirectSoundCreate(pguid, &ds, pUnkIn);
    Py_END_ALLOW_THREADS if (FAILED(hr))
    {
        PyCom_BuildPyException(hr);
        goto done;
    }
    ret = new PyIDirectSound(ds);
done:
    if (pUnkIn)
        pUnkIn->Release();

    return ret;
}

BOOL CALLBACK dsEnumCallback(LPGUID guid, LPCTSTR desc, LPCTSTR module, LPVOID context)
{
    PyObject *list = (PyObject *)context;
    PyObject *item = PyTuple_New(3);
    PyObject *oguid;

    // abort enumeration if we cannot create a tuple
    if (!item) {
        return FALSE;
    }

    if (guid) {
        oguid = PyWinObject_FromIID(*guid);
    }
    else {
        Py_INCREF(Py_None);
        oguid = Py_None;
    }

    if (PyTuple_SetItem(item, 0, oguid))
        return FALSE;

    if (PyTuple_SetItem(item, 1, PyWinObject_FromTCHAR(desc)))
        return FALSE;

    if (PyTuple_SetItem(item, 2, PyWinObject_FromTCHAR(module)))
        return FALSE;

    if (PyList_Append(list, item))
        return FALSE;

    return TRUE;
}

// @pymethod <o list>|directsound|DirectSoundEnumerate|Enumerates DirectSound drivers installed in the system.
static PyObject *directsound_DirectSoundEnumerate(PyObject *, PyObject *args)
{
    if (!PyArg_ParseTuple(args, ":DirectSoundEnumerate")) {
        return NULL;
    }

    PyObject *list = PyList_New(0);
    if (!list) {
        return NULL;
    }

    HRESULT hr;
    Py_BEGIN_ALLOW_THREADS hr = ::DirectSoundEnumerate(dsEnumCallback, list);
    Py_END_ALLOW_THREADS

        if (PyErr_Occurred())
    {
        Py_DECREF(list);
        return NULL;
    }

    if (FAILED(hr)) {
        Py_DECREF(list);
        PyCom_BuildPyException(hr);
        return NULL;
    }

    return list;
}

// @pymethod <o PyIUnknown>|directsound|DirectSoundCaptureCreate|Creates and initializes a new object that supports the
// IDirectSoundCapture interface.
static PyObject *directsound_DirectSoundCaptureCreate(PyObject *, PyObject *args)
{
    PyObject *ret = NULL;
    PyObject *obGUID = NULL, *obUnk = NULL;
    IUnknown *pUnkIn = NULL;
    GUID guid, *pguid = NULL;
    LPDIRECTSOUNDCAPTURE dsc;
    HRESULT hr;

    if (!PyArg_ParseTuple(args, "|OO:DirectSoundCaptureCreate",
                          &obGUID,  // @pyparm <o PyIID>|guid|None|Address of the GUID that identifies the sound device.
                                    // The value of this parameter must be one of the GUIDs returned by
                                    // DirectSoundCaptureEnumerate, or None for the default device.
                          &obUnk))  // @pyparm <o PyIUknown>|unk|None|The IUnknown for COM aggregation.
    {
        return NULL;
    }

    if (obUnk) {
        if (!PyCom_InterfaceFromPyInstanceOrObject(obUnk, IID_IUnknown, (void **)&pUnkIn, TRUE))
            goto done;
    }

    if (obGUID && obGUID != Py_None) {
        if (!PyWinObject_AsIID(obGUID, &guid))
            goto done;

        pguid = &guid;
    }

    Py_BEGIN_ALLOW_THREADS hr = ::DirectSoundCaptureCreate(pguid, &dsc, pUnkIn);
    Py_END_ALLOW_THREADS if (FAILED(hr))
    {
        PyCom_BuildPyException(hr);
        goto done;
    }
    ret = new PyIDirectSoundCapture(dsc);
done:
    if (pUnkIn)
        pUnkIn->Release();

    return ret;
}

// @pymethod <o list>|directsound|DirectSoundCaptureEnumerate|Enumerates DirectSoundCapture drivers installed in the
// system.
static PyObject *directsound_DirectSoundCaptureEnumerate(PyObject *, PyObject *args)
{
    if (!PyArg_ParseTuple(args, ":DirectSoundCaptureEnumerate")) {
        return NULL;
    }

    PyObject *list = PyList_New(0);
    if (!list) {
        return NULL;
    }

    HRESULT hr;
    Py_BEGIN_ALLOW_THREADS hr = ::DirectSoundCaptureEnumerate(dsEnumCallback, list);
    Py_END_ALLOW_THREADS

        if (PyErr_Occurred())
    {
        Py_DECREF(list);
        return NULL;
    }

    if (FAILED(hr)) {
        Py_DECREF(list);
        PyCom_BuildPyException(hr);
        return NULL;
    }

    return list;
}

/* List of module functions */
/* @module directsound|A module encapsulating the DirectSound interfaces. See <l DirectSound examples> for a quick
 * overview. */
static struct PyMethodDef directsound_methods[] = {
    {"DirectSoundCreate", directsound_DirectSoundCreate,
     1},  // @pymeth DirectSoundCreate|Creates and initializes a new object that supports the IDirectSound interface.
    {"DirectSoundEnumerate", directsound_DirectSoundEnumerate,
     1},  // @pymeth DirectSoundEnumerate|The DirectSoundEnumerate function enumerates the DirectSound drivers installed
          // in the system.
    {"DirectSoundCaptureCreate", directsound_DirectSoundCaptureCreate,
     1},  // @pymeth DirectSoundCaptureCreate|The DirectSoundCaptureCreate function creates and initializes an object
          // that supports the IDirectSoundCapture interface.
    {"DirectSoundCaptureEnumerate", directsound_DirectSoundCaptureEnumerate,
     1},  // @pymeth DirectSoundCaptureEnumerate|The DirectSoundCaptureEnumerate function enumerates the
          // DirectSoundCapture objects installed in the system.
    {"DSCAPS", PyWinMethod_NewDSCAPS, 1},              // @pymeth DSCAPS|Creates a new <o PyDSCAPS> object.
    {"DSBCAPS", PyWinMethod_NewDSBCAPS, 1},            // @pymeth DSBCAPS|Creates a new <o PyDSBCAPS> object.
    {"DSCCAPS", PyWinMethod_NewDSCCAPS, 1},            // @pymeth DSCCAPS|Creates a new <o PyDSCCAPS> object.
    {"DSCBCAPS", PyWinMethod_NewDSCBCAPS, 1},          // @pymeth DSCBCAPS|Creates a new <o PyDSCBCAPS> object.
    {"DSBUFFERDESC", PyWinMethod_NewDSBUFFERDESC, 1},  // @pymeth DSBUFFERDESC|Creates a new <o PyDSBUFFERDESC> object.
    {"DSCBUFFERDESC", PyWinMethod_NewDSCBUFFERDESC,
     1},  // @pymeth DSCBUFFERDESC|Creates a new <o PyDSCBUFFERDESC> object.
    {NULL, NULL},
};

#define ADD_CONSTANT(tok)                                 \
    if (PyModule_AddIntConstant(module, #tok, tok) == -1) \
        PYWIN_MODULE_INIT_RETURN_ERROR;

static const PyCom_InterfaceSupportInfo g_interfaceSupportData[] = {
    PYCOM_INTERFACE_CLIENT_ONLY(DirectSound),
    PYCOM_INTERFACE_CLIENT_ONLY(DirectSoundBuffer),
    PYCOM_INTERFACE_CLIENT_ONLY(DirectSoundNotify),
    PYCOM_INTERFACE_CLIENT_ONLY(DirectSoundCapture),
    PYCOM_INTERFACE_CLIENT_ONLY(DirectSoundCaptureBuffer),
};

/* Module initialisation */
PYWIN_MODULE_INIT_FUNC(directsound)
{
    PYWIN_MODULE_INIT_PREPARE(directsound, directsound_methods, "A module encapsulating the DirectSound interfaces.");

    // Register all of our interfaces, gateways and IIDs.
    PyCom_RegisterExtensionSupport(dict, g_interfaceSupportData,
                                   sizeof(g_interfaceSupportData) / sizeof(g_interfaceSupportData[0]));

    // @const directsound|DSCAPS_PRIMARYMONO|The device supports monophonic primary buffers.
    ADD_CONSTANT(DSCAPS_PRIMARYMONO);
    // @const directsound|DSCAPS_PRIMARYSTEREO|The device supports stereo primary buffers.
    ADD_CONSTANT(DSCAPS_PRIMARYSTEREO);
    // @const directsound|DSCAPS_PRIMARY8BIT|The device supports hardware-mixed secondary buffers with 8-bit samples.
    ADD_CONSTANT(DSCAPS_PRIMARY8BIT);
    // @const directsound|DSCAPS_PRIMARY16BIT|The device supports primary sound buffers with 16-bit samples.
    ADD_CONSTANT(DSCAPS_PRIMARY16BIT);
    // @const directsound|DSCAPS_CONTINUOUSRATE|The device supports all sample rates between the
    // dwMinSecondarySampleRate and dwMaxSecondarySampleRate member values. Typically, this means that the actual output
    // rate will be within +/- 10 hertz (Hz) of the requested frequency.
    ADD_CONSTANT(DSCAPS_CONTINUOUSRATE);
    // @const directsound|DSCAPS_EMULDRIVER|The device does not have a DirectSound driver installed, so it is being
    // emulated through the waveform-audio functions. Performance degradation should be expected.
    ADD_CONSTANT(DSCAPS_EMULDRIVER);
    // @const directsound|DSCAPS_CERTIFIED|This driver has been tested and certified by Microsoft.
    ADD_CONSTANT(DSCAPS_CERTIFIED);
    // @const directsound|DSCAPS_SECONDARYMONO|The device supports hardware-mixed monophonic secondary buffers.
    ADD_CONSTANT(DSCAPS_SECONDARYMONO);
    // @const directsound|DSCAPS_SECONDARYSTEREO|The device supports hardware-mixed stereo secondary buffers.
    ADD_CONSTANT(DSCAPS_SECONDARYSTEREO);
    // @const directsound|DSCAPS_SECONDARY8BIT|The device supports hardware-mixed secondary buffers with 8-bit samples.
    ADD_CONSTANT(DSCAPS_SECONDARY8BIT);
    // @const directsound|DSCAPS_SECONDARY16BIT|The device supports hardware-mixed secondary sound buffers with 16-bit
    // samples.
    ADD_CONSTANT(DSCAPS_SECONDARY16BIT);

    // @const directsound|DSBPLAY_LOOPING|Once the end of the audio buffer is reached, play restarts at the beginning of
    // the buffer. Play continues until explicitly stopped. This flag must be set when playing primary sound buffers.
    ADD_CONSTANT(DSBPLAY_LOOPING);
    // @const directsound|DSBSTATUS_PLAYING|The buffer is playing. If this value is not set, the buffer is stopped.
    ADD_CONSTANT(DSBSTATUS_PLAYING);
    // @const directsound|DSBSTATUS_BUFFERLOST|The buffer is lost and must be restored before it can be played or
    // locked.
    ADD_CONSTANT(DSBSTATUS_BUFFERLOST);
    // @const directsound|DSBSTATUS_LOOPING|The buffer is being looped. If this value is not set, the buffer will stop
    // when it reaches the end of the sound data. Note that if this value is set, the buffer must also be playing.
    ADD_CONSTANT(DSBSTATUS_LOOPING);
    // @const directsound|DSBLOCK_FROMWRITECURSOR|Locks from the current write cursor, making a call to
    // DirectSoundBuffer.getCurrentPosition unnecessary. If this flag is specified, the start parameter is ignored. This
    // flag is optional.
    ADD_CONSTANT(DSBLOCK_FROMWRITECURSOR);
    // @const directsound|DSBLOCK_ENTIREBUFFER|Unknown.
    ADD_CONSTANT(DSBLOCK_ENTIREBUFFER);
    // @const directsound|DSSCL_NORMAL|Sets the application to a fully cooperative status. Most applications should use
    // this level, because it has the smoothest multitasking and resource-sharing behavior.
    ADD_CONSTANT(DSSCL_NORMAL);
    // @const directsound|DSSCL_PRIORITY|Sets the application to the priority level. Applications with this cooperative
    // level can call the DirectSoundBuffer.setFormat and DirectSound.compact methods.
    ADD_CONSTANT(DSSCL_PRIORITY);
    // @const directsound|DSSCL_EXCLUSIVE|Sets the application to the exclusive level. When it has the input focus, the
    // application will be the only one audible (sounds from applications with the DSBCAPS_GLOBALFOCUS flag set will be
    // muted). With this level, it also has all the privileges of the DSSCL_PRIORITY level. DirectSound will restore the
    // hardware format, as specified by the most recent call to the DirectSoundBuffer.setFormat method, once the
    // application gains the input focus. (Note that DirectSound will always restore the wave format, no matter what
    // priority level is set.)
    ADD_CONSTANT(DSSCL_EXCLUSIVE);
    // @const directsound|DSSCL_WRITEPRIMARY|This is the highest priority level. The application has write access to the
    // primary sound buffers. No secondary sound buffers in any application can be played.
    ADD_CONSTANT(DSSCL_WRITEPRIMARY);
    // @const directsound|DS3DMODE_NORMAL|Normal processing. This is the default mode.
    ADD_CONSTANT(DS3DMODE_NORMAL);
    // @const directsound|DS3DMODE_HEADRELATIVE|Sound parameters (position, velocity, and orientation) are relative to
    // the listener's parameters. In this mode, the absolute parameters of the sound are updated automatically as the
    // listener's parameters change, so that the relative parameters remain constant.
    ADD_CONSTANT(DS3DMODE_HEADRELATIVE);
    // @const directsound|DS3DMODE_DISABLE|Processing of 3D sound is disabled. The sound seems to originate from the
    // center of the listener's head.
    ADD_CONSTANT(DS3DMODE_DISABLE);

    // @const directsound|DSBCAPS_PRIMARYBUFFER|Indicates that the buffer is a primary sound buffer. If this value is
    // not specified, a secondary sound buffer will be created.
    ADD_CONSTANT(DSBCAPS_PRIMARYBUFFER);
    // @const directsound|DSBCAPS_STATIC|Indicates that the buffer will be used for static sound data. Typically, these
    // buffers are loaded once and played many times. These buffers are candidates for hardware memory.
    ADD_CONSTANT(DSBCAPS_STATIC);
    // @const directsound|DSBCAPS_LOCHARDWARE|The buffer is in hardware memory and uses hardware mixing.
    ADD_CONSTANT(DSBCAPS_LOCHARDWARE);
    // @const directsound|DSBCAPS_LOCSOFTWARE|The buffer is in software memory and uses software mixing.
    ADD_CONSTANT(DSBCAPS_LOCSOFTWARE);
    // @const directsound|DSBCAPS_CTRL3D|The buffer is either a primary buffer or a secondary buffer that uses 3-D
    // control. To create a primary buffer, the dwFlags member of the DSBUFFERDESC structure should include the
    // DSBCAPS_PRIMARYBUFFER flag.
    ADD_CONSTANT(DSBCAPS_CTRL3D);
    // @const directsound|DSBCAPS_CTRLFREQUENCY|The buffer must have frequency control capability.
    ADD_CONSTANT(DSBCAPS_CTRLFREQUENCY);
    // @const directsound|DSBCAPS_CTRLPAN|The buffer must have pan control capability.
    ADD_CONSTANT(DSBCAPS_CTRLPAN);
    // @const directsound|DSBCAPS_CTRLVOLUME|The buffer must have volume control capability.
    ADD_CONSTANT(DSBCAPS_CTRLVOLUME);
    // @const directsound|DSBCAPS_CTRLPOSITIONNOTIFY|The buffer must have control position notify capability.
    ADD_CONSTANT(DSBCAPS_CTRLPOSITIONNOTIFY);
    // @const directsound|DSBCAPS_STICKYFOCUS|Changes the focus behavior of the sound buffer. This flag can be specified
    // in an IDirectSound::CreateSoundBuffer call. With this flag set, an application using DirectSound can continue to
    // play its sticky focus buffers if the user switches to another application not using DirectSound. In this
    // situation, the application's normal buffers are muted, but the sticky focus buffers are still audible. This is
    // useful for nongame applications, such as movie playback (DirectShow™), when the user wants to hear the soundtrack
    // while typing in Microsoft Word or Microsoft® Excel, for example. However, if the user switches to another
    // DirectSound application, all sound buffers, both normal and sticky focus, in the previous application are muted.
    ADD_CONSTANT(DSBCAPS_STICKYFOCUS);
    // @const directsound|DSBCAPS_GLOBALFOCUS|The buffer is a global sound buffer. With this flag set, an application
    // using DirectSound can continue to play its buffers if the user switches focus to another application, even if the
    // new application uses DirectSound. The one exception is if you switch focus to a DirectSound application that uses
    // the DSSCL_EXCLUSIVE or DSSCL_WRITEPRIMARY flag for its cooperative level. In this case, the global sounds from
    // other applications will not be audible.
    ADD_CONSTANT(DSBCAPS_GLOBALFOCUS);
    // @const directsound|DSBCAPS_GETCURRENTPOSITION2|Indicates that IDirectSoundBuffer::GetCurrentPosition should use
    // the new behavior of the play cursor. In DirectSound in DirectX 1, the play cursor was significantly ahead of the
    // actual playing sound on emulated sound cards; it was directly behind the write cursor. Now, if the
    // DSBCAPS_GETCURRENTPOSITION2 flag is specified, the application can get a more accurate play position. If this
    // flag is not specified, the old behavior is preserved for compatibility. Note that this flag affects only emulated
    // sound cards; if a DirectSound driver is present, the play cursor is accurate for DirectSound in all versions of
    // DirectX.
    ADD_CONSTANT(DSBCAPS_GETCURRENTPOSITION2);
    // @const directsound|DSBCAPS_MUTE3DATMAXDISTANCE|The sound is reduced to silence at the maximum distance. The
    // buffer will stop playing when the maximum distance is exceeded, so that processor time is not wasted.
    ADD_CONSTANT(DSBCAPS_MUTE3DATMAXDISTANCE);

    // @const directsound|DSCBCAPS_WAVEMAPPED|The Win32 wave mapper will be used for formats not supported by the
    // device.
    ADD_CONSTANT(DSCBCAPS_WAVEMAPPED);

    // @const directsound|DSSPEAKER_HEADPHONE|The speakers are headphones.
    ADD_CONSTANT(DSSPEAKER_HEADPHONE);
    // @const directsound|DSSPEAKER_MONO|The speakers are monaural.
    ADD_CONSTANT(DSSPEAKER_MONO);
    // @const directsound|DSSPEAKER_QUAD|The speakers are quadraphonic.
    ADD_CONSTANT(DSSPEAKER_QUAD);
    // @const directsound|DSSPEAKER_STEREO|The speakers are stereo (default value).
    ADD_CONSTANT(DSSPEAKER_STEREO);
    // @const directsound|DSSPEAKER_SURROUND|The speakers are surround sound.
    ADD_CONSTANT(DSSPEAKER_SURROUND);
    // @const directsound|DSSPEAKER_GEOMETRY_MIN|The speakers are directed over an arc of 5 degrees.
    ADD_CONSTANT(DSSPEAKER_GEOMETRY_MIN);
    // @const directsound|DSSPEAKER_GEOMETRY_NARROW|The speakers are directed over an arc of 10 degrees.
    ADD_CONSTANT(DSSPEAKER_GEOMETRY_NARROW);
    // @const directsound|DSSPEAKER_GEOMETRY_WIDE|The speakers are directed over an arc of 20 degrees.
    ADD_CONSTANT(DSSPEAKER_GEOMETRY_WIDE);
    // @const directsound|DSSPEAKER_GEOMETRY_MAX|The speakers are directed over an arc of 180 degrees.
    ADD_CONSTANT(DSSPEAKER_GEOMETRY_MAX);
    // real macros - todo if can be bothered
    // ADD_CONSTANT(DSSPEAKER_COMBINED);
    // ADD_CONSTANT(DSSPEAKER_CONFIG);
    // ADD_CONSTANT(DSSPEAKER_GEOMETRY);
    ADD_CONSTANT(DSBFREQUENCY_MIN);
    ADD_CONSTANT(DSBFREQUENCY_MAX);
    ADD_CONSTANT(DSBFREQUENCY_ORIGINAL);
    ADD_CONSTANT(DSBPAN_LEFT);
    ADD_CONSTANT(DSBPAN_CENTER);
    ADD_CONSTANT(DSBPAN_RIGHT);
    ADD_CONSTANT(DSBVOLUME_MIN);
    ADD_CONSTANT(DSBVOLUME_MAX);
    ADD_CONSTANT(DSBSIZE_MIN);
    ADD_CONSTANT(DSBSIZE_MAX);
    // @const directsound|DSCCAPS_EMULDRIVER|The device does not have a DirectSound driver installed, so it is being
    // emulated through the waveform-audio functions. Performance degradation should be expected.
    ADD_CONSTANT(DSCCAPS_EMULDRIVER);
    ADD_CONSTANT(DSCBLOCK_ENTIREBUFFER);
    ADD_CONSTANT(DSCBSTATUS_CAPTURING);
    ADD_CONSTANT(DSCBSTATUS_LOOPING);
    ADD_CONSTANT(DSCBSTART_LOOPING);
    ADD_CONSTANT(DSBPN_OFFSETSTOP);

    if (PyType_Ready(&PyDSCAPSType) == -1 || PyType_Ready(&PyDSBCAPSType) == -1 ||
        PyType_Ready(&PyDSBUFFERDESCType) == -1 || PyType_Ready(&PyDSCCAPSType) == -1 ||
        PyType_Ready(&PyDSCBCAPSType) == -1 || PyType_Ready(&PyDSCBUFFERDESCType) == -1 ||
        PyDict_SetItemString(dict, "DSCAPSType", (PyObject *)&PyDSCAPSType) == -1 ||
        PyDict_SetItemString(dict, "DSBCAPSType", (PyObject *)&PyDSBCAPSType) == -1 ||
        PyDict_SetItemString(dict, "DSBUFFERDESCType", (PyObject *)&PyDSBUFFERDESCType) == -1 ||
        PyDict_SetItemString(dict, "DSCCAPSType", (PyObject *)&PyDSCCAPSType) == -1 ||
        PyDict_SetItemString(dict, "DSCBCAPSType", (PyObject *)&PyDSCBCAPSType) == -1 ||
        PyDict_SetItemString(dict, "DSCBUFFERDESCType", (PyObject *)&PyDSCBUFFERDESCType) == -1)
        PYWIN_MODULE_INIT_RETURN_ERROR;

    PYWIN_MODULE_INIT_RETURN_SUCCESS;
}

/* @topic DirectSound examples|

@ex Our raison d'etre - playing sounds:|

WAV_HEADER_SIZE = struct.calcsize('<4sl4s4slhhllhh4sl')

def wav_header_unpack(data):
    '''Unpack a wav header and stuff it into a WAVEFORMATEX structure'''
    (riff, riffsize, wave, fmt, fmtsize, format, nchannels, samplespersecond,
     datarate, blockalign, bitspersample, data, datalength) \
     = struct.unpack('<4sl4s4slhhllhh4sl', data)

    if riff != 'RIFF' or fmtsize != 16 or fmt != 'fmt ' or data != 'data':
        raise ValueError, 'illegal wav header'

    wfx = pywintypes.WAVEFORMATEX()
    wfx.wFormatTag = format
    wfx.nChannels = nchannels
    wfx.nSamplesPerSec = samplespersecond
    wfx.nAvgBytesPerSec = datarate
    wfx.nBlockAlign = blockalign
    wfx.wBitsPerSample = bitspersample

    return wfx, datalength

# Play a wav file and wait until it's finished
fname = os.path.join(os.path.dirname(__file__), "01-Intro.wav")
f = open(fname, 'rb')

# Read and unpack the wav header
hdr = f.read(WAV_HEADER_SIZE)
wfx, size = wav_header_unpack(hdr)

d = ds.DirectSoundCreate(None, None)
d.SetCooperativeLevel(None, ds.DSSCL_PRIORITY)

sdesc = ds.DSBUFFERDESC()
sdesc.dwFlags = ds.DSBCAPS_STICKYFOCUS | ds.DSBCAPS_CTRLPOSITIONNOTIFY
sdesc.dwBufferBytes = size
sdesc.lpwfxFormat = wfx

buffer = d.CreateSoundBuffer(sdesc, None)

event = win32event.CreateEvent(None, 0, 0, None)

  notify = buffer.QueryInterface(ds.IID_IDirectSoundNotify)
notify.SetNotificationPositions((ds.DSBPN_OFFSETSTOP, event))

buffer.Update(0, f.read(size))
buffer.Play(0)
win32event.WaitForSingleObject(event, -1)

@ex This example shows how to record into a wav file:|

import pywintypes
import struct
import win32event
import win32com.directsound.directsound as ds

def wav_header_pack(wfx, datasize):
    return struct.pack('<4sl4s4slhhllhh4sl', 'RIFF', 36 + datasize,
                       'WAVE', 'fmt ', 16,
                       wfx.wFormatTag, wfx.nChannels, wfx.nSamplesPerSec,
                       wfx.nAvgBytesPerSec, wfx.nBlockAlign,
                       wfx.wBitsPerSample, 'data', datasize);

d = ds.DirectSoundCaptureCreate(None, None)

sdesc = ds.DSCBUFFERDESC()
sdesc.dwBufferBytes = 352800 # 2 seconds
sdesc.lpwfxFormat = pywintypes.WAVEFORMATEX()
sdesc.lpwfxFormat.wFormatTag = pywintypes.WAVE_FORMAT_PCM
sdesc.lpwfxFormat.nChannels = 2
sdesc.lpwfxFormat.nSamplesPerSec = 44100
sdesc.lpwfxFormat.nAvgBytesPerSec = 176400
sdesc.lpwfxFormat.nBlockAlign = 4
sdesc.lpwfxFormat.wBitsPerSample = 16

buffer = d.CreateCaptureBuffer(sdesc)

event = win32event.CreateEvent(None, 0, 0, None)
notify = buffer.QueryInterface(ds.IID_IDirectSoundNotify)

notify.SetNotificationPositions((ds.DSBPN_OFFSETSTOP, event))

buffer.Start(0)

win32event.WaitForSingleObject(event, -1)

# in real life, more, smaller buffers should be retrieved
data = buffer.Update(0, 352800)

f = open('recording.wav', 'wb')
f.write(wav_header_pack(sdesc.lpwfxFormat, 352800))
f.write(data)
*/
