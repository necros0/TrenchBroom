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

#ifndef TrenchBroom_EntityDefinition_h
#define TrenchBroom_EntityDefinition_h

#include <string>
#include <map>
#include "Math.h"

using namespace std;

namespace TrenchBroom {
    
    typedef enum {
        EDT_POINT,
        EDT_BRUSH,
        EDT_BASE
    } EEntityDefinitionType;
    
    class EntityDefinition {
    public:
        EEntityDefinitionType type;
        string name;
        TVector4f color;
        TVector3f center;
        TBoundingBox bounds;
        TBoundingBox maxBounds;
        map<string, string> flags;
        vector<string> properties;
        string description;
        int usageCount;
    };
}

#endif
