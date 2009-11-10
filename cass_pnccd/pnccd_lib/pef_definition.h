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

#include "xonline_constants.h"

#ifndef __PEF_H
#define __PEF_H

typedef struct pef_fh_t {
	short		npefh;		/* size of struct ? */
	short		nchan;		/* number of channels */
	short		nrow;		/* number of rows */
	short		unused;		// alignement filler
	long		nevtot;		/* number of events */
	long		tbeg;		/* start time in seconds */
	long		tend;		/* end time in seconds */
	short		sigma;		/* mean noise sigma leading factor */
	short		sigma_n;	/* ??? */
	short		bcut;		/* cut in noise histogram? == ZERO */
	short		badpx;		/* nr. of bad pixels in map */
	char		date[24];	/* date ? */
	short		nres;		/* norm factor for amplitude */
	short		resoff;		/* norm factor for offset */
	short		lstart;		/* first line in window mode */
// next lines added by PxH
	float		tempbeg;	// termperature reading at begin and...
	float		tempend;	// end of the recroding in [mV]

// size adjusted by PxH
	char		rest[64];	/* info string ? */
} pef_fh_t;



typedef struct pef_t {
	short		neigh;
	short 		id;
  	int		ampl;
	int		tsec;
	int		tusec;
	short		x;
	short		y;
	int		index;
	unsigned short	rawx[EVE_MAXRAW], rawy[EVE_MAXRAW], rawv[EVE_MAXRAW];
} pef_t;


// Raw event type for event storage in a file:

typedef struct raw_evt_t {
    unsigned short x;        // x coordinate
    unsigned short y;        // y coordinate
    unsigned int   amp;      // signal amplitude in adu
    unsigned int   index;    // running index of the event
    unsigned int   frm_idx;  // index of the frame where the event
                             // was detected
    unsigned int   tsec;     // frame timestamp seconds
    unsigned int   tusec;    // frame timestamp microseconds
} raw_evt_t;

/*****************************************************************

File structure for pef data:

HE                   // Beginning of the file header
VERSION <int>        // Version of the data format
ID <char*>           // Data set ID
CF <char*>           // Calibration file ID
TH <double>          // Event threshold
SI <int> <int>       // Frame size width and height
DT <int> <int> <int> // Date year, month, day
TM <int> <int> <int> // Time hours, minutes, seconds
SF <int> <int> <int> // Start of new frame with index, seconds and
                     // microseconds from time stamp
HT <int> <int> <int> // Signal pixel hit with x, y and amplitude
.                    //
.                    // Hits of the frame which started with SF
.                    //
SF <int> <int> <int> // Start of next frame with index, seconds and
                     // microseconds from time stamp
HT <int> <int> <int> // First signal pixel hit of the new frame
                     // with x, y and amplitude

*****************************************************************/

/* PxH prototypes */

/*
void	pefRwrite(void), pefRpause(void), pefRstop(int),
	pefRopen(void *, char *),
	pefRecord(frameHeaderType *);
*/

/* globals */

/*
extern pef_fh_t		PefHeader;
*/

#endif /* __PEF_H */
