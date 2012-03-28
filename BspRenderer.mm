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

#import "BspRenderer.h"
#import <OpenGL/gl.h>
#import "IntData.h"
#import "Entity.h"
#import "Texture.h"
#import "PreferencesManager.h"
#include "Bsp.h"

using namespace TrenchBroom;

@implementation BspRenderer

- (id)initWithBsp:(void *)theBsp vbo:(Vbo *)theVbo palette:(NSData *)thePalette {
    NSAssert(theBsp != nil, @"BSP must not be nil");
    NSAssert(theVbo != NULL, @"VBO must not be nil");
    NSAssert(thePalette != nil, @"palette must not be nil");
    
    if ((self = [self init])) {
        bsp = theBsp;
        vbo = theVbo;
        block = NULL;
        palette = [thePalette retain];
        textures = [[NSMutableDictionary alloc] init];
        indices = [[NSMutableDictionary alloc] init];
        counts = [[NSMutableDictionary alloc] init];
    }
    
    return self;
}

- (void)dealloc {
    [palette release];
    if (block != NULL)
        freeVboBlock(block);
    [textures release];
    [indices release];
    [counts release];
    [super dealloc];
}

- (void)renderWithEntity:(id<Entity>)theEntity {
    [self renderAtOrigin:[theEntity origin] angle:[theEntity angle]];
}

- (void)renderAtOrigin:(const TVector3f *)theOrigin angle:(NSNumber *)theAngle {
    if (block == nil) {
        mapVbo(vbo);
        
        BspModel& model = *((Bsp *)bsp)->models[0];
        int modelVertexCount = model.vertexCount;

        block = allocVboBlock(vbo, modelVertexCount * 5 * sizeof(float));
        int address = block->address;
        uint8_t* vboBuffer = vbo->buffer;
        
        for (vector<BspFace*>::iterator face = model.faces.begin(); face != model.faces.end(); face++) {
            BspTextureInfo* texInfo = (*face)->textureInfo;
            BspTexture* bspTexture = texInfo->texture;
            NSString* textureKey = [NSString stringWithCString:bspTexture->name.c_str() encoding:NSASCIIStringEncoding];
            
            Texture* texture = [textures objectForKey:textureKey];
            if (texture == nil) {
                texture = [[Texture alloc] initWithBspTexture:bspTexture palette:palette];
                [textures setObject:texture forKey:textureKey];
                [texture release];
            }
            
            IntData* indexBuffer = [indices objectForKey:[texture name]];
            if (indexBuffer == nil) {
                indexBuffer = [[IntData alloc] init];
                [indices setObject:indexBuffer forKey:[texture name]];
                [indexBuffer release];
            }
            
            IntData* countBuffer = [counts objectForKey:[texture name]];
            if (countBuffer == nil) {
                countBuffer = [[IntData alloc] init];
                [counts setObject:countBuffer forKey:[texture name]];
                [countBuffer release];
            }
            
            [indexBuffer appendInt:(address - block->address) / (5 * sizeof(float))];
            [countBuffer appendInt:(*face)->vertices.size()];

            for (vector<TVector3f>::iterator it = (*face)->vertices.begin(); it != (*face)->vertices.end(); it++) {
                TVector3f* vertex = it.base();
                TVector2f texCoords = (*face)->textureCoordinates(*vertex);
                address = writeVector2f(&texCoords, vboBuffer, address);
                address = writeVector3f(vertex, vboBuffer, address);
            }
        }
        
        unmapVbo(vbo);
    }
    
    glTranslatef(theOrigin->x, theOrigin->y, theOrigin->z);
    
    if (theAngle != nil) {
        int intAngle = [theAngle intValue];
        if (intAngle == -1)
            glRotatef(90, 1, 0, 0);
        else if (intAngle == -2)
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

    glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
    glInterleavedArrays(GL_T2F_V3F, 0, (const GLvoid *)(long)block->address);
    
    for (Texture* texture in [textures allValues]) {
        IntData* indexBuffer = [indices objectForKey:[texture name]];
        IntData* countBuffer = [counts objectForKey:[texture name]];
        
        GLint* indexBytes = (GLint *)[indexBuffer bytes];
        GLsizei* countBytes = (GLsizei *)[countBuffer bytes];
        GLsizei primCount = [indexBuffer count];
        
        [texture activate];
        glMultiDrawArrays(GL_POLYGON, indexBytes, countBytes, primCount);
    }
    
    glPopClientAttrib();
}


- (const TVector3f *)center {
    BspModel& model = *((Bsp *)bsp)->models[0];
    return &model.center;
}

- (const TBoundingBox *)bounds {
    BspModel& model = *((Bsp *)bsp)->models[0];
    return &model.bounds;
}

- (const TBoundingBox *)maxBounds {
    BspModel& model = *((Bsp *)bsp)->models[0];
    return &model.maxBounds;
}
@end
