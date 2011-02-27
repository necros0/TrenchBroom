//
//  FaceInspectorController.m
//  TrenchBroom
//
//  Created by Kristian Duske on 04.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "InspectorController.h"
#import "MapWindowController.h"
#import "MapDocument.h"
#import "GLResources.h"
#import "SelectionManager.h"
#import "TextureManager.h"
#import "GLFontManager.h"
#import "TextureView.h"
#import "SingleTextureView.h"
#import "TextureNameFilter.h"
#import "TextureUsageFilter.h"
#import "MapDocument.h"
#import "Brush.h"
#import "Face.h"

static InspectorController* sharedInstance = nil;

@implementation InspectorController

+ (InspectorController *)sharedInspector {
    @synchronized(self) {
        if (sharedInstance == nil)
            sharedInstance = [[self alloc] init];
    }
    return sharedInstance;
}

+ (id)allocWithZone:(NSZone *)zone {
    @synchronized(self) {
        if (sharedInstance == nil) {
            sharedInstance = [super allocWithZone:zone];
            return sharedInstance;  // assignment and return on first allocation
        }
    }
    return nil; // on subsequent allocation attempts return nil
}

- (id)copyWithZone:(NSZone *)zone {
    return self;
}

- (id)retain {
    return self;
}

- (NSUInteger)retainCount {
    return UINT_MAX;  // denotes an object that cannot be released
}

- (void)release {
    //do nothing
}

- (id)autorelease {
    return self;
}

- (NSString *)windowNibName {
    return @"Inspector";
}

- (void)windowDidLoad {
    [super windowDidLoad];
}

- (void)updateTextureControls {
    SelectionManager* selectionManager = [mapWindowController selectionManager];
    NSSet* selectedFaces = [selectionManager mode] == SM_FACES ? [selectionManager selectedFaces] : [selectionManager selectedBrushFaces];
    
    NSMutableSet* selectedTextureNames = nil;
    if ([selectedFaces count] > 0) {
        selectedTextureNames = [[NSMutableSet alloc] init];

        [xOffsetField setEnabled:YES];
        [yOffsetField setEnabled:YES];
        [xScaleField setEnabled:YES];
        [yScaleField setEnabled:YES];
        [rotationField setEnabled:YES];
        
        NSEnumerator* faceEn = [selectedFaces objectEnumerator];
        Face* face = [faceEn nextObject];
        
        int xOffset = [face xOffset];
        int yOffset = [face yOffset];
        float xScale = [face xScale];
        float yScale = [face yScale];
        float rotation = [face rotation];
        NSString* textureName = [face texture];
        
        [selectedTextureNames addObject:textureName];
        
        BOOL xOffsetMultiple = NO;
        BOOL yOffsetMultiple = NO;
        BOOL xScaleMultiple = NO;
        BOOL yScaleMultiple = NO;
        BOOL rotationMultiple = NO;
        BOOL textureMultiple = NO;
        
        while ((face = [faceEn nextObject])) {
            xOffsetMultiple  |= xOffset  != [face xOffset];
            yOffsetMultiple  |= yOffset  != [face yOffset];
            xScaleMultiple   |= xScale   != [face xScale];
            yScaleMultiple   |= yScale   != [face yScale];
            rotationMultiple |= rotation != [face rotation];
            textureMultiple  |= ![textureName isEqualToString:[face texture]];
            [selectedTextureNames addObject:[face texture]];
        }
        
        if (xOffsetMultiple) {
            [[xOffsetField cell] setPlaceholderString:@"multiple"];
            [xOffsetField setStringValue:@""];
        } else {
            [xOffsetField setIntValue:xOffset];
        }
        
        if (yOffsetMultiple) {
            [[yOffsetField cell] setPlaceholderString:@"multiple"];
            [yOffsetField setStringValue:@""];
        } else {
            [yOffsetField setIntValue:yOffset];
        }
        
        if (xScaleMultiple) {
            [[xScaleField cell] setPlaceholderString:@"multiple"];
            [xScaleField setStringValue:@""];
        } else {
            [xScaleField setFloatValue:xScale];
        }
        
        if (yScaleMultiple) {
            [[yScaleField cell] setPlaceholderString:@"multiple"];
            [yScaleField setStringValue:@""];
        } else {
            [yScaleField setFloatValue:yScale];
        }
        
        if (rotationMultiple) {
            [[rotationField cell] setPlaceholderString:@"multiple"];
            [rotationField setStringValue:@""];
        } else {
            [rotationField setFloatValue:rotation];
        }
        
        if (textureMultiple) {
            [[textureNameField cell] setPlaceholderString:@"multiple"];
            [textureNameField setStringValue:@""];
            [singleTextureView setTexture:nil];
        } else {
            [textureNameField setStringValue:textureName];
            
            MapDocument* document = [mapWindowController document];
            GLResources* glResources = [document glResources];
            TextureManager* textureManager = [glResources textureManager];
            Texture* texture = [textureManager textureForName:textureName];
            [singleTextureView setTexture:texture];
        }
    } else {
        [xOffsetField setEnabled:NO];
        [yOffsetField setEnabled:NO];
        [xScaleField setEnabled:NO];
        [yScaleField setEnabled:NO];
        [rotationField setEnabled:NO];
        
        [[xOffsetField cell] setPlaceholderString:@"n/a"];
        [[yOffsetField cell] setPlaceholderString:@"n/a"];
        [[xScaleField cell] setPlaceholderString:@"n/a"];
        [[yScaleField cell] setPlaceholderString:@"n/a"];
        [[rotationField cell] setPlaceholderString:@"n/a"];
        [[textureNameField cell] setPlaceholderString:@"n/a"];
        
        [xOffsetField setStringValue:@""];
        [yOffsetField setStringValue:@""];
        [xScaleField setStringValue:@""];
        [yScaleField setStringValue:@""];
        [rotationField setStringValue:@""];
        [textureNameField setStringValue:@""];
        [singleTextureView setTexture:nil];
    }
    
    [textureView setSelectedTextureNames:selectedTextureNames];
    [selectedTextureNames release];
}

