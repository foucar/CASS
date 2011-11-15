/**
 * @file pausablethread.cpp definition of a pausable QThread
 *
 * @author Lutz Foucar
 */

#include <iostream>
#include <stdexcept>

#include "pausablethread.h"

using namespace lmf;
using namespace std;

PausableThread::~PausableThread()
{
  if(isRunning())
  {
    terminate();
  }
  wait();
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
