/*
 *  Created by Lutz Foucar on 23.02.2010
 */

#ifndef _CCD_ANALYSIS_H_
#define _CCD_ANALYSIS_H_

#include <QtCore/QPoint>
#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>
#include <vector>

#include "cass_ccd.h"
#include "analysis_backend.h"
#include "parameter_backend.h"
#include "ccd_detector.h"

namespace cass
{
  //forward declaration
  class CASSEvent;

  namespace CCD
  {
    /*! Parameters of the commercial ccd analysis

      @author Lutz Foucar
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

    public:
      uint16_t   _threshold;    //!< the threshold above which pixels are identified
      uint32_t   _rebinfactor;  //!< the rebinning factor by which the image gets rebinned
    };

    /*! Analysis of the commercial CCD.
      Analyses the vmi image and calculates parameters
      @author Lutz Foucar
    */
    class CASS_CCDSHARED_EXPORT Analysis : public cass::AnalysisBackend
    {
    public:
      /** constructor will load the settings*/
      Analysis()            {loadSettings();}
      /** load the settings, lock it first*/
      void loadSettings()   {_param.load();}
      /** save the settings, lock it first*/
      void saveSettings()   {_param.save();}

      //! called for every event will calculate the things
      void operator()(CASSEvent*);

    private:
      QMutex                      _mutex; //!< mutex to block the tmp frame + parameter
      Parameter                   _param; //!< the parameters to analyze
      cass::CCDDetector::frame_t  _tmp;   //!< temp frame for rebinning
    };
  }//end namespace ccd
}//end namespace cass

#endif
