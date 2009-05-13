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
    DLog(@"DisplayController init:");    
    self = [super init];
    param = [[ImageParameters alloc] init];
    return self;
}


- (void)awakeFromNib
{
    DLog(@"DisplayController awakeFromNib:");
}


- (void)start: (id)sender
{
    DLog(@"DisplayController start:");
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
    DLog(@"DisplayController stop:");
    [param->lock lock];
    param->acquire = FALSE;
    [param->lock unlock];
    [timer invalidate];
}


- (void)displayImage:(NSImage *)image
{
    DLog(@"DisplayController displayImage:");    
    NSAssert([image isValid], @"DisplayController displayImage: - image is not valid");
    NSAssert([NSThread isMainThread], @"DisplayController displayImage: - we are not on main thread!");
    [imageView setImage: image];
}



    
- (void)updateDisplay: (id)sender
{
    DLog(@"DisplayController updateDisplay:");    
    [self displayImage: param->image];
}



@end