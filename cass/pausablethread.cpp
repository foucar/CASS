/**
 * @file pausablethread.cpp definition of a pausable QThread
 *
 * @author Lutz Foucar
 */

#include <iostream>

#include "pausablethread.h"

#include "log.h"

using namespace lmf;
using namespace std;
using namespace cass;

PausableThread::~PausableThread()
{
  if(isRunning())
  {
    terminate();
  }
  wait();
}

void PausableThread::run()
{
  try
  {
    runthis();
  }
  catch (const invalid_argument &error)
  {
    Log::add(Log::DEBUG4,string("PausableThread::run(): catch invalid argument exception '") +
             error.what() + "'");
    _exception_thrown = INVALID_ARGUMENT_EXCEPTION;
    _invarg_excep = error;
  }
  catch (const runtime_error &error)
  {
    Log::add(Log::DEBUG4,string("PausableThread::run(): catch runtime erro exception '") +
             error.what() + "'");
    _exception_thrown = RUNTIME_ERROR_EXCEPTION;
    _runt_excep = error;
  }
  catch (const out_of_range &error)
  {
    Log::add(Log::DEBUG4,string("PausableThread::run(): catch out of range exception '") +
             error.what() + "'");
    _exception_thrown = OUT_OF_RANGE_EXCEPTION;
    _outrange_excep = error;
  }
  catch (...)
  {
    Log::add(Log::DEBUG4,string("PausableThread::run(): catch unknown exception '"));
    _exception_thrown = UNKNOWN_EXCEPTION;
  }
}

void PausableThread::pause(bool wait)
{
  if (_control == _pause)
    throw runtime_error("PausableThread::pause(): Thread is already told to pause");
  _control = _pause;
  ++_pausecount;
  if (_status != notstarted && wait)
    waitUntilPaused();
}

void PausableThread::waitUntilPaused()
{
  QMutex mutex;
  QMutexLocker lock(&mutex);
  if (_control != _pause)
    throw runtime_error("PausableThread::waitUntilPaused(): Threat is not told to be paused");
  if(_status == paused)
    return;
    _waitUntilPausedCondition.wait(&mutex);
}

void PausableThread::resume()
{
  QMutexLocker lock(&_pauseMutex);
  if (_control == _run)
    throw runtime_error("PausableThread::resume(): Thread is already told to resume");
  if(_status == running)
    throw runtime_error("PausableThread::resume(): Thread is already running");
  --_pausecount;
  if (_pausecount == 0)
  {
    _control = _run;
    _pauseCondition.wakeAll();
  }
}

void PausableThread::pausePoint()
{
  if (_control == _pause)
  {
    QMutexLocker lock(&_pauseMutex);
    _status = paused;
    _waitUntilPausedCondition.wakeOne();
    _pauseCondition.wait(&_pauseMutex);
    _status = running;
  }
}

void PausableThread::rethrowException()const
{
  switch (_exception_thrown)
  {
  case INVALID_ARGUMENT_EXCEPTION:
    throw _invarg_excep;
    break;
  case lmf::PausableThread::RUNTIME_ERROR_EXCEPTION:
    throw _runt_excep;
    break;
  case lmf::PausableThread::OUT_OF_RANGE_EXCEPTION:
    throw _outrange_excep;
    break;
  case lmf::PausableThread::UNKNOWN_EXCEPTION:
    throw 0;
    break;
  case lmf::PausableThread::NO_EXCEPTION:
  default:
    break;
  }
}
