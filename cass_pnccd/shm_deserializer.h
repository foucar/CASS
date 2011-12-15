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

namespace cass
{
class CASSEvent;

namespace pnCCD
{
/** deserialize the data stream of shm2tcp program
 *
 * details
 *
 * @author Lutz Foucar
 */
struct deserializeSHM
{
  /** do it
   *
   * @return true when the stream was successfully deserialized
   * @param stream The stream that contains the serialized data
   * @param evt The CASS Event that the data should be deserialized to.
   */
  bool operator()(QDataStream& stream, CASSEvent& evt);
};
}//end namespace pnCCD
}//end namespace CASS

#endif
