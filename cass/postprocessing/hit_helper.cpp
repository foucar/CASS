//Copyright (C) 2010 Lutz Foucar

#include <QtCore/QSettings>
#include <cmath>

#include "hit_helper.h"
#include "ccd_device.h"
#include "pixel_detector.h"
#include "cass.h"
#include "histogram.h"

//initialize static members//
QMutex cass::Hit::HitHelper::_mutex;
cass::Hit::HitHelper *cass::Hit::HitHelper::_instance(0);

cass::Hit::HitHelper* cass::Hit::HitHelper::instance()
{
  //lock this//
  QMutexLocker lock(&_mutex);
  //check if an instance of the helper class already exists//
  //return it, otherwise create one and return it//
  if (0 == _instance)
  {
    VERBOSEOUT(std::cout << "creating an instance of the Hit Helper"
               <<std::endl);
    _instance = new HitHelper();
  }
  return _instance;
}

void cass::Hit::HitHelper::destroy()
{
  //lock this//
  QMutexLocker lock(&_mutex);
  delete _instance;
  _instance=0;
}


cass::Hit::HitHelper::HitHelper()
{
  using namespace std;
  loadSettings();
  for (size_t i=0 ; i<NbrOfWorkers ; ++i)
    _conditionList.push_back(make_pair(0,false));
}

void cass::Hit::HitHelper::loadSettings()
{
  using namespace std;
  QSettings settings;
  settings.beginGroup("HitSpecial");
  _threshold = settings.value("Threshold", 4e6).toFloat();

  _device = static_cast<CASSEvent::Device>(0);
  _detector = 1;
}

bool cass::Hit::HitHelper::process(const CASSEvent& event)
{
  bool cond(false);
  using namespace std;

  if (event.devices().find(_device)->second->detectors()->size() <= _detector)
    throw std::runtime_error(QString("Hit Helper: Detector %2 does not exist in Device %3")
                             .arg(_detector)
                             .arg(_device).toStdString());

  const PixelDetector::frame_t& frame
      ((*(event.devices().find(_device)->second)->detectors())[_detector].frame());
  
  float integral = std::accumulate(frame.begin(), frame.end(), 0.0);
  if (integral >= _threshold) cond=true;

  return cond;
}


