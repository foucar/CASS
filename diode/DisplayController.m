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
    timer = [NSTimer scheduledTimerWithTimeInterval: 0.2
                                             target: self
                                           selector: @selector(updateDisplay:)
                                           userInfo: 0
                                            repeats: YES];
}


- (void)stop: (id)sender
{
    NSLog(@"DisplayController: stop");
    [param->lock lock];
    param->acquire = FALSE;
    [param->lock unlock];
    [timer invalidate];
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



    
- (void)updateDisplay: (id)sender
{
    NSLog(@"DisplayController: updateDisplay");    
    [self displayImage: param->image];
}



@end