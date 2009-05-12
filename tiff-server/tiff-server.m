#import <Foundation/Foundation.h>
#import <AppKit/NSImage.h>
#import <Foundation/NSString.h>
#import "NSImageSampling.h"

int main (int argc, const char * argv[]) {
    NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];

    NSLog(@"TCP TIFF server for data acquisitions simulations");

    // load full simulation
    NSString *filename = [[NSString alloc] initWithUTF8String: argv[1]];
    NSImage *simulation = [[NSImage alloc] initByReferencingFile: filename];
    NSLog(@"TCP TIFF server: image loaded");    
    
    // create snapshot from original file
    NSImage *snapshot = [simulation sampleFromImage];
    NSLog(@"TCP TIFF server: snapshot created");

    // save snapshot
    NSData * data = [snapshot TIFFRepresentation];
    filename = [[NSString alloc] initWithUTF8String: "snapshot.tiff"];
    // filename = [filename stringByExpandingTildeInPath];
    NSLog(@"%@", filename);
    if (YES == [data writeToFile: filename
                      atomically: NO])
        NSLog(@"TCP TIFF server: snapshot saved");
    else
        NSLog(@"TCP TIFF server: problems saving snapshot");
    
    [pool drain];
    return 0;
}
