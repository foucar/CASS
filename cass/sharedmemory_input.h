// Copyright (C) 2009,2010 Lutz Foucar

/**
 * @file sharedmemory_input.h file contains declaration of class that interfaces
 *                            the LCLS shared memory
 *
 * @author Lutz Foucar
 */

#ifndef __SHAREDMEMORYINPUT_H__
#define __SHAREDMEMORYINPUT_H__

#include "cass.h"
#include "input_base.h"
#include "pdsdata/app/XtcMonitorClient.hh"
#include "ringbuffer.h"
#include "cass_event.h"

namespace cass
{
  //forward declaration//
  class FormatConverter;

  /** Shared Memory Input for receiving xtc datagrams
   *
   * This class is a thread that connects to the sahred memory of LCLS. The
   * baseclass does all the connection and once there is new data available
   * it calles processDatagram, where we can add code that we want to use.
   * The first datagram that will be send to this class is a datagram
   * containing the last known configure transition. This is to make sure that
   * when starting the program we will always get the latest state of the DAQ.
   *
   * @author Lutz Foucar
   */
  class SharedMemoryInput
    : public InputBase, Pds::XtcMonitorClient
  {
  public:
    /** constructor
     *
     * creates the thread. The base class will create the interface to the shared
     * memory.
     *
     * @param PartitionTag the name of the partition tag we want to connect to
     * @param buffer the ringbuffer, that we take events out and fill it
     *        with the incomming information
     * @param index the client index of the shared memory
     * @param parent the parent of this object
     */
    SharedMemoryInput(char * PartitionTag,
                      int index,
                      cass::RingBuffer<cass::CASSEvent,cass::RingBufferSize>& buffer,
                      QObject *parent=0);

    /** deletes the thread*/
    ~SharedMemoryInput();

    /** starts the thread
     *
     * Starts the thread and the loop in the shared memory client we inerhited
     * from. The loop will be notified when there are new events available in
     * the shared memory.
     */
    void run();

    /** overwrite the base class function with our code
     *
     * This is called once the eventqueue has new data available.
     *
     * @return the errorcode, when != 0, then the baseclasses will exit its loop
     * @param[in] dg The datagram we can work on
     */
    int processDgram(Pds::Dgram*dg);

    /** do all clean up when quitting
     *
     * this function from the base class needs to be overwritten, since it might
     * happen that we loose connection to the shared memory in wich case we
     * will never be able to check the quit status. Therefore after waiting 5 s
     * this will just terminate the thread.
     */
    void end();

    /** load the parameters of the FormatConverter */
    void load();

  private:
    /** the name of the partition tag we connect to*/
    char *_partitionTag;

    /** the client index of the shared memory */
    int _index;

    /** a reference to the format converter functor
     *
     * The converter will convert the incomming data to our CASSEvent
     */
    FormatConverter &_convert;
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
