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

#ifdef SINGLEPARTICLE_HIT
#include "hit_helper.h"
#include "hit_helper2.h"
#endif /* SINGLEPARTICLE_HIT */


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

cass::PostProcessors::active_t cass::pp100::dependencies()
{
  PostProcessors::active_t list;
  if (_useCondition) list.push_front(_condition);
  return list;
}

void cass::pp100::loadSettings(size_t)
{
  QSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());

  //dependancy:
  if (settings.contains("ConditionName")) 
  {
    _useCondition = true;
    if (!retrieve_and_validate(_pp,_key,"ConditionName",_condition)) return;
  } 
  else
  {
    _useCondition = false;
  }

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
  using namespace std;

  // if condition is in use and not met, don't do anything:
  if (_useCondition) {
      const Histogram0DFloat*cond
          (reinterpret_cast<Histogram0DFloat*>(histogram_checkout(_condition)));
      if (!cond->isTrue()) return;
  }

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
  copy(frame.begin(), frame.end(), _image->memory().begin());
  _image->lock.unlock();
}









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
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  _device = static_cast<CASSEvent::Device>(settings.value("Device",0).toUInt());
  _detector = settings.value("Detector",0).toUInt();
  _adu2eV = settings.value("Adu2eV",1).toFloat();
  _pp.histograms_delete(_key);
  _spec=0;
  set1DHist(_spec,_key);
  _pp.histograms_replace(_key,_spec);
  std::cout<<"Postprocessor "<<_key<<":"
      <<" will display ccd spectrum of detector "<<_detector
      <<" in device "<<_device
      <<". Pixelvalues will be converter by factor "<<_adu2eV
      <<std::endl;
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
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  _device = static_cast<CASSEvent::Device>(settings.value("Device",0).toUInt());
  _detector = settings.value("Detector",0).toUInt();
  _pp.histograms_delete(_key);
  _image=0;
  set2DHist(_image,_key);
  _pp.histograms_replace(_key,_image);

  std::cout<<"Postprocessor "<<_key<<":"
      <<" will display ccd image of detector "<<_detector
      <<" in device "<<_device
      <<std::endl;
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


//----------------Integral over the Image---------------------------------------------
cass::pp101::pp101(PostProcessors &pp, const PostProcessors::key_t &key)
  :cass::PostprocessorBackend(pp,key),
  _ImageIntegral(0)
{
  loadSettings(0);
}

cass::pp101::~pp101()
{
  _pp.histograms_delete(_key);
  _ImageIntegral=0;
}

void cass::pp101::loadSettings(size_t)
{
  using namespace std;
  QSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  _device = static_cast<CASSEvent::Device>(settings.value("Device",0).toUInt());
  _detector = settings.value("Detector",0).toUInt();
  switch(_device)
  {
  case CASSEvent::CCD:
    break;
  case CASSEvent::pnCCD:
    break;
  default:
    throw std::runtime_error(QString("%1: Device %2 is not an ccd containing device")
                             .arg(_key.c_str())
                             .arg(_device).toStdString());
    break;
  }
  _pp.histograms_delete(_key);
  _ImageIntegral = new Histogram0DFloat();
  _pp.histograms_replace(_key,_ImageIntegral);
  std::cout <<std::endl<< "PostProcessor "<<_key
      <<": retrieves the Integral over the whole "<<_detector
      <<" detector."
      <<std::endl;
}

void cass::pp101::operator()(const cass::CASSEvent &evt)
{
  using namespace std;
  //check whether detector exists
  if (evt.devices().find(_device)->second->detectors()->size() <= _detector)
    throw std::runtime_error(QString("PostProcessor_%1: Detector %2 does not exist in Device %3")
                             .arg(_key.c_str())
                             .arg(_detector)
                             .arg(_device).toStdString());

  //get frame and fill image//
  const float& integral
      (static_cast<float>(
          (*(evt.devices().find(_device)->second)->detectors())[_detector].integral()));


  _ImageIntegral->lock.lockForWrite();
  _ImageIntegral->fill(integral);
  _ImageIntegral->lock.unlock();
}



//----------------Integral over the Image for pixel(s) above thres-----------------------------
cass::pp102::pp102(PostProcessors &pp, const PostProcessors::key_t &key)
  :cass::PostprocessorBackend(pp,key),
  _ImageIntegralOverThres(0)
{
  loadSettings(0);
}

