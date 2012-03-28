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

#import "GLUtils.h"

float const EdgeOffset = 0.0001f;

void glNormalV3f(const TVector3f* v) {
    glNormal3f(v->x, v->y, v->z);
}

void glVertexV3f(const TVector3f* v) {
    glVertex3f(v->x, v->y, v->z);
}

void glColorV4f(const TVector4f* v) {
    glColor4f(v->x, v->y, v->z, v->w);
}

void glColorV4fAlpha(const TVector4f* v, float alpha) {
    glColor4f(v->x, v->y, v->z, alpha);
}

void glSetEdgeOffset(float f) {
    glDepthRange(0.0, 1.0 - EdgeOffset * f);
}

void glResetEdgeOffset() {
    glDepthRange(EdgeOffset, 1.0);
}
