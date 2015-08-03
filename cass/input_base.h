// Copyright (C) 2011 Lutz Foucar

/**
 * @file input_base.h contains the base class for all input modules
 *
 * @author Lutz Foucar
 */

#ifndef _INPUTBASE_H_
#define _INPUTBASE_H_

#include <string>
#include <tr1/memory>

#include "pausablethread.h"

#include "cass.h"
#include "cass_event.h"
#include "ringbuffer.hpp"

namespace cass
{
class Ratemeter;

/** Input base class
 *
 * This class acts as base class for all input modules.
 *
 * @author Lutz Foucar
 */
class InputBase : public lmf::PausableThread
{
public:
  /** shared pointer of this type */
  typedef std::tr1::shared_ptr<InputBase> shared_pointer;

  /** destructor */
  virtual ~InputBase(){}

  /** function with the main loop
   *
   * the implementation of this function needs to start by setting the _status
   * variable of the PausableThread to running.like follows:
   * _status = lmf::PausableThread::running;
   */
  virtual void runthis()=0;

  /** load the settings of the input module from ini file
   *
   * @note before calling this the caller has to make sure that the input has
   *       paused. And resume it afterwards.
   */
  virtual void load()=0;

  /** increment the numer of events received in the ratemeter
   *
   * To indicate that we are done processing an event this signal is emitted.
   * This is used for by the ratemeter to evaluate how fast we get events.
   *
   * @param eventsize size of the event in bytes
   */
  void newEventAdded(const size_t eventsize);

  /** retrieve a reference to the the ringbuffer
   *
   * @return reference to the ringbuffer
   */
  RingBuffer<CASSEvent>& ringbuffer();

  /** get the signelton instance
   *
   * throws logic error when instance does not exist yet
   *
   * @return the singleton instance
   */
  static shared_pointer instance();

  /** get reference to the singelton instance
   *
   * throws logic error when instance does not exist yet
   *
   * @return reference to the singleton
   */
  static shared_pointer::element_type& reference();

  /** a mutex so that external program can lock access to this */
  QMutex lock;

  /** define an item in the ringbuffer */
  typedef RingBuffer<CASSEvent>::iter_type rbItem_t;

  /** retrieve an iterator to the next fillable event
   *
   * try to get an event to be filled from the buffer. Do this until either an
   * event could be retrieved or the thread has been told to quit.
   *
   * @return iterator to the next fillable event, when the end of the ringbuffer
   *         has been returned, the thread should quit
   */
  rbItem_t getNextFillable(unsigned timeout=500);

protected:
  /** protected constructor since it should be a singelton
   *
   * @param ringbuffer reference to the ringbuffer containing the CASSEvents
   * @param ratemeter reference to the ratemeter to measure the rate of the input
   * @param loadmeter reference to the ratemeter to measure the load of the input
   * @param parent The parent QT Object of this class
   */
  InputBase(RingBuffer<CASSEvent>& ringbuffer,
            Ratemeter & ratemeter,
            Ratemeter & loadmeter,
            QObject *parent=0)
    : PausableThread(lmf::PausableThread::_run,parent),
      _ringbuffer(ringbuffer),
      _ratemeter(ratemeter),
      _loadmeter(loadmeter)
  {}

  /** reference to the ringbuffer */
  RingBuffer<CASSEvent> &_ringbuffer;

  /** ratemeter to measure the rate */
  Ratemeter & _ratemeter;

  /** meter to measure the data load */
  Ratemeter & _loadmeter;

  /** singelton instance */
  static shared_pointer _instance;

private:
  /** a mutex to lock operations */
  static QMutex _mutex;
};

}//end namespace cass

#endif
