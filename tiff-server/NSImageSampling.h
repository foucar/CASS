//
//  NSImageSampling.h
//  Sample sub-set from Image
//
//  Created by Jochen Küpper
//  Copyright (C) 2009 Jochen Küpper. All rights reserved.
//

#import <AppKit/NSImage.h>


@interface NSImage (Sampling) 

- (NSImage *) sampleFromImage;

@end
