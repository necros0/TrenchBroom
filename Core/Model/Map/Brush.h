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

#ifndef TrenchBroom_Brush_h
#define TrenchBroom_Brush_h

#include <vector>
#include "Entity.h"
#include "Face.h"
#include "Texture.h"
#include "BrushGeometry.h"

using namespace std;

namespace TrenchBroom {
    
    class Entity;
    class Face;
    class Vertex;
    class Edge;
    class BrushGeometry;
    class MoveResult;
    
    class Brush {
    protected:
        int m_brushId;
        Entity* m_entity;
        vector<Face*> m_faces;
        
        BrushGeometry* m_geometry;
        
        bool m_onGrid;
        const TBoundingBox& m_worldBounds;
        
        int m_filePosition;
        bool m_selected;
        
        void init();
        void rebuildGeometry();
    public:
        Brush(const TBoundingBox& worldBounds);
        Brush(const TBoundingBox& worldBounds, const Brush& brushTemplate);
        Brush(const TBoundingBox& worldBounds, const TBoundingBox& brushBounds, Texture& texture);
        ~Brush();
        
        void restore(const Brush& brushTemplate);
        
        int brushId() const;
        Entity* entity() const;
        void setEntity(Entity* entity);
        const vector<Face*>& faces() const;
        const TBoundingBox& bounds() const;
        const TBoundingBox& worldBounds() const;
        TVector3f center();
        const vector<Vertex*>& vertices() const;
        const vector<Edge*>& edges() const;
        
        bool containsPoint(TVector3f point);
        bool intersectsBrush(const Brush& brush);
        bool containsBrush(const Brush& brush);
        bool intersectsEntity(const Entity& entity);
        bool containsEntity(const Entity& entity);
        
        bool addFace(Face* face);
        bool canDeleteFace(Face& face);
        void deleteFace(Face& face);
        
        void translate(TVector3f delta, bool lockTextures);
        void rotate90CW(EAxis axis, TVector3f center, bool lockTextures);
        void rotate90CCW(EAxis axis, TVector3f center, bool lockTextures);
        void rotate(TQuaternion rotation, TVector3f center, bool lockTextures);
        void flip(EAxis axis, TVector3f center, bool lockTextures);
        bool canResize(Face& face, float dist);
        void resize(Face& face, float dist, bool lockTextures);
        void enlarge(float delta, bool lockTextures);
        void snap();
        
        MoveResult moveVertex(int vertexIndex, TVector3f delta);
        MoveResult moveEdge(int edgeIndex, TVector3f delta);
        MoveResult moveFace(int faceIndex, TVector3f delta);
        
        int filePosition() const;
        void setFilePosition(int filePosition);
        bool selected() const;
        void setSelected(bool selected);
    };

}

#endif
