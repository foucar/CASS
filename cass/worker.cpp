// Copyright (C) 2009, 2010,2013 Lutz Foucar
// Copyright (C) 2010 Jochen Küpper

/**
 * @file worker.cpp file contains definition of class Worker and Workers
 *
 * @author Lutz Foucar
 */

#include <stdexcept>
#include <QtCore/QMutexLocker>

#include "worker.h"
#include "format_converter.h"
#include "ratemeter.h"
#include "processor_manager.h"
#include "log.h"


using namespace cass;
using namespace std;

Worker::Worker(RingBuffer<CASSEvent> &ringbuffer,
               Ratemeter &ratemeter,
               QObject *parent)
  : PausableThread(lmf::PausableThread::_run,parent),
    _ringbuffer(ringbuffer),
    _postprocess(*PostProcessors::instance()),
    _ratemeter(ratemeter)
{}

void Worker::runthis()
{
  _status = lmf::PausableThread::running;
  RingBuffer<CASSEvent>::iter_type rbItem;
  while(_control != _quit)
  {
    pausePoint();
    if ((rbItem = _ringbuffer.nextToProcess(1000)) != _ringbuffer.end())
    {
      _postprocess(*rbItem->element);
      _ringbuffer.doneProcessing(rbItem);
      _ratemeter.count();
    }
  }
}








// ============define static members (do not touch)==============
Workers::shared_pointer Workers::_instance;
QMutex Workers::_mutex;

Workers::shared_pointer Workers::instance(RingBuffer<CASSEvent> &ringbuffer,
                                          Ratemeter &ratemeter,
                                          QObject *parent)
{
  QMutexLocker lock(&_mutex);
  if(!_instance)
    _instance = Workers::shared_pointer(new Workers(ringbuffer, ratemeter, parent));
  return _instance;
}

Workers::shared_pointer::element_type& Workers::reference()
{
  QMutexLocker lock(&_mutex);
  if (!_instance)
    throw logic_error("Workers::reference(): The instance has not yet been created");
  return *_instance;
}
//===============================================================


//-----------------------the wrapper for more than 1 worker--------------------
Workers::Workers(RingBuffer<CASSEvent> &ringbuffer,
                 Ratemeter &ratemeter,
                 QObject *parent)
  : _workers(NbrOfWorkers),
    _rb(ringbuffer)
{
  for (size_t i=0;i<_workers.size();++i)
    _workers[i] = Worker::shared_pointer(new Worker(ringbuffer,ratemeter,parent));
}

void Workers::start()
{
  for (size_t i=0;i<_workers.size();++i)
    _workers[i]->start();
}

void Workers::pause()
{
  for (size_t i=0;i<_workers.size();++i)
    _workers[i]->pause();
  for (size_t i=0;i<_workers.size();++i)
    _workers[i]->waitUntilPaused();
}

void Workers::resume()
{
  for (size_t i=0;i<_workers.size();++i)
    _workers[i]->resume();
}

void Workers::end()
{
  /** wait until the ringbuffer is empty */
  _rb.waitUntilEmpty();
  for (size_t i=0;i<_workers.size();++i)
    _workers[i]->end();
  for (size_t i=0;i<_workers.size();++i)
    _workers[i]->wait();
  PostProcessors::instance()->aboutToQuit();

  /** rethrow the exception thrown by the workers */
  for (size_t i=0;i<_workers.size();++i)
    _workers[i]->rethrowException();
}

bool Workers::running()const
{
  bool running(true);
  for (size_t i=0;i<_workers.size();++i)
    running = running && _workers[i]->isRunning();
  return running;
}
