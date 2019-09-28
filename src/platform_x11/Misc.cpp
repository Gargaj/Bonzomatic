// Dummy Keymap functions that don't actually map anything,
// just to fix the SDL build.

#include <string.h>
#include "../Misc.h"

#include <unistd.h>
#include <dirent.h>

#include <tinyxml2.h>

using namespace tinyxml2;

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

static std::string ScanFontDir(const std::string path, const char **fontNames)
{
  if (!Misc::FileExists(path.c_str())) {
    return "";
  }

  DIR *d = opendir(path.c_str());
  if (d == NULL) {
    return "";
  }
  struct dirent *de;

  while ((de = readdir(d)) != NULL) {
    if (strcmp(de->d_name, ".") == 0 ||
        strcmp(de->d_name, "..") == 0) {
      continue;
    }
    if (de->d_type == DT_REG) {
      for (int i = 0; fontNames[i] != NULL; i++) {
        if (strcmp(de->d_name, fontNames[i]) == 0) {
          std::string ret = path;
          ret += "/";
          ret += fontNames[i];
          return ret;
        }
      }
    } else if (de->d_type == DT_DIR) {
      std::string subdir = path;
      subdir += "/";
      subdir += de->d_name;
      std::string subdir_match = ScanFontDir(subdir, fontNames);
      if (!subdir_match.empty()) {
        closedir(d);
        return subdir_match;
      }
    }
  }

  closedir(d);
  return "";
}

const char * Misc::GetDefaultFontPath()
{
  // Linux case
  // font names we're looking for
  const char *fontNames[] =
  {
    "DejaVuSansMono.ttf",
    "FreeMono.ttf",
    "LiberationMono-Regular.ttf",
    "UbuntuMono-R.ttf",
    "VeraMono.ttf",
    "cour.ttf",
    NULL
  };
  // full paths we're looking for first
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
    "/usr/share/fonts/truetype/ubuntu-font-family/UbuntuMono-R.ttf",
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

  /*
   * Fast lookup didn't work, let's read fonts.conf and
   * let's start scanning directories
   */

  XMLDocument fontsconf;
  if (fontsconf.LoadFile("/etc/fonts/fonts.conf") != XML_SUCCESS) {
    return NULL;
  }

  XMLNode *d = fontsconf.RootElement()->FirstChildElement("dir");
  if (d == NULL) {
    return NULL;
  }

  while (d != NULL) {
    std::string tmp = ScanFontDir((const char *)d->FirstChild()->Value(),
        fontNames);
    if (!tmp.empty()) {
      static char fontPath[PATH_MAX] = { 0 };
      strcpy(fontPath, tmp.c_str());
      return fontPath;
    }
    d = d->NextSiblingElement("dir");
  }

  return NULL;
}
