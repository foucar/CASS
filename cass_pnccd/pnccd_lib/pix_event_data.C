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


// pix_event_data.C
// Definition of the class PixEventData. PixEventData takes a raw
// data frame and extracts the X-ray events from the raw data.
// The pixel vents are eventually grouped into event clusters and
// further processed to spectra, event maps etc. The calibration
// of the gain and CTI values of each channel is also performed
// by this class.

#include "pix_event_data.h"

PixEventData::PixEventData
(void)
{
    analysis_flag_     = NOCMMD_NOEVT;
    frameAnalysisOp_   = &PixEventData::frameAnlNoCmmdNoEvt_;
    frame_processed_   = false;
    stop_processing_   = false;
    data_ready_        = false;
    pixstats_set_      = false;
    badmap_set_        = false;
    evt_storage_alloc_ = false;
    frame_width_       = 0;
    frame_height_      = 0;
    number_adcs_       = 1;
    last_nadcs_        = 1;
    adc_channels_      = 0;
    frame_arraysize_   = 0;
    avrg_evts_frame_   = 0.0;
// Initialize the analysis parameters and the array
// addresses:
    this->initAnalysisResources_();
// Set the default analyis parameters:
    this->setDefaultAnlParams();
}

PixEventData::~PixEventData
()
{
// Delete the local storage arrays for events and
// frame data:
    clearAnalysisResources_();
}

int
PixEventData::setCurrentFrame
(shmBfrType* frame, int width, int height)
{
// Check if the size of the frame map makes sense or
// if the size of the frame differs from the size
// of the pixel statistics map:
    if( (width < 1) || (height < 1) ) return -1;
    if( (frame_width_ > 0) && (frame_height_ > 0) ) {
	if( (width!=frame_width_) || (height!=frame_height_) ) {
	    return -2;
	}
    }
// Assign the local frame pointer to the external frame
// data address. The validity and the correct size of the
// frame data array must be guaranteed:
    frame_buffer_    = frame;
    data_ready_      = true;
    frame_processed_ = false;
    return 0;
}

bool
PixEventData::setFrmAnalysisMode
(FRMANL_MODE mode)
{
// Choose the new analysis mode, return false if an undefined
// mode is set in the argument:
    switch( mode )
    {
	case NOCMMD_NOEVT:
	    frameAnalysisOp_ = &PixEventData::frameAnlNoCmmdNoEvt_;
	    break;
	case CMMD_NOEVT:
	    break;
	case NOCMMD_EVT:
	    break;
	case CMMD_EVT:
	    frameAnalysisOp_ = &PixEventData::frameAnlCmmdEvt_;
	    break;
	default:
	    return false;
	    break;
    }

    return true;
}

int
PixEventData::analyzeCurrentFrame
(void)
{
    int num_events;
// Check if the calibration data is set:
    if( (!badmap_set_) || (!pixstats_set_) || (!data_ready_) ) return -1;
// Check if the stop_processing_ flag is set:
    if( stop_processing_ ) return 0;
// Extract the pixel events from the current frame:

//    num_events = this->extractFrameEvents_(
//	frame_buffer_,frame_width_,frame_height_,2);
    num_events = (this->*(frameAnalysisOp_))(
	frame_buffer_,frame_width_,frame_height_,2);

// Analyze the extracted pixel events of the frame:
    num_events = this->pixelEventAnalysis_();

    frame_processed_ = true;
    return num_events;
}

void
PixEventData::setStopProcessingFlag
(bool stop)
{
    stop_processing_ = stop;

    return;
}

bool
PixEventData::setFramePixStatMap
(staDataType* pixStatMap, int width, int height)
{
// Check if the size of the pixel statistics map makes sense.
// The size of the pixel statistics map defines the size that
// all frame data and the bad pixel map must have.
    if( (width < 1) || (height < 1) ) return false;
// Set the frame size:
    frame_width_     = width;
    frame_height_    = height;
    adc_channels_    = width/number_adcs_;
    frame_arraysize_ = width*height;
// Set the used frame area in the event analysis properties:
    event_analysis_props_.leftChannel  = 0;
    event_analysis_props_.rightChannel = frame_width_ - 1;
    event_analysis_props_.lowerLine    = 0;
    event_analysis_props_.upperLine    = frame_height_ -1;
// Assign the local pixel statistics map pointer to the external
// address in the argument. The validity of the external address
// must be guaranteed:
    pix_stat_map_ = pixStatMap;
    pixstats_set_ = true;
// Now that the frame dimensions ar known, the storage resources
// can be allocated:
    if( !evt_storage_alloc_ ) allocEvtStorageResources_();
// Allocate the storage arrays for the common mode values:
    this->allocCmodeStorageArrays_();
// Create the event threshold map:
    createEvtThreshMap_();
    return true;
}

int
PixEventData::setFrameBadPixMap
(char *badmap, int width, int height)
{
// Check if the size of the bad pixel map makes sense or
// if the size of the bad pixel map differs from the size
// of the pixel statistics map:
    if( (width < 1) || (height < 1) ) return -1;
    if( (frame_width_ > 0) && (frame_height_ > 0) ) {
	if( (width!=frame_width_) || (height!=frame_height_) ) {
	    return -2;
	}
    }
// Assign the local bad pixel map pointer to the argument, the
// valididty of the externally stored bad pixel map must be
// guaranteed:
    badpix_map_ = badmap;
    badmap_set_ = true;
    return 0;
}

int
PixEventData::setPixSignalBfrAddr
(pxType *pix_signals, int width, int height)
{
// Check if the size of the signal map makes sense or
// if the size of the signal map differs from the size
// of the pixel statistics map:
    if( (width < 1) || (height < 1) ) return -1;
    if( (frame_width_ > 0) && (frame_height_ > 0) ) {
	if( (width!=frame_width_) || (height!=frame_height_) ) {
	    return -2;
	}
    }
// Assign the local pixel signal map pointer to the argument, the
// valididty of the externally stored pixel signal map must be
// guaranteed:
    pixsignal_buffer_ = pix_signals;
    return 0;
}

void
PixEventData::clearEvtAnalysisResults
(void)
{
    clearAnalysisResults_();
    return;
}

//
// Set the analysis parameters:
//

void
PixEventData::setDefaultAnlParams
(void)
{
    event_threshold_ = 4.0;
// The event analysis property structure:
    event_analysis_props_.minEnergy   = 0;
    event_analysis_props_.maxEnergy   = 16383;
    event_analysis_props_.mipThresh   = 16000;
    event_analysis_props_.minClu      = 1;
    event_analysis_props_.maxClu      = 9;
    event_analysis_props_.qlin        = 0.0;
    event_analysis_props_.isoEvents   = 0;
    event_analysis_props_.firstEvents = 0;
    event_analysis_props_.badShapes   = 0;

    return;
}

// Set the number of ADC boards or CAMEX chips:

bool
PixEventData::setNumberOfAdcs
(const int& nadcs)
{
  std::cout << "\n Number of adcs in PixEventData: "
	    << nadcs << std::flush;
  if( nadcs < 1 ) return false;

  number_adcs_  = nadcs;
  adc_channels_ = frame_width_/number_adcs_;
// Allocate the storage arrays for the common mode values:
  this->allocCmodeStorageArrays_();

  return true;
}

// Set the pixel event threshold

bool
PixEventData::setPixEventThreshold
(double noisesigma_multiple)
{
// Check the validity of the argument:
    if( noisesigma_multiple <= 0.00001 ) {
	error_msg_.str("");
	error_msg_ << "Error in setPixEventThreshold() in file: "
		   << __FILE__ << " , in line: " << __LINE__
		   << " , invalid selection of the event"
		   << " threshold. The event threshold must"
		   << " be larger than 0.00001, a threshold of"
		   << " 4.0 is used instead.";
//	AskUserDiags::askContinue(parent_,"Xonline",error_msg_.str());
	event_threshold_ = 4.0;
	return false;
    }
    event_threshold_ = noisesigma_multiple;
    this->createEvtThreshMap_();
    return true;
}

bool
PixEventData::setMinEvtEnergyLimit
(int min_evtenergy)
{
// Check the validity of the argument:
    if( (min_evtenergy < 0) ||
	(min_evtenergy >= (std::numeric_limits<int>::max() - 1)) ) {
	error_msg_.str("");
	error_msg_ << "Error in setMinEvtEnergyLimit() in file: "
		   << __FILE__ << " , in line: " << __LINE__
		   << " , invalid selection of the minimum"
		   << " event energy limit. The minimum event energy"
		   << " must be at least 0 and smaller than "
		   << (std::numeric_limits<int>::max() - 1)
		   << " . The value remains unchanged.";
//	AskUserDiags::askContinue(parent_,"Xonline",error_msg_.str());
	return false;
    }
    event_analysis_props_.minEnergy = min_evtenergy;
    return true;
}

bool
PixEventData::setMaxEvtEnergyLimit
(int max_evtenergy)
{
// Check the validity of the argument:
    if( (max_evtenergy <= 0) ||
	(max_evtenergy >= std::numeric_limits<int>::max()) ) {
	error_msg_.str("");
	error_msg_ << "Error in setMaxEvtEnergyLimit() in file: "
		   << __FILE__ << " , in line: " << __LINE__
		   << " , invalid selection of the maximum"
		   << " event energy limit. The maximum event energy"
		   << " must be larger than 0 and smaller than "
		   << (std::numeric_limits<int>::max())
		   << " 16383. The value remains unchanged.";
//	AskUserDiags::askContinue(parent_,"Xonline",error_msg_.str());
	return false;
    }
    event_analysis_props_.maxEnergy = max_evtenergy;
    return true;
}

