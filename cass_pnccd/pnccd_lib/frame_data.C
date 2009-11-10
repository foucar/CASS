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


// frame_data.C
// Implementation of the class FrameData and its methods
// as declared in frame_data.h

#include "frame_data.h"

FrameData::FrameData
(void)
{
// Initialize the member variables:
    calib_done_ = false;
// Set the default values of the calibration parameters
// to avoid an uninitialized calibration:
    emptypixcut_     = 90;
    maxevtsinfrm_    = 200;
    num_iter_before_ = 4;
    num_iter_after_  = 5;
    maxpixsigma_     = 32.0;
    minpixsigma_     = 1.0;
    satpixcut_       = 3000.0;
    calib_evtthresh_ = 4.0;
    event_threshold_ = 4.0;
// Set the default info messages:
    info_msg_.str("pnCCDlib: no infos");
    error_msg_.str("pnCCDlib: no errors");
// Set the values of the calibration input and output
// data arrays and structures:
    infoType calinfo  = {0,0,0,0,0,0,0,0,0,0,0.0,0.0,0.0,0.0};
    calib_info_       = calinfo;
    badpix_map_       = 0;
    evtthresh_map_    = 0;
    noise_map_        = 0;
    offset_map_       = 0;
    pixelstat_map_    = 0;
    channel_noise_    = 0;
    num_badpix_       = 0;
    raw_line_cmodes_  = 0;
    corr_line_cmodes_ = 0;
    line_offsets_     = 0;
    n_cmodevals_      = 0;
    num_calfrms_      = 0;
    frame_width_      = 0;
    old_fwidth_       = 0;
    frame_height_     = 0;
    old_fheight_      = 0;
    old_arraysize_    = 0;
    number_adcs_      = 1;
    adc_channels_     = 0;
    calframe_buffers_ = 0;
}

FrameData::~FrameData
(void)
{
// Delete the locally reserved memory:
    if( channel_noise_ ) delete[] channel_noise_;
    if( badpix_map_ )    delete[] badpix_map_;
    if( evtthresh_map_ ) delete[] evtthresh_map_;
    if( noise_map_ )     delete[] noise_map_;
    if( offset_map_ )    delete[] offset_map_;
    if( pixelstat_map_ ) delete[] pixelstat_map_;
// Delete the common mode memory:
    this->releaseCmmdMemory_();
}

// Assign a set of calibration frames. Take care that the frame memory
// is not deleted while it is in use by FrameData. Also make shure
// that the number of frames and the array dimensions are correct.
// Otherwise segmentation faults are guaranteed.

int
FrameData::setCalibFrames
(shmBfrType** frameset, int nframes, int width, int height)
{
// The frame memory is provided by an external object. Here, the
// number of frames and the array dimensions are set and the frame
// buffer pointer is assigned to the externally provided address.
    num_calfrms_      = nframes;
    frame_width_      = width;
    frame_height_     = height;
    adc_channels_     = width/number_adcs_;
// And now hope that we can believe the values above!
    calframe_buffers_ = *frameset;

    std::cout << "\n Set frames, first pixel of the first frame: "
	      << frameset[0]->px[0] << std::flush;

// Return the number of assigned frames but consider that to do
// statistics for each pixel, at least two frames are needed:
    if( num_calfrms_ > 1 ) return num_calfrms_;
    else return -1;
}

// An alternative way to assign calibration frames. One frame after
// another is added, but now the memory must be reserved for each
// frame and the frame storage is local.

int
FrameData::addCalibFrame
(shmBfrType* frame, int width, int height)
{
    frame_width_  = width;
    frame_height_ = height;
    adc_channels_ = width/number_adcs_;
    return -1;
}

// Remove the calibration frames. Assign the default values to the
// frame number and dimensions and delete the frame memory or assign
// zero to the frame buffer pointer.

bool
FrameData::removeCalibFrames
(void)
{
// If no calibration frames are available, nothing can be removed:
    if( num_calfrms_ < 1 ) return false;
    num_calfrms_  = 0;
    frame_width_  = 0;
    frame_height_ = 0;
// Local storage not implemented yet, assign zero to the calibration
// frame buffer pointer:
    calframe_buffers_ = 0;
    return true;
}

// Start the pixel statistics calibration. This makes available the
// offset map, the noise map, the bad pixel map, the mean noise
// value for each channel and all global parameters like mean noise,
// the total number of bad pixels and so on.

