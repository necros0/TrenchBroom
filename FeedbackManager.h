//
//  FeedbackManager.h
//  TrenchBroom
//
//  Created by Kristian Duske on 17.03.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

extern NSString* const FeedbackAdded;
extern NSString* const FeedbackRemoved;

extern NSString* const FeedbackBrushKey;

@class MapWindowController;
@class Ray3D;

@interface FeedbackManager : NSObject {
    @private
    MapWindowController* windowController;
    NSMutableSet* brushes;
    Ray3D* currentRay;
}

- (id)initWithWindowController:(MapWindowController *)theWindowController;

- (void)updateWithRay:(Ray3D *)theRay;

@end
