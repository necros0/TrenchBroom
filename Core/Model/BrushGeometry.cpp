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

namespace TrenchBroom {
    
    Vertex::Vertex(float x, float y, float z) {
        position.x = x;
        position.y = y;
        position.z = z;
        mark = VM_NEW;
    }
    
    Edge::Edge(Vertex* start, Vertex* end) : start(start), end(end) {}

    void centerOfVertices(vector<Vertex*>& vertices, TVector3f& center) {
        center = vertices[0]->position;
        for (int i = 1; i < vertices.size(); i++)
            addV3f(&center, &vertices[0]->position, &center);
        scaleV3f(&center, 1.0f / vertices.size(), &center);
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