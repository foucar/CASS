// Copyright (C) 2009, 2010 Lutz Foucar

#ifndef _FILEINPUT_H_
#define _FILEINPUT_H_

#include <QtCore/QObject>
#include <QThread>
#include <QMutex>

#include <string>

#include "cass.h"
#include "ringbuffer.h"
#include "cass_event.h"

namespace cass
{
  //forward declaration//
  class FormatConverter;

  /** File Input for cass
   *
   * This class will be used in offline modus. I will take an string that
   * contains a filename. In the file has to be a list with files, that one
   * wants to analyze. The filenames name  can be passed to the program with
   * the -i parameter.
   *
   * For each file in the filelist it will iterate through the datagrams and
   * does the same thing that the shared memory input does with the datagrams:
   * - call the user selected converters
   * - if the iteration through the datagram was sucessfull put into the
   *   ringbuffer marked to be analyzed.
   *
   * @author Lutz Foucar
   */
  class CASSSHARED_EXPORT FileInput : public QThread
  {
    Q_OBJECT;
  public:
    /** constructor */
    FileInput(std::string filelistname,
              cass::RingBuffer<cass::CASSEvent,cass::RingBufferSize>&,
              QObject *parent=0);

    /** destructor */
    ~FileInput();

    /** function with the main loop */
    void run();

    /** suspends the thread.
     * Suspends the thread after it has executed the event we are working on
     * right now. Will return only when the thread has really suspenden by calling
     * @see waitUntilSuspended() internally.
     */
    void suspend();

    /** resumes the thread, when it was suspended. Otherwise it just retruns*/
    void resume();

  public slots:
    /** slot to quit the input */
    void end();

    /** load the parameters used for this thread*/
    void loadSettings(size_t what);

  signals:
    /** signal to indicate that we are done processing an event.
     * this is used for by the ratemeter to evaluate how fast we get events.
     */
    void newEventAdded();

  protected:
    /** function that will wait until we really suspended.
     * will be called by suspend, so that it returns only when thread has really
     * suspended.
     */
    void waitUntilSuspended();

  private:
    /** reference to the ringbuffer */
    cass::RingBuffer<cass::CASSEvent,cass::RingBufferSize>  &_ringbuffer;

    /** flag to quit the input */
    bool _quit;

    /** name of the file containing all files that we need to process */
    std::string _filelistname;

    /** a pointer to the format converter.
     * The converter will convert the incomming data to our CASSEvent
     */
    FormatConverter *_converter;

    /** a mutex for suspending the thread*/
    QMutex _pauseMutex;

    /** a condition that we will wait on until we are not suspended anymore*/
    QWaitCondition _pauseCondition;

    /** flag telling whether we shouodl suspend ourselves*/
    bool _pause;

    /** flag telling whether we are already suspended*/
    bool _paused;

    /** condition that will wait until the thread is rally suspended*/
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
