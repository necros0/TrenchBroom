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

#include "Pak.h"
#include <cstdio>
#include <cstring>
#include <dirent.h>
#include <numeric>

namespace TrenchBroom {
    Pak::Pak(string path) {
        char magic[PAK_HEADER_MAGIC_LENGTH];
        char entryName[PAK_ENTRY_NAME_LENGTH];
        uint32_t directoryAddr, directorySize, entryCount;
        
        this->path = path;
        mStream.open(this->path.c_str());
        if (mStream.is_open()) {
            mStream.seekg(PAK_HEADER_ADDRESS, ios::beg);
            mStream.read((char *)magic, PAK_HEADER_MAGIC_LENGTH); // todo check and throw exception
            mStream.read((char *)&directoryAddr, sizeof(uint32_t));
            mStream.read((char *)&directorySize, sizeof(uint32_t));
            entryCount = directorySize / PAK_ENTRY_LENGTH;
            
            for (int i = 0; i < entryCount; i++) {
                PakEntry entry;
                
                mStream.seekg(directoryAddr + i * PAK_ENTRY_LENGTH, ios::beg);
                mStream.read(entryName, PAK_ENTRY_NAME_LENGTH);
                mStream.read((char *)entry.address, sizeof(int32_t));
                mStream.read((char *)entry.length, sizeof(int32_t));
                
                entries[entry.name] = entry;
            }
            mStream.close();
        }

    }

    char* Pak::streamForEntry(string name) {
        char* buffer;
        PakEntry* entry;
        
        map<string, PakEntry>::iterator it = entries.find(name);
        if (it == entries.end())
            return NULL;
        
        entry = &it->second;
        if (!mStream.is_open())
            mStream.open(path.c_str());

        mStream.seekg(entry->address, ios::beg);
        mStream.read(buffer, entry->length);
        return buffer;
    }
    
    char* PakManager::streamForEntry(string name, vector<string> paths) {
        vector<string>::reverse_iterator path;
        for (path = paths.rbegin(); path < paths.rend(); ++path) {
            vector<Pak*>* paks = paksAtPath(*path);
            if (paks != NULL) {
                vector<Pak*>::reverse_iterator pak;
                for (pak = paks->rbegin(); pak < paks->rend(); ++pak) {
                    char* buffer = (*pak)->streamForEntry(name);
                    if (buffer != NULL)
                        return buffer;
                }
            }
        }
        
        string nicePaths = accumulate(paths.begin(), paths.end(), string(", "));
        fprintf(stdout, "Warning: Could not find pak entry %s at pak paths %s", name.c_str(), nicePaths.c_str());
        return NULL;
    }

    PakManager::~PakManager() {
        map<string, vector<Pak*> >::iterator it;
        for (it = paks.begin(); it != paks.end(); it++)
            it->second.erase(it->second.begin(), it->second.end());
    }

    int comparePaks(const Pak* pak1, const Pak* pak2) {
        return pak1->path.compare(pak2->path);
    }
    
    vector<Pak*>* PakManager::paksAtPath(string path) {
        DIR* dir;
        struct dirent* entry;
        vector<Pak*> newPaks;
        
        map<string, vector<Pak*> >::iterator it = paks.find(path);
        if (it != paks.end())
            return &it->second;
        
        dir = opendir(path.c_str());
        if (!dir) {
            fprintf(stdout, "Warning: Could not open pak path %s", path.c_str());
            return NULL;
        }
        
        entry = readdir(dir);
        if (!entry) {
            fprintf(stdout, "Warning: %s does not contain any pak files", path.c_str());
            closedir(dir);
            return NULL;
        }
        
        do {
            if (strncmp(entry->d_name + entry->d_namlen - 5, ".pak", 4) == 0) {
                string pakPath = path;
                if (pakPath[pakPath.length() - 1] != '/')
                    pakPath += '/';
                pakPath += entry->d_name;
                
                Pak* pak = new Pak(pakPath);
                newPaks.push_back(pak);
            }
        } while (entry);
        closedir(dir);
        
        sort(newPaks.begin(), newPaks.end(), comparePaks);
        paks[path] = newPaks;
        return &paks[path];
    }
}