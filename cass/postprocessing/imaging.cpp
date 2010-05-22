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














// *** postprocessors 211 in 1d histogram ***

cass::pp211::pp211(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key), _hist(0)
{
  loadSettings(0);
}

cass::pp211::~pp211()
{
  _pp.histograms_delete(_key);
  _hist = 0;
}

cass::PostProcessors::active_t cass::pp211::dependencies()
{
  PostProcessors::active_t list;
  list.push_front(_idAverage);
  return list;
}

void cass::pp211::loadSettings(size_t)
{
  QSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  _device = static_cast<CASSEvent::Device>(settings.value("Device",0).toUInt());
  _detector = settings.value("Detector",0).toUInt();
  if (!retrieve_and_validate(_pp,_key,"AveragedImage",_idAverage))
    return;
  if (!retrieve_and_validate(_pp,_key,"Condition",_condition))
    return;
  _pp.histograms_delete(_key);
  _hist=0;
  set1DHist(_hist,_key);
  _pp.histograms_replace(_key,_hist);
  std::cout<<"Postprocessor "<<_key
      <<": averging image for this pp:"<< _idAverage
      <<" condition on pp:"<<_condition
      <<std::endl;
}

void cass::pp211::operator()(const CASSEvent& event)
{
  using namespace std;
  //check whether we need to update the histogram//
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
    PixelDetector::frame_t corFrame
        (HelperAveragingOffsetCorrection::instance(_idAverage)->correctFrame(event.id(),frame,image->memory()));
    PixelDetector::frame_t::const_iterator corFrameIt (corFrame.begin());
    image->lock.unlock();
     //fill histogram with all pixels
    _hist->lock.lockForWrite();
    while(corFrameIt != corFrame.end())
      _hist->fill(*corFrameIt++);
    _hist->lock.unlock();
  }
}


