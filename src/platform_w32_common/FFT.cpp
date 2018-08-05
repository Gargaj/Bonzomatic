#include <bass.h>
#include <basswasapi.h>
#include <stdio.h>
#include "../FFT.h"

namespace FFT
{
  FFT_SETTINGS * settings;
  HRECORD hRecord = NULL;
  bool bActive = false;
  DWORD CALLBACK OutWasapiProc(void *buffer, DWORD length, void *user)
  {
	  return length;
  }
  int GetLoopbackDevice()
  {
	  const DWORD flags = BASS_DEVICE_ENABLED | BASS_DEVICE_LOOPBACK | BASS_DEVICE_INPUT;
	  BASS_WASAPI_DEVICEINFO info;
	  for (DWORD deviceId = 0; BASS_WASAPI_GetDeviceInfo(deviceId, &info); deviceId++) {
		  if ((info.flags & flags) == flags)
			  return deviceId;
	  }
	  return -1;
  }
  bool Open( FFT_SETTINGS * settings )
  {
    FFT::settings = settings;
    const int freq = 44100;
    const int channels = 1;
    int device = -1;

    if (settings->bLoopback)
    {
      if (!BASS_Init(0, freq, BASS_DEVICE_DEFAULT, 0, NULL))
      {
        printf("[FFT] BASS_Init failed: %08X\n", BASS_ErrorGetCode());
        return false;
      }

      device = GetLoopbackDevice();
      if (!BASS_WASAPI_Init(device, 0, 0, BASS_WASAPI_BUFFER, 0.1, 0, OutWasapiProc, NULL))
      {
        printf("[FFT] BASS_WASAPI_Init failed: %08X\n", BASS_ErrorGetCode());
        return false;
      }

      if (!BASS_WASAPI_Start())
      {
        printf("[FFT] BASS_WASAPI_Start failed: %08X\n", BASS_ErrorGetCode());
        return false;
      }
    }
    else
    {
      if (!BASS_RecordInit(device))
      {
        printf("[FFT] BASS_RecordInit failed: %08X\n", BASS_ErrorGetCode());
        return false;
      }

      hRecord = BASS_RecordStart(freq, channels, BASS_SAMPLE_8BITS, 0, 0);
      if (!hRecord)
      {
        printf("[FFT] BASS_RecordStart failed: %08X\n", BASS_ErrorGetCode());
        return false;
      }
    }
    bActive = true;
    return true;
  }
  bool GetFFT(float * samples)
  {
    if (!bActive)
      return false;

    unsigned int len = 0;

    switch (settings->nFFTsize * 2) // for 256 fft, only 128 values will contain DC in our case
    {
    case 256:
      len = BASS_DATA_FFT256;
      break;
    case 512:
      len = BASS_DATA_FFT512;
      break;
    case 1024:
      len = BASS_DATA_FFT1024;
      break;
    case 2048:
      len = BASS_DATA_FFT2048;
      break;
    case 4096:
      len = BASS_DATA_FFT4096;
      break;
    case 8192:
      len = BASS_DATA_FFT8192;
      break;
    case 16384:
      len = BASS_DATA_FFT16384;
      break;
    case 32768:
      len = BASS_DATA_FFT32768;
      break;
    default:
      fprintf( stderr, "[FFT] BASS invalid fft window size\n" );
      Close();
      break;
    }

    const int numBytes = settings->bLoopback ? BASS_WASAPI_GetData(samples, len | BASS_DATA_FFT_REMOVEDC) :
                                               BASS_ChannelGetData(hRecord, samples, len | BASS_DATA_FFT_REMOVEDC);
    if (numBytes <= 0)
      return false;

    return true;
  }
  void Close()
  {
    bActive = false;

    if (hRecord)
    {
      BASS_ChannelStop(hRecord);
      hRecord = NULL;
    }
    if (BASS_WASAPI_IsStarted())
    {
      BASS_WASAPI_Stop(true);
    }

    BASS_WASAPI_Free();
    BASS_RecordFree();
  }
}