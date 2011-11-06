// Copyright (C) 2009 - 2011 Lutz Foucar

/**
 * @file file_input.h file contains declaration of xtcfile input
 *
 * @author Lutz Foucar
 */

#ifndef _FILEINPUT_H_
#define _FILEINPUT_H_

#include <string>

#include "input_base.h"
#include "cass.h"
#include "ringbuffer.h"
#include "cass_event.h"
#include "file_reader.h"

namespace cass
{
/** File Input for cass
 *
 * This class will be used in offline modus. I will take an string that
 * contains a filename. In the file that the filename points to has to be a
 * list with files, that one wants to analyze.
 * The filename name must be passed to the program with the -i parameter.
 *
 * For each file in the filelist it will open the file, and call the readers
 * to extract the data from the file.
 *
 * @cassttng FileInput/{Rewind}\n
 *           Tells the program to start over running over all files when true.
 *           Default is false.
 *
 * @author Lutz Foucar
 */
class FileInput :  public InputBase
{
public:
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
  FileInput(std::string filelistname,
            RingBuffer<CASSEvent,RingBufferSize>&,
            Ratemeter &ratemeter,
            bool quitwhendone,
            QObject *parent=0);

  /** destructor */
  ~FileInput();

  /** function with the main loop */
  void run();

  /** load the parameters used for the multifile input */
  void load();

private:
  /** flag that tells the input to rewind to the beginning of the eventlist */
  bool _rewind;

  /** flag to tell the thread to quit when its done with all files */
  bool _quitWhenDone;

  /** name of the file containing all files that we need to process */
  std::string _filelistname;

  /** shared pointer to the actual reader */
  FileReader::shared_pointer _read;
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
