// Dummy Keymap functions that don't actually map anything,
// just to fix the SDL build.

#include <string.h>
#include "../Misc.h"

#include <unistd.h>

void Misc::PlatformStartup()
{
}

void Misc::PlatformShutdown()
{
}

void Misc::InitKeymaps()
{
	return;
}

void Misc::GetKeymapName(char* sz)
{
	strncpy(sz,"<native>",7);
}

bool Misc::ExecuteCommand( const char * cmd, const char * param )
{
	return false;
}

bool Misc::FileExists(const char * path)
{
  return access(path, R_OK) != -1;
}

const char * Misc::GetDefaultFontPath()
{
  // Linux case
  // TODO: use fonts.conf(5) or X resources or something like that
  const char* fontPaths[] =
  {
    "/usr/share/fonts/TTF/DejaVuSansMono.ttf",
    "/usr/share/fonts/TTF/FreeMono.ttf",
    "/usr/share/fonts/TTF/LiberationMono-Regular.ttf",
    "/usr/share/fonts/TTF/VeraMono.ttf",
    "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf",
    "/usr/share/fonts/truetype/freefont/FreeMono.ttf",
    "/usr/share/fonts/truetype/liberation/LiberationMono-Regular.ttf",
    "/usr/share/fonts/truetype/msttcorefonts/cour.ttf",
    "/usr/share/fonts/corefonts/cour.ttf",
    NULL
  };
  for (int i = 0; fontPaths[i]; ++i)
  {
    if (FileExists(fontPaths[i]))
    {
      return fontPaths[i];
    }
  }
  return NULL;
}
