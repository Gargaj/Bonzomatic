#include <Processing.NDI.Lib.h>
#include <string>
#include "Capture.h"
#include "Renderer.h"

namespace Capture
{
  CAPTURE_SETTINGS settings;

  unsigned int * pBuffer[2] = { NULL, NULL };
  unsigned int nBufferIndex = 0;
  NDIlib_video_frame_t pNDIFrame;
  NDIlib_send_instance_t pNDI_send;

  void ApplySettings(const CAPTURE_SETTINGS & settings)
  {
    Capture::settings = settings;
  }
  bool Open(const RENDERER_SETTINGS & rendererSettings)
  {
    if (settings.bNDIEnabled)
    {
      if (!NDIlib_initialize())
      {
        printf("[Capture] Cannot initialize NDI");
        return false;
      }

      NDIlib_send_create_t pNDICreateDesc;
      const std::string sNDIIdentifier = "BONZOMATIC" + (settings.sNDIIdentifier.length() ? (" - " + settings.sNDIIdentifier) : "");
      pNDICreateDesc.p_ndi_name = sNDIIdentifier.c_str();
      pNDICreateDesc.p_groups = NULL;
      pNDICreateDesc.clock_video = true;
      pNDICreateDesc.clock_audio = false;

      pNDI_send = NDIlib_send_create(&pNDICreateDesc);
      if (!pNDI_send)
      {
        printf("[Capture] Cannot create NDI source");
        return false;
      }

      NDIlib_metadata_frame_t pNDIConnType;
      pNDIConnType.length = settings.sNDIConnectionString.length();
      pNDIConnType.timecode = NDIlib_send_timecode_synthesize;
      pNDIConnType.p_data = (char*)settings.sNDIConnectionString.c_str();

      NDIlib_send_add_connection_metadata(pNDI_send, &pNDIConnType);

      pNDIFrame.xres = rendererSettings.nWidth;
      pNDIFrame.yres = rendererSettings.nHeight;
      pNDIFrame.FourCC = NDIlib_FourCC_type_BGRA;
      pNDIFrame.frame_rate_N = settings.fNDIFrameRate * 100;
      pNDIFrame.frame_rate_D = 100;
      pNDIFrame.picture_aspect_ratio = rendererSettings.nWidth / (float)rendererSettings.nHeight;
      pNDIFrame.frame_format_type = settings.bNDIProgressive ? NDIlib_frame_format_type_progressive : NDIlib_frame_format_type_interleaved;
      pNDIFrame.timecode = NDIlib_send_timecode_synthesize;
      pBuffer[0] = new unsigned int[rendererSettings.nWidth * rendererSettings.nHeight * 4];
      pBuffer[1] = new unsigned int[rendererSettings.nWidth * rendererSettings.nHeight * 4];
      pNDIFrame.p_data = NULL;
      pNDIFrame.line_stride_in_bytes = rendererSettings.nWidth * 4;
    }
    return true;
  }
  void CaptureFrame()
  {
    if (pBuffer[0] && pBuffer[1])
    {
      pNDIFrame.p_data = (unsigned char*)pBuffer[ nBufferIndex ];
      nBufferIndex = (nBufferIndex + 1) & 1;
      if (Renderer::GrabFrame( pNDIFrame.p_data ))
      {
        unsigned int * p = (unsigned int *)pNDIFrame.p_data;
        for(int i=0; i < pNDIFrame.xres * pNDIFrame.yres; i++)
          p[i] = (p[i] & 0x00FF00) | ((p[i] >> 16) & 0xFF) | ((p[i] & 0xFF) << 16) | 0xFF000000;
        NDIlib_send_send_video_async(pNDI_send, &pNDIFrame);
      }
    }
  }
  void Close()
  {
    if (pBuffer[0] && pBuffer[1])
    {
      NDIlib_send_send_video_async(pNDI_send, NULL); // stop async thread

      delete[] pBuffer[0];
      delete[] pBuffer[1];
      NDIlib_send_destroy(pNDI_send);
      NDIlib_destroy();
    }
  }
}
