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

#include "Entity.h"
#include <math.h>
#include <iostream>
#include <sstream>

using namespace std;

namespace TrenchBroom {

    void Entity::init() {
        static int currentId = 1;
        m_entityDefinition = NULL;
        m_entityId = currentId++;
        m_map = NULL;
        m_filePosition = -1;
        m_selected = false;
        m_vboBlock = NULL;
        m_origin = NullVector;
        m_angle = NAN;
        rebuildGeometry();
    }

    void Entity::rebuildGeometry() {
        m_bounds.min = m_bounds.max = NullVector;
        m_maxBounds.min = m_maxBounds.max = NullVector;
        if (m_entityDefinition == NULL || m_entityDefinition->type == EDT_BRUSH) {
            if (!m_brushes.empty()) {
                m_bounds = m_brushes[0]->bounds();
                for (int i = 1; i < m_brushes.size(); i++) {
                    TBoundingBox brushBounds = m_brushes[i]->bounds();
                    mergeBoundsWithBounds(&m_bounds, &brushBounds, &m_bounds);
                }
            }
        } else if (m_entityDefinition->type == EDT_POINT) {
            m_bounds = m_entityDefinition->bounds;
            addV3f(&m_bounds.min, &m_origin, &m_bounds.min);
            addV3f(&m_bounds.max, &m_origin, &m_bounds.max);
            centerOfBounds(&m_bounds, &m_center);
        }

        centerOfBounds(&m_bounds, &m_center);
        TVector3f diff;
        subV3f(&m_bounds.max, &m_center, &diff);
        diff.x = diff.y = diff.z = lengthV3f(&diff);
        subV3f(&m_center, &diff, &m_maxBounds.min);
        addV3f(&m_center, &diff, &m_maxBounds.max);
    }
    
    void Entity::rotate90(EAxis axis, TVector3f rotationCenter, bool clockwise) {
        if (m_entityDefinition != NULL && m_entityDefinition->type != EDT_BRUSH)
            return;
        
        TVector3f temp;
        subV3f(&m_origin, &rotationCenter, &temp);
        if (clockwise) rotate90CWV3f(&temp, axis, &temp);
        else rotate90CCWV3f(&temp, axis, &temp);
        addV3f(&temp, &rotationCenter, &temp);
        setProperty(OriginKey, temp, true);
        
        TVector3f direction;
        
        if (m_angle >= 0) {
            direction.x = cos(2 * M_PI - m_angle * M_PI / 180);
            direction.y = sin(2 * M_PI - m_angle * M_PI / 180);
            direction.z = 0;
        } else if (m_angle == -1) {
            direction = ZAxisPos;
        } else if (m_angle == -2) {
            direction = ZAxisNeg;
        } else {
            return;
        }
        
        if (clockwise) rotate90CWV3f(&direction, axis, &direction);
        else rotate90CCWV3f(&direction, axis, &direction);
        if (direction.z > 0.9) {
            setProperty(AngleKey, -1, true);
        } else if (direction.z < -0.9) {
            setProperty(AngleKey, -2, true);
        } else {
            if (direction.z != 0) {
                direction.z = 0;
                normalizeV3f(&direction, &direction);
            }
            
            m_angle = roundf(acos(direction.x) * 180 / M_PI);
            crossV3f(&direction, &XAxisPos, &temp);
            if (!nullV3f(&temp) && temp.z < 0)
                m_angle = 360 - m_angle;
            setProperty(AngleKey, m_angle, true);
        }
    }

    int Entity::entityId() const {
        return m_entityId;
    }
    
    const EntityDefinition* Entity::entityDefinition() const {
        return m_entityDefinition;
    }
    
    void Entity::setEntityDefinition(EntityDefinition* entityDefinition) {
        if (m_entityDefinition != NULL)
            m_entityDefinition->usageCount--;
        m_entityDefinition = entityDefinition;
        if (m_entityDefinition != NULL)
            m_entityDefinition->usageCount++;
        rebuildGeometry();
    }
    
    const TBoundingBox& Entity::bounds() const {
        return m_bounds;
    }
    
    const TBoundingBox& Entity::maxBounds() const {
        return m_maxBounds;
    }

    Map* Entity::quakeMap() const {
        return m_map;
    }
    
