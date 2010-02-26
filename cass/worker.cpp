#include "worker.h"
#include "analyzer.h"
#include "format_converter.h"
#include "post_processor.h"

cass::Worker::Worker(lmf::RingBuffer<cass::CASSEvent,cass::RingBufferSize> &ringbuffer,const char* OutputFileName, QObject *parent)
  :QThread(parent),
    _ringbuffer(ringbuffer),
    _analyzer(cass::Analyzer::instance()),
    _postprocessor(cass::PostProcessor::instance(OutputFileName)),
    _quit(false)
{
}

cass::Worker::~Worker()
{
  std::cout <<"worker "<<this<<" is closing"<<std::endl;
  _postprocessor->destroy();
  _analyzer->destroy();
  std::cout<< "worker "<<this<<" is closed" <<std::endl;
}

void cass::Worker::end()
{
  std::cout << "worker "<<this<<" got signal to close"<<std::endl;
  _quit = true;
}

void cass::Worker::run()
{
  std::cout << "worker \""<<std::hex<<this<<std::dec <<"\" is starting"<<std::endl;
  //a pointer that we use//
  cass::CASSEvent *cassevent=0;
  //run als long as we are told not to stop//
  while(!_quit)
  {
    //retrieve a new cassevent from the eventbuffer//
    _ringbuffer.nextToProcess(cassevent, 1000);

    //when the cassevent has been set work on it//
    if (cassevent)
    {
      //analyze the cassevent//
      _analyzer->processEvent(cassevent);

      //here the usercode that will work on the cassevent will be called//
      _postprocessor->postProcess(*cassevent);

      //we are done, so tell the ringbuffer//
      _ringbuffer.doneProcessing(cassevent);

      //tell outside that we are done, when we should have anlyzed the event//
      emit processedEvent();
    }
  }
  std::cout <<"worker "<<this<<" is closing down"<<std::endl;
}

void cass::Worker::loadSettings()
{
  _postprocessor->loadSettings();
  _analyzer->loadSettings();
}

void cass::Worker::saveSettings()
{
  _postprocessor->saveSettings();
  _analyzer->saveSettings();
}



//-----------------------the wrapper for more than 1 worker--------------------
cass::Workers::Workers(lmf::RingBuffer<cass::CASSEvent,cass::RingBufferSize> &ringbuffer, const char* OutputFileName, QObject *parent)
    :_workers(cass::NbrOfWorkers,0)
{
  //create the worker instances//
  for (size_t i=0;i<_workers.size();++i)
    _workers[i] = new cass::Worker(ringbuffer,OutputFileName, parent);
}

cass::Workers::~Workers()
{
  std::cout <<"workers are closing"<<std::endl;
  //delete the worker instances//
  for (size_t i=0;i<_workers.size();++i)
    delete _workers[i];
  std::cout<< "workers are closed" <<std::endl;
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
  //now wait until all have finished//
  for (size_t i=0;i<_workers.size();++i)
    _workers[i]->wait();
  //emit that all workers are finished//
  emit finished();
}
