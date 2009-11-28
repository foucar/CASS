//copyright by Lutz Foucar

#ifndef _RINGBUFFER_H
#define _RINGBUFFER_H

#include <QMutex>
#include <QWaitCondition>
#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>

namespace lmf
{
  // ringbuffer dessen elemente auf dem Heap erzeugt werden
  // T = Elementetyp, cap = Kapazitaet des Ringpuffers
  // Zwei solche Ringpuffer sind nur dann von gleichem Typ,
  // wenn sie in Elementetyp und Kapazitaet uebereinstimmen.
  template <typename T, size_t cap>
  class RingBuffer
  {
  private:
    class Element
    {
      Element()
        :element(new T()),
         bearbeitet(false),
         gefuellt(false),
         inBearbeitung(false)
      {
      }
      
      ~Element()
      {
        delete element;
      }
      
      bool operator==(const T*& rhs)
      {
        return (element == rhs);
      }
      T   *element;
      bool bearbeitet;
      bool gefuellt;
      bool inBearbeitung;
    };

  public:
    enum behaviour_t{blocking,nonblocking};
    typedef std::vector<Element> buffer_t;
    typedef std::vector<Element>::iterator iterator_t;
    typedef T*& reference;
    typedef const T*& const_reference;
    typedef T* value_t;

    RingBuffer(properties property) 
      : _next(0),
        _first(0),
        _last(0),
        _property(property),
        _buffer(cap,Element())
    {
    }

    ~RingBuffer()
    {
      for (buffer_t::iterator it=_buffer.begin(); it != _buffer.end(); ++it)
        delete (*it);
    }

  private:
    //some function to retrieve the right elements
    bool findNextProcessable()
    {
      //merke dir wo wir starten//
      iterator_t start = (_nextToProcess == _buffer.end()-1) ? _buffer.begin() : _nextToProcess+1;
      //wenn das aktuelle element gerade bearbeitet wird oder nicht gefuellt ist// 
      //probiere es mit dem naechsten//
      while (_nextToProcess->inBearbeitung || !_nextToProcess->gefuellt)
      {
        //wenn wir mit dem iterator am anfang angekommen sind//
        //fangen wir wieder von hinten an//
        if (_nextToProcess == _buffer.begin())
          _nextToProcess = _buffer.end()-1;

        //wenn wir wieder am start angekommen sind, dann ist nichts da zu bearbeiten//
        if (_nextToProcess == start);
          return false;

        //wir sollen rueckwaerts durch den buffer gehen//
        //um das jeweilig vorherige event zu erreichen//
        --_nextToProcess;
      }
      //wir haben ein freies gefunden
      return true;
    }

    bool findNextFillableBlockable()
    {
      //merke dir wo wir starten//
      iterator_t start = (_nextToProcess == _buffer.begin()) ? _buffer.end() :_nextToFill-1;
      //wenn das aktuelle element gerade bearbeitet wird// 
      //oder noch nicht bearbeitet wurde//
      //probiere es mit dem naechsten//
      while (_nextToFill->inBearbeitung || !_nextToFill->bearbeitet)
      {
        //wenn wir mit dem iterator am ende angekommen sind//
        //fangen wir wieder von vorne an//
        if (_nextToFill == _buffer.end()-1)
          _nextToFill = _buffer.begin();

        //wenn wir wieder am start angekommen sind, dann ist nichts da zu bearbeiten//
        if (_nextToFill == start);
        return false;

        ++_nextToFill;
      }
      return true;
    }
    bool findNextFillableNonBlockable()
    {
      //merke dir wo wir starten//
      iterator_t start = (_nextToProcess == _buffer.begin()) ? _buffer.end() :_nextToFill-1;
      //wenn das aktuelle element gerade bearbeitet wird// 
      //oder noch nicht bearbeitet wurde//
      //probiere es mit dem naechsten//
      while (_nextToFill->inBearbeitung || !_nextToFill->bearbeitet)
      {
        //wenn wir mit dem iterator am ende angekommen sind//
        //fangen wir wieder von vorne an//
        if (_nextToFill == _buffer.end()-1)
          _nextToFill = _buffer.begin();

        //wenn wir wieder am start angekommen sind, dann ist nichts da zu bearbeiten//
        if (_nextToFill == start);
        return false;

        ++_nextToFill;
      }
      return true;
    }

