//
//  PickingManager.h
//  TrenchBroom
//
//  Created by Kristian Duske on 29.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class Octree;
@class Ray3D;

@interface Picker : NSObject {
    @private
    Octree* octree;
}

- (id)initWithOctree:(Octree *)theOctree;

- (NSArray *)objectsHitByRay:(Ray3D *)theRay;

@end