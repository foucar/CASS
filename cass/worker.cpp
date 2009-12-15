#include "worker.h"
#include "analyzer.h"
#include "format_converter.h"
#include "database.h"
#include "remi_event.h"

cass::Worker::Worker(lmf::RingBuffer<cass::CASSEvent,4> &ringbuffer, QObject *parent)
  :QThread(parent),
    _ringbuffer(ringbuffer),
    _analyzer(cass::Analyzer::instance()),
    _converter(cass::FormatConverter::instance()),
    _database(new cass::database::Database()),
    _quit(false)
{
}

cass::Worker::~Worker()
{
  delete _database;
  _converter->destroy();
  _analyzer->destroy();
}

void cass::Worker::end()
{
  std::cout << "worker \""<<std::hex<<this<<std::dec <<"\" got signal to close"<<std::endl;
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
//    std::cout <<"retrieving a new cassevent from ringbuffer"<<std::endl;
    //retrieve a new cassevent from the eventbuffer//
    _ringbuffer.nextToProcess(cassevent, 1000);
//    std::cout <<"done retrieving"<<std::endl;

    //when the cassevent has been set work on it//
    if (cassevent)
    {
      //convert the datagrambuffer to something useful//
      //this will tell us whether this transition should be analyzed further//
//      std::cout << "worker converts"<<std::endl;
      const bool shouldBeAnalyzed  = _converter->processDatagram(cassevent);
//      std::cout << "done converting"<<std::endl;

      //when the formatconverter told us, then analyze the cassevent//
      if (shouldBeAnalyzed)
      {
 //       std::cout << "worker analyses"<<std::endl;
        _analyzer->processEvent(cassevent);
 //       std::cout << "done analysing"<<std::endl;
      }

      //here the usercode that will work on the cassevent will be called//
      if (shouldBeAnalyzed)
      {
//        std::cout << "worker adds event to database"<<std::endl;
        _database->add(cassevent);
//        std::cout << "done adding"<<std::endl;
      }
//      std::cout << "putting event back to ringbuffer"<<std::endl;
      //we are done, so tell the ringbuffer//
      _ringbuffer.doneProcessing(cassevent);
//      std::cout << "done putting"<<std::endl;

      //tell outside that we are done, when we should have anlyzed the event//
      if (shouldBeAnalyzed) emit processedEvent();
    }
  }
  std::cout <<"worker \""<<std::hex<<this<<std::dec <<"\" is closing down"<<std::endl;
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