bool
FrameData::pixelStatCalibration
(bool reset)
{
    int i;
    int npass;
    double noise_sigma, nsigma_diff;
    double nsigma_old=1e10;
// Reset the number of bad pixels:
    num_badpix_ = 0;
// Check if there are enough calibration frames:
    if( num_calfrms_ < 2 ) {
	error_msg_.str("");
	error_msg_ << "Error in pixelStatCalibration(), in file: "
		   << __FILE__ << " , in line: " << __LINE__
		   << " , not enough calibration frames are"
		   << " available";
//	AskUserDiags::askContinue(parent_,"Xonline",error_msg_.str());
	return false;
    }
// Check if calibration was already done:
    if( calib_done_ ) {
	if( !reset ) {
	    error_msg_.str("");
	    error_msg_ << "Error in pixelStatCalibration(), in file: "
		       << __FILE__ << " , in line: " << __LINE__
		       << " , calibration is already done, but no"
		       << " reset is requested. Check the call of "
		       << " pixelStatCalibration().";
//	    AskUserDiags::askContinue(parent_,"Xonline",error_msg_.str());
	    return false;
	}
    }
    std::cout << "\n Alloc calib resources in pixelStatCalibration():"
	      << std::flush;
// Allocate memory space for noise maps etc. :
    if( !allocCalibResources_() ) return false;
// Free the common mode value memory:
    this->releaseCmmdMemory_();
// Allocate the common mode memory:
    if( !this->allocCmmdMemory_() ) return false;

    std::cout << "\n Get frame buffer indices:" << std::flush;
// Reset the values in the calibration info structure:
    calib_info_.firstFrame = calframe_buffers_[0].frH.index;
    calib_info_.lastFrame  = calframe_buffers_[num_calfrms_-1].frH.index;
    std::cout << "\n Frame indices in calib_info_, first: "
	      << calib_info_.firstFrame
	      << " , last: " << calib_info_.lastFrame
	      << "\n" << std::flush;
// Do multiple calibration iterations with an increasing depth
// in the common mode correction which filters out possible events:
    for( i=npass=0; i<num_iter_before_; i++ ) {
//	std::cout << "\n Calculate noise sigma before bad map: " << std::flush;
	noise_sigma = pixelStatCalStep_(npass);
//	std::cout << noise_sigma << std::flush;
// Increase the number of common mode iterations:
	if( i<3 || (npass%2) ) npass++;
	std::cout << std::fixed << std::showpoint << std::setprecision(3)
		  << "mean noise sigma = " << noise_sigma << "\n";
// Stop the calibration if the noise sigma changes by less than
// two per mille:
	nsigma_diff = (nsigma_old - noise_sigma)/nsigma_old;
	if( nsigma_diff > -0.000001 && nsigma_diff < 0.002 ) break;
	nsigma_old = noise_sigma;
    }
// Build the bad pixel map:
    num_badpix_ = createBadPixelMap_();
    std::cout << num_badpix_ << " bad pixels found\n";
// Do the second calibration iterations only if bad pixels were found:
    if( num_badpix_ ) {
// Do multiple calibration iterations with an increasing depth
// in the common mode correction which filters out possible
// events, stop if no further improvement is achieved:
	nsigma_old = 1e10;
	for( i=npass=0; i<num_iter_after_; i++ ) {
	    noise_sigma = pixelStatCalStep_(npass);
	    if( i<3 || (npass%2) ) npass++;
	    std::cout << std::fixed << std::showpoint << std::setprecision(3)
		      << "mean noise sigma = " << noise_sigma << "\n";
// Build a new bad pixel map if the noise sigma changes by less than
// five per mille:
	    nsigma_diff = (nsigma_old - noise_sigma)/nsigma_old;
	    if( fabs(nsigma_diff) < 0.005 ) {
		num_badpix_ = createBadPixelMap_();
		std::cout << num_badpix_ << " bad pixels added\n";
	    }
// Stop the calibration if the noise sigma changes by less than
// one per mille and no new bad pixels are found:
	    if( nsigma_diff > -0.000001 &&
		nsigma_diff < 0.002 &&
		!num_badpix_ ) break;
	    nsigma_old = noise_sigma;
	}
// Build the final bad pixel map if not yet done:
	if( fabs(nsigma_diff) >= 0.005 ) {
	    std::cout << createBadPixelMap_() << " bad pixels added\n";
	}
    }
// Create the thresholds for event selection:
//    createEvtThreshMap_(EVETHRESH);
    createEvtThreshMap_(CALTHRESH);
// Calculate the mean noise of each channel/column of the detector:
    calcChannelNoise_();
// Print the mean pixel offset value:
    std::cout << "\n mean pixel offset = "
	      << calib_info_.meanOffset
	      << " adu\n";

// Store the start time:

//    std::cout << "\n Store the start time: " << std::flush;

    calib_info_.StartTime = calframe_buffers_[num_calfrms_-1].frH.tv_sec;

//    std::cout << "\n Start time: " << calib_info_.StartTime << std::flush;

    return true;
}

//
// The set and get functions for the calibration parameters. They
// perform only rudimentary tests on the assigned values.
//

// The maximum allowed noise sigma of a pixel, otherwise it is
// rejected as being noisy:

bool
FrameData::setMaxPixelSigma
(const double& maxpixsigma)
{
    if( maxpixsigma < 1.0 ) return false;
    maxpixsigma_ = maxpixsigma;
    return true;
}

double
FrameData::getMaxPixelSigma
(void)
{
    return maxpixsigma_;
}

// The minimum allowd noise sigma of a pixel, otherwise it is
// rejected as being cold/dead:

bool
FrameData::setMinPixelSigma
(const double& minpixsigma)
{
    if( minpixsigma < 0.0 ) return false;
    minpixsigma_ = minpixsigma;
    return true;
}

double
FrameData::getMinPixelSigma
(void)
{
    return minpixsigma_;
}

// The saturation pixel cut. This is the maximum allowed mean adc
// value of pixel. Otherwise it is rejected as a saturated pixel.

bool
FrameData::setSaturationPixelCut
(const double& satpixcut)
{
    if( satpixcut < 1.0 ) return false;
    satpixcut_ = satpixcut;
    return true;
}

double
FrameData::getSaturationPixelCut
(void)
{
    return satpixcut_;
}

// If less than emptypixcut pixel values are available for
// statistics calculations of a pixel, it is rejected as
// an empty pixel.

bool
FrameData::setEmptyPixelCut
(const int& emptypixcut)
{
    if( emptypixcut < 2 ) return false;
    emptypixcut_ = emptypixcut;
    return true;
}

int
FrameData::getEmptyPixelCut
(void)
{
    return emptypixcut_;
}

// The maximum number tolerated number of events in a
// calibration frame. If it is exceeded, the whole frame
// is rejected from the calibration data set.

bool
FrameData::setMaxEventsInFrame
(const int& maxevtsinfrm)
{
    if( maxevtsinfrm < 0 ) return false;
    maxevtsinfrm_ = maxevtsinfrm;
    return true;
}

int
FrameData::getMaxEventsInFrame
(void)
{
    return maxevtsinfrm_;
}

// The event threshold used to find events in calibration frames.
// It is defined as a multiple of the pixel noise sigma.

bool
FrameData::setCalibEventThresh
(const double& calevtthresh)
{
    if( calevtthresh < 0.1 ) return false;
    calib_evtthresh_ = calevtthresh;
    return true;
}

double
FrameData::getCalibEventThresh
(void)
{
    return calib_evtthresh_;
}

// The event threshold used to find events in data frames.
// It is defined as a multiple of the pixel noise sigma.

bool
FrameData::setEventThreshold
(const double& eventthresh)
{
    if( eventthresh < 0.1 ) return false;
    event_threshold_ = eventthresh;
    return true;
}

double
FrameData::getEventThreshold
(void)
{
    return event_threshold_;
}

// The number of calibration iterations before the bad pixel
// map is created:

bool
FrameData::setNumIterBefore
(const int& niterbefore)
{
    if( niterbefore < 1 ) return false;
    num_iter_before_ = niterbefore;
    return true;
}

