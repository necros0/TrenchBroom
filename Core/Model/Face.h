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

#ifndef TrenchBroom_Face_h
#define TrenchBroom_Face_h

#include <vector>
#include "Brush.h"
#include "Texture.h"
#include "Math.h"
#include "Vbo.h"
#include "BrushGeometry.h"

using namespace std;

namespace TrenchBroom {
    
    class Face {
    private:
        int m_faceId;
        Brush* m_brush;
        
        TVector3f m_points[3];
        TPlane m_boundary;
        TBoundingBox& m_worldBounds;
        
        Texture* m_texture;
        float m_xOffset;
        float m_yOffset;
        float m_rotation;
        float m_xScale;
        float m_yScale;
        
        vector<Vertex*>* m_vertices;
        vector<Edge*>* m_edges;
        
        int m_texPlaneNormIndex;
        int m_texFaceNormIndex;
        TVector3f m_texAxisX;
        TVector3f m_texAxisY;
        TVector3f m_scaledTexAxisX;
        TVector3f m_scaledTexAxisY;
        bool m_valid;
        
        int m_filePosition;
        bool m_selected;
        VboBlock* m_vboBlock;
        
        void init();
    public:
        Face(TBoundingBox& worldBounds);
        Face(TBoundingBox& worldBounds, TVector3f point1, TVector3f point2, TVector3f point3);
        Face(TBoundingBox& worldBounds, Face& faceTemplate);
        ~Face();
        
        void restore(Face& faceTemplate);
        
        int faceId();
        Brush* brush();
        void setBrush(Brush* brush);
        
        void points(TVector3f& point1, TVector3f& point2, TVector3f& point3);
        void updatePoints();
        TVector3f normal();
        TPlane boundary();
        const TBoundingBox& worldBounds();
        const vector<Vertex*>& vertices();
        const vector<Edge*>& edges();
        
        Texture* texture();
        void setTexture(Texture* texture);
        int xOffset();
        void setXOffset(int xOffset);
        int yOffset();
        void setYOffset(int yOffset);
        float rotation();
        void setRotation(float rotation);
        float xScale();
        void setXScale(float xScale);
        float yScale();
        void setYScale(float yScale);
        
        void translateOffsets(float delta, TVector3f dir);
        void rotateTexture(float angle);
        void translate(TVector3f delta, bool lockTexture);
        void rotate90CW(EAxis axis, TVector3f center, bool lockTexture);
        void rotate90CCW(EAxis axis, TVector3f center, bool lockTexture);
        void rotate(TQuaternion rotation, TVector3f center, bool lockTexture);
        void flip(EAxis axis, TVector3f center, bool lockTexture);
        void move(float dist, bool lockTexture);
        
        TVector2f textureCoords(TVector3f vertex);
        TVector2f gridCoords(TVector3f vertex);
        
        int filePosition();
        void setFilePosition(int filePosition);
        bool selected();
        void setSelected(bool selected);
        VboBlock* vboBlock();
        void setVboBlock(VboBlock* vboBlock);
    };
    
}

#endif
