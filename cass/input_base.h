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
/** Input base class
 *
 * This class acts as base class for all input modules.
 *
 * @author Lutz Foucar
 */
class InputBase : public lmf::PausableThread
{
  Q_OBJECT

public:
  /** shared pointer of this type */
  typedef std::tr1::shared_ptr<InputBase> shared_pointer;

  /** constructor
   *
   * @param ringbuffer reference to the ringbuffer containing the CASSEvents
   * @param parent The parent QT Object of this class
   */
  InputBase(RingBuffer<CASSEvent,RingBufferSize>& ringbuffer, QObject *parent=0)
    :PausableThread(lmf::PausableThread::_run,parent),
     _ringbuffer(ringbuffer)
  {}

  /** destructor */
  virtual ~InputBase(){}

  /** function with the main loop
   *
   * the implementation of this function needs to start by setting the _status
   * variable of the PausableThread to running.like follows:
   * _status = lmf::PausableThread::running;
   */
  virtual void run()=0;

  /** load the settings of the input module from ini file */
  virtual void load()=0;

public slots:
  /** load the parameters used for this input module
   *
   * pauses the thread then loads the settings of the MulitFileInput. Then
   * calls the pure virtual function load. Then resumes the thread.
   *
   * @param what unused parameter
   */
  void loadSettings(size_t/* what*/)
  {
    pause(true);
    load();
    resume();
  }

//  /** return an instance of the requested input module type
//   *
//   * @return shared pointer of the requested type
//   * @param type the type of input module that should be returned
//   */
//  static shared_pointer instance(std::string type);

signals:
  /** signal emitted when done with one event
   *
   * To indicate that we are done processing an event this signal is emitted.
   * This is used for by the ratemeter to evaluate how fast we get events.
   */
  void newEventAdded();

protected:
  /** reference to the ringbuffer */
  RingBuffer<CASSEvent,RingBufferSize>  &_ringbuffer;
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
