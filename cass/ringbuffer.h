//Copyright (C) 2008-2010 Lutz Foucar

#ifndef _RINGBUFFER_H
#define _RINGBUFFER_H

#include <QMutex>
#include <QWaitCondition>
#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>

namespace cass
{
  /** A Ringbuffer, handles communication between Input and Worker Threads.
   *
   * The ringbuffer handles the main communication between the single producer
   * (shared memory input) and the multiple consumers (worker).
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
   * @tparam cap Capacity of the ringbuffer
   * @author Lutz Foucar
   * @todo find out how one can use std::find to find the right element
   * @todo maybe create a ReadWriteLock for each element to get rid of the mutexes
   * @todo separeate declaration and definition to make class more readable
   */
  template <typename T, size_t cap>
  class RingBuffer
  {
  public:

    /** type for references to the elements of the container*/
    typedef T*& reference;

    /** type for const references to the elements of the container*/
    typedef const T*& const_reference;

    /** type for pointers to the elements of the container*/
    typedef T* value_t;


  private:

    /** an element of the ringbuffer.
     * contains the status of the element and a pointer
     * to the actual element.
     * @author Lutz Foucar
     * @todo Internationalize variable names
     */
    class Element
    {
    public:
      /** constructor.
       * will initalize the status flags correcty.
       */
      Element()
        :element(),
        bearbeitet(true),
        gefuellt(false),
        inBearbeitung(false)
      {}
      /** Element equality.
       *
       * Two elements are equal when their addresses are equal.
       * @param Element to compare me to.
       */
      bool operator==(const_reference rhs) { return (element == rhs); }
      /** the pointer to the element*/
      T   *element;
      /** status whether the element has been worked on*/
      bool bearbeitet;
      /** status whether the element has been filled*/
      bool gefuellt;
      /** status whether the element is workend on right now*/
      bool inBearbeitung;
    };


  public:

    /** type of the container of all elements*/
    typedef std::vector<Element> buffer_t;

    /** type of the interator over the elements of the container*/
    typedef typename buffer_t::iterator iterator_t;

    /** constructor.
     * This will create the buffer, fill it with the requested amount of elements,
     * and initialize the iterators.
     */
    RingBuffer()
      : _buffer(cap,Element()),
        _nextToProcess(_buffer.begin()),
        _nextToView(_buffer.begin()),
        _nextToFill(_buffer.begin())
    {
      //create the elements in the ringbuffer//
      for (size_t i=0; i<_buffer.size(); ++i)
        _buffer[i].element = new T();
    }

    /** destructor.
     * deletes all elements of the buffer
     */
    ~RingBuffer()
    {
      //delete all elements in the ringbuffer//
      for (size_t i=0; i<_buffer.size(); ++i)
        delete _buffer[i].element;
    }

  private:

    /** finds the processable element.
     * will go through the ringbuffer backwards starting at the
     * position where we added the filled element. It will check
     * whether the current element is not currently processed and has already
     * been filled.
     * @return true when a processable element has been found
     */
    bool findNextProcessable()
    {
      //remember where we should end our loop//
      //we should end where the next fillable element is, because elements//
      //before the next fillable are already processed or are in processing/
      iterator_t letztesFreiesElement(_nextToFill);
      //if the current element is currently processed or not filled yet//
      //try the next one//
      while (_nextToProcess->inBearbeitung || !_nextToProcess->gefuellt)
      {
        //if we are at the position where the next fillable is,//
        //then there is nothing to work on anymore//
        if (_nextToProcess == letztesFreiesElement)
          return false;

        //we go backwards through the buffer to have always the latestest element
        //to process. If we come to the beginning of the vector, then we
        //we have to jump to the back//
        if (_nextToProcess == _buffer.begin())
          _nextToProcess = _buffer.end()-1;
        else
          --_nextToProcess;
      }
      //we have found an element that we can work on
      return true;
    }

