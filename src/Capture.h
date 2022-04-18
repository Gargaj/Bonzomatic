namespace Capture
{
  void LoadSettings(jsonxx::Object & o);
  bool Open(RENDERER_SETTINGS & settings);
  void CaptureFrame();
  void Close();
}