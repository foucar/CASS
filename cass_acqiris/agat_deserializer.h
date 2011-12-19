// Copyright (C) 2011 Lutz Foucar

/**
 * @file agat_deserializer.h contains functions to deserialize the data stream
 *                           sent by agat.
 *
 * @author Lutz Foucar
 */

#ifndef __AGATDESERIALIZE_H__
#define __AGATDESERIALIZE_H__

class QDataStream;

#include "tcp_streamer.h"

namespace cass
{
class CASSEvent;

namespace ACQIRIS
{
/** deserialize the data stream of the regular agat program
 *
 * see operator for details
 *
 * @author Lutz Foucar
 */
class AGATStreamer : public TCPStreamer
{
public:
  /** deserialize stream
   *
   * @return number of bytes read off the stream
   * @param stream The stream that contains the serialized data
   * @param evt The CASS Event that the data should be deserialized to.
   */
  size_t operator()(QDataStream& stream, CASSEvent& evt);
};
}//end namespace ACQIRIS
}//end namespace CASS

#endif