    void Entity::setMap(Map* quakeMap) {
        m_map = quakeMap;
    }
    
    const vector<Brush*>& Entity::brushes() const {
        return m_brushes;
    }

    const map<string, string> Entity::properties() const {
        return m_properties;
    }
    
    const string* Entity::propertyForKey(const string& key) const {
        map<string, string>::const_iterator it = m_properties.find(key);
        if (it == m_properties.end())
            return NULL;
        return &it->second;
        
    }
    
    bool Entity::propertyWritable(const string& key) const {
        return ClassnameKey != key;
    }
    
    bool Entity::propertyDeletable(const string& key) const {
        if (ClassnameKey == key)
            return false;
        if (OriginKey == key)
            return false;
        if (SpawnFlagsKey == key)
            return false;
        return true;
    }
    
    void Entity::setProperty(const string& key, const string& value) {
        setProperty(key, &value);
    }
    
    void Entity::setProperty(const string& key, const string* value) {
        if (key == ClassnameKey) {
            fprintf(stdout, "Warning: Cannot overwrite classname property");
            return;
        } else if (key == OriginKey) {
            if (value == NULL) {
                fprintf(stdout, "Warning: Cannot set origin to NULL");
                return;
            }
            if (!parseV3f(key.c_str(), 0, value->length(), &m_origin)) {
                fprintf(stdout, "Warning: Cannot parse origin value %s", value->c_str());
                return;
            }
            roundV3f(&m_origin, &m_origin);
        } else if (key == AngleKey) {
            if (value != NULL) m_angle = strtof(value->c_str(), NULL);
            else m_angle = NAN;
        }
        
        const string* oldValue = propertyForKey(key);
        if (oldValue != NULL && oldValue == value) return;
        m_properties[key] = *value;
        rebuildGeometry();
    }
    
    void Entity::setProperty(const string& key, TVector3f value, bool round) {
        stringstream valueStr;
        if (round) valueStr << (int)roundf(value.x) << " " << (int)roundf(value.y) << " " << (int)roundf(value.z);
        else valueStr << value.x << " " << value.y << " " << value.z;
        setProperty(key, valueStr.str());
    }

    void Entity::setProperty(const string& key, float value, bool round) {
        stringstream valueStr;
        if (round) valueStr << (int)roundf(value);
        else valueStr << value;
        setProperty(key, valueStr.str());
    }

    void setProperties(map<string, string> properties, bool replace) {
    }

    void Entity::deleteProperty(const string& key) {
        if (!propertyDeletable(key)) {
            fprintf(stdout, "Warning: Cannot delete property '%s'", key.c_str());
            return;
        }
        
        if (key == AngleKey) m_angle = NAN;
        if (m_properties.count(key) == 0) return;
        m_properties.erase(key);
        rebuildGeometry();
    }
    
    const string* Entity::classname() const {
        return propertyForKey(ClassnameKey);
    }
    
    const int Entity::angle() const {
        return roundf(m_angle);
    }
    
    bool Entity::worldSpawn() const {
        return *classname() == WorldspawnClassname;
    }
    
    bool Entity::group() const {
        return *classname() == GroupClassname;
    }
    
    void Entity::addBrush(Brush* brush) {
        if (m_entityDefinition != NULL && m_entityDefinition->type != EDT_BRUSH)
            return;
        
        brush->setEntity(this);
        m_brushes.push_back(brush);
        rebuildGeometry();
    }
    
    void Entity::addBrushes(vector<Brush*>& brushes) {
        if (m_entityDefinition != NULL && m_entityDefinition->type != EDT_BRUSH)
            return;

        for (int i = 0; i < brushes.size(); i++) {
            brushes[i]->setEntity(this);
            m_brushes.push_back(brushes[i]);
        }
        rebuildGeometry();
    }

    void Entity::brushChanged(Brush* brush) {
        rebuildGeometry();
    }
    
    void Entity::deleteBrush(Brush* brush) {
        if (m_entityDefinition != NULL && m_entityDefinition->type != EDT_BRUSH)
            return;
        
        brush->setEntity(NULL);
        m_brushes.erase(find(m_brushes.begin(), m_brushes.end(), brush));
        rebuildGeometry();
    }

