#include <iostream>
#include "../Renderer.h"
#include "../FFT.h"
#include "../SetupDialog.h"
#include <CoreFoundation/CoreFoundation.h>
#include <CoreGraphics/CoreGraphics.h>
#include <set>

namespace SetupDialog
{

struct Resolution
{
  unsigned int w, h;
  Resolution( int _w, int _h) : w(_w), h(_h) {}
};
inline bool operator==(const Resolution& l, const Resolution& r) { return (l.w==r.w)&&(l.h==r.h); }
inline bool operator<(const Resolution& l, const Resolution& r) { return (l.w<r.w)?true:((l.w==r.w)?((l.h<r.h)?true:false):false); }

void BuildListOfMainDisplayResolutions(std::set<Resolution> &resolutions)
{
  CGDirectDisplayID monitorID = CGMainDisplayID();
  CFArrayRef modes = CGDisplayCopyAllDisplayModes(monitorID, NULL);
  CFIndex found = CFArrayGetCount(modes);
  
  for (CFIndex i = 0;  i < found;  i++)
  {
    CGDisplayModeRef dm = (CGDisplayModeRef) CFArrayGetValueAtIndex(modes, i);
    resolutions.insert(Resolution((int)CGDisplayModeGetWidth(dm), (int)CGDisplayModeGetHeight(dm)));
  }
  CFRelease(modes);
}

bool Open( SetupDialog::SETTINGS * settings )
{
  std::set<Resolution> resolutions;
  
  // Force some default resolutions
  resolutions.insert(Resolution(1280, 720));
  resolutions.insert(Resolution(1920, 1080));
  // Also add the resolution found in the settings
  resolutions.insert(Resolution(settings->sRenderer.nWidth, settings->sRenderer.nHeight));
  
  BuildListOfMainDisplayResolutions(resolutions);
  
  CFURLRef urlRef = CFBundleCopyResourceURL(CFBundleGetMainBundle(), CFSTR("icon"), CFSTR("icns"), NULL);
  
  // The list of resolutions for the dialog
  CFMutableArrayRef popupVals = CFArrayCreateMutable(NULL, resolutions.size(), &kCFTypeArrayCallBacks);
  for (const auto &r: resolutions) {
    CFArrayAppendValue(popupVals, CFStringCreateWithFormat(NULL, NULL, CFSTR("%d x %d"), r.w, r.h));
  }
  
  // The fullscreen checkbox for the dialog
  CFTypeRef checkboxKeys[1] = { CFSTR("Fullscreen?") };
  CFArrayRef checkboxVals = CFArrayCreate( NULL, checkboxKeys, 1, &kCFTypeArrayCallBacks);
  
  const void* keys[] = {
    kCFUserNotificationIconURLKey,
    kCFUserNotificationAlertHeaderKey,
    kCFUserNotificationAlertMessageKey,
    kCFUserNotificationDefaultButtonTitleKey,
    kCFUserNotificationPopUpTitlesKey,
    kCFUserNotificationCheckBoxTitlesKey
  };
  const void* values[] = {
    urlRef,
    CFSTR("Bonzomatic"),
    CFSTR("Choose your resolution, it's a REVOLUTION!\n\nNote: The list is polled from your main display, but 1920x1080 and 1280x720 are force-inserted and NOT guaranteed to work on your device.\nWe also add whatever was found in the \"config.json\", regardless of whether it will actually work.\n"),
    CFSTR("Ignition"),
    popupVals,
    checkboxVals
  };
  CFDictionaryRef parameters = CFDictionaryCreate(NULL, keys, values, sizeof(keys)/sizeof(*keys), &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
  
  // Setup defaults according to the settings
  CFOptionFlags settingsFlags = 0;
  if (settings->sRenderer.windowMode == RENDERER_WINDOWMODE_FULLSCREEN) {
    settingsFlags |= CFUserNotificationCheckBoxChecked(0);
  }
  int idx=0;
  for (const auto &r: resolutions) {
    if ((settings->sRenderer.nWidth == r.w) && (settings->sRenderer.nHeight == r.h)) {
      settingsFlags |= CFUserNotificationPopUpSelection(idx);
      break;
    }
    idx++;
  }
  
  SInt32 err = 0;
  CFUserNotificationRef dialog = CFUserNotificationCreate(NULL, 0, kCFUserNotificationPlainAlertLevel | settingsFlags, &err, parameters);
  
  CFOptionFlags responseFlags = 0;
  CFUserNotificationReceiveResponse(dialog, 0, &responseFlags);
  
  // Collect the user selection and feed the settings
  if (responseFlags & CFUserNotificationCheckBoxChecked(0)) {
    settings->sRenderer.windowMode = RENDERER_WINDOWMODE_FULLSCREEN;
  } else {
    settings->sRenderer.windowMode = RENDERER_WINDOWMODE_WINDOWED;
  }
  idx = (int)responseFlags >> 24;
  if ((idx >= 0) && (idx < resolutions.size())) {
    const auto &r = *std::next(resolutions.cbegin(), idx);
    settings->sRenderer.nWidth = r.w;
    settings->sRenderer.nHeight = r.h;
  }
  
  CFRelease(dialog);
  CFRelease(parameters);
  CFRelease(checkboxVals);
  CFRelease(popupVals);

  return true;
}

}
