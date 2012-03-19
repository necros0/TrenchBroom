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

#import "AliasSkin.h"


@implementation AliasSkin
- (id)initSingleSkin:(const unsigned char *)thePicture width:(int)theWidth height:(int)theHeight {
    NSAssert(thePicture != nil, @"picture must not be nil");
    
    if ((self = [self init])) {
        count = 1;
        times = NULL;
        width = theWidth;
        height = theHeight;
        pictures = malloc(width * height);
        memcpy(pictures, thePicture, width * height);
    }
    
    return self;
}

- (id)initMultiSkin:(const unsigned char *)thePictures times:(float *)theTimes count:(int)theCount width:(int)theWidth height:(int)theHeight {
    NSAssert(thePictures != nil, @"picture array must not be nil");
    NSAssert(theTimes != NULL, @"time array must not be NULL");

    if ((self = [self init])) {
        count = theCount;
        width = theWidth;
        height = theHeight;
        times = malloc(count * sizeof(float));
        memcpy(times, theTimes, count * sizeof(float));
        pictures = malloc(count * width * height);
        memcpy(pictures, thePictures, count * width * height);
    }
    
    return self;
}

- (void)dealloc {
    if (times != NULL)
        free(times);
    free(pictures);
    [super dealloc];
}

- (int)width {
    return width;
}

- (int)height {
    return height;
}

- (const unsigned char *)pictureAtIndex:(int)theIndex {
    NSAssert(theIndex >= 0 && theIndex < [self pictureCount], @"index out of bounds");
    return pictures + theIndex * width * height;
}

- (float)timeAtIndex:(int)theIndex {
    NSAssert([self pictureCount] > 1, @"only multi skins can have times");
    NSAssert(theIndex >= 0 && theIndex < [self pictureCount], @"index out of bounds");
    return times[theIndex];
}

- (int)pictureCount {
    return count;
}

@end
