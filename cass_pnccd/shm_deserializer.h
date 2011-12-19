// Copyright (C) 2011 Lutz Foucar

/**
 * @file shm_deserializer.h contains functors to deserialize the data stream
 *                          sent by shm2tcp.
 *
 * @author Lutz Foucar
 */

#ifndef __SHMDESERIALIZE_H__
#define __SHMDESERIALIZE_H__

class QDataStream;

#include <vector>
#include <stdint.h>

#include "tcp_streamer.h"

namespace cass
{
class CASSEvent;

namespace pnCCD
{
/** deserialize the data stream of shm2tcp program
 *
 * see operator() for details
 *
 * @author Lutz Foucar
 */
class SHMStreamer : public TCPStreamer
{
public:
  /** deserialize stream
   *
   * @return the number of bytes read off the stream
   * @param stream The stream that contains the serialized data
   * @param evt The CASS Event that the data should be deserialized to.
   */
  size_t operator()(QDataStream& stream, CASSEvent& evt);

  /** read header off from stream
   *
   * @return the number of bytes read off the stream
   * @param stream The stream that contains the serialized data
   */
  size_t operator()(QDataStream& stream);

private:
  /** the width of the framaes*/
  int _width;

  /** a buffer to store frame data that needs to be converted to the cass structur */
  std::vector<int16_t> _hllFrameBuffer;
 };
}//end namespace pnCCD
}//end namespace CASS

#endif
