#include <bass.h>
#include <stdio.h>
#include "../FFT.h"

namespace FFT
{
  HRECORD hRecord = NULL;
  bool Open()
  {
    const int freq = 44100;
    int device = -1;

    int res = BASS_Init( device, freq, 0, 0, 0 );
    if( !res )
    {
      printf("[FFT] BASS_Init failed: %08X\n",res);
      return false;
    }

    res = BASS_RecordInit( device );
    if( !res )
    {
      printf("[FFT] BASS_RecordInit failed: %08X\n",res);
      return false;
    }

    //DWORD usedDevice = BASS_RecordGetDevice();

    const int channels = 1;

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

    switch( FFT_SIZE*2 ) // for 256 fft, only 128 values will contain DC in our case
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
      default:
        //fprintf( stderr, "BASS invalid fft window size\n" );
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
    BASS_Free();
  }
}
