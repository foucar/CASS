// Copyright (C) 2010 Lutz Foucar

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
#include "postprocessor.h"
#include "imaging.h"
#include "acqiris_detectors_helper.h"
#include "tof_detector.h"
#include "averaging_offsetcorrection_helper.h"












// *** postprocessors 210 advanced photon finding ***

cass::pp210::pp210(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key),_image(0)
{
  loadSettings(0);
}

cass::pp210::~pp210()
{
  _pp.histograms_delete(_key);
  _image = 0;
}

cass::PostProcessors::active_t cass::pp210::dependencies()
{
  PostProcessors::active_t list;
  list.push_front(_idAverage);
  list.push_front(_condition);
  return list;
}

void cass::pp210::loadSettings(size_t)
{
  QSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  _device = static_cast<CASSEvent::Device>(settings.value("Device",0).toUInt());
  _detector = settings.value("Detector",0).toUInt();
  _gate = std::make_pair<float,float>(settings.value("LowerGateEnd",-1e6).toFloat(),
                                      settings.value("UpperGateEnd", 1e6).toFloat());
  if (!retrieve_and_validate(_pp,_key,"Condition",_condition))
    return;
  if (!retrieve_and_validate(_pp,_key,"AveragedImage",_idAverage))
    return;
  const HistogramFloatBase *image
      (dynamic_cast<HistogramFloatBase*>(_pp.histograms_checkout().find(_idAverage)->second));
  _pp.histograms_release();
  _pp.histograms_delete(_key);
  _image = new Histogram2DFloat(image->axis()[HistogramBackend::xAxis].size(),
                                image->axis()[HistogramBackend::yAxis].size());
  _pp.histograms_replace(_key,_image);
  std::cout<<"Postprocessor "<<_key
      <<": Lower Gate:"<<_gate.first<<" Upper Gate:"<<_gate.second
      <<" averging image for this pp:"<< _idAverage
      <<" condition on postprocessor:"<<_condition
      <<std::endl;
}



void cass::pp210::operator()(const CASSEvent& event)
{
  using namespace std;
  bool update = dynamic_cast<Histogram0DFloat*>(_pp.histograms_checkout().find(_condition)->second)->isTrue();
  if (update)
  {
    if (event.devices().find(_device)->second->detectors()->size() <=_detector)
      throw std::runtime_error(QString("PostProcessor_%1: detector %2 does not exist in device %3")
                               .arg(_key.c_str())
                               .arg(_detector)
                               .arg(_device).toStdString());
    const PixelDetector &det((*event.devices().find(_device)->second->detectors())[_detector]);
    const PixelDetector::frame_t& frame(det.frame());
    //get the averaged histogram//
    HistogramFloatBase *image
        (dynamic_cast<HistogramFloatBase*>(_pp.histograms_checkout().find(_idAverage)->second));
    _pp.histograms_release();
    //correct the histogram with the help of the helper, retrive a const reference to the corrected frame//
    image->lock.lockForRead();
    PixelDetector::frame_t::const_iterator corFrameIt
        (HelperAveragingOffsetCorrection::instance(_idAverage)->correctFrame(event.id(),frame,image->memory()).begin());
    image->lock.unlock();
    //write it to resulting image
    _image->lock.lockForWrite();
    HistogramFloatBase::storage_t::iterator hIt(_image->memory().begin());
    while(hIt != _image->memory().end())
    {
      *hIt++ += ((_gate.first < *corFrameIt) && (*corFrameIt  < _gate.second));
      ++corFrameIt;
    }
    _image->lock.unlock();
  }
}














// *** postprocessors 166 in 1d histogram ***

