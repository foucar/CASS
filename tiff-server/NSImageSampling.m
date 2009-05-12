//
//  NSImageSampling.h
//  Sample sub-set from Image
//
//  Created by Jochen Küpper
//  Copyright (C) 2009 Jochen Küpper. All rights reserved.
//

#import "NSImageSampling.h"
#import "ImageRepManipulation.h"


@implementation NSImage (Sampling) 

- (NSImage *) sampleFromImage
{
	NSImage * sampledImage = [[[self class] alloc] initWithSize:[self size]];
    NSBitmapImageRep * sampledRep = [[[self representations] objectAtIndex:0] sampleImageRep];
	[sampledImage addRepresentation:sampledRep];
	return [sampledImage autorelease];
}	

@end