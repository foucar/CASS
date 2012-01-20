// Copyright (C) 2011 Lutz Foucar

/**
 * @file multifile_input.h file contains declaration of file input reading
 *                         multiple files in parallel.
 *
 * @author Lutz Foucar
 */

#ifndef _MULTIFILEINPUT_H_
#define _MULTIFILEINPUT_H_

#include <string>

#include "input_base.h"

#include "cass.h"
#include "ringbuffer.h"
#include "cass_event.h"
#include "file_reader.h"

namespace cass
{
/** Multi File Input for cass
 *
 * This class will be used in offline modus. It will take an string that
 * points to a file containing a list with filenames.
 * The filename name must be passed to the program with the -i parameter.
 *
 * Purpose of this class is that one can read multiple files and associate
 * the data of the different files using the event id. Therefore this class
 * contains a map that allows to map an event id to a set of filestreams
 * where the data associated with this event id can be found in the different
 * files.
 *
 * Depending on the file extension of the file name it will call an instance
 * to the right file parser. Once all files have been parsed an the event map
 * has been build up it will iterate through the event map and read the data
 * in the different files. The data is then converted to a CASSEvent and put
 * into the RingBuffer
 *
 * @cassttng MultiFileInput/{NbrDifferentSources}\n
 *           Tells the program how many different sources it should expect. If
 *           for one eventId there are less sources recognized than this number
 *           a warning will be issued and the event is skipped. Default is 2.
 * @cassttng MultiFileInput/{UseNewContainer}\n
 *           set to true if you want to use the new container for pixeldetector
 *           data that can make use of the new analysis chain. Default is false.
 * @cassttng MultiFileInput/{Rewind}\n
 *           Tells the program to start over running over all files when true.
 *           Default is false.
 *
 * @author Lutz Foucar
 */
class MultiFileInput : public InputBase
{
public:
  /** create an instance of this
   *
   * @param filelistname name of the file containing all files that should be
   *                     processed
   * @param ringbuffer reference to the ringbuffer containing the CASSEvents
   * @param ratemeter reference to the ratemeter to measure the rate of the input
   * @param quitwhendone flag that tells this class that it should quit the
   *                     Program when its done reading all events
   * @param parent The parent QT Object of this class
   */
  static void instance(const std::string& filelistname,
                       RingBuffer<CASSEvent,RingBufferSize>& ringbuffer,
                       Ratemeter &ratemeter,
                       bool quitwhendone,
                       QObject *parent=0);

  /** destructor */
  ~MultiFileInput();

  /** function with the main loop */
  void run();

  /** load the parameters used for the multifile input */
  void load();

private:
  /** constructor
   *
   * @param filelistname name of the file containing all files that should be
   *                     processed
   * @param ringbuffer reference to the ringbuffer containing the CASSEvents
   * @param ratemeter reference to the ratemeter to measure the rate of the input
   * @param quitwhendone flag that tells this class that it should quit the
   *                     Program when its done reading all events
   * @param parent The parent QT Object of this class
   */
  MultiFileInput(const std::string& filelistname,
                 RingBuffer<CASSEvent,RingBufferSize>& ringbuffer,
                 Ratemeter &ratemeter,
                 bool quitwhendone,
                 QObject *parent=0);

  /** read the information from the different files into the cassevent
   *
   * @param eventIt iterator to the map containing all events
   */
  void readEventData(event2positionreaders_t::iterator &eventIt);

  /** flag to tell the thread to quit when its done with all files */
  bool _quitWhenDone;

  /** the name of the file that contains the list of file to process */
  std::string _filelistname;

  /** flag that tells the input to rewind to the beginning of the eventlist */
  bool _rewind;

  /** string to identify whether one want to use the new container */
  std::string _new;

  /** the number of different sources */
  size_t _nbrDifferentSources;
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
