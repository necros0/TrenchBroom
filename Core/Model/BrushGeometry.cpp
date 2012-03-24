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
            m_edges.push_back(edges[i]);
            if (invert[i]) {
                edges[i]->left = this;
                m_vertices.push_back(edges[i]->end);
            } else {
                edges[i]->right = this;
                m_vertices.push_back(edges[i]->start);
            }
        }
        
        m_face = NULL;
        m_mark = SM_NEW;
    }
    
    Side::Side(Face& face, vector<Edge*>& edges) {
        for (int i = 0; i < edges.size(); i++) {
            edges[i]->left = this;
            m_edges.push_back(edges[i]);
            m_vertices.push_back(edges[i]->startVertex(this));
        }
        
        m_face = &face;
        m_face->setSide(this);
        m_mark = SM_NEW;
    }
    
    vector<Vertex*>& Side::vertices() {
        return m_vertices;
    }
    
    vector<Edge*>& Side::edges() {
        return m_edges;
    }
    
    Face* Side::face() {
        return m_face;
    }
    
    ESideMark Side::mark() {
        return m_mark;
    }

    void Side::setMark(ESideMark mark) {
        m_mark = mark;
    }

    Edge* Side::split() {
        int keep = 0;
        int drop = 0;
        int split = 0;
        int undecided = 0;
        Edge* undecidedEdge = NULL;
        
        int splitIndex1 = -2;
        int splitIndex2 = -2;
        
        Edge* edge = m_edges.back();
        EEdgeMark lastMark = edge->mark;
        for (int i = 0; i < m_edges.size(); i++) {
            edge = m_edges[i];
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
                    splitIndex1 = i > 0 ? i - 1 : m_edges.size() - 1;
                drop++;
            }
            lastMark = currentMark;
        }
        
        if (keep == m_edges.size()) {
            m_mark = SM_KEEP;
            return NULL;
        }
        
        if (undecided == 1 && keep == m_edges.size() - 1) {
            m_mark = SM_KEEP;
            return undecidedEdge;
        }
        
        if (drop + undecided == m_edges.size()) {
            m_mark = SM_DROP;
            return NULL;
        }
        
        assert(splitIndex1 >= 0 && splitIndex2 >= 0);
        m_mark = SM_SPLIT;
        
        Edge* newEdge = new Edge();
        newEdge->start = m_edges[splitIndex1]->endVertex(this);
        newEdge->end = m_edges[splitIndex2]->startVertex(this);
        newEdge->left = NULL;
        newEdge->right = this;
        newEdge->mark = EM_NEW;
        
        replaceEdges(splitIndex1, splitIndex2, newEdge);
        return newEdge;
    }

    void Side::replaceEdges(int index1, int index2, Edge* edge) {
        vector<Edge*> newEdges;
        vector<Vertex*> newVertices;
        
        if (index2 > index1) {
            for (int i = 0; i <= index1; i++) {
                newEdges.push_back(m_edges[i]);
                newVertices.push_back(m_edges[i]->startVertex(this));
            }
            
            newEdges.push_back(edge);
            newVertices.push_back(edge->startVertex(this));
            
            for (int i = index2; i < m_edges.size(); i++) {
                newEdges.push_back(m_edges[i]);
                newVertices.push_back(m_edges[i]->startVertex(this));
            }
        } else {
            for (int i = index2; i <= index1; i++) {
                newEdges.push_back(m_edges[i]);
                newVertices.push_back(m_edges[i]->startVertex(this));
            }
            
            newEdges.push_back(edge);
            newVertices.push_back(edge->startVertex(this));
        }

        m_edges = newEdges;
        m_vertices = newVertices;
    }

    
    BrushGeometry::BrushGeometry(const TBoundingBox& bounds) : m_bounds(bounds) {
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
        
        m_vertices.resize(8);
        m_vertices[0] = lfd;
        m_vertices[1] = lfu;
        m_vertices[2] = lbd;
        m_vertices[3] = lbu;
        m_vertices[4] = rfd;
        m_vertices[5] = rfu;
        m_vertices[6] = rbd;
        m_vertices[7] = rbu;
        
        m_edges.resize(12);
        m_edges[ 0] = lfdlbd;
        m_edges[ 1] = lbdlbu;
        m_edges[ 2] = lbulfu;
        m_edges[ 3] = lfulfd;
        m_edges[ 4] = rfdrfu;
        m_edges[ 5] = rfurbu;
        m_edges[ 6] = rburbd;
        m_edges[ 7] = rbdrfd;
        m_edges[ 8] = lfurfu;
        m_edges[ 9] = rfdlfd;
        m_edges[10] = lbdrbd;
        m_edges[11] = rbulbu;
        
        m_sides.resize(6);
        m_sides[0] = left;
        m_sides[1] = right;
        m_sides[2] = front;
        m_sides[3] = back;
        m_sides[4] = top;
        m_sides[5] = down;
    }
    
    BrushGeometry::BrushGeometry(const TBoundingBox& worldBounds, vector<Face*>& faces) {
    }
    
    const vector<Vertex*>& BrushGeometry::vertices() const {
        return m_vertices;
    }
    
    const vector<Edge*>& BrushGeometry::edges() const {
        return m_edges;
    }
    
    const vector<Side*>& BrushGeometry::sides() const {
        return m_sides;
    }
    
    const TBoundingBox& BrushGeometry::bounds() const {
        return m_bounds;
    }

    ECutResult BrushGeometry::addFace(Face& face, vector<Face*>& droppedFaces) {
        TPlane boundary = face.boundary();
        
        int keep = 0;
        int drop = 0;
        int undecided = 0;
        
        // mark vertices
        for (int i = 0; i < m_vertices.size(); i++) {
            Vertex& vertex = *m_vertices[i];
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
        
        if (keep + undecided == m_vertices.size())
            return CR_REDUNDANT;
        
        if (drop + undecided == m_vertices.size())
            return CR_NULL;
        
        // mark and split edges
        for (int i = 0; i < m_edges.size(); i++) {
            Edge& edge = *m_edges[i];
            edge.updateMark();
            if (edge.mark == EM_SPLIT) {
                Vertex* vertex = edge.split(boundary);
                m_vertices.push_back(vertex);
            }
        }
        
        // mark, split and drop sides
        vector<Edge*> newEdges;
        vector<Side*>::iterator sideIt;
        for (sideIt = m_sides.begin(); sideIt != m_sides.end(); sideIt++) {
            Side* side = *sideIt;
            Edge* newEdge = side->split();
            
            if (side->mark() == SM_DROP) {
                Face* face = side->face();
                if (face != NULL) {
                    droppedFaces.push_back(face);
                    face->setSide(NULL);
                }
                delete side;
                sideIt = m_sides.erase(sideIt);
            } else if (side->mark() == SM_SPLIT) {
                m_edges.push_back(newEdge);
                newEdges.push_back(newEdge);
                side->setMark(SM_UNKNOWN);
            } else if (side->mark() == SM_KEEP && newEdge != NULL) {
                // the edge is an undecided edge, so it needs to be flipped in order to act as a new edge
                if (newEdge->right != side)
                    newEdge->flip();
                newEdges.push_back(newEdge);
                side->setMark(SM_UNKNOWN);
            } else {
                side->setMark(SM_UNKNOWN);
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
        m_sides.push_back(newSide);
        
        // clean up
        // delete dropped vertices
        vector<Vertex*>::iterator vertexIt;
        for (vertexIt = m_vertices.begin(); vertexIt != m_vertices.end(); vertexIt++) {
            Vertex* vertex = *vertexIt;
            if (vertex->mark == VM_DROP) {
                delete vertex;
                vertexIt = m_vertices.erase(vertexIt);
            } else {
                vertex->mark = VM_UNDECIDED;
            }
        }
        
        // delete dropped edges
        vector<Edge*>::iterator edgeIt;
        for (edgeIt = m_edges.begin(); edgeIt != m_edges.end(); edgeIt++) {
            Edge* edge = *edgeIt;
            if (edge->mark == EM_DROP) {
                delete edge;
                edgeIt = m_edges.erase(edgeIt);
            } else {
                edge->mark = EM_UNDECIDED;
            }
        }
        
        m_bounds = boundsOfVertices(m_vertices);
        return CR_SPLIT;
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