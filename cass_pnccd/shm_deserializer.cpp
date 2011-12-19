// Copyright (C) 2011 Lutz Foucar

/**
 * @file shm_deserializer.cpp contains functors to deserialize the data stream
 *                            sent by shm2tcp.
 *
 * @author Lutz Foucar
 */

#include <QtCore/QDataStream>

#include "shm_deserializer.h"

#include "cass_event.h"
//#include "pixeldetector.hpp"
#include "pnccd_device.h"
#include "pixel_detector.h"
#include "frms6_file_header.h"

using namespace cass;
using namespace pnCCD;
using namespace std;

void deserializeSHM::operator ()(QDataStream& stream)
{
  frms6File::FileHeader fileHead;
  stream >> fileHead;
  _width = fileHead.the_width;
}

bool deserializeSHM::operator ()(QDataStream& stream, CASSEvent& evt)
{
  frms6File::FrameHeader frameHead;
  stream >> frameHead;
  const size_t framesize(_width * frameHead.the_height);
  const size_t framesizeBytes(framesize * sizeof(frms6File::pixel));
  _hllFrameBuffer.resize(framesize);
  stream.readRawData(reinterpret_cast<char*>(&_hllFrameBuffer.front()), framesizeBytes);

  evt.id() = frameHead.external_id;

  pnCCDDevice *dev(dynamic_cast<pnCCDDevice*>(evt.devices()[CASSEvent::pnCCD]));
  if(dev->detectors()->empty())
    dev->detectors()->resize(1);
  PixelDetector& det(dev->detectors()->front());
  det.columns() = _width/2;
  det.rows()    = frameHead.the_height*2;
  det.originalcolumns() = _width/2;
  det.originalrows()    = frameHead.the_height*2;
  det.frame().resize(_hllFrameBuffer.size());

  /** @todo: remove code duplication: HLL2CASS */
  size_t quadrantColumns = frameHead.the_height;
  size_t quadrantRows = quadrantColumns; /** @todo: read out somehow? */
  size_t HLLColumns = _width;

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
  return true;
}
