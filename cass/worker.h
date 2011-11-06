//Copyright (C) 2009,2010 Lutz Foucar

/**
 * @file worker.h file contains declaration of class Worker and Workers
 *
 * @todo instead of using a singnal to provide a costum command string to the
 *       postprocessors we need to do it like the histogram getter.
 *
 * @author Lutz Foucar
 */

#ifndef __WORKER_H__
#define __WORKER_H__

#include <QtCore/QMutex>
#include <QtCore/QObject>

#include <map>
#include <utility>
#include <string>
#include <tr1/memory>

#include "pausablethread.h"
#include "cass.h"
#include "ringbuffer.h"
#include "cass_event.h"
#include "postprocessor.h"


namespace cass
{
  //forward declarations
  class Analyzer;
  class Ratemeter;

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
  class Worker : public lmf::PausableThread
  {
  public:

    /** a shared pointer of this */
    typedef std::tr1::shared_ptr<Worker> shared_pointer;

    /** constructor.
     *
     * @param rb the rinbguffer we get the events from
     * @param ratemeter the ratemeter object to measure the rate
     * @param outputfilename a name that is passed to special pp. Can be defined
     *                       using -o in the commandline call of cass.
     * @param parent the qt parent of this object
     */
    Worker(RingBuffer<CASSEvent,RingBufferSize>&rb,
           Ratemeter &ratemeter,
           std::string outputfilename,
           QObject *parent=0);

    /** will destory the analyzer and the postprocessors*/
    ~Worker();

    /** start the thread.
     *
     * retrieve a cassevent from the ringbuffer, but with a timeout. If we got
     * a new event from the ringbuffer put it into the pre analyzer chain and
     * then into the postanalysis chain. Then put it back to the ringbuffer for
     * new refilling and increase the counter of the ratemeter.
     */
    void run();

    /** notice the preanalyzer and the postprocessor that we are about to quit */
    void aboutToQuit();

    /** tells the thread to load the settings */
    void loadSettings(size_t what);

    /** save the settings */
    void saveSettings();

    /** clear histogram with id */
    void clear(const PostProcessors::key_t &key) { _postprocessor->clear(key); }

    /** process command in pp with id */
    void receiveCommand(const PostProcessors::key_t &key, const std::string &command)
    {
      _postprocessor->receiveCommand(key, command);
    }

  private:
    /** the ringbuffer */
    RingBuffer<CASSEvent,RingBufferSize>  &_ringbuffer;

    /** pointer to the analyzer */
    Analyzer *_analyzer;

    /** pointer to the postprocessors */
    PostProcessors *_postprocessor;

    /** the ratemeter to measure the analysis rate */
    Ratemeter &_ratemeter;
  };







  /** Worker Thread Handler.
   *
   * a class that will handle the requested amount of workers threads.
   * The amount of threads can be set in cass.h via parameters
   * @see NbrOfWorkers.
   *
   * @author Lutz Foucar
   */
  class  Workers : public QObject
  {
    Q_OBJECT;

  public:
    /** constructor.
     *
     * will create the requested amount of threads. and calls the load settings
     * member of one of them.
     *
     * @param rb the rinbguffer we get the events from
     * @param ratemeter the ratemeter object to measure the rate
     * @param outputfilename a name that is passed to special pp. Can be defined
     *                       using -o in the commandline call of cass.
     * @param parent the qt parent of this object
     */
    Workers(RingBuffer<CASSEvent,RingBufferSize> &rb,
            Ratemeter &ratemeter,
            std::string outputfilename,
            QObject *parent=0);

    /** deletes all workers*/
    ~Workers();

    /** starts the threads */
    void start();

  public slots:
    /** will set the flags to end the threads
     *
     * this function is locked so that it can be reentrant. Will call the end()
     * member of all workers. Then resumes all workers in case they are still
     * running. Then waits until all workers are finished executing then send.
     * the aboutToQuit to only one worker. Once this is done the finished signal
     * is emitted.
     */
    void end();

    /** will cause the loading of the settings from the cass.ini file
     *
     * pause all threads, wait until they are paused and then call the load
     * settings of only one worker. This will work, because the analyser and
     * the postprocessors are singletons. Then resume all threads.
     * This function is protected by a mutex to make it reentrant.
     *
     * @param what unused parameter
     */
    void loadSettings(size_t what);

    /** save the settings
     *
     * pause all threads, wait until they are paused and then call the save
     * settings of only one worker. This will work, because the analyser and
     * the postprocessors are singletons. Then resume all threads.
     */
    void saveSettings();

    /** clear histogram with id
     *
     * pause all threads, wait until they are paused and then call the clear
     * histogram of postpressor with key of only one worker. This will work,
     * because the analyser and the postprocessors are singletons. Then resume
     * all threads.
     *
     * @param key the postprocessor whos histograms should be cleared
     */
    void clearHistogram(PostProcessors::key_t key);

    /** process command in postprocessor with id
     *
     * pause all threads, wait until they are paused and then call the call the
     * command of postpressor with key of only one worker. This will work,
     * because the analyser and the postprocessors are singletons. Then resume
     * all threads.
     *
     * @param key the postprocessor who will receive the command
     * @param comand the command string
     */
    void receiveCommand(PostProcessors::key_t key, std::string command);

  signals:
    /** this is emmitted once all workers have stoped */
    void finished();

    /** this is emmitted when a worker is finished with an event */
    void processedEvent();

  private:
    /** container of workers */
    std::vector<Worker::shared_pointer> _workers;

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
