// Copyright (C) 2010 lmf
// Copyright (C) 2010 Jochen Küpper

#include <utility>
#include <algorithm>
#include <cassert>
#include <stdexcept>

#include <QtCore/QSettings>
#include <QtCore/QString>

#include "ccd_postprocessor.h"
#include "pnccd_device.h"
#include "histogram.h"
#include "post_processor.h"
#include "cass_event.h"


using namespace cass;


pp1::pp1(cass::PostProcessors::histograms_t& hist, cass::PostProcessors::id_t id)
    : PostprocessorBackend(hist, id),
      _image(new Histogram2DFloat(1024, 0, 1023, 1024, 0, 1023))
{
    assert(hist == _histograms);
    switch(id) {
    case PostProcessors::Pnccd1LastImage:
        _detector = 0;
        break;
    case PostProcessors::Pnccd2LastImage:
        _detector = 1;
        break;
    default:
        throw std::invalid_argument("Impossible postprocessor id for PostprocessorPnccdLastImage");
    };
    // save storage in PostProcessors container
    _histograms[_id] = _image;
}


pp1::~pp1()
{
    delete _image;
    _image = 0;
}


void pp1::operator()(const cass::CASSEvent& event)
{
    using namespace cass::pnCCD;
    const pnCCDDevice *dev(dynamic_cast<const pnCCDDevice *>(event.devices().find(cass::CASSEvent::pnCCD)->second));
    const CCDDetector::frame_t& frame(dev->detectors()[_detector].frame());
    std::copy(frame.begin(), frame.end(), _image->memory().begin());
}



// *** postprocessors 101, 102 ***

pp101::pp101(PostProcessors::histograms_t& hist, cass::PostProcessors::id_t id)
    : cass::PostprocessorBackend(hist, id)
{
    assert(hist == _histograms);
    loadSettings(0);
    // save more setup parameters
    switch(id) {
    case PostProcessors::Pnccd1BinnedRunningAverage:
        _detector = 0;
        break;
    default:
        throw std::invalid_argument("Impossible postprocessor id for pp101");
        break;
    };
}


cass::pp101::~pp101()
{
    delete _image;
    _image = 0;
}


void cass::pp101::loadSettings(size_t)
{
    QSettings settings;
    settings.beginGroup("postprocessors");
    settings.beginGroup(QString("processor_") + QString::number(_id));
    _average = settings.value("average").toUInt();
    std::pair<unsigned, unsigned> binning(std::make_pair(settings.value("bin_horizontal").toUInt(),
                                                         settings.value("bin_vertical").toUInt()));
    if((binning.first != _binning.first) ||(binning.second != _binning.second)) {
        _binning = binning;
        // create new histogram storage
        size_t horizontal(1024/_binning.first);
        size_t vertical(1024/_binning.second);
#warning multithreding: block access to _image
        delete _image;
        _image = new Histogram2DFloat(horizontal, 0, horizontal-1, vertical, 0, vertical-1);
    }
    // save storage in PostProcessors container
    _histograms[_id] = _image;
}



void cass::pp101::operator()(const CASSEvent& event)
{
    // Running average of pnCCD-1 images with geometric binning (x and y) of
    // postprocessors/101/binning and average length of postprocessors/101/average using namespace
    // cass::pnCCD;
    using namespace cass::pnCCD;
    const pnCCDDevice *dev(dynamic_cast<const pnCCDDevice *>(event.devices().find(cass::CASSEvent::pnCCD)->second));
    const CCDDetector::frame_t& frame(dev->detectors()[_detector].frame());
#warning Implement!
    std::copy(frame.begin(), frame.end(), _image->memory().begin());
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




// Local Variables:
// coding: utf-8
// mode: C++
// c-file-style: "Stroustrup"
// c-file-offsets: ((c . 0) (innamespace . 0))
// fill-column: 100
// End:
