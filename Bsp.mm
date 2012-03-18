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

#import "Bsp.h"
#import "IO.h"
#import "Texture.h"
#import "BspModel.h"
#import "BspFace.h"
#include <istream>

using namespace std;

#define BSP_HEADER_VERSION            0x0
#define BSP_DIR_ENTITIES_ADDRESS      0x4
#define BSP_DIR_ENTITIES_SIZE         0x8
#define BSP_DIR_PLANES_ADDRESS        0xC
#define BSP_DIR_PLANES_SIZE           0x10
#define BSP_DIR_TEXTURES_ADDRESS      0x14
#define BSP_DIR_TEXTURES_SIZE         0x18
#define BSP_DIR_VERTICES_ADDRESS      0x1C
#define BSP_DIR_VERTICES_SIZE         0x20
#define BSP_DIR_VISILIST_ADDRESS      0x24
#define BSP_DIR_VISILIST_SIZE         0x28
#define BSP_DIR_NODES_ADDRESS         0x2C
#define BSP_DIR_NODES_SIZE            0x30
#define BSP_DIR_TEXINFOS_ADDRESS      0x34
#define BSP_DIR_TEXINFOS_SIZE         0x38
#define BSP_DIR_FACES_ADDRESS         0x3C
#define BSP_DIR_FACES_SIZE            0x40
#define BSP_DIR_LIGHTMAPS_ADDRESS     0x44
#define BSP_DIR_LIGHTMAPS_SIZE        0x48
#define BSP_DIR_CLIPNODES_ADDRESS     0x4C
#define BSP_DIR_CLIPNODES_SIZE        0x50
#define BSP_DIR_LEAVES_ADDRESS        0x54
#define BSP_DIR_LEAVES_SIZE           0x58
#define BSP_DIR_LEAF_FACES_ADDRESS    0x5C
#define BSP_DIR_LEAF_FACES_SIZE       0x60
#define BSP_DIR_EDGES_ADDRESS         0x64
#define BSP_DIR_EDGES_SIZE            0x68
#define BSP_DIR_FACE_EDGES_ADDRESS    0x6C
#define BSP_DIR_FACE_EDGES_SIZE       0x70
#define BSP_DIR_MODEL_ADDRESS         0x74
#define BSP_DIR_MODEL_SIZE            0x78

#define BSP_TEXTURE_DIR_COUNT         0x0
#define BSP_TEXTURE_DIR_OFFSETS       0x4
#define BSP_TEXTURE_NAME              0x0
#define BSP_TEXTURE_NAME_LENGTH       0x10
#define BSP_TEXTURE_WIDTH             0x10
#define BSP_TEXTURE_HEIGHT            0x14
#define BSP_TEXTURE_MIP0              0x18
#define BSP_TEXTURE_MIP1              0x1C
#define BSP_TEXTURE_MIP2              0x20
#define BSP_TEXTURE_MIP3              0x24

#define BSP_VERTEX_SIZE               0xC
#define BSP_VERTEX_X                  0x0
#define BSP_VERTEX_Y                  0x4
#define BSP_VERTEX_Z                  0x8

#define BSP_EDGE_SIZE                 0x4
#define BSP_EDGE_VERTEX0              0x0
#define BSP_EDGE_VERTEX1              0x2

#define BSP_FACE_SIZE                 0x14
#define BSP_FACE_EDGE_INDEX           0x4
#define BSP_FACE_EDGE_COUNT           0x8
#define BSP_FACE_TEXINFO              0xA
#define BSP_FACE_REST                 (BSP_FACE_SIZE - 0xC)

#define BSP_TEXINFO_SIZE              0x28
#define BSP_TEXINFO_S_AXIS            0x0
#define BSP_TEXINFO_S_OFFSET          0xC
#define BSP_TEXINFO_T_AXIS            0x10
#define BSP_TEXINFO_T_OFFSET          0x1C
#define BSP_TEXINFO_TEXTURE           0x20
#define BSP_TEXINFO_REST              (BSP_TEXINFO_SIZE - 0x24)

#define BSP_FACE_EDGE_SIZE            0x4

