#define MINI_AL_IMPLEMENTATION
#include <mini_al.h>

#include <kiss_fft.h>
#include <kiss_fftr.h>
#include <stdio.h>
#include <memory.h>
#include "FFT.h"

namespace FFT
{
  kiss_fftr_cfg fftcfg;
  mal_context context;
  mal_device captureDevice;
  float lastFFT[ FFT_SIZE ];
  float sampleBuf[ FFT_SIZE * 2 ];
  int sampleCursor = 0;

  void OnReceiveFrames( mal_device* pDevice, mal_uint32 frameCount, const void* pSamples )
  {
    const mal_int16 * samples = (const mal_int16 *)pSamples;
    for ( int i = 0; i < frameCount; i++ )
    {
      sampleBuf[ sampleCursor++ ] = ( samples[ 0 ] + samples[ 1 ] ) / 65535.0f; // int16 max * 2 -> -1..1
      samples += 2;      
      if ( sampleCursor == FFT_SIZE * 2 )
      {
        kiss_fft_cpx out[ FFT_SIZE + 1 ];
        kiss_fftr( fftcfg, sampleBuf, out );

        for ( int i = 0; i < FFT_SIZE; i++ )
        {
          static const float scaling = 1.0f / (float)FFT_SIZE;
          lastFFT[ i ] = 2.0 * sqrtf( out[ i ].r*out[ i ].r + out[ i ].i*out[ i ].i ) * scaling;
        }

        sampleCursor = 0;
      }
    }
  }

  bool Open()
  {
    memset( lastFFT, 0, sizeof( float ) * FFT_SIZE );

    fftcfg = kiss_fftr_alloc( FFT_SIZE * 2, false, NULL, NULL );

    if ( mal_context_init( NULL, 0, NULL, &context ) != MAL_SUCCESS )
    {
      printf( "[FFT] Failed to initialize context." );
      return false;
    }

    mal_device_config config = mal_device_config_init( mal_format_s16, 2, 44100, OnReceiveFrames, NULL );
    
    if ( mal_device_init( &context, mal_device_type_capture, NULL, &config, NULL, &captureDevice ) != MAL_SUCCESS )
    {
      mal_context_uninit( &context );
      printf( "[FFT] Failed to initialize capture device.\n" );
      return false;
    }

    if ( mal_device_start( &captureDevice ) != MAL_SUCCESS )
    {
      mal_device_uninit( &captureDevice );
      mal_context_uninit( &context );
      printf( "[FFT] Failed to start capture device.\n" );
      return false;
    }

    return true;
  }
  bool GetFFT( float * samples )
  {
    memcpy( samples, lastFFT, sizeof( float )*FFT_SIZE );
    return true;
  }
  void Close()
  {
    mal_device_stop( &captureDevice );

    mal_device_uninit( &captureDevice );
    mal_context_uninit( &context );

    kiss_fft_free( fftcfg );
  }
}