- (void)faceFlagsChanged:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    NSSet* face = [userInfo objectForKey:FaceKey];
    
    SelectionManager* selectionManager = [mapWindowController selectionManager];
    NSSet* selectedFaces = [selectionManager mode] == SM_FACES ? [selectionManager selectedFaces] : [selectionManager selectedBrushFaces];

    if ([selectedFaces containsObject:face])
        [self updateTextureControls];
}

- (void)selectionRemoved:(NSNotification *)notification {
    [self updateTextureControls];
}

- (void)selectionAdded:(NSNotification *)notification {
    [self updateTextureControls];
}

- (void)setMapWindowController:(MapWindowController *)theMapWindowController {
    if (mapWindowController == theMapWindowController)
        return;
    
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];

    if (mapWindowController != nil) {
        MapDocument* map = [mapWindowController document];
        GLResources* glResources = [map glResources];

        TextureManager* textureManager = [glResources textureManager];
        [center removeObserver:self name:TexturesAdded object:textureManager];
        [center removeObserver:self name:TexturesRemoved object:textureManager];
        
        SelectionManager* selectionManager = [mapWindowController selectionManager];
        [center removeObserver:self name:SelectionAdded object:selectionManager];
        [center removeObserver:self name:SelectionRemoved object:selectionManager];

        [center removeObserver:self name:FaceFlagsChanged object:map];
        
        [mapWindowController release];
    }
    
    mapWindowController = [theMapWindowController retain];

    if (mapWindowController != nil) {
        MapDocument* map = [mapWindowController document];
        GLResources* glResources = [map glResources];
        NSOpenGLContext* context = [[NSOpenGLContext alloc] initWithFormat:[singleTextureView pixelFormat] shareContext:[glResources openGLContext]];
        [singleTextureView setOpenGLContext:context];
        [context release];

        [textureView setGLResources:glResources];

        TextureManager* textureManager = [glResources textureManager];
        [center addObserver:self selector:@selector(textureManagerChanged:) name:TexturesAdded object:textureManager];
        [center addObserver:self selector:@selector(textureManagerChanged:) name:TexturesRemoved object:textureManager];

        SelectionManager* selectionManager = [mapWindowController selectionManager];
        [center addObserver:self selector:@selector(selectionAdded:) name:SelectionAdded object:selectionManager];
        [center addObserver:self selector:@selector(selectionRemoved:) name:SelectionRemoved object:selectionManager];
        
        [center addObserver:self selector:@selector(faceFlagsChanged:) name:FaceFlagsChanged object:map];
    } else {
        [textureView setGLResources:nil];
    }

    [self updateTextureControls];
}

