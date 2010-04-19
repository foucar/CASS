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
  /** A Ringbuffer.
   * The Ringbuffers Elements will be created on the Heap.
   * @note can we create the ringbuffer to have the blockable / nonblockable property
   *       already at compile time?
   * @tparam T Element typ
   * @tparam cap Capacity of the ringbuffer
   * @author Lutz Foucar
   */
  template <typename T, size_t cap>
  class RingBuffer
  {
  private:
    /** an element of the ringbuffer.
     * contains the status of the element and a pointer
     * to the actual element.
     * @author Lutz Foucar
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
      /** Comparison of two elements.
       * Two Ringbuffers are the same only when their elements type and
       * capacity are equal
       */
      bool operator==(const T*& rhs)
      {
        return (element == rhs);
      }
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
    /** enum describing the behaviour of the ringbuffer*/
    enum behaviour_t{blocking,nonblocking};
    /** typedef describing the container of all elements*/
    typedef std::vector<Element> buffer_t;
    /** typedef describing an interator for the elements of the container*/
    typedef typename buffer_t::iterator iterator_t;
    /** typedef describing a reference to the elements of the container*/
    typedef T*& reference;
    /** typedef describing a const reference to the elements of the container*/
    typedef const T*& const_reference;
    /** typedef describing a pointer to the elements of the container*/
    typedef T* value_t;

    /** constructor.
     * will create the buffer and fill it with the requested amount of elements
     * and initialize the iterators.
     */
    RingBuffer() 
      : _behaviour(blocking),
        _buffer(cap,Element()),
        _nextToProcess(_buffer.begin()),
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
      iterator_t letztesFreiesElement = _nextToFill;
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
     * this function is used when the behaviour of the ringbuffer is blockable
     * it will iterate through the buffer and checks the elements for the
     * status in progress (inBearbeitung) and processed (bearbeitet)
     * it will only return true when its not in progress and already processed.
     * @return true when a fillable element has been found
     */
    bool findNextFillableBlockable()
    {
      //remember where you are starting//
      iterator_t start =
          (_nextToFill == _buffer.begin()) ? _buffer.end()-1 :_nextToFill-1;
      //if the current element is currently processed or has not been processed//
      //try the next element//
      while (_nextToFill->inBearbeitung || !_nextToFill->bearbeitet)
      {
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
    /** finds the next fillable element.
     * this function is used when the behaviour of the ringbuffer is nonblockable
     * it will iterate through the buffer and checks the elements for
     * only the status in progress (inBearbeitung).
     * it will only return true when its not in progress.
     * @return true when a fillable element has been found
     */
    bool findNextFillableNonBlockable()
    {
      //remember where you are starting//
      iterator_t start =
          (_nextToFill == _buffer.begin()) ? _buffer.end()-1 :_nextToFill-1;
      //if the current element is currently processed//
      //try the next element//
      while (_nextToFill->inBearbeitung)
      {
        //if we end up where we started, then the elements are not yet processed//
        //so retrun that we have not found anything yet.//
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
    /** sets the behaviour of the ringbuffer.
     * @return void
     * @param behaviour the behaviour that the ringbuffer should have
     */
    void behaviour(behaviour_t behaviour)
    {
      _behaviour = behaviour;
    }

    //liefere das naechste nicht bearbeitete aber gefuellte Element zurueck
    //entweder das neu hinzugefuegt oder wenn dies schon bearbeitet wurde
    //das davor hinzugefuegte
    void nextToProcess(reference element, int timeout=ULONG_MAX)
    {
      //create a lock//
      QMutexLocker lock(&_mutex);
      while (!findNextProcessable())
      {
        //wenn nichts gefunden wurd warte bis wir benachrichtigt werden//
        //dass ein neues element hinzugefuegt wurde//
        if(!_processcondition.wait(lock.mutex(),timeout))
        {
          element = 0;
          return;
        }
      }
      //setze die eigenschaften und weise den pointer zu//
      _nextToProcess->inBearbeitung = true;
      element = _nextToProcess->element;
      //wir erwarten dass als naechstes, wenn nichts gefuellt wird,//
      //das vorherige element genutzt wird//
      if (_nextToProcess == _buffer.begin())
        _nextToProcess = _buffer.end()-1;
      else
      --_nextToProcess;
    }

    //wenn die funktion, die das element abgeholt hat, fertig ist mit dem
    //bearbeiten, dann soll sie bescheid geben
    void doneProcessing(reference element)
    {
      //create a lock//
      QMutexLocker lock(&_mutex);
      //finde den index des elements und setze den dazugehoerigen status um
      iterator_t iElement = _buffer.begin();
      for ( ; iElement != _buffer.end() ; ++iElement)
        if (iElement->element == element)
          break;
      iElement->inBearbeitung = false;
      iElement->bearbeitet    = true;
      iElement->gefuellt      = false;
      //notify the waiting condition that something new is in the buffer//
      //we need to unlock the lock before//
      lock.unlock();
      _fillcondition.wakeOne();
    }

    //liefere das naechste abgearbeitete Element zurueck
    //wenn der buffer non-blocking ist, liefere auch ein noch nicht
    //abgearbeitete element zurueck
    //anderenfalls warte bis mindestens ein element abgearbeitete wurde
    void nextToFill(reference element)
    {
      //create lock//
      QMutexLocker lock(&_mutex);
      if (_behaviour == blocking)
      {
        while(!findNextFillableBlockable())
        {
          _fillcondition.wait(lock.mutex());
        }
      }
      else
      {
        while(!findNextFillableNonBlockable())
        {
          _fillcondition.wait(lock.mutex());
        }
      }
      //nun haben wir eins gefunden//
      //setze die eigenschaften und weise den pointer zu//
      _nextToFill->inBearbeitung = true;
      element = _nextToFill->element;
      //wir erwarten dass das naechste element genutzt wird//
      if (_nextToFill == _buffer.end()-1)
        _nextToFill = _buffer.begin();
      else
        ++_nextToFill;
    }

    //wenn die funktion, die das element abgeholt hat, fertig ist mit dem
    //fuellen, dann soll sie bescheid geben
    void doneFilling(reference element, bool fillstatus=true)
    {
      //create a lock//
      QMutexLocker lock(&_mutex);
      //finde den index des elements und setze den dazugehoerigen status um
      iterator_t iElement = _buffer.begin();
      for ( ; iElement != _buffer.end() ; ++iElement)
        if (iElement->element == element)
          break;
      iElement->inBearbeitung = false;
      iElement->bearbeitet    = !fillstatus;
      iElement->gefuellt      = fillstatus;
      //setze den process iterator auf dieses element//
      //so dass dieser das naechste mal das neu hinzugefuegte element//
      //zurueckliefert//
      _nextToProcess = iElement;
      //notify the waiting condition that something new is in the buffer//
      //we need to unlock the lock before//
      lock.unlock();
      _processcondition.wakeOne();
    }

    //hole das letzt hinzugefügte element des processors, wenn das schon abgeholt wurde, das davor...//
    //ein element ist anzuschauen, wenn es gefüllt und processiert wurde//
   void nextToView(reference element, int timeout=ULONG_MAX)
   {
      //create a lock//
      QMutexLocker lock(&_mutex);
      while (!findNextViewable())
      {
        //wenn nichts gefunden wurd warte bis wir benachrichtigt werden//
        //dass ein neues element hinzugefuegt wurde//
        if(!_viewcondition.wait(lock.mutex(),timeout))
        {
          element = 0;
          return;
        }
      }
      //setze die eigenschaften und weise den pointer zu//
      _nextToView->inBearbeitung = true;
      element = _nextToView->element;
      //wir erwarten dass als naechstes, wenn nichts gefuellt wird,//
      //das vorherige element genutzt wird//
      if (_nextToView == _buffer.begin())
        _nextToView = _buffer.end()-1;
      else
      --_nextToView;
    }
    //wenn die funktion, die das element abgeholt hat, fertig ist mit dem
    //anschauen, dann soll sie bescheid geben
    void doneViewing(reference element)
    {
      //create a lock//
      QMutexLocker lock(&_mutex);
      //finde den index des elements und setze den dazugehoerigen status um
      iterator_t iElement = _buffer.begin();
      for ( ; iElement != _buffer.end() ; ++iElement)
        if (iElement->element == element)
          break;
      iElement->inBearbeitung = false;
      //notify the waiting condition that something new is in the buffer//
      //we need to unlock the lock before//
      lock.unlock();
      _fillcondition.wakeOne();
    }

  private:
    QMutex            _mutex;             //!< mutex to sync the processing part
    QWaitCondition    _fillcondition;     //!< condition to sync the filling part
    QWaitCondition    _processcondition;  //!< condition to sync the processing part
    QWaitCondition    _viewcondition;     //!< condition to sync the viewing part
    behaviour_t       _behaviour;         //!< container's behaviour
    buffer_t          _buffer;            //!< the Container
    iterator_t        _nextToProcess;     //!< Iterator pointing to the next processable element
    iterator_t        _nextToView;        //!< Iterator pointing to the next viewable element
    iterator_t        _nextToFill;        //!< Iterator pointing to the next fillable element
  };
}
#endif
