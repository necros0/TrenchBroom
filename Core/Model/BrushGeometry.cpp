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

#include "BrushGeometry.h"
#include <assert.h>
#include <map>

namespace TrenchBroom {
    
    Vertex::Vertex(float x, float y, float z) {
        position.x = x;
        position.y = y;
        position.z = z;
        mark = VM_NEW;
    }
    
    Vertex::Vertex() {
        mark = VM_NEW;
    }
    
    Edge::Edge(Vertex* start, Vertex* end) : start(start), end(end), mark((EM_NEW)) {}
    Edge::Edge() : mark(EM_NEW) {}
    
    Vertex* Edge::startVertex(Side* side) {
        if (left == side) return end;
        if (right == side) return start;
        return NULL;
    }
    
    Vertex* Edge::endVertex(Side* side) {
        if (left == side) return start;
        if (right == side) return end;
        return NULL;
    }

    void Edge::updateMark() {
        int keep = 0;
        int drop = 0;
        int undecided = 0;
        
        if (start->mark == VM_KEEP) keep++;
        else if (start->mark == VM_DROP) drop++;
        else if (start->mark == VM_UNDECIDED) undecided++;
        
        if (end->mark == VM_KEEP) keep++;
        else if (end->mark == VM_DROP) drop++;
        else if (end->mark == VM_UNDECIDED) undecided++;
        
        if (keep == 1 && drop == 1) mark = EM_SPLIT;
        else if (keep > 0) mark = EM_KEEP;
        else if (drop > 0) mark = EM_DROP;
        else mark = EM_UNDECIDED;
    }
    
    
    Vertex* Edge::split(TPlane plane) {
        TLine line;
        Vertex* newVertex = new Vertex();
        setLinePoints(&line, &start->position, &end->position);
        
        float dist = intersectPlaneWithLine(&plane, &line);
        linePointAtDistance(&line, dist, &newVertex->position);
        snapV3f(&newVertex->position, &newVertex->position);
        newVertex->mark = VM_NEW;
        
        if (start->mark == VM_DROP) start = newVertex;
        else end = newVertex;
        
        return newVertex;
    }

    void Edge::flip() {
        Side* tempSide = left;
        left = right;
        right = tempSide;
        Vertex* tempVertex = start;
        start = end;
        end = tempVertex;
    }

    Side::Side(Edge* edges[], bool invert[], int count) {
        for (int i = 0; i < count; i++) {
            this->edges.push_back(edges[i]);
            if (invert[i]) {
                edges[i]->left = this;
                vertices.push_back(edges[i]->end);
            } else {
                edges[i]->right = this;
                vertices.push_back(edges[i]->start);
            }
        }
        
        face = NULL;
        mark = SM_NEW;
    }
    
    Side::Side(Face& face, vector<Edge*>& edges) {
        for (int i = 0; i < edges.size(); i++) {
            edges[i]->left = this;
            edges.push_back(edges[i]);
            vertices.push_back(edges[i]->startVertex(this));
        }
        
        this->face = &face;
        this->face->setSide(this);
        mark = SM_NEW;
    }
    
    void Side::replaceEdges(int index1, int index2, Edge* edge) {
        vector<Edge*> newEdges;
        vector<Vertex*> newVertices;
        
        if (index2 > index1) {
            for (int i = 0; i <= index1; i++) {
                newEdges.push_back(edges[i]);
                newVertices.push_back(edges[i]->startVertex(this));
            }
            
            newEdges.push_back(edge);
            newVertices.push_back(edge->startVertex(this));
            
            for (int i = index2; i < edges.size(); i++) {
                newEdges.push_back(edges[i]);
                newVertices.push_back(edges[i]->startVertex(this));
            }
        } else {
            for (int i = index2; i <= index1; i++) {
                newEdges.push_back(edges[i]);
                newVertices.push_back(edges[i]->startVertex(this));
            }
            
            newEdges.push_back(edge);
            newVertices.push_back(edge->startVertex(this));
        }
        
        edges = newEdges;
        vertices = newVertices;
    }
    
