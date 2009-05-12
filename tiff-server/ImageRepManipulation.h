//
//  ImageRepManipualtion.h
//  Image represention manipulations
//
//  Created by Jochen Küpper on 11. May 2009.
//  Copyright (C) 2009 Jochen Küpper. All rights reserved.
//

#import <AppKit/NSImage.h>

@interface NSBitmapImageRep (Negative)

- (NSBitmapImageRep *) negativeImageRep;

@end


@interface NSBitmapImageRep (Sampling)

- (NSBitmapImageRep *) sampleImageRep;

@end
