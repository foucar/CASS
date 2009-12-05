#include "worker.h"
#include "analyzer.h"
#include "format_converter.h"
#include "database.h"
#include "remi_event.h"

cass::Worker::Worker(lmf::RingBuffer<cass::CASSEvent,4> &ringbuffer, QObject *parent)
  :QThread(parent),
    _ringbuffer(ringbuffer),
    _analyzer(new cass::Analyzer()),
    _converter(cass::FormatConverter::instance()),
    _database(new cass::database::Database()),
    _quit(false)
{
}

cass::Worker::~Worker()
{
}

void cass::Worker::end()
{
  std::cout << "worker got signal to close"<<std::endl;
  _quit = true;
}

void cass::Worker::run()
{
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
      //convert the datagrambuffer to something useful//
      //this will tell us whether this transition should be analyzed further//
      const bool shouldBeAnalyzed  = _converter->processDatagram(cassevent);

      //when the formatconverter told us, then analyze the cassevent//
      if (shouldBeAnalyzed) _analyzer->processEvent(cassevent);

      //here the usercode that will work on the cassevent will be called//
      if (shouldBeAnalyzed) _database->add(cassevent);

      //we are done, so tell the ringbuffer//
      _ringbuffer.doneProcessing(cassevent);

      //tell outside that we are done, when we should have anlyzed the event//
      if (shouldBeAnalyzed) emit processedEvent();
    }
  }
  std::cout <<"worker is closing down"<<std::endl;
}

void cass::Worker::loadSettings()
{
  _analyzer->loadSettings();
  _database->loadSettings();
}

void cass::Worker::saveSettings()
{
  _analyzer->saveSettings();
  _database->saveSettings();
}

