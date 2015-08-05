// Copyright (C) 2011 Lutz Foucar

/**
 * @file tcp_streamer.h contains base class for all tcp streamers
 *
 * @author Lutz Foucar
 */

#ifndef _TCPSTREAMER_H_
#define _TCPSTREAMER_H_

#include <tr1/memory>
#include <string>

#include <QtCore/QDataStream>

namespace cass
{
class CASSEvent;

/** base class for all tcp streamers
 *
 * @author Lutz Foucar
 */
class TCPStreamer
{
public:
  /** typedef the shared pointer of this */
  typedef std::tr1::shared_ptr<TCPStreamer> shared_pointer;

  /** virtual destructor */
  virtual ~TCPStreamer() {}

  /** create an instance of the requested type and return a reference
   *
   * @return a refrerence to the object contained in the shared pointer
   * @param type the reqested type
   */
  static TCPStreamer& instance(const std::string &type);

  /** return a reference to the derefenced instance
   *
   * @return a refrerence to the object contained in the shared pointer
   */
  static TCPStreamer& instance();

  /** deserialize stream
   *
   * @return the nbr of bytes read of from stream
   * @param stream The stream that contains the serialized data
   * @param evt The CASS Event that the data should be deserialized to.
   */
  virtual size_t operator()(QDataStream &stream, CASSEvent& evt)=0;

  /** deserialize the file header
   *
   * @return the nbr of bytes read of from stream
   * @param stream the datastream containing the header information
   */
  virtual size_t operator()(QDataStream&/*stream*/) {return 0;}

protected:
  /** only inheritants can create this */
  TCPStreamer() {}

  /** a sigleton instance */
  static shared_pointer _instance;
};
} //end namespace cass
#endif