int
FrameData::getNumIterBefore
(void)
{
    return num_iter_before_;
}

// The number of calibration iterations after the bad pixel
// map is created:

bool
FrameData::setNumIterAfter
(const int& niterafter)
{
    if( niterafter < 1 ) return false;
    num_iter_after_ = niterafter;
    return true;
}

int
FrameData::getNumIterAfter
(void)
{
    return num_iter_after_;
}

// The number of ADCs, each with maximmaly two CAMEX
// chips:

bool
FrameData::setNumberOfAdcs
(const int& nadcs)
{
  std::cout << "\n Number of adcs in FrameData: "
	    << nadcs << std::flush;
  if( nadcs < 1 ) return false;
  number_adcs_  = nadcs;
  adc_channels_ = frame_width_/number_adcs_;
// Resize the vector which stores the pixels values for
// the common mode determination:
  cmmd_line_.resize(adc_channels_);

  return true;
}

// Get the start time of the calibration:

long
FrameData::getCalibStartTime
(void)
{
    return calib_info_.StartTime;
}

// Get the index of the first calibration frame:

int
FrameData::getIndexFirstFrame
(void)
{
    return calib_info_.firstFrame;
}

// Get the index of the last calibration frame:

int
FrameData::getIndexLastFrame(void)
{
    return calib_info_.lastFrame;
}

// Get the number of cold pixels which have a noise sigma
// below the user defined minimum value:

int
FrameData::getNumColdPixels
(void)
{
    return calib_info_.nBadCold;
}

// Get the number of noisy pixels which have a noise sigma
// above the user defined maximum value:

int
FrameData::getNumNoisyPixels
(void)
{
    return calib_info_.nBadNoisy;
}

// Get the number of empty pixels which were rejected more
// often during the calibration than the user defined value:

int
FrameData::getNumEmptyPixels
(void)
{
    return calib_info_.nBadEmpty;
}

// Get the number of saturated pixels which have a mean value
// above the user defined limit:

int
FrameData::getNumSaturatedPixels
(void)
{
    return calib_info_.nBadSatur;
}

// Get the number of detected pixel events in the calibration frames
// which are above the user defined event threshold:

int
FrameData::getNumCalEvents
(void)
{
    return calib_info_.nEvents;
}

// Get the number of rejected calibration frames:

int
FrameData::getNumRejCalFrames
(void)
{
    return calib_info_.nRejFrames;
}

// Get the mean noise sigma of all pixels which are not
// flagged bad:

double
FrameData::getMeanNoiseSigma
(void)
{
    return calib_info_.meanSigma;
}

// Get the mean offset of all pixels which are not flagged
// bad:

double
FrameData::getMeanPixelOffset
(void)
{
    return calib_info_.meanOffset;
}

// Get the bad pixel map, each pixel can have one flag:

char*
FrameData::getBadPixelMap
(int& width, int& height)
{
    if( frame_width_*frame_height_ < 1 ) return 0;
    width  = frame_width_;
    height = frame_height_;

    return badpix_map_;
}

// Get the event threshold map, it has integer values. The thresholds
// are directly compared with the signal in adc units:

pxType*
FrameData::getEventThreshMap
(int& width, int& height)
{
    if( frame_width_*frame_height_ < 1 ) return 0;
    width  = frame_width_;
    height = frame_height_;

    return evtthresh_map_;
}

// Get the values of the pixel offsets. The pixel offset value
// is the mean value of a pixel, without common mode correction.
// Events are rejected.

double*
FrameData::getPixelOffsetMap
(int& width, int& height)
{
    if( frame_width_*frame_height_ < 1 ) return 0;
    width  = frame_width_;
    height = frame_height_;
    createOffsetMap_();

    return offset_map_;
}

// Get tthe values of the pixel noise.

double*
FrameData::getPixelNoiseMap
(int& width, int& height)
{
    if( frame_width_*frame_height_ < 1 ) return 0;
    width  = frame_width_;
    height = frame_height_;
    createNoiseMap_();

    return noise_map_;
}

staDataType*
FrameData::getPixStatMap
(int& width, int& height)
{
    if( frame_width_*frame_height_ < 1 ) return 0;
    width  = frame_width_;
    height = frame_height_;

    return pixelstat_map_;
}

int
FrameData::getLineCmodeValues
(pxType **cmodevals)
{
    int    pix_x, pix_y, cmx_idx, frm_idx;
    double line_offset;
// Check the array size and return with zero if the
// common mode value array or the line offset array is
// not allocated:
    if( (n_cmodevals_ < 1) ||
	!raw_line_cmodes_  ||
	!corr_line_cmodes_ ||
	!line_offsets_ ) {
	*cmodevals = 0;
	return 0;
    }
// Create an offset map of the CCD frame:
    this->createOffsetMap_();
// Calculate the offset values of each CAMEX line in the
// frame:
    for( pix_y=0; pix_y<frame_height_; pix_y++ )
    {
	for( cmx_idx=0; cmx_idx<number_adcs_; cmx_idx++ )
	{
	    line_offset = 0.0;
	    for( pix_x=0; pix_x<adc_channels_; pix_x++ )
	    {
		line_offset += offset_map_[
		    pix_x +
		    cmx_idx*adc_channels_ +
		    pix_y*frame_width_ ];
	    }
	    line_offset /= static_cast<double>(adc_channels_);
	    line_offsets_[cmx_idx + pix_y*number_adcs_]
		= static_cast<pxType>(nearbyint(line_offset));
	}
    }
// Correct the  line common mode values for the
// line offset values:
    for( frm_idx=0; frm_idx<num_calfrms_; frm_idx++ )
    {
	for( pix_y=0; pix_y<frame_height_; pix_y++ )
	{
	    for( cmx_idx=0; cmx_idx<number_adcs_; cmx_idx++ ) 
	    {
	       corr_line_cmodes_[
		   cmx_idx + 
		   number_adcs_*pix_y +
		   number_adcs_*frame_height_*frm_idx]
		   = raw_line_cmodes_[
		       cmx_idx + 
		       number_adcs_*pix_y +
		       number_adcs_*frame_height_*frm_idx]
		   - line_offsets_[
		       cmx_idx
		       + pix_y*number_adcs_];
	    }
	}
    }
// Return with the size of the array and assign the array
// address to the argument:
    *cmodevals = corr_line_cmodes_;
    return n_cmodevals_;
}

