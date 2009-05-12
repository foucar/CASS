//
//  ImageRepManipulation.m
//  Image represenation manipulation methods
//
//  Created by Jochen Küpper on 11. May 2009
//  Copyright (C) 2009 Jochen Küpper. All rights reserved.

#import "ImageRepManipulation.h"
#import "PixelTypes.h"
	
	
@implementation NSBitmapImageRep (Negative)

- (NSBitmapImageRep *) negativeImageRep
{
    Pixel * pixels = (Pixel *)[self bitmapData];  // -bitmapData returns a void*, not an NSData object ;-)
		
	int row, column, widthInPixels = [self pixelsWide], heightInPixels = [self pixelsHigh];
		
    for (row = 0; row < heightInPixels; row++)		
		for (column = 0; column < widthInPixels; column++)
        {
			Pixel * thisPixel = (Pixel *) &(pixels[((widthInPixels * row) + column)]);
			*thisPixel = (255 - (*thisPixel));
        }
    return self;		
}	
	
@end


@implementation NSBitmapImageRep (Sampling)

- (NSBitmapImageRep *) sampleImageRep
{
	int height = [self pixelsHigh], width = [self pixelsWide];
    NSBitmapImageRep *sampledRep = [NSBitmapImageRep alloc];
    [sampledRep initWithBitmapDataPlanes: NULL 
                              pixelsWide: width
                              pixelsHigh: height
                           bitsPerSample: 8
                         samplesPerPixel: [self bitsPerPixel] / 8
                                hasAlpha: [self hasAlpha]
                                isPlanar: NO
                          colorSpaceName: [self colorSpaceName]
                             bytesPerRow: [self bytesPerRow]
                            bitsPerPixel: 0];
    // clear new image
	Pixel* dest = (Pixel *)[sampledRep bitmapData];
    int i, row, col;
    for (row = 0; row < height; ++row)
		for (col = 0; col < width; ++col)
        {
			Pixel * thisPixel = (Pixel *) &(dest[((width * row) + col)]);
			*thisPixel = 0;
        }    
    // sample from original image
	Pixel* source = (Pixel *)[self bitmapData];
	for (i = 0; i < 100; ++i)
    {
        row = random() % height;
        col = random() % width;
        dest[(width * row) + col] = source[(width * row) + col];
    }
    return [sampledRep autorelease];
}

@end

