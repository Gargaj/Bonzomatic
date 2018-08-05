#include <bass.h>
#include <stdio.h>
#include "../FFT.h"

namespace FFT
{
  FFT_SETTINGS * settings;
  HRECORD hRecord = NULL;
  bool Open( FFT_SETTINGS * settings )
  {
    FFT::settings = settings;
    const int freq = 44100;
    const int channels = 1;
    int device = -1;

	if (settings->bLoopback)
	{
      printf("[FFT] This OS does not support loopback. Microphone will be used instead.\n");
	}

    if( !BASS_RecordInit( device ) )
    {
      printf("[FFT] BASS_RecordInit failed: %08X\n",BASS_ErrorGetCode());
      return false;
    }

    hRecord = BASS_RecordStart( freq, channels, BASS_SAMPLE_8BITS, 0, 0 );
    if (!hRecord)
    {
      printf("[FFT] BASS_RecordStart failed: %08X\n",BASS_ErrorGetCode());
      return false;
    }
    return true;
  }
  bool GetFFT( float * samples )
  {
    if (!hRecord)
      return false;

    unsigned int len = 0;

    switch(settings->nFFTsize * 2) // for 256 fft, only 128 values will contain DC in our case
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
	    fprintf(stderr, "[FFT] BASS invalid fft window size\n");
	    Close();
        break;
    }

    const int numBytes = BASS_ChannelGetData( hRecord, samples, len | BASS_DATA_FFT_REMOVEDC );
    if( numBytes <= 0 )
      return false;

    return true;
  }
  void Close()
  {
    if (hRecord)
    {
      BASS_ChannelStop( hRecord );
      hRecord = NULL;
    }

    BASS_RecordFree();
  }
}