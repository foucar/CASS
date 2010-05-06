// Copyright (C) 2010 Lutz Foucar

#ifndef _PNCCD_DETECTOR_H_
#define _PNCCD_DETECTOR_H_

#include "pixel_detector.h"
#include "cass_pnccd.h"

namespace cass
{
  namespace pnCCD
  {
    /** pnccd detector.
     *
     * The pnccd detector has more information than a regular ccd camera
     * which is defined in PixelDetector. One finds the additional info here.
     *
     * @author Lutz Foucar
     */
    class CASS_PNCCDSHARED_EXPORT PnCCDDetector : public PixelDetector
    {
    public:

    protected:
      /** magic camax info, encodes ie. the gain of the ccd*/
      uint32_t _camaxMagic;
      /** infostring of the detector, telling the name of the detector*/
      std::string _info;
      /** filename of the file containing the timing info of the sequenzer*/
      std::string _timingFilename;
    };
  }//end pnCCD
}//end cass

#endif