#define BSP_MODEL_SIZE                0x40
#define BSP_MODEL_ORIGIN              0x18
#define BSP_MODEL_FACE_INDEX          0x38
#define BSP_MODEL_FACE_COUNT          0x3C

@interface Bsp (Private)

- (void)readTextures:(istream *)theStream;
- (void)readVertices:(istream *)theStream count:(int)theCount result:(TVector3f *)theResult;
- (void)readEdges:(istream *)theStream count:(int)theCount result:(TEdge *)theResult;
- (void)readFaces:(istream *)theStream count:(int)theCount result:(TFace *)theResult;
- (void)readTexInfos:(istream *)theStream count:(int)theCount textures:(NSArray *)theTextures result:(TTextureInfo *)theResult;
- (void)readFaceEdges:(istream *)theStream count:(int)theCount result:(int *)theResult;

@end

@implementation Bsp (Private)

- (void)readTextures:(istream *)theStream {
    int32_t count, offset;
    uint32_t width, height, mip0Offset;
    char textureName[BSP_TEXTURE_NAME_LENGTH];
    unsigned char* mip0;
    int base;
    
    base = theStream->tellg();
    theStream->read((char *)&count, sizeof(int32_t));
    
    for (int i = 0; i < count; i++) {
        theStream->seekg(base + BSP_TEXTURE_DIR_OFFSETS + i * 4, ios::beg);
        theStream->read((char *)&offset, sizeof(int32_t));
        theStream->seekg(base + offset + BSP_TEXTURE_NAME, ios::beg);
        theStream->read(textureName, BSP_TEXTURE_NAME_LENGTH);
        theStream->read((char *)&width, sizeof(uint32_t));
        theStream->read((char *)&height, sizeof(uint32_t));
        theStream->read((char *)&mip0Offset, sizeof(uint32_t));
        
        mip0 = new unsigned char[width * height];
        theStream->seekg(base + offset + mip0Offset, ios::beg);
        theStream->read((char *)mip0, width * height);
        
        BspTexture* texture = [[BspTexture alloc] initWithName:[NSString stringWithCString:textureName encoding:NSASCIIStringEncoding] image:mip0 width:width height:height];
        [textures addObject:texture];
        [texture release];
    }
}

- (void)readVertices:(istream *)theStream count:(int)theCount result:(TVector3f *)theResult {
    for (int i = 0; i < theCount; i++)
        theStream->read((char *)&theResult[i], BSP_VERTEX_SIZE);
}

- (void)readEdges:(istream *)theStream count:(int)theCount result:(TEdge *)theResult {
    uint16_t vertex0, vertex1;
    
    for (int i = 0; i < theCount; i++) {
        theStream->read((char *)&vertex0, sizeof(uint16_t));
        theStream->read((char *)&vertex0, sizeof(uint16_t));
        theResult[i].vertex0 = vertex0;
        theResult[i].vertex1 = vertex1;
    }
}

- (void)readFaces:(istream *)theStream count:(int)theCount result:(TFace *)theResult {
    int32_t edgeIndex;
    uint16_t edgeCount, textureInfoIndex;
    
    for (int i = 0; i < theCount; i++) {
        theStream->seekg(BSP_FACE_EDGE_INDEX, ios::cur);
        theStream->read((char *)&edgeIndex, sizeof(uint32_t));
        theStream->read((char *)&edgeCount, sizeof(uint16_t));
        theStream->read((char *)&textureInfoIndex, sizeof(uint16_t));
        theStream->seekg(BSP_FACE_REST, ios::cur);
        
        theResult[i].edgeIndex = edgeIndex;
        theResult[i].edgeCount = edgeCount;
        theResult[i].textureInfoIndex = textureInfoIndex;
    }
}

- (void)readTexInfos:(istream *)theStream count:(int)theCount textures:(NSArray *)theTextures result:(TTextureInfo *)theResult {
    uint32_t textureIndex;
    
    for (int i = 0; i < theCount; i++) {
        theStream->read((char *)&theResult[i].sAxis, 3 * sizeof(float));
        theStream->read((char *)&theResult[i].sOffset, sizeof(float));
        theStream->read((char *)&theResult[i].tAxis, 3 * sizeof(float));
        theStream->read((char *)&theResult[i].tOffset, sizeof(float));
        theStream->read((char *)&textureIndex, sizeof(uint32_t));
        theResult[i].texture = [textures objectAtIndex:textureIndex];
        theStream->seekg(BSP_TEXINFO_REST, ios::cur);
    }
}