//cass::pp166::pp166(PostProcessors& pp, cass::PostProcessors::id_t id)
//  : PostprocessorBackend(pp, id),
//  _hist(0)
//{
//  loadSettings(0);
//  switch(id)
//  {
//  case PostProcessors::AdvancedPhotonFinderFrontPnCCD1dHist:
//  case PostProcessors::AdvancedPhotonFinderFrontPnCCDTwo1dHist:
//      _detector = 0; _device=CASSEvent::pnCCD;
//      break;
//  case PostProcessors::AdvancedPhotonFinderBackPnCCD1dHist:
//  case PostProcessors::AdvancedPhotonFinderBackPnCCDTwo1dHist:
//      _detector = 1; _device=CASSEvent::pnCCD;
//      break;
//  case PostProcessors::AdvancedPhotonFinderCommercialCCD1dHist:
//  case PostProcessors::AdvancedPhotonFinderCommercialCCDTwo1dHist:
//      _detector = 0; _device=CASSEvent::CCD;
//      break;
//  default:
//      throw std::invalid_argument("Impossible postprocessor id for pp166");
//      break;
//  };
//
//}
//
//cass::pp166::~pp166()
//{
//  _pp.histograms_delete(_id);
//  _hist = 0;
//}
//
//std::list<cass::PostProcessors::id_t> cass::pp166::dependencies()
//{
//  std::list<PostProcessors::id_t> list;
//  list.push_front(_idAverage);
//  return list;
//}
//
//void cass::pp166::loadSettings(size_t)
//{
//  using namespace cass::ACQIRIS;
//
//  QSettings settings;
//  settings.beginGroup("PostProcessor");
//  settings.beginGroup(QString("p") + QString::number(_id));
//
//  _invert = settings.value("Invert",false).toBool();
//  _idAverage = static_cast<PostProcessors::id_t>(settings.value("AveragedImage",100).toInt());
//
//  std::string name(settings.value("ConditionDetector","InvalidDetector").toString().toStdString());
//  if (name=="YAGPhotodiode")
//    _conditionDetector = YAGPhotodiode;
//  else if (name=="HexDetector")
//    _conditionDetector = HexDetector;
//  else if (name=="QuadDetector")
//    _conditionDetector = QuadDetector;
//  else if (name=="VMIMcp")
//    _conditionDetector = VMIMcp;
//  else if (name=="FELBeamMonitor")
//    _conditionDetector = FELBeamMonitor;
//  else if (name=="FsPhotodiode")
//    _conditionDetector = FsPhotodiode;
//  else
//    _conditionDetector = InvalidDetector;
//
//  //load condition detector's settings//
//  if (_conditionDetector)
//    HelperAcqirisDetectors::instance(_conditionDetector)->loadSettings();
//
//  std::cout<<"Postprocessor_"<<_id
//      <<" averging image for this pp:"<< _idAverage
//      <<" condition on detector:"<<name
//      <<" which has id:"<<_conditionDetector
//      <<" The Condition will be inverted:"<<std::boolalpha<<_invert
//      <<std::endl;
//
//  //create the histogram
//  _pp.histograms_delete(_id);
//  _hist=0;
//  set1DHist(_hist,_id);
//  _pp.histograms_replace(_id,_hist);
//
//}
//
//void cass::pp166::operator()(const CASSEvent& event)
//{
//  using namespace cass::ACQIRIS;
//  using namespace std;
//
//  //check whether we need to update the histogram//
//  bool update(true);
//  if (_conditionDetector != InvalidDetector)
//  {
//    TofDetector *det =
//        dynamic_cast<TofDetector*>(HelperAcqirisDetectors::instance(_conditionDetector)->detector(event));
//    update = det->mcp().peaks().size();
//    update ^= _invert;
//  }
//
//  if (update)
//  {
//    //check whether detector exists
//    if (event.devices().find(_device)->second->detectors()->size() <=_detector)
//      throw std::runtime_error(QString("PostProcessor_%1: detector %2 does not exist in device %3").arg(_id).arg(_detector).arg(_device).toStdString());
//
//    const PixelDetector &det((*event.devices().find(_device)->second->detectors())[_detector]);
//    const PixelDetector::frame_t& frame(det.frame());
//    //get the averaged histogram//
//    const PostProcessors::histograms_t container (_pp.histograms_checkout());
//    PostProcessors::histograms_t::const_iterator histIt(container.find(_idAverage));
//    const HistogramFloatBase::storage_t average (dynamic_cast<Histogram2DFloat *>(histIt->second)->memory());
//    _pp.histograms_release();
//    //correct the histogram with the help of the helper, retrive a const reference to the corrected frame//
//    histIt->second->lock.lockForRead();
//    const PixelDetector::frame_t &corFrame
//        (HelperAveragingOffsetCorrection::instance(_idAverage)->correctFrame(event.id(),frame,average));
//    PixelDetector::frame_t::const_iterator corFrameIt(corFrame.begin());
//    histIt->second->lock.unlock();
//    //fill histogram with all pixels
//    _hist->lock.lockForWrite();
//    while(corFrameIt != corFrame.end())
//      _hist->fill(*corFrameIt++);
//    _hist->lock.unlock();
//  }
//}
//