    Edge* Side::split() {
        int keep = 0;
        int drop = 0;
        int split = 0;
        int undecided = 0;
        Edge* undecidedEdge = NULL;
        
        int splitIndex1 = -2;
        int splitIndex2 = -2;
        
        Edge* edge = edges.back();
        EEdgeMark lastMark = edge->mark;
        for (int i = 0; i < edges.size(); i++) {
            edge = edges[i];
            EEdgeMark currentMark = edge->mark;
            if (currentMark == EM_SPLIT) {
                Vertex* start = edge->startVertex(this);
                if (start->mark == VM_KEEP)
                    splitIndex1 = i;
                else
                    splitIndex2 = i;
                split++;
            } else if (currentMark == EM_UNDECIDED) {
                undecided++;
                undecidedEdge = edge;
            } else if (currentMark == EM_KEEP) {
                if (lastMark == EM_DROP)
                    splitIndex2 = i;
                keep++;
            } else if (currentMark == EM_DROP) {
                if (lastMark == EM_KEEP)
                    splitIndex1 = i > 0 ? i - 1 : edges.size() - 1;
                drop++;
            }
            lastMark = currentMark;
        }
        
        if (keep == edges.size()) {
            mark = SM_KEEP;
            return NULL;
        }
        
        if (undecided == 1 && keep == edges.size() - 1) {
            mark = SM_KEEP;
            return undecidedEdge;
        }
        
        if (drop + undecided == edges.size()) {
            mark = SM_DROP;
            return NULL;
        }
        
        assert(splitIndex1 >= 0 && splitIndex2 >= 0);
        mark = SM_SPLIT;
        
        Edge* newEdge = new Edge();
        newEdge->start = edges[splitIndex1]->endVertex(this);
        newEdge->end = edges[splitIndex2]->startVertex(this);
        newEdge->left = NULL;
        newEdge->right = this;
        newEdge->mark = EM_NEW;
        
        replaceEdges(splitIndex1, splitIndex2, newEdge);
        return newEdge;
    }

    void Side::flip() {
        Vertex* tempVertex;
        int j;
        for (int i = 0; i < vertices.size() / 2; i++) {
            j = vertices.size() - i - 1;
            tempVertex = vertices[i];
            vertices[i] = vertices[j];
            vertices[j] = tempVertex;
        }
    }
    
    MoveResult BrushGeometry::moveVertex(int vertexIndex, bool killable, TVector3f delta, vector<Face*>& newFaces, vector<Face*>& droppedFaces) {
    }
    
    MoveResult BrushGeometry::splitAndMoveEdge(int edgeIndex, TVector3f delta, vector<Face*>& newFaces, vector<Face*>& droppedFaces) {
    }
    
    MoveResult BrushGeometry::splitAndMoveSide(int sideIndex, TVector3f delta, vector<Face*>& newFaces, vector<Face*>& droppedFaces) {
    }

    void BrushGeometry::copy(const BrushGeometry& original) {
        map<Vertex*, Vertex*> vertexMap;
        map<Edge*, Edge*> edgeMap;
        map<Side*, Side*> sideMap;
        
        vertices.reserve(original.vertices.size());
        edges.reserve(original.edges.size());
        sides.reserve(original.sides.size());
        
        for (int i = 0; i < original.vertices.size(); i++) {
            Vertex* originalVertex = original.vertices[i];
            Vertex* copyVertex = new Vertex(*originalVertex);
            vertexMap[originalVertex] = copyVertex;
            vertices.push_back(copyVertex);
        }
        
        for (int i = 0; i < original.edges.size(); i++) {
            Edge* originalEdge = original.edges[i];
            Edge* copyEdge = new Edge(*originalEdge);
            copyEdge->start = vertexMap[originalEdge->start];
            copyEdge->end = vertexMap[originalEdge->end];
            edgeMap[originalEdge] = copyEdge;
            edges.push_back(copyEdge);
        }
        
        for (int i = 0; i < original.sides.size(); i++) {
            Side* originalSide = original.sides[i];
            Side* copySide = new Side(*originalSide);
            copySide->vertices.clear();
            copySide->edges.clear();
            
            for (int j = 0; j < originalSide->edges.size(); j++) {
                Edge* originalEdge = originalSide->edges[j];
                Edge* copyEdge = edgeMap[originalEdge];
                
                if (originalEdge->left == originalSide) copyEdge->left = copySide;
                else copyEdge->right = copySide;
                copySide->edges.push_back(copyEdge);
                copySide->vertices.push_back(copyEdge->startVertex(copySide));
            }
        }
        
        bounds = original.bounds;
    }
    
