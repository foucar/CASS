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


// frame_data.h
// FrameData provides an interface to frame data and
// the calibration routines that work on every pixel
// of a frame. These are the dark frame calibration
// routines which provide the values of the noise
// and the offset of every pixel. This class also
// contains a common-mode correction routine for a
// line of pixels in a frame. FrameData can work on
// a set of frames with a size of at least two frames.
// FrameCalibPrms is a data structure that holds
// the input parameters of the frame calibration,
// FrameCalib

#ifndef FRAMEDATA_H
#define FRAMEDATA_H

#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>

// #include "askuserdiags.h"

// external c headers, specify the linkage:

extern "C" {
#include "xonline_data_types.h"
#include "fformat.h"
}

class FrameData {
public:
    FrameData(void);
    ~FrameData(void);
// Set the frame set or add a single frame that is analyzed:
    int setCalibFrames(shmBfrType** frameset, int nframes,
		       int width, int height);
    int addCalibFrame(shmBfrType* frame, int width, int height);
// Remove the calibration frames:
    bool removeCalibFrames(void);
// The calibration routine. Calibrations in this context means that
// statistical data for each pixel of the frame are evaluated.
// The statistical pixel data of the calibration frame set are
// converted into values for the common mode of a pixel line, the
// offset level of a pixel and the noise sigma of a pixel.
    bool pixelStatCalibration(bool reset);

//
// Set and get the analysis paramters:
//
// Maximum allowed noise sigma value of a pixel before it is rejected:
    bool setMaxPixelSigma(const double& maxpixsigma);
    double getMaxPixelSigma(void);
// Minimum allowed noise sigma value of a pixel before it is rejected:
    bool setMinPixelSigma(const double& minpixsigma);
    double getMinPixelSigma(void);
// Maximum allowed mean ADC value of a pixel before it is marked as bad:
    bool setSaturationPixelCut(const double& satpixcut);
    double getSaturationPixelCut(void);
// Minimum number of accepted values for each pixel in the frame set:
    bool setEmptyPixelCut(const int& emptypixcut);
    int getEmptyPixelCut(void);
// Maximum number of events in a frame before it is rejected:
    bool setMaxEventsInFrame(const int& maxevtsinfrm);
    int getMaxEventsInFrame(void);
// Event threshold used for pixel statistics calibration in
// multiples of the noise sigma:
    bool setCalibEventThresh(const double& calevtthresh);
    double getCalibEventThresh(void);
// Event threshold in multiples of the noise sigma of a pixel:
    bool setEventThreshold(const double& eventthresh);
    double getEventThreshold(void);
// Number of calibration iterations before bad map construction:
    bool setNumIterBefore(const int& niterbefore);
    int getNumIterBefore(void);
// Number of calibration iterations after bad map construction:
    bool setNumIterAfter(const int& niterafter);
    int getNumIterAfter(void);
// The number of the ADCs, for common mode correction:
    bool setNumberOfAdcs(const int& nadcs);
// Get the analysis results:
//
// Global parameters:
//
// The start time of the first calibration frame in seconds:
    long getCalibStartTime(void);
// Index of the first frame:
    int getIndexFirstFrame(void);
// Index of the last frame:
    int getIndexLastFrame(void);
// Number of cold pixels:
    int getNumColdPixels(void);
// Number of noisy pixels:
    int getNumNoisyPixels(void);
// Number of empty pixels:
    int getNumEmptyPixels(void);
// Number of saturated pixels:
    int getNumSaturatedPixels(void);
// Number of events in the calibration frames:
    int getNumCalEvents(void);
// Number of rejected calibration frames:
    int getNumRejCalFrames(void);
// The mean noise sigma of all pixels without a bad flag:
    double getMeanNoiseSigma(void);
// The mean offset of all pixels without a bad flag:
    double getMeanPixelOffset(void);
//
// Pixel maps:
//
// Map of the pixels with bad flags:
    char* getBadPixelMap(int& width, int& height);
// Map of the event thresholds in ADC counts:
    pxType* getEventThreshMap(int& width, int& height);
// Map of the pixel offsets:
    double* getPixelOffsetMap(int& width, int& height);
// Map of the pixel noise sigma:
    double* getPixelNoiseMap(int& width, int& height);
// Map of the statistics data structures for each pixel:
    staDataType* getPixStatMap(int& width, int& height );
// The array of the common mode values, return with the
// size of the array:
    int     getLineCmodeValues(pxType **cmodevals);
private:
// One step of the iterative pixel statistics calibration, the index
// of the current step is passed as an argument:
    double pixelStatCalStep_(const int& n_cmodesteps);
// Calculate the common mode offset of a line of pixels:
    pxType lineCommonMode_(pxType* pixval_line, pxType* evtthresh_line,
			   char* badflag_line, staDataType* pixstat_line,
			   int n_cmodesteps);
// Common mode correction for DESY / FLASH, acoording to
// Christian Reich: find the lowest n pixels, discard the pixel with
// the lowest value and take the mean of the remaining n - 1 pixels:
    pxType lowestNValsCmmd_(pxType *pixval_line, pxType *evtthresh_line,
			    char *badflag_line, staDataType *pixstat_line,
			    int n_cmodesteps);
// Check if the pixel values in a frame are ok:
    pxType* checkIfFrameIsValid_(const int& n_cmodesteps, shmBfrType* frame,
				 pxType* pix_minval, pxType* pix_maxval);
// Create a bad pixel map with the calibration results:
    int  createBadPixelMap_(void);
// Create the map with the event thresholds:
    void createEvtThreshMap_(const int& which_type);
// Calculate the mean noise of each channel:
    void calcChannelNoise_(void);
// Create the offset map from the pixel statistics data:
    void createOffsetMap_(void);
// Create the noise map from the pixel statistics data:
    void createNoiseMap_(void);
// Allocate memory for the calibration results:
    bool allocCalibResources_(void);
// Allocate or free temporary memory for the common mode values:
    bool allocCmmdMemory_(void);
    bool releaseCmmdMemory_(void);
//
// Private variables:
//
// The parent widget for calls to AskUserDiags:
//    QWidget* parent_;
// Control switches:
    bool calib_done_;
//
// Info and error messages:
//
    std::ostringstream info_msg_;
    std::ostringstream error_msg_;
//
// Calibration parameters:
//
    int          emptypixcut_;
    int          maxevtsinfrm_;
    int          num_iter_before_;
    int          num_iter_after_;
    double       maxpixsigma_;
    double       minpixsigma_;
    double       satpixcut_;

    double       calib_evtthresh_;
    double       event_threshold_;
//
// Data types for calibration information:
//
    infoType     calib_info_;
// Calibration data:
    char*        badpix_map_;
    pxType*      evtthresh_map_;
    double*      noise_map_;
    double*      offset_map_;
    staDataType* pixelstat_map_;

    double*      channel_noise_;
    int          num_badpix_;
// Line value storage for common mode
// determination:
    std::vector<pxType> cmmd_line_;
//Common mode results:
    pxType      *raw_line_cmodes_;
    pxType      *corr_line_cmodes_;
    pxType      *line_offsets_;
    int          n_cmodevals_;
// Buffer for the calibration frames:
    int          num_calfrms_;
    int          frame_width_;
    int          old_fwidth_;
    int          frame_height_;
    int          old_fheight_;
    int          old_arraysize_;
    int          number_adcs_;
    int          adc_channels_;
    shmBfrType*  calframe_buffers_;
};

#endif // FRAMEDATA_H

// Local Variables:
// coding: utf-8
// mode: C++
// c-file-style: "Stroustrup"
// fill-column: 100
// End:

// end of file frame_data.h