- (void)readFaceEdges:(istream *)theStream count:(int)theCount result:(int *)theResult {
    int32_t index;
    
    for (int i = 0; i < theCount; i++) {
        theStream->read((char *)&index, sizeof(int32_t));
        theResult[i] = index;
    }
}

@end

@implementation Bsp

- (id)init {
    if ((self = [super init])) {
        models = [[NSMutableArray alloc] init];
        textures = [[NSMutableArray alloc] init];
    }
    
    return self;
}

- (id)initWithName:(NSString *)theName stream:(void *)theStream {
    NSAssert(theName != nil, @"name must not be nil");
    NSAssert(theStream != NULL, @"stream must not be NULL");
    
    if ((self = [self init])) {
        name = [theName retain];
        istream* stream = (istream *)theStream;
        streampos pos = stream->tellg();
        
        int32_t textureAddr;
        stream->seekg(BSP_DIR_TEXTURES_ADDRESS, ios::beg);
        stream->read((char *)&textureAddr, sizeof(int32_t));
        stream->seekg(textureAddr, ios::beg);
        [self readTextures:stream];
        
        int32_t texInfosAddr, texInfosLength;
        int texInfoCount;
        stream->seekg(BSP_DIR_TEXINFOS_ADDRESS, ios::beg);
        stream->read((char *)texInfosAddr, sizeof(int32_t));
        stream->read((char *)texInfosLength, sizeof(int32_t));
        texInfoCount = texInfosLength / BSP_TEXINFO_SIZE;
        
        TTextureInfo texInfos[texInfoCount];
        stream->seekg(texInfosAddr, ios::beg);
        [self readTexInfos:stream count:texInfoCount textures:textures result:texInfos];

        int32_t verticesAddr, verticesLength;
        int vertexCount;
        stream->seekg(BSP_DIR_VERTICES_ADDRESS, ios::beg);
        stream->read((char *)&verticesAddr, sizeof(int32_t));
        stream->read((char *)&verticesLength, sizeof(int32_t));
        vertexCount = verticesLength / BSP_VERTEX_SIZE;
        
        TVector3f vertices[vertexCount];
        stream->seekg(verticesAddr, ios::beg);
        [self readVertices:stream count:vertexCount result:vertices];

        int32_t edgesAddr, edgesLength;
        int edgeCount;
        stream->seekg(BSP_DIR_EDGES_ADDRESS, ios::beg);
        stream->read((char *)&edgesAddr, sizeof(int32_t));
        stream->read((char *)&edgesLength, sizeof(int32_t));
        edgeCount = edgesLength / BSP_EDGE_SIZE;
        
        TEdge edges[edgeCount];
        stream->seekg(edgesAddr, ios::beg);
        [self readEdges:stream count:edgeCount result:edges];

        int32_t facesAddr, facesLength;
        int faceCount;
        stream->seekg(BSP_DIR_FACES_ADDRESS, ios::beg);
        stream->read((char *)&facesAddr, sizeof(int32_t));
        stream->read((char *)&facesLength, sizeof(int32_t));
        faceCount = facesLength / BSP_FACE_SIZE;
        
        TFace faces[faceCount];
        stream->seekg(facesAddr, ios::cur);
        [self readFaces:stream count:faceCount result:faces];
        
        int32_t faceEdgesAddr, faceEdgesLength;
        int faceEdgesCount;
        stream->seekg(BSP_DIR_FACE_EDGES_ADDRESS, ios::beg);
        stream->read((char *)&faceEdgesAddr, sizeof(int32_t));
        stream->read((char *)&faceEdgesLength, sizeof(int32_t));
        faceEdgesCount = faceEdgesLength / BSP_FACE_EDGE_SIZE;
        
        int faceEdges[faceEdgesCount];
        stream->seekg(faceEdgesAddr, ios::beg);
        [self readFaceEdges:stream count:faceEdgesCount result:faceEdges];

        int32_t modelsAddr, modelsLength;
        int modelCount;
        stream->seekg(BSP_DIR_MODEL_ADDRESS, ios::beg);
        stream->read((char *)&modelsAddr, sizeof(int32_t));
        stream->read((char *)&modelsLength, sizeof(int32_t));
        modelCount = modelsLength / BSP_MODEL_SIZE;
        
        BOOL vertexMark[vertexCount];
        memset(vertexMark, 0, vertexCount * sizeof(BOOL));
        int modelVertices[vertexCount];

        for (int i = 0; i < modelCount; i++) {
            int32_t faceIndex, faceCount;
            int totalVertexCount = 0;
            int modelVertexCount = 0;
            
            stream->seekg(BSP_MODEL_FACE_INDEX, ios::cur);
            stream->read((char *)&faceIndex, sizeof(int32_t));
            stream->read((char *)&faceCount, sizeof(int32_t));
            
            NSMutableArray* bspFaces = [[NSMutableArray alloc] initWithCapacity:faceCount];
            for (int j = 0; j < faceCount; j++) {
                TFace* face = &faces[faceIndex + j];
                TTextureInfo* texInfo = &texInfos[face->textureInfoIndex];
                
                int faceVertexCount = face->edgeCount;
                TVector3f faceVertices[faceVertexCount];
                for (int k = 0; k < face->edgeCount; k++) {
                    int faceEdgeIndex = faceEdges[face->edgeIndex + k];
                    int vertexIndex;
                    if (faceEdgeIndex < 0)
                        vertexIndex = edges[-faceEdgeIndex].vertex1;
                    else
                        vertexIndex = edges[faceEdgeIndex].vertex0;
                    faceVertices[k] = vertices[vertexIndex];
                    if (!vertexMark[vertexIndex]) {
                        vertexMark[vertexIndex] = YES;
                        modelVertices[modelVertexCount++] = vertexIndex;
                    }
                }
                
                BspFace* bspFace = [[BspFace alloc] initWithTextureInfo:texInfo vertices:faceVertices vertexCount:faceVertexCount];
                [bspFaces addObject:bspFace];
                [bspFace release];
                
                totalVertexCount += faceVertexCount;
            }
            
            TVector3f center;
            TBoundingBox bounds;

            center = vertices[modelVertices[0]];
            bounds.min = vertices[modelVertices[0]];
            bounds.max = vertices[modelVertices[0]];
            
            for (int i = 1; i < modelVertexCount; i++) {
                int vertexIndex = modelVertices[i];
                addV3f(&center, &vertices[vertexIndex], &center);
                mergeBoundsWithPoint(&bounds, &vertices[vertexIndex], &bounds);
                vertexMark[vertexIndex] = NO;
            }
            
            scaleV3f(&center, 1.0f / modelVertexCount, &center);

            TBoundingBox maxBounds;
            TVector3f diff;
            float distSquared = 0;
            
            for (int i = 0; i < modelVertexCount; i++) {
                int vertexIndex = modelVertices[i];
                subV3f(&vertices[vertexIndex], &center, &diff);
                distSquared = fmax(distSquared, lengthSquaredV3f(&diff));
            }

            float dist = sqrt(distSquared);
            maxBounds.min = center;
            maxBounds.min.x -= dist;
            maxBounds.min.y -= dist;
            maxBounds.min.z -= dist;
            maxBounds.max = center;
            maxBounds.max.x += dist;
            maxBounds.max.y += dist;
            maxBounds.max.z += dist;
            
            BspModel* bspModel = [[BspModel alloc] initWithFaces:bspFaces vertexCount:totalVertexCount center:&center bounds:&bounds maxBounds:&maxBounds];
            [bspFaces release];
            
            [models addObject:bspModel];
            [bspModel release];
        }
    }
    
    return self;
}

- (void)dealloc {
    [name release];
    [models release];
    [textures release];
    [super dealloc];
}

- (NSString *)name {
    return name;
}

- (NSArray *)models {
    return models;
}

@end
