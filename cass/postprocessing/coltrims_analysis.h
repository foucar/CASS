//Copyright (C) 2011 Lutz Foucar

/**
 * @file coltrims_analysis.h file contains the postprocessor specific for
 *                           coltrims analysis
 *
 * @author Lutz Foucar
 */

#ifndef _COLTRIMSANALYSIS_H
#define _COLTRIMSANALYSIS_H

#include "postprocessing/postprocessor.h"
#include "postprocessing/backend.h"
#include "cass_acqiris.h"
#include "acqiris_detectors_helper.h"
#include "signal_producer.h"
#include "delayline_detector.h"

namespace cass
{
  //forward declarations//
  class Histogram0DFloat;
  class Histogram1DFloat;
  class Histogram2DFloat;


  /** Electron energy.
    *
    * retrieve electron energy from recoil momentum of Particle that belong to a
    * detector
    *
    * To set up the channel assignment for the requested detector one needs to
    * set up the detector parameters.
    * @see cass::ACQIRIS::TofDetector or cass::ACQIRIS::DelaylineDetector and
    *      cass::ACQIRIS::Signal
    *
    * @see PostprocessorBackend for a list of all commonly available cass.ini
    *      settings.
    *
    * @cassttng PostProcessor/\%name\%/{XNbrBins|XLow|XUp}\n
    *           properties of the 1d histogram
    * @cassttng PostProcessor/\%name\%/{Detector}\n
    *           Name of the first detector that we work on. Default is "blubb"
    * @cassttng PostProcessor/\%name\%/{Particle}\n
    *           Name of the particle whose momentum we want to convert to
    *           electron energy in eV
    *
    * @author Daniel Rolles
    * @author Benedikt Rudek
    */
   class pp5000 : public PostprocessorBackend
   {
   public:
     /** Constructor for Number of Signals*/
     pp5000(PostProcessors&, const PostProcessors::key_t&);

     /** Retrieve the number of Signals and histogram it */
     virtual void process(const CASSEvent&);

     /** load the histogram settings from file*/
     virtual void loadSettings(size_t);

   protected:
     /** The detector we are there for*/
      ACQIRIS::HelperAcqirisDetectors::helperinstancesmap_t::key_type _detector;

      /** the particle we are working on */
      ACQIRIS::DelaylineDetector::particles_t::key_type _particle;
    };



   /** Tripple coincidence spectra.
    *
    * This postprocessor will create Tripple Photo-Ion Coincidence Spectra.
    *
    * To set up the channel assignment for the requested detector one needs to set
    * up the detector parameters.
    * @see cass::ACQIRIS::TofDetector or cass::ACQIRIS::DelaylineDetector and
    *      cass::ACQIRIS::Signal
    *
    * @see PostprocessorBackend for a list of all commonly available cass.ini
    *      settings.
    *
    * @cassttng PostProcessor/\%name\%/{XNbrBins|XLow|XUp|YNbrBins|YLow|YUp}\n
    *           properties of the 2d histogram
    * @cassttng PostProcessor/\%name\%/{Detector}\n
    *           Name of the first detector that we work on. Default is "blubb"
    *
    * @author Lutz Foucar
    */
   class pp5001 : public PostprocessorBackend
   {
   public:
     /** Constructor for Number of Signals*/
     pp5001(PostProcessors&, const PostProcessors::key_t&);

     /** Retrieve the number of Signals and histogram it */
     virtual void process(const CASSEvent&);

     /** load the histogram settings from file*/
     virtual void loadSettings(size_t);

   protected:
     /** The first detector of the cooincdence*/
     ACQIRIS::HelperAcqirisDetectors::helperinstancesmap_t::key_type _detector;
   };

}
#endif
