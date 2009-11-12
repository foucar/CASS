// pnccd_analysis_lib.cpp : Nils Kimmel 2009
// pnCCD analysis functionality that provides the analysis
// algorithms of Xonline

#include "pnccd_analysis_lib.h"

cass::pnCCD::pnCCDFrameAnalysis::pnCCDFrameAnalysis
(void)
{
// Allocate instances of the private members:
  darkcal_file_loader_    = new DarkFrameCaldata();
  dark_frame_calibrator_  = new FrameData();
  signal_frame_processor_ = new PixEventData();
// Set start values of the private members:
  dark_caldata_ok_        = false;
  det_columns_            = 0;
  det_rows_               = 0;

  return;
}

cass::pnCCD::pnCCDFrameAnalysis::~pnCCDFrameAnalysis
()
{
  if( darkcal_file_loader_ )    delete darkcal_file_loader_;
  if( dark_frame_calibrator_ )  delete dark_frame_calibrator_;
  if( signal_frame_processor_ ) delete signal_frame_processor_;
}

bool
cass::pnCCD::pnCCDFrameAnalysis::loadDarkCalDataFromFile
(const std::string& fname)
{
  return true;
}

bool
cass::pnCCD::pnCCDFrameAnalysis::processPnCCDDetectorData
(cass::pnCCD::pnCCDDetector *detector)
{
  int16_t    *raw_frm_addr;
  int16_t    *corr_frm_addr;
  shmBfrType  frame_buffer;
// Check if the dark frame calibration has either been set
// or performed. If not, do nothing:
  if( !dark_caldata_ok_ ) return false;

// Check whether the geometry of the frame is equal to the
// geometry which has been defined by the dark frame calibration
// /file loading before:
  if( (det_columns_ != detector->columns()) ||
      (det_rows_    != detector->rows()) )
  {
    return false;
  }
// Get the address of the first elements of the raw frame
// and corr frame data vectors:
  raw_frm_addr  = &detector->rawFrame()[0];
  corr_frm_addr = &detector->correctedFrame()[0];
// Still a bit complicated, but necessary since the data analysis uses
// information from the frame header:
  frame_buffer.frH.index   = 1;
  frame_buffer.frH.tv_sec  = 1;
  frame_buffer.frH.tv_usec = 1;
  frame_buffer.px          = raw_frm_addr;

// Set the raw frame in the signal frame processor:
  signal_frame_processor_->setCurrentFrame(
    &frame_buffer,static_cast<int>(det_columns_),static_cast<int>(det_rows_));
// Set the address of the pixel signal map in the frame processor:
  signal_frame_processor_->setPixSignalBfrAddr(
    corr_frm_addr,static_cast<int>(det_columns_),static_cast<int>(det_rows_));
// Analyze the frame:
  signal_frame_processor_->analyzeCurrentFrame();

  return true;
}

//
// Private function members:
//

bool
cass::pnCCD::pnCCDFrameAnalysis::setDefaultAnalysisParams_
(void)
{
  return true;
}



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-style: "Stroustrup"
// fill-column: 100
// End:

// end of file pnccd_analysis_lib.cpp
