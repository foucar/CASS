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
    param = [[ImageParameters alloc] init];
    return self;
}


- (void)awakeFromNib
{
    NSLog(@"DisplayController: awakeFromNib");
}


- (void)start: (id)sender
{
    NSLog(@"DisplayController: start");
    [param->lock lock];
    if(FALSE == param->acquire) {
        ImageHandler * handler = [[ImageHandler alloc] init];
        [NSThread detachNewThreadSelector: @selector(run:)
                                 toTarget: handler
                               withObject: param]; 
    }
    [param->lock unlock];
}


- (void)stop: (id)sender
{
    NSLog(@"DisplayController: stop");
    [param->lock lock];
    param->acquire = FALSE;
    [param->lock unlock];
}


- (void)displayImage:(NSImage *)image
{
    NSLog(@"DisplayController displayImage");    
    if(TRUE != [image isValid]) {
        NSLog(@"DisplayController: displayImage - image is not valid");
        return;
    }
    if(TRUE != [NSThread isMainThread]) {
        NSLog(@"DisplayController: displayImage - we are not on main thread!");
        return;
    }
    [imageView setImage: image];
}



- (void)displayString:(NSString *)string
{
    NSLog(@"DisplayController displayString");    
    if(TRUE != [NSThread isMainThread]) {
        NSLog(@"DisplayController: displayString - we are not on main thread!");
        return;
    }
    NSLog(string);
}



@end