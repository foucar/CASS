// Copyright (C) 2010 lmf
// Copyright (C) 2010 Jochen Küpper

#include <QtCore/QMutex>
#include <QtCore/QSettings>

#include "post_processor.h"
#include "cass_event.h"
#include "pnccd_device.h"


namespace cass
{

// define static members of singleton -- do not touch
PostProcessors *cass::PostProcessors::_instance(0);
QMutex PostProcessors::_mutex;


// file-local helper function -- convert QVariant to id_t
static inline PostProcessors::id_t QVarianttoId_t(QVariant i)
{
    return PostProcessors::id_t(i.toInt());
}




PostProcessors::PostProcessors(const char *)
{
    // set up set of all active postprocessors/histograms
    // fill maps of histograms and postprocessors
    // For histograms we simply make sure the pointer is 0 and let the postprocessor
    // correctly initialize it whenever it wants to
    _histograms[PnccdLastImage1] = 0;
    _postprocessors[PnccdLastImage1] = new PostprocessorPnccdLastImage(_histograms, PnccdLastImage1);
    _histograms[PnccdLastImage2] = 0;
    _postprocessors[PnccdLastImage2] = new PostprocessorPnccdLastImage(_histograms, PnccdLastImage2);
}


// create an instance of the singleton
PostProcessors *PostProcessors::instance(const char *OutputFileName)
{
    QMutexLocker locker(&_mutex);
    if(0 == _instance)
        _instance = new PostProcessors(OutputFileName);
    return _instance;
}


// destroy the instance of the singleton
void PostProcessors::destroy()
{
    QMutexLocker locker(&_mutex);
    delete _instance;
    _instance = 0;
}



void PostProcessors::process(cass::CASSEvent& event)
{
    for(postprocessors_t::iterator iter(_postprocessors.begin()); iter != _postprocessors.end(); ++iter)
        (*(iter->second))(event);
}



void PostProcessors::readIni()
{
    QSettings settings;
    settings.beginGroup("postprocessors");
    QVariantList list(settings.value("active").toList());
    _active.resize(list.size());
    transform(list.begin(), list.end(), _active.begin(), QVarianttoId_t);
}



void PostprocessorPnccdLastImage::operator()(const CASSEvent& event)
{
    using namespace pnCCD;
    const pnCCDDevice *dev(dynamic_cast<const pnCCDDevice *>(event.devices().find(cass::CASSEvent::pnCCD)->second));
    const CCDDetector::frame_t& frame(dev->detectors()[_detector].frame());
    copy(frame.begin(), frame.end(), _image->memory().begin());
}



// void PostprocessorAveragePnCCD::operator()(const CASSEvent&)
// {
// /*
// pnCCD::pnCCDDevice &dev(*(dynamic_cast<pnCCDDevice *>(event->devices()[cass::CASSEvent::pnCCD])));
// CCDDetector::frame_t &frame(dev.detectors()[0].frame());
// for(HistogramFloatBasehisto_t::iterator h=_backend->memory().begin(),
// CCDDetector::frame_t::iterator f=frame.begin(); f != frame.end(); ++f, ++h)
// *h = 0.5 * *h + 0.5 * *f;
// */
// }



} // Namespace cass



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
