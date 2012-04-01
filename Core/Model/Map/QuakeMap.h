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

#ifndef TrenchBroom_QuakeMap_h
#define TrenchBroom_QuakeMap_h

#include <vector>
#include "Observer.h"
#include "Selection.h"
#include "Groups.h"
#include "EntityDefinition.h"
#include "Entity.h"
#include "Brush.h"
#include "Face.h"

namespace TrenchBroom {
    
    static const string FacesWillChange = "FacesWillChange";
    static const string FacesDidChange = "FacesDidChange";

    static const string BrushesAdded = "BrushesAdded";
    static const string BrushesWillBeRemoved = "BrushesWillBeRemoved";
    static const string BrushesWillChange = "BrushesWillChange";
    static const string BrushesDidChange = "BrushesDidChange";
    
    static const string EntitiesAdded = "EntitiesAdded";
    static const string EntitiesWillBeRemoved = "EntitiesWillBeRemoved";
    static const string PropertiesWillChange = "PropertiesWillChange";
    static const string PropertiesDidChange = "PropertiesDidChange";

    static const string PointFileLoaded = "PointFileLoaded";
    static const string PointFileUnloaded = "PointFileUnloaded";

    static const string MapCleared = "MapCleared";
    static const string MapLoaded = "MapLoaded";
    
    class Selection;
    class QuakeMap : public Observable {
    private:
        Selection* m_selection;
        EntityDefinitionManager* m_entityDefinitionManager;
        GroupManager* m_groupManager;
        
        vector<Entity*> m_entities;
        Entity* m_worldspawn;
        TBoundingBox m_worldBounds;
        
        vector<TVector3f> m_leakPoints;
    public:
        QuakeMap(const string& entityDefinitionFilePath);
        ~QuakeMap();
        void clear();
        
# pragma mark Point File Support
        void loadPointFile(const string& path);
        void unloadPointFile();
        const vector<TVector3f>& leakPoints() const;
        
# pragma mark Entity related functions
        vector<Entity*>& entities();
        Entity* worldspawn(bool create);
        Entity* createEntity(const string& classname);
        Entity* createEntity(const map<string, string> properties);
        void setEntityDefinition(Entity* entity);
        void setEntityProperty(const string& key, const string* value);

# pragma mark Brush related functions
        void addBrushesToEntity(Entity& entity);
        void moveBrushesToEntity(Entity& entity);
        Brush* createBrush(Entity& entity, const Brush& brushTemplate);
        Brush* createBrush(Entity& entity, TBoundingBox bounds, Texture& texture);
        void snapBrushes();
        bool resizeBrushes(vector<Face*>& faces, float delta, bool lockTextures);
        
# pragma mark Common functions
        void duplicateObjects(vector<Entity*>& newEntities, vector<Brush*>& newBrushes);
        void translateObjects(TVector3f delta, bool lockTextures);
        void rotateObjects90CW(EAxis axis, TVector3f center, bool lockTextures);
        void rotateObjects90CCW(EAxis axis, TVector3f center, bool lockTextures);
        void rotateObjects(TQuaternion rotation, TVector3f center, bool lockTextures);
        void flipObjects(EAxis axis, TVector3f center, bool lockTextures);
        void deleteObjects();
        
# pragma mark Face related functoins
        void setXOffset(int xOffset);
        void setYOffset(int yOffset);
        void translateFaces(float delta, TVector3f dir);
        void setRotation(float rotation);
        void rotateFaces(float angle);
        void setXScale(float xScale);
        void setYScale(float yScale);
        void deleteFaces();
        
# pragma mark Vertex related functions
        void moveVertex(Brush& brush, int theVertexIndex, TVector3f delta);
        void moveEdge(Brush& brush, int theEdgeIndex, TVector3f delta);
        void moveFace(Brush& brush, int theFaceIndex, TVector3f delta);
        
# pragma mark getters
        Selection& selection();
        EntityDefinitionManager& entityDefinitionManager();
        GroupManager& groupManager();
    };

}

#endif
