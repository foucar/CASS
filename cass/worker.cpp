// Copyright (C) 2009, 2010 Lutz Foucar
// Copyright (C) 2010 Jochen Küpper

/**
 * @file worker.cpp file contains definition of class Worker and Workers
 *
 * @author Lutz Foucar
 */

#include <stdexcept>
#include <QtCore/QMutexLocker>

#include "worker.h"
#include "analyzer.h"
#include "format_converter.h"
#include "ratemeter.h"
#include "postprocessor.h"


using namespace cass;
using namespace std;

Worker::Worker(RingBuffer<CASSEvent,RingBufferSize> &ringbuffer,
               Ratemeter &ratemeter,
               QObject *parent)
  :PausableThread(lmf::PausableThread::_run,parent),
    _ringbuffer(ringbuffer),
    _preanalyze(*Analyzer::instance()),
    _postprocess(*PostProcessors::instance()),
    _ratemeter(ratemeter)
{}

void Worker::run()
{
  _status = lmf::PausableThread::running;
  while(_control != _quit)
  {
    pausePoint();
    CASSEvent *cassevent(0);
    _ringbuffer.nextToProcess(cassevent, 1000);
    if (cassevent)
    {
      _preanalyze(cassevent);
      _postprocess(*cassevent);
      _ringbuffer.doneProcessing(cassevent);
      _ratemeter.count();
    }
  }
}








// ============define static members (do not touch)==============
Workers::shared_pointer Workers::_instance;
QMutex Workers::_mutex;

Workers::shared_pointer Workers::instance(RingBuffer<CASSEvent,RingBufferSize> &ringbuffer,
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
Workers::Workers(RingBuffer<CASSEvent,RingBufferSize> &ringbuffer,
                 Ratemeter &ratemeter,
                 QObject *parent)
  :_workers(NbrOfWorkers)
{
  for (size_t i=0;i<_workers.size();++i)
    _workers[i] = Worker::shared_pointer(new Worker(ringbuffer,ratemeter,parent));
}

void Workers::start()
{
  QMutexLocker lock(&_mutex);
  for (size_t i=0;i<_workers.size();++i)
    _workers[i]->start();
}

void Workers::pause()
{
  QMutexLocker lock(&_mutex);
  for (size_t i=0;i<_workers.size();++i)
    _workers[i]->pause();
  for (size_t i=0;i<_workers.size();++i)
    _workers[i]->waitUntilPaused();
}

void Workers::resume()
{
  QMutexLocker lock(&_mutex);
  for (size_t i=0;i<_workers.size();++i)
    _workers[i]->resume();
}

void Workers::end()
{
  QMutexLocker lock(&_mutex);
  for (size_t i=0;i<_workers.size();++i)
    _workers[i]->end();
  for (size_t i=0;i<_workers.size();++i)
    _workers[i]->wait();
  PostProcessors::instance()->aboutToQuit();
  Analyzer::instance()->aboutToQuit();
  emit finished();
}



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