bool
PixEventData::setLeftChannel
(int leftchan)
{
    event_analysis_props_.leftChannel = leftchan - 1;
    return true; 
}

bool
PixEventData::setRightChannel
(int rightchan)
{
    event_analysis_props_.rightChannel = rightchan - 1;
    return true;
}

bool
PixEventData::setLowerLine
(int lowerline)
{
    event_analysis_props_.lowerLine = lowerline - 1;
    return true;
}

bool
PixEventData::setUpperLine
(int upperline)
{
    event_analysis_props_.upperLine = upperline - 1;
    return true;
}

bool
PixEventData::setEvtClusterSizes
(int min_size, int max_size)
{
// Check the validity of the arguments:
    if( (min_size<1) || (min_size>max_size) || (max_size<1) ) {
	error_msg_.str("");
	error_msg_ << "Error in setEvtClusterSizes() in file: "
		   << __FILE__ << " , in line: " << __LINE__
		   << " , invalid selection of the cluster"
		   << " sizes. The minimum values are 1 and"
		   << " the maximum size must be larger or"
		   << " equal the minimum size. A minimum of 1"
		   << " and a maximum of 4 is chosen instead.";
//	AskUserDiags::askContinue(parent_,"Xonline",error_msg_.str());
	event_analysis_props_.minClu = 1;
	event_analysis_props_.maxClu = 4;
	return false;
    }
    event_analysis_props_.minClu = min_size;
    event_analysis_props_.maxClu = max_size;

    return true;
}

bool
PixEventData::setMipEvtAdcValThresh
(int mip_thresh)
{
// Check if the argument is valid:
    if( (mip_thresh < 1) ||
	(mip_thresh >= std::numeric_limits<int>::max()) ) {
	error_msg_.str("");
	error_msg_ << "Error in setMipEvtAdcValThresh() in file: "
		   << __FILE__ << " , in line: " << __LINE__
		   << " , invalid selection of the MIP"
		   << " threshold. The minimum value is 1."
		   << " A MIP threshold of 65535 is used instead";
//	AskUserDiags::askContinue(parent_,"Xonline",error_msg_.str());
	event_analysis_props_.mipThresh = 65535;
	return false;
    }
    event_analysis_props_.mipThresh = mip_thresh;
    return true;
}

bool
PixEventData::setLinCorrectionFactor
(double lin_corrval)
{
// Check if the argument is valid:
    if( lin_corrval < 0.0 ) {
	error_msg_.str("");
	error_msg_ << "Error in setLinCorrectionFactor() in file: "
		   << __FILE__ << " , in line: " << __LINE__
		   << " , invalid selection of the linearity"
		   << " correction factor. The minimum value is 0.0 ,"
		   << " the value remains unchanged. ";
//	AskUserDiags::askContinue(parent_,"Xonline",error_msg_.str());
	event_analysis_props_.qlin = 0.0;
	return false;
    }
    event_analysis_props_.qlin = lin_corrval;
    return true;
}

bool
PixEventData::setEvtRecombination
(char recomb_flag)
{
// Check if the argument is valid:
    if( (recomb_flag < 0) || (recomb_flag > 2) ) {
	error_msg_.str("");
	error_msg_ << "Error in setEvtRecombination() in file: "
		   << __FILE__ << " , in line: " << __LINE__
		   << " , invalid selection of the recombination flag"
		   << " only values from 0 to 2 are allowed."
		   << " The value remains unchanged. ";
//	AskUserDiags::askContinue(parent_,"Xonline",error_msg_.str());
	return false;
    }
    event_analysis_props_.isoEvents = recomb_flag;
    return true;
}

bool
PixEventData::setFirstEvtSelection
(bool firstevt_flag)
{
    if(firstevt_flag) event_analysis_props_.firstEvents = 1;
    else event_analysis_props_.firstEvents = 0;
    return true;
}

bool
PixEventData::setBadEvtShapeReject
(bool reject_badshapes)
{
    if(reject_badshapes) event_analysis_props_.badShapes = 1;
    else event_analysis_props_.badShapes = 0;
    return true;
}

//
// Get the analysis results:
//

int
PixEventData::getNumEventsInFrame
(void)
{
    return event_info_.frameCount;
}

int
PixEventData::getNumAllEvents
(void)
{
    return event_info_.count;
}

int
PixEventData::getNumSplitEvents
(void)
{
    return event_info_.mHits;
}

int
PixEventData::getNumSingleEvts
(void)
{
    return event_info_.nSplitMultip[0];
}

double
PixEventData::getPercSingleEvts
(void)
{
    if( event_info_.count > 0 ) {
	return (100.0*
		static_cast<double>(event_info_.nSplitMultip[0])/
		static_cast<double>(event_info_.count));
    }
    else return 0.0;
}

int
PixEventData::getNumDoubleEvts
(void)
{
    return event_info_.nSplitMultip[1];
}

double
PixEventData::getPercDoubleEvts
(void)
{
    if( event_info_.count > 0 ) {
	return (100.0*
		static_cast<double>(event_info_.nSplitMultip[1])/
		static_cast<double>(event_info_.count));
    }
    else return 0.0;
}

int
PixEventData::getNumTripleEvts
(void)
{
    return event_info_.nSplitMultip[2];
}

double
PixEventData::getPercTripleEvts
(void)
{
    if( event_info_.count > 0 ) {
	return (100.0*
		static_cast<double>(event_info_.nSplitMultip[2])/
		static_cast<double>(event_info_.count));
    }
    else return 0.0;
}

int
PixEventData::getNumQuadEvts
(void)
{
    return event_info_.nSplitMultip[3];
}

double
PixEventData::getPercQuadEvts
(void)
{
    if( event_info_.count > 0 ) {
	return (100.0*
		static_cast<double>(event_info_.nSplitMultip[3])/
		static_cast<double>(event_info_.count));
    }
    else return 0.0;
}

double
PixEventData::getPercLeftDoubles
(void)
{
    if( num_double_evts_ > 0.0 ) {
	return (100.0*
		static_cast<double>(event_info_.dLeftSplit)/
		num_double_evts_);
    }
    else return 0.0;
}

double
PixEventData::getPercRightDoubles
(void)
{
    if( num_double_evts_ > 0.0 ) {
	return (100.0*
		static_cast<double>(event_info_.dRightSplit)/
		num_double_evts_);
    }
    else return 0.0;
}

double
PixEventData::getPercForwardDoubles
(void)
{
    if( num_double_evts_ > 0.0 ) {
	return (100.0*
		static_cast<double>(event_info_.dForwardSplit)/
		num_double_evts_);
    }
    else return 0.0;
}

double
PixEventData::getPercBackwardDoubles
(void)
{
    if( num_double_evts_ > 0.0 ) {
	return (100.0*
		static_cast<double>(event_info_.dBackwardSplit)/
		num_double_evts_);
    }
    else return 0.0;
}

double
PixEventData::getLeftRightDoubleRatio
(void)
{
    if( event_info_.dRightSplit > 0 ) {
	return static_cast<double>(event_info_.dLeftSplit)/
               static_cast<double>(event_info_.dRightSplit);
    }
    else return 1.0;
}

double
PixEventData::getFwrdBackwDoubleRatio
(void)
{
    if( event_info_.dBackwardSplit > 0 ) {
	return static_cast<double>(event_info_.dForwardSplit)/
               static_cast<double>(event_info_.dBackwardSplit);
    }
    else return 1.0;
}

double
PixEventData::getAvrgEventsPerFrame
(void)
{
    return avrg_evts_frame_;
}

// Get the frame map with the pixel signals:

pxType*
PixEventData::getPixelSignalFrame
(void)
{
    return pixsignal_buffer_;
}

// Get the values of the line common modes for each
// CAMEX section of this frame:

pxType**
PixEventData::getLineCmodeValues
(int *nlines, int *ncmx)
{
// Set the values of the arguments:
    *nlines = frame_height_;
// ncmx and number_adcs_ have the same meaning, you
// can even define more CAMEX sections than in the
// detector setup:
    *ncmx   = number_adcs_;
// Return the address of the common mode matrix:
    return line_cmodes_;
}

// Get the array with all uncorrected events:

eventType*
PixEventData::getAllUncorrEvents
(int& num_events)
{
// Assign the number of all stored events:
// Warning: segfault possible:
//    num_events = event_info_.count;
// Better, evtbuf_fillstate_ is limited to EBFRSIZE:
    num_events = evtbuf_fillstate_;
// Return the event buffer address:
    return  event_info_.begin;
}

// Get the array address with the non-recombined raw
// events of  the last frame without gain or CTI
// correction:

raw_evt_t*
PixEventData::getRawFrameEvents
(int& num_events)
{
// Assign the number of detected events:
    num_events = event_info_.rawCount;
// Return the address of the raw event buffer:
    return event_info_.rawevts;
}

// Get the array with all uncorrected events of
// the last data frame:

eventType*
PixEventData::getUncorrFrameEvents
(int& num_events)
{
// Assign the number of events in the last buffer:
    num_events = event_info_.frameCount;
// Return the frame event buffer address:
    return event_info_.frame;
}

// Get the array with all corrected events of
// the last data frame:

eventType*
PixEventData::getCorrectedFrameEvents
(int& num_events)
{
// Assign the number of events in the last buffer:
    num_events = event_info_.frameCount;
// Return the frame event buffer address:
    return event_info_.corr;
}

//
// Definition of the private function members:
//

int
PixEventData::frameAnlNoCmmdNoEvt_
(shmBfrType* frame, int width, int height, int n_cmodesteps)
{
    char          *badflags;
    int            n_adc;
    int            pix_x, pix_y;
    pxType         common_mode, signal_value;
    pxType        *pixelval, *pix_evtthresh, *pix_signal;
    staDataType   *pixelstats;
// Return if the dark frame statistics (calibration) data is
// not set or if the current frame has already been processed:
    if( frame_processed_ || !pixstats_set_ ) return -1;
// Set the event number in the current frame to zero:
    frame_info_.nEmpty = 0;
    startEventStorage_();
// Assign the local pointers to the frame arrays:
    pixelval      = frame->px;
    pix_signal    = pixsignal_buffer_;
    pix_evtthresh = evtthresh_map_;
    badflags      = badpix_map_;
    pixelstats    = pix_stat_map_;
// Set the minima and maxima of the pixel signal map 
// to their start values:
    pix_minval_ = pix_maxval_ = 0;
// Process the window events:
//    QCoreApplication::processEvents();
// Decide whether to continue the event analysis:
    if( stop_processing_ )
    {
// Reset the counter for the number of stored raw events:
	event_info_.rawCount   = 0;
// Reset the counter for the event number in the current frame:
	event_info_.frameCount = 0;
// Set the pointer to the current raw event back to the beginning
// of the raw event buffer:
	event_info_.curr_rawevt = event_info_.rawevts;
// Set the pointer to the current uncorrected event back to the
// beginning of the storage array for uncorrected events in the
// current frame:
	event_info_.current    = event_info_.frame;
	return 0;
    }
// Loop over the lines of the frame:
    for( pix_y=0; pix_y<frame_height_; pix_y++ ) {
// Loop over the number of ADC boards:
	for( n_adc=0; n_adc<number_adcs_; n_adc++ ) {
// Determine the common mode offset for this line segment:
	    line_cmodes_[n_adc][pix_y] = common_mode = lineCommonMode_(
		pixelval,pix_evtthresh,badflags,pixelstats,n_cmodesteps);
// Loop over all pixels of the line segment:
	    for( pix_x=0; pix_x<adc_channels_; pix_x++,
		     pixelval++, pix_signal++, pix_evtthresh++,
		     badflags++, pixelstats++ ) {
		if( (!*badflags) && (common_mode>0) ) {
// Calculate the pixel signal which is the common mode subtracted from
// the pixel value:

//		    signal_value = *pixelval - common_mode - pixelstats->mean;
		    signal_value = *pixelval
			- static_cast<pxType>(pixelstats->offset);

// If the signal value is above the threshold, store the pixel
// signal as an event:

/*
		    if( signal_value < 0 )
		    {
			signal_value = 0;
		    }
*/

		    if( signal_value < pix_minval_ )
		    {
			pix_minval_ = signal_value;
		    }
		    else if( signal_value > pix_maxval_ )
		    {
			pix_maxval_ = signal_value;
		    }
		    *pix_signal = signal_value;
		}
		else {
		    *pix_signal = EMPTYPIX;
		}
	    }
	}
    }
// Return zero, no events were extracted:
    return 0;
}

int
PixEventData::frameAnlCmmdEvt_
(shmBfrType* frame, int width, int height, int n_cmodesteps)
{
    char          *badflags;
    int            n_adc;
    int            pix_x, pix_y;
    int            num_events;
    int            signal_to_store;
    pxType         common_mode, signal_value;
    pxType        *pixelval, *pix_evtthresh, *pix_signal;
    staDataType   *pixelstats;
// Return if the dark frame statistics (calibration) data is
// not set or if the current frame has already been processed:
    if( frame_processed_ || !pixstats_set_ ) return -1;
// Start the storage of events for the current frame:
    frame_info_.nEmpty = 0;
    startEventStorage_();
// Assign the local pointers to the frame arrays:
    pixelval      = frame->px;
    pix_signal    = pixsignal_buffer_;
    pix_evtthresh = evtthresh_map_;
    badflags      = badpix_map_;
    pixelstats    = pix_stat_map_;
// Set the minima and maxima of the pixel signal map and the event
// counter to their start values:
    num_events  = 0;
    pix_minval_ = pix_maxval_ = 0;

// Process the window events:
//    QCoreApplication::processEvents();
// Decide whether to continue the event analysis:
    if( stop_processing_ )
    {
// Reset the counter for the number of stored raw events:
	event_info_.rawCount   = 0;
// Reset the counter for the event number in the current frame:
	event_info_.frameCount = 0;
// Set the pointer to the current raw event back to the beginning
// of the raw event buffer:
	event_info_.curr_rawevt = event_info_.rawevts;
// Set the pointer to the current uncorrected event back to the
// beginning of the storage array for uncorrected events in the
// current frame:
	event_info_.current    = event_info_.frame;
	return 0;
    }

// Loop over the lines of the frame:
    for( pix_y=0; pix_y<frame_height_; pix_y++ ) {
// Loop over the number of ADC boards:
	for( n_adc=0; n_adc<number_adcs_; n_adc++ ) {
// Determine the common mode offset for this line segment:
	    line_cmodes_[n_adc][pix_y] = common_mode = lineCommonMode_(
		pixelval,pix_evtthresh,badflags,pixelstats,n_cmodesteps);
// Loop over all pixels of the line segment:
	    for( pix_x=0; pix_x<adc_channels_; pix_x++,
		     pixelval++, pix_signal++, pix_evtthresh++,
		     badflags++, pixelstats++ ) {
		if( (!*badflags) && (common_mode>0) ) {
// Calculate the pixel signal which is the common mode subtracted from
// the pixel value:
		    signal_value = *pixelval - common_mode - pixelstats->mean;
// If the signal value is above the threshold, store the pixel
// signal as an event:
		    if( signal_value > *pix_evtthresh ) {
			signal_to_store = static_cast<int>(signal_value);
// Store the event for later event export:
			storeRawEvent_(static_cast<unsigned short>(
					   pix_x + n_adc*adc_channels_),
				       static_cast<unsigned short>(
					   pix_y),
				       static_cast<unsigned int>(
					   signal_value));
// Store the event for later recombination and processing:
			storeFrameEvent_(static_cast<short int>(
					     pix_x + n_adc*adc_channels_),
					 static_cast<short int>(
					     pix_y),
					 signal_to_store);
			num_events++;
		    }

/*
		    if( signal_value < 0 ) signal_value = 0;
*/

		    if( signal_value < pix_minval_ ) pix_minval_ = signal_value;
		    else if( signal_value > pix_maxval_ ) pix_maxval_ = signal_value;
		    *pix_signal = signal_value;
		}
		else {
		    *pix_signal = EMPTYPIX;
		}
	    }
	}
    }
// Return the number of extracted events:
    return num_events;
}

pxType
PixEventData::lineCommonMode_
(pxType* pixval_line, pxType* evtthresh_line,
 char* badflag_line, staDataType* pixstat_line,
 int n_cmodesteps)
{
    char*          badflags;
    int            num_pixels, numevent_pix;
    int            pixelval_sum, eventval_sum, pix_x;
    pxType         cmode, cmode_prev;
    pxType        *pixelval, *pix_evtthresh;
    static pxType  pixval_max = 16383;
    staDataType   *pixelstats;

    cmode = -1;
// The first pass is always done. Events are not rejected because the
// event threshold is not yet calculated.
    pixelval     = pixval_line;
    badflags     = badflag_line;
    pixelstats   = pixstat_line;
    num_pixels   = 0;
    pixelval_sum = 0;

//    std::cout << "\n Common mode, start to loop over the pixels:" << std::flush;

    for( pix_x=0;
	 pix_x<adc_channels_;
	 pix_x++, pixelval++, badflags++, pixelstats++ ) {
// If a pixel has a value smaller than zero, skip the pixel and assign
// the maximum pixel value. The pixel will later be rejected as an event.
// Reject pixels with a bad flag.

/*
	std::cout << "\n Map values, pixel: " << std::flush;
	std::cout << *pixelval
		  << " bad map: " << *badflags << std::flush;
*/
	if( *pixelval < 0 )
	{
	    *pixelval = pixval_max;
	}
	if( !*badflags && (*pixelval > 0) ) {
	    num_pixels++;
	    pixelval_sum += (*pixelval - pixelstats->mean);
	}
    }
// If there are not enough accepted pixels in this line, skip it:
    if( num_pixels < 8 ) return cmode;
// Calculate the common mode as the mean value of the accepted pixels:
    cmode = static_cast<int>(rint(static_cast<double>(pixelval_sum)/
				  static_cast<double>(num_pixels)));
// Send an error message if the common mode is smaller or equal zero:
    if( cmode <= 0 ) {
	std::cerr << " Error in lineCommonMode_(): common mode = "
		  << cmode << " after initial iteration!" << std::endl;
    }
// Now continue with further iterations to reject events. Only do this
// if the number of iteration steps is larger than zero:
    while( n_cmodesteps-- ) {
	pixelval      = pixval_line;
	pix_evtthresh = evtthresh_line;
	badflags      = badflag_line;
	pixelstats    = pixstat_line;
	numevent_pix  = 0;
	eventval_sum  = 0;
	cmode_prev    = cmode;
// Filter out the pixels with event hits in the line:
	for( pix_x=0;
	     pix_x<adc_channels_;
	     pix_x++, pixelval++, pix_evtthresh++, badflags++, pixelstats++ ) {
// Select events only if they are not in a bad pixel:
	    if( !*badflags
		&& ((*pixelval - cmode) >
		    (*pix_evtthresh + pixelstats->mean)) ) {
		numevent_pix++;
		eventval_sum += (*pixelval - pixelstats->mean);
	    }
	}

/////////////////////////////////////////////////////////////////////////////
/*
	if( numevent_pix >= (adc_channels_ - 2) )
	{
	std::cerr << "\n Error in lineCommonMode_(): common mode = "
		  << cmode
		  << "\n accepted pixels = " << num_pixels
		  << " rejected pixels with events = "
		  << numevent_pix
		  << "\n pixel values in the line: \n\n";
	for( pix_x=0; pix_x<frame_width_; pix_x++ ) {
	    std::cerr.setf(std::ios::right);
	    std::cerr.width(5);
	    std::cerr << " " << pixval_line[pix_x] << " ";
	}
	std::cerr << "\n";
	}
*/
/////////////////////////////////////////////////////////////////////////////


// If no events were found, one can already quit:
	if( !numevent_pix ) break;
// Calculate the corrected common mode without contributions by the
// pixels with detected events:
	cmode =
	    static_cast<int>(rint(static_cast<double>(
				      pixelval_sum - eventval_sum)/
				  static_cast<double>(
				      num_pixels - numevent_pix)));
// If the common mode did not change, quit:
	if( cmode == cmode_prev ) break;
    }
// Send an error message if the common mode value is smaller or
// equal zero:
/*
    if( cmode <= 0 ) {
	std::cerr << "\n Error in lineCommonMode_(): common mode = "
		  << cmode
		  << "\n accepted pixels = " << num_pixels
		  << " rejected pixels with events = "
		  << numevent_pix
		  << "\n pixel values in the line: \n\n";
	for( pix_x=0; pix_x<frame_width_; pix_x++ ) {
	    std::cerr << " " << std::setw(5) << std::ios::right
		      << pixval_line[pix_x] << " ";
	}
	std::cerr << "\n";
    }
*/

// Return the common mode value:
//    std::cout << "\n common mode = " << cmode << std::flush;

    return cmode;
}

