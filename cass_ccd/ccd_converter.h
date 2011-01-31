//Copyright (C) 2009,2010, 2011 Lutz Foucar

#ifndef _CCD_CONVERTER_H_
#define _CCD_CONVERTER_H_

#include "cass_ccd.h"
#include "conversion_backend.h"

namespace cass
{
  class CASSEvent;

  namespace CCD
  {
    /** Converter for commercial ccd cameras
     *
     * This converter will convert the OPAL Camera image to the ccd detector
     * type of cassevent.
     *
     * @author Lutz Foucar
     */
    class CASS_CCDSHARED_EXPORT Converter : public cass::ConversionBackend
    {
    public:
      /** constructor
       *
       * sets up the pds type ids that it is responsible for
       */
      Converter();

      /** convert contents of xtc to cassevent
       *
       * @param[in] xtc The XTC that contains the info about the frame and the frame itself
       * @param[out] evt The cassevent that we store the information from the xtc in.
       */
      void operator()(const Pds::Xtc*xtc, cass::CASSEvent*evt);
    };
  }//end namespace ccd
}//end namespace cass

#endif