    BrushGeometry::BrushGeometry(const TBoundingBox& bounds) {
        Vertex* lfd = new Vertex(bounds.min.x, bounds.min.y, bounds.min.z);
        Vertex* lfu = new Vertex(bounds.min.x, bounds.min.y, bounds.max.z);
        Vertex* lbd = new Vertex(bounds.min.x, bounds.max.y, bounds.min.z);
        Vertex* lbu = new Vertex(bounds.min.x, bounds.max.y, bounds.max.z);
        Vertex* rfd = new Vertex(bounds.max.x, bounds.min.y, bounds.min.z);
        Vertex* rfu = new Vertex(bounds.max.x, bounds.min.y, bounds.max.z);
        Vertex* rbd = new Vertex(bounds.max.x, bounds.max.y, bounds.min.z);
        Vertex* rbu = new Vertex(bounds.max.x, bounds.max.y, bounds.max.z);
        
        Edge* lfdlbd = new Edge(lfd, lbd);
        Edge* lbdlbu = new Edge(lbd, lbu);
        Edge* lbulfu = new Edge(lbu, lfu);
        Edge* lfulfd = new Edge(lfu, lfd);
        Edge* rfdrfu = new Edge(rfd, rfu);
        Edge* rfurbu = new Edge(rfu, rbu);
        Edge* rburbd = new Edge(rbu, rbd);
        Edge* rbdrfd = new Edge(rbd, rfd);
        Edge* lfurfu = new Edge(lfu, rfu);
        Edge* rfdlfd = new Edge(rfd, lfd);
        Edge* lbdrbd = new Edge(lbd, rbd);
        Edge* rbulbu = new Edge(rbu, lbu);
        
        bool invertNone[4] = {false, false, false, false};
        bool invertAll[4] = {true, true, true, true};
        bool invertOdd[4] = {false, true, false, true};
        
        Edge* leftEdges[4] = {lfdlbd, lbdlbu, lbulfu, lfulfd};
        Side* left = new Side(leftEdges, invertNone, 4);
        
        Edge* rightEdges[4] = {rfdrfu, rfurbu, rburbd, rbdrfd};
        Side* right = new Side(rightEdges, invertNone, 4);
        
        Edge* frontEdges[4] = {lfurfu, rfdrfu, rfdlfd, lfulfd};
        Side* front = new Side(frontEdges, invertOdd, 4);
        
        Edge* backEdges[4] = {rbulbu, lbdlbu, lbdrbd, rburbd};
        Side* back = new Side(backEdges, invertOdd, 4);
        
        Edge* topEdges[4] = {lbulfu, rbulbu, rfurbu, lfurfu};
        Side* top = new Side(topEdges, invertAll, 4);
        
        Edge* downEdges[4] = {rfdlfd, rbdrfd, lbdrbd, lfdlbd};
        Side* down = new Side(downEdges, invertAll, 4);
        
        vertices.resize(8);
        vertices[0] = lfd;
        vertices[1] = lfu;
        vertices[2] = lbd;
        vertices[3] = lbu;
        vertices[4] = rfd;
        vertices[5] = rfu;
        vertices[6] = rbd;
        vertices[7] = rbu;
        
        edges.resize(12);
        edges[ 0] = lfdlbd;
        edges[ 1] = lbdlbu;
        edges[ 2] = lbulfu;
        edges[ 3] = lfulfd;
        edges[ 4] = rfdrfu;
        edges[ 5] = rfurbu;
        edges[ 6] = rburbd;
        edges[ 7] = rbdrfd;
        edges[ 8] = lfurfu;
        edges[ 9] = rfdlfd;
        edges[10] = lbdrbd;
        edges[11] = rbulbu;
        
        sides.resize(6);
        sides[0] = left;
        sides[1] = right;
        sides[2] = front;
        sides[3] = back;
        sides[4] = top;
        sides[5] = down;
        
        this->bounds = bounds;
    }
    
