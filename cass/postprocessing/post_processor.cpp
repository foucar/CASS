//Copyright (C) 2010 lmf

#include <QtCore/QMutex>

#include "post_processor.h"
#include "cass_event.h"

namespace cass
{

//========================this should not be touched===========================
// define static members
PostProcessor *cass::PostProcessor::_instance(0);
QMutex PostProcessor::_mutex;


PostProcessor::PostProcessor(const char* OutputFileName)
{
    // fill maps of histograms and postprocessors
    _histograms[std::make_pair(0,0)] = (HistogramBackend *)0;
    // _postprocessors[std::make_pair(0,0)] = new PostprocessorAveragePnCCD(_histograms(std::make_pair(0,0)));
}

  
//create an instance of the singleton
cass::PostProcessor* cass::PostProcessor::instance(const char * OutputFileName)
{
  QMutexLocker locker(&_mutex);
  if(0 == _instance)
    _instance = new PostProcessor(OutputFileName);
  return _instance;
}

//destroy the instance of the singleton
void cass::PostProcessor::destroy()
{
  QMutexLocker locker(&_mutex);
  delete _instance;
  _instance = 0;
}
//=============================================================================


void cass::PostProcessor::postProcess(cass::CASSEvent &cassevent)
{
}


void PostprocessorAveragePnCCD::operator()(const CASSEvent& event)
{
/*
    pnCCD::pnCCDDevice &dev(*(dynamic_cast<pnCCDDevice *>(event->devices()[cass::CASSEvent::pnCCD])));
    CCDDetector::frame_t &frame(dev.detectors()[0].frame());
    for(HistogramFloatBasehisto_t::iterator h=_backend->memory().begin(),
            CCDDetector::frame_t::iterator f=frame.begin(); f != frame.end(); ++f, ++h)
        *h = 0.5 * *h + 0.5 * *f;
*/
}

}

