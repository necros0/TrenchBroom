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
    
    class Brush;
    class Side;
    class Face {
    protected:
        int m_faceId;
        Brush* m_brush;
        
        TVector3f m_points[3];
        TPlane m_boundary;
        const TBoundingBox& m_worldBounds;
        
        Texture* m_texture;
        float m_xOffset;
        float m_yOffset;
        float m_rotation;
        float m_xScale;
        float m_yScale;

        Side* m_side;
        
        int m_texPlaneNormIndex;
        int m_texFaceNormIndex;
        TVector3f m_texAxisX;
        TVector3f m_texAxisY;
        TVector3f m_scaledTexAxisX;
        TVector3f m_scaledTexAxisY;
        bool m_texAxesValid;
        
        int m_filePosition;
        bool m_selected;
        VboBlock* m_vboBlock;
        
        void init();
        void texAxesAndIndices(const TVector3f& faceNormal, TVector3f& xAxis, TVector3f& yAxis, int& planeNormIndex, int& faceNormIndex) const;
        void validateTexAxes(const TVector3f& faceNormal);
        void compensateTransformation(const TMatrix4f& transformation);
    public:
        Face(const TBoundingBox& worldBounds);
        Face(const TBoundingBox& worldBounds, TVector3f point1, TVector3f point2, TVector3f point3);
        Face(const TBoundingBox& worldBounds, const Face& faceTemplate);
        ~Face();
        
        void restore(const Face& faceTemplate);
        
        int faceId() const;
        Brush* brush() const;
        void setBrush(Brush* brush);
        void setSide(Side* side);
        
        void points(TVector3f& point1, TVector3f& point2, TVector3f& point3) const;
        void updatePoints();
        TVector3f normal() const;
        TPlane boundary() const;
        TVector3f center() const;
        const TBoundingBox& worldBounds() const;
        const vector<Vertex*>& vertices() const;
        const vector<Edge*>& edges() const;
        
        Texture* texture() const;
        void setTexture(Texture* texture);
        int xOffset() const;
        void setXOffset(int xOffset);
        int yOffset() const;
        void setYOffset(int yOffset);
        float rotation() const;
        void setRotation(float rotation);
        float xScale() const;
        void setXScale(float xScale);
        float yScale() const;
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
        
        int filePosition() const;
        void setFilePosition(int filePosition);
        bool selected() const;
        void setSelected(bool selected);
        VboBlock* vboBlock() const;
        void setVboBlock(VboBlock* vboBlock);
    };
    
}

#endif
