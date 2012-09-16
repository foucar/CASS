// Copyright (C) 2012 Lutz Foucar

/**
 * @file test_input.h file contains declaration of a input for testing purposes
 *
 * @author Lutz Foucar
 */

#ifndef _TESTINPUT_H_
#define _TESTINPUT_H_

#include <vector>
#include <tr1/memory>

#include "input_base.h"
#include "cass.h"
#include "ringbuffer.h"
#include "cass_event.h"

namespace cass
{
//forward declarations
class DataGenerator;

/** Testing Input for cass
 *
 * It should be used when testing cass.
 *
 * It will fill the cassevents with data for several devices, such that
 * PostProcessors can be tested. It uses the data generators to fill the cassevents.
 *
 * @cassttng TestInput/{key}\n
 *
 * @author Lutz Foucar
 */
class TestInput :  public InputBase
{
public:
  /** create instance of this
   *
   * @param ringbuffer reference to the ringbuffer containing the CASSEvents
   * @param ratemeter reference to the ratemeter to measure the rate of the input
   * @param parent The parent QT Object of this class
   */
  static void instance(RingBuffer<CASSEvent,RingBufferSize>&,
                       Ratemeter &ratemeter,
                       QObject *parent=0);

  /** function with the main loop */
  void run();

  /** load the parameters used for the input */
  void load();

private:
  /** constructor
   *
   * @param ringbuffer reference to the ringbuffer containing the CASSEvents
   * @param ratemeter reference to the ratemeter to measure the rate of the input
   * @param parent The parent QT Object of this class
   */
  TestInput(RingBuffer<CASSEvent,RingBufferSize>&,
            Ratemeter &ratemeter,
            QObject *parent=0);

  /** define a container for all data generators */
  typedef std::vector<std::tr1::shared_ptr<DataGenerator> > generators_t;

  /** container for all used fillers */
  generators_t _generators;

  /** a counter to create a fake event id */
  size_t _counter;
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