    void Entity::deleteBrushes(vector<Brush*>& brushes) {
        if (m_entityDefinition != NULL && m_entityDefinition->type != EDT_BRUSH)
            return;
        
        for (int i = 0; i < brushes.size(); i++) {
            brushes[i]->setEntity(NULL);
            m_brushes.erase(find(m_brushes.begin(), m_brushes.end(), brushes[i]));
        }
        rebuildGeometry();
    }

    void Entity::translate(TVector3f delta) {
        if (m_entityDefinition != NULL && m_entityDefinition->type != EDT_POINT)
            return;

        TVector3f temp;
        addV3f(&m_origin, &delta, &temp);
        setProperty(OriginKey, temp, true);
    }
    
    void Entity::rotate90CW(EAxis axis, TVector3f rotationCenter) {
        rotate90(axis, rotationCenter, true);
    }
    
    void Entity::rotate90CCW(EAxis axis, TVector3f rotationCenter) {
        rotate90(axis, rotationCenter, false);
    }
    
    void Entity::rotate(TQuaternion rotation, TVector3f rotationCenter) {
        if (m_entityDefinition != NULL && m_entityDefinition->type != EDT_BRUSH)
            return;
        
        TVector3f direction, temp, offset;
        subV3f(&m_center, &m_origin, &offset);
        subV3f(&m_center, &rotationCenter, &m_center);
        rotateQ(&rotation, &m_center, &m_center);
        addV3f(&m_center, &rotationCenter, &m_center);
        subV3f(&m_center, &offset, &temp);
        roundV3f(&temp, &temp);
        setProperty(OriginKey, temp, true);
        setProperty(AngleKey, 0, true);
        
        if (m_angle >= 0) {
            direction.x = cos(2 * M_PI - m_angle * M_PI / 180);
            direction.y = sin(2 * M_PI - m_angle * M_PI / 180);
            direction.z = 0;
        } else if (m_angle == -1) {
            direction = ZAxisPos;
        } else if (m_angle == -2) {
            direction = ZAxisNeg;
        } else {
            return;
        }
        
        rotateQ(&rotation, &direction, &direction);
        if (direction.z > 0.9) {
            setProperty(AngleKey, -1, true);
        } else if (direction.z < -0.9) {
            setProperty(AngleKey, -2, true);
        } else {
            if (direction.z != 0) {
                direction.z = 0;
                normalizeV3f(&direction, &direction);
            }
            
            m_angle = roundf(acos(direction.x) * 180 / M_PI);
            crossV3f(&direction, &XAxisPos, &temp);
            if (!nullV3f(&temp) && temp.z < 0)
                m_angle = 360 - m_angle;
            setProperty(AngleKey, m_angle, true);
        }
    }
    
    void Entity::flip(EAxis axis, TVector3f flipCenter) {
        if (m_entityDefinition != NULL && m_entityDefinition->type != EDT_BRUSH)
            return;
        
        TVector3f temp, offset;
        subV3f(&m_center, &m_origin, &offset);
        subV3f(&m_center, &flipCenter, &m_center);
        
        switch (axis) {
            case A_X:
                m_center.x *= -1;
                break;
            case A_Y:
                m_center.y *= -1;
                break;
            default:
                m_center.z *= -1;
                break;
        }
        
        addV3f(&m_center, &flipCenter, &m_center);
        addV3f(&m_center, &offset, &temp);
        setProperty(OriginKey, temp, true);
        setProperty(AngleKey, 0, true);

        if (m_angle >= 0)
            m_angle = (m_angle + 180) - (int)((m_angle / 360)) * m_angle;
        else if (m_angle == -1)
            m_angle = -2;
        else if (m_angle == -2)
            m_angle = -1;
        setProperty(AngleKey, m_angle, true);
    }

    int Entity::filePosition() const {
        return m_filePosition;
    }
    
    void Entity::setFilePosition(int filePosition) {
        m_filePosition = filePosition;
    }
    
    bool Entity::selected() const {
        return m_selected;
    }
    
    void Entity::setSelected(bool selected) {
        m_selected = selected;
    }
    
    VboBlock* Entity::vboBlock() const {
        return m_vboBlock;
    }
    
    void Entity::setVboBlock(VboBlock* vboBlock) {
        if (m_vboBlock != NULL)
            freeVboBlock(m_vboBlock);
        m_vboBlock = vboBlock;
    }
}
