//
//  ImageHandler.h
//  diode
//
//  Created by Jochen Küpper on 12.05.09.
//  Copyright 2009 Jochen Küpper. All rights reserved.
//

#import <AppKit/AppKit.h>
#import <Cocoa/Cocoa.h>

@class DisplayController;


@interface ImageParameters : NSObject
{
@public
    NSLock *lock;
    BOOL acquire;
    BOOL updated;
    NSImage *image;
}

- (id) init;

@end



@interface ImageHandler : NSObject
{
    NSImage *image;
    ImageParameters *param;
    BOOL _continue;
}

- (id)init;

- (void)run: (id)p;

@end
