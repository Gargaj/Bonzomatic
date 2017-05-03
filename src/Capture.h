#ifndef CAPTURE_H
#define CAPTURE_H

#include <string>

struct CAPTURE_SETTINGS {
  bool bNDIEnabled;
  std::string sNDIConnectionString;
  std::string sNDIIdentifier;
  float fNDIFrameRate;
  bool bNDIProgressive;
};

struct RENDERER_SETTINGS;

namespace Capture
{
  void ApplySettings(const CAPTURE_SETTINGS & settings);
  bool Open(const RENDERER_SETTINGS & rendererSettings);
  void CaptureFrame();
  void Close();
}

#endif // CAPTURE_H
