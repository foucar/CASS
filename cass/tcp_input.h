// Copyright (C) 2011, 2012 Lutz Foucar

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

class QTcpSocket;

namespace cass
{
/** TCP Input for receiving data
 *
 * This class is a thread that to a TCP Server and retrieves the data from it.
 * it expects that before the payload conatining the data arrives the size of
 * the payload is transmitted.
 *
 * @cassttng TCPInput/{Server}\n
 *           The name or ip address of the machine that the server is running on.
 *           Default is "localhost"
 * @cassttng TCPInput/{Port}\n
 *           The port that the TCP Server is listening for connections on.
 *           Default is "9090"
 * @cassttng TCPInput/{DataType}\n
 *           The type of data that is streamed from the tcp server. Default is
 *           "agat". Possible values are:
 *           - "agat": The type of data that is streamed from a normal version
 *                     of AGAT3.
 *           - "shmToOld": The type of data that is streamed from the RACOON
 *                         shm2tcp server. The output is written to the old
 *                         pnCCD container to be used with the old analysis chain.
 *           - "shm": Type of data that is streamed from RACOON shm2tcp server.
 *                    is to be used with the new ccd analysis chain.
 *
 * @author Lutz Foucar
 */
class TCPInput : public InputBase
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
   * @param parent the parent of this object
   */
  static void instance(RingBuffer<CASSEvent,RingBufferSize>& buffer,
                       Ratemeter &ratemeter,
                       QObject *parent=0);

  /** starts the thread
   *
   * Starts the thread and the loop that waits for data. When an timout occured
   * it will just restart the loop until the quit flag is set.
   */
  void run();

  /** do not load anything */
  void load() {}

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
   * @param parent the parent of this object
   */
  TCPInput(RingBuffer<CASSEvent,RingBufferSize>& buffer,
           Ratemeter &ratemeter,
           QObject *parent=0);

  /** check the socket
   *
   * if the socket is connected then just return otherwise try connecting it to
   * the server
   *
   * @param socket the socket to check
   */
  void checkSocket(QTcpSocket &socket);

  /** connect the socket to server
   *
   * tries to connect the socket to the server as long as user did not finish
   * the program or the socket has connected to the server
   *
   * @param socket the socket that one should connect to the server
   */
  void connectToServer(QTcpSocket &socket);
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
