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





// Local Variables:
// coding: utf-8
// mode: C++
// c-file-style: "Stroustrup"
// c-file-offsets: ((c . 0) (innamespace . 0))
// fill-column: 100
// End:
