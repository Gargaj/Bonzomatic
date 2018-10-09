//
//  TouchBar.mm
//  Bonzomatic
//
//  Created by Nightfox on 13/5/18.
//
#ifdef BONZOMATIC_ENABLE_TOUCHBAR

#include <Carbon/Carbon.h>
#import <Foundation/Foundation.h>
#include <AppKit/AppKit.h>
#import "../TouchBar.h"

static NSString *touchBarCustomizationId = @"com.something.customization_id";
static NSString *touchBarItemIdCompile = @"bonzomatic.compile";
static NSString *touchBarItemIdToggleGUI = @"bonzomatic.toggleGUI";
static NSString *touchBarItemIdTextures = @"bonzomatic.textures";
static NSString *touchBarItemIdSeparator = @"bonzomatic.separator";
static NSString *touchBarItemIdQuit = @"bonzomatic.quit";

@interface TouchBar : NSObject <NSTouchBarDelegate>
- (NSTouchBar *)makeTouchBar;
- (NSTouchBarItem *)touchBar:(NSTouchBar *)touchBar makeItemForIdentifier:(NSTouchBarItemIdentifier)identifier;

typedef enum {
    toggleTextures,
    toggleGUI,
    compile
} BonzomaticCommand;

@end

@implementation TouchBar
- (NSTouchBar *)makeTouchBar
{
  // Create TouchBar object
  NSTouchBar *touchBar = [[NSTouchBar alloc] init];
  touchBar.delegate = self;
  touchBar.customizationIdentifier = touchBarCustomizationId;
	
  // Set the default ordering of items.
  touchBar.defaultItemIdentifiers = @[touchBarItemIdCompile, touchBarItemIdToggleGUI, touchBarItemIdTextures, touchBarItemIdSeparator, touchBarItemIdQuit, NSTouchBarItemIdentifierOtherItemsProxy];

  return touchBar;
}

- (NSTouchBarItem *)touchBar:(NSTouchBar *)touchBar makeItemForIdentifier:(NSTouchBarItemIdentifier)identifier
{
  if ([identifier isEqualToString:touchBarItemIdCompile]) {
    NSButton *button = [NSButton buttonWithTitle:NSLocalizedString(@"Compile", @"") target:self action:@selector(compile)];
    button.bezelColor = [NSColor colorWithRed:0.0 green:0.5 blue:0.3 alpha:1.0];
		
    NSCustomTouchBarItem* g_TouchBarItem = [[NSCustomTouchBarItem alloc] initWithIdentifier:touchBarItemIdCompile];
    g_TouchBarItem.view = button;
    g_TouchBarItem.customizationLabel = NSLocalizedString(@"Truth Button", @"");
		
    return g_TouchBarItem;
  }
	
  if ([identifier isEqualToString:touchBarItemIdToggleGUI]) {
    NSButton *button = [NSButton buttonWithTitle:NSLocalizedString(@"Toggle GUI", @"") target:self action:@selector(toggleGui)];
		
    NSCustomTouchBarItem* g_TouchBarItem = [[NSCustomTouchBarItem alloc] initWithIdentifier:touchBarItemIdToggleGUI];
    g_TouchBarItem.view = button;
    g_TouchBarItem.customizationLabel = NSLocalizedString(@"Truth Button", @"");
		
    return g_TouchBarItem;
  }
	
  if ([identifier isEqualToString:touchBarItemIdTextures]) {
    NSButton *button = [NSButton buttonWithTitle:NSLocalizedString(@"Textures", @"") target:self action:@selector(toggleTextures)];
		
    NSCustomTouchBarItem* g_TouchBarItem = [[NSCustomTouchBarItem alloc] initWithIdentifier:touchBarItemIdTextures];
    g_TouchBarItem.view = button;
    g_TouchBarItem.customizationLabel = NSLocalizedString(@"Truth Button", @"");
		
    return g_TouchBarItem;
  }
	
  if ([identifier isEqualToString:touchBarItemIdSeparator]) {
    NSCustomTouchBarItem* g_TouchBarItem = [[NSCustomTouchBarItem alloc] initWithIdentifier:touchBarItemIdSeparator];
		g_TouchBarItem.view = [[NSView alloc] init];
		g_TouchBarItem.customizationLabel = NSLocalizedString(@"Truth Button", @"");
		
		return g_TouchBarItem;
	}
	
	
  if ([identifier isEqualToString:touchBarItemIdQuit]) {
    NSButton *button = [NSButton buttonWithTitle:NSLocalizedString(@"Quit", @"") target:self action:@selector(quit)];
    button.bezelColor = [NSColor redColor];
		
    NSCustomTouchBarItem* g_TouchBarItem = [[NSCustomTouchBarItem alloc] initWithIdentifier:touchBarItemIdQuit];
    g_TouchBarItem.view = button;
    g_TouchBarItem.customizationLabel = NSLocalizedString(@"Truth Button", @"");
		
    return g_TouchBarItem;
  }
	return nil;
}

- (void)quit {
    exit(0);
}

- (void)toggleGui {
    [self sendKeyboardCommandForCommand:toggleGUI];
}

- (void)toggleTextures {
    [self sendKeyboardCommandForCommand:toggleTextures];
}

- (void)compile {
    [self sendKeyboardCommandForCommand:compile];
}

- (void)sendKeyboardCommandForCommand:(BonzomaticCommand)command {
    pid_t pid = [NSRunningApplication currentApplication].processIdentifier;
    
    CGEventRef event1, event2;
    
    switch (command) {
        case toggleGUI:
            event1 = CGEventCreateKeyboardEvent (NULL, kVK_F11, true);
            event2 = CGEventCreateKeyboardEvent (NULL, kVK_F11, false);
            break;
        case toggleTextures:
            event1 = CGEventCreateKeyboardEvent (NULL, kVK_F2, true);
            event2 = CGEventCreateKeyboardEvent (NULL, kVK_F2, false);
            break;
        case compile:
            event1 = CGEventCreateKeyboardEvent (NULL, kVK_F5, true);
            event2 = CGEventCreateKeyboardEvent (NULL, kVK_F5, false);
            break;
    }
    
    CGEventPostToPid(pid, event1);
    CGEventPostToPid(pid, event2);
    
    CFRelease(event1);
    CFRelease(event2);
}

@end

void ShowTouchBar(GLFWwindow *window) {
    if (@available(macOS 10.12.2, *)) {
        TouchBar* g_TouchBarDelegate = [[TouchBar alloc] init];
        [NSApplication sharedApplication].automaticCustomizeTouchBarMenuItemEnabled = YES;
		
        NSTouchBar *touchBar = [g_TouchBarDelegate makeTouchBar];
        NSWindow *nswin = glfwGetCocoaWindow(window);
        nswin.touchBar = touchBar;
    }
}

#endif 
