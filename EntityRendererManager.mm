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

#import "EntityRendererManager.h"
#import "EntityRenderer.h"
#import "EntityDefinition.h"
#import "Entity.h"
#import "ModelProperty.h"
#import "Alias.h"
#import "Bsp.h"
#import "AliasRenderer.h"
#import "BspRenderer.h"
#import "Vbo.h"
#import "PreferencesManager.h"

using namespace TrenchBroom;

@interface EntityRendererManager (Private)

- (void)preferencesDidChange:(NSNotification *)notification;
- (id <NSCopying>)rendererKey:(ModelProperty *)theModelProperty pakPaths:(NSArray *)thePaths;
- (id <EntityRenderer>)entityRendererForModelProperty:(ModelProperty *)theModelProperty mods:(NSArray *)theMods;

@end

@implementation EntityRendererManager (Private)

- (void)preferencesDidChange:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    if (DefaultsQuakePath != [userInfo objectForKey:DefaultsKey])
        return;
    
    [entityRenderers removeAllObjects];
}

- (id <NSCopying>)rendererKey:(ModelProperty *)theModelProperty pakPaths:(NSArray *)thePaths {
    return [NSString stringWithFormat:@"%@ %@ %@ %i", [thePaths componentsJoinedByString:@":"], [theModelProperty modelPath], [theModelProperty flagName], [theModelProperty skinIndex]];
}

- (id <EntityRenderer>)entityRendererForModelProperty:(ModelProperty *)theModelProperty mods:(NSArray *)theMods {
    NSString* quakePath = [[PreferencesManager sharedManager] quakePath];
    if (quakePath == nil)
        return nil;
    
    NSMutableArray* pakPaths = [[NSMutableArray alloc] initWithCapacity:[theMods count]];
    for (NSString* mod in theMods)
        [pakPaths addObject:[quakePath stringByAppendingPathComponent:mod]];
    
    id <NSCopying> rendererKey = [self rendererKey:theModelProperty pakPaths:pakPaths];
    id <EntityRenderer> entityRenderer = [entityRenderers objectForKey:rendererKey];
    
    if (entityRenderer == nil) {
        NSString* modelName = [[theModelProperty modelPath] substringFromIndex:1];
        string cppName = [modelName cStringUsingEncoding:NSASCIIStringEncoding];
        vector<string> cppPaths;
        for (NSString* path in pakPaths)
            cppPaths.push_back([path cStringUsingEncoding:NSASCIIStringEncoding]);

        if ([[modelName pathExtension] isEqualToString:@"mdl"]) {
            AliasManager& aliasManager = AliasManager::sharedManager();
            Alias* alias = aliasManager.aliasForName(cppName, cppPaths);
            
            if (alias != NULL) {
                int skinIndex = [theModelProperty skinIndex];
                
                entityRenderer = [[AliasRenderer alloc] initWithAlias:alias skinIndex:skinIndex vbo:&vbo palette:palette];
                [entityRenderers setObject:entityRenderer forKey:rendererKey];
                [entityRenderer release];
            } else {
                NSLog(@"Model '%@' not found in %@", modelName, [pakPaths componentsJoinedByString:@", "]);
            }
        } else if ([[modelName pathExtension] isEqualToString:@"bsp"]) {
            BspManager& bspManager = BspManager::sharedManager();
            Bsp* bsp = bspManager.bspForName(cppName, cppPaths);
            
            if (bsp != NULL) {
                entityRenderer = [[BspRenderer alloc] initWithBsp:bsp vbo:&vbo palette:palette];
                [entityRenderers setObject:entityRenderer forKey:rendererKey];
                [entityRenderer release];
            } else {
                NSLog(@"Model '%@' not found in %@", modelName, [pakPaths componentsJoinedByString:@", "]);
            }
        }
    }
    
    [pakPaths release];
    
    return entityRenderer;
}

@end

@implementation EntityRendererManager

- (id)initWithPalette:(NSData *)thePalette {
    NSAssert(thePalette != nil, @"palette must not be nil");
    
    if ((self = [self init])) {
        entityRenderers = [[NSMutableDictionary alloc] init];
        initVbo(&vbo, GL_ARRAY_BUFFER, 0xFFFF);
        palette = [thePalette retain];
        
        PreferencesManager* preferences = [PreferencesManager sharedManager];
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        [center addObserver:self selector:@selector(preferencesDidChange:) name:DefaultsDidChange object:preferences];
    }
    
    return self;
}

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [entityRenderers release];
    freeVbo(&vbo);
    [palette release];
    [super dealloc];
}

- (id <EntityRenderer>)entityRendererForDefinition:(EntityDefinition *)theDefinition mods:(NSArray *)theMods {
    NSAssert(theDefinition != nil, @"entity definition must not be nil");
    NSAssert(theMods != nil, @"mod list must not be nil");
    NSAssert([theMods count] > 0, @"mod list must not be empty");
    
    ModelProperty* modelProperty = [theDefinition defaultModelProperty];
    if (modelProperty == nil)
        return nil;
    
    return [self entityRendererForModelProperty:modelProperty mods:theMods];
}

- (id <EntityRenderer>)entityRendererForEntity:(id<Entity>)theEntity mods:(NSArray *)theMods {
    NSAssert(theEntity != nil, @"entity must not be nil");
    NSAssert(theMods != nil, @"mod list must not be nil");
    NSAssert([theMods count] > 0, @"mod list must not be empty");
    
    EntityDefinition* definition = [theEntity entityDefinition];
    if (definition == nil)
        return nil;
    
    ModelProperty* modelProperty = [definition modelPropertyForEntity:theEntity];
    if (modelProperty == nil)
        return nil;
    
    return [self entityRendererForModelProperty:modelProperty mods:theMods];
}

- (void)clear {
    [entityRenderers removeAllObjects];
}

- (void)activate {
    activateVbo(&vbo);
}

- (void)deactivate {
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    deactivateVbo(&vbo);
}


@end