    BrushGeometry::BrushGeometry(const BrushGeometry& original) {
        copy(original);
    }
    
    BrushGeometry::~BrushGeometry() {
        while(!sides.empty()) delete sides.back(), sides.pop_back();
        while(!edges.empty()) delete edges.back(), edges.pop_back();
        while(!vertices.empty()) delete vertices.back(), vertices.pop_back();
    }

    bool BrushGeometry::closed() const {
        for (int i = 0; i < sides.size(); i++)
            if (sides[i]->face == NULL)
                return false;
        return true;
    }

    void BrushGeometry::restoreFaceSides() {
        for (int i = 0; i < sides.size(); i++)
            sides[i]->face->setSide(sides[i]);
    }
    
    ECutResult BrushGeometry::addFace(Face& face, vector<Face*>& droppedFaces) {
        TPlane boundary = face.boundary();
        
        int keep = 0;
        int drop = 0;
        int undecided = 0;
        
        // mark vertices
        for (int i = 0; i < vertices.size(); i++) {
            Vertex& vertex = *vertices[i];
            EPointStatus vs = pointStatusFromPlane(&boundary, &vertex.position);
            if (vs == PS_ABOVE) {
                vertex.mark = VM_DROP;
                drop++;
            } else if (vs == PS_BELOW) {
                vertex.mark  = VM_KEEP;
                keep++;
            } else {
                vertex.mark = VM_UNDECIDED;
                undecided++;
            }
        }
        
        if (keep + undecided == vertices.size())
            return CR_REDUNDANT;
        
        if (drop + undecided == vertices.size())
            return CR_NULL;
        
        // mark and split edges
        for (int i = 0; i < edges.size(); i++) {
            Edge& edge = *edges[i];
            edge.updateMark();
            if (edge.mark == EM_SPLIT) {
                Vertex* vertex = edge.split(boundary);
                vertices.push_back(vertex);
            }
        }
        
        // mark, split and drop sides
        vector<Edge*> newEdges;
        vector<Side*>::iterator sideIt;
        for (sideIt = sides.begin(); sideIt != sides.end(); sideIt++) {
            Side* side = *sideIt;
            Edge* newEdge = side->split();
            
            if (side->mark == SM_DROP) {
                Face* face = side->face;
                if (face != NULL) {
                    droppedFaces.push_back(face);
                    face->setSide(NULL);
                }
                delete side;
                sideIt = sides.erase(sideIt);
            } else if (side->mark == SM_SPLIT) {
                edges.push_back(newEdge);
                newEdges.push_back(newEdge);
                side->mark = SM_UNKNOWN;
            } else if (side->mark == SM_KEEP && newEdge != NULL) {
                // the edge is an undecided edge, so it needs to be flipped in order to act as a new edge
                if (newEdge->right != side)
                    newEdge->flip();
                newEdges.push_back(newEdge);
                side->mark = SM_UNKNOWN;
            } else {
                side->mark = SM_UNKNOWN;
            }
        }
        
        // create new side from newly created edges
        // first, sort the new edges to form a polygon in clockwise order
        for (int i = 0; i < newEdges.size() - 1; i++) {
            Edge* edge = newEdges[i];
            for (int j = i + 2; j < newEdges.size(); j++) {
                Edge* candidate = newEdges[j];
                if (edge->start == candidate->end) {
                    newEdges[j] = newEdges[i + 1];
                    newEdges[i + 1] = candidate;
                }
            }
        }
        
        // now create the new side
        Side* newSide = new Side(face, newEdges);
        sides.push_back(newSide);
        
        // clean up
        // delete dropped vertices
        vector<Vertex*>::iterator vertexIt;
        for (vertexIt = vertices.begin(); vertexIt != vertices.end(); vertexIt++) {
            Vertex* vertex = *vertexIt;
            if (vertex->mark == VM_DROP) {
                delete vertex;
                vertexIt = vertices.erase(vertexIt);
            } else {
                vertex->mark = VM_UNDECIDED;
            }
        }
        
        // delete dropped edges
        vector<Edge*>::iterator edgeIt;
        for (edgeIt = edges.begin(); edgeIt != edges.end(); edgeIt++) {
            Edge* edge = *edgeIt;
            if (edge->mark == EM_DROP) {
                delete edge;
                edgeIt = edges.erase(edgeIt);
            } else {
                edge->mark = EM_UNDECIDED;
            }
        }
        
        bounds = boundsOfVertices(vertices);
        return CR_SPLIT;
    }
    
