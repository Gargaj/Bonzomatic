// Dummy Keymap functions that don't actually map anything,
// just to fix the SDL build.

#include <string.h>
#include "../Misc.h"

#include <unistd.h>

#include <sys/param.h> // For MAXPATHLEN
#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>
#import <AppKit/AppKit.h>

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
  NSString *fontPath = [NSString stringWithUTF8String:path];
  return [[NSFileManager defaultManager] isReadableFileAtPath:fontPath];
}

const char * Misc::GetDefaultFontPath()
{
  const char *fontNames[] =
  {
    "CourierNewPSMT",
    NULL
  };
  for (int i = 0; fontNames[i]; ++i)
  {
    NSString *fontName = [NSString stringWithUTF8String:fontNames[i]];
    NSFont *font = [NSFont fontWithName:fontName size:12];
    if (font) {
      CTFontDescriptorRef fontRef = CTFontDescriptorCreateWithNameAndSize ((CFStringRef)[font fontName], [font pointSize]);
      CFURLRef url = (CFURLRef)CTFontDescriptorCopyAttribute(fontRef, kCTFontURLAttribute);
      NSString *fontPath = [NSString stringWithString:[(NSURL *)CFBridgingRelease(url) path]];
      const char *path = (char *)[fontPath UTF8String];

      if (FileExists(path))
      {
        return path;
      }
    }
  }
  return NULL;
}
