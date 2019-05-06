// Dummy Keymap functions that don't actually map anything,
// just to fix the SDL build.

#include <stdlib.h>
#include <string.h>
#include "../Misc.h"

#include <unistd.h>

#include <sys/param.h> // For MAXPATHLEN
#include "CoreFoundation/CoreFoundation.h"

const char * Misc::postExitCmd = "";

void Misc::PlatformStartup()
{
  char appPath[MAXPATHLEN];
  CFBundleRef bundle = CFBundleGetMainBundle();
  if (!bundle) return;

  CFURLRef bundleURL = CFBundleCopyBundleURL(bundle);
  CFURLRef pathURL = CFURLCreateCopyDeletingLastPathComponent(NULL, bundleURL);
  if (!CFURLGetFileSystemRepresentation(pathURL, true, (UInt8*)appPath, MAXPATHLEN))
  {
    CFRelease(bundleURL);
    CFRelease(pathURL);
    return;
  }
  CFRelease(bundleURL);
  CFRelease(pathURL);

  chdir(appPath);
}

void Misc::PlatformShutdown()
{
}

void Misc::InitKeymaps()
{
}

void Misc::GetKeymapName(char* sz)
{
	strncpy(sz,"<native>",7);
}

bool Misc::ExecuteCommand( const char * cmd, const char * param )
{
  char command[512];
  sprintf(command, "%s %s", cmd, param);
  system(command);

  return true;
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