bool
PixEventData::createEvtThreshMap_
(void)
{
    int i, arraysize;
    double tmp_threshval;
    pxType* pix_evtthresh;
    staDataType* pix_stats;
// Calculate the frame array size and assign the array maps to their
// local pointers:
    arraysize     = frame_height_*frame_width_;
    if( (arraysize < 1) || !pixstats_set_ ) return false;
    pix_evtthresh = evtthresh_map_;
    pix_stats     = pix_stat_map_;
// Check the event threshold. This is normally already done when the 
// event threshold is set. In this case, event threshold means the
// noise sigma multiplication factor.
    if( event_threshold_ < 0.0 ) {
	    error_msg_.str("");
	    error_msg_ << "Error in createEvtThreshMap_() in file: "
		       << __FILE__ << " , in line: " << __LINE__
		       << " , invalid selection of the event"
		       << " threshold. The event threshold must"
		       << " be larger than zero.";
//	    AskUserDiags::askContinue(parent_,"Xonline",error_msg_.str());
	    return false;
    }
// Calculate the threshold value for each pixel of the frame:
    for( i=0; i<arraysize; i++, pix_stats++, pix_evtthresh++ ) {
//	tmp_threshval = pix_stats->mean + event_threshold_*(pix_stats->sigma);
	tmp_threshval = event_threshold_*(pix_stats->sigma);
// Limit the treshold value to a maximum value of 32767.0, the adc value
// of a pixel is a 16bit signed integer type:
	if( tmp_threshval > 32767.0 ) {
	    *pix_evtthresh = 32767;
	}
	else {
	    *pix_evtthresh = static_cast<pxType>(rint(tmp_threshval));
	}
    }
    return false;
}

void
PixEventData::startEventStorage_
(void)
{
// If this function is called for the first time, make shure that
// the storage arrays for the event data are allocated:
    if( !evt_storage_alloc_ ) allocEvtStorageResources_();
// Reset the counter for the number of stored raw events:
    event_info_.rawCount   = 0;
// Reset the counter for the event number in the current frame:
    event_info_.frameCount = 0;
// Set the pointer to the current raw event back to the beginning
// of the raw event buffer:
    event_info_.curr_rawevt = event_info_.rawevts;
// Set the pointer to the current uncorrected event back to the
// beginning of the storage array for uncorrected events in the
// current frame:
    event_info_.current    = event_info_.frame;
}

// Store a detected event in the data array for raw events:

int
PixEventData::storeRawEvent_
(unsigned short pix_x,
 unsigned short pix_y,
 unsigned int   signal_value)
{
    raw_evt_t *curr_rawevt;

// Increment the raw event counter:
    event_info_.rawCount++;
// Make shure that no more events are stored if the storage buffer
// is full:
    if( event_info_.rawCount >= EFRMSIZE ) {
	    error_msg_.str("");
	    error_msg_ << "Error in storeRawEvent_() in file: "
		       << __FILE__ << " , in line: " << __LINE__
		       << " , WARNING: The event buffer is full."
		       << " No more events can be stored. The buffer"
		       << " size must be increased!";
//	    AskUserDiags::askContinue(parent_,"Xonline",error_msg_.str());
	    return event_info_.rawCount;
    }
// Everything is ok, store the data in the event structure:
    curr_rawevt          = event_info_.curr_rawevt++;
    curr_rawevt->x       = pix_x;
    curr_rawevt->y       = pix_y;
    curr_rawevt->amp     = signal_value;
    curr_rawevt->index   = event_info_.rawCount;
    curr_rawevt->frm_idx = frame_buffer_->frH.index;
    curr_rawevt->tsec    = frame_buffer_->frH.tv_sec;
    curr_rawevt->tusec   = frame_buffer_->frH.tv_usec;
// Return with the increased number of stored events:
    return event_info_.rawCount;
}

// Store a detected event for this frame in the internal
// data structure:

int
PixEventData::storeFrameEvent_
(short int pix_x, short int pix_y, int signal_value)
{
    eventType    *current_event;
// Make shure that no more events are stored if the storage buffer
// is full:
    if( event_info_.frameCount >= EFRMSIZE ) {
	    error_msg_.str("");
	    error_msg_ << "Error in storeFrameEvent_() in file: "
		       << __FILE__ << " , in line: " << __LINE__
		       << " , WARNING: The event buffer is full."
		       << " No more events can be stored. The buffer"
		       << " size must be increased!";
//	    AskUserDiags::askContinue(parent_,"Xonline",error_msg_.str());
	    return event_info_.frameCount;
    }
// Everything is ok, store the data in the event structure:
    current_event          = event_info_.current++;
    current_event->x       = pix_x;
    current_event->y       = pix_y;
    current_event->value   = signal_value;
// Required for the new pulse height correction methods:
    current_event->q       = 1;
    current_event->rawx[0] = pix_x;
    current_event->rawy[0] = pix_y;
    current_event->rawv[0] = signal_value;

// Return with the increased number of stored events:
    return event_info_.frameCount++;
}

// Perform the event analysis, recombine events to patterns if this
// option is set:

int
PixEventData::pixelEventAnalysis_
(void)
{
// Decide if only isolated events should be accepted or event patterns
// should be accepted too:
    if( event_analysis_props_.isoEvents == 1 ) {
	findIsolatedEvents_();
    }
    if( event_analysis_props_.isoEvents == 2 ) {
	findJoinedEvents_();
    }
// Find the events which are first in a column. This test is performed
// in the member function findIsolatedEvents_() if only isolated/single
// events are accepted for the further analysis.
    else if( event_analysis_props_.firstEvents ) {
	findFirstEvents_();
    }
// Select the events in the energy range set by the user and store
// them in the buffer for all frame events:
    selectEventsByEnergy_();
    storeFrameEventBuffer_();
// Calculate the number of doubles in order to facilitate the
// calculation of the split factors for double events later:
    if( event_analysis_props_.isoEvents == 2 ) {
	num_double_evts_ = static_cast<double>(
	    event_info_.dLeftSplit     +
	    event_info_.dRightSplit    +
	    event_info_.dForwardSplit  +
	    event_info_.dBackwardSplit );
    }
// Calculate the gliding average of the event number per frame:
    avrg_evts_frame_ = (avrg_evts_frame_ +
			0.02*static_cast<double>(event_info_.frameCount))/
                        1.02;
// Return with the number of events in this frame:
    return event_info_.frameCount;
}

// Sort out and select the isolated or 'single' events from all events
// detected with extractFrameEvents_() :

