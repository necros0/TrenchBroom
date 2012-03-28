/*
 Copyright (C) 2010-2012 Kristian Duske
 
 This file is part of TrenchBroom.
 
 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 TrenchBroom is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#import "PreferencesController.h"
#import "QuakePathFormatter.h"
#import "MapWindowController.h"
#import "PreferencesManager.h"
#import "CompilerProfileManager.h"
#import "ControllerUtils.h"
#import "PreferencesViewAnimation.h"

static NSString* const GeneralToolbarItemIdentifier    = @"GeneralToolbarItem";
static NSString* const CompilerToolbarItemIdentifier   = @"CompilerToolbarItem";

static PreferencesController* sharedInstance = nil;

@interface PreferencesController (private)

- (void)updateExecutableList;
- (void)preferencesDidChange:(NSNotification *)notification;
- (void)activateToolbarItemWithId:(NSString *)toolbarItemIdentifier view:(NSView *)view animate:(BOOL)animate;

@end

@implementation PreferencesController (private)

- (void)updateExecutableList {
    [quakeExecutablePopUp removeAllItems];
    updateMenuWithExecutables([quakeExecutablePopUp menu], YES, NULL);
    
    PreferencesManager* preferences = [PreferencesManager sharedManager];
    NSString* executable = [preferences quakeExecutable];
    if (executable != nil)
        [quakeExecutablePopUp selectItemWithTitle:[executable stringByDeletingPathExtension]];
    [quakeExecutablePopUp synchronizeTitleAndSelectedItem];
}

- (void)preferencesDidChange:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    if (DefaultsQuakePath != [userInfo objectForKey:DefaultsKey])
        return;

    [self updateExecutableList];
}

- (void)activateToolbarItemWithId:(NSString *)toolbarItemIdentifier view:(NSView *)newView animate:(BOOL)animate {
    if (toolbarItemIdentifier == lastSelectedIdentifier)
        return;
    
    if (currentAnimation != nil)
        [currentAnimation stopAnimation];

    NSView* contentView = [[self window] contentView];
    NSView* oldView = [[contentView subviews] count] > 0 ? [[contentView subviews] objectAtIndex:0] : nil;
    
    NSRect oldWindowFrame = [[self window] frame];
    NSRect newViewFrame = [newView frame];

    float rest = NSHeight(oldWindowFrame) - NSHeight([contentView frame]);
    float heightDiff = NSHeight(newViewFrame) - NSHeight([contentView frame]);
    
    newViewFrame.origin.y = 0;
    [newView setFrame:newViewFrame];
    
    NSRect newFrame = NSMakeRect(NSMinX(oldWindowFrame), NSMinY(oldWindowFrame) - heightDiff, NSWidth(newViewFrame), NSHeight(newViewFrame) + rest);
    if (animate) {
        [currentAnimation release];
        currentAnimation = [[PreferencesViewAnimation alloc] initWithDuration:0.25 animationCurve:NSAnimationEaseInOut];
        [currentAnimation setWindow:[self window]];
        [currentAnimation setOutView:oldView];
        [currentAnimation setInView:newView];
        [currentAnimation setTargetFrame:newFrame];
        [currentAnimation startAnimation];
    } else {
        [[self window] setFrame:newFrame display:YES];

        NSView* contentView = [[self window] contentView];
        [contentView addSubview:newView];
        [oldView removeFromSuperview];
        
        [oldView setAlphaValue:0];
        [newView setAlphaValue:1];
    }
    
    [toolbar setSelectedItemIdentifier:toolbarItemIdentifier];
    lastSelectedIdentifier = toolbarItemIdentifier;
}

@end

@implementation PreferencesController

+ (PreferencesController *)sharedPreferences {
    @synchronized(self) {
        if (sharedInstance == nil)
            sharedInstance = [[self alloc] init];
    }
    return sharedInstance;
}

+ (id)allocWithZone:(NSZone *)zone {
    @synchronized(self) {
        if (sharedInstance == nil) {
            sharedInstance = [super allocWithZone:zone];
            return sharedInstance;  // assignment and return on first allocation
        }
    }
    return nil; // on subsequent allocation attempts return nil
}

- (id)copyWithZone:(NSZone *)zone {
    return self;
}

- (id)retain {
    return self;
}

- (NSUInteger)retainCount {
    return UINT_MAX;  // denotes an object that cannot be released
}

- (oneway void)release {
    //do nothing
}

- (id)autorelease {
    return self;
}

- (id)init {
    if ((self = [super init])) {
        toolbarItemToViewMap = [[NSMutableDictionary alloc] init];
    }
    
    
    return self;
}

- (void)dealloc {
    [toolbarItemToViewMap release];
    [super dealloc];
}

- (NSString *)windowNibName {
    return @"PreferencesWindow";
}

- (void)windowDidLoad {
    [toolbarItemToViewMap setObject:generalView forKey:GeneralToolbarItemIdentifier];
    [toolbarItemToViewMap setObject:compilerView forKey:CompilerToolbarItemIdentifier];
    
    [generalView setAlphaValue:0];
    [compilerView setAlphaValue:0];
    
    PreferencesManager* preferences = [PreferencesManager sharedManager];
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center addObserver:self selector:@selector(preferencesDidChange:) name:DefaultsDidChange object:preferences];
    [self updateExecutableList];
    [self activateToolbarItemWithId:GeneralToolbarItemIdentifier view:generalView animate:NO];
}

- (IBAction)chooseQuakePath:(id)sender {
    NSOpenPanel* openPanel = [NSOpenPanel openPanel];
    [openPanel setCanChooseFiles:NO];
    [openPanel setCanChooseDirectories:YES];
    [openPanel setAllowsMultipleSelection:NO];
    [openPanel setAllowsOtherFileTypes:NO];
    [openPanel setTitle:@"Choose Quake Path"];
    [openPanel setNameFieldLabel:@"Quake Path"];
    [openPanel setCanCreateDirectories:NO];
    
    if ([openPanel runModal] == NSFileHandlingPanelOKButton) {
        for (NSURL* url in [openPanel URLs]) {
            NSString* quakePath = [url path];
            if (quakePath != nil) {
                PreferencesManager* preferences = [PreferencesManager sharedManager];
                [preferences setQuakePath:quakePath];
            }
        }
    }
}

- (PreferencesManager *)preferences {
    return [PreferencesManager sharedManager];
}

- (CompilerProfileManager *)compilerProfileManager {
    return [CompilerProfileManager sharedManager];
}

- (IBAction)generalToolbarItemSelected:(id)sender {
    [self activateToolbarItemWithId:GeneralToolbarItemIdentifier view:generalView animate:YES];
}

- (IBAction)compilerToolbarItemSelected:(id)sender {
    [self activateToolbarItemWithId:CompilerToolbarItemIdentifier view:compilerView animate:YES];
}

@end
