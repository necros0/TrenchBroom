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

#import "Texture.h"
#import "IdGenerator.h"
#import "Math.h"
#import "Wad.h"
#import "Bsp.h"
#import "Alias.h"

using namespace TrenchBroom;

@implementation Texture

- (void)convertTexture:(const unsigned char *)theTextureData palette:(NSData *)thePalette {
    int size;
    
    size = width * height;
    textureBuffer = new unsigned char[size * 3];
    
    for (int i = 0; i < size; i++) {
        unsigned char paletteIndex = theTextureData[i];
        [thePalette getBytes:&textureBuffer[i * 3] range:NSMakeRange(paletteIndex * 3, 3)];
        
        color.x += textureBuffer[i * 3 + 0] / 255.0f;
        color.y += textureBuffer[i * 3 + 1] / 255.0f;
        color.z += textureBuffer[i * 3 + 2] / 255.0f;
    }

    color.x /= size;
    color.y /= size;
    color.z /= size;
    color.w = 1;
}

- (id)initWithWadEntry:(void *)theEntry palette:(NSData *)thePalette {
    NSAssert(theEntry != NULL, @"texture entry must not be nil");
    
    Mip* mip = (Mip *)theEntry;
    return [self initWithName:[NSString stringWithCString:mip->name.c_str() encoding:NSASCIIStringEncoding] image:mip->mip0 width:mip->width height:mip->height palette:thePalette];
}

- (id)initWithName:(NSString *)theName skin:(void *)theSkin index:(int)theIndex palette:(NSData *)thePalette {
    NSAssert(theSkin != NULL, @"skin must not be NULL");
    
    AliasSkin* skin = (AliasSkin *)theSkin;
    return [self initWithName:theName image:skin->pictures[theIndex] width:skin->width height:skin->height palette:thePalette];
}

- (id)initWithBspTexture:(void *)theBspTexture palette:(NSData *)thePalette {
    NSAssert(theBspTexture != NULL, @"BSP texture must not be NULL");
    
    BspTexture* bspTexture = (BspTexture *)theBspTexture;
    return [self initWithName:[NSString stringWithCString:bspTexture->name.c_str() encoding:NSASCIIStringEncoding] image:bspTexture->image width:bspTexture->width height:bspTexture->height palette:thePalette];
}

- (id)initWithName:(NSString *)theName image:(const unsigned char *)theImage width:(int)theWidth height:(int)theHeight palette:(NSData *)thePalette {
    NSAssert(theName != nil, @"name must not be nil");
    NSAssert(theImage != nil, @"image must not be nil");
    NSAssert(theWidth > 0, @"width must be positive");
    NSAssert(theHeight > 0, @"height must be positive");
    NSAssert(thePalette != nil, @"palette must no be nil");
    
    if ((self = [self init])) {
        uniqueId = [[[IdGenerator sharedGenerator] getId] retain];

        name = [[NSString alloc] initWithString:theName];
        width = theWidth;
        height = theHeight;
        [self convertTexture:theImage palette:thePalette];
        
        textureId = 0;
    }
    
    return self;
}

- (id)initDummyWithName:(NSString *)theName {
    NSAssert(theName != nil, @"name must not be nil");
    
    if ((self = [self init])) {
        name = [[NSString alloc] initWithString:theName];
        width = 1;
        height = 1;
        textureId = -1;
    }
    
    return self;
}

- (NSString *)name {
    return name;
}

- (NSNumber *)uniqueId {
    return uniqueId;
}

- (int)width {
    return width;
}

- (int)height {
    return height;
}

- (const TVector4f *)color {
    return &color;
}

- (BOOL)dummy {
    return textureId == -1;
}

- (void)incUsageCount {
    usageCount++;
}

- (void)decUsageCount {
    usageCount--;
}

- (void)setUsageCount:(int)theUsageCount {
    usageCount = theUsageCount;
}

- (int)usageCount {
    return usageCount;
}

- (void)activate {
    if (textureId == 0) {
        if (textureBuffer != NULL) {
            glGenTextures(1, &textureId);
            
            glBindTexture(GL_TEXTURE_2D, textureId);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, textureBuffer);
            delete[] textureBuffer;
            textureBuffer = NULL;
        } else {
            NSLog(@"Warning: cannot recreate texture '%@'", self);
        }
    }
    
    glBindTexture(GL_TEXTURE_2D, textureId);
}

- (void)deactivate {
    glBindTexture(GL_TEXTURE_2D, 0);
}

- (NSComparisonResult)compareByName:(Texture *)texture {
    return [name compare:[texture name]];
}

- (NSComparisonResult)compareByUsageCount:(Texture *)texture {
    if (usageCount > [texture usageCount])
        return NSOrderedAscending;
    if (usageCount < [texture usageCount])
        return NSOrderedDescending;
    return [self compareByName:texture];
}

- (NSString *)description {
    return [NSString stringWithFormat:@"Name: %@, %i*%i, ID: %i, GL ID: %i, used %i times",
            name, width, height, [uniqueId intValue], textureId, usageCount
            ];
}

- (void)dealloc {
    if (textureId != 0)
        glDeleteTextures(1, &textureId);
    [uniqueId release];
    [name release];
    delete textureBuffer;
    [super dealloc];
}
@end
