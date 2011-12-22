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

#include <map>
#include <utility>
#include <string>
#include <tr1/memory>

#include "pausablethread.h"
#include "cass.h"
#include "ringbuffer.h"
#include "cass_event.h"


namespace cass
{
//forward declarations
class Ratemeter;
class Analyzer;
class PostProcessors;

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
   * @param parent the qt parent of this object
   */
  Worker(RingBuffer<CASSEvent,RingBufferSize>&rb,
         Ratemeter &ratemeter,
         QObject *parent=0);

  /** start the thread.
   *
   * While the thread has not been quitted do
   * retrieve a cassevent from the ringbuffer, but with a timeout. If we got
   * a new event from the ringbuffer put it into the pre analyzer chain and
   * then into the postanalysis chain. Then put it back to the ringbuffer for
   * new refilling and increase the counter of the ratemeter.
   */
  void run();

private:
  /** the ringbuffer */
  RingBuffer<CASSEvent,RingBufferSize>  &_ringbuffer;

  /** the pre analyzer */
  Analyzer &_preanalyze;

  /** the postprocessors */
  PostProcessors &_postprocess;

  /** the ratemeter to measure the analysis rate */
  Ratemeter &_ratemeter;
};







/** Worker Thread Handler.
 *
 * a class that will handle the requested amount of workers threads.
 * The amount of threads can be set in cass.h via parameters
 * @see NbrOfWorkers.
 *
 * @todo find a way to close the program without using signal slot mechnism
 * @note the problem is that the input should also have control to quit the
 *       program. Maybe one can instead of starting the global eventloop just
 *       wait until the input has finished?
 *
 * @author Lutz Foucar
 */
class  Workers
{
public:
  /** a shared pointer of this class */
  typedef std::tr1::shared_ptr<Workers> shared_pointer;

  /** create and return an instance of this singleton
   *
   * when the instance has not yet been created, call the constructor otherwise
   * just return the instance.
   *
   * @param rb the rinbguffer we get the events from
   * @param ratemeter the ratemeter object to measure the rate
   * @param parent the qt parent of this object
   */
  static shared_pointer instance(RingBuffer<CASSEvent,RingBufferSize> &rb,
                                 Ratemeter &ratemeter,
                                 QObject *parent=0);

  /** return a reference to the instance itselve if it exists */
  static shared_pointer::element_type& reference();

  /** starts the threads
   *
   * function is not reentrant. One needs to use the _lock mutex to prevent
   * simultanious calling of this function.
   */
  void start();

  /** pause the threads.
   *
   * Blocks until all threads are paused
   *
   * function is not reentrant. One needs to use the _lock mutex to prevent
   * simultanious calling of this function.
   */
  void pause();

  /** resumes the threads
   *
   * function is not reentrant. One needs to use the _lock mutex to prevent
   * simultanious calling of this function.
   */
  void resume();

  /** will set the flags to end the threads
   *
   * Will call the end()
   * member of all workers. Then waits until all workers are finished. After this
   * the aboutToQuit member of the postprocessor and the Analyzer are notified.
   *
   * function is not reentrant. One needs to use the _lock mutex to prevent
   * simultanious calling of this function.
   */
  void end();

  /** a lock to be used by functions that are using this worker */
  QMutex _lock;

private:
  /** constructor.
   *
   * will create the requested amount of threads. and calls the load settings
   * member of one of them.
   *
   * @param rb the rinbguffer we get the events from
   * @param ratemeter the ratemeter object to measure the rate
   * @param parent the qt parent of this object
   */
  Workers(RingBuffer<CASSEvent,RingBufferSize> &rb,
          Ratemeter &ratemeter,
          QObject *parent=0);

  /** container of workers */
  std::vector<Worker::shared_pointer> _workers;

  /** the instance of this class */
  static shared_pointer _instance;

  /** mutex to protect the creation of the signelton */
  static QMutex _mutex;
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