    /** finds a viewable element for serialization
     * will go through the buffer and find an element that
     * can be retrieved by the client
     * @note check whether this will work. It has just been implemented
     *       to have the event_getter workable.
     * @return true when a viewable element has been found
     */
    bool findNextViewable()
    {
      //remember where we should end our loop//
      //we should end where the next fillable element is, because elements//
      //before the next fillable are already processed or are in processing/
      iterator_t letztesElement = _nextToView;
      //if the current element is currently processed or not filled yet//
      //try the next one//
      while (_nextToView->inBearbeitung || !_nextToView->bearbeitet)
      {

        //if we are at the position where the next fillable is,//
        //then there is nothing to retrieve on anymore//
        if (_nextToView == letztesElement)
          return false;

        //we go backwards through the buffer to have always the latestest element
        //to process. If we come to the beginning of the vector, then we
        //we have to jump to the back//
        if (_nextToView == _buffer.begin())
          _nextToView = _buffer.end()-1;
        else
          --_nextToView;
      }
      //we have found an element that we can serialize
      return true;
    }

    /** finds the next fillable element.
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
     * @return true when a fillable element has been found
     */
    bool findNextFillable()
    {
      //remember where you are starting//
      iterator_t start((_nextToFill == _buffer.begin()) ? _buffer.end()-1 : _nextToFill-1);
      //if the current element is currently processed or has not been processed//
      //try the next element//
#ifdef RINGBUFFER_BLOCKING
      while (_nextToFill->inBearbeitung || !_nextToFill->bearbeitet) {
#else
      while (_nextToFill->inBearbeitung) {
#endif
        //if we end up where we started, then the elements are not yet processed//
        //or still in progress, so retrun that we have not found anything yet.//
        if (_nextToFill == start)
          return false;

        //if we hit the end of the vector, then we will jump to the
        //beginning of the vector//
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
     * @return void
     * @param[out] element reference to a pointer to an element,
     *             we copy the to be processed element pointer to the reference.
     * @param[in] timeout Time that we will wait that a new element is beeing put into
     *            the buffer. It is defaulted to ULONG_MAX
     */
    void nextToProcess(reference element, unsigned long timeout=ULONG_MAX)
    {
      //create a lock//
      QMutexLocker lock(&_mutex);
      while (!findNextProcessable())
      {
        //if nothing was found, wait unitil we get noticed that
        //a new element was added to the buffer//
        if(!_processcondition.wait(lock.mutex(),timeout))
        {
          element = 0;
          return;
        }
      }
      //set the property flags of that element and assign the pointer//
      _nextToProcess->inBearbeitung = true;
      element = _nextToProcess->element;
      //we expect that the next element that will be asked for is the previous
      //one. Unless a new element to be processed has been added to the buffer//
      //therefore let the iterator point to the previous element//
      if (_nextToProcess == _buffer.begin())
        _nextToProcess = _buffer.end()-1;
      else
      --_nextToProcess;
    }

    /** putting the processed element back to the buffer.
     *
     * This function will put the element that we just processed back to the buffer.
     * It will will search the buffer for the element and then set the
     * flags of that element according to its current state.
     *
     * @return void
     * @return[in] element reference to the pointer of the element
     */
    void doneProcessing(reference element)
    {
      //create a lock//
      QMutexLocker lock(&_mutex);
      //find the iterator that points to the element that the user wants to submit//
//#warning Why not use the find?
//      iterator_t iElement =
//          std::find(_buffer.begin(),_buffer.end(),element);

      iterator_t iElement = _buffer.begin();
      for ( ; iElement != _buffer.end() ; ++iElement)
        if (iElement->element == element)
          break;
//      *iElement == element;
      //set its flags according to its state//
      iElement->inBearbeitung = false;
      iElement->bearbeitet    = true;
      iElement->gefuellt      = false;
      //notify the waiting condition that something new is in the buffer//
      //we need to unlock the lock before//
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
     * @return void
     * @param[out] element A reference to the pointer of the Element.
     */
    void nextToFill(reference element)
    {
      //create lock//
      QMutexLocker lock(&_mutex);
      while(!findNextFillable()) {
        _fillcondition.wait(lock.mutex());
      }
      //we found an element. Set the flags accordingly//
      _nextToFill->inBearbeitung = true;
      element = _nextToFill->element;
      //we expect that the next element is the one that we are going//
      //to fill next. Therefore advance the iterator by one//
      if (_nextToFill == _buffer.end()-1)
        _nextToFill = _buffer.begin();
      else
        ++_nextToFill;
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
    void doneFilling(reference element, bool fillstatus=true)
    {
      //search for the element the user wants to put back.//
      //Retrieve the iterator for the element//
      iterator_t iElement = _buffer.begin();
      for ( ; iElement != _buffer.end() ; ++iElement)
        if (iElement->element == element)
          break;
      //create a lock//
      QMutexLocker lock(&_mutex);
      //now set the status properties according to the fillstatus//
      iElement->inBearbeitung = false;
      iElement->bearbeitet    = !fillstatus;
      iElement->gefuellt      = fillstatus;
      //set the next to process iterator to this element, since its
      //the next element that we should process. This should shorten
      //the time we are searching for the next processable element
      _nextToProcess = iElement;
      //notify the waiting condition that something new is in the buffer//
      //we need to unlock the lock before//
      lock.unlock();
      _processcondition.wakeOne();
    }

    /** retrieve the "to be viewed" element.
     * This function will retrieve the next element that we can serialize.
     * @return void
     * @param[out] element reference to a pointer to an element,
     *             we copy the to be serialized element pointer to the reference.
     * @param[in] timeout Time that we will wait that a new viewableelement is beeing
     *            put into the buffer. It is defaulted to ULONG_MAX
     */
   void nextToView(reference element, int timeout=ULONG_MAX)
   {
      //create a lock//
      QMutexLocker lock(&_mutex);
      while (!findNextViewable())
      {
        //if we did not find a serializable element, wait until a processed event//
        //is put into the buffer//
        if(!_viewcondition.wait(lock.mutex(),timeout))
        {
          element = 0;
          return;
        }
      }
      //assign the properties and the pointer//
      _nextToView->inBearbeitung = true;
      element = _nextToView->element;
      //we expect that the next element that the user wants to serialize is the//
      //previous one, so set the iterator to the previous one.//
      if (_nextToView == _buffer.begin())
        _nextToView = _buffer.end()-1;
      else
      --_nextToView;
    }

   /** putting the serialized element back to the buffer.
    *
    * This function will put the element that we just serialized back to the buffer.
    * It will will search the buffer for the element and then set the
    * flags of that element according to its current state.
    * @return void
    * @return[in] element reference to the pointer of the element
    */
    void doneViewing(reference element)
    {
      //create a lock//
      QMutexLocker lock(&_mutex);
      //retrieve iterator the the element that the user wants to put back to the buffer//
      iterator_t iElement = _buffer.begin();
      for ( ; iElement != _buffer.end() ; ++iElement)
        if (iElement->element == element)
          break;
      //set the property flags accordingly//
      iElement->inBearbeitung = false;
      //notify the waiting condition that something new is in the buffer//
      //we need to unlock the lock before//
      lock.unlock();
      _fillcondition.wakeOne();
    }

  private:
    QMutex            _mutex;             //!< sync the processing part
    QWaitCondition    _fillcondition;     //!< sync the filling part
    QWaitCondition    _processcondition;  //!< sync the processing part
    QWaitCondition    _viewcondition;     //!< sync the viewing part
    buffer_t          _buffer;            //!< the Container
    iterator_t        _nextToProcess;     //!< the next processable element
    iterator_t        _nextToView;        //!< the next viewable element
    iterator_t        _nextToFill;        //!< the next fillable element
  };
}
#endif
