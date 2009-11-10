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


// xonline_data_types.h
// Declare the types of data which are exchanged
// between the data handling and allocation objects.

#include "xonline_constants.h"
#include "pef_definition.h"
#include "fformat.h"

#ifndef XONLINEDATATYPES_H
#define XONLINEDATATYPES_H

//
// Display parameters:
//



// Calibration information for a frame set:

typedef struct
	{
	long		StartTime;	// time of first calibration frame
	int		firstFrame;	// first index of calibration frames
	int		lastFrame;	// last index of calibration frames
	int		nBadCold;	// number of too cold pixels in BadMap
	int		nBadNoisy;	// number of too noisy pixels in BadMap
	int		nBadEmpty;	// number of pixels in badmap
					// due to too view counts
	int		nBadSatur;	// number of pixels above upper limit in badmap
	int		nEmpty;		// total number of empty pixels,
					// in calibration frames
	int		nEvents;	// number of events of last frame,
					// in calibration data respectively
	int		nRejFrames;	// number of rejected frames
	double		meanSigma;	// mean noise in sigma of all pixels
        double          meanOffset;     // mean offset of all pixels in adu

	double		tStart, tStop;	// start/stop temperature of calibration
	} infoType;

// Statistical data for a pixel:

typedef struct
{
//    int	   count;	// internal use
    double sum;		// internal use
//    pxType mean;	// offset mean value of pixel (common mode corrected)
    double offset;	// offset mean value of pixel (raw)
    double sigma;       // noise sigma value of pixel
    double sumSq;       // internal use
    int	   count;	// internal use
    pxType mean;	// offset mean value of pixel (common mode corrected)

} staDataType;

// Parameters for the event analysis:

typedef struct
	{
	int		minEnergy, maxEnergy;		// active energy band
	int		leftChannel, rightChannel;	// active event region
	int		lowerLine, upperLine;		// active event region
	int		mipThresh;			// threshold to tag MIPs
	int		minClu, maxClu;			// minimum/maximum cluster size
	double		threshold;			// event cut in sigma
	double		qlin;				// quadratic term for linearity correction
	char		isoEvents;			// flag for isolated events selection
	char		firstEvents;			// flag for first event/channel only
	char		badShapes;			// flag to exclude bad event shapes
	} eventPropsType;

// Newly included data types, 12 Feb. 2008, these belong here, not in frame data!

/* new value! */
#define  EMPTYPIX	0x00

// structure type event buffer
typedef struct myType
	{
	float cv, cx, cy;		// coordinates of joined event // added in 2.9.0b7
	struct myType *next, *prev;	// neighboring events
	unsigned short cluster;		// add with 2.9.0b7,  all pixels of same event
					// have same cluster number
	short x, y, q;			// quality of pixel is stored in q
	int value;			// ADC signal of pixel
        int aux_value;                  // Auxiliary value for temporary
                                        // storage of calibration results
        int corrval;                    // The gain and CTE corrected ADC signal
	short	rawx[EVE_MAXRAW],	// raw data of the event cluster
		rawy[EVE_MAXRAW],
		rawv[EVE_MAXRAW];
// abs(q) is number of neighbors
// q = 1 very good, single pixel event :-)
// q > 1 split, but good event
// q < 0 split, bad, probably particle track event
// q = 0 should never happen (this may change)
	} eventType;

// Enumeration for the selection of the event signal
// pulse height value type:

typedef enum { EVT_RAWV, EVT_AUXV, EVT_CORRV } EVT_PHGHT_TYPE;

// event bookkeeping info
typedef struct
	{
	int		maxNEvents;	 // maximum number of events per frame
	int		count;		 // total number of events stored
        int             rawCount;        // number of stored raw events
	int		frameCount;	 // number of events in current frame
	int		mHits;		 // total number of multi-hit events
	int		dLeftSplit;	 // number of doubles, split in forward direction	// PxH from long to int, 17-Aug-07
	int		dRightSplit;	 // dito in backward direction				// PxH from long to int, 17-Aug-07
	int		dBackwardSplit;	 // number of doubles, split in forward direction	// PxH from long to int, 17-Aug-07
	int		dForwardSplit;	 // dito in backward direction				// PxH from long to int, 17-Aug-07
        int             nSplitMultip[4]; // New split event info, NiK 08/05/29

        raw_evt_t       *curr_rawevt;    // pointer to current raw event buffer
                                         // position
	eventType	*current;	 // pointer to current event struct
        raw_evt_t       *rawevts;        // pointer to raw events for later
                                         // file storage
	eventType	*frame;		 // raw events of current frame
	eventType	*corr;		 // corrected events of current frame
	eventType	*begin;		 // pointer to begin of event buffer 
	eventType	*last;		 // pointer to last entry of  event buffer
	} eventInfoType;





#endif // XONLINEDATATYPES_H

