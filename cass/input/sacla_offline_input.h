// Copyright (C) 2014, 2015 Lutz Foucar

/**
 * @file sacla_offline_input.h file contains declaration of sacla offline input
 *
 * @author Lutz Foucar
 */

#ifndef _SACLAOFFLINEINPUT_
#define _SACLAOFFLINEINPUT_

#include <string>

#include "input_base.h"
#include "cass.h"
#include "ringbuffer.hpp"
#include "cass_event.h"

namespace cass
{
/** forward declaration of the tag processor */
class TagListProcessor;

/** SACLA Offline Input for cass
 *
 * This class will be used in offline modus. I will take an string that
 * contains a filename. In the file that the filename points to has to be a
 * list with beamline number and runnumbers to analyse. These need to be given
 * as a comma separated list. If only these two numbers are provided the list
 * of tags will be retrieved for the given run. Optionally one can also provide
 * a list of tags to be analysed for that given run / beamline number
 * combination. This has to be provided as comma separated list immediatly
 * following the beamline / runnumber combination.
 * The filename name must be passed to the program with the -i parameter.
 *
 * For each run in the list it will retrieve the tagnumbers and extract the data
 * associated with the tag number using the cass::SACLAConverter
 *
 * @cassttng SACLAOfflineInput/{NbrThreads}\n
 *           Will tell CASS in how many chunks the tag list will be splitted.
 *           Each chunk will be processed individually by a separate thread.
 *           Default is 1.
 *
 * @author Lutz Foucar
 */
class SACLAOfflineInput :  public InputBase
{
public:
  /** create instance of this
   *
   * @param runlistname name of the file containing all runs that should be
   *                    processed
   * @param ringbuffer reference to the ringbuffer containing the CASSEvents
   * @param ratemeter reference to the ratemeter to measure the rate of the input
   * @param loadmeter reference to the ratemeter to measure the load of the input
   * @param quitwhendone flag that tells this class that it should quit the
   *                     Program when its done reading all events
   * @param parent The parent QT Object of this class
   */
  static void instance(std::string runlistname,
                       RingBuffer<CASSEvent>&,
                       Ratemeter &ratemeter,
                       Ratemeter &loadmeter,
                       bool quitwhendone,
                       QObject *parent=0);

  /** function with the main loop */
  void runthis();

  /** load the parameters used for the input */
  void load();

  /** retrieve the progress state
   *
   * @return the processing progress
   */
  double progress();

  /** retrieve the number of processed events
   *
   * @return number of processed events
   */
  uint64_t eventcounter();

  /** retrieve the number of skipped processed events
   *
   * @return number of processed events
   */
  uint64_t skippedeventcounter();

private:
  /** constructor
   *
   * @param runlistname name of the file containing all runs that should be
   *                    processed
   * @param ringbuffer reference to the ringbuffer containing the CASSEvents
   * @param ratemeter reference to the ratemeter to measure the rate of the input
   * @param loadmeter reference to the ratemeter to measure the load of the input
   * @param quitwhendone flag that tells this class that it should quit the
   *                     Program when its done reading all events
   * @param parent The parent QT Object of this class
   */
  SACLAOfflineInput(std::string runlistname,
                    RingBuffer<CASSEvent>&,
                    Ratemeter &ratemeter,
                    Ratemeter &loadmeter,
                    bool quitwhendone,
                    QObject *parent=0);


  /** number of chuncks that the list should be split into */
  int _chunks;

  /** flag to tell the thread to quit when its done with all files */
  bool _quitWhenDone;

  /** name of the file containing all files that we need to process */
  std::string _runlistname;

  /** define the processors container */
  typedef std::vector<std::tr1::shared_ptr<TagListProcessor> > proc_t;

  /** the processor container */
  proc_t _procs;
};

}//end namespace cass

#endif
