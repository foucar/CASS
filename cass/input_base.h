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
#include "ringbuffer.h"

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
  virtual void run()=0;

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
   */
  void newEventAdded();

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

  /** load the parameters used for this input module
   *
   * pauses the thread then loads the settings of the MulitFileInput. Then
   * calls the pure virtual function load. Then resumes the thread.
   *
   * @param what unused parameter
   */
//  void loadSettings(size_t/* what*/);

  /** a mutex so that external program can lock access to this */
  QMutex lock;

protected:
  /** protected constructor since it should be a singelton
   *
   * @param ringbuffer reference to the ringbuffer containing the CASSEvents
   * @param ratemeter reference to the ratemeter to measure the rate of the input
   * @param parent The parent QT Object of this class
   */
  InputBase(RingBuffer<CASSEvent,RingBufferSize>& ringbuffer,
            Ratemeter & ratemeter,
            QObject *parent=0)
    :PausableThread(lmf::PausableThread::_run,parent),
     _ringbuffer(ringbuffer),
     _ratemeter(ratemeter)
  {}

  /** reference to the ringbuffer */
  RingBuffer<CASSEvent,RingBufferSize>  &_ringbuffer;

  /** ratemeter to measure the rate */
  Ratemeter & _ratemeter;

  /** singelton instance */
  static shared_pointer _instance;

private:
  /** a mutex to lock operations */
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
