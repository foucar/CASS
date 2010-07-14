// Copyright (C) 2009,2010 Lutz Foucar

/**
 * @file sharedmemory_input.h file contains declaration of class that interfaces
 *                            the LCLS shared memory
 *
 * @author Lutz Foucar
 */

#ifndef __SHAREDMEMORYINPUT_H__
#define __SHAREDMEMORYINPUT_H__

#include <QtCore/QMutex>
#include <QtCore/QObject>
#include <QtCore/QThread>
#include <QtCore/QWaitCondition>

#include "cass.h"
#include "pdsdata/app/XtcMonitorClient.hh"
#include "ringbuffer.h"
#include "cass_event.h"

namespace cass
{
  //forward declaration//
  class FormatConverter;

  /** Shared Memory Input.
   * This class is a thread that connects to the sahred memory of LCLS. The
   * baseclass does all the connection and once there is new data available
   * it calles processDatagram, where we can add code that we want to use.
   * The first datagram that will be send to this class is a datagram
   * containing the last known configure transition. This is to make sure that
   * when starting the program we will always get the latest state of the DAQ.
   * @author Lutz Foucar
   */
  class CASSSHARED_EXPORT SharedMemoryInput
    : public QThread, Pds::XtcMonitorClient
  {
    Q_OBJECT;
  public:
    /** constructor.
     * creates the thread. The base class will create the interface to the shared
     * memory.
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

    /** starts the thread.
     * Starts the thread and the loop in the shared memory client we inerhited
     * from. The loop will be notified when there are new events available in
     * the shared memory.
     */
    void run();

    /** suspends the thread.
     * Suspends the thread after it has executed the event we are working on
     * right now. Will return only when the thread has really suspenden by calling
     * @see waitUntilSuspended() internally.
     */
    void suspend();

    /** resumes the thread, when it was suspended. Otherwise it just retruns*/
    void resume();

    /** overwrite the base class function with our code.
     * This is called once the eventqueue has new data available.
     * @return the errorcode, when != 0, then the baseclasses will exit its loop
     * @param[in] dg The datagram we can work on
     */
    int  processDgram(Pds::Dgram*dg);

  signals:
    /** signal to indicate that we are done processing an event.
     * this is used for by the ratemeter to evaluate how fast we get events.
     */
    void newEventAdded();

  public slots:
    /** call this slot, when you want to quit the thread.
     * Makes the thread execute the last event and then quit normally.
     */
    void end();

    /** load the parameters used for this thread*/
    void loadSettings(size_t what);

  protected:
    /** function that will wait until we really suspended.
     * will be called by suspend, so that it returns only when thread has really
     * suspended.
     */
    void waitUntilSuspended();

  private:
    /** the Ringbuffer we take events out and fill them with the incomming data*/
    cass::RingBuffer<cass::CASSEvent,cass::RingBufferSize>  &_ringbuffer;

    /** the name of the partition tag we connect to*/
    char *_partitionTag;

    /** the client index of the shared memory */
    int _index;

    /** a flag to tell the thread wther the user wants to quit, is set by @see end()*/
    bool _quit;

    /** a pointer to the format converter.
     * The converter will convert the incomming data to our CASSEvent
     */
    FormatConverter *_converter;

    /** a mutex for suspending the thread*/
    QMutex _pauseMutex;

    /** a condition that we will wait on until we are not suspended anymore*/
    QWaitCondition _pauseCondition;

    /** flag telling whether we shouodl suspend ourselves*/
    bool _pause;

    /** flag telling whether we are already suspended*/
    bool _paused;

    /** condition that will wait until the thread is rally suspended*/
    QWaitCondition _waitUntilpausedCondition;
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
