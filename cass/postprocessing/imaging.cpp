// Copyright (C) 2010 Lutz Foucar

#include <algorithm>
#include <cassert>
#include <functional>
#include <numeric>
#include <stdexcept>
#include <utility>
#include <math.h>

#include <QtCore/QString>

#include "ccd_device.h"
#include "pnccd_device.h"
#include "histogram.h"
#include "cass_event.h"
#include "postprocessor.h"
#include "imaging.h"
#include "acqiris_detectors_helper.h"
#include "tof_detector.h"
#include "convenience_functions.h"
#include "cass_settings.h"


// *** Postprocessor 212 - advanced photon finding and dump events to file ***

cass::pp212::pp212(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void cass::pp212::loadSettings(size_t)
{
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());

  // Pre-gate (don't even think about pixels outside this range)
  _pregate = std::make_pair(settings.value("LowerPreGate", -1e6).toFloat(),
                            settings.value("UpperPreGate",  1e6).toFloat());

  // Gate (store events within this range after coalescing)
  _gate = std::make_pair(settings.value("LowerGate", -1e6).toFloat(),
                         settings.value("UpperGate",  1e6).toFloat());

  // Coalesce pixels into single events?
  _coalesce = settings.value("Coalesce",false).toBool();

  // Input image
  _input = setupDependency("HistName");

  setupGeneral();

  bool ret (setupCondition());

  // If either dependency did not succeed, go away for now and try again later
  if ( !_input && !ret ) return;

  // Output filename
  _filename = settings.value("Filename", "events.lst").toString().toStdString();

  // Create dummy histogram to make sure we actually get called
  _result = new Histogram0DFloat();
  createHistList(1);

  // Open file
  _fh.open(_filename.c_str());
  _fh << " x   y  val/ADU npix" << std::endl;

  std::cout << "Postprocessor " << _key << ":"
      << "Lower Gate:" << _gate.first << " "
      << "Upper Gate: " << _gate.second << " "
      << "Lower Pre-Gate:" << _pregate.first << " "
      << "Upper Pre-Gate:" << _pregate.second << " "
      << "Input:" << _input->key() << " "
      << "Filename: " << _filename
      <<". Condition is "<<_condition->key()
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


void cass::pp212::process(const CASSEvent & evt)
{
  using namespace std;

  // Get the input histogram
  const HistogramFloatBase &image
      (dynamic_cast<const HistogramFloatBase&>((*_input)(evt)));

  // Get width/height
  const int w(image.axis()[HistogramBackend::xAxis].size());
  const int h(image.axis()[HistogramBackend::yAxis].size());

  // Detect events and write to file

  if ( _coalesce )
  {
    QMutexLocker lock(&_output_lock);

    // Copy the input, because it'll get modified here
    float *in(new float[w*h+8]);
    copy(image.memory().begin(), image.memory().end(), in);

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
          if ( (sum >= _gate.first) && (sum <= _gate.second) ) {
            _fh << x << " " << y << " " << sum << " " << depth << std::endl;
          }
        }

      }
    }

    delete[](in);

  } else {

    QMutexLocker lock(&_output_lock);

    // Do it the easier and faster way
    for ( int x=0; x<w; x++ ) {
      for ( int y=0; y<h; y++ ) {

        // Skip over quickly if this pixel isn't interesting
        if ( (image.memory()[x+w*y] < _gate.first)
          || (image.memory()[x+w*y] > _gate.second) ) continue;

        // Save pixel
        _fh << x << " " << y << " " << image.memory()[x+w*y] << " 1" << std::endl;

      }
    }

  }

  _fh << std::endl;
}










// *** test image ***

cass::pp240::pp240(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void cass::pp240::loadSettings(size_t)
{
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  _sizeX = settings.value("sizeX", 1024).toInt();
  _sizeY = settings.value("sizeY", 1024).toInt();
  _result = new Histogram2DFloat(_sizeX, _sizeY);
  createHistList(1);
  setupGeneral();
  std::cout<<"Postprocessor "<<_key
      <<": creates test image and has size X:"<<_sizeX
      <<" Y:"<<_sizeY
      <<std::endl;
}

void cass::pp240::process(const CASSEvent& /*event*/)
{
  using namespace std;
  //write test image
  _result->lock.lockForWrite();
  for (int xx=0; xx<_sizeX; ++xx)
    for (int yy=0; yy<_sizeY; ++yy)
    {
      dynamic_cast<Histogram2DFloat*>(_result)->bin(xx,yy) = xx*yy;
    }
  _result->lock.unlock();
}



