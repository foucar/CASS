// Copyright (C) 2010 Lutz Foucar
// Copyright (C) 2010 Jochen Küpper

#include <algorithm>
#include <cassert>
#include <functional>
#include <numeric>
#include <stdexcept>
#include <utility>
#include <cmath>

#include <QtCore/QString>

#include "ccd.h"
#include "cass.h"
#include "ccd_device.h"
#include "pnccd_device.h"
#include "histogram.h"
#include "cass_event.h"
#include "postprocessor.h"
#include "acqiris_detectors_helper.h"
#include "tof_detector.h"
#include "convenience_functions.h"
#include "cass_settings.h"

// *** postprocessor 100 -- single images from a CCD ***

cass::pp100::pp100(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  :PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void cass::pp100::loadSettings(size_t)
{
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
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
  setupGeneral();
  if (!setupCondition())
    return;
  _result = new Histogram2DFloat(cols,rows);
  createHistList(2*cass::NbrOfWorkers);
  std::cout<<"Postprocessor "<<_key
      <<": will display ccd image of detector "<<_detector
      <<" in device "<<_device
      <<". The image has "<<rows
      <<" rows and "<<cols
      <<" columns. It will use condition "<<_condition->key()
      <<std::endl;
}

void cass::pp100::process(const cass::CASSEvent& evt)
{
  using namespace std;
  //check whether detector exists
  if (evt.devices().find(_device)->second->detectors()->size() <= _detector)
    throw std::runtime_error(QString("PostProcessor_%1: Detector %2 does not exist in Device %3")
                             .arg(_key.c_str())
                             .arg(_detector)
                             .arg(_device).toStdString());

  //get frame and fill image//
  const PixelDetector::frame_t& frame
      ((*(evt.devices().find(_device)->second)->detectors())[_detector].frame());
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
  _result->lock.lockForWrite();
  copy(frame.begin(),
       frame.end(),
       dynamic_cast<Histogram2DFloat*>(_result)->memory().begin());
  ++_result->nbrOfFills();
  _result->lock.unlock();
}







// *** postprocessor 101:  Integral over the Image ***

cass::pp101::pp101(PostProcessors &pp, const PostProcessors::key_t &key)
  :cass::PostprocessorBackend(pp,key)
{
  loadSettings(0);
}

void cass::pp101::loadSettings(size_t)
{
  using namespace std;
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  _device = static_cast<CASSEvent::Device>(settings.value("Device",0).toUInt());
  _detector = settings.value("Detector",0).toUInt();
  _result = new Histogram0DFloat();
  setupGeneral();
  if (!setupCondition())
    return;
  createHistList(2*cass::NbrOfWorkers);
  std::cout <<std::endl<< "PostProcessor "<<_key
      <<": retrieves the Integral over the whole "<<_detector
      <<" detector"
      <<". Condition is"<<_condition->key()
      <<std::endl;
}

void cass::pp101::process(const cass::CASSEvent &evt)
{
  using namespace std;
  if (evt.devices().find(_device)->second->detectors()->size() <= _detector)
    throw std::runtime_error(QString("PostProcessor_%1: Detector %2 does not exist in Device %3")
                             .arg(_key.c_str())
                             .arg(_detector)
                             .arg(_device).toStdString());
  const float& integral
      (static_cast<float>(
          (*(evt.devices().find(_device)->second)->detectors())[_detector].integral()));
  _result->lock.lockForWrite();
  dynamic_cast<Histogram0DFloat*>(_result)->fill(integral);
  _result->lock.unlock();
}









// *** Integral over the Image for pixel(s) above thres ***

cass::pp102::pp102(PostProcessors &pp, const PostProcessors::key_t &key)
  :cass::PostprocessorBackend(pp,key)
{
  loadSettings(0);
}

void cass::pp102::loadSettings(size_t)
{
  using namespace std;
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  _device = static_cast<CASSEvent::Device>(settings.value("Device",0).toUInt());
  _detector = settings.value("Detector",0).toUInt();
  _result = new Histogram0DFloat();
  setupGeneral();
  if (!setupCondition())
    return;
  createHistList(2*cass::NbrOfWorkers);
  std::cout <<std::endl<< "PostProcessor "<<_key
      <<": retrieves the Integral over the whole "<<_detector
      <<" detector calculated using pixels over threshold."
      <<". Condition is"<<_condition->key()
      <<std::endl;
}

void cass::pp102::process(const cass::CASSEvent &evt)
{
  using namespace std;

  if (evt.devices().find(_device)->second->detectors()->size() <= _detector)
    throw std::runtime_error(QString("PostProcessor_%1: Detector %2 does not exist in Device %3")
                             .arg(_key.c_str())
                             .arg(_detector)
                             .arg(_device).toStdString());
  const float& integral_overthres
      (static_cast<float>(
          (*(evt.devices().find(_device)->second)->detectors())[_detector].integral_overthres()));
  _result->lock.lockForWrite();
  dynamic_cast<Histogram0DFloat*>(_result)->fill(integral_overthres);
  _result->lock.unlock();
}








// *** A Postprocessor that will display the photonhits of ccd detectors in 1D hist***

cass::pp140::pp140(PostProcessors& pp, const cass::PostProcessors::key_t &key)
    : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}


void cass::pp140::loadSettings(size_t)
{
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  _device = static_cast<CASSEvent::Device>(settings.value("Device",0).toUInt());
  _detector = settings.value("Detector",0).toUInt();
  _adu2eV = settings.value("Adu2eV",1).toFloat();
  setupGeneral();
  if (!setupCondition())
    return;
  set1DHist(_result,_key);
  createHistList(2*cass::NbrOfWorkers);
  std::cout<<"Postprocessor "<<_key<<":"
      <<" will display ccd spectrum of detector "<<_detector
      <<" in device "<<_device
      <<". Pixelvalues will be converter by factor "<<_adu2eV
      <<". Condition is"<<_condition->key()
      <<std::endl;
}

void cass::pp140::process(const CASSEvent& evt)
{
  if (evt.devices().find(_device)->second->detectors()->size() <= _detector)
    throw std::runtime_error(QString("PostProcessor_%1: Detector %2 does not exist in Device %3")
                             .arg(_key.c_str())
                             .arg(_detector)
                             .arg(_device).toStdString());
  const PixelDetector::pixelList_t& pixellist
      ((*(evt.devices().find(_device)->second)->detectors())[_detector].pixellist());
  PixelDetector::pixelList_t::const_iterator it(pixellist.begin());
  _result->clear();
  _result->lock.lockForWrite();
  for (; it != pixellist.end();++it)
    dynamic_cast<Histogram1DFloat*>(_result)->fill(it->z()*_adu2eV);
  _result->lock.unlock();
}









// *** A Postprocessor that will display the photonhits of ccd detectors ***

cass::pp141::pp141(PostProcessors& pp, const cass::PostProcessors::key_t &key)
    : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void cass::pp141::loadSettings(size_t)
{
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  _device = static_cast<CASSEvent::Device>(settings.value("Device",0).toUInt());
  _detector = settings.value("Detector",0).toUInt();
  setupGeneral();
  if (!setupCondition())
    return;
  set2DHist(_result,_key);
  createHistList(2*cass::NbrOfWorkers);
  std::cout<<"Postprocessor "<<_key<<":"
      <<" will display ccd image of detector "<<_detector
      <<" in device "<<_device
      <<". Condition is"<<_condition->key()
      <<std::endl;
}

void cass::pp141::process(const CASSEvent& evt)
{
  using namespace std;
  if (evt.devices().find(_device)->second->detectors()->size() <= _detector)
    throw std::runtime_error(QString("PostProcessor_%1: Detector %2 does not exist in Device %3")
                             .arg(_key.c_str())
                             .arg(_detector)
                             .arg(_device).toStdString());
  const PixelDetector::pixelList_t& pixellist
      ((*(evt.devices().find(_device)->second)->detectors())[_detector].pixellist());
  PixelDetector::pixelList_t::const_iterator it(pixellist.begin());
  _result->clear();
  _result->lock.lockForWrite();
  for (; it != pixellist.end();++it)
    dynamic_cast<Histogram2DFloat*>(_result)->fill(it->x(),it->y());
  _result->lock.unlock();
}




// Local Variables:
// coding: utf-8
// mode: C++
// c-file-style: "Stroustrup"
// c-file-offsets: ((c . 0) (innamespace . 0))
// fill-column: 100
// End:
