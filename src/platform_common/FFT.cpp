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
  bool bCreated = false;

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

  void EnumerateDevices( FFT_ENUMERATE_FUNC pEnumerationFunction, void * pUserContext )
  {
    if (!bCreated)
    {
      return;
    }

    ma_device_info * pPlaybackDevices = NULL;
    ma_device_info * pCaptureDevices = NULL;
    ma_uint32 nPlaybackDeviceCount = 0;
    ma_uint32 nCaptureDeviceCount = 0;
    ma_result result = ma_context_get_devices(&context, &pPlaybackDevices, &nPlaybackDeviceCount, &pCaptureDevices, &nCaptureDeviceCount);
    if (result != MA_SUCCESS) {
      printf("[FFT] Failed to enumerate audio devices: %d\n", result);
      return;
    }

    pEnumerationFunction( true, "<default device>", NULL, pUserContext );
    for (ma_uint32 i = 0; i < nCaptureDeviceCount; i++) 
    {
      pEnumerationFunction( true, pCaptureDevices[i].name, &pCaptureDevices[i].id, pUserContext );
    }
    if (ma_is_loopback_supported(context.backend))
    {
      pEnumerationFunction( false, "<default device>", NULL, pUserContext );
      for (ma_uint32 i = 0; i < nPlaybackDeviceCount; i++) 
      {
        pEnumerationFunction( false, pPlaybackDevices[i].name, &pPlaybackDevices[i].id, pUserContext );
      }
    }
  }

  bool Create()
  {
    bCreated = false;
    ma_context_config context_config = ma_context_config_init();
    context_config.logCallback = OnLog;
    ma_result result = ma_context_init( NULL, 0, &context_config, &context );
    if ( result != MA_SUCCESS )
    {
      printf( "[FFT] Failed to initialize context: %d", result );
      return false;
    }

    printf( "[FFT] MAL context initialized, backend is '%s'\n", ma_get_backend_name( context.backend ) );
    bCreated = true;
    return true;
  }

  bool Destroy()
  {
    if (!bCreated)
    {
      return false;
    }

    ma_context_uninit( &context );

    bCreated = false;

    return true;
  }

  bool Open( FFT_SETTINGS * pSettings )
  {
    if (!bCreated)
    {
      return false;
    }

    memset( sampleBuf, 0, sizeof( float ) * FFT_SIZE * 2 );

    fftcfg = kiss_fftr_alloc( FFT_SIZE * 2, false, NULL, NULL );

    bool useLoopback = ma_is_loopback_supported( context.backend ) && !pSettings->bUseRecordingDevice;
    ma_device_config config = ma_device_config_init( useLoopback ? ma_device_type_loopback : ma_device_type_capture );
    config.capture.pDeviceID = (ma_device_id*)pSettings->pDeviceID;
    config.capture.format = ma_format_f32;
    config.capture.channels = 2;
    config.sampleRate = 44100;
    config.dataCallback = OnReceiveFrames;
    config.pUserData = NULL;

    ma_result result = ma_device_init( &context, &config, &captureDevice );
    if ( result != MA_SUCCESS )
    {
      printf( "[FFT] Failed to initialize capture device: %d\n", result );
      return false;
    }

    printf( "[FFT] Selected capture device: %s\n", captureDevice.capture.name );

    result = ma_device_start( &captureDevice );
    if ( result != MA_SUCCESS )
    {
      ma_device_uninit( &captureDevice );
      printf( "[FFT] Failed to start capture device: %d\n", result );
      return false;
    }

    return true;
  }
  bool GetFFT( float * _samples )
  {
    if (!bCreated)
    {
      return false;
    }

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
    if (!bCreated)
    {
      return;
    }

    ma_device_stop( &captureDevice );

    ma_device_uninit( &captureDevice );

    kiss_fft_free( fftcfg );
  }
}
