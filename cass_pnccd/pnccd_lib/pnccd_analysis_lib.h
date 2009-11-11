// pnccd_analysis_lib.h : Nils Kimmel 2009
// pnCCD analysis functionality that provides the analysis
// algorithms of Xonline

#ifndef PNCCD_ANALYSIS_LIB_H
#define PNCCD_ANALYSIS_LIB_H

#ifndef ROOT_Rtypes
#include "Rtypes.h"
#endif

#ifndef ROOT_TObject
#include "TObject.h"
#endif

// Include as defined in C99. This should thus hopefully work
// on all systems which support this standard:
#include <vector>
//#include "pdsdata/pnCCD/fformat.h"
//#include <inttypes.h>
#include <string.h>
#include <stdint.h>

//#include "pnccd_photon_hit.h" //!!!when needed replace with pnccd_detector.h

#include "pnccd_event.h"
#include "pnccd_detector.h"

#include "dark_frame_caldata.h"
#include "frame_data.h"
#include "pix_event_data.h"

//#include "cass_pnccd.h"

// pnCCDFrameAnalysis: the wrapper class for the analysis
// functionality of Raccoon/Xonline:

namespace cass
{
  namespace pnCCD
  {
    class pnCCDFrameAnalysis
    {
    public:
// The standard constructor which initializes the analysis
// parameters to defaults except for the offset, noise and
// bad pixel map.
      pnCCDFrameAnalysis(void);
      ~pnCCDFrameAnalysis();
      bool loadDarkCalDataFromFile(const std::string& fname);
      bool processPnCCDDetectorData(cass::pnCCD::pnCCDDetector *detector);
    private:
// Private function members:
      bool setDefaultAnalysisParams_(void);
// The necessary class members for the analysis of a raw
// pnCCD data frame:
      DarkFrameCaldata *darkcal_file_loader_;
      FrameData        *dark_frame_calibrator_;
      PixEventData     *signal_frame_processor_;
// Status flags:
      bool              dark_caldata_ok_;
// Detector parameters:
      uint16_t det_columns_;
      uint16_t det_rows_;

    };
  } // End of namespace pnCCD
} // End of namespace cass

#endif // PNCCD_ANALYSIS_LIB_H

// Local Variables:
// coding: utf-8
// mode: C++
// c-file-style: "Stroustrup"
// fill-column: 100
// End:

// end of file pnccd_analysis_lib.h




