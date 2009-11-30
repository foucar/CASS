/*************************************************************************

 Copyright (C) 2007-2009 by Peter Holl and Nils Kimmel.
 All rights reserved.
 
 This code implementation is the intellectual property of

 Peter Holl    <pxh@hll.mpg.de> , MPI semiconductor laboratory,
                                  PNSensor GmbH

 Nils Kimmel   <nik@hll.mpg.de> , MPI semiconductor laboratory,
                                  Max-Planck-Institut fuer extra-
                                  terrestrische Physik

 By copying, distributing or modifying the Program (or any work
 based on the Program) you indicate your acceptance of this statement,
 and all its terms.

*************************************************************************/


// xonline_constants.h
// Global constants for array sizes, switch identifiers etc.

#ifndef XONLINECONSTANTS_H
#define XONLINECONSTANTS_H

//

#define NCOLORS		128
//#define EBFRSIZE	8388608	// storage size for events from mulitple frames
#define EBFRSIZE	65536	// storage size for events from mulitple frames
#define EFRMSIZE	65536	// storage size for events per frame
#define EVE_MAXRAW	10	// max. number of stored pixels in a cluster
#define	MAXCHANNEL	300	// maximum channels to support
#define	MAXLINES	600	// maximum lines to support
#ifndef TRUE
# define TRUE		1
#endif

#define NOSHUTTER		// if NOSHUTTER is defined, no shutter support
				//	   is included

#define LINEAR		1
#define	LOG		2

#define CALTHRESH	1
#define EVETHRESH	2

#define NEWDATA		1
#define OLDDATA		2

#define PEFRECORD	1
#define	PIXELDISPLAY	2
#define	EVENTDISPLAY	3
#define	CMODEDISPLAY	4
#define	XSCATTERPLOT	5
#define	YSCATTERPLOT	6
#define	SPECTRUMDISPLAY	7
#define	PLUGINDISPLAY	8
#define YSPLITDISPLAY	9


#define GOOD		0
#define BAD_NOISE	1
#define BAD_EMPTY	2
#define BAD_CHAN	3
#define BAD_COLD	4
#define BAD_SATUR	5
#define BAD_USER	6

#define BLACK		0
#define RED		1
#define GREEN		2
#define WHITE		3
#define BLUE		4
#define MAGENTA		5
#define PINK		6

#define	TOFILE		1
#define	TOPRINTER	2

#define NO_FIT		0
#define GAUSS_FIT	1
#define GAUSSA_FIT	2

#define NO_WEIGHT	0
#define EXP_WEIGHT	1
#define REC_WEIGHT	2

//

#endif // XONLINECONSTANTS_H

