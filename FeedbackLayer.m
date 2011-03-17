//
//  FeedbackLayer.m
//  TrenchBroom
//
//  Created by Kristian Duske on 17.03.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "FeedbackLayer.h"
#import <OpenGL/gl.h>
#import "RenderContext.h"
#import "Figure.h"
#import "Face.h"
#import "Vector3f.h"
#import "Options.h"

@implementation FeedbackLayer
- (void)renderFaces:(RenderContext *)renderContext {
    switch ([[renderContext options] renderMode]) {
        case RM_TEXTURED:
            glEnable(GL_TEXTURE_2D);
            glPolygonMode(GL_FRONT, GL_FILL);
            glColor4f(0, 0, 1, 1);
            glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
            [self renderTextured:renderContext];
            
            glDisable(GL_TEXTURE_2D);
            glDisable(GL_DEPTH_TEST);
            glPolygonMode(GL_FRONT, GL_LINE);
            [self renderWireframe:renderContext];
            glEnable(GL_DEPTH_TEST);
            break;
        case RM_FLAT:
            break;
        case RM_WIREFRAME:
            glDisable(GL_TEXTURE_2D);
            glDisable(GL_DEPTH_TEST);
            glPolygonMode(GL_FRONT, GL_LINE);
            glColor4f(0, 0, 1, 0.5);
            [self renderWireframe:renderContext];
            glEnable(GL_DEPTH_TEST);
            break;
    }
}
@end
