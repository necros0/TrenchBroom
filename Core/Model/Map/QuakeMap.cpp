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

#include "QuakeMap.h"
#include <algorithm>
#include <fstream>
#include <assert.h>
#include "Utils.h"

namespace TrenchBroom {
    QuakeMap::QuakeMap(const string& entityDefinitionFilePath) : Observable() {
        m_worldBounds.min.x = -0x1000;
        m_worldBounds.min.y = -0x1000;
        m_worldBounds.min.z = -0x1000;
        m_worldBounds.max.x = +0x1000;
        m_worldBounds.max.y = +0x1000;
        m_worldBounds.max.z = +0x1000;
        
        m_selection = new Selection();
        m_entityDefinitionManager = new EntityDefinitionManager(entityDefinitionFilePath);
        m_groupManager = new GroupManager(*this);
    }
    
    QuakeMap::~QuakeMap() {
        setPostNotifications(false);
        clear();
        delete m_selection;
        delete m_entityDefinitionManager;
        delete m_groupManager;
    }

    void QuakeMap::clear() {
        m_selection->removeAll();
        unloadPointFile();
        while(!m_entities.empty()) delete m_entities.back(), m_entities.pop_back();
        m_worldspawn = NULL;
        postNotification(MapCleared, NULL);
    }
    
    # pragma mark Point File Support

    void QuakeMap::loadPointFile(const string& path) {
        if (!m_leakPoints.empty()) unloadPointFile();
        
        string line;
        ifstream stream(path.c_str());
        assert(stream.is_open());
        
        while (!stream.eof()) {
            TVector3f point;
            getline(stream, line);
            line = trim(line);
            if (line.length() > 0) {
                assert(parseV3f(line.c_str(), 0, line.length(), &point));
                m_leakPoints.push_back(point);
            }
        }
        
        postNotification(PointFileLoaded, NULL);
    }
    
    void QuakeMap::unloadPointFile() {
        m_leakPoints.clear();
        postNotification(PointFileUnloaded, NULL);
        
    }
    
    const vector<TVector3f>& QuakeMap::leakPoints() const {
        return m_leakPoints;
    }

    # pragma mark Entity related functions
    vector<Entity*>& QuakeMap::entities() {
        return m_entities;
    }
    
    Entity* QuakeMap::worldspawn(bool create) {
        if (m_worldspawn != NULL)
            return m_worldspawn;
        for (int i = 0; i < m_entities.size(); i++) {
            Entity* entity = m_entities[i];
            if (entity->worldspawn()) {
                m_worldspawn = entity;
                return m_worldspawn;
            }
        }
        
        if (create)
            m_worldspawn = createEntity(WorldspawnClassname);
        return m_worldspawn;
    }

    Entity* QuakeMap::createEntity(const string& classname) {
        EntityDefinition* entityDefinition = m_entityDefinitionManager->definition(classname);
        if (entityDefinition == NULL) {
            fprintf(stdout, "Warning: No entity definition found for class name '%s'", classname.c_str());
            return NULL;
        }
        
        Entity* entity = new Entity();
        entity->setProperty(ClassnameKey, classname);
        entity->setEntityDefinition(entityDefinition);
        m_entities.push_back(entity);
        return entity;
    }
    
    Entity* QuakeMap::createEntity(const map<string, string> properties) {
        map<string, string>::const_iterator it = properties.find(ClassnameKey);
        assert(it != properties.end());

        string classname = it->second;
        EntityDefinition* entityDefinition = m_entityDefinitionManager->definition(classname);
        if (entityDefinition == NULL) {
            fprintf(stdout, "Warning: No entity definition found for class name '%s'", classname.c_str());
            return NULL;
        }
        
        Entity* entity = new Entity(properties);
        entity->setEntityDefinition(entityDefinition);
        m_entities.push_back(entity);
        return entity;
    }
    
    void QuakeMap::setEntityDefinition(Entity* entity) {
        const string* classname = entity->classname();
        if (classname != NULL) {
            EntityDefinition* entityDefinition = m_entityDefinitionManager->definition(*classname);
            if (entityDefinition != NULL)
                entity->setEntityDefinition(entityDefinition);
            else
                fprintf(stdout, "Warning: No entity definition found for class name '%s'", classname->c_str());
        } else {
            fprintf(stdout, "Warning: Entity with id %i is missing classname property (line %i)", entity->entityId(), entity->filePosition());
        }
    }
    
    void QuakeMap::setEntityProperty(const string& key, const string* value) {
        const vector<Entity*>& entities = m_selection->entities();
        if (entities.empty()) return;
        
        vector<Entity*> changedEntities;
        for (int i = 0; i < entities.size(); i++) {
            Entity* entity = entities[i];
            const string* oldValue = entity->propertyForKey(key);
            if (oldValue != value) changedEntities.push_back(entity);
        }
        
        if (!changedEntities.empty()) {
            postNotification(PropertiesWillChange, &changedEntities);
            for (int i = 0; i < changedEntities.size(); i++) {
                if (value == NULL) entities[i]->deleteProperty(key);
                else entities[i]->setProperty(key, value);
            }
            postNotification(PropertiesDidChange, &changedEntities);
        }
    }

