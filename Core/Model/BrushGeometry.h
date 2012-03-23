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

#ifndef TrenchBroom_BrushGeometry_h
#define TrenchBroom_BrushGeometry_h

#include <vector>
#include "Math.h"
#include "Face.h"

using namespace std;

namespace TrenchBroom {
    
    typedef enum {
        VM_DROP,
        VM_KEEP,
        VM_UNDECIDED,
        VM_NEW,
        VM_UNKNOWN
    } EVertexMark;
    
    typedef enum {
        EM_KEEP,
        EM_DROP,
        EM_SPLIT,
        EM_UNDECIDED,
        EM_NEW,
        EM_UNKNOWN
    } EEdgeMark;
    
    typedef enum {
        SM_KEEP,
        SM_DROP,
        SM_SPLIT,
        SM_NEW,
        SM_UNKNOWN
    } ESideMark;

    class Vertex {
    public:
        TVector3f position;
        EVertexMark mark;
        Vertex(float x, float y, float z);
    };
    
    class Face;
    class Edge {
    public:
        Vertex* start;
        Vertex* end;
        Face* left;
        Face* right;
        EEdgeMark mark;
        Edge(Vertex* start, Vertex* end);
    };
    
    void centerOfVertices(vector<Vertex*>& vertices, TVector3f& center);
    EPointStatus vertexStatusFromRay(TVector3f origin, TVector3f direction, const vector<Vertex*>& vertices);
}

#endif
