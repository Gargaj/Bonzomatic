// Dummy Keymap functions that don't actually map anything,
// just to fix the SDL build.

#include <string.h>
#include "../Misc.h"

void Misc::InitKeymaps() {
	return;
}

void Misc::GetKeymapName(char* sz) {
	strncpy(sz,"<native>",7);
}

bool Misc::ExecuteCommand( char * cmd, char * param )
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
    "/Library/Fonts/Courier New.ttf",
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