//
// Private member functions:
//

// Changed on 11 Feb 2008: Common mode for each ADC/CMX

double
FrameData::pixelStatCalStep_
(const int& n_cmodesteps)
{
    char* badmap_ptr;
    int pixel_count;
    int i, n_adc;
    int arraysize, frmheight, pix_x, pix_y;
    double nsigma_mean, noffset_mean, tmp_evtthresh;
    pxType common_mode, tmp_pixval;
    pxType *pixval, *pix_evtthresh;
    staDataType *pix_stats;

    pixel_count  = 0;
    nsigma_mean  = 0.0;
    noffset_mean = 0.0;
// Reset the values in the calibration info info structure:
    calib_info_.nEvents    = 0;
    calib_info_.nEmpty     = 0;
    calib_info_.nRejFrames = 0;
    calib_info_.meanSigma  = 0.0;
    calib_info_.meanOffset = 0.0;
// Set the array size and clear the values in the pixel statistics
// map:
    arraysize = frame_width_*frame_height_;
    pix_stats = pixelstat_map_;
    for( i=0; i<arraysize; i++, pix_stats++ ) {
	pix_stats->count  = 0;
	pix_stats->sum    = 0.0;
	pix_stats->offset = 0.0;
	pix_stats->sumSq  = 0.0;
    }

//    std::cout << "\n Start to loop over " << num_calfrms_
//	      << " calibration frames:" << std::flush;

// Loop over all calibration frames:
    for( i=0; i<num_calfrms_; i++ ) {
// Process pending GUI events:
//	QCoreApplication::processEvents();
// Check if the current frame has reasonable pixel values i.e. not
// all pixels are the same value:

//	std::cout << "\n Check the frame with index " << i << std::flush;

	if( !(&calframe_buffers_[i]) ||
	    (n_cmodesteps && !checkIfFrameIsValid_(
		n_cmodesteps,&calframe_buffers_[i],
		&common_mode,&tmp_pixval))
	    ) {
	    calib_info_.nRejFrames++;
	    continue;
	}
// Set the variable frmheight, frames may have an incomplete
// last line, so frmheight is decremented:
	frmheight = frame_height_;
// Check if the last line of the frame is complete and skip it
// if not:
	if( calframe_buffers_[i].frH.info & INFO_INCMP_LINE ) --frmheight;
// Assign the private storage arrays of FrameData to the local pointers:
	pixval        = calframe_buffers_[i].px;
	pix_evtthresh = evtthresh_map_;
	pix_stats     = pixelstat_map_;
	badmap_ptr    = badpix_map_;

//	std::cout << "\n Start to loop over"
//		  << " all lines of the frame:" << std::flush;

// Loop over all lines of the current calibration frame:
	for( pix_y=0; pix_y<frmheight; pix_y++ ) {
// Determine the common mode value of the current line:

/*
	    std::cout << "\n Value of the first pixel in frame buffer: "
		      << std::flush;
	    std::cout << calframe_buffers_[i]->px[0] << std::flush;


	    std::cout << "\n Calculate the common mode:" << std::flush;
*/
// Loop over the number of adc boards:
	    for ( n_adc=0; n_adc<number_adcs_; n_adc++ )
	    {
		common_mode = lineCommonMode_(pixval,pix_evtthresh,
					      badmap_ptr,pix_stats,
					      n_cmodesteps);
/*
		common_mode = lowestNValsCmmd_(pixval,pix_evtthresh,
					       badmap_ptr,pix_stats,
					       n_cmodesteps);
*/
// Save the common mode value  for the common mode statistics:
		raw_line_cmodes_[n_adc + 
				 number_adcs_*pix_y +
				 number_adcs_*frame_height_*i]
		    = common_mode;
// Loop over all pixels of the current line:

//	std::cout << "\n Start to loop over"
//		  << " all pixels of the line:" << std::flush;

		for( pix_x=0; pix_x<adc_channels_; pix_x++, pixval++,
			 pix_evtthresh++, badmap_ptr++,
			 pix_stats++ ) {
// Use only pixels which are not marked bad and do not have a value
// of zero:
//		    if( !*badmap_ptr && (tmp_pixval = *pixval) ) {
		    tmp_pixval = *pixval; {
			tmp_pixval-=common_mode;
// Check if the pixel has an event hit after the first iteration
// of the common mode correction. Reject pixels with event hits.
			if( n_cmodesteps
			    && (tmp_pixval >
				(*pix_evtthresh + pix_stats->mean))
			    && !*badmap_ptr ) {
			    calib_info_.nEvents++;
			    continue;
			}
// Set the statistics values of the pixel:
			pix_stats->count++;
			pix_stats->offset += *pixval;
			pix_stats->sum    += static_cast<double>(tmp_pixval);
			pix_stats->sumSq  += (static_cast<double>(tmp_pixval)*
					      static_cast<double>(tmp_pixval));
			if( pix_stats->sumSq < 0 ) {
			    std::cout << " square sum of pixel < 0 !"
				      << std::endl;
			}
		    }
		}
	    }
	}
	
    }
// Go through each pixel of the statistics map and calculate the mean,
// sigma, offset and the threshold value for each pixel which is not
// marked bad, i.e. each pixel which has statistics counts:

// Set pointers back to the beginning of the arrays:
    pix_evtthresh = evtthresh_map_;
    pix_stats     = pixelstat_map_;
    for( i=0; i<arraysize; i++, pix_evtthresh++, pix_stats++ ) {
	if( pix_stats->count ) {
	    pix_stats->mean = static_cast<pxType>(
		nearbyint(pix_stats->sum/static_cast<double>(pix_stats->count)));

//	    std::cout << "\n mean = " << pix_stats->mean;

	    pix_stats->offset /= static_cast<double>(pix_stats->count);
	    pix_stats->sigma = sqrt(pix_stats->sumSq/static_cast<double>(pix_stats->count)
				    - ((pix_stats->sum*pix_stats->sum)/
				       static_cast<double>(pix_stats->count*pix_stats->count)));
/*
	    if( (pix_stats->sumSq/static_cast<double>(pix_stats->count)
		 - static_cast<double>(pix_stats->mean*pix_stats->mean)) < 0.0 )
	    {
		std::cout << "\n Argument of sqrt = "
			  << (pix_stats->sumSq/static_cast<double>(pix_stats->count)
			      - static_cast<double>(pix_stats->mean*pix_stats->mean))
			  << " parts = "
			  << (pix_stats->sumSq/static_cast<double>(pix_stats->count))
			  << " and "
			  << (static_cast<double>(pix_stats->mean*pix_stats->mean))
			  << " sigma = " << pix_stats->sigma;
	    }
*/

// Build the new threshold value of the pixel:
	    tmp_evtthresh = calib_evtthresh_*pix_stats->sigma;
//	    tmp_evtthresh = pix_stats->mean 
//		          + calib_evtthresh_*pix_stats->sigma;
// Caution, the threshold map is of type pxType which
// can only hold 16 bits:
	    if( tmp_evtthresh > 1.6e+4 ) *pix_evtthresh = 16000;
	    else *pix_evtthresh = static_cast<int>(nearbyint(tmp_evtthresh));

	    pixel_count++;
	    nsigma_mean  += pix_stats->sigma;
	    noffset_mean += pix_stats->offset;
	}
    }
// Calculate the mean sigma of all pixels:
    calib_info_.meanSigma  = nsigma_mean/static_cast<double>(pixel_count);
// Calculate the mean offset off all pixels which are not bad:
    if( (pixel_count - num_badpix_) > 0 ) {
	calib_info_.meanOffset =
	    noffset_mean/static_cast<double>(pixel_count - num_badpix_);
    }
    else {
	calib_info_.meanOffset =
	    noffset_mean/static_cast<double>(pixel_count);
    }
// Return the mean sigma value:
    return calib_info_.meanSigma;
}

