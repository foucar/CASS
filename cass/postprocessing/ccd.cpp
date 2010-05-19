// Copyright (C) 2010 Lutz Foucar
// Copyright (C) 2010 Jochen Küpper

#include <algorithm>
#include <cassert>
#include <functional>
#include <numeric>
#include <stdexcept>
#include <utility>
#include <cmath>

#include <QtCore/QSettings>
#include <QtCore/QString>

#include "ccd_device.h"
#include "pnccd_device.h"
#include "histogram.h"
#include "cass_event.h"
#include "postprocessor.h"
#include "ccd.h"
#include "acqiris_detectors_helper.h"
#include "tof_detector.h"


// *** postprocessor 100 -- single images from a CCD ***

cass::pp100::pp100(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  :PostprocessorBackend(pp, key),_image(0)
{
  loadSettings(0);
}


cass::pp100::~pp100()
{
  _pp.histograms_delete(_key);
  _image = 0;
}

void cass::pp100::loadSettings(size_t)
{
  QSettings settings;
  settings.beginGroup("PostProcessor/active");
  settings.beginGroup(_key.c_str());
  _device = static_cast<CASSEvent::Device>(settings.value("Device",0).toUInt());
  _detector = settings.value("Detector",0).toUInt();

  int cols(0); int rows(0);
  switch(_device)
  {
  case CASSEvent::CCD:
    cols = CCD::opal_default_size; rows = CCD::opal_default_size;
    break;
  case CASSEvent::pnCCD:
    cols = pnCCD::default_size; rows = pnCCD::default_size;
    break;
  default:
    throw std::runtime_error(QString("%1: Device %2 is not an ccd containing device")
                             .arg(_key.c_str())
                             .arg(_device).toStdString());
    break;
  }

  std::cout<<"Postprocessor "<<_key<<":"
      <<" will display ccd image of detector "<<_detector
      <<" in device "<<_device
      <<". The image has "<<rows
      <<" rows and "<<cols
      <<" columns."
      <<std::endl;

  _pp.histograms_delete(_key);
  _image=0;
  _image = new Histogram2DFloat(cols,rows);
  _pp.histograms_replace(_key,_image);
}

void cass::pp100::operator()(const cass::CASSEvent& event)
{
  //check whether detector exists
  if (event.devices().find(_device)->second->detectors()->size() <= _detector)
    throw std::runtime_error(QString("PostProcessor_%1: Detector %2 does not exist in Device %3")
                             .arg(_key.c_str())
                             .arg(_detector)
                             .arg(_device).toStdString());

  //get frame and fill image//
  const PixelDetector::frame_t& frame
      ((*(event.devices().find(_device)->second)->detectors())[_detector].frame());
/*
  // the following block is reasonable, if the frames are already rebinned within the Analysis::operator
  if(frame.size()!=_image->shape().first *_image->shape().second)
  {
    size_t cols = _image->shape().first;
    size_t rows = _image->shape().second;
    size_t ratio= cols * rows /frame.size();
    size_t side_ratio = static_cast<size_t>(sqrt( static_cast<double>(ratio) ));
    //std::cout<<"ratio of sizes, ratio of axis are: "<< ratio << " , "<< side_ratio <<std::endl;
    _image = new Histogram2DFloat(cols/side_ratio, 0, cols-1, rows/side_ratio, 0, rows-1);
  }

  const PixelDetector &det((*event.devices().find(_device)->second->detectors())[_detector]);

  const cass::ROI::ROIiterator_t& ROIiterator_pp(det.ROIiterator_pp());
  std::cout<< "cacca " << ROIiterator_pp.size()
      <<std::endl;
*/

  _image->lock.lockForWrite();
  std::copy(frame.begin(), frame.end(), _image->memory().begin());
  _image->lock.unlock();
}











//// *** postprocessors 101, 103, 105 ***
//
//pp101::pp101(PostProcessors& pp, cass::PostProcessors::key_t key)
//    : PostprocessorBackend(pp, key),
//      _scale(1.), _binning(std::make_pair(1, 1)), _image(0)
//{
//    loadSettings(0);
//    switch(_key)
//    {
//    case PostProcessors::FirstPnccdFrontBinnedConditionalRunningAverage:
//    case PostProcessors::SecondPnccdFrontBinnedConditionalRunningAverage:
//        _detector = 0; _device=CASSEvent::pnCCD;
//        break;
//    case PostProcessors::FirstPnccdBackBinnedConditionalRunningAverage:
//    case PostProcessors::SecondPnccdBackBinnedConditionalRunningAverage:
//        _detector = 1; _device=CASSEvent::pnCCD;
//        break;
//    case PostProcessors::FirstCommercialCCDBinnedConditionalRunningAverage:
//    case PostProcessors::SecondCommercialCCDBinnedConditionalRunningAverage:
//        _detector = 0; _device=CASSEvent::CCD;
//        break;
//    default:
//        throw std::invalid_argument("Impossible postprocessor id for pp101");
//        break;
//    };
//}
//
//cass::pp101::~pp101()
//{
//    _pp.histograms_delete(_key);
//    _image = 0;
//}
//
//void cass::pp101::loadSettings(size_t)
//{
//    using namespace cass::ACQIRIS;
//    int cols(0); int rows(0);
//    switch(_key)
//    {
//    case PostProcessors::FirstPnccdFrontBinnedConditionalRunningAverage:
//    case PostProcessors::SecondPnccdFrontBinnedConditionalRunningAverage:
//    case PostProcessors::FirstPnccdBackBinnedConditionalRunningAverage:
//    case PostProcessors::SecondPnccdBackBinnedConditionalRunningAverage:
//        cols = pnCCD::default_size; rows = pnCCD::default_size;
//        break;
//    case PostProcessors::FirstCommercialCCDBinnedConditionalRunningAverage:
//    case PostProcessors::SecondCommercialCCDBinnedConditionalRunningAverage:
//        cols = CCD::opal_default_size; rows = CCD::opal_default_size;
//        break;
//    default:
//        throw std::invalid_argument("Impossible postprocessor id for pp101");
//        break;
//    };
//    QSettings settings;
//    settings.beginGroup("PostProcessor/active");
//    settings.beginGroup(_key.c_str());
//    _average = settings.value("average", 1).toUInt();
//    _scale =  2./(_average+1);
//    std::pair<unsigned, unsigned> binning(std::make_pair(settings.value("bin_horizontal", 1).toUInt(),
//                                                         settings.value("bin_vertical", 1).toUInt()));
//    std::string name(settings.value("ConditionDetector","InvalidDetector").toString().toStdString());
//    if (name=="YAGPhotodiode")
//      _conditionDetector = YAGPhotodiode;
//    else if (name=="HexDetector")
//      _conditionDetector = HexDetector;
//    else if (name=="QuadDetector")
//      _conditionDetector = QuadDetector;
//    else if (name=="VMIMcp")
//      _conditionDetector = VMIMcp;
//    else if (name=="FELBeamMonitor")
//      _conditionDetector = FELBeamMonitor;
//    else if (name=="FsPhotodiode")
//      _conditionDetector = FsPhotodiode;
//    else
//      _conditionDetector = InvalidDetector;
//
//    _invert = settings.value("Invert",false).toBool();
//
//    std::cout<<"Postprocessor_"<<_key<<": alpha for the averaging:"<<_scale<<" average:"<<_average
//        <<" condition on detector:"<<name
//        <<" which has id:"<<_conditionDetector
//        <<" The Condition will be inverted:"<<std::boolalpha<<_invert
//        <<std::endl;
//
//    if (_conditionDetector)
//      HelperAcqirisDetectors::instance(_conditionDetector)->loadSettings();
//
//    _pp.histograms_delete(_key);
//    _image = new Histogram2DFloat(cols,rows);
//    _pp.histograms_replace(_key,_image);
//
//    _firsttime = true;
//}
//
//void cass::pp101::operator()(const CASSEvent& event)
//{
//    using namespace cass::ACQIRIS;
//
//    //check whether detector exists
//    if (event.devices().find(_device)->second->detectors()->size() <= _detector)
//        throw std::runtime_error(QString("PostProcessor_%1: Detector %2 does not exist in Device %3")
//                                 .arg(_key.c_str())
//                                 .arg(_detector)
//                                 .arg(_device).toStdString());
//
//    const PixelDetector &det((*event.devices().find(_device)->second->detectors())[_detector]);
//    const PixelDetector::frame_t& frame(det.frame());
//
//    //find out whether we should update//
//    bool update(true);
//    if (_conditionDetector != InvalidDetector)
//    {
//      TofDetector *det =
//          dynamic_cast<TofDetector*>(HelperAcqirisDetectors::instance(_conditionDetector)->detector(event));
//      update = det->mcp().peaks().size();
//      update ^= _invert;
//    }
//    // running average of data:
//    _image->lock.lockForWrite();
//    if (update)
//    {
//      ++_image->nbrOfFills();
//      float scale = (_image->nbrOfFills() < _average)? 1./_image->nbrOfFills() :_scale;
//      transform(frame.begin(),frame.end(),
//                _image->memory().begin(),
//                _image->memory().begin(),
//                Average(scale));
//    }
//    _image->lock.unlock();
//}







// *** A Postprocessor that will display the photonhits of ccd detectors in 1D hist***

cass::pp140::pp140(PostProcessors& pp, const cass::PostProcessors::key_t &key)
    : PostprocessorBackend(pp, key), _spec(0)
{
  loadSettings(0);
}

cass::pp140::~pp140()
{
  _pp.histograms_delete(_key);
  _spec = 0;
}

void cass::pp140::loadSettings(size_t)
{
  QSettings settings;
  settings.beginGroup("PostProcessor/active");
  settings.beginGroup(_key.c_str());
  _device = static_cast<CASSEvent::Device>(settings.value("Device",0).toUInt());
  _detector = settings.value("Detector",0).toUInt();
  _adu2eV = settings.value("Adu2eV",1).toFloat();

  std::cout<<"Postprocessor "<<_key<<":"
      <<" will display ccd spectrum of detector "<<_detector
      <<" in device "<<_device
      <<". Pixelvalues will be converter by factor "<<_adu2eV
      <<std::endl;

  _pp.histograms_delete(_key);
  _spec=0;
  set1DHist(_spec,_key);
  _pp.histograms_replace(_key,_spec);
}

void cass::pp140::operator()(const CASSEvent& evt)
{
  //check whether detector exists
  if (evt.devices().find(_device)->second->detectors()->size() <= _detector)
    throw std::runtime_error(QString("PostProcessor_%1: Detector %2 does not exist in Device %3")
                             .arg(_key.c_str())
                             .arg(_detector)
                             .arg(_device).toStdString());

  const PixelDetector::pixelList_t& pixellist
      ((*(evt.devices().find(_device)->second)->detectors())[_detector].pixellist());
  PixelDetector::pixelList_t::const_iterator it(pixellist.begin());
  _spec->lock.lockForWrite();
  fill(_spec->memory().begin(),_spec->memory().end(),0.f);
  for (; it != pixellist.end();++it)
    _spec->fill(it->z()*_adu2eV);
  _spec->lock.unlock();
}









// *** A Postprocessor that will display the photonhits of ccd detectors ***

cass::pp141::pp141(PostProcessors& pp, const cass::PostProcessors::key_t &key)
    : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

cass::pp141::~pp141()
{
  _pp.histograms_delete(_key);
  _image = 0;
}

void cass::pp141::loadSettings(size_t)
{
  QSettings settings;
  settings.beginGroup("PostProcessor/active");
  settings.beginGroup(_key.c_str());
  _device = static_cast<CASSEvent::Device>(settings.value("Device",0).toUInt());
  _detector = settings.value("Detector",0).toUInt();

  std::cout<<"Postprocessor "<<_key<<":"
      <<" will display ccd image of detector "<<_detector
      <<" in device "<<_device
      <<std::endl;

  //create the histogram
  _pp.histograms_delete(_key);
  _image=0;
  set2DHist(_image,_key);
  _pp.histograms_replace(_key,_image);
}

void cass::pp141::operator()(const CASSEvent& evt)
{
  using namespace std;
  //check whether detector exists
  if (evt.devices().find(_device)->second->detectors()->size() <= _detector)
    throw std::runtime_error(QString("PostProcessor_%1: Detector %2 does not exist in Device %3")
                             .arg(_key.c_str())
                             .arg(_detector)
                             .arg(_device).toStdString());

  const PixelDetector::pixelList_t& pixellist
      ((*(evt.devices().find(_device)->second)->detectors())[_detector].pixellist());
  PixelDetector::pixelList_t::const_iterator it(pixellist.begin());
  _image->lock.lockForWrite();
  fill(_image->memory().begin(),_image->memory().end(),0.f);
  for (; it != pixellist.end();++it)
    _image->fill(it->x(),it->y());
  _image->lock.unlock();
}








//// *** A Postprocessor that will display the photonhits of ccd detectors in 1D hist***
//// *** energies in eV
//// ***  used by postprocessors 116-118 ***
//
//pp116::pp116(PostProcessors& pp, cass::PostProcessors::key_t key)
//    : PostprocessorBackend(pp, key), _hist(0)
//{
//    switch(_key)
//    {
//    case PostProcessors::VMIPhotonHitseV1d:
//        _device=CASSEvent::CCD; _detector = 0;
//        break;
//    case PostProcessors::PnCCDFrontPhotonHitseV1d:
//        _device=CASSEvent::pnCCD; _detector = 0;
//        break;
//    case PostProcessors::PnCCDBackPhotonHitseV1d:
//        _device=CASSEvent::pnCCD; _detector = 1;
//        break;
//    default:
//        throw std::invalid_argument("Impossible postprocessor id for pp113");
//        break;
//    };
//    loadSettings(0);
//}
//
//cass::pp116::~pp116()
//{
//    _pp.histograms_delete(_key);
//    _hist = 0;
//}
//
//void cass::pp116::loadSettings(size_t)
//{
//  std::cout <<std::endl<< "load the parameters of postprocessor "<<_key
//      <<" it histograms the Nbr of Mcp Peaks"
//      <<" of detector "<<_detector
//      <<" of device "<<_device
//      <<std::endl;
//  QSettings param;
//  param.beginGroup("PostProcessor/actives");
//  param.beginGroup(_key.c_str());
//  //load the condition on the third component//
//  adu2eV = param.value("adu2eV",5.).toDouble();
//  if(adu2eV<=0.) adu2eV=1.;
//  //create the histogram
//  _pp.histograms_delete(_key);
//  _hist=0;
//  if(param.value("adu2eV",5.).toDouble()!=0.)
//  {
//    std::cerr << "Creating 1D histogram with"
//              <<" XNbrBins:"<<param.value("XNbrBins",1).toUInt()
//              <<" XLow:"<<param.value("XLow",0).toFloat()
//              <<" XUp:"<<16384./param.value("adu2eV",0).toFloat()
//              <<std::endl;
//    _hist = new cass::Histogram1DFloat(param.value("XNbrBins",1).toUInt(),
//                                       param.value("XLow",0).toFloat(),
//                                       16384./param.value("adu2eV",0).toFloat());
//  }
//  else  set1DHist(_hist,_key);
//  _pp.histograms_replace(_key,_hist);
//}
//
//void cass::pp116::operator()(const CASSEvent& evt)
//{
//    //check whether detector exists
//    if (evt.devices().find(_device)->second->detectors()->size() <= _detector)
//        throw std::runtime_error(QString("PostProcessor_%1: Detector %2 does not exist in Device %3")
//                                 .arg(_key.c_str())
//                                 .arg(_detector)
//                                 .arg(_device).toStdString());
//
//    //retrieve the detector's photon hits of the device we are working for.
//    const PixelDetector::pixelList_t& pixellist
//        ((*(evt.devices().find(_device)->second)->detectors())[_detector].pixellist());
//
//    //cass::pnCCD::DetectorParameter &dp = _param._detectorparameters[iDet];
//    //const double adu2eV = 15.;//((*(evt.devices().find(_device)->second)->detectors()->detectorparameters)[_detector]._adu2eV());
//    PixelDetector::pixelList_t::const_iterator it(pixellist.begin());
//    _hist->lock.lockForWrite();
//    for (; it != pixellist.end();++it)
//        _hist->fill(it->z()/adu2eV);
//    _hist->lock.unlock();
//}
//
//
//
//
//
//
//
//





//// *** postprocessor 141 -- integral over last image from VMI CCD ***
//
//pp141::pp141(PostProcessors& pp, cass::PostProcessors::key_t key)
//    : PostprocessorBackend(pp, key), _value(new Histogram0DFloat)
//{
//    _pp.histograms_replace(_key, _value);
//}
//
//pp141::~pp141()
//{
//    _pp.histograms_delete(_key);
//}
//
//void pp141::operator()(const cass::CASSEvent&)
//{
//    HistogramFloatBase *hist(dynamic_cast<HistogramFloatBase *>(_pp.histograms_checkout().find(PostProcessors::VmiCcdLastImage)->second));
//    _pp.histograms_release();
//    hist->lock.lockForRead();
//    const HistogramFloatBase::storage_t& val(hist->memory());
//    HistogramFloatBase::value_t sum(0);
//    std::accumulate(val.begin(), val.end(), sum);
//    hist->lock.unlock();
//    _value->lock.lockForWrite();
//    *_value = sum;
//    _value->lock.unlock();
//}







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
