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

    class MoveResult {
    public:
        int index;
        bool moved;
        MoveResult() {};
        MoveResult(int index, bool moved) : index(index), moved(moved) {}
    };
    
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
        TVector3f vector();
        TVector3f center();
        void updateMark();
        Vertex* split(TPlane plane);
        void flip();
    };

    class Face;
    class Side {
    public:
        vector<Vertex*> vertices;
        vector<Edge*> edges;
        Face* face;
        ESideMark mark;
        
        Side() : mark(SM_NEW) {}
        Side(Edge* edges[], bool invert[], int count);
        Side(Face& face, vector<Edge*>& edges);

        void replaceEdges(int index1, int index2, Edge* edge);
        Edge* split();
        void flip();
        void shift(int offset);
    };
    
    class BrushGeometry {
    private:
        vector<Side*> incidentSides(int vertexIndex);
        void deleteDegenerateTriangle(Side* side, Edge* edge, vector<Face*>& newFaces, vector<Face*>& droppedFaces);
        void triangulateSide(Side* side, int vertexIndex, vector<Face*>& newFaces);
        void splitSide(Side* side, int vertexIndex, vector<Face*>& newFaces);
        void splitSides(vector<Side*>& sides, TRay ray, int vertexIndex, vector<Face*>& newFaces, vector<Face*>& droppedFaces);
        void mergeVertices(Vertex* keepVertex, Vertex* dropVertex, vector<Face*>& newFaces, vector<Face*>& droppedFaces);
        void mergeEdges();
        void mergeNeighbours(Side* side, int edgeIndex);
        void mergeSides(vector<Face*>& newFaces, vector<Face*>&droppedFaces);
        float minVertexMoveDist(const vector<Side*>& sides, const Vertex* vertex, TRay ray, float maxDist);
        MoveResult moveVertex(int vertexIndex, bool mergeIncidentVertex, TVector3f delta, vector<Face*>& newFaces, vector<Face*>& droppedFaces);
        MoveResult splitAndMoveEdge(int index, TVector3f delta, vector<Face*>& newFaces, vector<Face*>& droppedFaces);
        MoveResult splitAndMoveSide(int sideIndex, TVector3f delta, vector<Face*>& newFaces, vector<Face*>& droppedFaces);
        void copy(const BrushGeometry& original);
    public:
        vector<Vertex*> vertices;
        vector<Edge*> edges;
        vector<Side*> sides;
        TBoundingBox bounds;

        BrushGeometry(const TBoundingBox& bounds);
        BrushGeometry(const BrushGeometry& original);
        ~BrushGeometry();
        
        bool closed() const;
        void restoreFaceSides();
        
        ECutResult addFace(Face& face, vector<Face*>& droppedFaces);
        bool addFaces(vector<Face*>& faces, vector<Face*>& droppedFaces);
        
        void translate(TVector3f delta);
        void rotate90CW(EAxis axis, TVector3f center);
        void rotate90CCW(EAxis axis, TVector3f center);
        void rotate(TQuaternion rotation, TVector3f center);
        void flip(EAxis axis, TVector3f center);
        void snap();
        
        MoveResult moveVertex(int vertexIndex, TVector3f delta, vector<Face*>& newFaces, vector<Face*>& droppedFaces);
        MoveResult moveEdge(int edgeIndex, TVector3f delta, vector<Face*>& newFaces, vector<Face*>& droppedFaces);
        MoveResult moveSide(int sideIndex, TVector3f delta, vector<Face*>& newFaces, vector<Face*>& droppedFaces);
    };
    

    template <class T> int indexOf(const vector<T*>& vec, const T* element);
    template <class T> bool removeElement(vector<T*>& vec, T* element);
    template <class T> bool deleteElement(vector<T*>& vec, T* element);
    int indexOf(const vector<Vertex*>& vertices, TVector3f v);
    int indexOf(const vector<Edge*>& edges, TVector3f v1, TVector3f v2);
    int indexOf(const vector<Side*>& sides, const vector<TVector3f>& vertices);

    TVector3f centerOfVertices(vector<Vertex*>& vertices);
    TBoundingBox boundsOfVertices(vector<Vertex*>& vertices);
    EPointStatus vertexStatusFromRay(TVector3f origin, TVector3f direction, const vector<Vertex*>& vertices);
}

#endif
