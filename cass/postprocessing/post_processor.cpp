// Copyright (C) 2010 lmf
// Copyright (C) 2010 Jochen Küpper

#include <QtCore/QMutex>
#include <QtCore/QSettings>

#include "post_processor.h"
#include "cass_event.h"
#include "pnccd_device.h"


namespace cass
{

using namespace std;

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
    // set up list of all active postprocessors/histograms
    // and fill maps of histograms and postprocessors
    readIni();
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
    for(list<id_t>::iterator iter(_active.begin()); iter != _active.end(); ++iter)
        (*(_postprocessors[*iter]))(event);
}



void PostProcessors::readIni()
{
    QSettings settings;
    settings.beginGroup("postprocessors");
    QVariantList list(settings.value("active").toList());
    _active.resize(list.size());
    transform(list.begin(), list.end(), _active.begin(), QVarianttoId_t);
    setup();
}



void PostProcessors::setup()
{
    // delete all unused PostProcessors
    for(postprocessors_t::iterator iter = _postprocessors.begin(); iter != _postprocessors.end(); ++iter)
        if(_active.end() == find(_active.begin(), _active.end(), iter->first))
            _postprocessors.erase(iter);
    // Add newly added PostProcessors -- for histograms we simply make sure the pointer is 0 and let
    // the postprocessor correctly initialize it whenever it wants to
    for(list<id_t>::iterator iter = _active.begin(); iter != _active.end(); ++iter) {
        if(_postprocessors.end() == _postprocessors.find(*iter)) {
            _histograms[*iter] = 0;
            _postprocessors[*iter] = create(_histograms, *iter);
        }
    }
}



PostprocessorBackend * PostProcessors::create(histograms_t hs, id_t id)
{
    PostprocessorBackend * processor(0);
    switch(id) {
    case Pnccd1LastImage:
    case Pnccd2LastImage:
        processor = new PostprocessorPnccdLastImage(hs, id);
        break;
    default:
        throw std::invalid_argument("Impossible postprocessor id for PostprocessorPnccdLastImage");
    }
    return processor;
}



void PostprocessorPnccdLastImage::operator()(const CASSEvent& event)
{
    using namespace pnCCD;
    const pnCCDDevice *dev(dynamic_cast<const pnCCDDevice *>(event.devices().find(cass::CASSEvent::pnCCD)->second));
    const CCDDetector::frame_t& frame(dev->detectors()[_detector].frame());
    copy(frame.begin(), frame.end(), _image->memory().begin());
}



PostprocessorPnccdBinnedRunningAverage::PostprocessorPnccdBinnedRunningAverage(
    PostProcessors::histograms_t& hist, PostProcessors::id_t id)
    : PostprocessorBackend(hist, id)
{
    readIni();
    // _image(new Histogram2DFloat(1024, 0, 1023, 1024, 0, 1023))
    //     {
    //         switch(id) {
    //         case PostProcessors::PnccdLastImage1:
    //             _detector = 0;
    //             break;
    //         case PostProcessors::PnccdLastImage2:
    //             _detector = 1;
    //             break;
    //         };
    //         // save storage in PostProcessors container
    //         assert(hist == _histograms);
    //         _histograms[_id] = _image;
    //     };
}



void PostprocessorPnccdBinnedRunningAverage::readIni()
{
    QSettings settings;
    settings.beginGroup("postprocessors");
    settings.beginGroup(QString("processor_") + QString::number(_id));
    _binning = make_pair(settings.value("bin_vertical").toUInt(), settings.value("bin_vertical").toUInt());
}



void PostprocessorPnccdBinnedRunningAverage::operator()(const CASSEvent& event)
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
