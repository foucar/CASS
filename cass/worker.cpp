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
  :QThread(parent),
    _ringbuffer(ringbuffer),
    _analyzer(Analyzer::instance()),
    _postprocessor(PostProcessors::instance(outputfilename)),
    _quit(false),
    _pause(false),
    _paused(false),
    _ratemeter(ratemeter)
{}

Worker::~Worker()
{
  VERBOSEOUT(cout <<"worker "<<this<<" will be deleted"<<endl);
  _postprocessor->destroy();
  _analyzer->destroy();
  VERBOSEOUT(cout<< "worker "<<this<<" is deleted" <<endl);
}

void Worker::end()
{
  VERBOSEOUT(cout << "worker "<<this<<" is told to close"<<endl);
  _quit = true;
}

void Worker::aboutToQuit()
{
  _postprocessor->aboutToQuit();
  _analyzer->aboutToQuit();
}

void Worker::suspend()
{
  VERBOSEOUT(cout<<"Worker::"<<this<<": suspend(): signaled to suspend"<<endl);
  //set flag to pause//
  _pause=true;
}

void Worker::resume()
{
  VERBOSEOUT(cout<<"Worker::"<<this<<": resume(): I am signaled to resume."<<endl);
  //if the thread has not been paused return here//
  if(!_pause)
  {
    VERBOSEOUT(cout<<"Worker::"<<this<<": resume(): I am already running! Return"<<endl);
    return;
  }
  //reset the pause flag;
  _pause=false;
  VERBOSEOUT(cout<<"Worker::"<<this<<": resume(): telling myself that I need to resume"<<endl);
  //tell run to resume via the waitcondition//
  _pauseCondition.wakeOne();
}

void Worker::waitUntilSuspended()
{
  VERBOSEOUT(cout<<"Worker::"<<this<<": waitUntilSuspended(): check if I am suspended"<<endl);
  //if it is already paused then retrun imidiatly//
  if(_paused)
  {
    VERBOSEOUT(cout<<"Worker::"<<this<<": waitUntilSuspended(): I am already suspended. Returning"<<endl);
    return;
  }
  //otherwise wait until the conditions has been called//
  QMutex mutex;
  QMutexLocker lock(&mutex);
  VERBOSEOUT(cout<<"Worker::"<<this<<": waitUntilSuspended(): Not yet suspended. Wait until i am signaled that I am suspended"<<endl);
  _waitUntilpausedCondition.wait(&mutex);
  VERBOSEOUT(cout<<"Worker::"<<this<<": waitUntilSuspended(): Now I am suspended. Returning"<<endl);
}


void Worker::run()
{
  VERBOSEOUT(cout << "worker \""<<this <<"\" is starting"<<endl);
  //a pointer that we use//
  CASSEvent *cassevent=0;
  //run als long as we are told not to stop//
  while(!_quit)
  {
    //pause execution if suspend has been called//
    if (_pause)
    {
      VERBOSEOUT(cout<<"Worker::"<<this<<": run(): I should suspend."<<endl);
      //lock the mutex to prevent that more than one thread is calling pause//
      _pauseMutex.lock();
      //set the status flag to paused//
      _paused=true;
      //tell the wait until paused condtion that we are now pausing//
      VERBOSEOUT(cout<<"Worker::"<<this<<": run(): Tell suspend() that I am suspending."<<endl);
      _waitUntilpausedCondition.wakeOne();
      //wait until the condition is called again
      _pauseCondition.wait(&_pauseMutex);
      //set the status flag//
      _paused=false;
      //unlock the mutex, such that others can work again//
      _pauseMutex.unlock();
      VERBOSEOUT(cout<<"Worker::"<<this<<": run(): I am now running again"<<endl);
      //start over again//
      continue;
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
      _ratemeter.count();
    }
  }
  VERBOSEOUT(cout <<"worker "<<this<<" is closing down"<<endl);
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
  :_workers(NbrOfWorkers,0)
{
  //create the worker instances//
  //connect all workers output to this output//
  for (size_t i=0;i<_workers.size();++i)
  {
    _workers[i] = new Worker(ringbuffer,ratemeter,outputfilename);
    connect(_workers[i],SIGNAL(processedEvent()),this,SIGNAL(processedEvent()));
  }
  //load the settings for one worker, which will load the settings for all other
  //worker, since they are singletons//
  _workers[0]->loadSettings(0);
}

Workers::~Workers()
{
  VERBOSEOUT(cout <<"workers are beeing deleted"<<endl);
  //delete the worker instances//
  for (size_t i=0;i<_workers.size();++i)
    delete _workers[i];
  VERBOSEOUT(cout<< "workers are deleted" <<endl);
}

void Workers::loadSettings(size_t what)
{
  //make sure there is at least one worker//
  if(_workers.empty())
    throw bad_exception();
  //lock this from here on, so that it is reentrant
  QMutexLocker lock(&_mutex);
  VERBOSEOUT(cout << "Workers: Load Settings: suspend all workers before loading settings"
      <<endl);
  //suspend all workers//
  for (size_t i=0;i<_workers.size();++i)
    _workers[i]->suspend();
  for (size_t i=0;i<_workers.size();++i)
    _workers[i]->waitUntilSuspended();
  VERBOSEOUT(cout << "Workers: Load Settings: Workers are suspend. Load Settings of one Worker,"
      <<" which shares all settings."<<endl);
  //load the settings of one worker//
  //since the workers have only singletons this will make sure//
  //that the parameters are the same for all workers//
  _workers[0]->loadSettings(what);
  //resume the workers tasks//
  VERBOSEOUT(cout << "Workers: Load Settings: Done Loading. Now resume all workers"
      <<endl);
  for (size_t i=0;i<_workers.size();++i)
    _workers[i]->resume();
  VERBOSEOUT(cout << "Workers: Load Settings: Workers are resumed"
      <<endl);
}

void Workers::saveSettings()
{
  //make sure there is at least one worker//
  if(_workers.empty())
    throw bad_exception();
  //lock this from here on, so that it is reentrant
  QMutexLocker lock(&_mutex);
  VERBOSEOUT(cout << "Workers: Save Settings: suspend all workers before saving settings"
      <<endl);
  //suspend all workers//
  for (size_t i=0;i<_workers.size();++i)
    _workers[i]->suspend();
  for (size_t i=0;i<_workers.size();++i)
    _workers[i]->waitUntilSuspended();
  VERBOSEOUT(cout << "Workers: Load Settings: Workers are suspend. Save Settings of one Worker,"
      <<" which shares all settings."<<endl);
  //load the settings of one worker//
  //since the workers have only singletons this will make sure//
  //that the parameters are the same for all workers//
  _workers[0]->saveSettings();
  //resume the workers tasks//
  VERBOSEOUT(cout << "Workers: Save Settings: Done Saving. Now resume all workers"
      <<endl);
  for (size_t i=0;i<_workers.size();++i)
    _workers[i]->resume();
  VERBOSEOUT(cout << "Workers: Load Settings: Workers are resumed"
      <<endl);
}

void Workers::clearHistogram(PostProcessors::key_t key)
{
  //make sure there is at least one worker//
  if(_workers.empty())
    throw bad_exception();
  //lock this from here on, so that it is reentrant
  QMutexLocker lock(&_mutex);
  VERBOSEOUT(cout << "Workers: Clear: suspend all workers before clearing histogram"
      <<endl);
  //suspend all workers//
  for (size_t i=0;i<_workers.size();++i)
    _workers[i]->suspend();
  for (size_t i=0;i<_workers.size();++i)
    _workers[i]->waitUntilSuspended();
  VERBOSEOUT(cout << "Workers: Clear: Workers are suspend."
      <<" Clear the requested histogram."<<endl);
  //clear histogram of one worker//
  //since the workers have only singletons this will make sure//
  //that the parameters are the same for all workers//
  _workers[0]->clear(key);
  //resume the workers tasks//
  VERBOSEOUT(cout << "Workers: Clear: Done clearing. Now resume all workers"
      <<endl);
  for (size_t i=0;i<_workers.size();++i)
    _workers[i]->resume();
  VERBOSEOUT(cout << "Workers: Clear: Workers are resumed"
      <<endl);
}

void Workers::receiveCommand(PostProcessors::key_t key, string command)
{
  //make sure there is at least one worker//
  if(_workers.empty())
    throw bad_exception();
  //lock this from here on, so that it is reentrant
  QMutexLocker lock(&_mutex);
  VERBOSEOUT(cout << "Workers: receiveCommand: suspend all workers before processing command."
      <<endl);
  //suspend all workers//
  for (size_t i=0;i<_workers.size();++i)
    _workers[i]->suspend();
  for (size_t i=0;i<_workers.size();++i)
    _workers[i]->waitUntilSuspended();
  VERBOSEOUT(cout << "Workers: receiveCommand: Workers are suspend."
      <<" process command in requested histogram."<<endl);
  //process command of one worker//
  //since the workers have only singletons this will make sure//
  //that the parameters are the same for all workers//
  _workers[0]->receiveCommand(key, command);
  //resume the workers tasks//
  VERBOSEOUT(cout << "Workers: receiveCommand: Done. Now resume all workers"
      <<endl);
  for (size_t i=0;i<_workers.size();++i)
    _workers[i]->resume();
  VERBOSEOUT(cout << "Workers: receiveCommand: Workers are resumed"
      <<endl);
}

void Workers::start()
{
  //start all workers//
  for (size_t i=0;i<_workers.size();++i)
    _workers[i]->start();
}


void Workers::end()
{
  QMutexLocker locker(&_mutex);
  VERBOSEOUT(cout << "got signal to close the workers"<<endl);
  //tell all workers that they should quit//
  for (size_t i=0;i<_workers.size();++i)
    _workers[i]->end();
  //resume the threads when they have been suspended//
  for (size_t i=0;i<_workers.size();++i)
    _workers[i]->resume();
  //wait until we run has really finished//
  for (size_t i=0;i<_workers.size();++i)
    _workers[i]->wait();
  //tell one worker that we are about to quit//
  _workers[0]->aboutToQuit();
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
