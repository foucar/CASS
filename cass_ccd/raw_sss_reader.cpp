// Copyright (C) 2011 Lutz Foucar

/**
 * @file cass_ccd/raw_sss_reader.cpp contains the class to read commercial ccd
 *                                   files created with Per Johnsonn's program
 *
 * @author Lutz Foucar
 */
#include <sstream>
#include <stdexcept>

#include "raw_sss_reader.h"

#include "cass_event.h"
#include "cass_settings.h"
#include "ccd_device.h"
#include "log.h"

using namespace cass;
using namespace cass::CCD;
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
                        toString(_imagecounter)+
                        "' images in the file than there reported to be in the file" +
                        " by the header '" + toString(_header.nFrames) + "'");
  event.id() = Streaming::retrieve<uint32_t>(file);

  file.read(reinterpret_cast<char*>(&_imageBuffer.front()),_imageSize);

  CCDDevice *dev(dynamic_cast<CCDDevice*>(event.devices()[CASSEvent::CCD]));
  if(dev->detectors()->empty())
    dev->detectors()->resize(1);
  PixelDetector& det(dev->detectors()->front());
  det.columns() = _header.width;
  det.rows()    = _header.height;
  det.originalcolumns() = _header.width;
  det.originalrows()    = _header.height;
  det.frame().resize(_imageBuffer.size());

  copy(_imageBuffer.begin(),_imageBuffer.end(),det.frame().begin());

  return event.id();
}
