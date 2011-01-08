//Copyright (C) 2009, 2010 Lutz Foucar

/**
 * @file delayline_detector_analyzer_simple.h file contains the declaration of
 *                                            classes that analyzses a delayline
 *                                            detector.
 *
 * @author Lutz Foucar
 */

#ifndef __DELAYLINE_DETECTOR_ANALYZER_SIMPLE_H_
#define __DELAYLINE_DETECTOR_ANALYZER_SIMPLE_H_

#include "delayline_detector_analyzer_backend.h"
#include "delayline_detector.h"

namespace cass
{
  namespace ACQIRIS
  {
    /** Simple sorter of hits from a Quadanode delayline detector.
     *
     * Do a simple sorting by checking the timesum for each MCP Signal that was
     * identified.
     *
     * @note after making sure that the waveform signal container will create
     *       the list of singals / peaks itselve, we no longer will need the
     *       info about the waveform analyzers in the constructor.
     *
     * @author Lutz Foucar
     */
    class CASS_ACQIRISSHARED_EXPORT DelaylineDetectorAnalyzerSimple
        : public DelaylineDetectorAnalyzerBackend
    {
    public:
      /** constuctor
       *
       * outputs what we are
       *
       * @param waveformanalyzer the container that contains all waveformanalyzers
       */
      DelaylineDetectorAnalyzerSimple(waveformanalyzers_t* waveformanalyzer)
          :DelaylineDetectorAnalyzerBackend(waveformanalyzer)
      {
        VERBOSEOUT(std::cout << "adding simple quad delayline detector analyzer"<<std::endl);
      }

      /** the function creating the detectorhit list*/
      virtual void operator()(DetectorBackend&,const Device&);

    protected:
      /** first anode layer */
      std::pair<DelaylineDetector::anodelayers_t::key_type,DelaylineDetector::anodelayers_t::key_type> _usedLayers;
    };

  }//end namespace acqiris
}//end namespace cass

#endif
