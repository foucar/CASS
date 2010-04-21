// Copyright (C) 2009, 2010 lmf
// Copyright (C) 2010 Jochen Küpper

#include <exception>
#include <QtCore/QMutexLocker>

#include "worker.h"
#include "analyzer.h"
#include "format_converter.h"
#include "postprocessing/postprocessor.h"

cass::Worker::Worker(cass::RingBuffer<cass::CASSEvent,cass::RingBufferSize> &ringbuffer, QObject *parent)
  :QThread(parent),
    _ringbuffer(ringbuffer),
    _analyzer(cass::Analyzer::instance()),
    _postprocessor(cass::PostProcessors::instance()),
    _quit(false),
    _pause(false),
    _paused(false)
{
}

cass::Worker::~Worker()
{
  std::cout <<"worker "<<this<<" will be deleted"<<std::endl;
  _postprocessor->destroy();
  _analyzer->destroy();
  std::cout<< "worker "<<this<<" is deleted" <<std::endl;
}

void cass::Worker::end()
{
  std::cout << "worker "<<this<<" is told to close"<<std::endl;
  //tell run that we want to close
  _quit = true;
  //wait until we run has really finished
  wait();
}

void cass::Worker::suspend()
{
  //set flag to pause//
  _pause=true;
  //wait until it is suspended//
  waitUntilSuspended();
}

void cass::Worker::resume()
{
  //if the thread has not been paused return here//
  if(!_pause)
    return;
  //reset the pause flag;
  _pause=false;
  //tell run to resume via the waitcondition//
  _pauseCondition.wakeOne();
}

void cass::Worker::waitUntilSuspended()
{
  //if it is already paused then retrun imidiatly//
  if(_paused)
    return;
  //otherwise wait until the conditions has been called//
  QMutex mutex;
  QMutexLocker lock(&mutex);
  _waitUntilpausedCondition.wait(&mutex);
}

const cass::PostProcessors::histograms_t& cass::Worker::histograms()const
{
  return _postprocessor->histograms();
}

void cass::Worker::run()
{
  std::cout << "worker \""<<std::hex<<this<<std::dec <<"\" is starting"<<std::endl;
  //a pointer that we use//
  cass::CASSEvent *cassevent=0;
  //run als long as we are told not to stop//
  while(!_quit)
  {
    //pause execution if suspend has been called//
    if (_pause)
    {
      //lock the mutex to prevent that more than one thread is calling pause//
      _pauseMutex.lock();
      //set the status flag to paused//
      _paused=true;
      //tell the wait until paused condtion that we are now pausing//
      _waitUntilpausedCondition.wakeOne();
      //wait until the condition is called again
      _pauseCondition.wait(&_pauseMutex);
      //set the status flag//
      _paused=false;
      //unlock the mutex, such that others can work again//
      _pauseMutex.unlock();
    }

    //reset the cassevent//
    cassevent=0;
    //retrieve a new cassevent from the eventbuffer//
    _ringbuffer.nextToProcess(cassevent, 1000);

    //when the cassevent has been set work on it//
    if (cassevent)
    {
      //analyze the cassevent//
      _analyzer->processEvent(cassevent);

      //here the usercode that will work on the cassevent will be called//
      _postprocessor->process(*cassevent);

      //we are done, so tell the ringbuffer//
      _ringbuffer.doneProcessing(cassevent);

      //tell outside that we are done, when we should have anlyzed the event//
      emit processedEvent();
    }
  }
  std::cout <<"worker "<<this<<" is closing down"<<std::endl;
}

void cass::Worker::loadSettings(size_t what)
{
  _postprocessor->loadSettings(what);
  _analyzer->loadSettings(what);
}

void cass::Worker::saveSettings()
{
  _postprocessor->saveSettings();
  _analyzer->saveSettings();
}



//-----------------------the wrapper for more than 1 worker--------------------
cass::Workers::Workers(cass::RingBuffer<cass::CASSEvent,cass::RingBufferSize> &ringbuffer,
                       QObject *parent)
                         :_workers(cass::NbrOfWorkers,0)
{
  //create the worker instances//
  //connect all workers output to this output//
  for (size_t i=0;i<_workers.size();++i)
  {
    _workers[i] = new cass::Worker(ringbuffer, parent);
    connect(_workers[i],SIGNAL(processedEvent()),this,SIGNAL(processedEvent()));
  }
}

cass::Workers::~Workers()
{
  std::cout <<"workers are beeing deleted"<<std::endl;
  //delete the worker instances//
  for (size_t i=0;i<_workers.size();++i)
    delete _workers[i];
  std::cout<< "workers are deleted" <<std::endl;
}

const cass::PostProcessors::histograms_t& cass::Workers::histograms() const
{
    if(_workers.empty())
        throw std::bad_exception();
    return _workers[0]->histograms();
}

void cass::Workers::loadSettings(size_t what)
{
  std::cout << "Workers: Load Settings: suspend all workers before laoding settings"
      <<std::endl;
  //suspend all workers//
  for (size_t i=0;i<_workers.size();++i)
    _workers[i]->suspend();
  std::cout << "Workers: Load Settings: Workers are suspend. Load Settings of one Worker,"
      <<" which shares all settings."<<std::endl;
  //load the settings of one worker//
  //since the workers have only singletons this will make sure//
  //that the parameters are the same for all workers//
  _workers[0]->loadSettings(what);
  //resume the workers tasks//
  std::cout << "Workers: Load Settings: Done Loading. Now resume all workers"
      <<std::endl;
  for (size_t i=0;i<_workers.size();++i)
    _workers[i]->resume();
  std::cout << "Workers: Load Settings: Workers are resumed"
      <<std::endl;
}

void cass::Workers::start()
{
  //start all workers//
  for (size_t i=0;i<_workers.size();++i)
    _workers[i]->start();
}


void cass::Workers::end()
{
  std::cout << "got signal to close the workers"<<std::endl;
  //tell all workers that they should quit//
  for (size_t i=0;i<_workers.size();++i)
    _workers[i]->end();
  //emit that all workers are finished//
  emit finished();
}



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
