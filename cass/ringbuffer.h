//Copyright (C) 2008-2010 Lutz Foucar

/**
 * @file ringbuffer.h file contains the ringbuffer class
 *
 * @author Lutz Foucar
 */

#ifndef _RINGBUFFER_H
#define _RINGBUFFER_H

#include <QMutex>
#include <QWaitCondition>
#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>

#include "cass_exceptions.h"
#include "cass.h"

namespace cass
{
/** A Ringbuffer, handles communication between Input and Worker Threads.
 *
 * The ringbuffer handles the main communication between the single producers
 * (input derived from InputBase) and the multiple consumers (worker).
 *
 * The ringbuffer can be compiled or non blocking by defining RINGBUFFER_BLOCKING
 * or not, respectively.
 *
 * It is designed in such a way, that in the nonblocking case, the consumers
 * do not block the producer from putting new entries into the ringbuffer.
 * If the producers velocity in filling the buffer varies, then this buffer
 * will make sure, that it can be faster than the consumers. When the producer
 * fills elements slower than the consumers consume them, then the consumers
 * can consume the elements that have already been put into the buffer.
 * They will do this by going backwards through the buffer from the last
 * element that the producer has put into the buffer.
 * The ringbuffers' elements will be created on the Heap.
 *
 * @tparam T Element typ
 *
 * @todo find out how one can use std::find to find the right element
 * @todo maybe create a ReadWriteLock for each element to get rid of the mutexes
 * @todo separeate declaration and definition to make class more readable
 *
 * @author Lutz Foucar
 */
template <typename T>
class RingBuffer
{
private:
  /** an element of the ringbuffer.
   *
   * contains the status of the element and a pointer
   * to the actual element.
   *
   * @author Lutz Foucar
   */
  class Element
  {
  public:
    /** constructor.
     *
     * will initalize the status flags correcty.
     */
    Element()
      : element(0),
        processed(true),
        filled(false),
        inUse(false)
    {}

    /** the pointer to the element */
    T *element;

    /** status whether the element has been worked on*/
    bool processed;

    /** status whether the element has been filled*/
    bool filled;

    /** status whether the element is workend on right now*/
    bool inUse;
  };

public:
  /** type of the container of all elements*/
  typedef std::vector<Element> buffer_t;

  /** type of the interator over the elements of the container*/
  typedef typename buffer_t::iterator iter_type;

  /** constructor.
   *
   * This will create the buffer, fill it with the requested amount of elements,
   * and initialize the iterators.
   *
   * @param size The size of the ringbuffer
   */
  RingBuffer(size_t size)
    : _buffer(size),
      _nextToProcess(_buffer.begin()),
      _nextToFill(_buffer.begin())
  {
    for (size_t i=0; i<_buffer.size(); ++i)
      _buffer[i].element = new T();
  }

  /** destructor.
   *
   * deletes all elements of the buffer
   */
  ~RingBuffer()
  {
    for (size_t i=0; i<_buffer.size(); ++i)
      delete _buffer[i].element;
  }

private:
  /** advances the _nextToProcess iterator to the next processable element.
   *
   * will go through the ringbuffer backwards starting at the
   * position where we added the filled element. It will check
   * whether the current element is not currently processed and has already
   * been filled.
   *
   * @return true when a processable element has been found
   */
  bool findNextProcessable()
  {
    /** we should end where the next fillable element is, because elements
     *  before the next fillable are already processed or are in processing
     */
    iter_type lastEmpty(_nextToFill);

    /** search until the current element is not currently in use or
     *  not filled yet
     */
    while (_nextToProcess->inUse || !_nextToProcess->filled)
    {
      /** if we are at the position where the next fillable is,
       *  then there is nothing to work on anymore
       */
      if (_nextToProcess == lastEmpty)
        return false;

      /** we go backwards through the buffer to have always the latest
       *  element to process. If we come to the beginning of the vector, then
       *  we have to jump to the back
       */
      if (_nextToProcess == _buffer.begin())
        _nextToProcess = _buffer.end()-1;
      else
        --_nextToProcess;
    }
    return true;
  }

