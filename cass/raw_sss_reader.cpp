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
#include "cass_settings.h"
#include "ccd_device.h"

using namespace cass;
using namespace cass::CCD;
using namespace std;

RAWSSSReader::RAWSSSReader()
  :_newFile(true),
    _height(0),
    _width(0),
    _nimages(0),
    _imagecounter(0)
{}

void RAWSSSReader::loadSettings()
{
}

bool RAWSSSReader::operator ()(ifstream &file, CASSEvent& event)
{
  /** if it is a new file read the file header first */
  if (_newFile)
  {
    _newFile = false;
    _imagecounter = 0;
    _width = FileStreaming::retrieve<uint32_t>(file);
    _height = FileStreaming::retrieve<uint32_t>(file);
    _nimages = FileStreaming::retrieve<uint32_t>(file);
  }

  ++_imagecounter;
  if (_imagecounter > _nimages)
  {
    stringstream ss;
    ss << "RAWSSSReader(): We are trying to read more '"<<_imagecounter
       <<"' images in the file than there reported to be in the file"
       <<" by the header '"<<_nimages<<"'";
    throw runtime_error(ss.str());
  }

  uint32_t eventId(FileStreaming::retrieve<uint32_t>(file));
  vector<uint8_t> image(_width*_height,0);
  file.read(reinterpret_cast<char*>(&image.front()),image.size());
  uint32_t heightCompare(FileStreaming::retrieve<uint32_t>(file));

  if (heightCompare != _height)
  {
    stringstream ss;
    ss << "RAWSSSReader(): The read height '"<<heightCompare
       <<"' does not match to the height given in the header '"<<_height<<"'";
    throw runtime_error(ss.str());
  }

  CCDDevice *dev(dynamic_cast<CCDDevice*>(event.devices()[CASSEvent::CCD]));
  if(dev->detectors()->empty())
    dev->detectors()->resize(1);
  PixelDetector& det(dev->detectors()->front());
  det.columns() = _width;
  det.rows()    = _height;
  det.originalcolumns() = _width;
  det.originalrows()    = _height;
  det.frame().resize(image.size());
  copy(image.begin(),image.end(),det.frame().begin());

  return true;
}
