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

  return;
}

cass::pnCCD::pnCCDFrameAnalysis::~pnCCDFrameAnalysis
()
{
  ;
}

bool
cass::pnCCD::pnCCDFrameAnalysis::loadDarkCalDataFromFile
(const std::string& fname)
{
  return true;
}

bool
cass::pnCCD::pnCCDFrameAnalysis::processPnCCDDetektorData
(cass::pnCCD::pnCCDDetector *detector)
{
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