  /** advances the _nextToFill itertor to the next fillable element.
   *
   * this function is used when the behaviour of the ringbuffer is blockable
   * it will iterate through the buffer and checks the elements for the
   * status in progress (inBearbeitung) and processed (bearbeitet)
   * it will only return true when its not in progress and already processed.
   *
   * this function is used when the behaviour of the ringbuffer is nonblockable
   * it will iterate through the buffer and checks the elements for
   * only the status in progress (inBearbeitung).
   * it will only return true when its not in progress.
   *
   * @return true when a fillable element has been found
   */
  bool findNextFillable()
  {
    /** the start point is one before the current point where we started */
    iter_type start((_nextToFill == _buffer.begin()) ? _buffer.end()-1 : _nextToFill-1);

#ifdef RINGBUFFER_BLOCKING
    /** search until the current element is not currently in use or
     *  has been processed
     */
    while (_nextToFill->inUse || !_nextToFill->processed) {
#else
    /** search until the current element is not currently in use */
    while (_nextToFill->inUse) {
#endif
      /** if we end up where we started, then the elements are not yet
       *  processed or still in progress, so retrun that we have not found
       *  anything yet.
       */
      if (_nextToFill == start)
        return false;

      /** wrap to beginning if we hit the end  */
      if (_nextToFill == _buffer.end()-1)
        _nextToFill = _buffer.begin();
      else
        ++_nextToFill;
    }
    return true;
  }

public:
  /** return the next filled but non processed element.
   *
   * This function will return the next filled element, which will
   * either be the one just filled by the shared memory input or
   * one or more before, depending on how fast elements are retrieved
   * before they are filled again.
   * When there are no Elements that we can work on, this function will wait
   * until there is a new element that we can process.
   *
   * @note this can be the reason why only one of the threads is working at
   *       a time.
   *
   * @return iterator to the element of the ringbuffer that is processable
   * @param[in] timeout Time that we will wait that a new element is beeing
   *                    put into the buffer. It is defaulted to ULONG_MAX
   */
  iter_type nextToProcess(unsigned long timeout=ULONG_MAX)
  {
    QMutexLocker lock(&_mutex);

    /** if nothing was found, wait until we get noticed that
     *  a new element was added to the buffer and return 0 if
     *  waited long enough
     */
    while (!findNextProcessable())
      if(!_processcondition.wait(lock.mutex(),timeout))
       throw ProcessableTimedout(std::string("RingBuffer::nextToProcess(): ") +
                                 "couldn't retrieve a new element to process " +
                                 "within '" + toString(timeout) + "' ms");

    /** set the flags of that element */
    _nextToProcess->inUse = true;
    iter_type iter(_nextToProcess);

    /** The next element that will be asked for is the previous
     *  one. Unless a new element to be processed has been added to the buffer
     *  therefore let the iterator point to the previous element
     */
    if (_nextToProcess == _buffer.begin())
      _nextToProcess = _buffer.end()-1;
    else
      --_nextToProcess;

    return iter;
  }

  /** putting the processed element back to the buffer.
   *
   * This function will put the element that we just processed back to the buffer.
   * It will will search the buffer for the element and then set the
   * flags of that element according to its current state.
   *
   * @return void
   * @return[in] iterator to the element in the buffer that is done processing
   */
  void doneProcessing(iter_type iter)
  {
    QMutexLocker lock(&_mutex);

    /** set flags */
    iter->inUse     = false;
    iter->processed = true;
    iter->filled    = false;

    /** notify the waiting condition that something new is in the buffer
     *
     * @note we need to unlock the lock before
     */
    lock.unlock();
    _fillcondition.wakeOne();
  }

  /** retrieve the "to be filled" element.
   *
   * This function will retrieve the next element that we can fill.
   * Depending on the behaviour of the ringbuffer, we check whether
   * it has been processed or not. When the behaviour is blocking then
   * we only retrieve elements that are processed, if not then we just
   * return the next element that is not in process.
   * In the blocking case this function will only return when a processed
   * event was put into the buffer.
   *
   * @return iterator to the element that can be filled
   * @param[in] timeout Time that we will wait that a new fillable element is
   *                    beeing made available. It is defaulted to ULONG_MAX
   */
  iter_type nextToFill(unsigned long timeout=ULONG_MAX)
  {
    QMutexLocker lock(&_mutex);

    /** find an fillable element of the buffer, if there is no fillable, wait
     *  until a new element has been processed
     *
     *  @note this is blocking until an element has been processed
     */
    while(!findNextFillable())
    {
      if (!_fillcondition.wait(lock.mutex(),timeout))
       throw ProcessableTimedout(std::string("RingBuffer::nextToFill(): ") +
                                 "couldn't retrieve a new element to fill " +
                                 "within '" + toString(timeout) + "' ms");
    }

    /** Set the flags accordingly */
    _nextToFill->inUse = true;
    iter_type iter(_nextToFill);

    /** the next element should be one that we are going
     *  to fill next. Therefore decrease the iterator by one
     */
    if (_nextToFill == _buffer.end()-1)
      _nextToFill = _buffer.begin();
    else
      ++_nextToFill;

    return iter;
  }

  /** putting the filled element back to the buffer.
   *
   * This function will put the element that we just filled back to the buffer.
   * It will will search the buffer for the element and then set the
   * flags of that element according to its current state and depending on the
   * fillstatus. Using the fillstatus we can say that this element should be processed
   * or not.
   *
   * @return void
   * @return[in] element reference to the pointer of the element
   * @return[in] fillstatus True when the element should be processed,
   *             false if not.
   */
  void doneFilling(iter_type iter, bool fillstatus=true)
  {
    QMutexLocker lock(&_mutex);

    /** set the status properties according to the fillstatus */
    iter->inUse = false;
    iter->processed = !fillstatus;
    iter->filled    = fillstatus;

    /** set the next to process iterator to this element, since its
     *  the next element that we should process. This should shorten
     *  the time we are searching for the next processable element
     */
    _nextToProcess = iter;

    /** notify the waiting condition that something new is in the buffer
     *
     *  @note we need to unlock the lock before
     */
    lock.unlock();
    _processcondition.wakeOne();
  }

  /** count how many elements of the buffer are not processed
   *
   * The number of elements in the buffer that are not processed tell how many
   * are still beeing processed.
   *
   * @return number of elements that are in processing state
   */
  int countProcessing()
  {
    int count(0);
    iter_type it(_buffer.begin());
    iter_type end(_buffer.end());
    for (; it != end; ++it)
      if (!it->processed || it->inUse)
        ++count;
    return count;
  }

  /** wait until no element that needs processing is on the list
   *
   * this function is blocking until all elements in the buffer are in the
   * processed state.
   */
  void waitUntilEmpty()
  {
    QMutexLocker lock(&_mutex);
    while (countProcessing() != 0)
      _fillcondition.wait(lock.mutex(),100);
  }

private:
  /** mutex to protect the iterators and the buffer elements */
  QMutex _mutex;

  /** sync the filling part */
  QWaitCondition _fillcondition;

  /** sync the processing part */
  QWaitCondition _processcondition;

  /** the ringbuffer container */
  buffer_t _buffer;

  /** iterator to the next processable element */
  iter_type _nextToProcess;

  /** iterator to the next fillable element */
  iter_type _nextToFill;
};
}
#endif
