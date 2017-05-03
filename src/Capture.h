#include <string>

typedef struct
{
  bool bNDIEnabled;
  std::string sNDIConnectionString;
  std::string sNDIIdentifier;
  float fNDIFrameRate;
  bool bNDIProgressive;
} CAPTURE_SETTINGS;

namespace Capture
{
  void LoadSettings(jsonxx::Object & o);
  bool Open(RENDERER_SETTINGS & settings);
  void CaptureFrame();
  void Close();
}