    # pragma mark Brush related functions

    void QuakeMap::addBrushesToEntity(Entity& entity) {
        const vector<Brush*>& brushes = m_selection->brushes();
        if (brushes.empty()) return;

        entity.addBrushes(brushes);
        postNotification(BrushesAdded, &brushes);
    }
    
    void QuakeMap::moveBrushesToEntity(Entity& entity) {
        const vector<Brush*> brushes = m_selection->brushes();
        if (brushes.empty()) return;

        postNotification(BrushesWillChange, &brushes);
        entity.addBrushes(brushes);
        postNotification(BrushesDidChange, &brushes);
    }
    
    Brush* QuakeMap::createBrush(Entity& entity, const Brush& brushTemplate) {
        TBoundingBox templateBounds = brushTemplate.bounds();
        if (!boundsContainBounds(&m_worldBounds, &templateBounds)) return NULL;
        
        Brush* brush = new Brush(m_worldBounds, brushTemplate);
        m_selection->removeAll();
        m_selection->addBrush(*brush);
        addBrushesToEntity(entity);
        return brush;
    }
    
    Brush* QuakeMap::createBrush(Entity& entity, TBoundingBox bounds, Texture& texture) {
        if (!boundsContainBounds(&m_worldBounds, &bounds)) return NULL;
        
        Brush* brush = new Brush(m_worldBounds, bounds, texture);
        m_selection->removeAll();
        m_selection->addBrush(*brush);
        addBrushesToEntity(entity);
        return brush;
    }
    
    void QuakeMap::snapBrushes() {
        const vector<Brush*>& brushes = m_selection->brushes();
        if (brushes.empty()) return;
        
        postNotification(BrushesWillChange, &brushes);
        for (int i = 0; i < brushes.size(); i++)
            brushes[i]->snap();
        postNotification(BrushesDidChange, &brushes);
    }
    
    bool QuakeMap::resizeBrushes(vector<Face*>& faces, float delta, bool lockTextures) {
        if (faces.empty()) return false;
        if (delta == 0) return false;
        
        bool drag = true;
        vector<Brush*> changedBrushes;
        for (int i = 0; i < faces.size() && drag; i++) {
            Face* face = faces[i];
            Brush* brush = face->brush();
            drag &= brush->selected() && brush->canResize(*face, delta);
            changedBrushes.push_back(brush);
        }
        
        if (drag) {
            postNotification(BrushesWillChange, &changedBrushes);
            for (int i = 0; i < faces.size(); i++) {
                Face* face = faces[i];
                Brush* brush = face->brush();
                brush->resize(*face, delta, lockTextures);
            }
            postNotification(BrushesDidChange, &changedBrushes);
        }
        
        return drag;
    }

    # pragma mark Common functions
    
    void QuakeMap::duplicateObjects(vector<Entity*>& newEntities, vector<Brush*>& newBrushes) {
        const vector<Entity*>& entities = m_selection->entities();
        const vector<Brush*>& brushes = m_selection->brushes();

        if (!entities.empty()) {
            for (int i = 0; i < entities.size(); i++) {
                Entity* entity = entities[i];
                Entity* newEntity = new Entity(entity->properties());
                
                EntityDefinition* entityDefinition = m_entityDefinitionManager->definition(*newEntity->classname());
                assert(entityDefinition != NULL);
                newEntity->setEntityDefinition(entityDefinition);
                
                newEntities.push_back(newEntity);
                m_entities.push_back(newEntity);
                
                for (int i = 0; i < entity->brushes().size(); i++) {
                    Brush* newBrush = new Brush(m_worldBounds, *entity->brushes()[i]);
                    newBrushes.push_back(newBrush);
                    newEntity->addBrush(newBrush);
                }
            }
        }
        
        if (!brushes.empty()) {
            for (int i = 0; i < brushes.size(); i++) {
                Brush* newBrush = new Brush(m_worldBounds, *brushes[i]);
                newBrushes.push_back(newBrush);
                worldspawn(true)->addBrush(newBrush);
            }
        }
        
        if (!newEntities.empty())
            postNotification(EntitiesAdded, &newEntities);
        if (!newBrushes.empty())
            postNotification(BrushesAdded, &newBrushes);
    }
    
    void QuakeMap::translateObjects(TVector3f delta, bool lockTextures) {
        const vector<Entity*>& entities = m_selection->entities();
        const vector<Brush*>& brushes = m_selection->brushes();
        
        if (!entities.empty()) {
            postNotification(PropertiesWillChange, &entities);
            for (int i = 0; i < entities.size(); i++)
                entities[i]->translate(delta);
            postNotification(PropertiesDidChange, &entities);
        }
        
        if (!brushes.empty()) {
            postNotification(BrushesWillChange, &brushes);
            for (int i = 0; i < brushes.size(); i++)
                brushes[i]->translate(delta, lockTextures);
            postNotification(BrushesDidChange, &brushes);
        }
    }
    
