// Copyright (C) 2009, 2010 Lutz Foucar
// Copyright (C) 2010 Jochen Küpper

#include <exception>
#include <QtCore/QMutexLocker>

#include "worker.h"
#include "analyzer.h"
#include "format_converter.h"
#include "postprocessing/postprocessor.h"

cass::Worker::Worker(cass::RingBuffer<cass::CASSEvent,cass::RingBufferSize> &ringbuffer,
                     std::string outputfilename,
                     QObject *parent)
  :QThread(parent),
    _ringbuffer(ringbuffer),
    _analyzer(cass::Analyzer::instance()),
    _postprocessor(cass::PostProcessors::instance(outputfilename)),
    _quit(false),
    _pause(false),
    _paused(false)
{}

cass::Worker::~Worker()
{
  VERBOSEOUT(std::cout <<"worker "<<this<<" will be deleted"<<std::endl);
  _postprocessor->destroy();
  _analyzer->destroy();
  VERBOSEOUT(std::cout<< "worker "<<this<<" is deleted" <<std::endl);
}

void cass::Worker::end()
{
  VERBOSEOUT(std::cout << "worker "<<this<<" is told to close"<<std::endl);
  //tell run that we want to close
  _quit = true;
}

void cass::Worker::suspend()
{
  VERBOSEOUT(std::cout<<"Worker::"<<this<<": suspend(): signaled to suspend"<<std::endl);
  //set flag to pause//
  _pause=true;
}

void cass::Worker::resume()
{
  VERBOSEOUT(std::cout<<"Worker::"<<this<<": resume(): I am signaled to resume."<<std::endl);
  //if the thread has not been paused return here//
  if(!_pause)
  {
    VERBOSEOUT(std::cout<<"Worker::"<<this<<": resume(): I am already running! Return"<<std::endl);
    return;
  }
  //reset the pause flag;
  _pause=false;
  VERBOSEOUT(std::cout<<"Worker::"<<this<<": resume(): telling myself that I need to resume"<<std::endl);
  //tell run to resume via the waitcondition//
  _pauseCondition.wakeOne();
}

void cass::Worker::waitUntilSuspended()
{
  VERBOSEOUT(std::cout<<"Worker::"<<this<<": waitUntilSuspended(): check if I am suspended"<<std::endl);
  //if it is already paused then retrun imidiatly//
  if(_paused)
  {
    VERBOSEOUT(std::cout<<"Worker::"<<this<<": waitUntilSuspended(): I am already suspended. Returning"<<std::endl);
    return;
  }
  //otherwise wait until the conditions has been called//
  QMutex mutex;
  QMutexLocker lock(&mutex);
  VERBOSEOUT(std::cout<<"Worker::"<<this<<": waitUntilSuspended(): Not yet suspended. Wait until i am signaled that I am suspended"<<std::endl);
  _waitUntilpausedCondition.wait(&mutex);
  VERBOSEOUT(std::cout<<"Worker::"<<this<<": waitUntilSuspended(): Now I am suspended. Returning"<<std::endl);
}


