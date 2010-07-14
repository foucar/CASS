//Copyright (C) 2009,2010 Lutz Foucar

/**
 * @file worker.h file contains declaration of class Worker and Workers
 *
 * @author Lutz Foucar
 */

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
   * - put the event back to the buffer
   *
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
     *
     * this will be called by the start member of the thread
     * contains a while loop to do the job.
     */
    void run();

    /** suspend thread.
     *
     * will suspend the thread when it is done working on the event.
     * by calling @see waitUntilSuspended() it makes sure only to return when
     * the thread is really suspended.
     *
     * @returns when thread is suspended.
     */
    void suspend();

    /** waits until thread is suspended.
     *
     * once the pause flag is set, then this function waits
     * until the thread is really suspended
     */
    void waitUntilSuspended();

    /** will continue the thread */
    void resume();

    /** notice that we are about to quit */
    void aboutToQuit();

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

    /** process command in pp with id */
    void receiveCommand(PostProcessors::key_t key, std::string command)
    {
      _postprocessor->receiveCommand(key, command);
    }

  private:
    /** the ringbuffer */
    cass::RingBuffer<cass::CASSEvent,cass::RingBufferSize>  &_ringbuffer;

    /** pointer to the analyzer */
    Analyzer      *_analyzer;

    /** pointer to the postprocessors */
    PostProcessors*_postprocessor;

    /** flag to quit the thread */
    bool           _quit;

    /** mutex for suspending the thread */
    QMutex         _pauseMutex;

    /** condition to suspend the thread */
    QWaitCondition _pauseCondition;

    /** flag to suspend the thread */
    bool           _pause;

    /** flag to retrieve the state of the thread */
    bool           _paused;

    /** condition to notice once the thread has been paused */
    QWaitCondition _waitUntilpausedCondition;
  };







  /** Worker Thread Handler.
   *
   * a class that will handle the requested amount of workers threads.
   * The amount of threads can be set in cass.h via parameters
   * @see cass::NbrOfWorkers.
   *
   * @author Lutz Foucar
   */
  class CASSSHARED_EXPORT Workers : public QObject
  {
    Q_OBJECT;

  public:
    /** constructor.
     *
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

    /** process command in postprocessor with id */
    void receiveCommand(cass::PostProcessors::key_t, std::string command);

  signals:
    /** this is emmitted once all workers have stoped */
    void finished();

    /** this is emmitted when a worker is finished with an event */
    void processedEvent();

  private:
    /** container of workers */
    std::vector<cass::Worker*> _workers;

    /** mutex to make loadSettings reentrant */
    QMutex _mutex;
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
