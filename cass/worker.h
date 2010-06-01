//Copyright (C) 2009,2010 Lutz Foucar

#ifndef __WORKER_H__
#define __WORKER_H__

#include <QtCore/QMutex>
#include <QtCore/QObject>
#include <QtCore/QThread>
#include <QtCore/QWaitCondition>

#include <map>
#include <utility>
#include <string>

#include "cass.h"
#include "ringbuffer.h"
#include "cass_event.h"
#include "postprocessor.h"


namespace cass
{
  //forward declarations
  class Analyzer;

  /** The worker thread.
   *
   * The thread will do the following tasks in a loop:
   * - retrive an event form the buffer,
   * - analyze it using the analyzer,
   * - postanalyze it using the selected postanalyzers,
   * - put the event back to the buffer,
   * @author Lutz Foucar
   */
  class CASSSHARED_EXPORT Worker : public QThread
  {
    Q_OBJECT;
  public:
    /** constructor.
     *
     * @param rb the rinbguffer we get the events from
     * @param outputfilename a name that is passed to special pp. Can be defined
     *                       using -o in the commandline call of cass.
     * @param parent the qt parent of this object
     */
    Worker(cass::RingBuffer<cass::CASSEvent,cass::RingBufferSize>&rb,
           std::string outputfilename,
           QObject *parent=0);

    /** will destory the analyzer and the postprocessors*/
    ~Worker();

    /** start the thread.
     * this will be called by the start member of the thread
     * contains a while loop to do the job.
     */
    void run();

    /** suspend thread.
     * will suspend the thread when it is done working on the event.
     * by calling @see waitUntilSuspended() it makes sure only to return when
     * the thread is really suspended.
     * @returns when thread is suspended.
     */
    void suspend();

    /** waits until thread is suspended.
     * once the pause flag is set, then this function waits
     * until the thread is really suspended
     */
    void waitUntilSuspended();

    /** will continue the thread */
    void resume();

  signals:
    /** emit signal when you are done with one event */
    void processedEvent();

  public slots:
    /** sets the flag to end this thread and waits until the thread finished */
    void end();

    /** tells the thread to load the settings */
    void loadSettings(size_t what);

    /** save the settings */
    void saveSettings();

    /** clear histogram with id */
    void clear(PostProcessors::key_t key) { _postprocessor->clear(key); }

  private:
    cass::RingBuffer<cass::CASSEvent,cass::RingBufferSize>  &_ringbuffer; //!< the ringbuffer
    Analyzer      *_analyzer;         //!< pointer to the analyzer
    PostProcessors*_postprocessor;    //!< pointer to the postprocessors
    bool           _quit;             //!< flag to quit the thread
    QMutex         _pauseMutex;       //!< mutex for suspending the thread
    QWaitCondition _pauseCondition;   //!< condition to suspend the thread
    bool           _pause;            //!< flag to suspend the thread
    bool           _paused;           //!< flag to retrieve the state of the thread
    QWaitCondition _waitUntilpausedCondition; //!< condition to notice once the thread has been paused
  };







  /** Worker Thread Handler.
   * a class that will handle the requested amount of workers threads.
   * The amount of threads can be set in cass.h via parameters
   * @see cass::NbrOfWorkers.
   * @author Lutz Foucar
   */
  class CASSSHARED_EXPORT Workers : public QObject
  {
    Q_OBJECT;

  public:
    /** constructor.
     * will create the requested amount of threads.
     * @param rb the rinbguffer we get the events from
     * @param outputfilename a name that is passed to special pp. Can be defined
     *                       using -o in the commandline call of cass.
     * @param parent the qt parent of this object
     */
    Workers(cass::RingBuffer<cass::CASSEvent,cass::RingBufferSize>&rb,
            std::string outputfilename,
            QObject *parent=0);

    /** deletes all workers*/
    ~Workers();

    //! starts the threads
    void start();

  public slots:
    /** will set the flags to end the threads */
    void end();

    /** will cause the loading of the settings from the cass.ini file */
    void loadSettings(size_t what);

    /** save the settings */
    void saveSettings();

    /** clear histogram with id */
    void clearHistogram(cass::PostProcessors::key_t);

  signals:
    /** this is emmitted once all workers have stoped */
    void finished();

    /** this is emmitted when a worker is finished with an event */
    void processedEvent();

  private:
    std::vector<cass::Worker*> _workers; //!< container of workers
    QMutex                     _mutex;   //!< mutex to make loadSettings reentrant
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
