// Copyright (C) 2011 Lutz Foucar

/**
 * @file frms6_reader.cpp contains class to read frms6 files created by Xonline
 *
 * @author Lutz Foucar
 */

#include <iterator>

#include "frms6_reader.h"

#include "cass_event.h"
#include "cass_settings.h"
#include "pnccd_device.h"

using namespace cass;
using namespace pnCCD;
using namespace std;

Frms6Reader::Frms6Reader()
{}

void Frms6Reader::loadSettings()
{
}

void Frms6Reader::readHeaderInfo(std::ifstream &file)
{
  file >> _fileHead;
}

bool Frms6Reader::operator ()(ifstream &file, CASSEvent& evt)
{
  file >> _frameHead;
  const size_t framesize(_fileHead.the_width * _frameHead.the_height);
  const size_t framesizeBytes(framesize * sizeof(hllDataTypes::pixel));
  _hllFrameBuffer.resize(framesize);
  file.read(reinterpret_cast<char*>(&_hllFrameBuffer.front()), framesizeBytes);

  evt.id() = _frameHead.external_id;

  pnCCDDevice *dev(dynamic_cast<pnCCDDevice*>(evt.devices()[CASSEvent::pnCCD]));
  if(dev->detectors()->empty())
    dev->detectors()->resize(1);
  PixelDetector& det(dev->detectors()->front());
  det.columns() = _fileHead.the_width/2;
  det.rows()    = _frameHead.the_height*2;
  det.originalcolumns() = _fileHead.the_width/2;
  det.originalrows()    = _frameHead.the_height*2;
  det.frame().resize(_hllFrameBuffer.size());

  /** convert hll frame format to cass frame format */
  size_t quadrantColumns = _frameHead.the_height;
  size_t quadrantRows = quadrantColumns; /** @todo: read out somehow? */
  size_t HLLColumns = _fileHead.the_width;
  hllDataTypes::HLL2CASS(_hllFrameBuffer,det.frame(),quadrantColumns,quadrantRows,HLLColumns);

  return (evt.id());
}
