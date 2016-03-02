// Copyright (C) 2009 - 2016 Lutz Foucar

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
#include "ringbuffer.hpp"
#include "cass_event.h"
#include "file_reader.h"

namespace cass
{
/** forward declaration of the file processors */
class FileProcessor;

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
 * @cassttng FileInput/{Parallelize}\n
 *           When true, it will read the files on the fileinput list in parallel.
 *           Default is false.
 *
 * @author Lutz Foucar
 */
class FileInput :  public InputBase
{
public:
  /** create instance of this
   *
   * @param filelistname name of the file containing all files that should be
   *                     processed
   * @param ringbuffer reference to the ringbuffer containing the CASSEvents
   * @param ratemeter reference to the ratemeter to measure the rate of the input
   * @param loadmeter reference to the ratemeter to measure the load of the input
   * @param quitwhendone flag that tells this class that it should quit the
   *                     Program when its done reading all events
   * @param parent The parent QT Object of this class
   */
  static void instance(std::string filelistname,
                       RingBuffer<CASSEvent>&,
                       Ratemeter &ratemeter,
                       Ratemeter &loadmeter,
                       bool quitwhendone,
                       QObject *parent=0);

  /** function with the main loop */
  void runthis();

  /** load the parameters used for the multifile input */
  void load();

  /** retrieve the averaged progress state of all file processors
   *
   * @return the processing progress
   */
  double progress();

  /** retrieve the number of processed events
   *
   * @return number of processed events
   */
  uint64_t eventcounter();

private:
  /** constructor
   *
   * @param filelistname name of the file containing all files that should be
   *                     processed
   * @param ringbuffer reference to the ringbuffer containing the CASSEvents
   * @param ratemeter reference to the ratemeter to measure the rate of the input
   * @param loadmeter reference to the ratemeter to measure the load of the input
   * @param quitwhendone flag that tells this class that it should quit the
   *                     Program when its done reading all events
   * @param parent The parent QT Object of this class
   */
  FileInput(std::string filelistname,
            RingBuffer<CASSEvent>&,
            Ratemeter &ratemeter,
            Ratemeter &loadmeter,
            bool quitwhendone,
            QObject *parent=0);

  /** flag that tells the input should analyze the files in parallel */
  bool _parallelize;

  /** flag to tell the thread to quit when its done with all files */
  bool _quitWhenDone;

  /** name of the file containing all files that we need to process */
  std::string _filelistname;

  /** define the container for all file processors */
  typedef std::vector<std::tr1::shared_ptr<FileProcessor> > fileProcessors_t;

  /** the file processor container */
  fileProcessors_t _fProcs;
};

}//end namespace cass

#endif
