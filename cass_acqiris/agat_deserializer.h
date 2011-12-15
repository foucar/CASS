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
struct deserializeNormalAgat
{
  /** deserialize stream
   *
   * @return true when the stream was successfully deserialized
   * @param stream The stream that contains the serialized data
   * @param evt The CASS Event that the data should be deserialized to.
   */
  bool operator()(QDataStream& stream, CASSEvent& evt);
};
}//end namespace ACQIRIS
}//end namespace CASS

#endif