- (MapWindowController *)mapWindowController {
    return mapWindowController;
}

- (void)textureManagerChanged:(NSNotification *)notification {
}

- (IBAction)xOffsetTextChanged:(id)sender {
    SelectionManager* selectionManager = [mapWindowController selectionManager];
    int xOffset = [xOffsetField intValue];
    NSSet* faces = [selectionManager mode] == SM_FACES ? [selectionManager selectedFaces] : [selectionManager selectedBrushFaces];
    NSEnumerator* faceEn = [faces objectEnumerator];
    Face* face;
    while ((face = [faceEn nextObject]))
        [face setXOffset:xOffset];
}

- (IBAction)yOffsetTextChanged:(id)sender {
    SelectionManager* selectionManager = [mapWindowController selectionManager];
    int yOffset = [yOffsetField intValue];
    NSSet* faces = [selectionManager mode] == SM_FACES ? [selectionManager selectedFaces] : [selectionManager selectedBrushFaces];
    NSEnumerator* faceEn = [faces objectEnumerator];
    Face* face;
    while ((face = [faceEn nextObject]))
        [face setYOffset:yOffset];
}

- (IBAction)xScaleTextChanged:(id)sender {
    SelectionManager* selectionManager = [mapWindowController selectionManager];
    float xScale = [xScaleField floatValue];
    NSSet* faces = [selectionManager mode] == SM_FACES ? [selectionManager selectedFaces] : [selectionManager selectedBrushFaces];
    NSEnumerator* faceEn = [faces objectEnumerator];
    Face* face;
    while ((face = [faceEn nextObject]))
        [face setXScale:xScale];
}

- (IBAction)yScaleTextChanged:(id)sender {
    SelectionManager* selectionManager = [mapWindowController selectionManager];
    float yScale = [yScaleField floatValue];
    NSSet* faces = [selectionManager mode] == SM_FACES ? [selectionManager selectedFaces] : [selectionManager selectedBrushFaces];
    NSEnumerator* faceEn = [faces objectEnumerator];
    Face* face;
    while ((face = [faceEn nextObject]))
        [face setYScale:yScale];
}

- (IBAction)rotationTextChanged:(id)sender {
    SelectionManager* selectionManager = [mapWindowController selectionManager];
    float rotation = [rotationField floatValue];
    NSSet* faces = [selectionManager mode] == SM_FACES ? [selectionManager selectedFaces] : [selectionManager selectedBrushFaces];
    NSEnumerator* faceEn = [faces objectEnumerator];
    Face* face;
    while ((face = [faceEn nextObject]))
        [face setRotation:rotation];
}

- (void)textureSelected:(Texture *)texture {
    SelectionManager* selectionManager = [mapWindowController selectionManager];
    NSSet* faces = [selectionManager mode] == SM_FACES ? [selectionManager selectedFaces] : [selectionManager selectedBrushFaces];
    NSEnumerator* faceEn = [faces objectEnumerator];
    Face* face;
    while ((face = [faceEn nextObject]))
        [face setTexture:[texture name]];
}

- (void)updateFilter {
    id<TextureFilter> filter = nil;
    NSString* pattern = [textureNameFilterField stringValue];
    
    if (pattern != nil && [pattern length] > 0)
        filter = [[TextureNameFilter alloc] initWithPattern:pattern];
    
    if ([textureUsageFilterSC selectedSegment] == 1) {
        id<TextureFilter> temp = [[TextureUsageFilter alloc] initWithFilter:filter];
        [filter release];
        filter = temp;
    }
    
    [textureView setTextureFilter:filter];
    [filter release];
}

- (IBAction)textureNameFilterTextChanged:(id)sender {
    [self updateFilter];
}

- (IBAction)textureUsageFilterChanged:(id)sender {
    [self updateFilter];
}

- (IBAction)textureSortCriterionChanged:(id)sender {
    if ([textureSortCriterionSC selectedSegment] == 0)
        [textureView setSortCriterion:SC_NAME];
    else
        [textureView setSortCriterion:SC_USAGE];
}

- (void)dealloc {
    [self setMapWindowController:nil];
    [super dealloc];
}

@end