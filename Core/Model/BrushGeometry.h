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
        CR_REDUNDANT, // the given face is redundant and need not be added to the brush
        CR_NULL, // the given face has nullified the entire brush
        CR_SPLIT // the given face has split the brush
    } ECutResult;

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
        Vertex();
    };
    
    class Side;
    class Edge {
    public:
        Vertex* start;
        Vertex* end;
        Side* left;
        Side* right;
        EEdgeMark mark;
        Edge(Vertex* start, Vertex* end);
        Edge();
        Vertex* startVertex(Side* side);
        Vertex* endVertex(Side* side);
        void updateMark();
        Vertex* split(TPlane plane);
        void flip();
    };

    class Face;
    class Side {
    private:
        vector<Vertex*> m_vertices;
        vector<Edge*> m_edges;
        Face* m_face;
        ESideMark m_mark;
    public:
        Side(Edge* edges[], bool invert[], int count);
        Side(Face& face, vector<Edge*>& edges);

        vector<Vertex*>& vertices();
        vector<Edge*>& edges();
        Face* face();
        ESideMark mark();
        void setMark(ESideMark mark);

        Edge* split();
        void replaceEdges(int index1, int index2, Edge* edge);
    };
    
    class BrushGeometry {
    private:
        vector<Vertex*> m_vertices;
        vector<Edge*> m_edges;
        vector<Side*> m_sides;
        TBoundingBox m_bounds;

        void init();
    public:
        BrushGeometry(const TBoundingBox& bounds);
        BrushGeometry(const TBoundingBox& worldBounds, vector<Face*>& faces);
        
        const vector<Vertex*>& vertices() const;
        const vector<Edge*>& edges() const;
        const vector<Side*>& sides() const;
        const TBoundingBox& bounds() const;
        
        ECutResult addFace(Face& face, vector<Face*>& droppedFaces);
    };
    
    TVector3f centerOfVertices(vector<Vertex*>& vertices);
    TBoundingBox boundsOfVertices(vector<Vertex*>& vertices);
    EPointStatus vertexStatusFromRay(TVector3f origin, TVector3f direction, const vector<Vertex*>& vertices);
}

#endif