    bool BrushGeometry::addFaces(vector<Face*>& faces, vector<Face*>& droppedFaces) {
        for (int i = 0; i < faces.size(); i++)
            if (addFace(*faces[i], droppedFaces) == CR_NULL)
                return false;
        return true;
    }
    
    void BrushGeometry::translate(TVector3f delta) {
        for (int i = 0; i < vertices.size(); i++)
            addV3f(&vertices[i]->position, &delta, &vertices[i]->position);
        translateBounds(&bounds, &delta, &bounds);
    }

    void BrushGeometry::rotate90CW(EAxis axis, TVector3f center) {
        for (int i = 0; i < vertices.size(); i++) {
            subV3f(&vertices[i]->position, &center, &vertices[i]->position);
            rotate90CWV3f(&vertices[i]->position, axis, &vertices[i]->position);
            addV3f(&vertices[i]->position, &center, &vertices[i]->position);
        }
        rotateBounds90CW(&bounds, axis, &center, &bounds);
    }
    
    void BrushGeometry::rotate90CCW(EAxis axis, TVector3f center) {
        for (int i = 0; i < vertices.size(); i++) {
            subV3f(&vertices[i]->position, &center, &vertices[i]->position);
            rotate90CCWV3f(&vertices[i]->position, axis, &vertices[i]->position);
            addV3f(&vertices[i]->position, &center, &vertices[i]->position);
        }
        rotateBounds90CCW(&bounds, axis, &center, &bounds);
    }
    
    void BrushGeometry::rotate(TQuaternion rotation, TVector3f center) {
        for (int i = 0; i < vertices.size(); i++) {
            subV3f(&vertices[i]->position, &center, &vertices[i]->position);
            rotateQ(&rotation, &vertices[i]->position, &vertices[i]->position);
            addV3f(&vertices[i]->position, &center, &vertices[i]->position);
        }
        rotateBounds(&bounds, &rotation, &center, &bounds);
    }
    
    void BrushGeometry::flip(EAxis axis, TVector3f center) {
        float min, max;
        switch (axis) {
            case A_X:
                for (int i = 0; i < vertices.size(); i++) {
                    vertices[i]->position.x -= center.x;
                    vertices[i]->position.x *= -1;
                    vertices[i]->position.x += center.x;
                }
                
                min = bounds.max.x;
                max = bounds.min.x;
                min -= center.x;
                min *= -1;
                min += center.x;
                max -= center.x;
                max *= -1;
                max += center.x;
                bounds.min.x = min;
                bounds.max.x = max;
                break;
            case A_Y:
                for (int i = 0; i < vertices.size(); i++) {
                    vertices[i]->position.y -= center.y;
                    vertices[i]->position.y *= -1;
                    vertices[i]->position.y += center.y;
                }
                
                min = bounds.max.y;
                max = bounds.min.y;
                min -= center.y;
                min *= -1;
                min += center.y;
                max -= center.y;
                max *= -1;
                max += center.y;
                bounds.min.y = min;
                bounds.max.y = max;
                break;
            default:
                for (int i = 0; i < vertices.size(); i++) {
                    vertices[i]->position.z -= center.z;
                    vertices[i]->position.z *= -1;
                    vertices[i]->position.z += center.z;
                }
                
                min = bounds.max.z;
                max = bounds.min.z;
                min -= center.z;
                min *= -1;
                min += center.z;
                max -= center.z;
                max *= -1;
                max += center.z;
                bounds.min.z = min;
                bounds.max.z = max;
                break;
        }
        
        for (int i = 0; i < edges.size(); i++)
            edges[i]->flip();
        
        for (int i = 0; i < sides.size(); i++)
            sides[i]->flip();

    }
    
