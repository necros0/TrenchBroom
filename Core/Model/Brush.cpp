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

#include "Brush.h"
#include <assert.h>

namespace TrenchBroom {
    void Brush::init() {
        static int currentId = 0;
        m_brushId = currentId++;
        m_entity = NULL;
        m_onGrid = false;
        m_filePosition = -1;
        m_selected = false;
    }
    
    void Brush::init(const TBoundingBox& bounds, Texture* texture) {
        init();
        
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
        Face* left = new Face(m_worldBounds, leftEdges, invertNone, 4);
        left->setTexture(texture);
        left->setBrush(this);
        
        Edge* rightEdges[4] = {rfdrfu, rfurbu, rburbd, rbdrfd};
        Face* right = new Face(m_worldBounds, rightEdges, invertNone, 4);
        right->setTexture(texture);
        right->setBrush(this);
        
        Edge* frontEdges[4] = {lfurfu, rfdrfu, rfdlfd, lfulfd};
        Face* front = new Face(m_worldBounds, frontEdges, invertOdd, 4);
        front->setTexture(texture);
        front->setBrush(this);
        
        Edge* backEdges[4] = {rbulbu, lbdlbu, lbdrbd, rburbd};
        Face* back = new Face(m_worldBounds, backEdges, invertOdd, 4);
        back->setTexture(texture);
        back->setBrush(this);
        
        Edge* topEdges[4] = {lbulfu, rbulbu, rfurbu, lfurfu};
        Face* top = new Face(m_worldBounds, topEdges, invertAll, 4);
        top->setTexture(texture);
        top->setBrush(this);
        
        Edge* downEdges[4] = {rfdlfd, rbdrfd, lbdrbd, lfdlbd};
        Face* down = new Face(m_worldBounds, downEdges, invertAll, 4);
        down->setTexture(texture);
        down->setBrush(this);
        
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
        
        m_faces.resize(6);
        m_faces[0] = left;
        m_faces[1] = right;
        m_faces[2] = front;
        m_faces[3] = back;
        m_faces[4] = top;
        m_faces[5] = down;
        
        m_bounds = bounds;
    }

    void Brush::reset() {
        while(!m_faces.empty()) delete m_faces.back(), m_faces.pop_back();
        while(!m_edges.empty()) delete m_edges.back(), m_faces.pop_back();
        while(!m_vertices.empty()) delete m_vertices.back(), m_faces.pop_back();
        init(m_worldBounds, NULL);
    }
    
    Brush::Brush(const TBoundingBox& worldBounds, Texture* texture) : m_worldBounds(worldBounds) {
        init(m_worldBounds, texture);
    }
    
    Brush::Brush(const TBoundingBox& worldBounds, const Brush& brushTemplate) : m_worldBounds(worldBounds) {
        init();
        restore(brushTemplate);
    }
    
    Brush::Brush(const TBoundingBox& worldBounds, const TBoundingBox& brushBounds, Texture* texture) : m_worldBounds(worldBounds) {
        init(brushBounds, texture);
    }
    
    void Brush::restore(const Brush& brushTemplate) {
        assert(m_brushId == brushTemplate.brushId());
        
        reset();
        vector<Face* > templateFaces = brushTemplate.faces();
        for (int i = 0; i < templateFaces.size(); i++) {
            Face* face = new Face(m_worldBounds, *templateFaces[i]);
            addFace(*face);
        }
        m_bounds = brushTemplate.bounds();
        
        m_entity->brushChanged(*this);
    }
    
    int Brush::brushId() const {
        return m_brushId;
    }
    
    Entity* Brush::entity() const {
        return m_entity;
    }
    
    void Brush::setEntity(Entity* entity) {
        m_entity = entity;
    }
    
    const vector<Face*>& Brush::faces() const {
        return m_faces;
    }
    
    const TBoundingBox& Brush::bounds() const {
        return m_bounds;
    }
    
    const TBoundingBox& Brush::worldBounds() const {
        return m_worldBounds;
    }

    const vector<Vertex*>& Brush::vertices() const {
        return m_vertices;
    }
    
    const vector<Edge*>& Brush::edges() const {
        return m_edges;
    }

    bool Brush::containsPoint(TVector3f point) {
        if (!boundsContainPoint(&m_bounds, &point))
            return false;
        
        for (int i = 0; i < m_faces.size(); i++) {
            TPlane boundary = m_faces[i]->boundary();
            if (pointStatusFromPlane(&boundary, &point) == PS_ABOVE)
                return false;
        }
        return true;
    }
    
