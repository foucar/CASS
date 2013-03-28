// Copyright (C) 2011 Lutz Foucar

/**
 * @file cass_pixeldetector/shm_deserializer.cpp contains functors to
 *                            deserialize the data stream sent by shm2tcp.
 *
 * @author Lutz Foucar
 */

#include <QtCore/QDataStream>

#include "shm_deserializer.h"

#include "cass_event.h"
#include "pixeldetector.hpp"
#include "hlltypes.h"

using namespace cass;
using namespace pixeldetector;
using namespace std;
using Streaming::operator >>;

size_t SHMStreamer::operator ()(QDataStream& stream)
{
  hllDataTypes::Frms6FileHeader fileHead;
  stream >> fileHead;
  _width = fileHead.the_width;
  return sizeof(hllDataTypes::Frms6FileHeader);
}

size_t SHMStreamer::operator ()(QDataStream& stream, CASSEvent& evt)
{
  size_t nBytesRead(0);
  /** read frame header and calculate the frame data size from the contained
   *  info. Set the eventid according to the id in the info Then read the frame
   *  from the stream into the frame buffer.
   */
  hllDataTypes::FrameHeader frameHead;
  stream >> frameHead;
  nBytesRead += sizeof(hllDataTypes::FrameHeader);
  evt.id() = frameHead.external_id;
  const size_t framesize(_width * frameHead.the_height);
  const size_t framesizeBytes(framesize * sizeof(hllDataTypes::pixel));
  _hllFrameBuffer.resize(framesize);
  stream.readRawData(reinterpret_cast<char*>(&_hllFrameBuffer.front()), framesizeBytes);
  nBytesRead += framesizeBytes;

  /** get the detector associated with the frame info id from the event */
  CASSEvent::devices_t &devices(evt.devices());
  CASSEvent::devices_t::iterator devIt(devices.find(CASSEvent::PixelDetectors));
  if(devIt == devices.end())
    throw runtime_error("SHMStreamer: There is no pixeldetector device within the CASSEvent");
  Device &dev(*dynamic_cast<Device*>(devIt->second));
  Detector &det(dev.dets()[frameHead.id]);

  /** set the information of the frame to the detector */
  det.columns() = _width/2;
  det.rows()    = frameHead.the_height*2;
  det.frame().resize(_hllFrameBuffer.size());

  /** convert the hll type frame to the cass type frame */
  const size_t quadrantColumns = frameHead.the_height;
  const size_t quadrantRows = quadrantColumns; /** @todo: read out somehow? */
  const size_t HLLColumns = _width;
  hllDataTypes::HLL2CASS(_hllFrameBuffer,det.frame(),quadrantColumns,quadrantRows,HLLColumns);

  return nBytesRead;
}