void cass::Worker::run()
{
  VERBOSEOUT(std::cout << "worker \""<<this <<"\" is starting"<<std::endl);
  //a pointer that we use//
  cass::CASSEvent *cassevent=0;
  //run als long as we are told not to stop//
  while(!_quit)
  {
    //pause execution if suspend has been called//
    if (_pause)
    {
      VERBOSEOUT(std::cout<<"Worker::"<<this<<": run(): I should suspend."<<std::endl);
      //lock the mutex to prevent that more than one thread is calling pause//
      _pauseMutex.lock();
      //set the status flag to paused//
      _paused=true;
      //tell the wait until paused condtion that we are now pausing//
      VERBOSEOUT(std::cout<<"Worker::"<<this<<": run(): Tell suspend() that I am suspending."<<std::endl);
      _waitUntilpausedCondition.wakeOne();
      //wait until the condition is called again
      _pauseCondition.wait(&_pauseMutex);
      //set the status flag//
      _paused=false;
      //unlock the mutex, such that others can work again//
      _pauseMutex.unlock();
      VERBOSEOUT(std::cout<<"Worker::"<<this<<": run(): I am now running again"<<std::endl);
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
  VERBOSEOUT(std::cout <<"worker "<<this<<" is closing down"<<std::endl);
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
                       std::string outputfilename,
                       QObject */*parent*/)
                         :_workers(cass::NbrOfWorkers,0)
{
  //create the worker instances//
  //connect all workers output to this output//
  for (size_t i=0;i<_workers.size();++i)
  {
    _workers[i] = new cass::Worker(ringbuffer,outputfilename);
    connect(_workers[i],SIGNAL(processedEvent()),this,SIGNAL(processedEvent()));
  }
  //load the settings for one worker, which will load the settings for all other
  //worker, since they are singletons//
  _workers[0]->loadSettings(0);
}

cass::Workers::~Workers()
{
  VERBOSEOUT(std::cout <<"workers are beeing deleted"<<std::endl);
  //delete the worker instances//
  for (size_t i=0;i<_workers.size();++i)
    delete _workers[i];
  VERBOSEOUT(std::cout<< "workers are deleted" <<std::endl);
}

void cass::Workers::loadSettings(size_t what)
{
  //make sure there is at least one worker//
  if(_workers.empty())
    throw std::bad_exception();
  //lock this from here on, so that it is reentrant
  QMutexLocker lock(&_mutex);
  VERBOSEOUT(std::cout << "Workers: Load Settings: suspend all workers before loading settings"
      <<std::endl);
  //suspend all workers//
  for (size_t i=0;i<_workers.size();++i)
    _workers[i]->suspend();
  for (size_t i=0;i<_workers.size();++i)
    _workers[i]->waitUntilSuspended();
  VERBOSEOUT(std::cout << "Workers: Load Settings: Workers are suspend. Load Settings of one Worker,"
      <<" which shares all settings."<<std::endl);
  //load the settings of one worker//
  //since the workers have only singletons this will make sure//
  //that the parameters are the same for all workers//
  _workers[0]->loadSettings(what);
  //resume the workers tasks//
  VERBOSEOUT(std::cout << "Workers: Load Settings: Done Loading. Now resume all workers"
      <<std::endl);
  for (size_t i=0;i<_workers.size();++i)
    _workers[i]->resume();
  VERBOSEOUT(std::cout << "Workers: Load Settings: Workers are resumed"
      <<std::endl);
}

void cass::Workers::saveSettings()
{
  //make sure there is at least one worker//
  if(_workers.empty())
    throw std::bad_exception();
  //lock this from here on, so that it is reentrant
  QMutexLocker lock(&_mutex);
  VERBOSEOUT(std::cout << "Workers: Save Settings: suspend all workers before saving settings"
      <<std::endl);
  //suspend all workers//
  for (size_t i=0;i<_workers.size();++i)
    _workers[i]->suspend();
  for (size_t i=0;i<_workers.size();++i)
    _workers[i]->waitUntilSuspended();
  VERBOSEOUT(std::cout << "Workers: Load Settings: Workers are suspend. Save Settings of one Worker,"
      <<" which shares all settings."<<std::endl);
  //load the settings of one worker//
  //since the workers have only singletons this will make sure//
  //that the parameters are the same for all workers//
  _workers[0]->saveSettings();
  //resume the workers tasks//
  VERBOSEOUT(std::cout << "Workers: Save Settings: Done Saving. Now resume all workers"
      <<std::endl);
  for (size_t i=0;i<_workers.size();++i)
    _workers[i]->resume();
  VERBOSEOUT(std::cout << "Workers: Load Settings: Workers are resumed"
      <<std::endl);
}

void cass::Workers::clearHistogram(PostProcessors::key_t key)
{
  //make sure there is at least one worker//
  if(_workers.empty())
    throw std::bad_exception();
  //lock this from here on, so that it is reentrant
  QMutexLocker lock(&_mutex);
  VERBOSEOUT(std::cout << "Workers: Clear: suspend all workers before loading settings"
      <<std::endl);
  //suspend all workers//
  for (size_t i=0;i<_workers.size();++i)
    _workers[i]->suspend();
  for (size_t i=0;i<_workers.size();++i)
    _workers[i]->waitUntilSuspended();
  VERBOSEOUT(std::cout << "Workers: Clear: Workers are suspend."
      <<" Clear the requested histogram."<<std::endl);
  //load the settings of one worker//
  //since the workers have only singletons this will make sure//
  //that the parameters are the same for all workers//
  _workers[0]->clear(key);
  //resume the workers tasks//
  VERBOSEOUT(std::cout << "Workers: Clear: Done Loading. Now resume all workers"
      <<std::endl);
  for (size_t i=0;i<_workers.size();++i)
    _workers[i]->resume();
  VERBOSEOUT(std::cout << "Workers: Clear: Workers are resumed"
      <<std::endl);
}

void cass::Workers::start()
{
  //start all workers//
  for (size_t i=0;i<_workers.size();++i)
    _workers[i]->start();
}


void cass::Workers::end()
{
  VERBOSEOUT(std::cout << "got signal to close the workers"<<std::endl);
  //tell all workers that they should quit//
  for (size_t i=0;i<_workers.size();++i)
    _workers[i]->end();
  //wait until we run has really finished
  for (size_t i=0;i<_workers.size();++i)
  _workers[i]->wait();
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
