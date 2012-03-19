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

#import "Alias.h"
#import "AliasSkin.h"
#import "AliasNormals.h"
#import "AliasFrame.h"
#import "AliasFrameGroup.h"
#include <istream>

using namespace std;

#define MDL_HEADER_SCALE              0x8
#define MDL_HEADER_ORIGIN             0x14
#define MDL_HEADER_RADIUS             0x20
#define MDL_HEADER_OFFSETS            0x24
#define MDL_HEADER_NUMSKINS           0x30
#define MDL_HEADER_SKINWIDTH          0x34
#define MDL_HEADER_SKINHEIGHT         0x38
#define MDL_HEADER_NUMVERTS           0x3C
#define MDL_HEADER_NUMTRIS            0x40
#define MDL_HEADER_NUMFRAMES          0x44
#define MDL_HEADER_SYNCTYPE           0x48
#define MDL_HEADER_FLAGS              0x4C
#define MDL_HEADER_SIZE               0x50

#define MDL_SKINS                     0x54
#define MDL_SKIN_GROUP                0x0
#define MDL_SKIN_NUMPICS              0x4
#define MDL_SKIN_TIME                 0x8

#define MDL_SINGLE_SKIN               0x4

#define MDL_MULTI_SKIN_NUMPICS        0x4
#define MDL_MULTI_SKIN_TIMES          0x8
#define MDL_MULTI_SKIN                0xC

#define MDL_VERTEX_ONSEAM             0x0
#define MDL_VERTEX_S                  0x4
#define MDL_VERTEX_T                  0x8
#define MDL_VERTEX_SIZE               0xC

#define MDL_TRI_FRONT                 0x0
#define MDL_TRI_VERTICES              0x4
#define MDL_TRI_SIZE                  0x10

#define MDL_FRAME_VERTEX_SIZE         0x4

#define MDL_FRAME_TYPE                0x0

#define MDL_SIMPLE_FRAME_MIN          0x0
#define MDL_SIMPLE_FRAME_MAX          0x4
#define MDL_SIMPLE_FRAME_NAME         0x8
#define MDL_SIMPLE_FRAME_NAME_SIZE    0x10
#define MDL_SIMPLE_FRAME_VERTICES     0x18
#define MDL_SIMPLE_FRAME_SIZE         0x18

#define MDL_MULTI_FRAME_NUMFRAMES     0x0
#define MDL_MULTI_FRAME_MIN           0x4
#define MDL_MULTI_FRAME_MAX           0x8
#define MDL_MULTI_FRAME_TIMES         0xC
#define MDL_MULTI_FRAME_SIZE          0xC

void readFrameVertex(istream* stream, TPackedFrameVertex* vertex) {
    unsigned char x, y, z, i;
    stream->read((char *)&x, sizeof(unsigned char));
    stream->read((char *)&y, sizeof(unsigned char));
    stream->read((char *)&z, sizeof(unsigned char));
    stream->read((char *)&i, sizeof(unsigned char));
    vertex->x = x;
    vertex->y = y;
    vertex->z = z;
    vertex->i = i;
}

void unpackFrameVertex(const TPackedFrameVertex* packedVertex, const TVector3f* origin, const TVector3f* size, TVector3f* result) {
    result->x = size->x * packedVertex->x + origin->x;
    result->y = size->y * packedVertex->y + origin->y;
    result->z = size->z * packedVertex->z + origin->z;
}

void setTexCoordinates(const TSkinVertex* skinVertex, BOOL front, int skinWidth, int skinHeight, TFrameVertex* vertex) {
    vertex->texCoords.x = (float)skinVertex->s / skinWidth;
    vertex->texCoords.y = (float)skinVertex->t / skinHeight;
    if (skinVertex->onseam && !front)
        vertex->texCoords.x += 0.5f;
}

