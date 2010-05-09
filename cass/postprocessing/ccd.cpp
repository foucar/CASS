// Copyright (C) 2010 Lutz Foucar
// Copyright (C) 2010 Jochen Küpper

#include <algorithm>
#include <cassert>
#include <functional>
#include <numeric>
#include <stdexcept>
#include <utility>
#include <math.h>

#include <QtCore/QSettings>
#include <QtCore/QString>

#include "ccd_device.h"
#include "pnccd_device.h"
#include "histogram.h"
#include "cass_event.h"
#include "postprocessing/postprocessor.h"
#include "postprocessing/ccd.h"
#include "acqiris_detectors_helper.h"
#include "tof_detector.h"


namespace cass
{


// *** postprocessors 1, 2, 3 -- last images from a CCD ***

pp1::pp1(PostProcessors& pp, cass::PostProcessors::id_t id)
    : PostprocessorBackend(pp, id)
{
    int cols(0);
    int rows(0);
    switch(id)
    {
    case PostProcessors::Pnccd1LastImage:
        _device=CASSEvent::pnCCD; _detector = 0; cols = 1024; rows = 1024;
        break;
    case PostProcessors::Pnccd2LastImage:
        _device=CASSEvent::pnCCD; _detector = 1; cols = 1024; rows = 1024;
        break;
    case PostProcessors::VmiCcdLastImage:
        _device=CASSEvent::CCD; _detector = 0; cols = 1000; rows = 1000;
        break;

    default:
        throw std::invalid_argument("class not responsible for requested postprocessor");
    };
    std::cout<<"Postprocessor_"<<_id<<": set up: cols:"<<cols<<" rows:"<<rows<<std::endl;
    // save storage in PostProcessors container
    _image = new Histogram2DFloat(cols, 0, cols-1, rows, 0, rows-1);
    //_image->setMimeType(std::string("application/image"));     // in future, default mime-type of 2d histograms is 2d histogram, not image. Mime type for individual postprocessors can be specialized like this.
    _pp.histograms_replace(_id, _image);
    VERBOSEOUT(std::cout<<"Postprocessor_"<<_id<<"done."<<std::endl);
}


pp1::~pp1()
{
    _pp.histograms_delete(_id);
    _image = 0;
}


void pp1::operator()(const cass::CASSEvent& event)
{
    //check whether detector exists
    // std::cout<<"BLA"<< event.devices().find(_device)->second->detectors()->size() << " "<< _detector <<std::endl;
    if (event.devices().find(_device)->second->detectors()->size() <= _detector)
        throw std::runtime_error(QString("PostProcessor_%1: Detector %2 does not exist in Device %3").arg(_id).arg(_detector).arg(_device).toStdString());

    //get frame and fill image//
    const PixelDetector::frame_t& frame
        ((*(event.devices().find(_device)->second)->detectors())[_detector].frame());
    _image->lock.lockForWrite();
//    if(frame.size()!=1024*1024)
//    {
//      size_t ratio=1024*1024/frame.size();
//      size_t side_ratio = static_cast<size_t>(sqrt( static_cast<double>(ratio) ));
//      //std::cout<<"allora "<< ratio << " "<< side_ratio <<std::endl;
//      //      _image(Histogram2DFloat(1024/side_ratio, 0, 1023, 1024/side_ratio, 0, 1023));
//    }
    std::copy(frame.begin(), frame.end(), _image->memory().begin());
    _image->lock.unlock();
}











// *** postprocessors 101, 103, 105 ***

pp101::pp101(PostProcessors& pp, cass::PostProcessors::id_t id)
    : PostprocessorBackend(pp, id),
      _scale(1.), _binning(std::make_pair(1, 1)), _image(0)
{
    loadSettings(0);
    switch(id)
    {
    case PostProcessors::PnccdFrontBinnedRunningAverage:
        _detector = 0; _device=CASSEvent::pnCCD;
        break;
    case PostProcessors::PnccdBackBinnedRunningAverage:
        _detector = 1; _device=CASSEvent::pnCCD;
        break;
    case PostProcessors::CommercialCCDBinnedRunningAverage:
        _detector = 0; _device=CASSEvent::CCD;
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
    using namespace cass::ACQIRIS;
    int cols(0); int rows(0);
    switch(_id)
    {
    case PostProcessors::PnccdFrontBinnedRunningAverage:
        cols = 1024; rows = 1024;
        break;
    case PostProcessors::PnccdBackBinnedRunningAverage:
        cols = 1024; rows = 1024;
        break;
    case PostProcessors::CommercialCCDBinnedRunningAverage:
        cols = 1000; rows = 1000;
        break;
    default:
        throw std::invalid_argument("Impossible postprocessor id for pp101");
        break;
    };
    QSettings settings;
    settings.beginGroup("PostProcessor");
    settings.beginGroup(QString("p") + QString::number(_id));
    _average = settings.value("average", 1).toUInt();
    _scale =  2./(_average+1);
    std::pair<unsigned, unsigned> binning(std::make_pair(settings.value("bin_horizontal", 1).toUInt(),
                                                         settings.value("bin_vertical", 1).toUInt()));
    std::string name(settings.value("ConditionDetector","InvalidDetector").toString().toStdString());
    if (name=="YAGPhotodiode")
      _conditionDetector = YAGPhotodiode;
    else if (name=="HexDetector")
      _conditionDetector = HexDetector;
    else if (name=="QuadDetector")
      _conditionDetector = QuadDetector;
    else if (name=="VMIMcp")
      _conditionDetector = VMIMcp;
    else if (name=="FELBeamMonitor")
      _conditionDetector = FELBeamMonitor;
    else if (name=="FsPhotodiode")
      _conditionDetector = FsPhotodiode;
    else
      _conditionDetector = InvalidDetector;

    _invert = settings.value("Invert",false).toBool();

    std::cout<<"Postprocessor_"<<_id<<": alpha for the averaging:"<<_scale<<" average:"<<_average
        <<" condition on detector:"<<name
        <<" which has id:"<<_conditionDetector
        <<" The Condition will be inverted:"<<std::boolalpha<<_invert
        <<std::endl;

    if (_conditionDetector)
      HelperAcqirisDetectors::instance(_conditionDetector)->loadSettings();

    _pp.histograms_delete(_id);
    _image = new Histogram2DFloat(cols,rows);
    _pp.histograms_replace(_id,_image);
}



void cass::pp101::operator()(const CASSEvent& event)
{
    using namespace cass::ACQIRIS;

    //check whether detector exists
    if (event.devices().find(_device)->second->detectors()->size() <= _detector)
        throw std::runtime_error(QString("PostProcessor_%1: Detector %2 does not exist in Device %3").arg(_id).arg(_detector).arg(_device).toStdString());

    const PixelDetector &det((*event.devices().find(_device)->second->detectors())[_detector]);
    const PixelDetector::frame_t& frame(det.frame());

    //find out whether we should update//
    bool update(true);
    if (_conditionDetector != InvalidDetector)
    {
      TofDetector *det =
          dynamic_cast<TofDetector*>(HelperAcqirisDetectors::instance(_conditionDetector)->detector(event));
      update = det->mcp().peaks().size();
      update ^= _invert;
    }
    // running average of data:
    _image->lock.lockForWrite();
    if (update)
      transform(frame.begin(),frame.end(),
                _image->memory().begin(),
                _image->memory().begin(),
                Average(_scale));
    _image->lock.unlock();
}

















// *** A Postprocessor that will display the photonhits of ccd detectors***
// ***  used by postprocessors 110-112 ***

pp110::pp110(PostProcessors& pp, cass::PostProcessors::id_t id)
    : PostprocessorBackend(pp, id)
{
    switch(id)
    {
    case PostProcessors::VMIPhotonHits:
        _device=CASSEvent::CCD; _detector = 0;
        break;
    case PostProcessors::PnCCDFrontPhotonHits:
        _device=CASSEvent::pnCCD; _detector = 0;
        break;
    case PostProcessors::PnCCDBackPhotonHits:
        _device=CASSEvent::pnCCD; _detector = 1;
        break;
    default:
        throw std::invalid_argument("Impossible postprocessor id for class pp110");
        break;
    };
    loadSettings(0);
}

cass::pp110::~pp110()
{
    _pp.histograms_delete(_id);
    _image = 0;
}

void cass::pp110::loadSettings(size_t)
{
  std::cout <<std::endl<< "load the parameters of postprocessor "<<_id
      <<" it histograms the Nbr of Mcp Peaks"
      <<" of detector "<<_detector
      <<" of device "<<_device
      <<std::endl;
  //create the histogram
  set2DHist(_image,_id);
  _pp.histograms_replace(_id,_image);
}

void cass::pp110::operator()(const CASSEvent& evt)
{
    //check whether detector exists
    if (evt.devices().find(_device)->second->detectors()->size() <= _detector)
        throw std::runtime_error(QString("PostProcessor_%1: Detector %2 does not exist in Device %3").arg(_id).arg(_detector).arg(_device).toStdString());
    //retrieve the detector's photon hits of the device we are working for.
    const PixelDetector::pixelList_t& pixellist
        ((*(evt.devices().find(_device)->second)->detectors())[_detector].pixellist());
    PixelDetector::pixelList_t::const_iterator it(pixellist.begin());
    for (; it != pixellist.end();++it)
        _image->fill(it->x(),it->y());
}






















// *** A Postprocessor that will display the photonhits of ccd detectors in 1D hist***
// ***  used by postprocessors 113-115 ***

pp113::pp113(PostProcessors& pp, cass::PostProcessors::id_t id)
    : PostprocessorBackend(pp, id)
{
    switch(id)
    {
    case PostProcessors::VMIPhotonHits1d:
        _device=CASSEvent::CCD; _detector = 0;
        break;
    case PostProcessors::PnCCDFrontPhotonHits1d:
        _device=CASSEvent::pnCCD; _detector = 0;
        break;
    case PostProcessors::PnCCDBackPhotonHits1d:
        _device=CASSEvent::pnCCD; _detector = 1;
        break;
    default:
        throw std::invalid_argument("Impossible postprocessor id for pp113");
        break;
    };
    loadSettings(0);
}

cass::pp113::~pp113()
{
    _pp.histograms_delete(_id);
    _hist = 0;
}

void cass::pp113::loadSettings(size_t)
{
  std::cout <<std::endl<< "load the parameters of postprocessor "<<_id
      <<" it histograms the Nbr of Mcp Peaks"
      <<" of detector "<<_detector
      <<" of device "<<_device
      <<std::endl;
  //create the histogram
  set1DHist(_hist,_id);
  _pp.histograms_replace(_id,_hist);
}

void cass::pp113::operator()(const CASSEvent& evt)
{
    //check whether detector exists
    if (evt.devices().find(_device)->second->detectors()->size() <= _detector)
        throw std::runtime_error(QString("PostProcessor_%1: Detector %2 does not exist in Device %3").arg(_id).arg(_detector).arg(_device).toStdString());

    //retrieve the detector's photon hits of the device we are working for.
    const PixelDetector::pixelList_t& pixellist
        ((*(evt.devices().find(_device)->second)->detectors())[_detector].pixellist());
    PixelDetector::pixelList_t::const_iterator it(pixellist.begin());
    for (; it != pixellist.end();++it)
        _hist->fill(it->z());
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
