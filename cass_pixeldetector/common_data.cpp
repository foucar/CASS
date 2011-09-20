//Copyright (C) 2011 Lutz Foucar

/**
 * @file common_data.cpp data contains commonly used for all AdvancedDetectors.
 *
 * @author Lutz Foucar
 */

#include <stdexcept>
#include <sstream>

#include "common_data.h"

#include "cass_settings.h"
#include "cass_event.h"
#include "advanced_pixeldetector.h"

using namespace cass;
using namespace pixeldetector;
using namespace std;


//initialize static members//
CommonData::instancesmap_t CommonData::_instances;
QMutex CommonData::_mutex;

CommonData::shared_pointer CommonData::instance(const instancesmap_t::key_type& detector)
{
  QMutexLocker lock(&_mutex);
  if (_instances.find(detector) == _instances.end())
  {
    VERBOSEOUT(std::cout << "CommonData::instance(): creating an"
               <<" instance of the Pixel Detector Helper for detector '"<<detector
               <<"'"
               <<std::endl);
    _instances[detector] = CommonData::shared_pointer(new CommonData(detector));
  }
  return _instances[detector];
}

CommonData::CommonData(const instancesmap_t::key_type& /*detname*/)
{}

void CommonData::loadSettings(CASSSettings &s)
{
  s.beginGroup("CorrectionMaps");
  string mapcreatortype(s.value("MapCreatorType","none").toString().toStdString());
  _mapcreator = MapCreatorBase::instance(mapcreatortype);
  _mapcreator->loadSettings(s);
  string offsetfilename(s.value("OffsetNoiseFilename","").toString().toStdString());
  string offsetfiletype(s.value("OffsetNoiseFiletype","hll").toString().toStdString());
  if (offsetfiletype == "hll")
    readHLLOffsetFile(offsetfilename, *this);
  else if(offsetfiletype == "cass")
    readCASSOffsetFile(offsetfilename, *this);
  else
  {
    stringstream ss;
    ss <<"CommonData::loadSettings: OffsetNoiseFiletype '"<<offsetfiletype
       <<"' does not exist";
    throw invalid_argument(ss.str());
  }
  string gainfilename(s.value("CTEGainFilename","").toString().toStdString());
  string gainfiletype(s.value("CTEGainFiletype","hll").toString().toStdString());
  if (gainfiletype == "hll")
    readHLLGainFile(gainfilename, *this);
  else if(gainfiletype == "cass")
    readCASSGainFile(gainfilename, *this);
  else
  {
    stringstream ss;
    ss <<"CommonData::loadSettings: CTEGainFiletype '"<<offsetfiletype
       <<"' does not exist";
    throw invalid_argument(ss.str());
  }
  s.endGroup();
}

void CommonData::createMaps(const Frame &frame)
{
  const MapCreatorBase& createMaps(*_mapcreator);
  createMaps(frame);
}
