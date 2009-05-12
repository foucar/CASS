//
//  ImageHandler.m
//  diode
//
//  Created by Jochen Küpper on 12.05.09.
//  Copyright 2009 Fritz-Haber-Institut der MPG. All rights reserved.
//

#import "ImageHandler.h"
#import "DisplayController.h"


@implementation ImageHandler

NSImage *image;
BOOL _continue;

- (id)init
{
    NSLog(@"ImageHandler: init");
    [super init];
    _continue = FALSE;
    // load image file
    NSString *filename = [[NSString alloc] initWithString: @"/Users/jochen/simulation.tiff"];
    image = [[NSImage alloc] initByReferencingFile: filename];
    if(YES != [image isValid])
        NSLog(@"DisplayController: error loading image");
    NSLog(@"DisplayController: image loaded");    
    return self;
}


- (oneway void)start: (DisplayController *)controller
{
    NSLog(@"ImageHandler: start");
    _continue = TRUE;
    while(_continue) {
        // wait 1 s
        [NSThread sleepUntilDate:[NSDate dateWithTimeIntervalSinceNow: 1]];
        NSLog(@"ImageHandler: start - creating and sending new snapshot");    
        // create snapshot
        
        // display new image
        [controller displayImage: image];
        
        _continue = FALSE;
    }
}


+ (void)connectWithPorts: (NSArray *)portArray
{
    NSLog(@"ImageHandler: connectWithPorts");
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    NSConnection *connection = [NSConnection connectionWithReceivePort: [portArray objectAtIndex:0]
                                                              sendPort: [portArray objectAtIndex:1]];
    ImageHandler *server = [[self alloc] init];
    [((DisplayController *)[connection rootProxy]) setServer:server];
    [server release];
    [[NSRunLoop currentRunLoop] run];
    [pool release];
    return;
}


@end