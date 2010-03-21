// Copyright (C) 2010 lmf
// Copyright (C) 2010 Jochen Küpper

#include <QtCore/QMutex>

#include "post_processor.h"
#include "cass_event.h"


namespace cass
{

// define static members of singleton -- do not touch
PostProcessors *cass::PostProcessors::_instance(0);
QMutex PostProcessors::_mutex;


PostProcessors::PostProcessors(const char *)
{
    // fill maps of histograms and postprocessors
    _histograms[0] = (HistogramBackend *)0;
    // _postprocessors[std::make_pair(0,0)] = new PostprocessorAveragePnCCD(_histograms(std::make_pair(0,0)));
}


// create an instance of the singleton
cass::PostProcessors *PostProcessors::instance(const char *OutputFileName)
{
    QMutexLocker locker(&_mutex);
    if(0 == _instance)
        _instance = new PostProcessors(OutputFileName);
    return _instance;
}


// destroy the instance of the singleton
void cass::PostProcessors::destroy()
{
    QMutexLocker locker(&_mutex);
    delete _instance;
    _instance = 0;
}



void cass::PostProcessors::process(cass::CASSEvent&)
{
}


void PostprocessorAveragePnCCD::operator()(const CASSEvent&)
{
/*
pnCCD::pnCCDDevice &dev(*(dynamic_cast<pnCCDDevice *>(event->devices()[cass::CASSEvent::pnCCD])));
CCDDetector::frame_t &frame(dev.detectors()[0].frame());
for(HistogramFloatBasehisto_t::iterator h=_backend->memory().begin(),
CCDDetector::frame_t::iterator f=frame.begin(); f != frame.end(); ++f, ++h)
*h = 0.5 * *h + 0.5 * *f;
*/
}

} // namespace cass



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