// The common mode correction routine. It calculates the common mode
// of a line of pixels. The common mode is substracted from the value
// of each pixel in the line.

// Changed on 11 Feb 2008: Common mode for each ADC

pxType
FrameData::lineCommonMode_
(pxType* pixval_line, pxType* evtthresh_line,
 char* badflag_line, staDataType* pixstat_line,
 int n_cmodesteps)
{
    char* badflags;
    int num_pixels, numevent_pix;
    int pixelval_sum, eventval_sum, pix_x;
    pxType cmode, cmode_prev;
    pxType *pixelval, *pix_evtthresh;
    static pxType pixval_max = 16383;
    staDataType *pixelstats;

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
		&& ((*pixelval - cmode) > (*pix_evtthresh + pixelstats->mean)) ) {
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
	for( pix_x=0; pix_x<adc_channels_; pix_x++ ) {
	    std::cerr << std::resetiosflags(std::ios::right|std::ios::left)
		      << std::setiosflags(std::ios::right)
		      << " " << std::setw(6) << std::setfill(' ')
		      << pixval_line[pix_x] << " ";
	}
	std::cerr << "\n";
    }
*/
// Return the common mode value:
    return cmode;
}

// Added on 30 March 2009: Mean value of the lowest n - 1 pixels, the
// lowest value of the n lowest pixels is discarded. Note that
// n_cmodesteps is unused since this is not an iterative common
// mode determination alogrithm:

pxType
FrameData::lowestNValsCmmd_
(pxType *pixval_line, pxType *evtthresh_line,
 char *badflag_line, staDataType *pixstat_line,
 int n_cmodesteps)
{
    char          *badflags;
    int            pix_x;
    pxType         cmode;
    pxType        *pixelval;
    static pxType  pixval_max = 16383;
    staDataType   *pixelstats;

    cmode = 0;
// Assign the pointer arguments to their local copies, the pointers
// in the argument are incremented by the function which calls
// the common mode determination routine:

    pixelval     = pixval_line;
    badflags     = badflag_line;
    pixelstats   = pixstat_line;

// Assign the pixel values in the line pixval_line to the storage
// vector:
    for( pix_x=0;
	 pix_x<adc_channels_;
	 pix_x++, pixelval++, badflags++, pixelstats++ )
    {
// Assign the largest pixel value to bad pixels in order to keep them
// out of the lowest n pixel values:
      if( *badflags || (*pixelval < 0) )
      {
	cmmd_line_[pix_x] = pixval_max;
      }
      else
      {
	cmmd_line_[pix_x] = *pixelval - pixelstats->mean;
      }
    }
// Partially sort the pixel values in cmmd_line_ in order to
// find the lowest n pixel values:
    std::partial_sort(cmmd_line_.begin(), 
		      (cmmd_line_.begin() + adc_channels_),
		      cmmd_line_.end());

    return cmode = cmmd_line_[adc_channels_ / 2];

// Calculate the mean of the elements in the index range 1 to 5,
// the element with the index 0 is discarded:
    for( pix_x = 3; pix_x < 10; pix_x++ )
    {
      /*
      std::cout << "\n cmmd_line_[" << pix_x
		<< "] = " << cmmd_line_[pix_x]
		<< std::flush;
      */
      cmode += cmmd_line_[pix_x];
    }
// Get the mean of these values:
    cmode /= 7;

    //    exit(1);

    return cmode;
}


pxType* 
FrameData::checkIfFrameIsValid_
(const int& n_cmodesteps, shmBfrType* frame,
 pxType* pix_minval, pxType* pix_maxval)
{
// This function should be obsolete. It filtered the events out of the
// calibration frames, but we don't need the events from these frames
// later. Pixels with event hits are already rejected in
// pixelStatCalStep_(). This function may better serve to select the events
// in a frame and reject the frame if it contains too many of them. This
// is not done in the old version of Xonline!
    return frame->px;
}

// Create a bad pixel map of the detector frame:

