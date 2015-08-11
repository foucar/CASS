/**
 * @file pausablethread.h declaration of a pausable QThread
 *
 * @author Lutz Foucar
 */

#ifndef PAUSABLETHREAD_H
#define PAUSABLETHREAD_H

#include <stdexcept>

#include <QtCore/QThread>
#include <QtCore/QMutex>
#include <QtCore/QWaitCondition>

namespace lmf
{
/** A QThread that has the ability to be paused and resumed
 *
 * This class inherits from QThread and enhances the QThread class to be able
 * to be paused and resumed
 *
 * @todo check the logic whether this is threadsafe (maybe need to add some
 *       more mutexlockers.)
 *
 * @author Lutz Foucar
 */
class PausableThread : public QThread
{
  Q_OBJECT
public:
  /** enum describing the internal status of the thread */
  enum status_t {running, paused, notstarted};

  /** enum describing the control status of the thread */
  enum control_t {_run, _quit, _pause};

  /** enum describing which exception was thrown */
  enum exception_t
  {
    NO_EXCEPTION,
    INVALID_ARGUMENT_EXCEPTION,
    RUNTIME_ERROR_EXCEPTION,
    OUT_OF_RANGE_EXCEPTION,
    LOGIC_ERROR_EXCEPTION,
    STANDART_EXCEPTION,
    UNKNOWN_EXCEPTION
  };

public:
  /** constructor
   *
   * You can construct this thread to be either paused or running when you
   * call the start() member of the created object.
   *
   * @param control the initial state of the thread
   * @param parent pointer to the parent object
   */
  PausableThread(control_t control=_run, QObject *parent = 0)
    : QThread(parent),
      _status(notstarted),
      _control(control),
      _pausecount((control==_run?0:1)),
      _exception_thrown(NO_EXCEPTION),
      _invarg_excep(""),
      _runt_excep(""),
      _outrange_excep(""),
      _logic_excep("")
  {}

  /** destructor
   *
   * stops the threads execution before deleting the thread. Make sure that
   * all waitconditions are properly shut down.
   */
  virtual ~PausableThread();

  /** run the thread
   *
   * this will call the pure virtual function runthis that needs to be
   * implemented with the code that the thread should execute.
   * It will handle all exceptions and ensures that they are not leaving the
   * thread. One can check after exectution of the thread which exceptions have
   * been thrown. Supported are
   * std::invalid_argument
   * std::runtime_error
   * std::out_of_range
   */
  void run();

  /** contains the code to run in the thread
   *
   * needs to be implemented by the derived threads
   */
  virtual void runthis()=0;

  /** pause the thread
   *
   * Will tell the thread to pause. The function can be told to block until
   * the thread is paused. It does that by waiting internaly on
   * waitUntilPaused() to return. If one does not want this function to block,
   * just call it with the default argument (false).
   *
   * @param block if true this function call will block until the thread is
   *        paused. If false this will return immidiatly (default).
   */
  void pause(bool block=false);

  /** waits until thread is paused
   *
   * Waits until the thread is paused by using the wait condition.
   */
  void waitUntilPaused();

  /** resume the thread
   *
   * Will tell the thread to resume by waking up the Condition
   */
  void resume();

  /** return the current status of the thread */
  status_t status()const   {return _status;}

  /** query whether this thread is told to quit
   *
   * @return true when it should quit, false otherwise
   */
  bool shouldQuit()const {return (_control == _quit);}

  /** rethrow the thrown exception
   *
   * this should only be called when the thread has quitted
   */
  void rethrowException()const;

  /** @return the exception thrown */
  exception_t exceptionThrown()const  {return _exception_thrown;}

public slots:
  /** tell the thread to quit */
  virtual void end() {_control = _quit;}

protected:
  /** point where the thread will be paused
   *
   * Call this function from within your run() at the point where you can
   * pause the thread whithout leaving it in an undefined state. It will check
   * whether the thread is requested to pause, if so it will pause the thread.
   */
  void pausePoint();

protected:
  /** mutex to wait on until thread is paused */
  QMutex _pauseMutex;

  /** wait condition to wait on until thread is resumed */
  QWaitCondition _pauseCondition;

  /** wait condition to wait unitl thread is paused */
  QWaitCondition _waitUntilPausedCondition;

  /** the internal status of the thread */
  status_t _status;

  /** the internal control status of the thread */
  control_t _control;

  /** a counter how many threads have pause this thread */
  size_t _pausecount;

  /** flag to show that general exception was thrown */
  exception_t _exception_thrown;

  /** the invalid arguemnt exception thrown */
  std::invalid_argument _invarg_excep;

  /** the invalid arguemnt exception thrown */
  std::runtime_error _runt_excep;

  /** the invalid arguemnt exception thrown */
  std::out_of_range _outrange_excep;

  /** the invalid arguemnt exception thrown */
  std::logic_error _logic_excep;
};

}//end namespace lmf
#endif // PAUSABLETHREAD_H
