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

#import "AliasRenderer.h"
#import "Texture.h"
#import "Math.h"
#import "Entity.h"
#import "PreferencesManager.h"
#import "Alias.h"

using namespace TrenchBroom;

@implementation AliasRenderer

- (id)initWithAlias:(void *)theAlias skinIndex:(int)theSkinIndex vbo:(Vbo *)theVbo palette:(NSData *)thePalette {
    NSAssert(theAlias != nil, @"alias must not be nil");
    NSAssert(theSkinIndex >= 0, @"skin index must be at least 0");
    NSAssert(theVbo != NULL, @"VBO must not be nil");
    NSAssert(thePalette != nil, @"palette must not be nil");
    
    if ((self = [self init])) {
        alias = theAlias;
        skinIndex = theSkinIndex;
        vbo = theVbo;
        vboBlock = NULL;
        palette = [thePalette retain];
    }
    
    return self;
}

- (void)dealloc {
    if (vboBlock != NULL)
        freeVboBlock(vboBlock);
    [texture release];
    [palette release];
    [super dealloc];
}

- (void)renderWithEntity:(id<Entity>)theEntity {
    [self renderAtOrigin:[theEntity origin] angle:[theEntity angle]];
}

- (void)renderAtOrigin:(const TVector3f *)theOrigin angle:(NSNumber *)theAngle {
    if (vboBlock == nil) {
        Alias* al = (Alias *)alias;
        AliasSkin& skin = *al->skins[skinIndex];
        NSString* aliasName = [NSString stringWithCString:al->name.c_str() encoding:NSASCIIStringEncoding];
        texture = [[Texture alloc] initWithName:aliasName skin:&skin index:0 palette:palette];
        
        AliasSingleFrame& frame = al->firstFrame();
        triangleCount = frame.triangles.size();
        int vertexSize = 3 * 8;
        
        vboBlock = allocVboBlock(vbo, triangleCount * vertexSize * sizeof(float));
        mapVbo(vbo);
        
        int address = vboBlock->address;
        unsigned char* vboBuffer = vbo->buffer;
        
        for (int i = 0; i < triangleCount; i++) {
            AliasFrameTriangle* triangle = frame.triangles[i];
            for (int j = 0; j < 3; j++) {
                AliasFrameVertex& vertex = triangle->vertices[j];
                // GL_T2F_N3F_V3F format
                address = writeVector2f(&vertex.texCoords, vboBuffer, address);
                address = writeVector3f(&vertex.norm, vboBuffer, address);
                address = writeVector3f(&vertex.position, vboBuffer, address);
            }
        }
        
        unmapVbo(vbo);
    }
    
    glTranslatef(theOrigin->x, theOrigin->y, theOrigin->z);
    
    if (theAngle != nil) {
        int intAngle = [theAngle intValue];
        if (intAngle == -1)
            glRotatef(90, 1, 0, 0);
        else if (intAngle == 1)
            glRotatef(-90, 1, 0, 0);
        else
            glRotatef(-intAngle, 0, 0, 1);
    }
    
    glPolygonMode(GL_FRONT, GL_FILL);
    glEnable(GL_TEXTURE_2D);
    
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
    PreferencesManager* preferences = [PreferencesManager sharedManager];
    float brightness = [preferences brightness];
    glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
    glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
    
    glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_TEXTURE);
    glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_PRIMARY_COLOR);
    glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_TEXTURE);
    
    glColor3f(brightness / 2, brightness / 2, brightness / 2);
    glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE, 2.0f);

    [texture activate];

    glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
    glInterleavedArrays(GL_T2F_N3F_V3F, 0, (const GLvoid *)(long)vboBlock->address);
    glDrawArrays(GL_TRIANGLES, 0, triangleCount * 3);
    glPopClientAttrib();

    [texture deactivate];
}

- (const TVector3f *)center {
    return &((Alias *)alias)->firstFrame().center;
}

- (const TBoundingBox *)bounds {
    return &((Alias *)alias)->firstFrame().bounds;
}

- (const TBoundingBox *)maxBounds {
    return &((Alias *)alias)->firstFrame().maxBounds;
}

@end