    void BrushGeometry::snap() {
    }
    
    MoveResult BrushGeometry::moveVertex(int vertexIndex, TVector3f delta, vector<Face*>& newFaces, vector<Face*>& droppedFaces) {
        assert(vertexIndex >= 0);
        assert(vertexIndex < vertices.size() + edges.size() + sides.size());
        
        MoveResult result;
        if (lengthSquaredV3f(&delta) == 0)
            result = MoveResult(vertexIndex, false);
        else if (vertexIndex < vertices.size())
            result = moveVertex(vertexIndex, true, delta, newFaces, droppedFaces);
        else if (vertexIndex < vertices.size() + edges.size())
            result = splitAndMoveEdge(vertexIndex, delta, newFaces, droppedFaces);
        else
            result = splitAndMoveSide(vertexIndex, delta, newFaces, droppedFaces);
        
        return result;
    }
    
    MoveResult BrushGeometry::moveEdge(int edgeIndex, TVector3f delta, vector<Face*>& newFaces, vector<Face*>& droppedFaces) {
        assert(edgeIndex >= 0 && edgeIndex < edges.size());
        
        if (lengthSquaredV3f(&delta) == 0)
            return MoveResult(edgeIndex, false);
        
        BrushGeometry testGeometry(*this);
        testGeometry.restoreFaceSides();
        
        TVector3f dir;
        Edge* edge = testGeometry.edges[edgeIndex];
        TVector3f start = edge->start->position;
        TVector3f end = edge->end->position;
        subV3f(&end, &start, &dir);
        addV3f(&start, &delta, &start);
        addV3f(&end, &delta, &end);
        
        MoveResult result;
        if (dotV3f(&dir, &delta) > 0) {
            result = testGeometry.moveVertex(indexOf<Vertex>(testGeometry.vertices, edge->end), false, delta, newFaces, droppedFaces);
            if (result.moved)
                result = testGeometry.moveVertex(indexOf<Vertex>(testGeometry.vertices, edge->start), false, delta, newFaces, droppedFaces);
        } else {
            result = testGeometry.moveVertex(indexOf<Vertex>(testGeometry.vertices, edge->start), false, delta, newFaces, droppedFaces);
            if (result.moved)
                result = testGeometry.moveVertex(indexOf<Vertex>(testGeometry.vertices, edge->end), false, delta, newFaces, droppedFaces);
        }
        
        if (result.moved) {
            result.index = indexOf(testGeometry.edges, start, end);
            copy(testGeometry);
        } else {
            result.index = edgeIndex;
            newFaces.clear();
            droppedFaces.clear();
        }
        
        restoreFaceSides();
        return result;
    }
    