int
PixEventData::findIsolatedEvents_
(void)
{
    int         stop_counter, stop_interval;
    int         i, mtx_idx, pix_x, pix_y;
    int         last_x, last_y;
    int         num_good_evts;
    eventType  *events, *good_events;

    stop_counter  = 0;
    stop_interval = 10000;

    last_x        = frame_width_  - 1;
    last_y        = frame_height_ - 1;
    num_good_evts = 0;
// Assign the event pointer to the first event in the frame:
    events = event_info_.frame;
// Build a hit matrix with a '1' at the pixel coordinates which have
// event hits:
    for( i=0; i<event_info_.frameCount; i++, events++ ) {
	mtx_idx = frame_width_*events->y + events->x;
	if( (mtx_idx>=frame_arraysize_) || (mtx_idx<0) ) {
	    std::cerr << "\n Error in findIsolatedEvents_():\n coordinates "
		      << events->x << " , " << events->y << " with index "
		      << mtx_idx << " are out of range!" << std::flush;
	    continue;
	}
	event_hit_matrix_[mtx_idx] = 1;
    }
// If only first events and only isolated events should be selected
// simultaneously, the member function findFirstEvents_() must be
// executed first:
    if( event_analysis_props_.firstEvents ) findFirstEvents_();
// Check the neighbourhood of all events to find the events
// without direct neighbours:
    events      = event_info_.frame;
    good_events = event_info_.frame;

    for( i=0; i<event_info_.frameCount; i++, events++ ) {
//
	if( stop_counter >= stop_interval )
	{
	    stop_counter = 0;
// Process the window events:
//	    QCoreApplication::processEvents();
// Decide whether to continue the event analysis:
	    if( stop_processing_ )
	    {
// Reset the counter for the number of stored raw events:
		event_info_.rawCount   = 0;
// Reset the counter for the event number in the current frame:
		event_info_.frameCount = 0;
// Set the pointer to the current raw event back to the beginning
// of the raw event buffer:
		event_info_.curr_rawevt = event_info_.rawevts;
// Set the pointer to the current uncorrected event back to the
// beginning of the storage array for uncorrected events in the
// current frame:
		event_info_.current    = event_info_.frame;
		return 0;
	    }
	}
	stop_counter++;
//
	pix_x = events->x;
	pix_y = events->y;
// Ignore the pixels at the border of the frame:
	if( (!pix_x) || (!pix_y) ||
	    (pix_x>=last_x) || (pix_y>=last_y) ) continue;

	mtx_idx = frame_width_*pix_y + pix_x;
// Are there neighbouring events in the same line:
	if( event_hit_matrix_[mtx_idx-1]                 ||
	    event_hit_matrix_[mtx_idx+1]                 ||
// Check for neighbouring events in the previous line:
	    event_hit_matrix_[mtx_idx-=frame_width_]     ||
	    event_hit_matrix_[mtx_idx-1]                 ||
	    event_hit_matrix_[mtx_idx+1]                 ||
// Check for neighbouring events in the following line:
	    event_hit_matrix_[mtx_idx+=(2*frame_width_)] ||
	    event_hit_matrix_[mtx_idx-1]                 ||
	    event_hit_matrix_[mtx_idx+1] ) continue;
// Also reject events with bad pixels as neighbours:

// Bad pixels in the following line:
	if( badpix_map_[mtx_idx-1]                       ||
	    badpix_map_[mtx_idx]                         ||
	    badpix_map_[mtx_idx+1]                       ||
// Bad pixels in the same line:
	    badpix_map_[mtx_idx-=frame_width_]           ||
	    badpix_map_[mtx_idx-1]                       ||
	    badpix_map_[mtx_idx+1]                       ||
// Bad pixels in the previous line:
	    badpix_map_[mtx_idx-=frame_width_]           ||
	    badpix_map_[mtx_idx-1]                       ||
	    badpix_map_[mtx_idx+1] ) continue;
// The event has no other events and no bad pixels as neighbours,
// store it as a good event:
	*good_events = *events;
// Set the quality/split multiplicity to 1:
	 good_events->q = 1;
// Set the raw event parameters:
	 good_events->rawx[0] = good_events->x;
	 good_events->rawy[0] = good_events->y;
	 good_events->rawv[0] = good_events->value;
// Increment the array addresses:
	 good_events++;
	 num_good_evts++;
    }
// Reset only the previously filled entries of the event hit matrix:

//WARNING: fails because the good events are moved to the beginning
// of the list and the original entries are lost!

/*
    events = event_info_.frame;
    for( i=0; i<event_info_.frameCount; i++, events++ ) {
	mtx_idx = frame_width_*events->y + events->x;
	if( (mtx_idx>=frame_arraysize_) || (mtx_idx<0) ) {
	    std::cerr << "\n Error in findIsolatedEvents_():\n coordinates "
		      << events->x << " , " << events->y << " with index "
		      << mtx_idx << " are out of range!" << std::flush;
	    continue;
	}
	event_hit_matrix_[mtx_idx] = 0;
    }
*/

// Reset all entries of the map, even if it takes more time!
    for( i=0; i<frame_arraysize_; i++ ) {
	event_hit_matrix_[i] = 0;
    }

// Return with the number of single/isolated events:
    event_info_.frameCount = num_good_evts;
    return num_good_evts;
}

// Selected all valid events (events excluding bad pixels) and
// recombine them to event patterns:

int
PixEventData::findJoinedEvents_
(void)
{
    bool        bad_pattern;
    int         stop_interval, stop_counter;
    int         i, index, pix_x, pix_y;
    int         last_x, last_y;
    int         event_count, pattern_count;
    int         mip_flag, pattern_q;
    int         min_x, sum_x, sum_y;
    int         max_signalvalue;
    static int  dx[4] = { -1,  1,  0, -1 };
    static int  dy[4] = {  0, -1, -1, -1 };
    float       corrected_signalvalue;
    //    double      qcorr_signalsum, qcorr_signalvalue;
    eventType  *events, *good_event, *evtmap_link;
    eventType  *neighbour_evt, *event_link, *pattern_main_evt;

    stop_interval = 10000;
    stop_counter  = 0;

    last_x        = frame_width_  -1;
    last_y        = frame_height_ -1;
    event_count   = 0;
    pattern_count = 0;
// Assign the map link to the first element of the frame event array:
    evtmap_link = event_info_.frame;
// Fill the event map of the detector frame with events at their
// pixel positions:
    for( i=0; i<event_info_.frameCount; i++, evtmap_link++ ) {
// Set to invalid event, to make shure it is avoided if it is not
// selected later:
	evtmap_link->q    = -1;
// Init the internal event links with zero values:
	evtmap_link->prev = 0;
	evtmap_link->next = 0;
// Set the event adress at the corresponding map position:
	frame_event_map_[evtmap_link->x][evtmap_link->y] = evtmap_link;
    }

// The first event scan, create lists of events in contiguous pixels:
    for( events=event_info_.frame; events<evtmap_link; events++ ) {
//
	if( stop_counter >= stop_interval )
	{
	    stop_counter = 0;
// Process the window events:
//	    QCoreApplication::processEvents();
// Decide whether to continue the event analysis:
	    if( stop_processing_ )
	    {
// Reset the counter for the number of stored raw events:
		event_info_.rawCount   = 0;
// Reset the counter for the event number in the current frame:
		event_info_.frameCount = 0;
// Set the pointer to the current raw event back to the beginning
// of the raw event buffer:
		event_info_.curr_rawevt = event_info_.rawevts;
// Set the pointer to the current uncorrected event back to the
// beginning of the storage array for uncorrected events in the
// current frame:
		event_info_.current    = event_info_.frame;
		return 0;
	    }
	}
	stop_counter++;
//

	for( i=0; i<4; i++ ) {
// Make sure that the map indices are not out of range:
	    if( (pix_x = events->x + dx[i]) < 0 ) continue;
	    if( (pix_y = events->y + dy[i]) < 0 ) continue;
// Look for neighbouring events. Note that the event map must be
// initialized with zeros and that all events must be cleared later.
	    if( (neighbour_evt = frame_event_map_[pix_x][pix_y]) ) {
		event_link = events;
// Sort the events if they are contiguous. The sorting order is the
// index in the array 'frame' which is a member of the structure
// eventType. In 'frame', the events are stored after a line by line
// scan of the data frame.
		while( neighbour_evt->next ) {
		    neighbour_evt = neighbour_evt->next;
		}
		while( event_link->prev ) {
		    event_link = event_link->prev;
		}
		if( neighbour_evt < event_link ) {
		    event_link->prev    = neighbour_evt;
		    neighbour_evt->next = event_link;
		}
	    }
	}
    }

// The events which form contiguous patterns are linked and sorted.
// Do a second iteration and evaluate the linked pixel events:
    for( events=event_info_.frame; events<evtmap_link; events++ ) {
//
	if( stop_counter >= stop_interval )
	{
	    stop_counter = 0;
// Process the window events:
//	    QCoreApplication::processEvents();
// Decide whether to continue the event analysis:
	    if( stop_processing_ )
	    {
// Reset the counter for the number of stored raw events:
		event_info_.rawCount   = 0;
// Reset the counter for the event number in the current frame:
		event_info_.frameCount = 0;
// Set the pointer to the current raw event back to the beginning
// of the raw event buffer:
		event_info_.curr_rawevt = event_info_.rawevts;
// Set the pointer to the current uncorrected event back to the
// beginning of the storage array for uncorrected events in the
// current frame:
		event_info_.current    = event_info_.frame;
		return 0;
	    }
	}
	stop_counter++;
//
	mip_flag    = 0;
	min_x       = 1000000;
	sum_x       = 0;
	sum_y       = 0;
	pattern_q   = 0;
	bad_pattern = false;
// Only begin at the pixel of a pattern which has the lowest index
// in 'frame':
	if( events->prev ) continue;
// Set the neighbour event to the current event:
	neighbour_evt = events;
// Iterate through the list of linked events:
	do {
// If one of the pixels is outside of the valid frame area, the event
// pattern is rejected:
	    if( (neighbour_evt->x < event_analysis_props_.leftChannel)  ||
		(neighbour_evt->x > event_analysis_props_.rightChannel) ||
		(neighbour_evt->y < event_analysis_props_.lowerLine)    ||
		(neighbour_evt->y > event_analysis_props_.upperLine)
		) bad_pattern = true;
// Increase the number of pixels in the pattern. pattern_q gives the size
// of the pattern and also its quality. Negative values of pattern_q
// indicate bad patterns.
	    pattern_q++;
// Calculate the coordinate sums to determine the elongation of the pattern
// in the 'x' and 'y' directions:
	    if( neighbour_evt->x < min_x ) min_x = neighbour_evt->x;
	    sum_x+=neighbour_evt->x;
	    sum_y+=neighbour_evt->y;
	    if( neighbour_evt->value >
		event_analysis_props_.mipThresh ) mip_flag = 1;

	} while( (neighbour_evt = neighbour_evt->next) );
// Continue if the pattern is flagged bad:
	if( bad_pattern ) continue;
// Calculate the new sums to determine the elongation of the pattern:
	sum_x-=( pattern_q*min_x );
	sum_y-=( pattern_q*events->y );
// If bad shapes should be rejected and the current pattern has a
// bad shape, mark the pattern as bad with a negative quality value:
	if( event_analysis_props_.badShapes ) {
	    if( sum_x > 2  ||
		sum_y > 2  ||
		sum_x < -2 ||
		sum_y < -2 ) pattern_q = -pattern_q;
	}
// If the event is marked as a MIP, also make pattern_q negative:
	if( mip_flag && (pattern_q > 0) ) pattern_q = -pattern_q;
// Set the pattern quality for all pixels of the pattern:
	neighbour_evt = events;
	do neighbour_evt->q = pattern_q;
	while( (neighbour_evt = neighbour_evt->next) );
// Count the number of double splits subdivided by their split
// orientation. The split orientation is specified as the orientation
// of the pixel with the smaller signal relative to the 'main' pixel
// with the larger signal:
	neighbour_evt = events->next;
/*
	if( (pattern_q == 2) && (events->x == neighbour_evt->x) ) {
	    if( events->value > neighbour_evt->value ) {
		++event_info_.dBackwardSplit;
	    }
	    else ++event_info_.dForwardSplit;
	}
	if( (pattern_q == 2) && (events->y == neighbour_evt->y) ) {
	    if( events->value > neighbour_evt->value ) {
		++event_info_.dRightSplit;
	    }
	    else ++event_info_.dLeftSplit;
	}
*/
    }

// Do the final evaluation of the single events and the event patterns.
// Finally store the events and patterns in the internal data structures
// for further analyses.
    good_event = event_info_.frame;
    for( events=event_info_.frame; events<evtmap_link; events++ ) {
//
	if( stop_counter >= stop_interval )
	{
	    stop_counter = 0;
// Process the window events:
//	    QCoreApplication::processEvents();
// Decide whether to continue the event analysis:
	    if( stop_processing_ )
	    {
// Reset the counter for the number of stored raw events:
		event_info_.rawCount   = 0;
// Reset the counter for the event number in the current frame:
		event_info_.frameCount = 0;
// Set the pointer to the current raw event back to the beginning
// of the raw event buffer:
		event_info_.curr_rawevt = event_info_.rawevts;
// Set the pointer to the current uncorrected event back to the
// beginning of the storage array for uncorrected events in the
// current frame:
		event_info_.current    = event_info_.frame;
		return 0;
	    }
	}
	stop_counter++;
//
	pix_x = events->x;
	pix_y = events->y;
// Remove the links in the frame event map. The links are only removed
// at the previous entries, this saves time compared to clearing every
// position of the map.
	frame_event_map_[pix_x][pix_y] = 0;
// Count the event patterns. An event pattern is a single event or a
// pattern of directly neighbouring events. All events of a pattern
// have the same pattern index.
	events->cluster = pattern_count;
// Always start processing at the first pixel of an event pattern:
	if( events->prev ) continue;
// The first event of the pattern is reached, increase the
// pattern count:
	++pattern_count;
// Issue a warning if the pattern quality is zero. This should never
// happen:
	if( events->q==0 ) {
	    std::cerr << "\n Error in findJoinedEvents_() in file: "
		      << __FILE__ << " , in line: " << __LINE__
		      <<" pattern_q has a value of zero!\n";
	}
// Start to process the event patterns. Process only those patterns
// which have a pattern multiplicity in the user selected range:
	if( (events->q >= event_analysis_props_.minClu) &&
	    (events->q <= event_analysis_props_.maxClu) ) {
// Set the start values of the event index, the maximum event
// signal pulse height in the pattern and the event with the maximum
// signal pulse height:
	    index            = 0;
	    max_signalvalue  = events->value;
	    pattern_main_evt = events;
// Treat the current event as a good event:
	    *good_event      = *events;
// Apply the gain and CTI correction to the event:
	    corrected_signalvalue = static_cast<float>(
		correctEvtPulseHeight_(events));
// Assign the corrected signal value and new coordinates to the
// good event for later use:
	    good_event->cv   = corrected_signalvalue;
	    good_event->cx   = corrected_signalvalue *
		static_cast<float>(events->x);
	    good_event->cy   = corrected_signalvalue *
		static_cast<float>(events->y);
	    good_event->rawx[index] = events->x;
	    good_event->rawy[index] = events->y;
	    good_event->rawv[index] = events->value;
	    ++index;
// Combine the events to a pattern:
	    if( events->q > 1 ) {
// Start at the first event of the pattern:
		neighbour_evt = events;
// Loop through all events of the pattern:
		while( neighbour_evt->next ) {
		    neighbour_evt = neighbour_evt->next;
// Limit the pattern size to avoid crossing array borders:
		    if( index < EVE_MAXRAW ) {
			good_event->rawx[index] = neighbour_evt->x;
			good_event->rawy[index] = neighbour_evt->y;
			good_event->rawv[index] = neighbour_evt->value;
		    }
		    ++index;
// Add the event pulse heights and weighted coordinates to the
// good event:
		    good_event->value += neighbour_evt->value;
		    corrected_signalvalue = static_cast<float>(
			correctEvtPulseHeight_(neighbour_evt));
		    good_event->cv += corrected_signalvalue;
		    good_event->cx += corrected_signalvalue *
			static_cast<float>(neighbour_evt->x);
		    good_event->cy += corrected_signalvalue *
			static_cast<float>(neighbour_evt->y);
// Find the maximum signal value and the main event which has the
// largest signal of all pattern members:
		    if( neighbour_evt->value > max_signalvalue ) {
			max_signalvalue = neighbour_evt->value;
			pattern_main_evt = neighbour_evt;
		    }
		}
// Increase the split event counter:
		++event_info_.mHits;
	    }
// Skip the event patterns, the main events of which are located
// on the outer rim of the frame:
	    if( !pattern_main_evt->x ||
		(pattern_main_evt->x >= (frame_width_ - 1)) ) continue;
	    if( !pattern_main_evt->y ||
		(pattern_main_evt->y >= (frame_height_ - 1)) ) continue;
// Calculated the position of the pattern center. The coordinates of
// the pattern center are the center of gravity. The middle of a pixel
// has the coordinates 0.5 , 0.5 :
	    good_event->cx = 0.5 + good_event->cx / good_event->cv;
	    good_event->cy = 0.5 + good_event->cy / good_event->cv;
// Set the pattern index to the index of the event with the maximum
// pulse height:
	    good_event->x  = pattern_main_evt->x;
	    good_event->y  = pattern_main_evt->y;
// Apply the non-linearity correction to the pulse heights of the
// events: (Skipped, new version applies qlin in the
// final correction)

/*
	    if( event_analysis_props_.qlin ) {
		qcorr_signalsum = 0.0;
		for( i=0; i<good_event->q; i++ ) {
		    qcorr_signalvalue =
			static_cast<double>(good_event->rawv[i]);
		    qcorr_signalsum+=qcorr_signalvalue + 
			event_analysis_props_.qlin*
			qcorr_signalvalue*qcorr_signalvalue;
		}
		good_event->value = static_cast<int>(
		    nearbyint(qcorr_signalsum));
	    }
*/

// Advance the good_event pointer and increase the event count:
	    ++good_event;
	    ++event_count;
	}
    }
// Return with the number of accepted pixel events:
    event_info_.frameCount = event_count;
    return event_count;
}

// Find and select the events which are first in
// a channel of the current frame. First means the events which
// are closest to the readout node of a channel.

int
PixEventData::findFirstEvents_
(void)
{
    int        i, evt_x, evt_y;
    int        good_count;
    eventType *events, *good_event;

// Set the event pointer to the beginning of the event storage
// array of the current frame:
    good_count = 0;
    events     = event_info_.frame;
// Set the y coordinates of the first events to a value larger
// than the array height:
    for( i=0; i<(frame_width_ + 1); i++ ) first_evt_coords_[i] = 65536;
// Find the firs event in each channel:
    for( i=0; i<event_info_.frameCount; i++, events++ ) {
	evt_x = events->x;
	evt_y = events->y;
// Check if the y coordinate of the events is smaller than the entry
// in the first event y coordinate array:
	if( evt_y<first_evt_coords_[evt_x] ) {
	    first_evt_coords_[evt_x] = evt_y;
	}
    }
// Keep only the first event of each channel, reset the event pointer
// to the beginning of the frame array:
    good_event = events = event_info_.frame;
    for( i=0; i<event_info_.frameCount; i++, events++ ) {
	if( first_evt_coords_[events->x] == events->y ) {
	    *good_event = *events;
	    good_event++;
	    good_count++;
	}
    }
// Set the frame count of events to the numbe rof first events
// and return with the number of first events:
    event_info_.frameCount = good_count;
    return good_count;
}

// Correct the pulse height of a pixel event with the gain and
// CTI tables:

int
PixEventData::correctEvtPulseHeight_
(eventType* event)
{
    double signal_value;

// The CTE is larger than one, something went wrong. Better do
// nothing and return with the uncorrected value:
    if( detector_cte_ >= 1.0 ) return event->value;
    signal_value = static_cast<double>(event->value);
// Apply the cte correction, the default value in the cte table
// entries is 1.0:
    if( detector_cte_ < 1.0 ) signal_value*=line_cte_table_[event->y];
// Apply the gain correction. If the gains are not yet determined,
// the gain map is filled with 1.0s as the default values:
    signal_value*=channel_gain_map_[event->x];
// Convert the corrected signal to a rounded int and return:
    return static_cast<int>(nearbyint(signal_value));
}

