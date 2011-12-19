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
struct deserializeSHM
{
  /** deserialize stream
   *
   * @return true when the stream was successfully deserialized
   * @param stream The stream that contains the serialized data
   * @param evt The CASS Event that the data should be deserialized to.
   */
  bool operator()(QDataStream& stream, CASSEvent& evt);

  /** read header off from stream
   *
   * @param stream The stream that contains the serialized data
   */
  void operator()(QDataStream& stream);

private:
  /** the width of the framaes*/
  int _width;

  /** a buffer to store frame data that needs to be converted to the cass structur */
  std::vector<int16_t> _hllFrameBuffer;
 };
}//end namespace pnCCD
}//end namespace CASS

#endif