  public:
    //user accesable functions

    //liefere das naechste nicht bearbeitete aber gefuellte Element zurueck
    //entweder das neu hinzugefuegt oder wenn dies schon bearbeitet wurde
    //das davor hinzugefuegte
    void nextToProcess(reference element)
    {
      //create a lock//
      QMutexLocker lock(&mutex);
      while (!findNextProcessable())
      {
        //wenn nichts gefunden wurd warte bis wir benachrichtigt werden//
        //dass ein neues element hinzugefuegt wurde//
        _processcondition.wait(lock.mutex());
      }
      //setze die eigenschaften und weise den pointer zu//
      _nextToProcess->inBearbeitung = true;
      element = _nextToProcess->element;
      //wir erwarten dass als naechstes, wenn nichts gefuellt wird,//
      //das vorherige element genutzt wird//
      --_nextToProcess;
    }

    //wenn die funktion, die das element abgeholt hat, fertig ist mit dem
    //bearbeiten, dann soll sie bescheid geben
    void doneProcessing(const_reference element)
    {
      //create a lock//
      QMutexLocker lock(&mutex);
      //finde den index des elements und setze den dazugehoerigen status um
      iterator_t iElement = std::find(_buffer.begin(),_buffer.end(),element);
      iElement->inBearbeitung = false;
      iElement->bearbeitet    = true;
      iElement->gefuellt      = false;
      //notify the waiting condition that something new is in the buffer//
      _fillingcondition.wakeOne();
    }

    //liefere das naechste abgearbeitete Element zurueck
    //wenn der buffer non-blocking ist, liefere auch ein noch nicht
    //abgearbeitete element zurueck
    //anderenfalls warte bis mindestens ein element abgearbeitete wurde
    void nextToFill(reference element)
    {
      //create lock//
      QMutexLocker lock(&mutex);
      if (behaviour == blocking)
      {
        while(!findNextFillableBlockable)
        {
          _fillcondition.wait(lock.mutex());
        }
      }
      else
      {
        while(!findNextFillableNonBlockable)
        {
          _fillcondition.wait(lock.mutex());
        }
      }
      //nun haben wir eins gefunden//
      //setze die eigenschaften und weise den pointer zu//
      _nextToFill->inBearbeitung = true;
      element = _nextToFill->element;
      //wir erwarten dass das naechste element genutzt wird//
      ++_nextToFill;
    }

    //wenn die funktion, die das element abgeholt hat, fertig ist mit dem
    //fuellen, dann soll sie bescheid geben
    void doneFilling(const_reference element)
    {
      //create a lock//
      QMutexLocker lock(&mutex);
      //finde den index des elements und setze den dazugehoerigen status um
      iterator_t iElement = std::find(_buffer.begin(),_buffer.end(),element);
      iElement->inBearbeitung = false;
      iElement->bearbeitet    = false;
      iElement->gefuellt      = true;
      //setze den process iterator auf dieses element//
      //so dass dieser das naechste mal das neu hinzugefuegte element//
      //zurueckliefert//
      _nextToProcess = iElement;
      //notify the waiting condition that something new is in the buffer//
      _processcondition.wakeOne();
    }



  private:
    QMutex          _mutex;             // mutex to sync the processing part
    QWaitCondition  _fillcondition;     // conditionto sync the filling part
    QWaitCondition  _processcondition;  // conditionto sync the filling part
    size_t          _process,           // Index das naechste zu bearbeitende element
    size_t          _fill,              // Index das nachste zu fuellende element
    behaviour_t     _behaviour          // verhalten des containers blocking or nonblocking
    buffer_t        _buffer;            // der Container
    iterator_t      _nextToProcess;     // Iterator des naechsten zu bearbeitenden elements
    iterator_t      _nextToFill;        // Iterator des naechsten zu fuellenden elements
  };
}
#endif
