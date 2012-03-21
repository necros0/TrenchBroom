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

#include "Face.h"
#include <assert.h>
#include <cmath>

namespace TrenchBroom {

    void Face::init() {
        static int currentId = 0;
        m_faceId = currentId++;
        m_texture = NULL;
        m_vboBlock = NULL;
        m_filePosition = -1;
        m_valid = false;
    }
    
    Face::Face(TBoundingBox& worldBounds) : m_worldBounds(worldBounds) {
        init();
    }
    
    Face::Face(TBoundingBox& worldBounds, TVector3f point1, TVector3f point2, TVector3f point3) : m_worldBounds(worldBounds) {
        init();
        m_points[0] = point1;
        m_points[1] = point2;
        m_points[3] = point3;
    }
    
    Face::Face(TBoundingBox& worldBounds, Face& faceTemplate) : m_worldBounds(worldBounds) {
        init();
        restore(faceTemplate);
    }
    
    Face::~Face() {
        if (m_vboBlock != NULL)
            freeVboBlock(m_vboBlock);
    }
    
    void Face::restore(Face& faceTemplate) {
        assert(m_faceId == faceTemplate.faceId());
               
        faceTemplate.points(m_points[0], m_points[1], m_points[2]);
        m_boundary = faceTemplate.boundary();
        m_xOffset = faceTemplate.xOffset();
        m_yOffset = faceTemplate.yOffset();
        m_rotation = faceTemplate.rotation();
        m_xScale = faceTemplate.xScale();
        m_yScale = faceTemplate.yScale();
        setTexture(faceTemplate.texture());
        m_valid = false;
    }
    
    int Face::faceId() {
        return m_faceId;
    }
    
    Brush* Face::brush() {
        return m_brush;
    }
    
    void Face::setBrush(Brush* brush) {
        m_brush = brush;
    }
    
    void Face::points(TVector3f& point1, TVector3f& point2, TVector3f& point3) {
        point1 = m_points[0];
        point2 = m_points[1];
        point3 = m_points[2];
    }
    
    void Face::updatePoints() {
        TVector3f v1, v2;
        
        float bestDot = 1;
        int best = -1;
        vector<Vertex*> vertices = this->vertices();
        int vertexCount = vertices.size();
        for (int i = 0; i < vertexCount && bestDot > 0; i++) {
            m_points[2] = vertices[(i - 1 + vertexCount) % vertexCount]->position;
            m_points[0] = vertices[i]->position;
            m_points[1] = vertices[(i + 1) % vertexCount]->position;
            
            subV3f(&m_points[2], &m_points[0], &v1);
            normalizeV3f(&v1, &v1);
            subV3f(&m_points[1], &m_points[0], &v2);
            normalizeV3f(&v2, &v2);
            
            float dot = fabsf(dotV3f(&v1, &v2));
            if (dot < bestDot) {
                bestDot = dot;
                best = i;
            }
        }
        
        assert(best != -1);
        
        m_points[2] = vertices[(best - 1 + vertexCount) % vertexCount]->position;
        m_points[0] = vertices[best]->position;
        m_points[1] = vertices[(best + 1) % vertexCount]->position;
        
        setPlanePointsV3f(&m_boundary, &m_points[0], &m_points[1], &m_points[2]);
    }
    
    TVector3f Face::normal() {
        return m_boundary.norm;
    }
    
    TPlane Face::boundary() {
        return m_boundary;
    }
    
    const TBoundingBox& Face::worldBounds() {
        return m_worldBounds;
    }
    
    const vector<Vertex*>& Face::vertices() {
        assert(m_vertices != NULL);
        return *m_vertices;
    }
    
    const vector<Edge*>& Face::edges() {
        assert(m_edges != NULL);
        return *m_edges;
    }
    
    Texture* Face::texture() {
        return m_texture;
    }
    
    void Face::setTexture(Texture* texture) {
        if (m_texture != NULL)
            m_texture->usageCount--;
        
        m_texture = texture;
        
        if (m_texture != NULL)
            m_texture->usageCount++;
    }
    
    int Face::xOffset() {
        return m_xOffset;
    }
    
    void Face::setXOffset(int xOffset) {
        m_xOffset = xOffset;
        m_valid = false;
    }
    
    int Face::yOffset() {
        return m_yOffset;
    }
    
    void Face::setYOffset(int yOffset) {
        m_yOffset = yOffset;
        m_valid = false;
    }
    
    float Face::rotation() {
        return m_rotation;
    }
    
    void Face::setRotation(float rotation) {
        m_rotation = rotation;
        m_valid = false;
    }
    
    float Face::xScale() {
        return m_xScale;
    }
    
    void Face::setXScale(float xScale) {
        m_xScale = xScale;
        m_valid = false;
    }

    float Face::yScale() {
        return m_yScale;
    }

    void Face::setYScale(float yScale) {
        m_yScale = yScale;
        m_valid = false;
    }
}