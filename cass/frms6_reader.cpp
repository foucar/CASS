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
  file.read(reinterpret_cast<char*>(&_fileHead), sizeof(frms6File::FileHeader));
}

bool Frms6Reader::operator ()(ifstream &file, CASSEvent& evt)
{
  file.read(reinterpret_cast<char*>(&_frameHead), sizeof(frms6File::FrameHeader) );
  const size_t framesize(_fileHead.the_width * _frameHead.the_height);
  const size_t framesizeBytes(framesize * sizeof(frms6File::pixel));
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

  /** @todo: remove code duplication: HLL2CASS */
  size_t quadrantColumns = _frameHead.the_height;
  size_t quadrantRows = quadrantColumns; /** @todo: read out somehow? */
  size_t HLLColumns = _fileHead.the_width;

  cass::PixelDetector::frame_t::iterator cass(det.frame().begin());

  frms6File::frame_t::const_iterator hllquadrant0
      (_hllFrameBuffer.begin());
  frms6File::frame_t::const_reverse_iterator hllquadrant1
      (_hllFrameBuffer.rbegin()+2*quadrantColumns);
  frms6File::frame_t::const_reverse_iterator hllquadrant2
      (_hllFrameBuffer.rbegin()+1*quadrantColumns);
  frms6File::frame_t::const_iterator hllquadrant3
      (_hllFrameBuffer.begin()+3*quadrantColumns);

  //copy quadrant read to right side (lower in CASS)
  for (size_t quadrantRow(0); quadrantRow < quadrantRows; ++quadrantRow)
  {
    copy(hllquadrant0,hllquadrant0+quadrantColumns,cass);
    advance(cass,quadrantColumns);
    copy(hllquadrant3,hllquadrant3+quadrantColumns,cass);
    advance(cass,quadrantColumns);

    advance(hllquadrant0,HLLColumns);
    advance(hllquadrant3,HLLColumns);
  }
  //copy quadrants read to left side (upper in CASS)
  for (size_t quadrantRow(0); quadrantRow < quadrantRows; ++quadrantRow)
  {
    copy(hllquadrant1,hllquadrant1+quadrantColumns,cass);
    advance(cass,quadrantColumns);
    copy(hllquadrant2,hllquadrant2+quadrantColumns,cass);
    advance(cass,quadrantColumns);

    advance(hllquadrant1,HLLColumns);
    advance(hllquadrant2,HLLColumns);
  }
  return (evt.id());
}
