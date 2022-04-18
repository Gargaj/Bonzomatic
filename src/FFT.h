#define FFT_SIZE 1024

typedef struct 
{
  bool bUseRecordingDevice;
  void * pDeviceID;
} FFT_SETTINGS;

namespace FFT
{
  typedef void (*FFT_ENUMERATE_FUNC)( const bool bIsCaptureDevice, const char * szDeviceName, void * pDeviceID, void * pUserContext );

  extern float fAmplification;

  void EnumerateDevices( FFT_ENUMERATE_FUNC pEnumerationFunction, void * pUserContext );

  bool Create();
  bool Destroy();
  bool Open( FFT_SETTINGS * pSettings );
  bool GetFFT( float * _samples );
  void Close();
}