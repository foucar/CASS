//
//  DisplayController.m
//  diode
//
//  Created by Jochen Küpper on 12. May 09.
//  Copyright 2009 Fritz-Haber-Institut der MPG. All rights reserved.
//

#import "DisplayController.h"
#import "ImageHandler.h"


@implementation DisplayController

- (id)init
{
    NSLog(@"DisplayController: init");    
    self = [super init];
    if (self != nil) {
        NSPort *port1 = [NSPort port];
        NSPort *port2 = [NSPort port];
        if (! ([port1 isValid] && [port2 isValid]))
            NSLog(@"DisplayController: init - ports not valid");   
        connection = [[NSConnection alloc] initWithReceivePort:port1 sendPort:port2];
        [connection setRootObject:self];
        if (! [connection isValid])
            NSLog(@"DisplayController: init - connection not valid");   
        server = [[ImageHandler alloc] init]; // the example had server = nil !
        NSArray *portArray = [NSArray arrayWithObjects:port2, port1, nil];
        [NSThread detachNewThreadSelector: @selector(connectWithPorts:)
                                 toTarget: [server class]
                               withObject: portArray];
        NSLog(@"DisplayController: init - connection created");    
    }
    return self;
}


- (void)awakeFromNib
{
    NSLog(@"DisplayController: awakeFromNib");
    // load image file
    NSString *filename = [[NSString alloc] initWithString: @"/Users/jochen/simulation.tiff"];
    NSImage *image = [[NSImage alloc] initByReferencingFile: filename];
    if(YES != [image isValid])
        NSLog(@"DisplayController: awakeFromNib: error loading image");
    NSLog(@"DisplayController: awakeFromNib: image loaded");    
    [imageView setImage: image];    
}


- (void)start: (id)sender
{
    NSLog(@"DisplayController: start");
    int i;
    for(i = 0; i < 10; ++i) {
        [NSThread sleepForTimeInterval: (NSTimeInterval)0.1];
        if (nil != server) {
            [server start: self];
            break;
        }
    }
    if (9 <= i) {
        NSLog(@"DisplayController: start - could not start server");
        exit(0);
    }
}


- (void)setServer:(id)serverObject
{
    NSLog(@"DisplayController: setServer");
    [serverObject setProtocolForProxy: @protocol(ImageInterface)];
    [serverObject retain];
    server = (id <ImageInterface>)serverObject;
}


- (oneway void)displayImage:(NSImage *)image
{
    NSLog(@"DisplayController displayImage");    
    if(YES != [image isValid])
        NSLog(@"DisplayController: displayImage - image is not valid");
    if(YES != [NSThread isMainThread])
        NSLog(@"DisplayController: displayImage - we are not on main thread!");
    else
        [imageView setImage: image];
}



@end