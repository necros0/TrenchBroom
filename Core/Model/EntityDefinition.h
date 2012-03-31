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
#include <vector>
#include <map>
#include "Math.h"

using namespace std;

namespace TrenchBroom {
    
    typedef enum {
        EDT_POINT,
        EDT_BRUSH,
        EDT_BASE
    } EEntityDefinitionType;
    
    typedef enum {
        EDP_CHOICE,
        EDP_MODEL,
        EDP_DEFAULT,
        EDP_BASE
    } EPropertyType;

    class Property {
    public:
        EPropertyType type;
        Property(EPropertyType type) : type(type) {};
    };
    
    class BaseProperty : public Property {
    public:
        string baseName;
        BaseProperty(string& baseName) : Property(EDP_BASE), baseName(baseName) {};
    };
    
    class DefaultProperty : public Property {
    public:
        string name;
        string value;
        DefaultProperty(string& name, string& value) : Property(EDP_DEFAULT), name(name), value(value) {};
    };
    
    class ModelProperty : public Property {
    public:
        string flagName;
        string modelPath;
        int skinIndex;
        ModelProperty(string& flagName, string& modelPath, int skinIndex) : Property(EDP_MODEL), flagName(flagName), modelPath(modelPath), skinIndex(skinIndex) {};
        ModelProperty(string& modelPath, int skinIndex) : Property(EDP_MODEL), flagName(""), modelPath(modelPath), skinIndex(skinIndex) {};
    };
    
    class ChoiceArgument {
    public:
        int key;
        string value;
        ChoiceArgument(int key, string& value) : key(key), value(value) {};
    };
    
    class ChoiceProperty : public Property {
    public:
        string name;
        vector<ChoiceArgument> arguments;
        ChoiceProperty(string& name, vector<ChoiceArgument>& arguments) : Property(EDP_CHOICE), name(name), arguments(arguments) {};
    };
    
    class SpawnFlag {
    public:
        string name;
        int flag;
        SpawnFlag() {};
        SpawnFlag(string& name, int flag) : name(name), flag(flag) {};
    };
    
    class EntityDefinition {
    public:
        static EntityDefinition* baseDefinition(string& name, map<string, SpawnFlag>& flags, vector<Property*>& properties);
        static EntityDefinition* pointDefinition(string& name, TVector4f& color, TBoundingBox& bounds, map<string, SpawnFlag>& flags, vector<Property*>& properties, string& description);
        static EntityDefinition* brushDefinition(string& name, TVector4f& color, map<string, SpawnFlag>& flags, vector<Property*> properties, string& description);
        ~EntityDefinition();
        EEntityDefinitionType type;
        string name;
        TVector4f color;
        TVector3f center;
        TBoundingBox bounds;
        TBoundingBox maxBounds;
        map<string, SpawnFlag> flags;
        vector<Property*> properties;
        string description;
        int usageCount;
    };
}

#endif