    MoveResult BrushGeometry::moveSide(int sideIndex, TVector3f delta, vector<Face*>& newFaces, vector<Face*>& droppedFaces) {
        assert(sideIndex >= 0 && sideIndex < sides.size());
        
        float dist = lengthV3f(&delta);
        if (dist == 0)
            return MoveResult(sideIndex, false);
        
        BrushGeometry testGeometry(*this);
        testGeometry.restoreFaceSides();
        
        TVector3f dir, diff;
        scaleV3f(&delta, 1 / dist, &dir);
        Side* side = testGeometry.sides[sideIndex];
        TVector3f center = centerOfVertices(side->vertices);
        
        int sideVertexCount = side->vertices.size();
        vector<TVector3f> sideVertices(sideVertexCount);
        vector<int> indices(sideVertexCount);
        vector<float> dots(sideVertexCount);
        for (int i = 0; i < sideVertexCount; i++) {
            sideVertices[i] = side->vertices[i]->position;
            subV3f(&sideVertices[i], &center, &diff);
            dots[i] = dotV3f(&diff, &dir);
            indices[i] = indexOf<Vertex>(testGeometry.vertices, side->vertices[i]);
            addV3f(&sideVertices[i], &delta, &sideVertices[i]);
        }
        
        // sort indices by dot value, eek, bubblesort
        bool switched = true;
        for (int j = sideVertexCount - 1; j >= 0 && switched; j--) {
            switched = false;
            for (int i = 0; i < j; i++) {
                if (dots[i] > dots[i + 1]) {
                    float dt = dots[i];
                    dots[i] = dots[i + 1];
                    dots[i + 1] = dt;
                    
                    int di = indices[i];
                    indices[i] = indices[i + 1];
                    indices[i + 1] = di;
                    switched = true;
                }
            }
        }
        
        MoveResult result(-1, true);
        for (int i = 0; i < sideVertexCount && result.moved; i++)
            result = moveVertex(indices[i], false, delta, newFaces, droppedFaces);
        
        if (result.moved) {
            result.index = indexOf(testGeometry.sides, sideVertices);
            copy(testGeometry);
        } else {
            result.index = sideIndex;
            newFaces.clear();
            droppedFaces.clear();
        }

        restoreFaceSides();
        return result;
    }

    template <class T>
    int indexOf(const vector<T*>& vec, const T* element) {
        for (int i = 0; i < vec.size(); i++)
            if (vec[i] == element)
                return i;
        return -1;
    }
    
    int indexOf(const vector<Edge*>& edges, TVector3f v1, TVector3f v2) {
        for (int i = 0; i < edges.size(); i++) {
            Edge* edge = edges[i];
            if ((equalV3f(&edge->start->position, &v1) && equalV3f(&edge->end->position, &v2)) ||
                (equalV3f(&edge->start->position, &v2) && equalV3f(&edge->end->position, &v1)))
                return i;
        }
        return -1;
    }

    int indexOf(const vector<Side*>& sides, const vector<TVector3f>& vertices) {
        for (int i = 0; i < sides.size(); i++) {
            Side* side = sides[i];
            if (side->vertices.size() == vertices.size()) {
                for (int j = 0; j < vertices.size(); j++) {
                    int k = 0;
                    while (k < vertices.size() && equalV3f(&side->vertices[(j + k) % vertices.size()]->position, &vertices[k]))
                        k++;
                    
                    if (k == vertices.size())
                        return i;
                }
            }
        }
        
        return -1;    
    }
    
    TVector3f centerOfVertices(vector<Vertex*>& vertices) {
        TVector3f center = vertices[0]->position;
        for (int i = 1; i < vertices.size(); i++)
            addV3f(&center, &vertices[0]->position, &center);
        scaleV3f(&center, 1.0f / vertices.size(), &center);
        return center;
    }
    
    TBoundingBox boundsOfVertices(vector<Vertex*>& vertices) {
        TBoundingBox bounds;
        bounds.min = vertices[0]->position;
        bounds.max = vertices[0]->position;
        
        for  (int i = 1; i < vertices.size(); i++)
            mergeBoundsWithPoint(&bounds, &vertices[i]->position, &bounds);
        return bounds;
    }
    
    EPointStatus vertexStatusFromRay(TVector3f origin, TVector3f direction, const vector<Vertex*>& vertices) {
        int above = 0;
        int below = 0;
        for (int i = 0; i < vertices.size(); i++) {
            EPointStatus status = pointStatusFromRay(&origin, &direction, &vertices[i]->position);
            if (status == PS_ABOVE)
                above++;
            else if (status == PS_BELOW)
                below++;
            if (above > 0 && below > 0)
                return PS_INSIDE;
        }
        
        return above > 0 ? PS_ABOVE : PS_BELOW;
    }
}