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

#ifndef TrenchBroom_Picker_h
#define TrenchBroom_Picker_h

#include <memory>
#include <vector>
#include "Math.h"
#include "Entity.h"
#include "Brush.h"
#include "Face.h"
#include "BrushGeometry.h"
#include "Filter.h"

namespace TrenchBroom {
    
    typedef enum {
        HT_ENTITY           = 1 << 0,
        HT_FACE             = 1 << 1,
        HT_CLOSE_FACE       = 1 << 2,
        HT_VERTEX_HANDLE    = 1 << 3,
        HT_EDGE_HANDLE      = 1 << 4,
        HT_FACE_HANDLE      = 1 << 5
    } EHitType;
    
    class PickingHit {
    public:
        void* object;
        EHitType type;
        TVector3f hitPoint;
        int index;
        float distance;
        
        PickingHit(Entity* entity, TVector3f hitPoint, float distance);
        PickingHit(Face* face, TVector3f hitPoint, float distance);
        PickingHit(Brush* brush, EHitType type, int index, TVector3f hitPoint, float distance);
        
    };
    
    class PickingHitList {
    private:
        vector<PickingHit*> m_hits;
        bool m_sorted;
    public:
        void addHit(PickingHit& hit);
        PickingHit* first(EHitType typeMask, bool ignoreOccluders);
        vector<PickingHit*> hits(EHitType typeMask);
        vector<PickingHit*> allHits();
    };
    
    class Map;
    class Picker {
    private:
        Map* m_map;
    public:
        Picker(Map* map);
        PickingHitList* pick(TRay ray, Filter* filter);
        void pickCloseFaces(TRay ray, const vector<Brush*>& brushes, float maxDistance, PickingHitList& hits, Filter* filter);
        void pickVertices(TRay ray, const vector<Brush*>& brushes, float handleRadius, PickingHitList& hits, Filter* filter);
    };
    
}

#endif