AliasFrame* readFrame(istream* stream, TVector3f* origin, TVector3f* scale, int skinWidth, int skinHeight, TSkinVertex* vertices, int vertexCount, TSkinTriangle* triangles, int triangleCount) {
    char name[MDL_SIMPLE_FRAME_NAME_SIZE];
    stream->seekg(MDL_SIMPLE_FRAME_NAME, ios::cur);
    stream->read(name, MDL_SIMPLE_FRAME_NAME_SIZE);
    
    TPackedFrameVertex packedFrameVertices[vertexCount];
    for (int i = 0; i < vertexCount; i++)
        readFrameVertex(stream, &packedFrameVertices[i]);
    
    TVector3f frameVertices[vertexCount];
    TVector3f center;
    TBoundingBox bounds;

    unpackFrameVertex(&packedFrameVertices[0], origin, scale, &frameVertices[0]);
    center = frameVertices[0];
    bounds.min = frameVertices[0];
    bounds.max = frameVertices[0];
    
    for (int i = 1; i < vertexCount; i++) {
        unpackFrameVertex(&packedFrameVertices[i], origin, scale, &frameVertices[i]);
        addV3f(&center, &frameVertices[i], &center);
        mergeBoundsWithPoint(&bounds, &frameVertices[i], &bounds);
    }
    
    scaleV3f(&center, 1.0f / vertexCount, &center);

    TVector3f diff;
    float distSquared;
    for (int i = 0; i < vertexCount; i++) {
        subV3f(&frameVertices[i], &center, &diff);
        distSquared = fmax(distSquared, lengthSquaredV3f(&diff));
    }
    
    float dist = sqrt(distSquared);
    
    TBoundingBox maxBounds;
    maxBounds.min = center;
    maxBounds.min.x -= dist;
    maxBounds.min.y -= dist;
    maxBounds.min.z -= dist;
    maxBounds.max = center;
    maxBounds.max.x += dist;
    maxBounds.max.y += dist;
    maxBounds.max.z += dist;
    
    TFrameTriangle frameTriangles[triangleCount];
    for (int i = 0; i < triangleCount; i++) {
        for (int j = 0; j < 3; j++) {
            int index = triangles[i].vertices[j];
            frameTriangles[i].vertices[j].position = frameVertices[index];
            frameTriangles[i].vertices[j].norm = AliasNormals[packedFrameVertices[index].i];
            setTexCoordinates(&vertices[index], triangles[i].front, skinWidth, skinHeight, &frameTriangles[i].vertices[j]);
        }
    }
    
    return [[[AliasFrame alloc] initWithName:[NSString stringWithCString:name encoding:NSASCIIStringEncoding] triangles:frameTriangles triangleCount:triangleCount center:&center bounds:&bounds maxBounds:&maxBounds] autorelease];
}

@implementation Alias

