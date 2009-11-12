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


// pix_event_data.h
// PixEventData provides an interface to pixel event
// data including recombined pixel events. It extracts
// pixel events from a frame if the calibration results
// for common mode, offset and noise are known for each
// pixel. PixelEventData also provides routines for the
// calibration of pixel events including the gain and cti
// values of the channels. PixelEventData provides analysis
// products created from the pixel event data like spectra,
// event count maps, scatter plots and so forth.
// EventCalibPrms is a data structure that holds both the
// input parameters of the event calibration and the
// main calibration products.

#ifndef PIXEVENTDATA_H
#define PIXEVENTDATA_H

#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <cmath>
#include <limits>

// #include "askuserdiags.h"

// external c headers, specify the linkage:

extern "C" {
#include "xonline_data_types.h"
#include "fformat.h"
}

class PixEventData {
public:
// Enumeration type for the selection of the frame analysis
// method:
    typedef enum { NOCMMD_NOEVT, CMMD_NOEVT,
		   NOCMMD_EVT, CMMD_EVT } FRMANL_MODE;
//
    PixEventData(void);
    ~PixEventData();
// Set the current frame to analyze. After the events are extracted,
// a flag is set to avoid double analysis of the frame.
    int setCurrentFrame(shmBfrType* frame, int width, int height);
// Set the analysis mode for the data frame:
    bool setFrmAnalysisMode(FRMANL_MODE mode);
// Analyze the current frame by extracting the pixel events and
// by performing the pixel event analysis. Decide whether cluster
// events are accepted and whether only the first event in each
// channel is selected.
    int analyzeCurrentFrame(void);
// Stop or start frame processing:
    void setStopProcessingFlag(bool stop);
// Set the pixel statistics map of the detector frame. This maps
// contains all the needed dark frame calibration data from FrameData:
    bool setFramePixStatMap(staDataType* pixStatMap,
			    int width, int height);
// Set the bad pixel map. The bad pixel map is created by the pixel
// statistics calibration class:
    int setFrameBadPixMap(char *badmap, int width, int height);
// Set the address of the buffer where the pixel signals
// of the currently analyzed frame are stored:
    int setPixSignalBfrAddr(pxType *pix_signals, int width, int height);
// Clear the analysis results:
    void clearEvtAnalysisResults(void);
// Set the analysis parameters:
// Set the default parameters:
    void setDefaultAnlParams(void);
// Set the number of ADCs, for the common mode
// correction:
    bool setNumberOfAdcs(const int& nadcs);
// The event threshold in multiples of the noise sigma of
// each pixel:
    bool setPixEventThreshold(double noisesigma_multiple);
// The lower energy selection limit:
    bool setMinEvtEnergyLimit(int min_evtenergy);
// The upper energy selection limit:
    bool setMaxEvtEnergyLimit(int max_evtenergy);
// The left channel:
    bool setLeftChannel(int leftchan);
// The right channel:
    bool setRightChannel(int rightchan);
// The lower line:
    bool setLowerLine(int lowerline);
// The upper line:
    bool setUpperLine(int upperline);
// The minimum and maximum multiplicity of accepted event
// clusters:
    bool setEvtClusterSizes(int min_size, int max_size);
// The upper ADC count threshold for X-ray events. Higher
// ADC signals are interpreted as MIP events.
    bool setMipEvtAdcValThresh(int mip_thresh);
// Set the factor for the quadratic linearity corretion term:
    bool setLinCorrectionFactor(double lin_corrval);
// Set the flag for the event type selection, 0 if events should
// not be recombined, 1 for isolated (single) events, 2 for event
// pattern recombination:
    bool setEvtRecombination(char recomb_flag);
// Set the flag for first event selection, i.e. only the first event
// in each channel is accepted, 0 means accept all events, 1 means
// accept only the first event of each channel:
    bool setFirstEvtSelection(bool firstevt_flag);
// Set the flag for the selection of event patterns with 'bad' shapes
// e.g. quadruples which are not in a 2x2 pixel area. 0 means accept
// bad shapes, 1 means reject bad shapes:
    bool setBadEvtShapeReject(bool reject_badshapes);
//
// The analysis results including offset corrected pixel
// signal maps and event lists:
//
    int        getNumEventsInFrame(void);
    int        getNumAllEvents(void);
    int        getNumSplitEvents(void);
    int        getNumSingleEvts(void);
    double     getPercSingleEvts(void);
    int        getNumDoubleEvts(void);
    double     getPercDoubleEvts(void);
    int        getNumTripleEvts(void);
    double     getPercTripleEvts(void);
    int        getNumQuadEvts(void);
    double     getPercQuadEvts(void);