    bool Brush::intersectsBrush(const Brush& brush) {
        TBoundingBox theirBounds = brush.bounds();
        if (!boundsIntersectWithBounds(&m_bounds, &theirBounds))
            return false;
        
        // separating axis theorem
        // http://www.geometrictools.com/Documentation/MethodOfSeparatingAxes.pdf

        const vector<Vertex*> theirVertices = brush.vertices();
        for (int i = 0; i < m_faces.size(); i++) {
            Face* myFace = m_faces[i];
            TVector3f origin = myFace->vertices()[0]->position;
            TVector3f direction = myFace->boundary().norm;
            if (vertexStatusFromRay(origin, direction, theirVertices) == PS_ABOVE)
                return false;
        }

        const vector<Face*>& theirFaces = brush.faces();
        for (int i = 0; i < theirFaces.size(); i++) {
            Face* theirFace = theirFaces[i];
            TVector3f origin = theirFace->vertices()[0]->position;
            TVector3f direction = theirFace->boundary().norm;
            if (vertexStatusFromRay(origin, direction, m_vertices) == PS_ABOVE)
                return false;
        }
        
        const vector<Edge*>& theirEdges = brush.edges();
        for (int i = 0; i < m_edges.size(); i++) {
            Edge* myEdge = m_edges[i];
            for (int j = 0; j < theirEdges.size(); j++) {
                Edge* theirEdge = theirEdges[i];
                TVector3f myEdgeVec, theirEdgeVec, direction;
                subV3f(&myEdge->end->position, &myEdge->start->position, &myEdgeVec);
                subV3f(&theirEdge->end->position, &theirEdge->start->position, &theirEdgeVec);
                
                crossV3f(&myEdgeVec, &theirEdgeVec, &direction);
                TVector3f origin = myEdge->start->position;
                
                EPointStatus myStatus = vertexStatusFromRay(origin, direction, m_vertices);
                if (myStatus != PS_INSIDE) {
                    EPointStatus theirStatus = vertexStatusFromRay(origin, direction, theirVertices);
                    if (theirStatus != PS_INSIDE) {
                        if (myStatus != theirStatus)
                            return false;
                    }
                }
            }
        }
        
        return true;
    }
    
    bool Brush::containsBrush(const Brush& brush) {
        TBoundingBox theirBounds = brush.bounds();
        if (!boundsContainBounds(&m_bounds, &theirBounds))
            return false;
        
        const vector<Vertex*>& theirVertices = brush.vertices();
        for (int i = 0; i < theirVertices.size(); i++)
            if (!containsPoint(theirVertices[i]->position))
                return false;
        return true;
    }
    
    bool Brush::intersectsEntity(const Entity& entity) {
        const TBoundingBox theirBounds = entity.bounds();
        if (!boundsIntersectWithBounds(&m_bounds, &theirBounds))
            return false;
        
        TVector3f point = theirBounds.min;
        if (containsPoint(point))
            return true;
        point.x = theirBounds.max.x;
        if (containsPoint(point))
            return true;
        point.y = theirBounds.max.y;
        if (containsPoint(point))
            return true;
        point.x = theirBounds.min.x;
        if (containsPoint(point))
            return true;
        point = theirBounds.max;
        if (containsPoint(point))
            return true;
        point.x = theirBounds.min.x;
        if (containsPoint(point))
            return true;
        point.y = theirBounds.min.y;
        if (containsPoint(point))
            return true;
        point.x = theirBounds.max.x;
        if (containsPoint(point))
            return true;
        return true;
    }
    
    bool Brush::containsEntity(const Entity& entity) {
        const TBoundingBox theirBounds = entity.bounds();
        if (!boundsContainBounds(&m_bounds, &theirBounds))
            return false;
        
        TVector3f point = theirBounds.min;
        if (!containsPoint(point))
            return false;
        point.x = theirBounds.max.x;
        if (!containsPoint(point))
            return false;
        point.y = theirBounds.max.y;
        if (!containsPoint(point))
            return false;
        point.x = theirBounds.min.x;
        if (!containsPoint(point))
            return false;
        point = theirBounds.max;
        if (!containsPoint(point))
            return false;
        point.x = theirBounds.min.x;
        if (!containsPoint(point))
            return false;
        point.y = theirBounds.min.y;
        if (!containsPoint(point))
            return false;
        point.x = theirBounds.max.x;
        if (!containsPoint(point))
            return false;
        return true;
    }

}