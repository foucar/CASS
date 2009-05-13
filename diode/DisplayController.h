//
//  DisplayController.h
//  diode
//
//  Created by Jochen Küpper on 12. May 09
//  Copyright 2009 Jochen Küpper. All rights reserved.
//

// #import <AppKit/NSImageView.h>

@class ImageHandler;
@class ImageParameters;


@interface DisplayController: NSObject 
{
    IBOutlet NSImageView *imageView;
    ImageParameters *param;
    NSTimer *timer;
}


- (void)displayImage: (NSImage *)image;

- (void)start: (id)sender;

- (void)stop: (id)sender;

- (void)updateDisplay: (id) sender;


@end