cass::pp102::~pp102()
{
  _pp.histograms_delete(_key);
  _ImageIntegralOverThres=0;
}

void cass::pp102::loadSettings(size_t)
{
  using namespace std;
  QSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  _device = static_cast<CASSEvent::Device>(settings.value("Device",0).toUInt());
  _detector = settings.value("Detector",0).toUInt();
  switch(_device)
  {
  case CASSEvent::CCD:
    break;
  case CASSEvent::pnCCD:
    break;
  default:
    throw std::runtime_error(QString("%1: Device %2 is not an ccd containing device")
                             .arg(_key.c_str())
                             .arg(_device).toStdString());
    break;
  }
  _pp.histograms_delete(_key);
  _ImageIntegralOverThres = new Histogram0DFloat();
  _pp.histograms_replace(_key,_ImageIntegralOverThres);
  //??::instance(_detector)->loadSettings();
  std::cout <<std::endl<< "PostProcessor "<<_key
      <<": retrieves the Integral over the whole "<<_detector
      <<" detector calculated using pixels over threshold."
      <<std::endl;
}

void cass::pp102::operator()(const cass::CASSEvent &evt)
{
  using namespace std;
  //check whether detector exists
  if (evt.devices().find(_device)->second->detectors()->size() <= _detector)
    throw std::runtime_error(QString("PostProcessor_%1: Detector %2 does not exist in Device %3")
                             .arg(_key.c_str())
                             .arg(_detector)
                             .arg(_device).toStdString());

  //get frame and fill image//
  const float& integral_overthres
      (static_cast<float>(
          (*(evt.devices().find(_device)->second)->detectors())[_detector].integral_overthres()));
//maxPixelValue
//integral_overthres
  _ImageIntegralOverThres->lock.lockForWrite();
  _ImageIntegralOverThres->fill(integral_overthres);
  _ImageIntegralOverThres->lock.unlock();
}



// *** postprocessor 103 -- single images from a CCD, only updated if the image is integral is over threshold ***

