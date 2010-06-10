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
  int r;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  _device = static_cast<CASSEvent::Device>(settings.value("Device",0).toUInt());
  _detector = settings.value("Detector",0).toUInt();
  _gate = std::make_pair<float,float>(settings.value("LowerGateEnd",-1e6).toFloat(),
                                      settings.value("UpperGateEnd", 1e6).toFloat());

  // Check dependencies
  r = retrieve_and_validate(_pp,_key,"Condition",_condition);
  r &= retrieve_and_validate(_pp,_key,"AveragedImage",_idAverage);
  // If either dependency did not succeed, go away for now and try again later
  if ( !r ) return;

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
  int r;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  _device = static_cast<CASSEvent::Device>(settings.value("Device",0).toUInt());
  _detector = settings.value("Detector",0).toUInt();

  // Check dependencies
  r = retrieve_and_validate(_pp,_key,"Condition",_condition);
  r &= retrieve_and_validate(_pp,_key,"AveragedImage",_idAverage);
  // If either dependency did not succeed, go away for now and try again later
  if ( !r ) return;  _pp.histograms_delete(_key);

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



// *** Postprocessor 212 - advanced photon finding and dump events to file ***

cass::pp212::pp212(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

cass::pp212::~pp212()
{
  _fh.close();
  _pp.histograms_delete(_key);
}

cass::PostProcessors::active_t cass::pp212::dependencies()
{
  PostProcessors::active_t list;
  list.push_front(_input);
  return list;
}

void cass::pp212::loadSettings(size_t)
{
  QSettings settings;
  int r;

  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());

  // Pre-gate (don't even think about pixels outside this range)
  _pregate = std::make_pair<float,float>
                            (settings.value("LowerPreGate", -1e6).toFloat(),
                             settings.value("UpperPreGate",  1e6).toFloat());

  // Gate (store events within this range after coalescing)
  _gate = std::make_pair<float,float>
                               (settings.value("LowerGate", -1e6).toFloat(),
                                settings.value("UpperGate",  1e6).toFloat());

  // Coalesce pixels into single events?
  _coalesce = settings.value("Coalesce",false).toBool();

  // Input image
  r = retrieve_and_validate(_pp, _key, "Input", _input);

  // Condition
  r &= retrieve_and_validate(_pp, _key, "ConditionName", _condition);

  // If either dependency did not succeed, go away for now and try again later
  if ( !r ) return;

  // Output filename
  _filename = settings.value("Filename", "events.lst").toString().toStdString();

  // Open file
  _fh.open(_filename.c_str());
  _fh << " x   y  val/ADU npix" << std::endl;

  std::cout << "Postprocessor " << _key << ":"
      << "Lower Gate:" << _gate.first << " "
      << "Upper Gate: " << _gate.second << " "
      << "Lower Pre-Gate:" << _pregate.first << " "
      << "Upper Pre-Gate:" << _pregate.second << " "
      << "Input:" << _input << " "
      << "Filename: " << _filename
      << std::endl;
}


// Recursively check this pixel and its neighbours
bool cass::pp212::check_pixel(float *f, int x, int y, int w, int h,
                              double &val, int &depth)
{
  if ( depth > 5 ) return 0;
  if ( (f[x+w*y]<_pregate.first) || (f[x+w*y]>_pregate.second) ) return 0;

  depth++;
  val += f[x+w*y];

  // Set this pixel to a value lower than the pre-gate value, so
  //  that it won't ever be returned to.
  f[x+w*y] = _pregate.first - 1.0;

  // Check all neighbours (including diagonals)
  if ( x<w-1 ) check_pixel(f, x+1, y, w, h, val, depth);
  if ( x>0 ) check_pixel(f, x-1, y, w, h, val, depth);
  if ( y<w-1) check_pixel(f, x, y+1, w, h, val, depth);
  if ( y>0) check_pixel(f, x, y-1, w, h, val, depth);
  if ( (x<w-1) && (y<h-1) ) check_pixel(f, x+1, y+1, w, h, val, depth);
  if ( (x>0) && (y<h-1) ) check_pixel(f, x-1, y+1, w, h, val, depth);
  if ( (x<w-1) && (y>0) ) check_pixel(f, x+1, y-1, w, h, val, depth);
  if ( (x>0) && (y>0) ) check_pixel(f, x-1, y-1, w, h, val, depth);

  return 1;
}


void cass::pp212::operator()(const CASSEvent &)
{
  using namespace std;

  // Check condition, and do nothing if not true
  const Histogram0DFloat*cond(reinterpret_cast<Histogram0DFloat*>
                              (histogram_checkout(_condition)));
  if ( !cond->isTrue() ) return;

  // Get the input histogram
  HistogramFloatBase *image(dynamic_cast<HistogramFloatBase*>
                            (_pp.histograms_checkout().find(_input)->second));
  _pp.histograms_release();

  // Get width/height
  const int w(image->axis()[HistogramBackend::xAxis].size());
  const int h(image->axis()[HistogramBackend::yAxis].size());

  // Detect events and write to file

  if ( _coalesce ) {

    // Copy the input, because it'll get modified here
    float *in(new float[w*h+8]);
    copy(image->memory().begin(), image->memory().end(), in);

    for ( int x=0; x<w; x++ ) {
    for ( int y=0; y<h; y++ ) {

      double sum = 0.0;
      int depth = 0;

      // Skip over quickly if this pixel isn't interesting
      if ( (in[x+w*y] < _pregate.first) || (in[x+w*y] > _pregate.second) )
         continue;

      // Check this pixel and its neighbours
      if ( check_pixel(in, x, y, w, h, sum, depth) )
      {
         _fh << x << " " << y << " " << sum << " " << depth << std::endl;
      }

    }
    }

    delete[](in);

  } else {

    // Do it the easier and faster way
    for ( int x=0; x<w; x++ ) {
    for ( int y=0; y<h; y++ ) {

      // Skip over quickly if this pixel isn't interesting
      if ( (image->memory()[x+w*y] < _gate.first)
        || (image->memory()[x+w*y] > _gate.second) ) continue;

      // Save pixel
      _fh << x << " " << y << " " << image->memory()[x+w*y] << " 1" << std::endl;

    }
    }

   }

   _fh << std::endl;
}
