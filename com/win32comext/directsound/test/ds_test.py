import unittest
import struct
import sys
import pywintypes
import win32event
# sys.path = ['.'] + sys.path
import win32com.directsound.directsound as ds
# next two lines are for for debugging:
# import win32com
# import directsound as ds


WAV_FORMAT_PCM = 1
WAV_HEADER_SIZE = struct.calcsize('<4sl4s4slhhllhh4sl')

def wav_header_unpack(data):
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

class WAVEFORMATTest(unittest.TestCase):
    def test_1_Type(self):
        'WAVEFORMATEX type'
        w = pywintypes.WAVEFORMATEX()
        self.failUnless(type(w) == pywintypes.WAVEFORMATEXType)

    def test_2_Attr(self):
        'WAVEFORMATEX attribute access'
        # A wav header for a soundfile from a CD should look like this...
        w = pywintypes.WAVEFORMATEX()
        w.wFormatTag = pywintypes.WAVE_FORMAT_PCM
        w.nChannels = 2
        w.nSamplesPerSec = 44100
        w.nAvgBytesPerSec = 176400
        w.nBlockAlign = 4
        w.wBitsPerSample = 16

        self.failUnless(w.wFormatTag == 1)
        self.failUnless(w.nChannels == 2)
        self.failUnless(w.nSamplesPerSec == 44100)
        self.failUnless(w.nAvgBytesPerSec == 176400)
        self.failUnless(w.nBlockAlign == 4)
        self.failUnless(w.wBitsPerSample == 16)

class DSCAPSTest(unittest.TestCase):
    def test_1_Type(self):
        'DSCAPS type'
        c = ds.DSCAPS()
        self.failUnless(type(c) == ds.DSCAPSType)

    def test_2_Attr(self):
        'DSCAPS attribute access'
        c = ds.DSCAPS()
        c.dwFlags = 1
        c.dwMinSecondarySampleRate = 2
        c.dwMaxSecondarySampleRate = 3
        c.dwPrimaryBuffers = 4
        c.dwMaxHwMixingAllBuffers = 5
        c.dwMaxHwMixingStaticBuffers = 6
        c.dwMaxHwMixingStreamingBuffers = 7
        c.dwFreeHwMixingAllBuffers = 8
        c.dwFreeHwMixingStaticBuffers = 9
        c.dwFreeHwMixingStreamingBuffers = 10
        c.dwMaxHw3DAllBuffers = 11
        c.dwMaxHw3DStaticBuffers = 12
        c.dwMaxHw3DStreamingBuffers = 13
        c.dwFreeHw3DAllBuffers = 14
        c.dwFreeHw3DStaticBuffers = 15
        c.dwFreeHw3DStreamingBuffers = 16
        c.dwTotalHwMemBytes = 17
        c.dwFreeHwMemBytes = 18
        c.dwMaxContigFreeHwMemBytes = 19
        c.dwUnlockTransferRateHwBuffers = 20
        c.dwPlayCpuOverheadSwBuffers = 21

        self.failUnless(c.dwFlags == 1)
        self.failUnless(c.dwMinSecondarySampleRate == 2)
        self.failUnless(c.dwMaxSecondarySampleRate == 3)
        self.failUnless(c.dwPrimaryBuffers == 4)
        self.failUnless(c.dwMaxHwMixingAllBuffers == 5)
        self.failUnless(c.dwMaxHwMixingStaticBuffers == 6)
        self.failUnless(c.dwMaxHwMixingStreamingBuffers == 7)
        self.failUnless(c.dwFreeHwMixingAllBuffers == 8)
        self.failUnless(c.dwFreeHwMixingStaticBuffers == 9)
        self.failUnless(c.dwFreeHwMixingStreamingBuffers == 10)
        self.failUnless(c.dwMaxHw3DAllBuffers == 11)
        self.failUnless(c.dwMaxHw3DStaticBuffers == 12)
        self.failUnless(c.dwMaxHw3DStreamingBuffers == 13)
        self.failUnless(c.dwFreeHw3DAllBuffers == 14)
        self.failUnless(c.dwFreeHw3DStaticBuffers == 15)
        self.failUnless(c.dwFreeHw3DStreamingBuffers == 16)
        self.failUnless(c.dwTotalHwMemBytes == 17)
        self.failUnless(c.dwFreeHwMemBytes == 18)
        self.failUnless(c.dwMaxContigFreeHwMemBytes == 19)
        self.failUnless(c.dwUnlockTransferRateHwBuffers == 20)
        self.failUnless(c.dwPlayCpuOverheadSwBuffers == 21)