cass::pp103::pp103(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  :PostprocessorBackend(pp, key),_image(0)
{
  loadSettings(0);
}

cass::pp103::~pp103()
{
  _pp.histograms_delete(_key);
  _image = 0;
}

cass::PostProcessors::active_t cass::pp103::dependencies()
{
  PostProcessors::active_t list;
  if (_useCondition) list.push_front(_condition);
  return list;
}

void cass::pp103::loadSettings(size_t)
{
  QSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());

  //dependancy:
  if (settings.contains("ConditionName")) 
  {
    _useCondition = true;
    if (!retrieve_and_validate(_pp,_key,"ConditionName",_condition)) return;
  } 
  else
  {
    _useCondition = false;
  }

  _threshold = settings.value("Threshold", 3.9e6).toFloat();

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

void cass::pp103::operator()(const cass::CASSEvent& event)
{
  using namespace std;

  // if condition is in use and not met, don't do anything:
  if (_useCondition) {
      const Histogram0DFloat*cond
          (reinterpret_cast<Histogram0DFloat*>(histogram_checkout(_condition)));
      if (!cond->isTrue()) return;
  }

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
  float integral = std::accumulate(frame.begin(), frame.end(), 0.0);
  if (integral >= _threshold)
  {
    _image->lock.lockForWrite();
    copy(frame.begin(), frame.end(), _image->memory().begin());
    ++_image->nbrOfFills();
    _image->lock.unlock();
  }
}


#ifdef SINGLEPARTICLE_HIT

// *** postprocessor 104 -- single images from a CCD, only updated if the hit-helper says it's a hit. ***/

cass::pp104::pp104(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  :PostprocessorBackend(pp, key),_image(0)
{
  loadSettings(0);
}

cass::pp104::~pp104()
{
  _pp.histograms_delete(_key);
  _image = 0;
}

cass::PostProcessors::active_t cass::pp104::dependencies()
{
  PostProcessors::active_t list;
  if (_useCondition) list.push_front(_condition);
  return list;
}

void cass::pp104::loadSettings(size_t)
{
  QSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());

  //dependancy:
  if (settings.contains("ConditionName")) 
  {
    _useCondition = true;
    if (!retrieve_and_validate(_pp,_key,"ConditionName",_condition)) return;
  } 
  else
  {
    _useCondition = false;
  }

  Hit::HitHelper::instance()->loadSettings();

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

void cass::pp104::operator()(const cass::CASSEvent& event)
{
  using namespace std;

  // if condition is in use and not met, don't do anything:
  if (_useCondition) {
      const Histogram0DFloat*cond
          (reinterpret_cast<Histogram0DFloat*>(histogram_checkout(_condition)));
      if (!cond->isTrue()) return;
  }

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

  if (Hit::HitHelper::instance()->condition(event) )
  {
    _image->lock.lockForWrite();
    copy(frame.begin(), frame.end(), _image->memory().begin());
    ++_image->nbrOfFills();
    _image->lock.unlock();
  }
}





// *** postprocessor 105 -- single images from a CCD, only updated if the hit-helper2 says it's a hit. ***/

cass::pp105::pp105(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  :PostprocessorBackend(pp, key),_image(0)
{
  loadSettings(0);
}

cass::pp105::~pp105()
{
  _pp.histograms_delete(_key);
  _image = 0;
}

cass::PostProcessors::active_t cass::pp105::dependencies()
{
  PostProcessors::active_t list;
  if (_useCondition) list.push_front(_condition);
  return list;
}

void cass::pp105::loadSettings(size_t)
{
  QSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());

  //dependancy:
  if (settings.contains("ConditionName")) 
  {
    _useCondition = true;
    if (!retrieve_and_validate(_pp,_key,"ConditionName",_condition)) return;
  } 
  else
  {
    _useCondition = false;
  }

  Hit2::HitHelper2::instance()->loadSettings();

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

void cass::pp105::operator()(const cass::CASSEvent& event)
{
  using namespace std;

  // if condition is in use and not met, don't do anything:
  if (_useCondition) {
      const Histogram0DFloat*cond
          (reinterpret_cast<Histogram0DFloat*>(histogram_checkout(_condition)));
      if (!cond->isTrue()) return;
  }

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

  if (Hit2::HitHelper2::instance()->wasHit(event) )
  {
    _image->lock.lockForWrite();
    copy(frame.begin(), frame.end(), _image->memory().begin());
    ++_image->nbrOfFills();
    _image->lock.unlock();
  }
}

#endif /* SINGLEPARTICLE_HIT */




// *** postprocessor 4100 -- single images from a CCD with extra requirements on integral ***

cass::pp4100::pp4100(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  :PostprocessorBackend(pp, key),_image(0)
{
  loadSettings(0);
}

cass::pp4100::~pp4100()
{
  _pp.histograms_delete(_key);
  _image = 0;
}

cass::PostProcessors::active_t cass::pp4100::dependencies()
{
  PostProcessors::active_t list;
  if (_useCondition) list.push_front(_condition);
  return list;
}

void cass::pp4100::loadSettings(size_t)
{
  QSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());

  //dependancy:
  if (settings.contains("ConditionName")) 
  {
    _useCondition = true;
    if (!retrieve_and_validate(_pp,_key,"ConditionName",_condition)) return;
  } 
  else
  {
    _useCondition = false;
  }
  _minIntegral=settings.value("minOfFrameIntegral",0.).toFloat();
  _minNofPhotons=static_cast<size_t>(settings.value("MinNofPhotons",0).toUInt());

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
      <<" columns. extra condition is minOfFrameIntegral > " <<_minIntegral
           << " and/or minNofPhotons > "<< _minNofPhotons
      <<std::endl;

  _pp.histograms_delete(_key);
  _image=0;
  _image = new Histogram2DFloat(cols,rows);
  _pp.histograms_replace(_key,_image);
}

void cass::pp4100::operator()(const cass::CASSEvent& event)
{
  using namespace std;

  // if condition is in use and not met, don't do anything:
  if (_useCondition) {
      const Histogram0DFloat*cond
          (reinterpret_cast<Histogram0DFloat*>(histogram_checkout(_condition)));
      if (!cond->isTrue()) return;
  }

  //check whether detector exists
  if (event.devices().find(_device)->second->detectors()->size() <= _detector)
    throw std::runtime_error(QString("PostProcessor_%1: Detector %2 does not exist in Device %3")
                             .arg(_key.c_str())
                             .arg(_detector)
                             .arg(_device).toStdString());

  //get frame and fill image//
  const PixelDetector::frame_t& frame
      ((*(event.devices().find(_device)->second)->detectors())[_detector].frame());
  _image->lock.lockForWrite();

  if(_minIntegral!=0||_minNofPhotons>0)
  {
    const float& integral
      (static_cast<float>(
          (*(event.devices().find(_device)->second)->detectors())[_detector].integral()));
    if(_minIntegral!=0 && integral>_minIntegral)
    {
      copy(frame.begin(), frame.end(), _image->memory().begin());
    }
    const size_t& numsofPhotons(
          (*(event.devices().find(_device)->second)->detectors())[_detector].pixellist().size());
    if(numsofPhotons>_minNofPhotons)
    {
      copy(frame.begin(), frame.end(), _image->memory().begin());
    }
  }
  _image->lock.unlock();
}


// *** postprocessor 4101 -- single half images from a CCD ***

cass::pp4101::pp4101(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  :PostprocessorBackend(pp, key),_image(0)
{
  loadSettings(0);
}

cass::pp4101::~pp4101()
{
  _pp.histograms_delete(_key);
  _image = 0;
}

cass::PostProcessors::active_t cass::pp4101::dependencies()
{
  PostProcessors::active_t list;
  if (_useCondition) list.push_front(_condition);
  return list;
}

void cass::pp4101::loadSettings(size_t)
{
  using namespace std;
  QSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());

  //dependancy:
  if (settings.contains("ConditionName")) 
  {
    _useCondition = true;
    if (!retrieve_and_validate(_pp,_key,"ConditionName",_condition)) return;
  } 
  else
  {
    _useCondition = false;
  }
  _Xmin=settings.value("XminOfFrame",0).toUInt();
  _Xmax=settings.value("XmaxOfFrame",1024).toUInt();

  _Ymin=settings.value("YminOfFrame",0).toUInt();
  _Ymax=settings.value("YmaxOfFrame",1024).toUInt();

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

  _Xmax=min(_Xmax,static_cast<size_t>(cols));
  _Ymax=min(_Ymax,static_cast<size_t>(rows));

  std::cout<<"Postprocessor "<<_key<<":"
      <<" will display subset of ccd image of detector "<<_detector
      <<" in device "<<_device
      <<". The image has "<<rows
      <<" rows and "<<cols
      <<" columns. " << _Xmax << " > x > " << _Xmin
      << "and " << _Ymax << " > y > " << _Ymin
      <<std::endl;

  _pp.histograms_delete(_key);
  _image=0;
  _image = new Histogram2DFloat(cols,rows);
  _pp.histograms_replace(_key,_image);
}

void cass::pp4101::operator()(const cass::CASSEvent& event)
{
  using namespace std;

  // if condition is in use and not met, don't do anything:
  if (_useCondition) {
      const Histogram0DFloat*cond
          (reinterpret_cast<Histogram0DFloat*>(histogram_checkout(_condition)));
      if (!cond->isTrue()) return;
  }

  //check whether detector exists
  if (event.devices().find(_device)->second->detectors()->size() <= _detector)
    throw std::runtime_error(QString("PostProcessor_%1: Detector %2 does not exist in Device %3")
                             .arg(_key.c_str())
                             .arg(_detector)
                             .arg(_device).toStdString());

  //get frame and fill image//
  const PixelDetector::pixelList_t& pixellist
      ((*(event.devices().find(_device)->second)->detectors())[_detector].pixellist());
  PixelDetector::pixelList_t::const_iterator it(pixellist.begin());
  _image->lock.lockForWrite();
  fill(_image->memory().begin(),_image->memory().end(),0.f);
  for (; it != pixellist.end();++it)
    if (it->x()>_Xmin && it->x() < _Xmax )
      if (it->y()>_Ymin && it->y() < _Ymax )
        _image->fill(it->x(),it->y(),it->z());

  //  copy(frame.begin(), frame.end(), _image->memory().begin());
  _image->lock.unlock();
}




// Local Variables:
// coding: utf-8
// mode: C++
// c-file-style: "Stroustrup"
// c-file-offsets: ((c . 0) (innamespace . 0))
// fill-column: 100
// End:
