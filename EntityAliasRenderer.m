//
//  AliasRenderer.m
//  TrenchBroom
//
//  Created by Kristian Duske on 12.06.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "EntityAliasRenderer.h"
#import "Entity.h"
#import "EntityDefinition.h"
#import "EntityDefinitionProperty.h"
#import "ModelProperty.h"
#import "AliasManager.h"
#import "Alias.h"
#import "BspManager.h"
#import "Bsp.h"
#import "BspModel.h"
#import "EntityRenderer.h"
#import "BspRenderer.h"
#import "AliasRenderer.h"
#import "VBOBuffer.h"
#import "Filter.h"

@interface EntityAliasRenderer (private)

- (id <NSCopying>)rendererKey:(ModelProperty *)theModelProperty;

@end

@implementation EntityAliasRenderer (private)

- (id <NSCopying>)rendererKey:(ModelProperty *)theModelProperty {
    return [NSString stringWithFormat:@"%@ %@ %i", [theModelProperty modelPath], [theModelProperty flagName], [theModelProperty skinIndex]];
}

@end

@implementation EntityAliasRenderer

- (id)init {
    if ((self = [super init])) {
        vbo = [[VBOBuffer alloc] initWithTotalCapacity:0xFFFF];
        entities = [[NSMutableSet alloc] init];
        entityRenderers = [[NSMutableDictionary alloc] init];

        NSBundle* mainBundle = [NSBundle mainBundle];
        NSString* palettePath = [mainBundle pathForResource:@"QuakePalette" ofType:@"lmp"];
        palette = [[NSData alloc] initWithContentsOfFile:palettePath];
    }
    
    return self;
}

- (void)dealloc {
    [entities release];
    [entityRenderers release];
    [vbo release];
    [palette release];
    [filter release];
    [super dealloc];
}

- (void)addEntity:(id <Entity>)entity {
    NSAssert(entity != nil, @"entity must no be nil");
    
    EntityDefinition* definition = [entity entityDefinition];
    if (definition == nil)
        return;
    
    ModelProperty* modelProperty = [definition modelPropertyForEntity:entity];
    if (modelProperty != nil) {
        id <NSCopying> rendererKey = [self rendererKey:modelProperty];
        id <EntityRenderer> entityRenderer = [entityRenderers objectForKey:rendererKey];
        if (entityRenderer == nil) {
            NSString* modelName = [[modelProperty modelPath] substringFromIndex:1];
            NSArray* pakPaths = [NSArray arrayWithObject:@"/Applications/Quake/id1"];
            if ([[modelName pathExtension] isEqualToString:@"mdl"]) {
                AliasManager* aliasManager = [AliasManager sharedManager];
                Alias* alias = [aliasManager aliasWithName:modelName paths:pakPaths];
                
                if (alias != nil) {
                    int skinIndex = [modelProperty skinIndex];
                    
                    entityRenderer = [[AliasRenderer alloc] initWithAlias:alias skinIndex:skinIndex vbo:vbo palette:palette];
                    [entityRenderers setObject:entityRenderer forKey:rendererKey];
                    [entityRenderer release];
                }
            } else if ([[modelName pathExtension] isEqualToString:@"bsp"]) {
                BspManager* bspManager = [BspManager sharedManager];
                Bsp* bsp = [bspManager bspWithName:modelName paths:pakPaths];
                
                if (bsp != nil) {
                    entityRenderer = [[BspRenderer alloc] initWithBsp:bsp vbo:vbo];
                    [entityRenderers setObject:entityRenderer forKey:rendererKey];
                    [entityRenderer release];
                }
            }
        }
        if (entityRenderer != nil)
            [entities addObject:entity];
    }
}

- (void)removeEntity:(id <Entity>)entity {
    [entities removeObject:entity];
}

- (void)render {
    [vbo activate];
    
    glMatrixMode(GL_MODELVIEW);
    NSEnumerator* entityEn = [entities objectEnumerator];
    id <Entity> entity;
    while ((entity = [entityEn nextObject])) {
        if (filter == nil || [filter isEntityRenderable:entity]) {
            EntityDefinition* definition = [entity entityDefinition];
            ModelProperty* modelProperty = [definition modelPropertyForEntity:entity];
            id <NSCopying> rendererKey = [self rendererKey:modelProperty];
            id <EntityRenderer> entityRenderer = [entityRenderers objectForKey:rendererKey];

            glPushMatrix();
            [entityRenderer renderWithEntity:entity];
            glPopMatrix();
        }
    }
    
    glDisable(GL_TEXTURE_2D);
    [vbo deactivate];
}

- (void)setFilter:(id <Filter>)theFilter {
    [filter release];
    filter = [theFilter retain];
}

@end