- (id)initWithName:(NSString *)theName stream:(void *)theStream {
    NSAssert(theName != nil, @"name must not be nil");
    NSAssert(theStream != NULL, @"stream must not be NULL");
    
    if ((self = [self init])) {
        name = [[NSString alloc] initWithString:theName];
        istream* stream = (istream *)theStream;
        
        TVector3f scale, origin;
        int32_t skinCount, skinWidth, skinHeight, skinSize;
        int32_t vertexCount, triangleCount, frameCount;
        
        stream->seekg(MDL_HEADER_SCALE, ios::beg);
        stream->read((char *)&scale, sizeof(TVector3f));
        stream->read((char *)&origin, sizeof(TVector3f));

        stream->seekg(MDL_HEADER_NUMSKINS, ios::beg);
        stream->read((char *)&skinCount, sizeof(int32_t));
        stream->read((char *)&skinWidth, sizeof(int32_t));
        stream->read((char *)&skinHeight, sizeof(int32_t));
        skinSize = skinWidth * skinHeight;
        
        stream->read((char *)&vertexCount, sizeof(int32_t));
        stream->read((char *)&triangleCount, sizeof(int32_t));
        stream->read((char *)&frameCount, sizeof(int32_t));

        skins = [[NSMutableArray alloc] initWithCapacity:skinCount];
        stream->seekg(MDL_SKINS, ios::beg);
        for (int i = 0; i < skinCount; i++) {
            int32_t skinGroup;
            
            stream->read((char *)&skinGroup, sizeof(int32_t));
            if (skinGroup == 0) {
                unsigned char skinPicture[skinSize];
                stream->read((char *)skinPicture, skinSize);
                AliasSkin* skin = [[AliasSkin alloc] initSingleSkin:skinPicture width:skinWidth height:skinHeight];
                [skins addObject:skin];
                [skin release];
            } else {
                int32_t numPics;
                stream->read((char *)&numPics, sizeof(int32_t));
                float times[numPics];
                unsigned char skinPictures[numPics * skinSize];
                
                streampos base = stream->tellg();
                for (int j = 0; j < numPics; j++) {
                    stream->seekg(j * sizeof(float) + base, ios::beg);
                    stream->read((char *)&times[j], sizeof(float));
                    
                    stream->seekg(numPics * sizeof(float) + j * skinSize + base, ios::beg);
                    stream->read((char *)&skinPictures[j], skinSize);
                }
                
                AliasSkin* skin = [[AliasSkin alloc] initMultiSkin:skinPictures times:times count:numPics width:skinWidth height:skinHeight];
                [skins addObject:skin];
                [skin release];
            }
        }
        
        // now stream is at the first skin vertex
        TSkinVertex vertices[vertexCount];
        for (int i = 0; i < vertexCount; i++) {
            int32_t onseam, s, t;
            stream->read((char *)&onseam, sizeof(int32_t));
            stream->read((char *)&s, sizeof(int32_t));
            stream->read((char *)&t, sizeof(int32_t));
            vertices[i].onseam = onseam != 0;
            vertices[i].s = s;
            vertices[i].t = t;
        }
        
        // now stream is at the first skin triangle
        TSkinTriangle triangles[triangleCount];
        for (int i = 0; i < triangleCount; i++) {
            int32_t front, vertex;
            stream->read((char *)&front, sizeof(int32_t));
            triangles[i].front = front;
            for (int j = 0; j < 3; j++) {
                stream->read((char *)&vertex, sizeof(int32_t));
                triangles[i].vertices[j] = vertex;
            }
        }
        
        // now stream is at the first frame
        frames = [[NSMutableArray alloc] initWithCapacity:frameCount];
        for (int i = 0; i < frameCount; i++) {
            int32_t type;
            stream->read((char *)&type, sizeof(int32_t));
            if (type == 0) { // single frame
                [frames addObject:readFrame(stream, &origin, &scale, skinWidth, skinHeight, vertices, vertexCount, triangles, triangleCount)];
            } else { // frame group
                int32_t groupFrameCount;
                streampos base = stream->tellg();
                stream->read((char *)&groupFrameCount, sizeof(int32_t));

                int timePos = MDL_MULTI_FRAME_TIMES + base;
                int framePos = MDL_MULTI_FRAME_TIMES + groupFrameCount * sizeof(float) + base;
                
                float groupFrameTimes[groupFrameCount];
                NSMutableArray* groupFrames = [[NSMutableArray alloc] initWithCapacity:groupFrameCount];
                for (int j = 0; j < groupFrameCount; j++) {
                    stream->seekg(timePos, ios::beg);
                    stream->read((char *)&groupFrameTimes[j], sizeof(float));
                    timePos = j * sizeof(float) + timePos;
                    
                    stream->seekg(framePos, ios::beg);
                    [groupFrames addObject:readFrame(stream, &origin, &scale, skinWidth, skinHeight, vertices, vertexCount, triangles, triangleCount)];
                    framePos = MDL_SIMPLE_FRAME_NAME_SIZE + (vertexCount + 2) * MDL_FRAME_VERTEX_SIZE + framePos;
                }
                
                AliasFrameGroup* frameGroup = [[AliasFrameGroup alloc] initWithFrames:groupFrames times:groupFrameTimes];
                [frames addObject:frameGroup];
                [frameGroup release];
                [groupFrames release];
            }
        }
    }
    
    return self;
}

- (void)dealloc {
    [name release];
    [frames release];
    [skins release];
    [super dealloc];
}

- (NSString *)name {
    return name;
}

- (AliasFrame *)firstFrame {
    id frame = [frames objectAtIndex:0];
    if ([frame isKindOfClass:[AliasFrame class]])
        return frame;
    
    AliasFrameGroup* group = (AliasFrameGroup *)frame;
    return [group frameAtIndex:0];
}

- (AliasSkin *)firstSkin {
    return [skins objectAtIndex:0];
}

- (AliasSkin *)skinWithIndex:(int)theSkinIndex {
    NSAssert(theSkinIndex >= 0 && theSkinIndex < [skins count], @"skin index out of range");
    return [skins objectAtIndex:theSkinIndex];
}

@end
