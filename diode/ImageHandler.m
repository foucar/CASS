//
//  ImageHandler.m
//  diode
//
//  Created by Jochen Küpper on 12. May 2009.
//  Copyright 2009 Jochen Küpper. All rights reserved.
//

#import "ImageHandler.h"
#import "DisplayController.h"


@implementation ImageParameters

- (id) init
{
    return [super init];
    lock = [[NSLock alloc] init];
    acquire = FALSE;
    updated = FALSE;
    // load image file
    NSString *filename = [[NSString alloc] initWithString: @"/Users/jochen/simulation.tiff"];
    image = [[NSImage alloc] initByReferencingFile: filename];
    NSAssert([image isValid], @"ImageParameters: init - error loading default image");
}

@end



@implementation ImageHandler


- (id)init
{
    DLog(@"ImageHandler init:");
    [super init];
    _continue = FALSE;
    // load image file
    NSString *filename = [[NSString alloc] initWithString: @"/Users/jochen/simulation.tiff"];
    image = [[NSImage alloc] initByReferencingFile: filename];
    NSAssert([image isValid], @"ImageHandler init: error loading image");
    return self;
}


- (void)run: (id)p
{
    DLog(@"ImageHandler: run");
    param = (ImageParameters *)p;
    [param->lock lock];
    param->acquire = TRUE;
    [param->lock unlock];
    while(param->acquire) {
        // wait 1 s
        [NSThread sleepUntilDate:[NSDate dateWithTimeIntervalSinceNow: 0.05]];
        // create snapshot
        // NSImage *snapshot = [[NSImage alloc] init];
        NSImage *snapshot = image;
        // store image
        [param->lock lock];
        param->image = snapshot;
        param->updated = TRUE;
        [param->lock unlock];
    }
}



@end