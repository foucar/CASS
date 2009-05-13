//
//  diode.h
//  diode
//
//  Created by Jochen Küpper on 12. May 2009
//  Copyright 2009 Jochen Küpper. All rights reserved.
//

// define a debugging DLog function
#ifdef DEBUG
// NSLog((@”%s %s:%d ” s), func, basename(FILE), LINE, ## VA_ARGS);
#    define DLog(...) NSLog(__VA_ARGS__)
#else
#    define DLog(...) do {} while(FALSE)
#endif