int
FrameData::createBadPixelMap_
(void)
{
    char* badmap_ptr;
    int coldpix_count, emptypix_count, noisypix_count, satpix_count;
    int badpix_channel;
    int i, j, arraysize;
    staDataType* pix_stats;

    coldpix_count  = 0;
    emptypix_count = 0;
    noisypix_count = 0;
    satpix_count   = 0;
    arraysize      = frame_width_*frame_height_;
    badmap_ptr     = badpix_map_;
    pix_stats      = pixelstat_map_;
// Reset the values in the calibration info structure:
    calib_info_.nBadCold  = 0;
    calib_info_.nBadEmpty = 0;
    calib_info_.nBadNoisy = 0;
    calib_info_.nBadSatur = 0;
// Loop over all pixels of the statics data map of the detector frame:
    for( i=0; i<arraysize; i++, badmap_ptr++, pix_stats++ ) {
	if( !*badmap_ptr ) {
	    if( pix_stats->count < emptypixcut_ ) {
		*badmap_ptr = BAD_EMPTY;
		emptypix_count++;
	    }
	    else if( pix_stats->mean > static_cast<pxType>(satpixcut_) ) {
		*badmap_ptr = BAD_SATUR;
		satpix_count++;
	    }
	    else if( pix_stats->sigma > maxpixsigma_ ) {
		*badmap_ptr = BAD_NOISE;
		noisypix_count++;
	    }
	    else if( pix_stats->sigma < minpixsigma_ ) {
		*badmap_ptr = BAD_COLD;
		coldpix_count++;
	    }
	}
// Increment the calibration info values if necessary:
	switch( *badmap_ptr ) {
	    case BAD_EMPTY:
		calib_info_.nBadEmpty++;
		break;
	    case BAD_SATUR:
		calib_info_.nBadSatur++;
		break;
	    case BAD_NOISE:
		calib_info_.nBadNoisy++;
		break;
	    case BAD_COLD:
		calib_info_.nBadCold++;
		break;
	    default:
		break;
	}
    }
// Check the channels of the bad pixel map and reject a channel
// if it contains too many bad pixels:
    for( i=0; i<frame_width_; i++ ) {
	badpix_channel = 0;
	badmap_ptr = badpix_map_ + i;
	for( j=0; j<frame_height_; j++, badmap_ptr+=frame_width_ ) {
	    if( *badmap_ptr && (*badmap_ptr != BAD_USER) ) {
		badpix_channel++;
	    }
	}
	if( badpix_channel < (frame_width_>>1) ) continue;
	badmap_ptr = badpix_map_ + i;
	for( j=0; j<frame_height_; j++, badmap_ptr+=frame_width_ ) {
	    if( !*badmap_ptr ) *badmap_ptr = BAD_CHAN;
	}
    }
    return emptypix_count + satpix_count + noisypix_count + coldpix_count;
}

// Create an event threshold map of the detector frame. The variable
// which_type is set to either CALTHRESH , the threshold used for the
// drak frame calibration or to EVETHRESH , the threshold used for the
// X-ray event selection.

void
FrameData::createEvtThreshMap_
(const int& which_type)
{
    int i, arraysize;
    double tmp_threshval, evtthreshold;
    pxType* pix_evtthresh;
    staDataType* pix_stats;
// Calculate the frame array size and assign the array maps to their
// local pointers:
    arraysize     = frame_height_*frame_width_;
    pix_evtthresh = evtthresh_map_;
    pix_stats     = pixelstat_map_;
// Decide which value of the event threshold should be used. The event
// threshold is defined as a multiple of the pixel noise sigma:
    switch( which_type ) {
	case CALTHRESH:
	    evtthreshold = calib_evtthresh_;
	    break;
	case EVETHRESH:
	    evtthreshold = event_threshold_;
	    break;
	default:
	    error_msg_.str("");
	    error_msg_ << "Error in createEvtThreshMap_() in file: "
		       << __FILE__ << " , in line: " << __LINE__
		       << " , undefined selection of the event"
		       << " threshold type. Calibration event"
		       << " threshold is used instead.";
//	    AskUserDiags::askContinue(parent_,"Xonline",error_msg_.str());
	    evtthreshold = calib_evtthresh_;
	    break;
    }
// Calculate the threshold value for each pixel of the frame:
    for( i=0; i<arraysize; i++, pix_stats++, pix_evtthresh++ ) {
//	tmp_threshval = pix_stats->mean + evtthreshold*(pix_stats->sigma);
	tmp_threshval = evtthreshold*(pix_stats->sigma);
// Limit the treshold value to a maximum value of 32767.0, the adc value
// of a pixel is a 16bit signed integer type:
	if( tmp_threshval > 32767.0 ) {
	    *pix_evtthresh = 32767;
	}
	else {
	    *pix_evtthresh = static_cast<pxType>(nearbyint(tmp_threshval));
	}
    }
    return;
}


// Calculate the mean noise of the pixels in each column/channel of the
// detector frame. This should give an impression of the noise contribution
// of the channels in the readout ASIC.

void
FrameData::calcChannelNoise_
(void)
{
    int pix_x, pix_y;
    double* channel_count;
    staDataType* pix_stats;

    pix_stats = pixelstat_map_;
// Allocate the vector channel_count with the appropriate size
// and assign 0 to each entry:
    channel_count = new double[frame_width_];
    for( pix_x=0; pix_x<frame_width_; pix_x++ ) {
	channel_count[pix_x] = 0.0;
    }
// Reset the values in the array channel_noise_ :
    for( pix_x=0; pix_x<frame_width_; pix_x++ ) {
	channel_noise_[pix_x] = 0.0;
    }
// Loop over all pixels in the pixel statistics map of the detector
// frame:
    for( pix_y=0; pix_y<frame_height_; pix_y++ ) {
	for( pix_x=0; pix_x<frame_width_; pix_x++, pix_stats++ ) {
	    channel_noise_[pix_x] += pix_stats->sigma;
	    channel_count[pix_x] += 1.0;
	}
    }
// Loop over all channels and calculate the mean noise value. Send
// a text message to stdout:
    for( pix_x=0; pix_x<frame_width_; pix_x++ ) {
	if( channel_count[pix_x] ) {
	    channel_noise_[pix_x]/=channel_count[pix_x];
	}
	if( pix_x%8 ) {
	    std::cout << " "
		      << std::setw(8) << std::fixed << std::showpoint
		      << std::setprecision(3) << channel_noise_[pix_x];
	}
	else {
	    std::cout << "\n " << std::resetiosflags(std::ios::right|std::ios::left)
		      << std::setiosflags(std::ios::left)
		      << std::setw(6) << std::setfill(' ')
		      << (pix_x + 1);
	    std::cout << std::resetiosflags(std::ios::left)
		      << std::setiosflags(std::ios::right)
		      << std::setw(8) << std::fixed
		      << std::showpoint << std::setprecision(3)
		      << channel_noise_[pix_x];
	}
    }
    std::cout << "\n";
// Delete the array channel_count:
    delete[] channel_count;
    return;
}

