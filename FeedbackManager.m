//
//  FeedbackManager.m
//  TrenchBroom
//
//  Created by Kristian Duske on 17.03.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "FeedbackManager.h"
#import "MapWindowController.h"
#import "SelectionManager.h"
#import "Ray3D.h"
#import "Brush.h"

NSString* const FeedbackAdded = @"FeedbackAdded";
NSString* const FeedbackRemoved = @"FeedbackRemoved";

NSString* const FeedbackBrushKey = @"FeedbackBrush";

@implementation FeedbackManager

- (void)addBrush:(id <Brush>)brush {
    if (![brushes containsObject:brush] && [brush pickFace:currentRay] != nil) {
        [brushes addObject:brush];
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:FeedbackAdded object:self userInfo:[NSDictionary dictionaryWithObject:brush forKey:FeedbackBrushKey]];
    }
}

- (void)removeBrush:(id <Brush>)brush {
    if ([brushes containsObject:brush]) {
        [brushes removeObject:brush];
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center postNotificationName:FeedbackRemoved object:self userInfo:[NSDictionary dictionaryWithObject:brush forKey:FeedbackBrushKey]];
    }
    
}

- (void)selectionAdded:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    NSSet* addedBrushes = [userInfo objectForKey:SelectionBrushes];
    
    NSEnumerator* brushEn = [addedBrushes objectEnumerator];
    id <Brush> brush;
    while ((brush = [brushEn nextObject]))
        if ([brush pickFace:currentRay] != nil)
            [self addBrush:brush];
}

- (void)selectionRemoved:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    NSSet* removedBrushes = [userInfo objectForKey:SelectionBrushes];
    
    NSEnumerator* brushEn = [removedBrushes objectEnumerator];
    id <Brush> brush;
    while ((brush = [brushEn nextObject]))
        [self removeBrush:brush];
}

- (id)init {
    if (self = [super init]) {
        brushes = [[NSMutableSet alloc] init];
    }
    
    return self;
}

- (id)initWithWindowController:(MapWindowController *)theWindowController {
    if (theWindowController == nil)
        [NSException raise:NSInvalidArgumentException format:@"window controller must not be nil"];
    
    if (self = [self init]) {
        windowController = [theWindowController retain];
        
        SelectionManager* selectionManager = [windowController selectionManager];
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center addObserver:self selector:@selector(selectionAdded:) name:SelectionAdded object:selectionManager];
    }
    
    return self;
}

- (void)updateWithRay:(Ray3D *)theRay {
    [currentRay release];
    currentRay = [theRay retain];

    NSSet* copy = [[NSSet alloc] initWithSet:brushes];
    NSEnumerator* brushEn = [copy objectEnumerator];
    id <Brush> brush;
    while ((brush = [brushEn nextObject])) {
        if ([brush pickFace:currentRay] == nil)
            [self removeBrush:brush];
    }
    
    SelectionManager* selectionManager = [windowController selectionManager];
    brushEn = [[selectionManager selectedBrushes] objectEnumerator];
    while ((brush = [brushEn nextObject]))
        if ([brush pickFace:currentRay] != nil)
            [self addBrush:brush];
}

- (void)dealloc {
    [windowController release];
    [brushes release];
    [currentRay release];
    [super dealloc];
}

@end
