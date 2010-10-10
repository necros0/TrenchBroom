//
//  Polyhedron.h
//  TrenchBroom
//
//  Created by Kristian Duske on 02.10.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Vector3f.h"
#import "HalfSpace3D.h"
#import "Polygon3D.h"

@interface Polyhedron : NSObject {
    NSMutableSet* sides;
}
+ (Polyhedron *)maximumCube;
+ (Polyhedron *)cuboidAt:(Vector3f *)center dimensions:(Vector3f *)dimensions;

- (id)initMaximumCube;
- (id)initCuboidAt:(Vector3f *)center dimensions:(Vector3f *)dimensions;

- (NSSet *)sides;

- (BOOL)intersectWith:(HalfSpace3D *)halfSpace;
@end