    double     getPercLeftDoubles(void);
    double     getPercRightDoubles(void);
    double     getPercForwardDoubles(void);
    double     getPercBackwardDoubles(void);
    double     getLeftRightDoubleRatio(void);
    double     getFwrdBackwDoubleRatio(void);
    double     getAvrgEventsPerFrame(void);
//
// Frame data:
//
// The frame with common mode corrected pixel signals:
    pxType*    getPixelSignalFrame(void);
// The common mode values of the lines for each CAMEX:
    pxType**   getLineCmodeValues(int *nlines, int *ncmx);
//
// Event data:
//
// All valid events of all frames without CTI and
// gain correction:
    eventType* getAllUncorrEvents(int& num_events);
// All valid non-recombined raw events of the last frame
// without CTI or gain correction:
    raw_evt_t* getRawFrameEvents(int& num_events);
// All valid events of the last frame without
// CTI and gain correction
    eventType* getUncorrFrameEvents(int& num_events);
// All valid events of the last frame, with CTI
// and gain correction:
    eventType* getCorrectedFrameEvents(int& num_events);
//
// Private functions:
//
private:
// Extract the signal values of the pixels by the subtraction
// of the offset and the common mode values:
    int frameAnlNoCmmdNoEvt_(shmBfrType* frame,
			     int width, int height,
			     int n_cmodesteps);
// Extract the events above the event threshold from  a data frame:
    int frameAnlCmmdEvt_(shmBfrType* frame,
			 int width, int height, int n_cmodesteps);
// Calculate the common mode offset of a pixel line:
    pxType lineCommonMode_(pxType* pixval_line, pxType* evtthresh_line,
			   char* badflag_line, staDataType *pixstat_line,
			   int n_cmodesteps);
// Create an event threshold map with the given pixel statistics data:
    bool createEvtThreshMap_(void);
// Start the storage of pixel events:
    void startEventStorage_(void);
// Add a raw event to the raw event buffer:
    int storeRawEvent_(unsigned short pix_x,
		       unsigned short pix_y,
		       unsigned int   signal_value);
// Add an event to the event list:
    int storeFrameEvent_(short int pix_x, short int pix_y,
			 int signal_value);
// Carry out the event analysis with the events detected in the
// current frame:
    int  pixelEventAnalysis_(void);
    int  findIsolatedEvents_(void);
    int  findJoinedEvents_(void);
    int  findFirstEvents_(void);
// Apply the gain and CTI correction to an event:
    int correctEvtPulseHeight_(eventType* event);
// Select events in a given energy band:
    int selectEventsByEnergy_(void);
// Store the events in the current frame event buffer in the
// global event buffer for all frames:
    void storeFrameEventBuffer_(void);
// Allocate the arrays where event data is stored:
    bool allocEvtStorageResources_(void);
//  Allocate the memory for the common mode arrays:
    bool allocCmodeStorageArrays_(void);
// Clear the analysis data results:
    void clearAnalysisResults_(void);
// Initialize the storage array adresses, the analysis parameters
// and the analysis results:
    void initAnalysisResources_(void);
// Delete the frame and event data arrays:
    void clearAnalysisResources_(void);
//
// Private variables:
//

// The parent widget, it is needed for the error message widgets:
//    QWidget *parent_;
// Function pointers to the frame analysis routines:
    typedef int  (PixEventData::*SIG_EXTRACT)(shmBfrType *frame,
					      int width, int height,
					      int n_cmodesteps);
    SIG_EXTRACT frameAnalysisOp_;
    FRMANL_MODE analysis_flag_;
// Flag to indicate that a data frame is available:
    bool data_ready_;
// Flag to indicate if the frame last set has already been processed:
    bool frame_processed_;
// Flag to indicate whether processing should be continued:
    bool stop_processing_;
// Flag to indicate if the statistics data of the detector pixels
// is available:
    bool pixstats_set_;
    bool badmap_set_;
//
// Info and error messages:
//
    std::ostringstream info_msg_;
    std::ostringstream error_msg_;
// The data frame and its dimensions:
    int          frame_width_;
    int          frame_height_;
    int          number_adcs_;
    int          last_nadcs_;
    int          adc_channels_;
    int          frame_arraysize_;
    shmBfrType  *frame_buffer_;
    double       mean_frame_rate_;
// The signal frame, after common mode and offset correction:
    pxType      *pixsignal_buffer_;
    pxType       pix_minval_;
    pxType       pix_maxval_;
    infoType     frame_info_;
// The pixel statistics map which is needed as the dark frame
// calibration data:
    staDataType *pix_stat_map_;
// The event threshold map which is used to discriminate the pixels
// with X-ray events:
    pxType      *evtthresh_map_;
    double       event_threshold_; // multiplication factor of noise sigma
// The bad pixel map:
    char        *badpix_map_;
// The array of line common mode values for each CAMEX/ADC:
    pxType     **line_cmodes_;
//
// Analysis products, the event info structure:
//
    eventInfoType  event_info_;
    int            evtbuf_fillstate_;
    bool           evt_storage_alloc_;
    eventPropsType event_analysis_props_;
//
// Analysis products, split event statistics:
//
    double num_double_evts_;
    double perc_left_doubles_;
    double perc_right_doubles_;
    double perc_fwrd_doubles_;
    double perc_bckw_doubles_;
    double left_right_ratio_;
    double bckw_fwrd_ratio_;
// Average events per frame:
    double avrg_evts_frame_;
// The gain and CTI/CTE values of the detector:
    double         mean_channel_gain_;
    double        *channel_gain_map_;
    double         detector_cte_;
    double        *line_cte_table_;
// The y coordinates of the first events in each channel:
    int           *first_evt_coords_;
//
// The event hit matrix of the frame and the frame map of events:
//
    int         *event_hit_matrix_;
    eventType ***frame_event_map_;

};

// This structure is obsolete if all parameters and results are
// stored in PixEventData. Storing the individual parameters and
// values in PixEventData does not result in an overhead compared
// to assigning them to the elements of a structure.

// typedef struct {
//     ;
// } EventCcalibPrms;

#endif // PIXEVENTDATA_H

// Local Variables:
// coding: utf-8
// mode: C++
// c-file-style: "Stroustrup"
// fill-column: 100
// End:

// end of file pix_event_data.h





