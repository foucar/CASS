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
#include "pnccd_device.h"
#include "pixel_detector.h"
#include "hlltypes.h"

using namespace cass;
using namespace pnCCD;
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

  /** get just the first detector from the event and copy the info from the header to it */
  pnCCDDevice *dev(dynamic_cast<pnCCDDevice*>(evt.devices()[CASSEvent::pnCCD]));
  if(dev->detectors()->empty())
    dev->detectors()->resize(1);
  PixelDetector& det(dev->detectors()->front());
  det.columns() = _width/2;
  det.rows()    = frameHead.the_height*2;
  det.originalcolumns() = _width/2;
  det.originalrows()    = frameHead.the_height*2;
  det.frame().resize(_hllFrameBuffer.size());

  /** convert the hll type frame to the cass type frame */
  size_t quadrantColumns = frameHead.the_height;
  size_t quadrantRows = quadrantColumns; /** @todo: read out somehow? */
  size_t HLLColumns = _width;
  hllDataTypes::HLL2CASS(_hllFrameBuffer,det.frame(),quadrantColumns,quadrantRows,HLLColumns);

  return nBytesRead;
}
