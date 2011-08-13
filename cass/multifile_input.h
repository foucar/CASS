// Copyright (C) 2011 Lutz Foucar

/**
 * @file multifile_input.h file contains declaration of file input reading
 *                         multiple files in parallel.
 *
 * @author Lutz Foucar
 */

#ifndef _MULTIFILEINPUT_H_
#define _MULTIFILEINPUT_H_

#include <QtCore/QObject>
#include <QThread>
#include <QMutex>

#include <string>

#include "cass.h"
#include "ringbuffer.h"
#include "cass_event.h"

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
   * @author Lutz Foucar
   */
  class CASSSHARED_EXPORT MultiFileInput : public QThread
  {
    Q_OBJECT;
  public:
    /** constructor
     *
     * @param filelistname name of the file containing all files that should be
     *                     processed
     * @param ringbuffer reference to the ringbuffer containing the CASSEvents
     * @param quitwhendone flag that tells this class that it should quit the
     *                     Program when its done reading all events
     * @param parent The parent QT Object of this class
     */
    MultiFileInput(const std::string& filelistname,
                   RingBuffer<CASSEvent,RingBufferSize>& ringbuffer,
                   bool quitwhendone,
                   QObject *parent=0);

    /** destructor */
    ~MultiFileInput();

    /** function with the main loop */
    void run();

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

  private:
    /** reference to the ringbuffer */
    RingBuffer<CASSEvent,RingBufferSize>  &_ringbuffer;

    /** flag to quit the input */
    bool _quit;

    /** flag to tell the thread to quit when its done with all files */
    bool _quitWhenDone;

    /** name of the file containing all files that we need to process */
    std::string _filelistname;
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
