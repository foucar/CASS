// Copyright (C) 2014 Lutz Foucar

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
#include "ringbuffer.h"
#include "cass_event.h"

namespace cass
{
/** SACLA Offline Input for cass
 *
 * This class will be used in offline modus. I will take an string that
 * contains a filename. In the file that the filename points to has to be a
 * list with files, that one wants to analyze.
 * The filename name must be passed to the program with the -i parameter.
 *
 * For each file in the filelist it will open the file, and call the readers
 * to extract the data from the file.
 *
 * @cassttng FileInput/{useNewContainer}\n
 *           set to true if you want to use the new container for pixeldetector
 *           data that can make use of the new analysis chain. Default is false.
 * @cassttng FileInput/{Rewind}\n
 *           Tells the program to start over running over all files when true.
 *           Default is false.
 *
 * @author Lutz Foucar
 */
class SACLAOfflineInput :  public InputBase
{
public:
  /** create instance of this
   *
   * @param runlistname name of the file containing all runs that should be
   *                     processed
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
  void run();

  /** load the parameters used for the multifile input */
  void load();

private:
  /** constructor
   *
   * @param runlistname name of the file containing all runs that should be
   *                     processed
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

  /** flag that tells the input to rewind to the beginning of the eventlist */
  bool _rewind;

  /** flag to tell the thread to quit when its done with all files */
  bool _quitWhenDone;

  /** name of the file containing all files that we need to process */
  std::string _runlistname;
};

}//end namespace cass

#endif
