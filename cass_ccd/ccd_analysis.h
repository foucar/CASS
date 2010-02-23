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
  class CASSEvent;

  namespace CCD
  {
    class CASS_CCDSHARED_EXPORT Parameter : public cass::ParameterBackend
    {
    public:
      Parameter()     {beginGroup("CCD");}
      ~Parameter()    {endGroup();}
      void load();
      void save();

    public:
      uint16_t   _threshold;    //the threshold above which pixels are identified
      uint32_t   _rebinfactor;
    };


    class CASS_CCDSHARED_EXPORT Analysis : public cass::AnalysisBackend
    {
    public:
      Analysis()            {loadSettings();}
      ~Analysis()           {}
      void loadSettings()   {QMutexLocker(&_mutex);_param.load();}
      void saveSettings()   {QMutexLocker(&_mutex);_param.save();}

      //called for every event//
      void operator()(CASSEvent*);

    private:
      QMutex                      _mutex; //mutex to block the tmp frame & parameter
      Parameter                   _param; //the parameters to analyze
      cass::CCDDetector::frame_t  _tmp;   //temp frame for rebinning
    };
  }//end namespace ccd
}//end namespace cass

#endif