    void QuakeMap::rotateObjects90CW(EAxis axis, TVector3f center, bool lockTextures) {
        const vector<Entity*>& entities = m_selection->entities();
        const vector<Brush*>& brushes = m_selection->brushes();
        
        if (!entities.empty()) {
            postNotification(PropertiesWillChange, &entities);
            for (int i = 0; i < entities.size(); i++)
                entities[i]->rotate90CW(axis, center);
            postNotification(PropertiesDidChange, &entities);
        }
        
        if (!brushes.empty()) {
            postNotification(BrushesWillChange, &brushes);
            for (int i = 0; i < brushes.size(); i++)
                brushes[i]->rotate90CW(axis, center, lockTextures);
            postNotification(BrushesDidChange, &brushes);
        }
    }
    
    void QuakeMap::rotateObjects90CCW(EAxis axis, TVector3f center, bool lockTextures) {
        const vector<Entity*>& entities = m_selection->entities();
        const vector<Brush*>& brushes = m_selection->brushes();
        
        if (!entities.empty()) {
            postNotification(PropertiesWillChange, &entities);
            for (int i = 0; i < entities.size(); i++)
                entities[i]->rotate90CCW(axis, center);
            postNotification(PropertiesDidChange, &entities);
        }
        
        if (!brushes.empty()) {
            postNotification(BrushesWillChange, &brushes);
            for (int i = 0; i < brushes.size(); i++)
                brushes[i]->rotate90CCW(axis, center, lockTextures);
            postNotification(BrushesDidChange, &brushes);
        }
    }
    
    void QuakeMap::rotateObjects(TQuaternion rotation, TVector3f center, bool lockTextures) {
        const vector<Entity*>& entities = m_selection->entities();
        const vector<Brush*>& brushes = m_selection->brushes();
        
        if (!entities.empty()) {
            postNotification(PropertiesWillChange, &entities);
            for (int i = 0; i < entities.size(); i++)
                entities[i]->rotate(rotation, center);
            postNotification(PropertiesDidChange, &entities);
        }
        
        if (!brushes.empty()) {
            postNotification(BrushesWillChange, &brushes);
            for (int i = 0; i < brushes.size(); i++)
                brushes[i]->rotate(rotation, center, lockTextures);
            postNotification(BrushesDidChange, &brushes);
        }
    }
    
    void QuakeMap::flipObjects(EAxis axis, TVector3f center, bool lockTextures) {
        const vector<Entity*>& entities = m_selection->entities();
        const vector<Brush*>& brushes = m_selection->brushes();
        
        if (!entities.empty()) {
            postNotification(PropertiesWillChange, &entities);
            for (int i = 0; i < entities.size(); i++)
                entities[i]->flip(axis, center);
            postNotification(PropertiesDidChange, &entities);
        }
        
        if (!brushes.empty()) {
            postNotification(BrushesWillChange, &brushes);
            for (int i = 0; i < brushes.size(); i++)
                brushes[i]->flip(axis, center, lockTextures);
            postNotification(BrushesDidChange, &brushes);
        }
    }
    
    void QuakeMap::deleteObjects() {
        const vector<Entity*>& entities = m_selection->entities();
        const vector<Brush*>& brushes = m_selection->brushes();
        
        vector<Entity*> removedEntities;
        if (!brushes.empty()) {
            vector<Brush*> removedBrushes = brushes;
            postNotification(BrushesWillBeRemoved, &removedBrushes);
            m_selection->removeBrushes(removedBrushes);
            for (int i = 0; i < removedBrushes.size(); i++) {
                Brush* brush = removedBrushes[i];
                Entity* entity = brush->entity();
                entity->removeBrush(brush);
                delete brush;
                
                if (entity->brushes().empty() && !entity->worldspawn())
                    removedEntities.push_back(entity);
            }
        }

        if (!removedEntities.empty() || !entities.empty()) {
            for (int i = 0; i < entities.size(); i++) {
                Entity* entity = entities[i];
                if (!entity->worldspawn()) {
                    worldspawn(true)->addBrushes(entity->brushes());
                    if (find(removedEntities.begin(), removedEntities.end(), entity) == removedEntities.end())
                        removedEntities.push_back(entity);
                }
            }
            
            postNotification(EntitiesWillBeRemoved, &removedEntities);
            m_selection->removeEntities(removedEntities);
            for (int i = 0; i < removedEntities.size(); i++) {
                remove(m_entities.begin(), m_entities.end(), removedEntities[i]);
                delete removedEntities[i];
            }
        }
    }
}