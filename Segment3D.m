//
//  Segment3D.m
//  TrenchBroom
//
//  Created by Kristian Duske on 19.10.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "Segment3D.h"

@implementation Segment3D
- (id)initWithStartVertex:(Vector3f *)start endVertex:(Vector3f *)end {
    if (start == nil)
        [NSException raise:NSInvalidArgumentException format:@"start vertex must not be nil"];
    if (end == nil)
        [NSException raise:NSInvalidArgumentException format:@"end vertex must not be nil"];
    
    if (self == [super init]) {
        startVertex = [[Vector3f alloc] initWithFloatVector:start];
        endVertex = [[Vector3f alloc] initWithFloatVector:end];
    }
}

- (Vector3f *)startVertex {
    return startVertex;
}

- (Vector3f *)endVertex {
    return endVertex;
}

- (void)dealloc {
    [startVertex release];
    [endVertex release];
    [super dealloc];
}
@end