#define FFT_SIZE 1024

namespace FFT
{
//////////////////////////////////////////////////////////////////////////

struct Settings
{
  bool bUseRecordingDevice;
  void * pDeviceID;
};

typedef void ( *FFT_ENUMERATE_FUNC )( const bool bIsCaptureDevice, const char * szDeviceName, void * pDeviceID, void * pUserContext );

extern float fAmplification;

void EnumerateDevices( FFT_ENUMERATE_FUNC pEnumerationFunction, void * pUserContext );

bool Create();
bool Destroy();
bool Open( Settings * pSettings );
bool GetFFT( float * _samples );
void Close();

//////////////////////////////////////////////////////////////////////////
}