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

#include "EntityDefinition.h"

namespace TrenchBroom {
    EntityDefinition* EntityDefinition::baseDefinition(string& name, map<string, SpawnFlag>& flags, vector<Property*>& properties) {
        EntityDefinition* definition = new EntityDefinition();
        definition->type = EDT_BASE;
        definition->name = name;
        definition->flags = flags;
        definition->properties = properties;
        return definition;
    }
    
    EntityDefinition* EntityDefinition::pointDefinition(string& name, TVector4f& color, TBoundingBox& bounds, map<string, SpawnFlag>& flags, vector<Property*>& properties, string& description) {
        EntityDefinition* definition = new EntityDefinition();
        definition->type = EDT_POINT;
        definition->name = name;
        definition->color = color;
        definition->bounds = bounds;
        definition->flags = flags;
        definition->properties = properties;
        definition->description = description;
        return definition;
    }
    
    EntityDefinition* EntityDefinition::brushDefinition(string& name, TVector4f& color, map<string, SpawnFlag>& flags, vector<Property*> properties, string& description) {
        EntityDefinition* definition = new EntityDefinition();
        definition->type = EDT_BRUSH;
        definition->name = name;
        definition->color = color;
        definition->flags = flags;
        definition->properties = properties;
        definition->description = description;
        return definition;
    }

    EntityDefinition::~EntityDefinition() {
        while(!properties.empty()) delete properties.back(), properties.pop_back();
    }
}