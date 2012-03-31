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

#ifndef TrenchBroom_EntityDefinitionManager_h
#define TrenchBroom_EntityDefinitionManager_h

#include <vector>
#include <map>
#include "EntityDefinition.h"

using namespace std;

namespace TrenchBroom {
    
    typedef enum {
        ES_NAME,
        ES_USAGE
    } EEntityDefinitionSortCriterion;

    class EntityDefinitionManager {
    private:
        map<const string, const EntityDefinition*> m_definitions;
        vector<const EntityDefinition*> m_definitionsByName;
    public:
        EntityDefinitionManager(const string& path);
        ~EntityDefinitionManager();
        const EntityDefinition* definition(const string& name) const;
        const vector<const EntityDefinition*> definitions() const;
        const vector<const EntityDefinition*> definitions(EEntityDefinitionType type) const;
        const vector<const EntityDefinition*>definitions(EEntityDefinitionType type, EEntityDefinitionSortCriterion criterion) const;
    };
    
}

#endif
