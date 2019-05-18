// Dummy Keymap functions that don't actually map anything,
// just to fix the SDL build.

#include <string.h>
#include "../Misc.h"

#include <unistd.h>

#include <sys/param.h> // For MAXPATHLEN
#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>

@interface MiscCocoa : NSObject

@end

@implementation MiscCocoa

+ (void)requestMicrophoneAccess {
  switch ([AVCaptureDevice authorizationStatusForMediaType:AVMediaTypeAudio])
  {
    case AVAuthorizationStatusAuthorized:
    {
      fprintf(stdout, "Microphone access already granted\n");
      break;
    }
    case AVAuthorizationStatusNotDetermined:
    {
      [AVCaptureDevice requestAccessForMediaType:AVMediaTypeAudio completionHandler:^(BOOL granted) {
        if (granted) {
            fprintf(stdout, "Microphone access granted\n");
        } else {
            fprintf(stderr, "Microphone access denied. No FFT support.\n");
        }
      }];
      return;
    }
    case AVAuthorizationStatusDenied:
    {
      fprintf(stderr, "Microphone access previously denied by user. Asking again.\n");
      [AVCaptureDevice requestAccessForMediaType:AVMediaTypeAudio completionHandler:^(BOOL granted) {
        if (granted) {
          fprintf(stdout, "Microphone access granted\n");
        } else {
          fprintf(stderr, "Microphone access denied. No FFT support.\n");
        }
      }];
      return;
    }
    case AVAuthorizationStatusRestricted:
    {
      fprintf(stderr, "The user can't grant microphone access due to restrictions.\n");
    }
  }
}

@end

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
  
  // Check if the user has granted microphone access to Bonzomatic. Else request it.
  if (@available(macOS 10.14, *)) {
    [MiscCocoa requestMicrophoneAccess];
  }
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
	if (cmd && *cmd) // param is always set
	{
		char command[512];
		sprintf(command, "%s %s", cmd, param);
		system(command);
		return true;
	}
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
