#include <string>

namespace Misc
{
  extern const char * defaultPostExitCmd;

  void PlatformStartup();
  void PlatformShutdown();

  void InitKeymaps();
  void GetKeymapName( char * sz );
  bool ExecuteCommand( const char * cmd, const char * param );

  bool FileExists(const char * path);
  const char * GetDefaultFontPath();
}