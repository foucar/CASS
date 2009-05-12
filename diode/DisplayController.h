//
//  DisplayController.h
//  diode
//
//  Created by Jochen Küpper on 12. May 09
//  Copyright 2009 Jochen Küpper. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <AppKit/NSImageView.h>

@class ImageHandler;


@interface DisplayController: NSObject 
{
    IBOutlet NSImageView *imageView;
    NSConnection *connection;
    ImageHandler *server; 
}


- (void)setServer: (id)serverObject;

- (void)start: (id)sender;

- (oneway void)displayImage: (NSImage *)image;


@end
