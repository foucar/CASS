// Copyright (C) 2009,2010 Lutz Foucar

#ifndef __SHAREDMEMORYINPUT_H__
#define __SHAREDMEMORYINPUT_H__

#include <QtCore/QObject>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>


#include "cass.h"
#include "pdsdata/app/XtcMonitorClient.hh"
#include "ringbuffer.h"
#include "cass_event.h"

namespace cass
{
  //forward declaration//
  class FormatConverter;

  /*! @brief Shared Memory Input

    This class is a thread that connects to the sahred memory of LCLS. The baseclass
    does all the connection and once there is new data available it calles processDatagram,
    where we can add code tha we want to use
    @author Lutz Foucar*/
  class CASSSHARED_EXPORT SharedMemoryInput : public QThread, Pds::XtcMonitorClient
  {
    Q_OBJECT;
  public:
    /** constructor creates the thread
      @param PartitionTag the name of the partition tag we want to connect to
      @param buffer the ringbuffer, that we take events out and fill it with the incomming information*/
    SharedMemoryInput(char * PartitionTag, cass::RingBuffer<cass::CASSEvent,4>& buffer, QObject *parent=0);
    /** deletes the thread*/
    ~SharedMemoryInput();
    /** starts the thread*/
    void run();
    /** suspends the thread after it has executed the event we are working on right now
    @todo maybe let this function only return when the thread is really suspended*/
    void suspend();
    /** resumes the thread, when it was suspended. Otherwise it just retruns*/
    void resume();
    /** function that will wait until we really suspended. Should be called after calling suspend
      to make sure we are done when we call other functions*/
    void waitUntilSuspended();
    /** overwrite the base class function with our code*/
    int  processDgram(Pds::Dgram*);

  signals:
    /** signal to indicate that we are done processing an event, used for the ratemeters */
    void newEventAdded();

  public slots:
    /** call this slot, when you want to quit the thread. Makes the thread execute the last event
      and then quit normally*/
    void end();
    /** load the parameters used for this  thread*/
    void loadSettings(size_t what);

  private:
    /** the Ringbuffer we take events out and fill them with the incomming data*/
    cass::RingBuffer<cass::CASSEvent,cass::RingBufferSize>  &_ringbuffer;
    /** the name of the partition tag we connect to*/
    char              *_partitionTag;
    /** a flag to tell the thread wther the user wants to quit, is set by @see end()*/
    bool               _quit;
    /** a pointer to the format convert, that will convert the incomming data to our CASSEvent*/
    FormatConverter   *_converter;
    /** a mutex for suspending the thread*/
    QMutex             _pauseMutex;
    /** a condition that we will wait on until we are not suspended anymore*/
    QWaitCondition     _pauseCondition;
    /** flag telling whether we shouodl suspend ourselves*/
    bool               _pause;
    /** flag telling whether we are already suspended*/
    bool               _paused;
    /** condition that will wait until the thread is rally suspended*/
    QWaitCondition     _waitUntilpausedCondition;
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
