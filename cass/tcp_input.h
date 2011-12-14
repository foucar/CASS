// Copyright (C) 2011 Lutz Foucar

/**
 * @file tcp_input.h contains input that uses tcp as interface
 *
 * @author Lutz Foucar
 */

#ifndef __TCPINPUT_H__
#define __TCPINPUT_H__

#include <string>

#include "cass.h"
#include "input_base.h"
#include "cass_event.h"
#include "ringbuffer.h"

namespace cass
{
  /** TCP Input for receiving data
   *
   * This class is a thread that to a TCP Server and retrieves the data from it.
   * it expects that before the payload conatining the data arrives the size of
   * the payload is transmitted.
   *
   * @author Lutz Foucar
   */
  class TCPInput : public InputBase
  {
  public:
    /** constructor
     *
     * creates the thread. The base class will create the interface to the shared
     * memory.
     *
     * @param buffer the ringbuffer, that we take events out and fill it
     *        with the incomming information
     * @param ratemeter reference to the ratemeter to measure the rate of the input
     * @param parent the parent of this object
     */
      TCPInput(RingBuffer<CASSEvent,RingBufferSize>& buffer,
               Ratemeter &ratemeter,
               QObject *parent=0);

    /** starts the thread
     *
     * Starts the thread and the loop that waits for data. When an timout occured
     * it will just restart the loop until the quit flag is set.
     */
    void run();

  private:
  };

}//end namespace cass

#endif



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
