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

#import <Cocoa/Cocoa.h>


@interface AliasSkin : NSObject {
    int width;
    int height;
    int count;
    float* times;
    unsigned char* pictures;
}

- (id)initSingleSkin:(const unsigned char *)thePicture width:(int)theWidth height:(int)theHeight;
- (id)initMultiSkin:(const unsigned char *)thePictures times:(float *)theTimes count:(int)theCount width:(int)theWidth height:(int)theHeight;

- (int)width;
- (int)height;

- (const unsigned char *)pictureAtIndex:(int)theIndex;
- (float)timeAtIndex:(int)theIndex;
- (int)pictureCount;

@end
