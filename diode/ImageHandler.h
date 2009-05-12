//
//  ImageHandler.h
//  diode
//
//  Created by Jochen Küpper on 12.05.09.
//  Copyright 2009 Jochen Küpper. All rights reserved.
//

#import <AppKit/AppKit.h>

@class DisplayController;



@protocol ImageInterface <NSObject>

/* Start reading images and send them back to the DisplayController */
- (oneway void)start:(DisplayController *)controller;

// /* Stop reading images */
// - (oneway void)stop:(DsiplayController *)controller;

@end



@interface ImageHandler: NSObject <ImageInterface>
{
}

+ (void)connectWithPorts: (NSArray *)portArray;

- (id)init;

@end
