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
#include "postprocessing/postprocessor.h"
#include "ratemeter.h"

using namespace cass;
using namespace std;

Worker::Worker(RingBuffer<CASSEvent,RingBufferSize> &ringbuffer,
               Ratemeter &ratemeter,
               string outputfilename,
               QObject *parent)
  :PausableThread(lmf::PausableThread::_run,parent),
    _ringbuffer(ringbuffer),
    _analyzer(Analyzer::instance()),
    _postprocessor(PostProcessors::instance(outputfilename)),
    _ratemeter(ratemeter)
{}

Worker::~Worker()
{
  _postprocessor->destroy();
  _analyzer->destroy();
}

void Worker::aboutToQuit()
{

  _postprocessor->aboutToQuit();
  _analyzer->aboutToQuit();
}

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
      _analyzer->processEvent(cassevent);
      _postprocessor->process(*cassevent);
      _ringbuffer.doneProcessing(cassevent);
      _ratemeter.count();
    }
  }
}

void Worker::loadSettings(size_t what)
{
  _postprocessor->loadSettings(what);
  _analyzer->loadSettings(what);
}

void Worker::saveSettings()
{
  _postprocessor->saveSettings();
  _analyzer->saveSettings();
}












//-----------------------the wrapper for more than 1 worker--------------------
Workers::Workers(RingBuffer<CASSEvent,RingBufferSize> &ringbuffer,
                 Ratemeter &ratemeter,
                 string outputfilename,
                 QObject */*parent*/)
  :_workers(NbrOfWorkers)
{
  for (size_t i=0;i<_workers.size();++i)
    _workers[i] = Worker::shared_pointer(new Worker(ringbuffer,ratemeter,outputfilename));
  _workers[0]->loadSettings(0);
}

Workers::~Workers()
{}

void Workers::loadSettings(size_t what)
{
  //make sure there is at least one worker//
  if(_workers.empty())
    throw bad_exception();
  QMutexLocker lock(&_mutex);
  for (size_t i=0;i<_workers.size();++i)
    _workers[i]->pause();
  for (size_t i=0;i<_workers.size();++i)
    _workers[i]->waitUntilPaused();
  _workers[0]->loadSettings(what);
  for (size_t i=0;i<_workers.size();++i)
    _workers[i]->resume();
}

void Workers::saveSettings()
{
  //make sure there is at least one worker//
  if(_workers.empty())
    throw bad_exception();
  QMutexLocker lock(&_mutex);
  for (size_t i=0;i<_workers.size();++i)
    _workers[i]->pause();
  for (size_t i=0;i<_workers.size();++i)
    _workers[i]->waitUntilPaused();
  _workers[0]->saveSettings();
  for (size_t i=0;i<_workers.size();++i)
    _workers[i]->resume();
}

void Workers::clearHistogram(PostProcessors::key_t key)
{
  if(_workers.empty())
    throw bad_exception();
  QMutexLocker lock(&_mutex);
  for (size_t i=0;i<_workers.size();++i)
    _workers[i]->pause();
  for (size_t i=0;i<_workers.size();++i)
    _workers[i]->waitUntilPaused();
  _workers[0]->clear(key);
  for (size_t i=0;i<_workers.size();++i)
    _workers[i]->resume();
}

void Workers::receiveCommand(PostProcessors::key_t key, string command)
{
  if(_workers.empty())
    throw bad_exception();
  QMutexLocker lock(&_mutex);
  for (size_t i=0;i<_workers.size();++i)
    _workers[i]->pause();
  for (size_t i=0;i<_workers.size();++i)
    _workers[i]->waitUntilPaused();
  _workers[0]->receiveCommand(key, command);
  for (size_t i=0;i<_workers.size();++i)
    _workers[i]->resume();
}

void Workers::start()
{
  for (size_t i=0;i<_workers.size();++i)
    _workers[i]->start();
}

void Workers::end()
{
  QMutexLocker locker(&_mutex);
  for (size_t i=0;i<_workers.size();++i)
    _workers[i]->end();
  for (size_t i=0;i<_workers.size();++i)
    _workers[i]->wait();
  _workers[0]->aboutToQuit();
  emit finished();
}



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