// Select only events which are in a given energy range:

int
PixEventData::selectEventsByEnergy_
(void)
{
    int        i;
    int        signal_value, good_count;
    eventType *events, *good_event;
// Set the event count to zero:
    good_count = 0;
// Set the event pointers to the beginning of the frame event
// array:
    events     = event_info_.frame;
    good_event = event_info_.frame;
// Loop through the previously detected events of the frame
// and find the ones in the given energy range:
    for( i=0; i<event_info_.frameCount; i++, events++ ) {
// Apply the gain and CTE corrections to the current event:
//	signal_value = correctEvtPulseHeight_(events);
	signal_value = events->value;
// Check whether the event fits in the energy band set by the user:
	if( signal_value < event_analysis_props_.minEnergy ||
	    signal_value > event_analysis_props_.maxEnergy ) continue;
// Check if the event lies in the user selected frame area:
	if( events->x < event_analysis_props_.leftChannel  ||
	    events->x > event_analysis_props_.rightChannel ||
	    events->y < event_analysis_props_.lowerLine    ||
	    events->y > event_analysis_props_.upperLine )    continue;
// Accept the event, add it to the good events and advance the
// good event pointer:
	*good_event = *events;
	good_event++;
	good_count++;
    }
// Return with the number of good events:
    event_info_.frameCount = good_count;
    return good_count;
}

// Store the events which were detected in the current frame in the
// event buffer for all events. The new events are added after the last
// event in the buffer:

void
PixEventData::storeFrameEventBuffer_
(void)
{
    int        i;
    eventType *corrected_evts, *source_evtbuffer;
    eventType *destination_evtbuffer;
    eventType *curr_evt;
// Set the pointers to their arrays:
    corrected_evts   = event_info_.corr;
    source_evtbuffer = event_info_.frame;
// Continue the storage after the last stored event of the
// previous frame. Jump to the beginning if the event buffer
// is full:
    destination_evtbuffer =
	&(event_info_.begin[event_info_.count % EBFRSIZE]);
// Loop through the events of the current frame and break
// if the number of events is larger than the maximum permitted
// number of events per frame:
    for( i=0; i<event_info_.frameCount; i++, corrected_evts++ ) {
	if( i>EFRMSIZE ) {
	    error_msg_.str("");
	    error_msg_ << "Error in storeFrameEventBuffer_() in file: "
		       << __FILE__ << " , in line: " << __LINE__
		       << " , too many events in this frame. The"
		       << " remaining events in this frame are ignored.";
	    std::cerr << "\n " << error_msg_.str() << std::flush;
	    break;
	}
// Set the initial values of the event pulse height:
	source_evtbuffer->corrval = source_evtbuffer->aux_value
	                          = source_evtbuffer->value;
	*corrected_evts           = *source_evtbuffer;
// Store the event and increment the array pointers:
	*destination_evtbuffer    = *source_evtbuffer;
// Count the number of events according to their
// multiplicity:
	if( (destination_evtbuffer->q > 0) &&
	    (destination_evtbuffer->q < 5) ) {
	    ++(event_info_.nSplitMultip[(destination_evtbuffer->q) - 1]);
	}
// Count the number of double splits subdivided by their split
// orientation. The split orientation is specified as the orientation
// of the pixel with the smaller signal relative to the 'main' pixel
// with the larger signal:
	if( destination_evtbuffer->q == 2 )
	{
	    curr_evt = destination_evtbuffer;
	    if( curr_evt->rawx[0] == curr_evt->rawx[1] ) {
		if( curr_evt->rawv[0] > curr_evt->rawv[1] ) {
		    ++event_info_.dBackwardSplit;
		}
		else ++event_info_.dForwardSplit;
	    }
	    if( curr_evt->rawy[0] == curr_evt->rawy[1] ) {
		if( curr_evt->rawv[0] > curr_evt->rawv[1] ) {
		    ++event_info_.dRightSplit;
		}
		else ++event_info_.dLeftSplit;
	    }
	}
// Increment the buffer pointers:
	destination_evtbuffer++;
	source_evtbuffer++;
// Jump to the beginning of the event buffer if its end is reached:
	if( destination_evtbuffer>=event_info_.last ) {
	    destination_evtbuffer = event_info_.begin;
	}
    }
// Add the number of events in the current frame to the total
// number of events:
    event_info_.count += event_info_.frameCount;
// Increment the fill state of the event buffer:
    if( (evtbuf_fillstate_ + event_info_.frameCount) >= EBFRSIZE ) {
	evtbuf_fillstate_ = EBFRSIZE - 1;
    }
    else {
	evtbuf_fillstate_ += event_info_.frameCount;
    }

    return;
}



// Allocate the data structures needed for the storage of pixel events:

bool
PixEventData::allocEvtStorageResources_
(void)
{
    int i, j;
// Exit if the allocation is already finished:
    if( evt_storage_alloc_ ) return false;

// Allocate the array for the storage of the gain values of
// the detector channels:
    if( channel_gain_map_ ) delete[] channel_gain_map_;
    channel_gain_map_ = new double[frame_width_ + 1];
    if( !channel_gain_map_ ) {
	error_msg_.str("");
	error_msg_ << "Error in allocEvtStorageResources_() in file: "
		   << __FILE__ << " , in line: " << __LINE__
		   << " , could not allocate the array for the"
		   << " gain values of the channels.";
//	AskUserDiags::askContinue(parent_,"Xonline",error_msg_.str());
	return false;
    }
// fill the array with default values and set the default value for the
// channel gain:
    mean_channel_gain_ = 1.0;
    for( i=0; i <= frame_width_; i++ ) {
	channel_gain_map_[i] = 1.0;
    }
// Allocate the array for the storage of the cte values of
// the detector channels:
    if( line_cte_table_ ) delete[] line_cte_table_;
    line_cte_table_ = new double[frame_width_ + 1];
    if( !line_cte_table_ ) {
	error_msg_.str("");
	error_msg_ << "Error in allocEvtStorageResources_() in file: "
		   << __FILE__ << " , in line: " << __LINE__
		   << " , could not allocate the array for the charge"
		   << " transfer efficiency values of the channels.";
//	AskUserDiags::askContinue(parent_,"Xonline",error_msg_.str());
	return false;
    }
// fill the array with default values and set the default value for the
// channel CTE:
    detector_cte_ = 1.0;
    for( i=0; i <= frame_width_; i++ ) {
	line_cte_table_[i] = 1.0;
    }
// Allocate the array for the storage of the y coordinates
// of the first events in each channel:
    if( first_evt_coords_ ) delete[] first_evt_coords_;
    first_evt_coords_ = new int[frame_width_ + 1];
    if( !first_evt_coords_ ) {
	error_msg_.str("");
	error_msg_ << "Error in allocEvtStorageResources_() in file: "
		   << __FILE__ << " , in line: " << __LINE__
		   << " , could not allocate the array for the y"
		   << " coordinates of the first events in each channel.";
//	AskUserDiags::askContinue(parent_,"Xonline",error_msg_.str());
	return false;
    }
// Allocate the buffer for the storage of photon events, plus one spare
// element:
    if( event_info_.begin ) delete[] event_info_.begin;
    event_info_.begin = new eventType[EBFRSIZE + 1];
    if( !event_info_.begin ) {
	error_msg_.str("");
	error_msg_ << "Error in allocEvtStorageResources_() in file: "
		   << __FILE__ << " , in line: " << __LINE__
		   << " , could not allocate the storage buffer"
		   << " for photon signal events.";
//	AskUserDiags::askContinue(parent_,"Xonline",error_msg_.str());
	return false;
    }
// Allocate the buffer for raw events events, plus one spare element:
    if( event_info_.rawevts ) delete[] event_info_.rawevts;
    event_info_.rawevts = new raw_evt_t[EFRMSIZE + 1];
    if( !event_info_.rawevts ) {
	error_msg_.str("");
	error_msg_ << "Error in allocEvtStorageResources_() in file: "
		   << __FILE__ << " , in line: " << __LINE__
		   << " , could not allocate the storage buffer"
		   << " for raw photon events.";
//	AskUserDiags::askContinue(parent_,"Xonline",error_msg_.str());
	return false;
    }
// Allocate the buffer for uncorrected photon events in the current
// frame, plus one spare element:
    if( event_info_.frame ) delete[] event_info_.frame;
    event_info_.frame = new eventType[EFRMSIZE + 1];
    if( !event_info_.frame ) {
	error_msg_.str("");
	error_msg_ << "Error in allocEvtStorageResources_() in file: "
		   << __FILE__ << " , in line: " << __LINE__
		   << " , could not allocate the storage buffer"
		   << " for photon signal events in the current frame.";
//	AskUserDiags::askContinue(parent_,"Xonline",error_msg_.str());
	return false;
    }
// Allocate the buffer for corrected photon events in the current
// frame, plus one spare element:
    if( event_info_.corr ) delete[] event_info_.corr;
    event_info_.corr = new eventType[EFRMSIZE + 1];
    if( !event_info_.corr ) {
	error_msg_.str("");
	error_msg_ << "Error in allocEvtStorageResources_() in file: "
		   << __FILE__ << " , in line: " << __LINE__
		   << " , could not allocate the storage buffer"
		   << " for gain and cti corrected photon signal"
		   << "  events in the current frame.";
//	AskUserDiags::askContinue(parent_,"Xonline",error_msg_.str());
	return false;
    }
// Allocate the buffer for pixel signals. It has the same size as the
// frame buffer:

//    if( pixsignal_buffer_ ) delete[] pixsignal_buffer_;
//    pixsignal_buffer_ = new pxType[frame_arraysize_];
//    if( !pixsignal_buffer_ ) {
//	error_msg_.str("");
//	error_msg_ << "Error in allocEvtStorageResources_() in file: "
//		   << __FILE__ << " , in line: " << __LINE__
//		   << " , could not allocate the storage buffer"
//		   << " for the common mode and offset corrected"
//		   << " signals od the current frame.";
//	AskUserDiags::askContinue(parent_,"Xonline",error_msg_.str());
//	return false;
//    }

// Allocate the event threshold map. It has the same size as the
// frame buffer:
    if( evtthresh_map_ ) delete[] evtthresh_map_;
    evtthresh_map_ = new pxType[frame_arraysize_];
    if( !evtthresh_map_ ) {
	error_msg_.str("");
	error_msg_ << "Error in allocEvtStorageResources_() in file: "
		   << __FILE__ << " , in line: " << __LINE__
		   << " , could not allocate the storage buffer"
		   << " for the event thersholds of each pixel.";
//	AskUserDiags::askContinue(parent_,"Xonline",error_msg_.str());
	return false;
    }
// Allocate the event hit matrix which is used to select isolated/single
// events. Set all entries to a value of zero after allocation.
    if( event_hit_matrix_ ) delete[] event_hit_matrix_;
    event_hit_matrix_ = new int[frame_arraysize_ + 1];
    if( !event_hit_matrix_ ) {
	error_msg_.str("");
	error_msg_ << "Error in allocEvtStorageResources_() in file: "
		   << __FILE__ << " , in line: " << __LINE__
		   << " , could not allocate the event hit"
		   << " matrix for the selection of single"
		   << " events.";
//	AskUserDiags::askContinue(parent_,"Xonline",error_msg_.str());
	return false;
    }
// Initialize all entries with zero values:
    for( i=0; i<=frame_arraysize_; i++ ) {
	event_hit_matrix_[i] = 0;
    }
// Allocate the frame event map which contains pointers to all
// events of the frame at a position which is the equivalent
// of the pixel coordinates of each event.
    if( frame_event_map_ ) {
	for( i=0; i<(frame_width_ + 1); i++ ) {
	    delete[] frame_event_map_[i];
	}
	delete[] frame_event_map_;
    }
    frame_event_map_ = new eventType**[frame_width_ + 1];
    for( i=0; i<(frame_width_ + 1); i++ ) {
	frame_event_map_[i] = new eventType*[frame_height_ + 1];
    }
// Set all values to zeros:
    for( i=0; i<(frame_width_ + 1); i++ ) {
	for( j=0; j<(frame_height_ + 1); j++ ) {
	    frame_event_map_[i][j] = 0;
	}
    }
// Set the members of event_info_ to their start values:

// curr_rawevt is the pointer to the last stored raw event
// in the raw event buffer:
    event_info_.curr_rawevt    = event_info_.rawevts;
// 'current' serves as the pointer to the current event. Set it
// to the beginning of 'frame', the storage buffer for all
// events in the current data frame:
    event_info_.current        = event_info_.frame;
// 'last' points to the last buffer element for the events extracted
// from all analyzed frames:
    event_info_.last           = event_info_.begin + EBFRSIZE;
// The number of all extracted events:
    event_info_.count          = 0;
// The number of extracted raw events:
    event_info_.rawCount       = 0;
// The fill state of the event buffer:
    evtbuf_fillstate_          = 0;
// The numbers of double splits with their direction:
    event_info_.dLeftSplit     = 0;
    event_info_.dRightSplit    = 0;
    event_info_.dBackwardSplit = 0;
    event_info_.dForwardSplit  = 0;
    for( i=0; i<4; i++ ) event_info_.nSplitMultip[i] = 0;
// Set the storage array allocation flag to true:
    evt_storage_alloc_ = true;

    return true;
}

