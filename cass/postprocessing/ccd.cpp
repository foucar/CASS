// Copyright (C) 2010 lmf
// Copyright (C) 2010 Jochen Küpper

#include <algorithm>
#include <cassert>
#include <functional>
#include <numeric>
#include <stdexcept>
#include <utility>

#include <QtCore/QSettings>
#include <QtCore/QString>

#include "ccd_device.h"
#include "pnccd_device.h"
#include "histogram.h"
#include "cass_event.h"
#include "postprocessing/postprocessor.h"
#include "postprocessing/ccd.h"


namespace cass
{


// *** postprocessors 1, 2 -- last images from pnCCD ***

pp1::pp1(PostProcessors& pp, cass::PostProcessors::id_t id)
    : PostprocessorBackend(pp, id),
      _image(new Histogram2DFloat(1024, 0, 1023, 1024, 0, 1023))
{
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
    _pp.histograms_replace(_id, _image);
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




// *** postprocessor 3 -- last image from VMI CCD***

pp3::pp3(cass::PostProcessors& pp, cass::PostProcessors::id_t id)
    : PostprocessorBackend(pp, id)
{
    /*
    // create _image storage
    using namespace cass::CCD;
    const CCDDevice *dev(dynamic_cast<const CCDDevice *>(event.devices().find(cass::CASSEvent::CCD)->second));
    const CCDDetector *detector(dev->detector());
    const CCDDetector::frame_t& frame(detector.frame());
    _image = new Histogram2DFloat(detector->rows(), detector-columns());
    // save storage in PostProcessors container
    assert(hist == _histograms);
    _histograms[_id] = _image;
    */
}



pp3::~pp3()
{
    delete _image;
    _image = 0;
}



void pp3::operator()(const cass::CASSEvent& event)
{
    using namespace cass::CCD;
    const CCDDevice *dev(dynamic_cast<const CCDDevice *>(event.devices().find(cass::CASSEvent::CCD)->second));
    const CCDDetector::frame_t& frame(dev->detector().frame());
    std::copy(frame.begin(), frame.end(), _image->memory().begin());
}



// *** postprocessors 101, 102 ***

pp101::pp101(PostProcessors& pp, cass::PostProcessors::id_t id)
    : PostprocessorBackend(pp, id),
      _scale(1.), _binning(std::make_pair(1, 1)), _image(0)
{
    loadSettings(0);
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
    _pp.histograms_delete(_id);
    _image = 0;
}


void cass::pp101::loadSettings(size_t)
{
    QSettings settings;
    settings.beginGroup("postprocessors");
    settings.beginGroup(QString("processor_") + QString::number(_id));
    _average = settings.value("average", 1).toUInt();
    _scale = 1. - 1./_average;
    std::pair<unsigned, unsigned> binning(std::make_pair(settings.value("bin_horizontal", 1).toUInt(),
                                                         settings.value("bin_vertical", 1).toUInt()));
    if((0 == _image) || (binning.first != _binning.first) || (binning.second != _binning.second)) {
        _binning = binning;
        // create new histogram storage and save it in PostProcessors container
        size_t horizontal(1024/_binning.first);
        size_t vertical(1024/_binning.second);
        _image = new Histogram2DFloat(horizontal, vertical);
        _pp.histograms_replace(_id, _image);
    }
}



void cass::pp101::operator()(const CASSEvent& event)
{
    // Running average of pnCCD-1 images with geometric binning (x and y) of
    // postprocessors/101/binning and average length of postprocessors/101/average using namespace
    // cass::pnCCD;
    using namespace cass::pnCCD;
    const pnCCDDevice *dev(dynamic_cast<const pnCCDDevice *>(event.devices().find(cass::CASSEvent::pnCCD)->second));
    const CCDDetector::frame_t& frame(dev->detectors()[_detector].frame());
    // running average binned data:
    //   new_average = new_sum = f * old_sum + data
    size_t rows(dev->detectors()[_detector].rows() / _binning.first);
    size_t cols(dev->detectors()[_detector].columns() / _binning.second);
    _image->lock.lockForWrite();
    for(unsigned r=0; r<rows; r+=_binning.first) {
        for(unsigned c=0; c<cols; c+=_binning.second) {
            pixel_t sum(0);
            for(unsigned row=r; row<r+_binning.first; ++row) {
                for(unsigned col=c; col<c+_binning.second; ++col) {
#warning Check and fix major/minor axis
                    sum += frame[row * cols + col];
                }
            }
            _image->memory()[r * cols/_binning.second + c] *= _scale;
            _image->memory()[r * cols/_binning.second + c] += sum / (_binning.first * _binning.second);
        }
    }
    _image->lock.unlock();
}



// *** postprocessor 141 -- integral over last image from VMI CCD ***

pp141::pp141(PostProcessors& pp, cass::PostProcessors::id_t id)
    : PostprocessorBackend(pp, id), _value(new Histogram0DFloat)
{
    _pp.histograms_replace(_id, _value);
}



pp141::~pp141()
{
    _pp.histograms_delete(_id);
}



void pp141::operator()(const cass::CASSEvent&)
{
    HistogramFloatBase *hist(dynamic_cast<HistogramFloatBase *>(_pp.histograms_checkout().find(PostProcessors::VmiCcdLastImage)->second));
    _pp.histograms_release();
    hist->lock.lockForRead();
    const HistogramFloatBase::storage_t& val(hist->memory());
    HistogramFloatBase::value_t sum(0);
    std::accumulate(val.begin(), val.end(), sum);
    hist->lock.unlock();
    _value->lock.lockForWrite();
    *_value = sum;
    _value->lock.unlock();
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



} // end namespace cass



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-style: "Stroustrup"
// c-file-offsets: ((c . 0) (innamespace . 0))
// fill-column: 100
// End:
