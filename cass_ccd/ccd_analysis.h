/*
 *  Created by Lutz Foucar on 23.02.2010
 */

#ifndef _CCD_ANALYSIS_H_
#define _CCD_ANALYSIS_H_

//#define CCD_default_size    1000
//#define CCD_default_size_sq 1000*1000
//
//#define pulnixCCD_default_height  480
//#define pulnixCCD_default_width   640
//#define pulnixCCD_default_size_sq 640*480

#include <QtCore/QPoint>
#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>
#include <vector>
#include <stdio.h>

#include "cass_ccd.h"
#include "analysis_backend.h"
#include "parameter_backend.h"

#include "pixel_detector.h"

namespace cass
{
  //forward declaration
  class CASSEvent;
  namespace CCD
  {
    /*! Parameters of the commercial ccd analysis

      @author Lutz Foucar, Nicola Coppola
    */
    class CASS_CCDSHARED_EXPORT Parameter : public cass::ParameterBackend
    {
    public:
      /** constructor creates group "CCD" */
      Parameter()     {beginGroup("CCD");}
      /** constructor closes group "CCD" */
      ~Parameter()    {endGroup();}
      /** load the parameters from cass.ini*/
      void load();
      /** save the parameters to cass.ini*/
      void save();
      // I will have to introduce something on this line
      //void loadDetectorParameter(size_t DetectorIndex);

    public:
      uint16_t   _threshold;                 //!< the threshold above which pixels are identified
      uint32_t   _rebinfactor;               //!< the rebinning factor by which the image gets rebinned
      bool       _This_is_a_dark_run;        //flag to set the dark/not-dark run condition
      cass::detROI_ _detROI;
      cass::ROI::ROImask_t _ROImask;         //!< The ROI mask
      cass::ROI::ROIiterator_t _ROIiterator; //!< The ROI iterators
    };

    /*! Analysis of the commercial CCD.
      Analyses the vmi image and calculates parameters
      @author Lutz Foucar
    */
    class CASS_CCDSHARED_EXPORT Analysis : public cass::AnalysisBackend
    {
    public:
      /** constructor will load the settings*/
      Analysis()          {loadSettings();}
      /** load the settings, lock it first*/
      void loadSettings();
      /** save the settings, lock it first*/
      void saveSettings()   {_param.save();}

      //! called for every event will calculate the things
      void operator()(CASSEvent*);

    private:
      QMutex                      _mutex; //!< mutex to block the tmp frame + parameter
      Parameter                   _param; //!< the parameters to analyze
      cass::PixelDetector::frame_t  _tmp; //!< temp frame for rebinning
    };
  }//end namespace ccd
}//end namespace cass

#endif
