//
//  ImageHandler.m
//  diode
//
//  Created by Jochen Küpper on 12.05.09.
//  Copyright 2009 Fritz-Haber-Institut der MPG. All rights reserved.
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
    if(YES != [image isValid]) {
        NSLog(@"ImageParameters: init - error loading default image");
        exit;
    }
}

@end



@implementation ImageHandler


- (id)init
{
    NSLog(@"ImageHandler: init");
    [super init];
    _continue = FALSE;
    // load image file
    NSString *filename = [[NSString alloc] initWithString: @"/Users/jochen/simulation.tiff"];
    image = [[NSImage alloc] initByReferencingFile: filename];
    if(YES != [image isValid])
        NSLog(@"ImageHandler: error loading image");
    NSLog(@"ImageHandler: image loaded");    
    return self;
}


- (void)run: (id)p
{
    NSLog(@"ImageHandler: run");
    param = (ImageParameters *)p;
    [param->lock lock];
    param->acquire = TRUE;
    [param->lock unlock];
    while(param->acquire) {
        // wait 1 s
        [NSThread sleepUntilDate:[NSDate dateWithTimeIntervalSinceNow: 0.05]];
        NSLog(@"ImageHandler: start - creating and sending new snapshot");    
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