// Allocate the storage arrays for the common mode values:

bool
PixEventData::allocCmodeStorageArrays_
(void)
{
    int        i;

// Allocate the array for the storage of the common mode values of
// the lines in the current frame:

// Delete them first if the line common mode arrays were already
// allocated:
    if( line_cmodes_ ) {
	for( i=0; i<last_nadcs_; i++ ) {
	    if( line_cmodes_[i] ) delete[] line_cmodes_[i];
	}
	delete[] line_cmodes_;
    }
// Allocate the new arrays with the correct  size:
    line_cmodes_ = new pxType*[number_adcs_];
    if( !line_cmodes_ ) {
	error_msg_.str("");
	error_msg_ << "Error in allocCmodeStorageArrays_() in file: "
		   << __FILE__ << " , in line: " << __LINE__
		   << " , could not allocate the array for the"
		   << " common mode values of the frame lines.";
//	AskUserDiags::askContinue(parent_,"Xonline",error_msg_.str());
	return false;
    }
    last_nadcs_ = number_adcs_;
    for( i=0; i<number_adcs_; i++ ) {
	line_cmodes_[i] = new pxType[frame_height_ + 1];
	if( !line_cmodes_[i] ) {
	    error_msg_.str("");
	    error_msg_ << "Error in allocCmodeStorageArrays_() in file: "
		       << __FILE__ << " , in line: " << __LINE__
		       << " , could not allocate the array for the"
		       << " common mode values of the frame lines.";
//	    AskUserDiags::askContinue(parent_,"Xonline",error_msg_.str());
	    return false;
	}
    }
    return true;
}

// Clear the event analysis results:

void
PixEventData::clearAnalysisResults_
(void)
{
    int i;
// Clear the values in event_info_:
    event_info_.mHits          = 0;
    event_info_.count          = 0;
    event_info_.rawCount       = 0;
    event_info_.frameCount     = 0;
    event_info_.dLeftSplit     = 0;
    event_info_.dRightSplit    = 0;
    event_info_.dBackwardSplit = 0;
    event_info_.dForwardSplit  = 0;
    for( i=0; i<4; i++ ) event_info_.nSplitMultip[i] = 0;
// Clear the fill state of the event buffer:
    evtbuf_fillstate_          = 0;
// Set the start temperature in frame_info_:
    frame_info_.tStart         = 0.0; //shmTemp();
// Clear the mean frame rate:
    mean_frame_rate_           = 0.0;
// Clear the average number of events per frame:
    avrg_evts_frame_           = 0.0;
}

// Initialize the analysis resources, set all variables to their
// start values:

void
PixEventData::initAnalysisResources_
(void)
{
    int i;
// The storage arrays for frame data,
// frame buffer is an external array:
    frame_buffer_              = 0;
    pixsignal_buffer_          = 0;
// The frame info structure:
    frame_info_.nEmpty         = 0;
    frame_info_.tStart         = 0.0;
// The pixel statistics map (external array):
    pix_stat_map_              = 0;
// The event threshold map:
    evtthresh_map_             = 0;
// The bad pixel map (external array):
    badpix_map_                = 0;
// The line common mode array:
    line_cmodes_               = 0;
// The event info structure:
    event_info_.count          = 0;
    event_info_.rawCount       = 0;
    event_info_.frameCount     = 0;
    event_info_.mHits          = 0;
    event_info_.dLeftSplit     = 0;
    event_info_.dRightSplit    = 0;
    event_info_.dBackwardSplit = 0;
    event_info_.dForwardSplit  = 0;
    for( i=0; i<4; i++ ) event_info_.nSplitMultip[i] = 0;
    event_info_.curr_rawevt    = 0;
    event_info_.current        = 0;
    event_info_.rawevts        = 0;
    event_info_.frame          = 0;
    event_info_.corr           = 0;
    event_info_.begin          = 0;
    event_info_.last           = 0;
// The fill state of the event buffer:
    evtbuf_fillstate_          = 0;
// The gain and CTI maps:
    channel_gain_map_          = 0;
    line_cte_table_            = 0;
// The y coordinates of the first events in each channel:
    first_evt_coords_          = 0;
// The event maps:
    event_hit_matrix_          = 0;
    frame_event_map_           = 0;
}

// Delete the arrays reserved for the storage of frame
// events and corrected events:

void
PixEventData::clearAnalysisResources_
(void)
{
    int i;

//    if( pixsignal_buffer_ )   delete[] pixsignal_buffer_;
    if( evtthresh_map_ )      delete[] evtthresh_map_;
    if( line_cmodes_ ) {
	for( i=0; i<number_adcs_; i++ ) {
	    delete[] line_cmodes_[i];
	}
	delete[] line_cmodes_;
    }
    if( event_info_.rawevts ) delete[] event_info_.rawevts;
    if( event_info_.frame )   delete[] event_info_.frame;
    if( event_info_.corr )    delete[] event_info_.corr;
    if( event_info_.begin )   delete[] event_info_.begin;
    if( channel_gain_map_ )   delete[] channel_gain_map_;
    if( line_cte_table_ )     delete[] line_cte_table_;
    if( first_evt_coords_ )   delete[] first_evt_coords_;
    if( event_hit_matrix_ )   delete[] event_hit_matrix_;
    if( frame_event_map_ ) {
	for( i=0; i<(frame_width_ + 1); i++ ) {
	    delete[] frame_event_map_[i];
	}
	delete[] frame_event_map_;
    }
}

// Local Variables:
// coding: utf-8
// mode: C++
// c-file-style: "Stroustrup"
// fill-column: 100
// End:

// end of file pix_event_data.C

