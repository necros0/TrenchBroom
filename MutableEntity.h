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

#import <Cocoa/Cocoa.h>
#import "Entity.h"
#import "Math.h"

@protocol Map;
@class MutableBrush;
@class Face;
@class EntityDefinition;

@interface MutableEntity : NSObject <Entity> {
    EntityDefinition* entityDefinition;
    NSNumber* entityId;
    id <Map> map;
	NSMutableArray* brushes;
	NSMutableDictionary* properties;
    TVector3f center;
    TVector3f origin;
    NSNumber* angle;
    TBoundingBox bounds;
    TBoundingBox maxBounds;
    BOOL valid;
    int filePosition;
    BOOL selected;
    
    VboBlock* boundsVboBlock;
}

- (id)initWithProperties:(NSDictionary *)theProperties;

- (void)addBrush:(MutableBrush *)brush;
- (void)removeBrush:(MutableBrush *)brush;
- (void)brushChanged:(MutableBrush *)brush;

- (void)translateBy:(const TVector3f *)theDelta;
- (void)rotate90CW:(EAxis)theAxis center:(const TVector3f *)theCenter;
- (void)rotate90CCW:(EAxis)theAxis center:(const TVector3f *)theCenter;
- (void)rotate:(const TQuaternion *)theRotation center:(const TVector3f *)theRotationCenter;
- (void)flipAxis:(EAxis)theAxis center:(const TVector3f *)theCenter;

- (void)replaceProperties:(NSDictionary *)theProperties;
- (void)setProperty:(NSString *)key value:(NSString *)value;
- (void)removeProperty:(NSString *)key;

- (void)setEntityDefinition:(EntityDefinition *)theEntityDefintion;
- (void)setMap:(id <Map>)theMap;

- (int)filePosition;
- (void)setFilePosition:(int)theFilePosition;
@end
