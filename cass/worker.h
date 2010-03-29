//Copyright (C) 2009,2010 lmf

#ifndef __WORKER_H__
#define __WORKER_H__

#include <QtCore/QObject>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>

#include <map>
#include <utility>

#include "cass.h"
#include "ringbuffer.h"
#include "cass_event.h"
#include "postprocessing/postprocessor.h"


namespace cass
{
  class Analyzer;

  //class that will do all the work
  //  retrive an event form the buffer
  //  analyze it using the analyzer
  //  postanalyze it using the selected postanalyzers
  //  put the event back to the buffer
  class CASSSHARED_EXPORT Worker : public QThread
  {
    Q_OBJECT;
  public:
    Worker(cass::RingBuffer<cass::CASSEvent,cass::RingBufferSize>&, QObject *parent=0);
    ~Worker();

    //this will be called by the start member of the thread//
    //contains a while loop to do the job//
    void run();
    //will set the pause flag of the thread//
    void suspend();
    //will continue the thread//
    void resume();
    //once the pause flag is set, then this function waits//
    //until the thread is really suspended//
    void waitUntilSuspended();

    //retrieve the histogram container from the postprocessor//
    const PostProcessors::histograms_t& histograms()const;


  signals:
    //emit signal when you are done with one event//
    void processedEvent();

  public slots:
    //sets the flag to end this thread//
    void end();
    //tells the thread to load the settings//
    void loadSettings(size_t what);
    //save the settings//
    void saveSettings();

  private:
    cass::RingBuffer<cass::CASSEvent,cass::RingBufferSize>  &_ringbuffer; //the ringbuffer
    Analyzer      *_analyzer;         //pointer to the analyzer
    PostProcessors*_postprocessor;    //pointer to the postprocessors
    bool           _quit;             //flag to quit the thread
    QMutex         _pauseMutex;       //mutex for suspending the thread
    QWaitCondition _pauseCondition;   //condition to suspend the thread
    bool           _pause;            //flag to suspend the thread
    bool           _paused;           //flag to retrieve the state of the thread
    QWaitCondition _waitUntilpausedCondition; //condition to notice once the thread has been paused

  };

  //a class that will handle the requested amount of workers
  class CASSSHARED_EXPORT Workers : public QObject
  {
    Q_OBJECT;
  public:
    Workers(cass::RingBuffer<cass::CASSEvent,cass::RingBufferSize>&, QObject *parent=0);
    ~Workers();

    //starts the threads//
    void start();
    //use this to retrive the histogram container in postprocessor//
    const cass::PostProcessors::histograms_t& histograms()const;

  public slots:
    //will set the flags to end the threads//
    void end();
    //will cause the loading of the settings from the cass.ini file//
    void loadSettings(size_t what);

  signals:
    //this is emmitted once all workers have stoped//
    void finished();
   //this is emmitted when a worker is finished with an event//
    void processedEvent();

  private:
    std::vector<cass::Worker*> _workers; //list of workers
  };

}

#endif



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
