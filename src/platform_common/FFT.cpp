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
  float sampleBuf[ FFT_SIZE * 2 ];

  void OnLog( mal_context* pContext, mal_device* pDevice, const char* message )
  {
    printf( "[FFT] [mal:%p:%p]\n %s", pContext, pDevice, message );
  }

  void OnReceiveFrames( mal_device* pDevice, mal_uint32 frameCount, const void* pSamples )
  {
    frameCount = frameCount < FFT_SIZE * 2 ? frameCount : FFT_SIZE * 2;

    // Just rotate the buffer; copy existing, append new
    const mal_int16 * samples = (const mal_int16 *)pSamples;
    float * p = sampleBuf;
    for ( int i = 0; i < FFT_SIZE * 2 - frameCount; i++ )
    {
      *( p++ ) = sampleBuf[ i + frameCount ];
    }
    for ( int i = 0; i < frameCount; i++ )
    {
      *( p++ ) = ( samples[ i * 2 ] + samples[ i * 2 + 1 ] ) / 65535.0f; // int16 max * 2 -> -1..1
    }
  }

  bool Open()
  {
    memset( sampleBuf, 0, sizeof( float ) * FFT_SIZE * 2 );

    fftcfg = kiss_fftr_alloc( FFT_SIZE * 2, false, NULL, NULL );

    const mal_context_config context_config = mal_context_config_init( OnLog );
    mal_result result = mal_context_init( NULL, 0, &context_config, &context );
    if ( result != MAL_SUCCESS )
    {
      printf( "[FFT] Failed to initialize context: %d", result );
      return false;
    }

    printf( "[FFT] MAL context initialized, backend is '%s'\n", mal_get_backend_name( context.backend ) );

    const mal_device_config config = mal_device_config_init( mal_format_s16, 2, 44100, OnReceiveFrames, NULL );
    
    result = mal_device_init( &context, mal_device_type_capture, NULL, &config, NULL, &captureDevice );
    if ( result != MAL_SUCCESS )
    {
      mal_context_uninit( &context );
      printf( "[FFT] Failed to initialize capture device: %d\n", result );
      return false;
    }

    result = mal_device_start( &captureDevice );
    if ( result != MAL_SUCCESS )
    {
      mal_device_uninit( &captureDevice );
      mal_context_uninit( &context );
      printf( "[FFT] Failed to start capture device: %d\n", result );
      return false;
    }

    return true;
  }
  bool GetFFT( float * samples )
  {
    kiss_fft_cpx out[ FFT_SIZE + 1 ];
    kiss_fftr( fftcfg, sampleBuf, out );

    for ( int i = 0; i < FFT_SIZE; i++ )
    {
      static const float scaling = 1.0f / (float)FFT_SIZE;
      samples[ i ] = 2.0 * sqrtf( out[ i ].r * out[ i ].r + out[ i ].i * out[ i ].i ) * scaling;
    }

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
