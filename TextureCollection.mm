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

#import "TextureCollection.h"
#import <OpenGL/gl.h>
#import "Texture.h"
#import "Wad.h"

using namespace TrenchBroom;

@implementation TextureCollection

- (id)initName:(NSString *)theName palette:(NSData *)thePalette wad:(void *)theWadDirectory {
    NSAssert(theName != nil, @"name must not be nil");
    NSAssert(thePalette != nil, @"palette must not be nil");
    NSAssert(theWadDirectory != NULL, @"wad directory must not be nil");
    
    if ((self = [self init])) {
        name = [theName retain];
        textures = [[NSMutableArray alloc] init];
        
        Wad* wad = (Wad *)theWadDirectory;
        vector<WadEntry>::iterator entry;
        for (entry = wad->entries.begin(); entry != wad->entries.end(); entry++) {
            if (entry->type == WT_MIP) {
                Mip* mip = wad->loadMipAtEntry(*entry);
                Texture* texture = [[Texture alloc] initWithWadEntry:mip palette:thePalette];
                [textures addObject:texture];
                [texture release];
                delete mip;
            }
        }
    }
    
    return self;
}

- (NSString *)name {
    return name;
}

- (NSArray *)textures {
    return textures;
}

- (void)dealloc {
    [name release];
    [textures release];
    [super dealloc];
}

@end