// Create an offset map with the values stored in the pixel statistics array:

void
FrameData::createOffsetMap_
(void)
{
    int i, arraysize;
    double* offsetmap_ptr;
    staDataType* pix_stats;

// Calculate the arraysize with the given frame dimensions. This is also
// the size of the statistics data array:
    if( (arraysize = frame_width_*frame_height_) < 1 ) return;
// Assign the local pointers to the global arrays:
    offsetmap_ptr = offset_map_;
    pix_stats     = pixelstat_map_;
// Loop over all array entries:
    for( i=0; i<arraysize; i++, offsetmap_ptr++, pix_stats++ ) {
	*offsetmap_ptr = pix_stats->offset;
    }
    return;
}

// Create a noise map with the values stored in the pixel statistics array:

void
FrameData::createNoiseMap_
(void)
{
    int i, arraysize;
    double* noisemap_ptr;
    staDataType* pix_stats;

// Calculate the arraysize with the given frame dimensions. This is also
// the size of the statistics data array:
    if( (arraysize = frame_width_*frame_height_) < 1 ) return;
// Assign the local pointers to the global arrays:
    noisemap_ptr  = noise_map_;
    pix_stats     = pixelstat_map_;
// Loop over all array entries:
    for( i=0; i<arraysize; i++, noisemap_ptr++, pix_stats++ ) {
	*noisemap_ptr = pix_stats->sigma;
    }
    return;
}

bool
FrameData::allocCalibResources_
(void)
{
    char*        badmap_ptr;
    pxType*      threshmap_ptr;
    double*      noisemap_ptr;
    double*      offsetmap_ptr;
    staDataType* statmap_ptr;
    int          i, arraysize;

    static staDataType empty_stats = {0,0,0,0.0,0.0,0.0};

    arraysize = frame_width_*frame_height_;
//
// Allocate or free the channel noise array only if the size
// changed
//
    if( frame_width_ > old_fwidth_ ) {
	if( old_fwidth_ > 0 ) delete[] channel_noise_;
// Allocate a new double array of the appropriate size:
	channel_noise_ = new double[frame_width_];
	if( !channel_noise_ ) {
	    error_msg_.str("");
	    error_msg_ << "Error in allocCalibResources_(), in file: "
		       << __FILE__ << " , in line: " << __LINE__
		       << " , failed to allocate memory for the "
		       << " channel noise values.";
//	    AskUserDiags::askContinue(parent_,"Xonline",error_msg_.str());
	    return false;
	}
    }
//
// Free and allocate the noise map each time to ensure that
// all values are cleared and the array size is correctly adjusted:
//
    if( old_arraysize_ > 0 ) delete[] noise_map_;
    noise_map_ = new double[arraysize];
    if( !noise_map_ ) {
	error_msg_.str("");
	error_msg_ << "Error in allocCalibResources_(), in file: "
		   << __FILE__ << " , in line: " << __LINE__
		   << " , failed to allocate memory for the "
		   << " pixel noise map.";
//	AskUserDiags::askContinue(parent_,"Xonline",error_msg_.str());
	return false;
    }
// Clear the newly allocated noise map:
    noisemap_ptr = noise_map_;
    for( i=0; i<arraysize; i++, noisemap_ptr++ ) {
	*noisemap_ptr = 0.0;
    }
//
// Free and allocate the offset map each time to ensure that
// all values are cleared and the array size is correctly adjusted:
//
    if( old_arraysize_ > 0 ) delete[] offset_map_;
    offset_map_ = new double[arraysize];
    if( !offset_map_ ) {
	error_msg_.str("");
	error_msg_ << "Error in allocCalibResources_(), in file: "
		   << __FILE__ << " , in line: " << __LINE__
		   << " , failed to allocate memory for the "
		   << " pixel offset map.";
//	AskUserDiags::askContinue(parent_,"Xonline",error_msg_.str());
	return false;
    }
// Clear the newly allocated offset map:
    offsetmap_ptr = offset_map_;
    for( i=0; i<arraysize; i++, offsetmap_ptr++ ) {
	*offsetmap_ptr = 0.0;
    }
//
// Allocate or free bad pixel map memory only if the size changed:
//
    if( arraysize > old_arraysize_ ) {
	if( old_arraysize_ > 0 ) delete[] badpix_map_;
// The bad pixel map was deleted, allocate a new char array of the
// appropriate size:
	badpix_map_ = new char[arraysize];
	if( !badpix_map_ ) {
	    error_msg_.str("");
	    error_msg_ << "Error in allocCalibResources_(), in file: "
		       << __FILE__ << " , in line: " << __LINE__
		       << " , failed to allocate memory for the "
		       << " bad pixel map.";
//	    AskUserDiags::askContinue(parent_,"Xonline",error_msg_.str());
	    return false;
	}
// Clear the newly allocated bad pixel map:
	badmap_ptr = badpix_map_;
	for( i=0; i<arraysize; i++, badmap_ptr++ ) {
	    *badmap_ptr = 0;
	}
    }
// The bad pixel map was not deleted, clear the existing map except for
// the bad pixels set by the user:
    else {
	badmap_ptr = badpix_map_;
	for( i=0; i<arraysize; i++, badmap_ptr++ ) {
	    if( *badmap_ptr != BAD_USER ) *badmap_ptr=0;
	}
    }
//
// Free and allocate the threshold map each time to ensure that
// all values are cleared and the array size is correctly adjusted:
//
    if( old_arraysize_ > 0 ) delete[] evtthresh_map_;
    evtthresh_map_ = new pxType[arraysize];
    if( !evtthresh_map_ ) {
	error_msg_.str("");
	error_msg_ << "Error in allocCalibResources_(), in file: "
		   << __FILE__ << " , in line: " << __LINE__
		   << " , failed to allocate memory for the "
		   << " event threshold map.";
//	AskUserDiags::askContinue(parent_,"Xonline",error_msg_.str());
	return false;
    }
// Clear the newly allocated event threshold map:
    threshmap_ptr = evtthresh_map_;
    for( i=0; i<arraysize; i++, threshmap_ptr++ ) {
	*threshmap_ptr = 0;
    }
//
// Free and allocate the pixel statistics map each time to ensure that
// all values are cleared and the array size is correctly adjusted:
//
    if( old_arraysize_ > 0 ) delete[] pixelstat_map_;
    pixelstat_map_ = new staDataType[arraysize];
    if( !pixelstat_map_ ) {
	error_msg_.str("");
	error_msg_ << "Error in allocCalibResources_(), in file: "
		   << __FILE__ << " , in line: " << __LINE__
		   << " , failed to allocate memory for the "
		   << " pixel statistics map.";
//	AskUserDiags::askContinue(parent_,"Xonline",error_msg_.str());
	return false;
    }
// Clear the newly allocated pixel statistics map:
    statmap_ptr = pixelstat_map_;
    for( i=0; i<arraysize; i++, statmap_ptr++ ) {
	*statmap_ptr = empty_stats;
    }
// Save the frame width and array size and return:
    old_fwidth_     = frame_width_;
    old_fheight_    = frame_height_;
    old_arraysize_  = arraysize;
    return true;
}

