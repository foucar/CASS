// Copyright (C) 2017 Lutz Foucar

/**
 * @file zmq_input.h contains input that uses ZMQ as interface
 *
 * @author Lutz Foucar
 */

#ifndef __XFELONLINEINPUT_H__
#define __XFELONLINEINPUT_H__

#include <string>

#include "cass.h"
#include "input_base.h"
#include "cass_event.h"
#include "ringbuffer.hpp"


namespace cass
{
/** XFEL Input for receiving data
 *
 * This class is a thread that connects to a XFEL Server and retrieves the data
 * from it.
 *
 * @cassttng ZMQInput/{Server}\n
 *           The name or ip address of the machine that the server is running on.
 *           Default is "localhost"
 * @cassttng ZMQInput/{PathToImage}\n
 *           path to the image data within the transferred data.
 *
 * @author Lutz Foucar
 */
class XFELOnlineInput : public InputBase
{
public:
  /** create an instance of this
   *
   * this initializes the _instance member of the base class. Check here if
   * it is already initialized, if so throw logic error.
   *
   * @param buffer the ringbuffer, that we take events out and fill it
   *        with the incomming information
   * @param ratemeter reference to the ratemeter to measure the rate of the input
   * @param loadmeter reference to the ratemeter to measure the load of the input
   * @param quitwhendone flag to tell whether to quit the input when done
   * @param parent the parent of this object
   */
  static void instance(RingBuffer<CASSEvent>& buffer,
                       Ratemeter &ratemeter, Ratemeter &loadmeter,
                       bool quitwhendone=false,
                       QObject *parent=0);

  /** starts the thread
   *
   * Starts the thread and the loop that waits for data. When an timout occured
   * it will just restart the loop until the quit flag is set.
   */
  void runthis();

  /** do not load anything */
  void load() {}

  /** retrieve the number of processed events
   *
   * @return number of processed events
   */
  uint64_t eventcounter() {return _counter;}

  /** retrieve the number of skipped processed events
   *
   * @return number of processed events
   */
  uint64_t skippedeventcounter() {return _scounter;}

private:
  /** constructor
   *
   * creates the thread. Connects to the tcp server and then retrieves the
   * data streams. The data within the stream will be deserialized with the
   * help of deserialization functions, where the user has to choose which
   * one is appropriate via the .ini file parameters. The thread runs as long
   * as noone calls the end() member of the base class.
   * In case a timeout occurs when waiting for a new event, it will just continue
   * and wait for the next timeout. In case that a timeout occurred when waiting
   * for the data of an event it throws an runtime error.
   *
   * @param buffer the ringbuffer, that we take events out and fill it
   *        with the incomming information
   * @param ratemeter reference to the ratemeter to measure the rate of the input
   * @param loadmeter reference to the ratemeter to measure the load of the input
   * @param quitwhendone flag to tell whether to quit the input when done
   * @param parent the parent of this object
   */
  XFELOnlineInput(RingBuffer<CASSEvent>& buffer,
                  Ratemeter &ratemeter, Ratemeter &loadmeter, bool quitwhendone,
                  QObject *parent=0);

  /** flag to tell the thread to quit when its done with all files */
  bool _quitWhenDone;

  /** the counter for all events */
  uint64_t _counter;

  /** the counter for all events */
  uint64_t _scounter;
};

}//end namespace cass

#endif
