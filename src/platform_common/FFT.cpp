#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>

#include <kiss_fft.h>
#include <kiss_fftr.h>
#include <stdio.h>
#include <memory.h>
#include "FFT.h"

namespace FFT
{
  kiss_fftr_cfg fftcfg;
  ma_context context;
  ma_device captureDevice;
  float sampleBuf[ FFT_SIZE * 2 ];
  float fAmplification = 1.0f;

  void OnLog( ma_context* pContext, ma_device* pDevice, ma_uint32 logLevel, const char* message )
  {
    printf( "[FFT] [mal:%p:%p]\n %s", pContext, pDevice, message );
  }

  void OnReceiveFrames( ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount )
  {
    frameCount = frameCount < FFT_SIZE * 2 ? frameCount : FFT_SIZE * 2;

    // Just rotate the buffer; copy existing, append new
    const float * samples = (const float *)pInput;
    float * p = sampleBuf;
    for ( int i = 0; i < FFT_SIZE * 2 - frameCount; i++ )
    {
      *( p++ ) = sampleBuf[ i + frameCount ];
    }
    for ( int i = 0; i < frameCount; i++ )
    {
      *( p++ ) = ( samples[ i * 2 ] + samples[ i * 2 + 1 ] ) / 2.0f * fAmplification;
    }
  }

  bool Open()
  {
    memset( sampleBuf, 0, sizeof( float ) * FFT_SIZE * 2 );

    fftcfg = kiss_fftr_alloc( FFT_SIZE * 2, false, NULL, NULL );

    ma_context_config context_config = ma_context_config_init();
    context_config.logCallback = OnLog;
    ma_result result = ma_context_init( NULL, 0, &context_config, &context );
    if ( result != MA_SUCCESS )
    {
      printf( "[FFT] Failed to initialize context: %d", result );
      return false;
    }

    printf( "[FFT] MAL context initialized, backend is '%s'\n", ma_get_backend_name( context.backend ) );

    ma_device_config config = ma_device_config_init( ma_device_type_capture );
    config.capture.pDeviceID = NULL;
    config.capture.format = ma_format_f32;
    config.capture.channels = 2;
    config.sampleRate = 44100;
    config.dataCallback = OnReceiveFrames;
    config.pUserData = NULL;

    result = ma_device_init( &context, &config, &captureDevice );
    if ( result != MA_SUCCESS )
    {
      ma_context_uninit( &context );
      printf( "[FFT] Failed to initialize capture device: %d\n", result );
      return false;
    }

    result = ma_device_start( &captureDevice );
    if ( result != MA_SUCCESS )
    {
      ma_device_uninit( &captureDevice );
      ma_context_uninit( &context );
      printf( "[FFT] Failed to start capture device: %d\n", result );
      return false;
    }

    return true;
  }
  bool GetFFT( float * _samples )
  {
    kiss_fft_cpx out[ FFT_SIZE + 1 ];
    kiss_fftr( fftcfg, sampleBuf, out );

    for ( int i = 0; i < FFT_SIZE; i++ )
    {
      static const float scaling = 1.0f / (float)FFT_SIZE;
      _samples[ i ] = 2.0 * sqrtf( out[ i ].r * out[ i ].r + out[ i ].i * out[ i ].i ) * scaling;
    }

    return true;
  }
  void Close()
  {
    ma_device_stop( &captureDevice );

    ma_device_uninit( &captureDevice );
    ma_context_uninit( &context );

    kiss_fft_free( fftcfg );
  }
}