bool
FrameData::allocCmmdMemory_
(void)
{
// Resize the vector which stores the pixels values for
// the common mode determination:
    cmmd_line_.resize(adc_channels_);
// Check if the memory for the raw common mode values
// is already reserved:
    if( n_cmodevals_ || raw_line_cmodes_ ) {
	error_msg_.str("");
	error_msg_ << "Error in allocCmmdMemory_(), in file: "
		   << __FILE__ << " , in line: " << __LINE__
		   << " , the memory for the common mode values "
		   << " has not been freed.";
//	AskUserDiags::askContinue(parent_,"Xonline",error_msg_.str());
//	return false;
	if( raw_line_cmodes_ ) delete[] raw_line_cmodes_;
	n_cmodevals_ = 0;
    }
// Check if the memory for the offset corrected common modes
// is already reserved:
    if( n_cmodevals_ || corr_line_cmodes_ ) {
	error_msg_.str("");
	error_msg_ << "Error in allocCmmdMemory_(), in file: "
		   << __FILE__ << " , in line: " << __LINE__
		   << " , the memory for the corrected line"
		   << " common mode values "
		   << " has not been freed.";
//	AskUserDiags::askContinue(parent_,"Xonline",error_msg_.str());
//	return false;
	if( corr_line_cmodes_ ) delete[] corr_line_cmodes_;
	n_cmodevals_ = 0;
    }
// Check if the memory for the offset values is already reserved:
    if( n_cmodevals_ || line_offsets_ ) {
	error_msg_.str("");
	error_msg_ << "Error in allocCmmdMemory_(), in file: "
		   << __FILE__ << " , in line: " << __LINE__
		   << " , the memory for the line offset values "
		   << " has not been freed.";
//	AskUserDiags::askContinue(parent_,"Xonline",error_msg_.str());
//	return false;
	if( line_offsets_ ) delete[] line_offsets_;
	n_cmodevals_ = 0;
    }
// Reserve the memory but check if the number of lines and calibration
// frames is ok:
    if( (frame_height_ < 1) ||
	(num_calfrms_ < 1)  ||
	(number_adcs_ < 1) ) {
	error_msg_.str("");
	error_msg_ << "Error in allocCmmdMemory_(), in file: "
		   << __FILE__ << " , in line: " << __LINE__
		   << " , the number of lines is " << frame_height_
		   << " and the number of calibration frames is "
		   << num_calfrms_;
//	AskUserDiags::askContinue(parent_,"Xonline",error_msg_.str());
	return false;
    }
// The number of common mode values:
    n_cmodevals_ = num_calfrms_*frame_height_*number_adcs_;
// Reserve the memory for the raw line common modes:
    raw_line_cmodes_ = new pxType[n_cmodevals_];
// Check if the allocation succeeded:
    if( !raw_line_cmodes_ ) {
	error_msg_.str("");
	error_msg_ << "Error in allocCmmdMemory_(), in file: "
		   << __FILE__ << " , in line: " << __LINE__
		   << " , could not allocate the space for "
		   << n_cmodevals_ << " common mode values.";
//	AskUserDiags::askContinue(parent_,"Xonline",error_msg_.str());
	return false;
    }
// Reserve the memory for the corrected line common modes:
    corr_line_cmodes_ = new pxType[n_cmodevals_];
// Check if the allocation succeeded:
    if( !corr_line_cmodes_ ) {
	error_msg_.str("");
	error_msg_ << "Error in allocCmmdMemory_(), in file: "
		   << __FILE__ << " , in line: " << __LINE__
		   << " , could not allocate the space for "
		   << n_cmodevals_ << " common mode values.";
//	AskUserDiags::askContinue(parent_,"Xonline",error_msg_.str());
	return false;
    }
// Reserve the memory for the line offset values:
    line_offsets_ = new pxType[n_cmodevals_];
// Check if the allocation succeeded:
    if( !line_offsets_ ) {
	error_msg_.str("");
	error_msg_ << "Error in allocCmmdMemory_(), in file: "
		   << __FILE__ << " , in line: " << __LINE__
		   << " , could not allocate the space for "
		   << n_cmodevals_ << " offset values.";
//	AskUserDiags::askContinue(parent_,"Xonline",error_msg_.str());
	return false;
    }

    return true;
}

bool
FrameData::releaseCmmdMemory_
(void)
{
// Check if the memory was already allocated. If not do
// nothing and return with false:
    if( !n_cmodevals_ ||
	!raw_line_cmodes_ ||
	!corr_line_cmodes_ ||
	!line_offsets_ ) {
// Check if raw_line_cmodes_ was allocated:
	if( raw_line_cmodes_ ) delete[] raw_line_cmodes_;
// Check if corr_line_cmodes_ was allocated:
	if( corr_line_cmodes_ ) delete[] corr_line_cmodes_;
// Check if line_offsets_ was allocated:
	if( line_offsets_ ) delete[] line_offsets_;
	n_cmodevals_ = 0;

	return  false;
    }
    delete[] raw_line_cmodes_;
    delete[] corr_line_cmodes_;
    delete[] line_offsets_;
    raw_line_cmodes_  = 0;
    corr_line_cmodes_ = 0;
    line_offsets_     = 0;
    n_cmodevals_      = 0;

    return  true;
}

// end of file frame_data.C