class DSBCAPSTest(unittest.TestCase):
    def test_1_Type(self):
        'DSBCAPS type'
        c = ds.DSBCAPS()
        self.failUnless(type(c) == ds.DSBCAPSType)

    def test_2_Attr(self):
        'DSBCAPS attribute access'
        c = ds.DSBCAPS()
        c.dwFlags = 1
        c.dwBufferBytes = 2
        c.dwUnlockTransferRate = 3
        c.dwPlayCpuOverhead = 4

        self.failUnless(c.dwFlags == 1)
        self.failUnless(c.dwBufferBytes == 2)
        self.failUnless(c.dwUnlockTransferRate == 3)
        self.failUnless(c.dwPlayCpuOverhead == 4)

class DSBUFFERDESCTest(unittest.TestCase):
    def test_1_Type(self):
        'DSBUFFERDESC type'
        c = ds.DSBUFFERDESC()
        self.failUnless(type(c) == ds.DSBUFFERDESCType)

    def test_2_Attr(self):
        'DSBUFFERDESC attribute access'
        c = ds.DSBUFFERDESC()
        c.dwFlags = 1
        c.dwBufferBytes = 2
        c.lpwfxFormat = pywintypes.WAVEFORMATEX()
        c.lpwfxFormat.wFormatTag = pywintypes.WAVE_FORMAT_PCM
        c.lpwfxFormat.nChannels = 2
        c.lpwfxFormat.nSamplesPerSec = 44100
        c.lpwfxFormat.nAvgBytesPerSec = 176400
        c.lpwfxFormat.nBlockAlign = 4
        c.lpwfxFormat.wBitsPerSample = 16

        self.failUnless(c.dwFlags == 1)
        self.failUnless(c.dwBufferBytes == 2)
        self.failUnless(c.lpwfxFormat.wFormatTag == 1)
        self.failUnless(c.lpwfxFormat.nChannels == 2)
        self.failUnless(c.lpwfxFormat.nSamplesPerSec == 44100)
        self.failUnless(c.lpwfxFormat.nAvgBytesPerSec == 176400)
        self.failUnless(c.lpwfxFormat.nBlockAlign == 4)
        self.failUnless(c.lpwfxFormat.wBitsPerSample == 16)

    def invalid_format(self, c):
        c.lpwfxFormat = 17

    def test_3_invalid_format(self):
        'DSBUFFERDESC invalid lpwfxFormat assignment'
        c = ds.DSBUFFERDESC()
        self.failUnlessRaises(ValueError, self.invalid_format, c)

class DirectSoundTest(unittest.TestCase):
    # basic tests - mostly just exercise the functions
    
    def testEnumerate(self):
        '''DirectSoundEnumerate() sanity tests'''

        devices = ds.DirectSoundEnumerate()
        # this might fail on machines without a sound card
        self.failUnless(len(devices))
        # if we have an entry, it must be a tuple of size 3
        self.failUnless(len(devices[0]) == 3)
        
    def testCreate(self):
        '''DirectSoundCreate()'''
        d = ds.DirectSoundCreate(None, None)

    def testPlay(self):
        '''Play a file'''

        f = open('d:/temp/01-Intro.wav', 'rb')
        hdr = f.read(WAV_HEADER_SIZE)
        wfx, size = wav_header_unpack(hdr)

        d = ds.DirectSoundCreate(None, None)
        d.SetCooperativeLevel(None, ds.DSSCL_PRIORITY)

        sdesc = ds.DSBUFFERDESC()
        sdesc.dwFlags = ds.DSBCAPS_PRIMARYBUFFER
        sdesc.dwBufferBytes = 0
        sdesc.lpwfxFormat = None

        # create primary buffer
        primary = d.CreateSoundBuffer(sdesc, None)

        sdesc.dwFlags = ds.DSBCAPS_STICKYFOCUS | ds.DSBCAPS_CTRLPOSITIONNOTIFY
        sdesc.dwBufferBytes = size
        sdesc.lpwfxFormat = wfx

        secondary = d.CreateSoundBuffer(sdesc, None)

        event = win32event.CreateEvent(None, 0, 0, None)
        notify = secondary.QueryInterface(ds.IID_IDirectSoundNotify)

        notify.SetNotificationPositions((ds.DSBPN_OFFSETSTOP, event))

        secondary.Update(0, f.read(size))

        secondary.Play(0)

        win32event.WaitForSingleObject(event, -1)

if __name__ == '__main__':
    unittest.main()
