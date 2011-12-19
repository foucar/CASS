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
#include "pixeldetector.hpp"
#include "pixel_detector.h"
#include "frms6_file_header.h"
#include "common_data.h"

using namespace cass;
using namespace pixeldetector;
using namespace std;

size_t SHMStreamer::operator ()(QDataStream& stream)
{
  frms6File::FileHeader fileHead;
  stream >> fileHead;
  _width = fileHead.the_width;
  return sizeof(frms6File::FileHeader);
}

size_t SHMStreamer::operator ()(QDataStream& stream, CASSEvent& evt)
{
  size_t nBytesRead(0);
  frms6File::FrameHeader frameHead;
  stream >> frameHead;
  nBytesRead += sizeof(frms6File::FrameHeader);
  const size_t framesize(_width * frameHead.the_height);
  const size_t framesizeBytes(framesize * sizeof(frms6File::pixel));
  _hllFrameBuffer.resize(framesize);
  stream.readRawData(reinterpret_cast<char*>(&_hllFrameBuffer.front()), framesizeBytes);
  nBytesRead += framesizeBytes;

  evt.id() = frameHead.external_id;
  CASSEvent::devices_t &devices(evt.devices());
  CASSEvent::devices_t::iterator devIt(devices.find(CASSEvent::PixelDetectors));
  if(devIt == devices.end())
    throw runtime_error("SHMStreamer: There is no pixeldetector device within the CASSEvent");
  Device &dev(*dynamic_cast<Device*>(devIt->second));
  Detector &det(dev.dets()[frameHead.id]);

  det.columns() = _width/2;
  det.rows()    = frameHead.the_height*2;
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
  return nBytesRead;
}
