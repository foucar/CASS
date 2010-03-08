//copyright by Lutz Foucar

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
  // ringbuffer dessen elemente auf dem Heap erzeugt werden
  // T = Elementetyp, cap = Kapazitaet des Ringpuffers
  // Zwei solche Ringpuffer sind nur dann von gleichem Typ,
  // wenn sie in Elementetyp und Kapazitaet uebereinstimmen.

  //todo ob es blockable oder nicht blockabel in die//
  //compilezeit uebernehmen//
  template <typename T, size_t cap>
  class RingBuffer
  {
  private:
    //ein Element des Rinbuffers//
    //enthaellt den Status des Elements und//
    //ein Zeiger auf das eigentliche Element//
    class Element
    {
    public:
      Element()
        :element(),
         bearbeitet(true),
         gefuellt(false),
         inBearbeitung(false)
      {
      }

      ~Element()
      {
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
    typedef typename buffer_t::iterator iterator_t;
    typedef T*& reference;
    typedef const T*& const_reference;
    typedef T* value_t;


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

    ~RingBuffer()
    {
      //delete all elements in the ringbuffer//
      for (size_t i=0; i<_buffer.size(); ++i)
        delete _buffer[i].element;
    }

  private:
    //some function to retrieve the right elements
    bool findNextProcessable()
    {
      //merke dir wo wir enden sollten//
      //dies sollte spaetestens dort passieren wo der next fillable steht sein//
      //da die elemente davor entweder schon bearbeitet worden sind//
      //oder noch bearbeitet werden//
      iterator_t letztesFreiesElement = _nextToFill;
      //wenn das aktuelle element gerade bearbeitet wird oder nicht gefuellt ist//
      //probiere es mit dem naechsten//
      while (_nextToProcess->inBearbeitung || !_nextToProcess->gefuellt)
      {

        //wenn wir wieder am dort wo der filler steht angekommen sind//
        //dann ist nichts da zu bearbeiten//
        if (_nextToProcess == letztesFreiesElement)
          return false;

        //wir sollen rueckwaerts durch den buffer gehen//
        //um das jeweilig vorherige event zu erreichen//
        //wenn wir mit dem iterator am anfang angekommen sind//
        //fangen wir wieder von hinten an//
        if (_nextToProcess == _buffer.begin())
          _nextToProcess = _buffer.end()-1;
        else
          --_nextToProcess;
      }
      //wir haben ein freies gefunden
      return true;
    }
    //some function to retrieve the right elements
    bool findNextViewable()
    {
      //merke dir wo wir enden sollten//
      //dies sollte spaetestens dort passieren wo der next fillable steht sein//
      //da die elemente davor entweder schon bearbeitet worden sind//
      //oder noch bearbeitet werden//
      iterator_t letztesElement = _nextToView;
      //wenn das aktuelle element gerade bearbeitet wird oder nicht gefuellt ist//
      //probiere es mit dem naechsten//
      while (_nextToView->inBearbeitung || !_nextToView->bearbeitet)
      {

        //wenn wir wieder am dort wo der filler steht angekommen sind//
        //dann ist nichts da zu bearbeiten//
        if (_nextToView == letztesElement)
          return false;

        //wir sollen rueckwaerts durch den buffer gehen//
        //um das jeweilig vorherige event zu erreichen//
        //wenn wir mit dem iterator am anfang angekommen sind//
        //fangen wir wieder von hinten an//
        if (_nextToView == _buffer.begin())
          _nextToView = _buffer.end()-1;
        else
          --_nextToView;
      }
      //wir haben ein anschaubares gefunden
      return true;
    }

    bool findNextFillableBlockable()
    {
      //merke dir wo wir starten//
      iterator_t start =
          (_nextToFill == _buffer.begin()) ? _buffer.end()-1 :_nextToFill-1;
      //wenn das aktuelle element gerade bearbeitet wird// 
      //oder noch nicht bearbeitet wurde//
      //probiere es mit dem naechsten//
      while (_nextToFill->inBearbeitung || !_nextToFill->bearbeitet)
      {
        //wenn wir wieder am start angekommen sind
        //sind sie entweder noch nicht bearbeitet oder noch in Bearbeitung//
        if (_nextToFill == start)
          return false;

        //wenn wir mit dem iterator am ende angekommen sind//
        //fangen wir wieder von vorne an//
        if (_nextToFill == _buffer.end()-1)
          _nextToFill = _buffer.begin();
        else
          ++_nextToFill;
      }
      return true;
    }
    bool findNextFillableNonBlockable()
    {
      //merke dir wo wir starten//
      iterator_t start =
          (_nextToFill == _buffer.begin()) ? _buffer.end()-1 :_nextToFill-1;
      //wenn das aktuelle element gerade bearbeitet wird// 
      //probiere es mit dem naechsten//
      while (_nextToFill->inBearbeitung)
      {
        //wenn wir wieder am start angekommen sind//
        //dann ist keins da das frei ist um es zu fuellen//
        if (_nextToFill == start)
          return false;

        //wenn wir mit dem iterator am ende angekommen sind//
        //fangen wir wieder von vorne an//
        if (_nextToFill == _buffer.end()-1)
          _nextToFill = _buffer.begin();
        else
          ++_nextToFill;
      }
      return true;
    }

  public:
    //user accesable functions
    
    //um das verhalten zu setzen
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
    QMutex            _mutex;             // mutex to sync the processing part
    QWaitCondition    _fillcondition;     // condition to sync the filling part
    QWaitCondition    _processcondition;  // condition to sync the processing part
    QWaitCondition    _viewcondition;     // condition to sync the viewing part
    behaviour_t       _behaviour;         // verhalten des containers
    buffer_t          _buffer;            // der Container
    iterator_t        _nextToProcess;     // Iterator des naechsten zu bearbeitenden elements
    iterator_t        _nextToView;        // Iterator des naechsten zu anzschauenden elements
    iterator_t        _nextToFill;        // Iterator des naechsten zu fuellenden elements
  };
}
#endif
