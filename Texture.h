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
#import <OpenGL/gl.h>
#import "Math.h"

@interface Texture : NSObject {
    NSString* name;
    NSNumber* uniqueId;
    GLuint textureId;
    int width;
    int height;
    int usageCount;
    TVector4f color;
    unsigned char* textureBuffer;
}

- (id)initWithWadEntry:(void *)theEntry palette:(NSData *)thePalette;
- (id)initWithName:(NSString *)theName skin:(void *)theSkin index:(int)theIndex palette:(NSData *)thePalette;
- (id)initWithBspTexture:(void *)theBspTexture palette:(NSData *)thePalette;
- (id)initWithName:(NSString *)theName image:(const unsigned char *)theImage width:(int)theWidth height:(int)theHeight palette:(NSData *)thePalette;
- (id)initDummyWithName:(NSString *)theName;

- (NSString *)name;
- (NSNumber *)uniqueId;
- (int)width;
- (int)height;
- (const TVector4f *)color;
- (BOOL)dummy;

- (void)incUsageCount;
- (void)decUsageCount;
- (void)setUsageCount:(int)theUsageCount;
- (int)usageCount;

- (void)activate;
- (void)deactivate;

- (NSComparisonResult)compareByName:(Texture *)texture;
- (NSComparisonResult)compareByUsageCount:(Texture *)texture;
@end
