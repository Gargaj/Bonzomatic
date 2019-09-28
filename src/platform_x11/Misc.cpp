// Dummy Keymap functions that don't actually map anything,
// just to fix the SDL build.

#include <string.h>
#include "../Misc.h"

#include <stdlib.h>
#include <unistd.h>
#include <limits.h>

#include <fontconfig/fontconfig.h>

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
  static char ret[PATH_MAX] = { 0 };
  FcPattern *pat;
  FcFontSet *fs;
  FcObjectSet *os;
  FcChar8 *file = NULL;
  FcConfig *config;
  FcBool result;
  int i;

  const char *familyNames[] =
  {
    "DejaVu Sans Mono",
    "FreeMono",
    "Liberation Mono",
    "Bitstream Vera Sans Mono",
    "Ubuntu Mono",
    NULL
  };

  result = FcInit();
  config = FcConfigGetCurrent();
  FcConfigSetRescanInterval(config, 0);

  pat = FcPatternCreate();
  os = FcObjectSetBuild (FC_FAMILY, FC_FILE, (char *) 0);
  FcPatternAddInteger(pat, FC_SPACING, FC_MONO);
  FcPatternAddInteger(pat, FC_WEIGHT, FC_WEIGHT_REGULAR);
  FcPatternAddInteger(pat, FC_SLANT, FC_SLANT_ROMAN);
  fs = FcFontList(config, pat, os);

  for (i=0; fs && i < fs->nfont; i++) {
    FcChar8 *family;
    FcPattern *font = fs->fonts[i];
    if (FcPatternGetString(font, FC_FAMILY, 0, &family) == FcResultMatch) {
      for (int i = 0; familyNames[i] != NULL; i++) {
        if (strcmp((const char *)family, familyNames[i]) == 0) {
          if (FcPatternGetString(font, FC_FILE, 0, &file) == FcResultMatch) {
            strcpy(ret, (const char *)file);
            goto out;
          }
        }
      }
    }
  }

out:
  FcFontSetDestroy(fs);
  FcObjectSetDestroy(os);
  FcPatternDestroy(pat);
  return ret;
}
