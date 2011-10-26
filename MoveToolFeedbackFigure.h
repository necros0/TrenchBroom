/*
Copyright (C) 2010-2011 Kristian Duske

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

#import <Foundation/Foundation.h>
#import "Figure.h"
#import "Math.h"
#import "MoveTool.h"

@class EditingSystem;

@interface MoveToolFeedbackFigure : NSObject <Figure> {
    EditingSystem* editingSystem;
    EMoveDirection moveDirection;
    TVector3f point;
    float arrowLength;
}

- (id)initWithArrowLength:(float)theArrowLength;

- (void)setEditingSystem:(EditingSystem *)theEditingSystem;
- (void)setPoint:(const TVector3f *)thePoint;
- (void)setMoveDirection:(EMoveDirection)theMoveDirection;
@end