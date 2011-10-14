// Copyright (C) 2009 - 2011 Lutz Foucar

/**
 * @file file_input.h file contains declaration of xtcfile input
 *
 * @author Lutz Foucar
 */

#ifndef _FILEINPUT_H_
#define _FILEINPUT_H_

#include <QtCore/QObject>
#include <QThread>
#include <QMutex>

#include <string>

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
   * @cassttng FileInput/{FileType}\n
   *           What kind of files do you want to analyze. Default value is "xtc"
   *           Possible values are:
   *           - xtc: reads xtc files recorded at slac see XtcReader
   *           - lma: reads lma files recorded by AGAT. see LmaReader
   *           - raw_sss: reads lma files recorded by Per's VMI Program. see 
   *                      RAWSSSReader
   *           - frm6: reads .frm6 files recorded by xOnline. see FRM6Reader
   *
   * @author Lutz Foucar
   */
  class CASSSHARED_EXPORT FileInput : public QThread
  {
    Q_OBJECT;
  public:
    /** constructor */
    FileInput(std::string filelistname,
              RingBuffer<CASSEvent,RingBufferSize>&,
              bool quitwhendone,
              QObject *parent=0);

    /** destructor */
    ~FileInput();

    /** function with the main loop */
    void run();

    /** suspends the thread
     *
     * blocks until the thread has suspended
     */
    void suspend();

    /** resumes the thread */
    void resume();

  public slots:
    /** tell the thread to quit */
    void end();

    /** load the parameters used for this thread
     *
     * @param what unused parameter
     */
    void loadSettings(size_t what);

  signals:
    /** signal emitted when done with one event
     *
     * To indicate that we are done processing an event this signal is emitted.
     * This is used for by the ratemeter to evaluate how fast we get events.
     */
    void newEventAdded();

  protected:
    /** helper function for suspending
     *
     * this function call will block until the thread is suspended.
     */
    void waitUntilSuspended();

    /** call this at the point you want to pause
     *
     * when told to pause, this will actually pause and only resume when resume
     * is called.
     */
    void pausePoint();

    /** tokenize the file containing the files that we want to process
     *
     * will return a list containing all non empty lines of the file. Before
     * returning the list strip the 'new line' and 'line feed' from the line.
     *
     * @return stringlist containing all non empty lines of the file
     * @param file the filestream to tokenize
     */
    std::vector<std::string> tokenize(std::ifstream &file);

  private:
    /** reference to the ringbuffer */
    RingBuffer<CASSEvent,RingBufferSize>  &_ringbuffer;

    /** flag to quit the input */
    bool _quit;

    /** flag to tell the thread to quit when its done with all files */
    bool _quitWhenDone;

    /** name of the file containing all files that we need to process */
    std::string _filelistname;

    /** shared pointer to the actual reader */
    FileReader::shared_pointer _read;

    /** mutex for suspending the thread */
    QMutex _pauseMutex;

    /** condition to signal that we need to resume */
    QWaitCondition _pauseCondition;

    /** flag to tell to pause this thread */
    bool _pause;

    /** flag telling whether thread is suspended */
    bool _paused;

    /** flag to start over with the files */
    bool _rewind;

    /** condition to signal that the thread is suspended */
    QWaitCondition _waitUntilpausedCondition;
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
