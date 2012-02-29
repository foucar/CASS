// Copyright (C) 2011 Lutz Foucar

/**
 * @file raw_sss_reader.cpp contains the class to read commercial ccd files
 *                          created with Per Johnsonn's program
 *
 * @author Lutz Foucar
 */
#include <sstream>
#include <stdexcept>

#include "raw_sss_reader.h"

#include "cass_event.h"
#include "pixeldetector.hpp"
#include "log.h"

using namespace cass;
using namespace pixeldetector;
using namespace std;
using Streaming::operator >>;

RAWSSSReader::RAWSSSReader()
  : _imagecounter(0)
{}

void RAWSSSReader::loadSettings()
{
}

void RAWSSSReader::readHeaderInfo(std::ifstream &file)
{
  _imagecounter = 0;
  file >> _header;
  _imageBuffer.resize(_header.width*_header.height,0);
  _imageSize = _imageBuffer.size() * sizeof(sssFile::image_t::value_type);
  Log::add(Log::VERBOSEINFO,"RAWSSSReader(): '" + _filename +"' contains '" +
           toString(_header.nFrames) +"' images");
}

bool RAWSSSReader::operator ()(ifstream &file, CASSEvent& event)
{
  ++_imagecounter;
  if (_imagecounter > _header.nFrames)
    throw runtime_error("RAWSSSReader(): We are trying to read more '" +
                        toString(_imagecounter) + "' images in '" + _filename +
                        "' than there reported to be in the file header '" +
                        toString(_header.nFrames) + "'");
  event.id() = Streaming::retrieve<uint32_t>(file);

  file.read(reinterpret_cast<char*>(&_imageBuffer.front()),_imageSize);

  CASSEvent::devices_t &devices(event.devices());
  CASSEvent::devices_t::iterator devIt(devices.find(CASSEvent::PixelDetectors));
  if(devIt == devices.end())
    throw runtime_error("RAWSSSReader: There is no pixeldetector device within the CASSEvent");
  Device &dev(*dynamic_cast<Device*>(devIt->second));
  Detector &det(dev.dets()[100]);
  det.columns() = _header.width;
  det.rows()    = _header.height;
  det.frame().resize(_imageBuffer.size());

  copy(_imageBuffer.begin(),_imageBuffer.end(),det.frame().begin());

  return event.id();
}
